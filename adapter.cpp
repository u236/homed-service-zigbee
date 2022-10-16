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

    m_panId = qToLittleEndian(static_cast <quint16> (config->value("zigbee/panid", "0x1A62").toString().toInt(nullptr, 16)));
    m_channel = static_cast <quint8>  (config->value("zigbee/channel").toInt());

    m_debug = config->value("zigbee/debug", false).toBool();
    m_write = config->value("zigbee/write", false).toBool();

    if (m_channel < 11 || m_channel > 26)
        m_channel = 11;

    logInfo << "Using channel" << m_channel;

    m_endpointsData.insert(0x01, EndpointData(new EndpointDataObject(PROFILE_HA,  0x0005)));
    m_endpointsData.insert(0x0C, EndpointData(new EndpointDataObject(PROFILE_ZLL, 0x0005)));
    m_endpointsData.insert(0xF2, EndpointData(new EndpointDataObject(PROFILE_HUE, 0x0005)));

    connect(m_port, &QSerialPort::readyRead, this, &Adapter::readyRead);
    connect(m_timer, &QTimer::timeout, this, &Adapter::receiveData);

    m_timer->setSingleShot(true);
}

bool Adapter::waitForSignal(const QObject *sender, const char *signal, int tiomeout) // TODO: use this in ZStack
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

bool Adapter::transmitData(const QByteArray &data, bool receive)
{
    if (!m_port->isOpen())
    {
        logWarning << "Port" << m_port->portName() << "is not open";
        reset();
        return false;
    }

    m_port->write(data);

    if (receive)
    {
        QEventLoop loop;

        if (!m_port->waitForReadyRead(ADAPTER_REQUEST_TIMEOUT))
        {
            logWarning << "Request timed out";
            reset();
            return false;
        }

        connect(m_timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        m_timer->start(ADAPTER_RECEIVE_TIMEOUT);
        loop.exec();
    }

    return true;
}

void Adapter::readyRead(void)
{
    m_timer->start(ADAPTER_RECEIVE_TIMEOUT);
}
