#include <QtEndian>
#include <QThread>
#include "logger.h"
#include "zboss.h"

static uint8_t const crc8Table[256] =
{
    0xea, 0xd4, 0x96, 0xa8, 0x12, 0x2c, 0x6e, 0x50, 0x7f, 0x41, 0x03, 0x3d, 0x87, 0xb9, 0xfb, 0xc5,
    0xa5, 0x9b, 0xd9, 0xe7, 0x5d, 0x63, 0x21, 0x1f, 0x30, 0x0e, 0x4c, 0x72, 0xc8, 0xf6, 0xb4, 0x8a,
    0x74, 0x4a, 0x08, 0x36, 0x8c, 0xb2, 0xf0, 0xce, 0xe1, 0xdf, 0x9d, 0xa3, 0x19, 0x27, 0x65, 0x5b,
    0x3b, 0x05, 0x47, 0x79, 0xc3, 0xfd, 0xbf, 0x81, 0xae, 0x90, 0xd2, 0xec, 0x56, 0x68, 0x2a, 0x14,
    0xb3, 0x8d, 0xcf, 0xf1, 0x4b, 0x75, 0x37, 0x09, 0x26, 0x18, 0x5a, 0x64, 0xde, 0xe0, 0xa2, 0x9c,
    0xfc, 0xc2, 0x80, 0xbe, 0x04, 0x3a, 0x78, 0x46, 0x69, 0x57, 0x15, 0x2b, 0x91, 0xaf, 0xed, 0xd3,
    0x2d, 0x13, 0x51, 0x6f, 0xd5, 0xeb, 0xa9, 0x97, 0xb8, 0x86, 0xc4, 0xfa, 0x40, 0x7e, 0x3c, 0x02,
    0x62, 0x5c, 0x1e, 0x20, 0x9a, 0xa4, 0xe6, 0xd8, 0xf7, 0xc9, 0x8b, 0xb5, 0x0f, 0x31, 0x73, 0x4d,
    0x58, 0x66, 0x24, 0x1a, 0xa0, 0x9e, 0xdc, 0xe2, 0xcd, 0xf3, 0xb1, 0x8f, 0x35, 0x0b, 0x49, 0x77,
    0x17, 0x29, 0x6b, 0x55, 0xef, 0xd1, 0x93, 0xad, 0x82, 0xbc, 0xfe, 0xc0, 0x7a, 0x44, 0x06, 0x38,
    0xc6, 0xf8, 0xba, 0x84, 0x3e, 0x00, 0x42, 0x7c, 0x53, 0x6d, 0x2f, 0x11, 0xab, 0x95, 0xd7, 0xe9,
    0x89, 0xb7, 0xf5, 0xcb, 0x71, 0x4f, 0x0d, 0x33, 0x1c, 0x22, 0x60, 0x5e, 0xe4, 0xda, 0x98, 0xa6,
    0x01, 0x3f, 0x7d, 0x43, 0xf9, 0xc7, 0x85, 0xbb, 0x94, 0xaa, 0xe8, 0xd6, 0x6c, 0x52, 0x10, 0x2e,
    0x4e, 0x70, 0x32, 0x0c, 0xb6, 0x88, 0xca, 0xf4, 0xdb, 0xe5, 0xa7, 0x99, 0x23, 0x1d, 0x5f, 0x61,
    0x9f, 0xa1, 0xe3, 0xdd, 0x67, 0x59, 0x1b, 0x25, 0x0a, 0x34, 0x76, 0x48, 0xf2, 0xcc, 0x8e, 0xb0,
    0xd0, 0xee, 0xac, 0x92, 0x28, 0x16, 0x54, 0x6a, 0x45, 0x7b, 0x39, 0x07, 0xbd, 0x83, 0xc1, 0xff
};

