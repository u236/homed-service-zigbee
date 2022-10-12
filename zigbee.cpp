#include <QtEndian>
#include <QFile>
#include <QRandomGenerator>
#include "gpio.h"
#include "logger.h"
#include "zcl.h"
#include "zigbee.h"

ZigBee::ZigBee(QSettings *config, QObject *parent) : QObject(parent), m_neighborsTimer(new QTimer(this)), m_queuesTimer(new QTimer(this)), m_statusTimer(new QTimer(this)), m_ledTimer(new QTimer(this)), m_transactionId(0), m_permitJoin(true)
{
    QList <QString> list = {"znp"};
    QString adapterType = config->value("zigbee/adapter", "znp").toString();

    ActionObject::registerMetaTypes();
    PollObject::registerMetaTypes();
    PropertyObject::registerMetaTypes();
    ReportingObject::registerMetaTypes();

    switch (list.indexOf(adapterType))
    {
        case 0: m_adapter = new ZStack(config, this); break;
        default: logWarning << "Unrecognized adapter type" << adapterType; return;
    }

    m_libraryFile = config->value("zigbee/library", "/usr/share/homed/zigbee.json").toString();
    m_databaseFile = config->value("zigbee/database", "/var/db/homed/zigbee.json").toString();
    m_ledPin = static_cast <qint16> (config->value("gpio/led", -1).toInt());

    GPIO::setDirection(m_ledPin, GPIO::Output);
    GPIO::setStatus(m_ledPin, false);

    connect(m_adapter, &ZStack::coordinatorReady, this, &ZigBee::coordinatorReady);
    connect(m_neighborsTimer, &QTimer::timeout, this, &ZigBee::updateNeighbors);
    connect(m_queuesTimer, &QTimer::timeout, this, &ZigBee::handleQueues);
    connect(m_statusTimer, &QTimer::timeout, this, &ZigBee::storeStatus);
    connect(m_ledTimer, &QTimer::timeout, this, &ZigBee::disableLed);

    m_statusTimer->setSingleShot(true);
    m_ledTimer->setSingleShot(true);
}

void ZigBee::init(void)
{
    QFile file(m_databaseFile);

    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();

        unserializeDevices(json.value("devices").toArray());
        m_permitJoin = json.value("permitJoin").toBool();

        file.close();
    }

    m_adapter->reset();
}

void ZigBee::setPermitJoin(bool enabled)
{
    m_permitJoin = enabled;
    m_adapter->setPermitJoin(m_permitJoin);
    storeStatus();
}

void ZigBee::setDeviceName(const QByteArray &ieeeAddress, const QString &name)
{
    auto it = m_devices.find(ieeeAddress);

    if (it == m_devices.end())
        return;

    it.value()->setName(name);
}

void ZigBee::removeDevice(const QByteArray &ieeeAddress)
{
    auto it = m_devices.find(ieeeAddress);

    if (it == m_devices.end())
        return;

    logInfo << "Device" << it.value()->name() << "removed";

    m_devices.erase(it);
    storeStatus();
}

void ZigBee::updateDevice(const QByteArray &ieeeAddress, bool reportings)
{
    Device device = m_devices.value(ieeeAddress);

    if (device.isNull())
        return;

    setupDevice(device);

    if (!reportings)
    {
        logInfo << "Device" << device->name() << "configuration updated without reportings";
        return;
    }

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        for (int i = 0; i < it.value()->reportings().count(); i++)
            configureReporting(it.value(), it.value()->reportings().at(i));

    logInfo << "Device" << device->name() << "configuration updated";
}

void ZigBee::updateReporting(const QByteArray &ieeeAddress, quint8 endpointId, const QString &reportingName, quint16 minInterval, quint16 maxInterval, quint16 valueChange)
{
    Device device = m_devices.value(ieeeAddress);

    if (device.isNull())
        return;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (endpointId && it.value()->id() != endpointId)
            continue;

        for (int i = 0; i < it.value()->reportings().count(); i++)
        {
            const Reporting &reporting = it.value()->reportings().at(i);

            if (!reportingName.isEmpty() && reporting->name() != reportingName)
                continue;

            if (minInterval)
                reporting->setMinInterval(minInterval);

            if (maxInterval)
                reporting->setMaxInterval(maxInterval);

            if (valueChange)
                reporting->setValueChange(valueChange);

            configureReporting(it.value(), reporting);
        }
    }
}

void ZigBee::bindingControl(const QByteArray &ieeeAddress, quint8 endpointId, quint16 clusterId, const QVariant &dstAddress, quint8 dstEndpointId, bool unbind)
{
    auto it = m_devices.find(ieeeAddress);

    if (it == m_devices.end())
        return;

    switch (dstAddress.type())
    {
        case QVariant::LongLong:
        {
            quint16 value = qToLittleEndian(static_cast <quint16> (dstAddress.toInt()));
            enqueueBindRequest(it.value(), endpointId, clusterId, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)), 0xFF, unbind);
            break;
        }

        case QVariant::String:
        {
            enqueueBindRequest(it.value(), endpointId, clusterId, QByteArray::fromHex(dstAddress.toString().toUtf8()), dstEndpointId, unbind);
            break;
        }

        default:
            break;
    }
}

void ZigBee::groupControl(const QByteArray &ieeeAddress, quint8 endpointId, quint16 groupId, bool remove)
{
    auto it = m_devices.find(ieeeAddress);
    zclHeaderStruct header;

    if (it == m_devices.end())
        return;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = remove ? 0x03 : 0x00;

    groupId = qFromLittleEndian(groupId);
    enqueueDataRequest(it.value(), endpointId ? endpointId : 1, CLUSTER_GROUPS, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&groupId), sizeof(groupId)).append(remove ? 0 : 1, 0x00));
}

void ZigBee::removeAllGroups(const QByteArray &ieeeAddress, quint8 endpointId)
{
    auto it = m_devices.find(ieeeAddress);
    zclHeaderStruct header;

    if (it == m_devices.end())
        return;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = 0x04;

    enqueueDataRequest(it.value(), endpointId ? endpointId : 1, CLUSTER_GROUPS, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)), QString("remove all groups request"));
}

