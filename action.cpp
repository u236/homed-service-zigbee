#include <QtEndian>
#include "action.h"
#include "color.h"
#include "device.h"

void ActionObject::registerMetaTypes(void)
{
    qRegisterMetaType <Actions::Status>                     ("statusAction");
    qRegisterMetaType <Actions::PowerOnStatus>              ("powerOnStatusAction");
    qRegisterMetaType <Actions::Level>                      ("levelAction");
    qRegisterMetaType <Actions::CoverStatus>                ("coverStatusAction");
    qRegisterMetaType <Actions::ColorHS>                    ("colorHSAction");
    qRegisterMetaType <Actions::ColorXY>                    ("colorXYAction");
    qRegisterMetaType <Actions::ColorTemperature>           ("colorTemperatureAction");

    qRegisterMetaType <ActionsPTVO::ChangePattern>          ("ptvoChangePatternAction");
    qRegisterMetaType <ActionsPTVO::Count>                  ("ptvoCountAction");
    qRegisterMetaType <ActionsPTVO::Pattern>                ("ptvoPatternAction");

    qRegisterMetaType <ActionsLUMI::PresenceSensor>         ("lumiPresenceSensorAction");
    qRegisterMetaType <ActionsLUMI::ButtonMode>             ("lumiButtonModeAction");
    qRegisterMetaType <ActionsLUMI::OperationMode>          ("lumiOperationModeAction");
    qRegisterMetaType <ActionsLUMI::CoverPosition>          ("lumiCoverPositionAction");

    qRegisterMetaType <ActionsTUYA::ElectricityMeter>       ("tuyaElectricityMeterAction");
    qRegisterMetaType <ActionsTUYA::MoesThermostat>         ("tuyaMoesThermostatAction");
    qRegisterMetaType <ActionsTUYA::NeoSiren>               ("tuyaNeoSirenAction");
    qRegisterMetaType <ActionsTUYA::WaterValve>             ("tuyaWaterValveAction");
    qRegisterMetaType <ActionsTUYA::PresenceSensor>         ("tuyaPresenceSensorAction");
    qRegisterMetaType <ActionsTUYA::ChildLock>              ("tuyaChildLockAction");
    qRegisterMetaType <ActionsTUYA::OperationMode>          ("tuyaOperationModeAction");
    qRegisterMetaType <ActionsTUYA::BacklightMode>          ("tuyaBacklightModeAction");
    qRegisterMetaType <ActionsTUYA::IndicatorMode>          ("tuyaIndicatorModeAction");
    qRegisterMetaType <ActionsTUYA::SwitchMode>             ("tuyaSwitchModeAction");
    qRegisterMetaType <ActionsTUYA::PowerOnStatus>          ("tuyaPowerOnStatusAction");

    qRegisterMetaType <ActionsOther::EfektaReportingDelay>  ("efektaReportingDelayAction");
    qRegisterMetaType <ActionsOther::PerenioSmartPlug>      ("perenioSmartPlugAction");
}

QVariant ActionObject::endpointOption(const QString &name)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    QVariant value;

    if (!endpoint || endpoint->device().isNull())
        return value;

    value = endpoint->device()->options().value(QString("%1-%2").arg(name, QString::number(endpoint->id())));
    return value.isValid() ? value : endpoint->device()->options().value(name);
}

Property ActionObject::endpointProperty(const QString &name)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);

    if (endpoint)
    {
        for (int i = 0; i < endpoint->properties().count(); i++)
        {
            const Property &property = endpoint->properties().at(i);

            if (property->name() != name)
                continue;

            return property;
        }
    }

    return Property();
}

QByteArray Actions::Status::request(const QString &, const QVariant &data)
{
    QList <QString> list = {"off", "on", "toggle"};
    qint8 command = static_cast <qint8> (list.indexOf(data.toString()));

    if (command < 0)
        return QByteArray();

    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, static_cast <quint8> (command));
}

