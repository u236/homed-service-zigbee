#include <QtEndian>
#include <QtMath>
#include "color.h"
#include "common.h"
#include "device.h"

using namespace Properties;

void Properties::BatteryVoltage::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributes.at(0) || data.at(0) == 0xFF)
        return;

    m_value = percentage(2850, 3000, static_cast <quint8> (data.at(0)) * 100);
}

void Properties::BatteryPercentage::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributes.at(0) || data.at(0) == 0xFF)
        return;

    m_value = static_cast <quint8> (data.at(0)) / (option(m_name, "undivided").toBool() ? 1.0 : 2.0);
}

void Properties::DeviceTemperature::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::Status::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributes.at(0))
        return;

    m_value = data.at(0) ? "on" : "off";

    if (!data.at(0))
    {
        EndpointObject *endpoint = static_cast <EndpointObject*> (m_parent);
        QList <QVariant> list = option("resetOnPowerOff").toList();

        if (list.isEmpty())
            return;

        for (int i = 0; i < endpoint->properties().count(); i++)
        {
            const Property &property = endpoint->properties().at(i);

            if (!list.contains(property->name()))
                continue;

            property->setValue(0);
        }
    }
}

void Properties::Level::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributes.at(0))
        return;

    m_value = static_cast <quint8> (data.at(0));
}

void Properties::CoverPosition::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map;
    qint8 value = static_cast <quint8> (option("invertCover").toBool() ? data.at(0) : 100 - data.at(0));

    if (attributeId != m_attributes.at(0) || value == meta("position", 0xFF).toInt())
        return;

    map.insert("cover", value ? "open" : "closed");
    map.insert("position", value);
    m_value = map;
}

void Properties::Thermostat::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x0000:
        case 0x0012:
        {
            qint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            map.insert(attributeId == 0x0000 ? "temperature" : "targetTemperature", qFromLittleEndian(value) / 100.0);
            break;
        }

        case 0x0010:
        case 0x0019:
        {
            QString name = attributeId == 0x0010 ? "temperatureOffset" : "hysteresis";
            map.insert(name, static_cast <qint8> (data.at(0)) / option(QString(name).append("Divider"), 10).toDouble());
            break;
        }

        case 0x001C:
        {
            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("systemMode", "off"); break;
                case 0x01: map.insert("systemMode", "auto"); break;
                case 0x03: map.insert("systemMode", "cool"); break;
                case 0x04: map.insert("systemMode", "heat"); break;
                case 0x07: map.insert("systemMode", "fan"); break;
                case 0x08: map.insert("systemMode", "dry"); break;
            }

            break;
        }

        case 0x0029:
        {
            map.insert("running", data.at(0) ? true : false);
            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void Thermostat::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    QMap <QString, QVariant> map = m_value.toMap();
    QList <QString> typeList = {"sunday", option("thermostatProgram").toString() == "moes" ? "weekday" : "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
    QString type;

    if (commandId != 0x00)
        return;

    for (int i = 0; i < typeList.count(); i++)
    {
        if (payload.at(1) & (1 << i))
        {
            type = typeList.at(i);
            break;
        }
    }

    setMeta(QString("%1Program").arg(type), true);

    for (int i = 0; i < payload.at(0); i++)
    {
        QString key = QString("%1P%2").arg(type).arg(i + 1);
        quint16 time = qFromLittleEndian <quint16> (*(reinterpret_cast <const quint16*> (payload.constData() + i * 4 + 3)));
        map.insert(QString("%1Hour").arg(key), static_cast <quint8> (time / 60));
        map.insert(QString("%1Minute").arg(key), static_cast <quint8> (time % 60));
        map.insert(QString("%1Temperature").arg(key), (qFromLittleEndian <quint16> (*(reinterpret_cast <const quint16*> (payload.constData() + i * 4 + 5)))) / 100.0);
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void Properties::ColorHS::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0000:
            m_h = static_cast <quint8> (data.at(0));
            break;

        case 0x0001:
            m_s = static_cast <quint8> (data.at(0));
            break;
    }

    if (m_h.isValid() || m_s.isValid())
    {
        Color color = Color::fromHS(m_h.toDouble() / 0xFF, m_s.toDouble() / 0xFF);
        m_value = QList <QVariant> {static_cast <quint8> (color.r() * 0xFF), static_cast <quint8> (color.g() * 0xFF), static_cast <quint8> (color.b() * 0xFF)};
    }
}

void Properties::ColorXY::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (static_cast <size_t> (data.length()) > sizeof(value))
        return;

    switch (attributeId)
    {
        case 0x0003:
            memcpy(&value, data.constData(), data.length());
            m_x = qFromLittleEndian(value);
            break;

        case 0x0004:
            memcpy(&value, data.constData(), data.length());
            m_y = qFromLittleEndian(value);
            break;
    }

    if (m_x.isValid() || m_y.isValid())
    {
        Color color = Color::fromXY(m_x.toDouble() / 0xFFFF, m_y.toDouble() / 0xFFFF);
        m_value = QList <QVariant> {static_cast <quint8> (color.r() * 0xFF), static_cast <quint8> (color.g() * 0xFF), static_cast <quint8> (color.b() * 0xFF)};
    }
}

