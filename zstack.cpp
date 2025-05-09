#include <QtEndian>
#include <QThread>
#include "logger.h"
#include "zstack.h"

ZStack::ZStack(QSettings *config, QObject *parent) : Adapter(config, parent), m_status(0), m_clear(false)
{
    quint32 channelList = qToLittleEndian <quint32> (1 << m_channel);

    m_nvItems.insert(ZCD_NV_PRECFGKEY,         m_networkKey);
    m_nvItems.insert(ZCD_NV_PRECFGKEYS_ENABLE, QByteArray(1, 0x01));
    m_nvItems.insert(ZCD_NV_PANID,             QByteArray(reinterpret_cast <char*> (&m_panId), sizeof(m_panId)));
    m_nvItems.insert(ZCD_NV_CHANLIST,          QByteArray(reinterpret_cast <char*> (&channelList), sizeof(channelList)));
    m_nvItems.insert(ZCD_NV_LOGICAL_TYPE,      QByteArray(1, 0x00));
    m_nvItems.insert(ZCD_NV_ZDO_DIRECT_CB,     QByteArray(1, 0x01));

    m_zdoClusters = {ZDO_NODE_DESCRIPTOR_REQUEST, ZDO_SIMPLE_DESCRIPTOR_REQUEST, ZDO_ACTIVE_ENDPOINTS_REQUEST, ZDO_BIND_REQUEST, ZDO_UNBIND_REQUEST, ZDO_LQI_REQUEST, ZDO_LEAVE_REQUEST};
}

bool ZStack::unicastRequest(quint8 id, quint16 networkAddress, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    zstackDataRequestStruct request;

    request.networkAddress = qToLittleEndian(networkAddress);
    request.dstEndpointId = dstEndPointId;
    request.srcEndpointId = srcEndPointId;
    request.clusterId = qToLittleEndian(clusterId);
    request.transactionId = id;
    request.options = ZSTACK_AF_DISCV_ROUTE;
    request.radius = ZSTACK_AF_DEFAULT_RADIUS;
    request.length = static_cast <quint8> (payload.length());

    return sendRequest(ZSTACK_AF_DATA_REQUEST, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(payload)) && !m_replyStatus;
}

bool ZStack::multicastRequest(quint8 id, quint16 groupId, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    return extendedRequest(id, groupId, dstEndPointId, 0x0000, srcEndPointId, clusterId, payload, true);
}

bool ZStack::unicastInterPanRequest(quint8 id, const QByteArray &ieeeAddress, quint16 clusterId, const QByteArray &payload)
{
    return extendedRequest(id, ieeeAddress, 0xFE, 0xFFFF, 0x0C, clusterId, payload);
}

bool ZStack::broadcastInterPanRequest(quint8 id, quint16 clusterId, const QByteArray &payload)
{
    return extendedRequest(id, 0xFFFF, 0xFE, 0xFFFF, 0x0C, clusterId, payload);
}

bool ZStack::setInterPanChannel(quint8 channel)
{
    if (!sendRequest(ZSTACK_AF_INTER_PAN_CTL, QByteArray(1, 0x01).append(static_cast <char> (channel))) || m_replyStatus)
    {
        logWarning << "Set Inter-PAN channel" << channel << "request failed";
        return false;
    }

    return true;
}

void ZStack::resetInterPanChannel(void)
{
    if (sendRequest(ZSTACK_AF_INTER_PAN_CTL, QByteArray(1, 0x00)) && !m_replyStatus)
        return;

    logWarning << "Reset Inter-PAN request failed";
}

bool ZStack::extendedRequest(quint8 id, const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &payload, bool group)
{
    zstackExtendedRequestStruct data;

    switch (address.length())
    {
        case 2:  data.dstAddressMode = group ? ADDRESS_MODE_GROUP : ADDRESS_MODE_16_BIT; break;
        case 8:  data.dstAddressMode = ADDRESS_MODE_64_BIT; break;
        default: return false;
    }

    memset(&data.dstAddress, 0, sizeof(data.dstAddress));
    memcpy(&data.dstAddress, address.constData(), address.length());

    if (data.dstAddressMode == ADDRESS_MODE_64_BIT)
        data.dstAddress = qToLittleEndian(qFromBigEndian(data.dstAddress));

    data.dstEndpointId = dstEndpointId;
    data.dstPanId = qToLittleEndian(dstPanId);
    data.srcEndpointId = srcEndpointId;
    data.clusterId = qToLittleEndian(clusterId);
    data.transactionId = id;
    data.options = 0x00;
    data.radius = dstPanId ? ZSTACK_AF_DEFAULT_RADIUS * 2 : ZSTACK_AF_DEFAULT_RADIUS;
    data.length = qToLittleEndian <quint16> (payload.length());

    return sendRequest(ZSTACK_AF_DATA_REQUEST_EXT, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(payload)) && !m_replyStatus;
}