void ZigBee::otaUpgrade(const QByteArray &ieeeAddress, quint8 endpointId, const QString &fileName)
{
    auto it = m_devices.find(ieeeAddress);
    zclHeaderStruct header;
    otaImageNotifyStruct payload;

    if (it == m_devices.end() || fileName.isEmpty() || !QFile::exists(fileName))
        return;

    m_otaUpgradeFile = fileName;

    header.frameControl = FC_CLUSTER_SPECIFIC | FC_SERVER_TO_CLIENT;
    header.transactionId = m_transactionId++;
    header.commandId = 0x00;

    payload.type = 0x00;
    payload.jitter = 0x64; // TODO: check this

    enqueueDataRequest(it.value(), endpointId ? endpointId : 1, CLUSTER_OTA_UPGRADE, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload)));
}

void ZigBee::touchLinkRequest(const QByteArray &ieeeAddress, quint8 channel, bool reset)
{
    if (m_adapter->setInterPanEndpointId(0x0C))
    {
        m_queuesTimer->stop();

        if (reset)
            touchLinkReset(ieeeAddress, channel);
        else
            touchLinkScan();

        m_adapter->resetInterPan();
        m_queuesTimer->start();
    }
}

void ZigBee::deviceAction(const QByteArray &ieeeAddress, quint8 endpointId, const QString &actionName, const QVariant &actionData)
{
    Device device = m_devices.value(ieeeAddress);

    if (device.isNull())
        return;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (endpointId && it.value()->id() != endpointId)
            continue;

        for (int i = 0; i < it.value()->actions().count(); i++)
        {
            const Action &action = it.value()->actions().at(i);

            if (action->name() == actionName)
            {
                QByteArray data = action->request(actionData);

                if (data.isEmpty())
                    continue;

                enqueueDataRequest(device, it.value()->id(), action->clusterId(), data, QString("%1 action").arg(action->name()));
                break;
            }
        }
    }
}

void ZigBee::groupAction(quint16 groupId, const QString &actionName, const QVariant &actionData)
{
    int type = QMetaType::type(QString(actionName).append("Action").toUtf8());

    if (type)
    {
        Action action(reinterpret_cast <ActionObject*> (QMetaType::create(type)));
        QByteArray data = action->request(actionData);

        if (data.isEmpty())
            return;

        m_adapter->extendedDataRequest(groupId, 0xFF, 0x0000, 0x01, action->clusterId(), data, true);
    }
}

void ZigBee::unserializeDevices(const QJsonArray &devices)
{
    for (auto it = devices.begin(); it != devices.end(); it++)
    {
        QJsonObject json = it->toObject();

        if (json.contains("ieeeAddress") && json.contains("networkAddress"))
        {
            Device device(new DeviceObject(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint16> (json.value("networkAddress").toInt())));

            device->setLogicalType(static_cast <LogicalType> (json.value("logicalType").toInt()));
            device->setVersion(static_cast <quint8> (json.value("version").toInt()));
            device->setManufacturerCode(static_cast <quint16> (json.value("manufacturerCode").toInt()));
            device->setManufacturerName(json.value("manufacturerName").toString());
            device->setModelName(json.value("modelName").toString());
            device->setName(json.value("name").toString());
            device->setLastSeen(json.value("lastSeen").toInt());

            unserializeEndpoints(device, json.value("endpoints").toArray());
            unserializeNeighbors(device, json.value("neighbors").toArray());

            if (json.value("ineterviewFinished").toBool())
                device->setInterviewFinished();

            if (device->interviewFinished())
                setupDevice(device);

            m_devices.insert(device->ieeeAddress(), device);
        }
    }

    if (m_devices.isEmpty())
        return;

    logInfo << m_devices.count() << "devices loaded";
}

void ZigBee::unserializeEndpoints(const Device &device, const QJsonArray &endpoints)
{
    for (auto it = endpoints.begin(); it != endpoints.end(); it++)
    {
        QJsonObject json = it->toObject();

        if (json.contains("endpointId"))
        {
            quint8 endpointId = static_cast <quint8> (json.value("endpointId").toInt());
            Endpoint endpoint(new EndpointObject(endpointId, device));
            QJsonArray inClusters = json.value("inClusters").toArray(), outClusters = json.value("outClusters").toArray();

            endpoint->setProfileId(static_cast <quint16> (json.value("profileId").toInt()));
            endpoint->setDeviceId(static_cast <quint16> (json.value("deviceId").toInt()));

            for (const QJsonValue &clusterId : inClusters)
                endpoint->inClusters().append(static_cast <quint16> (clusterId.toInt()));

            for (const QJsonValue &clusterId : outClusters)
                endpoint->outClusters().append(static_cast <quint16> (clusterId.toInt()));

            device->endpoints().insert(endpointId, endpoint);
        }
    }
}

void ZigBee::unserializeNeighbors(const Device &device, const QJsonArray &neighbors)
{
    for (auto it = neighbors.begin(); it != neighbors.end(); it++)
    {
        QJsonObject json = it->toObject();

        if (json.contains("networkAddress") && json.contains("linkQuality"))
            device->neighbors().insert(static_cast <quint16> (json.value("networkAddress").toInt()), static_cast <quint8> (json.value("linkQuality").toInt()));
    }
}

QJsonArray ZigBee::serializeDevices(void)
{
    QJsonArray array;

    for (auto it = m_devices.begin(); it != m_devices.end(); it++)
    {
        QJsonObject json = {{"ieeeAddress", QString(it.value()->ieeeAddress().toHex(':'))}, {"networkAddress", it.value()->networkAddress()}, {"logicalType", static_cast <quint8> (it.value()->logicalType())}};

        if (!it.value()->version())
            json.insert("version", it.value()->version());

        if (it.value()->manufacturerCode())
            json.insert("manufacturerCode", it.value()->manufacturerCode());

        if (!it.value()->manufacturerName().isEmpty())
            json.insert("manufacturerName", it.value()->manufacturerName());

        if (!it.value()->modelName().isEmpty())
            json.insert("modelName", it.value()->modelName());

        if (it.value()->name() != it.value()->ieeeAddress().toHex(':'))
            json.insert("name", it.value()->name());

        if (it.value()->logicalType() != LogicalType::Coordinator)
            json.insert("ineterviewFinished", it.value()->interviewFinished());

        if (it.value()->lastSeen())
            json.insert("lastSeen", it.value()->lastSeen());

        if (!it.value()->endpoints().isEmpty())
            json.insert("endpoints", serializeEndpoints(it.value()));

        if (!it.value()->neighbors().isEmpty())
            json.insert("neighbors", serializeNeighbors(it.value()));

        array.append(json);
    }

    return array;
}

