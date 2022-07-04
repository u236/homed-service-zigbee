#include <QtEndian>
#include <QEventLoop>
#include <QThread>
#include "gpio.h"
#include "logger.h"
#include "zstack.h"

ZStack::ZStack(QSettings *config, QObject *parent) : QObject(parent), m_port(new QSerialPort(this)), m_timer(new QTimer(this)), m_status(0), m_transactionId(0)
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
    m_debug = config->value("zigbee/debug", false).toBool();
    m_rts = config->value("zigbee/rts", false).toBool();

    m_nvValues.insert(ZCD_NV_PRECFGKEY, QByteArray::fromHex(config->value("security/key", "01030507090B0D0F00020406080A0C0D").toString().toUtf8()));
    m_nvValues.insert(ZCD_NV_PRECFGKEYS_ENABLE, QByteArray(1, config->value("security/enabled", false).toBool() ? 0x01 : 0x00));
    m_nvValues.insert(ZCD_NV_PANID, QByteArray(reinterpret_cast <char*> (&panId), sizeof(panId)));
    m_nvValues.insert(ZCD_NV_CHANLIST, QByteArray(reinterpret_cast <char*> (&chanList), sizeof(chanList)));
    m_nvValues.insert(ZCD_NV_LOGICAL_TYPE, QByteArray(1, 0x00));
    m_nvValues.insert(ZCD_NV_ZDO_DIRECT_CB, QByteArray(1, 0x01));

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

void ZStack::registerEndPoint(quint8 endPointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters)
{
    registerEndPointRequestStruct request;
    QByteArray data;

    request.endPointId = endPointId;
    request.profileId = qToLittleEndian(profileId);
    request.deviceId = qToLittleEndian(deviceId);
    request.version = 0;
    request.latency = 0;

    data.append(static_cast <char> (inClusters.count()));

    for (quint8 i = 0; i < static_cast <quint8> (inClusters.count()); i++)
    {
        quint16 clusterId = inClusters.value(i);
        data.append(reinterpret_cast <char*> (&clusterId), sizeof(clusterId));
    }

    data.append(static_cast <char> (outClusters.count()));

    for (quint8 i = 0; i < static_cast <quint8> (outClusters.count()); i++)
    {
        quint16 clusterId = outClusters.value(i);
        data.append(reinterpret_cast <char*> (&clusterId), sizeof(clusterId));
    }

    if (!sendRequest(AF_REGISTER, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyData.at(0))
    {
        logWarning << "EndPoint" << QString::asprintf("0x%02X", endPointId) << "registration error";
        return;
    }

    logInfo << "EndPoint" << QString::asprintf("0x%02X", endPointId) << "registered successfully";
}

void ZStack::setPermitJoin(bool enabled)
{
    permitJoinRequestStruct request;

    request.mode = PERMIT_JOIN_MODE_ADDREESS;
    request.dstAddress = qToLittleEndian <quint16> (PERMIT_JOIN_BROARCAST_ADDRESS);
    request.duration = enabled ? 0xFF : 0;
    request.significance = 0;

    if (!sendRequest(ZDO_MGMT_PERMIT_JOIN_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || m_replyData.at(0))
    {
        logWarning << "Set permit join error";
        return;
    }

    logInfo << "Permit join" << (enabled ? "enabled" : "disabled") << "successfully";
}

void ZStack::nodeDescriptorRequest(quint16 networkAddress)
{
    nodeDescriptorRequestStruct request;

    request.dstAddress = qToLittleEndian(networkAddress);
    request.networkAddress = qToLittleEndian(networkAddress);

    sendRequest(ZDO_NODE_DESC_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)));
}

void ZStack::simpleDescriptorRequest(quint16 networkAddress, quint8 endPointId)
{
    simpleDescriptorRequestStruct request;

    request.dstAddress = qToLittleEndian(networkAddress);
    request.networkAddress = qToLittleEndian(networkAddress);
    request.endPointId = endPointId;

    sendRequest(ZDO_SIMPLE_DESC_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)));
}

void ZStack::activeEndPointsRequest(quint16 networkAddress)
{
    activeEndPointsRequestStruct request;

    request.dstAddress = qToLittleEndian(networkAddress);
    request.networkAddress = qToLittleEndian(networkAddress);

    sendRequest(ZDO_ACTIVE_EP_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)));
}

