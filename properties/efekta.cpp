#include <QtEndian>
#include <QtMath>
#include "efekta.h"

void PropertiesEfekta::ReportingDelay::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0201 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void PropertiesEfekta::TemperatureSettings::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x0210:
        case 0x0221:
        case 0x0222:
        {
            qint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            value = qFromLittleEndian(value);

            switch (attributeId)
            {
                case 0x0210: map.insert("temperatureOffset", value / 10.0); break;
                case 0x0221: map.insert("temperatureHigh", value); break;
                case 0x0222: map.insert("temperatureLow", value); break;
            }

            break;
        }

        case 0x0220:
        case 0x0225:
        {
             map.insert(attributeId == 0x0220 ? "temperatureRelay" : "temperatureRelayInvert", data.at(0) ? true : false);
             break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesEfekta::HumiditySettings::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x0210:
        {
            qint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            map.insert("humidityOffset", qFromLittleEndian(value));
            break;
        }

        case 0x0221:
        case 0x0222:
        {
             map.insert(attributeId == 0x0221 ? "humidityHigh" : "humidityLow", static_cast <quint8> (data.at(0)));
             break;
        }

        case 0x0220:
        case 0x0225:
        {
             map.insert(attributeId == 0x0220 ? "humidityRelay" : "humidityRelayInvert", data.at(0) ? true : false);
             break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesEfekta::CO2Settings::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x0205:
        case 0x0207:
        case 0x0221:
        case 0x0222:
        {
            uint16_t value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            value = qFromLittleEndian(value);

            switch (attributeId)
            {
                case 0x0205: map.insert("altitude", value); break;
                case 0x0207: map.insert("co2ManualCalibration", value); break;
                case 0x0221: map.insert("co2High", value); break;
                case 0x0222: map.insert("co2Low", value); break;
            }

            break;
        }

        case 0x0209: map.insert("indicatorLevel", static_cast <quint8> (data.at(0))); break;

        default:
        {
            bool value = data.at(0) ? true : false;

            switch (attributeId)
            {
                case 0x0202: map.insert("co2ForceCalibration", value); break;
                case 0x0203: map.insert("autoBrightness", value); break;
                case 0x0204: map.insert("co2LongChart", value); break;
                case 0x0206: map.insert("co2FactoryReset", value); break;
                case 0x0211: map.insert("indicator", value); break;
                case 0x0220: map.insert("co2Relay", value); break;
                case 0x0225: map.insert("co2RelayInvert", value); break;
                case 0x0244: map.insert("pressureLongChart", value); break;
                case 0x0401: map.insert("nightBacklight", value); break;
                case 0x0402: map.insert("co2AutoCalibration", value); break;
            }

            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesEfekta::PMSensor::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x00C8:
        case 0x00C9:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            map.insert(attributeId == 0x00C8 ? "pm1" : "pm10", qFromLittleEndian(value));
            break;
        }

        case 0x0201:
        case 0x0221:
        case 0x0222:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            value = qFromLittleEndian(value);

            switch (attributeId)
            {
                case 0x0201: map.insert("readInterval", value); break;
                case 0x0221: map.insert("pm25High", value); break;
                case 0x0222: map.insert("pm25Low", value); break;
            }

            break;
        }

        case 0x0220:
        case 0x0225:
        {
            map.insert(attributeId == 0x0220 ? "pm25Relay" : "pm25RelayInvert", data.at(0) ? true : false);
            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesEfekta::VOCSensor::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x0055:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            map.insert("voc", qFromLittleEndian(value));
            break;
        }

        case 0x0221:
        case 0x0222:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            map.insert(attributeId == 0x0221 ? "vocHigh" : "vocLow", qFromLittleEndian(value));
            break;
        }

        case 0x0220:
        case 0x0225:
        {
            map.insert(attributeId == 0x0220 ? "vocRelay" : "vocRelayInvert", data.at(0) ? true : false);
            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}
