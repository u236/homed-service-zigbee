#include <QtEndian>
#include "ikea.h"

void PropertiesIKEA::Occupancy::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    quint16 value;

    if (commandId != 0x42)
        return;

    memcpy(&value, payload.constData(), sizeof(value));
    m_timeout = qFromLittleEndian(value) / 10;
    m_value = true;
}

void PropertiesIKEA::Occupancy::resetValue(void)
{
    m_value = false;
}

void PropertiesIKEA::StatusAction::parseCommand(quint16, quint8 commandId, const QByteArray &)
{
    if (meta().value("time").toLongLong() + 1000 > QDateTime::currentMSecsSinceEpoch())
        return;

    switch (commandId)
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
    }
}

void PropertiesIKEA::ArrowAction::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    switch (commandId)
    {
        case 0x07:

            if (payload.at(0) == 2)
                return;

            m_value = payload.at(0) ? "leftClick" : "rightClick";
            break;

        case 0x08:
            meta().insert("arrow", payload.at(0) ? "left" : "right");
            m_value = meta().value("arrow").toString().append("Hold");
            break;

        case 0x09:

            meta().insert("time", QDateTime::currentMSecsSinceEpoch());

            if (!meta().contains("arrow"))
                return;

            m_value = meta().value("arrow").toString().append("Release");
            meta().remove("arrow");
            break;
    }
}
