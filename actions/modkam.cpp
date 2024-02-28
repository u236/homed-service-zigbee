#include <QtEndian>
#include "modkam.h"

QByteArray ActionsModkam::TemperatureOffset::request(const QString &, const QVariant &data)
{
    qint16 value = qToLittleEndian <qint16> (data.toInt());
    return writeAttribute(DATA_TYPE_16BIT_SIGNED, &value, sizeof(value));
}

QByteArray ActionsModkam::HumidityOffset::request(const QString &, const QVariant &data)
{
    qint16 value = qToLittleEndian <qint16> (data.toInt());
    return writeAttribute(DATA_TYPE_16BIT_SIGNED, &value, sizeof(value));
}

QByteArray ActionsModkam::PressureOffset::request(const QString &, const QVariant &data)
{
    qint32 value = qToLittleEndian <qint32> (data.toInt() * 10);
    return writeAttribute(DATA_TYPE_32BIT_SIGNED, &value, sizeof(value));
}

QByteArray ActionsModkam::CO2Settings::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // co2AutoCalibration
        case 1: // ledFeedback
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            m_attributes = {static_cast <quint16> (index == 0 ? 0x0202 : 0x0203)};
            return writeAttribute(DATA_TYPE_BOOLEAN, &value, sizeof(value));
        }

        case 2: // co2Low
        case 3: // co2High
        {
            quint16 value = qToLittleEndian <quint16> (data.toInt());
            m_attributes = {static_cast <quint16> (index == 2 ? 0x0204 : 0x0205)};
            return writeAttribute(DATA_TYPE_16BIT_UNSIGNED, &value, sizeof(value));
        }
    }

    return QByteArray();
}
