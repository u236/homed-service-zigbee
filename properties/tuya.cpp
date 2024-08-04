#include <QtEndian>
#include <QtMath>
#include "tuya.h"

void PropertiesTUYA::Data::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    const tuyaHeaderStruct *header = reinterpret_cast <const tuyaHeaderStruct*> (payload.constData());
    QVariant data;

    switch (commandId)
    {
        case 0x01:
        case 0x02:
        case 0x05:
        case 0x06:
            data = parseData(header, payload.mid(sizeof(tuyaHeaderStruct)));
            break;

        default:
            return;
    }

    if (!data.isValid())
        return;

    update(header->dataPoint, data);
}

QVariant PropertiesTUYA::Data::parseData(const tuyaHeaderStruct *header, const QByteArray &data)
{
    switch (header->dataType)
    {
        case TUYA_TYPE_RAW:
            return data.mid(0, header->length);

        case TUYA_TYPE_BOOL:

            if (header->length == 1)
                return data.at(0) ? true : false;

            break;

        case TUYA_TYPE_VALUE:

            if (header->length == 4)
            {
                quint32 value = 0;
                memcpy(&value, data.constData(), header->length);
                return qFromBigEndian(value);
            }

            break;

        case TUYA_TYPE_ENUM:

            if (header->length == 1)
                return static_cast <quint8> (data.at(0));

            break;
    }

    return QVariant();
}

void PropertiesTUYA::DataPoints::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();
    QList <QVariant> list = option().toMap().value(QString::number(dataPoint)).toList();
    QList <QString> types = {"raw", "bool", "value", "enum"};

    for (int i = 0; i < list.count(); i++)
    {
        QMap <QString, QVariant> item = list.at(i).toMap(), options;
        QString name = item.value("name").toString();

        if(name.isEmpty())
            continue;

        options = option(name).toMap();

        switch (types.indexOf(item.value("type").toString()))
        {
            case 0: // raw
            {
                QList <QString> modelList = {"_TZE200_bcusnqt8", "_TZE200_lsanae15", "_TZE200_ves1ycwx", "_TZE204_lsanae15", "_TZE204_ves1ycwx"}, nameList = name.split('_');
                QByteArray payload = data.toByteArray();
                quint16 value = 0;

                if (nameList.value(0) != "elictricity")
                    break;

                if (modelList.contains(manufacturerName()))
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
                QString value = option(name).toMap().value("enum").toStringList().value(check ? 1 : 0);

                if (value.isEmpty())
                    map.insert(name, check);
                else
                    map.insert(name, value);

                break;
            }

            case 2: // value
            {
                bool hasMin, hasMax;
                double min = options.value("min").toDouble(&hasMin), max = options.value("max").toDouble(&hasMax), value = data.toInt() / item.value("divider", 1).toDouble() / item.value("propertyDivider", 1).toDouble() + item.value("offset").toDouble();

                if (item.value("round").toBool())
                    value = round(value);

                if ((hasMin && value < min) || (hasMax && value > max))
                    break;

                map.insert(name, value);
                break;
            }

            case 3: // enum
            {
                if (options.contains("enum"))
                {
                    QString value = options.value("enum").toStringList().value(data.toInt());

                    if (!value.isEmpty())
                        map.insert(name, value);

                    break;
                }

                map.insert(name, data.toInt());
                break;
            }

            default:
            {
                QVariant value = item.value("value");

                if (value.isValid())
                    map.insert(name, value);

                break;
            }
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::HolidayThermostatProgram::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (dataPoint == 0x70 || dataPoint == 0x71)
    {
        QList <QString> types = {"weekday", "holiday"}, names = {"Hour", "Minute", "Temperature"};
        QString type = types.value(dataPoint - 0x70);
        QByteArray program = data.toByteArray();

        meta().insert(QString("%1Program").arg(type), true);

        for (int i = 0; i < 18; i++)
        {
            quint8 value = static_cast <quint8> (program.at(i));
            map.insert(QString("%1P%2%3").arg(type).arg(i / 3 + 1).arg(names.value(i % 3)), value);
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::DailyThermostatProgram::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (dataPoint >= 0x1C && dataPoint <= 0x22)
    {
        QList <QString> types = {"monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday"};
        QString type = types.value(dataPoint - 0x1C);
        QByteArray program = data.toByteArray().mid(1);

        meta().insert(QString("%1Program").arg(type), true);

        for (int i = 0; i < 4; i++)
        {
            QString key = QString("%1P%2").arg(type).arg(i + 1);
            quint16 tempertarure;

            memcpy(&tempertarure, program.constData() + i * 4 + 2, sizeof(tempertarure));

            map.insert(QString("%1Hour").arg(key), static_cast <quint8> (program.at(i * 4)));
            map.insert(QString("%1Minute").arg(key), static_cast <quint8> (program.at(i * 4 + 1)));
            map.insert(QString("%1Temperature").arg(key), qFromBigEndian(tempertarure) / 10.0);
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::MoesThermostatProgram::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (dataPoint == 0x65)
    {
        QList <QString> types = {"weekday", "saturday", "sunday"}, names = {"Hour", "Minute", "Temperature"};
        QByteArray program = data.toByteArray();

        meta().insert("program", true);

        for (int i = 0; i < 36; i++)
        {
            double value = static_cast <double> (program.at(i));
            map.insert(QString("%1P%2%3").arg(types.value(i / 12)).arg(i / 3 % 4 + 1).arg(names.value(i % 3)), (i + 1) % 3 ? value : value / 2);
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

void PropertiesTUYA::CoverSwitch::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0xF000:

            switch (data.at(0))
            {
                case 0: map.insert("event", option("invertCover").toBool() ? "close" : "open"); break;
                case 1: map.insert("event", "stop"); break;
                case 2: map.insert("event", option("invertCover").toBool() ? "open" : "close"); break;
            }

            break;

        case 0xF001: map.insert("calibration", data.at(0) ? false : true); break;
        case 0xF002: map.insert("reverse", data.at(0) ? true : false); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::ChildLock::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x8000)
        return;

    m_value = data.at(0) ? true : false;
}

void PropertiesTUYA::ButtonAction::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    switch (commandId)
    {
        case 0xFC:

            switch (payload.at(0))
            {
                case 0x00: m_value = "rotateRight"; break;
                case 0x01: m_value = "rotateLeft"; break;
            }

            break;

        case 0xFD:

            switch (payload.at(0))
            {
                case 0x00: m_value = "singleClick"; break;
                case 0x01: m_value = "doubleClick"; break;
                case 0x02: m_value = "hold"; break;
            }

            break;
    }
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
            data = meta().value("message").toByteArray().mid(position, size);

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
