#ifndef ZBOSS_H
#define ZBOSS_H

#define ZBOSS_REQUEST_TIMEOUT                       15000

#define ZBOSS_SIGNATURE                             0xDEAD
#define ZBOSS_NCP_API_HL                            0x06
#define ZBOSS_PROTOCOL_VERSION                      0x00

//Flags in low level header
#define IS_ACK                                      0x01
#define RETRANSMIT                                  0x02
#define PACKET_SEQ                                  0x0C
#define ACK_SEQ                                     0x30
#define FIRST_FRAG                                  0x40
#define LAST_FRAG                                   0x80

//Control Type
#define REQUEST                                     0x00
#define RESPONSE                                    0x01
#define INDICATION                                  0x02

//Policy ids
#define POLICY_TC_LINK_KEYS_REQUIRED                0x0000
#define POLICY_IC_REQUIRED                          0x0001
#define POLICY_TC_REJOIN_ENABLED                    0x0002
#define POLICY_IGNORE_TC_REJOIN                     0x0003
#define POLICY_APS_INSECURE_JOIN                    0x0004
#define POLICY_DISABLE_NWK_MGMT_CHANNEL_UPDATE      0x0005

//NCP config command ids
#define ZBOSS_GET_MODULE_VERSION                    0x0001
#define ZBOSS_NCP_RESET                             0x0002
#define ZBOSS_GET_ZIGBEE_ROLE                       0x0004
#define ZBOSS_SET_ZIGBEE_ROLE                       0x0005
#define ZBOSS_GET_ZIGBEE_CHANNEL_MASK               0x0006
#define ZBOSS_SET_ZIGBEE_CHANNEL_MASK               0x0007
#define ZBOSS_GET_ZIGBEE_CHANNEL                    0x0008
#define ZBOSS_GET_PAN_ID                            0x0009
#define ZBOSS_SET_PAN_ID                            0x000a
#define ZBOSS_GET_LOCAL_IEEE_ADDR                   0x000b
#define ZBOSS_SET_LOCAL_IEEE_ADDR                   0x000c
#define ZBOSS_GET_TX_POWER                          0x0010
#define ZBOSS_SET_TX_POWER                          0x0011
#define ZBOSS_GET_RX_ON_WHEN_IDLE                   0x0012
#define ZBOSS_SET_RX_ON_WHEN_IDLE                   0x0013
#define ZBOSS_GET_JOINED                            0x0014
#define ZBOSS_GET_AUTHENTICATED                     0x0015
#define ZBOSS_GET_ED_TIMEOUT                        0x0016
#define ZBOSS_SET_ED_TIMEOUT                        0x0017
#define ZBOSS_SET_NWK_KEY                           0x001b
#define ZBOSS_GET_NWK_KEYS                          0x001e
#define ZBOSS_GET_APS_KEY_BY_IEEE                   0x001f
#define ZBOSS_GET_PARENT_ADDRESS                    0x0022
#define ZBOSS_GET_EXTENDED_PAN_ID                   0x0023
#define ZBOSS_GET_COORDINATOR_VERSION               0x0024
#define ZBOSS_GET_SHORT_ADDRESS                     0x0025
#define ZBOSS_GET_TRUST_CENTER_ADDRESS              0x0026
#define ZBOSS_NCP_RESET_IND                         0x002b
#define ZBOSS_NVRAM_WRITE                           0x002e
#define ZBOSS_NVRAM_READ                            0x002f
#define ZBOSS_NVRAM_ERASE                           0x0030
#define ZBOSS_NVRAM_CLEAR                           0x0031
#define ZBOSS_SET_TC_POLICY                         0x0032
#define ZBOSS_SET_EXTENDED_PAN_ID                   0x0033
#define ZBOSS_SET_MAX_CHILDREN                      0x0034
#define ZBOSS_GET_MAX_CHILDREN                      0x0035

