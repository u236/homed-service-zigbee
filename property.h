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
        m_name(name), m_clusterId(clusterId), m_singleShot(singleShot) {}

    virtual ~PropertyObject(void) {}
    virtual void parseAttribte(quint16, quint8, const QByteArray &) {}
    virtual void parseCommand(quint8, const QByteArray &) {}
    virtual void clearValue(void) { m_value = QVariant(); }
    virtual void resetValue(void) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline bool singleShot(void) { return m_singleShot; }

    inline void setParent(QObject *value) { m_parent = value; }

    inline bool multiple(void) { return m_multiple; }
    inline void setMultiple(bool value) { m_multiple = value; }

    inline QVariant value(void) { return m_value; }
    inline void setValue(const QVariant &value) { m_value = value; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;
    bool m_singleShot;

    QObject *m_parent;
    bool m_multiple;

    QVariant m_value;

    quint8 deviceVersion(void);
    QString deviceModelName(void);
    QVariant deviceOption(const QString &key);

    quint8 percentage(double min, double max, double value);

};

namespace Properties
{
    class BatteryVoltage : public PropertyObject
    {

    public:

        BatteryVoltage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class BatteryPercentage : public PropertyObject
    {

    public:

        BatteryPercentage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Status : public PropertyObject
    {

    public:

        Status(void) : PropertyObject("status", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Contact : public PropertyObject
    {

    public:

        Contact(void) : PropertyObject("contact", CLUSTER_ON_OFF, true) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class PowerOnStatus : public PropertyObject
    {

    public:

        PowerOnStatus(void) : PropertyObject("powerOnStatus", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Level : public PropertyObject
    {

    public:

        Level(void) : PropertyObject("level", CLUSTER_LEVEL_CONTROL) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class ColorHS : public PropertyObject
    {

    public:

        ColorHS(void) : PropertyObject("color", CLUSTER_COLOR_CONTROL) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_colorH, m_colorS;

    };

    class ColorXY : public PropertyObject
    {

    public:

        ColorXY(void) : PropertyObject("color", CLUSTER_COLOR_CONTROL) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_colorX, m_colorY;

    };

    class ColorTemperature : public PropertyObject
    {

    public:

        ColorTemperature(void) : PropertyObject("colorTemperature", CLUSTER_COLOR_CONTROL) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Illuminance : public PropertyObject
    {

    public:

        Illuminance(void) : PropertyObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Temperature : public PropertyObject
    {

    public:

        Temperature(void) : PropertyObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Humidity : public PropertyObject
    {

    public:

        Humidity(void) : PropertyObject("humidity", CLUSTER_RELATIVE_HUMIDITY) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Occupancy : public PropertyObject
    {

    public:

        Occupancy(void) : PropertyObject("occupancy", CLUSTER_OCCUPANCY_SENSING, true) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;
        void resetValue(void) override;

    };

    class Energy : public PropertyObject
    {

    public:

        Energy(void) : PropertyObject("energy", CLUSTER_SMART_ENERGY_METERING) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Voltage : public PropertyObject
    {

    public:

        Voltage(void) : PropertyObject("voltage", CLUSTER_ELECTRICAL_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Current : public PropertyObject
    {

    public:

        Current(void) : PropertyObject("current", CLUSTER_ELECTRICAL_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Power : public PropertyObject
    {

    public:

        Power(void) : PropertyObject("power", CLUSTER_ELECTRICAL_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Scene : public PropertyObject
    {

    public:

        Scene(void) : PropertyObject("scene", CLUSTER_SCENES, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class IdentifyAction : public PropertyObject
    {

    public:

        IdentifyAction(void) : PropertyObject("action", CLUSTER_IDENTIFY, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class LevelAction : public PropertyObject
    {

    public:

        LevelAction(void) : PropertyObject("action", CLUSTER_LEVEL_CONTROL, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class ColorAction : public PropertyObject
    {

    public:

        ColorAction(void) : PropertyObject("action", CLUSTER_COLOR_CONTROL, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };
}

namespace PropertiesIAS
{
    class ZoneStatus : public PropertyObject
    {

    public:

        ZoneStatus(const QString &name) : PropertyObject(name, CLUSTER_IAS_ZONE, true) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;
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
    class Temperature: public PropertyObject
    {

    public:

        Temperature(void) : PropertyObject("temperature", CLUSTER_ANALOG_INPUT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_buffer;

    };

    class CO2 : public PropertyObject
    {

    public:

        CO2(void) : PropertyObject("co2", CLUSTER_ANALOG_INPUT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_buffer;

    };

    class ChangePattern : public PropertyObject
    {

    public:

        ChangePattern(void) : PropertyObject("changePattern", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Pattern: public PropertyObject
    {

    public:

        Pattern(void) : PropertyObject("pattern", CLUSTER_ANALOG_INPUT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };
}

namespace PropertiesLUMI
{
    class Data : public PropertyObject
    {

    public:

        Data(void) : PropertyObject("lumiData", CLUSTER_LUMI) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        void parseData(quint16 dataPoint, quint8 dataType, const QByteArray &data, QMap <QString, QVariant> &map);

    };

    class BatteryVoltage : public PropertyObject
    {

    public:

        BatteryVoltage(void) : PropertyObject("battery", CLUSTER_BASIC) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Power : public PropertyObject
    {

    public:

        Power(void) : PropertyObject("power", CLUSTER_ANALOG_INPUT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class ButtonAction : public PropertyObject
    {

    public:

        ButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class CubeRotation : public PropertyObject
    {

    public:

        CubeRotation(void) : PropertyObject("action", CLUSTER_ANALOG_INPUT, true) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class CubeMovement : public PropertyObject
    {

    public:

        CubeMovement(void) :  PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };
}

namespace PropertiesTUYA
{
    class Data : public PropertyObject
    {

    public:

        Data(void) : PropertyObject("tuyaData", CLUSTER_TUYA_DATA) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    private:

        QVariant parseData(const tuyaHeaderStruct *header, const QByteArray &data);
        virtual void update(quint8 dataPoint, const QVariant &data) = 0;

    };

    class NeoSiren : public Data
    {

    public:

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
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class BacklightMode : public PropertyObject
    {

    public:

        BacklightMode(void) : PropertyObject("backlightMode", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class IndicatorMode : public PropertyObject
    {

    public:

        IndicatorMode(void) : PropertyObject("indicatorMode", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class SwitchMode : public PropertyObject
    {

    public:

        SwitchMode(void) : PropertyObject("switchMode", CLUSTER_TUYA_SWITCH_MODE) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class PowerOnStatus : public PropertyObject
    {

    public:

        PowerOnStatus(void) : PropertyObject("powerOnStatus", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };
}

namespace PropertiesOther
{
    class KonkeButtonAction : public PropertyObject
    {

    public:

        KonkeButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class LifeControlAirQuality : public PropertyObject
    {

    public:

        LifeControlAirQuality(void) : PropertyObject("lifeControlAirQuality", CLUSTER_TEMPERATURE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class PerenioSmartPlug : public PropertyObject
    {

    public:

        PerenioSmartPlug(void) : PropertyObject("perenioSmartPlug", CLUSTER_PERENIO) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };
}

#endif
