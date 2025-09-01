#include <QtEndian>
#include "other.h"
#include "zcl.h"

QVariant ActionsYandex::CommonSettings::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // powerMode
        case 1: // interlock
        {
            qint8 value = index ? data.toBool() ? 0x01 : 0x00 : static_cast <qint8> (enumIndex(name, data));
            return value < 0 ? QByteArray() : zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, index ? 0x07 : 0x03, m_manufacturerCode).append(value);
        }

        case 2: // indicator
        {
            quint8 value = data.toBool() ? 0x00 : 0x01;
            return writeAttribute(0x0005, DATA_TYPE_BOOLEAN, &value, sizeof(value));
        }
    }

    return QByteArray();
}

QVariant ActionsYandex::SwitchSettings::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // switchMode
        case 1: // switchType
        {
            qint8 value = static_cast <qint8> (enumIndex(name, data));
            return value < 0 ? QByteArray() : zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, index ? 0x02 : 0x01, m_manufacturerCode).append(value);
        }
    }

    return QByteArray();
}

QVariant ActionsCustom::Attribute::request(const QString &, const QVariant &data)
{
    QList <QString> types = {"bool", "value", "enum", "time"};
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

        case 3:
        {
            QList <QString> list = data.toString().split(':');
            value = list.value(0).toInt() * 3600 + list.value(1).toInt() * 60;
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
