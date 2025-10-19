#ifndef PROPERTIES_PTVO_H
#define PROPERTIES_PTVO_H

#include "property.h"
#include "zcl.h"

namespace PropertiesPTVO
{
    class Status : public PropertyObject
    {

    public:

        Status(const QString &name) : PropertyObject(name, CLUSTER_ON_OFF, 0x0000) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class AnalogInput: public PropertyObject
    {

    public:

        AnalogInput(const QString &name, const QString &id = QString()) : PropertyObject(name, CLUSTER_ANALOG_INPUT, {0x0055, 0x001C}), m_id(id) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    private:

        QString m_id;
        QVariant m_buffer;

        bool multiple(void);

    };

    class ChangePattern : public Status
    {

    public:

        ChangePattern(void) : Status("changePattern") {}

    };

    class WaterLeak : public Status
    {

    public:

        WaterLeak(void) : Status("waterLeak") {}

    };

    class CO2 : public AnalogInput
    {

    public:

        CO2(void) : AnalogInput("co2", "ppm") {}

    };

    class Temperature : public AnalogInput
    {

    public:

        Temperature(void) : AnalogInput("temperature", "C") {}

    };

    class Count: public AnalogInput
    {

    public:

        Count(void) : AnalogInput("count") {}

    };

    class Pattern: public AnalogInput
    {

    public:

        Pattern(void) : AnalogInput("pattern") {}

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class SerialData : public PropertyObject
    {

    public:

        SerialData(void) : PropertyObject("data", CLUSTER_MULTISTATE_VALUE) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class ButtonAction : public EnumProperty
    {

    public:

        ButtonAction(void) : EnumProperty("action", CLUSTER_MULTISTATE_INPUT, 0x0055) {}

    };
}

#endif