QJsonArray ZigBee::serializeEndpoints(const Device &device)
{
    QJsonArray array;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        QJsonObject json = {{"endpointId", it.key()}};

        if (it.value()->profileId())
            json.insert("profileId", it.value()->profileId());

        if (it.value()->deviceId())
            json.insert("deviceId", it.value()->deviceId());

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

        array.append(json);
    }

    return array;
}

QJsonArray ZigBee::serializeNeighbors(const Device &device)
{
    QJsonArray array;

    for (auto it = device->neighbors().begin(); it != device->neighbors().end(); it++)
    {
        QJsonObject json = {{"networkAddress", it.key()}, {"linkQuality", it.value()}};
        array.append(json);
    }

    return array;
}

Device ZigBee::findDevice(quint16 networkAddress)
{
    for (auto it = m_devices.begin(); it != m_devices.end(); it++)
        if (it.value()->networkAddress() == networkAddress)
            return it.value();

    return Device();
}

void ZigBee::enqueueBindRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind)
{
    BindRequest request(new BindRequestObject(device, endpointId, clusterId, dstAddress, dstEndpointId, unbind));

    for (int i = 0; i < m_bindQueue.length(); i++)
        if (*m_bindQueue.at(i).data() == *request.data())
            return;

    m_bindQueue.enqueue(request);
}

void ZigBee::enqueueDataRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &data, const QString &name)
{
    DataRequest request(new DataRequestObject(device, endpointId, clusterId, data, name));

    for (int i = 0; i < m_dataQueue.length(); i++)
        if (*m_dataQueue.at(i).data() == *request.data())
            return;

    m_dataQueue.enqueue(request);
}

void ZigBee::setupDevice(const Device &device)
{
    QFile file(m_libraryFile);
    QJsonArray array;
    bool check = false;

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        logWarning << "Can't open library file, devices not loaded";
        return;
    }

    array = QJsonDocument::fromJson(file.readAll()).object().value(device->manufacturerName()).toArray();
    file.close();

    if (array.isEmpty())
    {
        logWarning << "Device" << device->name() << "manufacturer name " << device->manufacturerName() << "unrecognized";
        return;
    }

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        disconnect(it.value()->timer(), &QTimer::timeout, this, &ZigBee::pollAttributes);
        it.value()->timer()->stop();

        it.value()->actions().clear();
        it.value()->properties().clear();
        it.value()->reportings().clear();
        it.value()->polls().clear();
    }

    for (auto it = array.begin(); it != array.end(); it++)
    {
        QJsonObject json = it->toObject();
        QJsonArray array = json.value("modelNames").toArray();

        if (array.contains(device->modelName()))
        {
            QJsonValue endpoinId = json.value("endpointId");
            QList <QVariant> list = endpoinId.type() == QJsonValue::Array ? endpoinId.toArray().toVariantList() : QList <QVariant> {endpoinId.toInt(1)};

            for (int i = 0; i < list.count(); i++)
            {
                const Endpoint &endpoint = device->endpoints().value(static_cast <quint8> (list.at(i).toInt()));

                if (endpoint.isNull())
                    continue;

                setupEndpoint(endpoint, json);
            }

            device->setMultipleEndpoints(endpoinId.type() == QJsonValue::Array);
            check = true;
        }
    }

    if (check)
        return;

    logWarning << "Device" << device->name() << "model name" << device->modelName() << "unrecognized";
}

void ZigBee::setupEndpoint(const Endpoint &endpoint, const QJsonObject &json)
{
    QJsonArray actions = json.value("actions").toArray(), properties = json.value("properties").toArray(), reportings = json.value("reportings").toArray(), polls = json.value("polls").toArray();
    quint32 pollInterval = static_cast <quint32> (json.value("pollInterval").toInt());

    for (auto it = actions.begin(); it != actions.end(); it++)
    {
        int type = QMetaType::type(QString(it->toString()).append("Action").toUtf8());

        if (type)
        {
            Action action(reinterpret_cast <ActionObject*> (QMetaType::create(type)));
            endpoint->actions().append(action);
            continue;
        }

        logWarning << "Device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "action" << it->toString() << "unrecognized";
    }

    for (auto it = properties.begin(); it != properties.end(); it++)
    {
        int type = QMetaType::type(QString(it->toString()).append("Property").toUtf8());

        if (type)
        {
            Property property(reinterpret_cast <PropertyObject*> (QMetaType::create(type)));
            property->setVersion(endpoint->device()->version());
            property->setModel(endpoint->device()->modelName());
            endpoint->properties().append(property);
            continue;
        }

        logWarning << "Device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "property" << it->toString() << "unrecognized";
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

        logWarning << "Device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "reporting" << it->toString() << "unrecognized";
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

        logWarning << "Device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "poll" << it->toString() << "unrecognized";
    }

    if (!endpoint->polls().isEmpty())
    {
        if (pollInterval)
        {
            connect(endpoint->timer(), &QTimer::timeout, this, &ZigBee::pollAttributes);
            endpoint->timer()->start(pollInterval * 1000);
        }

        for (int i = 0; i < endpoint->polls().count(); i++)
        {
            const Poll &poll = endpoint->polls().at(i);
            readAttributes(endpoint->device(), endpoint->id(), poll->clusterId(), poll->attributes());
        }
    }
}

