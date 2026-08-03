#include <QtEndian>
#include <QJsonArray>
#include <QRandomGenerator>
#include "ezsp.h"
#include "logger.h"
#include "zcl.h"

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

EZSP::EZSP(QSettings *config, QObject *parent) : Adapter(config, parent), m_timer(new QTimer(this)), m_version(0), m_reset(true)
{
    m_config.append({EZSP_CONFIG_TC_REJOINS_WELL_KNOWN_KEY_TIMEOUT_S,  qToLittleEndian <quint16> (0x005A)});
    m_config.append({EZSP_CONFIG_TRUST_CENTER_ADDRESS_CACHE_SIZE,      qToLittleEndian <quint16> (0x0002)});
    m_config.append({EZSP_CONFIG_FRAGMENT_DELAY_MS,                    qToLittleEndian <quint16> (0x0032)});
    m_config.append({EZSP_CONFIG_PAN_ID_CONFLICT_REPORT_THRESHOLD,     qToLittleEndian <quint16> (0x0002)});
    m_config.append({EZSP_CONFIG_INDIRECT_TRANSMISSION_TIMEOUT,        qToLittleEndian <quint16> (0x1E00)});
    m_config.append({EZSP_CONFIG_END_DEVICE_POLL_TIMEOUT,              qToLittleEndian <quint16> (0x000E)});
    m_config.append({EZSP_CONFIG_SECURITY_LEVEL,                       qToLittleEndian <quint16> (0x0005)});
    m_config.append({EZSP_CONFIG_STACK_PROFILE,                        qToLittleEndian <quint16> (0x0002)});
    m_config.append({EZSP_CONFIG_FRAGMENT_WINDOW_SIZE,                 qToLittleEndian <quint16> (0x0001)});
    m_config.append({EZSP_CONFIG_RETRY_QUEUE_SIZE,                     qToLittleEndian <quint16> (0x0010)});

    m_policy.append({EZSP_POLICY_BINDING_MODIFICATION_POLICY,          qToLittleEndian <quint16> (0x0012)});
    m_policy.append({EZSP_POLICY_APP_KEY_REQUEST,                      qToLittleEndian <quint16> (0x0060)});
    m_policy.append({EZSP_POLICY_TC_KEY_REQUEST,                       qToLittleEndian <quint16> (0x0051)});
    m_policy.append({EZSP_POLICY_TRUST_CENTER,                         qToLittleEndian <quint16> (0x0003)});

    m_values.append({EZSP_VALUE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE, 1, qToLittleEndian <quint16> (0x0003)});
    m_values.append({EZSP_VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE,     2, qToLittleEndian <quint16> (0x0052)});
    m_values.append({EZSP_VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE,     2, qToLittleEndian <quint16> (0x0052)});
    m_values.append({EZSP_VALUE_CCA_THRESHOLD,                      1, qToLittleEndian <quint16> (0x0000)});
    m_values.append({EZSP_VALUE_TRANSIENT_DEVICE_TIMEOUT,           2, qToLittleEndian <quint16> (0x2710)});

    connect(m_timer, &QTimer::timeout, this, &EZSP::resetManufacturerCode);
    m_timer->setSingleShot(true);
}

bool EZSP::unicastRequest(quint8 id, quint16 networkAddress, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    QByteArray data;

    if (m_version < 14)
    {
        ezspSendUnicastStruct request;

        request.type = EZSP_MESSAGE_TYPE_DIRECT;
        request.networkAddress = qToLittleEndian(networkAddress);
        request.profileId = qToLittleEndian <quint16> (m_endpoints.contains(srcEndPointId) ? m_endpoints.value(srcEndPointId)->profileId() : 0x0000);
        request.clusterId = qToLittleEndian(clusterId);
        request.srcEndpointId = srcEndPointId;
        request.dstEndpointId = dstEndPointId;
        request.options = qToLittleEndian <quint16> (EZSP_APS_OPTION_RETRY | EZSP_APS_OPTION_ENABLE_ROUTE_DISCOVERY | EZSP_APS_OPTION_ENABLE_ADDRESS_DISCOVERY);
        request.groupId = 0x0000;
        request.sequence = id;
        request.tag = id;
        request.length = static_cast <quint8> (payload.length());

        data = QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(payload);
    }
    else
    {
        ezspV14SendUnicastStruct request;

        request.type = EZSP_MESSAGE_TYPE_DIRECT;
        request.networkAddress = qToLittleEndian(networkAddress);
        request.profileId = qToLittleEndian <quint16> (m_endpoints.contains(srcEndPointId) ? m_endpoints.value(srcEndPointId)->profileId() : 0x0000);
        request.clusterId = qToLittleEndian(clusterId);
        request.srcEndpointId = srcEndPointId;
        request.dstEndpointId = dstEndPointId;
        request.options = qToLittleEndian <quint16> (EZSP_APS_OPTION_RETRY | EZSP_APS_OPTION_ENABLE_ROUTE_DISCOVERY | EZSP_APS_OPTION_ENABLE_ADDRESS_DISCOVERY);
        request.groupId = 0x0000;
        request.sequence = id;
        request.tag = qToLittleEndian <quint16> (id);
        request.length = static_cast <quint8> (payload.length());

        data = QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(payload);
    }

    if (m_extendedTimeout)
    {
        quint64 ieeeAddress;
        memcpy(&ieeeAddress, m_requestAddress.constData(), sizeof(ieeeAddress));
        ieeeAddress = qToLittleEndian(qFromBigEndian(ieeeAddress));
        sendFrame(EZSP_FRAME_SET_EXTENDED_TIMEOUT, QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)).append(1, 0x01));
    }

    return sendFrame(EZSP_FRAME_SEND_UNICAST, data) && !m_replyStatus;
}