bool ZStack::extendedRequest(quint8 id, quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &paylaod, bool group)
{
    address = qToLittleEndian(address);
    return extendedRequest(id, QByteArray(reinterpret_cast <char*> (&address), sizeof(address)), dstEndpointId, dstPanId, srcEndpointId, clusterId, paylaod, group);
}

bool ZStack::sendRequest(quint16 command, const QByteArray &data)
{
    QByteArray request;
    quint8 fcs = 0;

    logDebug(m_adapterDebug) << "-->" << QString::asprintf("0x%04x", command) << data.toHex(':');

    m_command = qToBigEndian(command);
    m_replyStatus = 0xFF;

    request.append(ZSTACK_PACKET_FLAG);
    request.append(static_cast <char> (data.length()));
    request.append(reinterpret_cast <char*> (&m_command), sizeof(m_command));
    request.append(data);

    for (int i = 1; i < request.length(); i++)
        fcs ^= request[i];

    sendData(request.append(static_cast <char> (fcs)));
    return waitForSignal(this, SIGNAL(dataReceived()), ZSTACK_REQUEST_TIMEOUT);
}

void ZStack::parsePacket(quint16 command, const QByteArray &data)
{
    logDebug(m_adapterDebug) << "<--" << QString::asprintf("0x%04x", command) << data.toHex(':');

    if (command & 0x2000)
    {
        if ((command ^ 0x4000) == qFromBigEndian(m_command))
        {
            m_replyStatus = static_cast <quint8> (data.at(0));
            m_replyData = data;
            emit dataReceived();
        }

        return;
    }

    switch (command)
    {
        case ZSTACK_ZDO_MGMT_PERMIT_JOIN_RSP:
        case ZSTACK_ZDO_MGMT_NWK_UPDATE_RSP:
        case ZSTACK_ZDO_SRC_RTG_IND:
        case ZSTACK_ZDO_CONCENTRATOR_IND:
        case ZSTACK_ZDO_TC_DEV_IND:
        case ZSTACK_ZDO_PERMIT_JOIN_IND:
            break;

        case ZSTACK_SYS_RESET_IND:
        {
            if (!startCoordinator())
            {
                logWarning << "Coordinator startup failed";
                break;
            }

            m_resetTimer->stop();
            break;
        }

        case ZSTACK_AF_DATA_CONFIRM:
        {
            const zstackDataConfirmStruct *message = reinterpret_cast <const zstackDataConfirmStruct*> (data.constData());
            emit requestFinished(message->transactionId, message->status);
            break;
        }

        case ZSTACK_AF_INCOMING_MSG:
        {
            const zstackIncomingMessageStruct *message = reinterpret_cast <const zstackIncomingMessageStruct*> (data.constData());
            emit zclMessageReveived(qFromLittleEndian(message->srcAddress), message->srcEndpointId, qFromLittleEndian(message->clusterId), message->linkQuality, data.mid(sizeof(zstackIncomingMessageStruct), message->length));
            break;
        }

        case ZSTACK_AF_INCOMING_MSG_EXT:
        {
            const zstackExtendedMessageStruct *message = reinterpret_cast <const zstackExtendedMessageStruct*> (data.constData());
            quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->srcAddress));

            if (message->srcAddressMode != 0x03)
            {
                logWarning << "Unsupported extended message address mode" << QString::asprintf("0x%02x", message->srcAddressMode);
                return;
            }

            emit rawMessageReveived(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->clusterId), message->linkQuality, data.mid(sizeof(zstackExtendedMessageStruct), message->length));
            break;
        }

        case ZSTACK_ZDO_STATE_CHANGE_IND:
        {
            if (data.length() == 1)
                m_status = static_cast <quint8> (data.at(0));

            if (m_version == ZStackVersion::ZStack12x && m_status == ZSTACK_COORDINATOR_STARTED)
            {
                m_ready = true;
                emit coordinatorReady();
            }

            break;
        }

        case ZSTACK_ZDO_END_DEVICE_ANNCE_IND:
        {
            const deviceAnnounceStruct *message = reinterpret_cast <const deviceAnnounceStruct*> (data.constData() + 2);
            quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->ieeeAddress));
            emit deviceJoined(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->networkAddress));
            break;
        }

        case ZSTACK_ZDO_LEAVE_IND:
        {
            const zstackDeviceLeaveStruct *message = reinterpret_cast <const zstackDeviceLeaveStruct*> (data.constData());
            quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->ieeeAddress));
            emit deviceLeft(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)));
            break;
        }

        case ZSTACK_ZDO_MSG_CB_INCOMING:
        {
            const zstackZdoMessageStruct *message = reinterpret_cast <const zstackZdoMessageStruct*> (data.constData());
            QByteArray payload = data.mid(sizeof(zstackZdoMessageStruct));
            emit requestFinished(message->transactionId, static_cast <quint8> (payload.at(0)));
            emit zdoMessageReveived(qFromLittleEndian(message->srcAddress), qFromLittleEndian(message->clusterId), payload);
            break;
        }

        case ZSTACK_APP_CNF_BDB_COMMISSIONING:
        {
            if (data.at(2))
                break;

            switch (m_status)
            {
                case ZSTACK_NOT_STARTED_AUTOMATICALLY: logWarning << "Network not started, PAN ID collision detected"; break;
                case ZSTACK_COORDINATOR_STARTED: m_ready = true; emit coordinatorReady(); break;
            };

            break;
        }

        default:
        {
            logDebug(m_adapterDebug) << "Unrecognized Z-Stack command" << QString::asprintf("0x%04x", command) << "with data" << (data.isEmpty() ? "(empty)" : data.toHex(':'));
            break;
        }
    }
}

