#include <QtEndian>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QThread>
#include "logger.h"
#include "zstack.h"

ZStack::ZStack(QSettings *config, QObject *parent) : Adapter(config, parent), m_restore(RestoreStatus::Unknown), m_status(0), m_clear(false)
{
    quint32 channelList = qToLittleEndian <quint32> (1 << m_channel);

    m_nvItems.insert(ZCD_NV_PRECFGKEY,         m_networkKey);
    m_nvItems.insert(ZCD_NV_PRECFGKEYS_ENABLE, QByteArray(1, 0x01));
    m_nvItems.insert(ZCD_NV_PANID,             QByteArray(reinterpret_cast <char*> (&m_panId), sizeof(m_panId)));
    m_nvItems.insert(ZCD_NV_CHANLIST,          QByteArray(reinterpret_cast <char*> (&channelList), sizeof(channelList)));
    m_nvItems.insert(ZCD_NV_LOGICAL_TYPE,      QByteArray(1, 0x00));
    m_nvItems.insert(ZCD_NV_ZDO_DIRECT_CB,     QByteArray(1, 0x01));

    m_zdoClusters = {ZDO_NODE_DESCRIPTOR_REQUEST, ZDO_SIMPLE_DESCRIPTOR_REQUEST, ZDO_ACTIVE_ENDPOINTS_REQUEST, ZDO_BIND_REQUEST, ZDO_UNBIND_REQUEST, ZDO_LQI_REQUEST, ZDO_LEAVE_REQUEST};
}

bool ZStack::unicastRequest(quint8 id, quint16 networkAddress, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    zstackDataRequestStruct request;

    request.networkAddress = qToLittleEndian(networkAddress);
    request.dstEndpointId = dstEndPointId;
    request.srcEndpointId = srcEndPointId;
    request.clusterId = qToLittleEndian(clusterId);
    request.transactionId = id;
    request.options = m_extendedTimeout ? ZSTACK_AF_DISCV_ROUTE | ZSTACK_AF_ACK_REQUEST : ZSTACK_AF_DISCV_ROUTE;
    request.radius = ZSTACK_AF_DEFAULT_RADIUS;
    request.length = static_cast <quint8> (payload.length());

    return sendRequest(ZSTACK_AF_DATA_REQUEST, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(payload)) && !m_replyStatus;
}

bool ZStack::multicastRequest(quint8 id, quint16 groupId, quint8 srcEndPointId, quint8 dstEndPointId, quint16 clusterId, const QByteArray &payload)
{
    return extendedRequest(id, groupId, dstEndPointId, 0x0000, srcEndPointId, clusterId, payload, true);
}

bool ZStack::unicastInterPanRequest(quint8 id, const QByteArray &ieeeAddress, quint16 clusterId, const QByteArray &payload)
{
    return extendedRequest(id, ieeeAddress, 0xFE, 0xFFFF, 0x0C, clusterId, payload);
}

bool ZStack::broadcastInterPanRequest(quint8 id, quint16 clusterId, const QByteArray &payload)
{
    return extendedRequest(id, 0xFFFF, 0xFE, 0xFFFF, 0x0C, clusterId, payload);
}

bool ZStack::setInterPanChannel(quint8 channel)
{
    if (!sendRequest(ZSTACK_AF_INTER_PAN_CTL, QByteArray(1, 0x01).append(static_cast <char> (channel))) || m_replyStatus)
    {
        logWarning << "Set Inter-PAN channel" << channel << "request failed";
        return false;
    }

    return true;
}

void ZStack::resetInterPanChannel(void)
{
    if (sendRequest(ZSTACK_AF_INTER_PAN_CTL, QByteArray(1, 0x00)) && !m_replyStatus)
        return;

    logWarning << "Reset Inter-PAN request failed";
}

