#include <QtEndian>
#include <QtMath>
#include "color.h"
#include "tuya.h"

void PropertiesTUYA::Data::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    QList <quint8> list = {0x01, 0x02, 0x05, 0x06};
    tuyaHeaderStruct header;
    int offset = 0;

    if (!list.contains(commandId))
        return;

    while (offset < payload.length())
    {
        QByteArray data;

        memcpy(offset ? &header.dataPoint : &header.status, payload.constData() + offset, sizeof(header) - (offset ? 2 : 0));
        data = payload.mid(sizeof(header) + (offset ? offset - 2 : 0), header.length);

        switch (header.dataType)
        {
            case TUYA_TYPE_RAW:
            {
                update(header.dataPoint, data.mid(0, header.length));
                break;
            }

            case TUYA_TYPE_BOOL:
            {
                if (header.length != 1)
                    return;

                update(header.dataPoint, data.at(0) ? true : false);
                break;
            }

            case TUYA_TYPE_VALUE:
            {
                qint32 value = 0;

                if (header.length != 4)
                    return;

                memcpy(&value, data.constData(), header.length);
                update(header.dataPoint, qFromBigEndian(value));
                break;
            }

            case TUYA_TYPE_ENUM:
            {
                if (header.length != 1)
                    return;

                update(header.dataPoint, static_cast <quint8> (data.at(0)));
                break;
            }

            default:
                return;
        }

        offset += header.length + sizeof(header) - (offset ? 2 : 0);
    }
}

