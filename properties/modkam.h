#ifndef PROPERTIES_MODKAM_H
#define PROPERTIES_MODKAM_H

#include "property.h"
#include "zcl.h"

namespace PropertiesModkam
{
    class ButtonAction : public PropertyObject
    {

    public:

        ButtonAction(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class TemperatureOffset : public PropertyObject
    {

    public:

        TemperatureOffset(void) : PropertyObject("temperatureOffset", CLUSTER_TEMPERATURE_MEASUREMENT) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class HumidityOffset : public PropertyObject
    {

    public:

        HumidityOffset(void) : PropertyObject("humidityOffset", CLUSTER_RELATIVE_HUMIDITY) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class PressureOffset : public PropertyObject
    {

    public:

        PressureOffset(void) : PropertyObject("pressureOffset", CLUSTER_PRESSURE_MEASUREMENT) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class CO2Settings : public PropertyObject
    {

    public:

        CO2Settings(void) : PropertyObject("co2Settings", CLUSTER_CO2_CONCENTRATION) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Geiger : public PropertyObject
    {

    public:

        Geiger(void) : PropertyObject("geiger", {CLUSTER_ON_OFF, CLUSTER_ILLUMINANCE_MEASUREMENT, CLUSTER_ILLUMINANCE_LEVEL_SENSING}) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };
}

#endif
