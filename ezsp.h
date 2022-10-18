#ifndef EZSP_H
#define EZSP_H

#define ASH_FLAG_BYTE                                           0x7E
#define ASH_REQUEST_TIMEOUT                                     1000

#define ASH_CONTROL_ACK                                         0x80
#define ASH_CONTROL_NAK                                         0xA0
#define ASH_CONTROL_RST                                         0xC0
#define ASH_CONTROL_RSTACK                                      0xC1
#define ASH_CONTROL_ERROR                                       0xC2

#define FRAME_VERSION                                           0x0000
#define FRAME_REGISTER_ENDPOINT                                 0x0002
#define FRAME_SET_CONCENTRATOR                                  0x0010
#define FRAME_NETWORK_INIT                                      0x0017
#define FRAME_NETWORK_STATUS                                    0x0018
#define FRAME_STACK_STATUS_HANDLER                              0x0019
#define FRAME_FORM_NERWORK                                      0x001E
#define FRAME_LEAVE_NETWORK                                     0x0020
#define FRAME_PERMIT_JOINING                                    0x0022
#define FRAME_TRUST_CENTER_JOIN_HANDLER                         0x0024
#define FRAME_GET_IEEE_ADDRESS                                  0x0026
#define FRAME_GET_NETWORK_PARAMETERS                            0x0028
#define FRAME_SEND_UNICAST                                      0x0034
#define FRAME_MESSAGE_SENT_HANDLER                              0x003F
#define FRAME_INCOMING_MESSAGE_HANDLER                          0x0045
#define FRAME_SET_CONFIG                                        0x0053
#define FRAME_SET_POLICY                                        0x0055
#define FRAME_SET_SOURCE_ROUTE_DISCOVERY_MODE                   0x005A
#define FRAME_LOOKUP_IEEE_ADDRESS                               0x0061
#define FRAME_SET_INITIAL_SECURITY_STATE                        0x0068
#define FRAME_CLEAR_TRANSIENT_LINK_KEYS                         0x006B
#define FRAME_FIND_KEY_TABLE_ENTRY                              0x0075
#define FRAME_ERASE_KEY_TABLE_ENTRY                             0x0076
#define SET_EXTENDED_TIMEOUT                                    0x007E
#define FRAME_SET_VALUE                                         0x00AB
#define FRAME_ADD_TRANSIENT_LINK_KEY                            0x00AF
#define FRAME_CLEAR_KEY_TABLE                                   0x00B1

#define CONFIG_PACKET_BUFFER_COUNT                              0x01
#define CONFIG_STACK_PROFILE                                    0x0C
#define CONFIG_SECURITY_LEVEL                                   0x0D

#define POLICY_TRUST_CENTER                                     0x00
#define POLICY_BINDING_MODIFICATION                             0x01
#define POLICY_UNICAST_REPLIES                                  0x02
#define POLICY_POLL_HANDLER                                     0x03
#define POLICY_MESSAGE_CONTENTS_IN_CALLBACK                     0x04
#define POLICY_TC_KEY_REQUEST                                   0x05
#define POLICY_APP_KEY_REQUEST                                  0x06
#define POLICY_PACKET_VALIDATE_LIBRARY                          0x07
#define POLICY_ZLL                                              0x08
#define POLICY_TC_REJOINS_USING_WELL_KNOWN_KEY                  0x09

#define DECISION_ALLOW_JOINS                                    0x01
#define DECISION_ALLOW_UNSECURED_REJOINS                        0x02
#define DECISION_DISALLOW_BINDING_MODIFICATION                  0x10
#define DECISION_HOST_WILL_NOT_SUPPLY_REPLY                     0x20
#define DECISION_POLL_HANDLER_IGNORE                            0x30
#define DECISION_MESSAGE_TAG_ONLY_IN_CALLBACK                   0x40
#define DECISION_ALLOW_TC_KEY_REQUESTS                          0x51
#define DECISION_ALLOW_APP_KEY_REQUESTS                         0x61
#define DECISION_PACKET_VALIDATE_LIBRARY_CHECKS_DISABLED        0x63

#define SECURITY_TRUST_CENTER_USES_HASHED_LINK_KEY              0x0084
#define SECURITY_HAVE_PRECONFIGURED_KEY                         0x0100
#define SECURITY_HAVE_NETWORK_KEY                               0x0200
#define SECURITY_REQUIRE_ENCRYPTED_KEY                          0x0800

