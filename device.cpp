#include <unistd.h>
#include <QFile>
#include "controller.h"
#include "device.h"
#include "logger.h"

DeviceList::DeviceList(QSettings *config) : m_databaseTimer(new QTimer(this)), m_propertiesTimer(new QTimer(this)), m_permitJoin(false), m_sync(false)
{
    PropertyObject::registerMetaTypes();
    ActionObject::registerMetaTypes();
    BindingObject::registerMetaTypes();
    ReportingObject::registerMetaTypes();
    PollObject::registerMetaTypes();
    DiscoveryObject::registerMetaTypes();

    m_libraryFile.setFileName(config->value("device/library", "/usr/share/homed/zigbee.json").toString());
    m_databaseFile.setFileName(config->value("device/database", "/opt/homed-zigbee/database.json").toString());
    m_propertiesFile.setFileName(config->value("device/properties", "/opt/homed-zigbee/properties.json").toString());
    m_optionsFile.setFileName(config->value("device/options", "/opt/homed-zigbee/options.json").toString());
    m_externalDir.setPath(config->value("device/external", "/opt/homed-zigbee/external").toString());
    m_offsets = config->value("device/offsets", true).toBool();

    connect(m_databaseTimer, &QTimer::timeout, this, &DeviceList::writeDatabase);
    connect(m_propertiesTimer, &QTimer::timeout, this, &DeviceList::writeProperties);

    m_databaseTimer->setSingleShot(true);
    m_propertiesTimer->setSingleShot(true);
}