bool ZStack::createBackup(QJsonObject &backup)
{
    zstackNetworkInfoStruct networkInfo;
    QMap <quint64, QJsonObject> map;
    QByteArray keySeed = QByteArray::fromHex(m_backup.value("keySeed").toString().toUtf8());
    QJsonArray devices = m_backup.value("devices").toArray();
    qint64 frameCounter = 0;

    if (nvItemLength(ZCD_NV_NWKKEY) == ZSTACK_NWKKEY_UNALIGNED_LENGTH)
    {
        logWarning << "Backup failed, unaligned NV memory layout is not supported";
        return false;
    }

    if (!readNetworkInfo(networkInfo))
    {
        logWarning << "Backup failed, NIB item is too short or empty";
        return false;
    }

    if (networkInfo.panId != m_panId)
    {
        logDebug(m_adapterDebug) << "Backup skipped, PAN ID" << QString::asprintf("0x%04x", networkInfo.panId) << "doesn't match configuration value";
        return false;
    }

    if (memcmp(&networkInfo.extendedPanId, m_ieeeAddress.constData(), sizeof(networkInfo.extendedPanId)))
        logWarning << "Extended PAN ID" << QByteArray(reinterpret_cast <char*> (&networkInfo.extendedPanId), sizeof(networkInfo.extendedPanId)).toHex(':') << "differs from coordinator IEEE address, backup restore may be inaccurate";

    if (keySeed.length() != 16)
        readNvItem(ZCD_NV_LEGACY_TCLK_TABLE_START, keySeed);

    for (int i = 0; i < devices.count(); i++)
    {
        QJsonObject json = devices.at(i).toObject();
        quint64 ieeeAddress;

        if (!json.contains("linkKey"))
            continue;

        memcpy(&ieeeAddress, QByteArray::fromHex(json.value("ieeeAddress").toString().toUtf8()).constData(), sizeof(ieeeAddress));
        map.insert(ieeeAddress, {{"linkKey", json.value("linkKey")}, {"txCounter", json.value("txCounter")}});
    }

    if (keySeed.length() == 16)
    {
        for (int i = 0; i < 256; i++)
        {
            zstackTcLinkKeyStruct item;
            QByteArray data;
            QJsonObject json;
            quint64 ieeeAddress;

            if (!readNvItem(ZCD_NV_EX_TCLK_TABLE, i, data, nvItemSize(ZCD_NV_EX_TCLK_TABLE)) || data.isEmpty())
                break;

            if (static_cast <size_t> (data.length()) < sizeof(zstackTcLinkKeyStruct))
                continue;

            memcpy(&item, data.constData(), sizeof(item));

            if (item.keyAttributes == 0xFF)
                continue;

            ieeeAddress = qToBigEndian(qFromLittleEndian(item.ieeeAddress));
            json = map.value(ieeeAddress);

            if (!json.contains("linkKey"))
            {
                QByteArray linkKey = keySeed.mid(item.seedShift).append(keySeed.mid(0, item.seedShift));

                for (int j = 0; j < linkKey.length(); j++)
                    linkKey[j] = linkKey.at(j) ^ reinterpret_cast <const char*> (&item.ieeeAddress)[j % sizeof(item.ieeeAddress)];

                json.insert("linkKey", QString(linkKey.toHex()));
            }

            json.insert("txCounter", QJsonValue::fromVariant(qFromLittleEndian(item.txCounter)));
            map.insert(ieeeAddress, json);
        }

        backup.insert("keySeed", QString(keySeed.toHex()));
    }

    devices = QJsonArray();

    for (int i = 0; i < 256; i++)
    {
        zstackAddressManagerStruct item;
        QByteArray data;
        QJsonObject json;

        if (!readNvItem(ZCD_NV_EX_ADDRMGR, i, data, nvItemSize(ZCD_NV_EX_ADDRMGR)) || data.isEmpty())
            break;

        if (static_cast <size_t> (data.length()) < sizeof(zstackAddressManagerStruct))
            continue;

        memcpy(&item, data.constData(), sizeof(item));

        if (!item.user || item.user == 0xFF)
            continue;

        item.networkAddress = qFromLittleEndian(item.networkAddress);
        item.ieeeAddress = qToBigEndian(qFromLittleEndian(item.ieeeAddress));

        json = map.value(item.ieeeAddress);
        json.insert("networkAddress", item.networkAddress);
        json.insert("ieeeAddress", QString(QByteArray(reinterpret_cast <char*> (&item.ieeeAddress), sizeof(item.ieeeAddress)).toHex()));
        json.insert("directChild", item.user & 0x01 ? true : false);

        devices.append(json);
    }

    for (int i = 0; i < 256; i++)
    {
        zstackSecurityMaterialStruct item;
        QByteArray data;

        if (!readNvItem(ZCD_NV_EX_NWK_SEC_MATERIAL_TABLE, i, data, nvItemSize(ZCD_NV_EX_NWK_SEC_MATERIAL_TABLE)) || data.isEmpty())
            break;

        if (static_cast <size_t> (data.length()) < sizeof(zstackSecurityMaterialStruct))
            continue;

        memcpy(&item, data.constData(), sizeof(item));
        item.frameCounter = qFromLittleEndian(item.frameCounter);
        item.extendedPanId = qToBigEndian(qFromLittleEndian(item.extendedPanId));

        if (item.extendedPanId == networkInfo.extendedPanId)
        {
            frameCounter = item.frameCounter;
            break;
        }

        if (item.extendedPanId != 0xFFFFFFFFFFFFFFFF)
            continue;

        frameCounter = item.frameCounter;
    }

    backup.insert("ieeeAddress", QString(m_ieeeAddress.toHex()));
    backup.insert("panId", m_panId);
    backup.insert("channel", m_channel);
    backup.insert("networkKey", QString(m_networkKey.toHex()));
    backup.insert("devices", devices);
    backup.insert("frameCounter", frameCounter);

    logInfo << "Backup created," << devices.count() << "devices," << map.count() << "link keys, frame counter:" << frameCounter;
    return true;
}

