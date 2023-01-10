#ifndef CONTROLLER_H
#define CONTROLLER_H

#define SERVICE_VERSION                 "3.1.8"
#define UPDATE_AVAILABILITY_INTERVAL    10000

#include "homed.h"
#include "zigbee.h"

class Controller : public HOMEd
{
    Q_OBJECT

public:

    Controller(void);

    enum class Command
    {
        setPermitJoin,
        setDeviceName,
        removeDevice,
        updateDevice,
        updateReporting,
        bindDevice,
        unbindDevice,
        addGroup,
        removeGroup,
        removeAllGroups,
        otaUpgrade,
        touchLinkScan,
        touchLinkReset
    };

    Q_ENUM(Command)

private:

    QTimer *m_timer;
    ZigBee *m_zigbee;

    QMetaEnum m_commands;

    bool m_names, m_discovery;
    QString m_discoveryPrefix, m_discoveryStatus;

    void publishDiscovery(const Device &device, bool remove = false);
    void publishProperties(void);

public slots:

    void quit(void) override;

private slots:

    void mqttConnected(void) override;
    void mqttReceived(const QByteArray &message, const QMqttTopicName &topic) override;

    void updateAvailability(void);

    void deviceEvent(const Device &device, ZigBee::Event event);
    void endpointUpdated(const Device &device, quint8 endpointId);
    void statusUpdated(const QJsonObject &json);

};

#endif
