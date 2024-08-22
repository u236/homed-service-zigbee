#ifndef DEVICE_H
#define DEVICE_H

#define STORE_DATABASE_DELAY        20
#define STORE_PROPERTIES_DELAY      1000

#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "action.h"
#include "adapter.h"
#include "binding.h"
#include "expose.h"
#include "poll.h"
#include "property.h"
#include "reporting.h"


enum class InterviewStatus
{
    NodeDescriptor,
    ActiveEndpoints,
    SimpleDescriptors,
    BasicAttributes,
    ApplicationVersion,
    ManufacturerName,
    ModelName,
    PowerSource,
    FirmwareVersion,
    ColorCapabilities,
    ZoneEnroll,
    Finished
};

enum class DescriptorStatus
{
    Unknown,
    Pending,
    Received
};

enum class ZoneStatus
{
    Unknown,
    SetAddress,
    Enroll,
    Enrolled
};

class OTA
{

public:

    OTA(void) : m_manufacturerCode(0), m_imageType(0), m_currentVersion(0), m_available(false), m_upgrade(false), m_running(false), m_progress(0) {}

    inline quint16 manufacturerCode(void) { return m_manufacturerCode; }
    inline void setManufacturerCode(quint16 value) { m_manufacturerCode = value; }

    inline quint16 imageType(void) { return m_imageType; }
    inline void setImageType(quint16 value) { m_imageType = value; }

    inline quint32 currentVersion(void) { return m_currentVersion; }
    inline void setCurrentVersion(quint32 value) { m_currentVersion = value; }

    inline QString fileName(void) { return m_fileName; }
    inline void clearFileName(void) { m_fileName.clear(); }

    inline quint32 fileVersion(void) { return m_fileVersion; }
    inline quint32 imageOffset(void) { return m_imageOffset; }
    inline quint32 imageSize(void) { return m_imageSize; }

    inline bool available(void) { return m_available; }
    inline void setAvailable(void) { m_available = true; }

    inline bool upgrade(void) { return m_upgrade; }
    inline void setUpgrade(bool value) { m_upgrade = value; }

    inline bool running(void) { return m_running; }
    inline void setRunning(bool value) { m_running = value; }

    inline double progress(void) { return m_progress; }
    inline void setProgress(double value) { m_progress = value; }

    inline void reset(void) { m_upgrade = false; m_running = false; m_progress = 0; }
    void refresh(const QDir &dir);

private:

    quint16 m_manufacturerCode, m_imageType;
    quint32 m_currentVersion;

    QString m_fileName;
    quint32 m_fileVersion, m_imageOffset, m_imageSize;

    bool m_available, m_upgrade, m_running;
    double m_progress;

};

class EndpointObject : public AbstractEndpointObject, public EndpointDataObject
{

public:

    EndpointObject(quint8 id, Device device, quint16 profileId = 0, quint16 deviceId = 0) :
        AbstractEndpointObject(id, device), EndpointDataObject(profileId, deviceId), m_timer(new QTimer(this)), m_pollInterval(0), m_pollTime(0), m_colorCapabilities(0xFFFF), m_zoneType(0), m_descriptorStatus(DescriptorStatus::Unknown), m_zoneStatus(ZoneStatus::Unknown), m_updated(false) {}

    inline QTimer *timer(void) { return m_timer; }

    inline quint32 pollInterval(void) { return m_pollInterval; }
    inline void setPollInterval(quint32 value) { m_pollInterval = value; }

    inline qint64 pollTime(void) { return m_pollTime; }
    inline void setPollTime(qint64 value) { m_pollTime = value; }

    inline quint16 colorCapabilities(void) { return m_colorCapabilities; }
    inline void setColorCapabilities(quint16 value) { m_colorCapabilities = value; }

    inline quint16 zoneType(void) { return m_zoneType; }
    inline void setZoneType(quint16 value) { m_zoneType = value; }

