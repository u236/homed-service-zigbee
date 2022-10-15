#include <QtEndian>
#include <QEventLoop>
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

static uint16_t CRC16_Calc(uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;

    while (length--)
        crc = (uint16_t) (crc << 8) ^ crcTable[(crc >> 8) ^ *data++];

    return crc;
}

EZSP::EZSP(QSettings *config, QObject *parent) : Adapter(config, parent), m_version(0)
{
    if (config->value("security/enabled", false).toBool())
        m_networkKey = QByteArray::fromHex(config->value("security/key", "000102030405060708090A0B0C0D0E0F").toString().remove("0x").toUtf8());

    m_config.insert(CONFIG_PACKET_BUFFER_COUNT, 0x00FF);
    m_config.insert(CONFIG_STACK_PROFILE, 0x0002);
    m_config.insert(CONFIG_SECURITY_LEVEL, 0x0005);

    m_policy.insert(POLICY_TRUST_CENTER, DECISION_ALLOW_JOINS | DECISION_ALLOW_UNSECURED_REJOINS);
    m_policy.insert(POLICY_BINDING_MODIFICATION, DECISION_DISALLOW_BINDING_MODIFICATION);
    m_policy.insert(POLICY_UNICAST_REPLIES, DECISION_HOST_WILL_NOT_SUPPLY_REPLY);
    m_policy.insert(POLICY_POLL_HANDLER, DECISION_POLL_HANDLER_IGNORE);
    m_policy.insert(POLICY_MESSAGE_CONTENTS_IN_CALLBACK, DECISION_MESSAGE_TAG_ONLY_IN_CALLBACK);
    m_policy.insert(POLICY_TC_KEY_REQUEST, DECISION_ALLOW_TC_KEY_REQUESTS);
    m_policy.insert(POLICY_APP_KEY_REQUEST, DECISION_ALLOW_APP_KEY_REQUESTS);
    m_policy.insert(POLICY_PACKET_VALIDATE_LIBRARY, DECISION_PACKET_VALIDATE_LIBRARY_CHECKS_DISABLED);
    m_policy.insert(POLICY_ZLL, DECISION_ALLOW_JOINS);
    m_policy.insert(POLICY_TC_REJOINS_USING_WELL_KNOWN_KEY, DECISION_ALLOW_JOINS);

    m_values.insert(VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE, QByteArray::fromHex("5200"));
    m_values.insert(VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE, QByteArray::fromHex("5200"));
    m_values.insert(VALUE_CCA_THRESHOLD, QByteArray(1, 0x00));
    m_values.insert(VALUE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE, QByteArray(1, 0x03));
}

void EZSP::reset(void)
{
    m_port->close();

    if (!m_port->open(QIODevice::ReadWrite))
    {
        logWarning << "Can't open port" << m_port->portName();
        return;
    }

    m_sequence = 0;
    m_ackNumber = 0;

    logInfo << "Resetting adapter...";
    sendRequest(ASH_CONTROL_RST);
}

void EZSP::setPermitJoin(bool enabled)
{
    QByteArray data = QByteArray::fromHex("FFFFFFFFFFFFFFFF5A6967426565416C6C69616E63653039");

    if (enabled && (!sendFrame(FRAME_ADD_TRANSIENT_LINK_KEY, data) || m_replyData.at(0)))
    {
        logWarning << "Add transient key request failed";
        return;
    }

    if (!sendFrame(FRAME_PERMIT_JOINING, QByteArray(1, enabled ? 0xFF : 0x00)) || m_replyData.at(0))
    {
        logWarning << "Set permit join request failed";
        return;
    }

    logInfo << "Permit join" << (enabled ? "enabled" : "disabled") << "successfully";
}

void EZSP::nodeDescriptorRequest(quint16 networkAddress)
{
    Q_UNUSED(networkAddress)
}

void EZSP::simpleDescriptorRequest(quint16 networkAddress, quint8 endpointId)
{
    Q_UNUSED(networkAddress)
    Q_UNUSED(endpointId)
}

void EZSP::activeEndpointsRequest(quint16 networkAddress)
{
    Q_UNUSED(networkAddress)
}

