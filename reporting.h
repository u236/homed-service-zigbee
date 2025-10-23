#ifndef REPORTING_H
#define REPORTING_H

#include <QSharedPointer>
#include "zcl.h"

class ReportingObject;
typedef QSharedPointer <ReportingObject> Reporting;

class ReportingObject
{

public:

    ReportingObject(const QString &name, quint16 clusterId, QList <quint16> attributes, quint8 dataType, quint16 minInterval, quint16 maxInterval, quint16 valueChange = 1) :
        m_name(name), m_clusterId(clusterId), m_attributes(attributes), m_dataType(dataType), m_minInterval(minInterval), m_maxInterval(maxInterval), m_valueChange(valueChange) {}

    ReportingObject(const QString &name, quint16 clusterId, quint16 attributeId, quint8 dataType, quint16 minInterval, quint16 maxInterval, quint64 valueChange = 1) :
        m_name(name), m_clusterId(clusterId), m_attributes({attributeId}), m_dataType(dataType), m_minInterval(minInterval), m_maxInterval(maxInterval), m_valueChange(valueChange) {}

    virtual ~ReportingObject(void) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QList <quint16> &attributes(void) { return m_attributes; }

    inline quint8 dataType(void) { return m_dataType; }

    inline quint16 minInterval(void) { return m_minInterval; }
    inline void setMinInterval(quint16 value) { m_minInterval = value; }

    inline quint16 maxInterval(void) { return m_maxInterval; }
    inline void setMaxInterval(quint16 value) { m_maxInterval = value; }

    inline quint64 valueChange(void) { return m_valueChange; }
    inline void setValueChange(quint64 value) { m_valueChange = value; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;
    QList <quint16> m_attributes;

    quint8 m_dataType;
    quint16 m_minInterval, m_maxInterval;
    quint64 m_valueChange;

};

namespace Reportings
{
    class BatteryVoltage : public ReportingObject
    {

    public:

        BatteryVoltage(void) : ReportingObject("battery", CLUSTER_POWER_CONFIGURATION, 0x0020, DATA_TYPE_8BIT_UNSIGNED, 60, 3600) {}

    };

    class BatteryPercentage : public ReportingObject
    {

    public:

        BatteryPercentage(void) : ReportingObject("battery", CLUSTER_POWER_CONFIGURATION, 0x0021, DATA_TYPE_8BIT_UNSIGNED, 60, 3600) {}

    };

    class DeviceTemperature : public ReportingObject
    {

    public:

        DeviceTemperature(void) : ReportingObject("deviceTemperature", CLUSTER_TEMPERATURE_CONFIGURATION, 0x0000, DATA_TYPE_16BIT_SIGNED, 10, 3600) {}

    };

    class Status : public ReportingObject
    {

    public:

        Status(void) : ReportingObject("status", CLUSTER_ON_OFF, 0x0000, DATA_TYPE_BOOLEAN, 0, 600) {}

    };

    class Level : public ReportingObject
    {

    public:

        Level(void) : ReportingObject("level", CLUSTER_LEVEL_CONTROL, 0x0000, DATA_TYPE_8BIT_UNSIGNED, 0, 600) {}

    };

    class AnalogInput : public ReportingObject
    {

    public:

        AnalogInput(void) : ReportingObject("analogInput", CLUSTER_ANALOG_INPUT, {0x0055, 0x001C}, DATA_TYPE_SINGLE_PRECISION, 0, 600, 0) {}

    };

    class AnalogOutput : public ReportingObject
    {

    public:

        AnalogOutput(void) : ReportingObject("analogOutput", CLUSTER_ANALOG_OUTPUT, 0x0055, DATA_TYPE_SINGLE_PRECISION, 0, 600, 0) {}

    };

    class CoverPosition : public ReportingObject
    {

    public:

        CoverPosition(void) : ReportingObject("coverPosition", CLUSTER_WINDOW_COVERING, 0x0008, DATA_TYPE_8BIT_UNSIGNED, 1, 3600) {}

    };

    class Thermostat : public ReportingObject
    {

    public:

        Thermostat(void) : ReportingObject("thermostat", CLUSTER_THERMOSTAT, {0x0000, 0x0012}, DATA_TYPE_16BIT_SIGNED, 1, 3600) {}

    };

    class ColorHS : public ReportingObject
    {

    public:

        ColorHS(void) : ReportingObject("colorHS", CLUSTER_COLOR_CONTROL, {0x0000, 0x0001}, DATA_TYPE_8BIT_UNSIGNED, 0, 600) {}

    };

    class ColorXY : public ReportingObject
    {

    public:

        ColorXY(void) : ReportingObject("colorXY", CLUSTER_COLOR_CONTROL, {0x0003, 0x0004}, DATA_TYPE_16BIT_UNSIGNED, 0, 600) {}

    };

    class ColorTemperature : public ReportingObject
    {

    public:

        ColorTemperature(void) : ReportingObject("colorTemperature", CLUSTER_COLOR_CONTROL, 0x0007, DATA_TYPE_16BIT_UNSIGNED, 0, 600) {}

    };

    class Illuminance : public ReportingObject
    {

    public:

