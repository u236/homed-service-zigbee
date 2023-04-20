#include "binding.h"

void BindingObject::registerMetaTypes(void)
{
    qRegisterMetaType <Bindings::Battery>               ("batteryBinding");
    qRegisterMetaType <Bindings::DeviceTemperature>     ("deviceTemperatureBinding");
    qRegisterMetaType <Bindings::Status>                ("statusBinding");
    qRegisterMetaType <Bindings::Level>                 ("levelBinding");
    qRegisterMetaType <Bindings::AnalogInput>           ("analogInputBinding");
    qRegisterMetaType <Bindings::Color>                 ("colorBinding");
    qRegisterMetaType <Bindings::Illuminance>           ("illuminanceBinding");
    qRegisterMetaType <Bindings::Temperature>           ("temperatureBinding");
    qRegisterMetaType <Bindings::Pressure>              ("pressureBinding");
    qRegisterMetaType <Bindings::Humidity>              ("humidityBinding");
    qRegisterMetaType <Bindings::Moisture>              ("moistureBinding");
    qRegisterMetaType <Bindings::CO2>                   ("co2Binding");
    qRegisterMetaType <Bindings::Energy>                ("energyBinding");
    qRegisterMetaType <Bindings::Power>                 ("powerBinding");
    qRegisterMetaType <Bindings::Perenio>               ("perenioBinding");
}
