#include <QtEndian>
#include "color.h"
#include "common.h"

QByteArray Actions::Status::request(const QString &, const QVariant &data)
{
    qint8 command = listIndex({"off", "on", "toggle"}, data);
    return command < 0 ? QByteArray() : zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, static_cast <quint8> (command));
}

QByteArray Actions::Level::request(const QString &, const QVariant &data)
{
    switch (data.type())
    {
        case QVariant::LongLong:
        {
            moveToLevelStruct payload;

            payload.level = static_cast <quint8> (data.toInt() < 0xFE ? data.toInt() : 0xFE);
            payload.time = 0;

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x04).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::List:
        {
            QList <QVariant> list = data.toList();
            moveToLevelStruct payload;

            payload.level = static_cast <quint8> (list.value(0).toInt() < 0xFE ? list.value(0).toInt() : 0xFE);
            payload.time = qToLittleEndian <quint16> (list.value(1).toInt());

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x04).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::String:
        {
            QList <QString> list {"moveUp", "moveDown", "stop"};
            int index = list.indexOf(data.toString());

            switch (index)
            {
                case 0 ... 1:
                {
                    moveLevelStruct payload;

                    payload.mode = index ? 0x01 : 0x00;
                    payload.rate = 0x55;

                    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, index ? 0x01 : 0x05).append(reinterpret_cast <char*> (&payload), sizeof(payload));
                }

                case 2:  return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x07);
                default: return QByteArray();
            }
        }

        default:
            return QByteArray();
    }
}

QByteArray Actions::AnalogOutput::request(const QString &, const QVariant &data)
{
    float value = qToLittleEndian(data.toFloat());
    return writeAttribute(DATA_TYPE_SINGLE_PRECISION, &value, sizeof(value));
}

QByteArray Actions::CoverStatus::request(const QString &, const QVariant &data)
{
    QList <QString> list = option("invertCover").toBool() ? QList <QString> {"close", "open", "stop"} : QList <QString> {"open", "close", "stop"};
    qint8 command = static_cast <qint8> (list.indexOf(data.toString()));
    return command < 0 ? QByteArray() : zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, static_cast <quint8> (command));
}

QByteArray Actions::CoverPosition::request(const QString &, const QVariant &data)
{
    quint8 value = static_cast <qint8> (data.toInt());

    meta().insert("position", endpointProperty()->value().toMap().value("position"));

    if (value > 100)
        value = 100;

    if (!option("invertCover").toBool())
        value = 100 - value;

    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x05).append(reinterpret_cast <char*> (&value), sizeof(value));
}

QByteArray Actions::CoverTilt::request(const QString &, const QVariant &data)
{
    quint8 value = static_cast <qint8> (data.toInt());

    meta().insert("tilt", endpointProperty()->value().toMap().value("tilt"));

    if (value > 100)
        value = 100;

    if (!option("invertCover").toBool())
        value = 100 - value;

    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x08).append(reinterpret_cast <char*> (&value), sizeof(value));
}

QByteArray Actions::Thermostat::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // temperatureOffset
        case 1: // hysteresis
        {
            qint8 value = static_cast <qint8> (data.toDouble() * 10);
            m_attributes = {static_cast <quint16> (index == 0 ? 0x0010 : 0x0019)};
            return writeAttribute(DATA_TYPE_8BIT_SIGNED, &value, sizeof(value));
        }

        case 2: // targetTemperature
        {
            qint16 value = qToLittleEndian <qint16> (data.toDouble() * 100);
            m_attributes = {0x0012};
            return writeAttribute(DATA_TYPE_16BIT_SIGNED, &value, sizeof(value));
        }

        case 3: // systemMode
        {
            qint8 value = listIndex({"off", "auto", "heat"}, data);

            if (value == 2)
                value = 0x04;

            m_attributes = {0x001C};
            return value < 0 ? QByteArray() : writeAttribute(DATA_TYPE_8BIT_ENUM, &value, sizeof(value));
        }
    }

    return QByteArray();
}

QByteArray Actions::ColorHS::request(const QString &, const QVariant &data)
{
    switch (data.type())
    {
        case QVariant::List:
        {
            QList <QVariant> list = data.toList();
            moveToColorHSStruct payload;
            Color color(list.value(0).toDouble() / 0xFF, list.value(1).toDouble() / 0xFF, list.value(2).toDouble() / 0xFF);
            double colorH, colorS;

            color.toHS(&colorH, &colorS);
            colorH *= 0xFF;
            colorS *= 0xFF;

            payload.colorH = static_cast <quint8> (colorH < 0xFE ? colorH : 0xFE);
            payload.colorS = static_cast <quint8> (colorS < 0xFE ? colorS : 0xFE);
            payload.time = qToLittleEndian <quint16> (list.value(3).toInt());

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x06).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

QByteArray Actions::ColorXY::request(const QString &, const QVariant &data)
{
    switch (data.type())
    {
        case QVariant::List:
        {
            QList <QVariant> list = data.toList();
            moveToColorXYStruct payload;
            Color color(list.value(0).toDouble() / 0xFF, list.value(1).toDouble() / 0xFF, list.value(2).toDouble() / 0xFF);
            double colorX, colorY;

            color.toXY(&colorX, &colorY);
            colorX *= 0xFFFF;
            colorY *= 0xFFFF;

            payload.colorX = qToLittleEndian <quint16> (colorX < 0xFEFF ? colorX : 0xFEFF);
            payload.colorY = qToLittleEndian <quint16> (colorY < 0xFEFF ? colorY : 0xFEFF);
            payload.time = qToLittleEndian <quint16> (list.value(3).toInt());

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x07).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

QByteArray Actions::ColorTemperature::request(const QString &, const QVariant &data)
{
    switch (data.type())
    {
        case QVariant::LongLong:
        {
            moveToColorTemperatureStruct payload;

            payload.colorTemperature = qToLittleEndian <quint16> (data.toInt() < 0xFEFF ? data.toInt() : 0xFEFF);
            payload.time = 0;

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x0A).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::List:
        {
            QList <QVariant> list = data.toList();
            moveToColorTemperatureStruct payload;

            payload.colorTemperature = qToLittleEndian <quint16> (list.value(0).toInt() < 0xFEFF ? list.value(0).toInt() : 0xFEFF);
            payload.time = qToLittleEndian <quint16> (list.value(1).toInt());

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x0A).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }


        case QVariant::String:
        {
            QList <QString> list {"moveUp", "moveDown", "stop"};
            moveColorTemperatureStruct payload;

            switch (list.indexOf(data.toString()))
            {
                case 0: payload.mode = 0x01; break;
                case 1: payload.mode = 0x03; break;
                case 2: payload.mode = 0x00; break;
                default: return QByteArray();
            }

            payload.rate = qToLittleEndian <quint16> (0x0055);
            payload.minMireds = qToLittleEndian <quint16> (0x0000);
            payload.maxMireds = qToLittleEndian <quint16> (0x03E8);

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x4B).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

QByteArray Actions::OccupancyTimeout::request(const QString &, const QVariant &data)
{
    quint16 value = qToLittleEndian <quint16> (data.toInt());
    return writeAttribute(DATA_TYPE_16BIT_UNSIGNED, &value, sizeof(value));
}

QByteArray Actions::ChildLock::request(const QString &, const QVariant &data)
{
    quint8 value = data.toBool() ? 0x01 : 0x00;
    return writeAttribute(DATA_TYPE_8BIT_ENUM, &value, sizeof(value));
}
