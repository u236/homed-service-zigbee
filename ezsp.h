#ifndef EZSP_H
#define EZSP_H

#define ASH_REQUEST_TIMEOUT                                 2000
#define ASH_REQUEST_RETRIES                                 3
#define ASH_PACKET_FLAG                                     0x7E

#define ASH_CONTROL_ACK                                     0x80
#define ASH_CONTROL_NAK                                     0xA0
#define ASH_CONTROL_RST                                     0xC0
#define ASH_CONTROL_RSTACK                                  0xC1
#define ASH_CONTROL_ERROR                                   0xC2

#define EZSP_FRAME_VERSION                                  0x0000
#define EZSP_FRAME_REGISTER_ENDPOINT                        0x0002
#define EZSP_FRAME_SET_CONCENTRATOR                         0x0010
#define EZSP_FRAME_SET_MANUFACTURER_CODE                    0x0015
#define EZSP_FRAME_NETWORK_INIT                             0x0017
#define EZSP_FRAME_NETWORK_STATUS                           0x0018
#define EZSP_FRAME_STACK_STATUS_HANDLER                     0x0019
#define EZSP_FRAME_FORM_NERWORK                             0x001E
#define EZSP_FRAME_LEAVE_NETWORK                            0x0020
#define EZSP_FRAME_PERMIT_JOINING                           0x0022
#define EZSP_FRAME_TRUST_CENTER_JOIN_HANDLER                0x0024
#define EZSP_FRAME_GET_IEEE_ADDRESS                         0x0026
#define EZSP_FRAME_GET_NETWORK_PARAMETERS                   0x0028
#define EZSP_FRAME_SEND_UNICAST                             0x0034
#define EZSP_FRAME_SEND_MULTICAST                           0x0038
#define EZSP_FRAME_MESSAGE_SENT_HANDLER                     0x003F
#define EZSP_FRAME_INCOMING_MESSAGE_HANDLER                 0x0045
#define EZSP_FRAME_MAC_FILTER_MATCH_MESSAGE_HANDLER         0x0046
#define EZSP_FRAME_SET_CONFIG                               0x0053
#define EZSP_FRAME_SET_POLICY                               0x0055
#define EZSP_FRAME_SET_SOURCE_ROUTE_DISCOVERY_MODE          0x005A
#define EZSP_FRAME_SET_MULTICAST_TABLE_ENTRY                0x0064
#define EZSP_FRAME_SET_INITIAL_SECURITY_STATE               0x0068
#define EZSP_FRAME_GET_KEY                                  0x006A
#define EZSP_FRAME_CLEAR_TRANSIENT_LINK_KEYS                0x006B
#define EZSP_FRAME_FIND_KEY_TABLE_ENTRY                     0x0075
#define EZSP_FRAME_ERASE_KEY_TABLE_ENTRY                    0x0076
#define EZSP_FRAME_SET_EXTENDED_TIMEOUT                     0x007E
#define EZSP_FRAME_SEND_RAW                                 0x0096
#define EZSP_FRAME_GET_VALUE                                0x00AA
#define EZSP_FRAME_SET_VALUE                                0x00AB
#define EZSP_FRAME_ADD_TRANSIENT_LINK_KEY                   0x00AF
#define EZSP_FRAME_CLEAR_KEY_TABLE                          0x00B1
#define EZSP_FRAME_SET_CHANNEL                              0x00B9
#define EZSP_FRAME_IMPORT_TRANSIENT_KEY                     0x0111
#define EZSP_FRAME_EXPORT_KEY                               0x0114

#define EZSP_CONFIG_PACKET_BUFFER_COUNT                     0x01
#define EZSP_CONFIG_STACK_PROFILE                           0x0C
#define EZSP_CONFIG_SECURITY_LEVEL                          0x0D
#define EZSP_CONFIG_INDIRECT_TRANSMISSION_TIMEOUT           0x12
#define EZSP_CONFIG_END_DEVICE_POLL_TIMEOUT                 0x13
#define EZSP_CONFIG_TRUST_CENTER_ADDRESS_CACHE_SIZE         0x19
#define EZSP_CONFIG_FRAGMENT_WINDOW_SIZE                    0x1C
#define EZSP_CONFIG_FRAGMENT_DELAY_MS                       0x1D
#define EZSP_CONFIG_PAN_ID_CONFLICT_REPORT_THRESHOLD        0x22
#define EZSP_CONFIG_RETRY_QUEUE_SIZE                        0x34
#define EZSP_CONFIG_TC_REJOINS_WELL_KNOWN_KEY_TIMEOUT_S     0x38