//ZDO command ids
#define ZBOSS_ZDO_NWK_ADDR_REQ                      0x0201
#define ZBOSS_ZDO_IEEE_ADDR_REQ                     0x0202
#define ZBOSS_ZDO_POWER_DESC_REQ                    0x0203
#define ZBOSS_ZDO_NODE_DESC_REQ                     0x0204
#define ZBOSS_ZDO_SIMPLE_DESC_REQ                   0x0205
#define ZBOSS_ZDO_ACTIVE_EP_REQ                     0x0206
#define ZBOSS_ZDO_MATCH_DESC_REQ                    0x0207
#define ZBOSS_ZDO_BIND_REQ                          0x0208
#define ZBOSS_ZDO_UNBIND_REQ                        0x0209
#define ZBOSS_ZDO_MGMT_LEAVE_REQ                    0x020a
#define ZBOSS_ZDO_PERMIT_JOINING_REQ                0x020b
#define ZBOSS_ZDO_DEV_ANNCE_IND                     0x020c
#define ZBOSS_ZDO_REJOIN                            0x020d
#define ZBOSS_ZDO_SYSTEM_SRV_DISCOVERY_REQ          0x020e
#define ZBOSS_ZDO_MGMT_BIND_REQ                     0x020f
#define ZBOSS_ZDO_MGMT_LQI_REQ                      0x0210
#define ZBOSS_ZDO_MGMT_NWK_UPDATE_REQ               0x0211
#define ZBOSS_ZDO_GET_STATS                         0x0213
#define ZBOSS_ZDO_DEV_AUTHORIZED_IND                0x0214
#define ZBOSS_ZDO_DEV_UPDATE_IND                    0x0215
#define ZBOSS_ZDO_SET_NODE_DESC_MANUF_CODE          0x0216

//APS command ids
#define APSDE_DATA_REQ                              0x0301
#define APSME_BIND                                  0x0302
#define APSME_UNBIND                                0x0303
#define APSME_ADD_GROUP                             0x0304
#define APSME_RM_GROUP                              0x0305
#define APSDE_DATA_IND                              0x0306
#define APSME_RM_ALL_GROUPS                         0x0307
#define APS_CHECK_BINDING                           0x0308
#define APS_GET_GROUP_TABLE                         0x0309
#define APSME_UNBIND_ALL                            0x030a

//NCP config command ids
#define NWK_FORMATION                               0x0401
#define NWK_DISCOVERY                               0x0402
#define NWK_NLME_JOIN                               0x0403
#define NWK_PERMIT_JOINING                          0x0404
#define NWK_GET_IEEE_BY_SHORT                       0x0405
#define NWK_GET_SHORT_BY_IEEE                       0x0406
#define NWK_GET_NEIGHBOR_BY_IEEE                    0x0407
#define NWK_REJOINED_IND                            0x0409
#define NWK_REJOIN_FAILED_IND                       0x040a
#define NWK_LEAVE_IND                               0x040b
#define PIM_SET_FAST_POLL_INTERVAL                  0x040e
#define PIM_SET_LONG_POLL_INTERVAL                  0x040f
#define PIM_START_FAST_POLL                         0x0410
#define PIM_START_LONG_POLL                         0x0411
#define PIM_START_POLL                              0x0412
#define PIM_STOP_FAST_POLL                          0x0414
#define PIM_STOP_POLL                               0x0415
#define PIM_ENABLE_TURBO_POLL                       0x0416
#define PIM_DISABLE_TURBO_POLL                      0x0417
#define NWK_PAN_ID_CONFLICT_RESOLVE                 0x041a
#define NWK_PAN_ID_CONFLICT_IND                     0x041b
#define NWK_ADDRESS_UPDATE_IND                      0x041c
#define NWK_START_WITHOUT_FORMATION                 0x041d
#define NWK_NLME_ROUTER_START                       0x041e
#define PARENT_LOST_IND                             0x0420
#define PIM_START_TURBO_POLL_PACKETS                0x0424
#define PIM_START_TURBO_POLL_CONTINUOUS             0x0425
#define PIM_TURBO_POLL_CONTINUOUS_LEAVE             0x0426
#define PIM_TURBO_POLL_PACKETS_LEAVE                0x0427
#define PIM_PERMIT_TURBO_POLL                       0x0428
#define PIM_SET_FAST_POLL_TIMEOUT                   0x0429
#define PIM_GET_LONG_POLL_INTERVAL                  0x042a
#define PIM_GET_IN_FAST_POLL_FLAG                   0x042b
#define SET_KEEPALIVE_MOVE                          0x042c
#define START_CONCENTRATOR_MODE                     0x042d
#define STOP_CONCENTRATOR_MODE                      0x042e
#define NWK_ENABLE_PAN_ID_CONFLICT_RESOLUTION       0x042f
#define NWK_ENABLE_AUTO_PAN_ID_CONFLICT_RESOLUTION  0x0430
#define PIM_TURBO_POLL_CANCEL_PACKET                0x0431

#include "adapter.h"

#pragma pack(push, 1)