void PropertiesTUYA::DataPoints::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap(), item = subOption(QString::number(dataPoint)).toMap();
    QList <QString> typeList = {"raw", "bool", "value", "enum"};
    QString name = item.value("name").toString();

    if (name.isEmpty())
        return;

    switch (typeList.indexOf(item.value("type").toString()))
    {
        case 0: // raw
        {
            QList <QString> modelList = {"_TZE200_bkkmqmyo", "_TZE200_eaac7dkw", "_TZE204_bkkmqmyo"}, nameList = name.split('_');
            QByteArray payload = data.toByteArray();
            quint16 value = 0;

            if (nameList.value(0) != "electricity")
                break;

            if (!modelList.contains(manufacturerName()))
            {
                quint8 id = static_cast <quint8> (nameList.value(1).toInt());

                memcpy(&value, payload.constData(), sizeof(value));
                map.insert(id ? QString("voltage_%1").arg(id) : "voltage", qFromBigEndian(value) / 10.0);

                memcpy(&value, payload.constData() + 3, sizeof(value));
                map.insert(id ? QString("current_%1").arg(id) : "current", qFromBigEndian(value) / 1000.0);

                memcpy(&value, payload.constData() + 6, sizeof(value));
                map.insert(id ? QString("power_%1").arg(id) : "power", qFromBigEndian(value));
            }
            else
            {
                memcpy(&value, payload.constData() + 11, sizeof(value));
                map.insert("current", qFromBigEndian(value) / 1000.0);

                memcpy(&value, payload.constData() + 13, sizeof(value));
                map.insert("voltage", qFromBigEndian(value) / 10.0);
            }

            break;
        }

        case 1: // bool
        {
            bool check = item.value("invert").toBool() ? !data.toBool() : data.toBool();
            QString value = subOption("enum", name).toStringList().value(check ? 1 : 0);

            if (value.isEmpty())
                map.insert(name, check);
            else
                map.insert(name, value);

            break;
        }

        case 2: // value
        {
            bool hasMin, hasMax;
            double min = subOption("min", name).toDouble(&hasMin), max = subOption("max", name).toDouble(&hasMax), value = data.toInt() / item.value("divider", 1).toDouble() / item.value("propertyDivider", 1).toDouble() + item.value("offset").toDouble();

            if (item.value("round").toBool())
                value = round(value);

            if ((hasMin && value < min) || (hasMax && value > max))
                break;

            map.insert(name, value);
            break;
        }

        case 3: // enum
        {
            QString value = subOption("enum", name).toStringList().value(data.toInt());

            if (!value.isEmpty())
                map.insert(name, value);
            else if (subOption("type", name).toString() == "toggle")
                map.insert(name, data.toInt() ? true : false);
            else
                map.insert(name, data.toInt());

            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::HolidayThermostatProgram::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (dataPoint == 0x70 || dataPoint == 0x71)
    {
        QList <QString> typeList = {"weekday", "holiday"}, nameList = {"Hour", "Minute", "Temperature"};
        QString type = typeList.value(dataPoint - 0x70);
        QByteArray program = data.toByteArray();

        setMeta(QString("%1Program").arg(type), true);

        for (int i = 0; i < 18; i++)
        {
            quint8 value = static_cast <quint8> (program.at(i));
            map.insert(QString("%1P%2%3").arg(type).arg(i / 3 + 1).arg(nameList.value(i % 3)), value);
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::DailyThermostatProgram::update(quint8 dataPoint, const QVariant &data)
{
    QList <QVariant> list = option("programDataPoints").toList();
    QMap <QString, QVariant> map = m_value.toMap();

    if (list.contains(dataPoint))
    {
        QList <QString> typeList = {"monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday"};
        QString type = typeList.value(list.indexOf(dataPoint));

        setMeta(QString("%1Program").arg(type), true);

        switch (option("programType").toInt())
        {
            case 1:
            {
                QByteArray program = data.toByteArray().mid(1);

                for (int i = 0; i < 6; i++)
                {
                    QString key = QString("%1P%2").arg(type).arg(i + 1);
                    quint16 time = qFromBigEndian <quint16> (*(reinterpret_cast <const quint16*> (program.constData() + i * 4))) & 0x0FFF;
                    map.insert(QString("%1Hour").arg(key), static_cast <quint8> (time / 60));
                    map.insert(QString("%1Minute").arg(key), static_cast <quint8> (time % 60));
                    map.insert(QString("%1Temperature").arg(key), (qFromBigEndian <quint16> (*(reinterpret_cast <const quint16*> (program.constData() + i * 4 + 2))) & 0x0FFF) / 10.0);
                }

                break;
            }

            case 2:
            {
                QByteArray program = data.toByteArray();

                for (int i = 0; i < 10; i++)
                {
                    QString key = QString("%1P%2").arg(type).arg(i + 1);
                    quint8 time = static_cast <quint8> (program.at(i * 3)), hour = time / 6, minute = time % 6 * 10;

                    if (hour == 24)
                    {
                        hour = 0;
                        minute = 0;
                    }

                    map.insert(QString("%1Hour").arg(key), hour);
                    map.insert(QString("%1Minute").arg(key), minute);
                    map.insert(QString("%1Temperature").arg(key), qFromBigEndian <quint16> (*(reinterpret_cast <const quint16*> (program.constData() + i * 3 + 1))) / 10.0);
                }

                break;
            }

            default:
            {
                QByteArray program = data.toByteArray().mid(1);

                for (int i = 0; i < option("programTransitions", 4).toInt(); i++)
                {
                    QString key = QString("%1P%2").arg(type).arg(i + 1);
                    map.insert(QString("%1Hour").arg(key), static_cast <quint8> (program.at(i * 4)));
                    map.insert(QString("%1Minute").arg(key), static_cast <quint8> (program.at(i * 4 + 1)));
                    map.insert(QString("%1Temperature").arg(key), qFromBigEndian <quint16> (*(reinterpret_cast <const quint16*> (program.constData() + i * 4 + 2))) / 10.0);
                }

                break;
            }
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::MoesThermostatProgram::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (dataPoint == 0x65)
    {
        QList <QString> typeList = {"weekday", "saturday", "sunday"}, nameList = {"Hour", "Minute", "Temperature"};
        QByteArray program = data.toByteArray();

        setMeta("program", true);

        for (int i = 0; i < 36; i++)
        {
            double value = static_cast <double> (program.at(i));
            map.insert(QString("%1P%2%3").arg(typeList.value(i / 12)).arg(i / 3 % 4 + 1).arg(nameList.value(i % 3)), (i + 1) % 3 ? value : value / 2);
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::LedController::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (dataPoint == 0x3D)
    {
        QList <QByteArray> list = {QByteArray::fromHex("0001001400"), QByteArray::fromHex("0000001400")};
        QByteArray payload = data.toByteArray();

        switch (list.indexOf(payload.mid(0, 5)))
        {
            case 0:
            {
                Color color = Color::fromHS(qFromBigEndian(*reinterpret_cast <const quint16*> (payload.constData() + 5)) / 360.0, qFromBigEndian(*reinterpret_cast <const quint16*> (payload.constData() + 7)) / 1000.0);
                map.insert("color", QList <QVariant> {static_cast <quint8> (color.r() * 0xFF), static_cast <quint8> (color.g() * 0xFF), static_cast <quint8> (color.b() * 0xFF)});
                map.insert("colorMode", true);
                break;
            }

            case 1:
            {
                map.insert("colorTemperature", round((1000 - qFromBigEndian(*reinterpret_cast <const quint16*> (payload.constData() + 7))) * 347 / 1000.0 + 153));
                map.insert("colorMode", false);
                break;
            }
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::CoverMotor::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (dataPoint == 0x02 || dataPoint == 0x03)
    {
        quint8 value = static_cast <quint8> (option("invertCover").toBool() ? data.toInt() : 100 - data.toInt());
        map.insert("cover", value ? "open" : "closed");
        map.insert("position", static_cast <quint8> (value));
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::CoverSwitch::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0xF000:

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("event", option("invertCover").toBool() ? "close" : "open"); break;
                case 0x01: map.insert("event", "stop"); break;
                case 0x02: map.insert("event", option("invertCover").toBool() ? "open" : "close"); break;
            }

            break;

        case 0xF001: map.insert("calibration", data.at(0) ? false : true); break;
        case 0xF002: map.insert("reverse", data.at(0) ? true : false); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::ChildLock::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributes.at(0))
        return;

    m_value = data.at(0) ? true : false;
}

void PropertiesTUYA::ButtonAction::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    switch (commandId)
    {
        case 0xFC:

            switch (static_cast <quint8> (payload.at(0)))
            {
                case 0x00: m_value = "rotateRight"; break;
                case 0x01: m_value = "rotateLeft"; break;
            }

            break;

        case 0xFD:

            switch (static_cast <quint8> (payload.at(0)))
            {
                case 0x00: m_value = "singleClick"; break;
                case 0x01: m_value = "doubleClick"; break;
                case 0x02: m_value = "hold"; break;
            }

            break;
    }
}

void PropertiesTUYA::ButtonScene::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    if (commandId != 0xFD)
        return;

    m_value = static_cast <quint8> (payload.at(2));
}

void PropertiesTUYA::IRCode::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    switch (commandId)
    {
        case 0x00:
        {
            if (payload.length() < 16)
                break;

            memcpy(&m_length, payload.constData() + 2, sizeof(m_length));
            m_length = qFromLittleEndian(m_length);
            m_buffer.clear();

            m_queue.enqueue({CLUSTER_TUYA_IR_DATA, zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId, 0x01).append(1, 0x00).append(payload)});
            m_queue.enqueue({CLUSTER_TUYA_IR_DATA, zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId, 0x02).append(payload.mid(0, 2)).append(4, 0x00).append(0x38)});
            break;
        }

        case 0x02:
        {
            quint32 position;
            quint8 size, crc = 0;
            QByteArray data;

            if (payload.length() < 7)
                break;

            memcpy(&position, payload.constData() + 2, sizeof(position));
            position = qFromLittleEndian(position);
            size = static_cast <quint8> (payload.at(payload.length() - 1));
            data = meta("message").toByteArray().mid(position, size);

            for (int i = 0; i < data.length(); i++)
                crc += data.at(i);

            m_queue.enqueue({CLUSTER_TUYA_IR_DATA, zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId, 0x03).append(1, 0x00).append(payload.mid(0, 6)).append(static_cast <char> (data.length())).append(data).append(static_cast <char> (crc))});
            break;
        }

        case 0x03:
        {
            quint32 position;
            quint8 crc = 0;
            QByteArray data = payload.mid(8, payload.at(7));

            for (int i = 0; i < data.length(); i++)
                crc += data.at(i);

            if (crc != static_cast <quint8> (payload.at(payload.length() - 1)))
                break;

            memcpy(&position, payload.constData() + 3, sizeof(position));
            position = qFromLittleEndian(position) + data.length();
            m_buffer.append(data);

            if (position >= m_length)
            {
                m_queue.enqueue({CLUSTER_TUYA_IR_DATA, zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId, 0x04).append(1, 0x00).append(payload.mid(1, 2)).append(2, 0x00)});
                break;
            }

            position = qToLittleEndian(position);
            m_queue.enqueue({CLUSTER_TUYA_IR_DATA, zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId, 0x02).append(payload.mid(1, 2)).append(reinterpret_cast <char*> (&position), sizeof(position)).append(0x38)});
            break;
        }

        case 0x05:
        {
            m_queue.enqueue({CLUSTER_TUYA_IR_CONTROL, zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId, 0x00).append(QJsonDocument(QJsonObject {{"study", 1}}).toJson(QJsonDocument::Compact))});
            m_value = m_buffer.toBase64();
            break;
        }
    }
}
