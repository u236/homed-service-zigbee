#include <QtEndian>
#include "tuya.h"

QByteArray ActionsTUYA::Request::makeRequest(quint8 transactionId, quint8 dataPoint, quint8 dataType, void *data, quint8 length = 0)
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
    QMap <QString, QVariant> map = option().toMap();
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
                    QList <QString> list = option(name).toStringList();
                    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));
                    bool check = item.value("invert").toBool() ? !data.toBool() : data.toBool();

                    if (value < 0)
                        value = check ? 1 : 0; // TODO: add toggle featute for status action

                    return makeRequest(m_transactionId++, static_cast <quint8> (it.key().toInt()), TUYA_TYPE_BOOL, &value);
                }

                case 1: // value
                {
                    quint32 value = static_cast <quint32> (data.toDouble() * item.value("divider", 1).toDouble());

                    // TODO: add min/max checks here

                    value = qToBigEndian(value);
                    return makeRequest(m_transactionId++, static_cast <quint8> (it.key().toInt()), TUYA_TYPE_VALUE, &value);
                }

                case 2: // enum
                {
                    qint8 value = static_cast <qint8> (option(name).toStringList().indexOf(data.toString()));

                    if (value < 0)
                        return QByteArray();

                    return makeRequest(m_transactionId++, static_cast <quint8> (it.key().toInt()), TUYA_TYPE_ENUM, &value);
                }
            }
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::LightDimmer::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // status
        {
            QList <QString> list = {"off", "on"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                value = endpointProperty()->value().toMap().value("status").toString() == "on" ? 0x00 : 0x01;

            return makeRequest(m_transactionId++, 0x01, TUYA_TYPE_BOOL, &value);
        }

        case 1: // level
        {
            quint32 value = static_cast <quint32> (data.toInt() * 1000.0 / 0xFE);

            if (value > 1000)
                value = 1000;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x02, TUYA_TYPE_VALUE, &value);
        }

        case 2: // levelMin
        {
            quint32 value = static_cast <quint32> (data.toInt() * 1000.0 / 0xFE);

            if (value > 1000)
                value = 1000;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x03, TUYA_TYPE_VALUE, &value);
        }

        case 3: // lightType
        {
            QList <QString> list = {"led", "incandescent", "halogen"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x04, TUYA_TYPE_ENUM, &value);
        }

        case 4: // levelMax
        {
            quint32 value = static_cast <quint32> (data.toInt() * 1000.0 / 0xFE);

            if (value > 1000)
                value = 1000;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x05, TUYA_TYPE_VALUE, &value);
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::ElectricityMeter::request(const QString &, const QVariant &data)
{
    QList <QString> modelList = {"_TZE200_byzdayie", "_TZE200_ewxhg6o9", "_TZE200_fsb6zw01"}, actionList = {"off", "on"};
    qint8 value = static_cast <qint8> (actionList.indexOf(data.toString()));

    if (value < 0)
        value = endpointProperty()->value().toMap().value("status").toString() == "on" ? 0x00 : 0x01;

    return makeRequest(m_transactionId++, modelList.contains(manufacturerName()) ? 0x01 : 0x10, TUYA_TYPE_BOOL, &value);
}

QByteArray ActionsTUYA::MultichannelRelay::request(const QString &name, const QVariant &data)
{
    QList <QString> list = {"off", "on"};
    qint8 index = static_cast <qint8> (m_actions.indexOf(name)), value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        value = endpointProperty()->value().toMap().value(name).toString() == "on" ? 0x00 : 0x01;

    return makeRequest(m_transactionId++, static_cast <quint8> (index < 6 ? index + 0x01 : index + 0x5F), TUYA_TYPE_BOOL, &value);
}

