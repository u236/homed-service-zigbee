#ifndef ACTIONS_EFEKTA_H
#define ACTIONS_EFEKTA_H

#include "action.h"
#include "zcl.h"

namespace ActionsEfekta
{
    class ReadInterval : public ActionObject
    {

    public:

        ReadInterval(void) : ActionObject("readInterval", CLUSTER_POWER_CONFIGURATION, 0x0000, 0x0201) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class TemperatureSettings : public ActionObject
    {

    public:

        TemperatureSettings(void) : ActionObject("temperatureSettings", CLUSTER_TEMPERATURE_MEASUREMENT, 0x0000, QList <QString> {"temperatureOffset", "temperatureHigh", "temperatureLow", "temperatureRelay", "temperatureRelayInvert"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class HumiditySettings : public ActionObject
    {

    public:

        HumiditySettings(void) : ActionObject("humiditySettings", CLUSTER_HUMIDITY_MEASUREMENT, 0x0000, QList <QString> {"humidityOffset", "humidityHigh", "humidityLow", "humidityRelay", "humidityRelayInvert"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class CO2Settings : public ActionObject
    {

    public:

        CO2Settings(void) : ActionObject("co2Settings", CLUSTER_CO2_CONCENTRATION, 0x0000, QList <QString> {"altitude", "co2ManualCalibration", "co2High", "co2Low", "indicatorLevel", "co2ForceCalibration", "autoBrightness", "co2LongChart", "co2FactoryReset", "indicator", "co2Relay", "co2RelayInvert", "pressureLongChart", "backlight", "co2AutoCalibration"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class PMSensor : public ActionObject
    {

    public:

        PMSensor(void) : ActionObject("pmSensor", CLUSTER_PM25_CONCENTRATION, 0x0000, QList <QString> {"readInterval", "pm25High", "pm25Low", "pm25Relay", "pm25RelayInvert"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class VOCSensor : public ActionObject
    {

    public:

        VOCSensor(void) : ActionObject("vocSensor", CLUSTER_ANALOG_INPUT, 0x0000, QList <QString> {"vocHigh", "vocLow", "vocRelay", "vocRelayInvert"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };
}

#endif
