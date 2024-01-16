#include "controller.h"
#include "logger.h"

Controller::Controller(const QString &configFile) : HOMEd(configFile), m_avaliabilityTimer(new QTimer(this)), m_propertiesTimer(new QTimer(this)), m_zigbee(new ZigBee(getConfig(), this)), m_commands(QMetaEnum::fromType <Command> ())
{
    logInfo << "Starting version" << SERVICE_VERSION;
    logInfo << "Configuration file is" << getConfig()->fileName();

    m_haStatus = getConfig()->value("homeassistant/status", "homeassistant/status").toString();

    connect(m_avaliabilityTimer, &QTimer::timeout, this, &Controller::updateAvailability);
    connect(m_propertiesTimer, &QTimer::timeout, this, &Controller::updateProperties);

    connect(m_zigbee, &ZigBee::deviceEvent, this, &Controller::deviceEvent);
    connect(m_zigbee, &ZigBee::endpointUpdated, this, &Controller::endpointUpdated);
    connect(m_zigbee, &ZigBee::statusUpdated, this, &Controller::statusUpdated);

    m_avaliabilityTimer->start(UPDATE_AVAILABILITY_INTERVAL);
    m_propertiesTimer->setSingleShot(true);

    m_zigbee->devices()->setNames(getConfig()->value("mqtt/names", false).toBool());
    m_zigbee->init();
}

void Controller::publishExposes(DeviceObject *device, bool remove)
{
    device->publishExposes(this, device->ieeeAddress().toHex(':'), device->ieeeAddress().toHex(), remove);

    if (remove)
        return;

    m_propertiesTimer->start(UPDATE_PROPERTIES_DELAY);
}

void Controller::quit(void)
{
    delete m_zigbee;
    HOMEd::quit();
}

void Controller::mqttConnected(void)
{
    logInfo << "MQTT connected";

    mqttSubscribe(mqttTopic("command/zigbee"));
    mqttSubscribe(mqttTopic("td/zigbee/#"));

    if (getConfig()->value("homeassistant/enabled", false).toBool())
        mqttSubscribe(m_haStatus);

    for (auto it = m_zigbee->devices()->begin(); it != m_zigbee->devices()->end(); it++)
        publishExposes(it.value().data());
}

void Controller::mqttReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    QString subTopic = topic.name().replace(mqttTopic(), QString());
    QJsonObject json = QJsonDocument::fromJson(message).object();

    if (subTopic == "command/zigbee")
    {
        Command command = static_cast <Command> (m_commands.keyToValue(json.value("action").toString().toUtf8().constData()));

        switch (command)
        {
            case Command::restartService:
                logWarning << "Restart request received...";
                mqttPublish(mqttTopic("command/zigbee"), QJsonObject(), true);
                QCoreApplication::exit(EXIT_RESTART);
                break;

            case Command::setPermitJoin:
                m_zigbee->setPermitJoin(json.value("enabled").toBool());
                break;

            case Command::togglePermitJoin:
                m_zigbee->togglePermitJoin();
                break;

            case Command::editDevice:
                m_zigbee->editDevice(json.value("device").toString(), json.value("name").toString(), json.value("room").toString(), json.value("active").toBool(true), json.value("discovery").toBool(true), json.value("cloud").toBool(true));
                break;

            case Command::removeDevice:
                m_zigbee->removeDevice(json.value("device").toString(), json.value("force").toBool());
                break;

            case Command::updateDevice:
                m_zigbee->updateDevice(json.value("device").toString(), json.value("reportings").toBool());
                break;

            case Command::updateReporting:
                m_zigbee->updateReporting(json.value("device").toString(), static_cast <quint8> (json.value("endpointId").toInt()), json.value("reporting").toString(), static_cast <quint16> (json.value("minInterval").toInt()), static_cast <quint16> (json.value("maxInterval").toInt()), static_cast <quint16> (json.value("valueChange").toInt()));
                break;

            case Command::bindDevice:
            case Command::unbindDevice:
                m_zigbee->bindingControl(json.value("device").toString(), static_cast <quint8> (json.value("endpointId").toInt()), static_cast <quint16> (json.value("clusterId").toInt()), json.value(json.contains("groupId") ? "groupId" : "dstDevice").toVariant(), static_cast <quint8> (json.value("dstEndpointId").toInt()), command == Command::unbindDevice);
                break;

            case Command::addGroup:
            case Command::removeGroup:
                m_zigbee->groupControl(json.value("device").toString(), static_cast <quint8> (json.value("endpointId").toInt()), static_cast <quint16> (json.value("groupId").toInt()), command == Command::removeGroup);
                break;

            case Command::removeAllGroups:
                m_zigbee->removeAllGroups(json.value("device").toString(), static_cast <quint8> (json.value("endpointId").toInt()));
                break;

            case Command::otaUpgrade:
                m_zigbee->otaUpgrade(json.value("device").toString(), static_cast <quint8> (json.value("endpointId").toInt()), json.value("fileName").toString());
                break;

            case  Command::getProperties:
                m_zigbee->getProperties(json.value("device").toString());
                break;

            case Command::clusterRequest:
            case Command::globalRequest:
                m_zigbee->clusterRequest(json.value("device").toString(), static_cast <quint8> (json.value("endpointId").toInt()), static_cast <quint16> (json.value("clusterId").toInt()), static_cast <quint16> (json.value("manufacturerCode").toInt()), static_cast <quint8> (json.value("commandId").toInt()), QByteArray::fromHex(json.value("payload").toString().toUtf8()), command == Command::globalRequest);
                break;

            case Command::touchLinkScan:
                m_zigbee->touchLinkRequest();
                break;

            case Command::touchLinkReset:
                m_zigbee->touchLinkRequest(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint8> (json.value("channel").toInt()), true);
                break;
        }
    }
    else if (subTopic.startsWith("td/zigbee/"))
    {
        QList <QString> list = subTopic.split('/');

        if (list.value(2) != "group")
        {
            for (auto it = json.begin(); it != json.end(); it++)
            {
                if (!it.value().toVariant().isValid())
                    continue;

                m_zigbee->deviceAction(list.value(2), static_cast <quint8> (list.value(3).toInt()), it.key(), it.value().toVariant());
            }
        }
        else
        {
            for (auto it = json.begin(); it != json.end(); it++)
            {
                if (!it.value().toVariant().isValid())
                    continue;

                m_zigbee->groupAction(static_cast <quint16> (list.value(3).toInt()), it.key(), it.value().toVariant());
            }
        }
    }
    else if (topic.name() == m_haStatus)
    {
        if (message != "online")
            return;

        m_propertiesTimer->start(UPDATE_PROPERTIES_DELAY);
    }
}

