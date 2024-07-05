#include <QtEndian>
#include <QEventLoop>
#include <QRandomGenerator>
#include "ezsp.h"
#include "gpio.h"
#include "logger.h"
#include "zboss.h"
#include "zcl.h"
#include "zigate.h"
#include "zigbee.h"
#include "zstack.h"

ZigBee::ZigBee(QSettings *config, QObject *parent) : QObject(parent), m_config(config), m_requestTimer(new QTimer(this)), m_neignborsTimer(new QTimer(this)), m_pingTimer(new QTimer(this)), m_statusLedTimer(new QTimer(this)), m_adapter(nullptr), m_devices(new DeviceList(m_config, this)), m_events(QMetaEnum::fromType <Event> ()), m_requestId(0), m_interPanLock(false)
{
    m_statusLedPin = m_config->value("gpio/status", "-1").toString();
    m_blinkLedPin = m_config->value("gpio/blink", "-1").toString();
    m_debounce = m_config->value("mqtt/debounce", true).toBool();
    m_discovery = m_config->value("default/discovery", true).toBool();
    m_cloud = m_config->value("default/cloud", true).toBool();
    m_debug = m_config->value("debug/zigbee", false).toBool();

    connect(m_devices, &DeviceList::statusUpdated, this, &ZigBee::statusUpdated);
    connect(m_devices, &DeviceList::endpointUpdated, this, &ZigBee::endpointUpdated);
    connect(m_devices, &DeviceList::pollRequest, this, &ZigBee::pollRequest);
    connect(m_statusLedTimer, &QTimer::timeout, this, &ZigBee::updateStatusLed);

    GPIO::direction(m_statusLedPin, GPIO::Output);
    GPIO::setStatus(m_statusLedPin, m_statusLedPin != m_blinkLedPin);

    if (m_statusLedPin == m_blinkLedPin)
        return;

    GPIO::direction(m_blinkLedPin, GPIO::Output);
    GPIO::setStatus(m_blinkLedPin, false);
}

ZigBee::~ZigBee(void)
{
    if (m_adapter)
    {
        disconnect(m_adapter, &Adapter::permitJoinUpdated, this, &ZigBee::permitJoinUpdated);
        m_adapter->setPermitJoin(false);
    }

    GPIO::setStatus(m_statusLedPin, false);
    GPIO::setStatus(m_blinkLedPin, false);
}

void ZigBee::init(void)
{
    QList <QString> list = {"ezsp", "zboss", "zigate", "znp"};
    QString adapterType = m_config->value("zigbee/adapter", "znp").toString();

    switch (list.indexOf(adapterType))
    {
        case 0:  m_adapter = new EZSP(m_config, this); break;
        case 1:  m_adapter = new ZBoss(m_config, this); break;
        case 2:  m_adapter = new ZiGate(m_config, this); break;
        case 3:  m_adapter = new ZStack(m_config, this); break;
        default: logWarning << "Unrecognized adapter type" << adapterType; return;
    }

    connect(m_adapter, &Adapter::adapterReset, this, &ZigBee::adapterReset);
    connect(m_adapter, &Adapter::coordinatorReady, this, &ZigBee::coordinatorReady);
    connect(m_adapter, &Adapter::permitJoinUpdated, this, &ZigBee::permitJoinUpdated);
    connect(m_adapter, &Adapter::requestFinished, this, &ZigBee::requestFinished);

    m_devices->init();
    m_adapter->init();
}

void ZigBee::setPermitJoin(bool enabled)
{
    if (!m_adapter)
        return;

    m_adapter->setPermitJoin(enabled);
}

void ZigBee::togglePermitJoin(void)
{
    if (!m_adapter)
        return;

    m_adapter->togglePermitJoin();
}

void ZigBee::updateDevice(const QString &deviceName, const QString &name, const QString &note, bool active, bool discovery, bool cloud)
{
    Device device = m_devices->byName(deviceName), other = m_devices->byName(name);
    bool check = false;

    if (device.isNull() || device->removed() || device->logicalType() == LogicalType::Coordinator)
        return;

    if (device != other && !other.isNull() && !other->removed())
    {
        logWarning << "Device" << device->name() << "rename failed, name already in use";
        emit deviceEvent(device.data(), Event::deviceNameDuplicate);
        return;
    }
    else if (device->name() != name)
    {
        emit deviceEvent(device.data(), Event::deviceAboutToRename);

        if (!other.isNull() && other->removed())
            m_devices->remove(other->ieeeAddress());

        device->setName(name.isEmpty() ? device->ieeeAddress().toHex(':') : name.trimmed());
        check = true;
    }

    if (device->note() != note)
    {
        device->setNote(note);
        check = true;
    }

    if (device->active() != active)
    {
        device->setAvailability(active ? Availability::Unknown : Availability::Inactive);
        device->setActive(active);
        check = true;
    }

    if (device->discovery() != discovery || device->cloud() != cloud)
    {
        device->setDiscovery(discovery);
        device->setCloud(cloud);
        check = true;
    }

    if (check)
    {
        emit deviceEvent(device.data(), Event::deviceUpdated);
        m_devices->storeDatabase();
    }
}

void ZigBee::removeDevice(const QString &deviceName, bool force)
{
    Device device = m_devices->byName(deviceName);

    if (device.isNull() || device->removed() || device->logicalType() == LogicalType::Coordinator)
        return;

    if (!force)
    {
        enqueueRequest(device, RequestType::Leave);
        return;
    }

    logInfo << "Device" << device->name() << "removed (force)";
    emit deviceEvent(device.data(), Event::deviceRemoved);

    m_devices->removeDevice(device);
    m_devices->storeDatabase();
}

void ZigBee::setupDevice(const QString &deviceName, bool reportings)
{
    Device device = m_devices->byName(deviceName);

    if (device.isNull() || device->removed() || !device->active() || device->logicalType() == LogicalType::Coordinator)
        return;

    m_devices->setupDevice(device);
    emit deviceEvent(device.data(), Event::deviceUpdated);

    if (!reportings)
    {
        logInfo << "Device" << device->name() << "configuration updated without reportings";
        return;
    }

    if (!configureDevice(device))
    {
        logWarning << "Device" << device->name() << "configuration failed";
        return;
    }

    logInfo << "Device" << device->name() << "configuration updated";
}

void ZigBee::setupReporting(const QString &deviceName, quint8 endpointId, const QString &reportingName, quint16 minInterval, quint16 maxInterval, quint16 valueChange)
{
    Device device = m_devices->byName(deviceName);

    if (device.isNull() || device->removed() || !device->active() || device->logicalType() == LogicalType::Coordinator)
        return;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (endpointId && it.key() != endpointId)
            continue;

        for (int i = 0; i < it.value()->bindings().count(); i++)
            if (!bindRequest(device, it.value()->id(), it.value()->bindings().at(i)->clusterId()))
                return;

        for (int i = 0; i < it.value()->reportings().count(); i++)
        {
            const Reporting &reporting = it.value()->reportings().at(i);

            if (!reportingName.isEmpty() && reporting->name() != reportingName)
                continue;

            if (minInterval)
                reporting->setMinInterval(minInterval);

            if (maxInterval)
                reporting->setMaxInterval(maxInterval);

            if (valueChange)
                reporting->setValueChange(valueChange);

            configureReporting(device, it.value()->id(), reporting);
        }
    }
}

void ZigBee::bindingControl(const QString &deviceName, quint8 endpointId, quint16 clusterId, const QVariant &dstName, quint8 dstEndpointId, bool unbind)
{
    Device device = m_devices->byName(deviceName);

    if (device.isNull() || device->removed() || !device->active() || device->logicalType() == LogicalType::Coordinator)
        return;

    switch (dstName.type())
    {
        case QVariant::LongLong:
        {
            quint16 value = qToLittleEndian <quint16> (dstName.toInt());

            if (value)
                bindRequest(device, endpointId, clusterId, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)), 0xFF, unbind);

            break;
        }

        case QVariant::String:
        {
            Device destination = m_devices->byName(dstName.toString());

            if (!destination.isNull() && !destination->removed() && destination->active())
                bindRequest(device, endpointId, clusterId, destination->ieeeAddress(), dstEndpointId, unbind);

            break;
        }

        default:
            break;
    }
}

void ZigBee::groupControl(const QString &deviceName, quint8 endpointId, quint16 groupId, bool remove)
{
    Device device = m_devices->byName(deviceName);

    if (device.isNull() || device->removed() || !device->active() || device->logicalType() == LogicalType::Coordinator)
        return;

    groupId = qFromLittleEndian(groupId);
    enqueueRequest(device, endpointId ? endpointId : 0x01, CLUSTER_GROUPS, zclHeader(FC_CLUSTER_SPECIFIC, m_requestId, remove ? 0x03 : 0x00).append(reinterpret_cast <char*> (&groupId), sizeof(groupId)).append(remove ? 0 : 1, 0x00), QString("%1 group request").arg(remove ? "remove" : "add"));
}