void ZigBee::interviewDevice(const Device &device)
{
    if (device->interviewFinished())
        return;

    if (!device->nodeDescriptorReceived())
    {
        m_adapter->nodeDescriptorRequest(device->networkAddress());
        return;
    }

    if (device->endpoints().isEmpty())
    {
        m_adapter->activeEndpointsRequest(device->networkAddress());
        return;
    }

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (!it.value()->profileId() && !it.value()->deviceId())
        {
            m_adapter->simpleDescriptorRequest(device->networkAddress(), it.key());
            return;
        }

        if (it.value()->inClusters().contains(CLUSTER_BASIC) && (device->manufacturerName().isEmpty() || device->modelName().isEmpty()))
        {
            readAttributes(device, it.key(), CLUSTER_BASIC, {0x0001, 0x0004, 0x0005});
            return;
        }
    }

    logInfo << "Device" << device->name() << "manufacturer name is" << device->manufacturerName() << "and model name is" << device->modelName();
    setupDevice(device);

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        for (int i = 0; i < it.value()->reportings().count(); i++)
            configureReporting(it.value(), it.value()->reportings().at(i));

    logInfo << "Device" << device->name() << "interview finished";
    device->setInterviewFinished();
    storeStatus();
}

void ZigBee::configureReporting(const Endpoint &endpoint, const Reporting &reporting)
{
    zclHeaderStruct header;
    QByteArray payload;

    header.frameControl = 0x00;
    header.transactionId = m_transactionId++;
    header.commandId = CMD_CONFIGURE_REPORTING;

    for (int i = 0; i < reporting->attributes().count(); i++)
    {
        configureReportingStruct item;

        item.direction = 0x00;
        item.attributeId = qToLittleEndian(reporting->attributes().at(i));
        item.dataType = reporting->dataType();
        item.minInterval = qToLittleEndian(reporting->minInterval());
        item.maxInterval = qToLittleEndian(reporting->maxInterval());
        item.valueChange = qToLittleEndian(reporting->valueChange());

        payload.append(reinterpret_cast <char*> (&item), sizeof(item) - sizeof(item.valueChange) + zclDataSize(item.dataType));
    }

    enqueueBindRequest(endpoint->device(), endpoint->id(), reporting->clusterId());
    enqueueDataRequest(endpoint->device(), endpoint->id(), reporting->clusterId(), QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(payload), QString("%1 reporting configuration").arg(reporting->name()));
}

void ZigBee::readAttributes(const Device &device, quint8 endpointId, quint16 clusterId, QList <quint16> attributes)
{
    zclHeaderStruct header;
    QByteArray payload;

    header.frameControl = 0x00;
    header.transactionId = m_transactionId++;
    header.commandId = CMD_READ_ATTRIBUTES;

    for (int i = 0; i < attributes.count(); i++)
    {
        quint16 attributeId = qToLittleEndian(attributes.at(i));
        payload.append(reinterpret_cast <char*> (&attributeId), sizeof(attributeId));
    }

    enqueueDataRequest(device, endpointId, clusterId, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(payload));
}

void ZigBee::parseAttribute(const Endpoint &endpoint, quint16 clusterId, quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    Device device = endpoint->device();
    bool check = false;

    if (clusterId == CLUSTER_BASIC && (attributeId == 0x0001 || attributeId == 0x0004 || attributeId == 0x0005))
    {
        switch (attributeId)
        {
            case 0x0001:

                if (dataType != DATA_TYPE_8BIT_UNSIGNED)
                    return;

                device->setVersion(static_cast <quint8> (data.at(0)));
                break;

            case 0x0004:

                if (device->interviewFinished() || dataType != DATA_TYPE_CHARACTER_STRING)
                    return;

                device->setManufacturerName(QString(data).trimmed());
                break;

            case 0x0005:

                if (device->interviewFinished() || dataType != DATA_TYPE_CHARACTER_STRING)
                    return;

                device->setModelName(QString(data).trimmed());
                break;
        }

        if (!device->manufacturerName().isEmpty() && !device->modelName().isEmpty())
        {
            if (device->modelName() == "TS0601") // vendor is model for devices based on Tuya TS0601
            {
                device->setModelName(device->manufacturerName());
                device->setManufacturerName("TUYA");
            }

            interviewDevice(device);
        }

        return;
    }

    if (!device->interviewFinished())
        return;

    for (int i = 0; i < endpoint->properties().count(); i++)
    {
        const Property &property = endpoint->properties().at(i);

        if (property->clusterId() == clusterId)
        {
            QVariant value = property->value();

            property->parseAttribte(attributeId, dataType, data);
            check = true;

            if (property->value() == value)
                continue;

            endpoint->setDataUpdated(true);
        }
    }

    if (!check)
        logWarning << "No property found for device" << device->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "cluster" << QString::asprintf("0x%04X", clusterId) << "attribute" << QString::asprintf("0x%04X", attributeId) << "with data type" << QString::asprintf("0x%02X", dataType) << "and data" << data.toHex(':');
}