        Illuminance(void) : ReportingObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_UNSIGNED, 10, 3600, 10) {}

    };

    class Temperature : public ReportingObject
    {

    public:

        Temperature(void) : ReportingObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_SIGNED, 10, 3600, 10) {}

    };

    class Pressure : public ReportingObject
    {

    public:

        Pressure(void) : ReportingObject("pressure", CLUSTER_PRESSURE_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_SIGNED, 10, 3600, 10) {}

    };

    class Flow : public ReportingObject
    {

    public:

        Flow(void) : ReportingObject("flow", CLUSTER_FLOW_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_UNSIGNED, 10, 3600, 10) {}

    };

    class Humidity : public ReportingObject
    {

    public:

        Humidity(void) : ReportingObject("humidity", CLUSTER_HUMIDITY_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_UNSIGNED, 10, 3600, 10) {}

    };

    class Occupancy : public ReportingObject
    {

    public:

        Occupancy(void) : ReportingObject("occupancy", CLUSTER_OCCUPANCY_SENSING, 0x0000, DATA_TYPE_8BIT_BITMAP, 0, 600) {}

    };

    class Moisture : public ReportingObject
    {

    public:

        Moisture(void) : ReportingObject("moisture", CLUSTER_MOISTURE_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_UNSIGNED, 10, 3600, 10) {}

    };

    class CO2 : public ReportingObject
    {

    public:

        CO2(void) : ReportingObject("co2", CLUSTER_CO2_CONCENTRATION, 0x0000, DATA_TYPE_SINGLE_PRECISION, 10, 3600, 0) {}

    };

    class PM25 : public ReportingObject
    {

    public:

        PM25(void) : ReportingObject("pm25", CLUSTER_PM25_CONCENTRATION, 0x0000, DATA_TYPE_SINGLE_PRECISION, 10, 3600, 0) {}

    };

    class Energy : public ReportingObject
    {

    public:

        Energy(void) : ReportingObject("energy", CLUSTER_SMART_ENERGY_METERING, 0x0000, DATA_TYPE_48BIT_UNSIGNED, 10, 600) {}

    };

    class EnergyT1 : public ReportingObject
    {

    public:

        EnergyT1(void) : ReportingObject("energyT1", CLUSTER_SMART_ENERGY_METERING, 0x0100, DATA_TYPE_48BIT_UNSIGNED, 10, 600) {}

    };

    class EnergyT2 : public ReportingObject
    {

    public:

        EnergyT2(void) : ReportingObject("energyT2", CLUSTER_SMART_ENERGY_METERING, 0x0102, DATA_TYPE_48BIT_UNSIGNED, 10, 600) {}

    };

    class EnergyT3 : public ReportingObject
    {

    public:

        EnergyT3(void) : ReportingObject("energyT3", CLUSTER_SMART_ENERGY_METERING, 0x0104, DATA_TYPE_48BIT_UNSIGNED, 10, 600) {}

    };

    class EnergyT4 : public ReportingObject
    {

    public:

        EnergyT4(void) : ReportingObject("energyT4", CLUSTER_SMART_ENERGY_METERING, 0x0106, DATA_TYPE_48BIT_UNSIGNED, 10, 600) {}

    };

    class Voltage : public ReportingObject
    {

    public:

        Voltage(void) : ReportingObject("voltage", CLUSTER_ELECTRICAL_MEASUREMENT, {0x0100, 0x0505}, DATA_TYPE_16BIT_UNSIGNED, 10, 600) {}

    };

    class Current : public ReportingObject
    {

    public:

        Current(void) : ReportingObject("current", CLUSTER_ELECTRICAL_MEASUREMENT, {0x0103, 0x0508}, DATA_TYPE_16BIT_UNSIGNED, 10, 600) {}

    };

    class Power : public ReportingObject
    {

    public:

        Power(void) : ReportingObject("power", CLUSTER_ELECTRICAL_MEASUREMENT, {0x0106, 0x050B}, DATA_TYPE_16BIT_SIGNED, 10, 600) {}

    };

    class Frequency : public ReportingObject
    {

    public:

        Frequency(void) : ReportingObject("frequency", CLUSTER_ELECTRICAL_MEASUREMENT,  0x0300, DATA_TYPE_16BIT_UNSIGNED, 10, 600) {}

    };

    class PowerFactor : public ReportingObject
    {

    public:

        PowerFactor(void) : ReportingObject("powerFactor", CLUSTER_ELECTRICAL_MEASUREMENT, 0x0510, DATA_TYPE_8BIT_SIGNED, 10, 600) {}

    };
}

namespace ReportingsEfekta
{
    class PMSensor : public ReportingObject
    {

    public:

        PMSensor(void) : ReportingObject("pmSensor", CLUSTER_PM25_CONCENTRATION, {0x0000, 0x00C8, 0x00C9}, DATA_TYPE_SINGLE_PRECISION, 0, 600, 0) {}

    };

    class VOCSensor : public ReportingObject
    {

    public:

        VOCSensor(void) : ReportingObject("vocSensor", CLUSTER_ANALOG_INPUT, 0x0055, DATA_TYPE_SINGLE_PRECISION, 0, 600, 0) {}

    };
}

#endif
