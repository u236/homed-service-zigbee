#include <QtEndian>
#include <QFile>
#include "logger.h"
#include "zigbee.h"

ZigBee::ZigBee(QSettings *config, QObject *parent) : QObject(parent), m_adapter(new ZStack(config, this)), m_timer(new QTimer(this)), m_transactionId(0), m_permitJoin(true)
{
    m_databaseFile = config->value("zigbee/database", "/var/db/homed/zigbee.json").toString();

    connect(m_adapter, &ZStack::coordinatorReady, this, &ZigBee::coordinatorReady);
    connect(m_adapter, &ZStack::endDeviceJoined, this, &ZigBee::endDeviceJoined);
    connect(m_adapter, &ZStack::endDeviceLeft, this, &ZigBee::endDeviceLeft);
    connect(m_adapter, &ZStack::nodeDescriptorReceived, this, &ZigBee::nodeDescriptorReceived);
    connect(m_adapter, &ZStack::activeEndPointsReceived, this, &ZigBee::activeEndPointsReceived);
    connect(m_adapter, &ZStack::simpleDescriptorReceived, this, &ZigBee::simpleDescriptorReceived);
    connect(m_adapter, &ZStack::messageReveived, this, &ZigBee::messageReveived);

    connect(m_timer, &QTimer::timeout, this, &ZigBee::storeStatus);
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

    m_timer->setSingleShot(true);
    m_adapter->init();
}

void ZigBee::setPermitJoin(bool enabled)
{
    m_permitJoin = enabled;
    m_adapter->setPermitJoin(m_permitJoin);
}

void ZigBee::deviceAction(const QByteArray &ieeeAddress, const QString &actionName, const QVariant &actionData)
{
    auto it = m_devices.find(ieeeAddress);

    if (it == m_devices.end())
        return;

    for (quint8 i = 0; i < static_cast <quint8> (it.value()->actions().count()); i++)
    {
        Action action = it.value()->actions().value(i);

        if (action->name() == actionName)
        {
            QByteArray data = action->request(actionData);

            if (data.isEmpty() || m_adapter->dataRequest(it.value()->networkAddress(), action->endPointId(), action->clusterId(), data))
                continue;

            logWarning << "Device" << it.value()->name() << actionName << "action failed";
        }
    }
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

            unserializeEndPoints(device, json.value("endPoints").toArray());

            if (device->interviewFinished())
                device->setProperties();

            m_devices.insert(device->ieeeAddress(), device);
        }
    }
}

void ZigBee::unserializeEndPoints(const Device &device, const QJsonArray &array)
{
    for (auto it = array.begin(); it != array.end(); it++)
    {
        QJsonObject json = it->toObject();

        if (json.contains("endPointId"))
        {
            EndPoint endPoint(new EndPointObject(device));
            QJsonArray inClustersArray = json.value("inClusters").toArray(), outClustersArray = json.value("outClusters").toArray();

            endPoint->setProfileId(static_cast <quint16> (json.value("profileId").toInt()));
            endPoint->setDeviceId(static_cast <quint16> (json.value("deviceId").toInt()));

            for (const QJsonValue &clusterId : inClustersArray)
                endPoint->inClusters().append(static_cast <quint16> (clusterId.toInt()));

            for (const QJsonValue &clusterId : outClustersArray)
                endPoint->outClusters().append(static_cast <quint16> (clusterId.toInt()));

            device->endPoints().insert(static_cast <quint8> (json.value("endPointId").toInt()), endPoint);
        }
    }
}

QJsonArray ZigBee::serializeDevices(void)
{
    QJsonArray array;

    for (auto it = m_devices.begin(); it != m_devices.end(); it++)
    {
        QJsonObject json;

        json.insert("ieeeAddress", QString(it.value()->ieeeAddress().toHex(':')));
        json.insert("networkAddress", it.value()->networkAddress());
        json.insert("logicalType", static_cast <quint8> (it.value()->logicalType()));
        json.insert("ineterviewFinished", it.value()->interviewFinished());

        if (it.value()->manufacturerCode())
            json.insert("manufacturerCode", it.value()->manufacturerCode());

        if (it.value()->name() != it.value()->ieeeAddress().toHex(':'))
            json.insert("name", it.value()->name());

        if (!it.value()->vendor().isEmpty())
            json.insert("vendor", it.value()->vendor());

        if (!it.value()->model().isEmpty())
            json.insert("model", it.value()->model());

        if (it.value()->lastSeen())
            json.insert("lastSeen", it.value()->lastSeen());

        if (!it.value()->endPoints().isEmpty())
            json.insert("endPoints", serializeEndPoints(it.value()));

        array.append(json);
    }

    return array;
}

