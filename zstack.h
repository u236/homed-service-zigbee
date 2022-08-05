#ifndef ZSTACK_H
#define ZSTACK_H

#define ADAPTER_RESET_DELAY                         10
#define ADAPTER_THROTTLE_DELAY                      20
#define ADAPTER_REQUEST_TIMEOUT                     10000
#define ADAPTER_CHANNEL_LIST                        0x07FFF800
#define ADAPTER_CONFIGURATION_MARKER                0x42

#define ZSTACK_PACKET_START                         0xFE
#define ZSTACK_COORDINATOR_STARTED                  0x09
#define ZSTACK_TX_POWER                             0x14

#define PERMIT_JOIN_MODE_ADDREESS                   0x0F
#define PERMIT_JOIN_MODE_BROADCAST                  0xFF
#define PERMIT_JOIN_BROARCAST_ADDRESS               0xFFFC

#define AF_ACK_REQUEST                              0x10
#define AF_DISCV_ROUTE                              0x20
#define AF_EN_SECURITY                              0x40
#define AF_SKIP_ROUTING                             0x80
#define AF_DEFAULT_RADIUS                           0x0F

#define BIND_ADDRESS_NOT_PRESENT                    0x00
#define BIND_GROUP_ADDRESS                          0x01
#define BIND_ADDRESS_16_BIT                         0x02
#define BIND_ADDRESS_64_BIT                         0x03
#define BIND_BROADCAST                              0xFF

#define SYS_OSAL_NV_ITEM_INIT                       0x2107
#define SYS_OSAL_NV_READ                            0x2108
#define SYS_OSAL_NV_WRITE                           0x2109
#define SYS_SET_TX_POWER                            0x2114
#define AF_REGISTER                                 0x2400
#define AF_DATA_REQUEST                             0x2401
#define AF_DATA_REQUEST_EXT                         0x2402
#define AF_INTER_PAN_CTL                            0x2410
#define ZDO_NODE_DESC_REQ                           0x2502
#define ZDO_SIMPLE_DESC_REQ                         0x2504
#define ZDO_ACTIVE_EP_REQ                           0x2505
#define ZDO_BIND_REQ                                0x2521
#define ZDO_MGMT_LQI_REQ                            0x2531
#define ZDO_MGMT_PERMIT_JOIN_REQ                    0x2536
#define ZDO_STARTUP_FROM_APP                        0x2540
#define UTIL_GET_DEVICE_INFO                        0x2700
#define APP_CNF_BDB_SET_CHANNEL                     0x2F08

#define SYS_RESET_IND                               0x4180
#define AF_DATA_CONFIRM                             0x4480
#define AF_INCOMING_MSG                             0x4481
#define AF_INCOMING_MSG_EXT                         0x4482
#define ZDO_NODE_DESC_RSP                           0x4582
#define ZDO_SIMPLE_DESC_RSP                         0x4584
#define ZDO_ACTIVE_EP_RSP                           0x4585
#define ZDO_BIND_RSP                                0x45A1
#define ZDO_MGMT_LQI_RSP                            0x45B1
#define ZDO_MGMT_PERMIT_JOIN_RSP                    0x45B6
#define ZDO_MGMT_NWK_UPDATE_RSP                     0x45B8
#define ZDO_STATE_CHANGE_IND                        0x45C0
#define ZDO_END_DEVICE_ANNCE_IND                    0x45C1
#define ZDO_SRC_RTG_IND                             0x45C4
#define ZDO_CONCENTRATOR_IND                        0x45C8
#define ZDO_LEAVE_IND                               0x45C9
#define ZDO_TC_DEV_IND                              0x45CA
#define APP_CNF_BDB_COMMISSIONING_NOTIFICATION      0x4F80

