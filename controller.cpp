#include <QFile>
#include "controller.h"

Controller::Controller(void) : m_zigbee(new ZigBee(getConfig(), this))
{
    connect(m_zigbee, &ZigBee::endPointUpdated, this, &Controller::endPointUpdated);
    connect(m_zigbee, &ZigBee::statusStored, this, &Controller::statusStored);
}

void Controller::mqttConnected(void)
{
    logInfo << "MQTT connected";
    m_zigbee->init();
}

void Controller::mqttReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    Q_UNUSED(message)
    Q_UNUSED(topic)
}

void Controller::endPointUpdated(const EndPoint &endPoint)
{
    QJsonObject json;

    for (quint8 i = 0; i < static_cast <quint8> (endPoint->device()->fromDevice().count()); i++)
    {
        Property property = endPoint->device()->fromDevice().value(i);

        if (!property->value().isValid())
            continue;

        json.insert(property->name(), QJsonValue::fromVariant(property->value()));
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
