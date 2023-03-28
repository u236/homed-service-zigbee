#include "meta.h"
#include "device.h"

QVariant MetaObject::endpointOption(const QString &name)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    QString optionName = name.isEmpty() ? m_name : name;
    QVariant value;

    if (!endpoint || endpoint->device().isNull())
        return value;

    value = endpoint->device()->options().value(QString("%1-%2").arg(optionName, QString::number(endpoint->id())));
    return value.isValid() ? value : endpoint->device()->options().value(optionName);
}

quint8 MetaObject::deviceVersion(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->version() : 0;
}

QString MetaObject::deviceManufacturerName(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->manufacturerName() : QString();
}

QString MetaObject::deviceModelName(void)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->modelName() : QString();
}
