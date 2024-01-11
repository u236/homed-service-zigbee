#include <QtEndian>
#include <QtMath>
#include "lumi.h"

void PropertiesLUMI::Data::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (attributeId == 0x00F7 || attributeId == 0xFF01)
    {
        for (quint8 i = 0; i < static_cast <quint8> (data.length()); i++)
        {
            quint8 offset = i + 2, size = zclDataSize(static_cast <quint8> (data.at(i + 1)), data, &offset);

            if (!size)
                break;

            parseData(static_cast <quint8> (data.at(i)), data.mid(offset, size), map);
            i += size + 1;
        }
    }
    else
        parseData(attributeId, data, map);

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesLUMI::Data::resetValue(void)
{
    QMap <QString, QVariant> map = m_value.toMap();

    if (modelName() == "lumi.motion.ac02")
        map.insert("occupancy", false);

    m_value = map;
}

void PropertiesLUMI::Data::parseData(quint16 dataPoint, const QByteArray &data, QMap <QString, QVariant> &map)
{
    if (m_multiple && dataPoint != 0x0200)
        return;

    switch (dataPoint)
    {
        case 0x0001:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            map.insert("battery", percentage(2850, 3200, qFromLittleEndian(value)));
            break;
        }

        case 0x0003:
        {
            QList <QString> list = {"lumi.airrtc.agl001", "lumi.magnet.acn001", "lumi.remote.b286opcn01", "lumi.remote.b486opcn01", "lumi.remote.b686opcn01", "lumi.remote.cagl02", "lumi.sen_ill.mgl01", "lumi.sensor_smoke.acn03"};

            if (list.contains(modelName()))
                break;

            map.insert("temperature", static_cast <qint8> (data.at(0)));
            break;
        }

        case 0x0005:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            map.insert("outageCount", qFromLittleEndian(value) - 1);
            break;
        }

        case 0x0009:
        {
            QList <QString> list = {"lumi.remote.b286opcn01", "lumi.remote.b486opcn01", "lumi.remote.b686opcn01"};

            if (!list.contains(modelName()))
                break;

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("operationMode", "command"); break;
                case 0x01: map.insert("operationMode", "event"); break;
            }

            break;
        }

        case 0x0064:
        {
            quint32 value = 0;

            if (modelName() != "lumi.sen_ill.mgl01" || static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            map.insert("illuminance", qFromLittleEndian(value));
            break;
        }

        case 0x0065:
        case 0x0142:
        {
            if (modelName() == "lumi.motion.ac01")
                map.insert("occupancy", data.at(0) ? true : false);

            break;
        }

        case 0x0066:
        case 0x010C:
        case 0x0143:
        {
            if (modelName() != "lumi.motion.ac01")
                break;

            if (dataPoint != 0x0066 ? dataPoint == 0x010C : version() >= 50)
            {
                switch (static_cast <quint8> (data.at(0)))
                {
                    case 0x01: map.insert("sensitivityMode", "low"); break;
                    case 0x02: map.insert("sensitivityMode", "medium"); break;
                    case 0x03: map.insert("sensitivityMode", "high"); break;
                }
            }
            else
            {
                switch (static_cast <quint8> (data.at(0)))
                {
                    case 0x00: map.insert("event", "enter"); break;
                    case 0x01: map.insert("event", "leave"); break;
                    case 0x02: map.insert("event", "enterLeft"); break;
                    case 0x03: map.insert("event", "leaveRight"); break;
                    case 0x04: map.insert("event", "enterRight"); break;
                    case 0x05: map.insert("event", "leaveLeft"); break;
                    case 0x06: map.insert("event", "approach"); break;
                    case 0x07: map.insert("event", "absent"); break;
                }

                map.insert("occupancy", data.at(0) != 0x01 ? true : false);
            }

            break;
        }

        case 0x0067:
        case 0x0144:
        {
            if (modelName() != "lumi.motion.ac01")
                break;

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("detectionMode", "undirected"); break;
                case 0x01: map.insert("detectionMode", "directed"); break;
            }

            break;
        }

        case 0x0069:
        case 0x0146:
        {
            if (modelName() != "lumi.motion.ac01")
                break;

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("distanceMode", "far"); break;
                case 0x01: map.insert("distanceMode", "middle"); break;
                case 0x02: map.insert("distanceMode", "near"); break;
            }

            break;
        }

        case 0x0095:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            map.insert("energy", round(qFromLittleEndian(value) * 100) / 100);
            break;
        }

        case 0x0096:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(),  data.length());
            map.insert("voltage", round(qFromLittleEndian(value)) / 10);
            break;
        }

        case 0x0097:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(),  data.length());
            map.insert("current", modelName() == "lumi.relay.c2acn01" ? qFromLittleEndian(value) : round(qFromLittleEndian(value)) / 1000);
            break;
        }

        case 0x0098:
        {
            float value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            map.insert("power", round(qFromLittleEndian(value) * 100) / 100);
            break;
        }

        case 0x00F0:
        {
            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("indicatorMode", "default"); break;
                case 0x01: map.insert("indicatorMode", "inverted"); break;
            }

            break;
        }

        case 0x0112:
        {
            quint32 value = 0;

            if (modelName() != "lumi.motion.ac02" || static_cast <size_t> (data.length()) > sizeof(value))
                break;

            memcpy(&value, data.constData(), data.length());
            value = qFromLittleEndian(value);
            map.insert("illuminance", (value > 130536 ? 0 : value & 0xFFFF));
            map.insert("occupancy", true);
            break;
        }

        case 0x0200:
        {
            QList <QString> list = {"lumi.switch.b2nc01", "lumi.switch.b2lc04"};

            if (!m_multiple && list.contains(modelName()))
                break;

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("switchMode", "decoupled"); break;
                case 0x01: map.insert("switchMode", "relay"); break;
            }

            break;
        }

        case 0x0201:
        case 0xFF19:
        {
            map.insert("statusMemory", data.at(0) ? true : false);
            break;
        }

        case 0x0271:
        {
            if (modelName() != "lumi.airrtc.agl001")
                break;

            map.insert("systemMode", data.at(0) ? "heat" : "off");
            break;
        }

        case 0x0272:
        {
            if (modelName() != "lumi.airrtc.agl001")
                break;

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x00: map.insert("operationMode", "manual"); break;
                case 0x01: map.insert("operationMode", "away"); break;
                case 0x02: map.insert("operationMode", "program"); break;
            }
            
            break;
        }

        case 0x0273:
        {
            if (modelName() != "lumi.airrtc.agl001")
                break;
            
            map.insert("windowDetection", data.at(0) ? true : false);
            break;
        }

        case 0x0277:
        {
            if (modelName() != "lumi.airrtc.agl001")
                break;
            
            map.insert("childLock", data.at(0) ? true : false);
            break;
        }

        case 0x0279:
        {
            quint32 value = 0;

            if (modelName() != "lumi.airrtc.agl001")
                break;

            memcpy(&value, data.constData(), sizeof(value));
            map.insert("awayTemperature", qFromLittleEndian(value) / 100.0);
            break;
        }

        case 0x027A:
        {
            if (modelName() != "lumi.airrtc.agl001")
                break;

            map.insert("windowOpen", data.at(0) ? true : false);
            break;
        }

        case 0x027E:
        {
            if (modelName() != "lumi.airrtc.agl001")
                break;

            map.insert("sensorType", data.at(0) ? "external" : "internal");
            break;
        }

        case 0xFF02:
        {
            quint16 value = 0;

            if (data.length() < 7)
                break;

            memcpy(&value, data.constData() + 5, sizeof(value));
            map.insert("battery", percentage(2850, 3200, qFromLittleEndian(value)));
            break;
        }

        case 0xFF0D:
        {
            if (modelName() != "lumi.vibration.aq1")
                break;

            switch (static_cast <quint8> (data.at(0)))
            {
                case 0x01: map.insert("sensitivityMode", "high"); break;
                case 0x0B: map.insert("sensitivityMode", "medium"); break;
                case 0x15: map.insert("sensitivityMode", "low"); break;
            }
        }
    }
}

