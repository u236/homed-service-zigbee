#include "device.h"
#include "logger.h"

void DeviceObject::setProperties(void)
{
    if (m_vendor == "eWeLink")
    {
        if (m_model == "TH01")
        {
            m_properties = {Property(new Properties::BatteryPercentage), Property(new Properties::Temperature), Property(new Properties::Humidity)};
            m_reportings = {Reporting(new Reportings::BatteryPercentage), Reporting(new Reportings::Temperature), Reporting(new Reportings::Humidity)};
            return;
        }
    }
    else if (m_vendor == "HaiPaiTech")
    {
        if (m_model == "vivi ZLight")
        {
            m_actions = {Action(new Actions::Status(11)), Action(new Actions::Level(11)), Action(new Actions::ColorHS(11))};
            m_properties = {Property(new Properties::Status), Property(new Properties::Level), Property(new Properties::ColorHS)};
            m_polls = {Poll(new Polls::Status(11)), Poll(new Polls::Level(11)), Poll(new Polls::ColorHS(11))};

            return;
        }
    }
    else if (m_vendor == "HOMEd")
    {
        if (m_model == "Button")
        {
            m_properties = {Property(new Properties::BatteryPercentage), Property(new Properties::SwitchActionPTVO)};
            return;
        }

        if (m_model == "CO2 Sensor")
        {
            m_properties = {Property(new Properties::AnalogCO2), Property(new Properties::AnalogTemperature)};
            return;
        }

        if (m_model == "LC Outlet")
        {
            m_actions = {Action(new Actions::Status)};
            m_properties = {Property(new Properties::Status)};
            return;
        }
    }
    else if (m_vendor == "IKEA of Sweden")
    {
        if (m_model == "TRADFRI on/off switch")
        {
            m_properties = {Property(new Properties::BatteryPercentageIKEA), Property(new Properties::IdentifyAction), Property(new Properties::SwitchAction), Property(new Properties::LevelAction)};
            m_reportings = {Reporting(new Reportings::BatteryPercentage)};
            return;
        }

        if (m_model == "TRADFRI bulb E14 WS 470lm")
        {
            m_actions = {Action(new Actions::Status), Action(new Actions::Level), Action(new Actions::ColorTemperature)};
            m_properties = {Property(new Properties::Status), Property(new Properties::Level), Property(new Properties::ColorTemperature)};
            m_reportings = {Reporting(new Reportings::Status), Reporting(new Reportings::Level), Reporting(new Reportings::ColorTemperature)};
            return;
        }

        if (m_model == "TRADFRIbulbE14WWclear250lm" || m_model == "TRADFRIbulbE27WWclear250lm" || m_model == "TRADFRI bulb E27 W opal 1000lm")
        {
            m_actions = {Action(new Actions::Status), Action(new Actions::Level)};
            m_properties = {Property(new Properties::Status), Property(new Properties::Level)};
            m_reportings = {Reporting(new Reportings::Status), Reporting(new Reportings::Level)};
            return;
        }
    }
    else if (m_vendor == "LUMI")
    {
        if (m_model == "lumi.plug.maeu01")
        {
            m_actions = {Action(new Actions::Status)};
            m_properties = {Property(new Properties::Status)}; // TODO: add power measuring properties
            m_reportings = {Reporting(new Reportings::Status)};
            return;
        }

        if (m_model == "lumi.sensor_cube" || m_model == "lumi.sensor_cube.aqgl01") // TODO: check this
        {
            m_properties = {Property(new Properties::BatteryVoltageLUMI), Property(new Properties::CubeMovement), Property(new Properties::CubeRotation)};
            return;
        }

        if (m_model == "lumi.sensor_ht" || m_model == "lumi.sens")
        {
            m_properties = {Property(new Properties::BatteryVoltageLUMI), Property(new Properties::Temperature), Property(new Properties::Humidity)};
            return;
        }

        if (m_model == "lumi.sensor_magnet")
        {
            m_properties = {Property(new Properties::BatteryVoltageLUMI), Property(new Properties::Status)};
            return;
        }

        if (m_model == "lumi.sensor_motion")
        {
            m_properties = {Property(new Properties::BatteryVoltageLUMI), Property(new Properties::Occupancy)};
            return;
        }

        if (m_model == "lumi.sensor_switch")
        {
            m_properties = {Property(new Properties::BatteryVoltageLUMI), Property(new Properties::SwitchActionLUMI)};
            return;
        }
    }
    else if (m_vendor == "XIAOMI")
    {
        if (m_model == "lumi.sen_ill.mgl01")
        {
            m_properties = {Property(new Properties::BatteryVoltage), Property(new Properties::IdentifyAction), Property(new Properties::Illuminance)};
            m_reportings = {Reporting(new Reportings::BatteryVoltage), Reporting(new Reportings::Illuminance)};
            return;
        }
    }

    logWarning << "Unrecognized device" << name() << "vendor" << m_vendor << "and model" << m_model;
}
