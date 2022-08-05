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

    PropertyObject(const QString &name, quint16 clusterId, bool invalidable = false) :
        m_name(name), m_clusterId(clusterId), m_invalidable(invalidable) {}

    virtual ~PropertyObject(void) {}
    virtual void parseAttribte(quint16, quint8, const QByteArray &) {}
    virtual void parseCommand(quint8, const QByteArray &) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QVariant value(void) { return m_value; }

    inline bool invalidable(void) { return m_invalidable; }
    inline void invalidate(void) { m_value = QVariant(); }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;
    QVariant m_value;
    bool m_invalidable;

    QVariant tuyaValue(const QByteArray &payload);
    quint8 toPercentage(double min, double max, double value);

};

namespace Properties
{
    class BatteryPercentage : public PropertyObject
    {

    public:

        BatteryPercentage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class BatteryVoltage : public PropertyObject
    {

    public:

        BatteryVoltage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Status : public PropertyObject
    {

    public:

        Status(void) : PropertyObject("status", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Level : public PropertyObject
    {

    public:

        Level(void) : PropertyObject("level", CLUSTER_LEVEL_CONTROL) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class AnalogCO2 : public PropertyObject
    {

    public:

        AnalogCO2(void) : PropertyObject("co2", CLUSTER_ANALOG_INPUT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_buffer;

    };

    class AnalogTemperature: public PropertyObject
    {

    public:

        AnalogTemperature(void) : PropertyObject("temperature", CLUSTER_ANALOG_INPUT) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_buffer;

    };

    class ColorHS : public PropertyObject
    {

    public:

        ColorHS(void) : PropertyObject("colorHS", CLUSTER_COLOR_CONTROL) {};
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

namespace PropertiesIKEA
{
    class BatteryPercentage : public PropertyObject
    {

    public:

        BatteryPercentage(void) : PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };
}

namespace PropertiesPTVO
{
    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };
}

namespace PropertiesLUMI
{
    class Dummy : public PropertyObject
    {

    public:

        Dummy(void) : PropertyObject("dummy", CLUSTER_LUMI) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class BatteryVoltage : public PropertyObject
    {

    public:

        BatteryVoltage(void) : PropertyObject("battery", CLUSTER_BASIC) {}
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

namespace PropertiesTUYA
{
    class Dummy : public PropertyObject
    {

    public:

        Dummy(void) : PropertyObject("dummy", CLUSTER_BASIC) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class PresenseSensor : public PropertyObject
    {

    public:

        PresenseSensor(void) : PropertyObject("tuya", CLUSTER_TUYA) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    private:

        QMap <QString, QVariant> m_map;

    };
}

#endif
