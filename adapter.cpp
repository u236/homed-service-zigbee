#include <QtEndian>
#include <QEventLoop>
#include "logger.h"
#include "adapter.h"

Adapter::Adapter(QSettings *config, QObject *parent) : QObject(parent), m_port(new QSerialPort(this)), m_timer(new QTimer(this))
{
    m_port->setPortName(config->value("zigbee/port", "/dev/ttyUSB0").toString());
    m_port->setBaudRate(QSerialPort::Baud115200);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);

    m_bootPin = static_cast <qint16> (config->value("gpio/boot", -1).toInt());
    m_resetPin = static_cast <qint16> (config->value("gpio/reset", -1).toInt());
    m_reset = config->value("zigbee/reset").toString();

    m_panId = qToLittleEndian <quint16> (config->value("zigbee/panid", "0x1A62").toString().toInt(nullptr, 16));
    m_channel = static_cast <quint8> (config->value("zigbee/channel").toInt());

    m_debug = config->value("zigbee/debug", false).toBool();
    m_write = config->value("zigbee/write", false).toBool();

    if (m_channel < 11 || m_channel > 26)
        m_channel = 11;

    logInfo << "Using channel" << m_channel;

    m_endpointsData.insert(0x01, EndpointData(new EndpointDataObject(PROFILE_HA,  0x0005)));
    m_endpointsData.insert(0x0C, EndpointData(new EndpointDataObject(PROFILE_GP,  0x0005)));
    m_endpointsData.insert(0xF2, EndpointData(new EndpointDataObject(PROFILE_ZLL, 0x0005)));

    // TODO: use defines here
    m_endpointsData.value(0x01)->inClusters() = {0x0000, 0x0003, 0x0006, 0x000A, 0x0019, 0x001A, 0x0300};
    m_endpointsData.value(0x01)->outClusters() = {0x0000, 0x0003, 0x0004, 0x0005, 0x0006, 0x0008, 0x0020, 0x0300, 0x0400, 0x0402, 0x0405, 0x0406, 0x0500, 0x0B01, 0x0B03, 0x0B04, 0x0702, 0x1000, 0xFC01, 0xFC02};
    m_endpointsData.value(0xF2)->outClusters() = {0x0021};

    connect(m_port, &QSerialPort::readyRead, this, &Adapter::readyRead);
    connect(m_timer, &QTimer::timeout, this, &Adapter::receiveData);

    m_timer->setSingleShot(true);
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

bool Adapter::transmitData(const QByteArray &data, quint32 timeout)
{
    if (!m_port->isOpen())
    {
        logWarning << "Port" << m_port->portName() << "is not open";
        reset();
        return false;
    }

    m_port->write(data);

    if (timeout)
    {
        QEventLoop loop;

        if (!m_port->waitForReadyRead(timeout))
        {
            logWarning << "Request timed out";
            reset();
            return false;
        }

        connect(m_timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        m_timer->start(SERIAL_RECEIVE_TIMEOUT);
        loop.exec();
    }

    return true;
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

void Adapter::readyRead(void)
{
    m_timer->start(SERIAL_RECEIVE_TIMEOUT);
}
