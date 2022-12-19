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

    qRegisterMetaType <ActionsTUYA::NeoSiren>               ("tuyaNeoSirenAction");
    qRegisterMetaType <ActionsTUYA::PresenceSensor>         ("tuyaPresenceSensorAction");
    qRegisterMetaType <ActionsTUYA::ChildLock>              ("tuyaChildLockAction");
    qRegisterMetaType <ActionsTUYA::BacklightMode>          ("tuyaBacklightModeAction");
    qRegisterMetaType <ActionsTUYA::IndicatorMode>          ("tuyaIndicatorModeAction");
    qRegisterMetaType <ActionsTUYA::SwitchMode>             ("tuyaSwitchModeAction");
    qRegisterMetaType <ActionsTUYA::PowerOnStatus>          ("tuyaPowerOnStatusAction");

    qRegisterMetaType <ActionsOther::PerenioSmartPlug>      ("perenioSmartPlugAction");
}

QVariant ActionObject::deviceOption(const QString &key)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->options().value(key) : QVariant();
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

    if (deviceOption("invertCover").toBool())
        value = 100 - value;

    value = qToLittleEndian(value);
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, 0x0055, DATA_TYPE_SINGLE_PRECISION, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
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

            return makeRequest(m_transactionId++, 0x05, 0x04, &value);
        }

        case 1: // duration
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value > 1800)
                return QByteArray();

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x07, 0x02, &value);
        }

        case 2: // alarm
        {
            quint8 value = data.toBool() ? 0x01 : 0x00;
            return makeRequest(m_transactionId++, 0x0D, 0x01, &value);
        }

        case 3: // melody
        {
            quint8 value = static_cast <quint8> (data.toInt());

            if (value < 1 || value > 18)
                return QByteArray();

            return makeRequest(m_transactionId++, 0x15, 0x04, &value);
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
            return makeRequest(m_transactionId++, 0x02, 0x02, &value);
        }

        case 1: // distanceMin
        {
            quint32 value = static_cast <quint32> (data.toDouble() * 100);

            if (value > 950)
                return QByteArray();

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x03, 0x02, &value);
        }

        case 2: // distanceMax
        {
            quint32 value = static_cast <quint32> (data.toDouble() * 100);

            if (value > 950)
                return QByteArray();

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x04, 0x02, &value);
        }

        case 3: // detectionDelay
        {
            quint32 value = static_cast <quint32> (data.toInt());

            if (value < 1 || value > 10)
                return QByteArray();

            value = qToBigEndian(value);
            return makeRequest(m_transactionId++, 0x65, 0x02, &value);
        }
    }

    return QByteArray();
}

QByteArray ActionsTUYA::ChildLock::request(const QString &, const QVariant &data)
{
    qint8 value = data.toBool() ? 0x01 : 0x00;
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_BOOLEAN, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
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

QByteArray ActionsOther::PerenioSmartPlug::request(const QString &name, const QVariant &data)
{
    switch (m_actions.indexOf(name))
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

            switch (m_actions.indexOf(name))
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