void Controller::updateAvailability(void)
{
    qint64 time = QDateTime::currentSecsSinceEpoch();

    for (auto it = m_zigbee->devices()->begin(); it != m_zigbee->devices()->end(); it++)
    {
        Availability check = it.value()->availability();
        qint64 timeout = it.value()->options().value("availability").toInt();

        if (it.value()->removed() || it.value()->logicalType() == LogicalType::Coordinator)
            continue;

        if (!timeout)
            timeout = it.value()->batteryPowered() ? 86400 : 600;

        it.value()->setAvailability(it.value()->active() ? time - it.value()->lastSeen() <= timeout ? Availability::Online : Availability::Offline : Availability::Inactive);

        if (it.value()->availability() == check)
            continue;

        mqttPublish(mqttTopic("device/zigbee/%1").arg(m_zigbee->devices()->names() ? it.value()->name() : it.value()->ieeeAddress().toHex(':')), {{"status", it.value()->availability() != Availability::Inactive ? it.value()->availability() == Availability::Online ? "online" : "offline" : "inactive"}}, true);
    }
}

void Controller::updateProperties(void)
{
    for (auto it = m_zigbee->devices()->begin(); it != m_zigbee->devices()->end(); it++)
    {
        const Device &device = it.value();

        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        {
            if (it.value()->properties().isEmpty())
                continue;

            endpointUpdated(device.data(), it.key());
        }
    }
}

void Controller::deviceEvent(DeviceObject *device, ZigBee::Event event, const QJsonObject &json)
{
    QMap <QString, QVariant> map = {{"device", device->name()}, {"event", m_zigbee->eventName(event)}};
    bool check = true, remove = false;

    map.insert(json.toVariantMap());

    switch (event)
    {
        case ZigBee::Event::deviceLeft:
        case ZigBee::Event::deviceRemoved:
        case ZigBee::Event::deviceAboutToRename:
            mqttPublish(mqttTopic("device/zigbee/%1").arg(m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':')), QJsonObject(), true);
            remove = true;
            break;

        case ZigBee::Event::deviceUpdated:
            mqttPublish(mqttTopic("device/zigbee/%1").arg(m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':')), {{"status", device->availability() != Availability::Inactive ? device->availability() == Availability::Online ? "online" : "offline" : "inactive"}}, true);
            break;

        default:

            if (event != ZigBee::Event::interviewFinished)
                check = false;

            break;
    }

    if (check)
        publishExposes(device, remove);

    mqttPublish(mqttTopic("event/zigbee"), QJsonObject::fromVariantMap(map));
}

void Controller::endpointUpdated(DeviceObject *device, quint8 endpointId)
{
    QMap <QString, QVariant> endpointMap, deviceMap = {{"linkQuality", device->linkQuality()}};
    bool retain = device->options().value("retain").toBool();

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        for (int i = 0; i < it.value()->properties().count(); i++)
        {
            const Property &property = it.value()->properties().at(i);
            QMap <QString, QVariant> &map = property->multiple() ? endpointMap : deviceMap;

            if (!property->value().isValid() || (property->multiple() && it.value()->id() != endpointId))
                continue;

            if (property->value().type() != QVariant::Map)
                map.insert(property->name(), property->value());
            else
                map.insert(property->value().toMap());

            if (property->name() == "action" || property->name() == "scene")
                property->clearValue();
        }

        it.value()->setUpdated(false);
    }

    if (!endpointMap.isEmpty())
        mqttPublish(mqttTopic("fd/zigbee/%1/%2").arg(m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':')).arg(endpointId), QJsonObject::fromVariantMap(endpointMap), retain);

    mqttPublish(mqttTopic("fd/zigbee/%1").arg(m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':')), QJsonObject::fromVariantMap(deviceMap), retain);
}

void Controller::statusUpdated(const QJsonObject &json)
{
    mqttPublish(mqttTopic("status/zigbee"), json, true);
}
