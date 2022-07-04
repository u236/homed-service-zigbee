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
