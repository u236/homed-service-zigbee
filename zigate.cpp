#include <QtEndian>
#include "logger.h"
#include "zigate.h"

ZiGate::ZiGate(QSettings *config, QObject *parent) : Adapter(config, parent)
{
    m_networkKeyEnabled = config->value("security/enabled", false).toBool();
    m_networkKey = QByteArray::fromHex(config->value("security/key", "000102030405060708090a0b0c0d0e0f").toString().remove("0x").toUtf8());
}

bool ZiGate::unicastRequest(quint8 id, quint16 networkAddress, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    return apsRequest(id, ADDRESS_MODE_16_BIT, networkAddress, srcEndPointId, dstEndPointId, clusterId, payload);
}

bool ZiGate::multicastRequest(quint8 id, quint16 groupId, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    return apsRequest(id, ADDRESS_MODE_GROUP, groupId, srcEndPointId, dstEndPointId, clusterId, payload);
}

bool ZiGate::unicastInterPanRequest(quint8, const QByteArray &, quint16, const QByteArray &)
{
    return false;
}

bool ZiGate::broadcastInterPanRequest(quint8, quint16, const QByteArray &)
{
    return false;
}

bool ZiGate::setInterPanChannel(quint8)
{
    return false;
}

void ZiGate::resetInterPanChannel(void)
{

}

bool ZiGate::zdoRequest(quint8 id, quint16 networkAddress, quint16 clusterId, const QByteArray &data)
{
    quint16 command, dstAddress = qToBigEndian(networkAddress);

    switch (clusterId)
    {
        case ZDO_NODE_DESCRIPTOR_REQUEST:   command = ZIGATE_NODE_DESCRIPTOR_REQUEST; break;
        case ZDO_SIMPLE_DESCRIPTOR_REQUEST: command = ZIGATE_SIMPLE_DESCRIPTOR_REQUEST; break;
        case ZDO_ACTIVE_ENDPOINTS_REQUEST:  command = ZIGATE_ACTIVE_ENDPOINTS_REQUEST; break;
        case ZDO_LQI_REQUEST:               command = ZIGATE_LQI_REQUEST; break;
        default: return false;
    }

    return sendRequest(command, QByteArray(reinterpret_cast <char*> (&dstAddress), sizeof(dstAddress)).append(data), id) && !m_replyStatus;
}

bool ZiGate::bindRequest(quint8 id, quint16, quint8 endpointId, quint16 clusterId, const QByteArray &address, quint8 dstEndpointId, bool unbind)
{
    QByteArray buffer = address.isEmpty() ? m_ieeeAddress : address;
    bindRequestStruct request;
    quint64 dstAddress;

    memcpy(&request.srcAddress, m_requestAddress.constData(), sizeof(request.srcAddress));
    memcpy(&dstAddress, buffer.constData(), sizeof(dstAddress));

    request.srcEndpointId = endpointId;
    request.clusterId = qToBigEndian(clusterId);
    request.dstAddressMode = buffer.length() == 2 ? ADDRESS_MODE_GROUP : ADDRESS_MODE_64_BIT;

    if (request.dstAddressMode == ADDRESS_MODE_GROUP)
    {
        quint16 value = qToBigEndian <quint16> (qFromLittleEndian(dstAddress));
        memcpy(&dstAddress, &value, sizeof(value));
    }

    return sendRequest(unbind ? ZIGATE_UNBIND_REQUEST : ZIGATE_BIND_REQUEST, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(reinterpret_cast <char*> (&dstAddress), request.dstAddressMode == ADDRESS_MODE_GROUP ? 2 : 8).append(static_cast <char> (dstEndpointId ? dstEndpointId : 1)), id) && !m_replyStatus;
}

bool ZiGate::leaveRequest(quint8 id, quint16 networkAddress)
{
    quint16 dstAddress = qToBigEndian(networkAddress);
    return sendRequest(ZIGATE_LEAVE_REQUEST, QByteArray(reinterpret_cast <char*> (&dstAddress), sizeof(dstAddress)).append(m_requestAddress).append(2, 0x00), id) && !m_replyStatus;
}

