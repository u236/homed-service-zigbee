#ifndef DEVICE_H
#define DEVICE_H

#define STORE_STATUS_INTERVAL           60000

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "action.h"
#include "adapter.h"
#include "poll.h"
#include "property.h"
#include "reporting.h"

class EndpointObject;
typedef QSharedPointer <EndpointObject> Endpoint;

class DeviceObject;
typedef QSharedPointer <DeviceObject> Device;

enum class ZoneStatus
{
    Unknown,
    SetAddress,
    Enroll,
    Enrolled
};

class EndpointObject : public EndpointDataObject
{

public:

    EndpointObject(quint8 id, Device device, quint16 profileId = 0, quint16 deviceId = 0) :
         EndpointDataObject(profileId, deviceId), m_timer(new QTimer(this)), m_id(id), m_device(device), m_zoneStatus(ZoneStatus::Unknown), m_dataUpdated(false) {}

    inline QTimer *timer(void) { return m_timer; }

    inline quint8 id(void) {return m_id; }
    inline Device device(void) { return m_device; }

    inline ZoneStatus zoneStatus(void) { return m_zoneStatus; }
    inline void setZoneStatus(ZoneStatus value) { m_zoneStatus = value; }

    inline bool dataUpdated(void) { return m_dataUpdated; }
    inline void setDataUpdated(bool value) { m_dataUpdated = value; }

    inline QList <Action> &actions(void) { return m_actions; }
    inline QList <Property> &properties(void) { return m_properties; }
    inline QList <Reporting> &reportings(void) { return m_reportings; }
    inline QList <Poll> &polls(void) { return m_polls; }

private:

    QTimer *m_timer;

    quint8 m_id;
    Device m_device;

    ZoneStatus m_zoneStatus;
    bool m_dataUpdated;

    QList <Action> m_actions;
    QList <Property> m_properties;
    QList <Reporting> m_reportings;
    QList <Poll> m_polls;

};

class DeviceObject : public QObject
{
    Q_OBJECT

public:

    DeviceObject(const QByteArray &ieeeAddress, quint16 networkAddress = 0) :
        QObject(nullptr), m_timer(new QTimer(this)), m_ieeeAddress(ieeeAddress), m_networkAddress(networkAddress), m_logicalType(LogicalType::EndDevice), m_version(0), m_manufacturerCode(0), m_nodeDescriptorReceived(false), m_activeEndpointsReceived(false), m_interviewFinished(false), m_multipleEndpoints(false), m_lastSeen(0), m_linkQuality(0) {}

    inline QTimer *timer(void) { return m_timer; }
    inline QByteArray ieeeAddress(void) { return m_ieeeAddress; }

    inline quint16 networkAddress(void) { return m_networkAddress; }
    inline void setNetworkAddress(quint16 value) { m_networkAddress = value; }

    inline LogicalType logicalType(void) { return m_logicalType; }
    inline void setLogicalType(LogicalType value) { m_logicalType = value; }

    inline quint8 version(void) { return m_version; }
    inline void setVersion(quint8 value) { m_version = value; }

    inline quint16 manufacturerCode(void) { return m_manufacturerCode; }
    inline void setManufacturerCode(quint16 value) { m_manufacturerCode = value; }

    inline QString manufacturerName(void) { return m_manufacturerName; }
    inline void setManufacturerName(const QString &value) { m_manufacturerName = value; }

    inline QString modelName(void) { return m_modelName; }
    inline void setModelName(const QString &value) { m_modelName = value; }

    inline QString name(void) { return m_name.isEmpty() ? m_ieeeAddress.toHex(':') : m_name; }
    inline void setName(const QString &value) { m_name = value; }

    inline QString description(void) { return m_description; }
    inline void setDescription(const QString &value) { m_description = value; }

    inline bool nodeDescriptorReceived(void) { return m_nodeDescriptorReceived; }
    inline void setNodeDescriptorReceived(void) { m_nodeDescriptorReceived = true; }

    inline bool activeEndpointsReceived(void) { return m_activeEndpointsReceived; }
    inline void setActiveEndpointsReceived(void) { m_activeEndpointsReceived = true; }

    inline bool interviewFinished(void) { return m_interviewFinished; }
    inline void setInterviewFinished(void) { m_interviewFinished = true; }

    inline bool multipleEndpoints(void) { return m_multipleEndpoints; }
    inline void setMultipleEndpoints(bool value) { m_multipleEndpoints = value; }

    inline qint64 lastSeen(void) { return m_lastSeen; }
    inline void setLastSeen(qint64 value) { m_lastSeen = value; }
    inline void updateLastSeen(void) { m_lastSeen = QDateTime::currentSecsSinceEpoch(); }

    inline quint8 linkQuality(void) { return m_linkQuality; }
    inline void setLinkQuality(qint64 value) { m_linkQuality = value; }

    inline QMap <quint8, Endpoint> &endpoints(void) { return m_endpoints; }
    inline QMap <quint16, quint8> &neighbors(void) { return m_neighbors; }

private:

    QTimer *m_timer;

    QByteArray m_ieeeAddress;
    quint16 m_networkAddress;

    LogicalType m_logicalType;
    quint8 m_version;
    quint16 m_manufacturerCode;
    QString m_manufacturerName, m_modelName, m_description, m_name;

    bool m_nodeDescriptorReceived, m_activeEndpointsReceived, m_interviewFinished, m_multipleEndpoints;

    qint64 m_lastSeen;
    quint8 m_linkQuality;

    QMap <quint8, Endpoint> m_endpoints;
    QMap <quint16, quint8> m_neighbors;

};

class DeviceList : public QObject, public QMap <QByteArray, Device>
{
    Q_OBJECT

public:

    DeviceList(QSettings *config);

    inline bool permitJoin(void) { return m_permitJoin; }
    inline void setPermitJoin(bool value) { m_permitJoin = value; }

    inline void setAdapterType(const QString &value) { m_adapterType = value; }
    inline void setAdapterVersion(const QString &value) { m_adapterVersion = value; }

    Device byNetwork(quint16 networkAddress);

private:

    QTimer *m_timer;

    QFile m_file;
    bool m_permitJoin;

    QString m_adapterType, m_adapterVersion;

    void unserializeDevices(const QJsonArray &devices);
    void unserializeEndpoints(const Device &device, const QJsonArray &endpoints);
    void unserializeNeighbors(const Device &device, const QJsonArray &neighbors);

    QJsonArray serializeDevices(void);
    QJsonArray serializeEndpoints(const Device &device);
    QJsonArray serializeNeighbors(const Device &device);

public slots:

    void storeStatus(void);

signals:

    void statusStored(const QJsonObject &json);

};

#endif
