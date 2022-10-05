#include <QtEndian>
#include <QEventLoop>
#include <QThread>
#include "gpio.h"
#include "logger.h"
#include "zstack.h"

ZStack::ZStack(QSettings *config, QObject *parent) : QObject(parent), m_port(new QSerialPort(this)), m_timer(new QTimer(this)), m_reset(false), m_status(0), m_transactionId(0)
{
    quint16 panId = qToLittleEndian(config->value("zigbee/panid", "0x1A62").toString().toInt(nullptr, 16));
    quint32 chanList = qToBigEndian(ADAPTER_CHANNEL_LIST);

    m_port->setPortName(config->value("zigbee/port", "/dev/ttyS0").toString());
    m_port->setBaudRate(QSerialPort::Baud115200);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);

    m_bootPin = static_cast <qint16> (config->value("gpio/boot", -1).toInt());
    m_resetPin = static_cast <qint16> (config->value("gpio/reset", -1).toInt());
    m_channel = static_cast <quint8>  (config->value("zigbee/channel", 11).toInt());

    m_write = config->value("zigbee/write", false).toBool();
    m_debug = config->value("zigbee/debug", false).toBool();
    m_rts = config->value("zigbee/rts", false).toBool();

    m_nvItems.insert(ZCD_NV_PRECFGKEY, QByteArray::fromHex(config->value("security/key", "000102030405060708090a0b0c0d0e0f").toString().remove("0x").toUtf8()));
    m_nvItems.insert(ZCD_NV_PRECFGKEYS_ENABLE, QByteArray(1, config->value("security/enabled", false).toBool() ? 0x01 : 0x00));
    m_nvItems.insert(ZCD_NV_PANID, QByteArray(reinterpret_cast <char*> (&panId), sizeof(panId)));
    m_nvItems.insert(ZCD_NV_CHANLIST, QByteArray(reinterpret_cast <char*> (&chanList), sizeof(chanList)));
    m_nvItems.insert(ZCD_NV_LOGICAL_TYPE, QByteArray(1, 0x00));
    m_nvItems.insert(ZCD_NV_ZDO_DIRECT_CB, QByteArray(1, 0x01));
    m_nvItems.insert(ZCD_NV_USER, QByteArray(1, ADAPTER_CONFIGURATION_MARKER));

    GPIO::setDirection(m_bootPin, GPIO::Output);
    GPIO::setDirection(m_resetPin, GPIO::Output);

    connect(m_port, &QSerialPort::readyRead, this, &ZStack::startTimer);
    connect(m_timer, &QTimer::timeout, this, &ZStack::receiveData);
}

void ZStack::init(void)
{
    if (!m_port->open(QIODevice::ReadWrite))
    {
        logWarning << "Can't open port" << m_port->portName();
        return;
    }

    logInfo << "Reseting adapter...";
    resetAdapter();
}

void ZStack::registerEndpoint(quint8 endpointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters)
{
    registerEndpointRequestStruct request;
    QByteArray data;

    request.endpointId = endpointId;
    request.profileId = qToLittleEndian(profileId);
    request.deviceId = qToLittleEndian(deviceId);
    request.version = 0;
    request.latency = 0;

    data.append(static_cast <char> (inClusters.count()));

    for (int i = 0; i < inClusters.count(); i++)
    {
        quint16 clusterId = inClusters.at(i);
        data.append(reinterpret_cast <char*> (&clusterId), sizeof(clusterId));
    }

    data.append(static_cast <char> (outClusters.count()));

    for (int i = 0; i < outClusters.count(); i++)
    {
        quint16 clusterId = outClusters.at(i);
        data.append(reinterpret_cast <char*> (&clusterId), sizeof(clusterId));
    }

    if (sendRequest(AF_REGISTER, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) && !m_replyData.at(0))
    {
        logInfo << "Endpoint" << QString::asprintf("0x%02X", endpointId) << "registered successfully";
        return;
    }

    logWarning << "Endpoint" << QString::asprintf("0x%02X", endpointId) << "register requet failed";
}

