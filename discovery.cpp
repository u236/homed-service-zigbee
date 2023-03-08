#include "device.h"
#include "discovery.h"

void DiscoveryObject::registerMetaTypes(void)
{
    qRegisterMetaType <LightObject>                 ("lightDiscovery");
    qRegisterMetaType <SwitchObject>                ("switchDiscovery");
    qRegisterMetaType <CoverObject>                 ("coverDiscovery");
    qRegisterMetaType <ThermostatObject>            ("thermostatDiscovery");

    qRegisterMetaType <Binary::Contact>             ("contactDiscovery");
    qRegisterMetaType <Binary::Gas>                 ("gasDiscovery");
    qRegisterMetaType <Binary::Occupancy>           ("occupancyDiscovery");
    qRegisterMetaType <Binary::Smoke>               ("smokeDiscovery");
    qRegisterMetaType <Binary::WaterLeak>           ("waterLeakDiscovery");
    qRegisterMetaType <Binary::Vibration>           ("vibrationDiscovery");

    qRegisterMetaType <Sensor::Action>              ("actionDiscovery");
    qRegisterMetaType <Sensor::Event>               ("eventDiscovery");
    qRegisterMetaType <Sensor::Scene>               ("sceneDiscovery");
    qRegisterMetaType <Sensor::Count>               ("countDiscovery");
    qRegisterMetaType <Sensor::Battery>             ("batteryDiscovery");
    qRegisterMetaType <Sensor::Temperature>         ("temperatureDiscovery");
    qRegisterMetaType <Sensor::Pressure>            ("pressureDiscovery");
    qRegisterMetaType <Sensor::Humidity>            ("humidityDiscovery");
    qRegisterMetaType <Sensor::Moisture>            ("moistureDiscovery");
    qRegisterMetaType <Sensor::CO2>                 ("co2Discovery");
    qRegisterMetaType <Sensor::VOC>                 ("vocDiscovery");
    qRegisterMetaType <Sensor::Illuminance>         ("illuminanceDiscovery");
    qRegisterMetaType <Sensor::Energy>              ("energyDiscovery");
    qRegisterMetaType <Sensor::Voltage>             ("voltageDiscovery");
    qRegisterMetaType <Sensor::Current>             ("currentDiscovery");
    qRegisterMetaType <Sensor::Power>               ("powerDiscovery");

    qRegisterMetaType <Number::Pattern>             ("patternDiscovery");
    qRegisterMetaType <Number::ReportingDelay>      ("reportingDelayDiscovery");
    qRegisterMetaType <Number::TemperatureOffset>   ("temperatureOffsetDiscovery");
    qRegisterMetaType <Number::Timer>               ("timerDiscovery");
    qRegisterMetaType <Number::Threshold>           ("thresholdDiscovery");

    qRegisterMetaType <Button::ResetCount>          ("resetCountDiscovery");
}

QVariant DiscoveryObject::endpointOption(const QString &name)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    QVariant value;

    if (!endpoint || endpoint->device().isNull())
        return value;

    value = endpoint->device()->options().value(QString("%1-%2").arg(name.isEmpty() ? m_name : name, QString::number(endpoint->id())));
    return value.isValid() ? value : endpoint->device()->options().value(name.isEmpty() ? m_name : name);
}

QJsonObject BinaryObject::reqest(void)
{
    QList <QString> list = {"alarm", "contact", "waterLeak"};
    QJsonObject json;

    switch (list.indexOf(m_name))
    {
        case 0:  break;
        case 1:  json.insert("device_class",        endpointOption().toString() == "window" ? "window" : "door"); break;
        case 2:  json.insert("device_class",        "moisture"); break;
        default: json.insert("device_class",        m_name); break;
    }

    json.insert("value_template",                   QString("{{ value_json.%1 }}").arg(m_name));
    json.insert("payload_on",                       true);
    json.insert("payload_off",                      false);
    json.insert("state_topic",                      m_stateTopic);

    return json;
}

