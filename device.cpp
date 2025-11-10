#include <QtEndian>
#include "actions/common.h"
#include "actions/other.h"
#include "actions/ptvo.h"
#include "properties/common.h"
#include "properties/ias.h"
#include "properties/other.h"
#include "properties/ptvo.h"
#include "controller.h"
#include "logger.h"

void OTA::refresh(const QDir &dir)
{
    QList <QString> list = dir.entryList(QDir::Files);
    QByteArray signature = QByteArray::fromHex("1ef1ee0b"), buffer;
    otaFileHeaderStruct header;
    int position;

    for (int i = 0; i < list.count(); i++)
    {
        QFile file(QString("%1/%2").arg(dir.path(), list.at(i)));
        bool check = false;

        m_fileName.clear();
        m_fileVersion = 0;
        m_imageOffset = 0;

        if (!file.open(QFile::ReadOnly))
            continue;

        buffer = file.read(OTA_FILE_BUFFER_SIZE);
        position = buffer.indexOf(signature);

        if (position >= 0 && position + sizeof(header) <= static_cast <size_t> (file.size()))
        {
            file.seek(position);
            memcpy(&header, file.read(sizeof(header)).constData(), sizeof(header));
            check = true;
        }

        if (check && qFromLittleEndian(header.manufacturerCode) == m_manufacturerCode && qFromLittleEndian(header.imageType) == m_imageType && qFromLittleEndian(header.imageSize) <= file.size())
        {
            m_fileName = list.at(i);
            m_fileVersion = qFromLittleEndian(header.fileVersion);
            m_imageSize = qFromLittleEndian(header.imageSize);
            break;
        }
    }
}

bool EndpointObject::bindingExists(quint16 clusterId)
{
    for (int i = 0; i < m_bindings.count(); i++)
        if (m_bindings.at(i)->clusterId() == clusterId)
            return true;

    return false;
}

int DeviceObject::checkVersion(const QString &version)
{
    QList <QString> a = m_firmware.split('.'), b = version.split('.');

    for (int i = 0; i < qMax(a.count(), b.count()); i++)
    {
        int x = a.value(i).toInt(), y = b.value(i).toInt();

        if (x > y)
            return 1;

        if (x < y)
            return -1;
    }

    return 0;
}

DeviceList::DeviceList(QSettings *config, QObject *parent) : QObject(parent), m_databaseTimer(new QTimer(this)), m_propertiesTimer(new QTimer(this)), m_permitJoin(false)
{
    QFile file(config->value("device/expose", "/usr/share/homed-common/expose.json").toString());

    PropertyObject::registerMetaTypes();
    ActionObject::registerMetaTypes();
    BindingObject::registerMetaTypes();
    ReportingObject::registerMetaTypes();
    PollObject::registerMetaTypes();
    ExposeObject::registerMetaTypes();

    m_databaseFile.setFileName(config->value("device/database", "/opt/homed-zigbee/database.json").toString());
    m_propertiesFile.setFileName(config->value("device/properties", "/opt/homed-zigbee/properties.json").toString());
    m_optionsFile.setFileName(config->value("device/options", "/opt/homed-zigbee/options.json").toString());

    m_otaDir.setPath(config->value("device/ota", "/opt/homed-zigbee/ota").toString());
    m_externalDir.setPath(config->value("device/external", "/opt/homed-zigbee/external").toString());
    m_libraryDir.setPath(config->value("device/library", "/usr/share/homed-zigbee").toString());

    if (file.open(QFile::ReadOnly))
    {
        m_exposeOptions = QJsonDocument::fromJson(file.readAll()).object().toVariantMap();
        file.close();
    }

    m_specialExposes = {"switch", "lock", "light", "cover", "thermostat", "thermostatProgram"};

    connect(m_databaseTimer, &QTimer::timeout, this, &DeviceList::writeDatabase);
    connect(m_propertiesTimer, &QTimer::timeout, this, &DeviceList::writeProperties);

    m_databaseTimer->setSingleShot(true);
    m_propertiesTimer->setSingleShot(true);
}

DeviceList::~DeviceList(void)
{
    m_sync = true;

    writeDatabase();
    writeProperties();
}

void DeviceList::init(void)
{
    QJsonObject json;

    if (!m_databaseFile.open(QFile::ReadOnly))
        return;

    json = QJsonDocument::fromJson(m_databaseFile.readAll()).object();
    unserializeDevices(json.value("devices").toArray());
    m_databaseFile.close();

    if (!m_propertiesFile.open(QFile::ReadOnly))
        return;

    unserializeProperties(QJsonDocument::fromJson(m_propertiesFile.readAll()).object());
    m_propertiesFile.close();
}

void DeviceList::storeDatabase(bool sync)
{
    m_sync = sync;
    m_databaseTimer->start(STORE_DATABASE_DELAY);
}

void DeviceList::storeProperties(void)
{
    m_propertiesTimer->start(STORE_PROPERTIES_DELAY);
}

Device DeviceList::byName(const QString &name)
{
    for (auto it = begin(); it != end(); it++)
        if (it.value()->name() == name)
            return it.value();

    return value(QByteArray::fromHex(name.toUtf8()));
}

Device DeviceList::byNetwork(quint16 networkAddress)
{
    for (auto it = begin(); it != end(); it++)
        if (it.value()->networkAddress() == networkAddress)
            return it.value();

    return Device();
}

Endpoint DeviceList::endpoint(const Device &device, quint8 endpointId)
{
    auto it = device->endpoints().find(endpointId);

    if (it == device->endpoints().end())
        it = device->endpoints().insert(endpointId, Endpoint(new EndpointObject(endpointId, device)));

    return it.value();
}

