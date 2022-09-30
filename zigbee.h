#ifndef ZIGBEE_H
#define ZIGBEE_H

#define HANDLE_QUEUES_INTERVAL          1
#define UPDATE_NEIGHBORS_INTERVAL       3600000
#define STORE_STATUS_INTERVAL           60000
#define DEVICE_REJOIN_INTERVAL          10000

#define PROFILE_IPM                     0x0101 // Industrial Plant Monitoring
#define PROFILE_HA                      0x0104 // Home Automation
#define PROFILE_CBA                     0x0105 // Commercial Building Automation
#define PROFILE_TA                      0x0107 // Telecom Applications
#define PROFILE_PHHC                    0x0108 // Personal Home & Hospital Care
#define PROFILE_AMI                     0x0109 // Advanced Metering Initiative
#define PROFILE_HUE                     0xA1E0 // Philips HUE
#define PROFILE_ZLL                     0xC05E // ZigBee Light Link

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQueue>
#include "device.h"
#include "zstack.h"

class BindRequestObject;
typedef QSharedPointer <BindRequestObject> BindRequest;

class DataRequestObject;
typedef QSharedPointer <DataRequestObject> DataRequest;

class BindRequestObject
{

public:

    BindRequestObject(const Device &device, quint8 endpointId, quint16 clusterId) :
        m_device(device), m_endpointId(endpointId), m_clusterId(clusterId) {}

    inline Device device(void) {return m_device; }
    inline quint8 endpointId(void) {return m_endpointId; }
    inline quint16 clusterId(void) {return m_clusterId; }

    inline bool operator == (BindRequestObject &other) { return m_device == other.device() && m_endpointId == other.endpointId() && m_clusterId == other.clusterId(); }

private:

    Device m_device;
    quint8 m_endpointId;
    quint16 m_clusterId;

};

class DataRequestObject
{

public:

    DataRequestObject(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &data, const QString &name) :
        m_device(device), m_endpointId(endpointId), m_clusterId(clusterId), m_data(data), m_name(name) {}

    inline Device device(void) {return m_device; }
    inline quint8 endpointId(void) {return m_endpointId; }
    inline quint16 clusterId(void) {return m_clusterId; }
    inline QByteArray data(void) {return m_data; }
    inline QString name(void) {return m_name; }

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

    void otaUpgrade(const QByteArray &ieeeAddress, quint8 endpointId, const QString &fileName);
    void setDeviceName(const QByteArray &ieeeAddress, const QString &name);
    void removeDevice(const QByteArray &ieeeAddress);

    void updateDevice(const QByteArray &ieeeAddress, bool reportings);
    void updateReporting(const QByteArray &ieeeAddress, quint8 endpointId, const QString &reportingName, quint16 minInterval, quint16 maxInterval, quint16 valueChange);
    void deviceAction(const QByteArray &ieeeAddress, quint8 endpointId, const QString &actionName, const QVariant &actionData);

    void touchLinkRequest(const QByteArray &ieeeAddress = QByteArray(), quint8 channel = 11, bool reset = false);

private:

    ZStack *m_adapter;
    QTimer *m_neighborsTimer, *m_queuesTimer, *m_statusTimer, *m_ledTimer;

    QString m_libraryFile, m_databaseFile, m_otaUpgradeFile;
    qint16 m_ledPin;
    quint8 m_transactionId, m_interPanChannel;
    bool m_permitJoin;

    QMap <QByteArray, Device> m_devices;

    QQueue <BindRequest> m_bindQueue;
    QQueue <DataRequest> m_dataQueue;
    QQueue <Device> m_neighborsQueue;

    void unserializeDevices(const QJsonArray &devices);
    void unserializeEndpoints(const Device &device, const QJsonArray &endpoints);
    void unserializeNeighbors(const Device &device, const QJsonArray &neighbors);

    QJsonArray serializeDevices(void);
    QJsonArray serializeEndpoints(const Device &device);
    QJsonArray serializeNeighbors(const Device &device);

    Device findDevice(quint16 networkAddress);

    void enqueueBindRequest(const Device &device, quint8 endpointId, quint16 clusterId);
    void enqueueDataRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &data, const QString &name = QString());

    void setupDevice(const Device &device);
    void setupEndpoint(const Endpoint &endpoint, const QJsonObject &json);

    void interviewDevice(const Device &device);
    void configureReporting(const Endpoint &endpoint, const Reporting &reporting);

    void readAttributes(const Device &device, quint8 endpointId, quint16 clusterId, QList <quint16> attributes);
    void parseAttribute(const Endpoint &endpoint, quint16 clusterId, quint16 attributeId, quint8 dataType, const QByteArray &data);

    void clusterCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint8 commandId, const QByteArray &payload);
    void globalCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint8 commandId, QByteArray payload);

    void touchLinkReset(const QByteArray &ieeeAddress, quint8 channel);
    void touchLinkScan(void);

private slots:

    void coordinatorReady(const QByteArray &ieeeAddress);
    void deviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress, quint8 capabilities);
    void deviceLeft(const QByteArray &ieeeAddress, quint16 networkAddress);
    void nodeDescriptorReceived(quint16 networkAddress, LogicalType logicalType, quint16 manufacturerCode);
    void activeEndpointsReceived(quint16 networkAddress, const QByteArray data);
    void simpleDescriptorReceived(quint16 networkAddress, quint8 endpointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters);
    void neighborRecordReceived(quint16 networkAddress, quint16 neighborAddress, quint8 linkQuality, bool first);

    void messageReveived(quint16 networkAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);
    void messageReveivedExt(const QByteArray &ieeeAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);

    void pollAttributes(void);
    void updateNeighbors(void);
    void handleQueues(void);
    void storeStatus(void);
    void disableLed(void);

signals:

    void deviceEvent(bool join = true);
    void endpointUpdated(const Device &device, quint8 endpointId);
    void statusStored(const QJsonObject &json);

};

#endif
