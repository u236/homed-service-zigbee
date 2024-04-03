#ifndef ACTION_H
#define ACTION_H

#include <QSharedPointer>
#include <QVariant>
#include "property.h"

class ActionObject;
typedef QSharedPointer <ActionObject> Action;

class ActionObject : public AbstractMetaObject
{

public:

    ActionObject(const QString &name, quint16 clusterId, quint16 manufacturerCode = 0, QList <quint16> attributes = {}) :
        AbstractMetaObject(name), m_clusterId(clusterId), m_manufacturerCode(manufacturerCode), m_transactionId(0), m_properyUpdated(false), m_attributes(attributes) {}

    ActionObject(const QString &name, quint16 clusterId, quint16 manufacturerCode, quint16 attributeId) :
        AbstractMetaObject(name), m_clusterId(clusterId), m_manufacturerCode(manufacturerCode), m_transactionId(0), m_properyUpdated(false), m_attributes({attributeId}) {}

    ActionObject(const QString &name, quint16 clusterId, quint16 manufacturerCode, QList <QString> actions) :
        AbstractMetaObject(name), m_clusterId(clusterId), m_manufacturerCode(manufacturerCode), m_transactionId(0), m_properyUpdated(false), m_actions(actions) {}

    virtual ~ActionObject(void) {}
    virtual QByteArray request(const QString &name, const QVariant &data) = 0;

    inline quint16 clusterId(void) { return m_clusterId; }
    inline quint16 manufacturerCode(void) { return m_manufacturerCode; }
    inline bool propertyUpdated(void) { return m_properyUpdated; }

    inline QList <quint16> &attributes(void) { return m_attributes; }
    inline QList <QString> &actions(void) { return m_actions; }

    static void registerMetaTypes(void);

protected:

    quint16 m_clusterId, m_manufacturerCode;
    quint8 m_transactionId;
    bool m_properyUpdated;

    QList <quint16> m_attributes;
    QList <QString> m_actions;

    Property endpointProperty(const QString &name = QString());
    QByteArray writeAttribute(quint8 dataType, void *value, size_t length);
    qint8 listIndex(const QList <QString> &list, const QVariant &value);
    int enumIndex(const QString name, const QVariant &value);
};

class EnumAction : public ActionObject
{

public:

    EnumAction(const QString &name, quint16 clusterId, quint16 manufacturerCode, quint16 attributeId, quint8 dataType) :
        ActionObject(name, clusterId, manufacturerCode, attributeId), m_dataType(dataType) {}

    QByteArray request(const QString &name, const QVariant &data) override;

private:

    quint8 m_dataType;

};

#endif
