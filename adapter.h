#ifndef ADAPTER_H
#define ADAPTER_H

#define ADAPTER_RESET_DELAY             100
#define SERIAL_RECEIVE_TIMEOUT          10
#define NETWORK_REQUEST_TIMEOUT         10000

#define PROFILE_IPM                     0x0101 // Industrial Plant Monitoring
#define PROFILE_HA                      0x0104 // Home Automation
#define PROFILE_CBA                     0x0105 // Commercial Building Automation
#define PROFILE_TA                      0x0107 // Telecom Applications
#define PROFILE_PHHC                    0x0108 // Personal Home & Hospital Care
#define PROFILE_AMI                     0x0109 // Advanced Metering Initiative
#define PROFILE_HUE                     0xA1E0 // Philips HUE
#define PROFILE_ZLL                     0xC05E // ZigBee Light Link

#include <QSerialPort>
#include <QSettings>
#include <QSharedPointer>
#include <QTimer>

enum class LogicalType
{
    Coordinator,
    Router,
    EndDevice
};

class EndpointDataObject;
typedef QSharedPointer <EndpointDataObject> EndpointData;

class EndpointDataObject : public QObject
{
    Q_OBJECT

public:

    EndpointDataObject(quint16 profileId = 0, quint16 deviceId = 0) :
        QObject(nullptr), m_profileId(profileId), m_deviceId(deviceId) {}

    inline quint16 profileId(void) { return m_profileId; }
    inline void setProfileId(quint16 value) { m_profileId = value; }

    inline quint16 deviceId(void) { return m_deviceId; }
    inline void setDeviceId(quint16 value) { m_deviceId = value; }

    inline QList <quint16> &inClusters(void) { return m_inClusters; }
    inline QList <quint16> &outClusters(void) { return m_outClusters; }

private:

    quint16 m_profileId, m_deviceId;
    QList <quint16> m_inClusters, m_outClusters;

};

class Adapter : public QObject
{
    Q_OBJECT

public:

    Adapter(QSettings *config, QObject *parent);
    virtual ~Adapter(void) {}

    virtual void reset(void) = 0;
    virtual void setPermitJoin(bool enabled) = 0;

    virtual bool nodeDescriptorRequest(quint16 networkAddress) = 0;
    virtual bool simpleDescriptorRequest(quint16 networkAddress, quint8 endpointId) = 0;
    virtual bool activeEndpointsRequest(quint16 networkAddress) = 0;
    virtual bool lqiRequest(quint16 networkAddress, quint8 index = 0) = 0;

    virtual bool bindRequest(quint16 networkAddress, const QByteArray &srcAddress, quint8 srcEndpointId, quint16 clusterId, const QByteArray &dstAddress, quint8 dstEndpointId, bool unbind = false) = 0;
    virtual bool dataRequest(quint16 networkAddress, quint8 endpointId, quint16 clusterId, const QByteArray &data) = 0;

    virtual bool extendedDataRequest(const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group = false) = 0;
    virtual bool extendedDataRequest(quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &data, bool group = false) = 0;

    virtual bool setInterPanEndpointId(quint8 endpointId) = 0;
    virtual bool setInterPanChannel(quint8 channel) = 0;
    virtual void resetInterPan(void) = 0;

    virtual quint8 dataRequestStatus(void) = 0;

protected:

    QSerialPort *m_port;
    QTimer *m_timer;

    qint16 m_bootPin, m_resetPin;
    QString m_reset;

    quint16 m_panId;
    quint8 m_channel;
    bool m_debug, m_write;

    QMap <quint8, EndpointData> m_endpointsData;

    bool waitForSignal(const QObject *sender, const char *signal, int tiomeout);
    bool transmitData(const QByteArray &data, quint32 timeout = 0);

private slots:

    void readyRead(void);
    virtual void receiveData(void) = 0;

signals:

    void coordinatorReady(const QByteArray &ieeeAddress);
    void deviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress);
    void deviceLeft(const QByteArray &ieeeAddress);
    void nodeDescriptorReceived(quint16 networkAddress, LogicalType logicalType, quint16 manufacturerCode);
    void activeEndpointsReceived(quint16 networkAddress, const QByteArray data);
    void simpleDescriptorReceived(quint16 networkAddress, quint8 endpointId, quint16 profileId, quint16 deviceId, const QList <quint16> &inClusters, const QList <quint16> &outClusters);
    void neighborRecordReceived(quint16 networkAddress, quint16 neighborAddress, quint8 linkQuality, bool first);
    void messageReveived(quint16 networkAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);
    void extendedMessageReveived(const QByteArray &ieeeAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &data);

};

#endif