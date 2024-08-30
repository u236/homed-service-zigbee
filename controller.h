#ifndef CONTROLLER_H
#define CONTROLLER_H

#define SERVICE_VERSION                 "3.8.3"
#define UPDATE_DEVICE_DATA_INTERVAL     5000
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
        updateDevice,
        removeDevice,
        setupDevice,
        setupReporting,
        bindDevice,
        unbindDevice,
        addGroup,
        removeGroup,
        removeAllGroups,
        otaRefresh,
        otaUpgrade,
        getProperties,
        clusterRequest,
        globalRequest,
        touchLinkScan,
        touchLinkReset
    };

    Q_ENUM(Command)

private:

    QTimer *m_deviceDataTimer, *m_propertiesTimer;
    ZigBee *m_zigbee;

    QMetaEnum m_commands;
    QString m_haPrefix, m_haStatus;
    bool m_haEnabled, m_networkStarted;

    QMap <QByteArray, qint64> m_lastSeen;

    void publishExposes(DeviceObject *device, bool remove = false);
    void serviceOnline(void);

public slots:

    void quit(void) override;

private slots:

    void mqttConnected(void) override;
    void mqttReceived(const QByteArray &message, const QMqttTopicName &topic) override;

    void updateDeviceData(void);
    void updateProperties(void);

    void networkStarted(void);
    void deviceEvent(DeviceObject *device, ZigBee::Event event, const QJsonObject &json);
    void endpointUpdated(DeviceObject *device, quint8 endpointId);
    void statusUpdated(const QJsonObject &json);

};

#endif
