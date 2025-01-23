#include <QtEndian>
#include "other.h"

void PropertiesByun::Sensor::parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload)
{
    quint16 value = 0;

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

void PropertiesIKEA::Occupancy::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    quint16 value = 0;

    if (commandId != 0x42)
        return;

    memcpy(&value, payload.constData(), sizeof(value));
    m_timeout = qFromLittleEndian(value) / 10;
    m_value = true;
}

void PropertiesIKEA::Occupancy::resetValue(void)
{
    m_value = false;
}

void PropertiesIKEA::StatusAction::parseCommand(quint16, quint8 commandId, const QByteArray &)
{
    if (meta("time").toLongLong() + 1000 > QDateTime::currentMSecsSinceEpoch())
        return;

    switch (commandId)
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
    }
}

void PropertiesIKEA::ArrowAction::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    switch (commandId)
    {
        case 0x07:

            if (payload.at(0) == 2)
                return;

            m_value = payload.at(0) ? "leftClick" : "rightClick";
            break;

        case 0x08:
            setMeta("arrow", payload.at(0) ? "left" : "right");
            m_value = meta("arrow").toString().append("Hold");
            break;

        case 0x09:

            setMeta("time", QDateTime::currentMSecsSinceEpoch());

            if (meta("arrow").isValid())
                return;

            m_value = meta("arrow").toString().append("Release");
            clearMeta("arrow");
            break;
    }
}

void PropertiesYandex::Settings::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x0001: map.insert("switchMode", enumValue("switchMode", static_cast <quint8> (data.at(0)))); break;
        case 0x0002: map.insert("switchType", enumValue("switchType", static_cast <quint8> (data.at(0)))); break;
        case 0x0003: map.insert("powerMode", enumValue("powerMode", static_cast <quint8> (data.at(0)))); break;
        case 0x0004: map.insert("indicator", data.at(0) ? true : false); break;
        case 0x0005: map.insert("interlock", data.at(0) ? true : false); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesCustom::Command::parseCommand(quint16, quint8 commandId, const QByteArray &)
{
    m_value = enumValue(m_name, commandId);
}

void PropertiesCustom::Attribute::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QList <QString> types = {"bool", "value", "enum"}; // TODO: refactor this
    QByteArray buffer = data;
    QVariant value;

    if (attributeId != m_attributeId)
        return;

    if (buffer.length() < 8)
        buffer.append(8 - buffer.length(), 0x00);

    switch (m_dataType)
    {
        case DATA_TYPE_BOOLEAN:
        case DATA_TYPE_8BIT_ENUM:
        case DATA_TYPE_8BIT_UNSIGNED:
            value = static_cast <quint8> (buffer.at(0));
            break;

        case DATA_TYPE_8BIT_SIGNED:
            value = static_cast <qint8> (buffer.at(0));
            break;

        case DATA_TYPE_16BIT_UNSIGNED:
            value = qFromLittleEndian <quint16> (*(reinterpret_cast <quint16*> (buffer.data())));
            break;

        case DATA_TYPE_16BIT_SIGNED:
            value = qFromLittleEndian <qint16> (*(reinterpret_cast <qint16*> (buffer.data())));
            break;

        case DATA_TYPE_24BIT_UNSIGNED:
        case DATA_TYPE_32BIT_UNSIGNED:
            value = qFromLittleEndian <quint32> (*(reinterpret_cast <quint32*> (buffer.data())));
            break;

        case DATA_TYPE_24BIT_SIGNED:
        case DATA_TYPE_32BIT_SIGNED:
            value = qFromLittleEndian <qint32> (*(reinterpret_cast <qint32*> (buffer.data())));
            break;

        case DATA_TYPE_40BIT_UNSIGNED:
        case DATA_TYPE_48BIT_UNSIGNED:
        case DATA_TYPE_56BIT_UNSIGNED:
        case DATA_TYPE_64BIT_UNSIGNED:
            value = qFromLittleEndian <quint64> (*(reinterpret_cast <quint64*> (buffer.data())));
            break;

        case DATA_TYPE_40BIT_SIGNED:
        case DATA_TYPE_48BIT_SIGNED:
        case DATA_TYPE_56BIT_SIGNED:
        case DATA_TYPE_64BIT_SIGNED:
            value = qFromLittleEndian <qint64> (*(reinterpret_cast <qint64*> (buffer.data())));
            break;

        case DATA_TYPE_SINGLE_PRECISION:
            value = qFromLittleEndian <float> (*(reinterpret_cast <float*> (buffer.data())));
            break;

        case DATA_TYPE_DOUBLE_PRECISION:
            value = qFromLittleEndian <double> (*(reinterpret_cast <double*> (buffer.data())));
            break;
    }

    switch (types.indexOf(m_type))
    {
        case 0: m_value = value.toInt() ? true : false; break;     // bool
        case 1: m_value = value.toDouble() / m_divider; break;     // value
        case 2: m_value = enumValue(m_name, value.toInt()); break; // enum
    }
}
