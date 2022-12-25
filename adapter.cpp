#include <QtEndian>
#include <QEventLoop>
#include <QHostAddress>
#include <QThread>
#include "adapter.h"
#include "gpio.h"
#include "logger.h"
#include "zcl.h"

Adapter::Adapter(QSettings *config, QObject *parent) : QObject(parent), m_serial(new QSerialPort(this)), m_socket(new QTcpSocket(this)), m_socketTimer(new QTimer(this)), m_receiveTimer(new QTimer(this)), m_resetTimer(new QTimer(this)), m_permitJoinTimer(new QTimer(this)), m_connected(false), m_permitJoin(false)
{
    QString portName = config->value("zigbee/port", "/dev/ttyUSB0").toString();

    if (!portName.startsWith("tcp://"))
    {
        m_device = m_serial;

        m_serial->setPortName(portName);
        m_serial->setBaudRate(config->value("zigbee/baudrate", 115200).toInt());
        m_serial->setDataBits(QSerialPort::Data8);
        m_serial->setParity(QSerialPort::NoParity);
        m_serial->setStopBits(QSerialPort::OneStop);

        m_bootPin = config->value("gpio/boot", "-1").toString();
        m_resetPin = config->value("gpio/reset", "-1").toString();
        m_reset = config->value("zigbee/reset").toString();

        GPIO::direction(m_bootPin, GPIO::Output);
        GPIO::direction(m_resetPin, GPIO::Output);
    }
    else
    {
        QList <QString> list = portName.remove("tcp://").split(":");

        m_device = m_socket;
        m_adddress = QHostAddress(list.value(0));
        m_port = static_cast <quint16> (list.value(1).toInt());

        connect(m_socket, &QTcpSocket::connected, this, &Adapter::socketConnected);
        connect(m_socket, &QTcpSocket::errorOccurred, this, &Adapter::socketError);
        connect(m_socketTimer, &QTimer::timeout, this, &Adapter::socketReconnect);

        m_socketTimer->setSingleShot(true);
    }

    m_panId = static_cast <quint16> (config->value("zigbee/panid", "0x1A62").toString().toInt(nullptr, 16));
    m_channel = static_cast <quint8> (config->value("zigbee/channel").toInt());

    m_write = config->value("zigbee/write", false).toBool();
    m_debug = config->value("debug/adapter", false).toBool();

    if (m_channel < 11 || m_channel > 26)
        m_channel = 11;

    logInfo << "Using channel" << m_channel;

    m_endpointsData.insert(0x01, EndpointData(new EndpointDataObject(PROFILE_HA,  0x0005)));
    m_endpointsData.insert(0x0C, EndpointData(new EndpointDataObject(PROFILE_ZLL, 0x0005)));
    m_endpointsData.insert(0xF2, EndpointData(new EndpointDataObject(PROFILE_GP,  0x0061)));

    m_endpointsData.value(0x01)->inClusters()  = {CLUSTER_BASIC, CLUSTER_IDENTIFY, CLUSTER_ON_OFF, CLUSTER_TIME, CLUSTER_OTA_UPGRADE, CLUSTER_POWER_PROFILE, CLUSTER_COLOR_CONTROL};
    m_endpointsData.value(0x01)->outClusters() = {CLUSTER_BASIC, CLUSTER_IDENTIFY, CLUSTER_GROUPS, CLUSTER_SCENES, CLUSTER_ON_OFF, CLUSTER_LEVEL_CONTROL, CLUSTER_POLL_CONTROL, CLUSTER_COLOR_CONTROL, CLUSTER_ILLUMINANCE_MEASUREMENT, CLUSTER_TEMPERATURE_MEASUREMENT, CLUSTER_PRESSURE_MEASUREMENT, CLUSTER_RELATIVE_HUMIDITY, CLUSTER_OCCUPANCY_SENSING, CLUSTER_SOIL_MOISTURE, CLUSTER_IAS_ZONE, CLUSTER_SMART_ENERGY_METERING, CLUSTER_ELECTRICAL_MEASUREMENT, CLUSTER_TOUCHLINK};
    m_endpointsData.value(0xF2)->outClusters() = {CLUSTER_GREEN_POWER};

    connect(m_device, &QIODevice::readyRead, this, &Adapter::startTimer);
    connect(m_receiveTimer, &QTimer::timeout, this, &Adapter::readyRead);
    connect(m_resetTimer, &QTimer::timeout, this, &Adapter::resetTimeout);
    connect(m_permitJoinTimer, &QTimer::timeout, this, &Adapter::permitJoinTimeout);

    m_receiveTimer->setSingleShot(true);
    m_resetTimer->setSingleShot(true);
}

Adapter::~Adapter(void)
{
    if (m_connected)
        m_socket->disconnectFromHost();
}

