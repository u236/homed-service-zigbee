#include <math.h>
#include <QtEndian>
#include "property.h"
#include "logger.h"

void PropertyObject::registerMetaTypes(void)
{
    qRegisterMetaType <Properties::BatteryVoltage>         ("batteryVoltageProperty");
    qRegisterMetaType <Properties::BatteryPercentage>      ("batteryPercentageProperty");
    qRegisterMetaType <Properties::Status>                 ("statusProperty");
    qRegisterMetaType <Properties::Level>                  ("levelProperty");
    qRegisterMetaType <Properties::AnalogCO2>              ("analogCO2Property");
    qRegisterMetaType <Properties::AnalogTemperature>      ("analogTemperatureProperty");
    qRegisterMetaType <Properties::ColorHS>                ("colorHSProperty");
    qRegisterMetaType <Properties::ColorXY>                ("colorXYProperty");
    qRegisterMetaType <Properties::ColorTemperature>       ("colorTemperatureProperty");
    qRegisterMetaType <Properties::Illuminance>            ("illuminanceProperty");
    qRegisterMetaType <Properties::Temperature>            ("temperatureProperty");
    qRegisterMetaType <Properties::Humidity>               ("humidityProperty");
    qRegisterMetaType <Properties::Occupancy>              ("occupancyProperty");
    qRegisterMetaType <Properties::Energy>                 ("energyProperty");
    qRegisterMetaType <Properties::Power>                  ("powerProperty");
    qRegisterMetaType <Properties::IdentifyAction>         ("identifyActionProperty");
    qRegisterMetaType <Properties::SwitchAction>           ("switchActionProperty");
    qRegisterMetaType <Properties::LevelAction>            ("levelActionProperty");

    qRegisterMetaType <PropertiesIKEA::BatteryPercentage>  ("ikeaBatteryPercentageProperty");
    qRegisterMetaType <PropertiesPTVO::SwitchAction>       ("ptvoSwitchActionProperty");

    qRegisterMetaType <PropertiesLUMI::BatteryVoltage>     ("lumiBatteryVoltageProperty");
    qRegisterMetaType <PropertiesLUMI::CubeRotation>       ("lumiCubeRotationProperty");
    qRegisterMetaType <PropertiesLUMI::CubeMovement>       ("lumiCubeMovementProperty");
    qRegisterMetaType <PropertiesLUMI::SwitchAction>       ("lumiSwitchActionProperty");

    qRegisterMetaType <PropertiesTUYA::Dummy>              ("tuyaDummyProperty");
    qRegisterMetaType <PropertiesTUYA::Occupancy>          ("tuyaOccupancyProperty");
    qRegisterMetaType <PropertiesTUYA::Illuminance>        ("tuyaIlluminanceProperty");
}

quint8 PropertyObject::percentage(double min, double max, double value)
{
    if (value < min)
        value = min;

    if (value > max)
        value = max;

    return static_cast <quint8> ((value - min) / (max - min) * 100);
}

void Properties::BatteryPercentage::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0021 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = static_cast <quint8> (data.at(0)) / 2.0;
}

void Properties::BatteryVoltage::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0020 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = percentage(2850, 3200, static_cast <quint8> (data.at(0)) * 100);
}

void Properties::Status::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0000 || dataType != DATA_TYPE_BOOLEAN || data.length() != 1)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void Properties::Level::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0000 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = static_cast <quint8> (data.at(0));
}

void Properties::AnalogCO2::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0055:
        {
            float value = 0;

            if (dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
                return;

            memcpy(&value, data.constData(), data.length());
            m_buffer = value;
            break;
        }

        case 0x001C:
        {
            if (dataType != DATA_TYPE_STRING || QString(data) != "ppm")
                return;

            m_value = m_buffer;
            break;
        }
    }
}

