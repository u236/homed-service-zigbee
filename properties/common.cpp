#include <QtEndian>
#include <QtMath>
#include "color.h"
#include "common.h"

using namespace Properties;

void Properties::BatteryVoltage::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0020)
        return;

    m_value = percentage(2850, 3200, static_cast <quint8> (data.at(0)) * 100);
}

void Properties::BatteryPercentage::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0021)
        return;

    m_value = static_cast <quint8> (data.at(0)) / (option().toString() == "undivided" ? 1.0 : 2.0);
}

void Properties::DeviceTemperature::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) + option("temperatureOffset").toDouble();
}

void Properties::Status::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = data.at(0) ? "on" : "off";
}

void Properties::PowerOnStatus::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x4003)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
        case 0x02: m_value = "toggle"; break;
        case 0xFF: m_value = "previous"; break;
    }
}

void Properties::SwitchType::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "toggle"; break;
        case 0x01: m_value = "momentary"; break;
        case 0x02: m_value = "multifunction"; break;
    }
}

void Properties::SwitchMode::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0010)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "on"; break;
        case 0x01: m_value = "off"; break;
        case 0x02: m_value = "toggle"; break;
    }
}

void Properties::Level::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = static_cast <quint8> (data.at(0));
}

void Properties::CoverStatus::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0008)
        return;

    m_value = static_cast <quint8> (option("invertCover").toBool() ? 100 - data.at(0) : data.at(0)) ? "open" : "closed";
}

void Properties::CoverPosition::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint8 value = static_cast <quint8> (option("invertCover").toBool() ? 100 - data.at(0) : data.at(0));

    if (attributeId != 0x0008 || value == meta().value("position", 0xFF).toInt())
        return;

    m_value = value;
}

void Properties::CoverTilt::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint8 value = static_cast <quint8> (option("invertCover").toBool() ? 100 - data.at(0) : data.at(0));

    if (attributeId != 0x0009 || value == meta().value("tilt", 0xFF).toInt())
        return;

    m_value = static_cast <quint8> (option("invertCover").toBool() ? 100 - data.at(0) : data.at(0));
}

void Properties::ColorHS::parseAttribte(quint16 attributeId, const QByteArray &data)
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

void Properties::ColorXY::parseAttribte(quint16 attributeId, const QByteArray &data)
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

void Properties::ColorTemperature::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0007 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}

void Properties::Illuminance::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = (option().toString() == "raw" ? qFromLittleEndian(value) : static_cast <quint32> (value ? pow(10, (qFromLittleEndian(value) - 1) / 10000.0) : 0)) + option("illuminanceOffset").toDouble();
}

void Properties::Temperature::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0 + option("temperatureOffset").toDouble();
}

void Properties::Pressure::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    qint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 10.0 + option("pressureOffset").toDouble();
}

void Properties::Humidity::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / option("humidityDivider", 100).toDouble() + option("humidityOffset").toDouble();
}

void Properties::CO2::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = round(qFromLittleEndian(value) * 1000000);
}

void Properties::Occupancy::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = data.at(0) ? true : false;
}

void Properties::Moisture::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / 100.0 + option("moistureOffset").toDouble();
}

void Properties::Occupancy::resetValue(void)
{
    m_value = false;
}

void Properties::Energy::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    double divider = option("energyDivider", 1).toDouble();
    qint64 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / divider;
}

void Properties::Voltage::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    double divider = option("voltageDivider", 1).toDouble();
    qint16 value = 0;

    if (attributeId != 0x0505 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / divider + option("voltageOffset").toDouble();
}

void Properties::Current::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    double divider = option("currentDivider", 1).toDouble();
    qint16 value = 0;

    if (attributeId != 0x0508 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / divider + option("currentOffset").toDouble();
}

void Properties::Power::parseAttribte(quint16 attributeId, const QByteArray &data)
{
    double divider = option("powerDivider", 1).toDouble();
    qint16 value = 0;

    if (attributeId != 0x050B || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) / divider + option("powerOffset").toDouble();
}

void Properties::Thermostat::parseAttribte(quint16 attributeId, const QByteArray &data)
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
            map.insert(attributeId ? "temperatureOffset" : "temperature", qFromLittleEndian(value) / 100.0);
            break;
        }

        case 0x0010:
        {
            map.insert("temperatureOffset", static_cast <qint8> (data.at(0)) / 10.0);
            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void Properties::Scene::parseCommand(quint8 commandId, const QByteArray &payload)
{
    const recallSceneStruct *command = reinterpret_cast <const recallSceneStruct*> (payload.constData());
    QVariant scene = option().toMap().value(QString::number(command->sceneId));

    if (commandId != 0x05)
        return;

    m_value = scene.isValid() ? scene : command->sceneId;
}

void Properties::StatusAction::parseCommand(quint8 commandId, const QByteArray &)
{
    switch (commandId)
    {
        case 0x00: m_value = "off"; break;
        case 0x01: m_value = "on"; break;
        case 0x02: m_value = "toggle"; break;
    }
}

void Properties::LevelAction::parseCommand(quint8 commandId, const QByteArray &payload)
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

void Properties::ColorAction::parseCommand(quint8 commandId, const QByteArray &payload)
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
