#ifndef ADAPTER_H
#define ADAPTER_H

#include <QSettings>

enum class LogicalType
{
    Coordinator,
    Router,
    EndDevice
};

class Adapter : public QObject
{
    Q_OBJECT

public:

    virtual ~Adapter(void) {}

    virtual void reset(void) = 0;
    virtual void registerEndpoint(quint8 endpointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters) = 0;
    virtual void setPermitJoin(bool enabled) = 0;
    virtual void nodeDescriptorRequest(quint16 networkAddress) = 0;
    virtual void simpleDescriptorRequest(quint16 networkAddress, quint8 endpointId) = 0;
    virtual void activeEndpointsRequest(quint16 networkAddress) = 0;
    virtual void lqiRequest(quint16 networkAddress, quint8 index = 0) = 0;

    virtual bool bindRequest(quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind = false) = 0;
    virtual bool dataRequest(quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &data) = 0;

    virtual bool extendedDataRequest(const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group = false) = 0;
    virtual bool extendedDataRequest(quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group = false) = 0;

    virtual quint8 dataRequestStatus(void) = 0;

    virtual bool setInterPanEndpointId(quint8 endpointId) = 0;
    virtual bool setInterPanChannel(quint8 channel) = 0;
    virtual void resetInterPan(void) = 0;

signals:

    void coordinatorReady(const QByteArray &ieeeAddress);
    void deviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress, quint8 capabilities);
    void deviceLeft(const QByteArray &ieeeAddress, quint16 networkAddress);
    void nodeDescriptorReceived(quint16 networkAddress, LogicalType logicalType, quint16 manufacturerCode);
    void activeEndpointsReceived(quint16 networkAddress, const QByteArray data);
    void simpleDescriptorReceived(quint16 networkAddress, quint8 endpointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters);
    void neighborRecordReceived(quint16 networkAddress, quint16 neighborAddress, quint8 linkQuality, bool first);
    void messageReveived(quint16 networkAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);
    void extendedMessageReveived(const QByteArray &ieeeAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);

};

#endif
