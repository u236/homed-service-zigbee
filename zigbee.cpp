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

    connect(m_timer, &QTimer::timeout, this, &ZigBee::storeState);
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

    m_timer->start(STORE_DEVICES_INTERVAL);
    m_adapter->init();
}

void ZigBee::setPermitJoin(bool enabled)
{
    m_permitJoin = enabled;
    m_adapter->setPermitJoin(m_permitJoin);
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

            device->setName(json.value("name").toString());
            device->setManufacturerCode(static_cast <quint16> (json.value("manufacturerCode").toInt()));
            device->setLogicalType(static_cast <LogicalType> (json.value("logicalType").toInt()));
            device->setLastSeen(json.value("lastSeen").toInt());

            unserializeEndPoints(device, json.value("endPoints").toArray());
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

        if (it.value()->name() != it.value()->ieeeAddress().toHex(':'))
            json.insert("name", it.value()->name());

        if (it.value()->manufacturerCode())
            json.insert("manufacturerCode", it.value()->manufacturerCode());

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

        // TODO: check for vendor/modle is set or return here
        if (it.value()->inClusters().contains(CLUSTER_BASIC))
        {
            readAttributes(device, it.key(), CLUSTER_BASIC, {ATTRIBUTE_BASIC_VENDOR, ATTRIBUTE_BASIC_MODEL});
            // return;
        }
    }

    logInfo << "Device" << device->name() << "interview finished";
    device->setInterviewFinished();
    storeState();
}

void ZigBee::readAttributes(const Device &device, quint8 endPointId, quint16 clusterId, QList <quint16> attributes)
{
    zclHeaderStruct header;
    QByteArray payload;

    header.frameControl = 0x00;
    header.transactionId = m_transactionId;
    header.commandId = 0x00;

    for (quint8 i = 0; i < static_cast <quint8> (attributes.length()); i++)
    {
        quint16 attributeId = qToLittleEndian(attributes.value(i));
        payload.append(reinterpret_cast <char*> (&attributeId), sizeof(attributeId));
    }

    if (m_adapter->dataRequest(device->networkAddress(), endPointId, clusterId, QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(payload)))
        return;

    logWarning << "Device" << device->name() << "read attributes request failed";
}

void ZigBee::clusterCommandReceived(EndPoint endPoint, quint16 clusterId, quint8 transactionId, quint8 commandId, const QByteArray &payload)
{
    logInfo << "Cluster specific command" << commandId << "received from" << endPoint->device()->ieeeAddress().toHex(':') << "cluster" << QString::asprintf("0x%04X", clusterId) << "payload" << payload.toHex(':') << "id" << transactionId;
}

void ZigBee::globalCommandReceived(EndPoint endPoint, quint16 clusterId, quint8 transactionId, quint8 commandId, const QByteArray &payload)
{
    logInfo << "Global command" << commandId << "received from" << endPoint->device()->ieeeAddress().toHex(':') << "cluster" << QString::asprintf("0x%04X", clusterId) << "payload" << payload.toHex(':') << "id" << transactionId;
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
    storeState();
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

    if (device.isNull())
        return;

    logInfo << "Device" << device->name() << "active endPoints received:" << data.toHex('|');

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
    Q_UNUSED(linkQuality)

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

    device->updateLastSeen();
}

void ZigBee::storeState(void)
{
    QFile file(m_databaseFile);
    QJsonObject json;

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        logWarning << "Can't store state";
        return;
    }

    json.insert("devices", serializeDevices());
    json.insert("permitJoin", m_permitJoin);

    file.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    file.close();

    logInfo << "State stored successfully";
    emit stateUpdated(json);
}
