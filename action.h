#ifndef ACTION_H
#define ACTION_H

#include <QSharedPointer>
#include <QVariant>
#include "zcl.h"

#pragma pack(push, 1)

struct levelStruct
{
    quint8  level;
    quint16 time;
};

struct colorXYStruct
{
    quint16 colorX;
    quint16 colorY;
    quint16 time;
};

struct colorHSStruct
{
    quint8  colorH;
    quint8  colorS;
    quint16 time;
};

struct colorTemperatureStruct
{
    quint16 temperature;
    quint16 time;
};

#pragma pack(pop)

class ActionObject;
typedef QSharedPointer <ActionObject> Action;

class ActionObject
{

public:

    ActionObject(const QString &name, quint16 clusterId) :
        m_name(name), m_clusterId(clusterId), m_transactionId(0) {}

    virtual ~ActionObject(void) {}
    virtual QByteArray request(const QVariant &data) = 0;

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId;
    quint8 m_transactionId;

};

namespace Actions
{
    class Status : public ActionObject
    {

    public:

        Status(void) : ActionObject("status", CLUSTER_ON_OFF) {}
        QByteArray request(const QVariant &data) override;

    };

    class Level : public ActionObject
    {

    public:

        Level(void) : ActionObject("level", CLUSTER_LEVEL_CONTROL) {}
        QByteArray request(const QVariant &data) override;

    };

    class ColorHS : public ActionObject
    {

    public:

        ColorHS(void) : ActionObject("colorHS", CLUSTER_COLOR_CONTROL) {}
        QByteArray request(const QVariant &data) override;

    };

    class ColorXY : public ActionObject
    {

    public:

        ColorXY(void) : ActionObject("colorXY", CLUSTER_COLOR_CONTROL) {}
        QByteArray request(const QVariant &data) override;

    };

    class ColorTemperature : public ActionObject
    {

    public:

        ColorTemperature(void) : ActionObject("colorTemperature", CLUSTER_COLOR_CONTROL) {}
        QByteArray request(const QVariant &data) override;

    };
}

namespace ActionsPTVO
{
    class Pattern : public ActionObject
    {

    public:

        Pattern(void) : ActionObject("pattern", CLUSTER_ANALOG_INPUT) {}
        QByteArray request(const QVariant &data) override;

    };
}

namespace ActionsLUMI
{
    class Request
    {

    protected:

        QByteArray makeRequest(quint8 transactionId, quint16 attributeId, quint8 dataType, void *data);

    };

    class Sensitivity : public Request, public ActionObject
    {

    public:

        Sensitivity(void) : ActionObject("sensitivity", CLUSTER_LUMI) {}
        QByteArray request(const QVariant &data) override;

    };

    class Mode : public Request, public ActionObject
    {

    public:

        Mode(void) : ActionObject("mode", CLUSTER_LUMI) {}
        QByteArray request(const QVariant &data) override;

    };

    class Distance : public Request, public ActionObject
    {

    public:

        Distance(void) : ActionObject("distance", CLUSTER_LUMI) {}
        QByteArray request(const QVariant &data) override;

    };
}

namespace ActionsTUYA
{
    class Request
    {

    protected:

        QByteArray makeRequest(quint8 transactionId, quint8 dataPoint, quint8 dataType, void *data);

    };

    class Sensitivity : public Request, public ActionObject
    {

    public:

        Sensitivity(void) : ActionObject("sensitivity", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class RangeMin :  public Request, public ActionObject
    {

    public:

        RangeMin(void) : ActionObject("rangeMin", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class RangeMax :  public Request, public ActionObject
    {

    public:

        RangeMax(void) : ActionObject("rangeMax", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class DetectionDelay : public Request, public ActionObject
    {

    public:

        DetectionDelay(void) : ActionObject("detectionDelay", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };
}

#endif
