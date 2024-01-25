#ifndef PROPERTIES_OTHER_H
#define PROPERTIES_OTHER_H

#include "property.h"
#include "zcl.h"

namespace PropertiesOther
{
    class KonkeButtonAction : public PropertyObject
    {

    public:

        KonkeButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class SonoffButtonAction : public PropertyObject
    {

    public:

        SonoffButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF) {}
        void parseCommand(quint16 clusterId, quint8 commandId, const QByteArray &payload) override;

    };

    class LifeControlAirQuality : public PropertyObject
    {

    public:

        LifeControlAirQuality(void) : PropertyObject("airQuality", CLUSTER_TEMPERATURE_MEASUREMENT) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class PerenioSmartPlug : public PropertyObject
    {

    public:

        PerenioSmartPlug(void) : PropertyObject("smartPlug", CLUSTER_PERENIO) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class WoolleySmartPlug : public PropertyObject
    {

    public:

        WoolleySmartPlug(void) : PropertyObject("smartPlug", CLUSTER_WOOLLEY) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class WaterMeterVolume : public PropertyObject
    {

    public:

        WaterMeterVolume(void) : PropertyObject("volume", CLUSTER_SMART_ENERGY_METERING) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class WaterMeterSettings : public PropertyObject
    {

    public:

        WaterMeterSettings(void) : PropertyObject("waterMeterSettings", CLUSTER_SMART_ENERGY_METERING) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };
}

#endif