void DeviceList::setupDevice(const Device &device)
{
    QMap <QString, QVariant> userOptions;
    QList <QDir> list = {m_externalDir, m_libraryDir};
    QString manufacturerName, modelName;

    if (device->logicalType() == LogicalType::Coordinator)
        return;

    if (m_optionsFile.open(QFile::ReadOnly))
    {
        QString ieeeAddress = device->ieeeAddress().toHex(':');
        QJsonObject json = QJsonDocument::fromJson(m_optionsFile.readAll()).object(), options = json.value(json.contains(ieeeAddress) ? ieeeAddress : device->name()).toObject();

        for (auto it = options.begin(); it != options.end(); it++)
        {
            if (it.key().endsWith("Divider") && !it.value().toDouble())
                continue;

            userOptions.insert(it.key(), it.value().toVariant());
        }

        m_optionsFile.close();
    }

    device->setSupported(false);
    device->options().clear();

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        auto i = it.value()->bindings().begin();

        while (i != it.value()->bindings().end())
        {
            if (!i->data()->name().isEmpty())
            {
                i = it.value()->bindings().erase(i);
                continue;
            }

            i++;
        }

        it.value()->timer()->stop();
        it.value()->properties().clear();
        it.value()->actions().clear();
        it.value()->reportings().clear();
        it.value()->polls().clear();
        it.value()->exposes().clear();
    }

    identityHandler(device, manufacturerName, modelName);

    for (auto it = list.begin(); it != list.end() && !device->supported(); it++)
    {
        QList <QString> list = it->entryList(QDir::Files);
        bool check = false;

        for (int i = 0; i < list.count() && !device->supported(); i++)
        {
            QFile file(QString("%1/%2").arg(it->path(), list.at(i)));
            QJsonObject json;
            QJsonArray array;

            if (!file.open(QFile::ReadOnly))
            {
                if (!m_brokenFiles.contains(file.fileName()))
                {
                    logWarning << "Unable to open library file" << file.fileName();
                    m_brokenFiles.append(file.fileName());
                }

                continue;
            }

            json = QJsonDocument::fromJson(file.readAll()).object();
            file.close();

            if (json.isEmpty())
            {
                if (!m_brokenFiles.contains(file.fileName()))
                {
                    logWarning << "Library file" << file.fileName() << "JSON data is empty or not valid";
                    m_brokenFiles.append(file.fileName());
                }

                continue;
            }

            array = json.value(manufacturerName).toArray();
            m_brokenFiles.removeAll(file.fileName());

            if (array.isEmpty())
                continue;

            for (auto it = array.begin(); it != array.end(); it++)
            {
                QJsonObject json = it->toObject();
                QJsonArray modelNames = json.value("modelNames").toArray();
                bool found = modelNames.contains(modelName);

                if (found || (!check && manufacturerName == "TUYA" && modelNames.contains(device->modelName())))
                {
                    QJsonValue endpoinId = json.value("endpointId");
                    QList <QVariant> endpoints = endpoinId.type() == QJsonValue::Array ? endpoinId.toArray().toVariantList() : QList <QVariant> {endpoinId.toInt(1)};

                    if (found)
                        check = true;

                    if (json.contains("options"))
                    {
                        QJsonObject options = json.value("options").toObject();

                        if (endpoinId.type() == QJsonValue::Array)
                            for (auto it = options.begin(); it != options.end(); it++)
                                for (int i = 0; i < endpoints.count(); i++)
                                    device->options().insert(QString("%1_%2").arg(it.key(), endpoints.at(i).toString()), it.value().toVariant());
                        else
                            device->options().insert(options.toVariantMap());
                    }

                    device->options().insert(userOptions);

                    for (int i = 0; i < endpoints.count(); i++)
                        setupEndpoint(endpoint(device, static_cast <quint8> (endpoints.at(i).toInt())), json, endpoinId.type() == QJsonValue::Array);

                    if (json.contains("description"))
                        device->setDescription(json.value("description").toString());

                    device->setSupported(true);
                }
            }
        }
    }

    if (!device->supported())
    {
        if (device->manufacturerName() != "ptvo.info")
        {
            logWarning << device << "manufacturer name" << device->manufacturerName() << "and model name" << device->modelName() << "not found in library";
            device->setDescription(QString("%1/%2").arg(device->manufacturerName(), device->modelName()));
        }
        else
        {
            recognizePtvoDevice(device);
            device->setSupported(true);
        }

        recognizeDevice(device);
    }

    if (device->options().contains("logicalType"))
        device->setLogicalType(static_cast <LogicalType> (device->options().value("logicalType").toInt()));

    if (device->options().contains("powerSource"))
        device->setPowerSource(static_cast <quint8> (device->options().value("powerSource").toInt()));

    if (device->options().value("ikeaBatterty").toBool() && device->checkVersion("2.4.0") < 0)
        device->options().insert("battery", QMap <QString, QVariant> {{"undivided", true}});

    addCommonExposes(device);
}

void DeviceList::removeDevice(const Device &device)
{
    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (!it.value()->timer()->isActive())
            continue;

        disconnect(it.value()->timer(), &QTimer::timeout, this, &DeviceList::updateEndpoint);
        it.value()->timer()->stop();
    }

    if (device->name() != device->ieeeAddress().toHex(':'))
    {
        device->setActive(true);
        device->setRemoved(true);
        device->setInterviewStatus(InterviewStatus::NodeDescriptor);
        return;
    }

    remove(device->ieeeAddress());
}

void DeviceList::identityHandler(const Device &device, QString &manufacturerName, QString &modelName)
{
    QList <QString> lumi =
    {
        "aqara",
        "Aqara",
        "XIAOMI"
    };

    QList <QString> orvibo =
    {
        "131c854783bc45c9b2ac58088d09571c",
        "585fdfb8c2304119a2432e9845cf2623",
        "52debf035a1b4a66af56415474646c02",
        "700ae5aab3414ec09c1872efe7b8755a",
        "75a4bfe8ef9c4350830a25d13e3ab068",
        "895a2d80097f4ae2b2d40500d5e03dcc",
        "b2e57a0f606546cd879a1a54790827d6",
        "b7e305eb329f497384e966fe3fb0ac69",
        "c670e231d1374dbc9e3c6a9fffbd0ae6",
        "da2edf1ded0d44e1815d06f45ce02029",
        "e70f96b3773a4c9283c6862dbafb6a99",
        "fdd76effa0e146b4bdafa0c203a37192"
    };

    manufacturerName = device->manufacturerName();
    modelName = device->modelName();

    if (lumi.contains(manufacturerName))
    {
        manufacturerName = "LUMI";
        return;
    }

    if (orvibo.contains(modelName))
    {
        manufacturerName = "ORVIBO";
        return;
    }

    if (manufacturerName.contains("efekta", Qt::CaseInsensitive))
    {
        manufacturerName = "EFEKTA";
        return;
    }

    if (manufacturerName == "GLEDOPTO" && modelName == "GL-C-007")
    {
        QMap <quint8, quint16> map;

        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
            map.insert(it.key(), it.value()->deviceId());

        if (map == QMap <quint8, quint16> {{0x0B, 0x0210}, {0x0D, 0x0210}} || map == QMap <quint8, quint16> {{0x0B, 0x0210}, {0x0C, 0x0102}, {0x0D, 0xE15E}})
            modelName = "GL-C-007-1ID";
        else if (map == QMap <quint8, quint16> {{0x0B, 0x0210}, {0x0D, 0xE15E}, {0x0F, 0x0100}})
            modelName = "GL-C-007-2ID";
        else if (map == QMap <quint8, quint16> {{0x0B, 0x0210}, {0x0D, 0xE15E}, {0x0F, 0x0220}} || map == QMap <quint8, quint16> {{0x0B, 0x0210}, {0x0C, 0x0102}, {0x0D, 0xE15E}, {0x0F, 0x0100}})
            modelName = "GL-C-008-2ID";

        return;
    }

    if (QRegExp("^TS\\d{3}[0-9EF][AB]{0,1}$").exactMatch(modelName) || QRegExp("^_TZ[2,3,E]\\d{3}_\\S+$").exactMatch(manufacturerName) || manufacturerName.startsWith("_TYZB01_") || manufacturerName.startsWith("TUYA"))
    {
        device->options().insert("tuyaMagic", true);

        if (modelName == "TS0601")
            device->options().insert("tuyaDataQuery", true);

        modelName = manufacturerName;
        manufacturerName = "TUYA";
    }
}