QByteArray Actions::PowerOnStatus::request(const QString &, const QVariant &data)
{
    QList <QString> list = {"off", "on", "toggle", "previous"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0 || value > 2)
        value = 0xFF;

    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_ENUM, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
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

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x00).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::List:
        {
            QList <QVariant> list = data.toList();
            moveToLevelStruct payload;

            payload.level = static_cast <quint8> (list.value(0).toInt() < 0xFE ? list.value(0).toInt() : 0xFE);
            payload.time = qToLittleEndian <quint16> (list.value(1).toInt());

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x00).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::String:
        {
            QString action = data.toString();

            if (action != "stopLevel") // TODO: add step actions
            {
                quint8 payload[2] = {static_cast <quint8> (action == "moveLevelUp" ? 0x00 : 0x01), 0x55}; // TODO: check this
                return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x01).append(reinterpret_cast <char*> (&payload), sizeof(payload));
            }

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x07);
        }

        default:
            return QByteArray();
    }
}

QByteArray Actions::CoverStatus::request(const QString &, const QVariant &data)
{
    QList <QString> list = {"open", "close", "stop"};
    qint8 command = static_cast <qint8> (list.indexOf(data.toString()));

    if (command < 0)
        return QByteArray();

    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, static_cast <quint8> (command));
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

            payload.temperature = qToLittleEndian <quint16> (data.toInt() < 0xFEFF ? data.toInt() : 0xFEFF);
            payload.time = 0;

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x0A).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        case QVariant::List:
        {
            QList <QVariant> list = data.toList();
            moveToColorTemperatureStruct payload;

            payload.temperature = qToLittleEndian <quint16> (list.value(0).toInt() < 0xFEFF ? list.value(0).toInt() : 0xFEFF);
            payload.time = qToLittleEndian <quint16> (list.value(1).toInt());

            return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x0A).append(reinterpret_cast <char*> (&payload), sizeof(payload));
        }

        default:
            return QByteArray();
    }
}

QByteArray ActionsPTVO::Status::request(const QString &, const QVariant &data)
{
    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, data.toBool() ? 0x01 : 0x00);
}