void EZSP::lqiRequest(quint16 networkAddress, quint8 index)
{
    Q_UNUSED(networkAddress)
    Q_UNUSED(index)
}

bool EZSP::bindRequest(quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind)
{
    Q_UNUSED(networkAddress)
    Q_UNUSED(srcAddress)
    Q_UNUSED(srcEndpointId)
    Q_UNUSED(clusterId)
    Q_UNUSED(dstAddress)
    Q_UNUSED(dstEndpointId)
    Q_UNUSED(unbind)

    return true;
}

bool EZSP::dataRequest(quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &data)
{
    Q_UNUSED(networkAddress)
    Q_UNUSED(endpointId)
    Q_UNUSED(clusterId)
    Q_UNUSED(data)

    return true;
}

bool EZSP::extendedDataRequest(const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group)
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

quint8 EZSP::dataRequestStatus(void)
{
    return 0x00;
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

void EZSP::randomize(QByteArray &payload)
{
    quint8 *buffer = reinterpret_cast <quint8*> (payload.data()), byte = 0x42;

    for (int i = 0; i < payload.length(); i++)
    {
        buffer[i] ^= byte;
        byte = byte & 0x01 ? (byte >> 1) ^ 0xB8 : byte >> 1;
    }
}

bool EZSP::sendFrame(quint16 frameId, const QByteArray &data)
{
    QByteArray payload;
    quint8 control = (m_sequence & 0x07) << 4 | (m_ackNumber & 0x07);

    m_sequence++;

    m_frameId = frameId;
    m_replyStatus = false;

    if (frameId)
    {
        ezspHeaderStruct header;

        header.sequence = m_sequence;
        header.frameControlLow = 0x00;
        header.frameControlHigh = 0x01;
        header.frameId = qToLittleEndian(frameId);

        payload.append(reinterpret_cast <char*> (&header), sizeof(header));
    }
    else
    {
        payload.append(static_cast <char> (m_sequence));
        payload.append(2, 0x00);
    }

    payload.append(data);
    randomize(payload);

    return sendRequest(control, payload) && m_replyStatus;
}

bool EZSP::sendRequest(quint8 control, const QByteArray &payload)
{
    QByteArray request, buffer;
    quint16 crc;

    request.append(static_cast <quint8> (control));
    request.append(payload);

    crc = qToBigEndian(CRC16_Calc(reinterpret_cast <quint8*> (request.data()), request.length()));
    request.append(reinterpret_cast <char*> (&crc), sizeof(crc));

    for(int i = 0; i < request.length(); i++)
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

    return transmitData(buffer.append(ASH_FLAG_BYTE), control & 0x80 ? false : true);
}

void EZSP::parsePacket(const QByteArray &payload)
{
    const ezspHeaderStruct *header = reinterpret_cast <const ezspHeaderStruct*> (payload.constData());
    quint16 frameId;
    QByteArray data;

    if (header->sequence != m_sequence)
        return;

    if (!(header->frameControlHigh & 0x01))
    {
        m_version = static_cast <quint8> (payload.at(3));
        m_replyStatus = true;
        return;
    }

    frameId = qFromLittleEndian(header->frameId);
    data = payload.mid(sizeof(ezspHeaderStruct));

    if (!(header->frameControlLow & 0x18))
    {
        m_replyStatus = frameId == m_frameId;
        m_replyData = data;
        return;
    }

    switch (frameId)
    {
        case FRAME_STACK_STATUS_HANDLER:
        {
            m_stackStatus = static_cast <quint8> (data.at(0));
            emit stackStatusReceived();
            break;
        }

        case FRAME_TRUST_CENTER_JOIN_HANDLER:
        {
            const trustCenterJoinStruct *message = reinterpret_cast <const trustCenterJoinStruct*> (data.constData());
            quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->ieeeAddress));

            if (message->status == EMBER_DEVICE_LEFT)
            {
                emit deviceLeft(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->networkAddress));
                break;
            }

            if (message->status == EMBER_STANDARD_SECURITY_UNSECURED_JOIN)
            {
                sendFrame(FRAME_FIND_KEY_TABLE_ENTRY, QByteArray(reinterpret_cast <const char*> (&message->ieeeAddress), sizeof(message->ieeeAddress)).append(1, 0x01));
                sendFrame(FRAME_ERASE_KEY_TABLE_ENTRY, m_replyData);
            }

            emit deviceJoined(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->networkAddress));
            break;
        }

        default:
        {
            logInfo << "callback:" << QString::asprintf("0x%04X", frameId) << data.toHex(':');
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

    if (m_replyData.at(0) == static_cast <char> (EMBER_JOINED_NETWORK))
    {
        m_stackStatus = 0x00;

        if (!sendFrame(FRAME_LEAVE_NETWORK) || m_replyData.at(0))
        {
            logWarning << "Leave network request failed" << m_replyStatus << m_replyData.toHex(':');
            return false;
        }

        if (!m_replyData.at(0) && !m_stackStatus && !waitForSignal(this, SIGNAL(stackStatusReceived()), ADAPTER_REQUEST_TIMEOUT))
        {
            logWarning << "Stack status handler timed out";
            return false;
        }

        if (m_stackStatus != EMBER_NETWORK_DOWN)
        {
            logWarning << "Unexpected stack status" << QString::asprintf("0x%02X", m_stackStatus) << "received";
            return false;
        }
    }

    if (!sendFrame(FRAME_CLEAR_TRANSIENT_LINK_KEYS) || !sendFrame(FRAME_CLEAR_KEY_TABLE) || m_replyData.at(0))
    {
        logWarning << "Clear key table request failed";
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

    if (!sendFrame(FRAME_SET_INITIAL_SECURITY_STATE, QByteArray(reinterpret_cast <char*> (&security), sizeof(security))) || m_replyData.at(0))
    {
        logWarning << "Set initial security state request failed";
        return false;
    }

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
    {
        logWarning << "Set value item" << QString::asprintf("0x%02X", VALUE_STACK_TOKEN_WRITING) << "request failed" << m_replyData.toHex(':');
        return false;
    }

    if (!sendFrame(FRAME_NETWORK_STATUS))
    {
        logWarning << "Network status request failed";
        return false;
    }

    if (m_replyData.at(0) != static_cast <char> (EMBER_JOINED_NETWORK))
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
    quint64 ieeeAddress;

    if (!sendFrame(FRAME_VERSION, QByteArray(1, 0x08)))
    {
        logWarning << "Adapter version request failed";
        return false;
    }

    if (m_version < 8)
    {
        logWarning << "Unsupported EZSP version" << m_version << "adapter detected";
        return false;
    }

    logInfo << "EZSP" << QString::asprintf("v%d", m_version).toUtf8().constData() << "adapter detected";

    if (!sendFrame(FRAME_GET_IEEE_ADDRESS) || m_replyData.length() != sizeof(m_ieeeAddress))
    {
        logWarning << "Adapter address request failed";
        return false;
    }

    memcpy(&m_ieeeAddress, m_replyData.constData(), sizeof(m_ieeeAddress));

    for (auto it = m_config.begin(); it != m_config.end(); it++)
    {
        setConfigStruct data;

        data.id = it.key();
        data.value = qToLittleEndian(it.value());

        if (!sendFrame(FRAME_SET_CONFIG, QByteArray(reinterpret_cast <char*> (&data), sizeof(data))) || m_replyData.at(0))
        {
            logWarning << "Set config item" << QString::asprintf("0x%02X", it.key()) << "request failed";
            return false;
        }
    }

    for (auto it = m_policy.begin(); it != m_policy.end(); it++)
    {
        setPolicyStruct data;

        data.id = it.key();
        data.decision = it.value();

        if (!sendFrame(FRAME_SET_POLICY, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(1, 0x00)) || m_replyData.at(0))
        {
            logWarning << "Set policy item" << QString::asprintf("0x%02X", it.key()) << "request failed";
            return false;
        }
    }

    for (auto it = m_values.begin(); it != m_values.end(); it++)
    {
        setValueStruct data;

        data.id = it.key();
        data.length = static_cast <quint8> (it.value().length());

        if (!sendFrame(FRAME_SET_VALUE, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(it.value())) || m_replyData.at(0))
        {
            logWarning << "Set value item" << QString::asprintf("0x%02X", it.key()) << "request failed";
            return false;
        }
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
            logWarning << "Endpoint 0x01 register request failed" << m_replyStatus << m_replyData.toHex(':');
            return false;
        }

        logInfo << "Endpoint" << QString::asprintf("0x%02X", it.key()) << "registered successfully";
    }

    concentrator.enabled = 0x01;
    concentrator.type = qToLittleEndian(CONCENTRATOR_HIGH_RAM);
    concentrator.minTime = qToLittleEndian(MTOR_MIN_TIME);
    concentrator.maxTime = qToLittleEndian(MTOR_MAX_TIME);
    concentrator.routeErrorThreshold = MTOR_ROUTE_ERROR_THRESHOLD;
    concentrator.deliveryFailureThreshold = MTOR_DELIVERY_FAILURE_THRESHOLD;
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

    m_stackStatus = 0x00;

    if (!sendFrame(FRAME_NETWORK_INIT, QByteArray(2, 0x00)))
    {
        logWarning << "Network init request failed";
        return false;
    }

    if (!m_replyData.at(0) && !m_stackStatus && !waitForSignal(this, SIGNAL(stackStatusReceived()), ADAPTER_REQUEST_TIMEOUT))
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

    if (m_replyData.at(1) != 0x01 || network.extendedPanId != m_ieeeAddress || network.panId != qToLittleEndian(m_panId) || network.channel != m_channel || m_stackStatus != EMBER_NETWORK_UP)
    {
        // TODO: check network key

        if (!m_write)
        {
            logWarning << "Write protected";
            return false;
        }

        if (!startNetwork())
        {
            logWarning << "Network starup failed";
            return false;
        }
    }

    ieeeAddress = qToBigEndian(qFromLittleEndian(m_ieeeAddress));
    emit coordinatorReady(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)));

    return true;
}

