#ifndef ACTIONS_IAS_H
#define ACTIONS_IAS_H

#include "action.h"
#include "zcl.h"

namespace ActionsIAS
{
    class Warning : public ActionObject
    {

    public:

        Warning(void) : ActionObject("warning", CLUSTER_IAS_WD, 0x0000, QList <QString> {"sirenMode", "sirenLevel", "strobe", "strobeLevel", "dutyCycle", "duration"}) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

#endif
