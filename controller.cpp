#include "controller.h"

Controller::Controller(void) : m_zigbee(new ZigBee(getConfig(), this)), m_commands(QMetaEnum::fromType <Command> ())
{
    m_names = getConfig()->value("mqtt/names", false).toBool();
    m_discovery = getConfig()->value("discovery/enabled", false).toBool();
    m_discoveryPrefix = getConfig()->value("discovery/prefix", "discovery").toString();

    connect(m_zigbee, &ZigBee::deviceEvent, this, &Controller::deviceEvent);
    connect(m_zigbee, &ZigBee::endpointUpdated, this, &Controller::endpointUpdated);
    connect(m_zigbee, &ZigBee::statusUpdated, this, &Controller::statusUpdated);

    m_zigbee->init();
}

void Controller::publishDiscovery(const Device &device)
{
    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        for (int i = 0; i < it.value()->discoveries().count(); i++)
        {
            const Discovery &discovery = it.value()->discoveries().at(i);
            QString id = discovery->multiple() ? QString::number(it.value()->id()) : QString(), topic = m_names ? device->name() : device->ieeeAddress().toHex(':');
            QList <QString> object = {discovery->name()};
            QJsonObject node, json = discovery->reqest();

            if (!id.isEmpty())
            {
                topic.append("/").append(id);
                object.append(id);
            }

            node.insert("identifiers", QJsonArray {QString(device->ieeeAddress().toHex())});
            node.insert("name", device->name());

            if (discovery->control())
                json.insert("command_topic", mqttTopic("td/zigbee/%1").arg(topic));

            json.insert("device", node);
            json.insert("state_topic", mqttTopic("fd/zigbee/%1").arg(topic));
            json.insert("name", QString("%1 (%2)").arg(device->name(), object.join(' ')));
            json.insert("unique_id", QString("%1_%2").arg(device->ieeeAddress().toHex(), object.join('_')));

            if (discovery->name() == "action") // TODO: add scenes
            {
                QList <QString> list = device->options().value("actions").toStringList();

                for (int i = 0; i < list.count(); i++)
                {
                    QList <QString> event = {list.at(i)};
                    QJsonObject item;

                    if (!id.isEmpty())
                        event.append(id);

                    item.insert("automation_type", "trigger");
                    item.insert("device", node);
                    item.insert("payload", event.at(0));
                    item.insert("subtype", event.join(' '));
                    item.insert("topic", mqttTopic("fd/zigbee/%1").arg(topic));
                    item.insert("type", discovery->name());
                    item.insert("value_template", QString("{{ value_json.%1 }}").arg(discovery->name()));

                    mqttPublish(QString("%1/device_automation/%2/%3/config").arg(m_discoveryPrefix, device->ieeeAddress().toHex(), event.join('_')), item, true);
                }
            }

            mqttPublish(QString("%1/%2/%3/%4/config").arg(m_discoveryPrefix, discovery->component(), device->ieeeAddress().toHex(), object.join('_')), json, true);
        }
    }
}

void Controller::mqttConnected(void)
{
    logInfo << "MQTT connected";

    mqttSubscribe(mqttTopic("config/zigbee"));
    mqttSubscribe(mqttTopic("command/zigbee"));
    mqttSubscribe(mqttTopic("td/zigbee/#"));

    for (auto it = m_zigbee->devices()->begin(); it != m_zigbee->devices()->end(); it++)
    {
        const Device &device = it.value();

        if (m_discovery)
            publishDiscovery(device);

        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        {
            if (!it.value()->updated())
                continue;

            endpointUpdated(device, it.value()->id());
        }
    }
}

void Controller::mqttReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    QJsonObject json = QJsonDocument::fromJson(message).object();

    if (topic.name() == mqttTopic("config/zigbee") && json.contains("devices"))
    {
        QJsonArray array = json.value("devices").toArray();

        logInfo << "Configuration message received";

        for (auto it = array.begin(); it != array.end(); it++)
        {
            QJsonObject item = it->toObject();

            if (item.contains("ieeeAddress") && item.contains("deviceName"))
                m_zigbee->setDeviceName(QByteArray::fromHex(item.value("ieeeAddress").toString().toUtf8()), item.value("deviceName").toString(), false);
        }
    }
    else if (topic.name() == mqttTopic("command/zigbee") && json.contains("action"))
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

            case Command::touchLinkScan:
                m_zigbee->touchLinkRequest();
                break;

            case Command::touchLinkReset:
                m_zigbee->touchLinkRequest(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint8> (json.value("channel").toInt()), true);
                break;
        }
    }
    else if (topic.name().startsWith(mqttTopic("td/zigbee/")))
    {
        QList <QString> list = topic.name().split('/');

        if (list.value(3) != "group")
        {
            for (auto it = json.begin(); it != json.end(); it++)
            {
                if (!it.value().toVariant().isValid())
                    continue;

                m_zigbee->deviceAction(list.value(3), static_cast <quint8> (list.value(4).toInt()), it.key(), it.value().toVariant());
            }
        }
        else
        {
            for (auto it = json.begin(); it != json.end(); it++)
            {
                if (!it.value().toVariant().isValid())
                    continue;

                m_zigbee->groupAction(static_cast <quint16> (list.value(4).toInt()), it.key(), it.value().toVariant());
            }
        }
    }
}

void Controller::deviceEvent(const Device &device, const QString &event)
{
    mqttPublish(mqttTopic("event/zigbee"), {{"device", device->name()}, {"event", event}});
}

void Controller::endpointUpdated(const Device &device, quint8 endpointId)
{
    QMap <QString, QVariant> endpointMap, deviceMap;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        for (int i = 0; i < it.value()->properties().count(); i++)
        {
            const Property &property = it.value()->properties().at(i);
            QMap <QString, QVariant> &map = property->multiple() ? endpointMap : deviceMap;

            if (!property->value().isValid())
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
        mqttPublish(mqttTopic("fd/zigbee/%1/%2").arg(m_names ? device->name() : device->ieeeAddress().toHex(':')).arg(endpointId), QJsonObject::fromVariantMap(endpointMap));

    deviceMap.insert("linkQuality", device->linkQuality());
    mqttPublish(mqttTopic("fd/zigbee/%1").arg(m_names ? device->name() : device->ieeeAddress().toHex(':')), QJsonObject::fromVariantMap(deviceMap));
}

void Controller::statusUpdated(const QJsonObject &json)
{
    mqttPublish(mqttTopic("status/zigbee"), json, true);
}
