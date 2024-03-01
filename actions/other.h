#ifndef ACTIONS_OTHER_H
#define ACTIONS_OTHER_H

#include "action.h"

namespace ActionsCustom
{
    class Attribute : public ActionObject
    {

    public:

        Attribute(const QString &name, const QString &type, quint16 clusterId, quint16 manufacturerCode, quint16 attributeId, quint8 dataType, double divider) :
            ActionObject(name, clusterId, manufacturerCode, attributeId), m_type(type), m_dataType(dataType), m_divider(divider) {}

        QByteArray request(const QString &name, const QVariant &data) override;

    private:

        QString m_type;
        quint8 m_dataType;
        double m_divider;

    };
}

#endif
