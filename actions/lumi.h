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

    class ButtonMode : public ActionObject
    {

    public:

        ButtonMode(void) : ActionObject("buttonMode", CLUSTER_BASIC, MANUFACTURER_CODE_LUMI, QList <QString> {"buttonMode", "leftMode", "rightMode"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class OperationMode : public ActionObject
    {

    public:

        OperationMode(void) : ActionObject("operationMode", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0009) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class IndicatorMode : public ActionObject
    {

    public:

        IndicatorMode(void) : ActionObject("indicatorMode", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x00F0) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class SwitchMode : public ActionObject
    {

    public:

        SwitchMode(void) : ActionObject("switchMode", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0200) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class SwitchType : public ActionObject
    {

    public:

        SwitchType(void) : ActionObject("switchType", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x000A) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class StatusMemory : public ActionObject
    {

    public:

        StatusMemory(void) : ActionObject("statusMemory", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0201) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class Interlock : public ActionObject
    {

    public:

        Interlock(void) : ActionObject("interlock", CLUSTER_BINARY_OUTPUT, MANUFACTURER_CODE_LUMI, 0xFF06) {}
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
}

#endif
