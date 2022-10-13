#ifndef EZSP_H
#define EZSP_H

#include "adapter.h"

class EZSP : public Adapter
{
    Q_OBJECT

public:

    EZSP(QSettings *config, QObject *parent);

    void reset(void) override;
    void registerEndpoint(quint8 endpointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters) override;
    void setPermitJoin(bool enabled) override;
    void nodeDescriptorRequest(quint16 networkAddress) override;
    void simpleDescriptorRequest(quint16 networkAddress, quint8 endpointId) override;
    void activeEndpointsRequest(quint16 networkAddress) override;
    void lqiRequest(quint16 networkAddress, quint8 index = 0) override;

    bool bindRequest(quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind = false) override;
    bool dataRequest(quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &data) override;

    bool extendedDataRequest(const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group = false) override;
    bool extendedDataRequest(quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group = false) override;

    quint8 dataRequestStatus(void) override;

    bool setInterPanEndpointId(quint8 endpointId) override;
    bool setInterPanChannel(quint8 channel) override;
    void resetInterPan(void) override;

private:

    quint8 m_channel;
    qint16 m_bootPin, m_resetPin;

    QString m_reset;
    bool m_debug, m_write, m_clear;

private slots:

    void receiveData(void) override;

};

#endif
