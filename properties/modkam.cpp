#include <QtEndian>
#include "modkam.h"

void PropertiesModkam::ButtonAction::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0055)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "hold"; break;
        case 0x01: m_value = "singleClick"; break;
        case 0x02: m_value = "doubleClick"; break;
        case 0x03: m_value = "tripleClick"; break;
        case 0x04: m_value = "quadrupleClick"; break;
        case 0xFF: m_value = "release"; break;
        default:   m_value = "multipleClick"; break;
    }
}
