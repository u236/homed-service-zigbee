#include "ezsp.h"
#include "logger.h"

EZSP::EZSP(QSettings *config, QObject *parent) : Adapter(config, parent)
{
    logInfo << "Hello, there is EZSP adapter!";
}

void EZSP::reset(void)
{
}

void EZSP::registerEndpoint(quint8 endpointId, quint16 profileId, quint16 deviceId, const QList<quint16> &inClusters, const QList<quint16> &outClusters)
{
    Q_UNUSED(endpointId)
    Q_UNUSED(profileId)
    Q_UNUSED(deviceId)
    Q_UNUSED(inClusters)
    Q_UNUSED(outClusters)
}

void EZSP::setPermitJoin(bool enabled)
{
    Q_UNUSED(enabled)
}

void EZSP::nodeDescriptorRequest(quint16 networkAddress)
{
    Q_UNUSED(networkAddress)
}

void EZSP::simpleDescriptorRequest(quint16 networkAddress, quint8 endpointId)
{
    Q_UNUSED(networkAddress)
    Q_UNUSED(endpointId)
}

void EZSP::activeEndpointsRequest(quint16 networkAddress)
{
    Q_UNUSED(networkAddress)
}

void EZSP::lqiRequest(quint16 networkAddress, quint8 index)
{
    Q_UNUSED(networkAddress)
    Q_UNUSED(index)
}

bool EZSP::bindRequest(quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind)
{
    Q_UNUSED(networkAddress)
    Q_UNUSED(srcAddress)
    Q_UNUSED(srcEndpointId)
    Q_UNUSED(clusterId)
    Q_UNUSED(dstAddress)
    Q_UNUSED(dstEndpointId)
    Q_UNUSED(unbind)

    return true;
}

bool EZSP::dataRequest(quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &data)
{
    Q_UNUSED(networkAddress)
    Q_UNUSED(endpointId)
    Q_UNUSED(clusterId)
    Q_UNUSED(data)

    return true;
}

bool EZSP::extendedDataRequest(const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group)
{
    Q_UNUSED(address)
    Q_UNUSED(dstEndpointId)
    Q_UNUSED(dstPanId)
    Q_UNUSED(srcEndpointId)
    Q_UNUSED(clusterId)
    Q_UNUSED(data)
    Q_UNUSED(group)

    return true;
}

bool EZSP::extendedDataRequest(quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group)
{
    Q_UNUSED(address)
    Q_UNUSED(dstEndpointId)
    Q_UNUSED(dstPanId)
    Q_UNUSED(srcEndpointId)
    Q_UNUSED(clusterId)
    Q_UNUSED(data)
    Q_UNUSED(group)

    return true;
}

quint8 EZSP::dataRequestStatus(void)
{
    return 0x00;
}

bool EZSP::setInterPanEndpointId(quint8 endpointId)
{
    Q_UNUSED(endpointId)
    return true;
}

bool EZSP::setInterPanChannel(quint8 channel)
{
    Q_UNUSED(channel)
    return true;
}

void EZSP::resetInterPan(void)
{
}
