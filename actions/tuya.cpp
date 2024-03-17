#include <QtEndian>
#include "tuya.h"

QByteArray ActionsTUYA::Request::makeRequest(quint8 transactionId, quint8 dataPoint, quint8 dataType, void *data, quint8 length)
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

    return zclHeader(FC_CLUSTER_SPECIFIC, transactionId, 0x00).append(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (data), header.length);
}

QByteArray ActionsTUYA::DataPoints::request(const QString &name, const QVariant &data)
{
    QMap <QString, QVariant> map = option().toMap(), options = option(name).toMap();
    QList <QString> types = {"bool", "value", "enum"};

    for (auto it = map.begin(); it != map.end(); it++)
    {
        QList <QVariant> list = it.value().toList();

        for (int i = 0; i < list.count(); i++)
        {
            QMap <QString, QVariant> item = list.at(i).toMap();

            if (item.value("name").toString() != name || !item.value("action").toBool())
                continue;

            switch (types.indexOf(item.value("type").toString()))
            {
                case 0: // bool
                {
                    QList <QString> nameList = name.split('_'), actionList = option(name).toMap().value("enum").toStringList();
                    qint8 value = static_cast <qint8> (actionList.indexOf(data.toString()));
                    bool check = item.value("invert").toBool() ? !data.toBool() : data.toBool();

                    if (value < 0)
                        value = nameList.value(0) == "status" ? (endpointProperty()->value().toMap().value(name).toString() != "on" ? 0x01 : 0x00) : (check ? 0x01 : 0x00);

                    return makeRequest(m_transactionId++, static_cast <quint8> (it.key().toInt()), TUYA_TYPE_BOOL, &value);
                }

                case 1: // value
                {
                    bool hasMin, hasMax;
                    double check = data.toDouble(), min = options.value("min").toDouble(&hasMin), max = options.value("max").toDouble(&hasMax);
                    qint32 value;

                    if (hasMin && check < min)
                        check = min;

                    if (hasMax && check > max)
                        check = max;

                    value = qToBigEndian <qint32> (check * item.value("divider", 1).toDouble() * item.value("actionDivider", 1).toDouble() - item.value("offset").toDouble());
                    return makeRequest(m_transactionId++, static_cast <quint8> (it.key().toInt()), TUYA_TYPE_VALUE, &value);
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

                    return makeRequest(m_transactionId++, static_cast <quint8> (it.key().toInt()), TUYA_TYPE_ENUM, &value);
                }
            }
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::HolidayThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty();
    QList <QString> types = {"weekday", "holiday"};
    QString type = name.mid(0, name.indexOf('P'));
    QByteArray payload;

    if (m_data.isEmpty() || meta().value("program").toBool())
    {
        m_data = property->value().toMap();
        meta().insert(QString("%1Program").arg(type), false);
    }

    m_data.insert(name, data.toDouble());

    for (int i = 0; i < 6; i++)
    {
        QString key = QString("%1P%2").arg(type).arg(i + 1);
        payload.append(static_cast <char> (m_data.value(QString("%1Hour").arg(key), i * 4).toInt()));
        payload.append(static_cast <char> (m_data.value(QString("%1Minute").arg(key), 0).toInt()));
        payload.append(static_cast <char> (m_data.value(QString("%1Temperature").arg(key), 21).toInt()));
    }

    return makeRequest(m_transactionId++, static_cast <quint8> (0x70 + types.indexOf(type)), TUYA_TYPE_RAW, payload.data(), static_cast <quint8> (payload.length()));
}

QByteArray ActionsTUYA::DailyThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty();
    QList <QString> types = {"monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday"};
    QString type = name.mid(0, name.indexOf('P'));
    QByteArray payload = QByteArray(1, static_cast <char> (types.indexOf(type) + 1));

    if (m_data.isEmpty() || meta().value("program").toBool())
    {
        m_data = property->value().toMap();
        meta().insert(QString("%1Program").arg(type), false);
    }

    m_data.insert(name, data.toDouble());

    for (int i = 0; i < 4; i++)
    {
        QString key = QString("%1P%2").arg(type).arg(i + 1);
        quint16 temperature = qToBigEndian <quint16> (m_data.value(QString("%1Temperature").arg(key), 21).toDouble() * 10);
        payload.append(static_cast <char> (m_data.value(QString("%1Hour").arg(key), i * 6).toInt()));
        payload.append(static_cast <char> (m_data.value(QString("%1Minute").arg(key), 0).toInt()));
        payload.append(reinterpret_cast <char*> (&temperature), sizeof(temperature));
    }

    return makeRequest(m_transactionId++, static_cast <quint8> (0x1C + types.indexOf(type)), TUYA_TYPE_RAW, payload.data(), static_cast <quint8> (payload.length()));
}

QByteArray ActionsTUYA::MoesThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty();
    QList <QString> types = {"weekday", "saturday", "sunday"};
    QByteArray payload;

    if (m_data.isEmpty() || meta().value("program").toBool())
    {
        m_data = property->value().toMap();
        meta().insert("program", false);
    }

    m_data.insert(name, data.toDouble());

    for (int i = 0; i < 12; i++)
    {
        QString key = QString("%1P%2").arg(types.value(i / 4)).arg(i % 4 + 1);
        payload.append(static_cast <char> (m_data.value(QString("%1Hour").arg(key), i % 4 * 6).toInt()));
        payload.append(static_cast <char> (m_data.value(QString("%1Minute").arg(key), 0).toInt()));
        payload.append(static_cast <char> (m_data.value(QString("%1Temperature").arg(key), 21).toInt() * 2));
    }

    return makeRequest(m_transactionId++, 0x65, TUYA_TYPE_RAW, payload.data(), static_cast <quint8> (payload.length()));
}

QByteArray ActionsTUYA::CoverMotor::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // cover
        {
            QList <QString> modelList = {"_TYST11_cowvfni3", "_TZE200_cowvfni3", "_TZE200_wmcdj3aq", "_TZE204_r0jdjrvi"}, actionList = modelList.contains(manufacturerName()) ? QList <QString> {"close", "stop", "open"} : QList <QString> {"open", "stop", "close"};
            qint8 value;

            if (option("invertCover").toBool())
                actionList = QList <QString> (actionList.rbegin(), actionList.rend());

            value = static_cast <qint8> (actionList.indexOf(data.toString()));
            return value < 0 ? QByteArray() : makeRequest(m_transactionId++, 0x01, TUYA_TYPE_ENUM, &value);
        }

        case 1: // position
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value > 100)
                value = 100;

            if (!option("invertCover").toBool())
                value = 100 - value;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x02, TUYA_TYPE_VALUE, &value);
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::CoverSwitch::request(const QString &name, const QVariant &data)
{
    qint8 value;

    switch (m_actions.indexOf(name))
    {
        case 0: m_attributes = {0xF001}; value = data.toBool() ? 0x00 : 0x01; break; // calibration
        case 1: m_attributes = {0xF002}; value = data.toBool() ? 0x01 : 0x00; break; // reverse
    }

    return writeAttribute(DATA_TYPE_8BIT_ENUM, &value, sizeof(value));
}

QByteArray ActionsTUYA::ChildLock::request(const QString &, const QVariant &data)
{
    qint8 value = data.toBool() ? 0x01 : 0x00;
    return writeAttribute(DATA_TYPE_BOOLEAN, &value, sizeof(value));
}
