#ifndef ZBOSS_H
#define ZBOSS_H

#define ZBOSS_REQUEST_TIMEOUT                           2000
#define ZBOSS_RESET_DELAY                               2000

#define ZBOSS_SIGNATURE                                 0xDEAD
#define ZBOSS_PROTOCOL_VERSION                          0x00
#define ZBOSS_NCP_API_HL                                0x06

#define ZBOSS_ROUTE_DISCOVERY                           0x02
#define ZBOSS_DEFAULT_RADIUS                            0x03

#define ZBOSS_TYPE_REQUEST                              0x00
#define ZBOSS_TYPE_RESPONSE                             0x01

#define ZBOSS_FLAG_ACK                                  0x01
#define ZBOSS_FLAG_FIRST_FRAGMENT                       0x40
#define ZBISS_FLAG_LAST_FRAGMENT                        0x80

#define ZBOSS_GET_MODULE_VERSION                        0x0001
#define ZBOSS_NCP_RESET                                 0x0002
#define ZBOSS_GET_ZIGBEE_ROLE                           0x0004
#define ZBOSS_SET_ZIGBEE_ROLE                           0x0005
#define ZBOSS_GET_ZIGBEE_CHANNEL_MASK                   0x0006
#define ZBOSS_SET_ZIGBEE_CHANNEL_MASK                   0x0007
#define ZBOSS_GET_PAN_ID                                0x0009
#define ZBOSS_SET_PAN_ID                                0x000a
#define ZBOSS_GET_LOCAL_IEEE_ADDR                       0x000b
#define ZBOSS_SET_TX_POWER                              0x0011
#define ZBOSS_SET_NWK_KEY                               0x001b
#define ZBOSS_GET_NWK_KEYS                              0x001e
#define ZBOSS_NCP_RESET_IND                             0x002b
#define ZBOSS_SET_TC_POLICY                             0x0032
#define ZBOSS_AF_SET_SIMPLE_DESC                        0x0101
#define ZBOSS_ZDO_NODE_DESC_REQ                         0x0204
#define ZBOSS_ZDO_SIMPLE_DESC_REQ                       0x0205
#define ZBOSS_ZDO_ACTIVE_EP_REQ                         0x0206
#define ZBOSS_ZDO_BIND_REQ                              0x0208
#define ZBOSS_ZDO_UNBIND_REQ                            0x0209
#define ZBOSS_ZDO_MGMT_LEAVE_REQ                        0x020a
#define ZBOSS_ZDO_PERMIT_JOINING_REQ                    0x020b
#define ZBOSS_ZDO_DEV_ANNCE_IND                         0x020c
#define ZBOSS_ZDO_MGMT_LQI_REQ                          0x0210
#define ZBOSS_APSDE_DATA_REQ                            0x0301
#define ZBOSS_APSDE_DATA_IND                            0x0306
#define ZBOSS_NWK_FORMATION                             0x0401
#define ZBOSS_NWK_LEAVE_IND                             0x040b
#define ZBOSS_NWK_START_WITHOUT_FORMATION               0x041d

#define ZBOSS_POLICY_TC_LINK_KEYS_REQUIRED              0x0000
#define ZBOSS_POLICY_IC_REQUIRED                        0x0001
#define ZBOSS_POLICY_TC_REJOIN_ENABLED                  0x0002
#define ZBOSS_POLICY_IGNORE_TC_REJOIN                   0x0003
#define ZBOSS_POLICY_APS_INSECURE_JOIN                  0x0004
#define ZBOSS_POLICY_DISABLE_NWK_MGMT_CHANNEL_UPDATE    0x0005

#include "adapter.h"

#pragma pack(push, 1)

struct zbossLowLevelHeaderStruct
{
    quint16 signature;
    quint16 length;
    quint8  type;
    quint8  flags;
    quint8  crc;
};

struct zbossCommonHeaderStruct
{
    quint8  version;
    quint8  type;
    quint16 id;
};

struct zbossRegisterEndpointStruct
{
    quint8  endpointId;
    quint16 profileId;
    quint16 deviceId;
    quint8  version;
    quint8  inClustersCount;
    quint8  outClustersCount;
};

