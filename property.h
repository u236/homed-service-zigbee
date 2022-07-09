#ifndef PROPERTY_H
#define PROPERTY_H

#include <QMap>
#include <QSharedPointer>
#include <QVariant>

class AttributeObject;
typedef QSharedPointer <AttributeObject> Attribute;

class ClusterObject;
typedef QSharedPointer <ClusterObject> Cluster;

class PropertyObject;
typedef QSharedPointer <PropertyObject> Property;

class AttributeObject
{

public:

    AttributeObject(void) : m_dataType(0) {}

    inline quint8 dataType(void) { return m_dataType; }
    inline void setDataType(quint8 value) { m_dataType = value; }

    inline QByteArray data(void) { return m_data; }
    inline void setData(const QByteArray &value) { m_data = value; }

private:

    quint8 m_dataType;
    QByteArray m_data;

};

class ClusterObject
{

public:

    ClusterObject(void) {}

    inline quint8 commandId(void) { return m_commandId; }
    inline void setCommandId(quint8 value) { m_commandId = value; }

    inline QByteArray commandData(void) { return m_commandData; }
    inline void setCommandData(const QByteArray &value) { m_commandData = value; }

    Attribute attribute(quint16 attributeId);

private:

    quint8 m_commandId;
    QByteArray m_commandData;

    QMap <quint16, Attribute> m_attributes;

};

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

    virtual void parse(const Cluster &cluster, quint16 attributeId = 0) = 0;

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
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class BatteryVoltageLUMI : public PropertyObject
    {

    public:

        BatteryVoltageLUMI(void);
        virtual ~BatteryVoltageLUMI(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class BatteryPercentage : public PropertyObject
    {

    public:

        BatteryPercentage(void);
        virtual ~BatteryPercentage(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class BatteryPercentageIKEA : public PropertyObject
    {

    public:

        BatteryPercentageIKEA(void);
        virtual ~BatteryPercentageIKEA(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class Status : public PropertyObject
    {

    public:

        Status(void);
        virtual ~Status(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class Level : public PropertyObject
    {

    public:

        Level(void);
        virtual ~Level(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class AnalogCO2 : public PropertyObject
    {

    public:

        AnalogCO2(void);
        virtual ~AnalogCO2(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class AnalogTemperature: public PropertyObject
    {

    public:

        AnalogTemperature(void);
        virtual ~AnalogTemperature(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class ColorTemperature : public PropertyObject
    {

    public:

        ColorTemperature(void);
        virtual ~ColorTemperature(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class Illuminance : public PropertyObject
    {

    public:

        Illuminance(void);
        virtual ~Illuminance(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class Temperature : public PropertyObject
    {

    public:

        Temperature(void);
        virtual ~Temperature(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class Humidity : public PropertyObject
    {

    public:

        Humidity(void);
        virtual ~Humidity(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class Occupancy : public PropertyObject
    {

    public:

        Occupancy(void);
        virtual ~Occupancy(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class CubeAction : public PropertyObject
    {

    public:

        CubeAction(void);
        virtual ~CubeAction(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class CubeRotation : public PropertyObject
    {

    public:

        CubeRotation(void);
        virtual ~CubeRotation(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void);
        virtual ~SwitchAction(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class SwitchActionLUMI : public PropertyObject
    {

    public:

        SwitchActionLUMI(void);
        virtual ~SwitchActionLUMI(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class SwitchActionPTVO : public PropertyObject
    {

    public:

        SwitchActionPTVO(void);
        virtual ~SwitchActionPTVO(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };

    class LevelAction : public PropertyObject
    {

    public:

        LevelAction(void);
        virtual ~LevelAction(void) {}
        void parse(const Cluster &cluster, quint16 attributeId) override;

    };
}

#endif
