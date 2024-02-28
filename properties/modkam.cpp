#include <QtEndian>
#include "modkam.h"

void PropertiesModkam::ButtonAction::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0055)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "hold"; break;
        case 0x01: m_value = "singleClick"; break;
        case 0x02: m_value = "doubleClick"; break;
        case 0x03: m_value = "tripleClick"; break;
        case 0x04: m_value = "quadrupleClick"; break;
        case 0xFF: m_value = "release"; break;
        default:   m_value = "multipleClick"; break;
    }
}

void PropertiesModkam::TemperatureOffset::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0210 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void PropertiesModkam::HumidityOffset::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0210 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void PropertiesModkam::PressureOffset::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    qint32 value = 0;

    if (attributeId != 0x0210 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 10;
}

void PropertiesModkam::CO2Settings::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x0202:
        case 0x0203:
        {
            map.insert(attributeId == 0x0202 ? "co2AutoCalibration" : "ledFeedback", data.at(0) ? true : false);
            break;
        }

        case 0x0204:
        case 0x0205:
        {
            qint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            map.insert(attributeId == 0x0204 ? "co2Low" : "co2High", qFromLittleEndian(value));
            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}
