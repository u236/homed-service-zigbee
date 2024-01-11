#include <QtEndian>
#include <QtMath>
#include "tuya.h"

void PropertiesTUYA::Data::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    const tuyaHeaderStruct *header = reinterpret_cast <const tuyaHeaderStruct*> (payload.constData());
    QVariant data;

    if (commandId != 0x01 && commandId != 0x02)
        return;

    data = parseData(header, payload.mid(sizeof(tuyaHeaderStruct)));

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
                quint32 value;
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
                QList <QString> list = {"_TZE200_bcusnqt8", "_TZE200_lsanae15", "_TZE204_lsanae15"};
                QByteArray payload = data.toByteArray();
                quint16 value;

                if (name != "elictricity")
                    break;

                if (list.contains(manufacturerName()))
                {
                    memcpy(&value, payload.constData(), sizeof(value));
                    map.insert("voltage", qFromBigEndian(value) / 10.0);

                    memcpy(&value, payload.constData() + 3, sizeof(value));
                    map.insert("current", qFromBigEndian(value) / 1000.0);

                    memcpy(&value, payload.constData() + 6, sizeof(value));
                    map.insert("power", qFromBigEndian(value));
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
                double min = options.value("min").toDouble(&hasMin), max = options.value("max").toDouble(&hasMax), value = data.toInt() / item.value("divider", 1).toDouble() + item.value("offset").toDouble();

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

void PropertiesTUYA::WeekdayThermostatProgram::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (dataPoint == 0x70)
    {
        QList <QString> names = {"Hour", "Minute", "Temperature"};
        QByteArray program = data.toByteArray();

        meta().insert("program", true);

        for (int i = 0; i < 18; i++)
        {
            quint8 value = static_cast <quint8> (program.at(i));
            map.insert(QString("weekdayP%1%2").arg(i / 3 + 1).arg(names.value(i % 3)), value);
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesTUYA::HolidayThermostatProgram::update(quint8 dataPoint, const QVariant &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (dataPoint == 0x71)
    {
        QList <QString> names = {"Hour", "Minute", "Temperature"};
        QByteArray program = data.toByteArray();

        meta().insert("program", true);

        for (int i = 0; i < 18; i++)
        {
            quint8 value = static_cast <quint8> (program.at(i));
            map.insert(QString("holidayP%1%2").arg(i / 3 + 1).arg(names.value(i % 3)), value);
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

void PropertiesTUYA::OperationMode::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x8004)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "command"; break;
        case 0x01: m_value = "event"; break;
    }
}

void PropertiesTUYA::IndicatorMode::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x8001)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "default"; break;
        case 0x02: m_value = "inverted"; break;
        case 0x03: m_value = "on"; break;
    }
}

void PropertiesTUYA::SensitivityMode::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0013)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "low"; break;
        case 0x01: m_value = "medium"; break;
        case 0x02: m_value = "high"; break;
    }
}

void PropertiesTUYA::TimeoutMode::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0xF001)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "30s"; break;
        case 0x01: m_value = "60s"; break;
        case 0x02: m_value = "120s"; break;
    }
}

void PropertiesTUYA::SwitchType::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0xD030)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "toggle"; break;
        case 0x01: m_value = "static"; break;
        case 0x02: m_value = "momentary"; break;
    }
}

void PropertiesTUYA::PowerOnStatus::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x8002)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
        case 0x02: m_value = "previous"; break;
    }
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
