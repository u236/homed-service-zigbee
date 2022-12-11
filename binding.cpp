#include "binding.h"

void BindingObject::registerMetaTypes(void)
{
    qRegisterMetaType <Bindings::Battery>       ("batteryBinding");
    qRegisterMetaType <Bindings::Status>        ("statusBinding");
    qRegisterMetaType <Bindings::Level>         ("levelBinding");
    qRegisterMetaType <Bindings::Color>         ("colorBinding");
    qRegisterMetaType <Bindings::Illuminance>   ("illuminanceBinding");
    qRegisterMetaType <Bindings::Temperature>   ("temperatureBinding");
    qRegisterMetaType <Bindings::Humidity>      ("humidityBinding");
    qRegisterMetaType <Bindings::Energy>        ("energyBinding");
    qRegisterMetaType <Bindings::Power>         ("powerBinding");
    qRegisterMetaType <Bindings::Perenio>       ("perenioBinding");
}