void DeviceList::addCommonExposes(const Device &device)
{
    Expose messageCount(new SensorObject("messageCount")), linkQuality(new SensorObject("linkQuality"));

    if (!device->endpoints().count())
        device->endpoints().insert(1, Endpoint(new EndpointObject(1, device)));

    if (!m_debounce)
    {
        messageCount->setParent(device->endpoints().last().data());
        device->endpoints().last()->exposes().append(messageCount);
    }

    linkQuality->setParent(device->endpoints().last().data());
    device->endpoints().last()->exposes().append(linkQuality);
}

void DeviceList::setupEndpoint(const Endpoint &endpoint, const QJsonObject &json, bool multiple)
{
    const Device &device = endpoint->device();
    QJsonArray properties = json.value("properties").toArray(), actions = json.value("actions").toArray(), bindings = json.value("bindings").toArray(), reportings = json.value("reportings").toArray(), polls = json.value("polls").toArray(), exposes = json.value("exposes").toArray();
    QMap <QString, QVariant> customCommands = device->options().value(QString("customCommands_%2").arg(QString::number(endpoint->id())), device->options().value("customCommands")).toMap(), customAttributes = device->options().value(QString("customAttributes_%2").arg(QString::number(endpoint->id())), device->options().value("customAttributes")).toMap();
    int bindingIndex = 0;
    bool startTimer = false;

    for (auto it = properties.begin(); it != properties.end(); it++)
    {
        int type = QMetaType::type(QString(it->toString()).append("Property").toUtf8());

        if (type)
        {
            Property property(reinterpret_cast <PropertyObject*> (QMetaType::create(type)));

            property->setParent(endpoint.data());
            property->setMultiple(multiple);
            property->setTimeout(static_cast <quint32> (device->options().value(QString(property->name()).append("ResetTimeout")).toDouble() * 1000));

            if (property->timeout() || property->clusters().contains(CLUSTER_IAS_WD))
                startTimer = true;

            endpoint->properties().append(property);
            continue;
        }

        logWarning << device << endpoint << "property" << it->toString() << "unrecognized";
    }

    for (auto it = actions.begin(); it != actions.end(); it++)
    {
        int type = QMetaType::type(QString(it->toString()).append("Action").toUtf8());

        if (type)
        {
            Action action(reinterpret_cast <ActionObject*> (QMetaType::create(type)));
            action->setParent(endpoint.data());
            endpoint->actions().append(action);
            continue;
        }

        logWarning << device << endpoint << "action" << it->toString() << "unrecognized";
    }

    for (auto it = bindings.begin(); it != bindings.end(); it++)
    {
        int type = QMetaType::type(QString(it->toString()).append("Binding").toUtf8());

        if (type)
        {
            Binding binding(reinterpret_cast <BindingObject*> (QMetaType::create(type)));
            endpoint->bindings().insert(bindingIndex++, binding);
            continue;
        }

        logWarning << device << endpoint << "binding" << it->toString() << "unrecognized";
    }

    for (auto it = reportings.begin(); it != reportings.end(); it++)
    {
        int type = QMetaType::type(QString(it->toString()).append("Reporting").toUtf8());

        if (type)
        {
            Reporting reporting(reinterpret_cast <ReportingObject*> (QMetaType::create(type)));
            endpoint->reportings().append(reporting);
            continue;
        }

        logWarning << device << endpoint << "reporting" << it->toString() << "unrecognized";
    }

    for (auto it = polls.begin(); it != polls.end(); it++)
    {
        int type = QMetaType::type(QString(it->toString()).append("Poll").toUtf8());

        if (type)
        {
            Poll poll(reinterpret_cast <PollObject*> (QMetaType::create(type)));
            endpoint->polls().append(poll);
            continue;
        }

        logWarning << device << endpoint << "poll" << it->toString() << "unrecognized";
    }

    for (auto it = customCommands.begin(); it != customCommands.end(); it++)
    {
        QMap <QString, QVariant> option = it.value().toMap();
        quint16 clusterId = static_cast <quint16> (option.value("clusterId").toInt());
        Property property(new PropertiesCustom::Command(it.key(), clusterId));

        property->setParent(endpoint.data());
        property->setMultiple(multiple);
        endpoint->properties().append(property);

        if (option.value("binding").toBool() && !endpoint->bindingExists(clusterId))
            endpoint->bindings().append(Binding(new BindingObject(it.key(), clusterId)));
    }

    for (auto it = customAttributes.begin(); it != customAttributes.end(); it++)
    {
        QMap <QString, QVariant> option = it.value().toMap();
        QString type = option.value("type").toString();
        quint16 clusterId = static_cast <quint16> (option.value("clusterId").toInt()), attributeId = static_cast <quint16> (option.value("attributeId").toInt());
        quint8 dataType =static_cast <quint8> (option.value("dataType").toInt());
        double divider = option.value("divider").toDouble();
        Property property;

        if (!dataType)
            continue;

        property = Property(new PropertiesCustom::Attribute(it.key(), type, clusterId, attributeId, dataType, divider));
        property->setParent(endpoint.data());
        property->setMultiple(multiple);
        endpoint->properties().append(property);

        if (option.value("action").toBool())
        {
            Action action(new ActionsCustom::Attribute(it.key(), type, clusterId, static_cast <quint16> (option.value("manufacturerCode").toInt()), attributeId, dataType, divider));
            action->setParent(endpoint.data());
            endpoint->actions().append(action);
        }

        if (option.value("binding").toBool() && !endpoint->bindingExists(clusterId))
            endpoint->bindings().append(Binding(new BindingObject(it.key(), clusterId)));

        if (option.contains("reporting"))
        {
            QMap <QString, QVariant> reporting = option.value("reporting").toMap();
            endpoint->reportings().append(Reporting(new ReportingObject(it.key(), clusterId, attributeId, option.value("dataType").toInt(), reporting.value("minInterval", 0).toInt(), reporting.value("maxInterval", 3600).toInt(), reporting.value("valueChange", 0).toInt())));
        }
    }

    for (auto it = exposes.begin(); it != exposes.end(); it++)
    {
        QString exposeName = it->toString(), itemName = exposeName.split('_').value(0), optionName = multiple ? QString("%1_%2").arg(itemName, QString::number(endpoint->id())) : exposeName;
        QMap <QString, QVariant> option = m_exposeOptions.value(itemName).toMap();
        Expose expose;
        int type;

        if (!m_specialExposes.contains(itemName))
        {
            option.insert(device->options().value(optionName).toMap());
            device->options().insert(optionName, option);
        }

        type = QMetaType::type(QString(m_specialExposes.contains(itemName) ? itemName : option.value("type").toString()).append("Expose").toUtf8());

        expose = Expose(type ? reinterpret_cast <ExposeObject*> (QMetaType::create(type)) : new ExposeObject(itemName));
        expose->setName(exposeName);
        expose->setParent(endpoint.data());
        expose->setMultiple(multiple);

        endpoint->exposes().append(expose);
    }

    if (!endpoint->polls().isEmpty())
    {
        quint32 pollInterval = static_cast <quint32> (device->options().value(multiple ? QString("pollInterval_%1").arg(endpoint->id()) : "pollInterval").toDouble() * 1000);

        for (int i = 0; i < endpoint->polls().count(); i++)
        {
            const Poll &poll = endpoint->polls().at(i);
            emit pollRequest(endpoint.data(), poll);
        }

        if (pollInterval)
        {
            endpoint->setPollInterval(pollInterval);
            endpoint->setPollTime(QDateTime::currentSecsSinceEpoch());
            startTimer = true;
        }
    }

    if (!startTimer)
        return;

    connect(endpoint->timer(), &QTimer::timeout, this, &DeviceList::updateEndpoint, Qt::UniqueConnection);
    endpoint->timer()->start(UPDATE_ENDPOINT_INTERVAL);
}

