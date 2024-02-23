#include "binding.h"

void BindingObject::registerMetaTypes(void)
{
    qRegisterMetaType <Bindings::Battery>               ("batteryBinding");
    qRegisterMetaType <Bindings::DeviceTemperature>     ("deviceTemperatureBinding");
    qRegisterMetaType <Bindings::Scene>                 ("sceneBinding");
    qRegisterMetaType <Bindings::Status>                ("statusBinding");
    qRegisterMetaType <Bindings::Level>                 ("levelBinding");
    qRegisterMetaType <Bindings::Time>                  ("timeBinding");
    qRegisterMetaType <Bindings::AnalogInput>           ("analogInputBinding");
    qRegisterMetaType <Bindings::AnalogOutput>          ("analogOutputBinding");
    qRegisterMetaType <Bindings::MultistateInput>       ("multistateInputBinding");
    qRegisterMetaType <Bindings::PollControl>           ("pollControlBinding");
    qRegisterMetaType <Bindings::Cover>                 ("coverBinding");
    qRegisterMetaType <Bindings::Thermostat>            ("thermostatBinding");
    qRegisterMetaType <Bindings::Fan>                   ("fanBinding");
    qRegisterMetaType <Bindings::Color>                 ("colorBinding");
    qRegisterMetaType <Bindings::Illuminance>           ("illuminanceBinding");
    qRegisterMetaType <Bindings::Temperature>           ("temperatureBinding");
    qRegisterMetaType <Bindings::Pressure>              ("pressureBinding");
    qRegisterMetaType <Bindings::Humidity>              ("humidityBinding");
    qRegisterMetaType <Bindings::Occupancy>             ("occupancyBinding");
    qRegisterMetaType <Bindings::Moisture>              ("moistureBinding");
    qRegisterMetaType <Bindings::CO2>                   ("co2Binding");
    qRegisterMetaType <Bindings::PM25>                  ("pm25Binding");
    qRegisterMetaType <Bindings::Energy>                ("energyBinding");
    qRegisterMetaType <Bindings::Power>                 ("powerBinding");
    qRegisterMetaType <Bindings::Perenio>               ("perenioBinding");
}