void Properties::ColorTemperature::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void ColorMode::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributes.at(0))
        return;

    m_value = data.at(0) != 0x02;
}

void Properties::Illuminance::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = option(m_name, "raw").toBool() ? qFromLittleEndian(value) : static_cast <quint32> (value ? pow(10, (qFromLittleEndian(value) - 1) / 10000.0) : 0);
}

void Properties::Temperature::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0;
}

void Properties::Pressure::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 10.0;
}

void Properties::Flow::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 10.0;
}

void Properties::Humidity::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / option("humidityDivider", 100).toDouble();

    if (m_value.toDouble() <= 100)
        return;

    m_value = m_value.toDouble() / 10;
}

void Properties::Occupancy::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributes.at(0))
        return;

    m_value = data.at(0) ? true : false;
}

void Properties::Occupancy::resetValue(void)
{
    m_value = false;
}

void Properties::OccupancyTimeout::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::Moisture::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0;
}

void Properties::CO2::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    value = qFromLittleEndian(value);
    m_value = round(value < 1 ? value * 1000000 : value);
}

void Properties::PM25::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::Energy::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();
    double divider = option("energyDivider", 1).toDouble();
    quint32 value = 0;

    memcpy(&value, data.constData(), sizeof(value));
    value = qFromLittleEndian(value);

    switch (attributeId)
    {
        case 0x0000: map.insert("energy",   value / divider); break;
        case 0x0100: map.insert("energyT1", value / divider); break;
        case 0x0102: map.insert("energyT2", value / divider); break;
        case 0x0104: map.insert("energyT3", value / divider); break;
        case 0x0106: map.insert("energyT4", value / divider); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void Properties::Voltage::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (!m_attributes.contains(attributeId) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / option(attributeId != 0x0100 ? "acVoltageDivider" : "dcVoltageDivider", 1).toDouble();
}

void Properties::Current::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (!m_attributes.contains(attributeId) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / option(attributeId != 0x0103 ? "acCurrentDivider" : "dcCurrentDivider", 1).toDouble();
}

void Properties::Power::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (!m_attributes.contains(attributeId) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / option(attributeId != 0x0106 ? "acPowerDivider" : "dcPowerDivider", 1).toDouble();
}

void Properties::Frequency::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != m_attributes.at(0) || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / option("frequencyDivider", 1).toDouble();
}

void Properties::PowerFactor::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributes.at(0))
        return;

    m_value = static_cast <qint8> (data.at(0)) / 100.0;
}

void ChildLock::parseAttribute(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != m_attributes.at(0))
        return;

    m_value = data.at(0) ? true : false;
}

void Properties::Scene::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    const recallSceneStruct *command = reinterpret_cast <const recallSceneStruct*> (payload.constData());
    QVariant name = option(m_name, "enum").toMap().value(QString::number(command->sceneId));

    if (commandId != 0x05)
        return;

    m_value = name.isValid() ? name : command->sceneId;
}

void Properties::StatusAction::parseCommand(quint16, quint8 commandId, const QByteArray &)
{
    switch (commandId)
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
        case 0x02: m_value = "toggle"; break;
    }
}

