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

    connect(m_port, &QSerialPort::readyRead, this, &Adapter::readyRead);
    connect(m_timer, &QTimer::timeout, this, &Adapter::receiveData);

    m_timer->setSingleShot(true);
}

bool Adapter::sendData(const QByteArray &data)
{
    QEventLoop loop;

    if (!m_port->isOpen())
    {
        logWarning << "Port" << m_port->portName() << "is not open";
        reset();
        return false;
    }

    m_receiveBuffer.clear();
    m_port->write(data);

    if (!m_port->waitForReadyRead(ADAPTER_REQUEST_TIMEOUT))
    {
        logWarning << "Request timed out";
        reset();
        return false;
    }

    connect(m_timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    readyRead();
    loop.exec();

    return true;
}

void Adapter::readyRead(void)
{
    m_receiveBuffer.append(m_port->readAll());
    m_timer->start(ADAPTER_RECEIVE_TIMEOUT);
}
