#ifndef PROPERTIES_COMMON_H
#define PROPERTIES_COMMON_H

#include "property.h"
#include "zcl.h"

namespace Properties
{
    class BatteryVoltage : public PropertyObject
    {

    public:

        BatteryVoltage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION, 0x0020) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class BatteryPercentage : public PropertyObject
    {

    public:

        BatteryPercentage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION, 0x0021) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class DeviceTemperature : public PropertyObject
    {

    public:

        DeviceTemperature(void) : PropertyObject("temperature", CLUSTER_TEMPERATURE_CONFIGURATION, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Status : public PropertyObject
    {

    public:

        Status(void) : PropertyObject("status", CLUSTER_ON_OFF, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Level : public PropertyObject
    {

    public:

        Level(void) : PropertyObject("level", CLUSTER_LEVEL_CONTROL, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class AnalogInput : public PropertyObject
    {

    public:

        AnalogInput(void) : PropertyObject("analogInput", CLUSTER_ANALOG_INPUT, 0x0055) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class AnalogOutput : public PropertyObject
    {

    public:

        AnalogOutput(void) : PropertyObject("analogOutput", CLUSTER_ANALOG_OUTPUT, 0x0055) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class CoverPosition : public PropertyObject
    {

    public:

        CoverPosition(void) : PropertyObject("position", CLUSTER_WINDOW_COVERING, 0x0008) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Thermostat : public PropertyObject
    {

    public:

        Thermostat(void) : PropertyObject("thermostat", CLUSTER_THERMOSTAT, {0x0000, 0x0010, 0x0012, 0x0019, 0x001C, 0x001E}) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };

    class ColorHS : public PropertyObject
    {

    public:

        ColorHS(void) : PropertyObject("color", CLUSTER_COLOR_CONTROL, {0x0000, 0x0001}) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    private:

        QVariant m_h, m_s;

    };

    class ColorXY : public PropertyObject
    {

    public:

        ColorXY(void) : PropertyObject("color", CLUSTER_COLOR_CONTROL, {0x0003, 0x0004}) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    private:

        QVariant m_x, m_y;

    };

    class ColorTemperature : public PropertyObject
    {

    public:

        ColorTemperature(void) : PropertyObject("colorTemperature", CLUSTER_COLOR_CONTROL, 0x0007) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class ColorMode : public PropertyObject
    {

    public:

        ColorMode(void) : PropertyObject("colorMode", CLUSTER_COLOR_CONTROL, 0x0008) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Illuminance : public PropertyObject
    {

    public:

        Illuminance(void) : PropertyObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Temperature : public PropertyObject
    {

    public:

        Temperature(void) : PropertyObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Pressure : public PropertyObject
    {

    public:

        Pressure(void) : PropertyObject("pressure", CLUSTER_PRESSURE_MEASUREMENT, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Flow : public PropertyObject
    {

    public:

        Flow(void) : PropertyObject("flow", CLUSTER_FLOW_MEASUREMENT, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Humidity : public PropertyObject
    {

    public:

        Humidity(void) : PropertyObject("humidity", CLUSTER_HUMIDITY_MEASUREMENT, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Occupancy : public PropertyObject
    {

    public:

        Occupancy(void) : PropertyObject("occupancy", CLUSTER_OCCUPANCY_SENSING, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;
        void resetValue(void) override;

    };

    class OccupancyTimeout : public PropertyObject
    {

    public:

        OccupancyTimeout(void) : PropertyObject("occupancyTimeout", CLUSTER_OCCUPANCY_SENSING, 0x0010) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Moisture : public PropertyObject
    {

    public:

        Moisture(void) : PropertyObject("moisture", CLUSTER_MOISTURE_MEASUREMENT, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class CO2 : public PropertyObject
    {

    public:

        CO2(void) : PropertyObject("co2", CLUSTER_CO2_CONCENTRATION, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class PM25 : public PropertyObject
    {

    public:

        PM25(void) : PropertyObject("pm25", CLUSTER_PM25_CONCENTRATION, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Energy : public PropertyObject
    {

    public:

        Energy(void) : PropertyObject("energy", CLUSTER_SMART_ENERGY_METERING, {0x0000, 0x0100, 0x0102, 0x0104, 0x0106}) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Voltage : public PropertyObject
    {

    public:

        Voltage(void) : PropertyObject("voltage", CLUSTER_ELECTRICAL_MEASUREMENT, {0x0100, 0x0505}) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Current : public PropertyObject
    {

    public:

        Current(void) : PropertyObject("current", CLUSTER_ELECTRICAL_MEASUREMENT, {0x0103, 0x0508}) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Power : public PropertyObject
    {

    public:

        Power(void) : PropertyObject("power", CLUSTER_ELECTRICAL_MEASUREMENT, {0x0106, 0x050B}) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class ChildLock : public PropertyObject
    {

    public:

        ChildLock(void) : PropertyObject("childLock", CLUSTER_UI_CONFIGURATION, 0x0001) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };
    
    class Scene : public PropertyObject
    {

    public:

        Scene(void) : PropertyObject("scene", CLUSTER_SCENES) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };

    class StatusAction : public PropertyObject
    {

    public:

        StatusAction(void) : PropertyObject("action", CLUSTER_ON_OFF) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };

    class LevelAction : public PropertyObject
    {

    public:

        LevelAction(void) : PropertyObject("action", CLUSTER_LEVEL_CONTROL) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };

    class CoverAction : public PropertyObject
    {

    public:

        CoverAction(void) : PropertyObject("action", CLUSTER_WINDOW_COVERING) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };

    class ColorAction : public PropertyObject
    {

    public:

        ColorAction(void) : PropertyObject("action", CLUSTER_COLOR_CONTROL) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };

    class PowerOnStatus : public EnumProperty
    {

    public:

        PowerOnStatus(void) : EnumProperty("powerOnStatus", CLUSTER_ON_OFF, 0x4003) {}

    };

    class SwitchType : public EnumProperty
    {

    public:

        SwitchType(void) : EnumProperty("switchType", CLUSTER_SWITCH_CONFIGURATION, 0x0000) {}

    };

    class SwitchMode : public EnumProperty
    {

    public:

        SwitchMode(void) : EnumProperty("switchMode", CLUSTER_SWITCH_CONFIGURATION, 0x0010) {}

    };

    class FanMode : public EnumProperty
    {

    public:

        FanMode(void) : EnumProperty("fanMode", CLUSTER_FAN_CONTROL, 0x0000) {}

    };

    class DisplayMode : public EnumProperty
    {

    public:

        DisplayMode(void) : EnumProperty("displayMode", CLUSTER_UI_CONFIGURATION, 0x0000) {}

    };
}

#endif