QJsonObject SensorObject::reqest(void)
{
    QList <QString> list = {"action", "event", "scene", "count", "co2", "voc"}, valueTemplate = {QString("value_json.%1").arg(m_name)};
    QJsonObject json;

    switch (list.indexOf(m_name))
    {
        case 0:  json.insert("icon",                "mdi:gesture-double-tap"); break;
        case 1:  json.insert("icon",                "mdi:bell"); break;
        case 2:  json.insert("icon",                "mdi:gesture-tap-button"); break;
        case 3:  json.insert("icon",                "mdi:counter"); break;
        case 4:  json.insert("device_class",        "carbon_dioxide"); break;
        case 5:  json.insert("device_class",        "volatile_organic_compounds"); break;
        default: json.insert("device_class",        m_name); break;
    }

    if (!m_unit.isEmpty())
        json.insert("unit_of_measurement",          m_unit);

    if (m_round)
        valueTemplate.append(                       QString("round(%1)").arg(m_round));

    json.insert("value_template",                   QString("{{ %1 }}").arg(valueTemplate.join(" | ")));
    json.insert("state_topic",                      m_stateTopic);

    return json;
}

QJsonObject NumberObject::reqest(void)
{
    QMap <QString, QVariant> options = endpointOption().toMap();
    QJsonObject json;

    if (options.contains("min"))
        json.insert("min",                          options.value("min").toDouble());

    if (options.contains("max"))
        json.insert("max",                          options.value("max").toDouble());

    if (options.contains("step"))
        json.insert("step",                         options.value("step").toDouble());

    if (options.contains("unit"))
        json.insert("unit_of_measurement",          options.value("unit").toString());

    if (!m_icon.isEmpty())
        json.insert("icon",                         m_icon);

    json.insert("value_template",                   QString("{{ value_json.%1 }}").arg(m_name));
    json.insert("state_topic",                      m_stateTopic);

    json.insert("command_template",                 QString("{\"%1\":{{ value }}}").arg(m_name));
    json.insert("command_topic",                    m_commandTopic);

    return json;
}

QJsonObject ButtonObject::reqest(void)
{
    QJsonObject json;

    json.insert("payload_press",                    m_payload);
    json.insert("command_topic",                    m_commandTopic);

    return json;
}

QJsonObject LightObject::reqest(void)
{
    QList <QString> options = endpointOption().toStringList();
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
        QMap <QString, QVariant> colorTemperature = endpointOption("colorTemperature").toMap();

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

QJsonObject SwitchObject::reqest(void)
{
    QJsonObject json;

    json.insert("device_class",                     endpointOption().toString() == "outlet" ? "outlet" : "switch");

    json.insert("value_template",                   "{{ value_json.status }}");
    json.insert("state_on",                         "on");
    json.insert("state_off",                        "off");
    json.insert("state_topic",                      m_stateTopic);

    json.insert("payload_on",                       "{\"status\":\"on\"}");
    json.insert("payload_off",                      "{\"status\":\"off\"}");
    json.insert("command_topic",                    m_commandTopic);

    return json;
}

QJsonObject CoverObject::reqest(void)
{
    QJsonObject json;

    json.insert("device_class",                     endpointOption().toString() == "blind" ? "blind" : "curtain");

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

QJsonObject ThermostatObject::reqest(void)
{
    QMap <QString, QVariant> options = endpointOption().toMap();
    QJsonObject json;

    if (options.contains("operationMode"))
    {
        json.insert("preset_modes",                 QJsonArray::fromStringList(options.value("operationMode").toStringList()));

        json.insert("preset_mode_value_template",   "{{ value_json.operationMode }}");
        json.insert("preset_mode_state_topic",      m_stateTopic);

        json.insert("preset_mode_command_template", "{\"operationMode\":\"{{ value }}\"}");
        json.insert("preset_mode_command_topic",    m_commandTopic);
    }

    json.insert("modes",                            options.value("status").toBool() ? QJsonArray {"off", "heat"} : QJsonArray {"heat"});

    json.insert("mode_state_template",              "{{ \"heat\" if value_json.status == \"on\" else \"off\" }}");
    json.insert("mode_state_topic",                 m_stateTopic);

    json.insert("mode_command_template",            "{\"status\":\"{{ \"on\" if value == \"heat\" else \"off\" }}\"}");
    json.insert("mode_command_topic",               m_commandTopic);

    json.insert("action_template",                  "{{ \"heating\" if value_json.heating else \"off\" }}");
    json.insert("action_topic",                     m_stateTopic);

    json.insert("current_temperature_template",     "{{ value_json.localTemperature }}");
    json.insert("current_temperature_topic",        m_stateTopic);

    json.insert("temperature_state_template",       "{{ value_json.heatingPoint }}");
    json.insert("temperature_state_topic",          m_stateTopic);

    json.insert("temperature_command_template",     "{\"heatingPoint\":{{ value }}}");
    json.insert("temperature_command_topic",        m_commandTopic);

    return json;
}