void Adapter::init(void)
{
    if (m_device == m_serial)
    {
        m_serial->close();

        if (!m_serial->open(QIODevice::ReadWrite))
        {
            logWarning << "Can't open port" << m_serial->portName();
            return;
        }

        reset();
        return;
    }

    if (m_adddress.isNull() && !m_port)
    {
        logWarning << "Invalid connection address or port number";
        return;
    }

    if (m_connected)
        m_socket->disconnectFromHost();

    m_socket->connectToHost(m_adddress, m_port);
}

void Adapter::setPermitJoin(bool enabled)
{
    if (!permitJoin(enabled))
        return;

    if (m_permitJoin != enabled)
    {
        logInfo << "Permit join" << (enabled ? "enabled" : "disabled") << "successfully";

        if (enabled)
            m_permitJoinTimer->start(PERMIT_JOIN_TIMEOUT);
        else
            m_permitJoinTimer->stop();

        m_permitJoin = enabled;
        emit permitJoinUpdated(enabled);
    }
}

bool Adapter::nodeDescriptorRequest(quint8 id, quint16 networkAddress)
{
    quint16 data = qToLittleEndian(networkAddress);
    return unicastRequest(id, networkAddress, APS_NODE_DESCRIPTOR, 0x00, 0x00, QByteArray(1, static_cast <char> (id)).append(reinterpret_cast <char*> (&data), sizeof(data)));
}

bool Adapter::simpleDescriptorRequest(quint8 id, quint16 networkAddress, quint8 endpointId)
{
    quint16 data = qToLittleEndian(networkAddress);
    return unicastRequest(id, networkAddress, APS_SIMPLE_DESCRIPTOR, 0x00, 0x00, QByteArray(1, static_cast <char> (id)).append(reinterpret_cast <char*> (&data), sizeof(data)).append(static_cast <quint8> (endpointId)));
}

bool Adapter::activeEndpointsRequest(quint8 id, quint16 networkAddress)
{
    quint16 data = qToLittleEndian(networkAddress);
    return unicastRequest(id, networkAddress, APS_ACTIVE_ENDPOINTS, 0x00, 0x00, QByteArray(1, static_cast <char> (id)).append(reinterpret_cast <char*> (&data), sizeof(data)));
}

bool Adapter::bindRequest(quint8 id, quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind)
{
    bindRequestStruct request;
    quint64 src, dst = m_ieeeAddress;
    QTimer timer;

    if (!dstAddress.isEmpty())
    {
        memcpy(&dst, dstAddress.constData(), sizeof(dst));

        switch (dstAddress.length())
        {
            case 2:  break;
            case 8:  dst = qToLittleEndian(qFromBigEndian(dst)); break;
            default: return false;
        }
    }

    memcpy(&src, srcAddress.constData(), sizeof(src));

    request.srcAddress = qToLittleEndian(qFromBigEndian(src));
    request.srcEndpointId = srcEndpointId;
    request.clusterId = qToLittleEndian(clusterId);
    request.dstAddressMode = dstAddress.length() == 2 ? ADDRESS_MODE_GROUP : ADDRESS_MODE_64_BIT;

    return unicastRequest(id, networkAddress, unbind ? APS_UNBIND : APS_BIND, 0x00, 0x00, QByteArray(1, static_cast <char> (id)).append(reinterpret_cast <char*> (&request), sizeof(request)).append(reinterpret_cast <char*> (&dst), request.dstAddressMode == ADDRESS_MODE_GROUP ? 2 : 8).append(static_cast <char> (dstEndpointId ? dstEndpointId : 1)));
}

bool Adapter::lqiRequest(quint8 id, quint16 networkAddress, quint8 index)
{
    return unicastRequest(id, networkAddress, APS_LQI, 0x00, 0x00, QByteArray(1, static_cast <char> (id)).append(1, static_cast <char> (index)));
}

bool Adapter::leaveRequest(quint8 id, quint16 networkAddress, const QByteArray &ieeeAddress)
{
    quint64 address;

    memcpy(&address, ieeeAddress.constData(), sizeof(address));
    address = qToLittleEndian(qFromBigEndian(address));

    return unicastRequest(id, networkAddress, APS_LEAVE, 0x00, 0x00, QByteArray(1, static_cast <char> (id)).append(reinterpret_cast <char*> (&address), sizeof(address)).append(1, 0x00));
}

bool Adapter::dataRequest(quint8 id, quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &payload)
{
    return unicastRequest(id, networkAddress, clusterId, 0x01, endpointId, payload);
}

