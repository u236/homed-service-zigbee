#include "actions/common.h"
#include "actions/efekta.h"
#include "actions/ias.h"
#include "actions/lumi.h"
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
    qRegisterMetaType <Actions::AnalogOutput>                   ("analogOutputAction");
    qRegisterMetaType <Actions::CoverStatus>                    ("coverStatusAction");
    qRegisterMetaType <Actions::CoverPosition>                  ("coverPositionAction");
    qRegisterMetaType <Actions::CoverTilt>                      ("coverTiltAction");
    qRegisterMetaType <Actions::Thermostat>                     ("thermostatAction");
    qRegisterMetaType <Actions::FanMode>                        ("fanModeAction");
    qRegisterMetaType <Actions::DisplayMode>                    ("displayModeAction");
    qRegisterMetaType <Actions::ColorHS>                        ("colorHSAction");
    qRegisterMetaType <Actions::ColorXY>                        ("colorXYAction");
    qRegisterMetaType <Actions::ColorTemperature>               ("colorTemperatureAction");
    qRegisterMetaType <Actions::OccupancyTimeout>               ("occupancyTimeoutAction");

    qRegisterMetaType <ActionsIAS::Warning>                     ("iasWarningAction");

    qRegisterMetaType <ActionsLUMI::PresenceSensor>             ("lumiPresenceSensorAction");
    qRegisterMetaType <ActionsLUMI::ButtonMode>                 ("lumiButtonModeAction");
    qRegisterMetaType <ActionsLUMI::OperationMode>              ("lumiOperationModeAction");
    qRegisterMetaType <ActionsLUMI::IndicatorMode>              ("lumiIndicatorModeAction");
    qRegisterMetaType <ActionsLUMI::SwitchMode>                 ("lumiSwitchModeAction");
    qRegisterMetaType <ActionsLUMI::SwitchStatusMemory>         ("lumiSwitchStatusMemoryAction");
    qRegisterMetaType <ActionsLUMI::LightStatusMemory>          ("lumiLightStatusMemoryAction");
    qRegisterMetaType <ActionsLUMI::CoverPosition>              ("lumiCoverPositionAction");
    qRegisterMetaType <ActionsLUMI::VibrationSensitivity>       ("lumiVibrationSensitivityAction");

    qRegisterMetaType <ActionsTUYA::DataPoints>                 ("tuyaDataPointsAction");
    qRegisterMetaType <ActionsTUYA::HolidayThermostatProgram>   ("tuyaHolidayThermostatProgramAction");
    qRegisterMetaType <ActionsTUYA::DailyThermostatProgram>     ("tuyaDailyThermostatProgramAction");
    qRegisterMetaType <ActionsTUYA::MoesThermostatProgram>      ("tuyaMoesThermostatProgramAction");
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
    qRegisterMetaType <ActionsEfekta::CO2Settings>              ("efektaCO2SettingsAction");
    qRegisterMetaType <ActionsEfekta::PMSensor>                 ("efektaPMSensorAction");
    qRegisterMetaType <ActionsEfekta::VOCSensor>                ("efektaVOCSensorAction");

    qRegisterMetaType <ActionsPTVO::ChangePattern>              ("ptvoChangePatternAction");
    qRegisterMetaType <ActionsPTVO::Count>                      ("ptvoCountAction");
    qRegisterMetaType <ActionsPTVO::Pattern>                    ("ptvoPatternAction");
    qRegisterMetaType <ActionsPTVO::SerialData>                 ("ptvoSerialDataAction");
}

Property ActionObject::endpointProperty(const QString &name)
{
    EndpointObject *endpoint = static_cast <EndpointObject*> (m_parent);
    QString propertyName = name.isEmpty() ? m_name : name;

    for (int i = 0; i < endpoint->properties().count(); i++)
    {
        const Property &property = endpoint->properties().at(i);

        if (property->name() != propertyName)
            continue;

        return property;
    }

    return Property();
}

QByteArray ActionObject::writeAttribute(quint8 dataType, void *value, size_t length)
{
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), dataType, QByteArray(reinterpret_cast <char*> (value), length));
}

qint8 ActionObject::listIndex(const QList <QString> &list, const QVariant &value)
{
    return static_cast <qint8> (list.indexOf(value.toString()));
}

int ActionObject::enumIndex(const QString name, const QVariant &value)
{
    QVariant data = option(name).toMap().value("enum");

    switch (data.type())
    {
        case QVariant::Map:
        {
            QMap <QString, QVariant> map = data.toMap();

            for (auto it = map.begin(); it != map.end(); it++)
                if (it.value() == value)
                    return it.key().toInt();

            break;
        }

        case QVariant::List: return data.toList().indexOf(value.toString());
        default: break;
    }

    return -1;
}

QByteArray EnumAction::request(const QString &, const QVariant &data)
{
    int index = enumIndex(m_name, data);
    quint8 value = static_cast <quint8> (index);
    return index < 0 ? QByteArray() : writeAttribute(m_dataType, &value, sizeof(value));
}
