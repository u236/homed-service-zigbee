#ifndef ACTIONS_LUMI_H
#define ACTIONS_LUMI_H

#include "action.h"
#include "zcl.h"

namespace ActionsLUMI
{
    class PresenceSensor : public ActionObject
    {

    public:

        PresenceSensor(void) : ActionObject("presenceSensor", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, QList <QString> {"sensitivityMode", "detectionMode", "distanceMode", "resetPresence"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class Thermostat : public ActionObject
    {

    public:

        Thermostat(void) : ActionObject("thermostat", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, QList <QString> {"targetTemperature", "systemMode", "fanMode"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ButtonMode : public ActionObject
    {

    public:

        ButtonMode(void) : ActionObject("buttonMode", CLUSTER_BASIC, MANUFACTURER_CODE_LUMI, QList <QString> {"buttonMode", "leftMode", "rightMode"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class SwitchStatusMemory : public ActionObject
    {

    public:

        SwitchStatusMemory(void) : ActionObject("statusMemory", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0201) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class LightStatusMemory : public ActionObject
    {

    public:

        LightStatusMemory(void) : ActionObject("statusMemory", CLUSTER_BASIC, MANUFACTURER_CODE_LUMI, 0xFF19) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class CoverPosition : public ActionObject
    {

    public:

        CoverPosition(void) : ActionObject("position", CLUSTER_ANALOG_OUTPUT, 0x0000, 0x0055) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class VibrationSensitivity : public ActionObject
    {

    public:

        VibrationSensitivity(void) : ActionObject("sensitivityMode", CLUSTER_BASIC, MANUFACTURER_CODE_LUMI, 0xFF0D) {}
        QByteArray request(const QString &name, const QVariant &data) override;

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
