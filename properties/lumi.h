#ifndef PROPERTIES_LUMI_H
#define PROPERTIES_LUMI_H

#include "property.h"
#include "zcl.h"

namespace PropertiesLUMI
{
    class Data : public PropertyObject
    {

    public:

        Data(const quint16 cluster = CLUSTER_LUMI) : PropertyObject("lumiData", cluster) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;
        void resetValue(void) override;

    private:

        void parseData(quint16 dataPoint, const QByteArray &data, QMap <QString, QVariant> &map);

    };

    class Basic : public Data
    {

    public:

        Basic(void) : Data(CLUSTER_BASIC) {}

    };

    class ButtonMode : public PropertyObject
    {

    public:

        ButtonMode(void) : PropertyObject("buttonMode", CLUSTER_BASIC, {0xFF22, 0xFF23}) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Contact : public PropertyObject
    {

    public:

        Contact(void) : PropertyObject("contact", CLUSTER_ON_OFF, 0x0000) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Power : public PropertyObject
    {

    public:

        Power(void) : PropertyObject("power", CLUSTER_ANALOG_INPUT, 0x0055) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Cover : public PropertyObject
    {

    public:

        Cover(void) : PropertyObject("cover", CLUSTER_ANALOG_OUTPUT, 0x0055) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class ButtonAction : public PropertyObject
    {

    public:

        ButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF), m_check(false), m_hold(false) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;
        void resetValue(void) override;

    private:

        bool m_check, m_hold;

    };

    class SwitchAction : public PropertyObject
    {

    public:

        SwitchAction(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class DimmerAction : public PropertyObject
    {

    public:

        DimmerAction(void) : PropertyObject("action", CLUSTER_LUMI) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class CubeRotation : public PropertyObject
    {

    public:

        CubeRotation(void) : PropertyObject("action", CLUSTER_ANALOG_INPUT) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class CubeMovement : public PropertyObject
    {

    public:

        CubeMovement(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class Vibration : public PropertyObject
    {

    public:

        Vibration(void) : PropertyObject("vibration", CLUSTER_DOOR_LOCK, {0x0055, 0x0503, 0x0505, 0x0508}) {}
        void parseAttribute(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;
        void resetValue(void) override;

    };
}

#endif
