#ifndef ZSTACK_H
#define ZSTACK_H

#define ZSTACK_CONFIGURATION_MARKER                 0x42
#define ZSTACK_CLEAR_DELAY                          4000
#define ZSTACK_REQUEST_TIMEOUT                      5000

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

#define SYS_VERSION                                 0x2102
#define SYS_OSAL_NV_ITEM_INIT                       0x2107
#define SYS_OSAL_NV_READ                            0x2108
#define SYS_OSAL_NV_WRITE                           0x2109
#define AF_REGISTER                                 0x2400
#define AF_DATA_REQUEST                             0x2401
#define AF_DATA_REQUEST_EXT                         0x2402
#define AF_INTER_PAN_CTL                            0x2410
#define ZDO_NODE_DESC_REQ                           0x2502
#define ZDO_SIMPLE_DESC_REQ                         0x2504
#define ZDO_ACTIVE_EP_REQ                           0x2505
#define ZDO_BIND_REQ                                0x2521
#define ZDO_UNBIND_REQ                              0x2522
#define ZDO_MGMT_LQI_REQ                            0x2531
#define ZDO_MGMT_PERMIT_JOIN_REQ                    0x2536
#define ZDO_STARTUP_FROM_APP                        0x2540
#define ZB_READ_CONFIGURATION                       0x2604
#define ZB_WRITE_CONFIGURATION                      0x2605
#define UTIL_GET_DEVICE_INFO                        0x2700
#define APP_CNF_BDB_SET_CHANNEL                     0x2F08

#define SYS_RESET_REQ                               0x4100
#define SYS_RESET_IND                               0x4180
#define AF_DATA_CONFIRM                             0x4480
#define AF_INCOMING_MSG                             0x4481
#define AF_INCOMING_MSG_EXT                         0x4482
#define ZDO_NODE_DESC_RSP                           0x4582
#define ZDO_SIMPLE_DESC_RSP                         0x4584
#define ZDO_ACTIVE_EP_RSP                           0x4585
#define ZDO_BIND_RSP                                0x45A1
#define ZDO_UNBIND_RSP                              0x45A2
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
#define ZCD_NV_MARKER                               0x0060
#define ZCD_NV_PRECFGKEY                            0x0062
#define ZCD_NV_PRECFGKEYS_ENABLE                    0x0063
#define ZCD_NV_PANID                                0x0083
#define ZCD_NV_CHANLIST                             0x0084
#define ZCD_NV_LOGICAL_TYPE                         0x0087
#define ZCD_NV_ZDO_DIRECT_CB                        0x008F
#define ZCD_NV_TCLK_TABLE                           0x0101

#include "adapter.h"

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

struct extendedDataRequestStruct
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

struct readConfigurationReplyStruct
{
    quint8  status;
    quint8  id;
    quint8  length;
};

struct writeConfigurationRequestStruct
{
    quint8  id;
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

struct extendedIncomingMessageStruct
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

struct deviceLeaveStruct
{
    quint16 networkAddress;
    quint64 ieeeAddress;
    quint8  request;
    quint8  remove;
    quint8  rejoin;
};

#pragma pack(pop)

enum class ZStackVersion
{
    ZStack3x0,
    ZStack30x,
    ZStack12x
};

class ZStack : public Adapter
{
    Q_OBJECT

public:

    ZStack(QSettings *config, QObject *parent);

    void reset(void) override;
    void setPermitJoin(bool enabled) override;

    bool nodeDescriptorRequest(quint16 networkAddress) override;
    bool simpleDescriptorRequest(quint16 networkAddress, quint8 endpointId) override;
    bool activeEndpointsRequest(quint16 networkAddress) override;
    bool lqiRequest(quint16 networkAddress, quint8 index = 0) override;

    bool bindRequest(quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind = false) override;
    bool dataRequest(quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &data) override;

    bool extendedDataRequest(const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group = false) override;
    bool extendedDataRequest(quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group = false) override;

    bool setInterPanEndpointId(quint8 endpointId) override;
    bool setInterPanChannel(quint8 channel) override;
    void resetInterPan(void) override;

    inline quint8 dataRequestStatus(void) override { return m_dataRequestStatus; }

private:

    ZStackVersion m_version;

    quint8 m_status, m_transactionId;
    bool m_clear;

    quint16 m_bindAddress;
    bool m_bindRequestStatus;

    quint8 m_dataRequestStatus;
    bool m_dataRequestFinished;

    quint16 m_replyCommand;
    QByteArray m_replyData;

    QMap <quint16, QByteArray> m_nvItems;

    bool sendRequest(quint16 command, const QByteArray &data = QByteArray());
    void parsePacket(quint16 command, const QByteArray &data);

    bool writeNvItem(quint16 id, const QByteArray &data);
    bool writeConfiguration(quint16 id, const QByteArray &data);

    bool startCoordinator(void);
    void coordinatorStarted(void);

private slots:

    void receiveData(void) override;

signals:

    void bindResponse(void);
    void dataConfirm(void);

};

#endif