struct lowLeverHeaderStruct
{
    quint16 signature;
    quint16 length;
    quint8  type;
    quint8  flags;
    quint8  CRC;
};

struct commonHeaderStruct
{
    quint8 version;
    quint8 type;
    quint16 id;
};

struct moduleVersionResponseStruct
{
    quint8 fwVersionMajor;
    quint8 fwVersionMinor;
    quint8 fwVersionRevision;
    quint8 fwVersionCommit;

    quint8 stackVersionMajor;
    quint8 stackVersionMinor;
    quint8 stackVersionRevision;
    quint8 stackVersionCommit;

    quint8 protocolVersionMajor;
    quint8 protocolVersionMinor;
    quint8 protocolVersionRevision;
    quint8 protocolVersionCommit;
};

struct localIEEEResponseStruct
{
    quint8 macIfaceNum;
    quint64 ieeeAddress;
};

struct channelMaskRequestStruct
{
    quint8 page;
    quint32 mask;
};

struct nwkSetRequestStruct
{
    quint8 key[16];
    quint8 number;
};

struct setTCPolicyStruct
{
    quint16 id;
    quint8 value;
};

struct nwkForamtionStruct
{
    quint8 channelListLen;
    channelMaskRequestStruct channelList;
    quint8 scanDuration;
    quint8 flag;
    quint16 address;
    quint64 ieeeAddress;
};

struct nwkLeaveStruct
{
    quint64 ieeeAddress;
    quint8  rejoin;
};

struct apsdeDataIndicatonStruct
{
    quint8 paramLength;
    quint16 dataLength;
    quint8 frameFC;
    quint16 srcNetworkAddress;
    quint16 dstNetworkAddress;
    quint16 grpNetworkAddress;
    quint8 dstEndpointId;
    quint8 srcEndpointId;
    quint16 clusterId;
    quint16 profileId;
    quint8 counter;
    quint16 srcMAC;
    quint16 dstMAC;
    quint8 lqi;
    quint8 rssi;
    quint8 keyAttr;
};

struct apsdeDataRequestStruct
{
    quint8 paramLength;
    quint16 dataLength;
    quint64 ieeeAddress;
    quint16 profileId;
    quint16 clusterId;
    quint8 dstEndpointId;
    quint8 srcEndpointId;
    quint8 radius;
    quint8 dstMode;
    quint8 txMode;
    quint8 alias;
    quint16 srcAlias;
    quint8 aliasSeq;
};

struct deviceAnnounceIndicatonStruct
{
    quint16 networkAddress;
    quint64 ieeeAddress;
    quint8  capabilities;
};

struct zdoBindRequestStruct
{
    quint16 networkAddress;
    quint64 srcAddress;
    quint8 srcEndpointId;
    quint16 clusterId;
    quint8 dstMode;
    quint64 dstAddress;
    quint8 dstEndpointId;
};

struct zdoLeaveRequestStruct
{
    quint16 networkAddress;
    quint64 dstAddress;
    quint8 flags;
};

struct zdoNodeDescriptorResponseStruct
{
    quint16 flags;
    quint8 macCpb;
    quint16 manufacturerCode;
    quint8 maxBufferSize;
    quint16 maxTransferSize;
    quint16 serverMask;
    quint16 maxOutTransferSize;
    quint8 descriptorCapabilities;
    quint16 networkAddress;
};

struct zdoSimpleDescriptorResponseStruct
{
    quint8 endpointId;
    quint16 profileId;
    quint16 deviceId;
    quint8 version;
    quint8 inpClusterCount;
    quint8 outClusterCount;
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

    quint16 m_command;
    QByteArray m_replyData;

    QByteArray m_networkKey;
    QList <setTCPolicyStruct> m_policy;

    quint8 m_packet_seq;
    quint8 m_tsn;
    qint8 m_ack_seq;

    quint8 getCRC8(quint8 *data, quint32 length);
    quint16 getCRC16(quint8 *data, quint32 length);
    inline quint8 getTSN() {m_tsn = (m_tsn + 1) % 255; return m_tsn;}

    bool sendRequest(quint16 command, const QByteArray &data = QByteArray());
    void sendAck();
    void parsePacket(quint8 type, quint16 command, const QByteArray &data);

    bool startCoordinator(void);

    void softReset(void) override;
    void parseData(QByteArray &buffer) override;
    bool permitJoin(bool enabled) override;

private slots:

    void handleQueue(void) override;

signals:

    void dataReceived(void);

};

#endif
