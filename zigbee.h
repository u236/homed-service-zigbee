#ifndef ZIGBEE_H
#define ZIGBEE_H

#define HANDLE_QUEUES_INTERVAL          1
#define UPDATE_NEIGHBORS_INTERVAL       3600000
#define DEVICE_INTERVIEW_TIMEOUT        15000
#define STATUS_LED_TIMEOUT              200

#include "device.h"

class BindRequestObject;
typedef QSharedPointer <BindRequestObject> BindRequest;

class DataRequestObject;
typedef QSharedPointer <DataRequestObject> DataRequest;

class BindRequestObject
{

public:

    BindRequestObject(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind) :
        m_device(device), m_endpointId(endpointId), m_clusterId(clusterId), m_dstAddress(dstAddress), m_dstEndpointId(dstEndpointId), m_unbind(unbind) {}

    inline Device device(void) { return m_device; }
    inline quint8 endpointId(void) { return m_endpointId; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline QByteArray dstAddress(void) { return m_dstAddress; }
    inline quint8 dstEndpointId(void) { return m_dstEndpointId; }
    inline bool unbind(void) { return m_unbind; }

    inline bool operator == (BindRequestObject &other) { return m_device == other.device() && m_endpointId == other.endpointId() && m_clusterId == other.clusterId() && m_dstAddress == other.dstAddress() && m_dstEndpointId == other.dstEndpointId() && m_unbind == other.unbind(); }

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

class ZigBee : public QObject
{
    Q_OBJECT

public:

    ZigBee(QSettings *config, QObject *parent);

    void init(void);
    void setPermitJoin(bool enabled);

    void setDeviceName(const QString &deviceName, const QString &newName, bool store = true);
    void removeDevice(const QString &deviceName, bool force);

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
    QTimer *m_neighborsTimer, *m_queuesTimer, *m_statusLedTimer;

    DeviceList *m_devices;
    Adapter *m_adapter;

    QString m_statusLedPin, m_blinkLedPin, m_libraryFile, m_otaUpgradeFile;
    quint8 m_transactionId, m_interPanChannel;

    QQueue <BindRequest> m_bindQueue;
    QQueue <DataRequest> m_dataQueue;
    QQueue <Device> m_interviewQueue, m_neighborsQueue, m_removeQueue;

    Endpoint getEndpoint(const Device &device, quint8 endpointId);

    void enqueueBindRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &dstAddress = QByteArray(), quint8 dstEndpointId = 0, bool unbind = false);
    void enqueueDataRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &data, const QString &name = QString());

    void setupDevice(const Device &device);
    void setupEndpoint(const Endpoint &endpoint, const QJsonObject &json);

    void interviewDevice(const Device &device);
    void interviewRequest(const Device &device);
    void interviewFinished(const Device &device);
    void interviewError(const Device &device, const QString &reason);

    void configureReporting(const Endpoint &endpoint, const Reporting &reporting);

    bool readAttributes(const Device &device, quint8 endpointId, quint16 clusterId, QList <quint16> attributes, bool enqueue = true);
    void parseAttribute(const Endpoint &endpoint, quint16 clusterId, quint16 attributeId, quint8 dataType, const QByteArray &data);

    void clusterCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint8 commandId, const QByteArray &payload);
    void globalCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint8 commandId, QByteArray payload);

    void touchLinkReset(const QByteArray &ieeeAddress, quint8 channel);
    void touchLinkScan(void);

    void blink(quint16 timeout);

private slots:

    void coordinatorReady(void);
    void permitJoinUpdated(bool enabled);

    void deviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress);
    void deviceLeft(const QByteArray &ieeeAddress);
    void nodeDescriptorReceived(quint16 networkAddress, LogicalType logicalType, quint16 manufacturerCode);
    void activeEndpointsReceived(quint16 networkAddress, const QByteArray data);
    void simpleDescriptorReceived(quint16 networkAddress, quint8 endpointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters);
    void neighborRecordReceived(quint16 networkAddress, quint16 neighborAddress, quint8 linkQuality, bool start);
    void messageReveived(quint16 networkAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);
    void extendedMessageReveived(const QByteArray &ieeeAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);

    void interviewTimeout(void);
    void pollAttributes(void);
    void updateNeighbors(void);
    void handleQueue(void);

    void updateStatusLed(void);
    void updateBlinkLed(void);

signals:

    void deviceEvent(const Device &device, const QString &event);
    void endpointUpdated(const Device &device, quint8 endpointId);
    void statusStored(const QJsonObject &json);

};

#endif
