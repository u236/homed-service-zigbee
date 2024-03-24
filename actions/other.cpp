#include <QtEndian>
#include "other.h"
#include "zcl.h"

QByteArray ActionsCustom::Attribute::request(const QString &, const QVariant &data)
{
    QList <QString> types = {"bool", "value", "enum"}; // TODO: refactor this
    QVariant value;
    qint64 buffer;

    switch (types.indexOf(m_type))
    {
        case 0: value = data.toBool() ? 0x01 : 0x00; break; // bool
        case 1: value = data.toDouble() * m_divider; break; // value

        case 2: // enum
        {
            int index = enumIndex(m_name, data.toString());

            if (index < 0)
                return QByteArray();

            value = index;
            break;
        }
    }

    buffer = qToLittleEndian <qint64> (value.toDouble());
    return writeAttribute(m_dataType, &buffer, zclDataSize(m_dataType));
}
