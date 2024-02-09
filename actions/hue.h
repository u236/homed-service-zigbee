#ifndef ACTIONS_HUE_H
#define ACTIONS_HUE_H

#include "action.h"
#include "zcl.h"

namespace ActionsHUE
{
    class IndicatorMode : public ActionObject
    {

    public:

        IndicatorMode(void) : ActionObject("indicatorMode", CLUSTER_BASIC, 0x100B, 0x0033) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class SensivivityMode : public ActionObject
    {

    public:

        SensivivityMode(void) : ActionObject("sensivivityMode", CLUSTER_OCCUPANCY_SENSING, 0x100B, 0x0030) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

#endif
