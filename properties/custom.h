#ifndef PROPERTIES_CUSTOM_H
#define PROPERTIES_CUSTOM_H

#include "property.h"
#include "zcl.h"

namespace PropertiesCustom
{
    class Attribute : public PropertyObject
    {

    public:

        Attribute(const QString &name, quint16 clusterId, quint16 attributeId, quint8 dataType, double divider) :
            PropertyObject(name, clusterId), m_attributeId(attributeId), m_dataType(dataType), m_divider(divider) {}

        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    private:

        quint16 m_attributeId;
        quint8 m_dataType;
        double m_divider;

    };

    class Command : public PropertyObject
    {

    public:

        Command(const QString &name, quint16 clusterId, const QMap <QString, QVariant> &map) :
            PropertyObject(name, clusterId), m_map(map) {}

        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    private:

        QMap <QString, QVariant> m_map;

    };
}

#endif
