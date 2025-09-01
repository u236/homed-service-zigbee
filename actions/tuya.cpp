#include <QtEndian>
#include "color.h"
#include "tuya.h"

QByteArray ActionsTUYA::Request::makeRequest(quint8 transactionId, quint8 commandId, quint8 dataPoint, quint8 dataType, void *data, quint8 length)
{
    tuyaHeaderStruct header;

    header.status = 0x00;
    header.transactionId = transactionId;
    header.dataPoint = dataPoint;
    header.dataType = dataType;
    header.function = 0x00;

    switch (header.dataType)
    {
        case TUYA_TYPE_RAW:
            header.length = length;
            break;

        case TUYA_TYPE_BOOL:
        case TUYA_TYPE_ENUM:
            header.length = 1;
            break;

        case TUYA_TYPE_VALUE:
            header.length = 4;
            break;

        default:
            return QByteArray();
    }

    return zclHeader(FC_CLUSTER_SPECIFIC, transactionId, commandId).append(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (data), header.length);
}

QVariant ActionsTUYA::DataPoints::request(const QString &name, const QVariant &data)
{
    QMap <QString, QVariant> map = option().toMap(), options = option(name).toMap();
    QList <QString> typeList = {"bool", "value", "enum"};
    quint8 commandId = option("tuyaCustomCommand").toBool() ? 0x04 : 0x00;

    for (auto it = map.begin(); it != map.end(); it++)
    {
        QMap <QString, QVariant> item = it.value().toMap();

        if (item.value("name").toString() != name || !item.value("action").toBool())
            continue;

        switch (typeList.indexOf(item.value("type").toString()))
        {
            case 0: // bool
            {
                QList <QString> actionList = option(name).toMap().value("enum").toStringList(), nameList = name.split('_');
                qint8 value = static_cast <qint8> (actionList.indexOf(data.toString()));
                bool check = item.value("invert").toBool() ? !data.toBool() : data.toBool();

                if (value < 0)
                    value = nameList.value(0) == "status" ? (endpointProperty()->value().toMap().value(name).toString() != "on" ? 0x01 : 0x00) : (check ? 0x01 : 0x00);

                return makeRequest(m_transactionId++, commandId, static_cast <quint8> (it.key().toInt()), TUYA_TYPE_BOOL, &value);
            }

            case 1: // value
            {
                bool hasMin, hasMax;
                double number = data.toDouble(), min = options.value("min").toDouble(&hasMin), max = options.value("max").toDouble(&hasMax);
                qint32 value;

                if (hasMin && number < min)
                    number = min;

                if (hasMax && number > max)
                    number = max;

                value = qToBigEndian <qint32> ((number - item.value("offset").toDouble()) * item.value("divider", 1).toDouble() * item.value("actionDivider", 1).toDouble());
                return makeRequest(m_transactionId++, commandId, static_cast <quint8> (it.key().toInt()), TUYA_TYPE_VALUE, &value);
            }

            case 2: // enum
            {
                qint8 value;

                if (options.contains("min") && options.contains("max"))
                {
                    qint8 min = static_cast <qint8> (options.value("min").toInt()), max = static_cast <qint8> (options.value("max").toInt());

                    value = static_cast <qint8> (data.toInt());

                    if (value < min)
                        value = min;

                    if (value > max)
                        value = max;
                }
                else
                {
                    value = static_cast <qint8> (options.value("enum").toStringList().indexOf(data.toString()));

                    if (value < 0)
                        return QByteArray();
                }

                return makeRequest(m_transactionId++, commandId, static_cast <quint8> (it.key().toInt()), TUYA_TYPE_ENUM, &value);
            }
        }
    }

    return QByteArray();
}

