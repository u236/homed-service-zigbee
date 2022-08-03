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
        m_name(name), m_clusterId(clusterId), m_endpointId(1), m_attributes(attributes) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QList <quint16> &attributes(void) { return m_attributes; }

    inline quint8 endpointId(void) { return m_endpointId; }
    inline void setEndpointId(quint8 value) { m_endpointId = value; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;
    quint8 m_endpointId;

    QList <quint16> m_attributes;

};

namespace Polls
{
    class Status : public PollObject
    {

    public:

        Status(void) : PollObject("status", CLUSTER_ON_OFF, {0x0000}) {}

    };

    class Level : public PollObject
    {

    public:

        Level(void) : PollObject("level", CLUSTER_LEVEL_CONTROL, {0x0000}) {}

    };

    class ColorHS : public PollObject
    {

    public:

        ColorHS(void) : PollObject("colorHS", CLUSTER_COLOR_CONTROL, {0x0000, 0x0001}) {}

    };

    class ColorXY : public PollObject
    {

    public:

        ColorXY(void) : PollObject("colorXY", CLUSTER_COLOR_CONTROL, {0x0003, 0x0004}) {}

    };

    class ColorTemperature : public PollObject
    {

    public:

        ColorTemperature(void) : PollObject("colorTemperature", CLUSTER_COLOR_CONTROL, {0x0003, 0x0004}) {}

    };

    class Energy : public PollObject
    {

    public:

        Energy(void) : PollObject("energy", CLUSTER_SMART_ENERGY_METERING, {0x0301, 0x0302, 0x0000}) {}

    };

    class Power : public PollObject
    {

    public:

        Power(void) : PollObject("power", CLUSTER_ELECTRICAL_MEASUREMENT, {0x0604, 0x0605, 0x050B}) {}

    };
}

#endif
