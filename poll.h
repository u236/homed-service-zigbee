#ifndef POLL_H
#define POLL_H

#include <QSharedPointer>
#include "zcl.h"

class PollObject;
typedef QSharedPointer <PollObject> Poll;

class PollObject
{

public:

    PollObject(const QString &name, quint16 clusterId, QList <quint16> attributes) :
        m_name(name), m_clusterId(clusterId), m_attributes(attributes) {}

    PollObject(const QString &name, quint16 clusterId, quint16 attributeId) :
        m_name(name), m_clusterId(clusterId), m_attributes({attributeId}) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QList <quint16> &attributes(void) { return m_attributes; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;

    QList <quint16> m_attributes;

};

namespace Polls
{
    class Status : public PollObject
    {

    public:

        Status(void) : PollObject("status", CLUSTER_ON_OFF, 0x0000) {}

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
