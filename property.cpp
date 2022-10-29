#include <math.h>
#include <QtEndian>
#include "property.h"

void PropertyObject::registerMetaTypes(void)
{
    qRegisterMetaType <Properties::BatteryVoltage>          ("batteryVoltageProperty");
    qRegisterMetaType <Properties::BatteryPercentage>       ("batteryPercentageProperty");
    qRegisterMetaType <Properties::BatteryUndivided>        ("batteryUndividedProperty");
    qRegisterMetaType <Properties::Status>                  ("statusProperty");
    qRegisterMetaType <Properties::Contact>                 ("contactProperty");
    qRegisterMetaType <Properties::Level>                   ("levelProperty");
    qRegisterMetaType <Properties::ColorHS>                 ("colorHSProperty");
    qRegisterMetaType <Properties::ColorXY>                 ("colorXYProperty");
    qRegisterMetaType <Properties::ColorTemperature>        ("colorTemperatureProperty");
    qRegisterMetaType <Properties::Illuminance>             ("illuminanceProperty");
    qRegisterMetaType <Properties::Temperature>             ("temperatureProperty");
    qRegisterMetaType <Properties::Humidity>                ("humidityProperty");
    qRegisterMetaType <Properties::Occupancy>               ("occupancyProperty");
    qRegisterMetaType <Properties::Energy>                  ("energyProperty");
    qRegisterMetaType <Properties::Power>                   ("powerProperty");
    qRegisterMetaType <Properties::IdentifyAction>          ("identifyActionProperty");
    qRegisterMetaType <Properties::SwitchAction>            ("switchActionProperty");
    qRegisterMetaType <Properties::LevelAction>             ("levelActionProperty");

    qRegisterMetaType <PropertiesIAS::Contact>              ("iasContactProperty");
    qRegisterMetaType <PropertiesIAS::Occupancy>            ("iasOccupancyProperty");
    qRegisterMetaType <PropertiesIAS::Smoke>                ("iasSmokeProperty");
    qRegisterMetaType <PropertiesIAS::WaterLeak>            ("iasWaterLeakProperty");

    qRegisterMetaType <PropertiesLUMI::Data>                ("lumiDataProperty");
    qRegisterMetaType <PropertiesLUMI::BatteryVoltage>      ("lumiBatteryVoltageProperty");
    qRegisterMetaType <PropertiesLUMI::Power>               ("lumiPowerProperty");
    qRegisterMetaType <PropertiesLUMI::SwitchAction>        ("lumiSwitchActionProperty");
    qRegisterMetaType <PropertiesLUMI::CubeRotation>        ("lumiCubeRotationProperty");
    qRegisterMetaType <PropertiesLUMI::CubeMovement>        ("lumiCubeMovementProperty");

    qRegisterMetaType <PropertiesPTVO::CO2>                 ("ptvoCO2Property");
    qRegisterMetaType <PropertiesPTVO::Temperature>         ("ptvoTemperatureProperty");
    qRegisterMetaType <PropertiesPTVO::ChangePattern>       ("ptvoChangePatternProperty");
    qRegisterMetaType <PropertiesPTVO::Pattern>             ("ptvoPatternProperty");
    qRegisterMetaType <PropertiesPTVO::SwitchAction>        ("ptvoSwitchActionProperty");

    qRegisterMetaType <PropertiesLifeControl::AirQuality>   ("lifeControlAirQualityProperty");
    qRegisterMetaType <PropertiesTUYA::PresenceSensor>      ("tuyaPresenceSensorProperty");
}

quint8 PropertyObject::percentage(double min, double max, double value)
{
    if (value < min)
        value = min;

    if (value > max)
        value = max;

    return static_cast <quint8> ((value - min) / (max - min) * 100);
}

void Properties::BatteryVoltage::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0020 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = percentage(2850, 3200, static_cast <quint8> (data.at(0)) * 100);
}

void Properties::BatteryPercentage::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0021 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = static_cast <quint8> (data.at(0)) / 2.0;
}

void Properties::BatteryUndivided::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0021 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = static_cast <quint8> (data.at(0));
}

void Properties::Status::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0000 || dataType != DATA_TYPE_BOOLEAN || data.length() != 1)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void Properties::Contact::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0000 || dataType != DATA_TYPE_BOOLEAN || data.length() != 1)
        return;

    m_value = data.at(0) ? true : false;
}

