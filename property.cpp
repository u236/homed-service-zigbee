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

OnOffStatus::OnOffStatus(void) : PropertyObject(CLUSTER_ON_OFF, "enabled") {}

void OnOffStatus::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);

    if (attribute->dataType() != DATA_TYPE_BOOLEAN || attribute->data().length() != 1)
        return;

    m_value = attribute->data().at(0) ? true : false;
}

Illuminance::Illuminance(void) : PropertyObject(CLUSTER_ILLUMINANCE_MEASUREMENT, "illuminance") {}

void Illuminance::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);
    qint16 value;

    if (attribute->dataType() != DATA_TYPE_16BIT_UNSIGNED || attribute->data().length() != 2)
        return;

    memcpy(&value, attribute->data().constData(), sizeof(value));
    m_value = static_cast <quint32> (value ? pow(10, (value - 1) / 10000.0) : 0);
}

Temperature::Temperature(void) : PropertyObject(CLUSTER_TEMPERATURE_MEASUREMENT, "temperature") {}

void Temperature::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);
    qint16 value;

    if (attribute->dataType() != DATA_TYPE_16BIT_SIGNED || attribute->data().length() != 2)
        return;

    memcpy(&value, attribute->data().constData(), sizeof(value));
    m_value = static_cast <double> (qFromLittleEndian(value)) / 100;
}

Humidity::Humidity(void) : PropertyObject(CLUSTER_RELATIVE_HUMIDITY, "humidity") {}

void Humidity::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);
    qint16 value;

    if (attribute->dataType() != DATA_TYPE_16BIT_UNSIGNED || attribute->data().length() != 2)
        return;

    memcpy(&value, attribute->data().constData(), sizeof(value));
    m_value = static_cast <double> (qFromLittleEndian(value)) / 100;
}

Occupancy::Occupancy(void) : PropertyObject(CLUSTER_OCCUPANCY_SENSING, "occupied") {}

void Occupancy::convert(const Cluster &cluster)
{
    Attribute attribute = cluster->attribute(0x0000);

    if (attribute->dataType() != DATA_TYPE_8BIT_BITMAP || attribute->data().length() != 1)
        return;

    m_value = attribute->data().at(0) ? true : false;
}
