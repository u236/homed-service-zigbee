#include "controller.h"

Controller::Controller(void) : m_timer(new QTimer(this)), m_zigbee(new ZigBee(getConfig(), this)), m_commands(QMetaEnum::fromType <Command> ())
{
    QDate date = QDate::currentDate();

    if (date > QDate(date.year(), 12, 23) && date < QDate(date.year() + 1, 1, 15))
        logInfo << "Merry Christmas and a Happy New Year!" << "\xF0\x9F\x8E\x81\xF0\x9F\x8E\x84\xF0\x9F\x8D\xBA";

    logInfo << "Starting version" << SERVICE_VERSION;

    m_homeassistant = getConfig()->value("homeassistant/enabled", false).toBool();
    m_homeassistantPrefix = getConfig()->value("homeassistant/prefix", "homeassistant").toString();
    m_homeassistantStatus = getConfig()->value("homeassistant/status", "homeassistant/status").toString();

    connect(m_timer, &QTimer::timeout, this, &Controller::updateAvailability);
    connect(m_zigbee, &ZigBee::deviceEvent, this, &Controller::deviceEvent);
    connect(m_zigbee, &ZigBee::endpointUpdated, this, &Controller::endpointUpdated);
    connect(m_zigbee, &ZigBee::statusUpdated, this, &Controller::statusUpdated);

    m_timer->start(UPDATE_AVAILABILITY_INTERVAL);

    m_zigbee->devices()->setNames(getConfig()->value("mqtt/names", false).toBool());
    m_zigbee->init();
}

void Controller::publishExposes(const Device &device, bool remove)
{
    QMap <QString, QVariant> data, endpointName = device->options().value("endpointName").toMap();

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        for (int i = 0; i < it.value()->exposes().count(); i++)
        {
            const Expose &expose = it.value()->exposes().at(i);
            QVariant option = expose->endpointOption();

            if (m_homeassistant && expose->homeassistant())
            {
                QString id = expose->multiple() ? QString::number(it.key()) : QString(), name = endpointName.value(id).toString(), topic = m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':');
                QList <QString> object = {expose->name()};
                QJsonObject json, identity;
                QJsonArray availability;

                if (!id.isEmpty())
                {
                    topic.append("/").append(id);
                    object.append(id);
                }

                if (!remove)
                {
                    expose->setStateTopic(mqttTopic("fd/zigbee/%1").arg(topic));
                    expose->setCommandTopic(mqttTopic("td/zigbee/%1").arg(topic));

                    json = expose->request();

                    identity.insert("identifiers", QJsonArray {QString(device->ieeeAddress().toHex())});
                    identity.insert("model", device->description());
                    identity.insert("name", device->name());

                    availability.append(QJsonObject {{"topic", mqttTopic("device/zigbee/%1").arg(m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':'))}, {"value_template", "{{ value_json.status }}"}});
                    availability.append(QJsonObject {{"topic", mqttTopic("service/zigbee")}, {"value_template", "{{ value_json.status }}"}});

                    json.insert("availability", availability);
                    json.insert("availability_mode", "all");
                    json.insert("device", identity);
                    json.insert("name", QString("%1 %2").arg(device->name(), name.isEmpty() ? object.join(' ') : name));
                    json.insert("unique_id", QString("%1_%2").arg(device->ieeeAddress().toHex(), object.join('_')));
                }

                mqttPublish(QString("%1/%2/%3/%4/config").arg(m_homeassistantPrefix, expose->component(), device->ieeeAddress().toHex(), object.join('_')), json, true);

                if (expose->name() == "action" || expose->name() == "event" || expose->name() == "scene")
                {
                    QList <QString> list = expose->name() != "scene" ? option.toStringList() : QVariant(option.toMap().values()).toStringList();

                    for (int i = 0; i < list.count(); i++)
                    {
                        QList <QString> event = {list.at(i)};
                        QJsonObject item;

                        if (!id.isEmpty())
                            event.append(id);

                        if (!remove)
                        {
                            item.insert("automation_type", "trigger");
                            item.insert("device", identity);
                            item.insert("payload", event.at(0));
                            item.insert("subtype", name.isEmpty() ? event.join(' ') : QString("%1 %2").arg(name, list.at(i)));
                            item.insert("topic", mqttTopic("fd/zigbee/%1").arg(topic));
                            item.insert("type", expose->name());
                            item.insert("value_template", QString("{{ value_json.%1 }}").arg(expose->name()));
                        }

                        mqttPublish(QString("%1/device_automation/%2/%3/config").arg(m_homeassistantPrefix, device->ieeeAddress().toHex(), event.join('_')), item, true);
                    }
                }
            }

            if (!remove)
            {                
                QString id = QString::number(it.key()), key = expose->multiple() ? id : "common";
                QMap <QString, QVariant> map = data.value(key).toMap(), options = map.value("options").toMap();
                QList <QString> items = map.value("items").toStringList();

                items.append(expose->name());
                map.insert("items", QVariant(items));

                if (option.isValid())
                {
                    if (expose->name() == "light" && option.toStringList().contains("colorTemperature"))
                    {
                        QVariant colorTemperature = expose->endpointOption("colorTemperature");

                        if (colorTemperature.isValid())
                            options.insert("colorTemperature", colorTemperature);
                    }

                    options.insert(expose->name(), option);
                }

                if (endpointName.contains(id))
                    options.insert("name", endpointName.value(id));

                if (!options.isEmpty())
                    map.insert("options", options);

                data.insert(key, map);
            }
        }
    }

    mqttPublish(mqttTopic("expose/zigbee/%1").arg(m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':')), QJsonObject::fromVariantMap(data), true);
}