void Properties::Level::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0000 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = static_cast <quint8> (data.at(0));
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
            m_colorX = static_cast <double> (qFromLittleEndian(value)) / 0xFFFF;
            break;
        }

        case 0x0004:
        {
            quint16 value = 0;

            if (dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
                return;

            memcpy(&value, data.constData(), data.length());
            m_colorY = static_cast <double> (qFromLittleEndian(value)) / 0xFFFF;
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

void PropertiesIAS::ZoneStatus::parseCommand(quint8 commandId, const QByteArray &payload)
{
    quint16 value;

    if (commandId != 0x00)
        return;

    memcpy(&value, payload.constData(), sizeof(value));
    value = qFromLittleEndian(value);
    m_map.insert(m_name, (value & 0x0001) ? true : false);

    if (value & 0x0004)
        m_map.insert("tamper", true);
    else
        m_map.remove("tamper");

    if (value & 0x0008)
        m_map.insert("batteryLow", true);
    else
        m_map.remove("batteryLow");

    m_value = m_map;
}

void PropertiesLUMI::Data::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId == 0x00F7)
    {

        if (dataType != DATA_TYPE_OCTET_STRING)
            return;

        for (quint8 i = 0; i < static_cast <quint8> (data.length()); i++)
        {
            quint8 itemType = static_cast <quint8> (data.at(i + 1)), offset = i + 2, size = zclDataSize(itemType, data, &offset);

            if (!size)
                break;

            parseData(data.at(i), itemType, data.mid(offset, size));
            i += size + 1;
        }
    }

    else
        parseData(attributeId, dataType, data);

    if (m_map.isEmpty())
        return;

    m_value = m_map;
}

void PropertiesLUMI::Data::parseData(quint16 dataPoint, quint8 dataType, const QByteArray &data)
{
    switch (dataPoint)
    {
        case 0x0003:
        {
            if (m_model != "lumi.sen_ill.mgl01")
            {
                if (dataType != DATA_TYPE_8BIT_SIGNED || data.length() != 1)
                    break;

                m_map.insert("temperature", static_cast <qint8> (data.at(0)));
            }

            break;
        }

        case 0x0005:
        {
            quint16 value;

            if (dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
                break;

            memcpy(&value, data.constData(), data.length());
            m_map.insert("outageCount", qFromLittleEndian(value) - 1);
            break;
        }

        case 0x0064:
        {
            if (m_model == "lumi.sen_ill.mgl01")
            {
                quint32 value;

                if (dataType != DATA_TYPE_32BIT_UNSIGNED || data.length() != 4)
                    break;

                memcpy(&value, data.constData(), data.length());
                m_map.insert("illuminance", qFromLittleEndian(value));
            }
            else
            {
                if (dataType != DATA_TYPE_BOOLEAN || data.length() != 1)
                    break;

                m_map.insert("status", data.at(0) ? "on" : "off");
            }

            break;
        }

        case 0x0065:
        case 0x0142:
        {
            if (m_model == "lumi.motion.ac01")
            {
                if (dataType != DATA_TYPE_8BIT_SIGNED || data.length() != 1)
                    break;

                m_map.insert("occupancy", data.at(0) ? true : false);
            }

            break;
        }

        case 0x0066:
        case 0x010C:
        case 0x0143:
        {
            if (m_model == "lumi.motion.ac01")
            {
                if (dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
                    break;

                if (dataPoint != 0x0066 ? dataPoint == 0x010C : m_version < 50)
                {
                    QList <QString> list = {"low", "medium", "high"};
                    m_map.insert("sensitivity", list.value(data.at(0) - 1, "unknown"));
                }
                else
                {
                    QList <QString> list = {"enter", "leave", "enterLeft", "leaveRight", "enterRight", "leaveLeft", "approach", "absent"};
                    m_map.insert("event", list.value(data.at(0), "unknown"));
                    m_map.insert("occupancy", data.at(0) != 0x01 ? true : false);
                }
            }

            break;
        }

        case 0x0067:
        case 0x0144:
        {
            if (m_model == "lumi.motion.ac01")
            {
                QList <QString> list = {"undirected", "directed"};

                if (dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
                    break;

                m_map.insert("mode", list.value(data.at(0), "unknown"));
            }

            break;
        }

        case 0x0069:
        case 0x0146:
        {
            if (m_model == "lumi.motion.ac01")
            {
                QList <QString> list = {"far", "middle", "near"};

                if (dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
                    break;

                m_map.insert("distance", list.value(data.at(0), "unknown"));
            }

            break;
        }

        case 0x0095:
        {
            float value;

            if (dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
                break;

            memcpy(&value, data.constData(), data.length());
            m_map.insert("energy", static_cast <double> (round(value * 100)) / 100);
            break;
        }

        case 0x0096:
        {
            float value;

            if (dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
                break;

            memcpy(&value, data.constData(),  data.length());
            m_map.insert("voltage", static_cast <double> (round(value)) / 10);
            break;
        }

        case 0x0097:
        {
            float value;

            if (dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
                break;

            memcpy(&value, data.constData(),  data.length());
            m_map.insert("current", static_cast <double> (round(value)) / 1000);
            break;
        }

        case 0x0098:
        {
            float value;

            if (dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
                break;

            memcpy(&value, data.constData(), data.length());
            m_map.insert("power", static_cast <double> (round(value * 100)) / 100);
            break;
        }
    }
}

void PropertiesLUMI::BatteryVoltage::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0xFF01:
        {
            quint16 value = 0;

            if (dataType != DATA_TYPE_CHARACTER_STRING || data.length() < 4)
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

void PropertiesLUMI::Power::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0055 || dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = static_cast <double> (round(value * 100)) / 100;
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

void PropertiesPTVO::CO2::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
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
            if (dataType != DATA_TYPE_CHARACTER_STRING || QString(data) != "ppm")
                return;

            m_value = m_buffer;
            break;
        }
    }
}

void PropertiesPTVO::Temperature::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
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
            if (dataType != DATA_TYPE_CHARACTER_STRING || QString(data) != "C")
                return;

            m_value = m_buffer;
            break;
        }
    }
}

void PropertiesPTVO::ChangePattern::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0000 || dataType != DATA_TYPE_BOOLEAN || data.length() != 1)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void PropertiesPTVO::Pattern::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0055 || dataType != DATA_TYPE_SINGLE_PRECISION || data.length() != 4)
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = static_cast <quint8> (value);
}

