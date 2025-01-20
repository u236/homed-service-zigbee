#include <QtEndian>
#include "lumi.h"

QVariant ActionsLUMI::PresenceSensor::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0 ... 2:
        {
            qint8 value = static_cast <qint8> (enumIndex(name, data));

            switch (index)
            {
                case 0: m_attributes = {0x010C}; break; // sensitivityMode
                case 1: m_attributes = {0x0144}; break; // detectionMode
                case 2: m_attributes = {0x0146}; break; // distanceMode
            }

            return value < 0 ? QByteArray() : writeAttribute(DATA_TYPE_8BIT_UNSIGNED, &value, sizeof(value));
        }

        case 3: // resetPresence
        {
            m_attributes.clear();
            return !data.toBool() ? QByteArray() : writeAttributeRequest(m_transactionId++, m_manufacturerCode, 0x0157, DATA_TYPE_8BIT_UNSIGNED, QByteArray(1, 0x01)); // TODO: check payload
        }
    }

    return QByteArray();
}

QVariant ActionsLUMI::Thermostat::request(const QString &name, const QVariant &data)
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

            switch (enumIndex(name, data.toString()))
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
            value = static_cast <quint64> (0x0F - enumIndex(name, data.toString())) << 20;
            break;

        default:
            return QByteArray();
    }

    value = qToLittleEndian(~value);
    m_attributes = {0x024F};
    return writeAttribute(DATA_TYPE_64BIT_UNSIGNED, &value, sizeof(value));
}

QVariant ActionsLUMI::ButtonMode::request(const QString &name, const QVariant &data)
{
    QList <QString> list = {"relay", "leftRelay", "rightRelay", "decoupled"};
    qint8 value;

    switch (m_actions.indexOf(name))
    {
        case 2:  m_attributes = {0xFF23}; break; // rightMode
        default: m_attributes = {0xFF22}; break; // leftMode, buttonMode
    }

    switch (list.indexOf(data.toString()))
    {
        case 0:  value = 0x12; break;
        case 1:  value = 0x12; break;
        case 2:  value = 0x22; break;
        case 3:  value = 0xFE; break;
        default: return QByteArray();
    }

    return writeAttribute(DATA_TYPE_8BIT_UNSIGNED, &value, sizeof(value));
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
        list.append(writeAttribute(0x41, QByteArray::fromHex("aa8005d14707011001")));
        list.append(writeAttribute(0x41, QByteArray::fromHex("aa8003d3070801")));
    }
    else
    {
        list.append(writeAttribute(0x41, QByteArray::fromHex("aa8005d14709011000")));
        list.append(writeAttribute(0x41, QByteArray::fromHex("aa8003d3070a01")));
    }

    return list;
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
