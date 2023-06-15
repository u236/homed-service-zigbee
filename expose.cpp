#include "expose.h"

void ExposeObject::registerMetaTypes(void)
{
    qRegisterMetaType <LightObject>                 ("lightExpose");
    qRegisterMetaType <SwitchObject>                ("switchExpose");
    qRegisterMetaType <LockObject>                  ("lockExpose");
    qRegisterMetaType <CoverObject>                 ("coverExpose");
    qRegisterMetaType <ThermostatObject>            ("thermostatExpose");

    qRegisterMetaType <Binary::Contact>             ("contactExpose");
    qRegisterMetaType <Binary::BatteryLow>          ("batteryLowExpose");
    qRegisterMetaType <Binary::Gas>                 ("gasExpose");
    qRegisterMetaType <Binary::Occupancy>           ("occupancyExpose");
    qRegisterMetaType <Binary::Smoke>               ("smokeExpose");
    qRegisterMetaType <Binary::Tamper>              ("tamperExpose");
    qRegisterMetaType <Binary::WaterLeak>           ("waterLeakExpose");
    qRegisterMetaType <Binary::Vibration>           ("vibrationExpose");

    qRegisterMetaType <Sensor::Battery>             ("batteryExpose");
    qRegisterMetaType <Sensor::Temperature>         ("temperatureExpose");
    qRegisterMetaType <Sensor::Pressure>            ("pressureExpose");
    qRegisterMetaType <Sensor::Humidity>            ("humidityExpose");
    qRegisterMetaType <Sensor::Moisture>            ("moistureExpose");
    qRegisterMetaType <Sensor::CO2>                 ("co2Expose");
    qRegisterMetaType <Sensor::ECO2>                ("eco2Expose");
    qRegisterMetaType <Sensor::VOC>                 ("vocExpose");
    qRegisterMetaType <Sensor::Illuminance>         ("illuminanceExpose");
    qRegisterMetaType <Sensor::Energy>              ("energyExpose");
    qRegisterMetaType <Sensor::Voltage>             ("voltageExpose");
    qRegisterMetaType <Sensor::Current>             ("currentExpose");
    qRegisterMetaType <Sensor::Power>               ("powerExpose");
    qRegisterMetaType <Sensor::Count>               ("countExpose");
    qRegisterMetaType <Sensor::Position>            ("positionExpose");
    qRegisterMetaType <Sensor::Action>              ("actionExpose");
    qRegisterMetaType <Sensor::Event>               ("eventExpose");
    qRegisterMetaType <Sensor::Scene>               ("sceneExpose");

    qRegisterMetaType <Button::ResetCount>          ("resetCountExpose");
}

QJsonObject BinaryObject::request(void)
{
    QList <QString> list = {"alarm", "contact", "batteryLow", "waterLeak"};
    QJsonObject json;

    switch (list.indexOf(m_name))
    {
        case 0:  break;
        case 1:  json.insert("device_class",        option().toString() == "window" ? "window" : "door"); break;
        case 2:  json.insert("device_class",        "battery"); break;
        case 3:  json.insert("device_class",        "moisture"); break;
        default: json.insert("device_class",        m_name); break;
    }

    if (option().toString() == "diagnostic" || m_name == "batteryLow" || m_name == "tamper")
        json.insert("entity_category",              "diagnostic");

    json.insert("value_template",                   QString("{{ value_json.%1 }}").arg(m_name));
    json.insert("payload_on",                       true);
    json.insert("payload_off",                      false);
    json.insert("state_topic",                      m_stateTopic);

    return json;
}

