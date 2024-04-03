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

    PropertyObject(const QString &name, QList <quint16> clusters = {}) :
        AbstractMetaObject(name), m_clusters(clusters), m_multiple(false), m_timeout(0), m_time(0), m_transactionId(0) {}

    PropertyObject(const QString &name, quint16 clusterId) :
        AbstractMetaObject(name), m_clusters({clusterId}), m_multiple(false), m_timeout(0), m_time(0), m_transactionId(0) {}

    virtual ~PropertyObject(void) {}
    virtual void parseAttribte(quint16, quint16, const QByteArray &) {}
    virtual void parseCommand(quint16, quint8, const QByteArray &) {}
    virtual void resetValue(void) {}

    inline QList <quint16> &clusters(void) { return m_clusters; }

    inline bool multiple(void) { return m_multiple; }
    inline void setMultiple(bool value) { m_multiple = value; }

    inline quint32 timeout(void) { return m_timeout; }
    inline void setTimeout(quint32 value) { m_timeout = value; }

    inline qint64 time(void) { return m_time; }
    inline void setTime(qint64 value) { m_time = value; }

    inline quint8 transactionId(void) { return m_transactionId; }
    inline void setTransactionId(quint8 value) { m_transactionId = value; }

    inline QVariant value(void) { return m_value; }
    inline void setValue(const QVariant &value) { m_value = value; }
    inline void clearValue(void) { m_value = QVariant(); }

    static void registerMetaTypes(void);

protected:

    QList <quint16> m_clusters;
    bool m_multiple;

    quint32 m_timeout;
    qint64 m_time;

    quint8 m_transactionId;
    QVariant m_value;

    quint8 percentage(double min, double max, double value);
    QVariant enumValue(const QString &name, int index);

};

class EnumProperty : public PropertyObject
{

public:

    EnumProperty(const QString &name, quint16 clusterId, quint16 attributeId) :
        PropertyObject(name, clusterId), m_attributeId(attributeId) {}

    void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

private:

    quint16 m_attributeId;

};

#endif