void ZigBee::clusterCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint8 commandId, const QByteArray &payload)
{
    Device device = endpoint->device();
    bool check = false;

    if (!device->interviewFinished())
        return;

    if (clusterId == CLUSTER_GROUPS)
    {
        switch (commandId)
        {
            case 0x00:
            case 0x03:
            {
                const groupControlResponseStruct *response = reinterpret_cast <const groupControlResponseStruct*> (payload.constData());

                switch (response->status)
                {
                    case STATUS_SUCCESS:
                        logInfo << "Device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "group" << qFromLittleEndian(response->grpoupId) << "successfully" << (commandId ? "removed" : "added");
                        break;

                    case STATUS_INSUFFICIENT_SPACE:
                        logWarning << "Device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "group" << qFromLittleEndian(response->grpoupId) << "not added, no free space available";
                        break;

                    case STATUS_DUPLICATE_EXISTS:
                        logWarning << "Device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "group" << qFromLittleEndian(response->grpoupId) << "already exists";
                        break;

                    case STATUS_NOT_FOUND:
                        logWarning << "Device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "group" << qFromLittleEndian(response->grpoupId) << "not found";
                        break;

                    default:
                        logWarning << "Device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "group" << qFromLittleEndian(response->grpoupId) << (commandId ? "remove" : "add") << "command status" << QString::asprintf("0x%02X", response->status) << "unrecognized";
                        break;
                }

                break;
            }

            default:
                logWarning << "Unrecognized group control command" << QString::asprintf("0x%02X", commandId) << "received from device" << device->name() << "with payload:" << payload.toHex(':');
                break;
        }

        return;
    }

    if (clusterId == CLUSTER_OTA_UPGRADE)
    {
        QFile file(m_otaUpgradeFile);
        otaFileHeaderStruct fileHeader;
        zclHeaderStruct zclHeader;

        memset(&fileHeader, 0, sizeof(fileHeader));

        if (file.exists() && file.open(QFile::ReadOnly))
            memcpy(&fileHeader, file.read(sizeof(fileHeader)).constData(), sizeof(fileHeader));

        zclHeader.frameControl = FC_CLUSTER_SPECIFIC | FC_SERVER_TO_CLIENT | FC_DISABLE_DEFAULT_RESPONSE;
        zclHeader.transactionId = transactionId;

        switch (commandId)
        {
            case 0x01:
            {
                const otaNextImageRequestStruct *request = reinterpret_cast <const otaNextImageRequestStruct*> (payload.constData());
                otaNextImageResponseStruct response;

                zclHeader.commandId = 0x02;

                if (!file.isOpen() || request->manufacturerCode != fileHeader.manufacturerCode || request->imageType != fileHeader.imageType)
                {
                    enqueueDataRequest(device, endpoint->id(), CLUSTER_OTA_UPGRADE, QByteArray(reinterpret_cast <char*> (&zclHeader), sizeof(zclHeader)).append(STATUS_NO_IMAGE_AVAILABLE));
                    break;
                }

                if (request->fileVersion == fileHeader.fileVersion)
                {
                    logInfo << "Device" << device->name() << "OTA upgrade not started, version match:" << QString::asprintf("0x%08X", qFromLittleEndian(request->fileVersion)).toUtf8().constData();
                    enqueueDataRequest(device, endpoint->id(), CLUSTER_OTA_UPGRADE, QByteArray(reinterpret_cast <char*> (&zclHeader), sizeof(zclHeader)).append(STATUS_NO_IMAGE_AVAILABLE));
                    break;
                }

                logInfo << "Device" << device->name() << "OTA upgrade started...";

                response.status = 0x00;
                response.manufacturerCode = fileHeader.manufacturerCode;
                response.imageType = fileHeader.imageType;
                response.fileVersion = fileHeader.fileVersion;
                response.imageSize = fileHeader.imageSize;

                enqueueDataRequest(device, endpoint->id(), CLUSTER_OTA_UPGRADE, QByteArray(reinterpret_cast <char*> (&zclHeader), sizeof(zclHeader)).append(reinterpret_cast <char*> (&response), sizeof(response)));
                break;
            }

            case 0x03:
            {
                const otaImageBlockRequestStruct *request = reinterpret_cast <const otaImageBlockRequestStruct*> (payload.constData());
                otaImageBlockResponseStruct response;
                QByteArray data;

                zclHeader.commandId = 0x05;

                if (!file.isOpen() || request->manufacturerCode != fileHeader.manufacturerCode || request->imageType != fileHeader.imageType ||request->fileVersion != fileHeader.fileVersion)
                {
                    enqueueDataRequest(device, endpoint->id(), CLUSTER_OTA_UPGRADE, QByteArray(reinterpret_cast <char*> (&zclHeader), sizeof(zclHeader)).append(STATUS_NO_IMAGE_AVAILABLE));
                    break;
                }

                file.seek(qFromLittleEndian(request->fileOffset));
                data = file.read(request->dataSizeMax);

                // TODO: use percentage here?
                logInfo << "Device" << device->name() << "OTA upgrade writing" << data.length() << "bytes with offset" << QString::asprintf("0x%04X", qFromLittleEndian(request->fileOffset));

                response.status = 0x00;
                response.manufacturerCode = request->manufacturerCode;
                response.imageType = request->imageType;
                response.fileVersion = request->fileVersion;
                response.fileOffset = request->fileOffset;
                response.dataSize = static_cast <quint8> (data.length());

                enqueueDataRequest(device, endpoint->id(), CLUSTER_OTA_UPGRADE, QByteArray(reinterpret_cast <char*> (&zclHeader), sizeof(zclHeader)).append(reinterpret_cast <char*> (&response), sizeof(response)).append(data));
                break;
            }
            case 0x06:
            {
                const otaUpgradeEndRequestStruct *request = reinterpret_cast <const otaUpgradeEndRequestStruct*> (payload.constData());
                otaUpgradeEndResponseStruct response;

                zclHeader.commandId = 0x07;
                m_otaUpgradeFile.clear();

                if (request->status)
                {
                    logWarning << "Device" << device->name() << "OTA upgrade failed, status:" << QString::asprintf("%02X", request->status);;
                    break;
                }

                logInfo << "Device" << device->name() << "OTA upgrade finished successfully";

                response.manufacturerCode = request->manufacturerCode;
                response.imageType = request->imageType;
                response.fileVersion = request->fileVersion;
                response.currentTime = 0;
                response.upgradeTime = 0;

                enqueueDataRequest(device, endpoint->id(), CLUSTER_OTA_UPGRADE, QByteArray(reinterpret_cast <char*> (&zclHeader), sizeof(zclHeader)).append(reinterpret_cast <char*> (&response), sizeof(response)));
                break;
            }

            default:
                logWarning << "Unrecognized OTA upgrade command" << QString::asprintf("0x%02X", commandId) << "received from device" << device->name() << "with payload:" << payload.toHex(':');
                break;
        }

        if (file.isOpen())
            file.close();

        return;
    }

    for (int i = 0; i < endpoint->properties().count(); i++)
    {
        const Property &property = endpoint->properties().at(i);

        if (property->clusterId() == clusterId)
        {
            QVariant value = property->value();

            property->parseCommand(commandId, payload);
            check = true;

            if (property->value() == value)
                continue;

            endpoint->setDataUpdated(true);
        }
    }

    if (!check)
        logWarning << "No property found for device" << device->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "cluster" << QString::asprintf("0x%04X", clusterId) << "command" << QString::asprintf("0x%02X", commandId) << "with payload" << payload.toHex(':');
}

