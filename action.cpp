#include "actions/common.h"
#include "actions/efekta.h"
#include "actions/ias.h"
#include "actions/lumi.h"
#include "actions/other.h"
#include "actions/ptvo.h"
#include "actions/tuya.h"
#include "device.h"

void ActionObject::registerMetaTypes(void)
{
    qRegisterMetaType <Actions::Status>                         ("statusAction");
    qRegisterMetaType <Actions::Level>                          ("levelAction");
    qRegisterMetaType <Actions::AnalogOutput>                   ("analogOutputAction");
    qRegisterMetaType <Actions::CoverStatus>                    ("coverStatusAction");
    qRegisterMetaType <Actions::CoverPosition>                  ("coverPositionAction");
    qRegisterMetaType <Actions::Thermostat>                     ("thermostatAction");
    qRegisterMetaType <Actions::ThermostatProgram>              ("thermostatProgramAction");
    qRegisterMetaType <Actions::ColorHS>                        ("colorHSAction");
    qRegisterMetaType <Actions::ColorXY>                        ("colorXYAction");
    qRegisterMetaType <Actions::ColorTemperature>               ("colorTemperatureAction");
    qRegisterMetaType <Actions::OccupancyTimeout>               ("occupancyTimeoutAction");
    qRegisterMetaType <Actions::ChildLock>                      ("childLockAction");
    qRegisterMetaType <Actions::PowerOnStatus>                  ("powerOnStatusAction");
    qRegisterMetaType <Actions::SwitchType>                     ("switchTypeAction");
    qRegisterMetaType <Actions::SwitchMode>                     ("switchModeAction");
    qRegisterMetaType <Actions::FanMode>                        ("fanModeAction");
    qRegisterMetaType <Actions::DisplayMode>                    ("displayModeAction");

    qRegisterMetaType <ActionsIAS::Warning>                     ("iasWarningAction");

    qRegisterMetaType <ActionsLUMI::PresenceSensor>             ("lumiPresenceSensorAction");
    qRegisterMetaType <ActionsLUMI::RadiatorThermostat>         ("lumiRadiatorThermostatAction");
    qRegisterMetaType <ActionsLUMI::ElectricThermostat>         ("lumiElectricThermostatAction");
    qRegisterMetaType <ActionsLUMI::ThermostatProgram>          ("lumiThermostatProgramAction");
    qRegisterMetaType <ActionsLUMI::ButtonMode>                 ("lumiButtonModeAction");
    qRegisterMetaType <ActionsLUMI::SwitchStatusMemory>         ("lumiSwitchStatusMemoryAction");
    qRegisterMetaType <ActionsLUMI::LightStatusMemory>          ("lumiLightStatusMemoryAction");
    qRegisterMetaType <ActionsLUMI::BasicStatusMemory>          ("lumiBasicStatusMemoryAction");
    qRegisterMetaType <ActionsLUMI::CoverStatus>                ("lumiCoverStatusAction");
    qRegisterMetaType <ActionsLUMI::CoverStop>                  ("lumiCoverStopAction");
    qRegisterMetaType <ActionsLUMI::CoverPosition>              ("lumiCoverPositionAction");
    qRegisterMetaType <ActionsLUMI::VibrationSensitivity>       ("lumiVibrationSensitivityAction");
    qRegisterMetaType <ActionsLUMI::OperationMode>              ("lumiOperationModeAction");
    qRegisterMetaType <ActionsLUMI::IndicatorMode>              ("lumiIndicatorModeAction");
    qRegisterMetaType <ActionsLUMI::SwitchMode>                 ("lumiSwitchModeAction");
    qRegisterMetaType <ActionsLUMI::Language>                   ("lumiLanguageAction");

    qRegisterMetaType <ActionsTUYA::DataPoints>                 ("tuyaDataPointsAction");
    qRegisterMetaType <ActionsTUYA::HolidayThermostatProgram>   ("tuyaHolidayThermostatProgramAction");
    qRegisterMetaType <ActionsTUYA::DailyThermostatProgram>     ("tuyaDailyThermostatProgramAction");
    qRegisterMetaType <ActionsTUYA::MoesThermostatProgram>      ("tuyaMoesThermostatProgramAction");
    qRegisterMetaType <ActionsTUYA::LedController>              ("tuyaLedControllerAction");
    qRegisterMetaType <ActionsTUYA::CoverMotor>                 ("tuyaCoverMotorAction");
    qRegisterMetaType <ActionsTUYA::CoverSwitch>                ("tuyaCoverSwitchAction");
    qRegisterMetaType <ActionsTUYA::ChildLock>                  ("tuyaChildLockAction");
    qRegisterMetaType <ActionsTUYA::IRCode>                     ("tuyaIRCodeAction");
    qRegisterMetaType <ActionsTUYA::IRLearn>                    ("tuyaIRLearnAction");
    qRegisterMetaType <ActionsTUYA::OperationMode>              ("tuyaOperationModeAction");
    qRegisterMetaType <ActionsTUYA::IndicatorMode>              ("tuyaIndicatorModeAction");
    qRegisterMetaType <ActionsTUYA::SwitchType>                 ("tuyaSwitchTypeAction");
    qRegisterMetaType <ActionsTUYA::PowerOnStatus>              ("tuyaPowerOnStatusAction");

    qRegisterMetaType <ActionsEfekta::ReadInterval>             ("efektaReadIntervalAction");
    qRegisterMetaType <ActionsEfekta::TemperatureSettings>      ("efektaTemperatureSettingsAction");
    qRegisterMetaType <ActionsEfekta::HumiditySettings>         ("efektaHumiditySettingsAction");
    qRegisterMetaType <ActionsEfekta::CO2Settings>              ("efektaCO2SettingsAction");
    qRegisterMetaType <ActionsEfekta::PMSensor>                 ("efektaPMSensorAction");
    qRegisterMetaType <ActionsEfekta::VOCSensor>                ("efektaVOCSensorAction");

    qRegisterMetaType <ActionsPTVO::ChangePattern>              ("ptvoChangePatternAction");
    qRegisterMetaType <ActionsPTVO::Count>                      ("ptvoCountAction");
    qRegisterMetaType <ActionsPTVO::Pattern>                    ("ptvoPatternAction");
    qRegisterMetaType <ActionsPTVO::SerialData>                 ("ptvoSerialDataAction");

    qRegisterMetaType <ActionsYandex::CommonSettings>           ("yandexCommonSettingsAction");
    qRegisterMetaType <ActionsYandex::SwitchSettings>           ("yandexSwitchSettingsAction");
}

QByteArray ActionObject::ieeeAddress(void)
{
    return static_cast <EndpointObject*> (m_parent)->device()->ieeeAddress();
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

    return Property(new PropertyObject(propertyName));
}

QByteArray ActionObject::writeAttribute(quint16 attributeId, quint8 dataType, void *value, size_t length)
{
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, attributeId, dataType, QByteArray(reinterpret_cast <char*> (value), length));
}

QByteArray ActionObject::writeAttribute(quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, attributeId, dataType, data);
}

QByteArray ActionObject::writeAttribute(quint8 dataType, void *value, size_t length)
{
    return writeAttribute(m_attributes.at(0), dataType, value, length);
}

QByteArray ActionObject::writeAttribute(quint8 dataType, const QByteArray &data)
{
    return writeAttribute(m_attributes.at(0), dataType, data);
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

QVariant EnumAction::request(const QString &, const QVariant &data)
{
    int index = enumIndex(m_name, data);
    quint8 value = static_cast <quint8> (index);
    return index < 0 ? QByteArray() : writeAttribute(m_dataType, &value, sizeof(value));
}