void Properties::AnalogTemperature::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0055:
        {
            float value = 0;

            if (dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
                return;

            memcpy(&value, data.constData(), data.length());
            m_buffer = value;
            break;
        }

        case 0x001C:
        {
            if (dataType != DATA_TYPE_STRING || QString(data) != "C")
                return;

            m_value = m_buffer;
            break;
        }
    }
}

void Properties::ColorHS::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0000:

            if (dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
                return;

            m_colorH = static_cast <quint8> (data.at(0));
            break;

        case 0x0001:

            if (dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
                return;

            m_colorS = static_cast <quint8> (data.at(0));
            break;
    }

    if (!m_colorH.isValid() || !m_colorS.isValid())
        return;

    m_value = QList <QVariant> {m_colorH, m_colorS};
}

void Properties::ColorXY::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0003:
        {
            quint16 value = 0;

            if (dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
                return;

            memcpy(&value, data.constData(), data.length());
            m_colorX = qFromLittleEndian(value);
            break;
        }

        case 0x0004:
        {
            quint16 value = 0;

            if (dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
                return;

            memcpy(&value, data.constData(), data.length());
            m_colorY = qFromLittleEndian(value);
            break;
        }
    }

    if (!m_colorX.isValid() || !m_colorY.isValid())
        return;

    m_value = QList <QVariant> {m_colorX, m_colorY};
}

void Properties::ColorTemperature::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0007 || dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::Illuminance::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = static_cast <quint32> (value ? pow(10, (qFromLittleEndian(value) - 1) / 10000.0) : 0);
}

void Properties::Temperature::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || dataType != DATA_TYPE_16BIT_SIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0;
}

void Properties::Humidity::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0;
}

void Properties::Occupancy::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0000 || dataType != DATA_TYPE_8BIT_BITMAP || data.length() != 1)
        return;

    m_value = data.at(0) ? true : false;
}

void Properties::Energy::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0000:
        {
            qint64 value = 0;

            if (dataType != DATA_TYPE_48BIT_UNSIGNED || data.length() != 6 || !m_multiplier || !m_divider)
                return;

            memcpy(&value, data.constData(), data.length());
            m_value = qFromLittleEndian(value);

            if (m_multiplier > 1)
                m_value = m_value.toDouble() * m_multiplier;

            if (m_divider > 1)
                m_value = m_value.toDouble() / m_divider;

            break;
        }

        case 0x0301:
        {
            quint32 value = 0;

            if (dataType != DATA_TYPE_24BIT_UNSIGNED || data.length() != 3)
                return;

            memcpy(&value, data.constData(), data.length());
            m_multiplier = qFromLittleEndian(value);
            break;
        }

        case 0x0302:
        {
            quint32 value = 0;

            if (dataType != DATA_TYPE_24BIT_UNSIGNED || data.length() != 3)
                return;

            memcpy(&value, data.constData(), data.length());
            m_divider = qFromLittleEndian(value);
            break;
        }
    }
}

void Properties::Power::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x050B:
        {
            qint16 value = 0;

            if (dataType != DATA_TYPE_16BIT_SIGNED || data.length() != 2 || !m_multiplier || !m_divider)
                return;

            memcpy(&value, data.constData(), data.length());
            m_value = qFromLittleEndian(value);

            if (m_multiplier > 1)
                m_value = m_value.toDouble() * m_multiplier;

            if (m_divider > 1)
                m_value = m_value.toDouble() / m_divider;

            break;
        }

        case 0x0604:
        {
            quint16 value = 0;

            if (dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
                return;

            memcpy(&value, data.constData(), data.length());
            m_multiplier = qFromLittleEndian(value);
            break;
        }

        case 0x0605:
        {
            quint16 value = 0;

            if (dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
                return;

            memcpy(&value, data.constData(), data.length());
            m_divider = qFromLittleEndian(value);
            break;
        }
    }
}

