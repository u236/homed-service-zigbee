#ifndef ACTIONS_TUYA_H
#define ACTIONS_TUYA_H

#include "action.h"
#include "zcl.h"

namespace ActionsTUYA
{
    class Request
    {

    protected:

        QByteArray makeRequest(quint8 transactionId, quint8 commandId, quint8 dataPoint, quint8 dataType, void *data, quint8 length = 0);

    };

    class DataPoints : public Request, public ActionObject
    {

    public:

        DataPoints(void) : ActionObject("tuyaDataPoints", CLUSTER_TUYA_DATA) {}

    private:

        QVariant request(const QString &name, const QVariant &data) override;

    };

    class HolidayThermostatProgram : public Request, public ActionObject
    {

    public:

        HolidayThermostatProgram(void) : ActionObject("thermostatProgram", CLUSTER_TUYA_DATA, 0x0000) {}
        QVariant request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

    };

    class DailyThermostatProgram : public Request, public ActionObject
    {

    public:

        DailyThermostatProgram(void) : ActionObject("thermostatProgram", CLUSTER_TUYA_DATA, 0x0000) {}
        QVariant request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

    };

    class MoesThermostatProgram : public Request, public ActionObject
    {

    public:

        MoesThermostatProgram(void) : ActionObject("thermostatProgram", CLUSTER_TUYA_DATA, 0x0000) {}
        QVariant request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

    };

    class LedController: public Request, public ActionObject
    {

    public:

        LedController(void) : ActionObject("ledController", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"color", "colorTemperature"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class CoverMotor : public Request, public ActionObject
    {

    public:

        CoverMotor(void) : ActionObject("coverMotor", CLUSTER_TUYA_DATA, 0x0000, QList <QString> {"cover", "position"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class CoverSwitch : public ActionObject
    {

    public:

        CoverSwitch(void) : ActionObject("coverSwitch", CLUSTER_WINDOW_COVERING, 0x0000, QList <QString> {"calibration", "reverse"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class ChildLock : public ActionObject
    {

    public:

        ChildLock(void) : ActionObject("childLock", CLUSTER_ON_OFF, 0x0000, 0x8000) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class IRCode : public ActionObject
    {

    public:

        IRCode(void) : ActionObject("irCode", CLUSTER_TUYA_IR_DATA, 0x0000) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class IRLearn : public ActionObject
    {

    public:

        IRLearn(void) : ActionObject("learn", CLUSTER_TUYA_IR_CONTROL, 0x0000) {}
        QVariant request(const QString &name, const QVariant &data) override;

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
