#ifndef PROPERTIES_IAS_H
#define PROPERTIES_IAS_H

#include "property.h"
#include "zcl.h"

namespace PropertiesIAS
{
    class ZoneStatus : public PropertyObject
    {

    public:

        ZoneStatus(const QString &name) : PropertyObject(name, CLUSTER_IAS_ZONE) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;
        void resetValue(void) override;

    };

    class Warning : public PropertyObject
    {

    public:

        Warning(void) : PropertyObject("warning", CLUSTER_IAS_WD) {}
        void resetValue(void) override;

    };

    class Alarm : public ZoneStatus
    {

    public:

        Alarm(void) : ZoneStatus("alarm") {}

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

    class Vibration : public ZoneStatus
    {

    public:

        Vibration(void) : ZoneStatus("vibration") {}

    };

    class Rain : public ZoneStatus
    {

    public:

        Rain(void) : ZoneStatus("rain") {}

    };

    class ContolAction : public PropertyObject
    {

    public:

        ContolAction(const QString &name = "action") : PropertyObject(name, CLUSTER_IAS_ACE) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };
}

#endif
