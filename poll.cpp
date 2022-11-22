#include "poll.h"

void PollObject::registerMetaTypes(void)
{
    qRegisterMetaType <Polls::Status>                   ("statusPoll");
    qRegisterMetaType <Polls::Level>                    ("levelPoll");
    qRegisterMetaType <Polls::ColorHS>                  ("colorHSPoll");
    qRegisterMetaType <Polls::ColorXY>                  ("colorXYPoll");
    qRegisterMetaType <Polls::ColorTemperature>         ("colorTemperaturePoll");
    qRegisterMetaType <Polls::Energy>                   ("energyPoll");
    qRegisterMetaType <Polls::Voltage>                  ("voltagePoll");
    qRegisterMetaType <Polls::Current>                  ("currentPoll");
    qRegisterMetaType <Polls::Power>                    ("powerPoll");

    qRegisterMetaType <PollsOther::LumiPresenceSensor>  ("lumiPresenceSensorPoll");
    qRegisterMetaType <PollsOther::PerenioSmartPlug>    ("perenioSmartPlugPoll");
}