static uint16_t const crc16Table[256] =
{
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e, 0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd, 0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c, 0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb, 0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a, 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036, 0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5, 0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134, 0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3, 0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

ZBoss::ZBoss(QSettings *config, QObject *parent) : Adapter(config, parent), m_clear(false)
{
    m_policy.append({ZBOSS_POLICY_TC_LINK_KEYS_REQUIRED,           0x00});
    m_policy.append({ZBOSS_POLICY_IC_REQUIRED,                     0x00});
    m_policy.append({ZBOSS_POLICY_TC_REJOIN_ENABLED,               0x01});
    m_policy.append({ZBOSS_POLICY_IGNORE_TC_REJOIN,                0x00});
    m_policy.append({ZBOSS_POLICY_APS_INSECURE_JOIN,               0x00});
    m_policy.append({ZBOSS_POLICY_DISABLE_NWK_MGMT_CHANNEL_UPDATE, 0x00});
}

bool ZBoss::unicastRequest(quint8 id, quint16 networkAddress, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    zbossDataRequestStruct request;

    memset(&request, 0, sizeof(request));

    request.requestLength = 0x15;
    request.dataLength = qToLittleEndian <quint16> (payload.length());
    request.dstAddress = qToLittleEndian <quint64> (networkAddress);
    request.profileId = qToLittleEndian <quint16> (m_endpoints.contains(srcEndPointId) ? m_endpoints.value(srcEndPointId)->profileId() : 0x0000);
    request.clusterId = qToLittleEndian(clusterId);
    request.dstEndpointId = dstEndPointId;
    request.srcEndpointId = srcEndPointId;
    request.radius = ZBOSS_DEFAULT_RADIUS;
    request.addressMode = ADDRESS_MODE_16_BIT;
    request.options = ZBOSS_ROUTE_DISCOVERY;

    return sendRequest(ZBOSS_APSDE_DATA_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(payload), id);
}

bool ZBoss::multicastRequest(quint8 id, quint16 groupId, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    zbossDataRequestStruct request;

    memset(&request, 0, sizeof(request));

    request.requestLength = 0x15;
    request.dataLength = qToLittleEndian <quint16> (payload.length());
    request.dstAddress = qToLittleEndian <quint64> (groupId);
    request.profileId = qToLittleEndian <quint16> (m_endpoints.contains(srcEndPointId) ? m_endpoints.value(srcEndPointId)->profileId() : 0x0000);
    request.clusterId = qToLittleEndian(clusterId);
    request.dstEndpointId = dstEndPointId;
    request.srcEndpointId = srcEndPointId;
    request.radius = ZBOSS_DEFAULT_RADIUS;
    request.addressMode = ADDRESS_MODE_GROUP;
    request.options = ZBOSS_ROUTE_DISCOVERY;

    return sendRequest(ZBOSS_APSDE_DATA_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(payload), id);
}

bool ZBoss::unicastInterPanRequest(quint8, const QByteArray &, quint16 , const QByteArray &)
{
    return false;
}

bool ZBoss::broadcastInterPanRequest(quint8, quint16, const QByteArray &)
{
    return false;
}

bool ZBoss::setInterPanChannel(quint8)
{
    return false;
}

void ZBoss::resetInterPanChannel(void)
{

}

bool ZBoss::zdoRequest(quint8 id, quint16 networkAddress, quint16 clusterId, const QByteArray &data)
{
    quint16 command, dstAddress = qToLittleEndian(networkAddress);

    switch (clusterId)
    {
        case ZDO_NODE_DESCRIPTOR_REQUEST:   command = ZBOSS_ZDO_NODE_DESC_REQ; break;
        case ZDO_SIMPLE_DESCRIPTOR_REQUEST: command = ZBOSS_ZDO_SIMPLE_DESC_REQ; break;
        case ZDO_ACTIVE_ENDPOINTS_REQUEST:  command = ZBOSS_ZDO_ACTIVE_EP_REQ; break;
        default: return false;
    }

    return sendRequest(command, QByteArray(reinterpret_cast <char*> (&dstAddress), sizeof(dstAddress)).append(data), id);
}

bool ZBoss::bindRequest(quint8 id, quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &address, quint8 dstEndpointId, bool unbind)
{
    QByteArray buffer = address.isEmpty() ? m_ieeeAddress : address;
    zbossBindRequestStruct request;

    memcpy(&request.srcAddress, m_requestAddress.constData(), sizeof(request.srcAddress));
    memcpy(&request.dstAddress, buffer.constData(), sizeof(request.dstAddress));

    request.networkAddress = qToLittleEndian(networkAddress);
    request.srcAddress = qToLittleEndian(qFromBigEndian(request.srcAddress));
    request.srcEndpointId = endpointId;
    request.clusterId = qToLittleEndian(clusterId);
    request.dstAddressMode = buffer.length() == 2 ? ADDRESS_MODE_GROUP : ADDRESS_MODE_64_BIT;
    request.dstEndpointId = 0x00;

    if (request.dstAddressMode != ADDRESS_MODE_GROUP)
    {
        request.dstAddress = qToLittleEndian(qFromBigEndian(request.dstAddress));
        request.dstEndpointId = dstEndpointId ? dstEndpointId : 0x01;
    }

    return sendRequest(unbind ? ZBOSS_ZDO_UNBIND_REQ : ZBOSS_ZDO_BIND_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)), id);
}

