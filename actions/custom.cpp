#include <QtEndian>
#include "custom.h"

//
#include "logger.h"
//

QByteArray ActionsCustom::Attribute::request(const QString &, const QVariant &data)
{
    QByteArray payload;

    switch (m_dataType)
    {
        case DATA_TYPE_BOOLEAN: payload.append(1, data.toBool() ? 0x01 : 0x00); break;

        case DATA_TYPE_8BIT_UNSIGNED:
        case DATA_TYPE_8BIT_SIGNED:
        case DATA_TYPE_16BIT_UNSIGNED:
        case DATA_TYPE_16BIT_SIGNED:
        case DATA_TYPE_24BIT_UNSIGNED:
        case DATA_TYPE_24BIT_SIGNED:
        case DATA_TYPE_32BIT_UNSIGNED:
        case DATA_TYPE_32BIT_SIGNED:
        {
            qint32 value = qToLittleEndian <qint32> (data.toDouble() * m_divider);
            payload.append(reinterpret_cast <char*> (&value), zclDataSize(m_dataType));
            break;
        }

        case DATA_TYPE_40BIT_UNSIGNED:
        case DATA_TYPE_40BIT_SIGNED:
        case DATA_TYPE_48BIT_UNSIGNED:
        case DATA_TYPE_48BIT_SIGNED:
        case DATA_TYPE_56BIT_UNSIGNED:
        case DATA_TYPE_56BIT_SIGNED:
        case DATA_TYPE_64BIT_UNSIGNED:
        case DATA_TYPE_64BIT_SIGNED:
        {
            qint64 value = qToLittleEndian <qint64> (data.toDouble() * m_divider);
            payload.append(reinterpret_cast <char*> (&value), zclDataSize(m_dataType));
            break;
        }

        case DATA_TYPE_8BIT_ENUM:
        {
            qint8 value = listIndex(option().toMap().value("enum").toStringList(), data);

            if (value < 0)
                return QByteArray();

            payload.append(static_cast <char> (value));
        }
    }

    logInfo << "custom attibute action" << m_name << "payload" << payload.toHex(':');
    return writeAttribute(m_dataType, payload.data(), payload.length());
}