void DeviceList::init(void)
{
    QJsonObject json;

    if (!m_databaseFile.open(QFile::ReadOnly))
        return;

    json = QJsonDocument::fromJson(m_databaseFile.readAll()).object();
    unserializeDevices(json.value("devices").toArray());
    m_permitJoin = json.value("permitJoin").toBool();
    m_databaseFile.close();

    if (!m_propertiesFile.open(QFile::ReadOnly))
        return;

    unserializeProperties(QJsonDocument::fromJson(m_propertiesFile.readAll()).object());
    m_propertiesFile.close();

    logInfo << "Properties restored";
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

void DeviceList::updateIdentity(const Device &device, QString &manufacturerName, QString &modelName)
{
    QList <QString> lumi = {"aqara", "XIAOMI"}, orvibo = {"\u4e2d\u6027", "\u6b27\u745e", "\u94fe\u878d"};

    manufacturerName = device->manufacturerName();
    modelName = device->modelName();

    if (lumi.contains(manufacturerName))
    {
        manufacturerName = "LUMI";
        return;
    }

    if (orvibo.contains(manufacturerName))
    {
        manufacturerName = "ORVIBO";
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

    if (QRegExp("^TS\\d{3}[0-9F]$").exactMatch(modelName))
    {
        QList <QString> list = {"TS0001", "TS0011", "TS011F", "TS0201", "TS0202", "TS0207", "TS0601"};

        if (list.contains(modelName))
            modelName = manufacturerName;

        manufacturerName = "TUYA";
    }
}

void DeviceList::setupDevice(const Device &device)
{
    QString manufacturerName, modelName;
    QJsonArray array;

    updateIdentity(device, manufacturerName, modelName);

    device->setSupported(false);
    device->options().clear();

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        it.value()->timer()->stop();
        it.value()->properties().clear();
        it.value()->actions().clear();
        it.value()->bindings().clear();
        it.value()->reportings().clear();
        it.value()->polls().clear();
        it.value()->discoveries().clear();
    }

    if (m_externalDir.exists())
    {
        QList <QString> list = m_externalDir.entryList(QDir::Files);

        for (int i = 0; i < list.count(); i++)
        {
            QFile file(QString("%1/%2").arg(m_externalDir.path(), list.at(i)));

            if (!file.open(QFile::ReadOnly))
                continue;

            array = QJsonDocument::fromJson(file.readAll()).object().value(manufacturerName).toArray();
            file.close();

            if (!array.isEmpty())
                break;
        }
    }

    if (array.isEmpty())
    {
        if (!m_libraryFile.open(QFile::ReadOnly))
            logWarning << "Library file" << m_libraryFile.fileName() << "error:" << m_libraryFile.errorString();

        if (m_libraryFile.isOpen())
        {
            array = QJsonDocument::fromJson(m_libraryFile.readAll()).object().value(manufacturerName).toArray();
            m_libraryFile.close();
        }
    }

    if (!array.isEmpty())
    {
        for (auto it = array.begin(); it != array.end(); it++)
        {
            QJsonObject json = it->toObject();
            QJsonArray array = json.value("modelNames").toArray();

            if (array.contains(modelName))
            {
                QJsonValue endpoinId = json.value("endpointId");
                QList <QVariant> list = endpoinId.type() == QJsonValue::Array ? endpoinId.toArray().toVariantList() : QList <QVariant> {endpoinId.toInt(1)};

                if (json.contains("description"))
                    device->setDescription(json.value("description").toString());

                if (json.contains("options"))
                {
                    QJsonObject options = json.value("options").toObject();

                    if (endpoinId.type() == QJsonValue::Array)
                        for (auto it = options.begin(); it != options.end(); it++)
                            for (int i = 0; i < list.count(); i++)
                                device->options().insert(QString("%1-%2").arg(it.key(), list.at(i).toString()), it.value().toVariant());
                    else
                        device->options().insert(options.toVariantMap());
                }

                if (json.contains("properties"))
                {
                    for (int i = 0; i < list.count(); i++)
                        setupEndpoint(endpoint(device, static_cast <quint8> (list.at(i).toInt())), json, endpoinId.type() == QJsonValue::Array);

                    device->setSupported(true);
                }
            }
        }
    }

    if (m_optionsFile.open(QFile::ReadOnly))
    {
        QString key = device->ieeeAddress().toHex(':');
        QJsonObject data = QJsonDocument::fromJson(m_optionsFile.readAll()).object(), options = data.value(data.contains(key) ? key : device->name()).toObject();

        for (auto it = options.begin(); it != options.end(); it++)
        {
            if (!m_offsets && it.key().contains("Offset"))
                continue;

            device->options().insert(it.key(), it.value().toVariant());
        }

        m_optionsFile.close();
    }

    if (!device->supported()) // TODO: recognize if all device endpoints has no properties
    {
        logWarning << "Device" << device->name() << "manufacturer name" << device->manufacturerName() << "and model name" << device->modelName() << "unrecognized";
        device->setDescription(QString("Unsupported Device %1/%2").arg(device->manufacturerName(), device->modelName()));
        recognizeDevice(device);
    }
}

void DeviceList::setupEndpoint(const Endpoint &endpoint, const QJsonObject &json, bool multiple)
{
    Device device = endpoint->device();
    QJsonArray properties = json.value("properties").toArray(), actions = json.value("actions").toArray(), bindings = json.value("bindings").toArray(), reportings = json.value("reportings").toArray(), polls = json.value("polls").toArray(), discoveries = json.value("discoveries").toArray();

    for (auto it = properties.begin(); it != properties.end(); it++)
    {
        int type = QMetaType::type(QString(it->toString()).append("Property").toUtf8());

        if (type)
        {
            Property property(reinterpret_cast <PropertyObject*> (QMetaType::create(type)));
            property->setParent(endpoint.data());
            property->setMultiple(multiple);
            endpoint->properties().append(property);
            continue;
        }

        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "property" << it->toString() << "unrecognized";
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

        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "action" << it->toString() << "unrecognized";
    }

    for (auto it = bindings.begin(); it != bindings.end(); it++)
    {
        int type = QMetaType::type(QString(it->toString()).append("Binding").toUtf8());

        if (type)
        {
            Binding binding(reinterpret_cast <BindingObject*> (QMetaType::create(type)));
            endpoint->bindings().append(binding);
            continue;
        }

        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "binding" << it->toString() << "unrecognized";
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

        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "reporting" << it->toString() << "unrecognized";
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

        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "poll" << it->toString() << "unrecognized";
    }

    for (auto it = discoveries.begin(); it != discoveries.end(); it++)
    {
        int type = QMetaType::type(QString(it->toString()).append("Discovery").toUtf8());

        if (type)
        {
            Discovery discovery(reinterpret_cast <DiscoveryObject*> (QMetaType::create(type)));
            discovery->setParent(endpoint.data());
            discovery->setMultiple(multiple);
            endpoint->discoveries().append(discovery);
            continue;
        }

        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "discovery" << it->toString() << "unrecognized";
    }

    if (!endpoint->polls().isEmpty())
    {
        quint32 pollInterval = static_cast <quint32> (device->options().value("pollInterval").toInt());

        for (int i = 0; i < endpoint->polls().count(); i++)
        {
            const Poll &poll = endpoint->polls().at(i);
            emit pollRequest(endpoint.data(), poll);
        }

        if (pollInterval)
        {
            connect(endpoint->timer(), &QTimer::timeout, this, &DeviceList::pollAttributes, Qt::UniqueConnection);
            endpoint->timer()->start(pollInterval * 1000);
        }
    }
}

void DeviceList::recognizeDevice(const Device &device)
{
    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        for (int i = 0; i < it.value()->inClusters().count(); i++)
        {
            switch (it.value()->inClusters().at(i))
            {
                case CLUSTER_POWER_CONFIGURATION:

                    if (!device->batteryPowered())
                        break;

                    it.value()->properties().append(Property(new Properties::BatteryVoltage));
                    it.value()->properties().append(Property(new Properties::BatteryPercentage));
                    it.value()->discoveries().append(Discovery(new Sensor::Battery));
                    break;

                case CLUSTER_TEMPERATURE_CONFIGURATION:
                    it.value()->properties().append(Property(new Properties::DeviceTemperature));
                    it.value()->discoveries().append(Discovery(new Sensor::Temperature));
                    break;

                case CLUSTER_ON_OFF:

                    if (!device->batteryPowered())
                    {
                        it.value()->properties().append(Property(new Properties::Status));
                        it.value()->actions().append(Action(new Actions::Status));
                        it.value()->discoveries().append(it.value()->inClusters().contains(CLUSTER_LEVEL_CONTROL) || it.value()->inClusters().contains(CLUSTER_COLOR_CONTROL) ? Discovery(new LightObject) : Discovery(new SwitchObject));
                        break;
                    }

                    it.value()->properties().append(Property(new Properties::SwitchAction));
                    break;

                case CLUSTER_LEVEL_CONTROL:

                    if (!device->batteryPowered())
                    {
                        QList <QVariant> options = device->options().value("light").toList();
                        it.value()->properties().append(Property(new Properties::Level));
                        it.value()->actions().append(Action(new Actions::Level));
                        options.append("level");
                        device->options().insert("light", options);
                        break;
                    }

                    it.value()->properties().append(Property(new Properties::LevelAction));
                    break;

                case CLUSTER_COLOR_CONTROL:

                    if (!device->batteryPowered() && it.value()->colorCapabilities() && it.value()->colorCapabilities() <= 0x001F)
                    {
                        QList <QVariant> options = device->options().value("light").toList();

                        if (it.value()->colorCapabilities() & 0x0001)
                        {
                            it.value()->properties().append(Property(new Properties::ColorHS));
                            it.value()->actions().append(Action(new Actions::ColorHS));
                            options.append("color");
                        }
                        else if (it.value()->colorCapabilities() & 0x0008)
                        {
                            it.value()->properties().append(Property(new Properties::ColorXY));
                            it.value()->actions().append(Action(new Actions::ColorXY));
                            options.append("color");
                        }

                        if (it.value()->colorCapabilities() & 0x0010)
                        {
                            it.value()->properties().append(Property(new Properties::ColorTemperature));
                            it.value()->actions().append(Action(new Actions::ColorTemperature));
                            options.append("colorTemperature");
                        }

                        device->options().insert("light", options);
                        break;
                    }

                    it.value()->properties().append(Property(new Properties::ColorAction));
                    break;

                case CLUSTER_ILLUMINANCE_MEASUREMENT:
                    it.value()->properties().append(Property(new Properties::Illuminance));
                    it.value()->discoveries().append(Discovery(new Sensor::Illuminance));
                    break;

                case CLUSTER_TEMPERATURE_MEASUREMENT:
                    it.value()->properties().append(Property(new Properties::Temperature));
                    it.value()->discoveries().append(Discovery(new Sensor::Temperature));
                    break;

                case CLUSTER_RELATIVE_HUMIDITY:
                    it.value()->properties().append(Property(new Properties::Humidity));
                    it.value()->discoveries().append(Discovery(new Sensor::Humidity));
                    break;

                case CLUSTER_OCCUPANCY_SENSING:
                    it.value()->properties().append(Property(new Properties::Occupancy));
                    it.value()->discoveries().append(Discovery(new Binary::Occupancy));
                    break;

                case CLUSTER_SMART_ENERGY_METERING:
                    it.value()->properties().append(Property(new Properties::Energy));
                    it.value()->discoveries().append(Discovery(new Sensor::Energy));
                    break;

                case CLUSTER_ELECTRICAL_MEASUREMENT:
                    it.value()->properties().append(Property(new Properties::Voltage));
                    it.value()->properties().append(Property(new Properties::Current));
                    it.value()->properties().append(Property(new Properties::Power));
                    it.value()->discoveries().append(Discovery(new Sensor::Voltage));
                    it.value()->discoveries().append(Discovery(new Sensor::Current));
                    it.value()->discoveries().append(Discovery(new Sensor::Power));
                    break;

                case CLUSTER_IAS_ZONE:

                    switch (it.value()->zoneType())
                    {
                        case 0x000D:
                            it.value()->properties().append(Property(new PropertiesIAS::Occupancy));
                            it.value()->discoveries().append(Discovery(new Binary::Occupancy));
                            break;

                        case 0x0015:
                            it.value()->properties().append(Property(new PropertiesIAS::Contact));
                            it.value()->discoveries().append(Discovery(new Binary::Contact));
                            break;

                        case 0x0028:
                            it.value()->properties().append(Property(new PropertiesIAS::WaterLeak));
                            it.value()->discoveries().append(Discovery(new Binary::WaterLeak));
                            break;

                        default:
                            it.value()->properties().append(Property(new PropertiesIAS::ZoneStatus));
                            it.value()->discoveries().append(Discovery(new BinaryObject));
                            break;
                    }

                    break;
            }
        }

        if (!it.value()->properties().isEmpty())
        {
            QList <QString> list;

            for (int i = 0; i < it.value()->properties().count(); i++)
            {
                const Property &property = it.value()->properties().at(i);

                property->setParent(it.value().data());
                property->setMultiple(property->name() != "battery");

                if (list.contains(property->name()))
                    continue;

                list.append(property->name());
            }

            logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", it.value()->id()) << "properties recognized:" << list.join(", ");
        }

        if (!it.value()->actions().isEmpty())
        {
            QList <QString> list;

            for (int i = 0; i < it.value()->actions().count(); i++)
            {
                const Action &action = it.value()->actions().at(i);

                if (list.contains(action->name()))
                    continue;

                list.append(action->name());
            }

            logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", it.value()->id()) << "actions recognized:" << list.join(", ");
        }

        if (!it.value()->discoveries().isEmpty())
        {
            QList <QString> list;

            for (int i = 0; i < it.value()->discoveries().count(); i++)
            {
                const Discovery &discovery = it.value()->discoveries().at(i);

                discovery->setParent(it.value().data());
                discovery->setMultiple(discovery->name() != "battery");

                if (list.contains(discovery->name()))
                    continue;

                list.append(discovery->name());
            }

            logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", it.value()->id()) << "discoveries:" << list.join(", ");
        }
    }
}