void ZigBee::removeAllGroups(const QString &deviceName, quint8 endpointId)
{
    Device device = m_devices->byName(deviceName);

    if (device.isNull() || device->removed() || !device->active() || device->logicalType() == LogicalType::Coordinator)
        return;

    enqueueRequest(device, endpointId ? endpointId : 0x01, CLUSTER_GROUPS, zclHeader(FC_CLUSTER_SPECIFIC, m_requestId, 0x04), QString("remove all groups request"));
}

void ZigBee::otaUpgrade(const QString &deviceName, quint8 endpointId, const QString &fileName, bool force)
{
    Device device = m_devices->byName(deviceName);
    otaImageNotifyStruct payload;

    if (device.isNull() || device->removed() || !device->active() || device->logicalType() == LogicalType::Coordinator || fileName.isEmpty() || !QFile::exists(fileName))
        return;

    m_otaDevice = device;
    m_otaFile.setFileName(fileName);
    m_otaForce = force;

    payload.type = 0x00;
    payload.jitter = 0x64; // TODO: check this

    logInfo << "Device" << device->name() << "OTA upgrade notification enqueued";
    enqueueRequest(device, endpointId ? endpointId : 0x01, CLUSTER_OTA_UPGRADE, zclHeader(FC_CLUSTER_SPECIFIC | FC_SERVER_TO_CLIENT, m_requestId, 0x00).append(reinterpret_cast <char*> (&payload), sizeof(payload)));
}

void ZigBee::getProperties(const QString &deviceName)
{
    Device device = m_devices->byName(deviceName);

    if (device.isNull() || device->removed() || !device->active() || device->logicalType() == LogicalType::Coordinator)
        return;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (it.value()->properties().isEmpty() && !it.value()->inClusters().contains(CLUSTER_BASIC))
            continue;

        emit endpointUpdated(device.data(), it.key());
    }
}

void ZigBee::clusterRequest(const QString &deviceName, quint8 endpointId, quint16 clusterId, quint16 manufacturerCode, quint8 commandId, const QByteArray &payload, bool global)
{
    Device device = m_devices->byName(deviceName);
    QByteArray request;

    if (device.isNull() || device->removed() || !device->active() || device->logicalType() == LogicalType::Coordinator)
        return;

    request = zclHeader(global ? 0x00 : FC_CLUSTER_SPECIFIC, m_requestId, commandId, manufacturerCode).append(payload);
    logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpointId ? endpointId : 0x01) << "cluster" << QString::asprintf("0x%04x", clusterId) << "request" << m_requestId << "enqueued with data" << request.toHex(':');

    enqueueRequest(device, endpointId ? endpointId : 0x01, clusterId, request, QString("request %1").arg(m_requestId), true);
}

void ZigBee::touchLinkRequest(const QByteArray &ieeeAddress, quint8 channel, bool reset)
{
    if (m_interPanLock)
        return;

    m_interPanLock = true;

    if (reset)
        touchLinkReset(ieeeAddress, channel);
    else
        touchLinkScan();

    m_adapter->resetInterPanChannel();

    if (!m_requests.isEmpty())
        m_requestTimer->start();

    m_interPanLock = false;
}

void ZigBee::deviceAction(const QString &deviceName, quint8 endpointId, const QString &name, const QVariant &data)
{
    Device device = m_devices->byName(deviceName);

    if (device.isNull() || device->removed() || !device->active() || device->logicalType() == LogicalType::Coordinator)
        return;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (endpointId && it.key() != endpointId)
            continue;

        for (int i = 0; i < it.value()->actions().count(); i++)
        {
            const Action &action = it.value()->actions().at(i);

            if (action->name() == name || action->name() == "tuyaDataPoints" || action->actions().contains(name))
            {
                QByteArray request = action->request(name, data);

                if (request.isEmpty())
                    continue;

                if (data.type() != QVariant::String || !data.toString().isEmpty())
                    enqueueRequest(device, it.key(), action->clusterId(), request, QString("%1 action request").arg(name), false, action->manufacturerCode(), action);

                break;
            }
        }
    }
}

void ZigBee::groupAction(quint16 groupId, const QString &name, const QVariant &data)
{
    int type = QMetaType::type(QString(name).append("Action").toUtf8());

    if (type)
    {
        Action action(reinterpret_cast <ActionObject*> (QMetaType::create(type)));
        QByteArray request = action->request(name, data);

        if (request.isEmpty() || (data.type() == QVariant::String && data.toString().isEmpty()))
            return;

        if (!m_adapter->multicastRequest(m_requestId, groupId, 0x01, 0xFF, action->clusterId(), request))
        {
            logWarning << "Group" << groupId << action->name().toUtf8().constData() << "action request aborted";
            return;
        }

        logInfo << "Group" << groupId << action->name().toUtf8().constData() << "action request sent";
    }
}

void ZigBee::enqueueRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &data, const QString &name, bool debug, quint16 manufacturerCode, const Action &action)
{
    DataRequest request(new DataRequestObject(device, endpointId, clusterId, data, name, debug, manufacturerCode, action));

    if (!m_requestTimer->isActive() && !m_interPanLock)
        m_requestTimer->start();

    m_requests.insert(m_requestId++, Request(new RequestObject(QVariant::fromValue(request), RequestType::Data)));
}

void ZigBee::enqueueRequest(const Device &device, RequestType type)
{
    if (!m_requestTimer->isActive() && !m_interPanLock)
        m_requestTimer->start();

    m_requests.insert(m_requestId++, Request(new RequestObject(QVariant::fromValue(device), type)));
}

bool ZigBee::interviewRequest(quint8 id, const Device &device)
{
    m_adapter->setRequestParameters(device->ieeeAddress(), device->batteryPowered());

    if (device->manufacturerName().isEmpty() || device->modelName().isEmpty())
    {
        if (!device->descriptorReceived())
        {
            if (m_adapter->zdoRequest(id, device->networkAddress(), ZDO_NODE_DESCRIPTOR_REQUEST))
                return true;

            interviewError(device, "node descriptor request failed");
            return false;
        }

        if (!device->endpointsReceived())
        {
            if (m_adapter->zdoRequest(id, device->networkAddress(), ZDO_ACTIVE_ENDPOINTS_REQUEST))
                return true;

            interviewError(device, "active endpoints request failed");
            return false;
        }

        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        {
            if (!it.value()->descriptorReceived())
            {
                device->setInterviewEndpointId(it.key());

                if (m_adapter->zdoRequest(id, device->networkAddress(), ZDO_SIMPLE_DESCRIPTOR_REQUEST, QByteArray(1, static_cast <char> (it.key()))))
                    return true;

                interviewError(device, QString::asprintf("endpoint 0x%02x simple descriptor request failed", it.key()));
                return false;
            }
        }

        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        {
            if (it.value()->inClusters().contains(CLUSTER_BASIC))
            {
                if (m_adapter->unicastRequest(id, device->networkAddress(), 0x01, it.key(), CLUSTER_BASIC, readAttributesRequest(id, 0x0000, {0x0001, 0x0004, 0x0005, 0x0007, 0x4000})))
                    return true;

                interviewError(device, "read basic attributes request failed");
                return false;
            }
        }

        interviewError(device, "device has empty manufacturer name or model name");
        return false;
    }

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
    {
        if (!device->batteryPowered() && it.value()->inClusters().contains(CLUSTER_COLOR_CONTROL) && it.value()->colorCapabilities() != 0xFFFF)
        {
            if (m_adapter->unicastRequest(id, device->networkAddress(), 0x01, it.key(), CLUSTER_COLOR_CONTROL, readAttributesRequest(id, 0x0000, {0x400A})))
                return true;

            interviewError(device, "read color capabilities request failed");
            return false;
        }

        if (!it.value()->inClusters().contains(CLUSTER_IAS_ZONE))
            continue;

        switch (it.value()->zoneStatus())
        {
            case ZoneStatus::Unknown:
            {
                if (m_adapter->unicastRequest(id, device->networkAddress(), 0x01, it.key(), CLUSTER_IAS_ZONE, readAttributesRequest(id, 0x0000, {0x0000, 0x0001, 0x0010})))
                    return true;

                interviewError(device, "read current IAS zone status request failed");
                return false;
            }

            case ZoneStatus::SetAddress:
            {
                quint64 ieeeAddress;

                memcpy(&ieeeAddress, m_adapter->ieeeAddress().constData(), sizeof(ieeeAddress));
                ieeeAddress = qToLittleEndian(qFromBigEndian(ieeeAddress));

                if (m_adapter->unicastRequest(id, device->networkAddress(), 0x01, it.key(), CLUSTER_IAS_ZONE, writeAttributeRequest(id, 0x0000, 0x0010, DATA_TYPE_IEEE_ADDRESS, QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)))))
                    return true;

                interviewError(device, "write IAS zone CIE address request failed");
                return false;
            }

            case ZoneStatus::Enroll:
            {
                iasZoneEnrollResponseStruct payload;

                payload.responseCode = 0x00;
                payload.zoneId = IAS_ZONE_ID;

                m_adapter->unicastRequest(id, device->networkAddress(), 0x01, it.key(), CLUSTER_IAS_ZONE, zclHeader(FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE, id, 0x00).append(reinterpret_cast <char*> (&payload), sizeof(payload)));
                m_adapter->unicastRequest(id, device->networkAddress(), 0x01, it.key(), CLUSTER_IAS_ZONE, readAttributesRequest(id, 0x0000, {0x0000, 0x0010}));
                break;
            }

            case ZoneStatus::Enrolled:
            {
                logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", it.key()) << "IAS zone enrolled";
                break;
            }
        }
    }

    interviewFinished(device);
    return true;
}

