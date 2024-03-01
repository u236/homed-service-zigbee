#ifndef PROPERTIES_OTHER_H
#define PROPERTIES_OTHER_H

#include "property.h"
#include "zcl.h"

namespace PropertiesByun
{
    class Sensor : public PropertyObject
    {

    public:

        Sensor(const QString &name, QList <quint16> clusters) : PropertyObject(name, clusters) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class GasSensor : public Sensor
    {

    public:

        GasSensor(void) : Sensor("gas", {CLUSTER_BYUN, CLUSTER_IAS_ZONE}) {}

    };

    class SmokeSensor : public Sensor
    {

    public:

        SmokeSensor(void) : Sensor("smoke", {CLUSTER_PH_MEASUREMENT, CLUSTER_IAS_ZONE}) {}

    };
}

namespace PropertiesIKEA
{
    class Occupancy : public PropertyObject
    {

    public:

        Occupancy(void) : PropertyObject("occupancy", CLUSTER_ON_OFF) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;
        void resetValue(void) override;

    };

    class StatusAction : public PropertyObject
    {
    public:

        StatusAction(void) : PropertyObject("action", CLUSTER_ON_OFF) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };

    class ArrowAction : public PropertyObject
    {

    public:

        ArrowAction(void) : PropertyObject("action", CLUSTER_SCENES) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };
}

namespace PropertiesCustom
{
    class Command : public PropertyObject
    {

    public:

        Command(const QString &name, quint16 clusterId) : PropertyObject(name, clusterId) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };

    class Attribute : public PropertyObject
    {

    public:

        Attribute(const QString &name, const QString &type, quint16 clusterId, quint16 attributeId, quint8 dataType, double divider) :
            PropertyObject(name, clusterId), m_type(type), m_attributeId(attributeId), m_dataType(dataType), m_divider(divider) {}

        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    private:

        QString m_type;
        quint16 m_attributeId;
        quint8 m_dataType;
        double m_divider;

    };
}

#endif
