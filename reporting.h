#ifndef REPORTINGOBJECT_H
#define REPORTINGOBJECT_H

#include <QSharedPointer>
#include "zcl.h"

class ReportingObject;
typedef QSharedPointer <ReportingObject> Reporting;

class ReportingObject
{

public:

    ReportingObject(const QString &name, quint16 clusterId, quint16 attributeId, quint8 dataType, quint16 minInterval, quint16 maxInterval, quint16 valueChange = 0) :
        m_name(name), m_clusterId(clusterId), m_attributeId(attributeId), m_dataType(dataType), m_minInterval(minInterval), m_maxInteval(maxInterval), m_valueChange(valueChange), m_endPointId(1) {}

    virtual ~ReportingObject(void) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) {return m_clusterId; }
    inline quint16 attributeId(void) {return m_attributeId; }
    inline quint8 dataType(void) {return m_dataType; }
    inline quint16 minInterval(void) {return m_minInterval; }
    inline quint16 maxInterval(void) {return m_maxInteval; }
    inline quint16 valueChange(void) {return m_valueChange; }

    inline quint8 endPointId(void) { return m_endPointId; }
    inline void setEndPointId(quint8 value) { m_endPointId = value; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId, m_attributeId;
    quint8 m_dataType;
    quint16 m_minInterval, m_maxInteval, m_valueChange;
    quint8 m_endPointId;

};

namespace Reportings
{
    class BatteryVoltage : public ReportingObject
    {

    public:

        BatteryVoltage(void) : ReportingObject("battery", CLUSTER_POWER_CONFIGURATION, 0x0020, DATA_TYPE_8BIT_UNSIGNED, 30, 3600, 10) {}

    };

    class BatteryPercentage : public ReportingObject
    {

    public:

        BatteryPercentage(void) : ReportingObject("battery", CLUSTER_POWER_CONFIGURATION, 0x0021, DATA_TYPE_8BIT_UNSIGNED, 30, 3600, 10) {}

    };

    class Status : public ReportingObject
    {

    public:

        Status(void) : ReportingObject("status", CLUSTER_ON_OFF, 0x0000, DATA_TYPE_BOOLEAN, 0, 600) {}

    };

    class Level : public ReportingObject
    {

    public:

        Level(void) :  ReportingObject("level", CLUSTER_LEVEL_CONTROL, 0x0000, DATA_TYPE_8BIT_UNSIGNED, 0, 600) {}

    };

    class ColorHue : public ReportingObject
    {

    public:

        ColorHue(void) : ReportingObject("colorHue", CLUSTER_COLOR_CONTROL, 0x0000, DATA_TYPE_8BIT_UNSIGNED, 0, 600) {}

    };

    class ColorSaturation : public ReportingObject
    {

    public:

        ColorSaturation(void) : ReportingObject("colorSaturation", CLUSTER_COLOR_CONTROL, 0x0001, DATA_TYPE_8BIT_UNSIGNED, 0, 600) {}

    };

    class ColorX : public ReportingObject
    {

    public:

        ColorX(void) : ReportingObject("colorX", CLUSTER_COLOR_CONTROL, 0x0003, DATA_TYPE_16BIT_UNSIGNED, 0, 600) {}

    };

    class ColorY : public ReportingObject
    {

    public:

        ColorY(void) : ReportingObject("colorY", CLUSTER_COLOR_CONTROL, 0x0004, DATA_TYPE_16BIT_UNSIGNED, 0, 600) {}

    };

    class ColorTemperature : public ReportingObject
    {

    public:

        ColorTemperature(void) : ReportingObject("colorTemperature", CLUSTER_COLOR_CONTROL, 0x0007, DATA_TYPE_16BIT_UNSIGNED, 0, 600) {}

    };

    class Illuminance : public ReportingObject
    {

    public:

        Illuminance(void) : ReportingObject("illuminance", CLUSTER_ILLUMINANCE_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_UNSIGNED, 30, 600, 10) {}

    };

    class Temperature : public ReportingObject
    {

    public:

        Temperature(void) : ReportingObject("temperature", CLUSTER_TEMPERATURE_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_SIGNED, 30, 600, 10) {}

    };

    class Humidity : public ReportingObject
    {

    public:

        Humidity(void) : ReportingObject("humidity", CLUSTER_RELATIVE_HUMIDITY, 0x0000, DATA_TYPE_16BIT_UNSIGNED, 30, 600, 10) {}

    };
}

Q_DECLARE_METATYPE(Reportings::BatteryVoltage)
Q_DECLARE_METATYPE(Reportings::BatteryPercentage)
Q_DECLARE_METATYPE(Reportings::Status)
Q_DECLARE_METATYPE(Reportings::Level)
Q_DECLARE_METATYPE(Reportings::ColorHue)
Q_DECLARE_METATYPE(Reportings::ColorSaturation)
Q_DECLARE_METATYPE(Reportings::ColorX)
Q_DECLARE_METATYPE(Reportings::ColorY)
Q_DECLARE_METATYPE(Reportings::ColorTemperature)
Q_DECLARE_METATYPE(Reportings::Illuminance)
Q_DECLARE_METATYPE(Reportings::Temperature)
Q_DECLARE_METATYPE(Reportings::Humidity)

#endif
