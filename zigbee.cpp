#include <QtEndian>
#include <QFile>
#include <QRandomGenerator>
#include "gpio.h"
#include "logger.h"
#include "zcl.h"
#include "zigbee.h"

ZigBee::ZigBee(QSettings *config, QObject *parent) : QObject(parent), m_adapter(new ZStack(config, this)), m_neighborsTimer(new QTimer(this)), m_queuesTimer(new QTimer(this)), m_statusTimer(new QTimer(this)), m_ledTimer(new QTimer(this)), m_transactionId(0), m_coordinatorReady(false), m_permitJoin(true)
{
    ActionObject::registerMetaTypes();
    PollObject::registerMetaTypes();
    PropertyObject::registerMetaTypes();
    ReportingObject::registerMetaTypes();

    m_databaseFile = config->value("zigbee/database", "/var/db/homed/zigbee.json").toString();
    m_libraryFile = config->value("zigbee/library", "/usr/share/homed/zigbee.json").toString();
    m_ledPin = static_cast <qint16> (config->value("gpio/led", -1).toInt());

    GPIO::setDirection(m_ledPin, GPIO::Output);
    GPIO::setStatus(m_ledPin, false);

    connect(m_adapter, &ZStack::coordinatorReady, this, &ZigBee::coordinatorReady);
    connect(m_adapter, &ZStack::endDeviceJoined, this, &ZigBee::endDeviceJoined);
    connect(m_adapter, &ZStack::endDeviceLeft, this, &ZigBee::endDeviceLeft);
    connect(m_adapter, &ZStack::nodeDescriptorReceived, this, &ZigBee::nodeDescriptorReceived);
    connect(m_adapter, &ZStack::activeEndpointsReceived, this, &ZigBee::activeEndpointsReceived);
    connect(m_adapter, &ZStack::simpleDescriptorReceived, this, &ZigBee::simpleDescriptorReceived);
    connect(m_adapter, &ZStack::neighborRecordReceived, this, &ZigBee::neighborRecordReceived);
    connect(m_adapter, &ZStack::messageReveived, this, &ZigBee::messageReveived);
    connect(m_adapter, &ZStack::messageReveivedExt, this, &ZigBee::messageReveivedExt);

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

    m_adapter->init();
}

void ZigBee::setPermitJoin(bool enabled)
{
    if (!m_coordinatorReady)
        return;

    m_permitJoin = enabled;
    m_adapter->setPermitJoin(m_permitJoin);
    storeStatus();
}

void ZigBee::configureDevice(const QByteArray &ieeeAddress)
{
    auto it = m_devices.find(ieeeAddress);

    if (it == m_devices.end())
        return;

    setupDevice(it.value());
    configureReportings(it.value());

    logInfo << "Device" << it.value()->name() << "configuration updated";
}

void ZigBee::setDeviceName(const QByteArray &ieeeAddress, const QString &name)
{
    auto it = m_devices.find(ieeeAddress);

    if (it == m_devices.end())
        return;

    it.value()->setName(name);
}