void EZSP::receiveData(void)
{
    QByteArray buffer = m_port->readAll();

    while (!buffer.isEmpty())
    {
        QByteArray data;
        quint8 length, control;
        quint16 crc;

        if (buffer.length() < 4 || !buffer.contains(static_cast <char> (ASH_FLAG_BYTE)))
            return;

        if (buffer.startsWith(QByteArray::fromHex("1AC102")) || buffer.startsWith(QByteArray::fromHex("1AC202")))
            buffer.remove(0, 1);

        length = static_cast <quint8> (buffer.indexOf(ASH_FLAG_BYTE));

        for(int i = 0; i < length; i++)
        {
            if (buffer.at(i) == static_cast <char> (0x7D))
            {
                switch (buffer.at(++i))
                {
                    case 0x31: data.append(0x11); break;
                    case 0x33: data.append(0x13); break;
                    case 0x38: data.append(0x18); break;
                    case 0x3A: data.append(0x1A); break;
                    case 0x5D: data.append(0x7D); break;
                    case 0x5E: data.append(0x7E); break;

                    default:
                        logWarning << "Packet" << buffer.toHex(':') << "unstaffing failed at position" << i;
                        reset();
                        return;
                }
            }
            else
                data.append(buffer.at(i));
        }

        memcpy(&crc, data.constData() + data.length() - 2, sizeof(crc));

        if (crc != qToBigEndian(CRC16_Calc(reinterpret_cast <quint8*> (data.data()), data.length() - 2)))
        {
            logWarning << "Packet" << buffer.toHex(':') << "CRC mismatch";
            reset();
            return;
        }

        control = static_cast <quint8> (data.front());
        buffer.remove(0, length + 1);

        if (!(control & 0x80))
        {
            QByteArray payload = data.mid(1, data.length() - 3);

            randomize(payload);
            parsePacket(payload);

            sendRequest(ASH_CONTROL_ACK | (++m_ackNumber & 0x07));
            continue;
        }

        if (control == ASH_CONTROL_RSTACK)
        {
            if (!startCoordinator())
                logWarning << "Coordinator startup failed";

            continue;
        }

        logInfo << "ASH" << data.toHex(':');
    }
}