void Properties::IdentifyAction::parseCommand(quint8 commandId, const QByteArray &payload)
{
    Q_UNUSED(payload)

    if (commandId != 0x01)
        return;

    m_value = "identify";
}

void Properties::SwitchAction::parseCommand(quint8 commandId, const QByteArray &payload)
{
    Q_UNUSED(payload)

    switch (commandId)
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
        case 0x02: m_value = "toggle"; break;
    }
}

void Properties::LevelAction::parseCommand(quint8 commandId, const QByteArray &payload)
{
    Q_UNUSED(payload)

    switch (commandId)
    {
        case 0x01: m_value = "moveDown"; break;
        case 0x05: m_value = "moveUp"; break;
        case 0x07: m_value = "moveStop"; break;
    }
}

void PropertiesIKEA::BatteryPercentage::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0021 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = static_cast <quint8> (data.at(0));
}

void PropertiesPTVO::SwitchAction::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0055 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void PropertiesLUMI::BatteryVoltage::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0xFF01:
        {
            quint16 value = 0;

            if (dataType != DATA_TYPE_STRING || data.length() < 4)
                break;

            memcpy(&value, data.constData() + 2, sizeof(value));
            m_value = percentage(2850, 3200, qFromLittleEndian(value));
            break;
        }

        case 0xFF02:
        {
            quint16 value = 0;

            if (dataType != DATA_TYPE_STRUCTURE || data.length() < 7)
                break;

            memcpy(&value, data.constData() + 5, sizeof(value));
            m_value = percentage(2850, 3200, qFromLittleEndian(value));
            break;
        }
    }
}

void PropertiesLUMI::SwitchAction::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (((attributeId != 0x0000 || dataType != DATA_TYPE_BOOLEAN) && (attributeId != 0x8000 || dataType != DATA_TYPE_8BIT_UNSIGNED)) || data.length() != 1)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "on"; break;
        case 0x01: m_value = "off"; break;
        case 0x02: m_value = "doubleClick"; break;
        case 0x03: m_value = "tripleClick"; break;
        case 0x04: m_value = "quadrupleClick"; break;
        case 0x80: m_value = "multipleClick"; break;
    }
}

void PropertiesLUMI::CubeRotation::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0055 || dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = value < 0 ? "rotateLeft" : "rotateRight";
}

void PropertiesLUMI::CubeMovement::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0055 || dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), data.length());
    value = qFromLittleEndian(value);

    if (!value)
        m_value = "shake";
    else if (value == 2)
        m_value = "wake";
    else if (value == 3)
        m_value = "fall";
    else if (value >= 512)
        m_value = "tap";
    else if (value >= 256)
        m_value = "slide";
    else if (value >= 128)
        m_value = "flip";
    else if (value >= 64)
        m_value = "drop";
}

void PropertiesTUYA::Dummy::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) // just ignore basic cluster attribute reports
{
    Q_UNUSED(attributeId)
    Q_UNUSED(dataType)
    Q_UNUSED(data)
}

void PropertiesTUYA::Occupancy::parseCommand(quint8 commandId, const QByteArray &payload)
{
    const tuyaHeaderStruct *header = reinterpret_cast <const tuyaHeaderStruct*> (payload.constData());

    if (commandId != 0x02 || header->dataPoint != 0x01 || header->dataType != 0x04 || header->length != 1)
        return;

    m_value = payload.at(sizeof(tuyaHeaderStruct)) ? true : false;
}

void PropertiesTUYA::Illuminance::parseCommand(quint8 commandId, const QByteArray &payload)
{
    const tuyaHeaderStruct *header = reinterpret_cast <const tuyaHeaderStruct*> (payload.constData());
    quint32 value;

    if (commandId != 0x02 || header->dataPoint != 0x68 || header->dataType != 0x02 || header->length != 4)
        return;

    memcpy(&value, payload.constData() + sizeof(tuyaHeaderStruct), header->length);
    m_value = qFromBigEndian(value);
}
