#include <QtEndian>
#include "lumi.h"

QVariant ActionsLUMI::PresenceSensor::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0 ... 2:
        {
            quint16 attributeId;
            qint8 value = static_cast <qint8> (enumIndex(name, data));

            switch (index)
            {
                case 0: attributeId = 0x010C; break; // sensitivityMode
                case 1: attributeId = 0x0144; break; // detectionMode
                case 2: attributeId = 0x0146; break; // distanceMode
            }

            return value < 0 ? QByteArray() : writeAttribute(attributeId, DATA_TYPE_8BIT_UNSIGNED, &value, sizeof(value));
        }

        case 3: return !data.toBool() ? QByteArray() : writeAttribute(0x0157, DATA_TYPE_8BIT_UNSIGNED, QByteArray(1, 0x01)); // resetPresence
    }

    return QByteArray();
}

QVariant ActionsLUMI::RadiatorThermostat::request(const QString &name, const QVariant &data)
{
    QByteArray sensor = QByteArray::fromHex("00158d000ab7156d"), value;
    QList <QVariant> list;

    switch (m_actions.indexOf(name))
    {
        case 0: // sensorType
        {
            quint32 timestamp = qToBigEndian <quint32> (QDateTime::currentSecsSinceEpoch());

            if (data.toString() == "external")
            {
                value = QByteArray(reinterpret_cast <char*> (&timestamp), sizeof(timestamp)).append(QByteArray::fromHex("3d04")).append(ieeeAddress()).append(sensor).append(QByteArray::fromHex("00010055130a0200006404cec2b6c80000000000013d6465"));
                list.append(writeAttribute(0xFFF2, DATA_TYPE_OCTET_STRING, header(static_cast <quint8> (value.length()), 0x12, 0x02).append(value)));

                value = QByteArray(reinterpret_cast <char*> (&timestamp), sizeof(timestamp)).append(QByteArray::fromHex("3d05")).append(ieeeAddress()).append(sensor).append(QByteArray::fromHex("080007fd160a020ac9e8b1b8d4dacfdfc0eb0000000000013d0465"));
                list.append(writeAttribute(0xFFF2, DATA_TYPE_OCTET_STRING, header(static_cast <quint8> (value.length()), 0x13, 0x02).append(value)));
            }
            else
            {
                value = QByteArray(reinterpret_cast <char*> (&timestamp), sizeof(timestamp)).append(QByteArray::fromHex("3d05")).append(ieeeAddress()).append(12, 0x00);
                list.append(writeAttribute(0xFFF2, DATA_TYPE_OCTET_STRING, header(static_cast <quint8> (value.length()), 0x12, 0x04).append(value)));

                value = QByteArray(reinterpret_cast <char*> (&timestamp), sizeof(timestamp)).append(QByteArray::fromHex("3d04")).append(ieeeAddress()).append(12, 0x00);
                list.append(writeAttribute(0xFFF2, DATA_TYPE_OCTET_STRING, header(static_cast <quint8> (value.length()), 0x13, 0x04).append(value)));
            }

            return list;
        }

        case 1: // externalTemperature
        {
            const Property &property = endpointProperty("lumiData");
            QMap <QString, QVariant> map = property->value().toMap();
            float temperature = qToBigEndian <float> (data.toDouble() * 100);

            map.insert("externalTemperature", data.toDouble());
            property->setValue(map);
            m_properyUpdated = true;

            value = QByteArray(sensor).append(QByteArray::fromHex("00010055")).append(reinterpret_cast <char*> (&temperature), sizeof(temperature));
            return writeAttribute(0xFFF2, DATA_TYPE_OCTET_STRING, header(static_cast <quint8> (value.length()), 0x12, 0x05).append(value));
        }
    }

    return QByteArray();
}

QByteArray ActionsLUMI::RadiatorThermostat::header(quint8 length, quint8 counter, quint8 action)
{
    quint8 data[5] = {0xAA, 0x71, static_cast <quint8> (length + 3), 0x44, counter}, checksum = 0;
    QByteArray header(reinterpret_cast <char*> (data), sizeof(data));

    for (size_t i = 0; i < sizeof(data); i++)
        checksum += data[i];

    return header.append(static_cast <char> (512 - checksum)).append(static_cast <char> (action)).append(0x41).append(static_cast <char> (length));
}

QVariant ActionsLUMI::ElectricThermostat::request(const QString &name, const QVariant &data)
{
    quint64 value;

    switch (m_actions.indexOf(name))
    {
        case 0: // targetTemperature
            value = static_cast <quint64> (0xFFFF - data.toDouble() * 100) << 48;
            break;

        case 1: // systemMode

            if (endpointProperty("lumiData")->value().toMap().value("floor").toBool())
            {
                value = static_cast <quint64> (data.toString() == "off" ? 0xF7 : 0xE7) << 24;
                break;
            }

            switch (enumIndex(name, data))
            {
                case 0:  value = 0xF0; break;
                case 1:  value = 0x0F; break;
                case 2:  value = 0x0E; break;
                case 3:  value = 0x0B; break;
                default: return QByteArray();
            }

            value <<= 24;
            break;


        case 2: // fanMode
            value = static_cast <quint64> (0x0F - enumIndex(name, data)) << 20;
            break;

        default:
            return QByteArray();
    }

    value = qToLittleEndian(~value);
    return writeAttribute(DATA_TYPE_64BIT_UNSIGNED, &value, sizeof(value));
}