void ZigBee::deviceAction(const QByteArray &ieeeAddress, const QString &actionName, const QVariant &actionData)
{
    auto it = m_devices.find(ieeeAddress);

    if (it == m_devices.end())
        return;

    for (int i = 0; i < it.value()->actions().count(); i++)
    {
        const Action &action = it.value()->actions().at(i);

        if (action->name() == actionName)
        {
            QByteArray data = action->request(actionData);

            m_ledTimer->start(50);
            GPIO::setStatus(m_ledPin, true);

            if (data.isEmpty())
                continue;

            enqueueDataRequest(it.value(), action->endpointId(), action->clusterId(), data, QString("%1 action").arg(action->name()));
        }
    }
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

void ZigBee::touchLinkReset(const QByteArray &ieeeAddress, quint8 channel)
{
    zclHeaderStruct header;
    quint32 payload = QRandomGenerator::global()->generate();

    header.frameControl = FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE;
    header.transactionId = m_transactionId++;
    header.commandId = 0x00;

    if (!m_adapter->setInterPanEndpointId(0x0C) || !m_adapter->setInterPanChannel(channel))
    {
        m_adapter->resetInterPan();
        return;
    }

    if (!m_adapter->dataRequestExt(0xFFFF, 0xFE, 0xFFFF, 0x0C, CLUSTER_TOUCHLINK, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(QByteArray(reinterpret_cast <char*> (&payload), sizeof(payload)))))
    {
        logWarning << "TouchLink scan request failed, status:" << QString::asprintf("%02X", m_adapter->dataRequestStatus());
        m_adapter->resetInterPan();
        return;
    }

    header.commandId = 0x07;

    if (!m_adapter->dataRequestExt(ieeeAddress, 0xFE, 0xFFFF, 0x0C, CLUSTER_TOUCHLINK, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(QByteArray(reinterpret_cast <char*> (&payload), sizeof(payload)))))
    {
        logWarning << "TouchLink reset request failed, status:" << QString::asprintf("%02X", m_adapter->dataRequestStatus());
        m_adapter->resetInterPan();
        return;
    }

    logInfo << "TouchLink reset finished successfully";
    m_adapter->resetInterPan();
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

    if (!m_adapter->setInterPanEndpointId(0x0C))
    {
        m_adapter->resetInterPan();
        return;
    }

    for (m_interPanChannel = 11; m_interPanChannel <= 26; m_interPanChannel++)
    {
        if (!m_adapter->setInterPanChannel(m_interPanChannel))
        {
            m_adapter->resetInterPan();
            return;
        }

        if (!m_adapter->dataRequestExt(0xFFFF, 0xFE, 0xFFFF, 0x0C, CLUSTER_TOUCHLINK, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(QByteArray(reinterpret_cast <char*> (&payload), sizeof(payload)))))
        {
            logWarning << "TouchLink scan request failed, status:" << QString::asprintf("%02X", m_adapter->dataRequestStatus());
            m_adapter->resetInterPan();
            return;
        }
    }

    logInfo << "TouchLink scan finished successfully";
    m_adapter->resetInterPan();
}

void ZigBee::unserializeDevices(const QJsonArray &array)
{
    for (auto it = array.begin(); it != array.end(); it++)
    {
        QJsonObject json = it->toObject();

        if (json.contains("ieeeAddress") && json.contains("networkAddress"))
        {
            Device device(new DeviceObject(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint16> (json.value("networkAddress").toInt())));

            if (json.value("ineterviewFinished").toBool())
                device->setInterviewFinished();

            device->setManufacturerCode(static_cast <quint16> (json.value("manufacturerCode").toInt()));
            device->setLogicalType(static_cast <LogicalType> (json.value("logicalType").toInt()));
            device->setName(json.value("name").toString());
            device->setVendor(json.value("vendor").toString());
            device->setModel(json.value("model").toString());
            device->setLastSeen(json.value("lastSeen").toInt());

            unserializeEndpoints(device, json.value("endpoints").toArray());
            unserializeNeighbors(device, json.value("neighbors").toArray());

            if (device->interviewFinished())
                setupDevice(device);

            m_devices.insert(device->ieeeAddress(), device);
        }
    }

    if (m_devices.isEmpty())
        return;

    logInfo << "Loaded" << m_devices.count() << "devices";
}

void ZigBee::unserializeEndpoints(const Device &device, const QJsonArray &array)
{
    for (auto it = array.begin(); it != array.end(); it++)
    {
        QJsonObject json = it->toObject();

        if (json.contains("endpointId"))
        {
            quint8 endpointId = static_cast <quint8> (json.value("endpointId").toInt());
            Endpoint endpoint(new EndpointObject(endpointId, device));
            QJsonArray inClustersArray = json.value("inClusters").toArray(), outClustersArray = json.value("outClusters").toArray();

            endpoint->setProfileId(static_cast <quint16> (json.value("profileId").toInt()));
            endpoint->setDeviceId(static_cast <quint16> (json.value("deviceId").toInt()));

            for (const QJsonValue &clusterId : inClustersArray)
                endpoint->inClusters().append(static_cast <quint16> (clusterId.toInt()));

            for (const QJsonValue &clusterId : outClustersArray)
                endpoint->outClusters().append(static_cast <quint16> (clusterId.toInt()));

            device->endpoints().insert(endpointId, endpoint);
        }
    }
}

void ZigBee::unserializeNeighbors(const Device &device, const QJsonArray &array)
{
    for (auto it = array.begin(); it != array.end(); it++)
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

        if (it.value()->manufacturerCode())
            json.insert("manufacturerCode", it.value()->manufacturerCode());

        if (it.value()->name() != it.value()->ieeeAddress().toHex(':'))
            json.insert("name", it.value()->name());

        if (!it.value()->vendor().isEmpty())
            json.insert("vendor", it.value()->vendor());

        if (!it.value()->model().isEmpty())
            json.insert("model", it.value()->model());

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
            QJsonArray inClustersArray;

            for (int i = 0; i < it.value()->inClusters().count(); i++)
                inClustersArray.append(it.value()->inClusters().at(i));

            json.insert("inClusters", inClustersArray);
        }

        if (!it.value()->outClusters().isEmpty())
        {
            QJsonArray outClustersArray;

            for (int i = 0; i < it.value()->outClusters().count(); i++)
                outClustersArray.append(it.value()->outClusters().at(i));

            json.insert("outClusters", outClustersArray);
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

void ZigBee::enqueueBindRequest(const Device &device, quint8 endpointId, quint16 clusterId)
{
    BindRequest request(new BindRequestObject(device, endpointId, clusterId));

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

        if (it.value()->inClusters().contains(CLUSTER_BASIC) && (device->vendor().isEmpty() || device->model().isEmpty()))
        {
            readAttributes(device, it.key(), CLUSTER_BASIC, {0x0004, 0x0005});
            return;
        }
    }

    logInfo << "Device" << device->name() << "vendor is" << device->vendor() << "and model is" << device->model();
    setupDevice(device);
    configureReportings(device);

    logInfo << "Device" << device->name() << "interview finished";
    device->setInterviewFinished();
    storeStatus();
}

void ZigBee::setupDevice(const Device &device)
{
    QFile file(m_libraryFile);
    QJsonObject json;
    QJsonArray array;

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        logWarning << "Can't open library file, devices not loaded";
        return;
    }

    array = QJsonDocument::fromJson(file.readAll()).object().value(device->vendor()).toArray();
    file.close();

    if (array.isEmpty())
    {
        logWarning << "Device" << device->name() << "vendor" << device->vendor() << "unrecognized";
        return;
    }

    for (auto it = array.begin(); it != array.end(); it++)
    {
        QJsonObject item = it->toObject();
        QJsonValue model = item.value("model");

        if ((model.type() == QJsonValue::String && model.toString() == device->model()) || (model.type() == QJsonValue::Array && model.toArray().contains(device->model())))
        {
            json = QJsonObject(item);
            break;
        }
    }

    if (!json.isEmpty())
    {
        QJsonArray actionsArray = json.value("actions").toArray(), propertiesArray = json.value("properties").toArray(), reportingsArray = json.value("reportings").toArray(), pollsArray = json.value("polls").toArray();
        quint32 pollInterval = static_cast <quint8> (json.value("pollInterval").toInt());
        quint8 endpointId = static_cast <quint8> (json.value("endpointId").toInt());

        disconnect(device->timer(), &QTimer::timeout, this, &ZigBee::pollAttributes);
        device->timer()->stop();

        device->actions().clear();
        device->properties().clear();
        device->reportings().clear();
        device->polls().clear();

        for (auto it = actionsArray.begin(); it != actionsArray.end(); it++)
        {
            int type = QMetaType::type(QString(it->toString()).append("Action").toUtf8());

            if (type)
            {
                Action action(reinterpret_cast <ActionObject*> (QMetaType::create(type)));

                if (endpointId)
                    action->setEndpointId(endpointId);

                device->actions().append(action);
                continue;
            }

            logWarning << "Device" << device->name() << "action" << it->toString() << "unrecognized";
        }

        for (auto it = propertiesArray.begin(); it != propertiesArray.end(); it++)
        {
            int type = QMetaType::type(QString(it->toString()).append("Property").toUtf8());

            if (type)
            {
                Property property(reinterpret_cast <PropertyObject*> (QMetaType::create(type)));
                device->properties().append(property);
                continue;
            }

            logWarning << "Device" << device->name() << "property" << it->toString() << "unrecognized";
        }

        for (auto it = reportingsArray.begin(); it != reportingsArray.end(); it++)
        {
            int type = QMetaType::type(QString(it->toString()).append("Reporting").toUtf8());

            if (type)
            {
                Reporting reporting(reinterpret_cast <ReportingObject*> (QMetaType::create(type)));

                if (endpointId)
                    reporting->setEndpointId(endpointId);

                device->reportings().append(reporting);
                continue;
            }

            logWarning << "Device" << device->name() << "reporting" << it->toString() << "unrecognized";
        }

        for (auto it = pollsArray.begin(); it != pollsArray.end(); it++)
        {
            int type = QMetaType::type(QString(it->toString()).append("Poll").toUtf8());

            if (type)
            {
                Poll poll(reinterpret_cast <PollObject*> (QMetaType::create(type)));

                if (endpointId)
                    poll->setEndpointId(endpointId);

                device->polls().append(poll);
                continue;
            }

            logWarning << "Device" << device->name() << "poll" << it->toString() << "unrecognized";
        }

        if (pollInterval && !device->polls().isEmpty())
        {
            logInfo << "Device" << device->name() << "poll interval:" << pollInterval << "seconds";
            connect(device->timer(), &QTimer::timeout, this, &ZigBee::pollAttributes);
            device->timer()->start(pollInterval * 1000);
        }

        return;
    }

    logWarning << "Device" << device->name() << "model" << device->model() << "unrecognized";
}