bool ZigBee::interviewQuirks(const Device &device)
{
    if (device->options().value("ikeaCover").toBool())
    {
        QList <QString> list = device->firmware().split('.');

        if (list.value(0).toInt() >= 24)
        {
            quint32 value = qToLittleEndian <quint32> (172800);
            enqueueRequest(device, 0x01, CLUSTER_POLL_CONTROL, writeAttributeRequest(m_requestId, 0x0000, 0x0000, DATA_TYPE_32BIT_UNSIGNED, QByteArray(reinterpret_cast <char*> (&value), sizeof(value))), "polling configuration request");
        }
    }

    if (device->options().value("ikeaRemote").toBool())
    {
        QList <QString> list = device->firmware().split('.');
        quint16 groupId = qToLittleEndian <quint16> (IKEA_GROUP);
        bool check = list.value(0).toInt() < 2 || (list.value(0).toInt() == 2 && list.value(1).toInt() < 3) || (list.value(0).toInt() == 2 && list.value(1).toInt() == 3 && list.value(2).toInt() < 75);

        if ((check && !bindRequest(device, 0x01, CLUSTER_ON_OFF, QByteArray(reinterpret_cast <char*> (&groupId), sizeof(groupId)), 0xFF)) || (!check && !bindRequest(device, 0x01, CLUSTER_ON_OFF)))
            return false;
    }

    if (device->manufacturerName() == "LUMI" && device->modelName() == "lumi.switch.n3acn3") // TODO: make it optional?
        enqueueRequest(device, 0x01, CLUSTER_LUMI, writeAttributeRequest(m_requestId, MANUFACTURER_CODE_LUMI, 0x0200, DATA_TYPE_8BIT_UNSIGNED, QByteArray(1, 0x01)), "magic request");

    if (device->options().value("tuyaMagic").toBool())
        enqueueRequest(device, 0x01, CLUSTER_BASIC, readAttributesRequest(m_requestId, 0x0000, {0x0004, 0x0000, 0x0001, 0x0005, 0x0007, 0xFFFE}), "magic request");

    if (device->options().value("tuyaDataQuery").toBool())
        enqueueRequest(device, 0x01, CLUSTER_TUYA_DATA, zclHeader(FC_CLUSTER_SPECIFIC, m_requestId, 0x03), "data query request");

    if (device->manufacturerName() == "_TZ3000_xwh1e22x")
    {
        QByteArray payload = QByteArray(1, 0x08);

        for (quint8 i = 0; i < 8; i++)
        {
            quint16 value = qToLittleEndian <quint16> (i + 1001);
            payload.append(reinterpret_cast <char*> (&value), sizeof(value)).append(1, i + 1);
        }

        enqueueRequest(device, 0x01, CLUSTER_GROUPS, zclHeader(FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE, m_requestId, 0xF0).append(payload), "groups setup request");
    }

    return true;
}

void ZigBee::interviewDevice(const Device &device)
{
    if (device->interviewFinished())
        return;

    connect(device->timer(), &QTimer::timeout, this, &ZigBee::interviewTimeout, Qt::UniqueConnection);
    device->timer()->setSingleShot(true);
    device->timer()->start(DEVICE_INTERVIEW_TIMEOUT);

    enqueueRequest(device, RequestType::Interview);
}

void ZigBee::interviewFinished(const Device &device)
{
    device->timer()->stop();

    if (device->interviewFinished()) // TODO: figure out reasons for multiple interviewFinished calls
        return;

    logInfo << "Device" << device->name() << "manufacturer name is" << device->manufacturerName() << "and model name is" << device->modelName();
    m_devices->setupDevice(device);

    if (!device->firmware().isEmpty())
        logInfo << "Device" << device->name() << "firmware is" << device->firmware();

    if (!device->description().isEmpty())
        logInfo << "Device" << device->name() << "identified as" << device->description();

    if (!interviewQuirks(device) || !configureDevice(device))
    {
        logWarning << "Device" << device->name() << "interview finished with errors";
        emit deviceEvent(device.data(), Event::interviewError);
    }
    else
    {
        logInfo << "Device" << device->name() << "interview finished successfully";
        emit deviceEvent(device.data(), Event::interviewFinished);
        device->setInterviewFinished();
    }

    m_devices->storeDatabase();
}

void ZigBee::interviewError(const Device &device, const QString &reason)
{
    if (!device->timer()->isActive())
        return;

    logWarning << "Device" << device->name() << "interview error:" << reason;
    emit deviceEvent(device.data(), Event::interviewError);

    device->timer()->stop();
}

bool ZigBee::bindRequest(const Device &device, quint8 endpointId, quint16 clusterId, const QByteArray &address, quint8 dstEndpointId, bool unbind)
{
    m_adapter->setRequestParameters(device->ieeeAddress(), device->batteryPowered());
    m_replyId = m_requestId;
    m_replyReceived = false;

    if (!m_adapter->bindRequest(m_requestId, device->networkAddress(), endpointId, clusterId, address, dstEndpointId, unbind))
    {
        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpointId) << "cluster" << QString::asprintf("0x%04x", clusterId) << (unbind ? "unbinding" : "binding") << "request aborted";
        return false;
    }

    if (!m_replyReceived && !m_adapter->waitForSignal(this, SIGNAL(replyReceived()), NETWORK_REQUEST_TIMEOUT))
    {
        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpointId) << "cluster" << QString::asprintf("0x%04x", clusterId) << (unbind ? "unbinding" : "binding") << "timed out";
        return false;
    }

    m_requestId++;

    if (m_requestStatus)
    {
        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpointId) << "cluster" << QString::asprintf("0x%04x", clusterId) << (unbind ? "unbinding" : "binding") << "failed, status code:" << QString::asprintf("0x%02x", m_requestStatus);
        return false;
    }

    logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpointId) << "cluster" << QString::asprintf("0x%04x", clusterId) << (unbind ? "unbinding" : "binding") << "finished successfully";
    return true;
}

bool ZigBee::configureReporting(const Device &device, quint8 endpointId, const Reporting &reporting)
{
    QMap <QString, QVariant> options = device->options().value(device->options().contains("reporting") ? "reporting" : QString(reporting->name()).append("Reporting")).toMap();
    QByteArray request = zclHeader(0x00, m_requestId, CMD_CONFIGURE_REPORTING);

    for (int i = 0; i < reporting->attributes().count(); i++)
    {
        configureReportingStruct item;

        item.direction = 0x00;
        item.attributeId = qToLittleEndian(reporting->attributes().at(i));
        item.dataType = reporting->dataType();
        item.minInterval = qToLittleEndian <quint16> (options.contains("minInterval") ? options.value("minInterval").toInt() : reporting->minInterval());
        item.maxInterval = qToLittleEndian <quint16> (options.contains("maxInterval") ? options.value("maxInterval").toInt() : reporting->maxInterval());
        item.valueChange = qToLittleEndian <quint64> (options.contains("valueChange") ? options.value("valueChange").toInt() : reporting->valueChange());

        request.append(reinterpret_cast <char*> (&item), sizeof(item) - sizeof(item.valueChange) + zclDataSize(item.dataType));
    }

    m_adapter->setRequestParameters(device->ieeeAddress(), device->batteryPowered());
    m_replyId = m_requestId;
    m_replyReceived = false;

    if (!m_adapter->unicastRequest(m_requestId, device->networkAddress(), 0x01, endpointId, reporting->clusterId(), request))
    {
        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpointId) << reporting->name().toUtf8().constData() << "reporting configuration request aborted";
        return false;
    }

    if (!m_replyReceived && !m_adapter->waitForSignal(this, SIGNAL(replyReceived()), NETWORK_REQUEST_TIMEOUT))
    {
        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpointId) << reporting->name().toUtf8().constData() << "reporting configuration request timed out";
        return false;
    }

    m_requestId++;

    if (m_requestStatus)
    {
        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpointId) << reporting->name().toUtf8().constData() << "reporting configuration request failed, status code:" << QString::asprintf("0x%02x", m_requestStatus);
        return false;
    }

    if (reporting->name() == "battery")
        enqueueRequest(device, endpointId, CLUSTER_POWER_CONFIGURATION, readAttributesRequest(m_requestId, 0x0000, reporting->attributes()));

    logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpointId) << reporting->name().toUtf8().constData() << "reporting configuration request finished successfully";
    return true;
}

bool ZigBee::configureDevice(const Device &device)
{
    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        for (int i = 0; i < it.value()->bindings().count(); i++)
            if (!bindRequest(device, it.value()->id(), it.value()->bindings().at(i)->clusterId()))
                return false;

    for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        for (int i = 0; i < it.value()->reportings().count(); i++)
            if (!configureReporting(device, it.value()->id(), it.value()->reportings().at(i)))
                return false;

    return true;
}

