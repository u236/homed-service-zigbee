#include <QtEndian>
#include "action.h"
#include "zigbee.h"

using namespace Actions;

Actions::Status::Status(quint8 endPointId) : ActionObject("status", CLUSTER_ON_OFF, endPointId) {}

QByteArray Status::request(const QVariant &data)
{
    zclHeaderStruct header;
    QString status = data.toString();

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
            levelStruct payload;
            QList <QVariant> list = data.toList();

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
            colorTemperatureStruct payload;
            QList <QVariant> list = data.toList();

            payload.temperature = qToLittleEndian(static_cast <quint16> (list.value(0).toInt()));
            payload.time = qToLittleEndian(static_cast <quint16> (list.value(1).toInt()));

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

ColorHS::ColorHS(quint8 endPointId) : ActionObject("colorHS", CLUSTER_COLOR_CONTROL, endPointId) {}

QByteArray ColorHS::request(const QVariant &data)
{
    zclHeaderStruct header;

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = 0x06;

    switch (data.type())
    {
        case QVariant::List:
        {
            colorHSStruct payload;
            QList <QVariant> list = data.toList();
            quint16 colorH = static_cast <quint8> (list.value(0).toInt()), colorS = static_cast <quint8> (list.value(1).toInt());

            payload.colorH = colorH < 0xFE ? colorH : 0xFE;
            payload.colorS = colorS < 0xFE ? colorS : 0xFE;
            payload.time = qToLittleEndian(static_cast <quint16> (list.value(2).toInt()));

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
            colorXYStruct payload;
            QList <QVariant> list = data.toList();
            quint16 colorX = static_cast <quint16> (list.value(0).toDouble() * 0xFFFF), colorY = static_cast <quint16> (list.value(1).toDouble() * 0xFFFF);

            payload.colorX = qToLittleEndian(colorX < 0xFEFF ? colorX : 0xFEFF);
            payload.colorY = qToLittleEndian(colorY < 0xFEFF ? colorY : 0xFEFF);
            payload.time = qToLittleEndian(static_cast <quint16> (list.value(2).toInt()));

            return QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

