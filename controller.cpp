#include <QFile>
#include "controller.h"

Controller::Controller(void) : m_zigbee(new ZigBee(getConfig(), this))
{
    connect(m_zigbee, &ZigBee::stateUpdated, this, &Controller::stateUpdated);
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

void Controller::stateUpdated(const QJsonObject json)
{
    mqttPublish("homed/status/zigbee", json, true);
}
