#ifndef ZIGBEE_H
#define ZIGBEE_H

#define UPDATE_NEIGHBORS_INTERVAL       300000
#define STORE_STATUS_INTERVAL           60000
#define DEVICE_REJOIN_INTERVAL          10000

#define PROFILE_IPM                     0x0101 // Industrial Plant Monitoring
#define PROFILE_HA                      0x0104 // Home Automation
#define PROFILE_CBA                     0x0105 // Commercial Building Automation
#define PROFILE_TA                      0x0107 // Telecom Applications
#define PROFILE_PHHC                    0x0108 // Personal Home & Hospital Care
#define PROFILE_AMI                     0x0109 // Advanced Metering Initiative
#define PROFILE_HUE                     0xA1E0 // Philips HUE
#define PROFILE_ZLL                     0xC05E // ZigBee Light Link

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "device.h"
#include "zstack.h"

class ZigBee : public QObject
{
    Q_OBJECT

public:

    ZigBee(QSettings *config, QObject *parent);

    void init(void);
    void setPermitJoin(bool enabled);

    void configureDevice(const QByteArray &ieeeAddress);
    void setDeviceName(const QByteArray &ieeeAddress, const QString &name);
    void deviceAction(const QByteArray &ieeeAddress, const QString &actionName, const QVariant &actionData);
    void removeDevice(const QByteArray &ieeeAddress);

private:

    ZStack *m_adapter;
    QTimer *m_neighborsTimer, *m_statusTimer, *m_ledTimer;

    QMap <QByteArray, Device> m_devices;

    QString m_databaseFile, m_libraryFile;
    qint16 m_ledPin;
    quint8 m_transactionId;
    bool m_permitJoin;

    void unserializeDevices(const QJsonArray &devicesArray);
    void unserializeEndPoints(const Device &device, const QJsonArray &array);
    void unserializeNeighbors(const Device &device, const QJsonArray &array);

    QJsonArray serializeDevices(void);
    QJsonArray serializeEndPoints(const Device &device);
    QJsonArray serializeNeighbors(const Device &device);

    Device findDevice(quint16 networkAddress);

    void interviewDevice(const Device &device);
    void setupDevice(const Device &device);
    void configureReportings(const Device &device);

    void readAttributes(const Device &device, quint8 endPointId, quint16 clusterId, QList <quint16> attributes);
    void parseAttribute(const EndPoint &endPoint, quint16 clusterId, quint16 attributeId, quint8 dataType, const QByteArray &data);

    void clusterCommandReceived(const EndPoint &endPoint, quint16 clusterId, quint8 transactionId, quint8 commandId, const QByteArray &payload);
    void globalCommandReceived(const EndPoint &endPoint, quint16 clusterId, quint8 transactionId, quint8 commandId, QByteArray payload);

private slots:

    void coordinatorReady(const QByteArray &ieeeAddress);
    void endDeviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress, quint8 capabilities);
    void endDeviceLeft(const QByteArray &ieeeAddress, quint16 networkAddress);
    void nodeDescriptorReceived(quint16 networkAddress, quint16 manufacturerCode, LogicalType logicalType);
    void activeEndPointsReceived(quint16 networkAddress, const QByteArray data);
    void simpleDescriptorReceived(quint16 networkAddress, quint8 endPointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters);
    void neighborRecordReceived(quint16 networkAddress, quint16 neighborAddress, quint8 linkQuality, bool first);
    void messageReveived(quint16 networkAddress, quint8 endPointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);

    void pollAttributes(void);
    void updateNeighbors(void);
    void storeStatus(void);
    void disableLed(void);

signals:

    void endDeviceEvent(bool join = true);
    void endPointUpdated(const EndPoint &endPoint);
    void statusStored(const QJsonObject &json);

};

#endif