void DeviceList::removeDevice(const Device &device)
{
    if (device->name() != device->ieeeAddress().toHex(':'))
    {
        insert(device->ieeeAddress(), Device(new DeviceObject(device->ieeeAddress(), device->networkAddress(), device->name(), true)));
        return;
    }

    remove(device->ieeeAddress());
}

void DeviceList::storeDatabase(void)
{
    m_sync = true;
    m_databaseTimer->start(STORE_DATABASE_DELAY);
}

void DeviceList::storeProperties(void)
{
    m_propertiesTimer->start(STORE_PROPERTIES_DELAY);
}

void DeviceList::unserializeDevices(const QJsonArray &devices)
{
    quint16 count = 0;

    for (auto it = devices.begin(); it != devices.end(); it++)
    {
        QJsonObject json = it->toObject();

        if (json.contains("ieeeAddress") && json.contains("networkAddress"))
        {
            Device device(new DeviceObject(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint16> (json.value("networkAddress").toInt()), json.value("name").toString(), json.value("removed").toBool()));

            if (!device->removed())
            {
                QJsonArray endpointsArray = json.value("endpoints").toArray(), neighborsArray = json.value("neighbors").toArray();

                if (json.value("interviewFinished").toBool() || json.value("ineterviewFinished").toBool()) // TODO: fix this few releases later
                    device->setInterviewFinished();

                device->setLogicalType(static_cast <LogicalType> (json.value("logicalType").toInt()));
                device->setManufacturerCode(static_cast <quint16> (json.value("manufacturerCode").toInt()));
                device->setVersion(static_cast <quint8> (json.value("version").toInt()));
                device->setPowerSource(static_cast <quint8> (json.value("powerSource").toInt()));
                device->setManufacturerName(json.value("manufacturerName").toString());
                device->setModelName(json.value("modelName").toString());
                device->setLastSeen(json.value("lastSeen").toInt());
                device->setLinkQuality(json.value("linkQuality").toInt());

                for (auto it = endpointsArray.begin(); it != endpointsArray.end(); it++)
                {
                    QJsonObject item = it->toObject();

                    if (item.contains("endpointId"))
                    {
                        quint8 endpointId = static_cast <quint8> (item.value("endpointId").toInt());
                        Endpoint endpoint(new EndpointObject(endpointId, device));
                        QJsonArray inClusters = item.value("inClusters").toArray(), outClusters = item.value("outClusters").toArray();

                        endpoint->setProfileId(static_cast <quint16> (item.value("profileId").toInt()));
                        endpoint->setDeviceId(static_cast <quint16> (item.value("deviceId").toInt()));
                        endpoint->setColorCapabilities(static_cast <quint16> (item.value("colorCapabilities").toInt()));
                        endpoint->setZoneType(static_cast <quint16> (item.value("zoneType").toInt()));

                        for (const QJsonValue &clusterId : inClusters)
                            endpoint->inClusters().append(static_cast <quint16> (clusterId.toInt()));

                        for (const QJsonValue &clusterId : outClusters)
                            endpoint->outClusters().append(static_cast <quint16> (clusterId.toInt()));

                        device->endpoints().insert(endpointId, endpoint);
                    }
                }

                for (auto it = neighborsArray.begin(); it != neighborsArray.end(); it++)
                {
                    QJsonObject json = it->toObject();

                    if (!json.contains("networkAddress") || !json.contains("linkQuality"))
                        continue;

                    device->neighbors().insert(static_cast <quint16> (json.value("networkAddress").toInt()), static_cast <quint8> (json.value("linkQuality").toInt()));
                }

                count++;
            }

            if (device->interviewFinished())
                setupDevice(device);

            insert(device->ieeeAddress(), device);
        }
    }

    if (count)
        logInfo << count << "devices loaded";
}