bool ZStack::restoreBackup(const QJsonObject &backup)
{
    zstackNetworkInfoStruct networkInfo;
    QByteArray ieeeAddress = QByteArray::fromHex(backup.value("ieeeAddress").toString().toUtf8()), keySeed = QByteArray::fromHex(backup.value("keySeed").toString().toUtf8()), keyData = QByteArray(1, 0x00).append(m_networkKey);
    QJsonArray devices = backup.value("devices").toArray();
    quint32 frameCounter = qToLittleEndian <quint32> (backup.value("frameCounter").toVariant().toLongLong() + ZSTACK_FRAME_COUNTER_MARGIN);
    bool check = false;
    int count = 0;

    if (m_version != ZStackVersion::ZStack3x0)
    {
        logWarning << "Network restore is not supported for this Z-Stack version";
        return false;
    }

    if (nvItemLength(ZCD_NV_NWKKEY) == ZSTACK_NWKKEY_UNALIGNED_LENGTH)
    {
        logWarning << "Network restore failed, unaligned NV memory layout is not supported";
        return false;
    }

    if (ieeeAddress.length() != 8)
    {
        logWarning << "Network restore failed, backup data is not valid";
        return false;
    }

    for (int i = 0; i < ZSTACK_READ_NIB_RETRIES; i++)
    {
        check = readNetworkInfo(networkInfo) && networkInfo.panId != 0xFFFF;

        if (check)
            break;

        QThread::msleep(ZSTACK_READ_NIB_RETRY_INTERVAL);
    }

    if (!check)
    {
        logWarning << "Network restore failed, temporary network not formed";
        return false;
    }

    memcpy(&networkInfo.extendedPanId, ieeeAddress.constData(), sizeof(networkInfo.extendedPanId));
    networkInfo.panId = m_panId;
    networkInfo.channel = m_channel;

    if (!writeNetworkInfo(networkInfo))
    {
        logWarning << "Network restore failed, failed to write NIB data";
        return false;
    }

    networkInfo.extendedPanId = qToLittleEndian(qFromBigEndian(networkInfo.extendedPanId));
    networkInfo.panId = qToLittleEndian(networkInfo.panId);
    ieeeAddress = QByteArray(reinterpret_cast <char*> (&networkInfo.extendedPanId), sizeof(networkInfo.extendedPanId));

    writeNvItem(ZCD_NV_EXTADDR, ieeeAddress);
    writeNvItem(ZCD_NV_STARTUP_OPTION, QByteArray(1, 0x00));
    writeNvItem(ZCD_NV_EXTENDED_PAN_ID, ieeeAddress);
    writeNvItem(ZCD_NV_NWK_ACTIVE_KEY_INFO, keyData);
    writeNvItem(ZCD_NV_NWK_ALTERN_KEY_INFO, keyData);
    writeNvItem(ZCD_NV_APS_USE_EXT_PANID, ieeeAddress);
    writeNvItem(ZCD_NV_PANID, QByteArray(reinterpret_cast <char*> (&networkInfo.panId), sizeof(networkInfo.panId)));

    if (keySeed.length() == 16)
        writeNvItem(ZCD_NV_LEGACY_TCLK_TABLE_START, keySeed);

    writeNvItem(ZCD_NV_EX_NWK_SEC_MATERIAL_TABLE, 0, QByteArray(reinterpret_cast <char*> (&frameCounter), sizeof(frameCounter)).append(ieeeAddress));
    writeNvItem(ZCD_NV_EX_NWK_SEC_MATERIAL_TABLE, 1, QByteArray(reinterpret_cast <char*> (&frameCounter), sizeof(frameCounter)).append(8, 0xFF));

    for (int i = 0; i < devices.count(); i++)
    {
        zstackAddressManagerStruct addressItem;
        zstackTcLinkKeyStruct keyItem;
        QJsonObject device = devices.at(i).toObject();
        QByteArray ieeeAddress = QByteArray::fromHex(device.value("ieeeAddress").toString().toUtf8()), linkKey = QByteArray::fromHex(device.value("linkKey").toString().toUtf8());
        int shift = -1;

        if (ieeeAddress.length() != 8)
            continue;

        memcpy(&addressItem.ieeeAddress, ieeeAddress.constData(), sizeof(addressItem.ieeeAddress));

        addressItem.user = device.value("directChild").toBool() ? 0x03 : 0x02;
        addressItem.padding = 0xFF;
        addressItem.networkAddress = qToLittleEndian <quint16> (device.value("networkAddress").toInt());
        addressItem.ieeeAddress = qToLittleEndian(qFromBigEndian(addressItem.ieeeAddress));

        if (addressItem.networkAddress)
            writeNvItem(ZCD_NV_EX_ADDRMGR, i, QByteArray(reinterpret_cast <char*> (&addressItem), sizeof(addressItem)));

        if (linkKey.length() != 16 || keySeed.length() != 16)
            continue;

        for (int j = 0; j < linkKey.length(); j++)
            linkKey[j] = linkKey.at(j) ^ reinterpret_cast <const char*> (&addressItem.ieeeAddress)[j % sizeof(addressItem.ieeeAddress)];

        for (int j = 0; j < linkKey.length(); j++)
        {
            if (QByteArray(keySeed).mid(j).append(keySeed.mid(0, j)) != linkKey)
                continue;

            shift = j;
            break;
        }

        if (shift < 0)
        {
            logWarning << "Device" << ieeeAddress.toHex(':') << "link key is not derived from the seed and can't be restored";
            continue;
        }

        keyItem.txCounter = qToLittleEndian <quint32> (device.value("txCounter").toVariant().toULongLong() + ZSTACK_FRAME_COUNTER_MARGIN);
        keyItem.rxCounter = 0x00000000;
        keyItem.ieeeAddress = addressItem.ieeeAddress;
        keyItem.keyAttributes = 0x02;
        keyItem.keyType = 0x00;
        keyItem.seedShift = static_cast <quint8> (shift);
        keyItem.padding = 0x00;

        writeNvItem(ZCD_NV_EX_TCLK_TABLE, count++, QByteArray(reinterpret_cast <char*> (&keyItem), sizeof(keyItem)));
    }

    logInfo << "Network restored," << devices.count() << "devices," << count << "link keys, frame counter:" << frameCounter;
    return true;
}