    inline DescriptorStatus descriptorStatus(void) { return m_descriptorStatus; }
    inline void setDescriptorStatus(DescriptorStatus value) { m_descriptorStatus = value; }

    inline ZoneStatus zoneStatus(void) { return m_zoneStatus; }
    inline void setZoneStatus(ZoneStatus value) { m_zoneStatus = value; }

    inline bool updated(void) { return m_updated; }
    inline void setUpdated(bool value) { m_updated = value; }

    inline QList <Property> &properties(void) { return m_properties; }
    inline QList <Action> &actions(void) { return m_actions; }
    inline QList <Binding> &bindings(void) { return m_bindings; }
    inline QList <Reporting> &reportings(void) { return m_reportings; }
    inline QList <Poll> &polls(void) { return m_polls; }
    inline QList <quint16> &groups(void) { return m_groups; }

private:

    QTimer *m_timer;

    quint32 m_pollInterval;
    qint64 m_pollTime;

    quint16 m_colorCapabilities, m_zoneType;

    DescriptorStatus m_descriptorStatus;
    ZoneStatus m_zoneStatus;

    bool m_updated;

    QList <Property> m_properties;
    QList <Action> m_actions;
    QList <Binding> m_bindings;
    QList <Reporting> m_reportings;
    QList <Poll> m_polls;
    QList <quint16> m_groups;

};

class DeviceObject : public AbstractDeviceObject
{

public:

    DeviceObject(const QByteArray &ieeeAddress, quint16 networkAddress, const QString name = QString(), bool removed = false) :
        AbstractDeviceObject(name.isEmpty() ? ieeeAddress.toHex(':') : name), m_timer(new QTimer(this)), m_ieeeAddress(ieeeAddress), m_networkAddress(networkAddress), m_removed(removed), m_supported(false), m_interviewStatus(InterviewStatus::NodeDescriptor), m_logicalType(LogicalType::EndDevice), m_manufacturerCode(0), m_powerSource(POWER_SOURCE_UNKNOWN), m_joinTime(0), m_lastSeen(0), m_linkQuality(0), m_lqiRequestPending(false) {}

    inline QTimer *timer(void) { return m_timer; }
    inline QByteArray ieeeAddress(void) { return m_ieeeAddress; }

    inline quint16 networkAddress(void) { return m_networkAddress; }
    inline void setNetworkAddress(quint16 value) { m_networkAddress = value; }

    inline bool removed(void) { return m_removed; }
    inline void setRemoved(bool value) { m_removed = value; }

    inline bool supported(void) { return m_supported; }
    inline void setSupported(bool value) { m_supported = value; }

    inline InterviewStatus interviewStatus(void) { return m_interviewStatus; }
    inline void setInterviewStatus(InterviewStatus value) { m_interviewStatus = value; }

    inline quint8 interviewEndpointId(void) { return m_interviewEndpointId; }
    inline void setInterviewEndpointId(quint8 value) { m_interviewEndpointId = value; }

    inline LogicalType logicalType(void) { return m_logicalType; }
    inline void setLogicalType(LogicalType value) { m_logicalType = value; }

    inline quint16 manufacturerCode(void) { return m_manufacturerCode; }
    inline void setManufacturerCode(quint16 value) { m_manufacturerCode = value; }

    inline bool batteryPowered(void) { return m_powerSource != POWER_SOURCE_MAINS && m_powerSource != POWER_SOURCE_DC; }
    inline quint8 powerSource(void) { return m_powerSource; }
    inline void setPowerSource(quint8 value) { m_powerSource = value; }

    inline QString firmware(void) { return m_firmware; }
    inline void setFirmware(const QString &value) { m_firmware = value; }

    inline qint64 joinTime(void) { return m_joinTime; }
    inline void updateJoinTime(void) { m_joinTime = QDateTime::currentMSecsSinceEpoch(); }

