#include <QtEndian>
#include <QThread>
#include "logger.h"
#include "zstack.h"

ZStack::ZStack(QSettings *config, QObject *parent) : Adapter(config, parent), m_status(0), m_clear(false)
{
    quint32 channelList = qToLittleEndian <quint32> (1 << m_channel);

    m_nvItems.insert(ZCD_NV_MARKER,            QByteArray(1, ZSTACK_CONFIGURATION_MARKER));
    m_nvItems.insert(ZCD_NV_PRECFGKEY,         QByteArray::fromHex(config->value("security/key", "000102030405060708090a0b0c0d0e0f").toString().remove("0x").toUtf8()));
    m_nvItems.insert(ZCD_NV_PRECFGKEYS_ENABLE, QByteArray(1, config->value("security/enabled", false).toBool() ? 0x01 : 0x00));
    m_nvItems.insert(ZCD_NV_PANID,             QByteArray(reinterpret_cast <char*> (&m_panId), sizeof(m_panId)));
    m_nvItems.insert(ZCD_NV_CHANLIST,          QByteArray(reinterpret_cast <char*> (&channelList), sizeof(channelList)));
    m_nvItems.insert(ZCD_NV_LOGICAL_TYPE,      QByteArray(1, 0x00));
    m_nvItems.insert(ZCD_NV_ZDO_DIRECT_CB,     QByteArray(1, 0x01));

    m_apsClusters = {APS_NODE_DESCRIPTOR, APS_SIMPLE_DESCRIPTOR, APS_ACTIVE_ENDPOINTS, APS_BIND, APS_UNBIND, APS_LQI, APS_LEAVE};
}

bool ZStack::extendedDataRequest(quint8 id, const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &payload, bool group)
{
    extendedDataRequestStruct data;

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
    data.radius = dstPanId ? AF_DEFAULT_RADIUS * 2 : AF_DEFAULT_RADIUS;
    data.length = qToLittleEndian <quint16> (payload.length());

    return sendRequest(AF_DATA_REQUEST_EXT, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(payload)) && !m_replyData.at(0);
}

bool ZStack::extendedDataRequest(quint8 id, quint16 networkAddress, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group)
{
    networkAddress = qToLittleEndian(networkAddress);
    return extendedDataRequest(id, QByteArray(reinterpret_cast <char*> (&networkAddress), sizeof(networkAddress)), dstEndpointId, dstPanId, srcEndpointId, clusterId, data, group);
}

bool ZStack::setInterPanEndpointId(quint8 endpointId)
{
    quint8 data[2] = {0x02, endpointId};

    if (!sendRequest(AF_INTER_PAN_CTL, QByteArray(reinterpret_cast <char*> (&data), sizeof(data))) || m_replyData.at(0))
    {
        logWarning << "Set Inter-PAN endpointId" << QString::asprintf("0x%02x", endpointId) << "request failed";
        return false;
    }

    return true;
}

bool ZStack::setInterPanChannel(quint8 channel)
{
    quint8 data[2] = {0x01, channel};

    if (!sendRequest(AF_INTER_PAN_CTL, QByteArray(reinterpret_cast <char*> (&data), sizeof(data))) || m_replyData.at(0))
    {
        logWarning << "Set Inter-PAN channel" << channel << "request failed";
        return false;
    }

    return true;
}

void ZStack::resetInterPan(void)
{
    if (sendRequest(AF_INTER_PAN_CTL, QByteArray(1, 0x00)) || m_replyData.at(0))
        return;

    logWarning << "Reset Inter-PAN request failed";
}

bool ZStack::sendRequest(quint16 command, const QByteArray &data)
{
    QByteArray request;
    quint8 fcs = 0;

    if (m_debug)
        logInfo << "-->" << QString::asprintf("0x%04x", command) << data.toHex(':');

    m_command = qToBigEndian(command);

    request.append(ZSTACK_PACKET_FLAG);
    request.append(static_cast <char> (data.length()));
    request.append(reinterpret_cast <char*> (&m_command), sizeof(m_command));
    request.append(data);

    for (int i = 1; i < request.length(); i++)
        fcs ^= request[i];

    m_device->write(request.append(static_cast <char> (fcs)));
    return waitForSignal(this, SIGNAL(dataReceived()), ZSTACK_REQUEST_TIMEOUT);
}

