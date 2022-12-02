#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <QJsonObject>

class DiscoveryObject;
typedef QSharedPointer <DiscoveryObject> Discovery;

class DiscoveryObject
{

public:

    DiscoveryObject(const QString &component, const QString &name = QString(), bool control = true) :
        m_component(component), m_name(name.isEmpty() ? component : name), m_control(control) {}

    virtual ~DiscoveryObject(void) {}
    virtual QJsonObject reqest(void) = 0;

    inline QString component(void) { return m_component; }
    inline QString name(void) { return m_name; }
    inline bool control(void) { return m_control; }

    inline void setParent(QObject *value) { m_parent = value; }

    inline bool multiple(void) { return m_multiple; }
    inline void setMultiple(bool value) { m_multiple = value; }

    static void registerMetaTypes(void);

protected:

    QString m_component, m_name;
    bool m_control;

    QObject *m_parent;
    bool m_multiple;

    QVariant deviceOption(const QString &key);

};

class BinaryObject : public DiscoveryObject
{

public:

    BinaryObject(const QString &name) : DiscoveryObject("binary_sensor", name, false) {}
    QJsonObject reqest(void) override;

};

class SensorObject : public DiscoveryObject
{

public:

    SensorObject(const QString &name, const QString &unit = QString(), quint8 round = 0) : DiscoveryObject("sensor", name, false), m_unit(unit), m_round(round) {}
    QJsonObject reqest(void) override;

private:

    QString m_unit;
    quint8 m_round;

};

class LightObject : public DiscoveryObject
{

public:

    LightObject(void) : DiscoveryObject("light") {}
    QJsonObject reqest(void) override;

};

class SwitchObject : public DiscoveryObject
{

public:

    SwitchObject(void) : DiscoveryObject("switch") {}
    QJsonObject reqest(void) override;

};

class ButtonObject  : public DiscoveryObject
{

public:

    ButtonObject(const QString &name, const QString &payload) : DiscoveryObject("button", name, true), m_payload(payload) {}
    QJsonObject reqest(void) override;

private:

    QString m_payload;

};

namespace Binary
{
    class Contact : public BinaryObject
    {

    public:

        Contact(void) : BinaryObject("contact") {}

    };

    class Gas : public BinaryObject
    {

    public:

        Gas(void) : BinaryObject("gas") {}

    };

    class Occupancy : public BinaryObject
    {

    public:

        Occupancy(void) : BinaryObject("occupancy") {}

    };

    class Smoke : public BinaryObject
    {

    public:

        Smoke(void) : BinaryObject("smoke") {}

    };

    class WaterLeak : public BinaryObject
    {

    public:

        WaterLeak(void) : BinaryObject("waterLeak") {}

    };
}

namespace Sensor
{
    class Action : public SensorObject
    {

    public:

        Action(void) : SensorObject("action") {}

    };

    class Scene : public SensorObject
    {

    public:

        Scene(void) : SensorObject("scene") {}

    };

    class Count : public SensorObject
    {

    public:

        Count(void) : SensorObject("count") {}

    };

    class Battery : public SensorObject
    {

    public:

        Battery(void) : SensorObject("battery", "%", 1) {}

    };

    class Temperature : public SensorObject
    {

    public:

        Temperature(void) : SensorObject("temperature", "°C", 1) {}

    };

    class Humidity : public SensorObject
    {

    public:

        Humidity(void) : SensorObject("humidity", "%", 1) {}

    };

    class CO2 : public SensorObject
    {

    public:

        CO2(void) : SensorObject("co2", "ppm") {}

    };

    class VOC : public SensorObject
    {

    public:

        VOC(void) : SensorObject("voc", "ppb") {}

    };

    class Illuminance : public SensorObject
    {

    public:

        Illuminance(void) : SensorObject("illuminance", "lux") {}

    };

    class Energy : public SensorObject
    {

    public:

        Energy(void) : SensorObject("energy", "kW·h", 2) {}

    };

    class Voltage : public SensorObject
    {

    public:

        Voltage(void) : SensorObject("voltage", "V", 1) {}

    };

    class Current : public SensorObject
    {

    public:

        Current(void) : SensorObject("current", "A", 3) {}

    };

    class Power : public SensorObject
    {

    public:

        Power(void) : SensorObject("power", "W", 2) {}

    };
}

namespace Button
{
    class ResetCount : public ButtonObject
    {

    public:

        ResetCount(void) : ButtonObject("resetCount", "{\"count\":0}") {}

    };
}

#endif
