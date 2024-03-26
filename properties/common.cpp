#include <QtEndian>
#include <QtMath>
#include "color.h"
#include "common.h"

using namespace Properties;

void Properties::BatteryVoltage::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0020)
        return;

    m_value = percentage(2850, 3000, static_cast <quint8> (data.at(0)) * 100);
}

void Properties::BatteryPercentage::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0021)
        return;

    m_value = static_cast <quint8> (data.at(0)) / (option().toMap().value("undivided").toBool() ? 1.0 : 2.0);
}

void Properties::DeviceTemperature::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::Status::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void Properties::Level::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = static_cast <quint8> (data.at(0));
}

void Properties::AnalogInput::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::AnalogOutput::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::CoverPosition::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map;
    qint8 value = static_cast <quint8> (option("invertCover").toBool() ? data.at(0) : 100 - data.at(0));

    if (attributeId != 0x0008 || value == meta().value("position", 0xFF).toInt())
        return;

    map.insert("cover", value ? "open" : "closed");
    map.insert("position", value);
    m_value = map;
}

void Properties::CoverTilt::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map;
    qint8 value = static_cast <quint8> (option("invertCover").toBool() ? data.at(0) : 100 - data.at(0));

    if (attributeId != 0x0009 || value == meta().value("tilt", 0xFF).toInt())
        return;

    map.insert("cover", value ? "open" : "closed");
    map.insert("tilt", value);
    m_value = map;
}

void Properties::Thermostat::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
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
            map.insert(attributeId ? "targetTemperature" : "temperature", qFromLittleEndian(value) / 100.0);
            break;
        }

        case 0x0010:
        {
            map.insert("temperatureOffset", static_cast <qint8> (data.at(0)) / 10.0);
            break;
        }

        case 0x001C:
        {
            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("systemMode", "off"); break;
                case 0x01: map.insert("systemMode", "auto"); break;
                case 0x04: map.insert("systemMode", "heat"); break;
            }

            break;
        }

        case 0x001E:
        {
            map.insert("heating", data.at(0) == 0x04 ? true : false);
            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void Properties::ColorHS::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    switch (attributeId)
    {
        case 0x0000:
            m_colorH = static_cast <quint8> (data.at(0));
            break;

        case 0x0001:
            m_colorS = static_cast <quint8> (data.at(0));
            break;
    }

    if (m_colorH.isValid() || m_colorS.isValid())
    {
        Color color = Color::fromHS(m_colorH.toDouble() / 0xFF, m_colorS.toDouble() / 0xFF);
        m_value = QList <QVariant> {static_cast <quint8> (color.r() * 0xFF), static_cast <quint8> (color.g() * 0xFF), static_cast <quint8> (color.b() * 0xFF)};
    }
}

void Properties::ColorXY::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (static_cast <size_t> (data.length()) > sizeof(value))
        return;

    switch (attributeId)
    {
        case 0x0003:
            memcpy(&value, data.constData(), data.length());
            m_colorX = qFromLittleEndian(value);
            break;

        case 0x0004:
            memcpy(&value, data.constData(), data.length());
            m_colorY = qFromLittleEndian(value);
            break;
    }

    if (m_colorX.isValid() || m_colorY.isValid())
    {
        Color color = Color::fromXY(m_colorX.toDouble() / 0xFFFF, m_colorY.toDouble() / 0xFFFF);
        m_value = QList <QVariant> {static_cast <quint8> (color.r() * 0xFF), static_cast <quint8> (color.g() * 0xFF), static_cast <quint8> (color.b() * 0xFF)};
    }
}

void Properties::ColorTemperature::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0007 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::Illuminance::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = option().toMap().value("raw").toBool() ? qFromLittleEndian(value) : static_cast <quint32> (value ? pow(10, (qFromLittleEndian(value) - 1) / 10000.0) : 0);
}

void Properties::Temperature::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0;
}

void Properties::Pressure::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 10.0;
}

void Properties::Humidity::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / option("humidityDivider", 100).toDouble();
}

void Properties::Occupancy::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = data.at(0) ? true : false;
}

void Properties::Occupancy::resetValue(void)
{
    m_value = false;
}

void Properties::OccupancyTimeout::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value;

    if (attributeId != 0x0010 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::Moisture::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0;
}

void Properties::CO2::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    value = qFromLittleEndian(value);
    m_value = round(value < 1 ? value * 1000000 : value);
}

void Properties::PM25::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::Energy::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    double divider = option("energyDivider", 1).toDouble();
    qint64 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / divider;
}

void Properties::Voltage::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    double divider = option("voltageDivider", 1).toDouble();
    qint16 value = 0;

    if (attributeId != 0x0505 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / divider;
}

void Properties::Current::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    double divider = option("currentDivider", 1).toDouble();
    qint16 value = 0;

    if (attributeId != 0x0508 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / divider;
}

void Properties::Power::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    double divider = option("powerDivider", 1).toDouble();
    qint16 value = 0;

    if (attributeId != 0x050B || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / divider;
}

void Properties::Scene::parseCommand(quint16, quint8 commandId, const QByteArray &payload)
{
    const recallSceneStruct *command = reinterpret_cast <const recallSceneStruct*> (payload.constData());
    QVariant name = option().toMap().value("enum").toMap().value(QString::number(command->sceneId));

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
