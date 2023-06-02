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
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    private:

        QVariant parseData(const tuyaHeaderStruct *header, const QByteArray &data);
        virtual void update(quint8 dataPoint, const QVariant &data) = 0;

    };

    class LightDimmer : public Data
    {

    public:

        LightDimmer(void) : Data("lightDimmer") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class ElectricityMeter : public Data
    {

    public:

        ElectricityMeter(void) : Data("electricityMeter") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class MultichannelRelay : public Data
    {

    public:

        MultichannelRelay(void) : Data("multichannelRelay") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class RadiatorThermostat : public Data
    {

    public:

        RadiatorThermostat(void) : Data("radiatorThermostat") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class MoesElectricThermostat : public Data
    {

    public:

        MoesElectricThermostat(void) : Data("moesElectricThermostat") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class MoesRadiatorThermostat : public Data
    {

    public:

        MoesRadiatorThermostat(void) : Data("moesRadiatorThermostat") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class WeekdayThermostatProgram : public Data
    {

    public:

        WeekdayThermostatProgram(void) : Data("weekdayThermostatProgram") {}

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

    class NeoSiren : public Data
    {

    public:

        NeoSiren(void) : Data("neoSiren") {}

    public:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class WaterValve : public Data
    {

    public:

        WaterValve(void) : Data("waterValve") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class SmokeDetector : public Data
    {

    public:

        SmokeDetector(void) : Data("SmokeDetector") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class PresenceSensor : public Data
    {

    public:

        PresenceSensor(void) : Data("presenceSensor") {}

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class RadarSensor : public Data
    {

    public:

        RadarSensor(void) : Data("radarSensor") {}

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
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class ChildLock : public PropertyObject
    {

    public:

        ChildLock(void) : PropertyObject("childLock", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class OperationMode : public PropertyObject
    {

    public:

        OperationMode(void) : PropertyObject("operationMode", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class IndicatorMode : public PropertyObject
    {

    public:

        IndicatorMode(void) : PropertyObject("indicatorMode", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class SwitchType : public PropertyObject
    {

    public:

        SwitchType(void) : PropertyObject("switchType", CLUSTER_TUYA_SWITCH_MODE) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class PowerOnStatus : public PropertyObject
    {

    public:

        PowerOnStatus(void) : PropertyObject("powerOnStatus", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class ButtonAction : public PropertyObject
    {

    public:

        ButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };
}

#endif