void Adapter::reset(void)
{
    QList <QString> list = {"gpio", "flow"};

    m_device->readAll();
    m_resetTimer->start(ADAPTER_RESET_TIMEOUT);

    logInfo << "Resetting adapter" << QString("(%1)").arg(list.contains(m_reset) ? m_reset : "soft").toUtf8().constData();

    switch (list.indexOf(m_reset))
    {
        case 0:
            GPIO::setStatus(m_bootPin, true);
            GPIO::setStatus(m_resetPin, false);
            QThread::msleep(ADAPTER_RESET_DELAY);
            GPIO::setStatus(m_resetPin, true);
            break;

        case 1:
            m_serial->setDataTerminalReady(false);
            m_serial->setRequestToSend(true);
            QThread::msleep(ADAPTER_RESET_DELAY);
            m_serial->setRequestToSend(false);
            break;

        default:
            softReset();
            break;
    }
}

bool Adapter::waitForSignal(const QObject *sender, const char *signal, int tiomeout)
{
    QEventLoop loop;
    QTimer timer;

    connect(sender, signal, &loop, SLOT(quit()));
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.setSingleShot(true);
    timer.start(tiomeout);
    loop.exec();

    return timer.isActive();
}

void Adapter::parseMessage(quint16 networkAddress, quint16 clusterId, const QByteArray &payload)
{
    switch (clusterId & 0x00FF)
    {
        case APS_BIND:
        case APS_UNBIND:
        case APS_LEAVE:
            break;

        case APS_NODE_DESCRIPTOR:
        {
            const nodeDescriptorResponseStruct *response = reinterpret_cast <const nodeDescriptorResponseStruct*> (payload.constData());

            if (!response->status)
                emit nodeDescriptorReceived(qFromLittleEndian(response->networkAddress), static_cast <LogicalType> (response->logicalType & 0x03), qFromLittleEndian(response->manufacturerCode));

            break;
        }

        case APS_SIMPLE_DESCRIPTOR:
        {
            const simpleDescriptorResponseStruct *response = reinterpret_cast <const simpleDescriptorResponseStruct*> (payload.constData());
            QList <quint16> inClusters, outClusters;

            if (!response->status)
            {
                QByteArray clusterData = payload.mid(sizeof(simpleDescriptorResponseStruct));
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
                break;
            }

            emit simpleDescriptorReceived(qFromLittleEndian(response->networkAddress), 0, 0, 0, inClusters, outClusters);
            break;
        }

        case APS_ACTIVE_ENDPOINTS:
        {
            const activeEndpointsResponseStruct *response = reinterpret_cast <const activeEndpointsResponseStruct*> (payload.constData());

            if (!response->status)
                emit activeEndpointsReceived(qFromLittleEndian(response->networkAddress), payload.mid(sizeof(activeEndpointsResponseStruct), response->count));

            break;
        }

        case APS_LQI:
        {
            const lqiResponseStruct *response = reinterpret_cast <const lqiResponseStruct*> (payload.constData());

            if (!response->status)
            {
                for (quint8 i = 0; i < response->count; i++)
                {
                    const neighborRecordStruct *neighbor = reinterpret_cast <const neighborRecordStruct*> (payload.constData() + sizeof(lqiResponseStruct) + i * sizeof(neighborRecordStruct));
                    emit neighborRecordReceived(networkAddress, qFromLittleEndian(neighbor->networkAddress), neighbor->linkQuality, !(response->index | i));
                }

                if (response->index + response->count >= response->total)
                    break;

                lqiRequest(networkAddress, response->index + response->count);
            }

            break;
        }

        default:
        {
            logWarning << "Unrecognized ZDO message received from cluster" << QString::asprintf("0x%04x", clusterId) << "with payload" << (payload.isEmpty() ? "(empty)" : payload.toHex(':'));
            break;
        }
    }
}

void Adapter::socketConnected(void)
{
    logInfo << "Successfully connected to" << m_adddress.toString();
    m_connected = true;
    reset();
}

void Adapter::socketError(QAbstractSocket::SocketError error)
{
    logWarning << "Connection error:" << error;
    m_connected = false;
    m_socketTimer->start(SOCKET_RECONNECT_INTERVAL);
}

void Adapter::socketReconnect(void)
{
    m_socket->connectToHost(m_adddress, m_port);
}

void Adapter::startTimer(void)
{
    m_receiveTimer->start(DEVICE_RECEIVE_TIMEOUT);
}

void Adapter::readyRead(void)
{
    parseData();
    QTimer::singleShot(0, this, &Adapter::handleQueue);
}

void Adapter::resetTimeout(void)
{
    logWarning << "Adapter reset timed out";
    init();
}

void Adapter::permitJoinTimeout(void)
{
    if (permitJoin(true))
        return;

    m_permitJoinTimer->stop();
    emit permitJoinUpdated(false);
}
