#include <QtEndian>
#include <QThread>
#include "logger.h"
#include "zstack.h"

ZStack::ZStack(QSettings *config, QObject *parent) : Adapter(config, parent), m_status(0), m_transactionId(0), m_clear(false)
{
    quint32 channelList = qToLittleEndian <quint32> (1 << m_channel);

    m_nvItems.insert(ZCD_NV_MARKER,            QByteArray(1, ZSTACK_CONFIGURATION_MARKER));
    m_nvItems.insert(ZCD_NV_PRECFGKEY,         QByteArray::fromHex(config->value("security/key", "000102030405060708090a0b0c0d0e0f").toString().remove("0x").toUtf8()));
    m_nvItems.insert(ZCD_NV_PRECFGKEYS_ENABLE, QByteArray(1, config->value("security/enabled", false).toBool() ? 0x01 : 0x00));
    m_nvItems.insert(ZCD_NV_PANID,             QByteArray(reinterpret_cast <char*> (&m_panId), sizeof(m_panId)));
    m_nvItems.insert(ZCD_NV_CHANLIST,          QByteArray(reinterpret_cast <char*> (&channelList), sizeof(channelList)));
    m_nvItems.insert(ZCD_NV_LOGICAL_TYPE,      QByteArray(1, 0x00));
    m_nvItems.insert(ZCD_NV_ZDO_DIRECT_CB,     QByteArray(1, 0x01));
}

bool ZStack::nodeDescriptorRequest(quint16 networkAddress)
{
    quint16 data = qToLittleEndian(networkAddress);
    return sendRequest(ZDO_NODE_DESC_REQ, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(reinterpret_cast <char*> (&data), sizeof(data))) && !m_replyData.at(0);
}

bool ZStack::simpleDescriptorRequest(quint16 networkAddress, quint8 endpointId)
{
    quint16 data = qToLittleEndian(networkAddress);
    return sendRequest(ZDO_SIMPLE_DESC_REQ, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(reinterpret_cast <char*> (&data), sizeof(data)).append(1, static_cast <char> (endpointId))) && !m_replyData.at(0);
}

bool ZStack::activeEndpointsRequest(quint16 networkAddress)
{
    quint16 data = qToLittleEndian(networkAddress);
    return sendRequest(ZDO_ACTIVE_EP_REQ, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(reinterpret_cast <char*> (&data), sizeof(data))) && !m_replyData.at(0);
}

bool ZStack::lqiRequest(quint16 networkAddress, quint8 index)
{
    lqiRequestStruct data;

    data.networkAddress = qToLittleEndian(networkAddress);
    data.index = index;

    return sendRequest(ZDO_MGMT_LQI_REQ, QByteArray(reinterpret_cast <char*> (&data), sizeof(data))) && !m_replyData.at(0);
}

bool ZStack::bindRequest(quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind)
{
    QByteArray data = bindRequestPayload(srcAddress, srcEndpointId, clusterId, dstAddress, dstEndpointId);

    if (data.isEmpty())
        return false;

    m_bindAddress = qToLittleEndian(networkAddress);
    m_bindRequestStatus = false;

    if (!sendRequest(unbind ? ZDO_UNBIND_REQ : ZDO_BIND_REQ, QByteArray(reinterpret_cast <char*> (&m_bindAddress), sizeof(m_bindAddress)).append(data)) || m_replyData.at(0))
        return false;

    return waitForSignal(this, SIGNAL(bindResponse()), NETWORK_REQUEST_TIMEOUT) && m_bindRequestStatus;
}

bool ZStack::dataRequest(quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &payload)
{
    dataRequestStruct data;

    data.networkAddress = qToLittleEndian(networkAddress);
    data.dstEndpointId = endpointId;
    data.srcEndpointId = 0x01;
    data.clusterId = qToLittleEndian(clusterId);
    data.transactionId = m_transactionId;
    data.options = AF_DISCV_ROUTE;
    data.radius = AF_DEFAULT_RADIUS;
    data.length = static_cast <quint8> (payload.length());

    m_dataRequestFinished = false;

    if (!sendRequest(AF_DATA_REQUEST, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(payload)) || m_replyData.at(0))
        return false;

    if (!m_dataRequestFinished && !waitForSignal(this, SIGNAL(dataConfirm()), NETWORK_REQUEST_TIMEOUT))
        return false;

    m_transactionId++;
    return m_dataRequestStatus ? false : true;
}

