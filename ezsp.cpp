#include <QtEndian>
#include <QRandomGenerator>
#include "ezsp.h"
#include "logger.h"

static const uint16_t crcTable[256] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

EZSP::EZSP(QSettings *config, QObject *parent) : Adapter(config, parent), m_version(0), m_requestId(0)
{
    if (config->value("security/enabled", false).toBool())
        m_networkKey = QByteArray::fromHex(config->value("security/key", "000102030405060708090A0B0C0D0E0F").toString().remove("0x").toUtf8());

    m_config.append({CONFIG_SECURITY_LEVEL,                       qToLittleEndian <quint16> (0x0005)});
    m_config.append({CONFIG_STACK_PROFILE,                        qToLittleEndian <quint16> (0x0002)});
    m_config.append({CONFIG_PACKET_BUFFER_COUNT,                  qToLittleEndian <quint16> (0x00FF)});

    m_policy.append({POLICY_BINDING_MODIFICATION,                 qToLittleEndian <quint16> (DECISION_DISALLOW_BINDING_MODIFICATION)});
    m_policy.append({POLICY_UNICAST_REPLIES,                      qToLittleEndian <quint16> (DECISION_HOST_WILL_NOT_SUPPLY_REPLY)});
    m_policy.append({POLICY_POLL_HANDLER,                         qToLittleEndian <quint16> (DECISION_POLL_HANDLER_IGNORE)});
    m_policy.append({POLICY_MESSAGE_CONTENTS_IN_CALLBACK,         qToLittleEndian <quint16> (DECISION_MESSAGE_TAG_ONLY_IN_CALLBACK)});
    m_policy.append({POLICY_PACKET_VALIDATE_LIBRARY,              qToLittleEndian <quint16> (DECISION_PACKET_VALIDATE_LIBRARY_CHECKS_DISABLED)});
    m_policy.append({POLICY_ZLL,                                  qToLittleEndian <quint16> (DECISION_ALLOW_JOINS)});
    m_policy.append({POLICY_TC_REJOINS_USING_WELL_KNOWN_KEY,      qToLittleEndian <quint16> (DECISION_ALLOW_JOINS)});
    m_policy.append({POLICY_APP_KEY_REQUEST,                      qToLittleEndian <quint16> (DECISION_ALLOW_APP_KEY_REQUESTS)});
    m_policy.append({POLICY_TC_KEY_REQUEST,                       qToLittleEndian <quint16> (DECISION_ALLOW_TC_KEY_REQUESTS)});
    m_policy.append({POLICY_TRUST_CENTER,                         qToLittleEndian <quint16> (DECISION_ALLOW_JOINS | DECISION_ALLOW_UNSECURED_REJOINS)});

    m_values.append({VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE,     2, qToLittleEndian <quint16> (0x0052)});
    m_values.append({VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE,     2, qToLittleEndian <quint16> (0x0052)});
    m_values.append({VALUE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE, 1, qToLittleEndian <quint16> (0x0003)});
    m_values.append({VALUE_CCA_THRESHOLD,                      1, qToLittleEndian <quint16> (0x0000)});
}

bool EZSP::nodeDescriptorRequest(quint16 networkAddress)
{
    quint16 data = qToLittleEndian(networkAddress);
    return sendUnicast(networkAddress, 0x0000, APS_NODE_DESCRIPTOR, 0x00, 0x00, QByteArray(1, static_cast <char> (m_requestId)).append(reinterpret_cast <char*> (&data), sizeof(data)));
}

bool EZSP::simpleDescriptorRequest(quint16 networkAddress, quint8 endpointId)
{
    quint16 data = qToLittleEndian(networkAddress);
    return sendUnicast(networkAddress, 0x0000, APS_SIMPLE_DESCRIPTOR, 0x00, 0x00, QByteArray(1, static_cast <char> (m_requestId)).append(reinterpret_cast <char*> (&data), sizeof(data)).append(static_cast <quint8> (endpointId)));
}

