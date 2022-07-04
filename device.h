#ifndef DEVICE_H
#define DEVICE_H

#include <QDateTime>
#include <QSharedPointer>
#include "action.h"
#include "property.h"
#include "reporting.h"
#include "zstack.h"

class EndPointObject;
typedef QSharedPointer <EndPointObject> EndPoint;

class DeviceObject;
typedef QSharedPointer <DeviceObject> Device;

class EndPointObject
{

public:

    EndPointObject(Device device, quint16 profileId = 0, quint16 deviceId = 0) :
        m_device(device), m_profileId(profileId), m_deviceId(deviceId), m_dataUpdated(false) {}

    inline Device device(void) { return m_device; }

    inline quint16 profileId(void) { return m_profileId; }
    inline void setProfileId(quint16 value) { m_profileId = value; }

    inline quint16 deviceId(void) { return m_deviceId; }
    inline void setDeviceId(quint16 value) { m_deviceId = value; }

    inline bool dataUpdated(void) { return m_dataUpdated; }
    inline void setDataUpdated(bool value) { m_dataUpdated = value; }

    inline QList <quint16> &inClusters(void) { return m_inClusters; }
    inline QList <quint16> &outClusters(void) { return m_outClusters; }

    Cluster cluster(quint16 clusterId);

private:

    Device m_device;
    quint16 m_profileId, m_deviceId;
    bool m_dataUpdated;

    QList <quint16> m_inClusters, m_outClusters;
    QMap <quint16, Cluster> m_clusters;

};

class DeviceObject
{

public:

    DeviceObject(const QByteArray &ieeeAddress, quint16 networkAddress = 0) :
        m_ieeeAddress(ieeeAddress), m_networkAddress(networkAddress), m_manufacturerCode(0), m_logicalType(LogicalType::EndDevice), m_interviewFinished(false), m_linkQuality(0), m_lastSeen(0) {}

    inline QByteArray ieeeAddress(void) { return m_ieeeAddress; }

    inline quint16 networkAddress(void) { return m_networkAddress; }
    inline void setNetworkAddress(quint16 value) { m_networkAddress = value; }

    inline quint16 manufacturerCode(void) { return m_manufacturerCode; }
    inline void setManufacturerCode(quint16 value) { m_manufacturerCode = value; }

    inline LogicalType logicalType(void) { return m_logicalType; }
    inline void setLogicalType(LogicalType value) { m_logicalType = value; }

    inline QString name(void) { return m_name.isEmpty() ? m_ieeeAddress.toHex(':') : m_name; }
    inline void setName(const QString &value) { m_name = value; }

    inline QString vendor(void) { return m_vendor; }
    inline void setVendor(const QString &value) { m_vendor = value; }

    inline QString model(void) { return m_model; }
    inline void setModel(const QString &value) { m_model = value; }

    inline bool interviewFinished(void) { return m_interviewFinished; }
    inline void setInterviewFinished(void) { m_interviewFinished = true; }

    inline quint8 linkQuality(void) { return m_linkQuality; }
    inline void setLinkQuality(qint64 value) { m_linkQuality = value; }

    inline qint64 lastSeen(void) { return m_lastSeen; }
    inline void setLastSeen(qint64 value) { m_lastSeen = value; }
    inline void updateLastSeen(void) { m_lastSeen = QDateTime::currentSecsSinceEpoch(); }

    inline QMap <quint8, EndPoint> &endPoints(void) { return m_endPoints; }

    inline QList <Action> &actions(void) { return m_actions; }
    inline QList <Property> &properties(void) { return m_properties; }
    inline QList <Reporting> &reportings(void) { return m_reportings; }

    void setProperties(void);

private:

    QByteArray m_ieeeAddress;
    quint16 m_networkAddress;

    quint16 m_manufacturerCode;
    LogicalType m_logicalType;
    QString m_name, m_vendor, m_model;

    bool m_interviewFinished;
    quint8 m_linkQuality;
    qint64 m_lastSeen;

    QMap <quint8, EndPoint> m_endPoints;

    QList <Action> m_actions;
    QList <Property> m_properties;
    QList <Reporting> m_reportings;

};

#endif