QByteArray ActionsTUYA::RadiatorThermostat::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // targetTemperature
        {
            qint32 value = qToBigEndian <qint32> (data.toInt() * 10);
            return makeRequest(m_transactionId++, 0x02, TUYA_TYPE_VALUE, &value);
        }

        case 1: // operationMode
        {
            QList <QString> list = {"away", "program", "manual", "comfort", "eco", "boost", "complex"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x04, TUYA_TYPE_ENUM, &value);
        }

        case 2: // childLock
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x07, TUYA_TYPE_BOOL, &value);
        }

        case 3: // temperatureOffset
        {
            qint32 value = qToBigEndian <qint32> (data.toInt() * 10);
            return makeRequest(m_transactionId++, 0x2C, TUYA_TYPE_VALUE, &value);
        }

        case 4: // temperatureLimitMin
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x66, TUYA_TYPE_VALUE, &value);
        }

        case 5: // temperatureLimitMax
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x67, TUYA_TYPE_VALUE, &value);
        }

        case 6: // boostTimeout
        {
            quint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x69, TUYA_TYPE_VALUE, &value);
        }

        case 7: // systemMode
        {
            QList <QString> list = {"auto", "heat", "off"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x6A, TUYA_TYPE_ENUM, &value);
        }

        case 8: // comfortTemperature
        {
            quint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x6B, TUYA_TYPE_VALUE, &value);
        }

        case 9: // ecoTemperature
        {
            quint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x6C, TUYA_TYPE_VALUE, &value);
        }

        case 10: // weekMode
        {
            QList <QString> list = {"5+2", "6+1", "7+0"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x6F, TUYA_TYPE_ENUM, &value);
        }

        case 11: // awayTemperature
        {
            quint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x72, TUYA_TYPE_VALUE, &value);
        }

        case 12: // awayDays
        {
            quint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x75, TUYA_TYPE_VALUE, &value);
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::MoesElectricThermostat::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // systemMode
        {
            quint8 value = data.toString() == "heat" ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x01, TUYA_TYPE_BOOL, &value);
        }

        case 1: // operationMode
        {
            quint8 value = data.toString() == "program" ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x02, TUYA_TYPE_ENUM, &value);
        }

        case 2: // targetTemperature
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x10, TUYA_TYPE_VALUE, &value);
        }

        case 3: // temperatureLimitMax
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x12, TUYA_TYPE_VALUE, &value);
        }

        case 4: // deadZoneTemperature
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x14, TUYA_TYPE_VALUE, &value);
        }

        case 5: // temperatureLimitMin
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x1A, TUYA_TYPE_VALUE, &value);
        }

        case 6: // temperatureOffset
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x1B, TUYA_TYPE_VALUE, &value);
        }

        case 7: // childLock
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x28, TUYA_TYPE_BOOL, &value);
        }

        case 8: // sensorType
        {
            QList <QString> list = {"internal", "both", "external"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x2B, TUYA_TYPE_ENUM, &value);
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::MoesRadiatorThermostat::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // operationMode
        {
            QList <QString> list = {"program", "manual", "temporary", "holiday"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x01, TUYA_TYPE_ENUM, &value);
        }

        case 1: // targetTemperature
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x02, TUYA_TYPE_VALUE, &value);
        }

        case 2: // boost
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x04, TUYA_TYPE_BOOL, &value);
        }

        case 3: // childLock
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x0D, TUYA_TYPE_BOOL, &value);
        }

        case 4: // boostTimeout
        {
            quint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x67, TUYA_TYPE_VALUE, &value);
        }

        case 5: // temperatureOffset
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x69, TUYA_TYPE_VALUE, &value);
        }

        case 6: // ecoMode
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x6A, TUYA_TYPE_BOOL, &value);
        }

        case 7: // ecoTemperature
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x6B, TUYA_TYPE_VALUE, &value);
        }

        case 8: // temperatureLimitMax
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x6C, TUYA_TYPE_VALUE, &value);
        }

        case 9: // temperatureLimitMin
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x6D, TUYA_TYPE_VALUE, &value);
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::WeekdayThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty();
    QList <QString> names = {"Hour", "Minute", "Temperature"};
    QByteArray payload;

    if (property.isNull())
        return QByteArray();

    if (m_data.isEmpty() || property->meta().value("received").toBool())
    {
        m_data = property->value().toMap();
        property->meta().insert("received", false);
    }

    m_data.insert(name, data.toDouble());

    for (int i = 0; i < 18; i++)
    {
        QString key = QString("weekdayP%1%2").arg(i / 3 + 1).arg(names.value(i % 3));

        if (!m_data.contains(key))
        return QByteArray();

        payload.append(static_cast <char> (m_data.value(key).toInt()));
    }

    return makeRequest(m_transactionId++, 0x70, TUYA_TYPE_RAW, payload.data(), 18);
}

QByteArray ActionsTUYA::HolidayThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty();
    QList <QString> names = {"Hour", "Minute", "Temperature"};
    QByteArray payload;

    if (property.isNull())
        return QByteArray();

    if (m_data.isEmpty() || property->meta().value("received").toBool())
    {
        m_data = property->value().toMap();
        property->meta().insert("received", false);
    }

    m_data.insert(name, data.toDouble());

    for (int i = 0; i < 18; i++)
    {
        QString key = QString("holidayP%1%2").arg(i / 3 + 1).arg(names.value(i % 3));

        if (!m_data.contains(key))
        return QByteArray();

        payload.append(static_cast <char> (m_data.value(key).toInt()));
    }

    return makeRequest(m_transactionId++, 0x71, TUYA_TYPE_RAW, payload.data(), 18);
}

