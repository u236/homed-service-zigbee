#include <QFile>
#include "controller.h"

Controller::Controller(void) : m_zigbee(new ZigBee(getConfig(), this))
{
    connect(m_zigbee, &ZigBee::endPointUpdated, this, &Controller::endPointUpdated);
    connect(m_zigbee, &ZigBee::statusStored, this, &Controller::statusStored);

    m_zigbee->init();
}

void Controller::mqttConnected(void)
{
    logInfo << "MQTT connected";

    mqttSubscribe("homed/command/zigbee");
    mqttSubscribe("homed/td/zigbee/#");
}

void Controller::mqttReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    QJsonObject json = QJsonDocument::fromJson(message).object();

    if (topic.name() == "homed/command/zigbee" && json.contains("action"))
    {
        QString action = json.value("action").toString();

        if (action == "setPermitJoin" && json.contains("enabled"))
            m_zigbee->setPermitJoin(json.value("enabled").toBool());
    }
    else if (topic.name().startsWith("homed/td/zigbee/"))
    {
        QByteArray ieeeAddress = QByteArray::fromHex(topic.name().split('/').last().toLocal8Bit());

        for (auto it = json.begin(); it != json.end(); it++)
        {
            if (!it.value().toVariant().isValid())
                continue;

            m_zigbee->deviceAction(ieeeAddress, it.key(), it.value().toVariant());
        }
    }
}

void Controller::endPointUpdated(const EndPoint &endPoint)
{
    QJsonObject json;

    for (quint8 i = 0; i < static_cast <quint8> (endPoint->device()->properties().count()); i++)
    {
        Property property = endPoint->device()->properties().value(i);

        if (!property->value().isValid())
            continue;

        json.insert(property->name(), QJsonValue::fromVariant(property->value()));

        if (property->invalidable())
            property->invalidate();
    }

    if (json.isEmpty())
        return;

    json.insert("linkQuality", endPoint->device()->linkQuality());
    mqttPublish(QString("homed/fd/zigbee/").append(endPoint->device()->ieeeAddress().toHex(':')), json);
}

void Controller::statusStored(const QJsonObject &json)
{
    mqttPublish("homed/status/zigbee", json, true);
}