void ZStack::setPermitJoin(bool enabled)
{
    permitJoinRequestStruct request;

    request.mode = PERMIT_JOIN_MODE_ADDREESS;
    request.dstAddress = qToLittleEndian <quint16> (PERMIT_JOIN_BROARCAST_ADDRESS);
    request.duration = enabled ? 0xFF : 0;
    request.significance = 0;

    if (sendRequest(ZDO_MGMT_PERMIT_JOIN_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyData.at(0))
    {
        logInfo << "Permit join" << (enabled ? "enabled" : "disabled") << "successfully";
        return;
    }

    logWarning << "Set permit join request failed";
}

void ZStack::nodeDescriptorRequest(quint16 networkAddress)
{
    nodeDescriptorRequestStruct request;

    request.dstAddress = qToLittleEndian(networkAddress);
    request.networkAddress = qToLittleEndian(networkAddress);

    if (sendRequest(ZDO_NODE_DESC_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyData.at(0))
        return;

    logWarning << "Node descriptor request filed";
}

void ZStack::simpleDescriptorRequest(quint16 networkAddress, quint8 endpointId)
{
    simpleDescriptorRequestStruct request;

    request.dstAddress = qToLittleEndian(networkAddress);
    request.networkAddress = qToLittleEndian(networkAddress);
    request.endpointId = endpointId;

    if (sendRequest(ZDO_SIMPLE_DESC_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyData.at(0))
        return;

    logWarning << "Simple descriptor request filed";
}

void ZStack::activeEndpointsRequest(quint16 networkAddress)
{
    activeEndpointsRequestStruct request;

    request.dstAddress = qToLittleEndian(networkAddress);
    request.networkAddress = qToLittleEndian(networkAddress);

    if (sendRequest(ZDO_ACTIVE_EP_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyData.at(0))
        return;

    logWarning << "Active endpoints request filed";
}

void ZStack::lqiRequest(quint16 networkAddress, quint8 index)
{
    lqiRequestStruct request;

    request.networkAddress = qToLittleEndian(networkAddress);
    request.index = index;

    if (sendRequest(ZDO_MGMT_LQI_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) && !m_replyData.at(0))
        return;

    logWarning << "LQI request filed";
}

bool ZStack::bindRequest(quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind)
{
    bindRequestStruct request;
    quint64 src, dst;
    QEventLoop loop;
    QTimer timer;

    memcpy(&src, srcAddress.constData(), sizeof(src));
    memcpy(&dst, dstAddress.isEmpty() ? m_ieeeAddress.constData() : dstAddress.constData(), sizeof(dst));

    request.networkAddress = qToLittleEndian(networkAddress);
    request.srcAddress = qToBigEndian(src);
    request.srcEndpointId = srcEndpointId;
    request.clusterId = qToLittleEndian(clusterId);
    request.dstAddressMode = ADDRESS_MODE_64_BIT;
    request.dstAddress = qToBigEndian(dst);
    request.dstEndpointId = dstEndpointId ? dstEndpointId : 1;

    connect(this, &ZStack::bindResponse, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    m_bindAddress = networkAddress;
    m_bindRequestSuccess = false;

    if (!sendRequest(unbind ? ZDO_UNBIND_REQ : ZDO_BIND_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || m_replyData.at(0))
    {
        logWarning << (unbind ? "Unbind" : "Bind") << "request failed";
        return false;
    }

    timer.setSingleShot(true);
    timer.start(ADAPTER_REQUEST_TIMEOUT);
    loop.exec();

    return m_bindRequestSuccess;
}

bool ZStack::dataRequest(quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &data)
{
    dataRequestStruct request;

    request.networkAddress = qToLittleEndian(networkAddress);
    request.dstEndpointId = endpointId;
    request.srcEndpointId = 0x01;
    request.clusterId = qToLittleEndian(clusterId);
    request.transactionId = m_transactionId;
    request.options = AF_DISCV_ROUTE;
    request.radius = AF_DEFAULT_RADIUS;
    request.length = static_cast <quint8> (data.length());

    m_dataConfirmReceived = false;
    m_dataRequestSuccess = false;

    if (!sendRequest(AF_DATA_REQUEST, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyData.at(0))
    {
        logWarning << "Data request failed";
        return false;
    }

    if (!m_dataConfirmReceived)
    {
        QEventLoop loop;
        QTimer timer;

        connect(this, &ZStack::dataConfirm, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

        timer.setSingleShot(true);
        timer.start(ADAPTER_REQUEST_TIMEOUT);

        loop.exec();
    }

    m_transactionId++;
    return m_dataRequestSuccess;
}

bool ZStack::extendedDataRequest(const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group)
{
    extendedDataRequestStruct request;

    switch (address.length())
    {
        case 2: request.dstAddressMode = group ? ADDRESS_MODE_GROUP : ADDRESS_MODE_16_BIT; break;
        case 8: request.dstAddressMode = ADDRESS_MODE_64_BIT; break;
        default: return false;
    }

    memset(&request.dstAddress, 0, sizeof(request.dstAddress));
    memcpy(&request.dstAddress, address.constData(), address.length());

    if (request.dstAddressMode == ADDRESS_MODE_64_BIT)
        request.dstAddress = qToBigEndian(request.dstAddress);

    request.dstEndpointId = dstEndpointId;
    request.dstPanId = qToLittleEndian(dstPanId);
    request.srcEndpointId = srcEndpointId;
    request.clusterId = qToLittleEndian(clusterId);
    request.transactionId = m_transactionId;
    request.options = 0x00;
    request.radius = dstPanId ? AF_DEFAULT_RADIUS * 2 : AF_DEFAULT_RADIUS;
    request.length = qToLittleEndian(static_cast <quint16> (data.length()));

    m_dataConfirmReceived = false;
    m_dataRequestSuccess = false;

    if (!sendRequest(AF_DATA_REQUEST_EXT, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyData.at(0))
    {
        logWarning << "Data request failed";
        return false;
    }

    if (!m_dataConfirmReceived)
    {
        QEventLoop loop;
        QTimer timer;

        connect(this, &ZStack::dataConfirm, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

        timer.setSingleShot(true);
        timer.start(ADAPTER_REQUEST_TIMEOUT);

        loop.exec();
    }

    m_transactionId++;
    return m_dataRequestSuccess;
}

bool ZStack::extendedDataRequest(quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group)
{
    address = qToLittleEndian(address);
    return extendedDataRequest(QByteArray(reinterpret_cast <char*> (&address), sizeof(address)), dstEndpointId, dstPanId, srcEndpointId, clusterId, data, group);
}

bool ZStack::setInterPanEndpointId(quint8 endpointId)
{
    quint8 request[2] = {0x02, endpointId};

    if (!sendRequest(AF_INTER_PAN_CTL, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || m_replyData.at(0))
    {
        logWarning << "Set Inter-PAN endpointId" << QString::asprintf("0x%02X", endpointId) << "request failed";
        return false;
    }

    return true;
}

bool ZStack::setInterPanChannel(quint8 channel)
{
    quint8 request[2] = {0x01, channel};

    if (!sendRequest(AF_INTER_PAN_CTL, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || m_replyData.at(0))
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

void ZStack::parsePacket(quint16 command, const QByteArray &data)
{
    if (command & 0x2000)
    {
        m_replyCommand = command ^ 0x4000;
        m_replyData = data;
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
            if (!startCoordinator())
                logWarning << "Coordinator startup failed";

            break;
        }

        case AF_DATA_CONFIRM:
        {
            if (static_cast <quint8> (data.at(2)) == m_transactionId)
            {
                m_dataRequestStatus = data.at(0);
                m_dataRequestSuccess = m_dataRequestStatus ? false : true;
                m_dataConfirmReceived = true;
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
            quint64 srcAddress = qFromBigEndian(message->srcAddress);

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
            const nodeDescriptorResponseStruct *message = reinterpret_cast <const nodeDescriptorResponseStruct*> (data.constData());

            if (!message->status)
                emit nodeDescriptorReceived(qFromLittleEndian(message->srcAddress), static_cast <LogicalType> (message->logicalType & 0x03), qFromLittleEndian(message->manufacturerCode));

            break;
        }

        case ZDO_SIMPLE_DESC_RSP:
        {
            const simpleDescriptorResponseStruct *message = reinterpret_cast <const simpleDescriptorResponseStruct*> (data.constData());

            if (!message->status)
            {
                QByteArray clusterData = data.mid(sizeof(simpleDescriptorResponseStruct));
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

                emit simpleDescriptorReceived(qFromLittleEndian(message->srcAddress), message->endpointId, qFromLittleEndian(message->profileId), qFromLittleEndian(message->deviceId), inClusters, outClusters);
            }

            break;
        }

        case ZDO_ACTIVE_EP_RSP:
        {
            const activeEndpointsResponseStruct *message = reinterpret_cast <const activeEndpointsResponseStruct*> (data.constData());

            if (!message->status)
                emit activeEndpointsReceived(qFromLittleEndian(message->srcAddress), data.mid(sizeof(activeEndpointsResponseStruct), message->count));

            break;
        }

        case ZDO_BIND_RSP:
        case ZDO_UNBIND_RSP:
        {
            quint16 networkAddress;

            memcpy(&networkAddress, data.constData(), sizeof(networkAddress));

            if (qFromLittleEndian(networkAddress) == m_bindAddress)
            {
                m_bindRequestSuccess = data.at(2) ? false : true;
                emit bindResponse();
            }

            break;
        }

        case ZDO_MGMT_LQI_RSP:
        {
            const lqiResponseStruct *message = reinterpret_cast <const lqiResponseStruct*> (data.constData());

            if (!message->status)
            {
                for (quint8 i = 0; i < message->count; i++)
                {
                    const neighborRecordStruct *neighbor = reinterpret_cast <const neighborRecordStruct*> (data.constData() + sizeof(lqiResponseStruct) + i * sizeof(neighborRecordStruct));
                    emit neighborRecordReceived(qFromLittleEndian(message->networkAddress), qFromLittleEndian(neighbor->networkAddress), neighbor->linkQuality, !(message->index | i));
                }

                if (message->index + message->count < message->total)
                    lqiRequest(qFromLittleEndian(message->networkAddress), message->index + message->count);
            }

            break;
        }

        case ZDO_STATE_CHANGE_IND:
        {
            if (data.length() == 1)
                m_status = static_cast <quint8> (data.at(0));

            break;
        }

        case ZDO_END_DEVICE_ANNCE_IND:
        {
            const deviceAnnounceStruct *message = reinterpret_cast <const deviceAnnounceStruct*> (data.constData());
            quint64 ieeeAddress = qFromBigEndian(message->ieeeAddress);
            emit deviceJoined(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->networkAddress), message->capabilities);
            break;
        }

        case ZDO_LEAVE_IND:
        {
            const deviceLeaveStruct *message = reinterpret_cast <const deviceLeaveStruct*> (data.constData());
            quint64 ieeeAddress = qFromBigEndian(message->ieeeAddress);
            emit deviceLeft(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->networkAddress));
            break;
        }

        case APP_CNF_BDB_COMMISSIONING_NOTIFICATION:
        {
            if (!data.at(2) && m_status == ZSTACK_COORDINATOR_STARTED)
            {
                if (m_reset)
                {
                    QThread::msleep(5000);

                    m_reset = false;
                    resetAdapter();

                    return;
                }

                emit coordinatorReady(m_ieeeAddress);
            }

            break;
        }

        default:
            logWarning << "Unrecognized Z-Stack command" << QString::asprintf("0x%04X", command) << "with data" << data.toHex(':');
            break;
    }
}

bool ZStack::sendRequest(quint16 command, const QByteArray &data)
{
    QByteArray packet;
    QEventLoop loop;
    char fcs = 0;

    if (!m_port->isOpen())
    {
        logWarning << "Port" << m_port->portName() << "is not open";
        return false;
    }

    command = qToBigEndian(command);

    packet.append(ZSTACK_PACKET_START);
    packet.append(static_cast <char> (data.length()));
    packet.append(reinterpret_cast <char*> (&command), sizeof(command));
    packet.append(data);

    for (int i = 1; i < packet.length(); i++)
        fcs ^= packet[i];

    m_port->write(packet.append(fcs));

    if (m_debug)
        logInfo << "Packet sent" << packet.toHex(':');

    if (!m_port->waitForReadyRead(ADAPTER_REQUEST_TIMEOUT))
    {
        logWarning << "Request timed out, resetting adapter...";
        resetAdapter();
        return false;
    }

    connect(m_timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    startTimer();
    loop.exec();

    QThread::msleep(ADAPTER_THROTTLE_DELAY); // TODO: check this is needed
    return m_replyCommand == qFromBigEndian(command);
}

void ZStack::resetAdapter(void)
{
    if (m_rts)
    {
        m_port->setDataTerminalReady(false);
        m_port->setRequestToSend(true);
        QThread::msleep(ADAPTER_RESET_DELAY);
        m_port->setRequestToSend(false);
    }
    else
    {
        GPIO::setStatus(m_bootPin, true);
        GPIO::setStatus(m_resetPin, false);
        QThread::msleep(ADAPTER_RESET_DELAY);
        GPIO::setStatus(m_resetPin, true);
    }
}

bool ZStack::writeNvItem(quint16 id, const QByteArray &data)
{
    nvWriteRequestStruct writeRequest;

    writeRequest.id = qToLittleEndian(id);
    writeRequest.offset = 0;
    writeRequest.length = static_cast <quint8> (data.length());

    if(!sendRequest(SYS_OSAL_NV_WRITE, QByteArray(reinterpret_cast <char*> (&writeRequest), sizeof(writeRequest)).append(data)) || m_replyData.at(0))
    {
        logWarning << "NV item" << QString::asprintf("0x%04X", id) << "wtite request failed";
        return false;
    }

    return true;
}

bool ZStack::startCoordinator(void)
{
    deviceInfoResponseStruct *deviceInfo;
    setChannelRequestStruct channelRequest;
    quint64 ieeeAddress;

    if (!sendRequest(UTIL_GET_DEVICE_INFO) || m_replyData.at(0))
    {
        logWarning << "Device information request failed";
        return false;
    }

    deviceInfo = reinterpret_cast <deviceInfoResponseStruct*> (m_replyData.data());
    ieeeAddress = qFromBigEndian(deviceInfo->ieeeAddress);
    m_ieeeAddress = QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress));

    if (!m_reset)
    {
        bool check = false;

        for (auto it = m_nvItems.begin(); it != m_nvItems.end(); it++)
        {
            nvReadRequestStruct readRequest;
            nvReadReplyStruct *readReply;
            QByteArray data;

            readRequest.id = qToLittleEndian <quint16> (it.key());
            readRequest.offset = 0;

            if (!sendRequest(SYS_OSAL_NV_READ, QByteArray(reinterpret_cast <char*> (&readRequest), sizeof(readRequest))))
            {
                logWarning << "NV item" << QString::asprintf("0x%04X", it.key()) << "read request failed";
                return false;
            }

            readReply = reinterpret_cast <nvReadReplyStruct*> (m_replyData.data());
            data = m_replyData.mid(sizeof(nvReadReplyStruct));

            if (readReply->status || data != it.value())
            {
                if (!m_write)
                {
                    logWarning << "NV item" << QString::asprintf("0x%04X", ZCD_NV_USER) << "value" << data.toHex(':') << "does not match configuration value" << it.value().toHex(':');
                    return false;
                }

                if (it.key() == ZCD_NV_USER)
                {
                    logWarning << "Adapter configuration marker mismatch, resetting...";

                    writeNvItem(ZCD_NV_STARTUP_OPTION, QByteArray(1, 0x03));
                    m_reset = true;
                    resetAdapter();

                    return true;
                }

                if (!writeNvItem(it.key(), it.value()))
                    return false;

                logWarning << "NV item" << QString::asprintf("0x%04X", ZCD_NV_USER) << "value changed from" << data.toHex(':') << "to" << it.value().toHex(':');
                check = true;
            }
        }

        if (check)
        {
            logInfo << "Adapter configuration updated, resetting...";
            resetAdapter();
            return true;
        }
    }
    else
    {
        nvInitRequestStruct request;

        request.id = qToLittleEndian <quint16> (ZCD_NV_USER);
        request.itemLength = qToLittleEndian <quint16> (static_cast <quint16> (m_nvItems.value(ZCD_NV_USER).length()));
        request.dataLength = static_cast <quint8> (m_nvItems.value(ZCD_NV_USER).length());

        if (!sendRequest(SYS_OSAL_NV_ITEM_INIT, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(m_nvItems.value(ZCD_NV_USER))) || (m_replyData.at(0) && m_replyData.at(0) != 0x09))
        {
            logWarning << "NV item" << QString::asprintf("0x%04X", ZCD_NV_USER) << "init request failed";
            return false;
        }

        if (!m_replyData.at(0))
            writeNvItem(ZCD_NV_USER, m_nvItems.value(ZCD_NV_USER));
    }

    if (!sendRequest(SYS_SET_TX_POWER, QByteArray(1, ZSTACK_TX_POWER)) || m_replyData.at(0))
    {
        logWarning << "Set TX power request failed";
        return false;
    }

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

    if (!sendRequest(ZDO_STARTUP_FROM_APP, QByteArray(2, 0x00)) || m_replyData.at(0))
    {
        logWarning << "Strartup request failed";
        return false;
    }

    return true;
}

void ZStack::startTimer(void)
{
    m_timer->start(10);
}

void ZStack::receiveData(void)
{
    QByteArray data = m_port->readAll();

    while (!data.isEmpty())
    {
        quint8 *packet = reinterpret_cast <quint8*> (data.data()), length = packet[1], fcs = 0;
        quint16 command;

        if (data.length() < 5 || data.length() < length + 5 || packet[0] != ZSTACK_PACKET_START)
            return;

        for (quint8 i = 1; i < length + 4; i++)
            fcs ^= packet[i];

        if (fcs != packet[length + 4])
        {
            logWarning << "Packet" << data.left(length + 5).toHex(':') << "FCS mismatch";
            return;
        }

        if (m_debug)
            logInfo << "Packet received:" << data.left(length + 5).toHex(':');

        memcpy(&command, data.constData() + 2, sizeof(command));
        parsePacket(qFromBigEndian(command), QByteArray(reinterpret_cast <char*> (packet + 4), length));
        data.remove(0, length + 5);
    }
}
