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
        m_clusterId(clusterId), m_name(name), m_invalidable(invalidable) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QVariant value(void) { return m_value; }

    inline bool invalidable(void) { return m_invalidable; }
    inline void invalidate(void) { m_value = QVariant(); }

    virtual void parseAttribte(quint16, quint8, const QByteArray &) {}
    virtual void parseCommand(quint8, const QByteArray &) {}

protected:

    quint16 m_clusterId;
    QString m_name;
    QVariant m_value;
    bool m_invalidable;

};

namespace Properties
{
    class BatteryVoltage : public PropertyObject
    {

    public:

        BatteryVoltage(void) :
            PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}

        virtual ~BatteryVoltage(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class BatteryVoltageLUMI : public PropertyObject
    {

    public:

        BatteryVoltageLUMI(void) :
            PropertyObject("battery", CLUSTER_BASIC) {}

        virtual ~BatteryVoltageLUMI(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class BatteryPercentage : public PropertyObject
    {

    public:

        BatteryPercentage(void) :
            PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}

        virtual ~BatteryPercentage(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class BatteryPercentageIKEA : public PropertyObject
    {

    public:

        BatteryPercentageIKEA(void) :
            PropertyObject("battery", CLUSTER_POWER_CONFIGURATION) {}

        virtual ~BatteryPercentageIKEA(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Status : public PropertyObject
    {

    public:

        Status(void) :
             PropertyObject("status", CLUSTER_ON_OFF) {}

        virtual ~Status(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Level : public PropertyObject
    {

    public:

        Level(void) :
            PropertyObject("level", CLUSTER_LEVEL_CONTROL) {}

        virtual ~Level(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class AnalogCO2 : public PropertyObject
    {

    public:

        AnalogCO2(void) :
            PropertyObject("co2", CLUSTER_ANALOG_INPUT) {}

        virtual ~AnalogCO2(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_buffer;

    };

    class AnalogTemperature: public PropertyObject
    {

    public:

        AnalogTemperature(void) :
            PropertyObject("temperature", CLUSTER_ANALOG_INPUT) {}

        virtual ~AnalogTemperature(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_buffer;

    };

    class ColorHS : public PropertyObject
    {

    public:

        ColorHS(void) :
            PropertyObject("colorHS", CLUSTER_COLOR_CONTROL) {};

        virtual ~ColorHS(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_colorH, m_colorS;

    };

    class ColorXY : public PropertyObject
    {

    public:

        ColorXY(void) :
            PropertyObject("colorXY", CLUSTER_COLOR_CONTROL) {}

        virtual ~ColorXY(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_colorX, m_colorY;

    };

    class ColorTemperature : public PropertyObject
    {

    public:

        ColorTemperature(void) :
            PropertyObject("colorTemperature", CLUSTER_COLOR_CONTROL) {}

        virtual ~ColorTemperature(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Illuminance : public PropertyObject
    {

    public:

        Illuminance(void) :
            PropertyObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT) {}

        virtual ~Illuminance(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Temperature : public PropertyObject
    {

    public:

        Temperature(void) :
            PropertyObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT) {}

        virtual ~Temperature(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Humidity : public PropertyObject
    {

    public:

        Humidity(void) :
            PropertyObject("humidity", CLUSTER_RELATIVE_HUMIDITY) {}

        virtual ~Humidity(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Occupancy : public PropertyObject
    {

    public:

        Occupancy(void) :
            PropertyObject("occupancy", CLUSTER_OCCUPANCY_SENSING, true) {}

        virtual ~Occupancy(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class CubeMovement : public PropertyObject
    {

    public:

        CubeMovement(void) :
            PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}

        virtual ~CubeMovement(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class CubeRotation : public PropertyObject
    {

    public:

        CubeRotation(void) :
            PropertyObject("action", CLUSTER_ANALOG_INPUT, true) {}

        virtual ~CubeRotation(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class IdentifyAction : public PropertyObject
    {

    public:

        IdentifyAction(void) :
            PropertyObject("action", CLUSTER_IDENTIFY, true) {}

        virtual ~IdentifyAction(void) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) :
            PropertyObject("action", CLUSTER_ON_OFF, true) {}

        virtual ~SwitchAction(void) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class SwitchActionLUMI : public PropertyObject
    {

    public:

        SwitchActionLUMI(void) :
            PropertyObject("action", CLUSTER_ON_OFF, true) {}

        virtual ~SwitchActionLUMI(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class SwitchActionPTVO : public PropertyObject
    {

    public:

        SwitchActionPTVO(void) :
            PropertyObject("action", CLUSTER_MULTISTATE_INPUT, true) {}

        virtual ~SwitchActionPTVO(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class LevelAction : public PropertyObject
    {

    public:

        LevelAction(void) :
            PropertyObject("action", CLUSTER_LEVEL_CONTROL, true) {}

        virtual ~LevelAction(void) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };
}

#endif
