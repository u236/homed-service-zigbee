#ifndef PROPERTIES_HUE_H
#define PROPERTIES_HUE_H

#include "property.h"
#include "zcl.h"

namespace PropertiesHUE
{
    class IndicatorMode : public PropertyObject
    {

    public:

        IndicatorMode(void) : PropertyObject("indicatorMode", CLUSTER_BASIC) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };

    class SensitivityMode : public PropertyObject
    {

    public:

        SensitivityMode(void) : PropertyObject("sensitivityMode", CLUSTER_OCCUPANCY_SENSING) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };
}

#endif
