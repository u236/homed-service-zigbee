#include <math.h>
#include <QtEndian>
#include "color.h"
#include "device.h"
#include "property.h"

void PropertyObject::registerMetaTypes(void)
{
    qRegisterMetaType <Properties::BatteryVoltage>              ("batteryVoltageProperty");
    qRegisterMetaType <Properties::BatteryPercentage>           ("batteryPercentageProperty");
    qRegisterMetaType <Properties::DeviceTemperature>           ("deviceTemperatureProperty");
    qRegisterMetaType <Properties::Status>                      ("statusProperty");
    qRegisterMetaType <Properties::PowerOnStatus>               ("powerOnStatusProperty");
    qRegisterMetaType <Properties::Level>                       ("levelProperty");
    qRegisterMetaType <Properties::CoverStatus>                 ("coverStatusProperty");
    qRegisterMetaType <Properties::CoverPosition>               ("coverPositionProperty");
    qRegisterMetaType <Properties::CoverTilt>                   ("coverTiltProperty");
    qRegisterMetaType <Properties::ColorHS>                     ("colorHSProperty");
    qRegisterMetaType <Properties::ColorXY>                     ("colorXYProperty");
    qRegisterMetaType <Properties::ColorTemperature>            ("colorTemperatureProperty");
    qRegisterMetaType <Properties::Illuminance>                 ("illuminanceProperty");
    qRegisterMetaType <Properties::Temperature>                 ("temperatureProperty");
    qRegisterMetaType <Properties::Pressure>                    ("pressureProperty");
    qRegisterMetaType <Properties::Humidity>                    ("humidityProperty");
    qRegisterMetaType <Properties::Occupancy>                   ("occupancyProperty");
    qRegisterMetaType <Properties::Moisture>                    ("moistureProperty");
    qRegisterMetaType <Properties::Energy>                      ("energyProperty");
    qRegisterMetaType <Properties::Voltage>                     ("voltageProperty");
    qRegisterMetaType <Properties::Current>                     ("currentProperty");
    qRegisterMetaType <Properties::Power>                       ("powerProperty");
    qRegisterMetaType <Properties::Scene>                       ("sceneProperty");
    qRegisterMetaType <Properties::SwitchAction>                ("switchActionProperty");
    qRegisterMetaType <Properties::LevelAction>                 ("levelActionProperty");
    qRegisterMetaType <Properties::ColorAction>                 ("colorActionProperty");

    qRegisterMetaType <PropertiesIAS::Contact>                  ("iasContactProperty");
    qRegisterMetaType <PropertiesIAS::Gas>                      ("iasGasProperty");
    qRegisterMetaType <PropertiesIAS::Occupancy>                ("iasOccupancyProperty");
    qRegisterMetaType <PropertiesIAS::Smoke>                    ("iasSmokeProperty");
    qRegisterMetaType <PropertiesIAS::WaterLeak>                ("iasWaterLeakProperty");

    qRegisterMetaType <PropertiesPTVO::ChangePattern>           ("ptvoChangePatternProperty");
    qRegisterMetaType <PropertiesPTVO::Contact>                 ("ptvoContactProperty");
    qRegisterMetaType <PropertiesPTVO::Occupancy>               ("ptvoOccupancyProperty");
    qRegisterMetaType <PropertiesPTVO::WaterLeak>               ("ptvoWaterLeakProperty");
    qRegisterMetaType <PropertiesPTVO::CO2>                     ("ptvoCO2Property");
    qRegisterMetaType <PropertiesPTVO::Temperature>             ("ptvoTemperatureProperty");
    qRegisterMetaType <PropertiesPTVO::Humidity>                ("ptvoHumidityProperty");
    qRegisterMetaType <PropertiesPTVO::Count>                   ("ptvoCountProperty");
    qRegisterMetaType <PropertiesPTVO::Pattern>                 ("ptvoPatternProperty");
    qRegisterMetaType <PropertiesPTVO::SwitchAction>            ("ptvoSwitchActionProperty");

    qRegisterMetaType <PropertiesLUMI::Data>                    ("lumiDataProperty");
    qRegisterMetaType <PropertiesLUMI::Basic>                   ("lumiBasicProperty");
    qRegisterMetaType <PropertiesLUMI::ButtonMode>              ("lumiButtonModeProperty");
    qRegisterMetaType <PropertiesLUMI::Contact>                 ("lumiContactProperty");
    qRegisterMetaType <PropertiesLUMI::Power>                   ("lumiPowerProperty");
    qRegisterMetaType <PropertiesLUMI::Cover>                   ("lumiCoverProperty");
    qRegisterMetaType <PropertiesLUMI::Interlock>               ("lumiInterlockProperty");
    qRegisterMetaType <PropertiesLUMI::Illuminance>             ("lumiIlluminanceProperty");
    qRegisterMetaType <PropertiesLUMI::ButtonAction>            ("lumiButtonActionProperty");
    qRegisterMetaType <PropertiesLUMI::SwitchAction>            ("lumiSwitchActionProperty");
    qRegisterMetaType <PropertiesLUMI::CubeRotation>            ("lumiCubeRotationProperty");
    qRegisterMetaType <PropertiesLUMI::CubeMovement>            ("lumiCubeMovementProperty");
    qRegisterMetaType <PropertiesLUMI::Vibration>               ("lumiVibrationProperty");

    qRegisterMetaType <PropertiesTUYA::LightDimmer>             ("tuyaLightDimmerProperty");
    qRegisterMetaType <PropertiesTUYA::ElectricityMeter>        ("tuyaElectricityMeterProperty");
    qRegisterMetaType <PropertiesTUYA::Cover>                   ("tuyaCoverProperty");
    qRegisterMetaType <PropertiesTUYA::MoesElectricThermostat>  ("tuyaMoesElectricThermostatProperty");
    qRegisterMetaType <PropertiesTUYA::MoesRadiatorThermostat>  ("tuyaMoesRadiatorThermostatProperty");
    qRegisterMetaType <PropertiesTUYA::MoesThermostatProgram>   ("tuyaMoesThermostatProgramProperty");
    qRegisterMetaType <PropertiesTUYA::NeoSiren>                ("tuyaNeoSirenProperty");
    qRegisterMetaType <PropertiesTUYA::WaterValve>              ("tuyaWaterValveProperty");
    qRegisterMetaType <PropertiesTUYA::PresenceSensor>          ("tuyaPresenceSensorProperty");
    qRegisterMetaType <PropertiesTUYA::RadarSensor>             ("tuyaRadarSensorProperty");
    qRegisterMetaType <PropertiesTUYA::ChildLock>               ("tuyaChildLockProperty");
    qRegisterMetaType <PropertiesTUYA::OperationMode>           ("tuyaOperationModeProperty");
    qRegisterMetaType <PropertiesTUYA::IndicatorMode>           ("tuyaIndicatorModeProperty");
    qRegisterMetaType <PropertiesTUYA::SwitchMode>              ("tuyaSwitchModeProperty");
    qRegisterMetaType <PropertiesTUYA::PowerOnStatus>           ("tuyaPowerOnStatusProperty");
    qRegisterMetaType <PropertiesTUYA::ButtonAction>            ("tuyaButtonActionProperty");

    qRegisterMetaType <PropertiesEfekta::ReportingDelay>        ("efektaReportingDelayProperty");
    qRegisterMetaType <PropertiesEfekta::TemperatureOffset>     ("efektaTemperatureOffsetProperty");

    qRegisterMetaType <PropertiesOther::KonkeButtonAction>      ("konkeButtonActionProperty");
    qRegisterMetaType <PropertiesOther::SonoffButtonAction>     ("sonoffButtonActionProperty");
    qRegisterMetaType <PropertiesOther::LifeControlAirQuality>  ("lifeControlAirQualityProperty");
    qRegisterMetaType <PropertiesOther::PerenioSmartPlug>       ("perenioSmartPlugProperty");
}

