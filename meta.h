#ifndef METAOBJECT_H
#define METAOBJECT_H

#include <QVariant>

class MetaObject
{

public:

    MetaObject(const QString name) : m_name(name), m_parent(nullptr) {}

    inline QString name(void) { return m_name; }
    inline void setParent(QObject *value) { m_parent = value; }

    QVariant endpointOption(const QString &name = QString());
    quint8 endpointId(void);

protected:

    QString m_name;
    QObject *m_parent;

    quint8 deviceVersion(void);
    QString deviceManufacturerName(void);
    QString deviceModelName(void);

};

#endif
