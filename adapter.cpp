#include <QtEndian>
#include <QEventLoop>
#include <QHostAddress>
#include <QThread>
#include "adapter.h"
#include "gpio.h"
#include "logger.h"

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

    m_debug = config->value("zigbee/debug", false).toBool();
    m_write = config->value("zigbee/write", false).toBool();

    if (m_channel < 11 || m_channel > 26)
        m_channel = 11;

    logInfo << "Using channel" << m_channel;

    m_endpointsData.insert(0x01, EndpointData(new EndpointDataObject(PROFILE_HA,  0x0005)));
    m_endpointsData.insert(0x0C, EndpointData(new EndpointDataObject(PROFILE_ZLL, 0x0005)));
    m_endpointsData.insert(0xF2, EndpointData(new EndpointDataObject(PROFILE_GP,  0x0061)));

    // TODO: use defines here or remove it after tests
    m_endpointsData.value(0x01)->inClusters()  = {0x0000, 0x0003, 0x0006, 0x000A, 0x0019, 0x001A, 0x0300};
    m_endpointsData.value(0x01)->outClusters() = {0x0000, 0x0003, 0x0004, 0x0005, 0x0006, 0x0008, 0x0020, 0x0300, 0x0400, 0x0402, 0x0405, 0x0406, 0x0500, 0x0B01, 0x0B03, 0x0B04, 0x0702, 0x1000, 0xFC01, 0xFC02};
    m_endpointsData.value(0xF2)->outClusters() = {0x0021};

    connect(m_device, &QIODevice::readyRead, this, &Adapter::startTimer);
    connect(m_receiveTimer, &QTimer::timeout, this, &Adapter::readyRead);
    connect(m_resetTimer, &QTimer::timeout, this, &Adapter::resetTimeout);
    connect(m_permitJoinTimer, &QTimer::timeout, this, &Adapter::permitJoinTimeout);

    m_receiveTimer->setSingleShot(true);
    m_resetTimer->setSingleShot(true);
}

Adapter::~Adapter(void)
{
    if (m_socket && m_connected)
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

void Adapter::reset(void)
{
    QList <QString> list = {"gpio", "flow"};

    logInfo << "Resetting adapter" << QString("(%1)").arg(list.contains(m_reset) ? m_reset : "soft").toUtf8().constData();
    m_resetTimer->start(ADAPTER_RESET_TIMEOUT);

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

QByteArray Adapter::bindRequestPayload(const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId)
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
            default: return QByteArray();
        }
    }

    memcpy(&src, srcAddress.constData(), sizeof(src));

    request.srcAddress = qToLittleEndian(qFromBigEndian(src));
    request.srcEndpointId = srcEndpointId;
    request.clusterId = qToLittleEndian(clusterId);
    request.dstAddressMode = dstAddress.length() == 2 ? ADDRESS_MODE_GROUP : ADDRESS_MODE_64_BIT;

    return QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(reinterpret_cast <char*> (&dst), request.dstAddressMode == ADDRESS_MODE_GROUP ? 2 : 8).append(static_cast <char> (dstEndpointId ? dstEndpointId : 1));
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
