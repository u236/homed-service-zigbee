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
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class Level : public ActionObject
    {

    public:

        Level(void) : ActionObject("level", CLUSTER_LEVEL_CONTROL, 0x0000, 0x0000) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class AnalogOutput : public ActionObject
    {

    public:

        AnalogOutput(void) : ActionObject("analogOutput", CLUSTER_ANALOG_OUTPUT, 0x0000, 0x0055) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class CoverStatus : public ActionObject
    {

    public:

        CoverStatus(void) : ActionObject("cover", CLUSTER_WINDOW_COVERING) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class CoverPosition : public ActionObject
    {

    public:

        CoverPosition(void) : ActionObject("position", CLUSTER_WINDOW_COVERING, 0x0000, 0x0008) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class Thermostat : public ActionObject
    {

    public:

        Thermostat(void) : ActionObject("thermostat", CLUSTER_THERMOSTAT, 0x0000, QList <QString> {"temperatureOffset", "hysteresis", "targetTemperature", "systemMode", "mondayP1Hour", "mondayP1Minute", "mondayP1Temperature", "mondayP2Hour", "mondayP2Minute", "mondayP2Temperature", "mondayP3Hour", "mondayP3Minute", "mondayP3Temperature", "mondayP4Hour", "mondayP4Minute", "mondayP4Temperature", "mondayP5Hour", "mondayP5Minute", "mondayP5Temperature", "mondayP6Hour", "mondayP6Minute", "mondayP6Temperature", "tuesdayP1Hour", "tuesdayP1Minute", "tuesdayP1Temperature", "tuesdayP2Hour", "tuesdayP2Minute", "tuesdayP2Temperature", "tuesdayP3Hour", "tuesdayP3Minute", "tuesdayP3Temperature", "tuesdayP4Hour", "tuesdayP4Minute", "tuesdayP4Temperature", "tuesdayP5Hour", "tuesdayP5Minute", "tuesdayP5Temperature", "tuesdayP6Hour", "tuesdayP6Minute", "tuesdayP6Temperature", "wednesdayP1Hour", "wednesdayP1Minute", "wednesdayP1Temperature", "wednesdayP2Hour", "wednesdayP2Minute", "wednesdayP2Temperature", "wednesdayP3Hour", "wednesdayP3Minute", "wednesdayP3Temperature", "wednesdayP4Hour", "wednesdayP4Minute", "wednesdayP4Temperature", "wednesdayP5Hour", "wednesdayP5Minute", "wednesdayP5Temperature", "wednesdayP6Hour", "wednesdayP6Minute", "wednesdayP6Temperature", "thursdayP1Hour", "thursdayP1Minute", "thursdayP1Temperature", "thursdayP2Hour", "thursdayP2Minute", "thursdayP2Temperature", "thursdayP3Hour", "thursdayP3Minute", "thursdayP3Temperature", "thursdayP4Hour", "thursdayP4Minute", "thursdayP4Temperature", "thursdayP5Hour", "thursdayP5Minute", "thursdayP5Temperature", "thursdayP6Hour", "thursdayP6Minute", "thursdayP6Temperature", "fridayP1Hour", "fridayP1Minute", "fridayP1Temperature", "fridayP2Hour", "fridayP2Minute", "fridayP2Temperature", "fridayP3Hour", "fridayP3Minute", "fridayP3Temperature", "fridayP4Hour", "fridayP4Minute", "fridayP4Temperature", "fridayP5Hour", "fridayP5Minute", "fridayP5Temperature", "fridayP6Hour", "fridayP6Minute", "fridayP6Temperature", "saturdayP1Hour", "saturdayP1Minute", "saturdayP1Temperature", "saturdayP2Hour", "saturdayP2Minute", "saturdayP2Temperature", "saturdayP3Hour", "saturdayP3Minute", "saturdayP3Temperature", "saturdayP4Hour", "saturdayP4Minute", "saturdayP4Temperature", "saturdayP5Hour", "saturdayP5Minute", "saturdayP5Temperature", "saturdayP6Hour", "saturdayP6Minute", "saturdayP6Temperature", "sundayP1Hour", "sundayP1Minute", "sundayP1Temperature", "sundayP2Hour", "sundayP2Minute", "sundayP2Temperature", "sundayP3Hour", "sundayP3Minute", "sundayP3Temperature", "sundayP4Hour", "sundayP4Minute", "sundayP4Temperature", "sundayP5Hour", "sundayP5Minute", "sundayP5Temperature", "sundayP6Hour", "sundayP6Minute", "sundayP6Temperature"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

    };

    class ColorHS : public ActionObject
    {

    public:

        ColorHS(void) : ActionObject("color", CLUSTER_COLOR_CONTROL, 0x0000, QList <quint16> {0x0000, 0x0001, 0x0008}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class ColorXY : public ActionObject
    {

    public:

        ColorXY(void) : ActionObject("color", CLUSTER_COLOR_CONTROL, 0x0000, QList <quint16> {0x0003, 0x0004, 0x0008}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class ColorTemperature : public ActionObject
    {

    public:

        ColorTemperature(void) : ActionObject("colorTemperature", CLUSTER_COLOR_CONTROL, 0x0000, QList <quint16> {0x0007, 0x0008}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };
    
    class OccupancyTimeout : public ActionObject
    {

    public:

        OccupancyTimeout(void) : ActionObject("occupancyTimeout", CLUSTER_OCCUPANCY_SENSING, 0x0000, 0x0010) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class ChildLock : public ActionObject
    {

    public:

        ChildLock(void) : ActionObject("childLock", CLUSTER_THERMOSTAT_UI_CONFIGURATION, 0x0000, 0x0001) {}
        QVariant request(const QString &name, const QVariant &data) override;

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