bool ZBoss::leaveRequest(quint8 id, quint16 networkAddress)
{
    zbossLeaveRequestStruct request;

    memcpy(&request.dstAddress, m_requestAddress.constData(), sizeof(request.dstAddress));

    request.networkAddress = qToLittleEndian(networkAddress);
    request.dstAddress = qToLittleEndian(qFromBigEndian(request.dstAddress));
    request.flags = 0x00;

    return sendRequest(ZBOSS_ZDO_MGMT_LEAVE_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)), id);
}

bool ZBoss::lqiRequest(quint8 id, quint16 networkAddress, quint8 index)
{
    quint16 dstAddress = qToLittleEndian(networkAddress);
    m_lqiRequestAddress = networkAddress;
    return sendRequest(ZBOSS_ZDO_MGMT_LQI_REQ, QByteArray(reinterpret_cast <char*> (&dstAddress), sizeof(dstAddress)).append(static_cast <char> (index)), id);
}

quint8 ZBoss::getCRC8(quint8 *data, quint32 length)
{
    quint8 crc = 0x00;

    while (length--)
        crc = crc8Table[crc ^ *data++];

    return crc;
}

quint16 ZBoss::getCRC16(quint8 *data, quint32 length)
{
    quint16 crc = 0x0000;

    while (length--)
        crc = static_cast <quint16> (crc >> 8) ^ crc16Table[(crc ^ *data++) & 0xFF];

    return qToLittleEndian(crc);
}

bool ZBoss::sendRequest(quint16 command, const QByteArray &data, quint8 id)
{
    zbossLowLevelHeaderStruct lowLevelHeader;
    zbossCommonHeaderStruct commonHeader;
    QByteArray payload;
    quint16 crc;

    if (m_adapterDebug)
        logInfo << "-->" << QString::asprintf("0x%04x", command) << data.toHex(':');

    m_command = command;

    lowLevelHeader.signature = qToBigEndian <quint16> (ZBOSS_SIGNATURE);
    lowLevelHeader.length = data.length() + 12;
    lowLevelHeader.type = ZBOSS_NCP_API_HL;
    lowLevelHeader.flags = m_sequenceId << 2 | ZBOSS_FLAG_FIRST_FRAGMENT | ZBISS_FLAG_LAST_FRAGMENT;
    lowLevelHeader.crc = getCRC8(reinterpret_cast <quint8*> (&lowLevelHeader) + 2, sizeof(lowLevelHeader) - 3);

    commonHeader.version = ZBOSS_PROTOCOL_VERSION;
    commonHeader.type = ZBOSS_TYPE_REQUEST;
    commonHeader.id = qToLittleEndian(command);

    payload.append(reinterpret_cast <char*> (&commonHeader), sizeof(commonHeader));
    payload.append(1, static_cast <char> (id));
    payload.append(data);

    crc = getCRC16(reinterpret_cast <quint8*> (payload.data()), payload.length());

    sendData(QByteArray(reinterpret_cast <char*> (&lowLevelHeader), sizeof(lowLevelHeader)).append(reinterpret_cast <char*> (&crc), sizeof(crc)).append(payload));
    return waitForSignal(this, command & 0x0200 && command != ZBOSS_ZDO_PERMIT_JOINING_REQ ? SIGNAL(acknowledgeReceived()) : SIGNAL(dataReceived()), ZBOSS_REQUEST_TIMEOUT);
}

