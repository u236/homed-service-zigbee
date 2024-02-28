#ifndef PROPERTIES_TUYA_H
#define PROPERTIES_TUYA_H

#include "property.h"
#include "zcl.h"

namespace PropertiesTUYA
{
    class Data : public PropertyObject
    {

    public:

        Data(const QString &name) : PropertyObject(name, CLUSTER_TUYA_DATA) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    private:

        QVariant parseData(const tuyaHeaderStruct *header, const QByteArray &data);
        virtual void update(quint8 dataPoint, const QVariant &data) = 0;

    };

    class DataPoints : public Data
    {

    public:

        DataPoints(void) : Data("tuyaDataPoints") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class DailyThermostatProgram : public Data
    {

    public:

        DailyThermostatProgram(void) : Data("dailyThermostatProgram") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class HolidayThermostatProgram : public Data
    {

    public:

        HolidayThermostatProgram(void) : Data("holidayThermostatProgram") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class MoesThermostatProgram : public Data
    {

    public:

        MoesThermostatProgram(void) : Data("moesThermostatProgram") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class CoverMotor : public Data
    {

    public:

        CoverMotor(void) : Data("coverMotor") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class CoverSwitch : public PropertyObject
    {

    public:

        CoverSwitch(void) : PropertyObject("coverSwitch", CLUSTER_WINDOW_COVERING) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class ChildLock : public PropertyObject
    {

    public:

        ChildLock(void) : PropertyObject("childLock", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class OperationMode : public PropertyObject
    {

    public:

        OperationMode(void) : PropertyObject("operationMode", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class IndicatorMode : public PropertyObject
    {

    public:

        IndicatorMode(void) : PropertyObject("indicatorMode", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class SensitivityMode : public PropertyObject
    {

    public:

        SensitivityMode(void) : PropertyObject("sensitivityMode", CLUSTER_IAS_ZONE) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class TimeoutMode : public PropertyObject
    {

    public:

        TimeoutMode(void) : PropertyObject("timeoutMode", CLUSTER_IAS_ZONE) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class SwitchType : public PropertyObject
    {

    public:

        SwitchType(void) : PropertyObject("switchType", CLUSTER_TUYA_SWITCH_MODE) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class PowerOnStatus : public PropertyObject
    {

    public:

        PowerOnStatus(void) : PropertyObject("powerOnStatus", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class ButtonAction : public PropertyObject
    {

    public:

        ButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };
}

#endif