void Properties::LevelAction::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    switch (commandId)
    {
        case 0x00:
        {
            const moveToLevelStruct *data = reinterpret_cast <const moveToLevelStruct*> (payload.constData());
            m_value = QMap <QString, QVariant> {{"action", "moveToLevel"}, {"level", data->level}, {"moveTime", qFromLittleEndian(data->time)}};
            break;
        }

        case 0x01:
        case 0x05:
        {
            const moveLevelStruct *data = reinterpret_cast <const moveLevelStruct*> (payload.constData());
            m_value = QMap <QString, QVariant> {{"action", data->mode ? "moveLevelDown" : "moveLevelUp"}, {"moveRate", data->rate}};
            break;
        }

        case 0x02:
        case 0x06:
        {
            const stepLevelStruct *data = reinterpret_cast <const stepLevelStruct*> (payload.constData());
            m_value = QMap <QString, QVariant> {{"action", data->mode ? "stepLevelDown" : "stepLevelUp"}, {"stepSize", data->size}};
            break;
        }

        case 0x03:
        case 0x07:
        {
            m_value = "stopLevel";
            break;
        }
    }
}

void CoverAction::parseCommand(quint16, quint8 commandId, const QByteArray &)
{
    switch (commandId)
    {
        case 0x00: m_value = "open"; break;
        case 0x01: m_value = "close"; break;
        case 0x02: m_value = "stop"; break;
    }
}

void Properties::ColorAction::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    switch (commandId)
    {
        case 0x01:
        {
            const moveHueStruct *data = reinterpret_cast <const moveHueStruct*> (payload.constData());

            if (data->mode)
                m_value = QMap <QString, QVariant> {{"action", data->mode == 0x01 ? "moveHueUp" : "moveHueDown"}, {"moveRate", data->rate}};
            else
                m_value = "stopHue";

            break;
        }

        case 0x04:
        {
            const moveSaturationStruct *data = reinterpret_cast <const moveSaturationStruct*> (payload.constData());

            if (data->mode)
                m_value = QMap <QString, QVariant> {{"action", data->mode == 0x01 ? "moveSaturationUp" : "moveSaturationDown"}, {"moveRate", data->rate}};
            else
                m_value = "stopSaturation";

            break;
        }

        case 0x06:
        {
            const moveToColorHSStruct *data = reinterpret_cast <const moveToColorHSStruct*> (payload.constData());
            Color color = Color::fromHS(static_cast <double> (data->colorH) / 0xFF, static_cast <double> (data->colorS) / 0xFF);
            m_value = QMap <QString, QVariant> {{"action", "moveToColor"}, {"color", QList <QVariant> {static_cast <quint8> (color.r() * 0xFF), static_cast <quint8> (color.g() * 0xFF), static_cast <quint8> (color.b() * 0xFF)}}, {"moveTime", qFromLittleEndian(data->time)}};
            break;
        }

        case 0x0A:
        {
            const moveToColorTemperatureStruct *data = reinterpret_cast <const moveToColorTemperatureStruct*> (payload.constData());
            m_value = QMap <QString, QVariant> {{"action", "moveToColorTemperature"}, {"colorTemperature", data->colorTemperature}, {"moveTime", qFromLittleEndian(data->time)}};
            break;
        }

        case 0x47:
        {
            m_value = "stopColor";
            break;
        }

        case 0x4B:
        {
            const moveColorTemperatureStruct *data = reinterpret_cast <const moveColorTemperatureStruct*> (payload.constData());

            if (data->mode)
                m_value = QMap <QString, QVariant> {{"action", data->mode == 0x01 ? "moveColorTemperatureUp" : "moveColorTemperatureDown"}, {"moveRate", qFromLittleEndian(data->rate)}};
            else
                m_value = "stopColorTemperature";

            break;
        }

        case 0x4C:
        {
            const stepColorTemperatureStruct *data = reinterpret_cast <const stepColorTemperatureStruct*> (payload.constData());

            if (data->mode)
                m_value = QMap <QString, QVariant> {{"action", data->mode == 0x01 ? "stepColorTemperatureUp" : "stepColorTemperatureDown"}, {"stepSize", qFromLittleEndian(data->size)}};
            else
                m_value = "stopColorTemperature";

            break;
        }
    }
}