void ZBoss::sendAcknowledge(void)
{
    zbossLowLevelHeaderStruct lowLevelHeader;

    lowLevelHeader.signature = qToBigEndian <quint16> (ZBOSS_SIGNATURE);
    lowLevelHeader.length = 5;
    lowLevelHeader.type = ZBOSS_NCP_API_HL;
    lowLevelHeader.flags = ZBOSS_FLAG_ACK | m_acknowledgeId << 4;
    lowLevelHeader.crc = getCRC8(reinterpret_cast <quint8*> (&lowLevelHeader) + 2, sizeof(lowLevelHeader) - 3);

    sendData(QByteArray(reinterpret_cast <char*> (&lowLevelHeader), sizeof(lowLevelHeader)));
}

void ZBoss::parsePacket(quint8 type, quint16 command, const QByteArray &data)
{
    if (m_adapterDebug)
        logInfo << "<--" << QString::asprintf("0x%04x", qFromBigEndian(command)) << data.toHex(':');

    if (type == ZBOSS_TYPE_RESPONSE && command == m_command)
    {
        m_replyStatus = static_cast <quint8> (data.at(2));
        m_replyData = data.mid(3);
        emit dataReceived();
    }

    switch (command)
    {
        case ZBOSS_NCP_RESET:
        case ZBOSS_NCP_RESET_IND:
        {
            m_sequenceId = 0;

            if (!startCoordinator())
            {
                logWarning << "Coordinator startup failed";
                break;
            }

            m_resetTimer->stop();
            break;
        }

        case ZBOSS_ZDO_NODE_DESC_REQ:
        {
            const zbossNodeDescriptorResponseStruct *message = reinterpret_cast <const zbossNodeDescriptorResponseStruct*> (m_replyData.constData());
            quint16 networkAddress;
            QByteArray payload;

            memcpy(&networkAddress, m_replyData.mid(m_replyData.length() - sizeof(networkAddress)), sizeof(networkAddress));

            payload.append(1, static_cast <char> (m_replyStatus));
            payload.append(reinterpret_cast <const char*> (&networkAddress), sizeof(networkAddress));
            payload.append(reinterpret_cast <const char*> (message), sizeof(zbossNodeDescriptorResponseStruct));

            emit zdoMessageReveived(qFromLittleEndian(networkAddress), ZDO_NODE_DESCRIPTOR_REQUEST, payload);
            break;
        }

        case ZBOSS_ZDO_SIMPLE_DESC_REQ:
        {
            const zbossSimpleDescriptorResponseStruct *message = reinterpret_cast <const zbossSimpleDescriptorResponseStruct*> (m_replyData.constData());
            quint16 networkAddress;
            QByteArray payload;

            memcpy(&networkAddress, data.mid(data.length() - sizeof(networkAddress)), sizeof(networkAddress));

            payload.append(1, static_cast <char> (m_replyStatus));
            payload.append(reinterpret_cast <const char*> (&networkAddress), sizeof(networkAddress));
            payload.append(1, static_cast <char> (message->inClusterCount * 2 + message->outClusterCount * 2) + sizeof(zbossSimpleDescriptorResponseStruct));
            payload.append(reinterpret_cast <const char*> (message), sizeof(zbossSimpleDescriptorResponseStruct) - 2);
            payload.append(1, static_cast <char> (message->inClusterCount));
            payload.append(m_replyData.mid(sizeof(zbossSimpleDescriptorResponseStruct), message->inClusterCount * 2));
            payload.append(1, static_cast <char> (message->outClusterCount));
            payload.append(m_replyData.mid(sizeof(zbossSimpleDescriptorResponseStruct) + message->inClusterCount * 2), message->outClusterCount * 2);

            emit zdoMessageReveived(networkAddress, ZDO_SIMPLE_DESCRIPTOR_REQUEST, payload);
            break;
        }

        case ZBOSS_ZDO_ACTIVE_EP_REQ:
        {
            quint16 networkAddress;
            QByteArray payload;

            memcpy(&networkAddress, data.mid(data.length() - sizeof(networkAddress)), sizeof(networkAddress));

            payload.append(1, static_cast <char> (m_replyStatus));
            payload.append(reinterpret_cast <char*> (&networkAddress), sizeof(networkAddress));
            payload.append(data.mid(3, data.length() - 5));

            emit zdoMessageReveived(qFromLittleEndian(networkAddress), ZDO_ACTIVE_ENDPOINTS_REQUEST, payload);
            break;
        }

        case ZBOSS_ZDO_MGMT_LQI_REQ:
        {
            emit zdoMessageReveived(m_lqiRequestAddress, ZDO_LQI_REQUEST, data.mid(2));
            break;
        }

        case ZBOSS_ZDO_DEV_ANNCE_IND:
        {
            const zbossDeviceAnnounceStruct *message = reinterpret_cast <const zbossDeviceAnnounceStruct*> (data.constData());
            quint64 ieeeAddress = qToBigEndian <quint64> (message->ieeeAddress);
            emit deviceJoined(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), message->networkAddress);
            break;
        }

        case ZBOSS_APSDE_DATA_IND:
        {
            const zbossIncomingMessageStruct *message = reinterpret_cast <const zbossIncomingMessageStruct*> (data.constData());
            emit zclMessageReveived(message->srcAddress, message->srcEndpointId, message->clusterId, message->linkQuality, data.mid(sizeof(zbossIncomingMessageStruct), message->dataLength));
            break;
        }

        case ZBOSS_NWK_LEAVE_IND:
        {
            const zbossDeviceLeaveStruct *message = reinterpret_cast <const zbossDeviceLeaveStruct*> (data.constData());
            quint64 ieeeAddress = qToBigEndian <quint64> (message->ieeeAddress);
            emit deviceLeft(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)));
            break;
        }

        default:
        {
            if (m_adapterDebug)
                logWarning << "Unrecognized ZBoss command" << QString::asprintf("0x%04x", command) << "with data" << (data.isEmpty() ? "(empty)" : data.toHex(':'));

            break;
        }
    }

    emit requestFinished(static_cast <quint8> (data.at(0)), m_replyStatus);
}

