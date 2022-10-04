#include <QFile>
#include "controller.h"

Controller::Controller(void) : m_zigbee(new ZigBee(getConfig(), this))
{
    connect(m_zigbee, &ZigBee::deviceEvent, this, &Controller::deviceEvent);
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

            if (item.contains("ieeeAddress") && item.contains("deviceName"))
                m_zigbee->setDeviceName(QByteArray::fromHex(item.value("ieeeAddress").toString().toUtf8()), item.value("deviceName").toString());
        }
    }
    else if (topic.name() == "homed/command/zigbee" && json.contains("action"))
    {
        QList <QString> list = {"setPermitJoin", "otaUpgrade", "removeDevice", "updateDevice", "updateReporting", "bindDevice", "unbindDevice", "touchLinkReset", "touchLinkScan"};

        switch (list.indexOf(json.value("action").toString()))
        {
            case 0: m_zigbee->setPermitJoin(json.value("enabled").toBool()); break;
            case 1: m_zigbee->otaUpgrade(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint8> (json.value("endpointId").toInt()), json.value("fileName").toString()); break;
            case 2: m_zigbee->removeDevice(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8())); break;
            case 3: m_zigbee->updateDevice(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), json.value("reportings").toBool()); break;
            case 4: m_zigbee->updateReporting(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint8> (json.value("endpointId").toInt()), json.value("reporting").toString(), static_cast <quint16> (json.value("minInterval").toInt()), static_cast <quint16> (json.value("maxInterval").toInt()), static_cast <quint16> (json.value("valueChange").toInt())); break;
            case 5: m_zigbee->deviceBinding(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint8> (json.value("endpointId").toInt()), static_cast <quint16> (json.value("clusterId").toInt()), QByteArray::fromHex(json.value("dstAddress").toString().toUtf8()), static_cast <quint8> (json.value("dstEndpointId").toInt())); break;
            case 6: m_zigbee->deviceBinding(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint8> (json.value("endpointId").toInt()), static_cast <quint16> (json.value("clusterId").toInt()), QByteArray::fromHex(json.value("dstAddress").toString().toUtf8()), static_cast <quint8> (json.value("dstEndpointId").toInt()), true); break;
            case 7: m_zigbee->touchLinkRequest(QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()), static_cast <quint8> (json.value("channel").toInt()), true); break;
            case 8: m_zigbee->touchLinkRequest(); break;
        }
    }
    else if (topic.name().startsWith("homed/td/zigbee/"))
    {
        QList <QString> list = topic.name().split('/');
        QByteArray ieeeAddress = QByteArray::fromHex(list.value(3).toUtf8());
        quint8 endpointId = static_cast <quint8> (list.value(4).toInt());

        for (auto it = json.begin(); it != json.end(); it++)
        {
            if (!it.value().toVariant().isValid())
                continue;

            m_zigbee->deviceAction(ieeeAddress, endpointId, it.key(), it.value().toVariant());
        }
    }
}

void Controller::deviceEvent(bool join)
{
    mqttPublish("homed/td/display", {{"notification", QString("ZIGBEE DEVICE %1").arg(join ? "JOINED": "LEAVED")}});
}

void Controller::endpointUpdated(const Device &device, quint8 endpointId)
{
    QJsonObject json;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (device->multipleEndpoints() && it.value()->id() != endpointId)
            continue;

        for (int i = 0; i < it.value()->properties().count(); i++)
        {
            const Property &property = it.value()->properties().at(i);

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

            if (property->singleShot())
                property->clear();
        }
    }

    if (json.isEmpty())
        return;

    json.insert("linkQuality", device->linkQuality());
    mqttPublish(QString("homed/fd/zigbee/").append(device->ieeeAddress().toHex(':')).append(device->multipleEndpoints() ? QString("/%1").arg(endpointId) : QString()), json);
}

void Controller::statusStored(const QJsonObject &json)
{
    mqttPublish("homed/status/zigbee", json, true);
}