#define EZSP_POLICY_TRUST_CENTER                            0x00
#define EZSP_POLICY_BINDING_MODIFICATION_POLICY             0x01
#define EZSP_POLICY_TC_KEY_REQUEST                          0x05
#define EZSP_POLICY_APP_KEY_REQUEST                         0x06

#define EZSP_SECURITY_TRUST_CENTER_USES_HASHED_LINK_KEY     0x0084
#define EZSP_SECURITY_HAVE_PRECONFIGURED_KEY                0x0100
#define EZSP_SECURITY_HAVE_NETWORK_KEY                      0x0200
#define EZSP_SECURITY_REQUIRE_ENCRYPTED_KEY                 0x0800

#define EZSP_VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE           0x05
#define EZSP_VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE           0x06
#define EZSP_VALUE_STACK_TOKEN_WRITING                      0x07
#define EZSP_VALUE_VERSION_INFO                             0x11
#define EZSP_VALUE_CCA_THRESHOLD                            0x15
#define EZSP_VALUE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE       0x3F
#define EZSP_VALUE_TRANSIENT_DEVICE_TIMEOUT                 0x43

#define EZSP_CONCENTRATOR_HIGH_RAM                          0xFFF9
#define EZSP_CONCENTRATOR_MIN_TIME                          5
#define EZSP_CONCENTRATOR_MAX_TIME                          60
#define EZSP_CONCENTRATOR_ROUTE_ERROR_THRESHOLD             3
#define EZSP_CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD        1

#define EZSP_STACK_STATUS_NETWORK_UP                        0x90
#define EZSP_STACK_STATUS_NETWORK_DOWN                      0x91

#define EZSP_TRUST_CENTER_UNSECURED_JOIN                    0x01
#define EZSP_TRUST_CENTER_DEVICE_LEFT                       0x02

#define EZSP_MESSAGE_TYPE_DIRECT                            0x00

#define EZSP_APS_OPTION_RETRY                               0x0040
#define EZSP_APS_OPTION_ENABLE_ROUTE_DISCOVERY              0x0100
#define EZSP_APS_OPTION_ENABLE_ADDRESS_DISCOVERY            0x1000

#define EZSP_NETWORK_STATUS_JOINED                          0x02

#include "adapter.h"

#pragma pack(push, 1)

struct ezspHeaderStruct
{
    quint8  sequence;
    quint8  frameControlLow;
    quint8  frameControlHigh;
    quint16 frameId;
};

struct ezspRegisterEndpointStruct
{
    quint8  endpointId;
    quint16 profileId;
    quint16 deviceId;
    quint8  appFlags;
    quint8  inClustersCount;
    quint8  outClustersCount;
};

struct ezspSetConcentratorStruct
{
    quint8  enabled;
    quint16 type;
    quint16 minTime;
    quint16 maxTime;
    quint8  routeErrorThreshold;
    quint8  deliveryFailureThreshold;
    quint8  maxHops;
};

struct ezspTrustCenterJoinStruct
{
    quint16 networkAddress;
    quint64 ieeeAddress;
    quint8  status;
    quint8  decision;
    quint16 parentAddress;
};

struct ezspNetworkParametersStruct
{
    quint64 extendedPanId;
    quint16 panId;
    quint8  txPower;
    quint8  channel;
    quint8  joinMethod;
    quint16 networkManagerId;
    quint8  networkUpdateId;
    quint32 channelList;
};

struct ezspSendUnicastStruct
{
    quint8  type;
    quint16 networkAddress;
    quint16 profileId;
    quint16 clusterId;
    quint8  srcEndpointId;
    quint8  dstEndpointId;
    quint16 options;
    quint16 groupId;
    quint8  sequence;
    quint8  tag;
    quint8  length;
};

struct ezspSendMulticastStruct
{
    quint16 profileId;
    quint16 clusterId;
    quint8  srcEndpointId;
    quint8  dstEndpointId;
    quint16 options;
    quint16 groupId;
    quint8  sequence;
    quint8  hops;
    quint8  radius;
    quint8  tag;
    quint8  length;
};

