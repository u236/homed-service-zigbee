#ifndef ACTIONS_MODKAM_H
#define ACTIONS_MODKAM_H

#include "action.h"
#include "zcl.h"

namespace ActionsModkam
{
    class TemperatureOffset : public ActionObject
    {

    public:

        TemperatureOffset(void) : ActionObject("temperatureOffset", CLUSTER_TEMPERATURE_MEASUREMENT, 0x0000, 0x0210) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class HumidityOffset : public ActionObject
    {

    public:

        HumidityOffset(void) : ActionObject("humidityOffset", CLUSTER_HUMIDITY_MEASUREMENT, 0x0000, 0x0210) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class PressureOffset : public ActionObject
    {

    public:

        PressureOffset(void) : ActionObject("pressureOffset", CLUSTER_PRESSURE_MEASUREMENT, 0x0000, 0x0210) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class CO2Settings : public ActionObject
    {

    public:

        CO2Settings(void) : ActionObject("co2Settings", CLUSTER_CO2_CONCENTRATION, 0x0000, QList <QString> {"autoCalibration", "ledFeedback", "thresholdLow", "thresholdHigh"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class Geiger : public ActionObject
    {

    public:

        Geiger(void) : ActionObject("geiger", CLUSTER_ILLUMINANCE_LEVEL_SENSING, 0x0000, QList <QString> {"sensitivity", "ledFeedback", "buzzerFeedback", "sensorCount", "sensorType", "threshold"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

#endif
