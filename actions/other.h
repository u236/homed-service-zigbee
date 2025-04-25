#ifndef ACTIONS_OTHER_H
#define ACTIONS_OTHER_H

#include "action.h"
#include "zcl.h"

namespace ActionsYandex
{
    class CommonSettings : public ActionObject
    {

    public:

        CommonSettings(void) : ActionObject("settings", CLUSTER_YANDEX, 0x140A, QList <QString> {"powerMode", "interlock", "indicator"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };

    class SwitchSettings : public ActionObject
    {

    public:

        SwitchSettings(void) : ActionObject("settings", CLUSTER_YANDEX, 0x140A, QList <QString> {"switchMode", "switchType"}) {}
        QVariant request(const QString &name, const QVariant &data) override;

    };
}

namespace ActionsCustom
{
    class Attribute : public ActionObject
    {

    public:

        Attribute(const QString &name, const QString &type, quint16 clusterId, quint16 manufacturerCode, quint16 attributeId, quint8 dataType, double divider) :
            ActionObject(name, clusterId, manufacturerCode, attributeId), m_type(type), m_dataType(dataType), m_divider(divider > 0 ? divider : 1) {}

        QVariant request(const QString &name, const QVariant &data) override;

    private:

        QString m_type;
        quint8 m_dataType;
        double m_divider;

    };
}

#endif
