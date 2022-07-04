#include <QtEndian>
#include "action.h"
#include "zigbee.h"

#include "logger.h"

using namespace Actions;

Actions::Status::Status(quint8 endPointId) : ActionObject(endPointId, CLUSTER_ON_OFF, "status") {}

QByteArray Status::request(const QVariant &data)
{
    zclHeaderStruct header;
    QString command = data.toString();

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = command == "toggle" ? 0x02 : command == "on" ? 0x01 : 0x00;

    return QByteArray(reinterpret_cast <char*> (&header), sizeof(header));
}

Level::Level(quint8 endPointId) : ActionObject(endPointId, CLUSTER_LEVEL_CONTROL, "level") {}

QByteArray Level::request(const QVariant &data)
{
    zclHeaderStruct header;
    levelControlStruct payload;

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
