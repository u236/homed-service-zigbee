#include <QFile>
#include "device.h"
#include "logger.h"

DeviceList::DeviceList(QSettings *config) : m_databaseTimer(new QTimer(this)), m_propertiesTimer(new QTimer(this)), m_permitJoin(false)
{
    m_databaseFile.setFileName(config->value("zigbee/database", "/var/db/homed-zigbee-database.json").toString());
    m_propertiesFile.setFileName(config->value("zigbee/properties", "/var/db/homed-zigbee-properties.json").toString());

    if (m_databaseFile.open(QFile::ReadOnly | QFile::Text))
    {
        QJsonObject json = QJsonDocument::fromJson(m_databaseFile.readAll()).object();
        unserializeDevices(json.value("devices").toArray());
        m_permitJoin = json.value("permitJoin").toBool();
        m_databaseFile.close();
    }

    connect(m_databaseTimer, &QTimer::timeout, this, &DeviceList::writeDatabase);
    connect(m_propertiesTimer, &QTimer::timeout, this, &DeviceList::writeProperties);

    m_databaseTimer->setSingleShot(true);
    m_propertiesTimer->setSingleShot(true);
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
    m_databaseTimer->start(STORE_DATABASE_DELAY);
}

void DeviceList::storeProperties(void)
{
    m_propertiesTimer->start(STORE_PROPERTIES_DELAY);
}

void DeviceList::restoreProperties(void)
{
    if (!m_propertiesFile.open(QFile::ReadOnly | QFile::Text))
        return;

    unserializeProperties(QJsonDocument::fromJson(m_propertiesFile.readAll()).object());
    m_propertiesFile.close();

    logInfo << "Properties restored";
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

                if (json.value("ineterviewFinished").toBool())
                    device->setInterviewFinished();

                device->setLogicalType(static_cast <LogicalType> (json.value("logicalType").toInt()));
                device->setManufacturerCode(static_cast <quint16> (json.value("manufacturerCode").toInt()));
                device->setVersion(static_cast <quint8> (json.value("version").toInt()));
                device->setPowerSource(static_cast <quint8> (json.value("powerSource").toInt()));
                device->setManufacturerName(json.value("manufacturerName").toString());
                device->setModelName(json.value("modelName").toString());
                device->setLastSeen(json.value("lastSeen").toInt());

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

                        for (const QJsonValue &clusterId : inClusters)
                            endpoint->inClusters().append(static_cast <quint16> (clusterId.toInt()));

                        for (const QJsonValue &clusterId : outClusters)
                            endpoint->outClusters().append(static_cast <quint16> (clusterId.toInt()));

                        device->endpoints().insert(endpointId, endpoint);
                    }
                }

                for (auto it = neighborsArray.begin(); it != neighborsArray.end(); it++)
                {
                    QJsonObject item = it->toObject();

                    if (item.contains("endpointId"))
                    {
                        quint8 endpointId = static_cast <quint8> (item.value("endpointId").toInt());
                        Endpoint endpoint(new EndpointObject(endpointId, device));
                        QJsonArray inClusters = item.value("inClusters").toArray(), outClusters = item.value("outClusters").toArray();

                        endpoint->setProfileId(static_cast <quint16> (item.value("profileId").toInt()));
                        endpoint->setDeviceId(static_cast <quint16> (item.value("deviceId").toInt()));

                        for (const QJsonValue &clusterId : inClusters)
                            endpoint->inClusters().append(static_cast <quint16> (clusterId.toInt()));

                        for (const QJsonValue &clusterId : outClusters)
                            endpoint->outClusters().append(static_cast <quint16> (clusterId.toInt()));

                        device->endpoints().insert(endpointId, endpoint);
                    }
                }
            }

            insert(device->ieeeAddress(), device);
            count++;
        }
    }

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
                endpoint->setDataUpdated(true);
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
                json.insert("ineterviewFinished", device->interviewFinished());
                json.insert("manufacturerCode", device->manufacturerCode());

                if (device->version())
                    json.insert("version", device->version());

                if (device->powerSource())
                    json.insert("powerSource", device->powerSource());

                if (!device->manufacturerName().isEmpty())
                    json.insert("manufacturerName", device->manufacturerName());

                if (!device->modelName().isEmpty())
                    json.insert("modelName", device->modelName());

                if (device->lastSeen())
                    json.insert("lastSeen", device->lastSeen());
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

            item.insert(QString::number(it.value()->id()), data);
        }

        if (item.isEmpty())
            continue;

        json.insert(it.value()->ieeeAddress().toHex(':'), item);
    }

    return json;
}

void DeviceList::writeDatabase(void)
{
    QJsonObject json = {{"devices", serializeDevices()}, {"permitJoin", m_permitJoin}};

    m_databaseTimer->start(STORE_DATABASE_INTERVAL);

    if (m_databaseFile.open(QFile::WriteOnly | QFile::Text))
    {
        m_databaseFile.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        m_databaseFile.close();
    }
    else
        logWarning << "Can't open database file, database not stored";

    emit statusUpdated(json);
}

void DeviceList::writeProperties(void)
{
    QJsonObject json = serializeProperties();

    if (m_properties == json)
        return;

    if (m_propertiesFile.open(QFile::WriteOnly | QFile::Text))
    {
        m_propertiesFile.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        m_propertiesFile.close();
    }
    else
        logWarning << "Can't open properties file, properties not stored";

    m_properties = json;
}
