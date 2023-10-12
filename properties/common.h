#ifndef PROPERTIES_COMMON_H
#define PROPERTIES_COMMON_H

#include "property.h"
#include "zcl.h"

namespace Properties
{
    class BatteryVoltage : public PropertyObject
    {

    public:

        BatteryVoltage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class BatteryPercentage : public PropertyObject
    {

    public:

        BatteryPercentage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class DeviceTemperature : public PropertyObject
    {

    public:

        DeviceTemperature(void) : PropertyObject("temperature", CLUSTER_TEMPERATURE_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Status : public PropertyObject
    {

    public:

        Status(void) : PropertyObject("status", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class PowerOnStatus : public PropertyObject
    {

    public:

        PowerOnStatus(void) : PropertyObject("powerOnStatus", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class SwitchType : public PropertyObject
    {

    public:

        SwitchType(void) : PropertyObject("switchType", CLUSTER_SWITCH_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class SwitchMode : public PropertyObject
    {

    public:

        SwitchMode(void) : PropertyObject("switchMode", CLUSTER_SWITCH_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Level : public PropertyObject
    {

    public:

        Level(void) : PropertyObject("level", CLUSTER_LEVEL_CONTROL) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class CoverStatus : public PropertyObject
    {

    public:

        CoverStatus(void) : PropertyObject("cover", CLUSTER_WINDOW_COVERING) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class CoverPosition : public PropertyObject
    {

    public:

        CoverPosition(void) : PropertyObject("position", CLUSTER_WINDOW_COVERING) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class CoverTilt : public PropertyObject
    {

    public:

        CoverTilt(void) : PropertyObject("tilt", CLUSTER_WINDOW_COVERING) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class ColorHS : public PropertyObject
    {

    public:

        ColorHS(void) : PropertyObject("color", CLUSTER_COLOR_CONTROL) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    private:

        QVariant m_colorH, m_colorS;

    };

    class ColorXY : public PropertyObject
    {

    public:

        ColorXY(void) : PropertyObject("color", CLUSTER_COLOR_CONTROL) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    private:

        QVariant m_colorX, m_colorY;

    };

    class ColorTemperature : public PropertyObject
    {

    public:

        ColorTemperature(void) : PropertyObject("colorTemperature", CLUSTER_COLOR_CONTROL) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Illuminance : public PropertyObject
    {

    public:

        Illuminance(void) : PropertyObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Temperature : public PropertyObject
    {

    public:

        Temperature(void) : PropertyObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Pressure : public PropertyObject
    {

    public:

        Pressure(void) : PropertyObject("pressure", CLUSTER_PRESSURE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Humidity : public PropertyObject
    {

    public:

        Humidity(void) : PropertyObject("humidity", CLUSTER_RELATIVE_HUMIDITY) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Occupancy : public PropertyObject
    {

    public:

        Occupancy(void) : PropertyObject("occupancy", CLUSTER_OCCUPANCY_SENSING) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;
        void resetValue(void) override;

    };

    class Moisture : public PropertyObject
    {

    public:

        Moisture(void) : PropertyObject("moisture", CLUSTER_SOIL_MOISTURE) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class CO2 : public PropertyObject
    {

    public:

        CO2(void) : PropertyObject("co2", CLUSTER_CO2_CONCENTRATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Energy : public PropertyObject
    {

    public:

        Energy(void) : PropertyObject("energy", CLUSTER_SMART_ENERGY_METERING) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Voltage : public PropertyObject
    {

    public:

        Voltage(void) : PropertyObject("voltage", CLUSTER_ELECTRICAL_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Current : public PropertyObject
    {

    public:

        Current(void) : PropertyObject("current", CLUSTER_ELECTRICAL_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Power : public PropertyObject
    {

    public:

        Power(void) : PropertyObject("power", CLUSTER_ELECTRICAL_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };
    
    class Thermostat : public PropertyObject
    {

    public:
        
        Thermostat(void) : PropertyObject("thermostat", CLUSTER_THERMOSTAT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Scene : public PropertyObject
    {

    public:

        Scene(void) : PropertyObject("scene", CLUSTER_SCENES) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class StatusAction : public PropertyObject
    {

    public:

        StatusAction(void) : PropertyObject("action", CLUSTER_ON_OFF) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class LevelAction : public PropertyObject
    {

    public:

        LevelAction(void) : PropertyObject("action", CLUSTER_LEVEL_CONTROL) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class ColorAction : public PropertyObject
    {

    public:

        ColorAction(void) : PropertyObject("action", CLUSTER_COLOR_CONTROL) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };
}

#endif
