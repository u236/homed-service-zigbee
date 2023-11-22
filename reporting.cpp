#include "reporting.h"

void ReportingObject::registerMetaTypes(void)
{
    qRegisterMetaType <Reportings::BatteryVoltage>          ("batteryVoltageReporting");
    qRegisterMetaType <Reportings::BatteryPercentage>       ("batteryPercentageReporting");
    qRegisterMetaType <Reportings::DeviceTemperature>       ("deviceTemperatureReporting");
    qRegisterMetaType <Reportings::Status>                  ("statusReporting");
    qRegisterMetaType <Reportings::Level>                   ("levelReporting");
    qRegisterMetaType <Reportings::CoverPosition>           ("coverPositionReporting");
    qRegisterMetaType <Reportings::CoverTilt>               ("coverTiltReporting");
    qRegisterMetaType <Reportings::ColorHS>                 ("colorHSReporting");
    qRegisterMetaType <Reportings::ColorXY>                 ("colorXYReporting");
    qRegisterMetaType <Reportings::ColorTemperature>        ("colorTemperatureReporting");
    qRegisterMetaType <Reportings::Illuminance>             ("illuminanceReporting");
    qRegisterMetaType <Reportings::Temperature>             ("temperatureReporting");
    qRegisterMetaType <Reportings::Pressure>                ("pressureReporting");
    qRegisterMetaType <Reportings::Humidity>                ("humidityReporting");
    qRegisterMetaType <Reportings::Moisture>                ("moistureReporting");
    qRegisterMetaType <Reportings::CO2>                     ("co2Reporting");
    qRegisterMetaType <Reportings::PM25>                    ("pm25Reporting");
    qRegisterMetaType <Reportings::Energy>                  ("energyReporting");
    qRegisterMetaType <Reportings::Voltage>                 ("voltageReporting");
    qRegisterMetaType <Reportings::Current>                 ("currentReporting");
    qRegisterMetaType <Reportings::Power>                   ("powerReporting");


    qRegisterMetaType <ReportingsModkam::EventsPerMinute>   ("modkamEventsPerMinuteReporting");
    qRegisterMetaType <ReportingsModkam::DosePerHour>       ("modkamDosePerHourReporting");

    qRegisterMetaType <ReportingsPerenio::Voltage>          ("perenioVoltageReporting");
    qRegisterMetaType <ReportingsPerenio::Power>            ("perenioPowerReporting");
    qRegisterMetaType <ReportingsPerenio::Energy>           ("perenioEnergyReporting");
}
