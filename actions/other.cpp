#include <QtEndian>
#include "other.h"
#include "zcl.h"

QVariant ActionsSonoff::Thermostat::request(const QString &name, const QVariant &data)
{
    QMap <QString, QVariant> map = endpointProperty()->value().toMap();

    map.insert(name, data);

    switch (m_actions.indexOf(name))
    {
        case 0: // sensorType
        case 1: // externalTemperature
        {
            QByteArray payload = QByteArray::fromHex("2008000101000103");
            qint16 value = qToLittleEndian <qint16> (map.value("externalTemperature").toDouble() * 100);
            int index = enumIndex("sensorType", map.value("sensorType", "internal"));
            return index < 0 ? QByteArray() : writeAttribute(0x601E, DATA_TYPE_ARRAY, payload.append(static_cast <char> (index)).append(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 2: // hysteresisLow
        case 3: // hysteresisHigh
        {
            QByteArray payload = QByteArray::fromHex("0200");
            qint16 low = qToLittleEndian <qint16> (map.value("hysteresisLow", -0.2).toDouble() * 100), high = qToLittleEndian <qint16> (map.value("hysteresisHigh", 0.2).toDouble() * 100);
            return writeAttribute(0x601F, DATA_TYPE_STRUCTURE, payload.append(DATA_TYPE_16BIT_SIGNED).append(reinterpret_cast <char*> (&low), sizeof(low)).append(DATA_TYPE_16BIT_SIGNED).append(reinterpret_cast <char*> (&high), sizeof(high)));
        }
    }

    return QByteArray();
}

QVariant ActionsSonoff::ThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty("sonoffThermostat");
    QList <QString> typeList = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
    QString type = name.mid(0, name.indexOf('P'));
    QByteArray payload = QByteArray::fromHex("01010006");

    payload.append(static_cast <char> (1 << typeList.indexOf(type)));
    payload.append(0x01);

    if (m_data.isEmpty() || meta(QString("%1Program").arg(type)).toBool())
    {
        m_data = property->value().toMap();
        setMeta(QString("%1Program").arg(type), false);
    }

    m_data.insert(name, data);

    for (int i = 0; i < 6; i++)
    {
        QString key = QString("%1P%2").arg(type).arg(i + 1);
        quint16 time = qToLittleEndian(static_cast <quint16> (m_data.value(QString("%1Hour").arg(key), i * 4).toInt() * 60 + m_data.value(QString("%1Minute").arg(key), 0).toInt()));
        quint16 temperature = qToLittleEndian(static_cast <quint16> (m_data.value(QString("%1Temperature").arg(key), 21).toDouble() * 100));
        payload.append(reinterpret_cast <char*> (&time), sizeof(time));
        payload.append(reinterpret_cast <char*> (&temperature), sizeof(temperature));
    }

    return QList <QVariant> {zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x13).append(payload), zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x13).append(QByteArray::fromHex("010000"))};
}

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

QVariant ActionsCustom::Command::request(const QString &name, const QVariant &data)
{
    int index = enumIndex(name, data);
    quint8 value = static_cast <quint8> (index);
    return index < 0 ? QByteArray() : zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, value, m_manufacturerCode);
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

        case 3: // time
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
