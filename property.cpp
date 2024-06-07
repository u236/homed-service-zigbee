#include "properties/common.h"
#include "properties/efekta.h"
#include "properties/ias.h"
#include "properties/lumi.h"
#include "properties/other.h"
#include "properties/ptvo.h"
#include "properties/tuya.h"

void PropertyObject::registerMetaTypes(void)
{
    qRegisterMetaType <Properties::BatteryVoltage>                  ("batteryVoltageProperty");
    qRegisterMetaType <Properties::BatteryPercentage>               ("batteryPercentageProperty");
    qRegisterMetaType <Properties::DeviceTemperature>               ("deviceTemperatureProperty");
    qRegisterMetaType <Properties::Status>                          ("statusProperty");
    qRegisterMetaType <Properties::Level>                           ("levelProperty");
    qRegisterMetaType <Properties::AnalogInput>                     ("analogInputProperty");
    qRegisterMetaType <Properties::AnalogOutput>                    ("analogOutputProperty");
    qRegisterMetaType <Properties::CoverPosition>                   ("coverPositionProperty");
    qRegisterMetaType <Properties::CoverTilt>                       ("coverTiltProperty");
    qRegisterMetaType <Properties::Thermostat>                      ("thermostatProperty");
    qRegisterMetaType <Properties::ColorHS>                         ("colorHSProperty");
    qRegisterMetaType <Properties::ColorXY>                         ("colorXYProperty");
    qRegisterMetaType <Properties::ColorTemperature>                ("colorTemperatureProperty");
    qRegisterMetaType <Properties::ColorMode>                       ("colorModeProperty");
    qRegisterMetaType <Properties::Illuminance>                     ("illuminanceProperty");
    qRegisterMetaType <Properties::Temperature>                     ("temperatureProperty");
    qRegisterMetaType <Properties::Pressure>                        ("pressureProperty");
    qRegisterMetaType <Properties::Humidity>                        ("humidityProperty");
    qRegisterMetaType <Properties::Occupancy>                       ("occupancyProperty");
    qRegisterMetaType <Properties::OccupancyTimeout>                ("occupancyTimeoutProperty");
    qRegisterMetaType <Properties::Moisture>                        ("moistureProperty");
    qRegisterMetaType <Properties::CO2>                             ("co2Property");
    qRegisterMetaType <Properties::PM25>                            ("pm25Property");
    qRegisterMetaType <Properties::Energy>                          ("energyProperty");
    qRegisterMetaType <Properties::Voltage>                         ("voltageProperty");
    qRegisterMetaType <Properties::Current>                         ("currentProperty");
    qRegisterMetaType <Properties::Power>                           ("powerProperty");
    qRegisterMetaType <Properties::ChildLock>                       ("childLockProperty");
    qRegisterMetaType <Properties::Scene>                           ("sceneProperty");
    qRegisterMetaType <Properties::StatusAction>                    ("statusActionProperty");
    qRegisterMetaType <Properties::LevelAction>                     ("levelActionProperty");
    qRegisterMetaType <Properties::CoverAction>                     ("coverActionProperty");
    qRegisterMetaType <Properties::ColorAction>                     ("colorActionProperty");
    qRegisterMetaType <Properties::PowerOnStatus>                   ("powerOnStatusProperty");
    qRegisterMetaType <Properties::SwitchType>                      ("switchTypeProperty");
    qRegisterMetaType <Properties::SwitchMode>                      ("switchModeProperty");
    qRegisterMetaType <Properties::FanMode>                         ("fanModeProperty");
    qRegisterMetaType <Properties::DisplayMode>                     ("displayModeProperty");

    qRegisterMetaType <PropertiesIAS::Warning>                      ("iasWarningProperty");
    qRegisterMetaType <PropertiesIAS::Contact>                      ("iasContactProperty");
    qRegisterMetaType <PropertiesIAS::Gas>                          ("iasGasProperty");
    qRegisterMetaType <PropertiesIAS::Occupancy>                    ("iasOccupancyProperty");
    qRegisterMetaType <PropertiesIAS::Smoke>                        ("iasSmokeProperty");
    qRegisterMetaType <PropertiesIAS::WaterLeak>                    ("iasWaterLeakProperty");

    qRegisterMetaType <PropertiesLUMI::Data>                        ("lumiDataProperty");
    qRegisterMetaType <PropertiesLUMI::Basic>                       ("lumiBasicProperty");
    qRegisterMetaType <PropertiesLUMI::ButtonMode>                  ("lumiButtonModeProperty");
    qRegisterMetaType <PropertiesLUMI::Contact>                     ("lumiContactProperty");
    qRegisterMetaType <PropertiesLUMI::Power>                       ("lumiPowerProperty");
    qRegisterMetaType <PropertiesLUMI::Cover>                       ("lumiCoverProperty");
    qRegisterMetaType <PropertiesLUMI::ButtonAction>                ("lumiButtonActionProperty");
    qRegisterMetaType <PropertiesLUMI::SwitchAction>                ("lumiSwitchActionProperty");
    qRegisterMetaType <PropertiesLUMI::CubeRotation>                ("lumiCubeRotationProperty");
    qRegisterMetaType <PropertiesLUMI::CubeMovement>                ("lumiCubeMovementProperty");
    qRegisterMetaType <PropertiesLUMI::Vibration>                   ("lumiVibrationProperty");

    qRegisterMetaType <PropertiesTUYA::DataPoints>                  ("tuyaDataPointsProperty");
    qRegisterMetaType <PropertiesTUYA::HolidayThermostatProgram>    ("tuyaHolidayThermostatProgramProperty");
    qRegisterMetaType <PropertiesTUYA::DailyThermostatProgram>      ("tuyaDailyThermostatProgramProperty");
    qRegisterMetaType <PropertiesTUYA::MoesThermostatProgram>       ("tuyaMoesThermostatProgramProperty");
    qRegisterMetaType <PropertiesTUYA::CoverMotor>                  ("tuyaCoverMotorProperty");
    qRegisterMetaType <PropertiesTUYA::CoverSwitch>                 ("tuyaCoverSwitchProperty");
    qRegisterMetaType <PropertiesTUYA::ChildLock>                   ("tuyaChildLockProperty");
    qRegisterMetaType <PropertiesTUYA::ButtonAction>                ("tuyaButtonActionProperty");
    qRegisterMetaType <PropertiesTUYA::OperationMode>               ("tuyaOperationModeProperty");
    qRegisterMetaType <PropertiesTUYA::IndicatorMode>               ("tuyaIndicatorModeProperty");
    qRegisterMetaType <PropertiesTUYA::SwitchType>                  ("tuyaSwitchTypeProperty");
    qRegisterMetaType <PropertiesTUYA::PowerOnStatus>               ("tuyaPowerOnStatusProperty");

    qRegisterMetaType <PropertiesEfekta::ReportingDelay>            ("efektaReportingDelayProperty");
    qRegisterMetaType <PropertiesEfekta::TemperatureSettings>       ("efektaTemperatureSettingsProperty");
    qRegisterMetaType <PropertiesEfekta::HumiditySettings>          ("efektaHumiditySettingsProperty");
    qRegisterMetaType <PropertiesEfekta::CO2Settings>               ("efektaCO2SettingsProperty");
    qRegisterMetaType <PropertiesEfekta::PMSensor>                  ("efektaPMSensorProperty");
    qRegisterMetaType <PropertiesEfekta::VOCSensor>                 ("efektaVOCSensorProperty");

    qRegisterMetaType <PropertiesPTVO::ChangePattern>               ("ptvoChangePatternProperty");
    qRegisterMetaType <PropertiesPTVO::Contact>                     ("ptvoContactProperty");
    qRegisterMetaType <PropertiesPTVO::Occupancy>                   ("ptvoOccupancyProperty");
    qRegisterMetaType <PropertiesPTVO::WaterLeak>                   ("ptvoWaterLeakProperty");
    qRegisterMetaType <PropertiesPTVO::CO2>                         ("ptvoCO2Property");
    qRegisterMetaType <PropertiesPTVO::Temperature>                 ("ptvoTemperatureProperty");
    qRegisterMetaType <PropertiesPTVO::Humidity>                    ("ptvoHumidityProperty");
    qRegisterMetaType <PropertiesPTVO::Count>                       ("ptvoCountProperty");
    qRegisterMetaType <PropertiesPTVO::Pattern>                     ("ptvoPatternProperty");
    qRegisterMetaType <PropertiesPTVO::SwitchAction>                ("ptvoSwitchActionProperty");
    qRegisterMetaType <PropertiesPTVO::SerialData>                  ("ptvoSerialDataProperty");
    qRegisterMetaType <PropertiesPTVO::ButtonAction>                ("ptvoButtonActionProperty");

    qRegisterMetaType <PropertiesByun::GasSensor>                   ("byunGasSensorProperty");
    qRegisterMetaType <PropertiesByun::SmokeSensor>                 ("byunSmokeSensorProperty");

    qRegisterMetaType <PropertiesIKEA::Occupancy>                   ("ikeaOccupancyProperty");
    qRegisterMetaType <PropertiesIKEA::StatusAction>                ("ikeaStatusActionProperty");
    qRegisterMetaType <PropertiesIKEA::ArrowAction>                 ("ikeaArrowActionProperty");
}

quint8 PropertyObject::percentage(double min, double max, double value)
{
    if (value < min)
        value = min;

    if (value > max)
        value = max;

    return static_cast <quint8> ((value - min) / (max - min) * 100);
}

QVariant PropertyObject::enumValue(const QString &name, int index)
{
    QVariant data = option(name).toMap().value("enum");

    switch (data.type())
    {
        case QVariant::Map: return data.toMap().value(QString::number(index));
        case QVariant::List: return data.toList().value(index);
        default: return QVariant();
    }
}

void EnumProperty::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributeId)
        return;

    m_value = enumValue(m_name, static_cast <quint8> (data.at(0)));
}
