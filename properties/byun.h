#ifndef PROPERTIES_BYUN_H
#define PROPERTIES_BYUN_H

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

#endif
