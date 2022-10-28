#include <QFile>
#include "device.h"
#include "logger.h"

DeviceList::DeviceList(QSettings *config) : m_timer(new QTimer(this)), m_file(config->value("zigbee/database", "/var/db/homed/zigbee.json").toString()), m_permitJoin(false)
{
    if (m_file.open(QFile::ReadOnly | QFile::Text))
    {
        QJsonObject json = QJsonDocument::fromJson(m_file.readAll()).object();
        unserializeDevices(json.value("devices").toArray());
        m_permitJoin = json.value("permitJoin").toBool();
        m_file.close();
    }

    connect(m_timer, &QTimer::timeout, this, &DeviceList::storeStatus);
}

Device DeviceList::byNetwork(quint16 networkAddress)
{
    for (auto it = begin(); it != end(); it++)
        if (it.value()->networkAddress() == networkAddress)
            return it.value();

    return Device();
}

void DeviceList::unserializeDevices(const QJsonArray &devices)
{
    for (auto it = devices.begin(); it != devices.end(); it++)
    {
        QJsonObject json = it->toObject();

        if (json.contains("ieeeAddress") && json.contains("networkAddress"))
        {
            Device device(new DeviceObject(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint16> (json.value("networkAddress").toInt())));

            device->setLogicalType(static_cast <LogicalType> (json.value("logicalType").toInt()));
            device->setManufacturerCode(static_cast <quint16> (json.value("manufacturerCode").toInt()));
            device->setVersion(static_cast <quint8> (json.value("version").toInt()));
            device->setPowerSource(static_cast <quint8> (json.value("powerSource").toInt()));
            device->setManufacturerName(json.value("manufacturerName").toString());
            device->setModelName(json.value("modelName").toString());
            device->setName(json.value("name").toString());
            device->setLastSeen(json.value("lastSeen").toInt());

            unserializeEndpoints(device, json.value("endpoints").toArray());
            unserializeNeighbors(device, json.value("neighbors").toArray());

            if (json.value("ineterviewFinished").toBool())
                device->setInterviewFinished();

            insert(device->ieeeAddress(), device);
        }
    }

    if (isEmpty())
        return;

    logInfo << count() << "devices loaded";
}

void DeviceList::unserializeEndpoints(const Device &device, const QJsonArray &endpoints)
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

void DeviceList::unserializeNeighbors(const Device &device, const QJsonArray &neighbors)
{
    for (auto it = neighbors.begin(); it != neighbors.end(); it++)
    {
        QJsonObject json = it->toObject();

        if (!json.contains("networkAddress") || !json.contains("linkQuality"))
            continue;

        device->neighbors().insert(static_cast <quint16> (json.value("networkAddress").toInt()), static_cast <quint8> (json.value("linkQuality").toInt()));
    }
}

QJsonArray DeviceList::serializeDevices(void)
{
    QJsonArray array;

    for (auto it = begin(); it != end(); it++)
    {
        QJsonObject json = {{"ieeeAddress", QString(it.value()->ieeeAddress().toHex(':'))}, {"networkAddress", it.value()->networkAddress()}, {"logicalType", static_cast <quint8> (it.value()->logicalType())}};
        QJsonArray endpointsArray = serializeEndpoints(it.value()), neighborsArray = serializeNeighbors(it.value());

        if (it.value()->manufacturerCode())
            json.insert("manufacturerCode", it.value()->manufacturerCode());

        if (it.value()->logicalType() == LogicalType::Coordinator)
        {
             if (!m_adapterType.isEmpty())
                 json.insert("type", m_adapterType);

             if (!m_adapterVersion.isEmpty())
                 json.insert("version", m_adapterVersion);
        }
        else
        {
            if (it.value()->version())
                json.insert("version", it.value()->version());

            if (it.value()->powerSource())
                json.insert("powerSource", it.value()->powerSource());

            json.insert("ineterviewFinished", it.value()->interviewFinished());
        }

        if (!it.value()->manufacturerName().isEmpty())
            json.insert("manufacturerName", it.value()->manufacturerName());

        if (!it.value()->modelName().isEmpty())
            json.insert("modelName", it.value()->modelName());

        if (it.value()->name() != it.value()->ieeeAddress().toHex(':'))
            json.insert("name", it.value()->name());

        if (it.value()->lastSeen())
            json.insert("lastSeen", it.value()->lastSeen());

        if (!endpointsArray.isEmpty())
            json.insert("endpoints", endpointsArray);

        if (!neighborsArray.isEmpty())
            json.insert("neighbors", neighborsArray);

        array.append(json);
    }

    return array;
}

QJsonArray DeviceList::serializeEndpoints(const Device &device)
{
    QJsonArray array;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        QJsonObject json;

        if (!it.value()->profileId() && !it.value()->deviceId())
            continue;

        json.insert("endpointId", it.key());
        json.insert("profileId", it.value()->profileId());
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

QJsonArray DeviceList::serializeNeighbors(const Device &device)
{
    QJsonArray array;

    for (auto it = device->neighbors().begin(); it != device->neighbors().end(); it++)
    {
        QJsonObject json = {{"networkAddress", it.key()}, {"linkQuality", it.value()}};
        array.append(json);
    }

    return array;
}

void DeviceList::storeStatus(void)
{
    QJsonObject json = {{"devices", serializeDevices()}, {"permitJoin", m_permitJoin}};

    m_timer->start(STORE_STATUS_INTERVAL);

    if (m_file.open(QFile::WriteOnly | QFile::Text))
    {
        m_file.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        m_file.close();
    }
    else
        logWarning << "Can't open database file, status not saved";

    emit statusStored(json);
}
