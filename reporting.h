#ifndef REPORTINGOBJECT_H
#define REPORTINGOBJECT_H

#include <QSharedPointer>

class ReportingObject;
typedef QSharedPointer <ReportingObject> Reporting;

class ReportingObject
{

public:

    ReportingObject(const QString &name, quint16 clusterId, quint16 attributeId, quint8 dataType, quint16 minInterval, quint16 maxInterval, quint16 valueChange, quint8 endPointId) :
        m_name(name), m_clusterId(clusterId), m_attributeId(attributeId), m_dataType(dataType), m_minInterval(minInterval), m_maxInteval(maxInterval), m_valueChange(valueChange), m_endPointId(endPointId) {}

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) {return m_clusterId; }
    inline quint16 attributeId(void) {return m_attributeId; }
    inline quint8 dataType(void) {return m_dataType; }
    inline quint16 minInterval(void) {return m_minInterval; }
    inline quint16 maxInterval(void) {return m_maxInteval; }
    inline quint16 valueChange(void) {return m_valueChange; }
    inline quint8 endPointId(void) {return m_endPointId; }

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

        BatteryVoltage(quint8 endPointId = 1);
        virtual ~BatteryVoltage(void) {}

    };

    class BatteryPercentage : public ReportingObject
    {

    public:

        BatteryPercentage(quint8 endPointId = 1);
        virtual ~BatteryPercentage(void) {}

    };

    class Status : public ReportingObject
    {

    public:

        Status(quint8 endPointId = 1);
        virtual ~Status(void) {}

    };

    class Level : public ReportingObject
    {

    public:

        Level(quint8 endPointId = 1);
        virtual ~Level(void) {}

    };

    class ColorTemperature : public ReportingObject
    {

    public:

        ColorTemperature(quint8 endPointId = 1);
        virtual ~ColorTemperature(void) {}

    };

    class ColorHue : public ReportingObject
    {

    public:

        ColorHue(quint8 endPointId = 1);
        virtual ~ColorHue(void) {}

    };

    class ColorSaturation : public ReportingObject
    {

    public:

        ColorSaturation(quint8 endPointId = 1);
        virtual ~ColorSaturation(void) {}

    };

    class ColorX : public ReportingObject
    {

    public:

        ColorX(quint8 endPointId = 1);
        virtual ~ColorX(void) {}

    };

    class ColorY : public ReportingObject
    {

    public:

        ColorY(quint8 endPointId = 1);
        virtual ~ColorY(void) {}

    };

    class Illuminance : public ReportingObject
    {

    public:

        Illuminance(quint8 endPointId = 1);
        virtual ~Illuminance(void) {}

    };

    class Temperature : public ReportingObject
    {

    public:

        Temperature(quint8 endPointId = 1);
        virtual ~Temperature(void) {}

    };

    class Humidity : public ReportingObject
    {

    public:

        Humidity(quint8 endPointId = 1);
        virtual ~Humidity(void) {}

    };
}

#endif
