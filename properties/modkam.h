#ifndef PROPERTIES_MODKAM_H
#define PROPERTIES_MODKAM_H

#include "property.h"
#include "zcl.h"

namespace PropertiesModkam
{
    class ButtonAction : public PropertyObject
    {

    public:

        ButtonAction(void) : PropertyObject("action", CLUSTER_MULTISTATE_INPUT) {}
        void parseAttribte(quint16 clusterId, quint16 attributeId, const QByteArray &data) override;

    };
}

#endif
