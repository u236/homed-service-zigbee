#ifndef ACTION_H
#define ACTION_H

#include <QSharedPointer>

class ActionObject;
typedef QSharedPointer <ActionObject> Action;

class ActionObject
{

public:

    ActionObject(quint8 endPointId, quint16 clusterId, QString name) : m_endPointId(endPointId), m_clusterId(clusterId), m_name(name), m_transactionId(0) {}

    inline quint8 endPointId(void) { return m_endPointId; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QString name(void) { return m_name; }

    virtual QByteArray request(const QVariant &data) = 0;

protected:

    quint8 m_endPointId;
    quint16 m_clusterId;
    QString m_name;
    quint8 m_transactionId;

};

namespace Actions
{
    class Status : public ActionObject
    {

    public:

        Status(quint8 endPointId);
        virtual ~Status(void) {}
        QByteArray request(const QVariant &data) override;

    };

    class Level : public ActionObject
    {

    public:

        Level(quint8 endPointId);
        virtual ~Level(void) {}
        QByteArray request(const QVariant &data) override;

    };
}

#endif
