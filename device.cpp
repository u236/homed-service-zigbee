#include <QFile>
#include "device.h"
#include "logger.h"

DeviceList::DeviceList(QSettings *config) : m_databaseTimer(new QTimer(this)), m_stateTimer(new QTimer(this)), m_permitJoin(false)
{
    m_databaseFile.setFileName(config->value("zigbee/database", "/var/db/homed-zigbee-database.json").toString());
    m_stateFile.setFileName(config->value("zigbee/state", "/var/db/homed-zigbee-state.json").toString());

    if (m_databaseFile.open(QFile::ReadOnly | QFile::Text))
    {
        QJsonObject json = QJsonDocument::fromJson(m_databaseFile.readAll()).object();
        unserializeDevices(json.value("devices").toArray());
        m_permitJoin = json.value("permitJoin").toBool();
        m_databaseFile.close();
    }

    connect(m_databaseTimer, &QTimer::timeout, this, &DeviceList::storeDatabase);
    connect(m_stateTimer, &QTimer::timeout, this, &DeviceList::storeState);

    m_databaseTimer->setSingleShot(true);
    m_stateTimer->setSingleShot(true);
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

void DeviceList::restoreState(void)
{
    if (m_stateFile.open(QFile::ReadOnly | QFile::Text))
    {
        QJsonObject json = QJsonDocument::fromJson(m_stateFile.readAll()).object();

        for (auto it = begin(); it != end(); it++)
        {
            QJsonObject item = json.value(it.value()->ieeeAddress().toHex(':')).toObject();

            if (it.value()->removed() || item.isEmpty())
                continue;

            unserializeState(it.value(), item);
        }

        m_stateFile.close();
    }
}

void DeviceList::storeStateLater(void)
{
    m_stateTimer->start(STORE_PROPERTIES_INTERVAL);
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
                if (json.value("ineterviewFinished").toBool())
                    device->setInterviewFinished();

                device->setLogicalType(static_cast <LogicalType> (json.value("logicalType").toInt()));
                device->setManufacturerCode(static_cast <quint16> (json.value("manufacturerCode").toInt()));
                device->setVersion(static_cast <quint8> (json.value("version").toInt()));
                device->setPowerSource(static_cast <quint8> (json.value("powerSource").toInt()));
                device->setManufacturerName(json.value("manufacturerName").toString());
                device->setModelName(json.value("modelName").toString());
                device->setLastSeen(json.value("lastSeen").toInt());

                unserializeEndpoints(device, json.value("endpoints").toArray());
                unserializeNeighbors(device, json.value("neighbors").toArray());
            }

            insert(device->ieeeAddress(), device);
            count++;
        }
    }

    logInfo << count << "devices loaded";
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

void DeviceList::unserializeState(const Device &device, const QJsonObject &state)
{
    for (auto it = state.begin(); it != state.end(); it++)
    {
        Endpoint endpoint = device->endpoints().value(static_cast <quint8> (it.key().toInt()));
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

QJsonArray DeviceList::serializeDevices(void)
{
    QJsonArray array;

    for (auto it = begin(); it != end(); it++)
    {
        QJsonObject json = {{"ieeeAddress", QString(it.value()->ieeeAddress().toHex(':'))}, {"networkAddress", it.value()->networkAddress()}};

        if (!it.value()->removed())
        {
            if (it.value()->name() != it.value()->ieeeAddress().toHex(':'))
                json.insert("name", it.value()->name());

            json.insert("logicalType", static_cast <quint8> (it.value()->logicalType()));

            if (it.value()->logicalType() == LogicalType::Coordinator)
            {
                if (!m_adapterType.isEmpty())
                    json.insert("type", m_adapterType);

                if (!m_adapterVersion.isEmpty())
                    json.insert("version", m_adapterVersion);
            }
            else
            {
                json.insert("ineterviewFinished", it.value()->interviewFinished());
                json.insert("manufacturerCode", it.value()->manufacturerCode());

                if (it.value()->version())
                    json.insert("version", it.value()->version());

                if (it.value()->powerSource())
                    json.insert("powerSource", it.value()->powerSource());

                if (!it.value()->manufacturerName().isEmpty())
                    json.insert("manufacturerName", it.value()->manufacturerName());

                if (!it.value()->modelName().isEmpty())
                    json.insert("modelName", it.value()->modelName());

                if (it.value()->lastSeen())
                    json.insert("lastSeen", it.value()->lastSeen());
            }

            if (!it.value()->endpoints().isEmpty())
                json.insert("endpoints", serializeEndpoints(it.value()));

            if (!it.value()->neighbors().isEmpty())
                json.insert("neighbors", serializeNeighbors(it.value()));
        }
        else
        {
            json.insert("name", it.value()->name());
            json.insert("removed", true);
        }

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

QJsonObject DeviceList::serializeState(const Device &device)
{
    QJsonObject json;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        QJsonObject item;

        for (int i = 0; i < it.value()->properties().count(); i++)
        {
            const Property &property = it.value()->properties().at(i);

            if (!property->value().isValid())
                continue;

            item.insert(property->name(), QJsonValue::fromVariant(property->value()));
        }

        if (item.isEmpty())
            continue;

        json.insert(QString::number(it.value()->id()), item);
    }

    return json;
}

void DeviceList::storeDatabase(void)
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

void DeviceList::storeState(void)
{
    QJsonObject json;

    for (auto it = begin(); it != end(); it++)
    {
        QJsonObject item = serializeState(it.value());

        if (item.isEmpty())
            continue;

        json.insert(it.value()->ieeeAddress().toHex(':'), item);
    }

    if (m_state == json)
        return;

    if (m_stateFile.open(QFile::WriteOnly | QFile::Text))
    {
        m_stateFile.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        m_stateFile.close();
    }
    else
        logWarning << "Can't open state file, state not stored";

    m_state = json;
}
