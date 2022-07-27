#include "poll.h"

void PollObject::registerMetaTypes(void)
{
    qRegisterMetaType <Polls::Status>           ("statusPoll");
    qRegisterMetaType <Polls::Level>            ("levelPoll");
    qRegisterMetaType <Polls::ColorHS>          ("colorHSPoll");
    qRegisterMetaType <Polls::ColorXY>          ("colorXYPoll");
    qRegisterMetaType <Polls::ColorTemperature> ("colorTemperaturePoll");
    qRegisterMetaType <Polls::Energy>           ("energyPoll");
    qRegisterMetaType <Polls::Power>            ("powerPoll");
}
