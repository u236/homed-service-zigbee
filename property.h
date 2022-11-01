#ifndef PROPERTY_H
#define PROPERTY_H

#include <QMap>
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

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QVariant value(void) { return m_value; }

    inline bool singleShot(void) { return m_singleShot; }
    inline void clear(void) { m_value = QVariant(); }

    inline void setVersion(quint8 value) { m_version = value; }
    inline void setModel(const QString &value) { m_model = value; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;
    QVariant m_value;
    bool m_singleShot;

    quint8 m_version;
    QString m_model;

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

    class BatteryUndivided : public PropertyObject
    {

    public:

        BatteryUndivided(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}
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

        Contact(void) : PropertyObject("contact", CLUSTER_ON_OFF) {}
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

        ColorHS(void) : PropertyObject("colorHS", CLUSTER_COLOR_CONTROL) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_colorH, m_colorS;

    };

    class ColorXY : public PropertyObject
    {

    public:

        ColorXY(void) : PropertyObject("colorXY", CLUSTER_COLOR_CONTROL) {}
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

    };

    class Energy : public PropertyObject
    {

    public:

        Energy(void) : PropertyObject("energy", CLUSTER_SMART_ENERGY_METERING), m_multiplier(0), m_divider(0) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        quint32 m_multiplier, m_divider;

    };

    class Power : public PropertyObject
    {

    public:

        Power(void) : PropertyObject("power", CLUSTER_ELECTRICAL_MEASUREMENT), m_multiplier(0), m_divider(0) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        quint16 m_multiplier, m_divider;

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
}

namespace PropertiesIAS
{
    class ZoneStatus : public PropertyObject
    {

    public:

        ZoneStatus(const QString &name) : PropertyObject(name, CLUSTER_IAS_ZONE, true){}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    private:

        QMap <QString, QVariant> m_map;

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

namespace PropertiesLifeControl
{
    class AirQuality : public PropertyObject
    {

    public:

        AirQuality(void) : PropertyObject(QString(), CLUSTER_TEMPERATURE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QMap <QString, QVariant> m_map;

    };
}

namespace PropertiesLUMI
{
    class Data : public PropertyObject
    {

    public:

        Data(void) : PropertyObject(QString(), CLUSTER_LUMI) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QMap <QString, QVariant> m_map;
        void parseData(quint16 dataPoint, quint8 dataType, const QByteArray &data);

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

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) : PropertyObject("action", CLUSTER_ON_OFF, true) {}
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

namespace PropertiesPTVO
{
    class CO2 : public PropertyObject
    {

    public:

        CO2(void) : PropertyObject("co2", CLUSTER_ANALOG_INPUT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_buffer;

    };

    class Temperature: public PropertyObject
    {

    public:

        Temperature(void) : PropertyObject("temperature", CLUSTER_ANALOG_INPUT) {}
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

namespace PropertiesTUYA
{
    class Data : public PropertyObject
    {

    public:

        Data(void) : PropertyObject(QString(), CLUSTER_TUYA) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    protected:

        QMap <QString, QVariant> m_map;

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

    class PowerOnBehavior : public PropertyObject
    {

    public:

        PowerOnBehavior(void) : PropertyObject("powerOnBehavior", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class SwitchType : public PropertyObject
    {

    public:

        SwitchType(void) : PropertyObject("switchType", CLUSTER_TUYA_SWITCH_TYPE) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Unknown : public PropertyObject
    {

    public:

        Unknown(void) : PropertyObject(QString(), CLUSTER_TUYA_UNKNOWN) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };
}

#endif
