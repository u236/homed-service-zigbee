#ifndef ACTIONS_EFEKTA_H
#define ACTIONS_EFEKTA_H

#include "action.h"
#include "zcl.h"

namespace ActionsEfekta
{
    class ReportingDelay : public ActionObject
    {

    public:

        ReportingDelay(void) : ActionObject("reportingDelay", CLUSTER_POWER_CONFIGURATION, 0x0000, 0x0201) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class TemperatureSettings : public ActionObject
    {

    public:

        TemperatureSettings(void) : ActionObject("temperatureSettings", CLUSTER_TEMPERATURE_MEASUREMENT, 0x0000, QList <QString> {"temperatureOffset", "temperatureHigh", "temperatureLow", "temperatureRelay", "temperatureRelayInvert"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class HumiditySettings : public ActionObject
    {

    public:

        HumiditySettings(void) : ActionObject("humiditySettings", CLUSTER_RELATIVE_HUMIDITY, 0x0000, QList <QString> {"humidityOffset", "humidityHigh", "humidityLow", "humidityRelay", "humidityRelayInvert"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class CO2Sensor : public ActionObject
    {

    public:

        CO2Sensor(void) : ActionObject("co2Sensor", CLUSTER_CO2_CONCENTRATION, 0x0000, QList <QString> {"altitude", "co2ManualCalibration", "co2High", "co2Low", "indicatorLevel", "co2ForceCalibration", "autoBrightness", "co2LongChart", "co2FactoryReset", "indicator", "co2Relay", "co2RelayInvert", "pressureLongChart", "nightBacklight"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class VOCSensor : public ActionObject
    {

    public:

        VOCSensor(void) : ActionObject("vocSensor", CLUSTER_ANALOG_INPUT, 0x0000, QList <QString> {"vocHigh", "vocLow", "vocRelay", "vocRelayInvert"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

#endif