bool EZSP::activeEndpointsRequest(quint16 networkAddress)
{
    quint16 data = qToLittleEndian(networkAddress);
    return sendUnicast(networkAddress, 0x0000, APS_ACTIVE_ENDPOINTS, 0x00, 0x00, QByteArray(1, static_cast <char> (m_requestId)).append(reinterpret_cast <char*> (&data), sizeof(data)));
}

bool EZSP::lqiRequest(quint16 networkAddress, quint8 index)
{
    Q_UNUSED(networkAddress)
    Q_UNUSED(index)

    return true;
}

bool EZSP::bindRequest(quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind)
{
    QByteArray data = bindRequestPayload(srcAddress, srcEndpointId, clusterId, dstAddress, dstEndpointId);

    if (data.isEmpty())
        return false;

    return sendUnicast(networkAddress, 0x0000, unbind ? APS_UNBIND : APS_BIND, 0x00, 0x00, QByteArray(1, static_cast <char> (m_sequenceId)).append(data));
}

bool EZSP::dataRequest(quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &payload)
{
    return sendUnicast(networkAddress, PROFILE_HA, clusterId, 0x01, endpointId, payload);
}

bool EZSP::extendedDataRequest(const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &payload, bool group)
{
    Q_UNUSED(address)
    Q_UNUSED(dstEndpointId)
    Q_UNUSED(dstPanId)
    Q_UNUSED(srcEndpointId)
    Q_UNUSED(clusterId)
    Q_UNUSED(payload)
    Q_UNUSED(group)

    return true;
}

bool EZSP::extendedDataRequest(quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group)
{
    Q_UNUSED(address)
    Q_UNUSED(dstEndpointId)
    Q_UNUSED(dstPanId)
    Q_UNUSED(srcEndpointId)
    Q_UNUSED(clusterId)
    Q_UNUSED(data)
    Q_UNUSED(group)

    return true;
}

bool EZSP::setInterPanEndpointId(quint8 endpointId)
{
    Q_UNUSED(endpointId)
    return true;
}

bool EZSP::setInterPanChannel(quint8 channel)
{
    Q_UNUSED(channel)
    return true;
}

void EZSP::resetInterPan(void)
{
}

quint16 EZSP::getCRC(quint8 *data, quint32 length)
{
    quint16 crc = 0xFFFF;

    while (length--)
        crc = static_cast <quint16> (crc << 8) ^ crcTable[(crc >> 8) ^ *data++];

    return qToBigEndian(crc);
}

void EZSP::randomize(QByteArray &payload)
{
    quint8 *buffer = reinterpret_cast <quint8*> (payload.data()), byte = 0x42;

    for (int i = 0; i < payload.length(); i++)
    {
        buffer[i] ^= byte;
        byte = byte & 0x01 ? (byte >> 1) ^ 0xB8 : byte >> 1;
    }
}

bool EZSP::sendUnicast(quint16 networkAddress, quint16 profileId, quint16 clusterId, quint8 srcEndPointId, quint8 dstEndPointId, const QByteArray &payload)
{
    sendUnicastStruct request;

    request.type = MESSAGE_TYPE_DIRECT;
    request.networkAddress = qToLittleEndian(networkAddress);
    request.profileId = qToLittleEndian(profileId);
    request.clusterId = qToLittleEndian(clusterId);
    request.srcEndpointId = srcEndPointId;
    request.dstEndpointId = dstEndPointId;
    request.options = qToLittleEndian <quint16> (APS_OPTION_RETRY | APS_OPTION_ENABLE_ROUTE_DISCOVERY);
    request.groupId = 0x0000;
    request.sequence = m_sequenceId;
    request.tag = m_requestId;
    request.length = static_cast <quint8> (payload.length());

    m_messageSent = false;

    if (sendFrame(FRAME_LOOKUP_IEEE_ADDRESS, QByteArray(reinterpret_cast <char*> (&request.networkAddress), sizeof(request.networkAddress))) && !m_replyData.at(0))
        sendFrame(FRAME_SET_EXTENDED_TIMEOUT, m_replyData.mid(1).append(1, 0x01));

    if (!sendFrame(FRAME_SEND_UNICAST, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(payload)) || m_replyData.at(0))
        return false;

    if (!m_messageSent && !waitForSignal(this, SIGNAL(messageSent()), NETWORK_REQUEST_TIMEOUT))
        return false;

    m_requestId++;
    return m_requestStatus ? false : true;
}

