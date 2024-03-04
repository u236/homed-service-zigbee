#include <QtEndian>
#include "ptvo.h"

void PropertiesPTVO::Status::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = data.at(0) ? true : false;
}

void PropertiesPTVO::AnalogInput::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0055:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            (m_unit.isEmpty() ? m_value : m_buffer) = qFromLittleEndian(value) / option(QString(m_name).append("Divider"), 1).toDouble();
            break;
        }

        case 0x001C:
        {
            QList <QString> list = QString(data).split(',');

            if (m_unit.isEmpty() || list.value(0) != m_unit)
                return;

            m_value = m_buffer;
            break;
        }
    }
}

void PropertiesPTVO::SwitchAction::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0055)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void PropertiesPTVO::SerialData::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x000E)
        return;

    m_value = data.toHex(':');
}