bool ZBoss::startCoordinator(void)
{
    quint32 channelMask = qToLittleEndian <quint32> (1 << m_channel);
    quint64 ieeeAddress;

    if (!sendRequest(ZBOSS_GET_LOCAL_IEEE_ADDR) || m_replyStatus)
    {
        logWarning << "Local IEEE address request failed";
        return false;
    }

    memcpy(&ieeeAddress, m_replyData.constData() + 1, sizeof(ieeeAddress));

    for (int i = 0; i < m_policy.length(); i++)
    {
        zbossSetPolicyStruct request = m_policy.at(i);

        if (sendRequest(ZBOSS_SET_TC_POLICY, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyStatus)
            continue;

        logWarning << "Set policy" << QString::asprintf("0x%04x", request.id) << "request failed";
    }

    if (!m_clear)
    {
        bool check = false;

        if (!sendRequest(ZBOSS_GET_MODULE_VERSION) || m_replyStatus)
        {
            logWarning << "Adapter version request failed";
            return false;
        }

        m_manufacturerName = "Nordic Semiconductor";
        m_modelName = QString::asprintf("ZBOSS NCP");
        m_firmware = QString::asprintf("%d.%d.%d", static_cast <quint8> (m_replyData.at(1)), static_cast <quint8> (m_replyData.at(2)), static_cast <quint8> (m_replyData.at(3)));

        logInfo << QString("Adapter type: %1 (%2)").arg(m_modelName, m_firmware).toUtf8().constData();

        if (!sendRequest(ZBOSS_GET_ZIGBEE_ROLE) || m_replyStatus)
        {
            logWarning << "Get adapter logical type request failed";
            return false;
        }

        if (m_replyData.at(0) != static_cast <char> (LogicalType::Coordinator))
        {
            logWarning << "Adapter logical type doesn't match coordinator";
            check = true;
        }

        if (!sendRequest(ZBOSS_GET_ZIGBEE_CHANNEL_MASK) || m_replyStatus)
        {
            logWarning << "Get adapter channel request failed";
            return false;
        }

        if (*(reinterpret_cast <quint32*> (m_replyData.data() + 2)) != channelMask)
        {
            logWarning << "Adapter channel doesn't match configuration";
            check = true;
        }

        if (!sendRequest(ZBOSS_GET_PAN_ID) || m_replyStatus)
        {
            logWarning << "Get adapter panid request failed";
            return false;
        }

        if (*(reinterpret_cast <quint16*> (m_replyData.data())) != qToLittleEndian(m_panId))
        {
            logWarning << "Adapter panid doesn't match configuration";
            check = true;
        }

        if (!sendRequest(ZBOSS_GET_NWK_KEYS) || m_replyStatus)
        {
            logWarning << "Get adapter network key request failed";
            return false;
        }

        if (m_replyData.mid(0, m_networkKey.length()) != m_networkKey)
        {
            logWarning << "Adapter network key doesn't match configuration";
            check = true;
        }

        if (check)
        {
            if (!m_write)
            {
                logWarning << "Adapter configuration can't be changed, write protection enabled";
                return false;
            }

            m_clear = true;
            reset();

            return true;
        }

        if (!sendRequest(ZBOSS_NWK_START_WITHOUT_FORMATION) || m_replyStatus)
        {
            logWarning << "Network startup failed";
            return false;
        }
    }
    else
    {
        zbossNetworkForamtionStruct network;

        logInfo << "Starting new network...";
        m_clear = false;

        if (!sendRequest(ZBOSS_SET_ZIGBEE_ROLE, QByteArray(1, static_cast <char> (LogicalType::Coordinator))) || m_replyStatus)
        {
            logWarning << "Set adapter logical type request failed";
            return false;
        }

        if (!sendRequest(ZBOSS_SET_ZIGBEE_CHANNEL_MASK, QByteArray(1, 0x00).append(reinterpret_cast <char*> (&channelMask), sizeof(channelMask))) || m_replyStatus)
        {
            logWarning << "Set channel mask request failed";
            return false;
        }

        if (!sendRequest(ZBOSS_SET_PAN_ID, QByteArray(reinterpret_cast <char*> (&m_panId), sizeof(m_panId))) || m_replyStatus)
        {
            logWarning << "Set panid request failed";
            return false;
        }

        if (!sendRequest(ZBOSS_SET_NWK_KEY, QByteArray(m_networkKey).append(1, 0x00)) || m_replyStatus)
        {
            logWarning << "Set nwk request failed";
            return false;
        }

        memset(&network, 0, sizeof(network));

        network.channelLength = 0x01;
        network.channelMask = channelMask;
        network.scanDuration = 0x05;
        network.extendedPanId = ieeeAddress;

        if (!sendRequest(ZBOSS_NWK_FORMATION, QByteArray(reinterpret_cast <char*> (&network), sizeof(network))) || m_replyStatus)
        {
            logWarning << "Network startup failed";
            return false;
        }
    }

    for (auto it = m_endpoints.begin(); it != m_endpoints.end(); it++)
    {
        zbossRegisterEndpointStruct request;
        QByteArray data;

        request.endpointId = it.key();
        request.profileId = qToLittleEndian(it.value()->profileId());
        request.deviceId = qToLittleEndian(it.value()->deviceId());
        request.version = 0x00;
        request.inClustersCount = static_cast <quint8> (it.value()->inClusters().count());
        request.outClustersCount = static_cast <quint8> (it.value()->outClusters().count());

        for (int i = 0; i < it.value()->inClusters().count(); i++)
        {
            quint16 clusterId = qToLittleEndian(it.value()->inClusters().at(i));
            data.append(reinterpret_cast <char*> (&clusterId), sizeof(clusterId));
        }

        for (int i = 0; i < it.value()->outClusters().count(); i++)
        {
            quint16 clusterId = qToLittleEndian(it.value()->outClusters().at(i));
            data.append(reinterpret_cast <char*> (&clusterId), sizeof(clusterId));
        }

        if (!sendRequest(ZBOSS_AF_SET_SIMPLE_DESC, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyStatus)
        {
            logWarning << "Endpoint" << QString::asprintf("0x%02x", it.key()) << "register request failed";
            continue;
        }

        logInfo << "Endpoint" << QString::asprintf("0x%02x", it.key()) << "registered successfully";
    }

    if (!sendRequest(ZBOSS_SET_TX_POWER, QByteArray(1, static_cast <char> (m_power))) || m_replyStatus)
        logWarning << "Set TX power request failed";

    ieeeAddress = qToBigEndian(qFromLittleEndian(ieeeAddress));
    m_ieeeAddress = QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress));

    emit coordinatorReady();
    return true;
}

