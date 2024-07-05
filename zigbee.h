#ifndef ZIGBEE_H
#define ZIGBEE_H

#define UPDATE_NEIGHBORS_INTERVAL       3600000
#define PING_DEVICES_INTERVAL           300000
#define NETWORK_REQUEST_TIMEOUT         10000
#define DEVICE_REJOIN_TIMEOUT           5000
#define DEVICE_INTERVIEW_TIMEOUT        10000
#define INTER_PAN_CHANNEL_TIMEOUT       100
#define STATUS_LED_TIMEOUT              500

#define TIME_OFFSET                     946684800
#define IAS_ZONE_ID                     0x42

#include <QMetaEnum>
#include "device.h"

class DataRequestObject;
typedef QSharedPointer <DataRequestObject> DataRequest;

class RequestObject;
typedef QSharedPointer <RequestObject> Request;

enum class RequestType
{
    Data,
    Leave,
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

class DataRequestObject
{

public:

    DataRequestObject(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &data, const QString &name, bool debug, quint16 manufacturerCode, const Action &action) :
        m_device(device), m_endpointId(endpointId), m_clusterId(clusterId), m_data(data), m_name(name), m_debug(debug), m_manufacturerCode(manufacturerCode), m_action(action) {}

    inline Device device(void) { return m_device; }
    inline quint8 endpointId(void) { return m_endpointId; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QByteArray data(void) { return m_data; }

    inline QString name(void) { return m_name; }
    inline bool debug(void) { return m_debug; }

    inline quint16 manufacturerCode(void) { return m_manufacturerCode; }
    inline Action &action(void) { return m_action; }

private:

    Device m_device;
    quint8 m_endpointId;
    quint16 m_clusterId;
    QByteArray m_data;

    QString m_name;
    bool m_debug;

    quint16 m_manufacturerCode;
    Action m_action;

};

class RequestObject
{

public:

    RequestObject(const QVariant &data, RequestType type) :
        m_data(data), m_type(type), m_status(RequestStatus::Pending) {}

    inline QVariant data(void) { return m_data; }
    inline RequestType type(void) { return m_type; }

    inline RequestStatus status(void) { return m_status; }
    inline void setStatus(RequestStatus value) { m_status = value; }

private:

    QVariant m_data;
    RequestType m_type;
    RequestStatus m_status;

};

class ZigBee : public QObject
{
    Q_OBJECT

public:

    ZigBee(QSettings *config, QObject *parent);
    ~ZigBee(void);

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
        interviewTimeout,
        clusterRequest,
        globalRequest,
        requestFinished
    };

    Q_ENUM(Event)

    inline DeviceList *devices(void) { return m_devices; }
    inline const char *eventName(Event event) { return m_events.valueToKey(static_cast <int> (event)); }

    void init(void);
    void setPermitJoin(bool enabled);
    void togglePermitJoin(void);

    void updateDevice(const QString &deviceName, const QString &name, const QString &note, bool active, bool discovery, bool cloud);
    void removeDevice(const QString &deviceName, bool force);
    
    void setupDevice(const QString &deviceName, bool reportings);
    void setupReporting(const QString &deviceName, quint8 endpointId, const QString &reportingName, quint16 minInterval, quint16 maxInterval, quint16 valueChange);

    void bindingControl(const QString &deviceName, quint8 endpointId, quint16 clusterId, const QVariant &dstAddress, quint8 dstEndpointId, bool unbind);
    void groupControl(const QString &deviceName, quint8 endpointId, quint16 groupId, bool remove);
    void removeAllGroups(const QString &deviceName, quint8 endpointId);

    void otaUpgrade(const QString &deviceName, quint8 endpointId, const QString &fileName, bool force);
    void getProperties(const QString &deviceName);

    void clusterRequest(const QString &deviceName, quint8 endpointId, quint16 clusterId, quint16 manufacturerCode, quint8 commandId, const QByteArray &payload, bool global);
    void touchLinkRequest(const QByteArray &ieeeAddress = QByteArray(), quint8 channel = 11, bool reset = false);

    void deviceAction(const QString &deviceName, quint8 endpointId, const QString &name, const QVariant &data);
    void groupAction(quint16 groupId, const QString &name, const QVariant &data);

private:

    QSettings *m_config;
    QTimer *m_requestTimer, *m_neignborsTimer, *m_pingTimer, *m_statusLedTimer;

    Adapter *m_adapter;
    DeviceList *m_devices;

    QMetaEnum m_events;
    quint8 m_requestId, m_requestStatus, m_replyId, m_interPanChannel;
    bool m_replyReceived, m_interPanLock;

    QString m_statusLedPin, m_blinkLedPin;
    bool m_debounce, m_discovery, m_cloud, m_debug;

    Device m_otaDevice;
    QFile m_otaFile;
    bool m_otaForce;

    QMap <quint8, Request> m_requests;

    void enqueueRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &data, const QString &name = QString(), bool debug = false, quint16 manufacturerCode = 0, const Action &action = Action());
    void enqueueRequest(const Device &device, RequestType type);

    bool interviewRequest(quint8 id, const Device &device);
    bool interviewQuirks(const Device &device);
    void interviewDevice(const Device &device);
    void interviewFinished(const Device &device);
    void interviewError(const Device &device, const QString &reason);

    bool bindRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &address = QByteArray(), quint8 dstEndpointId = 0, bool unbind = false);
    bool configureReporting(const Device &device, quint8 endpointId, const Reporting &reporting);
    bool configureDevice(const Device &device);
    bool parseProperty(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint16 itemId, const QByteArray &data, bool command = false);

    void parseAttribute(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint16 attributeId, quint8 dataType, const QByteArray &data);
    void clusterCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint16 manufacturerCode, quint8 transactionId, quint8 commandId, const QByteArray &payload);
    void globalCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint16 manufacturerCode, quint8 transactionId, quint8 commandId, QByteArray payload);

    void touchLinkReset(const QByteArray &ieeeAddress, quint8 channel);
    void touchLinkScan(void);

    void interviewTimeoutHandler(const Device &device);
    void rejoinHandler(const Device &device);

    void otaError(const Endpoint &endpoint, quint16 manufacturerCode, quint8 transactionId, quint8 commandId, const QString &error = QString());
    void blink(quint16 timeout);

private slots:

    void adapterReset(void);
    void coordinatorReady(void);
    void permitJoinUpdated(bool enabled);

    void deviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress);
    void deviceLeft(const QByteArray &ieeeAddress);
    void zdoMessageReveived(quint16 networkAddress, quint16 clusterId, const QByteArray &payload);
    void zclMessageReveived(quint16 networkAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &payload);
    void rawMessageReveived(const QByteArray &ieeeAddress, quint16 clusterId, quint8 linkQuality, const QByteArray &data);
    void requestFinished(quint8 id, quint8 status);

    void handleRequests(void);
    void updateNeighbors(void);
    void pingDevices(void);
    void interviewTimeout(void);

    void pollRequest(EndpointObject *endpoint, const Poll &poll);

    void updateStatusLed(void);
    void updateBlinkLed(void);

signals:

    void networkStarted(void);
    void deviceEvent(DeviceObject *device, ZigBee::Event event, const QJsonObject &json = QJsonObject());
    void endpointUpdated(DeviceObject *device, quint8 endpointId);
    void statusUpdated(const QJsonObject &json);
    void replyReceived(void);

};

Q_DECLARE_METATYPE(DataRequest)

#endif