void ZStack::parsePacket(quint16 command, const QByteArray &data)
{
    if (m_debug)
        logInfo << "<--" << QString::asprintf("0x%04x", command) << data.toHex(':');

    if (command & 0x2000)
    {
        if ((command ^ 0x4000) == qFromBigEndian(m_command))
        {
            m_replyData = data;
            emit dataReceived();
        }

        return;
    }

    switch (command)
    {
        case ZDO_NODE_DESC_RSP:
        case ZDO_SIMPLE_DESC_RSP:
        case ZDO_ACTIVE_EP_RSP:
        case ZDO_MGMT_LQI_RSP:
        case ZDO_BIND_RSP:
        case ZDO_UNBIND_RSP:
        case ZDO_MGMT_LEAVE_RSP:
        case ZDO_MGMT_PERMIT_JOIN_RSP:
        case ZDO_MGMT_NWK_UPDATE_RSP:
        case ZDO_SRC_RTG_IND:
        case ZDO_CONCENTRATOR_IND:
        case ZDO_TC_DEV_IND:
            break;

        case SYS_RESET_IND:
        {
            m_resetTimer->stop();

            if (!startCoordinator())
                logWarning << "Coordinator startup failed";

            break;
        }

        case AF_DATA_CONFIRM:
        {
            emit requestFinished(static_cast <quint8> (data.at(2)), static_cast <quint8> (data.at(0)));
            break;
        }

        case AF_INCOMING_MSG:
        {
            const incomingMessageStruct *message = reinterpret_cast <const incomingMessageStruct*> (data.constData());
            emit messageReveived(qFromLittleEndian(message->srcAddress), message->srcEndpointId, qFromLittleEndian(message->clusterId), message->linkQuality, data.mid(sizeof(incomingMessageStruct), message->length));
            break;
        }

        case AF_INCOMING_MSG_EXT:
        {
            const extendedIncomingMessageStruct *message = reinterpret_cast <const extendedIncomingMessageStruct*> (data.constData());
            quint64 srcAddress = qToBigEndian(qFromLittleEndian(message->srcAddress));

            if (message->srcAddressMode != 0x03)
            {
                logWarning << "Unsupporned extended message address mode" << QString::asprintf("0x%02x", message->srcAddressMode);
                return;
            }

            emit extendedMessageReveived(QByteArray(reinterpret_cast <char*> (&srcAddress), sizeof(srcAddress)), message->srcEndpointId, qFromLittleEndian(message->clusterId), message->linkQuality, data.mid(sizeof(extendedIncomingMessageStruct), message->length));
            break;
        }

        case ZDO_STATE_CHANGE_IND:
        {
            if (data.length() == 1)
                m_status = static_cast <quint8> (data.at(0));

            if (m_version == ZStackVersion::ZStack12x && m_status == ZSTACK_COORDINATOR_STARTED)
                emit coordinatorReady();

            break;
        }

        case ZDO_END_DEVICE_ANNCE_IND:
        {
            const deviceAnnounceStruct *announce = reinterpret_cast <const deviceAnnounceStruct*> (data.constData() + 2);
            quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(announce->ieeeAddress));
            emit deviceJoined(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(announce->networkAddress));
            break;
        }

        case ZDO_LEAVE_IND:
        {
            const deviceLeaveStruct *leave = reinterpret_cast <const deviceLeaveStruct*> (data.constData());
            quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(leave->ieeeAddress));
            emit deviceLeft(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)));
            break;
        }

        case ZDO_MSG_CB_INCOMING:
        {
            const zdoMessageStruct *message = reinterpret_cast <const zdoMessageStruct*> (data.constData());
            QByteArray payload = data.mid(sizeof(zdoMessageStruct));
            parseMessage(qFromLittleEndian(message->srcAddress), qFromLittleEndian(message->clusterId), payload);
            emit requestFinished(message->transactionId, static_cast <quint8> (payload.at(0)));
            break;
        }

        case APP_CNF_BDB_COMMISSIONING_NOTIFICATION:
        {
            if (!data.at(2) && m_status == ZSTACK_COORDINATOR_STARTED)
                emit coordinatorReady();

            break;
        }

        default:
            logWarning << "Unrecognized Z-Stack command" << QString::asprintf("0x%04x", command) << "with data" << (data.isEmpty() ? "(empty)" : data.toHex(':'));
            break;
    }
}