void ZigBee::globalCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint8 commandId, QByteArray payload)
{
    Q_UNUSED(transactionId)

    switch (commandId)
    {
        case CMD_READ_ATTRIBUTES: // TODO: tyua sensor reading time cluster
        case CMD_WRITE_ATTRIBUTES_RESPONSE:
        case CMD_CONFIGURE_REPORTING_RESPONSE:
        case CMD_DEFAULT_RESPONSE:
            break;

        case CMD_READ_ATTRIBUTES_RESPONSE:
        case CMD_REPORT_ATTRIBUTES:
        {
            while (payload.length())
            {
                quint8 dataType, offset, size = 0;
                quint16 attributeId;

                if (commandId == CMD_READ_ATTRIBUTES_RESPONSE)
                {
                    if (payload.at(2))
                        break;

                    dataType = static_cast <quint8> (payload.at(3));
                    offset = 4;
                }
                else
                {
                    dataType = static_cast <quint8> (payload.at(2));
                    offset = 3;
                }

                memcpy(&attributeId, payload.constData(), sizeof(attributeId));
                attributeId = qFromLittleEndian(attributeId);
                size = zclDataSize(dataType, payload, &offset);

                if (!size)
                {
                    logWarning << "Unrecognized attribute" << QString::asprintf("0x%04X", attributeId) << "data type" << QString::asprintf("0x%02X", dataType) << "received from device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "cluster" << QString::asprintf("0x%04X", clusterId) << "with payload:" << payload.mid(offset).toHex(':');
                    return;
                }

                parseAttribute(endpoint, clusterId, attributeId, dataType, payload.mid(offset, size));
                payload.remove(0, offset + size);
            }

            break;
        }

        default:
            logWarning << "Unrecognized command" << QString::asprintf("0x%02X", commandId) << "received from device" << endpoint->device()->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "cluster" << QString::asprintf("0x%04X", clusterId) << "with payload:" << payload.toHex(':');
            break;
    }
}

void ZigBee::touchLinkReset(const QByteArray &ieeeAddress, quint8 channel)
{
    zclHeaderStruct header;
    touchLinkScanStruct payload;

    header.frameControl = FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE;
    header.transactionId = m_transactionId++;
    header.commandId = 0x00;

    payload.transactionId = QRandomGenerator::global()->generate();
    payload.zigBeeInformation = 0x04;
    payload.touchLinkInformation = 0x12;

    if (!m_adapter->setInterPanChannel(channel))
        return;

    if (!m_adapter->extendedDataRequest(0xFFFF, 0xFE, 0xFFFF, 0x0C, CLUSTER_TOUCHLINK, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(QByteArray(reinterpret_cast <char*> (&payload), sizeof(payload)))))
    {
        logWarning << "TouchLink scan request failed, status:" << QString::asprintf("%02X", m_adapter->dataRequestStatus());
        return;
    }

    header.commandId = 0x07;

    if (!m_adapter->extendedDataRequest(ieeeAddress, 0xFE, 0xFFFF, 0x0C, CLUSTER_TOUCHLINK, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(QByteArray(reinterpret_cast <char*> (&payload), sizeof(payload.transactionId)))))
    {
        logWarning << "TouchLink reset request failed, status:" << QString::asprintf("%02X", m_adapter->dataRequestStatus());
        return;
    }

    logInfo << "TouchLink reset finished successfully";
}

void ZigBee::touchLinkScan(void)
{
    zclHeaderStruct header;
    touchLinkScanStruct payload;

    header.frameControl = FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE;
    header.transactionId = m_transactionId++;
    header.commandId = 0x00;

    payload.transactionId = QRandomGenerator::global()->generate();
    payload.zigBeeInformation = 0x04;
    payload.touchLinkInformation = 0x12;

    logInfo << "TouchLink scan started...";

    for (m_interPanChannel = 11; m_interPanChannel <= 26; m_interPanChannel++)
    {
        if (!m_adapter->setInterPanChannel(m_interPanChannel))
            return;

        if (!m_adapter->extendedDataRequest(0xFFFF, 0xFE, 0xFFFF, 0x0C, CLUSTER_TOUCHLINK, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(QByteArray(reinterpret_cast <char*> (&payload), sizeof(payload)))))
        {
            logWarning << "TouchLink scan request failed, status:" << QString::asprintf("%02X", m_adapter->dataRequestStatus());
            return;
        }
    }

    logInfo << "TouchLink scan finished successfully";
}