void Controller::publishProperties(void)
{
    for (auto it = m_zigbee->devices()->begin(); it != m_zigbee->devices()->end(); it++)
    {
        const Device &device = it.value();

        publishExposes(device);

        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
            endpointUpdated(device, it.key());
    }
}

void Controller::quit(void)
{
    delete m_zigbee;
    HOMEd::quit();
}

void Controller::mqttConnected(void)
{
    logInfo << "MQTT connected";

    mqttSubscribe(mqttTopic("config/zigbee"));
    mqttSubscribe(mqttTopic("command/zigbee"));
    mqttSubscribe(mqttTopic("td/zigbee/#"));

    if (m_homeassistant)
        mqttSubscribe(m_homeassistantStatus);

    publishProperties();
}

void Controller::mqttReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    QString subTopic = topic.name().replace(mqttTopic(), QString());
    QJsonObject json = QJsonDocument::fromJson(message).object();

    if (subTopic == "config/zigbee" && json.contains("devices"))
    {
        QJsonArray array = json.value("devices").toArray();

        logInfo << "Configuration message received";

        for (auto it = array.begin(); it != array.end(); it++)
        {
            QJsonObject item = it->toObject();

            if (item.contains("ieeeAddress") && item.contains("deviceName"))
                m_zigbee->setDeviceName(item.value("ieeeAddress").toString(), item.value("deviceName").toString());
        }
    }
    else if (subTopic == "command/zigbee" && json.contains("action"))
    {
        Command command = static_cast <Command> (m_commands.keyToValue(json.value("action").toString().toUtf8().constData()));

        switch (command)
        {
            case Command::setPermitJoin:
                m_zigbee->setPermitJoin(json.value("enabled").toBool());
                break;

            case Command::setDeviceName:
                m_zigbee->setDeviceName(json.value("device").toString(), json.value("name").toString());
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
    else if (topic.name() == m_homeassistantStatus)
    {
        if (message != "online")
            return;

        publishProperties();
    }
}

void Controller::updateAvailability(void)
{
    qint64 time = QDateTime::currentSecsSinceEpoch();

    for (auto it = m_zigbee->devices()->begin(); it != m_zigbee->devices()->end(); it++)
    {
        AvailabilityStatus check = it.value()->availability();
        qint64 timeout = it.value()->options().value("availability").toInt();

        if (it.value()->removed() || it.value()->logicalType() == LogicalType::Coordinator)
            continue;

        if (!timeout)
            timeout = it.value()->batteryPowered() ? 28800 : 600;

        it.value()->setAvailability(time - it.value()->lastSeen() <= timeout ? AvailabilityStatus::Online : AvailabilityStatus::Offline);

        if (it.value()->availability() == check)
            continue;

        mqttPublish(mqttTopic("device/zigbee/%1").arg(m_zigbee->devices()->names() ? it.value()->name() : it.value()->ieeeAddress().toHex(':')), {{"status", it.value()->availability() == AvailabilityStatus::Online ? "online" : "offline"}}, true);
    }
}

void Controller::deviceEvent(const Device &device, ZigBee::Event event)
{
    switch (event)
    {
        case ZigBee::Event::deviceLeft:
        case ZigBee::Event::deviceRemoved:
        case ZigBee::Event::deviceAboutToRename:
            mqttPublish(mqttTopic("device/zigbee/%1").arg(m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':')), QJsonObject(), true);
            publishExposes(device, true);
            break;

        case ZigBee::Event::deviceUpdated:

            if (device->availability() != AvailabilityStatus::Unknown)
                mqttPublish(mqttTopic("device/zigbee/%1").arg(m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':')), {{"status", device->availability() == AvailabilityStatus::Online ? "online" : "offline"}}, true);

            publishExposes(device);
            break;

        case ZigBee::Event::interviewFinished:
            publishExposes(device);
            break;

        default:
            break;
    }

    mqttPublish(mqttTopic("event/zigbee"), {{"device", device->name()}, {"event", m_zigbee->eventName(event)}});
}

void Controller::endpointUpdated(const Device &device, quint8 endpointId)
{
    QMap <QString, QVariant> endpointMap, deviceMap;
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

            if (!property->singleShot())
                continue;

            property->clearValue();
        }

        it.value()->setUpdated(false);
    }

    if (!endpointMap.isEmpty())
        mqttPublish(mqttTopic("fd/zigbee/%1/%2").arg(m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':')).arg(endpointId), QJsonObject::fromVariantMap(endpointMap), retain);

    deviceMap.insert("linkQuality", device->linkQuality());
    mqttPublish(mqttTopic("fd/zigbee/%1").arg(m_zigbee->devices()->names() ? device->name() : device->ieeeAddress().toHex(':')), QJsonObject::fromVariantMap(deviceMap), retain);
}

void Controller::statusUpdated(const QJsonObject &json)
{
    mqttPublish(mqttTopic("status/zigbee"), json, true);
}
