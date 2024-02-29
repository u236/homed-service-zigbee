#include <QtEndian>
#include "custom.h"

//
#include "logger.h"
//

void PropertiesCustom::Command::parseCommand(quint16, quint8 commandId, const QByteArray &)
{
    m_value = enumValue(commandId);
}

void PropertiesCustom::Attribute::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributeId)
        return;

    switch (m_dataType)
    {
        case DATA_TYPE_BOOLEAN: m_value = data.at(0) ? true : false; break;
        case DATA_TYPE_8BIT_UNSIGNED: m_value = static_cast <quint8> (data.at(0)) / m_divider; break;
        case DATA_TYPE_8BIT_SIGNED: m_value = static_cast <qint8> (data.at(0)) / m_divider; break;

        case DATA_TYPE_16BIT_UNSIGNED:
        {
            quint16 value = 0;
            memcpy(&value, data.constData(), 2);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_16BIT_SIGNED:
        {
            qint16 value = 0;
            memcpy(&value, data.constData(), 2);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_24BIT_UNSIGNED:
        {
            quint32 value = 0;
            memcpy(&value, data.constData(), 3);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_24BIT_SIGNED:
        {
            qint32 value = 0;
            memcpy(&value, data.constData(), 3);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_32BIT_UNSIGNED:
        {
            quint32 value = 0;
            memcpy(&value, data.constData(), 4);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_32BIT_SIGNED:
        {
            qint32 value = 0;
            memcpy(&value, data.constData(), 4);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_40BIT_UNSIGNED:
        {
            quint64 value = 0;
            memcpy(&value, data.constData(), 5);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_40BIT_SIGNED:
        {
            qint64 value = 0;
            memcpy(&value, data.constData(), 5);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_48BIT_UNSIGNED:
        {
            quint64 value = 0;
            memcpy(&value, data.constData(), 6);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_48BIT_SIGNED:
        {
            qint64 value = 0;
            memcpy(&value, data.constData(), 6);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_56BIT_UNSIGNED:
        {
            quint64 value = 0;
            memcpy(&value, data.constData(), 7);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_56BIT_SIGNED:
        {
            qint64 value = 0;
            memcpy(&value, data.constData(), 7);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_64BIT_UNSIGNED:
        {
            quint64 value = 0;
            memcpy(&value, data.constData(), 8);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_64BIT_SIGNED:
        {
            qint64 value = 0;
            memcpy(&value, data.constData(), 8);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_SINGLE_PRECISION:
        {
            float value = 0;
            memcpy(&value, data.constData(), 4);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_DOUBLE_PRECISION:
        {
            double value = 0;
            memcpy(&value, data.constData(), 8);
            m_value = qFromLittleEndian(value) / m_divider;
            break;
        }

        case DATA_TYPE_8BIT_ENUM: m_value = enumValue(static_cast <qint8> (data.at(0))); break;
    }

    logInfo << "custom attribute" << m_name << "is" << m_value;
}