QVariant ActionsTUYA::HolidayThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty("holidayThermostatProgram");
    QList <QString> typeList = {"weekday", "holiday"};
    QString type = name.mid(0, name.indexOf('P'));
    QByteArray payload;

    if (m_data.isEmpty() || meta(QString("%1Program").arg(type)).toBool())
    {
        m_data = property->value().toMap();
        setMeta(QString("%1Program").arg(type), false);
    }

    m_data.insert(name, data);

    for (int i = 0; i < 6; i++)
    {
        QString key = QString("%1P%2").arg(type).arg(i + 1);
        payload.append(static_cast <char> (m_data.value(QString("%1Hour").arg(key), i * 4).toInt()));
        payload.append(static_cast <char> (m_data.value(QString("%1Minute").arg(key), 0).toInt()));
        payload.append(static_cast <char> (m_data.value(QString("%1Temperature").arg(key), 21).toInt()));
    }

    return makeRequest(m_transactionId++, 0x00, static_cast <quint8> (0x70 + typeList.indexOf(type)), TUYA_TYPE_RAW, payload.data(), static_cast <quint8> (payload.length()));
}

QVariant ActionsTUYA::DailyThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty("dailyThermostatProgram");
    QList <QVariant> list = option("programDataPoints").toList();
    QList <QString> modelList = {"_TZE204_ltwbm23f", "_TZE204_qyr2m29i"}, typeList = {"monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday"};
    QString type = name.mid(0, name.indexOf('P'));
    QByteArray payload = QByteArray(1, static_cast <char> (typeList.indexOf(type) + 1));

    if (m_data.isEmpty() || meta(QString("%1Program").arg(type)).toBool())
    {
        m_data = property->value().toMap();
        setMeta(QString("%1Program").arg(type), false);
    }

    m_data.insert(name, data);

    if (!modelList.contains(manufacturerName()))
    {
        int transitions = option("programTransitions", 4).toInt();

        for (int i = 0; i < transitions; i++)
        {
            QString key = QString("%1P%2").arg(type).arg(i + 1);
            quint16 temperature = qToBigEndian <quint16> (m_data.value(QString("%1Temperature").arg(key), 21).toDouble() * 10);
            payload.append(static_cast <char> (m_data.value(QString("%1Hour").arg(key), i * (24 / transitions)).toInt()));
            payload.append(static_cast <char> (m_data.value(QString("%1Minute").arg(key), 0).toInt()));
            payload.append(reinterpret_cast <char*> (&temperature), sizeof(temperature));
        }
    }
    else
    {
        for (int i = 0; i < 6; i++)
        {
            QString key = QString("%1P%2").arg(type).arg(i + 1);
            quint16 time = qToBigEndian <quint16> (0xA000 | static_cast <quint16> (m_data.value(QString("%1Hour").arg(key), i * 4).toInt() * 60 + m_data.value(QString("%1Minute").arg(key), 0).toInt()));
            quint16 temperature = qToBigEndian <quint16> (0x4000 | static_cast <quint16> (m_data.value(QString("%1Temperature").arg(key), 21).toDouble() * 10));
            payload.append(reinterpret_cast <char*> (&time), sizeof(time));
            payload.append(reinterpret_cast <char*> (&temperature), sizeof(temperature));
        }
    }

    return makeRequest(m_transactionId++, 0x00, static_cast <quint8> (list.value(typeList.indexOf(type)).toInt()), TUYA_TYPE_RAW, payload.data(), static_cast <quint8> (payload.length()));
}

QVariant ActionsTUYA::MoesThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty("moesThermostatProgram");
    QList <QString> typeList = {"weekday", "saturday", "sunday"};
    QByteArray payload;

    if (m_data.isEmpty() || meta("program").toBool())
    {
        m_data = property->value().toMap();
        setMeta("program", false);
    }

    m_data.insert(name, data);

    for (int i = 0; i < 12; i++)
    {
        QString key = QString("%1P%2").arg(typeList.value(i / 4)).arg(i % 4 + 1);
        payload.append(static_cast <char> (m_data.value(QString("%1Hour").arg(key), i % 4 * 6).toInt()));
        payload.append(static_cast <char> (m_data.value(QString("%1Minute").arg(key), 0).toInt()));
        payload.append(static_cast <char> (m_data.value(QString("%1Temperature").arg(key), 21).toInt() * 2));
    }

    return makeRequest(m_transactionId++, 0x00, 0x65, TUYA_TYPE_RAW, payload.data(), static_cast <quint8> (payload.length()));
}