QByteArray ActionsPTVO::AnalogInput::request(const QString &, const QVariant &data)
{
    float value = qToLittleEndian(data.toFloat());
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_SINGLE_PRECISION, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::PresenceSensor::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // sensitivity
        {
            QList <QString> list = {"low", "medium", "high"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()) + 1);

            m_attributes = {0x010C};

            if (value < 1)
                return QByteArray();

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 1: // mode
        {
            QList <QString> list = {"undirected", "directed"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            m_attributes = {0x0144};

            if (value < 0)
                return QByteArray();

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 2: // distance
        {
            QList <QString> list = {"far", "middle", "near"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            m_attributes = {0x0146};

            if (value < 0)
                return QByteArray();

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 3: // resetPresence
        {
            m_attributes.clear();

            if (!data.toBool())
                return QByteArray();

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, 0x0157, DATA_TYPE_8BIT_UNSIGNED, QByteArray(1, 0x01)); // TODO: check payload
        }
    }

    return QByteArray();
}

QByteArray ActionsLUMI::ButtonMode::request(const QString &name, const QVariant &data)
{
    QList <QString> list = {"relay", "leftRelay", "rightRelay", "decoupled"};
    qint8 value;

    switch (m_actions.indexOf(name))
    {
        case 2:  m_attributes = {0xFF23}; break; // rightMode
        default: m_attributes = {0xFF22}; break; // mode, leftMode
    }

    switch (list.indexOf(data.toString()))
    {
        case 0:  value = 0x12; break;
        case 1:  value = 0x12; break;
        case 2:  value = 0x22; break;
        case 3:  value = 0xFE; break;
        default: return QByteArray();
    }

    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::OperationMode::request(const QString &, const QVariant &data)
{
    QList <QString> list = {"command", "event"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::CoverPosition::request(const QString &, const QVariant &data)
{
    float value = data.toFloat();

    if (value > 100)
        value = 100;

    if (endpointOption("invertCover").toBool())
        value = 100 - value;

    value = qToLittleEndian(value);
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, 0x0055, DATA_TYPE_SINGLE_PRECISION, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

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

QByteArray ActionsTUYA::ElectricityMeter::request(const QString &, const QVariant &data)
{
    quint8 value = data.toString() == "on" ? 0x01 : 0x00;
    return makeRequest(m_transactionId++, 0x10, TUYA_TYPE_BOOL, &value);
}

QByteArray ActionsTUYA::MoesThermostat::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // status
        {
            quint8 value = data.toString() == "on" ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x01, TUYA_TYPE_BOOL, &value);
        }

        case 1: // mode
        {
            quint8 value = data.toString() == "program" ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x02, TUYA_TYPE_ENUM, &value);
        }

        case 2: // heatingPoint
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

        case 6: // temperatureCalibration
        {
            qint32 value = qToBigEndian <qint32> (data.toInt());
            return makeRequest(m_transactionId++, 0x1B, TUYA_TYPE_VALUE, &value);
        }

        case 7: // childLock
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x28, TUYA_TYPE_BOOL, &value);
        }

        case 8: // sensor
        {
            QList <QString> list = {"internal", "both", "external"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            if (value < 0)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x2B, TUYA_TYPE_ENUM, &value);
        }

        default: // program
        {
            const Property &property = endpointProperty("data");
            QList <QString> types = {"weekday", "saturday", "sunday"}, names = {"Hour", "Minute", "Temperature"};
            QByteArray payload;

            if (property.isNull())
                return QByteArray();

            if (m_data.isEmpty() || property->meta().value("programReceived").toBool())
            {
                m_data = property->value().toMap();
                property->meta().insert("programReceived", false);
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
    }
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
                return QByteArray();

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
            quint8 value = data.toString() == "on" ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x01, TUYA_TYPE_BOOL, &value);
        }

        case 1: // timeout
        {
            quint32 value = qToBigEndian <quint32> (data.toInt() * 60);

            if (value > 36000)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x09, TUYA_TYPE_VALUE, &value);
        }

        case 2: // threshold
        {
            quint32 value = qToBigEndian <quint32> (data.toInt());

            if (value > 100)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x65, TUYA_TYPE_VALUE, &value);
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::PresenceSensor::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
    {
        case 0: // sensitivity
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value > 9)
                break;

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x02, TUYA_TYPE_VALUE, &value);
        }

        case 1: // distanceMin
        {
            quint32 value = static_cast <quint32> (data.toDouble() * 100);

            if (value > 950)
                return QByteArray();

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x03, TUYA_TYPE_VALUE, &value);
        }

        case 2: // distanceMax
        {
            quint32 value = static_cast <quint32> (data.toDouble() * 100);

            if (value > 950)
                return QByteArray();

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x04, TUYA_TYPE_VALUE, &value);
        }

        case 3: // detectionDelay
        {
            quint32 value = static_cast <quint32> (data.toDouble() * 10);

            if (value > 100)
                return QByteArray();

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x65, TUYA_TYPE_VALUE, &value);
        }

        case 4: // fadingTime
        {
            quint32 value = static_cast <quint32> (data.toInt() * 10);

            if (value > 15000)
                return QByteArray();

            value = qToBigEndian(value );
            return makeRequest(m_transactionId++, 0x66, TUYA_TYPE_VALUE, &value);
        }
    }

    return QByteArray();
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

QByteArray ActionsTUYA::BacklightMode::request(const QString &, const QVariant &data)
{
    QList <QString> list = {"low", "medium", "high"};
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

QByteArray ActionsTUYA::SwitchMode::request(const QString &, const QVariant &data)
{
    QList <QString> list = {"toggle", "state", "momentary"};
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

QByteArray ActionsOther::EfektaReportingDelay::request(const QString &, const QVariant &data)
{
    quint16 value = static_cast <quint16> (data.toInt());
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_16BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsOther::PerenioSmartPlug::request(const QString &name, const QVariant &data)
{
    int index = m_actions.indexOf(name);

    switch (index)
    {
        case 0: // powerOnStatus
        {
            QList <QString> list = {"off", "on", "previous"};
            qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

            m_attributes = {0x0000};

            if (value < 0)
                return QByteArray();

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }

        case 1: // resetAlarms
        {
            m_attributes = {0x0001};

            if (!data.toBool())
                return QByteArray();

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_8BIT_UNSIGNED, QByteArray(1, 0x00)); // TODO: check payload
        }

        default:
        {
            quint16 value = qToLittleEndian(static_cast <quint16> (data.toInt()));

            switch (index)
            {
                case 2:  m_attributes = {0x0004}; break; // voltageMin
                case 3:  m_attributes = {0x0005}; break; // voltageMax
                case 4:  m_attributes = {0x000B}; break; // powerMax
                case 5:  m_attributes = {0x000F}; break; // energyLimit
                default: return QByteArray();
            }

            return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_16BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
        }
    }
}
