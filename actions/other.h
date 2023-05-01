#ifndef ACTIONS_OTHER_H
#define ACTIONS_OTHER_H

#include "action.h"
#include "zcl.h"

namespace ActionsOther
{
    class PerenioSmartPlug : public ActionObject
    {

    public:

        PerenioSmartPlug(void) : ActionObject("perenioSmartPlug", CLUSTER_PERENIO, 0x0000, QList <QString> {"powerOnStatus", "resetAlarms", "voltageMin", "voltageMax", "powerMax", "energyLimit"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

#endif