bool EZSP::sendFrame(quint16 frameId, const QByteArray &data, bool version)
{
    QByteArray payload;
    quint8 control = (m_sequenceId & 0x07) << 4 | m_acknowledgeId;

    if (version)
    {
        if (m_debug)
            logInfo << "-->" << QString::asprintf("%02x", m_sequenceId) << "(legacy version request)";

        payload.append(static_cast <char> (m_sequenceId));
        payload.append(2, 0x00);
    }
    else
    {
        ezspHeaderStruct header;

        header.sequence = m_sequenceId;
        header.frameControlLow = 0x00;
        header.frameControlHigh = 0x01;
        header.frameId = qToLittleEndian(frameId);

        if (m_debug)
            logInfo << "-->" << QString::asprintf("%02x", m_sequenceId) <<  QString::asprintf("%02X%02X", header.frameControlLow, header.frameControlHigh) << QString::asprintf("%04X", frameId) << data.toHex(':');

        payload.append(reinterpret_cast <char*> (&header), sizeof(header));
    }

    randomize(payload.append(data));

    for (quint8 i = 0; i < 3; i ++)
    {
        m_replyData.clear();
        m_replyReceived = false;

        sendRequest(control, payload);

        if (waitForSignal(this, SIGNAL(dataReceived()), ASH_REQUEST_TIMEOUT) && m_replyReceived)
            return true;

        if (m_errorReceived)
            return false;

        control |= 0x08;
    }

    return false;
}

void EZSP::sendRequest(quint8 control, const QByteArray &payload)
{
    QByteArray request, buffer;
    quint16 crc;

    request.append(static_cast <quint8> (control));
    request.append(payload);

    crc = getCRC(reinterpret_cast <quint8*> (request.data()), request.length());
    request.append(reinterpret_cast <char*> (&crc), sizeof(crc));

    for (int i = 0; i < request.length(); i++)
    {
        switch (request.at(i))
        {
            case 0x11: buffer.append(0x7D).append(0x31); break;
            case 0x13: buffer.append(0x7D).append(0x33); break;
            case 0x18: buffer.append(0x7D).append(0x38); break;
            case 0x1A: buffer.append(0x7D).append(0x3A); break;
            case 0x7D: buffer.append(0x7D).append(0x5D); break;
            case 0x7E: buffer.append(0x7D).append(0x5E); break;
            default: buffer.append(request.at(i)); break;
        }
    }

    m_errorReceived = false;
    m_device->write(buffer.append(static_cast <char> (ASH_FLAG_BYTE)));
}