void ZBoss::softReset(void)
{
    sendRequest(ZBOSS_NCP_RESET, QByteArray(1, m_clear ? 0x02 : 0x00));
}

void ZBoss::parseData(QByteArray &buffer)
{
    while (!buffer.isEmpty())
    {
        zbossLowLevelHeaderStruct *lowLevelHeader = reinterpret_cast <zbossLowLevelHeaderStruct*> (buffer.data());
        quint16 length = qFromLittleEndian(lowLevelHeader->length) + 2;

        if (lowLevelHeader->signature != qToBigEndian <quint16> (ZBOSS_SIGNATURE))
            return;

        if (lowLevelHeader->crc != getCRC8(reinterpret_cast <quint8*> (lowLevelHeader) + 2, sizeof(zbossLowLevelHeaderStruct) - 3))
        {
            logWarning << QString("Frame %1 low level header CRC mismatch").arg(QString(buffer.mid(0, length).toHex(':')));
            return;
        }

        if (m_portDebug)
            logInfo << "Frame received:" << buffer.mid(0, length).toHex(':');

        if (lowLevelHeader->flags & ZBOSS_FLAG_ACK)
        {
            if (m_sequenceId == (lowLevelHeader->flags >> 4 & 0x03))
            {
                m_sequenceId = (m_sequenceId + 1) & 0x03;
                emit acknowledgeReceived();
            }
        }
        else
        {
            m_acknowledgeId = lowLevelHeader->flags >> 2 & 0x03;
            sendAcknowledge();
        }

        if (length > 9)
        {
            if (*(reinterpret_cast <quint16*> (buffer.data() + 7)) != getCRC16(reinterpret_cast <quint8*> (buffer.data() + 9), length - 9))
            {
                logWarning << QString("Packet %1 CRC mismatch").arg(QString(buffer.mid(0, length).toHex(':')));
                return;
            }

            m_queue.enqueue(buffer.mid(9, length - 9));
        }

        buffer.remove(0, length);
    }
}

