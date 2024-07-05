#ifndef ZIGATE_H
#define ZIGATE_H

#define ZIGATE_REQUEST_TIMEOUT              2000
#define ZIGATE_SECURITY_MODE                0x02
#define ZIGATE_RADIUS                       0x1E

#define ZIGATE_SET_RAW_MODE                 0x0002
#define ZIGATE_GET_NETWORK_STATUS           0x0009
#define ZIGATE_GET_VERSION                  0x0010
#define ZIGATE_RESET                        0x0011
#define ZIGATE_ERASE_PERSISTENT_DATA        0x0012
#define ZIGATE_SET_EXTENDED_PANID           0x0020
#define ZIGATE_SET_CHANNEL_LIST             0x0021
#define ZIGATE_SET_NETWORK_KEY              0x0022
#define ZIGATE_SET_LOGICAL_TYPE             0x0023
#define ZIGATE_START_NETWORK                0x0024
#define ZIGATE_BIND_REQUEST                 0x0030
#define ZIGATE_UNBIND_REQUEST               0x0031
#define ZIGATE_NODE_DESCRIPTOR_REQUEST      0x0042
#define ZIGATE_SIMPLE_DESCRIPTOR_REQUEST    0x0043
#define ZIGATE_ACTIVE_ENDPOINTS_REQUEST     0x0045
#define ZIGATE_LEAVE_REQUEST                0x0047
#define ZIGATE_LQI_REQUEST                  0x004E
#define ZIGATE_SET_PERMIT_JOIN              0x0049
#define ZIGATE_ADD_GROUP                    0x0060
#define ZIGATE_APS_REQUEST                  0x0530

#define ZIGATE_STATUS                       0x8000
#define ZIGATE_DATA_INDICATION              0x8002
#define ZIGATE_RESTART_NON_FACTORY          0x8006
#define ZIGATE_RESTART_FACTORY              0x8007
#define ZIGATE_DATA_ACK                     0x8011

#define ZIGATE_DEVICE_ANNOUNCE              0x004D
#define ZIGATE_DEVICE_LEAVE_INDICATION      0x8048

#include "adapter.h"

#pragma pack(push, 1)

struct zigateHeaderStruct
{
    quint16 command;
    quint16 length;
    quint8  checksum;
};

struct zigateStatusStruct
{
    quint8  status;
    quint8  sequence;
    quint16 command;
};

struct zigateNetworkStatusStruct
{
    quint16 networkAddress;
    quint64 ieeeAddress;
    quint16 panId;
    quint64 extendedPanId;
    quint8  channel;
};

struct zigateDataIndicatonStruct
{
    quint8  status;
    quint16 profileId;
    quint16 clusterId;
    quint8  srcEndpointId;
    quint8  dstEndpointId;
};

struct zigateDataAcknowledgeStruct
{
    quint8  status;
    quint16 networkAddress;
    quint8  endpointId;
    quint16 clusterId;
    quint8  sequence;
};

struct zigateAddGroupStruct
{
    quint8  addressMode;
    quint16 address;
    quint8  srcEndpointId;
    quint8  dstEndpointId;
    quint16 groupId;
};

struct zigateApsRequestStruct
{
    quint8  addressMode;
    quint16 address;
    quint8  srcEndpointId;
    quint8  dstEndpointId;
    quint16 clusterId;
    quint16 profileId;
    quint8  securityMode;
    quint8  radius;
    quint8  length;
};

#pragma pack(pop)

class ZiGate : public Adapter
{
    Q_OBJECT

public:

    ZiGate(QSettings *config, QObject *parent) : Adapter(config, parent) {}

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

    bool m_commandReply;
    quint16 m_command;

    QByteArray m_replyData;
    quint8 m_requestId;

    QMap <quint8, quint8> m_requests;

    quint8 getChecksum(const zigateHeaderStruct *header, const QByteArray &payload);
    QByteArray encodeFrame(const QByteArray &data);

    bool sendRequest(quint16 command, const QByteArray &data = QByteArray(), quint8 id = 0);
    void parsePacket(quint16 command, const QByteArray &payload);

    bool startCoordinator(bool clear);
    bool apsRequest(quint8 id, quint8 addressMode, quint16 address, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload);

    void softReset(void) override;
    void parseData(QByteArray &buffer) override;
    bool permitJoin(bool enabled) override;

private slots:

    void handleQueue(void) override;

signals:

    void dataReceived(void);

};

#endif
