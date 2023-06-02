#ifndef METAOBJECT_H
#define METAOBJECT_H

#include <QVariant>

class MetaObject
{

public:

    MetaObject(const QString name) : m_name(name), m_parent(nullptr) {}

    inline QString name(void) { return m_name; }
    inline void setName(const QString &value) { m_name = value; }
    inline void setParent(QObject *value) { m_parent = value; }

    QVariant option(const QString &name = QString(), const QVariant &defaultValue = QVariant());

protected:

    QString m_name;
    QObject *m_parent;

    quint8 endpointId(void);
    quint8 version(void);

    QString manufacturerName(void);
    QString modelName(void);

};

#endif
