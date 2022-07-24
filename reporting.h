#ifndef REPORTINGOBJECT_H
#define REPORTINGOBJECT_H

#include <QSharedPointer>
#include "zcl.h"

class ReportingObject;
typedef QSharedPointer <ReportingObject> Reporting;

class ReportingObject
{

public:

    ReportingObject(const QString &name, quint8 endPointId, quint16 clusterId, quint16 attributeId, quint8 dataType, quint16 minInterval, quint16 maxInterval, quint16 valueChange = 0) :
        m_name(name), m_endPointId(endPointId), m_clusterId(clusterId), m_attributeId(attributeId), m_dataType(dataType), m_minInterval(minInterval), m_maxInteval(maxInterval), m_valueChange(valueChange) {}

    inline QString name(void) { return m_name; }
    inline quint8 endPointId(void) {return m_endPointId; }
    inline quint16 clusterId(void) {return m_clusterId; }
    inline quint16 attributeId(void) {return m_attributeId; }
    inline quint8 dataType(void) {return m_dataType; }
    inline quint16 minInterval(void) {return m_minInterval; }
    inline quint16 maxInterval(void) {return m_maxInteval; }
    inline quint16 valueChange(void) {return m_valueChange; }

protected:

    QString m_name;
    quint8 m_endPointId;
    quint16 m_clusterId, m_attributeId;
    quint8 m_dataType;
    quint16 m_minInterval, m_maxInteval, m_valueChange;

};

namespace Reportings
{
    class BatteryVoltage : public ReportingObject
    {

    public:

        BatteryVoltage(quint8 endPointId = 1) :
            ReportingObject("battery", endPointId, CLUSTER_POWER_CONFIGURATION, 0x0020, DATA_TYPE_8BIT_UNSIGNED, 30, 3600, 10) {}

        virtual ~BatteryVoltage(void) {}

    };

    class BatteryPercentage : public ReportingObject
    {

    public:

        BatteryPercentage(quint8 endPointId = 1) :
            ReportingObject("battery", endPointId, CLUSTER_POWER_CONFIGURATION, 0x0021, DATA_TYPE_8BIT_UNSIGNED, 30, 3600, 10) {}

        virtual ~BatteryPercentage(void) {}

    };

    class Status : public ReportingObject
    {

    public:

        Status(quint8 endPointId = 1) :
            ReportingObject("status", endPointId, CLUSTER_ON_OFF, 0x0000, DATA_TYPE_BOOLEAN, 0, 600) {}

        virtual ~Status(void) {}

    };

    class Level : public ReportingObject
    {

    public:

        Level(quint8 endPointId = 1) :
            ReportingObject("level", endPointId, CLUSTER_LEVEL_CONTROL, 0x0000, DATA_TYPE_8BIT_UNSIGNED, 0, 600) {}

        virtual ~Level(void) {}

    };

    class ColorHue : public ReportingObject
    {

    public:

        ColorHue(quint8 endPointId = 1) :
            ReportingObject("colorHue", endPointId, CLUSTER_COLOR_CONTROL, 0x0000, DATA_TYPE_8BIT_UNSIGNED, 0, 600) {}

        virtual ~ColorHue(void) {}

    };

    class ColorSaturation : public ReportingObject
    {

    public:

        ColorSaturation(quint8 endPointId = 1) :
            ReportingObject("colorSaturation", endPointId, CLUSTER_COLOR_CONTROL, 0x0001, DATA_TYPE_8BIT_UNSIGNED, 0, 600) {}

        virtual ~ColorSaturation(void) {}

    };

    class ColorX : public ReportingObject
    {

    public:

        ColorX(quint8 endPointId = 1) :
            ReportingObject("colorX", endPointId, CLUSTER_COLOR_CONTROL, 0x0003, DATA_TYPE_16BIT_UNSIGNED, 0, 600) {}

        virtual ~ColorX(void) {}

    };

    class ColorY : public ReportingObject
    {

    public:

        ColorY(quint8 endPointId = 1) :
            ReportingObject("colorY", endPointId, CLUSTER_COLOR_CONTROL, 0x0004, DATA_TYPE_16BIT_UNSIGNED, 0, 600) {}

        virtual ~ColorY(void) {}

    };

    class ColorTemperature : public ReportingObject
    {

    public:

        ColorTemperature(quint8 endPointId = 1) :
            ReportingObject("colorTemperature", endPointId, CLUSTER_COLOR_CONTROL, 0x0007, DATA_TYPE_16BIT_UNSIGNED, 0, 600) {}

        virtual ~ColorTemperature(void) {}

    };

    class Illuminance : public ReportingObject
    {

    public:

        Illuminance(quint8 endPointId = 1) :
            ReportingObject("illuminance", endPointId, CLUSTER_ILLUMINANCE_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_UNSIGNED, 30, 600, 10) {}

        virtual ~Illuminance(void) {}

    };

    class Temperature : public ReportingObject
    {

    public:

        Temperature(quint8 endPointId = 1) :
            ReportingObject("temperature", endPointId, CLUSTER_TEMPERATURE_MEASUREMENT, 0x0000, DATA_TYPE_16BIT_SIGNED, 30, 600, 10) {}

        virtual ~Temperature(void) {}

    };

    class Humidity : public ReportingObject
    {

    public:

        Humidity(quint8 endPointId = 1) :
            ReportingObject("humidity", endPointId, CLUSTER_RELATIVE_HUMIDITY, 0x0000, DATA_TYPE_16BIT_UNSIGNED, 30, 600, 10) {}

        virtual ~Humidity(void) {}

    };
}

#endif