bool EZSP::multicastRequest(quint8 id, quint16 groupId, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    QByteArray data;

    if (m_version < 14)
    {
        ezspSendMulticastStruct request;

        request.profileId = qToLittleEndian <quint16> (m_endpoints.contains(srcEndPointId) ? m_endpoints.value(srcEndPointId)->profileId() : 0x0000);
        request.clusterId = qToLittleEndian(clusterId);
        request.srcEndpointId = srcEndPointId;
        request.dstEndpointId = dstEndPointId;
        request.options = qToLittleEndian <quint16> (EZSP_APS_OPTION_ENABLE_ROUTE_DISCOVERY | EZSP_APS_OPTION_ENABLE_ADDRESS_DISCOVERY);
        request.groupId = qToLittleEndian(groupId);
        request.sequence = id;
        request.hops = 0x00;
        request.radius = 0x07;
        request.tag = id;
        request.length = static_cast <quint8> (payload.length());

        data = QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(payload);
    }
    else
    {
        ezspV14SendMulticastStruct request;

        request.profileId = qToLittleEndian <quint16> (m_endpoints.contains(srcEndPointId) ? m_endpoints.value(srcEndPointId)->profileId() : 0x0000);
        request.clusterId = qToLittleEndian(clusterId);
        request.srcEndpointId = srcEndPointId;
        request.dstEndpointId = dstEndPointId;
        request.options = qToLittleEndian <quint16> (EZSP_APS_OPTION_ENABLE_ROUTE_DISCOVERY | EZSP_APS_OPTION_ENABLE_ADDRESS_DISCOVERY);
        request.groupId = qToLittleEndian(groupId);
        request.sequence = id;
        request.hops = 0x00;
        request.broadcastAddress = qToLittleEndian <quint16> (0xFFFE);
        request.alias = 0x0000;
        request.networkSequence = 0x00;
        request.tag = qToLittleEndian <quint16> (id);
        request.length = static_cast <quint8> (payload.length());

        data = QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(payload);
    }

    return sendFrame(EZSP_FRAME_SEND_MULTICAST, data) && !m_replyStatus;
}

bool EZSP::unicastInterPanRequest(quint8 id, const QByteArray &ieeeAddress, quint16 clusterId, const QByteArray &payload)
{
    ezspSendIeeeRawStruct request;

    memcpy(&request.dstAddress, ieeeAddress.constData(), sizeof(request.dstAddress));
    memcpy(&request.srcAddress, m_ieeeAddress.constData(), sizeof(request.srcAddress));

    request.ieeeFrameControl = qToLittleEndian <quint16> (0xCC21);
    request.sequence = id;
    request.dstPanId = 0xFFFF;
    request.dstAddress = qToLittleEndian(qFromBigEndian(request.dstAddress));
    request.srcPanId = qToLittleEndian(m_panId);
    request.srcAddress = qToLittleEndian(qFromBigEndian(request.srcAddress));
    request.networkFrameControl = qToLittleEndian <quint16> (0x000B);
    request.appFrameControl = 0x03;
    request.clusterId = qToLittleEndian (clusterId);
    request.profileId = qToLittleEndian <quint16> (PROFILE_ZLL);

    return sendFrame(EZSP_FRAME_SEND_RAW, QByteArray(1, static_cast <char> (sizeof(request) + payload.length())).append(reinterpret_cast <char*> (&request), sizeof(request)).append(payload)) && !m_replyStatus;
}

bool EZSP::broadcastInterPanRequest(quint8 id, quint16 clusterId, const QByteArray &payload)
{
    ezspSendRawStruct request;

    memcpy(&request.srcAddress, m_ieeeAddress.constData(), sizeof(request.srcAddress));

    request.ieeeFrameControl = qToLittleEndian <quint16> (0xC801);
    request.sequence = id;
    request.dstPanId = 0xFFFF;
    request.dstAddress = 0xFFFF;
    request.srcPanId = qToLittleEndian(m_panId);
    request.srcAddress = qToLittleEndian(qFromBigEndian(request.srcAddress));
    request.networkFrameControl = qToLittleEndian <quint16> (0x000B);
    request.appFrameControl = 0x0B;
    request.clusterId = qToLittleEndian (clusterId);
    request.profileId = qToLittleEndian <quint16> (PROFILE_ZLL);

    return sendFrame(EZSP_FRAME_SEND_RAW, QByteArray(1, static_cast <char> (sizeof(request) + payload.length())).append(reinterpret_cast <char*> (&request), sizeof(request)).append(payload)) && !m_replyStatus;
}