void ZigBee::coordinatorReady(const QByteArray &ieeeAddress)
{
    Device device(new DeviceObject(ieeeAddress));

    logInfo << "Coordinator ready, address:" << ieeeAddress.toHex(':').constData();

    connect(m_adapter, &ZStack::deviceJoined, this, &ZigBee::deviceJoined);
    connect(m_adapter, &ZStack::deviceLeft, this, &ZigBee::deviceLeft);
    connect(m_adapter, &ZStack::nodeDescriptorReceived, this, &ZigBee::nodeDescriptorReceived);
    connect(m_adapter, &ZStack::activeEndpointsReceived, this, &ZigBee::activeEndpointsReceived);
    connect(m_adapter, &ZStack::simpleDescriptorReceived, this, &ZigBee::simpleDescriptorReceived);
    connect(m_adapter, &ZStack::neighborRecordReceived, this, &ZigBee::neighborRecordReceived);
    connect(m_adapter, &ZStack::messageReveived, this, &ZigBee::messageReveived);
    connect(m_adapter, &ZStack::extendedMessageReveived, this, &ZigBee::extendedMessageReveived);

    for (auto it = m_devices.begin(); it != m_devices.end(); it++)
    {
        if (it.key() == ieeeAddress || it.value()->logicalType() == LogicalType::Coordinator)
            m_devices.erase(it++);

        if (it == m_devices.end())
            break;
    }

    device->setLogicalType(LogicalType::Coordinator);
    device->setName("HOMEd Coordinator");
    device->setInterviewFinished();

    device->endpoints().insert(0x01, Endpoint(new EndpointObject(0x01, device, PROFILE_HA,   0x0005)));
    device->endpoints().insert(0x02, Endpoint(new EndpointObject(0x01, device, PROFILE_IPM,  0x0005)));
    device->endpoints().insert(0x03, Endpoint(new EndpointObject(0x03, device, PROFILE_HA,   0x0005)));
    device->endpoints().insert(0x04, Endpoint(new EndpointObject(0x04, device, PROFILE_TA,   0x0005)));
    device->endpoints().insert(0x05, Endpoint(new EndpointObject(0x05, device, PROFILE_PHHC, 0x0005)));
    device->endpoints().insert(0x06, Endpoint(new EndpointObject(0x06, device, PROFILE_AMI,  0x0005)));
    device->endpoints().insert(0x08, Endpoint(new EndpointObject(0x08, device, PROFILE_HA,   0x0005)));
    device->endpoints().insert(0x0A, Endpoint(new EndpointObject(0x0A, device, PROFILE_HA,   0x0005)));
    device->endpoints().insert(0x0B, Endpoint(new EndpointObject(0x0B, device, PROFILE_HA,   0x0400)));
    device->endpoints().insert(0x0C, Endpoint(new EndpointObject(0x0C, device, PROFILE_ZLL,  0x0005)));
    device->endpoints().insert(0x0D, Endpoint(new EndpointObject(0x0D, device, PROFILE_HA,   0x0005)));
    device->endpoints().insert(0x2F, Endpoint(new EndpointObject(0x2F, device, PROFILE_HA,   0x0005)));
    device->endpoints().insert(0x6E, Endpoint(new EndpointObject(0x6E, device, PROFILE_HA,   0x0005)));
    device->endpoints().insert(0xF2, Endpoint(new EndpointObject(0xF2, device, PROFILE_HUE,  0x0005)));

    device->endpoints().value(0x0B)->inClusters().append(CLUSTER_TIME);
    device->endpoints().value(0x0B)->inClusters().append(CLUSTER_IAS_ACE);
    device->endpoints().value(0x0B)->outClusters().append(CLUSTER_IAS_ZONE);
    device->endpoints().value(0x0B)->outClusters().append(CLUSTER_IAS_WD);
    device->endpoints().value(0x0D)->inClusters().append(CLUSTER_OTA_UPGRADE);

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        m_adapter->registerEndpoint(it.key(), it.value()->profileId(), it.value()->deviceId(), it.value()->inClusters(), it.value()->outClusters());

    m_devices.insert(ieeeAddress, device);
    m_adapter->setPermitJoin(m_permitJoin);

    m_queuesTimer->start(HANDLE_QUEUES_INTERVAL);
    m_neighborsTimer->start(UPDATE_NEIGHBORS_INTERVAL);

    storeStatus();
}

void ZigBee::deviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress, quint8 capabilities)
{
    auto it = m_devices.find(ieeeAddress);

    if (it != m_devices.end())
    {
        if (QDateTime::currentMSecsSinceEpoch() < it.value()->joinTime() + DEVICE_REJOIN_INTERVAL)
            return;

        logInfo << "Device" << it.value()->name() << "rejoined network";
        it.value()->setNetworkAddress(networkAddress);
    }
    else
    {
        logInfo << "Device" << ieeeAddress.toHex(':') << "joined network with address:" << QString::asprintf("0x%04X", networkAddress) << "and capabilities:" << QString::asprintf("0x%02X", capabilities);
        it = m_devices.insert(ieeeAddress, Device(new DeviceObject(ieeeAddress, networkAddress)));
    }

    m_ledTimer->start(500);
    GPIO::setStatus(m_ledPin, true);

    it.value()->updateJoinTime();
    it.value()->updateLastSeen();

    interviewDevice(it.value());
    emit deviceEvent();
}

void ZigBee::deviceLeft(const QByteArray &ieeeAddress, quint16 networkAddress)
{
    auto it = m_devices.find(ieeeAddress);

    if (it == m_devices.end())
        return;

    m_ledTimer->start(500);
    GPIO::setStatus(m_ledPin, true);

    logInfo << "Device" << it.value()->name() << "with address" << QString::asprintf("0x%04X", networkAddress) << "left network";

    m_devices.erase(it);
    storeStatus();

    emit deviceEvent(false);
}

void ZigBee::nodeDescriptorReceived(quint16 networkAddress, LogicalType logicalType, quint16 manufacturerCode)
{
    Device device = findDevice(networkAddress);

    if (device.isNull())
        return;

    device->setLogicalType(logicalType);
    device->setManufacturerCode(manufacturerCode);

    if (device->logicalType() == LogicalType::Router)
        logInfo << "Device" << device->name() << "is router";

    logInfo << "Device" << device->name() << "node descriptor received, manufacturer code:" << QString::asprintf("0x%04X", device->manufacturerCode());

    device->setNodeDescriptorReceived();
    device->updateLastSeen();
    interviewDevice(device);
}

void ZigBee::activeEndpointsReceived(quint16 networkAddress, const QByteArray data)
{
    Device device = findDevice(networkAddress);
    QList <QString> list;

    if (device.isNull())
        return;

    for (int i = 0; i < data.length(); i++)
        list.append(QString::asprintf("0x%02X", data.at(i)));

    logInfo << "Device" << device->name() << "active endpoints received:" << list.join(", ");

    for (int i = 0; i < data.length(); i++)
        if (device->endpoints().find(static_cast <quint8> (data.at(i))) == device->endpoints().end())
            device->endpoints().insert(static_cast <quint8> (data.at(i)), Endpoint(new EndpointObject(data.at(i), device)));

    device->updateLastSeen();
    interviewDevice(device);
}

void ZigBee::simpleDescriptorReceived(quint16 networkAddress, quint8 endpointId, quint16 profileId, quint16 deviceId, const QList<quint16> &inClusters, const QList<quint16> &outClusters)
{
    Device device = findDevice(networkAddress);
    Endpoint endpoint;

    if (device.isNull())
        return;

    endpoint = device->endpoints().value(endpointId);

    if (endpoint.isNull())
        return;

    endpoint->setProfileId(profileId);
    endpoint->setDeviceId(deviceId);

    endpoint->inClusters() = inClusters;
    endpoint->outClusters() = outClusters;

    logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02X", endpoint->id()) << "simple descriptor received";

    device->updateLastSeen();
    interviewDevice(device);
}