quint8 PropertyObject::percentage(double min, double max, double value)
{
    if (value < min)
        value = min;

    if (value > max)
        value = max;

    return static_cast <quint8> ((value - min) / (max - min) * 100);
}

bool PropertyObject::checkTransactionId(quint8 transactionId)
{
    if (m_transactionId == transactionId)
        return false;

    m_transactionId = transactionId;
    return true;
}

quint8 PropertyObject::deviceVersion(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->version() : 0;
}

QString PropertyObject::deviceManufacturerName(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->manufacturerName() : QString();
}

QString PropertyObject::deviceModelName(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->modelName() : QString();
}

QVariant PropertyObject::endpointOption(const QString &name)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    QVariant value;

    if (!endpoint || endpoint->device().isNull())
        return value;

    value = endpoint->device()->options().value(QString("%1-%2").arg(name, QString::number(endpoint->id())));
    return value.isValid() ? value : endpoint->device()->options().value(name);
}

void Properties::BatteryVoltage::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0020)
        return;

    m_value = percentage(2850, 3200, static_cast <quint8> (data.at(0)) * 100);
}

void Properties::BatteryPercentage::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0021)
        return;

    m_value = static_cast <quint8> (data.at(0)) / (endpointOption("batteryUndivided").toBool() ? 1.0 : 2.0);
}

void Properties::DeviceTemperature::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) + endpointOption("temperatureOffset").toDouble();
}

void Properties::Status::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void Properties::PowerOnStatus::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x4003)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
        case 0x02: m_value = "toggle"; break;
        case 0xFF: m_value = "previous"; break;
    }
}

void Properties::Level::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = static_cast <quint8> (data.at(0));
}

void Properties::CoverStatus::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0008)
        return;

    m_value = static_cast <quint8> (endpointOption("invertCover").toBool() ? 100 - data.at(0) : data.at(0)) ? "open" : "closed";
}

void Properties::CoverPosition::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0008)
        return;

    m_value = static_cast <quint8> (endpointOption("invertCover").toBool() ? 100 - data.at(0) : data.at(0));
}

void Properties::CoverTilt::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0009)
        return;

    m_value = static_cast <quint8> (endpointOption("invertCover").toBool() ? 100 - data.at(0) : data.at(0));
}

void Properties::ColorHS::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0000:
            m_colorH = static_cast <quint8> (data.at(0));
            break;

        case 0x0001:
            m_colorS = static_cast <quint8> (data.at(0));
            break;
    }

    if (m_colorH.isValid() || m_colorS.isValid())
    {
        Color color = Color::fromHS(m_colorH.toDouble() / 0xFF, m_colorS.toDouble() / 0xFF);
        m_value = QList <QVariant> {static_cast <quint8> (color.r() * 0xFF), static_cast <quint8> (color.g() * 0xFF), static_cast <quint8> (color.b() * 0xFF)};
    }
}

