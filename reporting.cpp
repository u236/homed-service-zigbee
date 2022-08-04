#include "reporting.h"

void ReportingObject::registerMetaTypes(void)
{
    qRegisterMetaType <Reportings::BatteryVoltage>      ("batteryVoltageReporting");
    qRegisterMetaType <Reportings::BatteryPercentage>   ("batteryPercentageReporting");
    qRegisterMetaType <Reportings::Status>              ("statusReporting");
    qRegisterMetaType <Reportings::Level>               ("levelReporting");
    qRegisterMetaType <Reportings::ColorHue>            ("colorHueReporting");
    qRegisterMetaType <Reportings::ColorSaturation>     ("colorSaturationReporting");
    qRegisterMetaType <Reportings::ColorX>              ("colorXReporting");
    qRegisterMetaType <Reportings::ColorY>              ("colorYReporting");
    qRegisterMetaType <Reportings::ColorTemperature>    ("colorTemperatureReporting");
    qRegisterMetaType <Reportings::Illuminance>         ("illuminanceReporting");
    qRegisterMetaType <Reportings::Temperature>         ("temperatureReporting");
    qRegisterMetaType <Reportings::Humidity>            ("humidityReporting");
    qRegisterMetaType <Reportings::Energy>              ("energyReporting");
    qRegisterMetaType <Reportings::Power>               ("powerReporting");
}
