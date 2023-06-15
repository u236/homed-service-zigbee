#include "device.h"
#include "meta.h"

QVariant MetaObject::option(const QString &name, const QVariant &defaultValue)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    QString optionName = name.isEmpty() ? m_name : name;
    QVariant value;

    if (!endpoint || endpoint->device().isNull())
        return value;

    value = endpoint->device()->options().value(QString("%1_%2").arg(optionName, QString::number(endpoint->id())));
    return value.isValid() ? value : endpoint->device()->options().value(optionName, defaultValue);
}

quint8 MetaObject::endpointId(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return endpoint ? endpoint->id() : 0x01;
}

quint8 MetaObject::version(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->version() : 0;
}

QString MetaObject::manufacturerName(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->manufacturerName() : QString();
}

QString MetaObject::modelName(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->modelName() : QString();
}
