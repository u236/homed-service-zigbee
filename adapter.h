#ifndef ADAPTER_H
#define ADAPTER_H

#define SOCKET_RECONNECT_INTERVAL       5000
#define DEVICE_RECEIVE_TIMEOUT          20

#define ADAPTER_RESET_DELAY             100
#define ADAPTER_RESET_TIMEOUT           10000

#define PERMIT_JOIN_TIMEOUT             60000
#define NETWORK_REQUEST_TIMEOUT         10000

#define PROFILE_IPM                     0x0101 // Industrial Plant Monitoring
#define PROFILE_HA                      0x0104 // Home Automation
#define PROFILE_CBA                     0x0105 // Commercial Building Automation
#define PROFILE_TA                      0x0107 // Telecom Applications
#define PROFILE_PHHC                    0x0108 // Personal Home & Hospital Care
#define PROFILE_AMI                     0x0109 // Advanced Metering Initiative
#define PROFILE_GP                      0xA1E0 // ZigBee Green Power
#define PROFILE_ZLL                     0xC05E // ZigBee Light Link

#define APS_NODE_DESCRIPTOR             0x0002
#define APS_SIMPLE_DESCRIPTOR           0x0004
#define APS_ACTIVE_ENDPOINTS            0x0005
#define APS_DEVICE_ANNOUNCE             0x0013
#define APS_BIND                        0x0021
#define APS_UNBIND                      0x0022
#define APS_LQI                         0x0031
#define APS_LEAVE                       0x0034

#define ADDRESS_MODE_NOT_PRESENT        0x00
#define ADDRESS_MODE_GROUP              0x01
#define ADDRESS_MODE_16_BIT             0x02
#define ADDRESS_MODE_64_BIT             0x03
#define ADDRESS_MODE_BROADCAST          0xFF

#include <QHostAddress>
#include <QQueue>
#include <QSerialPort>
#include <QSettings>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QTimer>

enum class LogicalType
{
    Coordinator,
    Router,
    EndDevice
};

#pragma pack(push, 1)

struct deviceAnnounceStruct
{
    quint16 networkAddress;
    quint64 ieeeAddress;
    quint8  capabilities;
};

struct nodeDescriptorResponseStruct
{
    quint8  status;
    quint16 networkAddress;
    quint8  logicalType;
    quint8  apsFlags;
    quint8  capabilityFlags;
    quint16 manufacturerCode;
    quint8  maxBufferSize;
    quint16 maxTransferSize;
    quint16 serverFlags;
    quint16 maxOutTransferSize;
    quint8  descriptorCapabilities;
};

struct simpleDescriptorResponseStruct
{
    quint8  status;
    quint16 networkAddress;
    quint8  length;
    quint8  endpointId;
    quint16 profileId;
    quint16 deviceId;
    quint8  version;
};

struct activeEndpointsResponseStruct
{
    quint8  status;
    quint16 networkAddress;
    quint8  count;
};

struct lqiResponseStruct
{
    quint8  status;
    quint8  total;
    quint8  index;
    quint8  count;
};

struct neighborRecordStruct
{
    quint64 extendedPanId;
    quint64 ieeeAddress;
    quint16 networkAddress;
    quint8  options;
    quint8  permitJoining;
    quint8  depth;
    quint8  linkQuality;
};

struct bindRequestStruct
{
    quint64 srcAddress;
    quint8  srcEndpointId;
    quint16 clusterId;
    quint8  dstAddressMode;
};

#pragma pack(pop)

class EndpointDataObject;
typedef QSharedPointer <EndpointDataObject> EndpointData;

class EndpointDataObject : public QObject
{
    Q_OBJECT

public:

    EndpointDataObject(quint16 profileId = 0, quint16 deviceId = 0) :
        QObject(nullptr), m_profileId(profileId), m_deviceId(deviceId) {}

    inline quint16 profileId(void) { return m_profileId; }
    inline void setProfileId(quint16 value) { m_profileId = value; }

    inline quint16 deviceId(void) { return m_deviceId; }
    inline void setDeviceId(quint16 value) { m_deviceId = value; }

    inline QList <quint16> &inClusters(void) { return m_inClusters; }
    inline QList <quint16> &outClusters(void) { return m_outClusters; }

private:

    quint16 m_profileId, m_deviceId;
    QList <quint16> m_inClusters, m_outClusters;

};

class Adapter : public QObject
{
    Q_OBJECT

public:

    Adapter(QSettings *config, QObject *parent);
    ~Adapter(void);

    virtual bool extendedDataRequest(quint8 id, const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group = false) = 0;
    virtual bool extendedDataRequest(quint8 id, quint16 networkAddress, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group = false) = 0;

    virtual bool setInterPanEndpointId(quint8 endpointId) = 0;
    virtual bool setInterPanChannel(quint8 channel) = 0;
    virtual void resetInterPan(void) = 0;

    inline QString type(void) { return m_typeString; }
    inline QString version(void) { return m_versionString; }
    inline quint64 ieeeAddress(void) { return m_ieeeAddress; }

    void init(void);
    void setPermitJoin(bool enabled);

    virtual bool nodeDescriptorRequest(quint8 id, quint16 networkAddress);
    virtual bool simpleDescriptorRequest(quint8 id, quint16 networkAddress, quint8 endpointId);
    virtual bool activeEndpointsRequest(quint8 id, quint16 networkAddress);
    virtual bool bindRequest(quint8 id, quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind = false);
    virtual bool lqiRequest(quint8 id, quint16 networkAddress, quint8 index = 0);
    virtual bool leaveRequest(quint8 id, quint16 networkAddress, const QByteArray &ieeeAddress);
    virtual bool dataRequest(quint8 id, quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &data);

protected:

    QSerialPort *m_serial;
    QTcpSocket *m_socket;
    QIODevice *m_device;

    QTimer *m_socketTimer, *m_receiveTimer, *m_resetTimer, *m_permitJoinTimer;

    QHostAddress m_adddress;
    quint16 m_port;
    bool m_connected;

    QString m_bootPin, m_resetPin, m_reset;
    quint16 m_panId;
    quint8 m_channel;
    bool m_write, m_debug;

    QString m_typeString, m_versionString;
    quint64 m_ieeeAddress;

    bool m_permitJoin;

    QQueue <QByteArray> m_queue;
    QMap <quint8, EndpointData> m_endpointsData;

    void reset(void);
    bool waitForSignal(const QObject *sender, const char *signal, int tiomeout);
    void parseMessage(quint16 networkAddress, quint16 clusterId, const QByteArray &payload);

private:

    virtual void softReset(void) = 0;
    virtual void parseData(void) = 0;
    virtual bool permitJoin(bool enabled) = 0;
    virtual bool unicastRequest(quint8 id, quint16 networkAddress, quint16 clusterId, quint8 srcEndPointId, quint8 dstEndPointId, const QByteArray &payload) = 0;

private slots:

    virtual void handleQueue(void) = 0;

    void socketConnected(void);
    void socketError(QTcpSocket::SocketError error);
    void socketReconnect(void);

    void startTimer(void);
    void readyRead(void);
    void resetTimeout(void);
    void permitJoinTimeout(void);

signals:

    void adapterReset(void);
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

};

#endif
