#include <QFile>
#include "controller.h"

Controller::Controller(void) : m_zigbee(new ZigBee(getConfig(), this))
{
    connect(m_zigbee, &ZigBee::endDeviceEvent, this, &Controller::endDeviceEvent);
    connect(m_zigbee, &ZigBee::endpointUpdated, this, &Controller::endpointUpdated);
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
        QList <QString> list = {"setPermitJoin", "configureDevice", "removeDevice", "touchLinkReset", "touchLinkScan"};

        switch (list.indexOf(json.value("action").toString()))
        {
            case 0: m_zigbee->setPermitJoin(json.value("enabled").toBool()); break;
            case 1: m_zigbee->configureDevice(QByteArray::fromHex(json.value("deviceAddress").toString().toUtf8())); break;
            case 2: m_zigbee->removeDevice(QByteArray::fromHex(json.value("deviceAddress").toString().toUtf8())); break;
            case 3: m_zigbee->touchLinkReset(QByteArray::fromHex(json.value("deviceAddress").toString().toUtf8()), static_cast <quint8> (json.value("channel").toInt())); break;
            case 4: m_zigbee->touchLinkScan(); break;
        }
    }
    else if (topic.name().startsWith("homed/td/zigbee/"))
    {
        QList <QString> list = topic.name().split('/');
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

void Controller::endpointUpdated(const Endpoint &endpoint)
{
    QJsonObject json;

    for (int i = 0; i < endpoint->device()->properties().count(); i++)
    {
        const Property &property = endpoint->device()->properties().at(i);

        if (!property->value().isValid())
            continue;

        if (property->value().type() == QVariant::Map)
        {
            QMap <QString, QVariant> map = json.toVariantMap();
            map.insert(property->value().toMap());
            json = QJsonObject::fromVariantMap(map);
        }
        else
            json.insert(property->name(), QJsonValue::fromVariant(property->value()));

        if (property->invalidable())
            property->invalidate();
    }

    if (json.isEmpty())
        return;

    json.insert("linkQuality", endpoint->device()->linkQuality());
    mqttPublish(QString("homed/fd/zigbee/").append(endpoint->device()->ieeeAddress().toHex(':')), json);
}

void Controller::statusStored(const QJsonObject &json)
{
    mqttPublish("homed/status/zigbee", json, true);
}
