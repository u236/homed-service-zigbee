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
        m_name(name), m_clusterId(clusterId), m_attributes({attributeId}) {}

    PollObject(const QString &name, quint16 clusterId, QList <quint16> attributes) :
        m_name(name), m_clusterId(clusterId), m_attributes(attributes) {}

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

    class Level : public PollObject
    {

    public:

        Level(void) : PollObject("level", CLUSTER_LEVEL_CONTROL, 0x0000) {}

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

        ColorTemperature(void) : PollObject("colorTemperature", CLUSTER_COLOR_CONTROL, 0x0007) {}

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

namespace PollsOther
{
    class LumiPresenceSensor : public PollObject
    {

    public:

        LumiPresenceSensor(void) : PollObject("presenceSensor", CLUSTER_LUMI, {0x010C, 0x0144, 0x0146}) {}

    };

    class PerenioSmartPlug : public PollObject
    {

    public:

        PerenioSmartPlug(void) : PollObject("smartPlug", CLUSTER_PERENIO, {0x0000, 0x0001, 0x0004, 0x0005, 0x000B, 0x000F}) {}

    };
}

#endif
