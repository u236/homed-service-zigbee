#ifndef POLL_H
#define POLL_H

#include <QSharedPointer>
#include "zcl.h"

class PollObject;
typedef QSharedPointer <PollObject> Poll;

class PollObject
{

public:

    PollObject(const QString &name, quint8 endPointId, quint16 clusterId, QList <quint16> attributes) :
        m_name(name), m_endPointId(endPointId), m_clusterId(clusterId), m_attributes(attributes) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline quint8 endPointId(void) { return m_endPointId; }
    inline QList <quint16> &attributes(void) { return m_attributes; }

protected:

    QString m_name;
    quint8 m_endPointId;
    quint16 m_clusterId;
    QList <quint16> m_attributes;

};

namespace Polls
{
    class Status : public PollObject
    {

    public:

        Status(quint8 endPointId = 1) :
            PollObject("status", endPointId, CLUSTER_ON_OFF, {0x0000}) {}

        virtual ~Status(void) {}

    };

    class Level : public PollObject
    {

    public:

        Level(quint8 endPointId = 1) :
            PollObject("level", endPointId, CLUSTER_LEVEL_CONTROL, {0x0000}) {}

        virtual ~Level(void) {}

    };

    class ColorHS : public PollObject
    {

    public:

        ColorHS(quint8 endPointId = 1) :
            PollObject("colorHS", endPointId, CLUSTER_COLOR_CONTROL, {0x0000, 0x0001}) {}

        virtual ~ColorHS(void) {}

    };

    class ColorXY : public PollObject
    {

    public:

        ColorXY(quint8 endPointId = 1) :
            PollObject("colorXY", endPointId, CLUSTER_COLOR_CONTROL, {0x0003, 0x0004}) {}

        virtual ~ColorXY(void) {}

    };

    class ColorTemperature : public PollObject
    {

    public:

        ColorTemperature(quint8 endPointId = 1) :
            PollObject("colorTemperature", endPointId, CLUSTER_COLOR_CONTROL, {0x0003, 0x0004}) {}

        virtual ~ColorTemperature(void) {}

    };
}

#endif