bool ZStack::dataRequest(quint16 networkAddress, quint8 endPointId, quint16 clusterId, const QByteArray &data)
{
    dataRequestStruct request;
    QEventLoop loop;
    QTimer timer;

    request.networkAddress = qToLittleEndian(networkAddress);
    request.dstEndPointId = endPointId;
    request.srcEndPointId = 0x01;
    request.clusterId = qToLittleEndian(clusterId);
    request.transactionId = m_transactionId;
    request.options = AF_ACK_REQUEST | AF_DISCV_ROUTE;
    request.radius = AF_DEFAULT_RADIUS;
    request.length = static_cast <quint8> (data.length());

    connect(this, &ZStack::dataConfirm, &loop, &QEventLoop::quit);

    if(!sendRequest(AF_DATA_REQUEST, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyData.at(0))
    {
        logWarning << "Data request failed";
        return false;
    }

    timer.singleShot(ADAPTER_REQUEST_TIMEOUT, &loop, &QEventLoop::quit);
    loop.exec();
    m_transactionId++;

    return timer.isActive() && m_requestSuccess;
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
        case ZDO_TC_DEV_IND:
            break;

        case SYS_RESET_IND:
        {
            if (!startCoordinator())
                logWarning << "Coordinator startup failed";

            break;
        }

        case AF_DATA_CONFIRM:

            if (static_cast <quint8> (data.at(2)) == m_transactionId)
            {
                m_requestSuccess = data.at(0) ? false : true;
                emit dataConfirm();
            }

            break;

        case AF_INCOMING_MSG:
        {
            const incomingMessageStruct *message = reinterpret_cast <const incomingMessageStruct*> (data.constData());
            emit messageReveived(qFromLittleEndian(message->srcAddress), message->srcEndPointId, qFromLittleEndian(message->clusterId), message->linkQuality, data.mid(sizeof(incomingMessageStruct), message->length));
            break;
        }

        case ZDO_NODE_DESC_RSP:
        {
            const nodeDescriptorResponseStruct *message = reinterpret_cast <const nodeDescriptorResponseStruct*> (data.constData());

            if (!message->status)
                emit nodeDescriptorReceived(qFromLittleEndian(message->srcAddress), qFromLittleEndian(message->manufacturerCode), static_cast <LogicalType> (message->logicalType & 0x03));

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

                emit simpleDescriptorReceived(qFromLittleEndian(message->srcAddress), message->endPointId, qFromLittleEndian(message->profileId), qFromLittleEndian(message->deviceId), inClusters, outClusters);
            }

            break;
        }

        case ZDO_ACTIVE_EP_RSP:
        {
            const activeEndPointsResponseStruct *message = reinterpret_cast <const activeEndPointsResponseStruct*> (data.constData());

            if (!message->status)
                emit activeEndPointsReceived(qFromLittleEndian(message->srcAddress), data.mid(sizeof(activeEndPointsResponseStruct), message->count));

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
            const endDeviceAnnounceStruct *message = reinterpret_cast <const endDeviceAnnounceStruct*> (data.constData());
            quint64 ieeeAddress = qFromBigEndian(message->ieeeAddress);
            emit endDeviceJoined(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->networkAddress), message->capabilities);
            break;
        }

        case ZDO_LEAVE_IND:
        {
            const endDeviceLeaveStruct *message = reinterpret_cast <const endDeviceLeaveStruct*> (data.constData());
            quint64 ieeeAddress = qFromBigEndian(message->ieeeAddress);
            emit endDeviceLeft(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->networkAddress));
            break;
        }

        case APP_CNF_BDB_COMMISSIONING_NOTIFICATION:
        {
            if (!data.at(2) && m_status == ZSTACK_COORDINATOR_STARTED)
                emit coordinatorReady(m_ieeeAddress);

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

    for (quint8 i = 1; i < packet.length(); i++)
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

bool ZStack::startCoordinator(void)
{
    deviceInfoResponseStruct *deviceInfo;
    setChannelRequestStruct channelRequest;
    quint64 ieeeAddress;
    bool reset = false;

    logInfo << "Adapter reset detected, starting coordinator...";

    // TODO: check settings marker

    for (auto it = m_nvValues.begin(); it != m_nvValues.end(); it++)
    {
        nvReadRequestStruct readRequest;
        nvReadReplyStruct *readReply;
        QByteArray data;

        readRequest.id = qToLittleEndian <qint16> (it.key());
        readRequest.offset = 0;

        if (!sendRequest(SYS_OSAL_NV_READ, QByteArray(reinterpret_cast <char*> (&readRequest), sizeof(readRequest))))
            return false;

        readReply = reinterpret_cast <nvReadReplyStruct*> (m_replyData.data());
        data = m_replyData.mid(sizeof(nvReadReplyStruct));

        if (readReply->status || readReply->length != it.value().length())
        {
            logWarning << "NV item" << QString::asprintf("0x%04X", it.key()) << "read error";
            return false;
        }

        if (data != it.value())
        {
            nvWriteRequestStruct writeRequest;

            writeRequest.id = qToLittleEndian(it.key());
            writeRequest.offset = 0;
            writeRequest.length = static_cast <quint8> (it.value().length());

            if (!sendRequest(SYS_OSAL_NV_WRITE, QByteArray(reinterpret_cast <char*> (&writeRequest), sizeof(writeRequest)).append(it.value())))
                return false;

            reset = true;
        }
    }

    if (reset)
    {
        logInfo << "Adapter configuration changed, resetting...";
        resetAdapter();
        return true;
    }

    if (!sendRequest(UTIL_GET_DEVICE_INFO, QByteArray(1, ZSTACK_TX_POWER)))
        return false;

    deviceInfo = reinterpret_cast <deviceInfoResponseStruct*> (m_replyData.data());
    ieeeAddress = qFromBigEndian(deviceInfo->ieeeAddress);
    m_ieeeAddress = QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress));

    if (!sendRequest(SYS_SET_TX_POWER, QByteArray(1, ZSTACK_TX_POWER)))
        return false;

    channelRequest.isPrimary = 0x01;
    channelRequest.channel = qToLittleEndian <quint32> (1 << m_channel);

    if (!sendRequest(APP_CNF_BDB_SET_CHANNEL, QByteArray(reinterpret_cast <char*> (&channelRequest), sizeof(channelRequest))))
        return false;

    channelRequest.isPrimary = 0x00;
    channelRequest.channel = 0x00;

    if (!sendRequest(APP_CNF_BDB_SET_CHANNEL, QByteArray(reinterpret_cast <char*> (&channelRequest), sizeof(channelRequest))))
        return false;

    return sendRequest(ZDO_STARTUP_FROM_APP, QByteArray(2, 0x00)) && !m_replyData.at(0);
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
