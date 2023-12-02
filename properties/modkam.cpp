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
            qint16 value;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            map.insert(attributeId == 0x0204 ? "co2Low" : "co2High", qFromLittleEndian(value));
            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesModkam::Geiger::parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (clusterId)
    {
        case CLUSTER_ON_OFF:

            if (attributeId != 0x0000)
                return;

            map.insert("alarm", data.at(0) ? true : false);
            break;

        case CLUSTER_ILLUMINANCE_MEASUREMENT:

            switch (attributeId)
            {
                case 0xF001:
                {
                    quint16 value;

                    if (static_cast <size_t> (data.length()) > sizeof(value))
                        return;

                    memcpy(&value, data.constData(), data.length());
                    map.insert("eventsPerMinute", qFromLittleEndian(value));
                    break;
                }

                case 0xF002:
                {
                    quint32 value;

                    if (static_cast <size_t> (data.length()) > sizeof(value))
                        return;

                    memcpy(&value, data.constData(), data.length());
                    map.insert("dosePerHour", qFromLittleEndian(value));
                    break;
                }
            }

            break;

        case CLUSTER_ILLUMINANCE_LEVEL_SENSING:

            switch (attributeId)
            {
                case 0xF000:
                {
                    quint16 value;

                    if (static_cast <size_t> (data.length()) > sizeof(value))
                        return;

                    memcpy(&value, data.constData(), data.length());
                    map.insert("sensitivity", qFromLittleEndian(value));
                    break;
                }

                case 0xF001:
                case 0xF002:
                {
                    map.insert(attributeId == 0xF001 ? "ledFeedback" : "buzzerFeedback", data.at(0) ? true : false);
                    break;
                }

                case 0xF003:
                {
                    map.insert("sensorCount", static_cast <quint8> (data.at(0)));
                    break;
                }

                case 0xF004:
                {
                    switch (static_cast <quint8> (data.at(0)))
                    {
                        case 0x00: map.insert("sensorType", "SBM-20/STS-5/BOI-33"); break;
                        case 0x01: map.insert("sensorType", "SBM-19/STS-6"); break;
                        case 0x02: map.insert("sensorType", "other"); break;
                    }

                    break;
                }

                case 0xF005:
                {
                    quint32 value;

                    if (static_cast <size_t> (data.length()) > sizeof(value))
                        return;

                    memcpy(&value, data.constData(), data.length());
                    map.insert("threshold", qFromLittleEndian(value));
                    break;
                }
            }

            break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}