bool EZSP::setInterPanChannel(quint8 channel)
{
    if (!sendFrame(EZSP_FRAME_SET_CHANNEL, QByteArray(1, static_cast <char> (channel))) || m_replyStatus)
    {
        logWarning << "Set Inter-PAN channel" << channel << "request failed";
        return false;
    }

    return true;
}

void EZSP::resetInterPanChannel(void)
{
    if (sendFrame(EZSP_FRAME_SET_CHANNEL, QByteArray(1, static_cast <char> (m_channel))) && !m_replyStatus)
        return;

    logWarning << "Reset Inter-PAN request failed";
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

bool EZSP::sendFrame(quint16 frameId, const QByteArray &data, bool version)
{
    QByteArray payload;
    quint8 control = (m_sequenceId & 0x07) << 4 | m_acknowledgeId;

    if (version)
    {
        logDebug(m_adapterDebug) << "-->" << QString::asprintf("0x%02x", m_sequenceId) << "(legacy version request)";
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

        logDebug(m_adapterDebug) << "-->" << QString::asprintf("0x%02x", m_sequenceId) << QString::asprintf("0x%02x%02x", header.frameControlLow, header.frameControlHigh) << QString::asprintf("0x%04x", frameId) << data.toHex(':');
        payload.append(reinterpret_cast <char*> (&header), sizeof(header));
    }

    randomize(payload.append(data));

    for (quint8 i = 0; i < ASH_REQUEST_RETRIES; i++)
    {
        m_replyStatus = 0xFF;
        m_replyData.clear();
        m_replyReceived = false;

        sendRequest(control, payload);

        if (waitForSignal(this, SIGNAL(dataReceived()), ADAPTER_REQUEST_TIMEOUT) && m_replyReceived)
        {
            m_errorCount = 0;
            return true;
        }

        if (m_errorReceived)
            break;

        control |= 0x08;
    }

    m_errorCount++;

    if (m_watchdog && m_errorCount == WATCHDOG_ERROR_COUNT)
        handleError(QString("Watchdog triggered after %1 request errors...").arg(WATCHDOG_ERROR_COUNT));

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
            default:   buffer.append(request.at(i)); break;
        }
    }

    m_errorReceived = false;
    sendData(buffer.append(static_cast <char> (ASH_PACKET_FLAG)));
}

