#include <math.h>
#include <QtEndian>
#include "property.h"
#include "zigbee.h"

Attribute ClusterObject::attribute(quint16 attributeId)
{
    auto it = m_attributes.find(attributeId);

    if (it == m_attributes.end())
        it = m_attributes.insert(attributeId, Attribute(new AttributeObject));

    return it.value();
}

using namespace Properties;

BatteryVoltage::BatteryVoltage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}

void BatteryVoltage::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0020);

    if (attribute->dataType() != DATA_TYPE_8BIT_UNSIGNED || attribute->data().length() != 1)
        return;

    m_value = QString("%1v").arg(static_cast <double> (attribute->data().at(0)) * 0.1);
}

BatteryPercentage::BatteryPercentage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}

void BatteryPercentage::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0021);

    if (attribute->dataType() != DATA_TYPE_8BIT_UNSIGNED || attribute->data().length() != 1)
        return;

    m_value = QString("%1%").arg(static_cast <double> (attribute->data().at(0)) / 2);
}

BatteryIKEA::BatteryIKEA(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}

void BatteryIKEA::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0021);

    if (attribute->dataType() != DATA_TYPE_8BIT_UNSIGNED || attribute->data().length() != 1)
        return;

    m_value = QString("%1%").arg(static_cast <double> (attribute->data().at(0)));
}

BatteryLUMI::BatteryLUMI(void) : PropertyObject("battery", CLUSTER_BASIC) {}

void BatteryLUMI::convert(const Cluster &cluster)
{
    QList <quint16> list = {0xFF01, 0xFF02};
    quint16 value;

    for (quint8 i = 0; i < static_cast <quint8> (list.count()); i++)
    {
        Attribute attribute = cluster->attribute(list.value(i));

        switch (list.value(i))
        {
            case 0xFF01:

                if (attribute->dataType() != DATA_TYPE_STRING || attribute->data().length() < 4)
                    break;

                memcpy(&value, attribute->data().constData() + 2, sizeof(value));
                m_value = QString("%1v").arg(qFromLittleEndian(value) / 1000.0);
                return;

            case 0xFF02:

                if (attribute->dataType() != DATA_TYPE_STRUCTURE || attribute->data().length() < 7)
                    break;

                memcpy(&value, attribute->data().constData() + 5, sizeof(value));
                m_value = QString("%1v").arg(qFromLittleEndian(value) / 1000.0);
                return;
        }
    }
}

Status::Status(void) : PropertyObject("status", CLUSTER_ON_OFF) {}

void Status::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);

    if (attribute->dataType() != DATA_TYPE_BOOLEAN || attribute->data().length() != 1)
        return;

    m_value = attribute->data().at(0) ? "on" : "off";
}

Level::Level(void) : PropertyObject("level", CLUSTER_LEVEL_CONTROL) {}

void Level::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);

    if (attribute->dataType() != DATA_TYPE_8BIT_UNSIGNED || attribute->data().length() != 1)
        return;

    m_value = static_cast <quint8> (attribute->data().at(0));
}

ColorTemperature::ColorTemperature(void) : PropertyObject("colorTemperature", CLUSTER_COLOR_CONTROL) {}

void ColorTemperature::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0007);
    qint16 value;

    if (attribute->dataType() != DATA_TYPE_16BIT_UNSIGNED || attribute->data().length() != 2)
        return;

    memcpy(&value, attribute->data().constData(), sizeof(value));
    m_value = qFromLittleEndian(value);
}

Illuminance::Illuminance(void) : PropertyObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT) {}

void Illuminance::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);
    qint16 value;

    if (attribute->dataType() != DATA_TYPE_16BIT_UNSIGNED || attribute->data().length() != 2)
        return;

    memcpy(&value, attribute->data().constData(), sizeof(value));
    m_value = static_cast <quint32> (value ? pow(10, (value - 1) / 10000.0) : 0);
}

Temperature::Temperature(void) : PropertyObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT) {}

void Temperature::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);
    qint16 value;

    if (attribute->dataType() != DATA_TYPE_16BIT_SIGNED || attribute->data().length() != 2)
        return;

    memcpy(&value, attribute->data().constData(), sizeof(value));
    m_value = static_cast <double> (qFromLittleEndian(value)) / 100;
}

Humidity::Humidity(void) : PropertyObject("humidity", CLUSTER_RELATIVE_HUMIDITY) {}

void Humidity::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);
    qint16 value;

    if (attribute->dataType() != DATA_TYPE_16BIT_UNSIGNED || attribute->data().length() != 2)
        return;

    memcpy(&value, attribute->data().constData(), sizeof(value));
    m_value = static_cast <double> (qFromLittleEndian(value)) / 100;
}

Occupancy::Occupancy(void) : PropertyObject("occupied", CLUSTER_OCCUPANCY_SENSING) {}

void Occupancy::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);

    if (attribute->dataType() != DATA_TYPE_8BIT_BITMAP || attribute->data().length() != 1)
        return;

    m_value = attribute->data().at(0) ? true : false;
}
