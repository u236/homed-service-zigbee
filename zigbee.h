#ifndef ZIGBEE_H
#define ZIGBEE_H

#define STORE_DEVICES_INTERVAL                  60000

#define FC_CLUSTER_SPECIFIC                     0x01
#define FC_MANUFACTURER_SPECIFIC                0x04
#define FC_SERVER_TO_CLIENT                     0x08
#define FC_DISABLE_DEFAULT_RESPONSE             0x10

#define PROFILE_IPM                             0x0101 // Industrial Plant Monitoring
#define PROFILE_HA                              0x0104 // Home Automation
#define PROFILE_CBA                             0x0105 // Commercial Building Automation
#define PROFILE_TA                              0x0107 // Telecom Applications
#define PROFILE_PHHC                            0x0108 // Personal Home & Hospital Care
#define PROFILE_AMI                             0x0109 // Advanced Metering Initiative
#define PROFILE_HUE                             0xA1E0 // Philips HUE
#define PROFILE_ZLL                             0xC05E // ZigBee Light Link

#define CLUSTER_BASIC                           0x0000
#define CLUSTER_POWER_CONFIGURATION             0x0001
#define CLUSTER_TEMPERATURE_CONFIGURATION       0x0002
#define CLUSTER_IDENTIFY                        0x0003
#define CLUSTER_GROUPS                          0x0004
#define CLUSTER_SCENES                          0x0005
#define CLUSTER_ON_OFF                          0x0006
#define CLUSTER_LEVEL_CONTROL                   0x0008
#define CLUSTER_ALARMS                          0x0009
#define CLUSTER_TIME                            0x000A
#define CLUSTER_ANALOG_INPUT                    0x000C
#define CLUSTER_MULTISTATE_INPUT                0x0012
#define CLUSTER_MULTISTATE_VALUE                0x0014
#define CLUSTER_OTA_UPGRADE                     0x0019
#define CLUSTER_POLL_CONTROL                    0x0020
#define CLUSTER_COLOR_CONTROL                   0x0300
#define CLUSTER_ILLUMINANCE_MEASUREMENT         0x0400
#define CLUSTER_TEMPERATURE_MEASUREMENT         0x0402
#define CLUSTER_RELATIVE_HUMIDITY               0x0405
#define CLUSTER_OCCUPANCY_SENSING               0x0406
#define CLUSTER_IAS_ZONE                        0x0500
#define CLUSTER_IAS_ACE                         0x0501
#define CLUSTER_IAS_WD                          0x0502
#define CLUSTER_SMART_ENERGY_METERING           0x0702
#define CLUSTER_ELECTRICAL_MEASUREMENT          0x0B04
#define CLUSTER_DIAGNOSTICS                     0x0B05
#define CLUSTER_TOUCHLINK                       0x1000

#define ATTRIBUTE_BASIC_VENDOR                  0x0004
#define ATTRIBUTE_BASIC_MODEL                   0x0005

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "device.h"
#include "zstack.h"

#pragma pack(push, 1)

struct zclHeaderStruct
{
    quint8 frameControl;
    quint8 transactionId;
    quint8 commandId;
};

#pragma pack(pop)

class ZigBee : public QObject
{
    Q_OBJECT

public:

    ZigBee(QSettings *config, QObject *parent);

    void init(void);
    void setPermitJoin(bool enabled);

private:

    ZStack *m_adapter;
    QTimer *m_timer;

    QMap <QByteArray, Device> m_devices;

    QString m_databaseFile;
    quint8 m_transactionId;
    bool m_permitJoin;

    void unserializeDevices(const QJsonArray &devicesArray);
    QJsonArray serializeDevices(void);

    void unserializeEndPoints(const Device &device, const QJsonArray &endPointsArray);
    QJsonArray serializeEndPoints(const Device &device);

    Device findDevice(quint16 networkAddress);
    void interviewDevice(const Device &device);
    void readAttributes(const Device &device, quint8 endPointId, quint16 clusterId, QList <quint16> attributes);

    void clusterCommandReceived(EndPoint endPoint, quint16 clusterId, quint8 transactionId, quint8 commandId, const QByteArray &payload);
    void globalCommandReceived(EndPoint endPoint, quint16 clusterId, quint8 transactionId, quint8 commandId, const QByteArray &payload);

private slots:

    void coordinatorReady(const QByteArray &ieeeAddress);
    void endDeviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress, quint8 capabilities);
    void endDeviceLeft(const QByteArray &ieeeAddress, quint16 networkAddress);
    void nodeDescriptorReceived(quint16 networkAddress, quint16 manufacturerCode, LogicalType logicalType);
    void activeEndPointsReceived(quint16 networkAddress, const QByteArray data);
    void simpleDescriptorReceived(quint16 networkAddress, quint8 endPointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters);
    void messageReveived(quint16 networkAddress, quint8 endPointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);

    void storeState(void);

signals:

    void stateUpdated(const QJsonObject json);

};

#endif