void EZSP::parsePacket(const QByteArray &payload)
{
    const ezspHeaderStruct *header = reinterpret_cast <const ezspHeaderStruct*> (payload.constData());
    QByteArray data = payload.mid(sizeof(ezspHeaderStruct));

    logDebug(m_adapterDebug) << "<--" << QString::asprintf("0x%02x", header->sequence) << QString::asprintf("0x%02x%02x", header->frameControlLow, header->frameControlHigh) << QString::asprintf("0x%04x", qFromLittleEndian(header->frameId)) << data.toHex(':');

    if (!(header->frameControlLow & 0x18) && header->sequence == m_sequenceId)
    {
        if (header->frameControlHigh & 0x01)
        {
            m_replyStatus = data.length() ? static_cast <quint8> (data.at(0)) : 0x00;
            m_replyData = data;
        }
        else
            m_version = static_cast <quint8> (payload.at(3));

        m_sequenceId++;
        m_replyReceived = true;

        emit dataReceived();
        return;
    }

    switch (qFromLittleEndian(header->frameId))
    {
        case EZSP_FRAME_STACK_STATUS_HANDLER:
        {
            m_stackStatus = static_cast <quint8> (data.at(0));
            emit stackStatusReceived();
            break;
        }

        case EZSP_FRAME_TRUST_CENTER_JOIN_HANDLER:
        {
            const ezspTrustCenterJoinStruct *message = reinterpret_cast <const ezspTrustCenterJoinStruct*> (data.constData());

            switch (message->status)
            {
                case EZSP_TRUST_CENTER_UNSECURED_JOIN:
                {
                    quint8 index;

                    if (!sendFrame(EZSP_FRAME_FIND_KEY_TABLE_ENTRY, QByteArray(reinterpret_cast <const char*> (&message->ieeeAddress), sizeof(message->ieeeAddress)).append(1, 0x01)) || m_replyData.length() <= statusOffset())
                        break;

                    index = static_cast <quint8> (m_replyData.at(statusOffset()));

                    if (index != 0xFF)
                        sendFrame(EZSP_FRAME_ERASE_KEY_TABLE_ENTRY, QByteArray(1, static_cast <char> (index)));

                    break;
                }

                case EZSP_TRUST_CENTER_DEVICE_LEFT:
                {
                    quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->ieeeAddress));
                    emit deviceLeft(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)));
                    break;
                }
            }

            break;
        }

        case EZSP_FRAME_MESSAGE_SENT_HANDLER:
        {
            if (m_version < 14)
            {
                const ezspMessageSentStruct *message = reinterpret_cast <const ezspMessageSentStruct*> (data.constData());
                emit requestFinished(message->tag, message->status);
            }
            else
            {
                const ezspV14MessageSentStruct *message = reinterpret_cast <const ezspV14MessageSentStruct*> (data.constData());
                emit requestFinished(static_cast <quint8> (qFromLittleEndian(message->tag)), static_cast <quint8> (qFromLittleEndian(message->status)));
            }

            break;
        }

        case EZSP_FRAME_INCOMING_MESSAGE_HANDLER:
        {
            quint16 networkAddress, profileId, clusterId;
            quint8 endpointId, linkQuality;
            QByteArray payload;

            if (m_version < 14)
            {
                const ezspIncomingMessageStruct *message = reinterpret_cast <const ezspIncomingMessageStruct*> (data.constData());
                networkAddress = qFromLittleEndian(message->networkAddress);
                profileId = message->profileId;
                clusterId = qFromLittleEndian(message->clusterId);
                endpointId = message->srcEndpointId;
                linkQuality = message->linkQuality;
                payload = data.mid(sizeof(ezspIncomingMessageStruct), message->length);
            }
            else
            {
                const ezspV14IncomingMessageStruct *message = reinterpret_cast <const ezspV14IncomingMessageStruct*> (data.constData());
                networkAddress = qFromLittleEndian(message->networkAddress);
                profileId = message->profileId;
                clusterId = qFromLittleEndian(message->clusterId);
                endpointId = message->srcEndpointId;
                linkQuality = message->linkQuality;
                payload = data.mid(sizeof(ezspV14IncomingMessageStruct), message->length);
            }

            if (profileId)
            {
                emit zclMessageReveived(networkAddress, endpointId, clusterId, linkQuality, payload);
                break;
            }

            if (clusterId == ZDO_DEVICE_ANNOUNCE)
            {
                const deviceAnnounceStruct *announce = reinterpret_cast <const deviceAnnounceStruct*> (payload.constData() + 1);
                quint64 address = qToBigEndian(qFromLittleEndian(announce->ieeeAddress));
                QByteArray ieeeAddress(reinterpret_cast <char*> (&address), sizeof(address));

                if (ieeeAddress.startsWith(QByteArray::fromHex("04cffc")) || ieeeAddress.startsWith(QByteArray::fromHex("54ef44")))
                {
                    setManufacturerCode(MANUFACTURER_CODE_LUMI);
                    m_timer->start(20000);
                }

                emit deviceJoined(ieeeAddress, qFromLittleEndian(announce->networkAddress));
                break;
            }

            emit zdoMessageReveived(networkAddress, clusterId, payload.mid(1));
            break;
        }

        case EZSP_FRAME_MAC_FILTER_MATCH_MESSAGE_HANDLER:
        {
            if (m_version < 14)
            {
                const ezspMacFilterMessageStruct *message = reinterpret_cast <const ezspMacFilterMessageStruct*> (data.constData());
                quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->srcAddress));
                emit rawMessageReveived(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->clusterId), message->linkQuality, data.mid(sizeof(ezspMacFilterMessageStruct)));
            }
            else
            {
                const ezspV14MacFilterMessageStruct *message = reinterpret_cast <const ezspV14MacFilterMessageStruct*> (data.constData());
                quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->srcAddress));
                emit rawMessageReveived(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->clusterId), message->linkQuality, data.mid(sizeof(ezspV14MacFilterMessageStruct)));
            }

            break;
        }

        default:
        {
            logDebug(m_adapterDebug) << "Unrecognized frame id:" << QString::asprintf("0x%04x", qFromLittleEndian(header->frameId));
            break;
        }
    }
}

bool EZSP::createBackup(QJsonObject &backup)
{
    ezspNetworkKeyInfoStruct keyInfo;
    QJsonArray devices;
    quint8 size;

    if (!sendFrame(EZSP_FRAME_GET_NETWORK_KEY_INFO))
    {
        logWarning << "Backup aborted, network key info request failed";
        return false;
    }

    memcpy(&keyInfo, m_replyData.constData() + (m_version < 14 ? 0 : 4), sizeof(keyInfo));

    if (!sendFrame(EZSP_FRAME_GET_CONFIGURATION_VALUE, QByteArray(1, EZSP_CONFIG_KEY_TABLE_SIZE)) || m_replyStatus)
    {
        logWarning << "Backup aborted, key table size request failed";
        return false;
    }

    size = static_cast <quint8> (m_replyData.at(statusOffset() + 1));

    for (quint8 i = 0; i < size; i++)
    {
        ezspSecurityManagerMetadataStruct metadata;
        QJsonObject json;
        quint32 status;
        quint64 ieeeAddress;

        if (!sendFrame(EZSP_FRAME_EXPORT_LINK_KEY_BY_INDEX, QByteArray(1, static_cast <char> (i))))
        {
            logWarning << "Backup aborted, link key export request failed";
            return false;
        }

        memcpy(&status, m_replyData.constData() + (m_version < 14 ? 36 : 0), sizeof(status));

        if (status)
            continue;

        memcpy(&ieeeAddress, m_replyData.constData() + (m_version < 14 ? 0 : 8), sizeof(ieeeAddress));
        memcpy(&metadata, m_replyData.constData() + (m_version < 14 ? 24 : 38), sizeof(metadata));

        ieeeAddress = qToBigEndian(qFromLittleEndian(ieeeAddress));

        json.insert("ieeeAddress", QString(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)).toHex()));
        json.insert("linkKey", QString(m_replyData.mid(m_version < 14 ? 8 : 22, 16).toHex()));
        json.insert("txCounter", QJsonValue::fromVariant(qFromLittleEndian(metadata.outgoingFrameCounter)));

        devices.append(json);
    }

    backup.insert("ieeeAddress", QString(m_ieeeAddress.toHex()));
    backup.insert("panId", m_panId);
    backup.insert("channel", m_channel);
    backup.insert("networkKey", QString(m_networkKey.toHex()));
    backup.insert("frameCounter", QJsonValue::fromVariant(qFromLittleEndian(keyInfo.frameCounter)));
    backup.insert("devices", devices);

    logInfo << "Backup created, frame counter:" << qFromLittleEndian(keyInfo.frameCounter);
    return true;
}