bool ZigBee::parseProperty(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint16 itemId, const QByteArray &data, bool command)
{
    Device device = endpoint->device();
    bool check = false;

    for (int i = 0; i < endpoint->properties().count(); i++)
    {
        const Property &property = endpoint->properties().at(i);

        if (property->clusters().contains(clusterId))
        {
            QVariant value = property->value();

            if (device->options().value("checkTransactionId").toBool() && property->transactionId() == transactionId)
                continue;

            if (command)
                property->parseCommand(clusterId, static_cast <quint8> (itemId), data);
            else
                property->parseAttribte(clusterId, itemId, data);

            property->setTransactionId(transactionId);
            check = true;

            while (!property->queue().isEmpty())
            {
                const PropertyRequest &request = property->queue().dequeue();
                enqueueRequest(device, endpoint->id(), request.clusterId, request.data);
            }

            if (property->timeout())
                property->setTime(QDateTime::currentSecsSinceEpoch());

            if (m_debounce && property->value() == value)
                continue;

            m_devices->storeProperties();
            endpoint->setUpdated(true);
        }
    }

    return check;
}

void ZigBee::parseAttribute(const Endpoint &endpoint, quint16 clusterId, quint8 transactionId, quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    Device device = endpoint->device();

    if (m_debug)
        logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "cluster" << QString::asprintf("0x%04x", clusterId) << "attribute" << QString::asprintf("0x%04x", attributeId) << "report received with type" << QString::asprintf("0x%02x", dataType) << "and data" << (data.isEmpty() ? "(empty)" : data.toHex(':')) << "and transaction id" << transactionId;

    if (clusterId == CLUSTER_BASIC && attributeId <= 0x4000)
    {
        switch (attributeId)
        {
            case 0x0001:

                if (dataType != DATA_TYPE_8BIT_UNSIGNED)
                    return;

                device->setVersion(static_cast <quint8> (data.at(0)));
                break;

            case 0x0004:

                if (dataType != DATA_TYPE_CHARACTER_STRING)
                    return;

                device->setManufacturerName(data != "\u0002KE" ? QString(data).trimmed() : "IKEA of Sweden");
                break;

            case 0x0005:

                if (dataType != DATA_TYPE_CHARACTER_STRING)
                    return;

                device->setModelName(QString(data).trimmed());
                break;

            case 0x0007:

                if (dataType != DATA_TYPE_8BIT_UNSIGNED && dataType != DATA_TYPE_8BIT_ENUM)
                    return;

                device->setPowerSource(static_cast <quint8> (data.at(0)));
                break;

            case 0x4000:

                if (dataType != DATA_TYPE_CHARACTER_STRING)
                    return;

                device->setFirmware(QString(data).trimmed());
                break;
        }

        if (!device->interviewFinished() && !device->manufacturerName().isEmpty() && !device->modelName().isEmpty() && (attributeId == 0x0004 || attributeId == 0x0005))
            interviewDevice(device);

        return;
    }

    if (clusterId == CLUSTER_TIME && device->manufacturerName().contains("efekta", Qt::CaseInsensitive))
    {
        QDateTime now = QDateTime::currentDateTime();
        quint32 value = qToLittleEndian <quint32> (now.toTime_t() + now.offsetFromUtc() - TIME_OFFSET);

        if (m_debug)
            logInfo << "Device" << device->name() << "requested EFEKTA time synchronization";

        enqueueRequest(device, endpoint->id(), CLUSTER_TIME, writeAttributeRequest(m_requestId, 0x0000, 0x0000, DATA_TYPE_UTC_TIME, QByteArray(reinterpret_cast <char*> (&value), sizeof(value))));
        return;
    }

    if (clusterId == CLUSTER_COLOR_CONTROL && attributeId == 0x400A)
    {
        quint16 value = 0xFFFF;

        if (dataType == DATA_TYPE_16BIT_BITMAP)
        {
            memcpy(&value, data.constData(), data.length());
            value = qFromLittleEndian(value);
        }

        endpoint->setColorCapabilities(value);
        logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "color capabilities:" << QString::asprintf("0x%04x", endpoint->colorCapabilities());
        interviewDevice(device);
        return;
    }

    if (clusterId == CLUSTER_IAS_ZONE)
    {
        if (attributeId == 0x0010 && dataType == DATA_TYPE_NO_DATA) // TODO: figure it out
        {
            endpoint->setZoneStatus(ZoneStatus::Enrolled);
            interviewDevice(device);
            return;
        }

        switch (attributeId)
        {
            case 0x0000:
            {
                if (dataType != DATA_TYPE_8BIT_ENUM)
                    return;

                endpoint->setZoneStatus(data.at(0) ? ZoneStatus::Enrolled : ZoneStatus::Enroll);
                return;
            }

            case 0x0001:
            {
                quint16 value;

                if (dataType != DATA_TYPE_16BIT_ENUM)
                    return;

                memcpy(&value, data.constData(), data.length());
                endpoint->setZoneType(qFromLittleEndian(value));
                logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "IAS zone type:" << QString::asprintf("0x%04x", endpoint->zoneType());
                return;
            }

            case 0x0010:
            {
                quint64 ieeeAddress;

                if (dataType != DATA_TYPE_IEEE_ADDRESS)
                    return;

                memcpy(&ieeeAddress, m_adapter->ieeeAddress().constData(), sizeof(ieeeAddress));
                ieeeAddress = qToLittleEndian(qFromBigEndian(ieeeAddress));

                if (memcmp(&ieeeAddress, data.constData(), sizeof(ieeeAddress)))
                    endpoint->setZoneStatus(ZoneStatus::SetAddress);

                interviewDevice(device);
                return;
            }
        }
    }

    if (!device->interviewFinished() || parseProperty(endpoint, clusterId, transactionId, attributeId, data) || !m_debug)
        return;

    logWarning << "No property found for device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "cluster" << QString::asprintf("0x%04x", clusterId) << "attribute" << QString::asprintf("0x%04x", attributeId) << "report with type" << QString::asprintf("0x%02x", dataType) << "and data" << (data.isEmpty() ? "(empty)" : data.toHex(':'));
}

