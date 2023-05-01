#include "actions/common.h"
#include "actions/efekta.h"
#include "actions/lumi.h"
#include "actions/other.h"
#include "actions/ptvo.h"
#include "actions/tuya.h"
#include "device.h"

void ActionObject::registerMetaTypes(void)
{
    qRegisterMetaType <Actions::Status>                         ("statusAction");
    qRegisterMetaType <Actions::PowerOnStatus>                  ("powerOnStatusAction");
    qRegisterMetaType <Actions::Level>                          ("levelAction");
    qRegisterMetaType <Actions::CoverStatus>                    ("coverStatusAction");
    qRegisterMetaType <Actions::CoverPosition>                  ("coverPositionAction");
    qRegisterMetaType <Actions::CoverTilt>                      ("coverTiltAction");
    qRegisterMetaType <Actions::ColorHS>                        ("colorHSAction");
    qRegisterMetaType <Actions::ColorXY>                        ("colorXYAction");
    qRegisterMetaType <Actions::ColorTemperature>               ("colorTemperatureAction");

    qRegisterMetaType <ActionsLUMI::PresenceSensor>             ("lumiPresenceSensorAction");
    qRegisterMetaType <ActionsLUMI::ButtonMode>                 ("lumiButtonModeAction");
    qRegisterMetaType <ActionsLUMI::OperationMode>              ("lumiOperationModeAction");
    qRegisterMetaType <ActionsLUMI::IndicatorMode>              ("lumiIndicatorModeAction");
    qRegisterMetaType <ActionsLUMI::SwitchMode>                 ("lumiSwitchModeAction");
    qRegisterMetaType <ActionsLUMI::StatusMemory>               ("lumiStatusMemoryAction");
    qRegisterMetaType <ActionsLUMI::Interlock>                  ("lumiInterlockAction");
    qRegisterMetaType <ActionsLUMI::CoverPosition>              ("lumiCoverPositionAction");

    qRegisterMetaType <ActionsTUYA::LightDimmer>                ("tuyaLightDimmerAction");
    qRegisterMetaType <ActionsTUYA::ElectricityMeter>           ("tuyaElectricityMeterAction");
    qRegisterMetaType <ActionsTUYA::MoesElectricThermostat>     ("tuyaMoesElectricThermostatAction");
    qRegisterMetaType <ActionsTUYA::MoesRadiatorThermostat>     ("tuyaMoesRadiatorThermostatAction");
    qRegisterMetaType <ActionsTUYA::MoesThermostatProgram>      ("tuyaMoesThermostatProgramAction");
    qRegisterMetaType <ActionsTUYA::NeoSiren>                   ("tuyaNeoSirenAction");
    qRegisterMetaType <ActionsTUYA::WaterValve>                 ("tuyaWaterValveAction");
    qRegisterMetaType <ActionsTUYA::PresenceSensor>             ("tuyaPresenceSensorAction");
    qRegisterMetaType <ActionsTUYA::RadarSensor>                ("tuyaRadarSensorAction");
    qRegisterMetaType <ActionsTUYA::CoverMotor>                 ("tuyaCoverMotorAction");
    qRegisterMetaType <ActionsTUYA::CoverSwitch>                ("tuyaCoverSwitchAction");
    qRegisterMetaType <ActionsTUYA::ChildLock>                  ("tuyaChildLockAction");
    qRegisterMetaType <ActionsTUYA::OperationMode>              ("tuyaOperationModeAction");
    qRegisterMetaType <ActionsTUYA::IndicatorMode>              ("tuyaIndicatorModeAction");
    qRegisterMetaType <ActionsTUYA::SwitchType>                 ("tuyaSwitchTypeAction");
    qRegisterMetaType <ActionsTUYA::PowerOnStatus>              ("tuyaPowerOnStatusAction");

    qRegisterMetaType <ActionsEfekta::ReportingDelay>           ("efektaReportingDelayAction");
    qRegisterMetaType <ActionsEfekta::TemperatureSettings>      ("efektaTemperatureSettingsAction");
    qRegisterMetaType <ActionsEfekta::HumiditySettings>         ("efektaHumiditySettingsAction");
    qRegisterMetaType <ActionsEfekta::CO2Sensor>                ("efektaCO2SensorAction");
    qRegisterMetaType <ActionsEfekta::VOCSensor>                ("efektaVOCSensorAction");

    qRegisterMetaType <ActionsPTVO::ChangePattern>              ("ptvoChangePatternAction");
    qRegisterMetaType <ActionsPTVO::Count>                      ("ptvoCountAction");
    qRegisterMetaType <ActionsPTVO::Pattern>                    ("ptvoPatternAction");

    qRegisterMetaType <ActionsOther::PerenioSmartPlug>          ("perenioSmartPlugAction");
}

Property ActionObject::endpointProperty(const QString &name)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
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