QVariant ActionsLUMI::ThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty("lumiData");
    QList <QString> list = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    QByteArray payload = QByteArray(1, 0x04);
    char days = 0;

    if (m_data.isEmpty() || meta("program").toBool())
    {
        m_data = property->value().toMap();
        setMeta("program", false);
    }

    m_data.insert(name, data);

    for (int i = 0; i < list.count(); i++)
        if (m_data.value(QString("schedule%1").arg(list.at(i))).toBool())
            days |= 1 << (i + 1);

    payload.append(days);

    for (int i = 0; i < 4; i++)
    {
        QString key = QString("scheduleP%1").arg(i + 1);
        quint16 time = qToBigEndian <quint16> (static_cast <quint16> (m_data.value(QString("%1Hour").arg(key), i * 6).toInt() * 60 + m_data.value(QString("%1Minute").arg(key), 0).toInt()));
        quint16 temperature = qToBigEndian <quint16> (static_cast <quint16> (m_data.value(QString("%1Temperature").arg(key), 21).toDouble() * 100));
        payload.append(reinterpret_cast <char*> (&time), sizeof(time));
        payload.append(2, 0x00);
        payload.append(reinterpret_cast <char*> (&temperature), sizeof(temperature));
    }

    return writeAttribute(DATA_TYPE_OCTET_STRING, payload);
}

QVariant ActionsLUMI::ButtonMode::request(const QString &name, const QVariant &data)
{
    QList <QString> list = {"relay", "leftRelay", "rightRelay", "decoupled"};
    qint8 value;

    switch (list.indexOf(data.toString()))
    {
        case 0:  value = 0x12; break; // relay
        case 1:  value = 0x12; break; // leftRelay
        case 2:  value = 0x22; break; // rightRelay
        case 3:  value = 0xFE; break; // decoupled
        default: return QByteArray();
    }

    switch (m_actions.indexOf(name))
    {
        case 2:  return writeAttribute(0xFF23, DATA_TYPE_8BIT_UNSIGNED, &value, sizeof(value)); // rightMode
        default: return writeAttribute(0xFF22, DATA_TYPE_8BIT_UNSIGNED, &value, sizeof(value)); // leftMode, buttonMode
    }
}

QVariant ActionsLUMI::SwitchStatusMemory::request(const QString &, const QVariant &data)
{
    quint8 value = data.toBool() ? 0x01 : 0x00;
    return writeAttribute(DATA_TYPE_BOOLEAN, &value, sizeof(value));
}

QVariant ActionsLUMI::LightStatusMemory::request(const QString &, const QVariant &data)
{
    quint8 value = data.toBool() ? 0x01 : 0x00;
    return writeAttribute(DATA_TYPE_BOOLEAN, &value, sizeof(value));
}

QVariant ActionsLUMI::BasicStatusMemory::request(const QString &, const QVariant &data)
{
    QList <QVariant> list;

    if (data.toBool())
    {
        list.append(writeAttribute(DATA_TYPE_OCTET_STRING, QByteArray::fromHex("aa8005d14707011001")));
        list.append(writeAttribute(DATA_TYPE_OCTET_STRING, QByteArray::fromHex("aa8003d3070801")));
    }
    else
    {
        list.append(writeAttribute(DATA_TYPE_OCTET_STRING, QByteArray::fromHex("aa8005d14709011000")));
        list.append(writeAttribute(DATA_TYPE_OCTET_STRING, QByteArray::fromHex("aa8003d3070a01")));
    }

    return list;
}

QVariant ActionsLUMI::CoverStatus::request(const QString &, const QVariant &data)
{
    QList <QString> list = option("invertCover").toBool() ? QList <QString> {"open", "close"} : QList <QString> {"close", "open"};
    float value;

    switch (list.indexOf(data.toString()))
    {
        case 0:  value = 100; break;
        case 1:  value = 0; break;
        default: return QByteArray();
    }

    value = qToLittleEndian(value);
    return writeAttribute(DATA_TYPE_SINGLE_PRECISION, &value, sizeof(value));
}

QVariant ActionsLUMI::CoverStop::request(const QString &, const QVariant &data)
{
    quint16 value = qToLittleEndian <quint16> (0x0002);
    return data.toString() != "stop" ? QByteArray(): writeAttribute(DATA_TYPE_16BIT_UNSIGNED, &value, sizeof(value));
}

QVariant ActionsLUMI::CoverPosition::request(const QString &, const QVariant &data)
{
    float value = data.toFloat();

    if (value > 100)
        value = 100;

    if (!option("invertCover").toBool())
        value = 100 - value;

    value = qToLittleEndian(value);
    return writeAttribute(DATA_TYPE_SINGLE_PRECISION, &value, sizeof(value));
}

QVariant ActionsLUMI::VibrationSensitivity::request(const QString &, const QVariant &data)
{
    qint8 value = listIndex({"high", "medium", "low"}, data);

    if (value < 0)
        return QByteArray();

    value = value * 10 + 1;
    return writeAttribute(DATA_TYPE_8BIT_UNSIGNED, &value, sizeof(value));
}