bool ZStack::extendedRequest(quint8 id, const QByteArray &address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &payload, bool group)
{
    zstackExtendedRequestStruct data;

    switch (address.length())
    {
        case 2:  data.dstAddressMode = group ? ADDRESS_MODE_GROUP : ADDRESS_MODE_16_BIT; break;
        case 8:  data.dstAddressMode = ADDRESS_MODE_64_BIT; break;
        default: return false;
    }

    memset(&data.dstAddress, 0, sizeof(data.dstAddress));
    memcpy(&data.dstAddress, address.constData(), address.length());

    if (data.dstAddressMode == ADDRESS_MODE_64_BIT)
        data.dstAddress = qToLittleEndian(qFromBigEndian(data.dstAddress));

    data.dstEndpointId = dstEndpointId;
    data.dstPanId = qToLittleEndian(dstPanId);
    data.srcEndpointId = srcEndpointId;
    data.clusterId = qToLittleEndian(clusterId);
    data.transactionId = id;
    data.options = 0x00;
    data.radius = dstPanId ? ZSTACK_AF_DEFAULT_RADIUS * 2 : ZSTACK_AF_DEFAULT_RADIUS;
    data.length = qToLittleEndian <quint16> (payload.length());

    return sendRequest(ZSTACK_AF_DATA_REQUEST_EXT, QByteArray(reinterpret_cast <char*> (&data), sizeof(data)).append(payload)) && !m_replyStatus;
}

bool ZStack::extendedRequest(quint8 id, quint16 address, quint8 dstEndpointId, quint16 dstPanId, quint8 srcEndpointId, quint16 clusterId, const QByteArray &payload, bool group)
{
    address = qToLittleEndian(address);
    return extendedRequest(id, QByteArray(reinterpret_cast <char*> (&address), sizeof(address)), dstEndpointId, dstPanId, srcEndpointId, clusterId, payload, group);
}

bool ZStack::sendRequest(quint16 command, const QByteArray &data)
{
    QByteArray request;
    quint8 fcs = 0;

    logDebug(m_adapterDebug) << "-->" << QString::asprintf("0x%04x", command) << data.toHex(':');

    m_command = qToBigEndian(command);
    m_replyStatus = 0xFF;

    request.append(ZSTACK_PACKET_FLAG);
    request.append(static_cast <char> (data.length()));
    request.append(reinterpret_cast <char*> (&m_command), sizeof(m_command));
    request.append(data);

    for (int i = 1; i < request.length(); i++)
        fcs ^= request[i];

    sendData(request.append(static_cast <char> (fcs)));
    return waitForSignal(this, SIGNAL(dataReceived()), ZSTACK_REQUEST_TIMEOUT);
}

