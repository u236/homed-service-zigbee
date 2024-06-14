#ifndef ACTIONS_TUYA_H
#define ACTIONS_TUYA_H

#include "action.h"
#include "zcl.h"

namespace ActionsTUYA
{
    class Request
    {

    protected:

        QByteArray makeRequest(quint8 transactionId, quint8 dataPoint, quint8 dataType, void *data, quint8 length = 0);

    };

    class DataPoints : public Request, public ActionObject
    {

    public:

        DataPoints(void) : ActionObject("tuyaDataPoints", CLUSTER_TUYA_DATA) {}

    private:

        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class HolidayThermostatProgram : public Request, public ActionObject
    {

    public:

        HolidayThermostatProgram(void) : ActionObject("holidayThermostatProgram", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"weekdayP1Hour", "weekdayP1Minute", "weekdayP1Temperature", "weekdayP2Hour", "weekdayP2Minute", "weekdayP2Temperature", "weekdayP3Hour", "weekdayP3Minute", "weekdayP3Temperature", "weekdayP4Hour", "weekdayP4Minute", "weekdayP4Temperature", "weekdayP5Hour", "weekdayP5Minute", "weekdayP5Temperature", "weekdayP6Hour", "weekdayP6Minute", "weekdayP6Temperature", "holidayP1Hour", "holidayP1Minute", "holidayP1Temperature", "holidayP2Hour", "holidayP2Minute", "holidayP2Temperature", "holidayP3Hour", "holidayP3Minute", "holidayP3Temperature", "holidayP4Hour", "holidayP4Minute", "holidayP4Temperature", "holidayP5Hour", "holidayP5Minute", "holidayP5Temperature", "holidayP6Hour", "holidayP6Minute", "holidayP6Temperature"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

    };

    class DailyThermostatProgram : public Request, public ActionObject
    {

    public:

        DailyThermostatProgram(void) : ActionObject("dailyThermostatProgram", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"mondayP1Hour", "mondayP1Minute","mondayP1Temperature", "mondayP2Hour", "mondayP2Minute", "mondayP2Temperature", "mondayP3Hour", "mondayP3Minute", "mondayP3Temperature", "mondayP4Hour", "mondayP4Minute", "mondayP4Temperature", "tuesdayP1Hour", "tuesdayP1Minute", "tuesdayP1Temperature", "tuesdayP2Hour", "tuesdayP2Minute", "tuesdayP2Temperature", "tuesdayP3Hour", "tuesdayP3Minute", "tuesdayP3Temperature", "tuesdayP4Hour", "tuesdayP4Minute", "tuesdayP4Temperature","wednesdayP1Hour", "wednesdayP1Minute", "wednesdayP1Temperature", "wednesdayP2Hour", "wednesdayP2Minute", "wednesdayP2Temperature", "wednesdayP3Hour", "wednesdayP3Minute", "wednesdayP3Temperature", "wednesdayP4Hour", "wednesdayP4Minute", "wednesdayP4Temperature","thursdayP1Hour", "thursdayP1Minute", "thursdayP1Temperature", "thursdayP2Hour", "thursdayP2Minute", "thursdayP2Temperature", "thursdayP3Hour", "thursdayP3Minute", "thursdayP3Temperature", "thursdayP4Hour", "thursdayP4Minute", "thursdayP4Temperature", "fridayP1Hour", "fridayP1Minute", "fridayP1Temperature", "fridayP2Hour", "fridayP2Minute", "fridayP2Temperature", "fridayP3Hour", "fridayP3Minute", "fridayP3Temperature", "fridayP4Hour", "fridayP4Minute", "fridayP4Temperature","saturdayP1Hour", "saturdayP1Minute", "saturdayP1Temperature", "saturdayP2Hour", "saturdayP2Minute", "saturdayP2Temperature", "saturdayP3Hour", "saturdayP3Minute", "saturdayP3Temperature", "saturdayP4Hour", "saturdayP4Minute", "saturdayP4Temperature", "sundayP1Hour", "sundayP1Minute", "sundayP1Temperature", "sundayP2Hour", "sundayP2Minute", "sundayP2Temperature", "sundayP3Hour", "sundayP3Minute", "sundayP3Temperature", "sundayP4Hour", "sundayP4Minute", "sundayP4Temperature"}) {}
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

        CoverMotor(void) : ActionObject("coverMotor", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"cover", "position"}) {}
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

    class IRCode : public ActionObject
    {

    public:

        IRCode(void) : ActionObject("irCode", CLUSTER_TUYA_IR_DATA, 0x0000) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class IRLearn : public ActionObject
    {

    public:

        IRLearn(void) : ActionObject("learn", CLUSTER_TUYA_IR_CONTROL, 0x0000) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class OperationMode : public EnumAction
    {

    public:

        OperationMode(void) : EnumAction("operationMode", CLUSTER_ON_OFF, 0x0000, 0x8004, DATA_TYPE_8BIT_ENUM) {}

    };

    class IndicatorMode : public EnumAction
    {

    public:

        IndicatorMode(void) : EnumAction("indicatorMode", CLUSTER_ON_OFF, 0x0000, 0x8001, DATA_TYPE_8BIT_ENUM) {}

    };

    class SwitchType : public EnumAction
    {

    public:

        SwitchType(void) : EnumAction("switchType", CLUSTER_TUYA_SWITCH_MODE, 0x0000, 0xD030, DATA_TYPE_8BIT_ENUM) {}

    };

    class PowerOnStatus : public EnumAction
    {

    public:

        PowerOnStatus(void) : EnumAction("powerOnStatus", CLUSTER_ON_OFF, 0x0000, 0x8002, DATA_TYPE_8BIT_ENUM) {}

    };
}

#endif