QJsonObject SensorObject::request(void)
{
    QList <QString> list = {"action", "event", "scene", "count", "position", "co2", "eco2", "voc"}, valueTemplate = {QString("value_json.%1").arg(m_name)};
    QJsonObject json;

    switch (list.indexOf(m_name))
    {
        case 0:  json.insert("icon",                "mdi:gesture-double-tap"); break;
        case 1:  json.insert("icon",                "mdi:bell"); break;
        case 2:  json.insert("icon",                "mdi:gesture-tap-button"); break;
        case 3:  json.insert("icon",                "mdi:counter"); break;
        case 4:  json.insert("icon",                "mdi:valve"); break;
        case 5:  json.insert("device_class",        "carbon_dioxide"); break;
        case 6:  json.insert("device_class",        "carbon_dioxide"); break;
        case 7:  json.insert("device_class",        "volatile_organic_compounds"); break;
        default: json.insert("device_class",        m_name); break;
    }

    if (option().toString() == "diagnostic")
        json.insert("entity_category",              "diagnostic");

    if (!m_unit.isEmpty() && option().toString() != "raw")
        json.insert("unit_of_measurement",          m_unit);

    if (m_round)
        valueTemplate.append(                       QString("round(%1)").arg(m_round));

    json.insert("value_template",                   QString("{{ %1 }}").arg(valueTemplate.join(" | ")));
    json.insert("state_topic",                      m_stateTopic);

    return json;
}

QJsonObject NumberObject::request(void)
{
    QMap <QString, QVariant> options = option().toMap();
    QJsonObject json;

    if (!options.value("control").toBool())
        json.insert("entity_category",              "config");

    if (options.contains("icon"))
        json.insert("icon",                         options.value("icon").toString());

    if (options.contains("step"))
        json.insert("step",                         options.value("step").toDouble());

    if (options.contains("unit"))
        json.insert("unit_of_measurement",          options.value("unit").toString());

    json.insert("min",                              options.value("min").toDouble());
    json.insert("max",                              options.value("max").toDouble());

    json.insert("value_template",                   QString("{{ value_json.%1 }}").arg(m_name));
    json.insert("state_topic",                      m_stateTopic);

    json.insert("command_template",                 QString("{\"%1\":{{ value }}}").arg(m_name));
    json.insert("command_topic",                    m_commandTopic);

    return json;
}

QJsonObject SelectObject::request(void)
{
    QMap <QString, QVariant> options = option().toMap();
    QJsonObject json;

    if (!options.value("control").toBool())
        json.insert("entity_category",              "config");

    json.insert("options",                          QJsonArray::fromStringList(options.value("enum").toStringList()));
    json.insert("icon",                             options.contains("icon") ? options.value("icon").toString() : "mdi:dip-switch");

    json.insert("value_template",                   QString("{{ value_json.%1 }}").arg(m_name));
    json.insert("state_topic",                      m_stateTopic);

    json.insert("command_template",                 QString("{\"%1\":\"{{ value }}\"}").arg(m_name));
    json.insert("command_topic",                    m_commandTopic);

    return json;
}

QJsonObject ButtonObject::request(void)
{
    QJsonObject json;

    json.insert("payload_press",                    m_payload);
    json.insert("command_topic",                    m_commandTopic);

    return json;
}

QJsonObject LightObject::request(void)
{
    QList <QString> options = option().toStringList();
    QString commandOnTemplate = "\"status\":\"on\"";
    QJsonObject json;

    if (options.contains("level"))
    {
        commandOnTemplate.append(                   "{% if brightness is defined %},\"level\":{{ brightness }}{% endif %}");
        json.insert("brightness_template",          "{{ value_json.level }}");
    }

    if (options.contains("color"))
    {
        commandOnTemplate.append(                   "{% if red is defined and green is defined and blue is defined %},\"color\":[{{ red }},{{ green }},{{ blue }}]{% endif %}");

        json.insert("red_template",                 "{{ value_json.color[0] }}");
        json.insert("green_template",               "{{ value_json.color[1] }}");
        json.insert("blue_template",                "{{ value_json.color[2] }}");
    }

    if (options.contains("colorTemperature"))
    {
        QMap <QString, QVariant> colorTemperature = option("colorTemperature").toMap();

        commandOnTemplate.append(                   "{% if color_temp is defined %},\"colorTemperature\":{{ color_temp }}{% endif %}");

        json.insert("color_temp_template",          "{{ value_json.colorTemperature }}");
        json.insert("min_mireds",                   colorTemperature.value("min", 150).toInt());
        json.insert("max_mireds",                   colorTemperature.value("max", 500).toInt());
    }

    json.insert("schema",                           "template");

    json.insert("state_template",                   "{{ value_json.status }}");
    json.insert("state_topic",                      m_stateTopic);

    json.insert("command_on_template",              QString("{%1}").arg(commandOnTemplate));
    json.insert("command_off_template",             "{\"status\":\"off\"}");
    json.insert("payload_on",                       "on");
    json.insert("payload_off",                      "off");
    json.insert("command_topic",                    m_commandTopic);

    return json;
}