bool ZBoss::permitJoin(bool enabled)
{
    zbossPermitJoinStruct request;

    memset(&request, 0, sizeof(request));
    request.duration = enabled ? 0xF0 : 0x00;

    if (!sendRequest(ZBOSS_ZDO_PERMIT_JOINING_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || m_replyStatus)
    {
        logWarning << "Local permit join request failed";
        return false;
    }

    request.dstAddress = qToLittleEndian <quint16> (PERMIT_JOIN_BROARCAST_ADDRESS);

    if (!sendRequest(ZBOSS_ZDO_PERMIT_JOINING_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || m_replyStatus)
    {
        logWarning << "Broadcast permit join request failed";
        return false;
    }

    return true;
}

void ZBoss::serialError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::SerialPortError::ReadError)
    {
        Adapter::serialError(error);
        return;
    }

    m_serial->close();
    QThread::msleep(ZBOSS_RESET_DELAY);
    m_serial->open(QIODevice::ReadWrite);
}

void ZBoss::handleQueue(void)
{
    while (!m_queue.isEmpty())
    {
        QByteArray packet = m_queue.dequeue();
        const zbossCommonHeaderStruct *header = reinterpret_cast <const zbossCommonHeaderStruct*> (packet.constData());
        parsePacket(header->type, qFromLittleEndian(header->id), packet.mid(sizeof(zbossCommonHeaderStruct)));
    }
}