void ZigBee::clusterCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint16 manufacturerCode, quint8 transactionId, quint8 commandId, const QByteArray &payload)
{
    Device device = endpoint->device();

    if (m_debug)
        logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "cluster" << QString::asprintf("0x%04x", clusterId) << "command" << QString::asprintf("0x%02x", commandId) << "received with payload" << (payload.isEmpty() ? "(empty)" : payload.toHex(':')) << "and transaction id" << transactionId;

    if (clusterId == CLUSTER_IDENTIFY)
        return;

    if (clusterId == CLUSTER_GROUPS)
    {
        switch (commandId)
        {
            case 0x00:
            case 0x03:
            {
                const groupControlResponseStruct *response = reinterpret_cast <const groupControlResponseStruct*> (payload.constData());

                switch (response->status)
                {
                    case STATUS_SUCCESS:
                        logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "group" << qFromLittleEndian(response->groupId) << "successfully" << (commandId ? "removed" : "added");
                        break;

                    case STATUS_INSUFFICIENT_SPACE:
                        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "group" << qFromLittleEndian(response->groupId) << "not added, no free space available";
                        break;

                    case STATUS_DUPLICATE_EXISTS:
                        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "group" << qFromLittleEndian(response->groupId) << "already exists";
                        break;

                    case STATUS_NOT_FOUND:
                        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "group" << qFromLittleEndian(response->groupId) << "not found";
                        break;

                    default:
                        logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "group" << qFromLittleEndian(response->groupId) << (commandId ? "remove" : "add") << "command status" << QString::asprintf("0x%02x", response->status) << "unrecognized";
                        break;
                }

                break;
            }

            default:
                logWarning << "Unrecognized group control command" << QString::asprintf("0x%02x", commandId) << "received from device" << device->name() << "with payload:" << (payload.isEmpty() ? "(empty)" : payload.toHex(':'));
                break;
        }

        return;
    }

    if (clusterId == CLUSTER_OTA_UPGRADE)
    {
        otaFileHeaderStruct header;

        if (device != m_otaDevice)
        {
            otaError(endpoint, manufacturerCode, transactionId, commandId);
            return;
        }

        if (!m_otaFile.exists() || !m_otaFile.open(QFile::ReadOnly))
        {
            otaError(endpoint, manufacturerCode, transactionId, commandId, "OTA upgrade failed, file open error");
            return;
        }

        memcpy(&header, m_otaFile.read(sizeof(header)).constData(), sizeof(header));

        switch (commandId)
        {
            case 0x01:
            {
                const otaNextImageRequestStruct *request = reinterpret_cast <const otaNextImageRequestStruct*> (payload.constData());
                otaNextImageResponseStruct response;

                if (!m_otaForce && (request->manufacturerCode != header.manufacturerCode || request->imageType != header.imageType))
                {
                    otaError(endpoint, manufacturerCode, transactionId, commandId, "OTA upgrade image request failed, header data mismatch request data");
                    break;
                }

                if (request->fileVersion == header.fileVersion)
                {
                    otaError(endpoint, manufacturerCode, transactionId, commandId, QString::asprintf("OTA upgrade not started, version match: 0x%08x", qFromLittleEndian(request->fileVersion)).toUtf8().constData());
                    break;
                }

                response.status = 0x00;
                response.manufacturerCode = m_otaForce ? request->manufacturerCode : header.manufacturerCode;
                response.imageType = m_otaForce ? request->imageType : header.imageType;
                response.fileVersion = header.fileVersion;
                response.imageSize = header.imageSize;

                logInfo << "Device" << device->name() << "OTA upgrade started...";
                enqueueRequest(device, endpoint->id(), CLUSTER_OTA_UPGRADE, zclHeader(FC_CLUSTER_SPECIFIC | FC_SERVER_TO_CLIENT | FC_DISABLE_DEFAULT_RESPONSE, transactionId, 0x02).append(reinterpret_cast <char*> (&response), sizeof(response)));
                break;
            }

            case 0x03:
            {
                const otaImageBlockRequestStruct *request = reinterpret_cast <const otaImageBlockRequestStruct*> (payload.constData());
                otaImageBlockResponseStruct response;
                QByteArray block;

                if (!m_otaForce && (request->manufacturerCode != header.manufacturerCode || request->imageType != header.imageType || request->fileVersion != header.fileVersion))
                {
                    otaError(endpoint, manufacturerCode, transactionId, commandId, "OTA upgrade block request failed, header data mismatch request data");
                    break;
                }

                m_otaFile.seek(qFromLittleEndian(request->fileOffset));
                block = m_otaFile.read(request->maxDataSize);

                response.status = 0x00;
                response.manufacturerCode = request->manufacturerCode;
                response.imageType = request->imageType;
                response.fileVersion = request->fileVersion;
                response.fileOffset = request->fileOffset;
                response.dataSize = static_cast <quint8> (block.length());

                logInfo << "Device" << device->name() << "OTA upgrade progress is" << QString::asprintf("%.2f%%", static_cast <double> (m_otaFile.pos() + block.size()) / m_otaFile.size() * 100).toUtf8().constData();
                enqueueRequest(device, endpoint->id(), CLUSTER_OTA_UPGRADE, zclHeader(FC_CLUSTER_SPECIFIC | FC_SERVER_TO_CLIENT | FC_DISABLE_DEFAULT_RESPONSE, transactionId, 0x05).append(reinterpret_cast <char*> (&response), sizeof(response)).append(block));
                break;
            }
            case 0x06:
            {
                const otaUpgradeEndRequestStruct *request = reinterpret_cast <const otaUpgradeEndRequestStruct*> (payload.constData());
                otaUpgradeEndResponseStruct response;

                if (request->status)
                {
                    logWarning << "Device" << device->name() << "OTA upgrade finished with error, status code:" << QString::asprintf("0x%02x", request->status);
                    break;
                }

                response.manufacturerCode = request->manufacturerCode;
                response.imageType = request->imageType;
                response.fileVersion = request->fileVersion;
                response.currentTime = 0;
                response.upgradeTime = 0;

                logInfo << "Device" << device->name() << "OTA upgrade finished successfully";
                enqueueRequest(device, endpoint->id(), CLUSTER_OTA_UPGRADE, zclHeader(FC_CLUSTER_SPECIFIC | FC_SERVER_TO_CLIENT | FC_DISABLE_DEFAULT_RESPONSE, transactionId, 0x07).append(reinterpret_cast <char*> (&response), sizeof(response)));
                m_devices->removeDevice(device);
                break;
            }

            default:
                logWarning << "Unrecognized OTA upgrade command" << QString::asprintf("0x%02x", commandId) << "received from device" << device->name() << "with payload:" << (payload.isEmpty() ? "(empty)" : payload.toHex(':'));
                break;
        }

        if (m_otaFile.isOpen())
            m_otaFile.close();

        return;
    }

    if (clusterId == CLUSTER_IAS_ZONE && commandId == 0x01)
    {
        iasZoneEnrollResponseStruct response;

        if (m_debug)
            logInfo << "Device" << device->name() << "requested IAS Zone enroll";

        response.responseCode = 0x00;
        response.zoneId = IAS_ZONE_ID;

        enqueueRequest(device, endpoint->id(), CLUSTER_IAS_ZONE, zclHeader(FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE, transactionId, 0x00).append(reinterpret_cast <char*> (&response), sizeof(response)));
        return;
    }

    if (clusterId == CLUSTER_TUYA_DATA)
    {
        switch (commandId)
        {
            case 0x24:
            {
                QDateTime now = QDateTime::currentDateTime();
                quint32 value = now.toTime_t();
                tuyaTimeStruct response;

                if (m_debug)
                    logInfo << "Device" << device->name() << "requested TUYA time synchronization";

                response.payloadSize = qToLittleEndian <quint16> (8);
                response.utcTimestamp = qToBigEndian(value);
                response.localTimestamp = qToBigEndian(value + now.offsetFromUtc());

                enqueueRequest(device, endpoint->id(), CLUSTER_TUYA_DATA, zclHeader(FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE, transactionId, 0x24).append(reinterpret_cast <char*> (&response), sizeof(response)));
                return;
            }

            case 0x25:
            {
                enqueueRequest(device, endpoint->id(), CLUSTER_TUYA_DATA, zclHeader(FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE, transactionId, 0x25).append(QByteArray::fromHex("010001")));
                return;
            }
        }
    }

    if (!device->interviewFinished() || parseProperty(endpoint, clusterId, transactionId, commandId, payload, true) || !m_debug)
        return;

    logWarning << "No property found for device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "cluster" << QString::asprintf("0x%04x", clusterId) << "command" << QString::asprintf("0x%02x", commandId) << "with payload" << (payload.isEmpty() ? "(empty)" : payload.toHex(':'));
}

void ZigBee::globalCommandReceived(const Endpoint &endpoint, quint16 clusterId, quint16 manufacturerCode, quint8 transactionId, quint8 commandId, QByteArray payload)
{
    Device device = endpoint->device();

    switch (commandId)
    {
        case CMD_CONFIGURE_REPORTING_RESPONSE:

            if (payload.at(0))
                logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "cluster" << QString::asprintf("0x%04x", clusterId) << "reporting configuration error, status code:" << QString::asprintf("0x%02x", static_cast <quint8> (payload.at(0)));

            break;

        case CMD_DEFAULT_RESPONSE:

            if (payload.at(1))
                logWarning << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "cluster" << QString::asprintf("0x%04x", clusterId) << "command" << QString::asprintf("0x%02x", payload.at(0)) << "default response received with error, status code:" << QString::asprintf("0x%02x", static_cast <quint8> (payload.at(1)));

            break;

        case CMD_READ_ATTRIBUTES:
        {
            QByteArray response = zclHeader(FC_SERVER_TO_CLIENT | FC_DISABLE_DEFAULT_RESPONSE, transactionId, CMD_READ_ATTRIBUTES_RESPONSE, manufacturerCode);
            quint16 attributeId;

            for (quint8 i = 0; i < payload.length(); i += sizeof(attributeId))
            {
                memcpy(&attributeId, payload.constData() + i, sizeof(attributeId));
                attributeId = qFromLittleEndian(attributeId);

                response.append(payload.mid(i, sizeof(attributeId)));

                if (clusterId == CLUSTER_TIME && (attributeId == 0x0000 || attributeId == 0x0002 || attributeId == 0x0007))
                {
                    QDateTime now = QDateTime::currentDateTime();
                    quint32 value;

                    response.append(1, static_cast <char> (STATUS_SUCCESS));

                    switch (attributeId)
                    {
                        case 0x0000:

                            if (m_debug)
                                logInfo << "Device" << device->name() << "requested UTC time";

                            value = qToLittleEndian <quint32> (now.toTime_t() + (device->options().value("utcTime").toBool() ? now.offsetFromUtc() : 0) - TIME_OFFSET);
                            response.append(1, static_cast <char> (DATA_TYPE_UTC_TIME)).append(reinterpret_cast <char*> (&value), sizeof(value));
                            break;

                        case 0x0002:

                            if (m_debug)
                                logInfo << "Device" << device->name() << "requested time zone";

                            value = qToLittleEndian <quint32> (now.offsetFromUtc());
                            response.append(1, static_cast <char> (DATA_TYPE_32BIT_SIGNED)).append(reinterpret_cast <char*> (&value), sizeof(value));
                            break;

                        case 0x0007:

                            if (m_debug)
                                logInfo << "Device" << device->name() << "requested local time";

                            value = qToLittleEndian <quint32> (now.toTime_t() + now.offsetFromUtc() - TIME_OFFSET);
                            response.append(1, static_cast <char> (DATA_TYPE_32BIT_UNSIGNED)).append(reinterpret_cast <char*> (&value), sizeof(value));
                            break;
                    }

                    continue;
                }

                logWarning << "Device" << device->name() << "requested unrecognized attribute" << QString::asprintf("0x%04x", attributeId) << "from cluster" << QString::asprintf("0x%04x", clusterId);
                response.append(1, static_cast <char> (STATUS_UNSUPPORTED_ATTRIBUTE));
            }

            enqueueRequest(device, endpoint->id(), clusterId, response);
            break;
        }

        case CMD_READ_ATTRIBUTES_RESPONSE:
        case CMD_REPORT_ATTRIBUTES:
        {
            while (payload.length() > 2)
            {
                quint8 dataType, offset, size = 0;
                quint16 attributeId;

                memcpy(&attributeId, payload.constData(), sizeof(attributeId));
                attributeId = qFromLittleEndian(attributeId);

                if (commandId == CMD_READ_ATTRIBUTES_RESPONSE)
                {
                    if (!payload.at(2))
                    {
                        dataType = static_cast <quint8> (payload.at(3));
                        offset = 4;
                    }
                    else
                    {
                        dataType = DATA_TYPE_NO_DATA;
                        offset = 3;
                    }
                }
                else
                {
                    dataType = static_cast <quint8> (payload.at(2));
                    offset = 3;
                }

                size = zclDataSize(dataType, payload, &offset);

                if (dataType != DATA_TYPE_NO_DATA && dataType != DATA_TYPE_OCTET_STRING && dataType != DATA_TYPE_CHARACTER_STRING && !size)
                {
                    logWarning << "Unrecognized attribute" << QString::asprintf("0x%04x", attributeId) << "type" << QString::asprintf("0x%02x", dataType) << "received from device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "cluster" << QString::asprintf("0x%04x", clusterId) << "with payload:" << payload.mid(offset).toHex(':');
                    return;
                }

                parseAttribute(endpoint, clusterId, transactionId, attributeId, dataType, payload.mid(offset, size));
                payload.remove(0, offset + size);
            }

            break;
        }

        case CMD_WRITE_ATTRIBUTES_RESPONSE:
        {
            if (clusterId == CLUSTER_IAS_ZONE && !payload.at(0))
            {
                endpoint->setZoneStatus(ZoneStatus::Enroll);
                interviewDevice(device);
            }

            break;
        }

        default:
            logWarning << "Unrecognized command" << QString::asprintf("0x%02x", commandId) << "received from device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "cluster" << QString::asprintf("0x%04x", clusterId) << "with payload:" << (payload.isEmpty() ? "(empty)" : payload.toHex(':'));
            break;
    }
}

