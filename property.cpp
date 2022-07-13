#include <math.h>
#include <QtEndian>
#include "property.h"
#include "zigbee.h"

using namespace Properties;

BatteryVoltage::BatteryVoltage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}

void BatteryVoltage::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0020 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = static_cast <quint8> (data.at(0)) * 0.1;
}

BatteryVoltageLUMI::BatteryVoltageLUMI(void) : PropertyObject("battery", CLUSTER_BASIC) {}

void BatteryVoltageLUMI::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0xFF01:
        {
            quint16 value;

            if (dataType != DATA_TYPE_STRING || data.length() < 4)
                break;

            memcpy(&value, data.constData() + 2, sizeof(value));
            m_value = qFromLittleEndian(value) / 1000.0;
            break;
        }

        case 0xFF02:
        {
            quint16 value;

            if (dataType != DATA_TYPE_STRUCTURE || data.length() < 7)
                break;

            memcpy(&value, data.constData() + 5, sizeof(value));
            m_value = qFromLittleEndian(value) / 1000.0;
            break;
        }
    }
}

BatteryPercentage::BatteryPercentage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}

void BatteryPercentage::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0021 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = static_cast <quint8> (data.at(0)) / 2.0;
}

BatteryPercentageIKEA::BatteryPercentageIKEA(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}

void BatteryPercentageIKEA::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0021 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = static_cast <quint8> (data.at(0));
}

Status::Status(void) : PropertyObject("status", CLUSTER_ON_OFF) {}

void Status::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0000 || dataType != DATA_TYPE_BOOLEAN || data.length() != 1)
        return;

    m_value = data.at(0) ? "on" : "off";
}

Level::Level(void) : PropertyObject("level", CLUSTER_LEVEL_CONTROL) {}

void Level::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0000 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = static_cast <quint8> (data.at(0));
}

AnalogCO2::AnalogCO2(void) : PropertyObject("co2", CLUSTER_ANALOG_INPUT) {}

void AnalogCO2::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0055:
        {
            float value;

            if (dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
                return;

            memcpy(&value, data.constData(), sizeof(value));
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

AnalogTemperature::AnalogTemperature(void) : PropertyObject("temperature", CLUSTER_ANALOG_INPUT) {}

void AnalogTemperature::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0055:
        {
            float value;

            if (dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
                return;

            memcpy(&value, data.constData(), sizeof(value));
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

ColorTemperature::ColorTemperature(void) : PropertyObject("colorTemperature", CLUSTER_COLOR_CONTROL) {}

void ColorTemperature::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    qint16 value;

    if (attributeId != 0x0007 || dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), sizeof(value));
    m_value = qFromLittleEndian(value);
}

Illuminance::Illuminance(void) : PropertyObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT) {}

void Illuminance::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    quint16 value;

    if (attributeId != 0x0000 || dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), sizeof(value));
    m_value = static_cast <quint32> (value ? pow(10, (qFromLittleEndian(value) - 1) / 10000.0) : 0);
}

Temperature::Temperature(void) : PropertyObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT) {}

void Temperature::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    qint16 value;

    if (attributeId != 0x0000 || dataType != DATA_TYPE_16BIT_SIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), sizeof(value));
    m_value = qFromLittleEndian(value) / 100.0;
}

Humidity::Humidity(void) : PropertyObject("humidity", CLUSTER_RELATIVE_HUMIDITY) {}

void Humidity::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    qint16 value;

    if (attributeId != 0x0000 || dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), sizeof(value));
    m_value = qFromLittleEndian(value) / 100.0;
}

Occupancy::Occupancy(void) : PropertyObject("occupancy", CLUSTER_OCCUPANCY_SENSING) {}

void Occupancy::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0000 || dataType != DATA_TYPE_8BIT_BITMAP || data.length() != 1)
        return;

    m_value = data.at(0) ? true : false;
}

CubeMovement::CubeMovement(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}

void CubeMovement::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    qint16 value;

    if (attributeId != 0x0055 || dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), sizeof(value));
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

CubeRotation::CubeRotation(void) : PropertyObject("action", CLUSTER_ANALOG_INPUT, true) {}

void CubeRotation::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    float value;

    if (attributeId != 0x0055 || dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
        return;

    memcpy(&value, data.constData(), sizeof(value));
    m_value = value < 0 ? "rotateLeft" : "rotateRight";
}

IdentifyAction::IdentifyAction(void) : PropertyObject("action", CLUSTER_IDENTIFY, true) {}

void IdentifyAction::parseCommand(quint8 commandId, const QByteArray &payload)
{
    Q_UNUSED(payload)

    if (commandId != 0x01)
        return;

    m_value = "identify";
}

SwitchAction::SwitchAction(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}

void SwitchAction::parseCommand(quint8 commandId, const QByteArray &payload)
{
    Q_UNUSED(payload)

    switch (commandId)
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
        case 0x02: m_value = "toggle"; break;
    }
}

SwitchActionLUMI::SwitchActionLUMI(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}

void SwitchActionLUMI::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if ((attributeId != 0x0000 && attributeId != 0x8000) || (dataType != DATA_TYPE_BOOLEAN && dataType != DATA_TYPE_8BIT_UNSIGNED) || data.length() != 1)
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

SwitchActionPTVO::SwitchActionPTVO(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}

void SwitchActionPTVO::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0055 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = data.at(0) ? "on" : "off";
}

LevelAction::LevelAction(void) : PropertyObject("action", CLUSTER_LEVEL_CONTROL, true) {}

void LevelAction::parseCommand(quint8 commandId, const QByteArray &payload)
{
    Q_UNUSED(payload)

    switch (commandId)
    {
        case 0x01: m_value = "moveDown"; break;
        case 0x05: m_value = "moveUp"; break;
        case 0x07: m_value = "moveStop"; break;
    }
}
