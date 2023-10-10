#ifndef ACTIONS_PTVO_H
#define ACTIONS_PTVO_H

#include "action.h"
#include "zcl.h"

namespace ActionsPTVO
{
    class Status : public ActionObject
    {

    public:

        Status(const QString &name) : ActionObject(name, CLUSTER_ON_OFF, 0x0000, 0x0000) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class AnalogInput : public ActionObject
    {

    public:

        AnalogInput(const QString &name) : ActionObject(name, CLUSTER_ANALOG_INPUT, 0x0000, 0x0055) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };

    class ChangePattern : public Status
    {

    public:

        ChangePattern(void) : Status("changePattern") {}

    };

    class Count : public AnalogInput
    {

    public:

        Count(void) : AnalogInput("count") {}

    };

    class Pattern : public AnalogInput
    {

    public:

        Pattern(void) : AnalogInput("pattern") {}

    };

    class SerialData : public ActionObject
    {

    public:

        SerialData(void) : ActionObject("data", CLUSTER_MULTISTATE_VALUE) {}
        QByteArray request(const QString &name, const QVariant &data) override;

    };
}

#endif
