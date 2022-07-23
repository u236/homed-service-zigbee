#ifndef ACTION_H
#define ACTION_H

#include <QSharedPointer>

#pragma pack(push, 1)

struct levelStruct
{
    quint8  level;
    quint16 time;
};

struct colorTemperatureStruct
{
    quint16 temperature;
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

#pragma pack(pop)

class ActionObject;
typedef QSharedPointer <ActionObject> Action;

class ActionObject
{

public:

    ActionObject(const QString &name, quint16 clusterId, quint8 endPointId) :
        m_name(name), m_clusterId(clusterId), m_endPointId(endPointId), m_transactionId(0) {}

    inline QString name(void) { return m_name; }
    inline quint8 endPointId(void) { return m_endPointId; }
    inline quint16 clusterId(void) { return m_clusterId; }

    virtual QByteArray request(const QVariant &data) = 0;

protected:

    QString m_name;
    quint16 m_clusterId;
    quint8 m_endPointId, m_transactionId;

};

namespace Actions
{
    class Status : public ActionObject
    {

    public:

        Status(quint8 endPointId = 1);
        virtual ~Status(void) {}
        QByteArray request(const QVariant &data) override;

    };

    class Level : public ActionObject
    {

    public:

        Level(quint8 endPointId = 1);
        virtual ~Level(void) {}
        QByteArray request(const QVariant &data) override;

    };

    class ColorTemperature : public ActionObject
    {

    public:

        ColorTemperature(quint8 endPointId = 1);
        virtual ~ColorTemperature(void) {}
        QByteArray request(const QVariant &data) override;

    };

    class ColorHS : public ActionObject
    {

    public:

        ColorHS(quint8 endPointId = 1);
        virtual ~ColorHS(void) {}
        QByteArray request(const QVariant &data) override;

    };

    class ColorXY : public ActionObject
    {

    public:

        ColorXY(quint8 endPointId = 1);
        virtual ~ColorXY(void) {}
        QByteArray request(const QVariant &data) override;

    };
}

#endif