struct ezspSendIeeeRawStruct
{
    quint16 ieeeFrameControl;
    quint8  sequence;
    quint16 dstPanId;
    quint64 dstAddress;
    quint16 srcPanId;
    quint64 srcAddress;
    quint16 networkFrameControl;
    quint8  appFrameControl;
    quint16 clusterId;
    quint16 profileId;
};

struct ezspSendRawStruct
{
    quint16 ieeeFrameControl;
    quint8  sequence;
    quint16 dstPanId;
    quint16 dstAddress;
    quint16 srcPanId;
    quint64 srcAddress;
    quint16 networkFrameControl;
    quint8  appFrameControl;
    quint16 clusterId;
    quint16 profileId;
};

struct ezspMessageSentStruct
{
    quint8  type;
    quint16 networkAddress;
    quint16 profileId;
    quint16 clusterId;
    quint8  srcEndpointId;
    quint8  dstEndpointId;
    quint16 options;
    quint16 groupId;
    quint8  sequence;
    quint8  tag;
    quint8  status;
    quint8  length;
};

struct ezspIncomingMessageStruct
{
    quint8  type;
    quint16 profileId;
    quint16 clusterId;
    quint8  srcEndpointId;
    quint8  dstEndpointId;
    quint16 options;
    quint16 groupId;
    quint8  sequence;
    quint8  linkQuality;
    quint8  rssi;
    quint16 networkAddress;
    quint8  bindingIndex;
    quint8  addressIndex;
    quint8  length;
};

struct ezspMacFilterMessageStruct
{
    quint8  index;
    quint8  type;
    quint8  linkQuality;
    quint8  rssi;
    quint8  length;
    quint16 ieeeFrameControl;
    quint8  sequence;
    quint16 dstPanId;
    quint64 dstAddress;
    quint16 srcPanId;
    quint64 srcAddress;
    quint16 networkFrameControl;
    quint8  appFrameControl;
    quint16 clusterId;
    quint16 profileId;
};

struct ezspSetConfigStruct
{
    quint8  id;
    quint16 value;
};

struct ezspSetValueStruct
{
    quint8  id;
    quint8  length;
    quint16 value;
};

struct ezspAddGroupStruct
{
    quint16 groupId;
    quint8  endpointId;
    quint8  index;
};

struct ezspSetInitialSecurityStruct
{
    quint16 bitmask;
    quint8  preconfiguredKey[16];
    quint8  networkKey[16];
    quint8  networkKeySequenceNumber;
    quint64 preconfiguredTrustCenter;
};

struct ezspVersionStruct
{
    quint16 build;
    quint8  major;
    quint8  minor;
    quint8  patch;
};

#pragma pack(pop)

class EZSP : public Adapter
{
    Q_OBJECT

public:

    EZSP(QSettings *config, QObject *parent);

    bool unicastRequest(quint8 id, quint16 networkAddress, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload) override;
    bool multicastRequest(quint8 id, quint16 groupId, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload) override;

    bool unicastInterPanRequest(quint8 id, const QByteArray &ieeeAddress, quint16 clusterId, const QByteArray &payload) override;
    bool broadcastInterPanRequest(quint8 id, quint16 clusterId, const QByteArray &payload) override;

    bool setInterPanChannel(quint8 channel) override;
    void resetInterPanChannel(void) override;

private:

    QTimer *m_timer;
    quint8 m_version, m_stackStatus, m_sequenceId, m_acknowledgeId;

    QByteArray m_replyData;
    bool m_replyReceived, m_errorReceived;

    QList <ezspSetConfigStruct> m_config, m_policy;
    QList <ezspSetValueStruct> m_values;

    quint16 getCRC(quint8 *data, quint32 length);
    void randomize(QByteArray &data);

    bool sendFrame(quint16 frameId, const QByteArray &data = QByteArray(), bool version = false);
    void sendRequest(quint8 control, const QByteArray &payload = QByteArray());
    void parsePacket(const QByteArray &payload);

    bool startNetwork(quint64 extendedPanId);
    bool startCoordinator(void);

    void setManufacturerCode(quint16 value);
    void handleError(const QString &reason);

    void softReset(void) override;
    void parseData(QByteArray &buffer) override;
    bool permitJoin(bool enabled) override;

private slots:

    void resetManufacturerCode(void);
    void handleQueue(void) override;

signals:

    void dataReceived(void);
    void stackStatusReceived(void);

};

#endif