struct zbossNetworkForamtionStruct
{
    quint8  channelLength;
    quint8  channelPage;
    quint32 channelMask;
    quint8  scanDuration;
    quint8  distributed;
    quint16 address;
    quint64 extendedPanId;
};

struct zbossPermitJoinStruct
{
    quint16 dstAddress;
    quint8  duration;
    quint8  significance;
};

struct zbossBindRequestStruct
{
    quint16 networkAddress;
    quint64 srcAddress;
    quint8  srcEndpointId;
    quint16 clusterId;
    quint8  dstAddressMode;
    quint64 dstAddress;
    quint8  dstEndpointId;
};

struct zbossLeaveRequestStruct
{
    quint16 networkAddress;
    quint64 dstAddress;
    quint8  flags;
};

struct zbossDataRequestStruct
{
    quint8  requestLength;
    quint16 dataLength;
    quint64 dstAddress;
    quint16 profileId;
    quint16 clusterId;
    quint8  dstEndpointId;
    quint8  srcEndpointId;
    quint8  radius;
    quint8  addressMode;
    quint8  options;
    quint8  alias;
    quint16 aliasAddress;
    quint8  aliasId;
};

struct zbossNodeDescriptorResponseStruct
{
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

struct zbossSimpleDescriptorResponseStruct
{
    quint8  endpointId;
    quint16 profileId;
    quint16 deviceId;
    quint8  version;
    quint8  inClusterCount;
    quint8  outClusterCount;
};


struct zbossIncomingMessageStruct
{
    quint8  requestLength;
    quint16 dataLength;
    quint8  options;
    quint16 srcAddress;
    quint16 dstAddress;
    quint16 groupId;
    quint8  dstEndpointId;
    quint8  srcEndpointId;
    quint16 clusterId;
    quint16 profileId;
    quint8  count;
    quint16 srcHeader;
    quint16 dstHeader;
    quint8  linkQuality;
    quint8  rssi;
    quint8  security;
};

struct zbossDeviceAnnounceStruct
{
    quint16 networkAddress;
    quint64 ieeeAddress;
    quint8  capabilities;
};

struct zbossDeviceLeaveStruct
{
    quint64 ieeeAddress;
    quint8  rejoin;
};

struct zbossSetPolicyStruct
{
    quint16 id;
    quint8  value;
};

#pragma pack(pop)

class ZBoss : public Adapter
{
    Q_OBJECT

public:

    ZBoss(QSettings *config, QObject *parent);

    bool unicastRequest(quint8 id, quint16 networkAddress, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload) override;
    bool multicastRequest(quint8 id, quint16 groupId, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload) override;

    bool unicastInterPanRequest(quint8 id, const QByteArray &ieeeAddress, quint16 clusterId, const QByteArray &payload) override;
    bool broadcastInterPanRequest(quint8 id, quint16 clusterId, const QByteArray &payload) override;

    bool setInterPanChannel(quint8 channel) override;
    void resetInterPanChannel(void) override;

    bool zdoRequest(quint8 id, quint16 networkAddress, quint16 clusterId, const QByteArray &data = QByteArray()) override;
    bool bindRequest(quint8 id, quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &address, quint8 dstEndpointId, bool unbind = false) override;
    bool leaveRequest(quint8 id, quint16 networkAddress) override;
    bool lqiRequest(quint8 id, quint16 networkAddress, quint8 index) override;

private:

    bool m_clear;

    quint16 m_command;
    QByteArray m_replyData;

    quint8 m_sequenceId, m_acknowledgeId;
    quint16 m_lqiRequestAddress;

    QList <zbossSetPolicyStruct> m_policy;

    quint8 getCRC8(quint8 *data, quint32 length);
    quint16 getCRC16(quint8 *data, quint32 length);

    bool sendRequest(quint16 command, const QByteArray &data = QByteArray(), quint8 id = 0);
    void sendAcknowledge(void);
    void parsePacket(quint8 type, quint16 command, const QByteArray &data);

    bool startCoordinator(void);

    void softReset(void) override;
    void parseData(QByteArray &buffer) override;
    bool permitJoin(bool enabled) override;

private slots:

    void handleQueue(void) override;
    void serialError(QSerialPort::SerialPortError error) override;

signals:

    void acknowledgeReceived(void);
    void dataReceived(void);

};

#endif