void Properties::ColorXY::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (static_cast <size_t> (data.length()) > sizeof(value))
        return;

    switch (attributeId)
    {
        case 0x0003:
            memcpy(&value, data.constData(), data.length());
            m_colorX = qFromLittleEndian(value);
            break;

        case 0x0004:
            memcpy(&value, data.constData(), data.length());
            m_colorY = qFromLittleEndian(value);
            break;
    }

    if (m_colorX.isValid() || m_colorY.isValid())
    {
        Color color = Color::fromXY(m_colorX.toDouble() / 0xFFFF, m_colorY.toDouble() / 0xFFFF);
        m_value = QList <QVariant> {static_cast <quint8> (color.r() * 0xFF), static_cast <quint8> (color.g() * 0xFF), static_cast <quint8> (color.b() * 0xFF)};
    }
}

void Properties::ColorTemperature::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0007 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::Illuminance::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = static_cast <quint32> (value ? pow(10, (qFromLittleEndian(value) - 1) / 10000.0) : 0) + endpointOption("illuminanceOffset").toDouble();
}

void Properties::Temperature::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0 + endpointOption("temperatureOffset").toDouble();
}

void Properties::Pressure::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 10.0 + endpointOption("pressureOffset").toDouble();
}

void Properties::Humidity::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0 + endpointOption("humidityOffset").toDouble();
}

void Properties::Occupancy::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = data.at(0) ? true : false;
}

void Properties::Moisture::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0 + endpointOption("moistureOffset").toDouble();
}

void Properties::Occupancy::resetValue(void)
{
    m_value = false;
}

void Properties::Energy::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    double divider = endpointOption("energyDivider").toDouble();
    qint64 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / (divider ? divider : 1);
}

void Properties::Voltage::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    double divider = endpointOption("voltageDivider").toDouble();
    qint16 value = 0;

    if (attributeId != 0x0505 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / (divider ? divider : 1) + endpointOption("voltageOffset").toDouble();
}

void Properties::Current::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    double divider = endpointOption("currentDivider").toDouble();
    qint16 value = 0;

    if (attributeId != 0x0508 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / (divider ? divider : 1) + endpointOption("currentOffset").toDouble();
}

void Properties::Power::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    double divider = endpointOption("powerDivider").toDouble();
    qint16 value = 0;

    if (attributeId != 0x050B || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / (divider ? divider : 1) + endpointOption("powerOffset").toDouble();
}

void Properties::Scene::parseCommand(quint8 commandId, const QByteArray &payload, quint8)
{
    const recallSceneStruct *command = reinterpret_cast <const recallSceneStruct*> (payload.constData());
    QVariant scene = endpointOption("scene").toMap().value(QString::number(command->sceneId));

    if (commandId != 0x05)
        return;

    m_value = scene.isValid() ? scene : command->sceneId;
}

void Properties::SwitchAction::parseCommand(quint8 commandId, const QByteArray &, quint8)
{
    switch (commandId)
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
        case 0x02: m_value = "toggle"; break;
    }
}

void Properties::LevelAction::parseCommand(quint8 commandId, const QByteArray &payload, quint8)
{
    switch (commandId)
    {
        case 0x01:
        case 0x05:
        {
            const moveLevelStruct *data = reinterpret_cast <const moveLevelStruct*> (payload.constData());
            m_value = QMap <QString, QVariant> ({{"action", data->mode ? "moveLevelDown" : "moveLevelUp"}, {"moveRate", data->rate}});
            break;
        }

        case 0x02:
        case 0x06:
        {
            const stepLevelStruct *data = reinterpret_cast <const stepLevelStruct*> (payload.constData());
            m_value = QMap <QString, QVariant> ({{"action", data->mode ? "stepLevelDown" : "stepLevelUp"}, {"stepSize", data->size}});
            break;
        }

        case 0x03:
        case 0x07:
        {
            m_value = "stopLevel";
            break;
        }
    }
}

void Properties::ColorAction::parseCommand(quint8 commandId, const QByteArray &payload, quint8)
{
    switch (commandId)
    {
        case 0x01:
        {
            const moveHueStruct *data = reinterpret_cast <const moveHueStruct*> (payload.constData());

            if (data->mode)
                m_value = QMap <QString, QVariant> ({{"action", data->mode == 0x01 ? "moveHueUp" : "moveHueDown"}, {"moveRate", data->rate}});
            else
                m_value = "stopHue";

            break;
        }

        case 0x04:
        {
            const moveSaturationStruct *data = reinterpret_cast <const moveSaturationStruct*> (payload.constData());

            if (data->mode)
                m_value = QMap <QString, QVariant> ({{"action", data->mode == 0x01 ? "moveSaturationUp" : "moveSaturationDown"}, {"moveRate", data->rate}});
            else
                m_value = "stopSaturation";

            break;
        }

        case 0x47:
        {
            m_value = "stopColor";
            break;
        }

        case 0x4B:
        {
            const moveColorTemperatureStruct *data = reinterpret_cast <const moveColorTemperatureStruct*> (payload.constData());

            if (data->mode)
                m_value = QMap <QString, QVariant> ({{"action", data->mode == 0x01 ? "moveColorTemperatureUp" : "moveColorTemperatureDown"}, {"moveRate", qFromLittleEndian(data->rate)}});
            else
                m_value = "stopColorTemperature";

            break;
        }

        case 0x4C:
        {
            const stepColorTemperatureStruct *data = reinterpret_cast <const stepColorTemperatureStruct*> (payload.constData());

            if (data->mode)
                m_value = QMap <QString, QVariant> ({{"action", data->mode == 0x01 ? "stepColorTemperatureUp" : "stepColorTemperatureDown"}, {"stepSize", qFromLittleEndian(data->size)}});
            else
                m_value = "stopColorTemperature";

            break;
        }
    }
}

