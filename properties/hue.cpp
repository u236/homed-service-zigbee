#include <QtEndian>
#include "hue.h"

void PropertiesHUE::IndicatorMode::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0033)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void PropertiesHUE::SensivivityMode::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0030)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "low"; break;
        case 0x01: m_value = "medium"; break;
        case 0x02: m_value = "high"; break;
        case 0x03: m_value = "ultra"; break;
        case 0x04: m_value = "max"; break;
    }
}
