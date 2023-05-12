#ifndef CONTROLLER_H
#define CONTROLLER_H

#define SERVICE_VERSION                 "3.2.5"
#define UPDATE_AVAILABILITY_INTERVAL    10000

#include "homed.h"
#include "zigbee.h"

class Controller : public HOMEd
{
    Q_OBJECT

public:

    Controller(const QString &configFile);

    enum class Command
    {
        setPermitJoin,
        removeDevice,
        setDeviceName,
        updateDevice,
        updateReporting,
        bindDevice,
        unbindDevice,
        addGroup,
        removeGroup,
        removeAllGroups,
        otaUpgrade,
        getProperties,
        clusterRequest,
        globalRequest,
        touchLinkScan,
        touchLinkReset
    };

    Q_ENUM(Command)

private:

    QTimer *m_timer;
    ZigBee *m_zigbee;

    QMetaEnum m_commands;

    bool m_homeassistant;
    QString m_homeassistantPrefix, m_homeassistantStatus;

    void publishExposes(const Device &device, bool remove = false);
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