void PropertiesLUMI::ButtonMode::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();
    bool check = modelName() == "lumi.ctrl_neutral1";
    QString value;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x12: value = check ? "relay" : "leftRelay"; break;
        case 0x22: value = "rightRelay"; break;
        case 0xFE: value = "decoupled"; break;
    }

    switch (attributeId)
    {
        case 0xFF22: map.insert(check ? "buttonMode" : "leftMode", value); break;
        case 0xFF23: map.insert("rightMode", value); break;
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesLUMI::SwitchType::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x000A)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x01: m_value = "toggle"; break;
        case 0x02: m_value = "momentary"; break;
    }
}

void PropertiesLUMI::Contact::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000)
        return;

    m_value = data.at(0) ? true : false;
}

void PropertiesLUMI::Interlock::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0xFF06)
        return;

    m_value = data.at(0) ? true : false;
}

void PropertiesLUMI::Power::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = round(qFromLittleEndian(value) * 100) / 100;
}

void PropertiesLUMI::Cover::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map;
    float value = 0;

    if (attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    value = qFromLittleEndian(value);

    if (!option("invertCover").toBool())
        value = 100 - value;

    map.insert("cover", value ? "open" : "closed");
    map.insert("position", value);
    m_value = map;
}

void PropertiesLUMI::Illuminance::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0000 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value);
}


