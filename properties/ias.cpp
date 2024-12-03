#include <QtEndian>
#include "ias.h"

void PropertiesIAS::ZoneStatus::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    QMap <QString, QVariant> map = m_value.toMap();
    quint16 value = 0;

    if (commandId != 0x00)
        return;

    memcpy(&value, payload.constData(), sizeof(value));
    value = qFromLittleEndian(value);

    map.insert(m_name, (value & (option("iasAlarm2").toBool() ? 0x0002 : 0x0001)) ? true : false);
    map.insert("tamper", (value & 0x0004) ? true : false);
    map.insert("batteryLow", (value & 0x0008) ? true : false);

    m_value = map;
}

void PropertiesIAS::ZoneStatus::resetValue(void)
{
    QMap <QString, QVariant> map = m_value.toMap();
    map.insert(m_name, false);
    m_value = map;
}

void PropertiesIAS::Warning::resetValue(void)
{
    QMap <QString, QVariant> map = m_value.toMap();
    map.insert("sirenMode", "stop");
    map.insert("strobe", false);
    m_value = map;
}

void PropertiesIAS::ContolAction::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    switch (commandId)
    {
        case 0x00:

            switch (static_cast <quint8> (payload.at(0)))
            {
                case 0x00: m_value = "disarm"; break;
                case 0x01: m_value = "armDayZones"; break;
                case 0x02: m_value = "armNightZones"; break;
                case 0x03: m_value = "armAllZones"; break;
                case 0x04: m_value = "exitDelay"; break;
                case 0x05: m_value = "entryDelay"; break;
            }

            break;

        case 0x02: m_value = "emergency"; break;
        case 0x03: m_value = "fire"; break;
        case 0x04: m_value = "panic"; break;
    }
}