void ZigBee::touchLinkReset(const QByteArray &ieeeAddress, quint8 channel)
{
    touchLinkScanStruct payload;

    payload.transactionId = QRandomGenerator::global()->generate();
    payload.zigBeeInformation = 0x04;
    payload.touchLinkInformation = 0x12;

    if (!m_adapter->setInterPanChannel(channel))
        return;

    if (!m_adapter->broadcastInterPanRequest(m_requestId, CLUSTER_TOUCHLINK, zclHeader(FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE, m_requestId, 0x00).append(QByteArray(reinterpret_cast <char*> (&payload), sizeof(payload)))))
    {
        logWarning << "TouchLink scan request failed";
        return;
    }

    if (!m_adapter->unicastInterPanRequest(m_requestId, ieeeAddress, CLUSTER_TOUCHLINK, zclHeader(FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE, m_requestId, 0x07).append(QByteArray(reinterpret_cast <char*> (&payload), sizeof(payload.transactionId)))))
    {
        logWarning << "TouchLink reset request failed";
        return;
    }

    logInfo << "TouchLink reset finished successfully";
}

void ZigBee::touchLinkScan(void)
{
    QByteArray request = zclHeader(FC_CLUSTER_SPECIFIC | FC_DISABLE_DEFAULT_RESPONSE, m_requestId, 0x00);
    touchLinkScanStruct payload;
    QEventLoop loop;
    QTimer timer;

    payload.transactionId = QRandomGenerator::global()->generate();
    payload.zigBeeInformation = 0x04;
    payload.touchLinkInformation = 0x12;

    request.append(QByteArray(reinterpret_cast <char*> (&payload), sizeof(payload)));
    logInfo << "TouchLink scan started...";

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    for (m_interPanChannel = 11; m_interPanChannel <= 26; m_interPanChannel++)
    {
        if (!m_adapter->setInterPanChannel(m_interPanChannel))
            return;

        if (!m_adapter->broadcastInterPanRequest(m_requestId, CLUSTER_TOUCHLINK, request))
        {
            logWarning << "TouchLink scan request failed";
            return;
        }

        timer.start(INTER_PAN_CHANNEL_TIMEOUT);
        loop.exec();
    }

    logInfo << "TouchLink scan finished successfully";
}

void ZigBee::interviewTimeoutHandler(const Device &device)
{
    if (device->modelName().startsWith("lumi")) // some LUMI devices send modelName attribute on join
    {
        device->setManufacturerCode(0x1037);
        device->setPowerSource(POWER_SOURCE_BATTERY);
        device->setManufacturerName("LUMI");
        interviewFinished(device);
        return;
    }

    logWarning << "Device" << device->name() << "interview timed out";
    emit deviceEvent(device.data(), Event::interviewTimeout);
}

void ZigBee::rejoinHandler(const Device &device)
{
    if (device->manufacturerName() == "IKEA of Sweden" && !device->batteryPowered())
        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
            for (int i = 0; i < it.value()->reportings().count(); i++)
                configureReporting(device, it.value()->id(), it.value()->reportings().at(i));

    if (device->options().value("tuyaDataQuery").toBool())
        enqueueRequest(device, 0x01, CLUSTER_TUYA_DATA, zclHeader(FC_CLUSTER_SPECIFIC, m_requestId, 0x03), "data query request");
}

void ZigBee::otaError(const Endpoint &endpoint, quint16 manufacturerCode, quint8 transactionId, quint8 commandId, const QString &error)
{
    Device device = endpoint->device();

    if (!error.isEmpty())
        logWarning << "Device" << device->name() << error;

    enqueueRequest(device, endpoint->id(), CLUSTER_OTA_UPGRADE, zclHeader(FC_CLUSTER_SPECIFIC | FC_SERVER_TO_CLIENT | FC_DISABLE_DEFAULT_RESPONSE, transactionId, commandId == 0x01 ? 0x02 : 0x05, manufacturerCode).append(STATUS_NO_IMAGE_AVAILABLE));
}

void ZigBee::blink(quint16 timeout)
{
    if (m_statusLedTimer->isActive() && m_statusLedPin == m_blinkLedPin)
        return;

    GPIO::setStatus(m_blinkLedPin, true);
    QTimer::singleShot(timeout, this, &ZigBee::updateBlinkLed);
}

void ZigBee::adapterReset(void)
{
    m_requestTimer->stop();
}

void ZigBee::coordinatorReady(void)
{
    Device device = m_devices->value(m_adapter->ieeeAddress());

    if (device.isNull())
    {
        device = Device(new DeviceObject(m_adapter->ieeeAddress(), 0x0000, "HOMEd Coordinator"));
        m_devices->insert(device->ieeeAddress(), device);
    }

    for (auto it = m_devices->begin(); it != m_devices->end(); it++)
    {
        if (it.value()->logicalType() == LogicalType::Coordinator && it.key() != device->ieeeAddress())
        {
            logWarning << "Coordinator" << it.value()->ieeeAddress().toHex(':') << "removed";
            m_devices->erase(it++);
        }

        if (it == m_devices->end())
            break;
    }

    device->setManufacturerName(m_adapter->manufacturerName());
    device->setModelName(m_adapter->modelName());
    device->setDiscovery(false);
    device->setCloud(false);
    device->setInterviewFinished();
    device->setRemoved(false);
    device->setLogicalType(LogicalType::Coordinator);
    device->setFirmware(m_adapter->firmware());

    connect(m_adapter, &Adapter::deviceJoined, this, &ZigBee::deviceJoined, Qt::UniqueConnection);
    connect(m_adapter, &Adapter::deviceLeft, this, &ZigBee::deviceLeft, Qt::UniqueConnection);
    connect(m_adapter, &Adapter::zdoMessageReveived, this, &ZigBee::zdoMessageReveived, Qt::UniqueConnection);
    connect(m_adapter, &Adapter::zclMessageReveived, this, &ZigBee::zclMessageReveived, Qt::UniqueConnection);
    connect(m_adapter, &Adapter::rawMessageReveived, this, &ZigBee::rawMessageReveived, Qt::UniqueConnection);

    connect(m_requestTimer, &QTimer::timeout, this, &ZigBee::handleRequests, Qt::UniqueConnection);
    connect(m_neignborsTimer, &QTimer::timeout, this, &ZigBee::updateNeighbors, Qt::UniqueConnection);
    connect(m_pingTimer, &QTimer::timeout, this, &ZigBee::pingDevices, Qt::UniqueConnection);

    logInfo << "Coordinator ready, address:" << device->ieeeAddress().toHex(':').constData();
    m_adapter->setPermitJoin(m_devices->permitJoin());

    if (!m_requests.isEmpty())
        m_requestTimer->start();

    if (!m_neignborsTimer->isActive())
        m_neignborsTimer->start(UPDATE_NEIGHBORS_INTERVAL);

    if (!m_pingTimer->isActive())
    {
        m_pingTimer->start(PING_DEVICES_INTERVAL);
        pingDevices();
    }

    emit networkStarted();
}

