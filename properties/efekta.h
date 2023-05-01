#ifndef PROPERTIES_EFEKTA_H
#define PROPERTIES_EFEKTA_H

#include "property.h"
#include "zcl.h"

namespace PropertiesEfekta
{
    class ReportingDelay : public PropertyObject
    {

    public:

        ReportingDelay(void) : PropertyObject("reportingDelay", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class TemperatureSettings : public PropertyObject
    {

    public:

        TemperatureSettings(void) : PropertyObject("temperatureSettings", CLUSTER_TEMPERATURE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class HumiditySettings : public PropertyObject
    {

    public:

        HumiditySettings(void) : PropertyObject("humiditySettings", CLUSTER_RELATIVE_HUMIDITY) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class CO2Sensor : public PropertyObject
    {

    public:

        CO2Sensor(void) : PropertyObject("co2Sensor", CLUSTER_CO2_CONCENTRATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class VOCSensor : public PropertyObject
    {

    public:

        VOCSensor(void) : PropertyObject("vocSensor", CLUSTER_ANALOG_INPUT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };
}

#endif
