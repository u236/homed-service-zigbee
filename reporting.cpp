#include "reporting.h"
#include "zigbee.h"

using namespace Reportings;

BatteryVoltage::BatteryVoltage(quint8 endPointId) :
    ReportingObject("battery", CLUSTER_POWER_CONFIGURATION, 0x0020, DATA_TYPE_8BIT_UNSIGNED, 30, 3600, 10, endPointId) {}

BatteryPercentage::BatteryPercentage(quint8 endPointId) :
    ReportingObject("battery", CLUSTER_POWER_CONFIGURATION, 0x0021, DATA_TYPE_8BIT_UNSIGNED, 30, 3600, 10, endPointId) {}

Status::Status(quint8 endPointId) :
    ReportingObject("status", CLUSTER_ON_OFF, 0x0000, DATA_TYPE_BOOLEAN, 0, 600, 0, endPointId) {}

Level::Level(quint8 endPointId) :
    ReportingObject("level", CLUSTER_LEVEL_CONTROL, 0x0000, DATA_TYPE_8BIT_UNSIGNED, 0, 600, 0, endPointId) {}

ColorTemperature::ColorTemperature(quint8 endPointId) :
    ReportingObject("colorTemperature", CLUSTER_COLOR_CONTROL, 0x0007, DATA_TYPE_16BIT_UNSIGNED, 0, 600, 0, endPointId) {}

Illuminance::Illuminance(quint8 endPointId) :
    ReportingObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_UNSIGNED, 30, 600, 10, endPointId) {}

Temperature::Temperature(quint8 endPointId) :
    ReportingObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_SIGNED, 30, 600, 10, endPointId) {}

Humidity::Humidity(quint8 endPointId) :
    ReportingObject("humidity", CLUSTER_RELATIVE_HUMIDITY, 0x0000, DATA_TYPE_16BIT_UNSIGNED, 30, 600, 10, endPointId) {}
