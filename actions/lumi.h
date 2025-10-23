#ifndef ACTIONS_LUMI_H
#define ACTIONS_LUMI_H

#include "action.h"
#include "zcl.h"

namespace ActionsLUMI
{
    class PresenceSensor : public ActionObject
    {

    public:

        PresenceSensor(void) : ActionObject("presenceSensor", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, {0x010C, 0x0144, 0x0146}, {"sensitivityMode", "detectionMode", "distanceMode", "resetPresence"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class RadiatorThermostat : public ActionObject
    {

    public:

        RadiatorThermostat(void) : ActionObject("thermostat", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x027E, {"sensorType", "externalTemperature"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    private:

        QByteArray header(quint8 length, quint8 counter, quint8 action);

    };

    class ElectricThermostat : public ActionObject
    {

    public:

        ElectricThermostat(void) : ActionObject("thermostat", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x024F, {"targetTemperature", "systemMode", "fanMode"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class ThermostatProgram : public ActionObject
    {

    public:

        ThermostatProgram(void) : ActionObject("thermostatProgram", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0276, {"scheduleMonday", "scheduleTuesday", "scheduleWednesday", "scheduleThursday", "scheduleFriday", "scheduleSaturday", "scheduleSunday"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

    };

    class ButtonMode : public ActionObject
    {

    public:

        ButtonMode(void) : ActionObject("buttonMode", CLUSTER_BASIC, MANUFACTURER_CODE_LUMI, {0xFF22, 0xFF23}, {"buttonMode", "leftMode", "rightMode"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class SwitchStatusMemory : public ActionObject
    {

    public:

        SwitchStatusMemory(void) : ActionObject("statusMemory", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0201) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class LightStatusMemory : public ActionObject
    {

    public:

        LightStatusMemory(void) : ActionObject("statusMemory", CLUSTER_BASIC, MANUFACTURER_CODE_LUMI, 0xFF19) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class BasicStatusMemory : public ActionObject
    {

    public:

        BasicStatusMemory(void) : ActionObject("statusMemory", CLUSTER_BASIC, MANUFACTURER_CODE_LUMI, 0xFFF0) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class CoverStatus : public ActionObject
    {

    public:

        CoverStatus(void) : ActionObject("cover", CLUSTER_ANALOG_OUTPUT, 0x0000, 0x0055) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class CoverStop : public ActionObject
    {

    public:

        CoverStop(void) : ActionObject("cover", CLUSTER_MULTISTATE_OUTPUT, 0x0000, 0x0055) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class CoverPosition : public ActionObject
    {

    public:

        CoverPosition(void) : ActionObject("position", CLUSTER_ANALOG_OUTPUT, 0x0000, 0x0055) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class VibrationSensitivity : public ActionObject
    {

    public:

        VibrationSensitivity(void) : ActionObject("sensitivityMode", CLUSTER_BASIC, MANUFACTURER_CODE_LUMI, 0xFF0D) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class OperationMode : public EnumAction
    {

    public:

        OperationMode(void) : EnumAction("operationMode", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0009, DATA_TYPE_8BIT_UNSIGNED) {}

    };

    class IndicatorMode : public EnumAction
    {

    public:

        IndicatorMode(void) : EnumAction("indicatorMode", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x00F0, DATA_TYPE_8BIT_UNSIGNED) {}

    };

    class SwitchMode : public EnumAction
    {

    public:

        SwitchMode(void) : EnumAction("switchMode", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0200, DATA_TYPE_8BIT_UNSIGNED) {}

    };

    class Language : public EnumAction
    {

    public:

        Language(void) : EnumAction("language", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0210, DATA_TYPE_8BIT_UNSIGNED) {}

    };

}

#endif