void EZSP::parsePacket(const QByteArray &payload)
{
    const ezspHeaderStruct *header = reinterpret_cast <const ezspHeaderStruct*> (payload.constData());
    QByteArray data = payload.mid(sizeof(ezspHeaderStruct));

    if (m_debug)
        logInfo << "<--" << QString::asprintf("%02x", header->sequence) <<  QString::asprintf("%02X%02X", header->frameControlLow, header->frameControlHigh) << QString::asprintf("%04X", qFromLittleEndian(header->frameId)) << data.toHex(':');

    if (!(header->frameControlLow & 0x18) && header->sequence == m_sequenceId)
    {
        if (!(header->frameControlHigh & 0x01))
            m_version = static_cast <quint8> (payload.at(3));
        else
            m_replyData = data;

        m_sequenceId++;
        m_replyReceived = true;

        emit dataReceived();
        return;
    }

    switch (qFromLittleEndian(header->frameId))
    {
        case FRAME_STACK_STATUS_HANDLER:
        {
            m_stackStatus = static_cast <quint8> (data.at(0));
            emit stackStatusReceived();
            break;
        }

        case FRAME_TRUST_CENTER_JOIN_HANDLER:
        {
            const trustCenterJoinHandlerStruct *message = reinterpret_cast <const trustCenterJoinHandlerStruct*> (data.constData());

            switch (message->status)
            {
                case TRUST_CENTER_UNSECURED_JOIN:
                {
                    sendFrame(FRAME_FIND_KEY_TABLE_ENTRY, QByteArray(reinterpret_cast <const char*> (&message->ieeeAddress), sizeof(message->ieeeAddress)).append(1, 0x01));

                    if (m_replyData.at(0) != static_cast <char> (0xFF))
                        sendFrame(FRAME_ERASE_KEY_TABLE_ENTRY, m_replyData);

                    break;
                }

                case TRUST_CENTER_DEVICE_LEFT:
                {
                    quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->ieeeAddress));
                    emit deviceLeft(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)));
                    break;
                }
            }

            break;
        }

        case FRAME_MESSAGE_SENT_HANDLER:
        {
            const messageSentHandlerStruct *message = reinterpret_cast <const messageSentHandlerStruct*> (data.constData());

            if (m_requestId == message->tag)
            {
                m_requestStatus = message->status;
                m_messageSent = true;
                emit messageSent();
            }

            break;
        }

        case FRAME_INCOMING_MESSAGE_HANDLER:
        {
            const incomingMessageHandlerStruct *message = reinterpret_cast <const incomingMessageHandlerStruct*> (data.constData());
            QByteArray payload = data.mid(sizeof(incomingMessageHandlerStruct), message->length);

            if (message->profileId)
            {
                emit messageReveived(qFromLittleEndian(message->networkAddress), qFromLittleEndian(message->srcEndpointId), qFromLittleEndian(message->clusterId), message->linkQuality, payload);
                break;
            }

            switch (qToLittleEndian(message->clusterId) & 0x00FF)
            {
                case APS_DEVICE_ANNOUNCE:
                {
                    const deviceAnnounceStruct *announce = reinterpret_cast <const deviceAnnounceStruct*> (payload.constData() + 1);
                    quint64 ieeeAddress;
                    ieeeAddress = qToBigEndian(qFromLittleEndian(announce->ieeeAddress));
                    emit deviceJoined(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(announce->networkAddress));
                    break;
                }

                case APS_NODE_DESCRIPTOR:
                {
                    const nodeDescriptorResponseStruct *response = reinterpret_cast <const nodeDescriptorResponseStruct*> (payload.constData() + 1);

                    if (!response->status)
                        emit nodeDescriptorReceived(qFromLittleEndian(response->networkAddress), static_cast <LogicalType> (response->logicalType & 0x03), qFromLittleEndian(response->manufacturerCode));

                    break;
                }

                case APS_SIMPLE_DESCRIPTOR:
                {
                    const simpleDescriptorResponseStruct *response = reinterpret_cast <const simpleDescriptorResponseStruct*> (payload.constData() + 1);

                    if (!response->status)
                    {
                        QByteArray clusterData = payload.mid(sizeof(simpleDescriptorResponseStruct) + 1);
                        QList <quint16> inClusters, outClusters;
                        quint16 clusterId;

                        for (quint8 i = 0; i < static_cast <quint8> (clusterData.at(0)); i++)
                        {
                            memcpy(&clusterId, clusterData.constData() + i * sizeof(clusterId) + 1, sizeof(clusterId));
                            inClusters.append(qFromLittleEndian(clusterId));
                        }

                        clusterData.remove(0, clusterData.at(0) * sizeof(clusterId) + 1);

                        for (quint8 i = 0; i < static_cast <quint8> (clusterData.at(0)); i++)
                        {
                            memcpy(&clusterId, clusterData.constData() + i * sizeof(clusterId) + 1, sizeof(clusterId));
                            outClusters.append(qFromLittleEndian(clusterId));
                        }

                        emit simpleDescriptorReceived(qFromLittleEndian(response->networkAddress), response->endpointId, qFromLittleEndian(response->profileId), qFromLittleEndian(response->deviceId), inClusters, outClusters);
                    }

                    break;
                }

                case APS_ACTIVE_ENDPOINTS:
                {
                    const activeEndpointsResponseStruct *response = reinterpret_cast <const activeEndpointsResponseStruct*> (payload.constData() + 1);

                    if (!response->status)
                        emit activeEndpointsReceived(qFromLittleEndian(response->networkAddress), payload.mid(sizeof(activeEndpointsResponseStruct) + 1, response->count));

                    break;
                }

                case APS_BIND:
                case APS_UNBIND:
                    break;

                default:
                {
                    logInfo << "Unrecognized ZDO message" << QString::asprintf("0x%04X", qToLittleEndian(message->clusterId)) << data.toHex(':');
                    break;
                }
            }

            break;
        }
    }
}