#define ZCD_NV_STARTUP_OPTION                       0x0003
#define ZCD_NV_PRECFGKEY                            0x0062
#define ZCD_NV_PRECFGKEYS_ENABLE                    0x0063
#define ZCD_NV_PANID                                0x0083
#define ZCD_NV_CHANLIST                             0x0084
#define ZCD_NV_LOGICAL_TYPE                         0x0087
#define ZCD_NV_ZDO_DIRECT_CB                        0x008F
#define ZCD_NV_USER                                 0x0060

#include <QSerialPort>
#include <QSettings>
#include <QTimer>

enum class LogicalType
{
    Coordinator,
    Router,
    EndDevice
};

#pragma pack(push, 1)

struct registerEndpointRequestStruct
{
    quint8  endpointId;
    quint16 profileId;
    quint16 deviceId;
    quint8  version;
    quint8  latency;
};

struct permitJoinRequestStruct
{
    quint8  mode;
    quint16 dstAddress;
    quint8  duration;
    quint8  significance;
};

struct nodeDescriptorRequestStruct
{
    quint16 dstAddress;
    quint16 networkAddress;
};

struct nodeDescriptorResponseStruct
{
    quint16 srcAddress;
    quint8  status;
    quint16 networkAddress;
    quint8  logicalType;
    quint8  apsFlags;
    quint8  capabilityFlags;
    quint16 manufacturerCode;
    quint8  maxBufferSize;
    quint16 maxTransferSize;
    quint16 serverMask;
    quint16 maxOutTransferSize;
    quint8  descriptorCapabilities;
};

struct simpleDescriptorRequestStruct
{
    quint16 dstAddress;
    quint16 networkAddress;
    quint8  endpointId;
};

struct simpleDescriptorResponseStruct
{
    quint16 srcAddress;
    quint8  status;
    quint16 networkAddress;
    quint8  length;
    quint8  endpointId;
    quint16 profileId;
    quint16 deviceId;
    quint8  version;
};

struct activeEndpointsRequestStruct
{
    quint16 dstAddress;
    quint16 networkAddress;
};

struct activeEndpointsResponseStruct
{
    quint16 srcAddress;
    quint8  status;
    quint16 networkAddress;
    quint8  count;
};

struct lqiRequestStruct
{
    quint16 networkAddress;
    quint8  index;
};

struct lqiResponseStruct
{
    quint16 networkAddress;
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
    quint16 networkAddress;
    quint64 srcIeeeAddress;
    quint8  srcEndpointId;
    quint16 clusterId;
    quint8  dstAddressMode;
    quint64 dstIeeeAddress;
    quint8  dstEndpointId;
};

struct dataRequestStruct
{
    quint16 networkAddress;
    quint8  dstEndpointId;
    quint8  srcEndpointId;
    quint16 clusterId;
    quint8  transactionId;
    quint8  options;
    quint8  radius;
    quint8  length;
};

struct dataRequestExtStruct
{
    quint8  dstAddressMode;
    quint64 dstAddress;
    quint8  dstEndpointId;
    quint16 dstPanId;
    quint8  srcEndpointId;
    quint16 clusterId;
    quint8  transactionId;
    quint8  options;
    quint8  radius;
    quint16 length;
};

struct deviceInfoResponseStruct
{
    quint8  status;
    quint64 ieeeAddress;
    quint16 networkAddress;
    quint8  deviceType;
    quint8  deviceState;
    quint8  assocCount;
};

struct setChannelRequestStruct
{
    quint8  isPrimary;
    quint32 channel;
};

struct nvReadRequestStruct
{
    quint16 id;
    quint8  offset;
};

struct nvReadReplyStruct
{
    quint8  status;
    quint8  length;
};

struct nvInitRequestStruct
{
    quint16 id;
    quint16 itemLength;
    quint8  dataLength;
};

struct nvWriteRequestStruct
{
    quint16 id;
    quint8  offset;
    quint8  length;
};

struct incomingMessageStruct
{
    quint16 groupId;
    quint16 clusterId;
    quint16 srcAddress;
    quint8  srcEndpointId;
    quint8  dstEndpointId;
    quint8  broadcast;
    quint8  linkQuality;
    quint8  security;
    quint32 timestamp;
    quint8  transactionId;
    quint8  length;
};

