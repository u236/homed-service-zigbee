#ifndef ADAPTER_H
#define ADAPTER_H

#define RECEIVE_TIMEOUT                 20

#define PERMIT_JOIN_TIMEOUT             60000
#define PERMIT_JOIN_BROARCAST_ADDRESS   0xFFFC

#define RESET_TIMEOUT                   15000
#define RESET_DELAY                     100

#define DEFAULT_GROUP                   0x0000
#define IKEA_GROUP                      0x0385
#define GREEN_POWER_GROUP               0x0B84

#define PROFILE_IPM                     0x0101 // Industrial Plant Monitoring
#define PROFILE_HA                      0x0104 // Home Automation
#define PROFILE_CBA                     0x0105 // Commercial Building Automation
#define PROFILE_TA                      0x0107 // Telecom Applications
#define PROFILE_PHHC                    0x0108 // Personal Home & Hospital Care
#define PROFILE_AMI                     0x0109 // Advanced Metering Initiative
#define PROFILE_GP                      0xA1E0 // ZigBee Green Power
#define PROFILE_ZLL                     0xC05E // ZigBee Light Link

#define ZDO_DEVICE_ANNOUNCE             0x0013
#define ZDO_NODE_DESCRIPTOR_REQUEST     0x0002
#define ZDO_SIMPLE_DESCRIPTOR_REQUEST   0x0004
#define ZDO_ACTIVE_ENDPOINTS_REQUEST    0x0005
#define ZDO_BIND_REQUEST                0x0021
#define ZDO_UNBIND_REQUEST              0x0022
#define ZDO_LQI_REQUEST                 0x0031
#define ZDO_LEAVE_REQUEST               0x0034

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
    quint8  permitJoin;
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

class EndpointDataObject
{

public:

    EndpointDataObject(quint16 profileId = 0, quint16 deviceId = 0) :
        m_profileId(profileId), m_deviceId(deviceId) {}

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

    virtual bool unicastRequest(quint8 id, quint16 networkAddress, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload) = 0;
    virtual bool multicastRequest(quint8 id, quint16 groupId, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload) = 0;

    virtual bool unicastInterPanRequest(quint8 id, const QByteArray &ieeeAddress, quint16 clusterId, const QByteArray &payload) = 0;
    virtual bool broadcastInterPanRequest(quint8 id, quint16 clusterId, const QByteArray &payload) = 0;

    virtual bool setInterPanChannel(quint8 channel) = 0;
    virtual void resetInterPanChannel(void) = 0;

    inline QString manufacturerName(void) { return m_manufacturerName; }
    inline QString modelName(void) { return m_modelName; }
    inline QString firmware(void) { return m_firmware; }

    inline QByteArray ieeeAddress(void) { return m_ieeeAddress; }
    inline quint8 replyStatus(void) { return m_replyStatus; }

    inline void setPermitJoinAddress(quint16 value) { m_permitJoinAddress = value; }
    inline void setRequestParameters(const QByteArray &value, bool extendedTimeout = true) { m_requestAddress = value; m_extendedTimeout = extendedTimeout; }

    void init(void);
    bool waitForSignal(const QObject *sender, const char *signal, int tiomeout);

    void setPermitJoin(bool enabled);
    void togglePermitJoin(void);

    virtual bool zdoRequest(quint8 id, quint16 networkAddress, quint16 clusterId, const QByteArray &data = QByteArray());
    virtual bool bindRequest(quint8 id, quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &address, quint8 dstEndpointId, bool unbind = false);
    virtual bool leaveRequest(quint8 id, quint16 networkAddress);
    virtual bool lqiRequest(quint8 id, quint16 networkAddress, quint8 index);

protected:

    QTimer *m_receiveTimer, *m_resetTimer, *m_permitJoinTimer;

    QSerialPort *m_serial;
    QTcpSocket *m_socket;
    QIODevice *m_device;

    bool m_serialError;

    QHostAddress m_adddress;
    quint16 m_port;
    bool m_connected;

    QString m_bootPin, m_resetPin, m_reset;
    quint16 m_panId;
    quint8 m_channel, m_power;
    bool m_write, m_portDebug, m_adapterDebug;

    QString m_manufacturerName, m_modelName, m_firmware;
    QByteArray m_networkKey, m_defaultKey, m_ieeeAddress;

    quint16 m_permitJoinAddress;
    bool m_permitJoin;

    QByteArray m_requestAddress;
    bool m_extendedTimeout;

    quint8 m_replyStatus;

    QMap <quint8, EndpointData> m_endpoints;
    QList <quint16> m_multicast;
    QQueue <QByteArray> m_queue;

    void reset(void);
    void sendData(const QByteArray &buffer);

private:

    virtual void softReset(void) = 0;
    virtual void parseData(QByteArray &buffer) = 0;
    virtual bool permitJoin(bool enabled) = 0;

protected slots:

    virtual void handleQueue(void) = 0;
    virtual void serialError(QSerialPort::SerialPortError error);

private slots:

    void socketError(QTcpSocket::SocketError error);
    void socketConnected(void);

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
    void zdoMessageReveived(quint16 networkAddress, quint16 clusterId, const QByteArray &payload);
    void zclMessageReveived(quint16 networkAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);
    void rawMessageReveived(const QByteArray &ieeeAddress, quint16 clusterId, quint8 linkQuality, const QByteArray &data);

};

#endif