void ZigBee::neighborRecordReceived(quint16 networkAddress, quint16 neighborAddress, quint8 linkQuality, bool first)
{
    Device device = findDevice(networkAddress);

    if (device.isNull())
        return;

    if (first)
    {
        logInfo << "Device" << device->name() << "neighbors list received";
        device->neighbors().clear();
    }

    if (findDevice(neighborAddress).isNull())
        return;

    device->neighbors().insert(neighborAddress, linkQuality);
    device->updateLastSeen();
}

void ZigBee::messageReveived(quint16 networkAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data)
{
    Device device = findDevice(networkAddress);
    Endpoint endpoint;
    zclHeaderStruct header;
    QByteArray payload;

    if (device.isNull())
        return;

    endpoint = device->endpoints().value(endpointId);

    if (endpoint.isNull())
        return;

    m_ledTimer->start(50);
    GPIO::setStatus(m_ledPin, true);

    header.frameControl = static_cast <quint8> (data.at(0));

    if (header.frameControl & FC_MANUFACTURER_SPECIFIC)
    {
        header.transactionId = static_cast <quint8> (data.at(3));
        header.commandId = static_cast <quint8> (data.at(4));
        payload = data.mid(5);
    }
    else
    {
        header.transactionId = static_cast <quint8> (data.at(1));
        header.commandId = static_cast <quint8> (data.at(2));
        payload = data.mid(3);
    }

    if (header.frameControl & FC_CLUSTER_SPECIFIC)
        clusterCommandReceived(endpoint, clusterId, header.transactionId, header.commandId, payload);
    else
        globalCommandReceived(endpoint, clusterId, header.transactionId, header.commandId, payload);

    device->setLinkQuality(linkQuality);
    device->updateLastSeen();

    if (endpoint->dataUpdated())
    {
        endpoint->setDataUpdated(false);
        emit endpointUpdated(device, endpoint->id());
    }

    if (!(header.frameControl & FC_DISABLE_DEFAULT_RESPONSE))
    {
        defaultResponseStruct response;

        header.frameControl = FC_SERVER_TO_CLIENT | FC_DISABLE_DEFAULT_RESPONSE;
        header.commandId = CMD_DEFAULT_RESPONSE;

        response.commandId = header.commandId;
        response.status = 0x00;

        enqueueDataRequest(device, endpoint->id(), clusterId, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(QByteArray(reinterpret_cast <char*> (&response), sizeof(response))));
    }
}

void ZigBee::extendedMessageReveived(const QByteArray &ieeeAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data)
{
    Q_UNUSED(endpointId)
    Q_UNUSED(linkQuality)

    const zclHeaderStruct *header = reinterpret_cast <const zclHeaderStruct*> (data.constData());

    if (clusterId == CLUSTER_TOUCHLINK && header->commandId == 0x01)
    {
        logInfo << "TouchLink scan response received from device" << ieeeAddress.toHex(':') << "at channel" << m_interPanChannel;
        return;
    }

    logWarning << "Unrecognized extended message received from" << ieeeAddress.toHex(':') << "endpoint" << QString::asprintf("0x%02X", endpointId) << "cluster" << QString::asprintf("0x%04X", clusterId) << "with payload:" << data.toHex(':');
}

void ZigBee::pollAttributes(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (sender()->parent());

    for (int i = 0; i < endpoint->polls().count(); i++)
    {
        const Poll &poll = endpoint->polls().at(i);
        readAttributes(endpoint->device(), endpoint->id(), poll->clusterId(), poll->attributes());
    }
}

void ZigBee::updateNeighbors(void)
{
    for (auto it = m_devices.begin(); it != m_devices.end(); it++)
    {
        if (it.value()->logicalType() == LogicalType::EndDevice || m_neighborsQueue.contains(it.value()))
            continue;

        m_neighborsQueue.enqueue(it.value());
    }
}

void ZigBee::handleQueues(void)
{
    while (!m_bindQueue.isEmpty())
    {
        BindRequest request = m_bindQueue.dequeue();

        if (m_adapter->bindRequest(request->device()->networkAddress(), request->device()->ieeeAddress(), request->endpointId(), request->clusterId(), request->dstAddress(), request->dstEndpointId(), request->unbind()))
        {
            if (!request->dstAddress().isEmpty())
                logInfo << "Device" << request->device()->name() << (request->unbind() ? "unbinding" : "binding") << "finished succesfully";

            continue;
        }

        logWarning << "Device" << request->device()->name() << "endpoint" << QString::asprintf("0x%02X", request->endpointId()) << "cluster" << QString::asprintf("0x%04X", request->clusterId()) << (request->unbind() ? "unbinding" : "binding") << "failed";
    }

    while (!m_dataQueue.isEmpty())
    {
        DataRequest request = m_dataQueue.dequeue();

        if (m_adapter->dataRequest(request->device()->networkAddress(), request->endpointId(), request->clusterId(), request->data()))
        {
            if (!request->name().isEmpty())
                logInfo << "Device" << request->device()->name() << request->name().toUtf8().constData() << "finished successfully";

            continue;
        }

        logWarning << "Device" << request->device()->name() << (!request->name().isEmpty() ? request->name().toUtf8().constData() : "data request") << "failed, status:" << QString::asprintf("%02X", m_adapter->dataRequestStatus());
    }

    if (!m_neighborsQueue.isEmpty())
    {
        Device device = m_neighborsQueue.dequeue();
        m_adapter->lqiRequest(device->networkAddress());
    }
}

void ZigBee::storeStatus(void)
{
    QFile file(m_databaseFile);
    QJsonObject json = {{"devices", serializeDevices()}, {"permitJoin", m_permitJoin}};

    m_statusTimer->start(STORE_STATUS_INTERVAL);

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        logWarning << "Can't open database file, status not saved";
        return;
    }

    file.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    file.close();

    emit statusStored(json);
}

void ZigBee::disableLed(void)
{
    GPIO::setStatus(m_ledPin, false);
}
