#ifndef ZIGBEE_H
#define ZIGBEE_H

#define UPDATE_NEIGHBORS_INTERVAL       3600000
#define PING_DEVICES_INTERVAL           300000
#define DEVICE_INTERVIEW_TIMEOUT        15000
#define STATUS_LED_TIMEOUT              500

#include <QMetaEnum>
#include "device.h"

class BindingRequestObject;
typedef QSharedPointer <BindingRequestObject> BindingRequest;

class DataRequestObject;
typedef QSharedPointer <DataRequestObject> DataRequest;

class RequestObject;
typedef QSharedPointer <RequestObject> Request;

enum class RequestType
{
    Binding,
    Data,
    Remove,
    LQI,
    Interview
};

enum class RequestStatus
{
    Pending,
    Sent,
    Finished,
    Aborted
};

class BindingRequestObject
{

public:

    BindingRequestObject(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind) :
        m_device(device), m_endpointId(endpointId), m_clusterId(clusterId), m_dstAddress(dstAddress), m_dstEndpointId(dstEndpointId), m_unbind(unbind) {}

    inline Device device(void) { return m_device; }
    inline quint8 endpointId(void) { return m_endpointId; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QByteArray dstAddress(void) { return m_dstAddress; }
    inline quint8 dstEndpointId(void) { return m_dstEndpointId; }
    inline bool unbind(void) { return m_unbind; }

    inline bool operator == (BindingRequestObject &other) { return m_device == other.device() && m_endpointId == other.endpointId() && m_clusterId == other.clusterId() && m_dstAddress == other.dstAddress() && m_dstEndpointId == other.dstEndpointId() && m_unbind == other.unbind(); }

private:

    Device m_device;
    quint8 m_endpointId;
    quint16 m_clusterId;
    QByteArray m_dstAddress;
    quint8 m_dstEndpointId;
    bool m_unbind;

};

class DataRequestObject
{

public:

    DataRequestObject(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &data, const QString &name) :
        m_device(device), m_endpointId(endpointId), m_clusterId(clusterId), m_data(data), m_name(name) {}

    inline Device device(void) { return m_device; }
    inline quint8 endpointId(void) { return m_endpointId; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QByteArray data(void) { return m_data; }
    inline QString name(void) { return m_name; }

    inline bool operator == (DataRequestObject &other) { return m_device == other.device() && m_endpointId == other.endpointId() && m_clusterId == other.clusterId() && m_data == other.data(); }

private:

    Device m_device;
    quint8 m_endpointId;
    quint16 m_clusterId;
    QByteArray m_data;
    QString m_name;

};

class RequestObject
{

public:

    RequestObject(const QVariant &request, RequestType type) :
        m_request(request), m_type(type), m_status(RequestStatus::Pending) {}

    inline QVariant request(void) { return m_request; }
    inline RequestType type(void) { return m_type; }

    inline RequestStatus status(void) { return m_status; }
    inline void setStatus(RequestStatus value) { m_status = value; }

private:

    QVariant m_request;
    RequestType m_type;
    RequestStatus m_status;

};

class ZigBee : public QObject
{
    Q_OBJECT

public:

    ZigBee(QSettings *config, QObject *parent);

    enum class Event
    {
        deviceJoined,
        deviceLeft,
        deviceRemoved,
        deviceNameDuplicate,
        deviceAboutToRename,
        deviceUpdated,
        interviewFinished,
        interviewError,
        interviewTimeout
    };

    Q_ENUM(Event)

    inline DeviceList *devices(void) { return m_devices; }
    inline const char *eventName(Event event) { return m_events.valueToKey(static_cast <int> (event)); }

    void init(void);
    void setPermitJoin(bool enabled);

    void removeDevice(const QString &deviceName, bool force);
    void setDeviceName(const QString &deviceName, const QString &name);

    void updateDevice(const QString &deviceName, bool reportings);
    void updateReporting(const QString &deviceName, quint8 endpointId, const QString &reportingName, quint16 minInterval, quint16 maxInterval, quint16 valueChange);

    void bindingControl(const QString &deviceName, quint8 endpointId, quint16 clusterId, const QVariant &dstAddress, quint8 dstEndpointId, bool unbind);
    void groupControl(const QString &deviceName, quint8 endpointId, quint16 groupId, bool remove);
    void removeAllGroups(const QString &deviceName, quint8 endpointId);
    void otaUpgrade(const QString &deviceName, quint8 endpointId, const QString &fileName);

    void touchLinkRequest(const QByteArray &ieeeAddress = QByteArray(), quint8 channel = 11, bool reset = false);

    void deviceAction(const QString &deviceName, quint8 endpointId, const QString &actionName, const QVariant &actionData);
    void groupAction(quint16 groupId, const QString &actionName, const QVariant &actionData);

private:

    QSettings *m_config;
    QTimer *m_requestTimer, *m_neignborsTimer, *m_pingTimer, *m_statusLedTimer;

    DeviceList *m_devices;
    Adapter *m_adapter;

    QMetaEnum m_events;
    quint8 m_requestId, m_interPanChannel;

    QString m_statusLedPin, m_blinkLedPin, m_otaUpgradeFile;
    bool m_debug;

    QMap <quint8, Request> m_requests;

    void enqueueBindingRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &dstAddress = QByteArray(), quint8 dstEndpointId = 0, bool unbind = false);
    void enqueueDataRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &data, const QString &name = QString());
    void enqueueDeviceRequest(const Device &device, RequestType type);

    bool interviewRequest(quint8 id, const Device &device);

    void interviewDevice(const Device &device);
    void interviewFinished(const Device &device);
    void interviewError(const Device &device, const QString &reason);

    void configureReporting(const Endpoint &endpoint, const Reporting &reporting);
    void parseAttribute(const Endpoint &endpoint, quint16 clusterId, quint16 attributeId, quint8 dataType, const QByteArray &data);

    void clusterCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint8 commandId, const QByteArray &payload);
    void globalCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint8 commandId, QByteArray payload);

    void touchLinkReset(const QByteArray &ieeeAddress, quint8 channel);
    void touchLinkScan(void);

    void startDeviceTimer(const Device &device, quint32 timeout);
    void blink(quint16 timeout);

private slots:

    void coordinatorReady(void);
    void permitJoinUpdated(bool enabled);
    void requestFinished(quint8 id, quint8 status);

    void deviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress);
    void deviceLeft(const QByteArray &ieeeAddress);
    void nodeDescriptorReceived(quint16 networkAddress, LogicalType logicalType, quint16 manufacturerCode);
    void activeEndpointsReceived(quint16 networkAddress, const QByteArray data);
    void simpleDescriptorReceived(quint16 networkAddress, quint8 endpointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters);
    void neighborRecordReceived(quint16 networkAddress, quint16 neighborAddress, quint8 linkQuality, bool start);
    void messageReveived(quint16 networkAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);
    void extendedMessageReveived(const QByteArray &ieeeAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);

    void handleRequests(void);
    void updateNeighbors(void);
    void pingDevices(void);
    void deviceTimeout(void);

    void pollRequest(EndpointObject *endpoint, const Poll &poll);

    void updateStatusLed(void);
    void updateBlinkLed(void);

signals:

    void deviceEvent(const Device &device, ZigBee::Event event);
    void endpointUpdated(const Device &device, quint8 endpointId);
    void statusUpdated(const QJsonObject &json);

};

Q_DECLARE_METATYPE(BindingRequest)
Q_DECLARE_METATYPE(DataRequest)

#endif
