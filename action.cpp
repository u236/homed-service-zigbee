#include "actions/common.h"
#include "actions/efekta.h"
#include "actions/lumi.h"
#include "actions/modkam.h"
#include "actions/other.h"
#include "actions/ptvo.h"
#include "actions/tuya.h"
#include "device.h"

void ActionObject::registerMetaTypes(void)
{
    qRegisterMetaType <Actions::Status>                         ("statusAction");
    qRegisterMetaType <Actions::PowerOnStatus>                  ("powerOnStatusAction");
    qRegisterMetaType <Actions::SwitchType>                     ("switchTypeAction");
    qRegisterMetaType <Actions::SwitchMode>                     ("switchModeAction");
    qRegisterMetaType <Actions::Level>                          ("levelAction");
    qRegisterMetaType <Actions::CoverStatus>                    ("coverStatusAction");
    qRegisterMetaType <Actions::CoverPosition>                  ("coverPositionAction");
    qRegisterMetaType <Actions::CoverTilt>                      ("coverTiltAction");
    qRegisterMetaType <Actions::ColorHS>                        ("colorHSAction");
    qRegisterMetaType <Actions::ColorXY>                        ("colorXYAction");
    qRegisterMetaType <Actions::ColorTemperature>               ("colorTemperatureAction");
    qRegisterMetaType <Actions::Thermostat>                     ("thermostatAction");

    qRegisterMetaType <ActionsLUMI::PresenceSensor>             ("lumiPresenceSensorAction");
    qRegisterMetaType <ActionsLUMI::ButtonMode>                 ("lumiButtonModeAction");
    qRegisterMetaType <ActionsLUMI::OperationMode>              ("lumiOperationModeAction");
    qRegisterMetaType <ActionsLUMI::IndicatorMode>              ("lumiIndicatorModeAction");
    qRegisterMetaType <ActionsLUMI::SwitchMode>                 ("lumiSwitchModeAction");
    qRegisterMetaType <ActionsLUMI::SwitchType>                 ("lumiSwitchTypeAction");
    qRegisterMetaType <ActionsLUMI::StatusMemory>               ("lumiStatusMemoryAction");
    qRegisterMetaType <ActionsLUMI::Interlock>                  ("lumiInterlockAction");
    qRegisterMetaType <ActionsLUMI::CoverPosition>              ("lumiCoverPositionAction");
    qRegisterMetaType <ActionsLUMI::VibrationSensitivity>       ("lumiVibrationSensitivityAction");
    qRegisterMetaType <ActionsLUMI::SystemMode>                 ("lumiSystemModeAction");
    qRegisterMetaType <ActionsLUMI::ThermostatMode>             ("lumiThermostatModeAction");
    qRegisterMetaType <ActionsLUMI::WindowDetection>            ("lumiWindowDetectionAction");
    qRegisterMetaType <ActionsLUMI::ChildLock>                  ("lumiChildLockAction");
    qRegisterMetaType <ActionsLUMI::AwayTemperature>            ("lumiAwayTemperatureAction");

    qRegisterMetaType <ActionsTUYA::DataPoints>                 ("tuyaDataPointsAction");
    qRegisterMetaType <ActionsTUYA::WeekdayThermostatProgram>   ("tuyaWeekdayThermostatProgramAction");
    qRegisterMetaType <ActionsTUYA::HolidayThermostatProgram>   ("tuyaHolidayThermostatProgramAction");
    qRegisterMetaType <ActionsTUYA::MoesThermostatProgram>      ("tuyaMoesThermostatProgramAction");
    qRegisterMetaType <ActionsTUYA::CoverMotor>                 ("tuyaCoverMotorAction");
    qRegisterMetaType <ActionsTUYA::CoverSwitch>                ("tuyaCoverSwitchAction");
    qRegisterMetaType <ActionsTUYA::ChildLock>                  ("tuyaChildLockAction");
    qRegisterMetaType <ActionsTUYA::OperationMode>              ("tuyaOperationModeAction");
    qRegisterMetaType <ActionsTUYA::IndicatorMode>              ("tuyaIndicatorModeAction");
    qRegisterMetaType <ActionsTUYA::SensitivityMode>            ("tuyaSensitivityModeAction");
    qRegisterMetaType <ActionsTUYA::TimeoutMode>                ("tuyaTimeoutModeAction");
    qRegisterMetaType <ActionsTUYA::SwitchType>                 ("tuyaSwitchTypeAction");
    qRegisterMetaType <ActionsTUYA::PowerOnStatus>              ("tuyaPowerOnStatusAction");

    qRegisterMetaType <ActionsEfekta::ReportingDelay>           ("efektaReportingDelayAction");
    qRegisterMetaType <ActionsEfekta::TemperatureSettings>      ("efektaTemperatureSettingsAction");
    qRegisterMetaType <ActionsEfekta::HumiditySettings>         ("efektaHumiditySettingsAction");
    qRegisterMetaType <ActionsEfekta::CO2Settings>              ("efektaCO2SettingsAction");
    qRegisterMetaType <ActionsEfekta::VOCSensor>                ("efektaVOCSensorAction");

    qRegisterMetaType <ActionsModkam::TemperatureOffset>        ("modkamTemperatureOffsetAction");
    qRegisterMetaType <ActionsModkam::HumidityOffset>           ("modkamHumidityOffsetAction");
    qRegisterMetaType <ActionsModkam::PressureOffset>           ("modkamPressureOffsetAction");
    qRegisterMetaType <ActionsModkam::CO2Settings>              ("modkamCO2SettingsAction");
    qRegisterMetaType <ActionsModkam::GeigerSettings>           ("modkamGeigerSettingsAction");

    qRegisterMetaType <ActionsPTVO::ChangePattern>              ("ptvoChangePatternAction");
    qRegisterMetaType <ActionsPTVO::Count>                      ("ptvoCountAction");
    qRegisterMetaType <ActionsPTVO::Pattern>                    ("ptvoPatternAction");
    qRegisterMetaType <ActionsPTVO::SerialData>                 ("ptvoSerialDataAction");

    qRegisterMetaType <ActionsOther::PerenioSmartPlug>          ("perenioSmartPlugAction");
}

Property ActionObject::endpointProperty(const QString &name)
{
    EndpointObject *endpoint = static_cast <EndpointObject*> (m_parent);
    QString propertyName = name.isEmpty() ? m_name : name;

    if (endpoint)
    {
        for (int i = 0; i < endpoint->properties().count(); i++)
        {
            const Property &property = endpoint->properties().at(i);

            if (property->name() != propertyName)
                continue;

            return property;
        }
    }

    return Property();
}
