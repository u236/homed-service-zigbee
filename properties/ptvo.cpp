#include <QtEndian>
#include "device.h"
#include "ptvo.h"

void PropertiesPTVO::Status::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributes.at(0))
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
            m_buffer = qFromLittleEndian(value) / option(QString(m_name).append("Divider"), 1).toDouble();

            if (m_id.isEmpty() || !multiple())
                m_value = m_buffer;

            break;
        }

        case 0x001C:
        {
            QList <QString> list = QString(data).split(',');

            if (!m_id.isEmpty() && list.value(0) == m_id)
                m_value = m_buffer;

            break;
        }
    }
}

bool PropertiesPTVO::AnalogInput::multiple(void)
{
    const QList <Property> list = reinterpret_cast <EndpointObject*> (m_parent)->properties();

    for (int i = 0; i < list.count(); i++)
        if (list.at(i)->clusters().value(0) == CLUSTER_ANALOG_INPUT && list.at(i) != this)
            return true;

    return false;
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