QJsonArray ZigBee::serializeEndPoints(const Device &device)
{
    QJsonArray array;

    for (auto it = device->endPoints().begin(); it != device->endPoints().end(); it++)
    {
        QJsonObject json;

        json.insert("endPointId", it.key());

        if (it.value()->profileId())
            json.insert("profileId", it.value()->profileId());

        if (it.value()->deviceId())
            json.insert("deviceId", it.value()->deviceId());

        if (!it.value()->inClusters().isEmpty())
        {
            QJsonArray inClustersArray;

            for (quint8 i = 0; i < static_cast <quint8> (it.value()->inClusters().count()); i++)
                inClustersArray.append(it.value()->inClusters().value(i));

            json.insert("inClusters", inClustersArray);
        }

        if (!it.value()->outClusters().isEmpty())
        {
            QJsonArray outClustersArray;

            for (quint8 i = 0; i < static_cast <quint8> (it.value()->outClusters().count()); i++)
                outClustersArray.append(it.value()->outClusters().value(i));

            json.insert("outClusters", outClustersArray);
        }

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

void ZigBee::interviewDevice(const Device &device)
{
    if (device->interviewFinished())
        return;

    if (!device->manufacturerCode())
    {
        m_adapter->nodeDescriptorRequest(device->networkAddress());
        return;
    }

    if (device->endPoints().isEmpty())
    {
        m_adapter->activeEndPointsRequest(device->networkAddress());
        return;
    }

    for (auto it = device->endPoints().begin(); it != device->endPoints().end(); it++)
    {
        if (!it.value()->profileId() && !it.value()->deviceId())
        {
            m_adapter->simpleDescriptorRequest(device->networkAddress(), it.key());
            return;
        }

        if (it.value()->inClusters().contains(CLUSTER_BASIC) && (device->vendor().isEmpty() || device->model().isEmpty()))
        {
            readAttributes(device, it.key(), CLUSTER_BASIC, {ATTRIBUTE_BASIC_VENDOR, ATTRIBUTE_BASIC_MODEL});
            return;
        }
    }

    logInfo << "Device" << device->name() << "vendor is" << device->vendor() << "and model is" << device->model();
    device->setProperties();

    for (quint8 i = 0; i < static_cast <quint8> (device->reportings().count()); i++)
    {
        Reporting reporting = device->reportings().value(i);
        zclHeaderStruct header;
        configureReportingStruct payload;
        size_t length = sizeof(payload);

        if (!m_adapter->bindRequest(device->networkAddress(), device->ieeeAddress(), reporting->endPointId(), reporting->clusterId()))
        {
            logWarning << "Device" << device->name() << "cluster" << QString::asprintf("0x%04X", reporting->clusterId()) << "binding failed";
            continue;
        }

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

        if (!m_adapter->dataRequest(device->networkAddress(), reporting->endPointId(), reporting->clusterId(), QByteArray(reinterpret_cast <char*> (&header)).append(reinterpret_cast <char*> (&payload), length)))
        {
            logWarning << "Device" << device->name() << reporting->name() << "reporting configuration failed";
            continue;
        }

        logInfo << "Device" << device->name() << reporting->name() << "reporting configured successfully";
    }

    logInfo << "Device" << device->name() << "interview finished";
    device->setInterviewFinished();
    storeStatus();
}

void ZigBee::readAttributes(const Device &device, quint8 endPointId, quint16 clusterId, QList <quint16> attributes)
{
    zclHeaderStruct header;
    QByteArray payload;

    header.frameControl = 0x00;
    header.transactionId = m_transactionId++;
    header.commandId = CMD_READ_ATTRIBUTES;

    for (quint8 i = 0; i < static_cast <quint8> (attributes.length()); i++)
    {
        quint16 attributeId = qToLittleEndian(attributes.value(i));
        payload.append(reinterpret_cast <char*> (&attributeId), sizeof(attributeId));
    }

    if (m_adapter->dataRequest(device->networkAddress(), endPointId, clusterId, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(payload)))
        return;

    logWarning << "Device" << device->name() << "read attributes request failed";
}

void ZigBee::parseAttribute(const EndPoint &endPoint, quint16 clusterId, quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (clusterId == CLUSTER_BASIC && (attributeId == ATTRIBUTE_BASIC_VENDOR || attributeId == ATTRIBUTE_BASIC_MODEL))
    {
        if (dataType != DATA_TYPE_STRING)
            return;

        switch (attributeId)
        {
            case ATTRIBUTE_BASIC_VENDOR:
                endPoint->device()->setVendor(QString(data).trimmed());
                break;

            case ATTRIBUTE_BASIC_MODEL:
                endPoint->device()->setModel(QString(data).trimmed());
                break;
        }

        if (!endPoint->device()->vendor().isEmpty() && !endPoint->device()->model().isEmpty())
            interviewDevice(endPoint->device());

        return;
    }

    if (!endPoint->device()->interviewFinished())
        return;

    for (quint8 i = 0; i < static_cast <quint8> (endPoint->device()->properties().count()); i++)
    {
        Property property = endPoint->device()->properties().value(i);

        if (property->clusterId() == clusterId)
        {
            Cluster cluster = endPoint->cluster(clusterId);
            Attribute attribute = cluster->attribute(attributeId);

            attribute->setDataType(dataType);
            attribute->setData(data);

            property->convert(cluster);
            endPoint->setDataUpdated(true);
        }
    }

    if (endPoint->dataUpdated())
        return;

    logWarning << "No property found for device" << endPoint->device()->name() << "cluster" << QString::asprintf("0x%04X", clusterId) << "attribute" << QString::asprintf("0x%04X", attributeId);
}

void ZigBee::clusterCommandReceived(const EndPoint &endPoint, quint16 clusterId, quint8 transactionId, quint8 commandId, QByteArray payload)
{
    Q_UNUSED(transactionId)
    logInfo << "Cluster specific command" << QString::asprintf("0x%02X", commandId) << "received from device" << endPoint->device()->ieeeAddress().toHex(':') << "cluster" << QString::asprintf("0x%04X", clusterId) << "with payload:" << payload.toHex(':');
}

void ZigBee::globalCommandReceived(const EndPoint &endPoint, quint16 clusterId, quint8 transactionId, quint8 commandId, QByteArray payload)
{   
    Q_UNUSED(transactionId)

    switch (commandId)
    {
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

                    case DATA_TYPE_16BIT_UNSIGNED:
                    case DATA_TYPE_16BIT_SIGNED:
                        size = 2;
                        break;

                    case DATA_TYPE_SINGLE_PRECISION:
                        size = 4;
                        break;

                    case DATA_TYPE_STRING:
                        size = static_cast <quint8> (payload.at(offset++));
                        break;

                        // TODO: check this
                    case DATA_TYPE_STRUCTURE:
                        size = static_cast <quint8> (payload.length() - 3);
                        break;

                    default:
                        logWarning << "Unrecognized attribute" << QString::asprintf("0x%04X", attributeId) << "data type" << QString::asprintf("0x%02X", dataType) << "received from device" << endPoint->device()->name();
                        break;
                }

                if (size && payload.length() >= offset + size)
                    parseAttribute(endPoint, clusterId, attributeId, dataType, payload.mid(offset, size));

                payload.remove(0, offset + size);
            }

            break;
        }

        default:
            logWarning << "Unrecognized command" << QString::asprintf("0x%02X", commandId) << "received from device" << endPoint->device()->name() << "cluster" << QString::asprintf("0x%04X", clusterId) << "with payload:" << payload.toHex(':');
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
    device->setInterviewFinished();

    device->endPoints().insert(0x01, EndPoint(new EndPointObject(device, PROFILE_HA,    0x0005)));
    device->endPoints().insert(0x02, EndPoint(new EndPointObject(device, PROFILE_IPM,   0x0005)));
    device->endPoints().insert(0x03, EndPoint(new EndPointObject(device, PROFILE_HA,    0x0005)));
    device->endPoints().insert(0x04, EndPoint(new EndPointObject(device, PROFILE_TA,    0x0005)));
    device->endPoints().insert(0x05, EndPoint(new EndPointObject(device, PROFILE_PHHC,  0x0005)));
    device->endPoints().insert(0x06, EndPoint(new EndPointObject(device, PROFILE_AMI,   0x0005)));
    device->endPoints().insert(0x08, EndPoint(new EndPointObject(device, PROFILE_HA,    0x0005)));
    device->endPoints().insert(0x0A, EndPoint(new EndPointObject(device, PROFILE_HA,    0x0005)));
    device->endPoints().insert(0x0B, EndPoint(new EndPointObject(device, PROFILE_HA,    0x0400)));
    device->endPoints().insert(0x0C, EndPoint(new EndPointObject(device, PROFILE_ZLL,   0x0005)));
    device->endPoints().insert(0x0D, EndPoint(new EndPointObject(device, PROFILE_HA,    0x0005)));
    device->endPoints().insert(0x2F, EndPoint(new EndPointObject(device, PROFILE_HA,    0x0005)));
    device->endPoints().insert(0x6E, EndPoint(new EndPointObject(device, PROFILE_HA,    0x0005)));
    device->endPoints().insert(0xF2, EndPoint(new EndPointObject(device, PROFILE_HUE,   0x0005)));

    device->endPoints().value(0x0B)->inClusters().append(CLUSTER_TIME);
    device->endPoints().value(0x0B)->inClusters().append(CLUSTER_IAS_ACE);
    device->endPoints().value(0x0B)->outClusters().append(CLUSTER_IAS_ZONE);
    device->endPoints().value(0x0B)->outClusters().append(CLUSTER_IAS_WD);
    device->endPoints().value(0x0D)->inClusters().append(CLUSTER_OTA_UPGRADE);

    for (auto it = device->endPoints().begin(); it != device->endPoints().end(); it++)
        m_adapter->registerEndPoint(it.key(), it.value()->profileId(), it.value()->deviceId(), it.value()->inClusters(), it.value()->outClusters());

    m_devices.insert(ieeeAddress, device);
    m_adapter->setPermitJoin(m_permitJoin);
    storeStatus();
}

