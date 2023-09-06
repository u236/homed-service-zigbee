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
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class SonoffButtonAction : public PropertyObject
    {

    public:

        SonoffButtonAction(void) : PropertyObject("action", CLUSTER_ON_OFF) {}
        void parseCommand(quint8 commandId, const QByteArray &payload) override;

    };

    class ModkamButtonAction : public PropertyObject
    {

    public:

        ModkamButtonAction(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class LifeControlAirQuality : public PropertyObject
    {

    public:

        LifeControlAirQuality(void) : PropertyObject("airQuality", CLUSTER_TEMPERATURE_MEASUREMENT) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class PerenioSmartPlug : public PropertyObject
    {

    public:

        PerenioSmartPlug(void) : PropertyObject("smartPlug", CLUSTER_PERENIO) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };

    class LmahmutovCO2 : public PropertyObject
    {

    public:

        LmahmutovCO2(void) : PropertyObject("co2", CLUSTER_LMAHMUTOV) {}
        void parseAttribte(quint16 attributeId, const QByteArray &data) override;

    };
}

#endif
