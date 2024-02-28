#ifndef ACTIONS_CUSTOM_H
#define ACTIONS_CUSTOM_H

#include "action.h"
#include "zcl.h"

namespace ActionsCustom
{
    class Attribute : public ActionObject
    {

    public:

        Attribute(const QString &name, quint16 clusterId, quint16 manufacturerCode, quint16 attributeId, quint8 dataType, double divider) :
            ActionObject(name, clusterId, manufacturerCode, attributeId), m_dataType(dataType), m_divider(divider) {}

        QByteArray request(const QString &name, const QVariant &data) override;

    private:

        quint8 m_dataType;
        double m_divider;

    };
}

#endif