void ZStack::parsePacket(quint16 command, const QByteArray &data)
{
    logDebug(m_adapterDebug) << "<--" << QString::asprintf("0x%04x", command) << data.toHex(':');

    if (command & 0x2000)
    {
        if ((command ^ 0x4000) == qFromBigEndian(m_command))
        {
            m_replyStatus = static_cast <quint8> (data.at(0));
            m_replyData = data;
            emit dataReceived();
        }

        return;
    }

    switch (command)
    {
        case ZSTACK_ZDO_MGMT_PERMIT_JOIN_RSP:
        case ZSTACK_ZDO_MGMT_NWK_UPDATE_RSP:
        case ZSTACK_ZDO_SRC_RTG_IND:
        case ZSTACK_ZDO_CONCENTRATOR_IND:
        case ZSTACK_ZDO_TC_DEV_IND:
        case ZSTACK_ZDO_PERMIT_JOIN_IND:
            break;

        case ZSTACK_SYS_RESET_IND:
        {
            if (!startCoordinator())
            {
                logWarning << "Coordinator startup failed";
                break;
            }

            m_resetTimer->stop();
            break;
        }

        case ZSTACK_AF_DATA_CONFIRM:
        {
            const zstackDataConfirmStruct *message = reinterpret_cast <const zstackDataConfirmStruct*> (data.constData());
            emit requestFinished(message->transactionId, message->status);
            break;
        }

        case ZSTACK_AF_INCOMING_MSG:
        {
            const zstackIncomingMessageStruct *message = reinterpret_cast <const zstackIncomingMessageStruct*> (data.constData());
            emit zclMessageReveived(qFromLittleEndian(message->srcAddress), message->srcEndpointId, qFromLittleEndian(message->clusterId), message->linkQuality, data.mid(sizeof(zstackIncomingMessageStruct), message->length));
            break;
        }

        case ZSTACK_AF_INCOMING_MSG_EXT:
        {
            const zstackExtendedMessageStruct *message = reinterpret_cast <const zstackExtendedMessageStruct*> (data.constData());
            quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->srcAddress));

            if (message->srcAddressMode != 0x03)
            {
                logWarning << "Unsupported extended message address mode" << QString::asprintf("0x%02x", message->srcAddressMode);
                return;
            }

            emit rawMessageReveived(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->clusterId), message->linkQuality, data.mid(sizeof(zstackExtendedMessageStruct), message->length));
            break;
        }

        case ZSTACK_ZDO_STATE_CHANGE_IND:
        {
            if (data.length() == 1)
                m_status = static_cast <quint8> (data.at(0));

            if (m_version == ZStackVersion::ZStack12x && m_status == ZSTACK_COORDINATOR_STARTED)
            {
                m_ready = true;
                emit coordinatorReady();
            }

            break;
        }

        case ZSTACK_ZDO_END_DEVICE_ANNCE_IND:
        {
            const deviceAnnounceStruct *message = reinterpret_cast <const deviceAnnounceStruct*> (data.constData() + 2);
            quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->ieeeAddress));
            emit deviceJoined(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)), qFromLittleEndian(message->networkAddress));
            break;
        }

        case ZSTACK_ZDO_LEAVE_IND:
        {
            const zstackDeviceLeaveStruct *message = reinterpret_cast <const zstackDeviceLeaveStruct*> (data.constData());
            quint64 ieeeAddress = qToBigEndian(qFromLittleEndian(message->ieeeAddress));
            emit deviceLeft(QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress)));
            break;
        }

        case ZSTACK_ZDO_MSG_CB_INCOMING:
        {
            const zstackZdoMessageStruct *message = reinterpret_cast <const zstackZdoMessageStruct*> (data.constData());
            QByteArray payload = data.mid(sizeof(zstackZdoMessageStruct));
            emit requestFinished(message->transactionId, static_cast <quint8> (payload.at(0)));
            emit zdoMessageReveived(qFromLittleEndian(message->srcAddress), qFromLittleEndian(message->clusterId), payload);
            break;
        }

        case ZSTACK_APP_CNF_BDB_COMMISSIONING:
        {
            if (data.at(2))
                break;

            switch (m_status)
            {
                case ZSTACK_NOT_STARTED_AUTOMATICALLY:
                    logWarning << "Network not started, check for PAN ID collision or adapter configuration";
                    break;

                case ZSTACK_COORDINATOR_STARTED:

                    if (m_restore == RestoreStatus::Running || m_ready)
                        break;

                    if (m_restore == RestoreStatus::Pending)
                    {
                        m_restore = RestoreStatus::Running;

                        if (restoreBackup(m_backup))
                        {
                            logInfo << "Network restored from backup, restarting adapter...";
                            reset();
                        }
                        else
                            logWarning << "Network restore failed";

                        m_restore = RestoreStatus::Unknown;
                        break;
                    }

                    m_ready = true;
                    emit coordinatorReady();
                    break;
            };

            break;
        }

        default:
        {
            logDebug(m_adapterDebug) << "Unrecognized Z-Stack command" << QString::asprintf("0x%04x", command) << "with data" << (data.isEmpty() ? "(empty)" : data.toHex(':'));
            break;
        }
    }
}

int ZStack::nvItemLength(quint16 id)
{
    quint16 value = qToLittleEndian(id);

    if (!sendRequest(ZSTACK_SYS_OSAL_NV_LENGTH, QByteArray(reinterpret_cast <char*> (&value), sizeof(value))) || static_cast <size_t> (m_replyData.length()) < sizeof(value))
        return -1;

    memcpy(&value, m_replyData.constData(), sizeof(value));
    return qFromLittleEndian(value);
}

