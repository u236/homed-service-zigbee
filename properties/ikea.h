#ifndef PROPERTIES_IKEA_H
#define PROPERTIES_IKEA_H

#include "property.h"
#include "zcl.h"

namespace PropertiesIKEA
{
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

#endif


