#ifndef CONTROLLER_H
#define CONTROLLER_H

#define SERVICE_VERSION     "3.0.10"

#include "homed.h"
#include "zigbee.h"

class Controller : public HOMEd
{
    Q_OBJECT

public:

    Controller(void);

private:

    ZigBee *m_zigbee;

private slots:

    void mqttConnected(void) override;
    void mqttReceived(const QByteArray &message, const QMqttTopicName &topic) override;

    void endDeviceEvent(bool join = true);
    void endPointUpdated(const EndPoint &endPoint);
    void statusStored(const QJsonObject &json);

};

#endif