bool EZSP::startNetwork(void)
{
    setInitialSecurityStateStruct security;
    networkParametersStruct network;
    setValueStruct value;

    logInfo << "Starting new network...";

    if (!sendFrame(FRAME_NETWORK_STATUS))
    {
        logWarning << "Network status request failed";
        return false;
    }

    if (m_replyData.at(0) == static_cast <char> (NETWORK_STATUS_JOINED))
    {
        m_stackStatus = 0x00;

        if (!sendFrame(FRAME_LEAVE_NETWORK) || m_replyData.at(0))
        {
            logWarning << "Leave existing network request failed";
            return false;
        }

        if (!m_replyData.at(0) && !m_stackStatus && !waitForSignal(this, SIGNAL(stackStatusReceived()), ASH_REQUEST_TIMEOUT))
        {
            logWarning << "Stack status handler timed out";
            return false;
        }

        if (m_stackStatus != STACK_STATUS_NETWORK_DOWN)
        {
            logWarning << "Unexpected stack status" << QString::asprintf("0x%02X", m_stackStatus) << "received";
            return false;
        }
    }

    if (!sendFrame(FRAME_CLEAR_KEY_TABLE) || m_replyData.at(0))
    {
        logWarning << "Clear key table request failed";
        return false;
    }

    if (!sendFrame(FRAME_CLEAR_TRANSIENT_LINK_KEYS))
    {
        logWarning << "Clear transient link keys request failed";
        return false;
    }

    memset(&security, 0, sizeof(security));
    security.bitmask = SECURITY_TRUST_CENTER_USES_HASHED_LINK_KEY | SECURITY_REQUIRE_ENCRYPTED_KEY;

    if (!m_networkKey.isEmpty())
    {
        security.bitmask |= SECURITY_HAVE_PRECONFIGURED_KEY | SECURITY_HAVE_NETWORK_KEY;

        for (quint8 i = 0; i < sizeof(security.preconfiguredKey); i += 4)
        {
            quint32 value = QRandomGenerator::global()->generate();
            memcpy(security.preconfiguredKey + i, &value, sizeof(value));
        }

        memcpy(security.networkKey, m_networkKey.constData(), sizeof(security.networkKey));
    }

    security.bitmask = qToLittleEndian(security.bitmask);

    if (!sendFrame(FRAME_SET_INITIAL_SECURITY_STATE, QByteArray(reinterpret_cast <char*> (&security), sizeof(security))) || m_replyData.at(0))
    {
        logWarning << "Set initial security state request failed";
        return false;
    }

    memset(&network, 0, sizeof(network));

    network.extendedPanId = m_ieeeAddress;
    network.panId = qToLittleEndian(m_panId);
    network.txPower = 0x05;
    network.channel = m_channel;
    network.channelList = qToLittleEndian(1 << m_channel);

    if (!sendFrame(FRAME_FORM_NERWORK, QByteArray(reinterpret_cast <char*> (&network), sizeof(network))) || m_replyData.at(0))
    {
        logWarning << "Form network request failed";
        return false;
    }

    value.id = VALUE_STACK_TOKEN_WRITING;
    value.length = 0x00;

    if (!sendFrame(FRAME_SET_VALUE, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)).append(1, 0x01)) || m_replyData.at(0))
        logWarning << "Set value" << QString::asprintf("0x%02X", VALUE_STACK_TOKEN_WRITING) << "request failed";

    if (!sendFrame(FRAME_NETWORK_STATUS))
    {
        logWarning << "Network status request failed";
        return false;
    }

    if (m_replyData.at(0) != static_cast <char> (NETWORK_STATUS_JOINED))
    {
        logWarning << "Unexpected network status" << QString::asprintf("0x%02X", m_replyData.at(0)) << "received";
        return false;
    }

    return true;
}