bool ZiGate::lqiRequest(quint8 id, quint16 networkAddress, quint8 index)
{
    quint16 data = qToBigEndian(networkAddress);
    return sendRequest(ZIGATE_LQI_REQUEST, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(static_cast <quint8> (index)), id) && !m_replyStatus;
}

quint8 ZiGate::getChecksum(const zigateHeaderStruct *header, const QByteArray &payload)
{
    QByteArray data = QByteArray(reinterpret_cast <const char*> (header), 4).append(payload);
    quint8 checksum = 0;

    for (int i = 0; i < data.length(); i++)
        checksum ^= data.at(i);

    return checksum;
}

QByteArray ZiGate::encodeFrame(const QByteArray &data)
{
    QByteArray frame = QByteArray(1, 0x01);

    for (int i = 0; i < data.length(); i++)
    {
        if (data.at(i) < 0x10)
            frame.append(1, 0x02);

        frame.append(1, data.at(i) < 0x10 ? data.at(i) ^ 0x10 : data.at(i));
    }

    return frame.append(1, 0x03);
}

bool ZiGate::sendRequest(quint16 command, const QByteArray &data, quint8 id)
{
    zigateHeaderStruct header;
    QByteArray payload;

    if (m_adapterDebug)
        logInfo << "-->" << QString::asprintf("0x%04x", command) << data.toHex(':');

    m_commandReply = data.isEmpty();
    m_command = command;

    m_replyStatus = 0xFF;
    m_replyData.clear();
    m_requestId = id;

    if (!data.isEmpty())
        payload = QByteArray(data).append(1, 0x00);

    header.command = qToBigEndian(command);
    header.length = qToBigEndian <quint16> (payload.length());
    header.checksum = getChecksum(&header, payload);

    sendData(encodeFrame(QByteArray(reinterpret_cast <char*> (&header), sizeof(header)).append(payload)));

    if (command == ZIGATE_RESET || command == ZIGATE_ERASE_PERSISTENT_DATA)
        return true;

    return waitForSignal(this, SIGNAL(dataReceived()), ZIGATE_REQUEST_TIMEOUT);
}

