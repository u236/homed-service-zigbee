#include <QtEndian>
#include "other.h"
#include "zcl.h"

QVariant ActionsYandex::Settings::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);
    qint8 value = index < 3 ? static_cast <qint8> (enumIndex(name, data)) : data.toBool() ? 1 : 0, commandId = index + 1;
    m_attributes = {static_cast <quint16> (index + 1)};
    return value < 0 ? QByteArray() : zclHeader(FC_DISABLE_DEFAULT_RESPONSE, m_transactionId++, commandId, m_manufacturerCode).append(value);
}

QVariant ActionsCustom::Attribute::request(const QString &, const QVariant &data)
{
    QList <QString> types = {"bool", "value", "enum"}; // TODO: refactor this
    QVariant value;

    switch (types.indexOf(m_type))
    {
        case 0: value = data.toBool() ? 0x01 : 0x00; break; // bool
        case 1: value = data.toDouble() * m_divider; break; // value

        case 2: // enum
        {
            int index = enumIndex(m_name, data);

            if (index < 0)
                return QByteArray();

            value = index;
            break;
        }
    }

    switch (m_dataType)
    {
        case DATA_TYPE_SINGLE_PRECISION:
        {
            float number = qToLittleEndian(value.toFloat());
            return writeAttribute(m_dataType, &number, sizeof(number));
        }

        case DATA_TYPE_DOUBLE_PRECISION:
        {
            double number = qToLittleEndian(value.toDouble());
            return writeAttribute(m_dataType, &number, sizeof(number));
        }

        default:
        {
            qint64 number = qToLittleEndian <qint64> (value.toDouble());
            return writeAttribute(m_dataType, &number, zclDataSize(m_dataType));
        }
    }
}