QVariant ActionsTUYA::LedController::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty("tuyaDataPoints");
    quint16 level = property->value().toMap().value("level", 255).toDouble() * 1000 / 255;

    switch (m_actions.indexOf(name))
    {
        case 0: // color
        {
            QByteArray payload = QByteArray::fromHex("0001001400");
            QList <QVariant> list = data.toList();
            Color color(list.value(0).toDouble() / 0xFF, list.value(1).toDouble() / 0xFF, list.value(2).toDouble() / 0xFF);
            double h, s;
            quint16 value[3];

            color.toHS(&h, &s);

            value[0] = qToBigEndian <quint16> (h * 360);
            value[1] = qToBigEndian <quint16> (s * 1000);
            value[2] = qToBigEndian <quint16> (level);

            payload.append(reinterpret_cast <char*> (&value), sizeof(value));
            return makeRequest(m_transactionId++, 0x00, 0x3D, TUYA_TYPE_RAW, payload.data(), static_cast <quint8> (payload.length()));
        }

        case 1: // colorTemperature
        {
            QByteArray payload = QByteArray::fromHex("0000001400");
            quint16 value[2] = {qToBigEndian <quint16> (level), qToBigEndian <quint16> (1000 - (data.toDouble() - 153) * 1000 / 347)};
            payload.append(reinterpret_cast <char*> (&value), sizeof(value));
            return makeRequest(m_transactionId++, 0x00, 0x3D, TUYA_TYPE_RAW, payload.data(), static_cast <quint8> (payload.length()));
        }
    }

    return QByteArray();
}

QVariant ActionsTUYA::CoverMotor::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // cover
        {
            QList <QString> list = QList <QString> {"open", "stop", "close"};
            qint8 value;

            if (option("invertCover").toBool() != option("invertControls").toBool())
                list = QList <QString> (list.rbegin(), list.rend());

            value = static_cast <qint8> (list.indexOf(data.toString()));
            return value < 0 ? QByteArray() : makeRequest(m_transactionId++, 0x00, 0x01, TUYA_TYPE_ENUM, &value);
        }

        case 1: // position
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value > 100)
                value = 100;

            if (!option("invertCover").toBool())
                value = 100 - value;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x00, 0x02, TUYA_TYPE_VALUE, &value);
        }
    }

    return QByteArray();
}

QVariant ActionsTUYA::CoverSwitch::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: return writeAttribute(0xF001, DATA_TYPE_8BIT_ENUM, QByteArray(1, data.toBool() ? 0x00 : 0x01)); // calibration
        case 1: return writeAttribute(0xF002, DATA_TYPE_8BIT_ENUM, QByteArray(1, data.toBool() ? 0x01 : 0x00)); // reverse
    }

    return QByteArray();
}

QVariant ActionsTUYA::ChildLock::request(const QString &, const QVariant &data)
{
    qint8 value = data.toBool() ? 0x01 : 0x00;
    return writeAttribute(DATA_TYPE_BOOLEAN, &value, sizeof(value));
}

QVariant ActionsTUYA::IRCode::request(const QString &, const QVariant &data)
{
    QByteArray message = QJsonDocument(QJsonObject {{"delay", 300}, {"key1", QJsonObject {{"freq", 38000}, {"key_code", data.toString()}, {"num", 1}, {"type", 1}}}, {"key_num", 1}}).toJson(QJsonDocument::Compact);
    quint32 length = qToLittleEndian <quint32> (message.length());
    setMeta("message", message);
    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x00).append(2, 0x00).append(reinterpret_cast <char*> (&length), sizeof(length)).append(QByteArray::fromHex("0000000004e001020000"));
}

QVariant ActionsTUYA::IRLearn::request(const QString &, const QVariant &data)
{
    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x00).append(QJsonDocument(QJsonObject {{"study", data.toBool() ? 0 : 1}}).toJson(QJsonDocument::Compact));
}
