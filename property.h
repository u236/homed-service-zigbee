#ifndef PROPERTY_H
#define PROPERTY_H

#include <QSharedPointer>
#include <QVariant>
#include "endpoint.h"

class PropertyObject;
typedef QSharedPointer <PropertyObject> Property;

class PropertyObject : public AbstractMetaObject
{

public:

    PropertyObject(const QString &name, quint16 clusterId) :
        AbstractMetaObject(name), m_clusterId(clusterId), m_multiple(false), m_timeout(0), m_time(0), m_transactionId(0) {}

    virtual ~PropertyObject(void) {}
    virtual void parseAttribte(quint16, const QByteArray &) {}
    virtual void parseCommand(quint8, const QByteArray &) {}
    virtual void resetValue(void) {}

    inline quint16 clusterId(void) { return m_clusterId; }

    inline bool multiple(void) { return m_multiple; }
    inline void setMultiple(bool value) { m_multiple = value; }

    inline quint32 timeout(void) { return m_timeout; }
    inline void setTimeout(quint32 value) { m_timeout = value; }

    inline qint64 time(void) { return m_time; }
    inline void setTime(qint64 value) { m_time = value; }

    inline QMap <QString, QVariant> &meta(void) { return m_meta; }

    inline quint8 transactionId(void) { return m_transactionId; }
    inline void setTransactionId(quint8 value) { m_transactionId = value; }

    inline QVariant value(void) { return m_value; }
    inline void setValue(const QVariant &value) { m_value = value; }
    inline void clearValue(void) { m_value = QVariant(); }

    static void registerMetaTypes(void);

protected:

    quint16 m_clusterId;
    bool m_multiple;

    quint32 m_timeout;
    qint64 m_time;

    QMap <QString, QVariant> m_meta;
    quint8 m_transactionId;
    QVariant m_value;

    quint8 percentage(double min, double max, double value);

};

#endif