void DeviceList::unserializeProperties(const QJsonObject &properties)
{
    for (auto it = begin(); it != end(); it++)
    {
        const Device &device = it.value();
        QJsonObject json = properties.value(it.value()->ieeeAddress().toHex(':')).toObject();

        if (device->removed() || json.isEmpty())
            continue;

        for (auto it = json.begin(); it != json.end(); it++)
        {
            const Endpoint &endpoint = device->endpoints().value(static_cast <quint8> (it.key().toInt()));
            QJsonObject item = it.value().toObject();

            if (endpoint.isNull())
                continue;

            for (int i = 0; i < endpoint->properties().count(); i++)
            {
                const Property &property = endpoint->properties().at(i);
                QVariant value = item.value(property->name()).toVariant();

                if (!value.isValid())
                    continue;

                property->setValue(value);
            }
        }
    }
}

QJsonArray DeviceList::serializeDevices(void)
{
    QJsonArray array;

    for (auto it = begin(); it != end(); it++)
    {
        const Device &device = it.value();
        QJsonObject json = {{"ieeeAddress", QString(it.value()->ieeeAddress().toHex(':'))}, {"networkAddress", it.value()->networkAddress()}};

        if (!device->removed())
        {
            if (device->name() != device->ieeeAddress().toHex(':'))
                json.insert("name", device->name());

            json.insert("logicalType", static_cast <quint8> (device->logicalType()));

            if (device->logicalType() == LogicalType::Coordinator)
            {
                if (!m_adapterType.isEmpty())
                    json.insert("type", m_adapterType);

                if (!m_adapterVersion.isEmpty())
                    json.insert("version", m_adapterVersion);
            }
            else
            {
                json.insert("supported", device->supported());
                json.insert("interviewFinished", device->interviewFinished());
                json.insert("manufacturerCode", device->manufacturerCode());
                json.insert("powerSource", device->powerSource());

                if (device->version())
                    json.insert("version", device->version());

                if (!device->manufacturerName().isEmpty())
                    json.insert("manufacturerName", device->manufacturerName());

                if (!device->modelName().isEmpty())
                    json.insert("modelName", device->modelName());

                if (device->lastSeen())
                    json.insert("lastSeen", device->lastSeen());

                if (device->linkQuality())
                    json.insert("linkQuality", device->linkQuality());
            }

            if (!device->endpoints().isEmpty())
            {
                QJsonArray array;

                for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
                {
                    QJsonObject item;

                    if (!it.value()->profileId() && !it.value()->deviceId())
                        continue;

                    item.insert("endpointId", it.key());
                    item.insert("profileId", it.value()->profileId());
                    item.insert("deviceId", it.value()->deviceId());

                    if (it.value()->colorCapabilities())
                        item.insert("colorCapabilities", it.value()->colorCapabilities());

                    if (it.value()->zoneType())
                        item.insert("zoneType", it.value()->zoneType());

                    if (!it.value()->inClusters().isEmpty())
                    {
                        QJsonArray inClusters;

                        for (int i = 0; i < it.value()->inClusters().count(); i++)
                            inClusters.append(it.value()->inClusters().at(i));

                        item.insert("inClusters", inClusters);
                    }

                    if (!it.value()->outClusters().isEmpty())
                    {
                        QJsonArray outClusters;

                        for (int i = 0; i < it.value()->outClusters().count(); i++)
                            outClusters.append(it.value()->outClusters().at(i));

                        item.insert("outClusters", outClusters);
                    }

                    array.append(item);
                }

                if (!array.isEmpty())
                    json.insert("endpoints", array);
            }

            if (!device->neighbors().isEmpty())
            {
                QJsonArray array;

                for (auto it = device->neighbors().begin(); it != device->neighbors().end(); it++)
                    array.append(QJsonObject {{"networkAddress", it.key()}, {"linkQuality", it.value()}});

                json.insert("neighbors", array);
            }
        }
        else
        {
            json.insert("name", device->name());
            json.insert("removed", true);
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
        QJsonObject item;

        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        {
            QJsonObject data;

            for (int i = 0; i < it.value()->properties().count(); i++)
            {
                const Property &property = it.value()->properties().at(i);

                if (!property->value().isValid())
                    continue;

                data.insert(property->name(), QJsonValue::fromVariant(property->value()));
            }

            if (data.isEmpty())
                continue;

            item.insert(QString::number(it.key()), data);
        }

        if (item.isEmpty())
            continue;

        json.insert(it.value()->ieeeAddress().toHex(':'), item);
    }

    return json;
}

bool DeviceList::writeFile(QFile &file, const QByteArray &data, bool sync)
{
    bool check = true;

    if (!file.open(QFile::WriteOnly))
    {
        logWarning << "File" << file.fileName() << "open error:" << file.errorString();
        return false;
    }

    if (file.write(data) != data.length())
    {
        logWarning << "File" << file.fileName() << "write error";
        check = false;
    }

    file.close();

    if (check && sync)
        system("sync");

    return check;
}

void DeviceList::writeDatabase(void)
{
    QJsonObject json = {{"devices", serializeDevices()}, {"permitJoin", m_permitJoin}, {"version", SERVICE_VERSION}};

    m_databaseTimer->start(STORE_DATABASE_INTERVAL);
    emit statusUpdated(json);

    if (!m_sync)
        return;

    m_sync = false;

    if (writeFile(m_databaseFile, QJsonDocument(json).toJson(QJsonDocument::Compact), true))
        return;

    logWarning << "Database not stored, file" << m_databaseFile.fileName() << "error:" << m_databaseFile.errorString();
}

void DeviceList::writeProperties(void)
{
    QJsonObject json = serializeProperties();

    if (writeFile(m_propertiesFile, QJsonDocument(json).toJson(QJsonDocument::Compact)))
        return;

    logWarning << "Properties not stored, file" << m_propertiesFile.fileName() << "error:" << m_propertiesFile.errorString();
}

void DeviceList::pollAttributes(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (sender()->parent());

    for (int i = 0; i < endpoint->polls().count(); i++)
    {
        const Poll &poll = endpoint->polls().at(i);
        emit pollRequest(endpoint, poll);
    }
}