void DeviceList::recognizeDevice(const Device &device)
{
    QList <quint16> list =
    {
        CLUSTER_POWER_CONFIGURATION,
        CLUSTER_ON_OFF,
        CLUSTER_ANALOG_INPUT,
        CLUSTER_ANALOG_OUTPUT,
        CLUSTER_WINDOW_COVERING,
        CLUSTER_THERMOSTAT,
        CLUSTER_FAN_CONTROL,
        CLUSTER_SMART_ENERGY_METERING,
        CLUSTER_ELECTRICAL_MEASUREMENT,
        CLUSTER_IAS_ZONE
    };

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        for (int i = 0; i < it.value()->inClusters().count(); i++)
        {
            quint16 clusterId = it.value()->inClusters().at(i);

            if (it.value()->meta().value("ptvo").toBool() && list.contains(clusterId))
                continue;

            switch (clusterId)
            {
                case CLUSTER_POWER_CONFIGURATION:

                    if (device->batteryPowered())
                    {
                        it.value()->properties().append(Property(new Properties::BatteryPercentage));
                        it.value()->bindings().append(Binding(new Bindings::Battery));
                        it.value()->reportings().append(Reporting(new Reportings::BatteryPercentage));
                        it.value()->exposes().append(Expose(new SensorObject("battery")));
                    }

                    break;

                case CLUSTER_ON_OFF:

                    if (!device->batteryPowered())
                    {
                        it.value()->properties().append(Property(new Properties::Status));
                        it.value()->actions().append(Action(new Actions::Status));
                        it.value()->bindings().append(Binding(new Bindings::Status));
                        it.value()->reportings().append(Reporting(new Reportings::Status));
                        it.value()->exposes().append(recognizeLight(device) ? Expose(new LightObject) : Expose(new SwitchObject));
                    }

                    break;

                case CLUSTER_LEVEL_CONTROL:

                    if (!device->batteryPowered())
                    {
                        QList <QVariant> options = device->options().value(QString("light_%1").arg(it.key())).toList();

                        it.value()->properties().append(Property(new Properties::Level));
                        it.value()->actions().append(Action(new Actions::Level));
                        it.value()->bindings().append(Binding(new Bindings::Level));
                        it.value()->reportings().append(Reporting(new Reportings::Level));

                        options.append("level");
                        device->options().insert(QString("light_%1").arg(it.key()), options);
                    }

                    break;

                case CLUSTER_WINDOW_COVERING:
                    it.value()->properties().append(Property(new Properties::CoverPosition));
                    it.value()->actions().append(Action(new Actions::CoverStatus));
                    it.value()->actions().append(Action(new Actions::CoverPosition));
                    it.value()->bindings().append(Binding(new Bindings::Cover));
                    it.value()->reportings().append(Reporting(new Reportings::CoverPosition));
                    it.value()->exposes().append(Expose(new CoverObject));
                    break;

                case CLUSTER_THERMOSTAT:
                    it.value()->properties().append(Property(new Properties::Thermostat));
                    it.value()->actions().append(Action(new Actions::Thermostat));
                    it.value()->bindings().append(Binding(new Bindings::Thermostat));
                    it.value()->reportings().append(Reporting(new Reportings::Thermostat));
                    it.value()->exposes().append(Expose(new ThermostatObject));
                    device->options().insert({{QString("targetTemperature_%1").arg(it.key()), QMap <QString, QVariant> {{"min", 5}, {"max", 35}, {"step", 0.1}, {"unit", "°C"}}}, {QString("systemMode_%1").arg(it.key()), QMap <QString, QVariant> {{"enum", QList <QVariant> {"off", "auto", "heat"}}}}});
                    break;

                case CLUSTER_FAN_CONTROL:
                    it.value()->properties().append(Property(new Properties::FanMode));
                    it.value()->actions().append(Action(new Actions::FanMode));
                    it.value()->bindings().append(Binding(new Bindings::Fan));
                    it.value()->exposes().append(Expose(new SelectObject("fanMode")));
                    device->options().insert(QString("fanMode_%1").arg(it.key()), QMap <QString, QVariant> {{"enum", QList <QVariant> {"off", "low", "medium", "high"}}});
                    break;

                case CLUSTER_COLOR_CONTROL:

                    if (!device->batteryPowered() && it.value()->meta().value("colorCapabilities").isValid())
                    {
                        QList <QVariant> options = device->options().value(QString("light_%1").arg(it.key())).toList();
                        quint16 capabilities = static_cast <quint16> (it.value()->meta().value("colorCapabilities").toInt());

                        if (!capabilities || capabilities > 0x001F)
                            break;

                        if (capabilities & 0x0009)
                        {
                            it.value()->properties().append(capabilities & 0x0008 ? Property(new Properties::ColorXY) : Property(new Properties::ColorHS));
                            it.value()->actions().append(capabilities & 0x0008 ? Action(new Actions::ColorXY) : Action(new Actions::ColorHS));
                            it.value()->reportings().append(capabilities & 0x0008 ? Reporting(new Reportings::ColorXY) : Reporting(new Reportings::ColorHS));
                            options.append("color");
                        }

                        if (capabilities & 0x0010)
                        {
                            it.value()->properties().append(Property(new Properties::ColorTemperature));
                            it.value()->actions().append(Action(new Actions::ColorTemperature));
                            it.value()->reportings().append(Reporting(new Reportings::ColorTemperature));
                            options.append("colorTemperature");
                        }

                        if (options.contains("color") && options.contains("colorTemperature"))
                        {
                            it.value()->properties().append(Property(new Properties::ColorMode));
                            options.append("colorMode");
                        }

                        it.value()->bindings().append(Binding(new Bindings::Color));
                        device->options().insert(QString("light_%1").arg(it.key()), options);
                        break;
                    }

                    break;

                case CLUSTER_ILLUMINANCE_MEASUREMENT:
                    it.value()->properties().append(Property(new Properties::Illuminance));
                    it.value()->bindings().append(Binding(new Bindings::Illuminance));
                    it.value()->reportings().append(Reporting(new Reportings::Illuminance));
                    it.value()->exposes().append(Expose(new SensorObject("illuminance")));
                    break;

                case CLUSTER_TEMPERATURE_MEASUREMENT:
                    it.value()->properties().append(Property(new Properties::Temperature));
                    it.value()->bindings().append(Binding(new Bindings::Temperature));
                    it.value()->reportings().append(Reporting(new Reportings::Temperature));
                    it.value()->exposes().append(Expose(new SensorObject("temperature")));
                    break;

                case CLUSTER_PRESSURE_MEASUREMENT:
                    it.value()->properties().append(Property(new Properties::Pressure));
                    it.value()->bindings().append(Binding(new Bindings::Pressure));
                    it.value()->reportings().append(Reporting(new Reportings::Pressure));
                    it.value()->exposes().append(Expose(new SensorObject("pressure")));
                    break;

                case CLUSTER_HUMIDITY_MEASUREMENT:
                    it.value()->properties().append(Property(new Properties::Humidity));
                    it.value()->bindings().append(Binding(new Bindings::Humidity));
                    it.value()->reportings().append(Reporting(new Reportings::Humidity));
                    it.value()->exposes().append(Expose(new SensorObject("humidity")));
                    break;

                case CLUSTER_OCCUPANCY_SENSING:
                    it.value()->properties().append(Property(new Properties::Occupancy));
                    it.value()->bindings().append(Binding(new Bindings::Occupancy));
                    it.value()->reportings().append(Reporting(new Reportings::Occupancy));
                    it.value()->exposes().append(Expose(new BinaryObject("occupancy")));
                    break;

                case CLUSTER_MOISTURE_MEASUREMENT:
                    it.value()->properties().append(Property(new Properties::Moisture));
                    it.value()->bindings().append(Binding(new Bindings::Moisture));
                    it.value()->reportings().append(Reporting(new Reportings::Moisture));
                    it.value()->exposes().append(Expose(new SensorObject("moisture")));
                    break;

                case CLUSTER_CO2_CONCENTRATION:
                    it.value()->properties().append(Property(new Properties::CO2));
                    it.value()->bindings().append(Binding(new Bindings::CO2));
                    it.value()->reportings().append(Reporting(new Reportings::CO2));
                    it.value()->exposes().append(Expose(new SensorObject("co2")));
                    break;

                case CLUSTER_PM25_CONCENTRATION:
                    it.value()->properties().append(Property(new Properties::PM25));
                    it.value()->bindings().append(Binding(new Bindings::PM25));
                    it.value()->reportings().append(Reporting(new Reportings::PM25));
                    it.value()->exposes().append(Expose(new SensorObject("pm25")));
                    break;

                case CLUSTER_SMART_ENERGY_METERING:
                    it.value()->properties().append(Property(new Properties::Energy));
                    it.value()->bindings().append(Binding(new Bindings::Energy));
                    it.value()->reportings().append(Reporting(new Reportings::Energy));
                    it.value()->exposes().append(Expose(new SensorObject("energy")));
                    device->options().insert(QString("energyDivider_%1").arg(it.key()), 1000);
                    break;

                case CLUSTER_ELECTRICAL_MEASUREMENT:

                    it.value()->properties().append(Property(new Properties::Voltage));
                    it.value()->reportings().append(Reporting(new Reportings::Voltage));
                    it.value()->exposes().append(Expose(new SensorObject("voltage")));
                    device->options().insert({{QString("acVoltageDivider_%1").arg(it.key()), 100}, {QString("dcVoltageDivider_%1").arg(it.key()), 100}});

                    it.value()->properties().append(Property(new Properties::Current));
                    it.value()->reportings().append(Reporting(new Reportings::Current));
                    it.value()->exposes().append(Expose(new SensorObject("current")));
                    device->options().insert({{QString("acCurrentDivider_%1").arg(it.key()), 1000}, {QString("dcCurrentDivider_%1").arg(it.key()), 1000}});

                    it.value()->properties().append(Property(new Properties::Power));
                    it.value()->reportings().append(Reporting(new Reportings::Power));
                    it.value()->exposes().append(Expose(new SensorObject("power")));
                    device->options().insert({{QString("acPowerDivider_%1").arg(it.key()), 10}, {QString("dcPowerDivider_%1").arg(it.key()), 10}});

                    it.value()->bindings().append(Binding(new Bindings::Power));
                    break;

                case CLUSTER_IAS_ZONE:

                    switch (static_cast <quint16> (it.value()->meta().value("zoneType").toInt()))
                    {
                        case 0x000D:
                            it.value()->properties().append(Property(new PropertiesIAS::Occupancy));
                            it.value()->exposes().append(Expose(new BinaryObject("occupancy")));
                            break;

                        case 0x0015:
                            it.value()->properties().append(Property(new PropertiesIAS::Contact));
                            it.value()->exposes().append(Expose(new BinaryObject("contact")));
                            break;

                        case 0x002A:
                            it.value()->properties().append(Property(new PropertiesIAS::WaterLeak));
                            it.value()->exposes().append(Expose(new BinaryObject("waterLeak")));
                            break;

                        case 0x002D:
                            it.value()->properties().append(Property(new PropertiesIAS::Vibration));
                            it.value()->exposes().append(Expose(new BinaryObject("vibration")));
                            break;

                        default:
                            it.value()->properties().append(Property(new PropertiesIAS::Alarm));
                            it.value()->exposes().append(Expose(new BinaryObject("alarm")));
                            break;
                    }

                    it.value()->exposes().append(Expose(new BinaryObject("batteryLow")));
                    it.value()->exposes().append(Expose(new BinaryObject("tamper")));
                    break;
            }
        }
    }

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (device->manufacturerName() == "ptvo.info")
        {
            it.value()->meta().remove("ptvo");
            it.value()->bindings().clear();
        }

        if (!it.value()->properties().isEmpty())
        {
            QList <QString> list;

            for (int i = 0; i < it.value()->properties().count(); i++)
            {
                const Property &property = it.value()->properties().at(i);

                property->setParent(it.value().data());
                recognizeMultipleProperty(device, it.value(), property);

                if (list.contains(property->name()))
                    continue;

                list.append(property->name());
            }

            if (!device->supported())
                logInfo << device << it.value() << "properties recognized:" << list.join(", ").toUtf8().constData();
        }

        if (!it.value()->actions().isEmpty())
        {
            QList <QString> list;

            for (int i = 0; i < it.value()->actions().count(); i++)
            {
                const Action &action = it.value()->actions().at(i);

                action->setParent(it.value().data());

                if (list.contains(action->name()))
                    continue;

                list.append(action->name());
            }

            if (!device->supported())
                logInfo << device << it.value() << "actions recognized:" << list.join(", ").toUtf8().constData();
        }

        if (!it.value()->exposes().isEmpty())
        {
            QList <QString> list;

            for (int i = 0; i < it.value()->exposes().count(); i++)
            {
                const Expose &expose = it.value()->exposes().at(i);
                QString name = QString("%1_%2").arg(expose->name(), QString::number(it.key()));

                expose->setParent(it.value().data());
                recognizeMultipleExpose(device, it.value(), expose);

                if (!m_specialExposes.contains(expose->name()))
                {
                    QMap <QString, QVariant> option = device->options().value(name).toMap();
                    option.insert(m_exposeOptions.value(expose->name()).toMap());
                    device->options().insert(name, option);
                }

                if (list.contains(expose->name()))
                    continue;

                list.append(expose->name());
            }

            if (!device->supported())
                logInfo << device << it.value() << "exposes recognized:" << list.join(", ").toUtf8().constData();
        }
    }
}

