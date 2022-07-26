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
        m_name(name), m_clusterId(clusterId), m_endPointId(1), m_attributes(attributes) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QList <quint16> &attributes(void) { return m_attributes; }

    inline quint8 endPointId(void) { return m_endPointId; }
    inline void setEndPointId(quint8 value) { m_endPointId = value; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;
    quint8 m_endPointId;

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
}

Q_DECLARE_METATYPE(Polls::Status)
Q_DECLARE_METATYPE(Polls::Level)
Q_DECLARE_METATYPE(Polls::ColorHS)
Q_DECLARE_METATYPE(Polls::ColorXY)
Q_DECLARE_METATYPE(Polls::ColorTemperature)

#endif