void PropertiesPTVO::SwitchAction::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    if (attributeId != 0x0055 || dataType != DATA_TYPE_8BIT_UNSIGNED || data.length() != 1)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void PropertiesLifeControl::AirQuality::parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    qint16 value;

    if (dataType != DATA_TYPE_16BIT_SIGNED || dataType != DATA_TYPE_16BIT_UNSIGNED || data.length() != 2)
        return;

    memcpy(&value, data.constData(), data.length());

    switch (attributeId)
    {
        case 0x0000: m_map.insert("tempertature", qFromLittleEndian(value) / 100.0); break;
        case 0x0001: m_map.insert("humidity", qFromLittleEndian(value) / 100.0); break;
        case 0x0002: m_map.insert("eco2", qFromLittleEndian(value)); break;
        case 0x0003: m_map.insert("voc", qFromLittleEndian(value)); break;
    }

    m_value = m_map;
}

void PropertiesTUYA::PresenceSensor::parseCommand(quint8 commandId, const QByteArray &payload)
{
    const tuyaHeaderStruct *header = reinterpret_cast <const tuyaHeaderStruct*> (payload.constData());
    QVariant data;

    if (commandId != 0x02)
        return;

    switch (header->dataType)
    {
        case 0x02:

            if (header->length == 4)
            {
                quint32 value;
                memcpy(&value, payload.constData() + sizeof(tuyaHeaderStruct), header->length);
                data = qFromBigEndian(value);
            }

            break;

        case 0x04:

            if (header->length == 1)
                data = payload.at(sizeof(tuyaHeaderStruct)) ? true : false;

            break;
    }

    if (!data.isValid())
        return;

    switch (header->dataPoint)
    {
        case 0x01: m_map.insert("occupancy", data.toBool()); break;
        case 0x02: m_map.insert("sensitivity", data.toInt()); break;
        case 0x03: m_map.insert("distanceMin", data.toDouble() / 100); break;
        case 0x04: m_map.insert("distanceMax", data.toDouble() / 100); break;
        case 0x65: m_map.insert("detectionDelay", data.toInt()); break;
        case 0x68: m_map.insert("illuminance", data.toInt()); break;
    }

    if (m_map.isEmpty())
        return;

    m_value = m_map;
}