void ZigBee::permitJoinUpdated(bool enabled)
{
    if (enabled)
    {
        m_statusLedTimer->start(STATUS_LED_TIMEOUT);
        GPIO::setStatus(m_statusLedPin, true);
    }
    else
    {
        m_statusLedTimer->stop();
        GPIO::setStatus(m_statusLedPin, m_statusLedPin != m_blinkLedPin);
    }

    m_devices->setPermitJoin(enabled);
    m_devices->storeDatabase();
}

void ZigBee::deviceJoined(const QByteArray &ieeeAddress, quint16 networkAddress)
{
    auto it = m_devices->find(ieeeAddress);

    if (it == m_devices->end())
    {
        logInfo << "Device" << ieeeAddress.toHex(':') << "joined network with address" << QString::asprintf("0x%04x", networkAddress);
        it = m_devices->insert(ieeeAddress, Device(new DeviceObject(ieeeAddress, networkAddress)));
        it.value()->setDiscovery(m_discovery);
        it.value()->setCloud(m_cloud);
    }
    else
    {
        if (it.value()->removed())
            it.value()->setRemoved(false);

        if (it.value()->joinTime() + DEVICE_REJOIN_TIMEOUT > QDateTime::currentMSecsSinceEpoch())
            return;

        logInfo << "Device" << it.value()->name() << "rejoined network with address" << QString::asprintf("0x%04x", networkAddress);
        rejoinHandler(it.value());
    }

    it.value()->updateJoinTime();
    it.value()->updateLastSeen();
    blink(500);

    if (it.value()->networkAddress() != networkAddress)
    {
        logInfo << "Device" << it.value()->name() << "network address updated";
        it.value()->setNetworkAddress(networkAddress);
    }

    if (!it.value()->interviewFinished() && !it.value()->timer()->isActive())
    {
        logInfo << "Device" << it.value()->name() << "interview started...";
        interviewDevice(it.value());
    }

    emit deviceEvent(it.value().data(), Event::deviceJoined);
}

void ZigBee::deviceLeft(const QByteArray &ieeeAddress)
{
    auto it = m_devices->find(ieeeAddress);

    if (it == m_devices->end() || it.value()->removed() || it.value()->logicalType() == LogicalType::Coordinator)
        return;

    it.value()->timer()->stop();
    blink(500);

    logInfo << "Device" << it.value()->name() << "left network";
    emit deviceEvent(it.value().data(), Event::deviceLeft);

    m_devices->removeDevice(it.value());
    m_devices->storeDatabase();
}

void ZigBee::zdoMessageReveived(quint16 networkAddress, quint16 clusterId, const QByteArray &payload)
{
    Device device = m_devices->byNetwork(networkAddress);

    if (device.isNull() || device->removed() || !device->active())
        return;

    switch (clusterId & 0x00FF)
    {
        case ZDO_BIND_REQUEST:
        case ZDO_UNBIND_REQUEST:
        case ZDO_LEAVE_REQUEST:
            // TODO: check response status here
            break;

        case ZDO_NODE_DESCRIPTOR_REQUEST:
        {
            const nodeDescriptorResponseStruct *response = reinterpret_cast <const nodeDescriptorResponseStruct*> (payload.constData());

            if (device->interviewFinished())
                break;

            if (!response->status)
            {
                device->setLogicalType(static_cast <LogicalType> (response->logicalType & 0x03));
                device->setManufacturerCode(qFromLittleEndian(response->manufacturerCode));
                device->setDescriptorReceived();

                logInfo << "Device" << device->name() << "node descriptor received, manufacturer code is" << QString::asprintf("0x%04x", device->manufacturerCode()) << "and logical type is" << QString(device->logicalType() == LogicalType::Router ? "router" : "end device");
                interviewDevice(device);
                break;
            }

            interviewError(device, QString::asprintf("node descriptor response error %0x02x", response->status));
            break;
        }

        case ZDO_SIMPLE_DESCRIPTOR_REQUEST:
        {
            const simpleDescriptorResponseStruct *response = reinterpret_cast <const simpleDescriptorResponseStruct*> (payload.constData());
            Endpoint endpoint = m_devices->endpoint(device, response->status ? device->interviewEndpointId() : response->endpointId);

            if (device->interviewFinished())
                break;

            if (!response->status)
            {
                QByteArray clusterData = payload.mid(sizeof(simpleDescriptorResponseStruct));
                quint16 clusterId;

                endpoint->setProfileId(qFromLittleEndian(response->profileId));
                endpoint->setDeviceId(qFromLittleEndian(response->deviceId));

                endpoint->inClusters().clear();
                endpoint->outClusters().clear();

                for (quint8 i = 0; i < static_cast <quint8> (clusterData.at(0)); i++)
                {
                    memcpy(&clusterId, clusterData.constData() + i * sizeof(clusterId) + 1, sizeof(clusterId));
                    endpoint->inClusters().append(qFromLittleEndian(clusterId));
                }

                clusterData.remove(0, clusterData.at(0) * sizeof(clusterId) + 1);

                for (quint8 i = 0; i < static_cast <quint8> (clusterData.at(0)); i++)
                {
                    memcpy(&clusterId, clusterData.constData() + i * sizeof(clusterId) + 1, sizeof(clusterId));
                    endpoint->outClusters().append(qFromLittleEndian(clusterId));
                }
            }

            endpoint->setDescriptorReceived();

            logInfo << "Device" << device->name() << "endpoint" << QString::asprintf("0x%02x", endpoint->id()) << "simple descriptor" << (response->status ? "unavailable" : "received");
            interviewDevice(device);
            break;
        }

        case ZDO_ACTIVE_ENDPOINTS_REQUEST:
        {
            const activeEndpointsResponseStruct *response = reinterpret_cast <const activeEndpointsResponseStruct*> (payload.constData());

            if (device->interviewFinished())
                break;

            if (!response->status)
            {
                QByteArray data = payload.mid(sizeof(activeEndpointsResponseStruct), response->count);
                QList <QString> list;

                for (int i = 0; i < data.length(); i++)
                {
                    quint8 endpointId = static_cast <quint8> (data.at(i));

                    if (device->endpoints().find(endpointId) == device->endpoints().end())
                        device->endpoints().insert(endpointId, Endpoint(new EndpointObject(endpointId, device)));

                    list.append(QString::asprintf("0x%02x", endpointId));
                }

                device->setEndpointsReceived();

                logInfo << "Device" << device->name() << "active endpoints received:" << list.join(", ");
                interviewDevice(device);
                break;
            }

            interviewError(device, QString::asprintf("active endpoints response error %0x02x", response->status));
            break;
        }

        case ZDO_LQI_REQUEST:
        {
            const lqiResponseStruct *response = reinterpret_cast <const lqiResponseStruct*> (payload.constData());

            if (!response->status)
            {
                if (!response->index)
                {
                    logInfo << "Device" << device->name() << "neighbors list received";
                    device->neighbors().clear();
                }

                for (quint8 i = 0; i < response->count; i++)
                {
                    const neighborRecordStruct *neighbor = reinterpret_cast <const neighborRecordStruct*> (payload.constData() + sizeof(lqiResponseStruct) + i * sizeof(neighborRecordStruct));
                    device->neighbors().insert(qFromLittleEndian(neighbor->networkAddress), neighbor->linkQuality);
                }

                if (response->total > response->index + response->count)
                {
                    device->setLqiRequestIndex(response->index + response->count);
                    enqueueRequest(device, RequestType::LQI);
                }

                break;
            }

            break;
        }

        default:
        {
            logWarning << "Unrecognized ZDO message" << QString::asprintf("0x%04x", clusterId) << "received with payload" << (payload.isEmpty() ? "(empty)" : payload.toHex(':'));
            break;
        }
    }

    device->updateLastSeen();
}