    inline qint64 lastSeen(void) { return m_lastSeen; }
    inline void setLastSeen(qint64 value) { m_lastSeen = value; }
    inline void updateLastSeen(void) { m_lastSeen = QDateTime::currentSecsSinceEpoch(); }

    inline quint8 linkQuality(void) { return m_linkQuality; }
    inline void setLinkQuality(quint8 value) { m_linkQuality = value; }

    inline quint8 lqiRequestIndex(void) { return m_lqiRequestIndex; }
    inline void setLqiRequestIndex(quint8 value) { m_lqiRequestIndex = value; }

    inline bool lqiRequestPending(void) { return m_lqiRequestPending; }
    inline void setLqiRequestPending(bool value) { m_lqiRequestPending = value; }

    inline OTA &ota(void) { return m_otaData; }
    inline QMap <quint16, quint8> &neighbors(void) { return m_neighbors; }

private:

    QTimer *m_timer;

    QByteArray m_ieeeAddress;
    quint16 m_networkAddress;
    bool m_removed, m_supported;

    InterviewStatus m_interviewStatus;
    quint8 m_interviewEndpointId;

    LogicalType m_logicalType;
    quint16 m_manufacturerCode;
    quint8 m_powerSource;
    QString m_firmware;

    qint64 m_joinTime, m_lastSeen;
    quint8 m_linkQuality;

    quint8 m_lqiRequestIndex;
    bool m_lqiRequestPending;

    OTA m_otaData;
    QMap <quint16, quint8> m_neighbors;

};

class DeviceList : public QObject, public QMap <QByteArray, Device>
{
    Q_OBJECT

public:

    DeviceList(QSettings *config, QObject *parent);
    ~DeviceList(void);

    inline QDir otaDir(void) { return m_otaDir; }

    inline bool names(void) { return m_names; }
    inline void setNames(bool value) { m_names = value; }

    inline bool permitJoin(void) { return m_permitJoin; }
    inline void setPermitJoin(bool value) { m_permitJoin = value; }

    void init(void);
    void storeDatabase(void);
    void storeProperties(void);

    Device byName(const QString &name);
    Device byNetwork(quint16 networkAddress);
    Endpoint endpoint(const Device &device, quint8 endpointId);

    void identityHandler(const Device &device, QString &manufacturerName, QString &modelName);
    void setupDevice(const Device &device);
    void setupEndpoint(const Endpoint &endpoint, const QJsonObject &json, bool multiple = false);

    void recognizeDevice(const Device &device);
    void recognizeMultipleProperty(const Device &device, const Endpoint &endpoint, const Property &property);
    void recognizeMultipleExpose(const Device &device, const Endpoint &endpoint, const Expose &expose);

    void removeDevice(const Device &device);

private:

    QSettings *m_config;
    QTimer *m_databaseTimer, *m_propertiesTimer;

    QFile m_databaseFile, m_propertiesFile, m_optionsFile;
    QDir m_otaDir, m_externalDir, m_libraryDir;
    bool m_names, m_permitJoin;

    QMap <QString, QVariant> m_exposeOptions;
    QList <QString> m_specialExposes, m_brokenFiles;

    void unserializeDevices(const QJsonArray &devices);
    void unserializeProperties(const QJsonObject &properties);

    QJsonArray serializeDevices(void);
    QJsonObject serializeProperties(void);

    bool writeFile(QFile &file, const QByteArray &data, bool sync = false);

private slots:

    void writeDatabase(void);
    void writeProperties(void);
    void endpointTimeout(void);

signals:

    void statusUpdated(const QJsonObject &json);
    void endpointUpdated(DeviceObject *device, quint8 endpointId);
    void pollRequest(EndpointObject *endpoint, const Poll &poll);

};

inline QDebug operator << (QDebug debug, const Endpoint &endpoint) { return debug << "endpoint" << QString::asprintf("0x%02x", endpoint->id()); }
inline QDebug operator << (QDebug debug, const Device &device) { return debug << "device" << device->name(); }

#endif
