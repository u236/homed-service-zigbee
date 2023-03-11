#ifndef ACTION_H
#define ACTION_H

#include <QSharedPointer>
#include <QVariant>
#include "property.h"
#include "zcl.h"

class ActionObject;
typedef QSharedPointer <ActionObject> Action;

class ActionObject
{

public:

    ActionObject(const QString &name, quint16 clusterId, quint16 manufacturerCode = 0, QList <quint16> attributes = {}) :
        m_name(name), m_clusterId(clusterId), m_manufacturerCode(manufacturerCode), m_attributes(attributes), m_transactionId(0) {}

    ActionObject(const QString &name, quint16 clusterId, quint16 manufacturerCode, quint16 attributeId) :
        m_name(name), m_clusterId(clusterId), m_manufacturerCode(manufacturerCode), m_attributes({attributeId}), m_transactionId(0) {}

    ActionObject(const QString &name, quint16 clusterId, quint16 manufacturerCode, QList <QString> actions) :
        m_name(name), m_clusterId(clusterId), m_manufacturerCode(manufacturerCode), m_actions(actions), m_transactionId(0) {}

    virtual ~ActionObject(void) {}
    virtual QByteArray request(const QString &name, const QVariant &data) = 0;

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline quint16 manufacturerCode(void) { return m_manufacturerCode; }

    inline QList <quint16> &attributes(void) { return m_attributes; }
    inline QList <QString> &actions(void) { return m_actions; }

    inline void setParent(QObject *value) { m_parent = value; }
    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId, m_manufacturerCode;

    QList <quint16> m_attributes;
    QList <QString> m_actions;

    QObject *m_parent;
    quint8 m_transactionId;

    QVariant endpointOption(const QString &name);
    Property endpointProperty(const QString &name);

};

namespace Actions
{
    class Status : public ActionObject
    {

    public:

