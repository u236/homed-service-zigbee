#include <netinet/tcp.h>
#include <QtEndian>
#include <QEventLoop>
#include <QThread>
#include "adapter.h"
#include "gpio.h"
#include "logger.h"
#include "zcl.h"

Adapter::Adapter(QSettings *config, QObject *parent) : QObject(parent), m_receiveTimer(new QTimer(this)), m_resetTimer(new QTimer(this)), m_permitJoinTimer(new QTimer(this)), m_serial(new QSerialPort(this)), m_socket(new QTcpSocket(this)), m_serialError(false), m_connected(false), m_permitJoinAddress(PERMIT_JOIN_BROARCAST_ADDRESS), m_permitJoin(false)
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

        connect(m_serial, &QSerialPort::errorOccurred, this, &Adapter::serialError);
    }
    else
    {
        QList <QString> list = portName.remove("tcp://").split(':');

        m_device = m_socket;
        m_adddress = QHostAddress(list.value(0));
        m_port = static_cast <quint16> (list.value(1).toInt());

        connect(m_socket, &QTcpSocket::errorOccurred, this, &Adapter::socketError);
        connect(m_socket, &QTcpSocket::connected, this, &Adapter::socketConnected);
    }

    m_panId = static_cast <quint16> (config->value("zigbee/panid", "0x1010").toString().toInt(nullptr, 16));
    m_channel = static_cast <quint8> (config->value("zigbee/channel", 11).toInt());
    m_power = static_cast <quint8> (config->value("zigbee/power", 20).toInt());

    m_write = config->value("zigbee/write", false).toBool();
    m_portDebug = config->value("debug/port", false).toBool();
    m_adapterDebug = config->value("debug/adapter", false).toBool();

    m_networkKey = QByteArray::fromHex(config->value("security/key", "000102030405060708090a0b0c0d0e0f").toString().remove("0x").toUtf8());

    if (m_channel < 11 || m_channel > 26)
        m_channel = 11;

    logInfo << "Using channel" << m_channel << "and PAN ID" << QString::asprintf("0x%04x", m_panId);
    m_defaultKey = QByteArray::fromHex("5a6967426565416c6c69616e63653039");

    m_endpoints.insert(0x01, EndpointData(new EndpointDataObject(PROFILE_HA,   0x0005)));
    m_endpoints.insert(0x02, EndpointData(new EndpointDataObject(PROFILE_IPM,  0x0005)));
    m_endpoints.insert(0x03, EndpointData(new EndpointDataObject(PROFILE_HA,   0x0005)));
    m_endpoints.insert(0x04, EndpointData(new EndpointDataObject(PROFILE_TA,   0x0005)));
    m_endpoints.insert(0x05, EndpointData(new EndpointDataObject(PROFILE_PHHC, 0x0005)));
    m_endpoints.insert(0x06, EndpointData(new EndpointDataObject(PROFILE_AMI,  0x0005)));
    m_endpoints.insert(0x07, EndpointData(new EndpointDataObject(PROFILE_HA,   0x0005)));
    m_endpoints.insert(0x08, EndpointData(new EndpointDataObject(PROFILE_HA,   0x0005)));
    m_endpoints.insert(0x0C, EndpointData(new EndpointDataObject(PROFILE_ZLL,  0x0005)));
    m_endpoints.insert(0xF2, EndpointData(new EndpointDataObject(PROFILE_GP,   0x0061)));

    m_endpoints.value(0x01)->inClusters() =
    {
        CLUSTER_BASIC,
        CLUSTER_ON_OFF,
        CLUSTER_LEVEL_CONTROL,
        CLUSTER_TIME,
        CLUSTER_OTA_UPGRADE,
        CLUSTER_POWER_PROFILE,
        CLUSTER_COLOR_CONTROL
    };

    m_endpoints.value(0x01)->outClusters() =
    {
        CLUSTER_BASIC,
        CLUSTER_GROUPS,
        CLUSTER_SCENES,
        CLUSTER_ON_OFF,
        CLUSTER_LEVEL_CONTROL,
        CLUSTER_POLL_CONTROL,
        CLUSTER_COLOR_CONTROL,
        CLUSTER_ILLUMINANCE_MEASUREMENT,
        CLUSTER_TEMPERATURE_MEASUREMENT,
        CLUSTER_PRESSURE_MEASUREMENT,
        CLUSTER_HUMIDITY_MEASUREMENT,
        CLUSTER_OCCUPANCY_SENSING,
        CLUSTER_MOISTURE_MEASUREMENT,
        CLUSTER_CO2_CONCENTRATION,
        CLUSTER_PM25_CONCENTRATION,
        CLUSTER_IAS_ZONE,
        CLUSTER_SMART_ENERGY_METERING,
        CLUSTER_ELECTRICAL_MEASUREMENT,
        CLUSTER_TOUCHLINK
    };

    m_endpoints.value(0xF2)->outClusters() =
    {
        CLUSTER_GREEN_POWER
    };

    m_multicast.append(DEFAULT_GROUP);
    m_multicast.append(IKEA_GROUP);
    m_multicast.append(GREEN_POWER_GROUP);

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
        if (m_serial->isOpen())
            m_serial->close();

        if (m_serial->open(QIODevice::ReadWrite))
        {
            logInfo << "Port" << m_serial->portName() << "opened successfully";
            reset();
        }
    }
    else
    {
        if (m_adddress.isNull() && !m_port)
        {
            logWarning << "Invalid connection address or port number";
            return;
        }

        if (m_connected)
            m_socket->disconnectFromHost();

        m_socket->connectToHost(m_adddress, m_port);
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

void Adapter::togglePermitJoin(void)
{
    setPermitJoin(m_permitJoin ? false : true);
}

bool Adapter::zdoRequest(quint8 id, quint16 networkAddress, quint16 clusterId, const QByteArray &data)
{
    quint16 dstAddress = qToLittleEndian(networkAddress);
    return unicastRequest(id, networkAddress, 0x00, 0x00, clusterId, QByteArray(1, static_cast <char> (id)).append(reinterpret_cast <char*> (&dstAddress), sizeof(dstAddress)).append(data));
}

bool Adapter::bindRequest(quint8 id, quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &address, quint8 dstEndpointId, bool unbind)
{
    QByteArray buffer = address.isEmpty() ? m_ieeeAddress : address;
    bindRequestStruct request;
    quint64 dstAddress;

    memcpy(&request.srcAddress, m_requestAddress.constData(), sizeof(request.srcAddress));
    memcpy(&dstAddress, buffer.constData(), sizeof(dstAddress));

    request.srcAddress = qToLittleEndian(qFromBigEndian(request.srcAddress));
    request.srcEndpointId = endpointId;
    request.clusterId = qToLittleEndian(clusterId);
    request.dstAddressMode = buffer.length() == 2 ? ADDRESS_MODE_GROUP : ADDRESS_MODE_64_BIT;

    if (request.dstAddressMode != ADDRESS_MODE_GROUP)
        dstAddress = qToLittleEndian(qFromBigEndian(dstAddress));

    return unicastRequest(id, networkAddress, 0x00, 0x00, unbind ? ZDO_UNBIND_REQUEST : ZDO_BIND_REQUEST, QByteArray(1, static_cast <char> (id)).append(reinterpret_cast <char*> (&request), sizeof(request)).append(reinterpret_cast <char*> (&dstAddress), request.dstAddressMode == ADDRESS_MODE_GROUP ? 2 : 8).append(static_cast <char> (dstEndpointId ? dstEndpointId : 0x01)));
}

bool Adapter::leaveRequest(quint8 id, quint16 networkAddress)
{
    quint64 dstAddress;

    memcpy(&dstAddress, m_requestAddress.constData(), sizeof(dstAddress));
    dstAddress = qToLittleEndian(qFromBigEndian(dstAddress));

    return unicastRequest(id, networkAddress, 0x00, 0x00, ZDO_LEAVE_REQUEST, QByteArray(1, static_cast <char> (id)).append(reinterpret_cast <char*> (&dstAddress), sizeof(dstAddress)).append(1, 0x00));
}

bool Adapter::lqiRequest(quint8 id, quint16 networkAddress, quint8 index)
{
    return unicastRequest(id, networkAddress, 0x00, 0x00, ZDO_LQI_REQUEST, QByteArray(1, static_cast <char> (id)).append(1, static_cast <char> (index)));
}

void Adapter::reset(void)
{
    QList <QString> list = {"gpio", "flow"};

    m_device->readAll();
    m_resetTimer->start(RESET_TIMEOUT);

    logInfo << "Resetting adapter" << QString("(%1)").arg(list.contains(m_reset) ? m_reset : "soft").toUtf8().constData();
    emit adapterReset();

    switch (list.indexOf(m_reset))
    {
        case 0:
            GPIO::setStatus(m_resetPin, false);
            GPIO::setStatus(m_bootPin, true);
            QThread::msleep(RESET_DELAY);
            GPIO::setStatus(m_resetPin, true);
            break;

        case 1:
            m_serial->setRequestToSend(true);
            m_serial->setDataTerminalReady(false);
            QThread::msleep(RESET_DELAY);
            m_serial->setRequestToSend(false);
            break;

        default:
            softReset();
            break;
    }
}

void Adapter::sendData(const QByteArray &buffer)
{
    logDebug(m_portDebug) << "Serial data sent:" << buffer.toHex(':');
    m_device->write(buffer);
}

void Adapter::serialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::SerialPortError::NoError)
    {
        m_serialError = false;
        return;
    }

    if (!m_serialError)
        logWarning << "Serial port error:" << error;

    m_resetTimer->start(RESET_TIMEOUT);
    m_serialError = true;
}

void Adapter::socketError(QTcpSocket::SocketError error)
{
    logWarning << "Connection error:" << error;
    m_resetTimer->start(RESET_TIMEOUT);
    m_connected = false;
}

void Adapter::socketConnected(void)
{
    int descriptor = m_socket->socketDescriptor(), keepAlive = 1, interval = 10, count = 3;

    setsockopt(descriptor, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive));
    setsockopt(descriptor, SOL_TCP, TCP_KEEPIDLE, &interval, sizeof(interval));
    setsockopt(descriptor, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    setsockopt(descriptor, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count));

    logInfo << "Successfully connected to" << QString("%1:%2").arg(m_adddress.toString()).arg(m_port);

    m_connected = true;
    reset();
}

void Adapter::startTimer(void)
{
    m_receiveTimer->start(RECEIVE_TIMEOUT);
}

void Adapter::readyRead(void)
{
    QByteArray buffer = m_device->readAll();

    logDebug(m_portDebug)  << "Serial data received:" << buffer.toHex(':');
    parseData(buffer);

    QTimer::singleShot(0, this, &Adapter::handleQueue);
}

void Adapter::resetTimeout(void)
{
    if (m_serial->isOpen() || m_connected)
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