void PropertiesIAS::ZoneStatus::parseCommand(quint8 commandId, const QByteArray &payload, quint8)
{
    QMap <QString, QVariant> map = m_value.toMap();
    quint16 value;

    if (commandId != 0x00)
        return;

    memcpy(&value, payload.constData(), sizeof(value));
    value = qFromLittleEndian(value);
    map.insert(m_name, (value & 0x0001) ? true : false);

    if (value & 0x0004)
        map.insert("tamper", true);
    else
        map.remove("tamper");

    if (value & 0x0008)
        map.insert("batteryLow", true);
    else
        map.remove("batteryLow");

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesIAS::ZoneStatus::clearValue(void)
{
    QMap <QString, QVariant> map = m_value.toMap();
    map.remove(m_name);
    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesIAS::ZoneStatus::resetValue(void)
{
    QMap <QString, QVariant> map = m_value.toMap();
    map.insert(m_name, false);
    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesPTVO::Status::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = data.at(0) ? true : false;
}

void PropertiesPTVO::AnalogInput::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0055:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            (m_unit.isEmpty() ? m_value : m_buffer) = qFromLittleEndian(value) + endpointOption(QString(m_name).append("Offset")).toDouble();
            break;
        }

        case 0x001C:
        {
            QList <QString> list = QString(data).split(',');

            if (list.value(0) != m_unit)
                return;

            m_value = m_buffer;
            break;
        }
    }
}

void PropertiesPTVO::SwitchAction::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0055)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void PropertiesLUMI::Data::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (attributeId == 0x00F7 || attributeId == 0xFF01)
    {
        for (quint8 i = 0; i < static_cast <quint8> (data.length()); i++)
        {
            quint8 offset = i + 2, size = zclDataSize(static_cast <quint8> (data.at(i + 1)), data, &offset);

            if (!size)
                break;

            parseData(static_cast <quint8> (data.at(i)), data.mid(offset, size), map);
            i += size + 1;
        }
    }
    else
        parseData(attributeId, data, map);

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesLUMI::Data::parseData(quint16 dataPoint, const QByteArray &data, QMap <QString, QVariant> &map)
{
    QString modelName = deviceModelName();

    if (m_multiple && dataPoint != 0x0200)
        return;

    switch (dataPoint)
    {
        case 0x0001:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            map.insert("battery", percentage(2850, 3200, qFromLittleEndian(value)));
            break;
        }


        case 0x0003:
        {
            if (modelName != "lumi.remote.b686opcn01" && modelName != "lumi.sen_ill.mgl01")
                map.insert("temperature", static_cast <qint8> (data.at(0)) + endpointOption("temperatureOffset").toDouble());

            break;
        }

        case 0x0005:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            map.insert("outageCount", qFromLittleEndian(value) - 1);
            break;
        }

        case 0x0009:
        {
            if (modelName != "lumi.remote.b286opcn01" && modelName != "lumi.remote.b486opcn01" && modelName != "lumi.remote.b686opcn01")
                break;

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("operationMode", "command"); break;
                case 0x01: map.insert("operationMode", "event"); break;
            }

            break;
        }

        case 0x0064:
        {
            if (modelName == "lumi.sen_ill.mgl01")
            {
                quint32 value = 0;

                if (static_cast <size_t> (data.length()) > sizeof(value))
                    break;

                memcpy(&value, data.constData(), data.length());
                map.insert("illuminance", qFromLittleEndian(value) + endpointOption("illuminanceOffset").toDouble());
            }

            break;
        }

        case 0x0065:
        case 0x0142:
        {
            if (modelName == "lumi.motion.ac01")
                map.insert("occupancy", data.at(0) ? true : false);

            break;
        }

        case 0x0066:
        case 0x010C:
        case 0x0143:
        {
            if (modelName != "lumi.motion.ac01")
                break;

            if (dataPoint != 0x0066 ? dataPoint == 0x010C : deviceVersion() >= 50)
            {
                switch (static_cast <quint8> (data.at(0)))
                {
                    case 0x01: map.insert("sensitivityMode", "low"); break;
                    case 0x02: map.insert("sensitivityMode", "medium"); break;
                    case 0x03: map.insert("sensitivityMode", "high"); break;
                }
            }
            else
            {
                switch (static_cast <quint8> (data.at(0)))
                {
                    case 0x00: map.insert("event", "enter"); break;
                    case 0x01: map.insert("event", "leave"); break;
                    case 0x02: map.insert("event", "enterLeft"); break;
                    case 0x03: map.insert("event", "leaveRight"); break;
                    case 0x04: map.insert("event", "enterRight"); break;
                    case 0x05: map.insert("event", "leaveLeft"); break;
                    case 0x06: map.insert("event", "approach"); break;
                    case 0x07: map.insert("event", "absent"); break;
                }

                map.insert("occupancy", data.at(0) != 0x01 ? true : false);
            }

            break;
        }

        case 0x0067:
        case 0x0144:
        {
            if (modelName != "lumi.motion.ac01")
                break;

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("detectionMode", "undirected"); break;
                case 0x01: map.insert("detectionMode", "directed"); break;
            }

            break;
        }

        case 0x0069:
        case 0x0146:
        {
            if (modelName != "lumi.motion.ac01")
                break;

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("distanceMode", "far"); break;
                case 0x01: map.insert("distanceMode", "middle"); break;
                case 0x02: map.insert("distanceMode", "near"); break;
            }

            break;
        }

        case 0x0095:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            value = round(qFromLittleEndian(value) * 100) / 100;
            map.insert("energy", value);
            break;
        }

        case 0x0096:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(),  data.length());
            value = round(qFromLittleEndian(value)) / 10;
            map.insert("voltage", value + endpointOption("voltageOffset").toDouble());
            break;
        }

        case 0x0097:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(),  data.length());
            value = deviceModelName() == "lumi.relay.c2acn01" ? qFromLittleEndian(value) : round(qFromLittleEndian(value)) / 1000;
            map.insert("current", value + endpointOption("currentOffset").toDouble());
            break;
        }

        case 0x0098:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            value = round(qFromLittleEndian(value) * 100) / 100;
            map.insert("power", value + endpointOption("powerOffset").toDouble());
            break;
        }

        case 0x00F0:
        {
            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("indicatorMode", "default"); break;
                case 0x01: map.insert("indicatorMode", "inverted"); break;
            }

            break;
        }

        case 0x0200:
        {
            if (!m_multiple && (modelName == "lumi.switch.b2nc01" || modelName == "lumi.switch.b2lc04"))
                break;

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("switchMode", "decoupled"); break;
                case 0x01: map.insert("switchMode", "relay"); break;
            }

            break;
        }

        case 0x0201:
        {
            map.insert("statusMemory", data.at(0) ? true : false);
            break;
        }

        case 0xFF02:
        {
            quint16 value = 0;

            if (data.length() < 7)
                break;

            memcpy(&value, data.constData() + 5, sizeof(value));
            map.insert("battery", percentage(2850, 3200, qFromLittleEndian(value)));
            break;
        }
    }
}

