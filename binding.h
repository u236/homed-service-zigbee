#ifndef BINDING_H
#define BINDING_H

#include <QSharedPointer>
#include "zcl.h"

class BindingObject;
typedef QSharedPointer <BindingObject> Binding;

class BindingObject
{

public:

    BindingObject(const QString &name, quint16 clusterId) :
        m_name(name), m_clusterId(clusterId), m_endpointId(0x00) {}

    BindingObject(quint16 clusterId, const QByteArray &address, quint8 endpointId) :
        m_clusterId(clusterId), m_address(address), m_endpointId(endpointId) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QByteArray address(void) { return m_address; }
    inline quint8 endpointId(void) { return m_endpointId; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;
    QByteArray m_address;
    quint8 m_endpointId;

};

namespace Bindings
{
    class Battery : public BindingObject
    {

    public:

        Battery(void) : BindingObject("battery", CLUSTER_POWER_CONFIGURATION) {}

    };

    class DeviceTemperature : public BindingObject
    {

    public:

        DeviceTemperature(void) : BindingObject("deviceTemperature", CLUSTER_TEMPERATURE_CONFIGURATION) {}

    };

    class Scene : public BindingObject
    {

    public:

        Scene(void) : BindingObject("scene", CLUSTER_SCENES) {}

    };

    class Status : public BindingObject
    {

    public:

        Status(void) : BindingObject("status", CLUSTER_ON_OFF) {}

    };

    class Level : public BindingObject
    {

    public:

        Level(void) : BindingObject("level", CLUSTER_LEVEL_CONTROL) {}

    };

    class Time : public BindingObject
    {

    public:

        Time(void) : BindingObject("time", CLUSTER_TIME) {}

    };

    class AnalogInput : public BindingObject
    {

    public:

        AnalogInput(void) : BindingObject("analogInput", CLUSTER_ANALOG_INPUT) {}

    };

    class AnalogOutput : public BindingObject
    {

    public:

        AnalogOutput(void) : BindingObject("analogOutput", CLUSTER_ANALOG_OUTPUT) {}

    };

    class MultistateInput : public BindingObject
    {

    public:

        MultistateInput(void) : BindingObject("multistateInput", CLUSTER_MULTISTATE_INPUT) {}

    };

    class PollControl : public BindingObject
    {

    public:

        PollControl(void) : BindingObject("pollControl", CLUSTER_POLL_CONTROL) {}

    };

    class Cover : public BindingObject
    {

    public:

        Cover(void) : BindingObject("cover", CLUSTER_WINDOW_COVERING) {}

    };

    class Thermostat : public BindingObject
    {

    public:

        Thermostat(void) : BindingObject("thermostat", CLUSTER_THERMOSTAT) {}

    };

    class Fan : public BindingObject
    {

    public:

        Fan(void) : BindingObject("fan", CLUSTER_FAN_CONTROL) {}

    };

    class Color : public BindingObject
    {

    public:

        Color(void) : BindingObject("color", CLUSTER_COLOR_CONTROL) {}

    };    

    class Illuminance : public BindingObject
    {

    public:

        Illuminance(void) : BindingObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT) {}

    };

    class Temperature : public BindingObject
    {

    public:

        Temperature(void) : BindingObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT) {}

    };

    class Pressure : public BindingObject
    {

    public:

        Pressure(void) : BindingObject("pressure", CLUSTER_PRESSURE_MEASUREMENT) {}

    };

    class Humidity : public BindingObject
    {

    public:

        Humidity(void) : BindingObject("temperature", CLUSTER_HUMIDITY_MEASUREMENT) {}

    };

    class Occupancy : public BindingObject
    {

    public:

        Occupancy(void) : BindingObject("occupancy", CLUSTER_OCCUPANCY_SENSING) {}

    };

    class Moisture : public BindingObject
    {

    public:

        Moisture(void) : BindingObject("moisture", CLUSTER_MOISTURE_MEASUREMENT) {}

    };

    class CO2 : public BindingObject
    {

    public:

        CO2(void) : BindingObject("co2", CLUSTER_CO2_CONCENTRATION) {}

    };

    class PM25 : public BindingObject
    {

    public:

        PM25(void) : BindingObject("pm25", CLUSTER_PM25_CONCENTRATION) {}

    };

    class Energy : public BindingObject
    {

    public:

        Energy(void) : BindingObject("energy", CLUSTER_SMART_ENERGY_METERING) {}

    };

    class Power : public BindingObject
    {

    public:

        Power(void) : BindingObject("power", CLUSTER_ELECTRICAL_MEASUREMENT) {}

    };

    class Perenio : public BindingObject
    {

    public:

        Perenio(void) : BindingObject("perenio", CLUSTER_PERENIO) {}

    };
}

#endif