void DeviceList::recognizePtvoDevice(const Device &device)
{
    QList <QString> typeList = {"*", "#", "Wh", "V", "A", "W", "Hz", "pf"}, binaryList = {"c", "g", "n", "o", "p", "m", "s", "t", "v", "w"}, itemList;
    QString description = endpoint(device, 0x01)->meta().value("ptvoDescription").toString();

    if (description.isEmpty() || description == "unknown")
        return;

    itemList = description.split(0x0d);

    for (int i = 0; i < itemList.count(); i++)
    {
        QString item = itemList.at(i), id;
        QList <QString> valueList = item.split(',');
        Reporting reporting;
        bool offset = item.startsWith("10"), check = false;
        const Endpoint &endpoint = device->endpoints().value(offset ? 0x10 : item.mid(0, 1).toInt(nullptr, 16));

        if (endpoint.isNull())
            continue;

        id = valueList.value(0).mid(offset ? 3 : 2);

        if (id == "UART")
            continue;

        switch (typeList.indexOf(id))
        {
            case 0: // *
                endpoint->properties().append(Property(new Properties::Status));
                endpoint->actions().append(Action(new Actions::Status));
                endpoint->exposes().append(recognizeLight(device) ? Expose(new LightObject) : Expose(new SwitchObject));
                reporting = Reporting(new Reportings::Status);
                break;

            case 1: // #
            {
                QString name = "alarm";

                switch (binaryList.indexOf(valueList.value(1)))
                {
                    case 0: name = "contact"; break;   // c
                    case 1: name = "gas"; break;       // g
                    case 2: name = "noise"; break;     // n
                    case 3: name = "occupancy"; break; // o
                    case 4: name = "occupancy"; break; // p
                    case 5: name = "smoke"; break;     // m
                    case 6: name = "sos"; break;       // s
                    case 7: name = "tamper"; break;    // t
                    case 8: name = "vibration"; break; // v
                    case 9: name = "waterLeak"; break; // w
                }

                endpoint->properties().append(Property(new PropertiesPTVO::Status(name)));
                endpoint->exposes().append(Expose(new BinaryObject(name)));
                reporting = Reporting(new Reportings::Status);
                break;
            }

            case 2: // Wh

                if (!endpoint->inClusters().contains(CLUSTER_SMART_ENERGY_METERING))
                {
                    check = true;
                    break;
                }

                endpoint->properties().append(Property(new Properties::Energy));
                endpoint->exposes().append(Expose(new SensorObject("energy")));
                device->options().insert(QString("energyDivider_%1").arg(endpoint->id()), 1000);
                reporting = Reporting(new Reportings::Energy);
                break;

            case 3: // V

                if (!endpoint->inClusters().contains(CLUSTER_ELECTRICAL_MEASUREMENT))
                {
                    check = true;
                    break;
                }

                endpoint->properties().append(Property(new Properties::Voltage));
                endpoint->exposes().append(Expose(new SensorObject("voltage")));
                device->options().insert({{QString("acVoltageDivider_%1").arg(endpoint->id()), 100}, {QString("dcVoltageDivider_%1").arg(endpoint->id()), 100}});
                reporting = Reporting(new Reportings::Voltage);
                break;

            case 4: // A

                if (!endpoint->inClusters().contains(CLUSTER_ELECTRICAL_MEASUREMENT))
                {
                    check = true;
                    break;
                }

                endpoint->properties().append(Property(new Properties::Current));
                endpoint->exposes().append(Expose(new SensorObject("current")));
                device->options().insert({{QString("acCurrentDivider_%1").arg(endpoint->id()), 1000}, {QString("dcCurrentDivider_%1").arg(endpoint->id()), 1000}});
                reporting = Reporting(new Reportings::Current);
                break;

            case 5: // W

                if (!endpoint->inClusters().contains(CLUSTER_ELECTRICAL_MEASUREMENT))
                {
                    check = true;
                    break;
                }

                endpoint->properties().append(Property(new Properties::Power));
                endpoint->exposes().append(Expose(new SensorObject("power")));
                device->options().insert(QString("dcPowerDivider_%1").arg(endpoint->id()), 10);
                reporting = Reporting(new Reportings::Power);
                break;

            case 6: // Hz

                if (!endpoint->inClusters().contains(CLUSTER_ELECTRICAL_MEASUREMENT))
                {
                    check = true;
                    break;
                }

                endpoint->properties().append(Property(new Properties::Frequency));
                endpoint->exposes().append(Expose(new SensorObject("frequency")));
                device->options().insert(QString("frequencyDivider_%1").arg(endpoint->id()), 10);
                reporting = Reporting(new Reportings::Frequency);
                break;

            case 7: // pf

                if (!endpoint->inClusters().contains(CLUSTER_ELECTRICAL_MEASUREMENT))
                {
                    check = true;
                    break;
                }

                endpoint->properties().append(Property(new Properties::PowerFactor));
                endpoint->exposes().append(Expose(new SensorObject("powerFactor")));
                reporting = Reporting(new Reportings::PowerFactor);
                break;

            default:
                check = true;
                break;
        }

        if (check)
        {
            QString type = valueList.value(0).mid(offset ? 2 : 1, 1).toLower(), title = valueList.value(1), unit = valueList.value(2), optionName;
            QList <QString> list = title.toLower().split(0x20);
            QMap <QString, QVariant> option;

            for (int i = 0; i < list.count(); i++)
            {
                QString part = list.at(i);

                if (i)
                    part.replace(0, 1, part.at(0).toUpper());

                optionName.append(part);
            }

            endpoint->properties().append(Property(new PropertiesPTVO::AnalogInput("analogInput", id)));
            option = m_exposeOptions.value(optionName).toMap();
            option.insert("title", title);

            if (type == "*" || type == "w")
            {
                endpoint->actions().append(Action(new ActionsPTVO::AnalogInput("analogInput")));
                endpoint->exposes().append(Expose(new NumberObject("analogInput")));
                option.insert("type", "number");
                option.insert("min", -1e9);
                option.insert("max", 1e9);
            }
            else
            {
                endpoint->exposes().append(Expose(new SensorObject("analogInput")));
                option.insert("type", "sensor");
            }

            if (!unit.isEmpty())
                option.insert("unit", unit);

            device->options().insert(QString("analogInput_%2").arg(endpoint->id()), option);
            reporting = Reporting(new Reportings::AnalogInput);
        }

        check = false;

        for (int j = 0; j < endpoint->reportings().count(); j++)
        {
            if (endpoint->reportings().at(j)->name() != reporting->name())
                continue;

            check = true;
            break;
        }

        if (check)
            continue;

        endpoint->meta().insert("ptvo", true);
        endpoint->reportings().append(reporting);
    }

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (it.value()->inClusters().contains(CLUSTER_MULTISTATE_VALUE))
        {
            it.value()->properties().append(Property(new PropertiesPTVO::SerialData));
            it.value()->actions().append(Action(new ActionsPTVO::SerialData));
        }
    }
}

