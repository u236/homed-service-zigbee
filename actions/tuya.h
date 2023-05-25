#ifndef ACTIONS_TUYA_H
#define ACTIONS_TUYA_H

#include "action.h"
#include "zcl.h"

namespace ActionsTUYA
{
    class Request
    {

    protected:

        QByteArray makeRequest(quint8 transactionId, quint8 dataPoint, quint8 dataType, void *data, quint8 length);

    };

    class LightDimmer : public Request, public ActionObject
    {

    public:

        LightDimmer(void) : ActionObject("lightDimmer", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"status", "level", "levelMin", "lightType", "levelMax"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ElectricityMeter : public Request, public ActionObject
    {

    public:

        ElectricityMeter(void) : ActionObject("status", CLUSTER_TUYA_DATA) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class RadiatorThermostat : public Request, public ActionObject
    {

    public:

        RadiatorThermostat(void) : ActionObject("radiatorThermostat", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"targetTemperature", "operationMode", "childLock", "temperatureOffset", "temperatureLimitMin", "temperatureLimitMax", "boostTimeout", "comfortTemperature", "ecoTemperature", "weekMode", "awayTemperature", "awayDays"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class MoesElectricThermostat : public Request, public ActionObject
    {

    public:

        MoesElectricThermostat(void) : ActionObject("moesElectricThermostat", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"status", "operationMode", "targetTemperature", "temperatureLimitMax", "deadZoneTemperature", "temperatureLimitMin", "temperatureOffset", "childLock", "sensorType"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;
    };

    class MoesRadiatorThermostat : public Request, public ActionObject
    {

    public:

        MoesRadiatorThermostat(void) : ActionObject("", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"operationMode", "targetTemperature", "boost", "childLock", "boostTimeout", "temperatureOffset", "ecoMode", "ecoTemperature", "temperatureLimitMax", "temperatureLimitMin"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class MoesThermostatProgram : public Request, public ActionObject
    {

    public:

        MoesThermostatProgram(void) : ActionObject("moesThermostatProgram", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"weekdayP1Hour", "weekdayP1Minute", "weekdayP1Temperature", "weekdayP2Hour", "weekdayP2Minute", "weekdayP2Temperature", "weekdayP3Hour", "weekdayP3Minute", "weekdayP3Temperature", "weekdayP4Hour", "weekdayP4Minute", "weekdayP4Temperature", "saturdayP1Hour", "saturdayP1Minute", "saturdayP1Temperature", "saturdayP2Hour", "saturdayP2Minute", "saturdayP2Temperature", "saturdayP3Hour", "saturdayP3Minute", "saturdayP3Temperature", "saturdayP4Hour", "saturdayP4Minute", "saturdayP4Temperature", "sundayP1Hour", "sundayP1Minute", "sundayP1Temperature", "sundayP2Hour", "sundayP2Minute", "sundayP2Temperature", "sundayP3Hour", "sundayP3Minute", "sundayP3Temperature", "sundayP4Hour", "sundayP4Minute", "sundayP4Temperature"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

    };

    class NeoSiren : public Request, public ActionObject
    {

    public:

        NeoSiren(void) : ActionObject("neoSiren", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"volume", "duration", "alarm", "melody"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class WaterValve : public Request, public ActionObject
    {

    public:

        WaterValve(void) : ActionObject("waterValve", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"status", "timeout", "threshold"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class PresenceSensor : public Request, public ActionObject
    {

    public:

        PresenceSensor(void) : ActionObject("presenceSensor", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"sensitivity", "distanceMin", "distanceMax", "detectionDelay", "fadingTime"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class RadarSensor : public Request, public ActionObject
    {

    public:

        RadarSensor(void) : ActionObject("radarSensor", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"radarSensitivity", "tumbleSwitch", "tumbleAlarmTime", "radarScene", "fallSensitivity"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class CoverMotor : public Request, public ActionObject
    {

    public:

        CoverMotor(void) : ActionObject("coverMotor", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"cover", "position", "reverse", "speed"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class CoverSwitch : public ActionObject
    {

    public:

        CoverSwitch(void) : ActionObject("coverSwitch", CLUSTER_WINDOW_COVERING, 0x0000, QList <QString> {"calibration", "reverse"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ChildLock : public ActionObject
    {

    public:

        ChildLock(void) : ActionObject("childLock", CLUSTER_ON_OFF, 0x0000, 0x8000) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class OperationMode : public ActionObject
    {

    public:

        OperationMode(void) : ActionObject("operationMode", CLUSTER_ON_OFF, 0x0000, 0x8004) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class IndicatorMode : public ActionObject
    {

    public:

        IndicatorMode(void) : ActionObject("indicatorMode", CLUSTER_ON_OFF, 0x0000, 0x8001) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class SwitchType : public ActionObject
    {

    public:

        SwitchType(void) : ActionObject("switchType", CLUSTER_TUYA_SWITCH_MODE, 0x0000, 0xD030) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class PowerOnStatus : public ActionObject
    {

    public:

        PowerOnStatus(void) : ActionObject("powerOnStatus", CLUSTER_ON_OFF, 0x0000, 0x8002) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

#endif
