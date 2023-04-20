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
        m_name(name), m_clusterId(clusterId) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;

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

    class AnalogInput : public BindingObject
    {

    public:

        AnalogInput(void) : BindingObject("analogInput", CLUSTER_ANALOG_INPUT) {}

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

        Humidity(void) : BindingObject("temperature", CLUSTER_RELATIVE_HUMIDITY) {}

    };

    class Moisture : public BindingObject
    {

    public:

        Moisture(void) : BindingObject("moisture", CLUSTER_SOIL_MOISTURE) {}

    };

    class CO2 : public BindingObject
    {

    public:

        CO2(void) : BindingObject("co2", CLUSTER_CO2_CONCENTRATION) {}

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