bool DeviceList::recognizeLight(const Device &device)
{
    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        if (it.value()->inClusters().contains(CLUSTER_LEVEL_CONTROL) || it.value()->inClusters().contains(CLUSTER_COLOR_CONTROL))
            return true;

    return false;
}

void DeviceList::recognizeMultipleProperty(const Device &device, const Endpoint &endpoint, const Property &property)
{
    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (it.value() == endpoint)
            continue;

        for (int i = 0; i < it.value()->properties().count(); i++)
        {
            if (it.value()->properties().at(i)->name() == property->name())
            {
                property->setMultiple(true);
                return;
            }
        }
    }
}

void DeviceList::recognizeMultipleExpose(const Device &device, const Endpoint &endpoint, const Expose &expose)
{
    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (it.value() == endpoint)
            continue;

        for (int i = 0; i < it.value()->exposes().count(); i++)
        {
            if (it.value()->exposes().at(i)->name() == expose->name())
            {
                expose->setMultiple(true);
                return;
            }
        }
    }
}

void DeviceList::unserializeDevices(const QJsonArray &devices)
{
    quint16 count = 0;

    for (auto it = devices.begin(); it != devices.end(); it++)
    {
        QJsonObject json = it->toObject();

        if (json.contains("ieeeAddress") && json.contains("networkAddress"))
        {
            Device device(new DeviceObject(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint16> (json.value("networkAddress").toInt()), mqttSafe(json.value("name").toString()), json.value("removed").toBool()));
            QJsonArray endpoints = json.value("endpoints").toArray();

            device->setLogicalType(static_cast <LogicalType> (json.value("logicalType").toInt()));
            device->setManufacturerCode(static_cast <quint16> (json.value("manufacturerCode").toInt()));
            device->setPowerSource(static_cast <quint8> (json.value("powerSource").toInt()));
            device->setFirmware(json.value("firmware").toString());
            device->setLastSeen(json.value("lastSeen").toInt());
            device->setLinkQuality(json.value("linkQuality").toInt());
            device->setVersion(static_cast <quint8> (json.value("version").toInt()));
            device->setManufacturerName(json.value("manufacturerName").toString());
            device->setModelName(json.value("modelName").toString());
            device->setNote(json.value("note").toString());
            device->setActive(json.value("active").toBool());
            device->setDiscovery(json.value("discovery").toBool());
            device->setCloud(json.value("cloud").toBool());

            for (auto it = endpoints.begin(); it != endpoints.end(); it++)
            {
                QJsonObject json = it->toObject();

                if (json.contains("endpointId"))
                {
                    quint8 endpointId = static_cast <quint8> (json.value("endpointId").toInt());
                    Endpoint endpoint(new EndpointObject(endpointId, device));
                    QJsonArray inClusters = json.value("inClusters").toArray(), outClusters = json.value("outClusters").toArray(), bindings = json.value("bindings").toArray(), groups = json.value("groups").toArray();

                    for (auto it = inClusters.begin(); it != inClusters.end(); it++)
                        endpoint->inClusters().append(static_cast <quint16> (it->toInt()));

                    for (auto it = outClusters.begin(); it != outClusters.end(); it++)
                        endpoint->outClusters().append(static_cast <quint16> (it->toInt()));

                    for (auto it = bindings.begin(); it != bindings.end(); it++)
                    {
                        QJsonObject json = it->toObject();

                        if (json.contains("groupId"))
                        {
                            quint16 groupId = qFromLittleEndian <quint16> (json.value("groupId").toInt());
                            endpoint->bindings().append(Binding(new BindingObject(static_cast <quint16> (json.value("clusterId").toInt()), QByteArray(reinterpret_cast <char*> (&groupId), sizeof(groupId)), 0xFF)));
                            continue;
                        }

                        endpoint->bindings().append(Binding(new BindingObject(static_cast <quint16> (json.value("clusterId").toInt()), QByteArray::fromHex(json.value("device").toString().toUtf8()), static_cast <quint8> (json.value("endpointId").toInt()))));
                    }

                    for (auto it = groups.begin(); it != groups.end(); it++)
                    {
                        quint16 groupId = static_cast <quint16> (it->toInt());

                        if (endpoint->groups().contains(groupId))
                            continue;

                        endpoint->groups().append(groupId);
                    }

                    endpoint->setProfileId(static_cast <quint16> (json.value("profileId").toInt()));
                    endpoint->setDeviceId(static_cast <quint16> (json.value("deviceId").toInt()));
                    endpoint->meta().insert(json.value("meta").toObject().toVariantMap());

                    device->endpoints().insert(endpointId, endpoint);
                }
            }

            if (!device->removed())
            {
                QJsonObject ota = json.value("ota").toObject();
                QJsonArray neighbors = json.value("neighbors").toArray();

                if (!ota.isEmpty())
                {
                    device->ota().setManufacturerCode(static_cast <quint16> (ota.value("manufacturerCode").toInt()));
                    device->ota().setImageType(static_cast <quint16> (ota.value("imageType").toInt()));
                    device->ota().setCurrentVersion(static_cast <quint32> (ota.value("currentVersion").toInt()));
                    device->ota().setAvailable();
                    device->ota().refresh(m_otaDir);
                }

                for (auto it = neighbors.begin(); it != neighbors.end(); it++)
                {
                    QJsonObject json = it->toObject();

                    if (!json.contains("networkAddress") || !json.contains("linkQuality"))
                        continue;

                    device->neighbors().insert(static_cast <quint16> (json.value("networkAddress").toInt()), static_cast <quint8> (json.value("linkQuality").toInt()));
                }

                if (json.value("interviewFinished").toBool())
                {
                    device->setInterviewStatus(InterviewStatus::Finished);
                    setupDevice(device);
                }

                count++;
            }

            insert(device->ieeeAddress(), device);
        }
    }

    if (count)
        logInfo << count << "devices loaded";
}