void ZigBee::zclMessageReveived(quint16 networkAddress, quint8 endpointId, quint16 clusterId, quint8 linkQuality, const QByteArray &payload)
{
    Device device = m_devices->byNetwork(networkAddress);
    Endpoint endpoint;
    quint16 manufacturerCode = 0;
    quint8 frameControl = static_cast <quint8> (payload.at(0)), transactionId, commandId;
    QByteArray data;
    Request request;

    if (device.isNull() || device->removed() || !device->active())
        return;

    device->setLinkQuality(linkQuality);
    endpoint = m_devices->endpoint(device, endpointId);
    blink(50);

    if (frameControl & FC_MANUFACTURER_SPECIFIC)
    {
        memcpy(&manufacturerCode, payload.constData() + 1, sizeof(manufacturerCode));
        manufacturerCode = qFromLittleEndian(manufacturerCode);
        transactionId = static_cast <quint8> (payload.at(3));
        commandId = static_cast <quint8> (payload.at(4));
        data = payload.mid(5);
    }
    else
    {
        transactionId = static_cast <quint8> (payload.at(1));
        commandId = static_cast <quint8> (payload.at(2));
        data = payload.mid(3);
    }

    request = m_requests.value(transactionId);

    if (!request.isNull() && request->type() == RequestType::Data && qvariant_cast <DataRequest> (request->data())->debug())
    {
        QJsonObject json = {{"endpointId", endpointId}, {"clusterId", clusterId}, {"commandId", commandId}, {"payload", data.toHex(':').constData()}};

        if (manufacturerCode)
            json.insert("manufacturerCode", manufacturerCode);

        emit deviceEvent(device.data(), frameControl & FC_CLUSTER_SPECIFIC ? Event::clusterRequest : Event::globalRequest, json);
    }

    if (frameControl & FC_CLUSTER_SPECIFIC)
        clusterCommandReceived(endpoint, clusterId, manufacturerCode, transactionId, commandId, data);
    else
        globalCommandReceived(endpoint, clusterId, manufacturerCode, transactionId, commandId, data);

    if (device->interviewFinished() && (frameControl & FC_CLUSTER_SPECIFIC || commandId == CMD_REPORT_ATTRIBUTES) && !(frameControl & FC_DISABLE_DEFAULT_RESPONSE))
    {
        defaultResponseStruct response;

        response.commandId = commandId;
        response.status = 0x00;

        enqueueRequest(device, endpoint->id(), clusterId, zclHeader(FC_SERVER_TO_CLIENT | FC_DISABLE_DEFAULT_RESPONSE, transactionId, CMD_DEFAULT_RESPONSE, manufacturerCode).append(QByteArray(reinterpret_cast <char*> (&response), sizeof(response))));
    }

    if (endpoint->updated() || (endpoint->properties().isEmpty() && endpoint->inClusters().contains(CLUSTER_BASIC)))
        emit endpointUpdated(device.data(), endpoint->id());

    device->updateLastSeen();
}

void ZigBee::rawMessageReveived(const QByteArray &ieeeAddress, quint16 clusterId, quint8 linkQuality, const QByteArray &data)
{
    if (clusterId == CLUSTER_TOUCHLINK && data.at(2) == 0x01)
    {
        logInfo << "TouchLink scan response received from device" << ieeeAddress.toHex(':') << "at channel" << m_interPanChannel << "with link quality" << linkQuality;
        return;
    }

    logWarning << "Unrecognized raw message received from" << ieeeAddress.toHex(':') << "cluster" << QString::asprintf("0x%04x", clusterId) << "with data" << (data.isEmpty() ? "(empty)" : data.toHex(':'));
}

void ZigBee::requestFinished(quint8 id, quint8 status)
{
    auto it = m_requests.find(id);

    if (id == m_replyId)
    {
        m_requestStatus = status;
        m_replyReceived = true;
        emit replyReceived();
    }

    if (it == m_requests.end() || it.value()->status() == RequestStatus::Finished)
        return;

    switch (it.value()->type())
    {
        case RequestType::Data:
        {
            DataRequest request = qvariant_cast <DataRequest> (it.value()->data());

            if (request->debug())
                emit deviceEvent(request->device().data(), Event::requestFinished, {{"status", status}});

            if (status)
            {
                logWarning << "Device" << request->device()->name() << (!request->name().isEmpty() ? request->name().toUtf8().constData() : "data request") << "failed, status code:" << QString::asprintf("0x%02x", status);
                break;
            }

            if (!request->name().isEmpty())
                logInfo << "Device" << request->device()->name() << request->name().toUtf8().constData() << "finished successfully";


            if (request->action().isNull())
                break;

            if (!request->action()->attributes().isEmpty() && !request->device()->options().value("skipAttributeRead").toBool())
                enqueueRequest(request->device(), request->endpointId(), request->clusterId(), readAttributesRequest(m_requestId, request->manufacturerCode(), request->action()->attributes()));

            if (request->action()->propertyUpdated())
            {
                m_devices->storeProperties();
                emit endpointUpdated(request->device().data(), request->endpointId());
            }

            break;
        }

        case RequestType::Leave:
        {
            Device device = qvariant_cast <Device> (it.value()->data());

            if (status)
                logWarning << "Device" << device->name() << "leave request failed, status code:" << QString::asprintf("0x%02x", status);

            if (device->removed() || device->logicalType() == LogicalType::Coordinator)
                break;

            logInfo << "Device" << device->name() << "removed";
            emit deviceEvent(device.data(), Event::deviceRemoved);

            m_devices->removeDevice(device);
            m_devices->storeDatabase();
            break;
        }

        default:
            break;
    }

    it.value()->setStatus(RequestStatus::Finished);
}

void ZigBee::handleRequests(void)
{
    for (auto it = m_requests.begin(); it != m_requests.end(); it++)
    {
        if (it.value()->status() != RequestStatus::Pending)
            continue;

        switch (it.value()->type())
        {
            case RequestType::Data:
            {
                const DataRequest &request = qvariant_cast <DataRequest> (it.value()->data());
                const Device &device = request->device();

                m_adapter->setRequestParameters(device->ieeeAddress(), device->batteryPowered());

                if (!m_adapter->unicastRequest(it.key(), device->networkAddress(), 0x01, request->endpointId(), request->clusterId(), request->data()))
                {
                    logWarning << "Device" << request->device()->name() << (!request->name().isEmpty() ? request->name().toUtf8().constData() : "data request") << "aborted, status code:" << QString::asprintf("0x%02x", m_adapter->replyStatus());
                    it.value()->setStatus(RequestStatus::Aborted);
                }

                break;
            }

            case RequestType::Leave:
            {
                const Device &device = qvariant_cast <Device> (it.value()->data());

                m_adapter->setRequestParameters(device->ieeeAddress(), device->batteryPowered());

                if (!m_adapter->leaveRequest(it.key(), device->networkAddress()))
                {
                    logWarning << "Device" << device->name() << "leave request aborted, status code:" << QString::asprintf("0x%02x", m_adapter->replyStatus());
                    it.value()->setStatus(RequestStatus::Aborted);
                }

                break;
            }

            case RequestType::LQI:
            {
                const Device &device = qvariant_cast <Device> (it.value()->data());

                m_adapter->setRequestParameters(device->ieeeAddress(), device->batteryPowered());

                if (!m_adapter->lqiRequest(it.key(), device->networkAddress(), device->lqiRequestIndex()))
                    it.value()->setStatus(RequestStatus::Aborted);

                break;
            }

            case RequestType::Interview:
            {
                const Device &device = qvariant_cast <Device> (it.value()->data());

                m_adapter->setRequestParameters(device->ieeeAddress(), device->batteryPowered());

                if (!interviewRequest(it.key(), device))
                    it.value()->setStatus(RequestStatus::Aborted);

                break;
            }
        }

        if (it.value()->status() == RequestStatus::Finished || it.value()->status() == RequestStatus::Aborted)
            continue;

        it.value()->setStatus(RequestStatus::Sent);
    }

    for (auto it = m_requests.begin(); it != m_requests.end(); it++)
    {
        if (it.value()->status() == RequestStatus::Finished || it.value()->status() == RequestStatus::Aborted)
            m_requests.erase(it++);

        if (it == m_requests.end())
            break;
    }

    m_requestTimer->stop();
}

void ZigBee::updateNeighbors(void)
{
    for (auto it = m_devices->begin(); it != m_devices->end(); it++)
    {
        if (it.value()->removed() || !it.value()->active() || it.value()->logicalType() == LogicalType::EndDevice)
            continue;

        it.value()->setLqiRequestIndex(0);
        enqueueRequest(it.value(), RequestType::LQI);
    }
}

void ZigBee::pingDevices(void)
{
    qint64 time = QDateTime::currentSecsSinceEpoch();

    for (auto it = m_devices->begin(); it != m_devices->end(); it++)
    {
        const Device &device = it.value();

        if (it.value()->removed() || !device->active() || it.value()->batteryPowered() || time - device->lastSeen() < PING_DEVICES_INTERVAL / 1000)
            continue;

        for (auto it = device->endpoints().begin(); it != device->endpoints().end(); it++)
        {
            if (it.value()->inClusters().contains(CLUSTER_BASIC))
            {
                enqueueRequest(device, it.key(), CLUSTER_BASIC, readAttributesRequest(m_requestId, 0x0000, {0x0000}));
                break;
            }
        }
    }
}

void ZigBee::interviewTimeout(void)
{
    Device device = m_devices->value(reinterpret_cast <DeviceObject*> (sender()->parent())->ieeeAddress());

    if (device->interviewFinished())
        return;

    interviewTimeoutHandler(device);
}

void ZigBee::pollRequest(EndpointObject *endpoint, const Poll &poll)
{
    enqueueRequest(endpoint->device(), endpoint->id(), poll->clusterId(), readAttributesRequest(m_requestId, 0x0000, poll->attributes()));
}

void ZigBee::updateStatusLed(void)
{
    GPIO::setStatus(m_statusLedPin, !GPIO::getStatus(m_statusLedPin));
}

void ZigBee::updateBlinkLed(void)
{
    GPIO::setStatus(m_blinkLedPin, false);
}
