#ifndef CONTROLLER_H
#define CONTROLLER_H

#define SERVICE_VERSION     "3.0.45"

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

    ZigBee *m_zigbee;
    QMetaEnum m_commands;

    bool m_names, m_discovery;
    QString m_discoveryPrefix;

    void publishDiscovery(const Device &device, bool remove = false);

private slots:

    void mqttConnected(void) override;
    void mqttReceived(const QByteArray &message, const QMqttTopicName &topic) override;

    void deviceEvent(const Device &device, ZigBee::Event event);
    void endpointUpdated(const Device &device, quint8 endpointId);
    void statusUpdated(const QJsonObject &json);

};

#endif