void DeviceList::unserializeProperties(const QJsonObject &properties)
{
    bool check = false;

    for (auto it = begin(); it != end(); it++)
    {
        const Device &device = it.value();
        QJsonObject json = properties.value(device->ieeeAddress().toHex(':')).toObject();

        if (device->removed() || json.isEmpty())
            continue;

        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        {
            for (int i = 0; i < it.value()->properties().count(); i++)
            {
                const Property &property = it.value()->properties().at(i);
                QVariant value = json.value(property->multiple() ? QString("%1_%2").arg(property->name()).arg(it.value()->id()) : property->name()).toVariant();

                if (!value.isValid())
                    continue;

                property->setValue(value);
                check = true;
            }
        }
    }

    if (check)
        logInfo << "Properties restored";
}

QJsonArray DeviceList::serializeDevices(void)
{
    QJsonArray array;

    for (auto it = begin(); it != end(); it++)
    {
        const Device &device = it.value();
        QJsonObject json = {{"ieeeAddress", QString(device->ieeeAddress().toHex(':'))}, {"networkAddress", device->networkAddress()}, {"logicalType", static_cast <quint8> (device->logicalType())}};
        QJsonArray endpoints;

        if (device->name() != device->ieeeAddress().toHex(':'))
            json.insert("name", device->name());

        if (!device->manufacturerName().isEmpty())
            json.insert("manufacturerName", device->manufacturerName());

        if (!device->modelName().isEmpty())
            json.insert("modelName", device->modelName());

        if (!device->firmware().isEmpty())
            json.insert("firmware", device->firmware());

        if (device->logicalType() != LogicalType::Coordinator)
        {
            json.insert("supported", device->supported());
            json.insert("interviewFinished", device->interviewStatus() == InterviewStatus::Finished ? true : false);
            json.insert("manufacturerCode", device->manufacturerCode());
            json.insert("powerSource", device->powerSource());
            json.insert("active", device->active());
            json.insert("discovery", device->discovery());
            json.insert("cloud", device->cloud());

            if (device->removed())
                json.insert("removed", true);

            if (device->lastSeen())
                json.insert("lastSeen", device->lastSeen());

            if (device->linkQuality())
                json.insert("linkQuality", device->linkQuality());

            if (device->version())
                json.insert("version", device->version());

            if (!device->description().isEmpty())
                json.insert("description", device->description());

            if (!device->note().isEmpty())
                json.insert("note", device->note());
        }

        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        {
            QJsonObject json;
            QJsonArray bindings, groups;

            if (!it.value()->inClusters().isEmpty())
            {
                QJsonArray inClusters;

                for (int i = 0; i < it.value()->inClusters().count(); i++)
                    inClusters.append(it.value()->inClusters().at(i));

                json.insert("inClusters", inClusters);
            }

            if (!it.value()->outClusters().isEmpty())
            {
                QJsonArray outClusters;

                for (int i = 0; i < it.value()->outClusters().count(); i++)
                    outClusters.append(it.value()->outClusters().at(i));

                json.insert("outClusters", outClusters);
            }

            for (int i = 0; i < it.value()->bindings().count(); i++)
            {
                const Binding &binding = it.value()->bindings().at(i);

                if (!binding->name().isEmpty())
                    continue;

                if (binding->endpointId() == 0xFF)
                {
                    bindings.append(QJsonObject {{"clusterId", binding->clusterId()}, {"groupId", qFromLittleEndian <quint16> (*(reinterpret_cast <quint16*> (binding->address().data())))}});
                    continue;
                }

                bindings.append(QJsonObject {{"clusterId", binding->clusterId()}, {"device", QString(binding->address().toHex(':'))}, {"endpointId", binding->endpointId()}});
            }

            for (int i = 0; i < it.value()->groups().count(); i++)
                groups.append(it.value()->groups().at(i));

            if (!bindings.isEmpty())
                json.insert("bindings", bindings);

            if (!groups.isEmpty())
                json.insert("groups", groups);

            if (it.value()->profileId())
               json.insert("profileId", it.value()->profileId());

            if (it.value()->deviceId())
                json.insert("deviceId", it.value()->deviceId());

            if (!it.value()->meta().isEmpty())
                json.insert("meta", QJsonObject::fromVariantMap(it.value()->meta()));

            if (!json.isEmpty())
            {
                json.insert("endpointId", it.key());
                endpoints.append(json);
            }
        }

        if (!endpoints.isEmpty())
            json.insert("endpoints", endpoints);

        if (!device->removed())
        {
            if (device->ota().available())
            {
                QJsonObject ota = {{"manufacturerCode", device->ota().manufacturerCode()}, {"imageType", device->ota().imageType()}, {"currentVersion", QJsonValue::fromVariant(device->ota().currentVersion())}, {"running", device->ota().running()}};

                if (!device->ota().fileName().isEmpty())
                {
                    ota.insert("fileName", device->ota().fileName());
                    ota.insert("fileVersion", QJsonValue::fromVariant(device->ota().fileVersion()));
                }

                json.insert("ota", ota);
            }

            if (!device->neighbors().isEmpty())
            {
                QJsonArray neighbors;

                for (auto it = device->neighbors().begin(); it != device->neighbors().end(); it++)
                    neighbors.append(QJsonObject {{"networkAddress", it.key()}, {"linkQuality", it.value()}});

                json.insert("neighbors", neighbors);
            }
        }

        array.append(json);
    }

    return array;
}

