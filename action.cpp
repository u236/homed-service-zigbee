#include <QtEndian>
#include "action.h"
#include "color.h"

void ActionObject::registerMetaTypes(void)
{
    qRegisterMetaType <Actions::Status>                     ("statusAction");
    qRegisterMetaType <Actions::PowerOnStatus>              ("powerOnStatusAction");
    qRegisterMetaType <Actions::Level>                      ("levelAction");
    qRegisterMetaType <Actions::ColorHS>                    ("colorHSAction");
    qRegisterMetaType <Actions::ColorXY>                    ("colorXYAction");
    qRegisterMetaType <Actions::ColorTemperature>           ("colorTemperatureAction");

    qRegisterMetaType <ActionsPTVO::ChangePattern>          ("ptvoChangePatternAction");
    qRegisterMetaType <ActionsPTVO::Pattern>                ("ptvoPatternAction");

    qRegisterMetaType <ActionsLUMI::OperationMode>          ("lumiOperationModeAction");
    qRegisterMetaType <ActionsLUMI::Sensitivity>            ("lumiSensitivityAction");
    qRegisterMetaType <ActionsLUMI::DetectionMode>          ("lumiDetectionModeAction");
    qRegisterMetaType <ActionsLUMI::Distance>               ("lumiDistanceAction");
    qRegisterMetaType <ActionsLUMI::ResetPresence>          ("lumiResetPresenceAction");

    qRegisterMetaType <ActionsTUYA::Volume>                 ("tuyaVolumeAction");
    qRegisterMetaType <ActionsTUYA::Duration>               ("tuyaDurationAction");
    qRegisterMetaType <ActionsTUYA::Alarm>                  ("tuyaAlarmAction");
    qRegisterMetaType <ActionsTUYA::Melody>                 ("tuyaMelodyAction");
    qRegisterMetaType <ActionsTUYA::Sensitivity>            ("tuyaSensitivityAction");
    qRegisterMetaType <ActionsTUYA::DistanceMin>            ("tuyaDistanceMinAction");
    qRegisterMetaType <ActionsTUYA::DistanceMax>            ("tuyaDistanceMaxAction");
    qRegisterMetaType <ActionsTUYA::DetectionDelay>         ("tuyaDetectionDelayAction");
    qRegisterMetaType <ActionsTUYA::ChildLock>              ("tuyaChildLockAction");
    qRegisterMetaType <ActionsTUYA::BacklightMode>          ("tuyaBacklightModeAction");
    qRegisterMetaType <ActionsTUYA::IndicatorMode>          ("tuyaIndicatorModeAction");
    qRegisterMetaType <ActionsTUYA::SwitchMode>             ("tuyaSwitchModeAction");
    qRegisterMetaType <ActionsTUYA::PowerOnStatus>          ("tuyaPowerOnStatusAction");

    qRegisterMetaType <ActionsPerenio::PowerOnStatus>       ("perenioPowerOnStatusAction");
    qRegisterMetaType <ActionsPerenio::ResetAlarms>         ("perenioResetAlarmsAction");
    qRegisterMetaType <ActionsPerenio::VoltageMin>          ("perenioVoltageMinAction");
    qRegisterMetaType <ActionsPerenio::VoltageMax>          ("perenioVoltageMaxAction");
    qRegisterMetaType <ActionsPerenio::PowerMax>            ("perenioPowerMaxAction");
    qRegisterMetaType <ActionsPerenio::EnergyLimit>         ("perenioEnergyLimitAction");
}

QByteArray ActionObject::writeAttributeRequest(const QByteArray &data)
{
    writeArrtibutesStruct payload;

    payload.attributeId = qToLittleEndian <quint16> (m_attributes.value(0));
    payload.dataType = m_dataType;

    return zclHeader(0x00, m_transactionId++, CMD_WRITE_ATTRIBUTES, m_manufacturerCode).append(reinterpret_cast <char*> (&payload), sizeof(payload)).append(data);
}

QByteArray Actions::Status::request(const QVariant &data)
{
    QString status = data.toString();
    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, status == "toggle" ? 0x02 : status == "on" ? 0x01 : 0x00);
}

QByteArray Actions::PowerOnStatus::request(const QVariant &data)
{
    QList <QString> list = {"off", "on", "toggle", "previous"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0 || value > 2)
        value = 0xFF;

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray Actions::Level::request(const QVariant &data)
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

QByteArray Actions::ColorHS::request(const QVariant &data)
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

QByteArray Actions::ColorXY::request(const QVariant &data)
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

QByteArray Actions::ColorTemperature::request(const QVariant &data)
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

QByteArray ActionsPTVO::ChangePattern::request(const QVariant &data)
{
    QString status = data.toString();
    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, status == "toggle" ? 0x02 : status == "on" ? 0x01 : 0x00);
}