QByteArray ActionsTUYA::MoesThermostatProgram::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty();
    QList <QString> types = {"weekday", "saturday", "sunday"}, names = {"Hour", "Minute", "Temperature"};
    QByteArray payload;

    if (property.isNull())
        return QByteArray();

    if (m_data.isEmpty() || property->meta().value("received").toBool())
    {
        m_data = property->value().toMap();
        property->meta().insert("received", false);
    }

    m_data.insert(name, data.toDouble());

    for (int i = 0; i < 36; i++)
    {
        QString key = QString("%1P%2%3").arg(types.value(i / 12)).arg(i / 3 % 4 + 1).arg(names.value(i % 3));

        if (!m_data.contains(key))
            return QByteArray();

        payload.append(static_cast <char> ((i + 1) % 3 ? m_data.value(key).toDouble() : m_data.value(key).toDouble() * 2));
    }

    return makeRequest(m_transactionId++, 0x65, TUYA_TYPE_RAW, payload.data(), 36);
}

QByteArray ActionsTUYA::NeoSiren::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // volume
        {
            QList <QString> list = {"low", "medium", "high"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x05, TUYA_TYPE_ENUM, &value);
        }

        case 1: // duration
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value > 1800)
                value = 1800;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x07, TUYA_TYPE_VALUE, &value);
        }

        case 2: // alarm
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x0D, TUYA_TYPE_BOOL, &value);
        }

        case 3: // melody
        {
            quint8 value = static_cast <quint8> (data.toInt());

            if (value < 1 || value > 18)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x15, TUYA_TYPE_ENUM, &value);
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::WaterValve::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // status
        {
            QList <QString> list = {"off", "on"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                value = endpointProperty()->value().toMap().value("status").toString() == "on" ? 0x00 : 0x01;

            return makeRequest(m_transactionId++, 0x01, TUYA_TYPE_BOOL, &value);
        }

        case 1: // timeout
        {
            quint32 value = static_cast <quint32> (data.toInt() * 60);

            if (value > 36000)
                value = 36000;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x09, TUYA_TYPE_VALUE, &value);
        }

        case 2: // threshold
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value > 100)
                value = 100;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x65, TUYA_TYPE_VALUE, &value);
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::PresenceSensor::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // sensitivity
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value > 9)
                value = 9;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x02, TUYA_TYPE_VALUE, &value);
        }

        case 1: // distanceMin
        case 2: // distanceMax
        {
            quint32 value = static_cast <quint32> (data.toDouble() * 100);

            if (value > 950)
                value = 950;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, index == 1 ? 0x03 : 0x04, TUYA_TYPE_VALUE, &value);
        }

        case 3: // detectionDelay
        {
            quint32 value = static_cast <quint32> (data.toDouble() * 10);

            if (value > 100)
                value = 100;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x65, TUYA_TYPE_VALUE, &value);
        }

        case 4: // fadingTime
        {
            quint32 value = static_cast <quint32> (data.toInt() * 10);

            if (value > 15000)
                value = 15000;

            value = qToBigEndian(value );
            return makeRequest(m_transactionId++, 0x66, TUYA_TYPE_VALUE, &value);
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::RadarSensor::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // radarSensitivity
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value > 10)
                value = 10;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x02, TUYA_TYPE_VALUE, &value);
        }

        case 1: // tumbleSwitch
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x69, TUYA_TYPE_BOOL, &value);
        }

        case 2: // tumbleAlarmTime
        {
            quint32 value = static_cast <quint32> (data.toInt() - 1);

            if (value < 1 || value > 5)
                return QByteArray();

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x6A, TUYA_TYPE_VALUE, &value);
        }

        case 3: // radarScene
        {
            QList <QString> list = {"default", "area", "toilet", "bedroom", "parlour", "office", "hotel"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x70, TUYA_TYPE_ENUM, &value);
        }

        case 4: // fallSensitivity
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value < 1 || value > 10)
                return QByteArray();

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x76, TUYA_TYPE_VALUE, &value);
        }
    }

    return QByteArray();
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

            if (value < 0)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x01, TUYA_TYPE_ENUM, &value);
        }

        case 1: // position
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value > 100)
                value = 100;

            if (option("invertCover").toBool())
                value = 100 - value;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x02, TUYA_TYPE_VALUE, &value);
        }

        case 2: // reverse
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x05, TUYA_TYPE_BOOL, &value);
        }

        case 3: // speed
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value > 255)
                value = 255;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x69, TUYA_TYPE_VALUE, &value);
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

    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_ENUM, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::ChildLock::request(const QString &, const QVariant &data)
{
    qint8 value = data.toBool() ? 0x01 : 0x00;
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_BOOLEAN, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::OperationMode::request(const QString &, const QVariant &data)
{
    QList <QString> list = {"command", "event"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_ENUM, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::IndicatorMode::request(const QString &, const QVariant &data)
{
    QList <QString> list = {"off", "default", "inverted", "on"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_ENUM, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::SwitchType::request(const QString &, const QVariant &data)
{
    QList <QString> list = {"toggle", "static", "momentary"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_ENUM, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::PowerOnStatus::request(const QString &, const QVariant &data)
{
    QList <QString> list = {"off", "on", "previous"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_ENUM, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}
