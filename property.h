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
    Attribute attribute(quint16 attributeId);

private:

    QMap <quint16, Attribute> m_attributes;

};

class PropertyObject
{

public:

    PropertyObject(const QString &name, quint16 clusterId) : m_clusterId(clusterId), m_name(name) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QVariant value(void) { return m_value; }

    virtual void convert(const Cluster &cluster) = 0;

protected:

    quint16 m_clusterId;
    QString m_name;
    QVariant m_value;

};

namespace Properties
{
    class Status : public PropertyObject
    {

    public:

        Status(void);
        virtual ~Status(void) {}
        void convert(const Cluster &cluster) override;

    };

    class Level : public PropertyObject
    {

    public:

        Level(void);
        virtual ~Level(void) {}
        void convert(const Cluster &cluster) override;

    };

    class ColorTemperature : public PropertyObject
    {

    public:

        ColorTemperature(void);
        virtual ~ColorTemperature(void) {}
        void convert(const Cluster &cluster) override;

    };

    class Illuminance : public PropertyObject
    {

    public:

        Illuminance(void);
        virtual ~Illuminance(void) {}
        void convert(const Cluster &cluster) override;

    };

    class Temperature : public PropertyObject
    {

    public:

        Temperature(void);
        virtual ~Temperature(void) {}
        void convert(const Cluster &cluster) override;

    };

    class Humidity : public PropertyObject
    {

    public:

        Humidity(void);
        virtual ~Humidity(void) {}
        void convert(const Cluster &cluster) override;

    };

    class Occupancy : public PropertyObject
    {

    public:

        Occupancy(void);
        virtual ~Occupancy(void) {}
        void convert(const Cluster &cluster) override;

    };
}

#endif