void ZigBee::configureReportings(const Device &device)
{
    for (int i = 0; i < device->reportings().count(); i++)
    {
        const Reporting &reporting = device->reportings().at(i);
        zclHeaderStruct header;
        configureReportingStruct payload;
        size_t length = sizeof(payload);

        header.frameControl = 0x00;
        header.transactionId = m_transactionId++;
        header.commandId = CMD_CONFIGURE_REPORTING;

        payload.direction = 0x00;
        payload.attributeId = qToLittleEndian(reporting->attributeId());
        payload.dataType = reporting->dataType();
        payload.minInterval = qToLittleEndian(reporting->minInterval());
        payload.maxInterval = qToLittleEndian(reporting->maxInterval());
        payload.valueChange = qToLittleEndian(reporting->valueChange());

        if (payload.dataType == DATA_TYPE_BOOLEAN || payload.dataType == DATA_TYPE_8BIT_UNSIGNED || payload.dataType == DATA_TYPE_8BIT_SIGNED)
            length -= 1;

        enqueueBindRequest(device, reporting->endpointId(), reporting->clusterId());
        enqueueDataRequest(device, reporting->endpointId(), reporting->clusterId(), QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), length), QString("%1 reporting configuration").arg(reporting->name()));
    }
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

    if (clusterId == CLUSTER_BASIC && (attributeId == 0x0004 || attributeId == 0x0005))
    {
        if (device->interviewFinished() || dataType != DATA_TYPE_CHARACTER_STRING)
            return;

        switch (attributeId)
        {
            case 0x0004:
                device->setVendor(QString(data).trimmed());
                break;

            case 0x0005:
                device->setModel(QString(data).trimmed());
                break;
        }

        if (!device->vendor().isEmpty() && !device->model().isEmpty())
        {
            if (device->model() == "TS0601") // vendor is model for devices based on Tuya TS0601
            {
                device->model() = device->vendor();
                device->vendor() = "TUYA";
            }

            interviewDevice(device);
        }

        return;
    }

    if (!device->interviewFinished())
        return;

    for (int i = 0; i < device->properties().count(); i++)
    {
        const Property &property = device->properties().at(i);

        if (property->clusterId() == clusterId)
        {
            QVariant value = property->value();

            property->parseAttribte(attributeId, dataType, data);
            check = true;

            if (property->value() == value)
                continue;

            endpoint->setDataUpdated();
        }
    }

    if (!check)
        logWarning << "No property found for device" << device->name() << "cluster" << QString::asprintf("0x%04X", clusterId) << "attribute" << QString::asprintf("0x%04X", attributeId) << "with data type" << QString::asprintf("0x%02X", dataType) << "and data" << data.toHex(':');
}