QJsonObject SwitchObject::request(void)
{
    QString name = QString(m_name).replace("switch", "status");
    QJsonObject json;

    json.insert("device_class",                     option().toString() == "outlet" ? "outlet" : "switch");

    json.insert("value_template",                   QString("{{ value_json.%1 }}").arg(name));
    json.insert("state_on",                         "on");
    json.insert("state_off",                        "off");
    json.insert("state_topic",                      m_stateTopic);

    json.insert("payload_on",                       QString("{\"%1\":\"on\"}").arg(name));
    json.insert("payload_off",                      QString("{\"%1\":\"off\"}").arg(name));
    json.insert("command_topic",                    m_commandTopic);

    return json;
}

QJsonObject LockObject::request(void)
{
    QJsonObject json;

    if (option().toString() == "valve")
        json.insert("icon",                         "mdi:pipe-valve");

    json.insert("value_template",                   "{{ value_json.status }}");
    json.insert("state_locked",                     "off");
    json.insert("state_unlocked",                   "on");
    json.insert("state_topic",                      m_stateTopic);

    json.insert("payload_lock",                     "{\"status\":\"off\"}");
    json.insert("payload_unlock",                   "{\"status\":\"on\"}");
    json.insert("command_topic",                    m_commandTopic);

    return json;
}

QJsonObject CoverObject::request(void)
{
    QJsonObject json;

    json.insert("device_class",                     option().toString() == "blind" ? "blind" : "curtain");

    json.insert("value_template",                   "{{ value_json.cover }}");
    json.insert("state_open",                       "open");
    json.insert("state_closed",                     "closed");
    json.insert("state_topic",                      m_stateTopic);

    json.insert("position_template",                "{{ value_json.position }}");
    json.insert("position_topic",                   m_stateTopic);

    json.insert("payload_open",                     "{\"cover\":\"open\"}");
    json.insert("payload_close",                    "{\"cover\":\"close\"}");
    json.insert("payload_stop",                     "{\"cover\":\"stop\"}");
    json.insert("command_topic",                    m_commandTopic);

    json.insert("set_position_template",            "{\"position\":{{ position }}}");
    json.insert("set_position_topic",               m_commandTopic);

    return json;
}

QJsonObject ThermostatObject::request(void)
{
    QList <QString> operationMode = option("operationMode").toMap().value("enum").toStringList(), systemMode = option("systemMode").toMap().value("enum").toStringList();
    QJsonObject json;

    if (!operationMode.isEmpty())
    {
        json.insert("preset_modes",                 QJsonArray::fromStringList(operationMode));

        json.insert("preset_mode_value_template",   "{{ value_json.operationMode }}");
        json.insert("preset_mode_state_topic",      m_stateTopic);

        json.insert("preset_mode_command_template", "{\"operationMode\":\"{{ value }}\"}");
        json.insert("preset_mode_command_topic",    m_commandTopic);
    }

    json.insert("modes",                            systemMode.isEmpty() ? QJsonArray {"heat"} : QJsonArray::fromStringList(systemMode));

    json.insert("mode_state_template",              "{{ value_json.systemMode }}");
    json.insert("mode_state_topic",                 m_stateTopic);

    json.insert("mode_command_template",            "{\"systemMode\":\"{{ value }}\"}");
    json.insert("mode_command_topic",               m_commandTopic);

    json.insert("action_template",                  "{{ \"heating\" if value_json.heating else \"off\" }}");
    json.insert("action_topic",                     m_stateTopic);

    json.insert("current_temperature_template",     "{{ value_json.temperature }}");
    json.insert("current_temperature_topic",        m_stateTopic);

    json.insert("temperature_state_template",       "{{ value_json.targetTemperature }}");
    json.insert("temperature_state_topic",          m_stateTopic);

    json.insert("temperature_command_template",     "{\"targetTemperature\":{{ value }}}");
    json.insert("temperature_command_topic",        m_commandTopic);

    return json;
}
