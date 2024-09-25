#ifndef POLL_H
#define POLL_H

#include <QSharedPointer>
#include "zcl.h"

class PollObject;
typedef QSharedPointer <PollObject> Poll;

class PollObject
{

public:

    PollObject(const QString &name, quint16 clusterId, quint16 attributeId) :
        m_name(name), m_clusterId(clusterId), m_attributeId(attributeId) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline quint16 attributeId(void) { return m_attributeId; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId, m_attributeId;

};

namespace Polls
{
    class Status : public PollObject
    {

    public:

        Status(void) : PollObject("status", CLUSTER_ON_OFF, 0x0000) {}

    };

    class TargetTemperature : public PollObject
    {

    public:

        TargetTemperature(void) : PollObject("targetTemperature", CLUSTER_THERMOSTAT, 0x0012) {}

    };

    class SystemMode : public PollObject
    {

    public:

        SystemMode(void) : PollObject("systemMode", CLUSTER_THERMOSTAT, 0x001C) {}

    };

    class Energy : public PollObject
    {

    public:

        Energy(void) : PollObject("energy", CLUSTER_SMART_ENERGY_METERING, 0x0000) {}

    };

    class Voltage : public PollObject
    {

    public:

        Voltage(void) : PollObject("voltage", CLUSTER_ELECTRICAL_MEASUREMENT, 0x0505) {}

    };

    class Current : public PollObject
    {

    public:

        Current(void) : PollObject("current", CLUSTER_ELECTRICAL_MEASUREMENT, 0x0508) {}

    };

    class Power : public PollObject
    {

    public:

        Power(void) : PollObject("power", CLUSTER_ELECTRICAL_MEASUREMENT, 0x050B) {}

    };
}

#endif