bool ZStack::writeNvItem(quint16 id, const QByteArray &data)
{
    zstackNvWriteStruct request;

    request.id = qToLittleEndian(id);
    request.offset = 0x00;
    request.length = static_cast <quint8> (data.length());

    if (!sendRequest(ZSTACK_SYS_OSAL_NV_WRITE, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyStatus)
    {
        logWarning << "NV item" << QString::asprintf("0x%04x", id) << "wtite request failed";
        return false;
    }

    return true;
}

bool ZStack::writeConfiguration(quint16 id, const QByteArray &data)
{
    zstackWriteConfigurationStruct request;

    request.id = id;
    request.length = static_cast <quint8> (data.length());

    if (!sendRequest(ZSTACK_ZB_WRITE_CONFIGURATION, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyStatus)
    {
        logWarning << "NV item" << QString::asprintf("0x%04x", id) << "wtite request failed";
        return false;
    }

    return true;
}

bool ZStack::startCoordinator(void)
{
    zstackVersionStruct version;

    if (!sendRequest(ZSTACK_SYS_VERSION))
    {
        logWarning << "Adapter version request failed";
        return false;
    }

    memcpy(&version, m_replyData.constData(), sizeof(version));

    switch (version.product)
    {
        case 0x01:
            m_modelName = "Z-Stack 3.x.0";
            m_version = ZStackVersion::ZStack3x0;
            break;

        case 0x02:
            m_modelName = "Z-Stack 3.0.x";
            m_version = ZStackVersion::ZStack30x;
            break;

        default:
            m_modelName = "Z-Stack 1.2.x";
            m_version = ZStackVersion::ZStack12x;
            break;
    }

    if (!m_clear)
    {
        quint64 ieeeAddress;

        m_manufacturerName = "Texas Instruments";
        m_firmware = QString::number(qFromLittleEndian(version.build));
        logInfo << QString("Adapter type: %1 (%2)").arg(m_modelName, m_firmware).toUtf8().constData();

        if (!sendRequest(ZSTACK_UTIL_GET_DEVICE_INFO) || m_replyStatus)
        {
            logWarning << "Device information request failed";
            return false;
        }

        memcpy(&ieeeAddress, m_replyData.constData() + 1, sizeof(ieeeAddress));
        ieeeAddress = qToBigEndian(qFromLittleEndian(ieeeAddress));
        m_ieeeAddress = QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress));

        for (auto it = m_nvItems.begin(); it != m_nvItems.end(); it++)
        {
            QByteArray data;
            quint8 status;

            if (m_version != ZStackVersion::ZStack12x || it.key() != ZCD_NV_PRECFGKEY)
            {
                zstackNvReadStruct request;
                zstackNvReplyStruct *reply;

                request.id = qToLittleEndian <quint16> (it.key());
                request.offset = 0x00;

                if (!sendRequest(ZSTACK_SYS_OSAL_NV_READ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))))
                {
                    logWarning << "NV item" << QString::asprintf("0x%04x", it.key()) << "read request failed";
                    return false;
                }

                reply = reinterpret_cast <zstackNvReplyStruct*> (m_replyData.data());
                data = m_replyData.mid(sizeof(zstackNvReplyStruct));
                status = reply->status;
            }
            else
            {
                zstackReadConfigurationStruct *reply;

                if (!sendRequest(ZSTACK_ZB_READ_CONFIGURATION, QByteArray(1, static_cast <char> (it.key()))))
                {
                    logWarning << "NV item" << QString::asprintf("0x%04x", it.key()) << "read request failed";
                    return false;
                }

                reply = reinterpret_cast <zstackReadConfigurationStruct*> (m_replyData.data());
                data = m_replyData.mid(sizeof(zstackReadConfigurationStruct));
                status = reply->status;
            }

            if (status || data != it.value())
            {
                logWarning << "NV item" << QString::asprintf("0x%04x", it.key()) << "value" << (data.isEmpty() ? "(empty)" : data.toHex(':')) << "doesn't match configuration value" << it.value().toHex(':');

                if (!m_write)
                {
                    logWarning << "Adapter configuration can't be changed, write protection enabled";
                    return false;
                }

                writeNvItem(ZCD_NV_STARTUP_OPTION, QByteArray(1, 0x03));
                m_clear = true;
                reset();

                return true;
            }
        }
    }
    else
    {
        logInfo << "Starting new network...";
        m_clear = false;

        for (auto it = m_nvItems.begin(); it != m_nvItems.end(); it++)
        {
            if (m_version != ZStackVersion::ZStack12x || it.key() != ZCD_NV_PRECFGKEY)
            {
                if (!writeNvItem(it.key(), it.value()))
                    return false;
            }
            else
            {
                if (!writeConfiguration(it.key(), it.value()) || !writeNvItem(ZCD_NV_TCLK_TABLE, QByteArray(8, 0xFF).append(m_defaultKey).append(8, 0x00)))
                    return false;
            }

            logWarning << "NV item" << QString::asprintf("0x%04x", it.key()) << "value set to" << it.value().toHex(':');
        }
    }

    if (m_version != ZStackVersion::ZStack12x)
    {
        zstackSetChannelStruct channelRequest;

        channelRequest.isPrimary = 0x01;
        channelRequest.channel = qToLittleEndian <quint32> (1 << m_channel);

        if (!sendRequest(ZSTACK_APP_CNF_BDB_SET_CHANNEL, QByteArray(reinterpret_cast <char*> (&channelRequest), sizeof(channelRequest))) || m_replyStatus)
        {
            logWarning << "Set primary channel request failed";
            return false;
        }

        channelRequest.isPrimary = 0x00;
        channelRequest.channel = 0x00;

        if (!sendRequest(ZSTACK_APP_CNF_BDB_SET_CHANNEL, QByteArray(reinterpret_cast <char*> (&channelRequest), sizeof(channelRequest))) || m_replyStatus)
        {
            logWarning << "Set secondary channel request failed";
            return false;
        }
    }

    for (int i = 0; i < m_zdoClusters.count(); i++)
    {
        quint16 request = qToLittleEndian(m_zdoClusters.at(i) | 0x8000);

        if (!sendRequest(ZSTACK_ZDO_MSG_CB_REGISTER, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))))
        {
            logWarning << "ZDO cluster" << QString::asprintf("0x%04x", m_zdoClusters.at(i)) << "callback register request failed";
            return false;
        }
    }

    for (auto it = m_endpoints.begin(); it != m_endpoints.end(); it++)
    {
        zstackRegisterEndpointStruct request;
        QByteArray data;

        request.endpointId = it.key();
        request.profileId = qToLittleEndian(it.value()->profileId());
        request.deviceId = qToLittleEndian(it.value()->deviceId());
        request.version = 0x00;
        request.latency = 0x00;

        data.append(static_cast <char> (it.value()->inClusters().count()));

        for (int i = 0; i < it.value()->inClusters().count(); i++)
        {
            quint16 clusterId = qToLittleEndian(it.value()->inClusters().at(i));
            data.append(reinterpret_cast <char*> (&clusterId), sizeof(clusterId));
        }

        data.append(static_cast <char> (it.value()->outClusters().count()));

        for (int i = 0; i < it.value()->outClusters().count(); i++)
        {
            quint16 clusterId = qToLittleEndian(it.value()->outClusters().at(i));
            data.append(reinterpret_cast <char*> (&clusterId), sizeof(clusterId));
        }

        if (!sendRequest(ZSTACK_AF_REGISTER, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyStatus)
        {
            logWarning << "Endpoint" << QString::asprintf("0x%02x", it.key()) << "register request failed";
            continue;
        }

        logInfo << "Endpoint" << QString::asprintf("0x%02x", it.key()) << "registered successfully";
    }

    if (!sendRequest(ZSTACK_AF_INTER_PAN_CTL, QByteArray(1, 0x02).append(0x0C)) || m_replyStatus)
    {
        logWarning << "Set Inter-PAN endpoint request failed";
        return false;
    }

    if (!sendRequest(ZSTACK_SYS_SET_TX_POWER, QByteArray(1, static_cast <char> (m_power))) || m_replyStatus)
        logWarning << "Set TX power request failed";

    if (!sendRequest(ZSTACK_ZDO_STARTUP_FROM_APP, QByteArray(2, 0x00)) || m_replyStatus == 0x02)
    {
        if (m_version == ZStackVersion::ZStack12x && m_status == ZSTACK_COORDINATOR_STARTED)
            return true;

        logWarning << "Startup request failed";
        return false;
    }

    return true;
}