void PropertiesLUMI::ButtonMode::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();
    bool check = deviceModelName() == "lumi.ctrl_neutral1";
    QString value;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x12: value = check ? "relay" : "leftRelay"; break;
        case 0x22: value = "rightRelay"; break;
        case 0xFE: value = "decoupled"; break;
    }

    switch (attributeId)
    {
        case 0xFF22: map.insert(check ? "buttonMode" : "leftMode", value); break;
        case 0xFF23: map.insert("rightMode", value); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesLUMI::Contact::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = data.at(0) ? true : false;
}

void PropertiesLUMI::Interlock::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0xFF06)
        return;

    m_value = data.at(0) ? true : false;
}

void PropertiesLUMI::Power::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    value = round(qFromLittleEndian(value) * 100) / 100;
    m_value = value + endpointOption("powerOffset").toDouble();
}

void PropertiesLUMI::Cover::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map;
    float value = 0;

    if (deviceModelName() == "ZNCLDJ12LM" || attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    value = round(qFromLittleEndian(value));

    if (endpointOption("invertCover").toBool())
        value = 100 - value;

    map.insert("cover", value ? "open" : "closed");
    map.insert("position", value);

    m_value = map;
}

void PropertiesLUMI::Illuminance::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) + endpointOption("illuminanceOffset").toDouble();
}


void PropertiesLUMI::ButtonAction::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000 && attributeId != 0x8000)
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

void PropertiesLUMI::SwitchAction::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());

    switch (qFromLittleEndian(value))
    {
        case 0x0000: m_value = "hold"; break;
        case 0x0001: m_value = "singleClick"; break;
        case 0x0002: m_value = "doubleClick"; break;
        case 0x0003: m_value = "tripleClick"; break;
        case 0x00FF: m_value = "release"; break;
    }
}

void PropertiesLUMI::CubeRotation::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) < 0 ? "rotateLeft" : "rotateRight";
}

void PropertiesLUMI::CubeMovement::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
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

