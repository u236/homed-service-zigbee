#include <QtEndian>
#include "custom.h"
#include "zcl.h"

void PropertiesCustom::Command::parseCommand(quint16, quint8 commandId, const QByteArray &)
{
    m_value = enumValue(commandId);
}

void PropertiesCustom::Attribute::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QList <QString> types = {"bool", "value", "enum"};
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
        case 0: m_value = value.toInt() ? true : false; break; // bool
        case 1: m_value = value.toDouble() / m_divider; break; // value
        case 2: m_value = enumValue(value.toInt()); break;     // enum
    }
}

