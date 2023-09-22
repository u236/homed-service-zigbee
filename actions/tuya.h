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

    class DataPoints : public Request, public ActionObject
    {

    public:

        DataPoints(void) : ActionObject("tuyaDataPoints", CLUSTER_TUYA_DATA) {}

    private:

        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class WeekdayThermostatProgram : public Request, public ActionObject
    {

    public:

        WeekdayThermostatProgram(void) : ActionObject("weekdayThermostatProgram", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"weekdayP1Hour", "weekdayP1Minute", "weekdayP1Temperature", "weekdayP2Hour", "weekdayP2Minute", "weekdayP2Temperature", "weekdayP3Hour", "weekdayP3Minute", "weekdayP3Temperature", "weekdayP4Hour", "weekdayP4Minute", "weekdayP4Temperature", "weekdayP5Hour", "weekdayP5Minute", "weekdayP5Temperature", "weekdayP6Hour", "weekdayP6Minute", "weekdayP6Temperature"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

    };

    class HolidayThermostatProgram : public Request, public ActionObject
    {

    public:

        HolidayThermostatProgram(void) : ActionObject("holidayThermostatProgram", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"holidayP1Hour", "holidayP1Minute", "holidayP1Temperature", "holidayP2Hour", "holidayP2Minute", "holidayP2Temperature", "holidayP3Hour", "holidayP3Minute", "holidayP3Temperature", "holidayP4Hour", "holidayP4Minute", "holidayP4Temperature", "holidayP5Hour", "holidayP5Minute", "holidayP5Temperature", "holidayP6Hour", "holidayP6Minute", "holidayP6Temperature"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

    };

    class MoesThermostatProgram : public Request, public ActionObject
    {

    public:

        MoesThermostatProgram(void) : ActionObject("moesThermostatProgram", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"weekdayP1Hour", "weekdayP1Minute", "weekdayP1Temperature", "weekdayP2Hour", "weekdayP2Minute", "weekdayP2Temperature", "weekdayP3Hour", "weekdayP3Minute", "weekdayP3Temperature", "weekdayP4Hour", "weekdayP4Minute", "weekdayP4Temperature", "saturdayP1Hour", "saturdayP1Minute", "saturdayP1Temperature", "saturdayP2Hour", "saturdayP2Minute", "saturdayP2Temperature", "saturdayP3Hour", "saturdayP3Minute", "saturdayP3Temperature", "saturdayP4Hour", "saturdayP4Minute", "saturdayP4Temperature", "sundayP1Hour", "sundayP1Minute", "sundayP1Temperature", "sundayP2Hour", "sundayP2Minute", "sundayP2Temperature", "sundayP3Hour", "sundayP3Minute", "sundayP3Temperature", "sundayP4Hour", "sundayP4Minute", "sundayP4Temperature"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

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

    class SensitivityMode : public ActionObject
    {

    public:

        SensitivityMode(void) : ActionObject("sensitivityMode", CLUSTER_IAS_ZONE, 0x0000, 0x0013) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class TimeoutMode : public ActionObject
    {

    public:

        TimeoutMode(void) : ActionObject("timeoutMode", CLUSTER_IAS_ZONE, 0x0000, 0xF001) {}
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
