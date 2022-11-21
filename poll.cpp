#include "poll.h"

void PollObject::registerMetaTypes(void)
{
    qRegisterMetaType <Polls::Status>                   ("statusPoll");
    qRegisterMetaType <Polls::Level>                    ("levelPoll");
    qRegisterMetaType <Polls::ColorHS>                  ("colorHSPoll");
    qRegisterMetaType <Polls::ColorXY>                  ("colorXYPoll");
    qRegisterMetaType <Polls::ColorTemperature>         ("colorTemperaturePoll");

    qRegisterMetaType <PollsOther::LumiPresenceSensor>  ("lumiPresenceSensorPoll");
    qRegisterMetaType <PollsOther::PerenioSmartPlug>    ("perenioSmartPlugPoll");
}
