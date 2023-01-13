#ifndef PROPERTY_H
#define PROPERTY_H

#include <QSharedPointer>
#include <QVariant>
#include "zcl.h"

class PropertyObject;
typedef QSharedPointer <PropertyObject> Property;

class PropertyObject
{

public:

    PropertyObject(const QString &name, quint16 clusterId, bool singleShot = false) :
        m_name(name), m_clusterId(clusterId), m_singleShot(singleShot), m_parent(nullptr), m_multiple(false), m_transactionId(0) {}

    virtual ~PropertyObject(void) {}
    virtual void parseAttribte(quint16, const QByteArray &) {}
    virtual void parseCommand(quint8, const QByteArray &, quint8) {}
    virtual void clearValue(void) { m_value = QVariant(); }
    virtual void resetValue(void) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline bool singleShot(void) { return m_singleShot; }

    inline void setParent(QObject *value) { m_parent = value; }

    inline bool multiple(void) { return m_multiple; }
    inline void setMultiple(bool value) { m_multiple = value; }

    inline QMap <QString, QVariant> &meta(void) { return m_meta; }

    inline QVariant value(void) { return m_value; }
    inline void setValue(const QVariant &value) { m_value = value; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;
    bool m_singleShot;

    QObject *m_parent;
    bool m_multiple;

    quint8 m_transactionId;
    QMap <QString, QVariant> m_meta;
    QVariant m_value;

    quint8 percentage(double min, double max, double value);
    bool checkTransactionId(quint8 transactionId);

    quint8 deviceVersion(void);
    QString deviceManufacturerName(void);
    QString deviceModelName(void);

    QVariant endpointOption(const QString &name);

};

namespace Properties
{
    class BatteryVoltage : public PropertyObject
    {

    public:

