#include <QtEndian>
#include "color.h"
#include "common.h"

QVariant Actions::Status::request(const QString &, const QVariant &data)
{
    qint8 command = listIndex({"off", "on", "toggle"}, data);
    return command < 0 ? QByteArray() : zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, static_cast <quint8> (command));
}

QVariant Actions::Level::request(const QString &, const QVariant &data)
{
    switch (data.type())
    {
        case QVariant::LongLong:
        {
            moveToLevelStruct payload;
            int value = data.toInt();

            payload.level = static_cast <quint8> (value < 0x00 ? 0x00 : value > 0xFE ? 0xFE : value);
            payload.time = 0;

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x04).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::List:
        {
            moveToLevelStruct payload;
            QList <QVariant> list = data.toList();
            int value = list.value(0).toInt();

            payload.level = static_cast <quint8> (value < 0x00 ? 0x00 : value > 0xFE ? 0xFE : value);
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

QVariant Actions::AnalogOutput::request(const QString &, const QVariant &data)
{
    float value = qToLittleEndian(data.toFloat());
    return writeAttribute(DATA_TYPE_SINGLE_PRECISION, &value, sizeof(value));
}

QVariant Actions::CoverStatus::request(const QString &, const QVariant &data)
{
    QList <QString> list = option("invertCover").toBool() != option("invertControls").toBool() ? QList <QString> {"close", "open", "stop"} : QList <QString> {"open", "close", "stop"};
    qint8 command = static_cast <qint8> (list.indexOf(data.toString()));
    return command < 0 ? QByteArray() : zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, static_cast <quint8> (command));
}

QVariant Actions::CoverPosition::request(const QString &, const QVariant &data)
{
    int value = data.toInt();
    quint8 position = value < 0 ? 0 : value > 100 ? 100 : value;

    setMeta("position", endpointProperty()->value().toMap().value("position"));

    if (!option("invertCover").toBool())
        position = 100 - position;

    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x05).append(static_cast <char> (position));
}

QVariant Actions::Thermostat::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // temperatureOffset
        case 1: // hysteresis
        {
            qint8 value = static_cast <qint8> (data.toDouble() * option(QString(name).append("Divider"), 10).toDouble());
            return writeAttribute(index == 0 ? 0x0010 : 0x0019, DATA_TYPE_8BIT_SIGNED, &value, sizeof(value));
        }

        case 2: // targetTemperature
        {
            qint16 value = qToLittleEndian <qint16> (data.toDouble() * 100);
            return writeAttribute(0x0012, DATA_TYPE_16BIT_SIGNED, &value, sizeof(value));
        }

        case 3: // systemMode
        {
            qint8 value = listIndex({"off", "auto", "cool", "heat", "fan", "dry"}, data);

            if (value > 1)
                value += value > 3 ? 3 : 1;

            return value < 0 ? QByteArray() : writeAttribute(0x001C, DATA_TYPE_8BIT_ENUM, &value, sizeof(value));
        }
    }

    return QByteArray();
}

QVariant Actions::ThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty("thermostat");
    QList <QString> typeList = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
    QString type = name.mid(0, name.indexOf('P'));
    QByteArray payload;

    payload.append(static_cast <char> (option("programTransitions", 4).toInt()));
    payload.append(static_cast <char> (1 << typeList.indexOf(type)));
    payload.append(0x01);

    if (m_data.isEmpty() || meta(QString("%1Program").arg(type)).toBool())
    {
        m_data = property->value().toMap();
        setMeta(QString("%1Program").arg(type), false);
    }

    m_data.insert(name, data);

    for (int i = 0; i < payload.at(0); i++)
    {
        QString key = QString("%1P%2").arg(type).arg(i + 1);
        quint16 time = qToLittleEndian <quint16> (static_cast <quint16> (m_data.value(QString("%1Hour").arg(key), i * 4).toInt() * 60 + m_data.value(QString("%1Minute").arg(key), 0).toInt()));
        quint16 temperature = qToLittleEndian <quint16> (static_cast <quint16> (m_data.value(QString("%1Temperature").arg(key), 21).toDouble() * 100));
        payload.append(reinterpret_cast <char*> (&time), sizeof(time));
        payload.append(reinterpret_cast <char*> (&temperature), sizeof(temperature));
    }

    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x01).append(payload);
}

QVariant Actions::ColorHS::request(const QString &, const QVariant &data)
{
    switch (data.type())
    {
        case QVariant::List:
        {
            moveToColorHSStruct payload;
            QList <QVariant> list = data.toList();
            Color color(list.value(0).toDouble() / 0xFF, list.value(1).toDouble() / 0xFF, list.value(2).toDouble() / 0xFF);
            double h, s;

            color.toHS(&h, &s);
            h *= 0xFF;
            s *= 0xFF;

            payload.colorH = static_cast <quint8> (h > 0xFE ? 0xFE : s);
            payload.colorS = static_cast <quint8> (s > 0xFE ? 0xFE : h);
            payload.time = qToLittleEndian <quint16> (list.value(3).toInt());

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x06).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

QVariant Actions::ColorXY::request(const QString &, const QVariant &data)
{
    switch (data.type())
    {
        case QVariant::List:
        {
            moveToColorXYStruct payload;
            QList <QVariant> list = data.toList();
            Color color(list.value(0).toDouble() / 0xFF, list.value(1).toDouble() / 0xFF, list.value(2).toDouble() / 0xFF);
            double x, y;

            color.toXY(&x, &y);
            x *= 0xFFFF;
            y *= 0xFFFF;

            payload.colorX = qToLittleEndian <quint16> (x > 0xFEFF ? 0xFEFF : x);
            payload.colorY = qToLittleEndian <quint16> (y > 0xFEFF ? 0xFEFF : y);
            payload.time = qToLittleEndian <quint16> (list.value(3).toInt());

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x07).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

QVariant Actions::ColorTemperature::request(const QString &, const QVariant &data)
{
    switch (data.type())
    {
        case QVariant::LongLong:
        {
            moveToColorTemperatureStruct payload;
            int value = data.toInt();

            payload.colorTemperature = qToLittleEndian <quint16> (value < 0x0000 ? 0x0000 : value > 0xFEFF ? 0xFEFF : value);
            payload.time = 0;

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x0A).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::List:
        {
            moveToColorTemperatureStruct payload;
            QList <QVariant> list = data.toList();
            int value = list.value(0).toInt();

            payload.colorTemperature = qToLittleEndian <quint16> (value < 0x0000 ? 0x0000 : value > 0xFEFF ? 0xFEFF : value);
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

QVariant Actions::OccupancyTimeout::request(const QString &, const QVariant &data)
{
    quint16 value = qToLittleEndian <quint16> (data.toInt());
    return writeAttribute(DATA_TYPE_16BIT_UNSIGNED, &value, sizeof(value));
}

QVariant Actions::ChildLock::request(const QString &, const QVariant &data)
{
    quint8 value = data.toBool() ? 0x01 : 0x00;
    return writeAttribute(DATA_TYPE_8BIT_ENUM, &value, sizeof(value));
}
