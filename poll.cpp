#include "poll.h"

void PollObject::registerMetaTypes(void)
{
    qRegisterMetaType <Polls::Status>   ("statusPoll");
    qRegisterMetaType <Polls::Energy>   ("energyPoll");
    qRegisterMetaType <Polls::Voltage>  ("voltagePoll");
    qRegisterMetaType <Polls::Current>  ("currentPoll");
    qRegisterMetaType <Polls::Power>    ("powerPoll");
}