        BatteryVoltage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class BatteryPercentage : public PropertyObject
    {

    public:

        BatteryPercentage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class DeviceTemperature : public PropertyObject
    {

    public:

        DeviceTemperature(void) : PropertyObject("temperature", CLUSTER_TEMPERATURE_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Status : public PropertyObject
    {

    public:

        Status(void) : PropertyObject("status", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class PowerOnStatus : public PropertyObject
    {

    public:

        PowerOnStatus(void) : PropertyObject("powerOnStatus", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Level : public PropertyObject
    {

    public:

        Level(void) : PropertyObject("level", CLUSTER_LEVEL_CONTROL) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class CoverStatus : public PropertyObject
    {

    public:

        CoverStatus(void) : PropertyObject("cover", CLUSTER_WINDOW_COVERING) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class CoverPosition : public PropertyObject
    {

    public:

        CoverPosition(void) : PropertyObject("position", CLUSTER_WINDOW_COVERING) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class CoverTilt : public PropertyObject
    {

    public:

        CoverTilt(void) : PropertyObject("tilt", CLUSTER_WINDOW_COVERING) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class ColorHS : public PropertyObject
    {

    public:

        ColorHS(void) : PropertyObject("color", CLUSTER_COLOR_CONTROL) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    private:

        QVariant m_colorH, m_colorS;

    };

    class ColorXY : public PropertyObject
    {

    public:

        ColorXY(void) : PropertyObject("color", CLUSTER_COLOR_CONTROL) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    private:

        QVariant m_colorX, m_colorY;

    };

    class ColorTemperature : public PropertyObject
    {

    public:

        ColorTemperature(void) : PropertyObject("colorTemperature", CLUSTER_COLOR_CONTROL) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Illuminance : public PropertyObject
    {

    public:

        Illuminance(void) : PropertyObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Temperature : public PropertyObject
    {

    public:

        Temperature(void) : PropertyObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Pressure : public PropertyObject
    {

    public:

        Pressure(void) : PropertyObject("pressure", CLUSTER_PRESSURE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Humidity : public PropertyObject
    {

    public:

        Humidity(void) : PropertyObject("humidity", CLUSTER_RELATIVE_HUMIDITY) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Occupancy : public PropertyObject
    {

    public:

        Occupancy(void) : PropertyObject("occupancy", CLUSTER_OCCUPANCY_SENSING, true) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;
        void resetValue(void) override;

    };

    class Moisture : public PropertyObject
    {

    public:

        Moisture(void) : PropertyObject("moisture", CLUSTER_SOIL_MOISTURE) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Energy : public PropertyObject
    {

    public:

        Energy(void) : PropertyObject("energy", CLUSTER_SMART_ENERGY_METERING) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Voltage : public PropertyObject
    {

    public:

        Voltage(void) : PropertyObject("voltage", CLUSTER_ELECTRICAL_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Current : public PropertyObject
    {

    public:

        Current(void) : PropertyObject("current", CLUSTER_ELECTRICAL_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Power : public PropertyObject
    {

    public:

        Power(void) : PropertyObject("power", CLUSTER_ELECTRICAL_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Scene : public PropertyObject
    {

    public:

        Scene(void) : PropertyObject("scene", CLUSTER_SCENES, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload, quint8 transactionId) override;

    };

    class IdentifyAction : public PropertyObject
    {

    public:

        IdentifyAction(void) : PropertyObject("action", CLUSTER_IDENTIFY, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload, quint8 transactionId) override;

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload, quint8 transactionId) override;

    };

    class LevelAction : public PropertyObject
    {

    public:

        LevelAction(void) : PropertyObject("action", CLUSTER_LEVEL_CONTROL, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload, quint8 transactionId) override;

    };

    class ColorAction : public PropertyObject
    {

    public:

        ColorAction(void) : PropertyObject("action", CLUSTER_COLOR_CONTROL, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload, quint8 transactionId) override;

    };
}

namespace PropertiesIAS
{
    class ZoneStatus : public PropertyObject
    {

    public:

        ZoneStatus(const QString &name = "alarm") : PropertyObject(name, CLUSTER_IAS_ZONE, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload, quint8 transactionId) override;
        void clearValue(void) override;
        void resetValue(void) override;

    };

    class Contact : public ZoneStatus
    {

    public:

        Contact(void) : ZoneStatus("contact") {}

    };

    class Gas : public ZoneStatus
    {

    public:

        Gas(void) : ZoneStatus("gas") {}

    };

    class Occupancy : public ZoneStatus
    {

    public:

        Occupancy(void) : ZoneStatus("occupancy") {}

    };

    class Smoke : public ZoneStatus
    {

    public:

        Smoke(void) : ZoneStatus("smoke") {}

    };

    class WaterLeak : public ZoneStatus
    {

    public:

        WaterLeak(void) : ZoneStatus("waterLeak") {}

    };
}

namespace PropertiesPTVO
{
    class Status : public PropertyObject
    {

    public:

        Status(const QString &name) : PropertyObject(name, CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class AnalogInput: public PropertyObject
    {

    public:

        AnalogInput(const QString &name, const QString &unit = QString()) : PropertyObject(name, CLUSTER_ANALOG_INPUT), m_unit(unit) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    private:

        QString m_unit;
        QVariant m_buffer;

    };

    class ChangePattern : public Status
    {

    public:

        ChangePattern(void) : Status("changePattern") {}

    };

    class Contact : public Status
    {

    public:

        Contact(void) : Status("contact") {}

    };

    class Occupancy : public Status
    {

    public:

        Occupancy(void) : Status("occupancy") {}

    };

    class WaterLeak : public Status
    {

    public:

        WaterLeak(void) : Status("waterLeak") {}

    };

    class CO2 : public AnalogInput
    {

    public:

        CO2(void) : AnalogInput("co2", "ppm") {}

    };

    class Temperature : public AnalogInput
    {

    public:

        Temperature(void) : AnalogInput("temperature", "C") {}

    };

    class Humidity : public AnalogInput
    {

    public:

        Humidity(void) : AnalogInput("humidity", "%") {}

    };

    class Count: public AnalogInput
    {

    public:

        Count(void) : AnalogInput("count") {}

    };

    class Pattern: public AnalogInput
    {

    public:

        Pattern(void) : AnalogInput("pattern") {}

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };
}

namespace PropertiesLUMI
{
    class Data : public PropertyObject
    {

    public:

        Data(const quint16 cluster = CLUSTER_LUMI) : PropertyObject("data", cluster) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    private:

        void parseData(quint16 dataPoint, const QByteArray &data, QMap <QString, QVariant> &map);

    };

    class Basic : public Data
    {

    public:

        Basic(void) : Data(CLUSTER_BASIC) {}

    };

    class ButtonMode : public PropertyObject
    {

    public:

        ButtonMode(const QString &name = "buttonMode") : PropertyObject(name, CLUSTER_BASIC) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Contact : public PropertyObject
    {

    public:

        Contact(void) : PropertyObject("contact", CLUSTER_ON_OFF, true) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Power : public PropertyObject
    {

    public:

        Power(void) : PropertyObject("power", CLUSTER_ANALOG_INPUT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Cover : public PropertyObject
    {

    public:

        Cover(void) : PropertyObject("cover", CLUSTER_ANALOG_OUTPUT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Interlock : public PropertyObject
    {

    public:

        Interlock(void) : PropertyObject("lock", CLUSTER_BINARY_OUTPUT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Illuminance : public PropertyObject
    {

    public:

        Illuminance(void) : PropertyObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class ButtonAction : public PropertyObject
    {

    public:

        ButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class CubeRotation : public PropertyObject
    {

    public:

        CubeRotation(void) : PropertyObject("action", CLUSTER_ANALOG_INPUT, true) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class CubeMovement : public PropertyObject
    {

    public:

        CubeMovement(void) :  PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class Vibration : public PropertyObject
    {

    public:

        Vibration(void) :  PropertyObject("vibration", CLUSTER_DOOR_LOCK) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;
        void resetValue(void) override;

    };
}

namespace PropertiesTUYA
{
    class Data : public PropertyObject
    {

    public:

        Data(void) : PropertyObject("data", CLUSTER_TUYA_DATA) {}
        void parseCommand(quint8 commandId, const QByteArray &payload, quint8 transactionId) override;

    private:

        QVariant parseData(const tuyaHeaderStruct *header, const QByteArray &data);
        virtual void update(quint8 dataPoint, const QVariant &data) = 0;

    };

    class ElectricityMeter : public Data
    {

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class MoesThermostat : public Data
    {

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class NeoSiren : public Data
    {

    public:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class WaterValve : public Data
    {

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

    };

    class PresenceSensor : public Data
    {

    private:

        void update(quint8 dataPoint, const QVariant &data) override;

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

        OperationMode(void) : PropertyObject("mode", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class BacklightMode : public PropertyObject
    {

    public:

        BacklightMode(void) : PropertyObject("backlightMode", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class IndicatorMode : public PropertyObject
    {

    public:

        IndicatorMode(void) : PropertyObject("indicatorMode", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class SwitchMode : public PropertyObject
    {

    public:

        SwitchMode(void) : PropertyObject("switchMode", CLUSTER_TUYA_SWITCH_MODE) {}
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

        ButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload, quint8 transactionId) override;

    };
}

namespace PropertiesOther
{
    class EfektaReportingDelay : public PropertyObject
    {

    public:

        EfektaReportingDelay(void) : PropertyObject("reportingDelay", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class KonkeButtonAction : public PropertyObject
    {

    public:

        KonkeButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class SonoffButtonAction : public PropertyObject
    {

    public:

        SonoffButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload, quint8 transactionId) override;

    };

    class LifeControlAirQuality : public PropertyObject
    {

    public:

        LifeControlAirQuality(void) : PropertyObject("airQuality", CLUSTER_TEMPERATURE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class PerenioSmartPlug : public PropertyObject
    {

    public:

        PerenioSmartPlug(void) : PropertyObject("smartPlug", CLUSTER_PERENIO) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };
}

#endif
