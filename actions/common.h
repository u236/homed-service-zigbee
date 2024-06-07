#ifndef ACTIONS_COMMON_H
#define ACTIONS_COMMON_H

#include "action.h"
#include "zcl.h"

namespace Actions
{
    class Status : public ActionObject
    {

    public:

        Status(void) : ActionObject("status", CLUSTER_ON_OFF, 0x0000, 0x0000) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class Level : public ActionObject
    {

    public:

        Level(void) : ActionObject("level", CLUSTER_LEVEL_CONTROL, 0x0000, 0x0000) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class AnalogOutput : public ActionObject
    {

    public:

        AnalogOutput(void) : ActionObject("analogOutput", CLUSTER_ANALOG_OUTPUT, 0x0000, 0x0055) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class CoverStatus : public ActionObject
    {

    public:

        CoverStatus(void) : ActionObject("cover", CLUSTER_WINDOW_COVERING) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class CoverPosition : public ActionObject
    {

    public:

        CoverPosition(void) : ActionObject("position", CLUSTER_WINDOW_COVERING, 0x0000, 0x0008) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class CoverTilt : public ActionObject
    {

    public:

        CoverTilt(void) : ActionObject("tilt", CLUSTER_WINDOW_COVERING, 0x0000, 0x0009) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class Thermostat : public ActionObject
    {

    public:

        Thermostat(void) : ActionObject("thermostat", CLUSTER_THERMOSTAT, 0x0000, QList <QString> {"temperatureOffset", "hysteresis", "targetTemperature", "systemMode"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ColorHS : public ActionObject
    {

    public:

        ColorHS(void) : ActionObject("color", CLUSTER_COLOR_CONTROL, 0x0000, QList <quint16> {0x0000, 0x0001, 0x0008}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ColorXY : public ActionObject
    {

    public:

        ColorXY(void) : ActionObject("color", CLUSTER_COLOR_CONTROL, 0x0000, QList <quint16> {0x0003, 0x0004, 0x0008}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ColorTemperature : public ActionObject
    {

    public:

        ColorTemperature(void) : ActionObject("colorTemperature", CLUSTER_COLOR_CONTROL, 0x0000, QList <quint16> {0x0007, 0x0008}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
    
    class OccupancyTimeout : public ActionObject
    {

    public:

        OccupancyTimeout(void) : ActionObject("occupancyTimeout", CLUSTER_OCCUPANCY_SENSING, 0x0000, 0x0010) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ChildLock : public ActionObject
    {

    public:

        ChildLock(void) : ActionObject("childLock", CLUSTER_THERMOSTAT_UI_CONFIGURATION, 0x0000, 0x0001) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class PowerOnStatus : public EnumAction
    {

    public:

        PowerOnStatus(void) : EnumAction("powerOnStatus", CLUSTER_ON_OFF, 0x0000, 0x4003, DATA_TYPE_8BIT_ENUM) {}

    };

    class SwitchType : public EnumAction
    {

    public:

        SwitchType(void) : EnumAction("switchType", CLUSTER_SWITCH_CONFIGURATION, 0x0000, 0x0000, DATA_TYPE_8BIT_ENUM) {}

    };

    class SwitchMode : public EnumAction
    {

    public:

        SwitchMode(void) : EnumAction("switchMode", CLUSTER_SWITCH_CONFIGURATION, 0x0000, 0x0010, DATA_TYPE_8BIT_ENUM) {}

    };

    class FanMode : public EnumAction
    {

    public:

        FanMode(void) : EnumAction("fanMode", CLUSTER_FAN_CONTROL, 0x0000, 0x0000, DATA_TYPE_8BIT_ENUM) {}

    };

    class DisplayMode : public EnumAction
    {

    public:

        DisplayMode(void) : EnumAction("displayMode", CLUSTER_THERMOSTAT_UI_CONFIGURATION, 0x0000, 0x0000, DATA_TYPE_8BIT_ENUM) {}

    };
}

#endif
