#include <QtEndian>
#include "action.h"
#include "zigbee.h"

using namespace Actions;

Actions::Status::Status(quint8 endPointId) : ActionObject("status", CLUSTER_ON_OFF, endPointId) {}

QByteArray Status::request(const QVariant &data)
{
    QString status = data.toString();
    zclHeaderStruct header;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = status == "toggle" ? 0x02 : status == "on" ? 0x01 : 0x00;

    return QByteArray(reinterpret_cast <char*> (&header), sizeof(header));
}

Level::Level(quint8 endPointId) : ActionObject("level", CLUSTER_LEVEL_CONTROL, endPointId) {}

QByteArray Level::request(const QVariant &data)
{
    zclHeaderStruct header;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;

    switch (data.type())
    {
        case QVariant::LongLong:
        {
            levelStruct payload;

            header.commandId = 0x00;
            payload.level = static_cast <quint8> (data.toInt());
            payload.time = qToLittleEndian <quint16> (5);

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::List:
        {
            QList <QVariant> list = data.toList();
            levelStruct payload;

            header.commandId = 0x00;
            payload.level = static_cast <quint8> (list.value(0).toInt());
            payload.time = qToLittleEndian(static_cast <quint16> (list.value(1).toInt()));

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::String:
        {
            QString action = data.toString();

            if (action != "moveStop")
            {
                quint8 payload[2] = {static_cast <quint8> (action == "moveUp" ? 0x00 : 0x01), 0x55}; // TODO: check it
                header.commandId = 0x01;
                return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
            }

            header.commandId = 0x07;
            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header));
        }

        default:
            return QByteArray();
    }
}

ColorTemperature::ColorTemperature(quint8 endPointId) : ActionObject("colorTemperature", CLUSTER_COLOR_CONTROL, endPointId) {}

QByteArray ColorTemperature::request(const QVariant &data)
{
    zclHeaderStruct header;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = 0x0A;

    switch (data.type())
    {
        case QVariant::LongLong:
        {
            colorTemperatureStruct payload;

            payload.temperature = qToLittleEndian(static_cast <quint16> (data.toInt()));
            payload.time = qToLittleEndian <quint16> (5);

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::List:
        {
            QList <QVariant> list = data.toList();
            colorTemperatureStruct payload;

            payload.temperature = qToLittleEndian(static_cast <quint16> (list.value(0).toInt()));
            payload.time = qToLittleEndian(static_cast <quint16> (list.value(1).toInt()));

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

ColorXY::ColorXY(quint8 endPointId) : ActionObject("colorXY", CLUSTER_COLOR_CONTROL, endPointId) {}

QByteArray ColorXY::request(const QVariant &data)
{
    zclHeaderStruct header;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = 0x07;

    switch (data.type())
    {
        case QVariant::List:
        {
            QList <QVariant> list = data.toList();
            colorXYStruct payload;

            payload.colorX = qToLittleEndian(static_cast <quint16> (list.value(0).toDouble() * 0xFFFF));
            payload.colorY = qToLittleEndian(static_cast <quint16> (list.value(1).toDouble() * 0xFFFF));
            payload.time = qToLittleEndian(static_cast <quint16> (list.value(2).toInt()));

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

