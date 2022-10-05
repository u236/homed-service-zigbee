#ifndef CONTROLLER_H
#define CONTROLLER_H

#define SERVICE_VERSION     "3.0.32"

#include <QMetaEnum>
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
        removeDevice,
        updateDevice,
        updateReporting,
        bindDevice,
        unbindDevice,
        addGroup,
        removeGroup,
        removeAllGroups,
        otaUpgrade,
        touchLinkReset,
        touchLinkScan
    };

    Q_ENUM(Command)

private:

    ZigBee *m_zigbee;
    QMetaEnum m_commands;

private slots:

    void mqttConnected(void) override;
    void mqttReceived(const QByteArray &message, const QMqttTopicName &topic) override;

    void deviceEvent(bool join = true);
    void endpointUpdated(const Device &device, quint8 endpointId);
    void statusStored(const QJsonObject &json);

};

#endif