bool EZSP::startNetwork(quint64 extendedPanId)
{
    ezspSetInitialSecurityStruct security;
    ezspNetworkParametersStruct network;
    ezspSetValueStruct value;

    if (!m_backup.isEmpty() && (m_backup.value("panId").toInt() != m_panId || m_backup.value("channel").toInt() != m_channel || QByteArray::fromHex(m_backup.value("networkKey").toString().toUtf8()) != m_networkKey))
    {
        logWarning << "Backup data doesn't match configuration, startup aborted to protect existing network, update configuration values to match backup, or remove backup file to start new network";
        return false;
    }

    logInfo << (m_backup.isEmpty() ? "Starting new network..." : "Restoring network from backup...");

    if (!sendFrame(EZSP_FRAME_NETWORK_STATUS))
    {
        logWarning << "Network status request failed";
        return false;
    }

    if (m_replyStatus == static_cast <char> (EZSP_NETWORK_STATUS_JOINED))
    {
        m_stackStatus = 0x00;

        if (!sendFrame(EZSP_FRAME_LEAVE_NETWORK, m_version < 14 ? QByteArray() : QByteArray(1, 0x00)) || m_replyStatus)
        {
            logWarning << "Leave existing network request failed";
            return false;
        }

        if (!m_replyStatus && !m_stackStatus && !waitForSignal(this, SIGNAL(stackStatusReceived()), ADAPTER_REQUEST_TIMEOUT))
        {
            logWarning << "Stack status handler timed out";
            return false;
        }

        if (m_version < 14 && m_stackStatus != EZSP_STACK_STATUS_NETWORK_DOWN)
        {
            logWarning << "Unexpected stack status:" << QString::asprintf("0x%02x", m_stackStatus);
            return false;
        }
    }

    if (!sendFrame(EZSP_FRAME_CLEAR_KEY_TABLE) || m_replyStatus)
    {
        logWarning << "Clear key table request failed";
        return false;
    }

    if (!sendFrame(EZSP_FRAME_CLEAR_TRANSIENT_LINK_KEYS))
    {
        logWarning << "Clear transient link keys request failed";
        return false;
    }

    if (!m_backup.isEmpty())
    {
        QJsonArray devices = m_backup.value("devices").toArray();
        quint32 frameCounter = qToLittleEndian <quint32> (m_backup.value("frameCounter").toVariant().toLongLong() + FRAME_COUNTER_MARGIN);

        if (!sendFrame(EZSP_FRAME_SET_VALUE, QByteArray(1, EZSP_VALUE_NWK_FRAME_COUNTER).append(1, sizeof(frameCounter)).append(reinterpret_cast <char*> (&frameCounter), sizeof(frameCounter))) || m_replyStatus)
        {
            logWarning << "Set network frame counter request failed";
            return false;
        }

        for (int i = 0; i < devices.count(); i++)
        {
            QJsonObject device = devices.at(i).toObject();
            QByteArray ieeeAddress = QByteArray::fromHex(device.value("ieeeAddress").toString().toUtf8()), linkKey = QByteArray::fromHex(device.value("linkKey").toString().toUtf8());
            quint64 address;

            if (ieeeAddress.length() != 8 || linkKey.length() != 16)
                continue;

            memcpy(&address, ieeeAddress.constData(), sizeof(address));
            address = qToLittleEndian(qFromBigEndian(address));

            if (sendFrame(EZSP_FRAME_IMPORT_LINK_KEY, QByteArray(1, static_cast <char> (i)).append(reinterpret_cast <char*> (&address), sizeof(address)).append(linkKey)) && !m_replyStatus)
                continue;

            logWarning << "Device" << ieeeAddress.toHex(':') << "link key import failed";
        }
    }

    memset(&security, 0, sizeof(security));
    security.bitmask = qToLittleEndian <quint16> (EZSP_SECURITY_TRUST_CENTER_USES_HASHED_LINK_KEY | EZSP_SECURITY_REQUIRE_ENCRYPTED_KEY | EZSP_SECURITY_HAVE_PRECONFIGURED_KEY | EZSP_SECURITY_HAVE_NETWORK_KEY | (m_backup.isEmpty() ? 0x0000 : EZSP_SECURITY_NO_FRAME_COUNTER_RESET));

    for (quint8 i = 0; i < sizeof(security.preconfiguredKey); i += 4)
    {
        quint32 value = QRandomGenerator::global()->generate();
        memcpy(security.preconfiguredKey + i, &value, sizeof(value));
    }

    memcpy(security.networkKey, m_networkKey.constData(), sizeof(security.networkKey));

    if (!sendFrame(EZSP_FRAME_SET_INITIAL_SECURITY_STATE, QByteArray(reinterpret_cast <char*> (&security), sizeof(security))) || m_replyStatus)
    {
        logWarning << "Set initial security state request failed";
        return false;
    }

    memset(&network, 0, sizeof(network));

    network.extendedPanId = extendedPanId;
    network.panId = qToLittleEndian(m_panId);
    network.txPower = m_power;
    network.channel = m_channel;
    network.channelList = qToLittleEndian(1 << m_channel);

    if (!sendFrame(EZSP_FRAME_FORM_NETWORK, QByteArray(reinterpret_cast <char*> (&network), sizeof(network))) || m_replyStatus)
    {
        logWarning << "Form network request failed";
        return false;
    }

    value.id = EZSP_VALUE_STACK_TOKEN_WRITING;
    value.length = 0;

    if (!sendFrame(EZSP_FRAME_SET_VALUE, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)).append(1, 0x01)) || m_replyStatus)
        logWarning << "Set value" << QString::asprintf("0x%02x", EZSP_VALUE_STACK_TOKEN_WRITING) << "request failed";

    if (!sendFrame(EZSP_FRAME_NETWORK_STATUS))
    {
        logWarning << "Network status request failed";
        return false;
    }

    if (m_replyStatus != static_cast <char> (EZSP_NETWORK_STATUS_JOINED))
    {
        logWarning << "Unexpected network status:" << QString::asprintf("0x%02x", m_replyStatus);
        return false;
    }

    return true;
}

