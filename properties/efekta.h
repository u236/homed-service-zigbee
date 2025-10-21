#ifndef PROPERTIES_EFEKTA_H
#define PROPERTIES_EFEKTA_H

#include "property.h"
#include "zcl.h"

namespace PropertiesEfekta
{
    class ReadInterval : public PropertyObject
    {

    public:

        ReadInterval(void) : PropertyObject("readInterval", CLUSTER_POWER_CONFIGURATION, 0x0201) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class TemperatureSettings : public PropertyObject
    {

    public:

        TemperatureSettings(void) : PropertyObject("temperatureSettings", CLUSTER_TEMPERATURE_MEASUREMENT, {0x0210, 0x0220, 0x0221, 0x0222, 0x0225}) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class HumiditySettings : public PropertyObject
    {

    public:

        HumiditySettings(void) : PropertyObject("humiditySettings", CLUSTER_HUMIDITY_MEASUREMENT, {0x0210, 0x0220, 0x0221, 0x0222, 0x0225}) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class CO2Settings : public PropertyObject
    {

    public:

        CO2Settings(void) : PropertyObject("co2Settings", CLUSTER_CO2_CONCENTRATION, {0x0202, 0x0203, 0x0204, 0x0205, 0x0206, 0x0207, 0x0209, 0x0211, 0x0220, 0x0221, 0x0222, 0x0225, 0x0244, 0x0401, 0x0402}) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class PMSensor : public PropertyObject
    {

    public:

        PMSensor(void) : PropertyObject("pmSenosor", CLUSTER_PM25_CONCENTRATION, {0x00C8, 0x00C9, 0x0201, 0x0220, 0x0221, 0x0222, 0x0225}) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class VOCSensor : public PropertyObject
    {

    public:

        VOCSensor(void) : PropertyObject("vocSensor", CLUSTER_ANALOG_INPUT, {0x0055, 0x0220, 0x0221, 0x0222, 0x0225}) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };
}

#endif