#define VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE                    0x05
#define VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE                    0x06
#define VALUE_STACK_TOKEN_WRITING                               0x07
#define VALUE_CCA_THRESHOLD                                     0x15
#define VALUE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE                0x3F

#define CONCENTRATOR_LOW_RAM                                    0xFFF8
#define CONCENTRATOR_HIGH_RAM                                   0xFFF9

//

#define APS_OPTION_RETRY                                        0x0040
#define APS_OPTION_ENABLE_ROUTE_DISCOVERY                       0x0100

//

#define MTOR_MIN_TIME                                           10
#define MTOR_MAX_TIME                                           90
#define MTOR_ROUTE_ERROR_THRESHOLD                              4
#define MTOR_DELIVERY_FAILURE_THRESHOLD                         3

#define EMBER_NETWORK_UP                                        0x90
#define EMBER_NETWORK_DOWN                                      0x91

#define EMBER_NO_NETWORK                                        0x00
#define EMBER_JOINING_NETWORK                                   0x01
#define EMBER_JOINED_NETWORK                                    0x02
#define EMBER_JOINED_NETWORK_NO_PARENT                          0x03
#define EMBER_LEAVING_NETWORK                                   0x04

#define EMBER_STANDARD_SECURITY_UNSECURED_JOIN                  0x01
#define EMBER_DEVICE_LEFT                                       0x02

#define EMBER_OUTGOING_DIRECT                                   0x00

//

#include "adapter.h"

#pragma pack(push, 1)

struct ezspHeaderStruct
{
    quint8  sequence;
    quint8  frameControlLow;
    quint8  frameControlHigh;
    quint16 frameId;
};

struct registerEndpointStruct
{
    quint8  endpointId;
    quint16 profileId;
    quint16 deviceId;
    quint8  appFlags;
    quint8  inClustersCount;
    quint8  outClustersCount;
};

struct setConcentratorStruct
{
    quint8  enabled;
    quint16 type;
    quint16 minTime;
    quint16 maxTime;
    quint8  routeErrorThreshold;
    quint8  deliveryFailureThreshold;
    quint8  maxHops;
};

struct trustCenterJoinHandlerStruct
{
    quint16 networkAddress;
    quint64 ieeeAddress;
    quint8  status;
    quint8  decision;
    quint16 parentAddress;
};

struct networkParametersStruct
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


struct sendUnicastStruct
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

struct messageSentHandlerStruct
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

struct incomingMessageHandlerStruct
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

struct setConfigStruct
{
    quint8  id;
    quint16 value;
};

struct setPolicyStruct
{
    quint8  id;
    quint8  decision;
};

struct setInitialSecurityStateStruct
{
    quint16 bitmask;
    quint8  preconfiguredKey[16];
    quint8  networkKey[16];
    quint8  networkKeySequenceNumber;
    quint64 preconfiguredTrustCenter;
};

struct setValueStruct
{
    quint8  id;
    quint8  length;
};

#pragma pack(pop)

class EZSP : public Adapter
{
    Q_OBJECT

public:

    EZSP(QSettings *config, QObject *parent);

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

    inline quint8 dataRequestStatus(void) override { return m_requestStatus; }

private:

    QByteArray m_networkKey;
    quint8 m_version, m_stackStatus, m_sequenceId, m_acknowledgeId, m_requestId, m_requestStatus;

    QByteArray m_replyData;
    bool m_replyStatus;

    QMap <quint8, quint16> m_config;
    QMap <quint8, quint16> m_policy;
    QMap <quint8, QByteArray> m_values;

    quint16 getCRC(quint8 *data, quint32 length);
    void randomize(QByteArray &data);

    bool sendUnicast(quint16 networkAddress, quint16 profileId, quint16 clusterId, quint8 srcEndPointId, quint8 dstEndPointId, const QByteArray &payload);
    bool sendFrame(quint16 frameId, const QByteArray &data = QByteArray(), bool version = false);

    void sendRequest(quint8 control, const QByteArray &payload = QByteArray());
    void parsePacket(const QByteArray &payload);

    bool startNetwork(void);
    bool startCoordinator(void);

private slots:

    void receiveData(void) override;

signals:
  
    void replyReceived(void);
    void stackStatusReceived(void);
    void messageSent(void);

};

#endif