QJsonObject DeviceList::serializeProperties(void)
{
    QJsonObject json;

    for (auto it = begin(); it != end(); it++)
    {
        const Device &device = it.value();

        if (!device->removed())
        {
            QJsonObject properties;

            for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
            {
                for (int i = 0; i < it.value()->properties().count(); i++)
                {
                    const Property &property = it.value()->properties().at(i);

                    if (!property->value().isValid())
                        continue;

                    properties.insert(property->multiple() ? QString("%1_%2").arg(property->name()).arg(it.value()->id()) : property->name(), QJsonValue::fromVariant(property->value()));
                }
            }

            if (properties.isEmpty())
                continue;

            json.insert(it.value()->ieeeAddress().toHex(':'), properties);
        }
    }

    return json;
}

void DeviceList::writeDatabase(void)
{
    QJsonObject json = {{"devices", serializeDevices()}, {"names", m_names}, {"permitJoin", m_permitJoin}, {"timestamp", QDateTime::currentSecsSinceEpoch()}, {"version", SERVICE_VERSION}};

    emit statusUpdated(json);

    if (!m_sync)
        return;

    json.remove("names");
    json.remove("permitJoin");
    m_sync = false;

    if (reinterpret_cast <Controller*> (parent())->writeFile(m_databaseFile, QJsonDocument(json).toJson(QJsonDocument::Compact)))
        return;

    logWarning << "Database not stored";
}

void DeviceList::writeProperties(void)
{
    QJsonObject json = serializeProperties();

    if (reinterpret_cast <Controller*> (parent())->writeFile(m_propertiesFile, QJsonDocument(json).toJson(QJsonDocument::Compact)))
        return;

    logWarning << "Properties not stored";
}

void DeviceList::updateEndpoint(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (sender()->parent());
    qint64 time = QDateTime::currentMSecsSinceEpoch();

    if (!endpoint->device()->active())
        return;

    for (int i = 0; i < endpoint->properties().count(); i++)
    {
        const Property &property = endpoint->properties().at(i);

        if (!property->time() || !property->timeout())
            continue;

        if (time - property->time() >= property->timeout())
        {
            QVariant value = property->value();

            property->resetValue();
            property->setTime(0);

            if (property->value() == value)
                continue;

            emit endpointUpdated(endpoint->device().data(), endpoint->id());
        }
    }

    if (!endpoint->pollInterval() || time - endpoint->pollTime() < endpoint->pollInterval())
        return;

    for (int i = 0; i < endpoint->polls().count(); i++)
        emit pollRequest(endpoint, endpoint->polls().at(i));

    endpoint->setPollTime(time);
}