bool ZStack::writeNvItem(quint16 id, const QByteArray &data)
{
    nvWriteRequestStruct request;

    request.id = qToLittleEndian(id);
    request.offset = 0x00;
    request.length = static_cast <quint8> (data.length());

    if (!sendRequest(SYS_OSAL_NV_WRITE, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyData.at(0))
    {
        logWarning << "NV item" << QString::asprintf("0x%04x", id) << "wtite request failed";
        return false;
    }

    return true;
}

bool ZStack::writeConfiguration(quint16 id, const QByteArray &data)
{
    writeConfigurationRequestStruct request;

    request.id = id;
    request.length = static_cast <quint8> (data.length());

    if (!sendRequest(ZB_WRITE_CONFIGURATION, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyData.at(0))
    {
        logWarning << "NV item" << QString::asprintf("0x%04x", id) << "wtite request failed";
        return false;
    }

    return true;
}

bool ZStack::startCoordinator(void)
{
    versionResponseStruct version;

    if (!sendRequest(SYS_VERSION))
    {
        logWarning << "Adapter version request failed";
        return false;
    }

    memcpy(&version, m_replyData.constData(), sizeof(version));

    switch (version.product)
    {
        case 0x01:
            m_typeString = "Z-Stack 3.x.0";
            m_version = ZStackVersion::ZStack3x0;
            break;

        case 0x02:
            m_typeString = "Z-Stack 3.0.x";
            m_version = ZStackVersion::ZStack30x;
            break;

        default:
            m_typeString = "Z-Stack 1.2.x";
            m_version = ZStackVersion::ZStack12x;
            break;
    }

    if (!m_clear)
    {
        m_versionString = QString::number(qFromLittleEndian(version.build));
        logInfo << QString("Adapter type: %1 (%2)").arg(m_typeString, m_versionString).toUtf8().constData();

        if (!sendRequest(UTIL_GET_DEVICE_INFO) || m_replyData.at(0))
        {
            logWarning << "Device information request failed";
            return false;
        }

        memcpy(&m_ieeeAddress, m_replyData.constData() + 1, sizeof(m_ieeeAddress));

        for (auto it = m_nvItems.begin(); it != m_nvItems.end(); it++)
        {
            QByteArray data;
            quint8 status;

            if (m_version != ZStackVersion::ZStack12x || it.key() != ZCD_NV_PRECFGKEY)
            {
                nvReadRequestStruct request;
                nvReadReplyStruct *reply;

                request.id = qToLittleEndian <quint16> (it.key());
                request.offset = 0x00;

                if (!sendRequest(SYS_OSAL_NV_READ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))))
                {
                    logWarning << "NV item" << QString::asprintf("0x%04x", it.key()) << "read request failed";
                    return false;
                }

                reply = reinterpret_cast <nvReadReplyStruct*> (m_replyData.data());
                data = m_replyData.mid(sizeof(nvReadReplyStruct));
                status = reply->status;
            }
            else
            {
                readConfigurationReplyStruct *reply;

                if (!sendRequest(ZB_READ_CONFIGURATION, QByteArray(1, static_cast <char> (it.key()))))
                {
                    logWarning << "NV item" << QString::asprintf("0x%04x", it.key()) << "read request failed";
                    return false;
                }

                reply = reinterpret_cast <readConfigurationReplyStruct*> (m_replyData.data());
                data = m_replyData.mid(sizeof(readConfigurationReplyStruct));
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
        nvInitRequestStruct request;

        request.id = qToLittleEndian <quint16> (ZCD_NV_MARKER);
        request.itemLength = qToLittleEndian <quint16> (static_cast <quint16> (m_nvItems.value(ZCD_NV_MARKER).length()));
        request.dataLength = static_cast <quint8> (m_nvItems.value(ZCD_NV_MARKER).length());

        if (!sendRequest(SYS_OSAL_NV_ITEM_INIT, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(m_nvItems.value(ZCD_NV_MARKER))) || (m_replyData.at(0) && m_replyData.at(0) != 0x09))
        {
            logWarning << "NV item" << QString::asprintf("0x%04x", ZCD_NV_MARKER) << "init request failed";
            return false;
        }

        logInfo << "Adapter configuration cleared";

        for (auto it = m_nvItems.begin(); it != m_nvItems.end(); it++)
        {
            if (m_version != ZStackVersion::ZStack12x || it.key() != ZCD_NV_PRECFGKEY)
            {
                if (!writeNvItem(it.key(), it.value()))
                    return false;
            }
            else
            {
                if (!writeConfiguration(it.key(), it.value()) || !writeNvItem(ZCD_NV_TCLK_TABLE, QByteArray::fromHex("ffffffffffffffff5a6967426565416c6c69616e636530390000000000000000")))
                    return false;
            }

            logWarning << "NV item" << QString::asprintf("0x%04x", it.key()) << "value set to" << it.value().toHex(':');
        }
    }

    if (m_version != ZStackVersion::ZStack12x)
    {
        setChannelRequestStruct channelRequest;

        channelRequest.isPrimary = 0x01;
        channelRequest.channel = qToLittleEndian <quint32> (1 << m_channel);

        if (!sendRequest(APP_CNF_BDB_SET_CHANNEL, QByteArray(reinterpret_cast <char*> (&channelRequest), sizeof(channelRequest))) || m_replyData.at(0))
        {
            logWarning << "Set primary channel request failed";
            return false;
        }

        channelRequest.isPrimary = 0x00;
        channelRequest.channel = 0x00;

        if (!sendRequest(APP_CNF_BDB_SET_CHANNEL, QByteArray(reinterpret_cast <char*> (&channelRequest), sizeof(channelRequest))) || m_replyData.at(0))
        {
            logWarning << "Set secondary channel request failed";
            return false;
        }
    }

    for (int i = 0; i < m_apsClusters.count(); i++)
    {
        quint16 request = qToLittleEndian(m_apsClusters.at(i) | 0x8000);

        if (!sendRequest(ZDO_MSG_CB_REGISTER, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))))
        {
            logWarning << "APS cluster" << QString::asprintf("0x%04x", m_apsClusters.at(i)) << "callback register request failed";
            return false;
        }
    }

    for (auto it = m_endpointsData.begin(); it != m_endpointsData.end(); it++)
    {
        registerEndpointRequestStruct request;
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

        if (!sendRequest(AF_REGISTER, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyData.at(0))
        {
            logWarning << "Endpoint" << QString::asprintf("0x%02x", it.key()) << "register request failed";
            continue;
        }

        logInfo << "Endpoint" << QString::asprintf("0x%02x", it.key()) << "registered successfully";
    }

    if (!sendRequest(ZDO_STARTUP_FROM_APP, QByteArray(2, 0x00)) || m_replyData.at(0) == 0x02)
    {
        if (m_version == ZStackVersion::ZStack12x && m_status == ZSTACK_COORDINATOR_STARTED)
            return true;

        logWarning << "Strartup request failed";
        return false;
    }

    return true;
}

void ZStack::softReset(void)
{
    m_device->write(QByteArray(1, ZSTACK_SKIP_BOOTLOADER));
    QThread::msleep(ADAPTER_RESET_DELAY);
    sendRequest(SYS_RESET_REQ, QByteArray(1, 0x01));
}

void ZStack::parseData(void)
{
    QByteArray buffer = m_device->readAll();

    while (!buffer.isEmpty())
    {
        quint8 length = static_cast <quint8> (buffer.at(1)), fcs = 0;

        if (buffer.at(0) != static_cast <char> (ZSTACK_PACKET_FLAG))
        {
            buffer.clear();
            break;
        }

        if (buffer.length() < 5 || buffer.length() < length + 5)
            break;

        for (quint8 i = 1; i < length + 4; i++)
            fcs ^= buffer.at(i);

        if (fcs != static_cast <quint8> (buffer.at(length + 4)))
        {
            logWarning << "Packet" << buffer.left(length + 5).toHex(':') << "FCS mismatch";
            buffer.clear();
            break;
        }

        m_queue.enqueue(buffer.mid(2, length + 2));
        buffer.remove(0, length + 5);
    }
}

bool ZStack::permitJoin(bool enabled)
{
    permitJoinRequestStruct request;

    request.mode = PERMIT_JOIN_MODE_ADDREESS;
    request.dstAddress = qToLittleEndian <quint16> (PERMIT_JOIN_BROARCAST_ADDRESS);
    request.duration = enabled ? 0xF0 : 0x00;
    request.significance = 0x00;

    if (!sendRequest(ZDO_MGMT_PERMIT_JOIN_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || m_replyData.at(0))
    {
        logWarning << "Set permit join request failed";
        return false;
    }

    return true;
}

bool ZStack::unicastRequest(quint8 id, quint16 networkAddress, quint16 clusterId, quint8 srcEndPointId, quint8 dstEndPointId, const QByteArray &payload)
{
    dataRequestStruct reqest;

    reqest.networkAddress = qToLittleEndian(networkAddress);
    reqest.dstEndpointId = dstEndPointId;
    reqest.srcEndpointId = srcEndPointId;
    reqest.clusterId = qToLittleEndian(clusterId);
    reqest.transactionId = id;
    reqest.options = AF_DISCV_ROUTE;
    reqest.radius = AF_DEFAULT_RADIUS;
    reqest.length = static_cast <quint8> (payload.length());

    return sendRequest(AF_DATA_REQUEST, QByteArray(reinterpret_cast <char*> (&reqest), sizeof(reqest)).append(payload)) && !m_replyData.at(0);
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
