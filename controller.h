#ifndef CONTROLLER_H
#define CONTROLLER_H

#define SERVICE_VERSION                 "3.6.2"
#define UPDATE_AVAILABILITY_INTERVAL    10000
#define UPDATE_PROPERTIES_DELAY         1000

#include "homed.h"
#include "zigbee.h"

class Controller : public HOMEd
{
    Q_OBJECT

public:

    Controller(const QString &configFile);

    enum class Command
    {
        restartService,
        setPermitJoin,
        togglePermitJoin,
        editDevice,
        removeDevice,
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

    QTimer *m_avaliabilityTimer, *m_propertiesTimer;
    ZigBee *m_zigbee;

    QMetaEnum m_commands;
    QString m_haStatus;

    void publishExposes(DeviceObject *device, bool remove = false);

public slots:

    void quit(void) override;

private slots:

    void mqttConnected(void) override;
    void mqttReceived(const QByteArray &message, const QMqttTopicName &topic) override;

    void updateAvailability(void);
    void updateProperties(void);

    void deviceEvent(DeviceObject *device, ZigBee::Event event, const QJsonObject &json);
    void endpointUpdated(DeviceObject *device, quint8 endpointId);
    void statusUpdated(const QJsonObject &json);

};

#endif