void ZigBee::clusterCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint8 commandId, const QByteArray &payload)
{
    Device device = endpoint->device();
    bool check = false;

    if (!device->interviewFinished())
        return;

    if (clusterId == CLUSTER_OTA_UPGRADE)
    {
        if (commandId == 0x01)
        {
            zclHeaderStruct header;

            header.frameControl = FC_CLUSTER_SPECIFIC | FC_SERVER_TO_CLIENT;
            header.transactionId = transactionId;
            header.commandId = 0x02;

            enqueueDataRequest(device, endpoint->id(), clusterId, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(0x98));
        }

        return;
    }

    for (int i = 0; i < device->properties().count(); i++)
    {
        const Property &property = device->properties().at(i);

        if (property->clusterId() == clusterId)
        {
            QVariant value = property->value();

            property->parseCommand(commandId, payload);
            check = true;

            if (property->value() == value)
                continue;

            endpoint->setDataUpdated();
        }
    }

    if (!check)
        logWarning << "No property found for device" << device->name() << "cluster" << QString::asprintf("0x%04X", clusterId) << "command" << QString::asprintf("0x%02X", commandId) << "with payload" << payload.toHex(':');
}

void ZigBee::globalCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint8 commandId, QByteArray payload)
{
    Q_UNUSED(transactionId)

    switch (commandId)
    {
        case CMD_READ_ATTRIBUTES: // TODO: tyua sensor reading time cluster
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

                switch (dataType)
                {
                    case DATA_TYPE_BOOLEAN:
                    case DATA_TYPE_8BIT_BITMAP:
                    case DATA_TYPE_8BIT_UNSIGNED:
                    case DATA_TYPE_8BIT_SIGNED:
                        size = 1;
                        break;

                    case DATA_TYPE_16BIT_BITMAP:
                    case DATA_TYPE_16BIT_UNSIGNED:
                    case DATA_TYPE_16BIT_SIGNED:
                        size = 2;
                        break;

                    case DATA_TYPE_24BIT_BITMAP:
                    case DATA_TYPE_24BIT_UNSIGNED:
                    case DATA_TYPE_24BIT_SIGNED:
                        size = 3;
                        break;

                    case DATA_TYPE_32BIT_BITMAP:
                    case DATA_TYPE_32BIT_UNSIGNED:
                    case DATA_TYPE_32BIT_SIGNED:
                    case DATA_TYPE_SINGLE_PRECISION:
                        size = 4;
                        break;

                    case DATA_TYPE_40BIT_BITMAP:
                    case DATA_TYPE_40BIT_UNSIGNED:
                    case DATA_TYPE_40BIT_SIGNED:
                        size = 5;
                        break;

                    case DATA_TYPE_48BIT_BITMAP:
                    case DATA_TYPE_48BIT_UNSIGNED:
                    case DATA_TYPE_48BIT_SIGNED:
                        size = 6;
                        break;

                    case DATA_TYPE_56BIT_BITMAP:
                    case DATA_TYPE_56BIT_UNSIGNED:
                    case DATA_TYPE_56BIT_SIGNED:
                        size = 7;
                        break;

                    case DATA_TYPE_64BIT_BITMAP:
                    case DATA_TYPE_64BIT_UNSIGNED:
                    case DATA_TYPE_64BIT_SIGNED:
                    case DATA_TYPE_DOUBLE_PRECISION:
                        size = 8;
                        break;

                    case DATA_TYPE_OCTET_STRING:
                    case DATA_TYPE_CHARACTER_STRING:
                        size = static_cast <quint8> (payload.at(offset++));
                        break;

                    case DATA_TYPE_STRUCTURE:
                        size = static_cast <quint8> (payload.length() - 3); // TODO: check this
                        break;

                    default:
                        logWarning << "Unrecognized attribute" << QString::asprintf("0x%04X", attributeId) << "data type" << QString::asprintf("0x%02X", dataType) << "received from device" << endpoint->device()->name() << "cluster" << QString::asprintf("0x%04X", clusterId) << "with payload:" << payload.mid(offset).toHex(':');
                        break;
                }

                if (size && payload.length() >= offset + size)
                    parseAttribute(endpoint, clusterId, attributeId, dataType, payload.mid(offset, size));

                payload.remove(0, offset + size);
            }

            break;
        }

        default:
            logWarning << "Unrecognized command" << QString::asprintf("0x%02X", commandId) << "received from device" << endpoint->device()->name() << "cluster" << QString::asprintf("0x%04X", clusterId) << "with payload:" << payload.toHex(':');
            break;
    }
}

