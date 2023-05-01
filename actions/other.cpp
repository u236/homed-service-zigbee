#include <QtEndian>
#include "other.h"

QByteArray ActionsOther::PerenioSmartPlug::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // powerOnStatus
        {
            QList <QString> list = {"off", "on", "previous"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            m_attributes = {0x0000};

            if (value < 0)
                return QByteArray();

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 1: // resetAlarms
        {
            m_attributes = {0x0001};

            if (!data.toBool())
                return QByteArray();

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_UNSIGNED, QByteArray(1, 0x00)); // TODO: check payload
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

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_16BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }
    }
}