bool EZSP::startCoordinator(void)
{
    QList <ezspSetConfigStruct> config = m_config;
    ezspSetConcentratorStruct concentrator;
    ezspNetworkParametersStruct network;
    ezspVersionStruct version;
    quint64 ieeeAddress, extendedPanId;
    bool check = false;

    if (!sendFrame(EZSP_FRAME_VERSION, QByteArray(), true))
    {
        logWarning << "Get adapter version request failed";
        return false;
    }

    if (m_version < 8)
    {
        logWarning << "Unsupported EZSP version" << m_version << "adapter detected";
        return false;
    }

    if (!sendFrame(EZSP_FRAME_VERSION, QByteArray(1, static_cast <char> (m_version))))
    {
        logWarning << "Set adapter version request failed";
        return false;
    }

    if (!sendFrame(EZSP_FRAME_GET_VALUE, QByteArray(1, static_cast <char> (EZSP_VALUE_VERSION_INFO))))
    {
        logWarning << "Version info request failed";
        return false;
    }

    memcpy(&version, m_replyData.constData() + statusOffset() + 2, sizeof(version));

    m_manufacturerName = "Silicon Labs";
    m_modelName = QString::asprintf("EZSP v%d", m_version);
    m_firmware = QString::asprintf("%d.%d.%d-%d", version.major, version.minor, version.patch, qFromLittleEndian(version.build));

    logInfo << QString("Adapter type: %1 (%2)").arg(m_modelName, m_firmware).toUtf8().constData();

    if (!sendFrame(EZSP_FRAME_GET_IEEE_ADDRESS) || m_replyData.length() != sizeof(ieeeAddress))
    {
        logWarning << "Adapter address request failed";
        return false;
    }

    memcpy(&ieeeAddress, m_replyData.constData(), sizeof(ieeeAddress));
    extendedPanId = ieeeAddress;

    if (!m_backup.isEmpty())
    {
        memcpy(&extendedPanId, QByteArray::fromHex(m_backup.value("ieeeAddress").toString().toUtf8()).constData(), sizeof(extendedPanId));
        extendedPanId = qToLittleEndian(qFromBigEndian(extendedPanId));
    }

    if (ieeeAddress != extendedPanId)
    {
        ezspSetTokenStruct request;

        request.token = qToLittleEndian <quint32> (EZSP_TOKEN_RESTORED_EUI64);
        request.index = 0x00000000;
        request.length = qToLittleEndian <quint32> (sizeof(extendedPanId));

        if (!sendFrame(EZSP_FRAME_SET_TOKEN_DATA, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(reinterpret_cast <char*> (&extendedPanId), sizeof(extendedPanId))) || m_replyStatus)
        {
            logWarning << "Restore coordinator address request failed";
            return false;
        }

        m_reset = true;
        reset();
        return true;
    }

    if (m_version < 12)
        config.append({EZSP_CONFIG_PACKET_BUFFER_COUNT, qToLittleEndian <quint16> (0x00FF)});

    for (int i = 0; i < config.length(); i++)
    {
        ezspSetConfigStruct request = config.at(i);

        if (sendFrame(EZSP_FRAME_SET_CONFIG, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyStatus)
            continue;

        logWarning << "Set config" << QString::asprintf("0x%02x", request.id) << "request failed";
    }

    for (int i = 0; i < m_policy.length(); i++)
    {
        ezspSetConfigStruct request = m_policy.at(i);

        if (sendFrame(EZSP_FRAME_SET_POLICY, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyStatus)
            continue;

        logWarning << "Set policy" << QString::asprintf("0x%02x", request.id) << "request failed";
    }

    for (int i = 0; i < m_values.length(); i++)
    {
        ezspSetValueStruct request = m_values.at(i);

        if (sendFrame(EZSP_FRAME_SET_VALUE, QByteArray(reinterpret_cast <char*> (&request), request.length + 2)) && !m_replyStatus)
            continue;

        logWarning << "Set value" << QString::asprintf("0x%02x", request.id) << "request failed";
    }

    concentrator.enabled = 0x01;
    concentrator.type = qToLittleEndian <quint16> (EZSP_CONCENTRATOR_HIGH_RAM);
    concentrator.minTime = qToLittleEndian <quint16> (EZSP_CONCENTRATOR_MIN_TIME);
    concentrator.maxTime = qToLittleEndian <quint16> (EZSP_CONCENTRATOR_MAX_TIME);
    concentrator.routeErrorThreshold = EZSP_CONCENTRATOR_ROUTE_ERROR_THRESHOLD;
    concentrator.deliveryFailureThreshold = EZSP_CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD;
    concentrator.maxHops = 0x00;

    if (!sendFrame(EZSP_FRAME_SET_CONCENTRATOR, QByteArray(reinterpret_cast <char*> (&concentrator), sizeof(concentrator))) || m_replyStatus)
    {
        logWarning << "Set concentrator request failed";
        return false;
    }

    if (!sendFrame(EZSP_FRAME_SET_SOURCE_ROUTE_DISCOVERY_MODE, QByteArray(1, 0x01)))
    {
        logWarning << "Set source route discovery mode request failed";
        return false;
    }

    for (auto it = m_endpoints.begin(); it != m_endpoints.end(); it++)
    {
        ezspRegisterEndpointStruct request;
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

        if (!sendFrame(EZSP_FRAME_REGISTER_ENDPOINT, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyStatus)
        {
            logWarning << "Endpoint" << QString::asprintf("0x%02x", it.key()) << "register request failed";
            continue;
        }

        logInfo << "Endpoint" << QString::asprintf("0x%02x", it.key()) << "registered successfully";
    }

    m_stackStatus = 0x00;

    if (!sendFrame(EZSP_FRAME_NETWORK_INIT, m_version < 14 ? QByteArray() : QByteArray(2, 0x00)))
    {
        logWarning << "Network init request failed";
        return false;
    }

    if (m_version < 14 && !m_replyStatus && !m_stackStatus && !waitForSignal(this, SIGNAL(stackStatusReceived()), ADAPTER_REQUEST_TIMEOUT))
    {
        logWarning << "Stack status handler timed out";
        return false;
    }

    if (!sendFrame(EZSP_FRAME_GET_NETWORK_PARAMETERS))
    {
        logWarning << "Network parameters request failed";
        return false;
    }

    memcpy(&network, m_replyData.constData() + statusOffset() + 2, sizeof(network));

    if (m_replyData.at(statusOffset() + 1) != 0x01 || network.extendedPanId != extendedPanId || network.panId != qToLittleEndian(m_panId) || network.channel != m_channel)
    {
        logWarning << "Adapter network parameters doesn't match configuration";
        check = true;
    }

    if (m_version < 13 ? !sendFrame(EZSP_FRAME_GET_KEY, QByteArray(1, 0x03)) : !sendFrame(EZSP_FRAME_EXPORT_KEY, QByteArray(1, 0x01).append(17, 0x00)))
    {
        logWarning << "Get network key request failed";
        return false;
    }

    if (m_replyData.mid(m_version == 13 ? 0 : 4, m_networkKey.length()) != m_networkKey)
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

        if (!startNetwork(extendedPanId))
        {
            logWarning << "Network startup failed";
            return false;
        }
    }

    for (int i = 0; i < m_multicast.length(); i++)
    {
        ezspAddGroupStruct request;

        request.groupId = qToLittleEndian(m_multicast.at(i));
        request.endpointId = m_multicast.at(i) == GREEN_POWER_GROUP ? 0xF2 : 0x01;
        request.index = 0x00;

        if (sendFrame(EZSP_FRAME_SET_MULTICAST_TABLE_ENTRY, QByteArray(1, static_cast <char> (i)).append(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyStatus)
            continue;

        logWarning << "Add group" << QString::asprintf("0x%04x", m_multicast.at(i)) << "request failed";
    }

    ieeeAddress = qToBigEndian(qFromLittleEndian(ieeeAddress));
    m_ieeeAddress = QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress));

    setManufacturerCode(MANUFACTURER_CODE_SILABS);
    m_ready = true;

    emit coordinatorReady();
    return true;
}

void EZSP::setManufacturerCode(quint16 value)
{
    value = qToLittleEndian(value);

    if (sendFrame(EZSP_FRAME_SET_MANUFACTURER_CODE, QByteArray(reinterpret_cast <char*> (&value), sizeof(value))))
        return;

    logWarning << "Set manufacturer code request failed";
}

void EZSP::handleError(const QString &reason)
{
    logWarning << reason.toUtf8().constData();

    m_errorCount = 0;
    m_errorReceived = true;

    emit dataReceived();

    m_reset = true;
    reset();
}

void EZSP::softReset(void)
{
    sendRequest(ASH_CONTROL_RST);
}

void EZSP::parseData(void)
{
    while (!m_buffer.isEmpty())
    {
        quint16 length, crc;
        QByteArray packet;

        if (m_buffer.length() < 4 || !m_buffer.contains(static_cast <char> (ASH_PACKET_FLAG)))
            return;

        length = static_cast <quint16> (m_buffer.indexOf(ASH_PACKET_FLAG));
        logDebug(m_portDebug) << "Frame received:" << m_buffer.mid(0, length + 1).toHex(':');

        for (int i = 0; i < length; i++)
        {
            if (m_buffer.at(i) == 0x11 || m_buffer.at(i) == 0x13)
                continue;

            if (m_buffer.at(i) != 0x7D)
            {
                packet.append(m_buffer.at(i));
                continue;
            }

            switch (m_buffer.at(++i))
            {
                case 0x31: packet.append(0x11); break;
                case 0x33: packet.append(0x13); break;
                case 0x38: packet.append(0x18); break;
                case 0x3A: packet.append(0x1A); break;
                case 0x5D: packet.append(0x7D); break;
                case 0x5E: packet.append(0x7E); break;

                default:

                    if (m_buffer.at(i) != 0x11 && m_buffer.at(i) != 0x13)
                    {
                        handleError(QString("Frame %1 unstaffing failed at position %2").arg(QString(m_buffer.mid(0, length + 1).toHex(':'))).arg(i));
                        m_buffer.clear();
                        return;
                    }

                    break;
            }
        }

        if (m_reset && packet.contains(0x1A))
            packet.remove(0, packet.indexOf(0x1A) + 1);

        memcpy(&crc, packet.constData() + packet.length() - 2, sizeof(crc));

        if (crc != getCRC(reinterpret_cast <quint8*> (packet.data()), packet.length() - 2))
        {
            handleError(QString("Packet %1 CRC mismatch").arg(QString(packet.toHex(':'))));
            m_buffer.clear();
            return;
        }

        m_queue.enqueue(packet);
        m_buffer.remove(0, length + 1);
    }
}

bool EZSP::permitJoin(bool enabled)
{
    if (enabled)
    {
        QByteArray request = QByteArray(8, 0xFF).append(m_defaultKey);
        ezspSetConfigStruct policy;

        if (m_version == 13)
            request.append(1, 0x00);

        if (!sendFrame(m_version < 13 ? EZSP_FRAME_ADD_TRANSIENT_LINK_KEY : EZSP_FRAME_IMPORT_TRANSIENT_KEY, request) || m_replyStatus)
        {
            logWarning << "Add transient key request failed";
            return false;
        }

        policy.id = EZSP_POLICY_TRUST_CENTER;
        policy.value = qToLittleEndian <quint16> (0x0003);

        if (!sendFrame(EZSP_FRAME_SET_POLICY, QByteArray(reinterpret_cast <char*> (&policy), sizeof(policy))) || m_replyStatus)
        {
            logWarning << "Set policy" << QString::asprintf("0x%02x", EZSP_POLICY_TRUST_CENTER) << "request failed";
            return false;
        }
    }

    if (!sendFrame(EZSP_FRAME_PERMIT_JOINING, QByteArray(1, enabled ? 0xF0 : 0x00)) || m_replyStatus)
    {
        logWarning << "Set permit join request failed";
        return false;
    }

    return true;
}

void EZSP::resetManufacturerCode(void)
{
    setManufacturerCode(MANUFACTURER_CODE_SILABS);
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
            logDebug(m_adapterDebug) << "Received NAK frame:" << QString::asprintf("%d, %d", m_acknowledgeId, control & 0x07).toUtf8().constData();
            emit dataReceived();
            break;
        }

        if (control == ASH_CONTROL_RSTACK)
        {
            m_sequenceId = 0;
            m_acknowledgeId = 0;
            m_reset = false;

            if (!startCoordinator())
            {
                logWarning << "Coordinator startup failed";
                break;
            }

            m_resetTimer->stop();
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