struct incomingMessageExtStruct
{
    quint16 groupId;
    quint16 clusterId;
    quint8  srcAddressMode;
    quint64 srcAddress;
    quint8  srcEndpointId;
    quint16 srcPanId;
    quint8  dstEndpointId;
    quint8  broadcast;
    quint8  linkQuality;
    quint8  security;
    quint32 timestamp;
    quint8  transactionId;
    quint16 length;
};

struct endDeviceAnnounceStruct
{
    quint16 srcAddress;
    quint16 networkAddress;
    quint64 ieeeAddress;
    quint8  capabilities;
};

struct endDeviceLeaveStruct
{
    quint16 networkAddress;
    quint64 ieeeAddress;
    quint8  request;
    quint8  remove;
    quint8  rejoin;
};

#pragma pack(pop)

class ZStack : public QObject
{
    Q_OBJECT

public:

    ZStack(QSettings *config, QObject *parent);

    inline quint8 dataRequestStatus(void) { return m_dataRequestStatus; }

    void init(void);
    void registerEndpoint(quint8 endpointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters);
    void setPermitJoin(bool enabled);
    void nodeDescriptorRequest(quint16 networkAddress);
    void simpleDescriptorRequest(quint16 networkAddress, quint8 endpointId);
    void activeEndpointsRequest(quint16 networkAddress);
    void lqiRequest(quint16 networkAddress, quint8 index = 0);

    bool bindRequest(quint16 networkAddress, const QByteArray &srcIeeeAddress, quint8 srcEndpointId, const QByteArray &dstIeeeAddress, quint8 dstEndpointId, quint16 clusterId);
    bool bindRequest(quint16 networkAddress, const QByteArray &ieeeAaddress, quint8 endpointId, quint16 clusterId);

    bool dataRequest(quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &data);
    bool dataRequestExt(const QByteArray &address, quint8 dstEndPointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data);
    bool dataRequestExt(quint16 networkAddress, quint8 dstEndPointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data);

    bool setInterPanEndpointId(quint8 endpointId);
    bool setInterPanChannel(quint8 channel);

    void resetInterPan(void);

private:

    QMap <quint16, QByteArray> m_nvValues;

    QSerialPort *m_port;
    QTimer *m_timer;

    qint16 m_bootPin, m_resetPin;
    quint8 m_channel;
    bool m_reset, m_debug, m_rts;

    quint8 m_status, m_transactionId;
    QByteArray m_ieeeAddress;

    quint16 m_bindAddress;
    bool m_bindRequestSuccess;

    quint8 m_dataRequestStatus;
    bool m_dataRequestSuccess, m_dataConfirmReceived;

    quint16 m_replyCommand;
    QByteArray m_replyData;

    void parsePacket(quint16 command, const QByteArray &data);
    bool sendRequest(quint16 command, const QByteArray &data = QByteArray());

    void resetAdapter(void);
    bool writeNvValue(quint16 id, const QByteArray &data);
    bool startCoordinator(void);

private slots:

    void startTimer(void);
    void receiveData(void);

signals:

    void coordinatorReady(const QByteArray &ieeeAddress);
    void endDeviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress, quint8 capabilities);
    void endDeviceLeft(const QByteArray &ieeeAddress, quint16 networkAddress);
    void nodeDescriptorReceived(quint16 networkAddress, quint16 manufacturerCode, LogicalType logicalType);
    void activeEndpointsReceived(quint16 networkAddress, const QByteArray data);
    void simpleDescriptorReceived(quint16 networkAddress, quint8 endpointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters);
    void neighborRecordReceived(quint16 networkAddress, quint16 neighborAddress, quint8 linkQuality, bool first);

    void messageReveived(quint16 networkAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);
    void messageReveivedExt(const QByteArray &ieeeAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);

    void bindResponse(void);
    void dataConfirm(void);

};

#endif