void PropertiesLUMI::ButtonAction::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0000 && attributeId != 0x8000)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00: m_value = "on"; break;
        case 0x01: m_value = "off"; break;
        case 0x02: m_value = "doubleClick"; break;
        case 0x03: m_value = "tripleClick"; break;
        case 0x04: m_value = "quadrupleClick"; break;
        case 0x80: m_value = "multipleClick"; break;
    }
}

void PropertiesLUMI::SwitchAction::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    if (attributeId != 0x0055)
        return;

    switch (static_cast <quint8> (data.at(0)))
    {
        case 0x00:
        case 0x10:
            m_value = "hold";
            break;

        case 0x01: m_value = "singleClick"; break;
        case 0x02: m_value = "doubleClick"; break;
        case 0x03: m_value = "tripleClick"; break;

        case 0x11:
        case 0xFF:
            m_value = "release";
            break;

        case 0x12: m_value = "shake"; break;
    }
}

void PropertiesLUMI::CubeRotation::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    float value = 0;

    if (attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    m_value = qFromLittleEndian(value) < 0 ? "rotateLeft" : "rotateRight";
}

void PropertiesLUMI::CubeMovement::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    quint16 value = 0;

    if (attributeId != 0x0055 || static_cast <size_t> (data.length()) > sizeof(value))
        return;

    memcpy(&value, data.constData(), data.length());
    value = qFromLittleEndian(value);

    if (!value)
        m_value = "shake";
    else if (value == 2)
        m_value = "wake";
    else if (value == 3)
        m_value = "fall";
    else if (value >= 512)
        m_value = "tap";
    else if (value >= 256)
        m_value = "slide";
    else if (value >= 128)
        m_value = "flip";
    else if (value >= 64)
        m_value = "drop";
}

void PropertiesLUMI::Vibration::parseAttribte(quint16, quint16 attributeId, const QByteArray &data)
{
    QMap <QString, QVariant> map = m_value.toMap();

    switch (attributeId)
    {
        case 0x0055:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            value = qFromLittleEndian(value);

            switch (value)
            {
                case 0x0001: map.insert("event", "vibration"); break;
                case 0x0002: map.insert("event", "tilt"); break;
                case 0x0003: map.insert("event", "drop"); break;
            }

            if (value == 0x0001)
                map.insert("vibration", true);

            break;
        }

        case 0x0503:
        {
            quint16 value = 0;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            map.insert("angle", qFromLittleEndian(value));
            break;
        }

        case 0x0505:
        {
            quint16 value = 0;
            memcpy(&value, data.constData(), sizeof(value));
            map.insert("strength", qFromBigEndian(value));
            break;
        }

        case 0x0508:
        {
            quint64 value;
            qint16 x, y, z;

            if (static_cast <size_t> (data.length()) > sizeof(value))
                return;

            memcpy(&value, data.constData(), data.length());
            value = qFromLittleEndian(value);

            x = static_cast <qint16> (value & 0xFFFF);
            y = static_cast <qint16> (value >> 16 & 0xFFFF);
            z = static_cast <qint16> (value >> 32 & 0xFFFF);

            map.insert("x", round(atan(x / sqrt(pow(y, 2) + pow(z, 2))) * 180 / M_PI));
            map.insert("y", round(atan(y / sqrt(pow(x, 2) + pow(z, 2))) * 180 / M_PI));
            map.insert("z", round(atan(z / sqrt(pow(x, 2) + pow(y, 2))) * 180 / M_PI));
            break;
        }
    }

    m_value = map.isEmpty() ? QVariant() : map;
}

void PropertiesLUMI::Vibration::resetValue(void)
{
    QMap <QString, QVariant> map = m_value.toMap();
    map.insert("vibration", false);
    m_value = map;
}
