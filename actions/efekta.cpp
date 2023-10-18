#include <QtEndian>
#include "efekta.h"

QByteArray ActionsEfekta::ReportingDelay::request(const QString &, const QVariant &data)
{
    quint16 value = qToLittleEndian <quint16> (data.toInt());
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_16BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsEfekta::TemperatureSettings::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // temperatureOffset
        {
            qint16 value = qToLittleEndian <qint16> (data.toDouble() * 10);
            m_attributes = {0x0210};
            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_16BIT_SIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 1: // temperatureHigh
        case 2: // temperatureLow
        {
            qint16 value = qToLittleEndian <qint16> (data.toInt());
            m_attributes = {static_cast <quint16> (index == 1 ? 0x0221 : 0x0222)};
            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_16BIT_SIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 3: // temperatureRelay
        case 4: // temperatureRelayInvert
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            m_attributes = {static_cast <quint16> (index == 3 ? 0x0220 : 0x0225)};
            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_BOOLEAN, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }
    }

    return QByteArray();
}

QByteArray ActionsEfekta::HumiditySettings::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // humidityOffset
        {
            qint16 value = qToLittleEndian <qint16> (data.toInt());
            m_attributes = {0x0210};
            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, 0x0210, DATA_TYPE_16BIT_SIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 1: // humidityHigh
        case 2: // humidityLow
        {
            quint8 value = qToLittleEndian <quint8> (data.toInt());
            m_attributes = {static_cast <quint16> (index == 1 ? 0x0221 : 0x0222)};
            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_16BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 3: // humidityRelay
        case 4: // humidityRelayInvert
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            m_attributes = {static_cast <quint16> (index == 3 ? 0x0220 : 0x0225)};
            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_BOOLEAN, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }
    }

    return QByteArray();
}

QByteArray ActionsEfekta::CO2Settings::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0 ... 3:
        {
            quint16 value = qToLittleEndian <quint16> (data.toInt());

            switch (index)
            {
                case 0:  m_attributes = {0x0205}; break; // altitude
                case 1:  m_attributes = {0x0207}; break; // co2ManualCalibration
                case 2:  m_attributes = {0x0221}; break; // co2High
                case 3:  m_attributes = {0x0222}; break; // co2Low
            }

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_16BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 4: // indicatorLevel
        {
            quint8 value = qToLittleEndian <quint8> (data.toInt());
            m_attributes = {0x0209};
            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        default:
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;

            switch (index)
            {
                case 5:  m_attributes = {0x0202}; break; // co2ForceCalibration
                case 6:  m_attributes = {0x0203}; break; // autoBrightness
                case 7:  m_attributes = {0x0204}; break; // co2LongChart
                case 8:  m_attributes = {0x0206}; break; // co2FactoryReset
                case 9:  m_attributes = {0x0211}; break; // indicator
                case 10: m_attributes = {0x0220}; break; // co2Relay
                case 11: m_attributes = {0x0225}; break; // co2RelayInvert
                case 12: m_attributes = {0x0244}; break; // pressureLongChart
                case 13: m_attributes = {0x0401}; break; // nightBacklight
                default: return QByteArray();
            }

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_BOOLEAN, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }
    }
}

QByteArray ActionsEfekta::VOCSensor::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // vocHigh
        case 1: // vocLow
        {
            quint16 value = qToLittleEndian <quint16> (data.toInt());
            m_attributes = {static_cast <quint16> (index == 0 ? 0x0221 : 0x2222)};
            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_16BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 2: // vocRelay
        case 3: // vocRelayInvert
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            m_attributes = {static_cast <quint16> (index == 2 ? 0x0220 : 0x0225)};
            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_BOOLEAN, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }
    }

    return QByteArray();
}