void ZigBee::coordinatorReady(const QByteArray &ieeeAddress)
{
    Device device(new DeviceObject(ieeeAddress));

    logInfo << "Coordinator ready, address" << ieeeAddress.toHex(':');

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

    m_coordinatorReady = true;
    m_adapter->setPermitJoin(m_permitJoin);

    m_queuesTimer->start(HANDLE_QUEUES_INTERVAL);
    m_neighborsTimer->start(UPDATE_NEIGHBORS_INTERVAL);

    storeStatus();
}

void ZigBee::endDeviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress, quint8 capabilities)
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
    emit endDeviceEvent();
}

void ZigBee::endDeviceLeft(const QByteArray &ieeeAddress, quint16 networkAddress)
{
    auto it = m_devices.find(ieeeAddress);

    if (it == m_devices.end())
        return;

    m_ledTimer->start(500);
    GPIO::setStatus(m_ledPin, true);

    logInfo << "Device" << it.value()->name() << "with address" << QString::asprintf("0x%04X", networkAddress) << "left network";

    m_devices.erase(it);
    storeStatus();

    emit endDeviceEvent(false);
}

void ZigBee::nodeDescriptorReceived(quint16 networkAddress, quint16 manufacturerCode, LogicalType logicalType)
{
    Device device = findDevice(networkAddress);

    if (device.isNull())
        return;

    device->setManufacturerCode(manufacturerCode);
    device->setLogicalType(logicalType);

    logInfo << "Device" << device->name() << "node descriptor received, manufacturer code:" << QString::asprintf("0x%04X", device->manufacturerCode());

    if (device->logicalType() == LogicalType::Router)
        logInfo << "Device" << device->name() << "is router";

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
        list.append(QString::number(static_cast <quint8> (data.at(i))));

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

    logInfo << "Device" << device->name() << "endpoint" << endpointId << "simple descriptor received";

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
        emit endpointUpdated(endpoint);
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

void ZigBee::messageReveivedExt(const QByteArray &ieeeAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data)
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
    auto it = m_devices.find(reinterpret_cast <DeviceObject*> (sender()->parent())->ieeeAddress());

    if (it == m_devices.end())
        return;

    for (int i = 0; i < it.value()->polls().count(); i++)
    {
        Poll poll = it.value()->polls().at(i);
        readAttributes(it.value(), poll->endpointId(), poll->clusterId(), poll->attributes());
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
    if (!m_coordinatorReady)
        return;

    while (!m_bindQueue.isEmpty())
    {
        BindRequest request = m_bindQueue.dequeue();

        if (m_adapter->bindRequest(request->device()->networkAddress(), request->device()->ieeeAddress(), request->endpointId(), request->clusterId()))
            continue;

        logWarning << "Device" << request->device()->name() << "cluster" << QString::asprintf("0x%04X", request->clusterId()) << "binding failed";
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
