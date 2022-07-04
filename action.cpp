#include <QtEndian>
#include "action.h"
#include "zigbee.h"

using namespace Actions;

Actions::Status::Status(quint8 endPointId) : ActionObject("status", CLUSTER_ON_OFF, endPointId) {}

QByteArray Status::request(const QVariant &data)
{
    zclHeaderStruct header;
    QString command = data.toString();

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = command == "toggle" ? 0x02 : command == "on" ? 0x01 : 0x00;

    return QByteArray(reinterpret_cast <char*> (&header), sizeof(header));
}

Level::Level(quint8 endPointId) : ActionObject("level", CLUSTER_LEVEL_CONTROL, endPointId) {}

QByteArray Level::request(const QVariant &data)
{
    zclHeaderStruct header;
    levelStruct payload;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = 0x00;

    if (data.type() == QVariant::List)
    {
        payload.level = static_cast <quint8> (data.toList().value(0).toInt());
        payload.time = qToLittleEndian(static_cast <quint16> (data.toList().value(1).toInt()));
    }
    else
    {
        payload.level = static_cast <quint8> (data.toInt());
        payload.time = qToLittleEndian <quint16> (5);
    }

    return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
}

ColorTemperature::ColorTemperature(quint8 endPointId) : ActionObject("colorTemperature", CLUSTER_COLOR_CONTROL, endPointId) {}

QByteArray ColorTemperature::request(const QVariant &data)
{
    zclHeaderStruct header;
    colorTemperatureStruct payload;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = 0x0A;

    if (data.type() == QVariant::List)
    {
        payload.temperature = qToLittleEndian(static_cast <quint16> (data.toList().value(0).toInt()));
        payload.time = qToLittleEndian(static_cast <quint16> (data.toList().value(1).toInt()));
    }
    else
    {
        payload.temperature = qToLittleEndian(static_cast <quint16> (data.toInt()));
        payload.time = qToLittleEndian <quint16> (5);
    }

    return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
}