void PropertiesLUMI::Vibration::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x0055:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            value = qFromLittleEndian(value);

            switch (value)
            {
                case 0x0001: map.insert("event", "vibration"); break;
                case 0x0002: map.insert("event", "tilt"); break;
                case 0x0003: map.insert("event", "drop"); break;
            }

            if (value == 0x0001)
                map.insert("vibration", true);

            break;
        }

        case 0x0503:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            map.insert("angle", qFromLittleEndian(value));
            break;
        }

        case 0x0505:
        {
            quint16 value = 0;
            memcpy(&value, data.constData(), sizeof(value));
            map.insert("strength", qFromBigEndian(value));
            break;
        }

        case 0x0508:
        {
            quint64 value;
            qint16 x, y, z;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            value = qFromLittleEndian(value);

            x = static_cast <qint16> (value & 0xFFFF);
            y = static_cast <qint16> (value >> 16 & 0xFFFF);
            z = static_cast <qint16> (value >> 32 & 0xFFFF);

            map.insert("x", round(atan(x / sqrt(pow(y, 2) + pow(z, 2))) * 180 / M_PI));
            map.insert("y", round(atan(y / sqrt(pow(x, 2) + pow(z, 2))) * 180 / M_PI));
            map.insert("z", round(atan(z / sqrt(pow(x, 2) + pow(y, 2))) * 180 / M_PI));
            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesLUMI::Vibration::resetValue(void)
{
    QMap <QString, QVariant> map = m_value.toMap();
    map.insert("vibration", false);
    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::Data::parseCommand(quint8 commandId, const QByteArray &payload, quint8)
{
    const tuyaHeaderStruct *header = reinterpret_cast <const tuyaHeaderStruct*> (payload.constData());
    QVariant data;

    if (commandId != 0x01 && commandId != 0x02)
        return;

    data = parseData(header, payload.mid(sizeof(tuyaHeaderStruct)));

    if (!data.isValid())
        return;

    update(header->dataPoint, data);
}

QVariant PropertiesTUYA::Data::parseData(const tuyaHeaderStruct *header, const QByteArray &data)
{
    switch (header->dataType)
    {
        case TUYA_TYPE_RAW:
            return data.mid(0, header->length);

        case TUYA_TYPE_BOOL:

            if (header->length == 1)
                return data.at(0) ? true : false;

            break;

        case TUYA_TYPE_VALUE:

            if (header->length == 4)
            {
                quint32 value;
                memcpy(&value, data.constData(), header->length);
                return qFromBigEndian(value);
            }

            break;

        case TUYA_TYPE_ENUM:

            if (header->length == 1)
                return static_cast <quint8> (data.at(0));

            break;
    }

    return QVariant();
}

void PropertiesTUYA::LightDimmer::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (dataPoint)
    {
        case 0x01: map.insert("status", data.toBool() ? "on" : "off"); break;
        case 0x02: map.insert("level", static_cast <quint8> (round(data.toInt() * 0xFE / 1000.0))); break;
        case 0x03: map.insert("levelMin", static_cast <quint8> (round(data.toInt() * 0xFE / 1000.0))); break;

        case 0x04:
        {
            switch (data.toInt())
            {
                case 0: map.insert("lightType", "led"); break;
                case 1: map.insert("lightType", "incandescent"); break;
                case 2: map.insert("lightType", "halogen"); break;
            }

            break;
        }

        case 0x05: map.insert("levelMax", static_cast <quint8> (round(data.toInt() * 0xFE / 1000.0))); break;
        case 0x06: map.insert("timeout", data.toInt()); break;

        case 0x0E:
        {
            switch (data.toInt())
            {
                case 0: map.insert("powerOnStatus", "off"); break;
                case 1: map.insert("powerOnStatus", "on"); break;
                case 2: map.insert("powerOnStatus", "previous"); break;
            }

            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::ElectricityMeter::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (deviceManufacturerName() == "_TZE200_lsanae15")
    {
        switch (dataPoint)
        {
            case 0x01: map.insert("energy", data.toInt() / 100.0); break;

            case 0x06:
            {
                QByteArray payload = data.toByteArray();
                quint16 value;

                memcpy(&value, payload.constData(), sizeof(value));
                map.insert("voltage", qFromBigEndian(value) / 10.0 + endpointOption("voltageOffset").toDouble());

                memcpy(&value, payload.constData() + 3, sizeof(value));
                map.insert("current", qFromBigEndian(value) / 1000.0 + endpointOption("currentOffset").toDouble());

                memcpy(&value, payload.constData() + 6, sizeof(value));
                map.insert("power", qFromBigEndian(value) + endpointOption("powerOffset").toDouble());

                break;
            }

            case 0x10: map.insert("status", data.toBool() ? "on" : "off"); break;
        }
    }
    else
    {
        switch (dataPoint)
        {
            case 0x01: map.insert("energy", data.toInt() / 100.0); break;

            case 0x06:
            {
                QByteArray payload = data.toByteArray();
                quint16 value;

                memcpy(&value, payload.constData() + 11, sizeof(value));
                map.insert("current", qFromBigEndian(value) / 1000.0 + endpointOption("currentOffset").toDouble());

                memcpy(&value, payload.constData() + 13, sizeof(value));
                map.insert("voltage", qFromBigEndian(value) / 10.0 + endpointOption("voltageOffset").toDouble());

                break;
            }

            case 0x10: map.insert("status", data.toBool() ? "on" : "off"); break;
            case 0x67: map.insert("power", data.toInt() / 100.0 + endpointOption("powerOffset").toDouble()); break;
            case 0x69: map.insert("frequency", data.toInt() / 100.0); break;
            case 0x6F: map.insert("powerFactor", data.toInt() / 10.0); break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::Cover::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (dataPoint)
    {
        case 0x02:
        case 0x03:
        {
            quint8 value = static_cast <quint8> (endpointOption("invertCover").toBool() ? 100 - data.toInt() : data.toInt());

            map.insert("cover", value ? "open" : "closed");
            map.insert("position", static_cast <quint8> (value));

            break;
        }

        case 0x05:  map.insert("reverse", data.toBool()); break;
        case 0x69:  map.insert("speed", data.toInt()); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::MoesElectricThermostat::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (dataPoint)
    {
        case 0x01: map.insert("status", data.toBool() ? "on" : "off"); break;
        case 0x02: map.insert("operationMode", data.toInt() ? "program" : "manual"); break;
        case 0x10: map.insert("heatingPoint", data.toInt()); break;
        case 0x12: map.insert("temperatureLimitMax", data.toInt()); break;
        case 0x13: map.insert("temperatureMax", data.toInt()); break;
        case 0x14: map.insert("deadZoneTemperature", data.toInt()); break;

        case 0x18:
        {
            QList <QString> list = {"_TZE200_ye5jkfsb", "_TZE200_ztvwu4nk"};
            double value = static_cast <double> (data.toInt());
            map.insert("localTemperature", list.contains(deviceManufacturerName()) ? value : value / 10);
            break;
        }

        case 0x1A: map.insert("temperatureLimitMin", data.toInt()); break;
        case 0x1B: map.insert("temperatureCalibration", data.toInt()); break;
        case 0x24: map.insert("heating", data.toInt() ? false : true); break;
        case 0x28: map.insert("childLock", data.toBool()); break;

        case 0x2B:
        {
            switch (data.toInt())
            {
                case 0:  map.insert("sensor", "internal"); break;
                case 1:  map.insert("sensor", "both"); break;
                case 2:  map.insert("sensor", "external"); break;
                default: map.insert("sensor", "unsupported"); break;
            }

            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::MoesRadiatorThermostat::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (dataPoint)
    {
        case 0x01:
        {
            switch (data.toInt())
            {
                case 0:  map.insert("operationMode", "program"); break;
                case 1:  map.insert("operationMode", "manual"); break;
                case 2:  map.insert("operationMode", "temporary"); break;
                default: map.insert("operationMode", "holiday"); break;
            }

            break;
        }

        case 0x02: map.insert("heatingPoint", data.toInt()); break;
        case 0x03: map.insert("localTemperature", data.toInt() / 10); break;
        case 0x04: map.insert("boost", data.toBool()); break;
        case 0x05: map.insert("boostCountdown", data.toInt()); break;
        case 0x07: map.insert("heating", data.toInt() ? false : true); break;
        case 0x08: map.insert("windowDetection", data.toBool()); break;
        case 0x09: map.insert("windowOpen", data.toInt() ? false : true); break;
        case 0x0D: map.insert("childLock", data.toBool()); break;
        case 0x0E: map.insert("battery", data.toInt()); break;
        case 0x67: map.insert("boostTimeout", data.toInt()); break;
        case 0x68: map.insert("valvePosition", data.toInt()); break;
        case 0x69: map.insert("temperatureCalibration", data.toInt()); break;
        case 0x6A: map.insert("ecoMode", data.toBool()); break;
        case 0x6B: map.insert("ecoModeTemperature", data.toInt()); break;
        case 0x6C: map.insert("temperatureLimitMax", data.toInt()); break;
        case 0x6D: map.insert("temperatureLimitMin", data.toInt()); break;
    }

    if (map.isEmpty())
    {
        m_value = QVariant();
        return;
    }

    map.insert("status", "on");
    m_value = map;
}

void PropertiesTUYA::MoesThermostatProgram::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (dataPoint == 0x65)
    {
        QList <QString> types = {"weekday", "saturday", "sunday"}, names = {"Hour", "Minute", "Temperature"};
        QByteArray program = data.toByteArray();

        for (int i = 0; i < 36; i++)
        {
            double value = static_cast <double> (program.at(i));
            map.insert(QString("%1P%2%3").arg(types.value(i / 12)).arg(i / 3 % 4 + 1).arg(names.value(i % 3)), (i + 1) % 3 ? value : value / 2);
        }

        m_meta.insert("programReceived", true);
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::NeoSiren::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (dataPoint)
    {
        case 0x05:
        {
            switch (data.toInt())
            {
                case 0: map.insert("volume", "low"); break;
                case 1: map.insert("volume", "medium"); break;
                case 2: map.insert("volume", "high"); break;
            }

            break;
        }

        case 0x07: map.insert("duration", data.toInt()); break;
        case 0x0D: map.insert("alarm", data.toBool()); break;
        case 0x0F: map.insert("battery", data.toInt()); break;
        case 0x15: map.insert("melody", data.toInt()); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::WaterValve::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (dataPoint)
    {
        case 0x01: map.insert("status", data.toBool() ? "on" : "off"); break;
        case 0x09: map.insert("timeout", data.toInt() / 60); break;
        case 0x65: map.insert("threshold", data.toInt()); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::PresenceSensor::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (dataPoint)
    {
        case 0x01: map.insert("occupancy", data.toBool()); break;
        case 0x02: map.insert("sensitivity", data.toInt()); break;
        case 0x03: map.insert("distanceMin", data.toDouble() / 100); break;
        case 0x04: map.insert("distanceMax", data.toDouble() / 100); break;
        case 0x09: map.insert("targetDistance", data.toDouble() / 100); break;
        case 0x65: map.insert("detectionDelay", data.toDouble() / 10); break;
        case 0x66: map.insert("fadingTime", data.toInt() / 10); break;
        case 0x68: map.insert("illuminance", data.toInt() + endpointOption("illuminanceOffset").toDouble()); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::RadarSensor::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (dataPoint)
    {
        case 0x01: map.insert("occupancy", data.toBool()); break;
        case 0x02: map.insert("radarSensitivity", data.toInt()); break;
        case 0x66: map.insert("motion", data.toInt() != 0x01 ? true : false); break;
        case 0x67: map.insert("illuminance", data.toInt() + endpointOption("illuminanceOffset").toDouble()); break;
        case 0x69: map.insert("tumbleSwitch", data.toBool()); break;
        case 0x6A: map.insert("tumbleAlarmTime", data.toInt() + 1); break;

        case 0x70:
        {
            switch (data.toInt())
            {
                case 0: map.insert("radarScene", "default"); break;
                case 1: map.insert("radarScene", "area"); break;
                case 2: map.insert("radarScene", "toilet"); break;
                case 3: map.insert("radarScene", "bedroom"); break;
                case 4: map.insert("radarScene", "parlour"); break;
                case 5: map.insert("radarScene", "office"); break;
                case 6: map.insert("radarScene", "hotel"); break;
            }

            break;
        }

        case 0x72:
        {
            switch (data.toInt())
            {
                case 0: map.insert("motionDirection", "standingStill"); break;
                case 1: map.insert("motionDirection", "movingForward"); break;
                case 2: map.insert("motionDirection", "movingBackward"); break;
            }

            break;
        }

        case 0x73: map.insert("motionSpeed", data.toInt()); break;

        case 0x74:
        {
            switch (data.toInt())
            {
                case 0: map.insert("fallDown", "none"); break;
                case 1: map.insert("fallDown", "maybe"); break;
                case 2: map.insert("fallDown", "fall"); break;
            }

            break;
        }

        case 0x75: map.insert("staticDwellAlarm", data.toInt()); break;
        case 0x76: map.insert("fallSensitivity", data.toInt()); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::ChildLock::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x8000)
        return;

    m_value = data.at(0) ? true : false;
}

void PropertiesTUYA::OperationMode::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x8004)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "command"; break;
        case 0x01: m_value = "event"; break;
    }
}

void PropertiesTUYA::IndicatorMode::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x8001)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "default"; break;
        case 0x02: m_value = "inverted"; break;
        case 0x03: m_value = "on"; break;
    }
}

void PropertiesTUYA::SwitchMode::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0xD030)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "toggle"; break;
        case 0x01: m_value = "state"; break;
        case 0x02: m_value = "momentary"; break;
    }
}

void PropertiesTUYA::PowerOnStatus::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x8002)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
        case 0x02: m_value = "previous"; break;
    }
}