void ZStack::softReset(void)
{
    sendData(QByteArray(1, ZSTACK_SKIP_BOOTLOADER));
    QThread::msleep(RESET_DELAY);
    sendRequest(ZSTACK_SYS_RESET_REQ, QByteArray(1, 0x01));
}

void ZStack::parseData(void)
{
    while (!m_buffer.isEmpty())
    {
        quint8 length, fcs = 0;

        if (!m_buffer.at(0))
            m_buffer.remove(0, 1);

        if (m_buffer.at(0) != static_cast <char> (ZSTACK_PACKET_FLAG) || m_buffer.length() < 5) // TODO: use offset
        {
            m_buffer.clear();
            return;
        }

        length = static_cast <quint8> (m_buffer.at(1));

        if (m_buffer.length() < length + 5)
            break;

        logDebug(m_portDebug) << "Frame received:" << m_buffer.mid(0, length + 5).toHex(':');

        for (quint8 i = 1; i < length + 4; i++)
            fcs ^= m_buffer.at(i);

        if (fcs != static_cast <quint8> (m_buffer.at(length + 4)))
        {
            logWarning << "Frame" << m_buffer.mid(0, length + 5).toHex(':') << "FCS mismatch";
            m_buffer.clear();
            return;
        }

        m_queue.enqueue(m_buffer.mid(2, length + 2));
        m_buffer.remove(0, length + 5);
    }
}

bool ZStack::permitJoin(bool enabled)
{
    zstackPermitJoinStruct request;

    request.mode = enabled && m_permitJoinAddress != PERMIT_JOIN_BROARCAST_ADDRESS ? 0x02 : 0x0F;
    request.dstAddress = qToLittleEndian <quint16> (enabled ? m_permitJoinAddress : PERMIT_JOIN_BROARCAST_ADDRESS);
    request.duration = enabled ? 0xF0 : 0x00;
    request.significance = 0x00;

    if (!sendRequest(ZSTACK_ZDO_MGMT_PERMIT_JOIN_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || m_replyStatus)
    {
        logWarning << "Set permit join request failed";
        return false;
    }

    return true;
}

void ZStack::handleQueue(void)
{
    while (!m_queue.isEmpty())
    {
        QByteArray packet = m_queue.dequeue();
        quint16 command;
        memcpy(&command, packet.constData(), sizeof(command));
        parsePacket(qFromBigEndian(command), packet.mid(2));
    }
}
