#include "device.h"

Cluster EndPointObject::cluster(quint16 clusterId)
{
    auto it = m_clusters.find(clusterId);

    if (it == m_clusters.end())
        it = m_clusters.insert(clusterId, Cluster(new ClusterObject));

    return it.value();
}

void DeviceObject::setProperties(void)
{
    if (m_vendor == "xiaomi" && m_model == "sensor")
    {
        m_fromDevice.append(Property(new Temperature));
        m_fromDevice.append(Property(new Humidity));
    }
}