void PropertiesTUYA::ButtonAction::parseCommand(quint8 commandId, const QByteArray &payload, quint8 transactionId)
{
    if (!checkTransactionId(transactionId))
        return;

    switch (commandId)
    {
        case 0xFC:

            switch (payload.at(0))
            {
                case 0x00: m_value = "rotateRight"; break;
                case 0x01: m_value = "rotateLeft"; break;
            }

            break;

        case 0xFD:

            switch (payload.at(0))
            {
                case 0x00: m_value = "singleClick"; break;
                case 0x01: m_value = "doubleClick"; break;
                case 0x02: m_value = "hold"; break;
            }

            break;
    }
}

void PropertiesEfekta::ReportingDelay::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    quint16 value;

    if (attributeId != 0x0201 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void PropertiesEfekta::TemperatureOffset::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value;

    if (attributeId != 0x0410 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 10.0;
}

void PropertiesOther::KonkeButtonAction::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x80: m_value = "singleClick"; break;
        case 0x81: m_value = "doubleClick"; break;
        case 0x82: m_value = "hold"; break;
    }
}

void PropertiesOther::SonoffButtonAction::parseCommand(quint8 commandId, const QByteArray &, quint8)
{
    switch (commandId)
    {
        case 0x00: m_value = "hold"; break;
        case 0x01: m_value = "doubleClick"; break;
        case 0x02: m_value = "singleClick"; break;
    }
}

