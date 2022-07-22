#include <QFile>
#include "controller.h"

Controller::Controller(void) : m_zigbee(new ZigBee(getConfig(), this))
{
    connect(m_zigbee, &ZigBee::endDeviceEvent, this, &Controller::endDeviceEvent);
    connect(m_zigbee, &ZigBee::endPointUpdated, this, &Controller::endPointUpdated);
    connect(m_zigbee, &ZigBee::statusStored, this, &Controller::statusStored);

    m_zigbee->init();
}

void Controller::mqttConnected(void)
{
    logInfo << "MQTT connected";

    mqttSubscribe("homed/config/zigbee");
    mqttSubscribe("homed/command/zigbee");
    mqttSubscribe("homed/td/zigbee/#");
}

void Controller::mqttReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    QJsonObject json = QJsonDocument::fromJson(message).object();

    if (topic.name() == "homed/config/zigbee" && json.contains("devices"))
    {
        QJsonArray array = json.value("devices").toArray();

        logInfo << "Configuration message received";

        for (auto it = array.begin(); it != array.end(); it++)
        {
            QJsonObject item = it->toObject();

            if (item.contains("deviceAddress") && item.contains("deviceName"))
                m_zigbee->setDeviceName(QByteArray::fromHex(item.value("deviceAddress").toString().toUtf8()), item.value("deviceName").toString());
        }
    }
    else if (topic.name() == "homed/command/zigbee" && json.contains("action"))
    {
        QString action = json.value("action").toString();

        if (action == "setPermitJoin" && json.contains("enabled"))
            m_zigbee->setPermitJoin(json.value("enabled").toBool());
        else if (action == "removeDevice" && json.contains("deviceAddress"))
            m_zigbee->removeDevice(QByteArray::fromHex(json.value("deviceAddress").toString().toUtf8()));
    }
    else if (topic.name().startsWith("homed/td/zigbee/"))
    {
        QStringList list = topic.name().split('/');
        QByteArray ieeeAddress = QByteArray::fromHex(list.value(3).toUtf8());

        for (auto it = json.begin(); it != json.end(); it++)
        {
            if (!it.value().toVariant().isValid())
                continue;

            m_zigbee->deviceAction(ieeeAddress, it.key(), it.value().toVariant());
        }
    }
}

void Controller::endDeviceEvent(bool join)
{
    mqttPublish("homed/td/display", {{"notification", QString("ZIGBEE DEVICE %1").arg(join ? "JOINED": "LEAVED")}});
}

void Controller::endPointUpdated(const EndPoint &endPoint)
{
    QJsonObject json;

    for (int i = 0; i < endPoint->device()->properties().count(); i++)
    {
        const Property &property = endPoint->device()->properties().at(i);

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