void ZiGate::parsePacket(quint16 command, const QByteArray &payload)
{
    if (m_adapterDebug)
        logInfo << "<--" << QString::asprintf("0x%04x", command) << payload.toHex(':');

    if (command == (m_command | 0x8000))
    {
        m_replyData = payload.mid(0, payload.length() - 1);

        if (m_commandReply)
            emit dataReceived();

        return;
    }

    switch (command)
    {
        case ZIGATE_STATUS:
        {
            const statusStruct *data = reinterpret_cast <const statusStruct*> (payload.constData());

            if (m_command == qFromBigEndian(data->command))
            {
                m_replyStatus = data->status;

                if (m_commandReply)
                    break;

                if (!m_replyStatus)
                    m_requests.insert(data->sequence, m_requestId);

                emit dataReceived();
            }

            break;
        }

        case ZIGATE_DATA_INDICATION:
        {
            const dataIndicatonStruct *message = reinterpret_cast <const dataIndicatonStruct*> (payload.constData());
            quint8 offset = static_cast <quint8> (sizeof(dataIndicatonStruct));
            quint16 networkAddress;

            if (payload.at(offset) != ADDRESS_MODE_16_BIT || (payload.at(offset + 3) != ADDRESS_MODE_GROUP && payload.at(offset + 3) != ADDRESS_MODE_16_BIT))
            {
                logWarning << "Unsupported address mode in incoming message:" << payload.toHex(':');
                break;
            }

            memcpy(&networkAddress, payload.constData() + offset + 1, sizeof(networkAddress));
            offset += 6;

            if (!message->profileId)
            {
                auto it = m_requests.find(payload.at(offset++));

                if (it != m_requests.end())
                {
                    emit requestFinished(it.value(), message->status);
                    m_requests.erase(it);
                }

                emit zdoMessageReveived(qFromBigEndian(networkAddress), qFromBigEndian(message->clusterId), payload.mid(offset));
                break;
            }

            emit zclMessageReveived(qFromBigEndian(networkAddress), message->srcEndpointId, qFromBigEndian(message->clusterId), static_cast <quint8> (payload.back()), payload.mid(offset));
            break;
        }

        case ZIGATE_RESTART_NON_FACTORY:
        case ZIGATE_RESTART_FACTORY:
        {

            if (!startCoordinator(command == ZIGATE_RESTART_FACTORY))
            {
                logWarning << "Coordinator startup failed";
                break;
            }

            m_resetTimer->stop();
            break;
        }

        case ZIGATE_DATA_ACK:
        {
            const dataAckStruct *message = reinterpret_cast <const dataAckStruct*> (payload.constData());
            auto it = m_requests.find(message->sequence);

            if (it != m_requests.end())
            {
                emit requestFinished(it.value(), message->status);
                m_requests.erase(it);
            }

            break;
        }

        case ZIGATE_DEVICE_ANNOUNCE:
        {
            const deviceAnnounceStruct *announce = reinterpret_cast <const deviceAnnounceStruct*> (payload.constData());
            emit deviceJoined(QByteArray(reinterpret_cast <const char*> (&announce->ieeeAddress), sizeof(announce->ieeeAddress)), qFromBigEndian(announce->networkAddress));
            break;
        }

        case ZIGATE_DEVICE_LEAVE_INDICATION:
        {
            emit deviceLeft(payload.mid(0, sizeof(quint64)));
            break;
        }
    }
}

