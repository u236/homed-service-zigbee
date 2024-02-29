#include <QtEndian>
#include "other.h"

QByteArray ActionsOther::PerenioSmartPlug::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // powerOnStatus
        {
            qint8 value = listIndex({"off", "on", "previous"}, data);
            m_attributes = {0x0000};
            return value < 0 ? QByteArray() : writeAttribute(DATA_TYPE_8BIT_UNSIGNED, &value, sizeof(value));
        }

        case 1: // resetAlarms
        {
            quint8 value = 0x00;
            m_attributes = {0x0001};
            return !data.toBool() ? QByteArray() : writeAttribute(DATA_TYPE_8BIT_UNSIGNED, &value, sizeof(value));
        }

        default:
        {
            quint16 value = qToLittleEndian <quint16> (data.toInt());

            switch (index)
            {
                case 2:  m_attributes = {0x0004}; break; // voltageMin
                case 3:  m_attributes = {0x0005}; break; // voltageMax
                case 4:  m_attributes = {0x000B}; break; // powerMax
                case 5:  m_attributes = {0x000F}; break; // energyLimit
                default: return QByteArray();
            }

            return writeAttribute(DATA_TYPE_16BIT_UNSIGNED, &value, sizeof(value));
        }
    }
}
