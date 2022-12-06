#include "device.h"
#include "discovery.h"

void DiscoveryObject::registerMetaTypes(void)
{
    qRegisterMetaType <LightObject>             ("lightDiscovery");
    qRegisterMetaType <SwitchObject>            ("switchDiscovery");

    qRegisterMetaType <Binary::Contact>         ("contactDiscovery");
    qRegisterMetaType <Binary::Gas>             ("gasDiscovery");
    qRegisterMetaType <Binary::Occupancy>       ("occupancyDiscovery");
    qRegisterMetaType <Binary::Smoke>           ("smokeDiscovery");
    qRegisterMetaType <Binary::WaterLeak>       ("waterLeakDiscovery");

    qRegisterMetaType <Sensor::Action>          ("actionDiscovery");
    qRegisterMetaType <Sensor::Scene>           ("sceneDiscovery");
    qRegisterMetaType <Sensor::Count>           ("countDiscovery");
    qRegisterMetaType <Sensor::Battery>         ("batteryDiscovery");
    qRegisterMetaType <Sensor::Temperature>     ("temperatureDiscovery");
    qRegisterMetaType <Sensor::Humidity>        ("humidityDiscovery");
    qRegisterMetaType <Sensor::CO2>             ("co2Discovery");
    qRegisterMetaType <Sensor::VOC>             ("vocDiscovery");
    qRegisterMetaType <Sensor::Illuminance>     ("illuminanceDiscovery");
    qRegisterMetaType <Sensor::Energy>          ("energyDiscovery");
    qRegisterMetaType <Sensor::Voltage>         ("voltageDiscovery");
    qRegisterMetaType <Sensor::Current>         ("currentDiscovery");
    qRegisterMetaType <Sensor::Power>           ("powerDiscovery");

    qRegisterMetaType <Button::ResetCount>      ("resetCountDiscovery");
}

QVariant DiscoveryObject::deviceOption(const QString &key)
{
    EndpointObject *endpoint = reinterpret_cast <EndpointObject*> (m_parent);
    return (endpoint && !endpoint->device().isNull()) ? endpoint->device()->options().value(key) : QVariant();
}

QJsonObject BinaryObject::reqest(void)
{
    QList <QString> list = {"contact", "waterLeak"};
    QJsonObject json;

    switch (list.indexOf(m_name))
    {
        case 0:  json.insert("device_class",    deviceOption("contact").toString() == "window" ? "window" : "door"); break;
        case 1:  json.insert("device_class",    "moisture"); break;
        default: json.insert("device_class",    m_name); break;
    }

    json.insert("payload_on",                   true);
    json.insert("payload_off",                  false);
    json.insert("value_template",               QString("{{ value_json.%1 }}").arg(m_name));

    return json;
}

QJsonObject SensorObject::reqest(void)
{
    QList <QString> list = {"action", "scene", "count", "co2", "voc"}, valueTemplate = {QString("value_json.%1").arg(m_name)};
    QJsonObject json;

    switch (list.indexOf(m_name))
    {
        case 0:  json.insert("icon",            "mdi:gesture-double-tap"); break;
        case 1:  json.insert("icon",            "mdi:gesture-tap-button"); break;
        case 2:  json.insert("icon",            "mdi:counter"); break;
        case 3:  json.insert("device_class",    "carbon_dioxide"); break;
        case 4:  json.insert("device_class",    "volatile_organic_compounds"); break;
        default: json.insert("device_class",    m_name); break;
    }

    if (!m_unit.isEmpty())
        json.insert("unit_of_measurement",      m_unit);

    if (m_round)
        valueTemplate.append(                   QString("round(%1)").arg(m_round));

    json.insert("value_template",               QString("{{ %1 }}").arg(valueTemplate.join(" | ")));
    return json;
}

QJsonObject LightObject::reqest(void)
{
    QList <QVariant> options = deviceOption("light").toList();
    QString commandOnTemplate = "\"status\":\"on\"";
    QJsonObject json;

    if (options.contains("level"))
    {
        commandOnTemplate.append(               "{% if brightness is defined %},\"level\":{{ brightness }}{% endif %}");
        json.insert("brightness_template",      "{{ value_json.level }}");
    }

    if (options.contains("color"))
    {
        commandOnTemplate.append(               "{% if red is defined and green is defined and blue is defined %},\"color\":[{{ red }},{{ green }},{{ blue }}]{% endif %}");
        json.insert("red_template",             "{{ value_json.color[0] }}");
        json.insert("green_template",           "{{ value_json.color[1] }}");
        json.insert("blue_template",            "{{ value_json.color[2] }}");
    }

    if (options.contains("colorTemperature"))
    {
        quint16 min = static_cast <quint16> (deviceOption("colorTemperatureMin").toInt()), max = static_cast <quint16> (deviceOption("colorTemperatureMax").toInt());

        commandOnTemplate.append(               "{% if color_temp is defined %},\"colorTemperature\":{{ color_temp }}{% endif %}");
        json.insert("color_temp_template",      "{{ value_json.colorTemperature }}");
        json.insert("min_mireds",               min ? min : 150);
        json.insert("max_mireds",               max ? max : 500);
    }

    json.insert("command_on_template",          QString("{%1}").arg(commandOnTemplate));
    json.insert("command_off_template",         "{\"status\":\"off\"}");
    json.insert("payload_on",                   "on");
    json.insert("payload_off",                  "off");
    json.insert("schema",                       "template");
    json.insert("state_template",               "{{ value_json.status }}");

    return json;
}

QJsonObject SwitchObject::reqest(void)
{
    QJsonObject json;

    json.insert("device_class",                 deviceOption("switch").toString() == "outlet" ? "outlet" : "switch");
    json.insert("payload_on",                   "{\"status\":\"on\"}");
    json.insert("payload_off",                  "{\"status\":\"off\"}");
    json.insert("state_on",                     "on");
    json.insert("state_off",                    "off");
    json.insert("value_template",               "{{ value_json.status }}");

    return json;
}

QJsonObject ButtonObject::reqest(void)
{
    return {{"payload_press",                   m_payload}};
}