void ZigBee::endDeviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress, quint8 capabilities)
{
    auto it = m_devices.find(ieeeAddress);

    logInfo << "Device" << ieeeAddress.toHex(':') << "with address" << QString::asprintf("0x%04X", networkAddress) << "joined network, capabilities:" << QString::asprintf("0x%02X", capabilities);

    if (it != m_devices.end())
    {
        it.value()->setNetworkAddress(networkAddress);

        logInfo << "here we are";
    }
    else
        it = m_devices.insert(ieeeAddress, Device(new DeviceObject(ieeeAddress, networkAddress)));

    interviewDevice(it.value());
    it.value()->updateLastSeen();
}

void ZigBee::endDeviceLeft(const QByteArray &ieeeAddress, quint16 networkAddress)
{
    auto it = m_devices.find(ieeeAddress);

    logInfo << "Device" << ieeeAddress.toHex(':') << "with address" << QString::asprintf("0x%04X", networkAddress) << "left network";

    if (it != m_devices.end())
        m_devices.erase(it);

    storeStatus();
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
    {
        logInfo << "Device" << device->name() << "is router";
        // m_zigbee->lqiRequest(device->network());
    }

    interviewDevice(device);
    device->updateLastSeen();
}

void ZigBee::activeEndPointsReceived(quint16 networkAddress, const QByteArray data)
{
    Device device = findDevice(networkAddress);
    QStringList list;

    if (device.isNull())
        return;

    for (quint8 i = 0; i < static_cast <quint8> (data.length()); i++)
        list.append(QString::number(static_cast <quint8> (data.at(i))));

    logInfo << "Device" << device->name() << "active endPoints received:" << list.join(", ");

    for (quint8 i = 0; i < static_cast <quint8> (data.length()); i++)
        if (device->endPoints().find(static_cast <quint8> (data.at(i))) == device->endPoints().end())
            device->endPoints().insert(static_cast <quint8> (data.at(i)), EndPoint(new EndPointObject(device)));

    interviewDevice(device);
    device->updateLastSeen();
}

