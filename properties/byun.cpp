#include <QtEndian>
#include "byun.h"

void PropertiesByun::Sensor::parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload)
{
    quint16 value;

    if (clusterId != CLUSTER_IAS_ZONE || commandId != 0x00)
        return;

    memcpy(&value, payload.constData(), sizeof(value));

    if (qFromLittleEndian(value) != 0x0021)
        return;

    m_value = true;
}

void PropertiesByun::Sensor::parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (clusterId == CLUSTER_IAS_ZONE || attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());

    if (qFromLittleEndian(value))
        return;

    m_value = false;
}
