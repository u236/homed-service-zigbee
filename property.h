#ifndef PROPERTY_H
#define PROPERTY_H

#include <QMap>
#include <QSharedPointer>
#include <QVariant>

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

        BatteryVoltage(void);
        virtual ~BatteryVoltage(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class BatteryVoltageLUMI : public PropertyObject
    {

    public:

        BatteryVoltageLUMI(void);
        virtual ~BatteryVoltageLUMI(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class BatteryPercentage : public PropertyObject
    {

    public:

        BatteryPercentage(void);
        virtual ~BatteryPercentage(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class BatteryPercentageIKEA : public PropertyObject
    {

    public:

        BatteryPercentageIKEA(void);
        virtual ~BatteryPercentageIKEA(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Status : public PropertyObject
    {

    public:

        Status(void);
        virtual ~Status(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Level : public PropertyObject
    {

    public:

        Level(void);
        virtual ~Level(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class AnalogCO2 : public PropertyObject
    {

    public:

        AnalogCO2(void);
        virtual ~AnalogCO2(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_buffer;

    };

    class AnalogTemperature: public PropertyObject
    {

    public:

        AnalogTemperature(void);
        virtual ~AnalogTemperature(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_buffer;

    };

    class ColorTemperature : public PropertyObject
    {

    public:

        ColorTemperature(void);
        virtual ~ColorTemperature(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class ColorXY : public PropertyObject
    {

    public:

        ColorXY(void);
        virtual ~ColorXY(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    private:

        QVariant m_colorX, m_colorY;

    };

    class Illuminance : public PropertyObject
    {

    public:

        Illuminance(void);
        virtual ~Illuminance(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Temperature : public PropertyObject
    {

    public:

        Temperature(void);
        virtual ~Temperature(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Humidity : public PropertyObject
    {

    public:

        Humidity(void);
        virtual ~Humidity(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class Occupancy : public PropertyObject
    {

    public:

        Occupancy(void);
        virtual ~Occupancy(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class CubeMovement : public PropertyObject
    {

    public:

        CubeMovement(void);
        virtual ~CubeMovement(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class CubeRotation : public PropertyObject
    {

    public:

        CubeRotation(void);
        virtual ~CubeRotation(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class IdentifyAction : public PropertyObject
    {

    public:

        IdentifyAction(void);
        virtual ~IdentifyAction(void) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void);
        virtual ~SwitchAction(void) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class SwitchActionLUMI : public PropertyObject
    {

    public:

        SwitchActionLUMI(void);
        virtual ~SwitchActionLUMI(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class SwitchActionPTVO : public PropertyObject
    {

    public:

        SwitchActionPTVO(void);
        virtual ~SwitchActionPTVO(void) {}
        void parseAttribte(quint16 attributeId, quint8 dataType, const QByteArray &data) override;

    };

    class LevelAction : public PropertyObject
    {

    public:

        LevelAction(void);
        virtual ~LevelAction(void) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };
}

#endif
