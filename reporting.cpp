#include "reporting.h"

void ReportingObject::registerMetaTypes(void)
{
    qRegisterMetaType <Reportings::BatteryVoltage>      ("batteryVoltageReporting");
    qRegisterMetaType <Reportings::BatteryPercentage>   ("batteryPercentageReporting");
    qRegisterMetaType <Reportings::DeviceTemperature>   ("deviceTemperatureReporting");
    qRegisterMetaType <Reportings::Status>              ("statusReporting");
    qRegisterMetaType <Reportings::Level>               ("levelReporting");
    qRegisterMetaType <Reportings::AnalogInput>         ("analogInputReporting");
    qRegisterMetaType <Reportings::AnalogOutput>        ("analogOutputReporting");
    qRegisterMetaType <Reportings::CoverPosition>       ("coverPositionReporting");
    qRegisterMetaType <Reportings::Thermostat>          ("thermostatReporting");
    qRegisterMetaType <Reportings::ColorHS>             ("colorHSReporting");
    qRegisterMetaType <Reportings::ColorXY>             ("colorXYReporting");
    qRegisterMetaType <Reportings::ColorTemperature>    ("colorTemperatureReporting");
    qRegisterMetaType <Reportings::Illuminance>         ("illuminanceReporting");
    qRegisterMetaType <Reportings::Temperature>         ("temperatureReporting");
    qRegisterMetaType <Reportings::Pressure>            ("pressureReporting");
    qRegisterMetaType <Reportings::Flow>                ("flowReporting");
    qRegisterMetaType <Reportings::Humidity>            ("humidityReporting");
    qRegisterMetaType <Reportings::Occupancy>           ("occupancyReporting");
    qRegisterMetaType <Reportings::Moisture>            ("moistureReporting");
    qRegisterMetaType <Reportings::CO2>                 ("co2Reporting");
    qRegisterMetaType <Reportings::PM25>                ("pm25Reporting");
    qRegisterMetaType <Reportings::Energy>              ("energyReporting");
    qRegisterMetaType <Reportings::EnergyT1>            ("energyT1Reporting");
    qRegisterMetaType <Reportings::EnergyT2>            ("energyT2Reporting");
    qRegisterMetaType <Reportings::EnergyT3>            ("energyT3Reporting");
    qRegisterMetaType <Reportings::EnergyT4>            ("energyT4Reporting");
    qRegisterMetaType <Reportings::Voltage>             ("voltageReporting");
    qRegisterMetaType <Reportings::Current>             ("currentReporting");
    qRegisterMetaType <Reportings::Power>               ("powerReporting");
    qRegisterMetaType <Reportings::Frequency>           ("frequencyReporting");
    qRegisterMetaType <Reportings::PowerFactor>         ("powerFactorReporting");

    qRegisterMetaType <ReportingsEfekta::PMSensor>      ("efektaPMSensorReporting");
    qRegisterMetaType <ReportingsEfekta::VOCSensor>     ("efektaVOCSensorReporting");
}
