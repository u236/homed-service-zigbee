#ifndef ZSTACK_H
#define ZSTACK_H

#define ZSTACK_CLEAR_DELAY                      4000
#define ZSTACK_REQUEST_TIMEOUT                  10000

#define ZSTACK_SKIP_BOOTLOADER                  0xEF
#define ZSTACK_PACKET_FLAG                      0xFE

#define ZSTACK_NOT_STARTED_AUTOMATICALLY        0x00
#define ZSTACK_COORDINATOR_STARTED              0x09

#define ZSTACK_AF_DISCV_ROUTE                   0x20
#define ZSTACK_AF_DEFAULT_RADIUS                0x0F

#define ZSTACK_SYS_VERSION                      0x2102
#define ZSTACK_SYS_OSAL_NV_READ                 0x2108
#define ZSTACK_SYS_OSAL_NV_WRITE                0x2109
#define ZSTACK_SYS_SET_TX_POWER                 0x2114
#define ZSTACK_AF_REGISTER                      0x2400
#define ZSTACK_AF_DATA_REQUEST                  0x2401
#define ZSTACK_AF_DATA_REQUEST_EXT              0x2402
#define ZSTACK_AF_INTER_PAN_CTL                 0x2410
#define ZSTACK_ZDO_MGMT_PERMIT_JOIN_REQ         0x2536
#define ZSTACK_ZDO_MSG_CB_REGISTER              0x253E
#define ZSTACK_ZDO_STARTUP_FROM_APP             0x2540
#define ZSTACK_ZDO_ADD_GROUP                    0x254B
#define ZSTACK_ZB_READ_CONFIGURATION            0x2604
#define ZSTACK_ZB_WRITE_CONFIGURATION           0x2605
#define ZSTACK_UTIL_GET_DEVICE_INFO             0x2700
#define ZSTACK_APP_CNF_BDB_SET_CHANNEL          0x2F08

#define ZSTACK_SYS_RESET_REQ                    0x4100
#define ZSTACK_SYS_RESET_IND                    0x4180
#define ZSTACK_AF_DATA_CONFIRM                  0x4480
#define ZSTACK_AF_INCOMING_MSG                  0x4481
#define ZSTACK_AF_INCOMING_MSG_EXT              0x4482
#define ZSTACK_ZDO_MGMT_PERMIT_JOIN_RSP         0x45B6
#define ZSTACK_ZDO_MGMT_NWK_UPDATE_RSP          0x45B8
#define ZSTACK_ZDO_STATE_CHANGE_IND             0x45C0
#define ZSTACK_ZDO_END_DEVICE_ANNCE_IND         0x45C1
#define ZSTACK_ZDO_SRC_RTG_IND                  0x45C4
#define ZSTACK_ZDO_CONCENTRATOR_IND             0x45C8
#define ZSTACK_ZDO_LEAVE_IND                    0x45C9
#define ZSTACK_ZDO_TC_DEV_IND                   0x45CA
#define ZSTACK_ZDO_PERMIT_JOIN_IND              0x45CB
#define ZSTACK_ZDO_MSG_CB_INCOMING              0x45FF
#define ZSTACK_APP_CNF_BDB_COMMISSIONING        0x4F80

#define ZCD_NV_STARTUP_OPTION                   0x0003
#define ZCD_NV_PRECFGKEY                        0x0062
#define ZCD_NV_PRECFGKEYS_ENABLE                0x0063
#define ZCD_NV_PANID                            0x0083
#define ZCD_NV_CHANLIST                         0x0084
#define ZCD_NV_LOGICAL_TYPE                     0x0087
#define ZCD_NV_ZDO_DIRECT_CB                    0x008F
#define ZCD_NV_TCLK_TABLE                       0x0101

#include "adapter.h"

#pragma pack(push, 1)

struct zstackVersionStruct
{
    quint8  transport;
    quint8  product;
    quint8  major;
    quint8  minor;
    quint8  patch;
    quint32 build;
};

struct zstackAddGroupStruct
{
    quint8  endpointId;
    quint16 groupId;
    quint8  nameLength;
};

struct zstackRegisterEndpointStruct
{
    quint8  endpointId;
    quint16 profileId;
    quint16 deviceId;
    quint8  version;
    quint8  latency;
};

struct zstackPermitJoinStruct
{
    quint8  mode;
    quint16 dstAddress;
    quint8  duration;
    quint8  significance;
};

struct zstackDataRequestStruct
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

struct zstackExtendedRequestStruct
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

struct zstackSetChannelStruct
{
    quint8  isPrimary;
    quint32 channel;
};

struct zstackNvReadStruct
{
    quint16 id;
    quint8  offset;
};

struct zstackNvReplyStruct
{
    quint8  status;
    quint8  length;
};

struct zstackNvWriteStruct
{
    quint16 id;
    quint8  offset;
    quint8  length;
};

struct zstackReadConfigurationStruct
{
    quint8  status;
    quint8  id;
    quint8  length;
};

struct zstackWriteConfigurationStruct
{
    quint8  id;
    quint8  length;
};

struct zstackDataConfirmStruct
{
    quint8  status;
    quint8  endpointId;
    quint8  transactionId;
};

struct zstackIncomingMessageStruct
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

struct zstackExtendedMessageStruct
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

struct zstackZdoMessageStruct
{
    quint16 srcAddress;
    quint8  broadcast;
    quint16 clusterId;
    quint8  security;
    quint8  transactionId;
    quint16 dstAddress;
};

struct zstackDeviceLeaveStruct
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

    bool unicastRequest(quint8 id, quint16 networkAddress, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload) override;
    bool multicastRequest(quint8 id, quint16 groupId, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload) override;

    bool unicastInterPanRequest(quint8 id, const QByteArray &ieeeAddress, quint16 clusterId, const QByteArray &payload) override;
    bool broadcastInterPanRequest(quint8 id, quint16 clusterId, const QByteArray &payload) override;

    bool setInterPanChannel(quint8 channel) override;
    void resetInterPanChannel(void) override;

private:

    ZStackVersion m_version;

    quint8 m_status;
    bool m_clear;

    quint16 m_command;
    QByteArray m_replyData;

    QMap <quint16, QByteArray> m_nvItems;
    QList <quint16> m_zdoClusters;

    bool extendedRequest(quint8 id, const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &payload, bool group = false);
    bool extendedRequest(quint8 id, quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &paylaod, bool group = false);

    bool sendRequest(quint16 command, const QByteArray &data = QByteArray());
    void parsePacket(quint16 command, const QByteArray &data);

    bool writeNvItem(quint16 id, const QByteArray &data);
    bool writeConfiguration(quint16 id, const QByteArray &data);
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