void ZigBee::simpleDescriptorReceived(quint16 networkAddress, quint8 endPointId, quint16 profileId, quint16 deviceId, const QList<quint16> &inClusters, const QList<quint16> &outClusters)
{
    Device device = findDevice(networkAddress);
    EndPoint endPoint;

    if (device.isNull())
        return;

    endPoint = device->endPoints().value(endPointId);

    if (endPoint.isNull())
        return;

    endPoint->setProfileId(profileId);
    endPoint->setDeviceId(deviceId);

    endPoint->inClusters() = inClusters;
    endPoint->outClusters() = outClusters;

    logInfo << "Device" << device->name() << "endPoint" << endPointId << "simple descriptor received";

    interviewDevice(device);
    device->updateLastSeen();
}

void ZigBee::messageReveived(quint16 networkAddress, quint8 endPointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data)
{
    Device device = findDevice(networkAddress);
    EndPoint endPoint;
    zclHeaderStruct header;
    QByteArray payload;

    if (device.isNull())
        return;

    endPoint = device->endPoints().value(endPointId);

    if (endPoint.isNull())
        return;

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
        clusterCommandReceived(endPoint, clusterId, header.transactionId, header.commandId, payload);
    else
        globalCommandReceived(endPoint, clusterId, header.transactionId, header.commandId, payload);

    //    if (!device->interviewFinished())
    //        interviewDevice(device);

    device->setLinkQuality(linkQuality);
    device->updateLastSeen();

    if (endPoint->dataUpdated())
    {
        endPoint->setDataUpdated(false);
        emit endPointUpdated(endPoint);
    }
}

void ZigBee::storeStatus(void)
{
    QFile file(m_databaseFile);
    QJsonObject json;

    m_timer->start(STORE_DEVICES_INTERVAL);

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        logWarning << "Can't store status";
        return;
    }

    json.insert("devices", serializeDevices());
    json.insert("permitJoin", m_permitJoin);

    file.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    file.close();

    logInfo << "Status stored successfully";
    emit statusStored(json);
}