bool ZiGate::startCoordinator(bool clear)
{
    networkStatusStruct networkStatus;
    quint32 channelList = qToBigEndian <quint32> (1 << m_channel);

    if (!sendRequest(ZIGATE_SET_RAW_MODE, QByteArray(1, 0x01)) || m_replyStatus)
    {
        logWarning << "Set raw mode request failed";
        return false;
    }

    if (!sendRequest(ZIGATE_GET_VERSION) || m_replyStatus)
    {
        logWarning << "Adapter version request failed";
        return false;
    }

    if (m_replyData.at(2) != 3 || m_replyData.at(3) < 0x1B)
    {
        logWarning << "Unsupported ZiGate version:" << m_replyData.toHex(':');
        return false;
    }

    m_manufacturerName = "NXP";
    m_modelName = "ZiGate";
    m_firmware = QString::asprintf("%x.%x", m_replyData.at(2), m_replyData.at(3));

    logInfo << "Adapter type: ZiGate" << m_firmware.toUtf8().constData();

    if (!sendRequest(ZIGATE_GET_NETWORK_STATUS) || m_replyStatus)
    {
        logWarning << "Network status request failed";
        return false;
    }

    memcpy(&networkStatus, m_replyData.constData(), sizeof(networkStatus));
    m_ieeeAddress = QByteArray(reinterpret_cast <char*> (&networkStatus.ieeeAddress), sizeof(networkStatus.ieeeAddress));

    if (clear && (!sendRequest(ZIGATE_SET_EXTENDED_PANID, m_ieeeAddress) || m_replyStatus))
    {
        logWarning << "Set extended PAN ID request failed";
        return false;
    }

    if (!sendRequest(ZIGATE_SET_CHANNEL_LIST, QByteArray(reinterpret_cast <char*> (&channelList), sizeof(channelList))) || m_replyStatus)
    {
        logWarning << "Set channel list request failed";
        return false;
    }

    if (!sendRequest(ZIGATE_SET_NETWORK_KEY, QByteArray(1, m_networkKeyEnabled ? 0x02 : 0x01).append(m_networkKey)) || m_replyStatus)
    {
        logWarning << "Set network key request failed";
        return false;
    }

    if (!sendRequest(ZIGATE_SET_LOGICAL_TYPE, QByteArray(1, static_cast <char> (LogicalType::Coordinator))) || m_replyStatus)
    {
        logWarning << "Set adapter logical type request failed";
        return false;
    }

    if (!sendRequest(ZIGATE_START_NETWORK) || m_replyStatus)
    {
        logWarning << "Start network request failed";
        return false;
    }

    if (clear && sendRequest(ZIGATE_GET_NETWORK_STATUS) && !m_replyStatus)
    {
        memcpy(&networkStatus, m_replyData.constData(), sizeof(networkStatus));
        logInfo << "New network started";
    }

    for (int i = 0; i < m_multicast.length(); i++)
    {
        addGroupStruct request;

        request.addressMode = ADDRESS_MODE_16_BIT;
        request.address = 0x0000;
        request.srcEndpointId = 0x01;
        request.dstEndpointId = 0x01;
        request.groupId = qToBigEndian(m_multicast.at(i));

        if (sendRequest(ZIGATE_ADD_GROUP, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || !m_replyStatus)
            continue;

        logWarning << "Add group" << QString::asprintf("0x%04x", m_multicast.at(i)) << "request failed";
    }

    logInfo << "ZiGate managed PAN ID:" << QString::asprintf("0x%04x", qFromBigEndian(networkStatus.panId));
    emit coordinatorReady();
    return true;
}

bool ZiGate::apsRequest(quint8 id, quint8 addressMode, quint16 address, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    apsRequestStruct request;

    request.addressMode = addressMode;
    request.address = qToBigEndian(address);
    request.srcEndpointId = srcEndPointId;
    request.dstEndpointId = dstEndPointId;
    request.clusterId = qToBigEndian(clusterId);
    request.profileId = qToBigEndian <quint16> (PROFILE_HA);
    request.securityMode = ZIGATE_SECURITY_MODE;
    request.radius = ZIGATE_RADIUS;
    request.length = static_cast <quint8> (payload.length());

    return sendRequest(ZIGATE_APS_REQUEST, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(payload), id) && !m_replyStatus;
}

void ZiGate::softReset(void)
{
    sendRequest(ZIGATE_RESET);
}

void ZiGate::parseData(QByteArray &buffer)
{
    while (!buffer.isEmpty())
    {
        int length = buffer.indexOf(0x03);
        QByteArray frame, data;

        if (!buffer.startsWith(0x01) || length < 6)
            return;

        if (m_portDebug)
            logInfo << "Packet received:" << buffer.mid(0, length + 1).toHex(':');

        frame = buffer.mid(1, length - 1);

        for (int i = 0; i < frame.length(); i++)
            data.append(1, frame.at(i) == 0x02 ? frame.at(++i) ^ 0x10 : frame.at(i));

        m_queue.enqueue(data);
        buffer.remove(0, length + 1);
    }
}

bool ZiGate::permitJoin(bool enabled)
{
    if (!sendRequest(ZIGATE_SET_PERMIT_JOIN, QByteArray(2, 0x00).append(1, enabled ? 0xF0 : 0x00)) || m_replyStatus)
    {
        logWarning << "Set permit join request failed";
        return false;
    }

    return true;
}

void ZiGate::handleQueue(void)
{
    while (!m_queue.isEmpty())
    {
        QByteArray data = m_queue.dequeue(), payload;
        const zigateHeaderStruct *header = reinterpret_cast <const zigateHeaderStruct*> (data.constData());

        payload = data.mid(sizeof(zigateHeaderStruct), qFromBigEndian(header->length));

        if (getChecksum(header, payload) != header->checksum)
        {
            logWarning << "Packet" << data.toHex(':') << "checksum mismatch";
            continue;
        }

        parsePacket(qFromBigEndian(header->command), payload);
    }
}