        Status(void) : ActionObject("status", CLUSTER_ON_OFF, 0x0000, 0x0000) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class PowerOnStatus : public ActionObject
    {

    public:

        PowerOnStatus(void) : ActionObject("powerOnStatus", CLUSTER_ON_OFF, 0x0000, 0x4003) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class Level : public ActionObject
    {

    public:

        Level(void) : ActionObject("level", CLUSTER_LEVEL_CONTROL, 0x0000, 0x0000) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class CoverStatus : public ActionObject
    {

    public:

        CoverStatus(void) : ActionObject("cover", CLUSTER_WINDOW_COVERING) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ColorHS : public ActionObject
    {

    public:

        ColorHS(void) : ActionObject("color", CLUSTER_COLOR_CONTROL, 0x0000, {0x0000, 0x0001}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ColorXY : public ActionObject
    {

    public:

        ColorXY(void) : ActionObject("color", CLUSTER_COLOR_CONTROL, 0x0000, {0x0003, 0x0004}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ColorTemperature : public ActionObject
    {

    public:

        ColorTemperature(void) : ActionObject("colorTemperature", CLUSTER_COLOR_CONTROL, 0x0000, 0x0007) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

namespace ActionsPTVO
{
    class Status : public ActionObject
    {

    public:

        Status(const QString &name) : ActionObject(name, CLUSTER_ON_OFF, 0x0000, 0x0000) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class AnalogInput : public ActionObject
    {

    public:

        AnalogInput(const QString &name) : ActionObject(name, CLUSTER_ANALOG_INPUT, 0x0000, 0x0055) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ChangePattern : public Status
    {

    public:

        ChangePattern(void) : Status("changePattern") {}

    };

    class Count : public AnalogInput
    {

    public:

        Count(void) : AnalogInput("count") {}

    };

    class Pattern : public AnalogInput
    {

    public:

        Pattern(void) : AnalogInput("pattern") {}

    };
}

namespace ActionsLUMI
{
    class PresenceSensor : public ActionObject
    {

    public:

        PresenceSensor(void) : ActionObject("presenceSensor", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, {"sensitivityMode", "detectionMode", "distanceMode", "resetPresence"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ButtonMode : public ActionObject
    {

    public:

        ButtonMode(void) : ActionObject("buttonMode", CLUSTER_BASIC, MANUFACTURER_CODE_LUMI, {"buttonMode", "leftMode", "rightMode"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class OperationMode : public ActionObject
    {

    public:

        OperationMode(void) : ActionObject("operationMode", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0009) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class IndicatorMode : public ActionObject
    {

    public:

        IndicatorMode(void) : ActionObject("indicatorMode", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x00F0) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class SwitchMode : public ActionObject
    {

    public:

        SwitchMode(void) : ActionObject("switchMode", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0200) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class StatusMemory : public ActionObject
    {

    public:

        StatusMemory(void) : ActionObject("statusMemory", CLUSTER_LUMI, MANUFACTURER_CODE_LUMI, 0x0201) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class Interlock : public ActionObject
    {

    public:

        Interlock(void) : ActionObject("interlock", CLUSTER_BINARY_OUTPUT, MANUFACTURER_CODE_LUMI, 0xFF06) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class CoverPosition : public ActionObject
    {

    public:

        CoverPosition(void) : ActionObject("position", CLUSTER_ANALOG_OUTPUT) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

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

        LightDimmer(void) : ActionObject("lightDimmer", CLUSTER_TUYA_DATA, 0x0000, {"status", "level", "levelMin", "lightType", "levelMax", "countdown", "powerOnStatus"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ElectricityMeter : public Request, public ActionObject
    {

    public:

        ElectricityMeter(void) : ActionObject("status", CLUSTER_TUYA_DATA) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class Cover : public Request, public ActionObject
    {

    public:

        Cover(void) : ActionObject("cover", CLUSTER_TUYA_DATA, 0x0000, {"cover", "position", "reverse", "speed"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class MoesElectricThermostat : public Request, public ActionObject
    {

    public:

        MoesElectricThermostat(void) : ActionObject("moesElectricThermostat", CLUSTER_TUYA_DATA, 0x0000, {"status", "operationMode", "heatingPoint", "temperatureLimitMax", "deadZoneTemperature", "temperatureLimitMin", "temperatureCalibration", "childLock", "sensor"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;
    };

    class MoesRadiatorThermostat : public Request, public ActionObject
    {

    public:

        MoesRadiatorThermostat(void) : ActionObject("moesElectricThermostat", CLUSTER_TUYA_DATA, 0x0000, {"operationMode", "heatingPoint", "boost", "windowDetection", "childLock", "boostTimeout", "temperatureCalibration", "ecoMode", "ecoModeTemperature", "temperatureLimitMax", "temperatureLimitMin"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class MoesThermostatProgram : public Request, public ActionObject
    {

    public:

        MoesThermostatProgram(void) : ActionObject("moesThermostatProgram", CLUSTER_TUYA_DATA, 0x0000, {"weekdayP1Hour", "weekdayP1Minute", "weekdayP1Temperature", "weekdayP2Hour", "weekdayP2Minute", "weekdayP2Temperature", "weekdayP3Hour", "weekdayP3Minute", "weekdayP3Temperature", "weekdayP4Hour", "weekdayP4Minute", "weekdayP4Temperature", "saturdayP1Hour", "saturdayP1Minute", "saturdayP1Temperature", "saturdayP2Hour", "saturdayP2Minute", "saturdayP2Temperature", "saturdayP3Hour", "saturdayP3Minute", "saturdayP3Temperature", "saturdayP4Hour", "saturdayP4Minute", "saturdayP4Temperature", "sundayP1Hour", "sundayP1Minute", "sundayP1Temperature", "sundayP2Hour", "sundayP2Minute", "sundayP2Temperature", "sundayP3Hour", "sundayP3Minute", "sundayP3Temperature", "sundayP4Hour", "sundayP4Minute", "sundayP4Temperature"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    private:

        QMap <QString, QVariant> m_data;

    };

    class NeoSiren : public Request, public ActionObject
    {

    public:

        NeoSiren(void) : ActionObject("neoSiren", CLUSTER_TUYA_DATA, 0x0000, {"volume", "duration", "alarm", "melody"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class WaterValve : public Request, public ActionObject
    {

    public:

        WaterValve(void) : ActionObject("waterValve", CLUSTER_TUYA_DATA, 0x0000, {"status", "timeout", "threshold"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class PresenceSensor : public Request, public ActionObject
    {

    public:

        PresenceSensor(void) : ActionObject("presenceSensor", CLUSTER_TUYA_DATA, 0x0000, {"sensitivity", "distanceMin", "distanceMax", "detectionDelay", "fadingTime"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class RadarSensor : public Request, public ActionObject
    {

    public:

        RadarSensor(void) : ActionObject("radarSensor", CLUSTER_TUYA_DATA, 0x0000, {"radarSensitivity", "tumbleSwitch", "tumbleAlarmTime", "radarScene", "fallSensitivity"}) {}
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

        SwitchType(void) : ActionObject("switchMode", CLUSTER_TUYA_SWITCH_MODE, 0x0000, 0xD030) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class PowerOnStatus : public ActionObject
    {

    public:

        PowerOnStatus(void) : ActionObject("powerOnStatus", CLUSTER_ON_OFF, 0x0000, 0x8002) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}


namespace ActionsEfekta
{
    class ReportingDelay : public ActionObject
    {

    public:

        ReportingDelay(void) : ActionObject("reportingDelay", CLUSTER_POWER_CONFIGURATION, 0x0000, 0x0201) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class TemperatureOffset : public ActionObject
    {

    public:

        TemperatureOffset(void) : ActionObject("temperatureOffset", CLUSTER_TEMPERATURE_MEASUREMENT, 0x0000, 0x0410) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

namespace ActionsOther
{
    class PerenioSmartPlug : public ActionObject
    {

    public:

        PerenioSmartPlug(void) : ActionObject("perenioSmartPlug", CLUSTER_PERENIO, 0x0000, {"powerOnStatus", "resetAlarms", "voltageMin", "voltageMax", "powerMax", "energyLimit"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

#endif
