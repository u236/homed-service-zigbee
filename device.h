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

    EndPointObject(quint8 id, Device device, quint16 profileId = 0, quint16 deviceId = 0) :
        m_id(id), m_device(device), m_profileId(profileId), m_deviceId(deviceId), m_dataUpdated(false) {}

    inline quint8 id(void) {return m_id; }
    inline Device device(void) { return m_device; }

    inline quint16 profileId(void) { return m_profileId; }
    inline void setProfileId(quint16 value) { m_profileId = value; }

    inline quint16 deviceId(void) { return m_deviceId; }
    inline void setDeviceId(quint16 value) { m_deviceId = value; }

    inline bool dataUpdated(void) { return m_dataUpdated; }
    inline void setDataUpdated(bool value = true) { m_dataUpdated = value; }

    inline QList <quint16> &inClusters(void) { return m_inClusters; }
    inline QList <quint16> &outClusters(void) { return m_outClusters; }

private:

    quint8 m_id;
    Device m_device;
    quint16 m_profileId, m_deviceId;
    bool m_dataUpdated;

    QList <quint16> m_inClusters, m_outClusters;

};

class DeviceObject
{

public:

    DeviceObject(const QByteArray &ieeeAddress, quint16 networkAddress = 0) :
        m_ieeeAddress(ieeeAddress), m_networkAddress(networkAddress), m_manufacturerCode(0), m_logicalType(LogicalType::EndDevice), m_nodeDescriptorReceived(false), m_interviewFinished(false), m_joinTime(0), m_lastSeen(0), m_linkQuality(0) {}

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

    inline bool nodeDescriptorReceived(void) { return m_nodeDescriptorReceived; }
    inline void setNodeDescriptorReceived(void) { m_nodeDescriptorReceived = true; }

    inline bool interviewFinished(void) { return m_interviewFinished; }
    inline void setInterviewFinished(bool value = true) { m_interviewFinished = value; }

    inline qint64 joinTime(void) { return m_joinTime; }
    inline void updateJoinTime(void) { m_joinTime = QDateTime::currentMSecsSinceEpoch(); }

    inline qint64 lastSeen(void) { return m_lastSeen; }
    inline void setLastSeen(qint64 value) { m_lastSeen = value; }
    inline void updateLastSeen(void) { m_lastSeen = QDateTime::currentSecsSinceEpoch(); }

    inline quint8 linkQuality(void) { return m_linkQuality; }
    inline void setLinkQuality(qint64 value) { m_linkQuality = value; }

    inline QList <Action> actions(void) { return m_actions; }
    inline QList <Property> properties(void) { return m_properties; }
    inline QList <Reporting> reportings(void) { return m_reportings; }

    inline QMap <quint8, EndPoint> &endPoints(void) { return m_endPoints; }
    inline QMap <quint16, quint8> &neighbors(void) { return m_neighbors; }

    void setProperties(void);

private:

    QByteArray m_ieeeAddress;
    quint16 m_networkAddress;

    quint16 m_manufacturerCode;
    LogicalType m_logicalType;
    QString m_name, m_vendor, m_model;
    bool m_nodeDescriptorReceived, m_interviewFinished;

    qint64 m_joinTime, m_lastSeen;
    quint8 m_linkQuality;

    QList <Action> m_actions;
    QList <Property> m_properties;
    QList <Reporting> m_reportings;

    QMap <quint8, EndPoint> m_endPoints;
    QMap <quint16, quint8> m_neighbors;

};

#endif