QByteArray ActionsPTVO::Pattern::request(const QVariant &data)
{
    float value = data.toFloat();
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::OperationMode::request(const QVariant &data)
{
    QList <QString> list = {"command", "event"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::Sensitivity::request(const QVariant &data)
{
    QList <QString> list = {"low", "medium", "high"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    value += 1;
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::DetectionMode::request(const QVariant &data)
{
    QList <QString> list = {"undirected", "directed"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::Distance::request(const QVariant &data)
{
    QList <QString> list = {"far", "middle", "near"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsLUMI::ResetPresence::request(const QVariant &data)
{
    quint8 value = 1; // TODO: check this

    if (!data.toBool())
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::Request::makeRequest(quint8 transactionId, quint8 dataPoint, quint8 dataType, void *data)
{
    tuyaHeaderStruct header;

    header.status = 0x00;
    header.transactionId = transactionId;
    header.dataPoint = dataPoint;
    header.dataType = dataType;
    header.function = 0x00;

    switch (header.dataType)
    {
        case 0x01:
        case 0x04:
            header.length = 1;
            break;

        case 0x02:
            header.length = 4;
            break;

        default:
            return QByteArray();
    }

    return zclHeader(FC_CLUSTER_SPECIFIC, transactionId, 0x00).append(reinterpret_cast <char*> (&header), sizeof(header)).append(reinterpret_cast <char*> (data), header.length);
}

QByteArray ActionsTUYA::Volume::request(const QVariant &data)
{
    QList <QString> list = {"low", "medium", "high"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return makeRequest(m_transactionId++, 0x05, 0x04, &value);
}

QByteArray ActionsTUYA::Duration::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toInt());

    if (value > 1800)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x07, 0x02, &value);
}

QByteArray ActionsTUYA::Alarm::request(const QVariant &data)
{
    quint8 value = data.toBool() ? 0x01 : 0x00;
    return makeRequest(m_transactionId++, 0x0D, 0x01, &value);
}

QByteArray ActionsTUYA::Melody::request(const QVariant &data)
{
    quint8 value = static_cast <quint8> (data.toInt());

    if (value < 1 || value > 18)
        return QByteArray();

    return makeRequest(m_transactionId++, 0x15, 0x04, &value);
}

QByteArray ActionsTUYA::Sensitivity::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toInt());

    if (value > 9)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x02, 0x02, &value);
}

QByteArray ActionsTUYA::DistanceMin::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toDouble() * 100);

    if (value > 950)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x03, 0x02, &value);
}

QByteArray ActionsTUYA::DistanceMax::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toDouble() * 100);

    if (value > 950)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x04, 0x02, &value);
}

QByteArray ActionsTUYA::DetectionDelay::request(const QVariant &data)
{
    quint32 value = static_cast <quint32> (data.toInt());

    if (value < 1 || value > 10)
        return QByteArray();

    value = qToBigEndian(value);
    return makeRequest(m_transactionId++, 0x65, 0x02, &value);
}

QByteArray ActionsTUYA::ChildLock::request(const QVariant &data)
{
    qint8 value = data.toBool() ? 0x01 : 0x00;
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::BacklightMode::request(const QVariant &data)
{
    QList <QString> list = {"low", "medium", "high"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::IndicatorMode::request(const QVariant &data)
{
    QList <QString> list = {"off", "default", "inverted", "on"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::SwitchMode::request(const QVariant &data)
{
    QList <QString> list = {"toggle", "state", "momentary"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsTUYA::PowerOnStatus::request(const QVariant &data)
{
    QList <QString> list = {"off", "on", "previous"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPerenio::PowerOnStatus::request(const QVariant &data)
{
    QList <QString> list = {"off", "on", "previous"};
    qint8 value = static_cast <qint8> (list.indexOf(data.toString()));

    if (value < 0)
        return QByteArray();

    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPerenio::ResetAlarms::request(const QVariant &data)
{
    if (!data.toBool())
        return QByteArray();

    return writeAttributeRequest(QByteArray(1, 0x00));
}

QByteArray ActionsPerenio::VoltageMin::request(const QVariant &data)
{
    quint16 value = static_cast <quint16> (data.toInt());
    value = qToLittleEndian(value);
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPerenio::VoltageMax::request(const QVariant &data)
{
    quint16 value = static_cast <quint16> (data.toInt());
    value = qToLittleEndian(value);
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPerenio::PowerMax::request(const QVariant &data)
{
    quint16 value = static_cast <quint16> (data.toInt());
    value = qToLittleEndian(value);
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPerenio::EnergyLimit::request(const QVariant &data)
{
    quint16 value = static_cast <quint16> (data.toInt());
    value = qToLittleEndian(value);
    return writeAttributeRequest(QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}