bool EZSP::startCoordinator(void)
{
    setConcentratorStruct concentrator;
    networkParametersStruct network;
    versionInfoStruct version;
    bool check = false;

    if (!sendFrame(FRAME_VERSION, QByteArray(), true))
    {
        logWarning << "Get adapter version request failed";
        return false;
    }

    if (m_version < 8)
    {
        logWarning << "Unsupported EZSP version" << m_version << "adapter detected";
        return false;
    }

    if (!sendFrame(FRAME_VERSION, QByteArray(1, static_cast <char> (m_version))))
    {
        logWarning << "Set adapter version request failed";
        return false;
    }

    if (!sendFrame(FRAME_GET_VALUE, QByteArray(1, static_cast <char> (VALUE_VERSION_INFO))))
    {
        logWarning << "Version info request failed";
        return false;
    }

    memcpy(&version, m_replyData.constData() + 2, sizeof(version));

    m_typeString = QString::asprintf("EZSP v%d", m_version);
    m_versionString = QString::asprintf("%d.%d.%d-%d", version.major, version.minor, version.patch, qFromLittleEndian(version.build));

    logInfo << QString("Adapter type: %1 (%2)").arg(m_typeString, m_versionString).toUtf8().constData();

    if (!sendFrame(FRAME_GET_IEEE_ADDRESS) || m_replyData.length() != sizeof(m_ieeeAddress))
    {
        logWarning << "Adapter address request failed";
        return false;
    }

    memcpy(&m_ieeeAddress, m_replyData.constData(), sizeof(m_ieeeAddress));

    for (int i = 0; i < m_config.length(); i++)
    {
        setConfigStruct request = m_config.at(i);

        if (sendFrame(FRAME_SET_CONFIG, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyData.at(0))
            continue;

        logWarning << "Set config" << QString::asprintf("0x%02X", request.id) << "request failed";
    }

    for (int i = 0; i < m_policy.length(); i++)
    {
        setConfigStruct request = m_policy.at(i);

        if (sendFrame(FRAME_SET_POLICY, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyData.at(0))
            continue;

        logWarning << "Set policy" << QString::asprintf("0x%02X", request.id) << "request failed";
    }

    for (int i = 0; i < m_values.length(); i++)
    {
        setValueStruct request = m_values.at(i);

        if (sendFrame(FRAME_SET_VALUE, QByteArray(reinterpret_cast <char*> (&request), request.length + 2)) && !m_replyData.at(0))
            continue;

        logWarning << "Set value" << QString::asprintf("0x%02X", request.id) << "request failed";
    }

    concentrator.enabled = 0x01;
    concentrator.type = qToLittleEndian <quint16> (CONCENTRATOR_HIGH_RAM);
    concentrator.minTime = qToLittleEndian <quint16> (CONCENTRATOR_MIN_TIME);
    concentrator.maxTime = qToLittleEndian <quint16> (CONCENTRATOR_MAX_TIME);
    concentrator.routeErrorThreshold = CONCENTRATOR_ROUTE_ERROR_THRESHOLD;
    concentrator.deliveryFailureThreshold = CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD;
    concentrator.maxHops = 0;

    if (!sendFrame(FRAME_SET_CONCENTRATOR, QByteArray(reinterpret_cast <char*> (&concentrator), sizeof(concentrator))) || m_replyData.at(0))
    {
        logWarning << "Set concentrator request failed";
        return false;
    }

    if (!sendFrame(FRAME_SET_SOURCE_ROUTE_DISCOVERY_MODE, QByteArray(1, 0x01)))
    {
        logWarning << "Set source route discovery mode request failed";
        return false;
    }

    for (auto it = m_endpointsData.begin(); it != m_endpointsData.end(); it++)
    {
        registerEndpointStruct request;
        QByteArray data;

        request.endpointId = it.key();
        request.profileId = qToLittleEndian(it.value()->profileId());
        request.deviceId = qToLittleEndian(it.value()->deviceId());
        request.appFlags = 0x00;
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

        if (!sendFrame(FRAME_REGISTER_ENDPOINT, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyData.at(0))
        {
            logWarning << "Endpoint" << QString::asprintf("0x%02X", it.key()) << "register request failed";
            return false;
        }

        logInfo << "Endpoint" << QString::asprintf("0x%02X", it.key()) << "registered successfully";
    }

    m_stackStatus = 0x00;

    if (!sendFrame(FRAME_NETWORK_INIT))
    {
        logWarning << "Network init request failed";
        return false;
    }

    if (!m_replyData.at(0) && !m_stackStatus && !waitForSignal(this, SIGNAL(stackStatusReceived()), ASH_REQUEST_TIMEOUT))
    {
        logWarning << "Stack status handler timed out";
        return false;
    }

    if (!sendFrame(FRAME_GET_NETWORK_PARAMETERS))
    {
        logWarning << "Network parameters request failed";
        return false;
    }

    memcpy(&network, m_replyData.constData() + 2, sizeof(network));

    if (m_replyData.at(1) != 0x01 || network.extendedPanId != m_ieeeAddress || network.panId != qToLittleEndian(m_panId) || network.channel != m_channel || m_stackStatus != STACK_STATUS_NETWORK_UP)
    {
        logWarning << "Adapter network parameters doesn't match configuration";
        check = true;
    }

    if (!sendFrame(FRAME_GET_GEY, QByteArray(1, static_cast <char> (CURRENT_NETWORK_KEY))))
    {
        logWarning << "Get network key request failed";
        return false;
    }

    if (m_replyData.mid(4, m_networkKey.length()) != m_networkKey)
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

        if (!startNetwork())
        {
            logWarning << "Network starup failed";
            return false;
        }
    }

    if (!sendFrame(FRAME_SET_MANUFACTURER_CODE, QByteArray::fromHex("4910")))
    {
        logWarning << "Set manufacturer code request failed";
        return false;
    }

    emit coordinatorReady();
    return true;
}

void EZSP::handleError(const QString &reason)
{
    logWarning << reason.toUtf8().constData();

    m_errorReceived = true;
    emit dataReceived();

    reset();
}

bool EZSP::permitJoin(bool enabled)
{
    if (enabled)
    {
        setConfigStruct policy;

        if (!sendFrame(FRAME_ADD_TRANSIENT_LINK_KEY, QByteArray::fromHex("FFFFFFFFFFFFFFFF5A6967426565416C6C69616E63653039")) || m_replyData.at(0))
        {
            logWarning << "Add transient key request failed";
            return false;
        }

        policy.id = POLICY_TRUST_CENTER;
        policy.value = qToLittleEndian <quint16> (DECISION_ALLOW_JOINS | DECISION_ALLOW_UNSECURED_REJOINS);

        if (!sendFrame(FRAME_SET_POLICY, QByteArray(reinterpret_cast <char*> (&policy), sizeof(policy))) || m_replyData.at(0))
        {
            logWarning << "Set policy item" << QString::asprintf("0x%02X", POLICY_TRUST_CENTER) << "request failed";
            return false;
        }
    }

    if (!sendFrame(FRAME_PERMIT_JOINING, QByteArray(1, enabled ? 0xF0 : 0x00)) || m_replyData.at(0))
    {
        logWarning << "Set permit join request failed";
        return false;
    }

    return true;
}

void EZSP::softReset(void)
{
    sendRequest(ASH_CONTROL_RST);
}

void EZSP::parseData(void)
{
    QByteArray buffer = m_device->readAll();

    while (!buffer.isEmpty())
    {
        QByteArray data;
        quint16 length, crc;

        if (buffer.startsWith(QByteArray::fromHex("1AC102")) || buffer.startsWith(QByteArray::fromHex("1AC202")))
            buffer.remove(0, 1);

        if (buffer.length() < ASH_MIN_LENGTH || !buffer.contains(static_cast <char> (ASH_FLAG_BYTE)))
            return;

        length = static_cast <quint16> (buffer.indexOf(ASH_FLAG_BYTE));

        for (int i = 0; i < length; i++)
        {
            if (buffer.at(i) != static_cast <char> (0x7D))
            {
                data.append(buffer.at(i));
                continue;
            }

            switch (buffer.at(++i))
            {
                case 0x31: data.append(0x11); break;
                case 0x33: data.append(0x13); break;
                case 0x38: data.append(0x18); break;
                case 0x3A: data.append(0x1A); break;
                case 0x5D: data.append(0x7D); break;
                case 0x5E: data.append(0x7E); break;

                default:
                    handleError(QString("Packet %1 unstaffing failed at position %2").arg(QString(buffer.toHex(':'))).arg(i));
                    return;
            }
        }

        memcpy(&crc, data.constData() + data.length() - 2, sizeof(crc));

        if (crc != getCRC(reinterpret_cast <quint8*> (data.data()), data.length() - 2))
        {
            handleError(QString("Packet %1 CRC mismatch").arg(QString(buffer.mid(0, length + 1).toHex(':'))));
            return;
        }

        m_queue.enqueue(data);
        buffer.remove(0, length + 1);
    }
}

void EZSP::handleQueue(void)
{
    while (!m_queue.isEmpty())
    {
        QByteArray packet = m_queue.dequeue();
        quint8 control = static_cast <quint8> (packet.at(0));

        if (!(control & 0x80))
        {
            QByteArray payload = packet.mid(1, packet.length() - 3);

            m_acknowledgeId = ((control >> 4) + 1) & 0x07;
            sendRequest(ASH_CONTROL_ACK | m_acknowledgeId);

            randomize(payload);
            parsePacket(payload);

            continue;
        }

        if ((control & 0xE0) == ASH_CONTROL_ACK)
        {
            m_replyReceived = true;
            emit dataReceived();
            continue;
        }

        if ((control & 0xE0) == ASH_CONTROL_NAK)
        {
            logWarning << "Received NAK frame:" << QString::asprintf("%d, %d", m_acknowledgeId, control & 0x07).toUtf8().constData();
            emit dataReceived();
            break;
        }

        if (control == ASH_CONTROL_RSTACK)
        {
            m_resetTimer->stop();

            m_sequenceId = 0;
            m_acknowledgeId = 0;

            if (!startCoordinator())
                logWarning << "Coordinator startup failed";

            break;
        }

        if (control == ASH_CONTROL_ERROR)
        {
            handleError("Received ERROR frame");
            break;
        }

        handleError(QString("Received unrecognized ASH frame:").arg(QString(packet.toHex(':'))));
    }

    m_queue.clear();
}