bool ZStack::extendedDataRequest(const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &payload, bool group)
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
    data.transactionId = m_transactionId;
    data.options = 0x00;
    data.radius = dstPanId ? AF_DEFAULT_RADIUS * 2 : AF_DEFAULT_RADIUS;
    data.length = qToLittleEndian <quint16> (payload.length());

    m_dataRequestFinished = false;

    if (!sendRequest(AF_DATA_REQUEST_EXT, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(payload)) || m_replyData.at(0))
        return false;

    if (!m_dataRequestFinished && !waitForSignal(this, SIGNAL(dataConfirm()), NETWORK_REQUEST_TIMEOUT))
        return false;

    m_transactionId++;
    return m_dataRequestStatus ? false : true;
}

bool ZStack::extendedDataRequest(quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group)
{
    address = qToLittleEndian(address);
    return extendedDataRequest(QByteArray(reinterpret_cast <char*> (&address), sizeof(address)), dstEndpointId, dstPanId, srcEndpointId, clusterId, data, group);
}

bool ZStack::setInterPanEndpointId(quint8 endpointId)
{
    quint8 data[2] = {0x02, endpointId};

    if (!sendRequest(AF_INTER_PAN_CTL, QByteArray(reinterpret_cast <char*> (&data), sizeof(data))) || m_replyData.at(0))
    {
        logWarning << "Set Inter-PAN endpointId" << QString::asprintf("0x%02X", endpointId) << "request failed";
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
        logInfo << "-->" << QString::asprintf("%04X", command) << data.toHex(':');

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
        logInfo << "<--" << QString::asprintf("%04X", command) << data.toHex(':');

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
            if (static_cast <quint8> (data.at(2)) == m_transactionId)
            {
                m_dataRequestStatus = data.at(0);
                m_dataRequestFinished = true;
                emit dataConfirm();
            }

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
                logWarning << "Unsupporned extended message address mode" << QString::asprintf("0x%02X", message->srcAddressMode);
                return;
            }

            emit extendedMessageReveived(QByteArray(reinterpret_cast <char*> (&srcAddress), sizeof(srcAddress)), message->srcEndpointId, qFromLittleEndian(message->clusterId), message->linkQuality, data.mid(sizeof(extendedIncomingMessageStruct), message->length));
            break;
        }

        case ZDO_NODE_DESC_RSP:
        {
            const nodeDescriptorResponseStruct *response = reinterpret_cast <const nodeDescriptorResponseStruct*> (data.constData() + 2);

            if (!response->status)
                emit nodeDescriptorReceived(qFromLittleEndian(response->networkAddress), static_cast <LogicalType> (response->logicalType & 0x03), qFromLittleEndian(response->manufacturerCode));

            break;
        }

        case ZDO_SIMPLE_DESC_RSP:
        {
            const simpleDescriptorResponseStruct *response = reinterpret_cast <const simpleDescriptorResponseStruct*> (data.constData() + 2);

            if (!response->status)
            {
                QByteArray clusterData = data.mid(sizeof(simpleDescriptorResponseStruct) + 2);
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

        case ZDO_ACTIVE_EP_RSP:
        {
            const activeEndpointsResponseStruct *response = reinterpret_cast <const activeEndpointsResponseStruct*> (data.constData() + 2);

            if (!response->status)
                emit activeEndpointsReceived(qFromLittleEndian(response->networkAddress), data.mid(sizeof(activeEndpointsResponseStruct) + 2, response->count));

            break;
        }

        case ZDO_BIND_RSP:
        case ZDO_UNBIND_RSP:
        {
            if (!memcmp(data.constData(), &m_bindAddress, sizeof(m_bindAddress)))
            {
                m_bindRequestStatus = data.at(2) ? false : true;
                emit bindResponse();
            }

            break;
        }

        case ZDO_MGMT_LQI_RSP:
        {
            const lqiResponseStruct *response = reinterpret_cast <const lqiResponseStruct*> (data.constData());

            if (!response->status)
            {
                for (quint8 i = 0; i < response->count; i++)
                {
                    const neighborRecordStruct *neighbor = reinterpret_cast <const neighborRecordStruct*> (data.constData() + sizeof(lqiResponseStruct) + i * sizeof(neighborRecordStruct));
                    emit neighborRecordReceived(qFromLittleEndian(response->networkAddress), qFromLittleEndian(neighbor->networkAddress), neighbor->linkQuality, !(response->index | i));
                }

                if (response->index + response->count < response->total)
                    lqiRequest(qFromLittleEndian(response->networkAddress), response->index + response->count);
            }

            break;
        }

        case ZDO_STATE_CHANGE_IND:
        {
            if (data.length() == 1)
                m_status = static_cast <quint8> (data.at(0));

            if (m_version == ZStackVersion::ZStack12x && m_status == ZSTACK_COORDINATOR_STARTED)
                coordinatorStarted();

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
            const deviceLeaveStruct *announce = reinterpret_cast <const deviceLeaveStruct*> (data.constData());
            quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(announce->ieeeAddress));
            emit deviceLeft(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)));
            break;
        }

        case APP_CNF_BDB_COMMISSIONING_NOTIFICATION:
        {
            if (!data.at(2) && m_status == ZSTACK_COORDINATOR_STARTED)
                coordinatorStarted();

            break;
        }

        default:
            logWarning << "Unrecognized Z-Stack command" << QString::asprintf("0x%04X", command) << "with data" << data.toHex(':');
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
        logWarning << "NV item" << QString::asprintf("0x%04X", id) << "wtite request failed";
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
        logWarning << "NV item" << QString::asprintf("0x%04X", id) << "wtite request failed";
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
                    logWarning << "NV item" << QString::asprintf("0x%04X", it.key()) << "read request failed";
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
                    logWarning << "NV item" << QString::asprintf("0x%04X", it.key()) << "read request failed";
                    return false;
                }

                reply = reinterpret_cast <readConfigurationReplyStruct*> (m_replyData.data());
                data = m_replyData.mid(sizeof(readConfigurationReplyStruct));
                status = reply->status;
            }

            if (status || data != it.value())
            {
                logWarning << "NV item" << QString::asprintf("0x%04X", it.key()) << "value" << data.toHex(':') << "doesn't match configuration value" << it.value().toHex(':');

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
            logWarning << "NV item" << QString::asprintf("0x%04X", ZCD_NV_MARKER) << "init request failed";
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
                if (!writeConfiguration(it.key(), it.value()) || !writeNvItem(ZCD_NV_TCLK_TABLE, QByteArray::fromHex("FFFFFFFFFFFFFFFF5A6967426565416C6C69616E636530390000000000000000")))
                    return false;
            }

            logWarning << "NV item" << QString::asprintf("0x%04X", it.key()) << "value set to" << it.value().toHex(':');
        }
    }

    if (m_version != ZStackVersion::ZStack12x)
    {
        setChannelRequestStruct channelRequest;

        channelRequest.isPrimary = 0x01;
        channelRequest.channel = qToLittleEndian <quint32> (1 << m_channel);

        if (!sendRequest(APP_CNF_BDB_SET_CHANNEL, QByteArray(reinterpret_cast <char*> (&channelRequest), sizeof(channelRequest))) || m_replyData.at(0))
        {
            logWarning << "Set primary channel request failed" << m_replyData.toHex(':');
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
            logWarning << "Endpoint" << QString::asprintf("0x%02X", it.key()) << "register request failed";
            return false;
        }

        logInfo << "Endpoint" << QString::asprintf("0x%02X", it.key()) << "registered successfully";
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

void ZStack::coordinatorStarted(void)
{
    quint64 ieeeAddress;
    ieeeAddress = qToBigEndian(qFromLittleEndian(m_ieeeAddress));
    emit coordinatorReady(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)));
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

void ZStack::softReset(void)
{
    m_device->write(QByteArray(1, ZSTACK_SKIP_BOOTLOADER));
    QThread::msleep(ADAPTER_RESET_DELAY);
    sendRequest(SYS_RESET_REQ, QByteArray(1, 0x01));
}

void ZStack::parseData(void)
{
    while (!m_buffer.isEmpty())
    {
        quint8 length = static_cast <quint8> (m_buffer.at(1)), fcs = 0;

        if (m_buffer.at(0) != static_cast <char> (ZSTACK_PACKET_FLAG))
        {
            m_buffer.clear();
            break;
        }

        if (m_buffer.length() < 5 || m_buffer.length() < length + 5)
            break;

        for (quint8 i = 1; i < length + 4; i++)
            fcs ^= m_buffer.at(i);

        if (fcs != static_cast <quint8> (m_buffer.at(length + 4)))
        {
            logWarning << "Packet" << m_buffer.left(length + 5).toHex(':') << "FCS mismatch";
            m_buffer.clear();
            break;
        }

        m_queue.enqueue(m_buffer.mid(2, length + 2));
        m_buffer.remove(0, length + 5);
    }
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
