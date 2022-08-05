#include <QtEndian>
#include "action.h"
#include "logger.h"

void ActionObject::registerMetaTypes(void)
{
    qRegisterMetaType <Actions::Status>                 ("statusAction");
    qRegisterMetaType <Actions::Level>                  ("levelAction");
    qRegisterMetaType <Actions::ColorHS>                ("colorHSAction");
    qRegisterMetaType <Actions::ColorXY>                ("colorXYAction");
    qRegisterMetaType <Actions::ColorTemperature>       ("colorTemperatureAction");

    qRegisterMetaType <ActionsTUYA::Sensitivity>        ("tuyaSensitivityAction");
    qRegisterMetaType <ActionsTUYA::RangeMin>           ("tuyaRangeMinAction");
    qRegisterMetaType <ActionsTUYA::RangeMax>           ("tuyaRangeMaxAction");
    qRegisterMetaType <ActionsTUYA::DetectionDelay>     ("tuyaDetectionDelayAction");
}

QByteArray Actions::Status::request(const QVariant &data)
{
    zclHeaderStruct header;
    QString status = data.toString();

    header.frameControl = FC_CLUSTER_SPECIFIC;
    header.transactionId = m_transactionId++;
    header.commandId = status == "toggle" ? 0x02 : status == "on" ? 0x01 : 0x00;

    return QByteArray(reinterpret_cast <char*> (&header), sizeof(header));
}

QByteArray Actions::Level::request(const QVariant &data)
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

QByteArray Actions::ColorHS::request(const QVariant &data)
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

QByteArray Actions::ColorXY::request(const QVariant &data)
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

QByteArray Actions::ColorTemperature::request(const QVariant &data)
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

QByteArray ActionsTUYA::Request::makeRequest(quint8 transactionId, quint8 dataPoint, quint8 dataType, void *payload)
{
    zclHeaderStruct zclHeared;
    tuyaHeaderStruct tuyaHeader;

    zclHeared.frameControl = FC_CLUSTER_SPECIFIC;
    zclHeared.transactionId = transactionId;
    zclHeared.commandId = 0x00;

    tuyaHeader.status = 0x00;
    tuyaHeader.transactionId = transactionId;
    tuyaHeader.dataPoint = dataPoint;
    tuyaHeader.dataType = dataType;
    tuyaHeader.function = 0x00;

    switch (tuyaHeader.dataType)
    {
        case 0x02: tuyaHeader.length = 4; break;
        case 0x04: tuyaHeader.length = 1; break;
        default: return QByteArray();
    }

    return QByteArray(reinterpret_cast <char*> (&zclHeared), sizeof(zclHeared)).append(reinterpret_cast <char*> (&tuyaHeader), sizeof(tuyaHeader)).append(reinterpret_cast <char*> (payload), tuyaHeader.length);
}

QByteArray ActionsTUYA::Sensitivity::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toInt());

    if (value > 9)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x02, 0x02, &value);
}

QByteArray ActionsTUYA::RangeMin::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toDouble() * 100);

    if (value > 950)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x03, 0x02, &value);
}

QByteArray ActionsTUYA::RangeMax::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toDouble() * 100);

    if (value > 950)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x04, 0x02, &value);
}

QByteArray ActionsTUYA::DetectionDelay::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toDouble() * 100);

    if (value < 1 || value > 10)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x65, 0x02, &value);
}