int ZStack::nvItemLength(quint16 id, quint16 subId)
{
    zstackNvLengthStruct request;

    request.system = ZSTACK_NVSYS_ZSTACK;
    request.id = qToLittleEndian(id);
    request.subId = qToLittleEndian(subId);

    if (!sendRequest(ZSTACK_SYS_NV_LENGTH, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || m_replyData.isEmpty())
        return -1;

    return static_cast <quint8> (m_replyData.at(0));
}

int ZStack::nvItemSize(quint16 id)
{
    int length = m_nvItemSize.value(id, 0);

    if (!length)
    {
        length = nvItemLength(id, 0);

        if (length > 0)
            m_nvItemSize.insert(id, static_cast <quint8> (length));
    }

    return length;
}

bool ZStack::readNvItem(quint16 id, QByteArray &data)
{
    int length = nvItemLength(id);

    if (length < 0)
        return false;

    data.clear();

    while (data.length() < length)
    {
        zstackNvReadExtendedStruct request;
        zstackNvReplyStruct *reply;

        request.id = qToLittleEndian(id);
        request.offset = qToLittleEndian <quint16> (data.length());

        if (!sendRequest(ZSTACK_SYS_OSAL_NV_READ_EXT, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || static_cast <size_t> (m_replyData.length()) < sizeof(zstackNvReplyStruct) || m_replyData.at(0))
            return false;

        reply = reinterpret_cast <zstackNvReplyStruct*> (m_replyData.data());

        if (!reply->length)
            break;

        data.append(m_replyData.mid(sizeof(zstackNvReplyStruct), reply->length));
    }

    return true;
}

bool ZStack::readNvItem(quint16 id, quint16 subId, QByteArray &data, int length)
{
    zstackNvItemStruct request;
    zstackNvReplyStruct *reply;

    data.clear();

    if (!length)
        length = nvItemLength(id, subId);

    if (length <= 0)
        return length ? false : true;

    request.system = ZSTACK_NVSYS_ZSTACK;
    request.id = qToLittleEndian(id);
    request.subId = qToLittleEndian(subId);
    request.offset = 0x0000;
    request.length = static_cast <quint8> (length);

    if (!sendRequest(ZSTACK_SYS_NV_READ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || static_cast <size_t> (m_replyData.length()) < sizeof(zstackNvReplyStruct) || m_replyData.at(0))
        return false;

    reply = reinterpret_cast <zstackNvReplyStruct*> (m_replyData.data());
    data = m_replyData.mid(sizeof(zstackNvReplyStruct), reply->length);
    return true;
}

bool ZStack::writeNvItem(quint16 id, const QByteArray &data)
{
    zstackNvWriteStruct request;

    if (nvItemLength(id) != data.length())
    {
        zstackNvInitStruct init;
        quint8 count = static_cast <quint8> (data.length() > 240 ? 240 : data.length());

        init.id = qToLittleEndian(id);
        init.length = qToLittleEndian <quint16> (data.length());
        init.count = count;

        sendRequest(ZSTACK_SYS_OSAL_NV_ITEM_INIT, QByteArray(reinterpret_cast <char*> (&init), sizeof(init)).append(data.mid(0, count)));
    }

    request.id = qToLittleEndian(id);
    request.offset = 0x00;
    request.length = static_cast <quint8> (data.length());

    if (!sendRequest(ZSTACK_SYS_OSAL_NV_WRITE, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyStatus)
    {
        logWarning << "NV item" << QString::asprintf("0x%04x", id) << "write request failed";
        return false;
    }

    return true;
}

bool ZStack::writeNvItem(quint16 id, quint16 subId, const QByteArray &data)
{
    zstackNvItemStruct request;

    request.system = ZSTACK_NVSYS_ZSTACK;
    request.id = qToLittleEndian(id);
    request.subId = qToLittleEndian(subId);
    request.offset = 0x0000;
    request.length = static_cast <quint8> (data.length());

    if (!sendRequest(ZSTACK_SYS_NV_WRITE, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyStatus)
    {
        logWarning << "NV item" << QString::asprintf("0x%04x:%d", id, subId) << "write request failed";
        return false;
    }

    return true;
}

bool ZStack::writeConfig(quint16 id, const QByteArray &data)
{
    zstackWriteConfigurationStruct request;

    request.id = id;
    request.length = static_cast <quint8> (data.length());

    if (!sendRequest(ZSTACK_ZB_WRITE_CONFIGURATION, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyStatus)
    {
        logWarning << "NV item" << QString::asprintf("0x%04x", id) << "write request failed";
        return false;
    }

    return true;
}

bool ZStack::readNetworkInfo(zstackNetworkInfoStruct &info)
{
    if (!readNvItem(ZCD_NV_NIB, m_nibData) || m_nibData.length() < ZSTACK_NIB_LENGTH)
        return false;

    memcpy(&info.panId, m_nibData.constData() + 36, sizeof(info.panId));
    memcpy(&info.extendedPanId, m_nibData.constData() + 57, sizeof(info.extendedPanId));

    info.extendedPanId = qToBigEndian(qFromLittleEndian(info.extendedPanId));
    info.panId = qFromLittleEndian(info.panId);
    info.channel = static_cast <quint8> (m_nibData.at(24));
    return true;
}

bool ZStack::writeNetworkInfo(const zstackNetworkInfoStruct &info)
{
    quint64 extendedPanId = qToLittleEndian(qFromBigEndian(info.extendedPanId));
    quint16 panId = qToLittleEndian(info.panId);
    quint32 channelList = qToLittleEndian <quint32> (1 << info.channel);

    if (m_nibData.length() < ZSTACK_NIB_LENGTH)
        return false;

    m_nibData.replace(24, sizeof(info.channel), reinterpret_cast <const char*> (&info.channel), sizeof(info.channel));
    m_nibData.replace(36, sizeof(panId), reinterpret_cast <char*> (&panId), sizeof(panId));
    m_nibData.replace(57, sizeof(extendedPanId), reinterpret_cast <char*> (&extendedPanId), sizeof(extendedPanId));
    m_nibData.replace(40, sizeof(channelList), reinterpret_cast <char*> (&channelList), sizeof(channelList));
    return writeNvItem(ZCD_NV_NIB, m_nibData);
}

bool ZStack::startCommissioning(void)
{
    if (!m_write)
    {
        logWarning << "Adapter configuration can't be changed, write protection enabled";
        return false;
    }

    if (!m_backup.isEmpty())
    {
        if (m_backup.value("panId").toInt() != m_panId || m_backup.value("channel").toInt() != m_channel || QByteArray::fromHex(m_backup.value("networkKey").toString().toUtf8()) != m_networkKey)
        {
            logWarning << "Backup data doesn't match configuration, startup aborted to protect existing network, update configuration values to match backup, or remove backup file to start new network";
            return false;
        }

        logInfo << "Network will be restored from backup";
        m_restore = RestoreStatus::Pending;
    }

    writeNvItem(ZCD_NV_STARTUP_OPTION, QByteArray(1, 0x03));
    m_clear = true;
    reset();

    return true;
}

bool ZStack::startCoordinator(void)
{
    zstackVersionStruct version;

    if (!sendRequest(ZSTACK_SYS_VERSION))
    {
        logWarning << "Adapter version request failed";
        return false;
    }

    memcpy(&version, m_replyData.constData(), sizeof(version));

    switch (version.product)
    {
        case 0x01:
            m_modelName = "Z-Stack 3.x.0";
            m_version = ZStackVersion::ZStack3x0;
            break;

        case 0x02:
            m_modelName = "Z-Stack 3.0.x";
            m_version = ZStackVersion::ZStack30x;
            break;

        default:
            m_modelName = "Z-Stack 1.2.x";
            m_version = ZStackVersion::ZStack12x;
            break;
    }

    if (!m_clear)
    {
        quint64 ieeeAddress;

        m_manufacturerName = "Texas Instruments";
        m_firmware = QString::number(qFromLittleEndian(version.build));
        logInfo << QString("Adapter type: %1 (%2)").arg(m_modelName, m_firmware).toUtf8().constData();

        if (!sendRequest(ZSTACK_UTIL_GET_DEVICE_INFO) || m_replyStatus)
        {
            logWarning << "Device information request failed";
            return false;
        }

        memcpy(&ieeeAddress, m_replyData.constData() + 1, sizeof(ieeeAddress));
        ieeeAddress = qToBigEndian(qFromLittleEndian(ieeeAddress));
        m_ieeeAddress = QByteArray(reinterpret_cast <char*> (&ieeeAddress), sizeof(ieeeAddress));

        if (m_version == ZStackVersion::ZStack3x0)
        {
            zstackNetworkInfoStruct info;

            if (!readNetworkInfo(info) || info.panId == 0xFFFF)
            {
                logWarning << "Adapter has no existing network";
                return startCommissioning();
            }
        }

        for (auto it = m_nvItems.begin(); it != m_nvItems.end(); it++)
        {
            QByteArray data;
            quint8 status;

            if (m_version != ZStackVersion::ZStack12x || it.key() != ZCD_NV_PRECFGKEY)
            {
                zstackNvReadStruct request;
                zstackNvReplyStruct *reply;

                request.id = qToLittleEndian(it.key());
                request.offset = 0x00;

                if (!sendRequest(ZSTACK_SYS_OSAL_NV_READ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))))
                {
                    logWarning << "NV item" << QString::asprintf("0x%04x", it.key()) << "read request failed";
                    return false;
                }

                reply = reinterpret_cast <zstackNvReplyStruct*> (m_replyData.data());
                data = m_replyData.mid(sizeof(zstackNvReplyStruct), reply->length);
                status = reply->status;
            }
            else
            {
                zstackReadConfigurationStruct *reply;

                if (!sendRequest(ZSTACK_ZB_READ_CONFIGURATION, QByteArray(1, static_cast <char> (it.key()))))
                {
                    logWarning << "NV item" << QString::asprintf("0x%04x", it.key()) << "read request failed";
                    return false;
                }

                reply = reinterpret_cast <zstackReadConfigurationStruct*> (m_replyData.data());
                data = m_replyData.mid(sizeof(zstackReadConfigurationStruct), reply->length);
                status = reply->status;
            }

            if (status || data != it.value())
            {
                logWarning << "Adapter network parameters don't match configuration";
                return startCommissioning();
            }
        }
    }
    else
    {
        quint16 panId = static_cast <quint16> (QRandomGenerator::global()->bounded(1, 0xFFFE));

        logInfo << "Starting" << (m_restore == RestoreStatus::Pending ? "temporary network for restore..." : "new network...");
        m_clear = false;

        for (auto it = m_nvItems.begin(); it != m_nvItems.end(); it++)
        {
            QByteArray value = it.value();

            if (m_restore == RestoreStatus::Pending && it.key() == ZCD_NV_PANID)
                value = QByteArray(reinterpret_cast <char*> (&panId), sizeof(panId));

            if (m_version != ZStackVersion::ZStack12x || it.key() != ZCD_NV_PRECFGKEY)
            {
                if (!writeNvItem(it.key(), value))
                    return false;
            }
            else
            {
                if (!writeConfig(it.key(), value) || !writeNvItem(ZCD_NV_LEGACY_TCLK_TABLE_START, QByteArray(8, 0xFF).append(m_defaultKey).append(8, 0x00)))
                    return false;
            }

            logDebug(m_adapterDebug) << "NV item" << QString::asprintf("0x%04x", it.key()) << "value set to" << value.toHex(':');
        }
    }

    if (m_version != ZStackVersion::ZStack12x)
    {
        zstackSetChannelStruct channelRequest;

        channelRequest.isPrimary = 0x01;
        channelRequest.channel = qToLittleEndian <quint32> (1 << m_channel);

        if (!sendRequest(ZSTACK_APP_CNF_BDB_SET_CHANNEL, QByteArray(reinterpret_cast <char*> (&channelRequest), sizeof(channelRequest))) || m_replyStatus)
        {
            logWarning << "Set primary channel request failed";
            return false;
        }

        channelRequest.isPrimary = 0x00;
        channelRequest.channel = 0x00;

        if (!sendRequest(ZSTACK_APP_CNF_BDB_SET_CHANNEL, QByteArray(reinterpret_cast <char*> (&channelRequest), sizeof(channelRequest))) || m_replyStatus)
        {
            logWarning << "Set secondary channel request failed";
            return false;
        }
    }

    for (int i = 0; i < m_zdoClusters.count(); i++)
    {
        quint16 request = qToLittleEndian(m_zdoClusters.at(i) | 0x8000);

        if (!sendRequest(ZSTACK_ZDO_MSG_CB_REGISTER, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))))
        {
            logWarning << "ZDO cluster" << QString::asprintf("0x%04x", m_zdoClusters.at(i)) << "callback register request failed";
            return false;
        }
    }

    for (auto it = m_endpoints.begin(); it != m_endpoints.end(); it++)
    {
        zstackRegisterEndpointStruct request;
        QByteArray data;

        request.endpointId = it.key();
        request.profileId = qToLittleEndian(it.value()->profileId());
        request.deviceId = qToLittleEndian(it.value()->deviceId());
        request.version = 0x00;
        request.latency = 0x00;

        data.append(static_cast <char> (it.value()->inClusters().count()));

        for (int i = 0; i < it.value()->inClusters().count(); i++)
        {
            quint16 clusterId = qToLittleEndian(it.value()->inClusters().at(i));
            data.append(reinterpret_cast <char*> (&clusterId), sizeof(clusterId));
        }

        data.append(static_cast <char> (it.value()->outClusters().count()));

        for (int i = 0; i < it.value()->outClusters().count(); i++)
        {
            quint16 clusterId = qToLittleEndian(it.value()->outClusters().at(i));
            data.append(reinterpret_cast <char*> (&clusterId), sizeof(clusterId));
        }

        if (!sendRequest(ZSTACK_AF_REGISTER, QByteArray(reinterpret_cast <char*> (&request), sizeof(request)).append(data)) || m_replyStatus)
        {
            logWarning << "Endpoint" << QString::asprintf("0x%02x", it.key()) << "register request failed";
            continue;
        }

        logInfo << "Endpoint" << QString::asprintf("0x%02x", it.key()) << "registered successfully";
    }

    if (!sendRequest(ZSTACK_AF_INTER_PAN_CTL, QByteArray(1, 0x02).append(0x0C)) || m_replyStatus)
    {
        logWarning << "Set Inter-PAN endpoint request failed";
        return false;
    }

    if (!sendRequest(ZSTACK_SYS_SET_TX_POWER, QByteArray(1, static_cast <char> (m_power))) || m_replyStatus)
        logWarning << "Set TX power request failed";

    if (!sendRequest(ZSTACK_ZDO_STARTUP_FROM_APP, QByteArray(2, 0x00)) || m_replyStatus == 0x02)
    {
        if (m_version == ZStackVersion::ZStack12x && m_status == ZSTACK_COORDINATOR_STARTED)
            return true;

        logWarning << "Startup request failed";
        return false;
    }

    return true;
}

void ZStack::softReset(void)
{
    sendData(QByteArray(1, ZSTACK_SKIP_BOOTLOADER));
    QThread::msleep(RESET_DELAY);
    sendRequest(ZSTACK_SYS_RESET_REQ, QByteArray(1, 0x01));
}

void ZStack::parseData(void)
{
    while (!m_buffer.isEmpty())
    {
        quint8 length, fcs = 0;

        if (!m_buffer.at(0))
            m_buffer.remove(0, 1);

        if (!m_buffer.length() || m_buffer.at(0) != static_cast <char> (ZSTACK_PACKET_FLAG) || m_buffer.length() < 5) // TODO: use offset
        {
            m_buffer.clear();
            return;
        }

        length = static_cast <quint8> (m_buffer.at(1));

        if (m_buffer.length() < length + 5)
            break;

        logDebug(m_portDebug) << "Frame received:" << m_buffer.mid(0, length + 5).toHex(':');

        for (quint8 i = 1; i < length + 4; i++)
            fcs ^= m_buffer.at(i);

        if (fcs != static_cast <quint8> (m_buffer.at(length + 4)))
        {
            logWarning << "Frame" << m_buffer.mid(0, length + 5).toHex(':') << "FCS mismatch";
            m_buffer.clear();
            return;
        }

        m_queue.enqueue(m_buffer.mid(2, length + 2));
        m_buffer.remove(0, length + 5);
    }
}

bool ZStack::permitJoin(bool enabled)
{
    zstackPermitJoinStruct request;

    request.mode = enabled && m_permitJoinAddress != PERMIT_JOIN_BROARCAST_ADDRESS ? 0x02 : 0x0F;
    request.dstAddress = qToLittleEndian <quint16> (enabled ? m_permitJoinAddress : PERMIT_JOIN_BROARCAST_ADDRESS);
    request.duration = enabled ? 0xF0 : 0x00;
    request.significance = 0x00;

    if (!sendRequest(ZSTACK_ZDO_MGMT_PERMIT_JOIN_REQ, QByteArray(reinterpret_cast <char*> (&request), sizeof(request))) || m_replyStatus)
    {
        logWarning << "Set permit join request failed";
        return false;
    }

    return true;
}

void ZStack::handleQueue(void)
{
    while (!m_queue.isEmpty())
    {
        QByteArray packet = m_queue.dequeue();
        quint16 command;
        memcpy(&command, packet.constData(), sizeof(command));
        parsePacket(qFromBigEndian(command), packet.mid(2));
    }
}