void PropertiesOther::LifeControlAirQuality::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();
    qint16 value = 0;

    if (static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());

    switch (attributeId)
    {
        case 0x0000: map.insert("temperature", qFromLittleEndian(value) / 100.0 + endpointOption("temperatureOffset").toDouble()); break;
        case 0x0001: map.insert("humidity", qFromLittleEndian(value) / 100.0 + endpointOption("humidityOffset").toDouble()); break;
        case 0x0002: map.insert("co2", qFromLittleEndian(value)); break;
        case 0x0003: map.insert("voc", qFromLittleEndian(value)); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesOther::PerenioSmartPlug::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x0000:
        {
            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("powerOnStatus", "off"); break;
                case 0x01: map.insert("powerOnStatus", "on"); break;
                case 0x02: map.insert("powerOnStatus", "prevoious"); break;
            }

            break;
        }

        case 0x0001:
        {
            map.insert("alarmVoltateMin",  data.at(0) & 0x01 ? true : false);
            map.insert("alarmVoltateMax",  data.at(0) & 0x02 ? true : false);
            map.insert("alarmPowerMax",    data.at(0) & 0x04 ? true : false);
            map.insert("alarmEnergyLimit", data.at(0) & 0x08 ? true : false);
            break;
        }

        case 0x000E:
        {
            quint32 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            map.insert("energy", static_cast <double> (qFromLittleEndian(value)) / 1000);
            break;
        }

        default:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());

            switch (attributeId)
            {
                case 0x0003: map.insert("voltage", qFromLittleEndian(value)); break;
                case 0x0004: map.insert("voltageMin", qFromLittleEndian(value)); break;
                case 0x0005: map.insert("voltageMax", qFromLittleEndian(value)); break;
                case 0x000A: map.insert("power", qFromLittleEndian(value)); break;
                case 0x000B: map.insert("powerMax", qFromLittleEndian(value)); break;
                case 0x000F: map.insert("energyLimit", qFromLittleEndian(value)); break;
            }

            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}
