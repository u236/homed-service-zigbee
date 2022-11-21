#ifndef ACTION_H
#define ACTION_H

#include <QSharedPointer>
#include <QVariant>
#include "zcl.h"

class ActionObject;
typedef QSharedPointer <ActionObject> Action;

class ActionObject
{

public:

    ActionObject(const QString &name, quint16 clusterId, quint16 manufacturerCode = 0, quint16 attributeId = 0, quint8 dataType = 0, bool poll = false) :
        m_name(name), m_clusterId(clusterId), m_manufacturerCode(manufacturerCode), m_attributeId(attributeId), m_dataType(dataType), m_poll(poll), m_transactionId(0) {}

    virtual ~ActionObject(void) {}
    virtual QByteArray request(const QVariant &data) = 0;

    inline QString name(void) { return m_name; }
    inline quint16 clusterId(void) { return m_clusterId; }
    inline quint16 manufacturerCode(void) { return m_manufacturerCode; }
    inline quint16 attributeId(void) { return m_attributeId; }
    inline quint8 dataType(void) { return m_dataType; }
    inline bool poll(void) { return m_poll; }

    static void registerMetaTypes(void);

protected:

    QString m_name;
    quint16 m_clusterId, m_manufacturerCode, m_attributeId;
    quint8 m_dataType;
    bool m_poll;

    quint8 m_transactionId;

    QByteArray writeAttributeRequest(const QByteArray &data); // TODO: move this to zcl

};

namespace Actions
{
    class Status : public ActionObject
    {

    public:

        Status(void) : ActionObject("status", CLUSTER_ON_OFF) {}
        QByteArray request(const QVariant &data) override;

    };

    class PowerOnStatus : public ActionObject
    {

    public:

        PowerOnStatus(void) : ActionObject("powerOnStatus", CLUSTER_ON_OFF, 0x0000, 0x4003, DATA_TYPE_8BIT_ENUM, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class Level : public ActionObject
    {

    public:

        Level(void) : ActionObject("level", CLUSTER_LEVEL_CONTROL) {}
        QByteArray request(const QVariant &data) override;

    };

    class ColorHS : public ActionObject
    {

    public:

        ColorHS(void) : ActionObject("colorHS", CLUSTER_COLOR_CONTROL) {}
        QByteArray request(const QVariant &data) override;

    };

    class ColorXY : public ActionObject
    {

    public:

        ColorXY(void) : ActionObject("colorXY", CLUSTER_COLOR_CONTROL) {}
        QByteArray request(const QVariant &data) override;

    };

    class ColorTemperature : public ActionObject
    {

    public:

        ColorTemperature(void) : ActionObject("colorTemperature", CLUSTER_COLOR_CONTROL) {}
        QByteArray request(const QVariant &data) override;

    };
}

namespace ActionsPTVO
{
    class ChangePattern : public ActionObject
    {

    public:

        ChangePattern(void) : ActionObject("changePattern", CLUSTER_ON_OFF) {}
        QByteArray request(const QVariant &data) override;

    };

    class Pattern : public ActionObject
    {

    public:

        Pattern(void) : ActionObject("pattern", CLUSTER_ANALOG_INPUT, 0x0000, 0x0055, DATA_TYPE_SINGLE_PRECISION, true) {}
        QByteArray request(const QVariant &data) override;

    };
}

namespace ActionsLUMI
{
    class OperationMode : public ActionObject
    {

    public:

        OperationMode(void) : ActionObject("mode", CLUSTER_LUMI, 0x115F, 0x0009, DATA_TYPE_8BIT_UNSIGNED, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class Sensitivity : public ActionObject
    {

    public:

        Sensitivity(void) : ActionObject("sensitivity", CLUSTER_LUMI, 0x115F, 0x010C, DATA_TYPE_8BIT_UNSIGNED, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class DetectionMode : public ActionObject
    {

    public:

        DetectionMode(void) : ActionObject("mode", CLUSTER_LUMI, 0x115F, 0x0144, DATA_TYPE_8BIT_UNSIGNED, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class Distance : public ActionObject
    {

    public:

        Distance(void) : ActionObject("distance", CLUSTER_LUMI, 0x115F, 0x0146, DATA_TYPE_8BIT_UNSIGNED, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class ResetPresence : public ActionObject
    {

    public:

        ResetPresence(void) : ActionObject("resetPresence", CLUSTER_LUMI, 0x115F, 0x0157, DATA_TYPE_8BIT_UNSIGNED) {}
        QByteArray request(const QVariant &data) override;

    };
}

namespace ActionsTUYA
{
    class Request
    {

    protected:

        QByteArray makeRequest(quint8 transactionId, quint8 dataPoint, quint8 dataType, void *data);

    };

    class Volume : public Request, public ActionObject
    {

    public:

        Volume(void) : ActionObject("volume", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class Duration : public Request, public ActionObject
    {

    public:

        Duration(void) : ActionObject("duration", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class Alarm : public Request, public ActionObject
    {

    public:

        Alarm(void) : ActionObject("alarm", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class Melody : public Request, public ActionObject
    {

    public:

        Melody(void) : ActionObject("melody", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class Sensitivity : public Request, public ActionObject
    {

    public:

        Sensitivity(void) : ActionObject("sensitivity", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class DistanceMin :  public Request, public ActionObject
    {

    public:

        DistanceMin(void) : ActionObject("distanceMin", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class DistanceMax :  public Request, public ActionObject
    {

    public:

        DistanceMax(void) : ActionObject("distanceMax", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class DetectionDelay : public Request, public ActionObject
    {

    public:

        DetectionDelay(void) : ActionObject("detectionDelay", CLUSTER_TUYA) {}
        QByteArray request(const QVariant &data) override;

    };

    class PowerOnStatus : public ActionObject
    {

    public:

        PowerOnStatus(void) : ActionObject("powerOnStatus", CLUSTER_ON_OFF, 0x0000, 0x8002, DATA_TYPE_8BIT_ENUM, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class SwitchMode : public ActionObject
    {

    public:

        SwitchMode(void) : ActionObject("mode", CLUSTER_TUYA_SWITCH_MODE, 0x0000, 0xD030, DATA_TYPE_8BIT_ENUM, true) {}
        QByteArray request(const QVariant &data) override;

    };
}

namespace ActionsPerenio
{
    class PowerOnStatus : public ActionObject
    {

    public:

        PowerOnStatus(void) : ActionObject("powerOnStatus", CLUSTER_PERENIO, 0x0000, 0x0000, DATA_TYPE_8BIT_UNSIGNED, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class ResetAlarms : public ActionObject
    {

    public:

        ResetAlarms(void) : ActionObject("resetAlarms", CLUSTER_PERENIO, 0x0000, 0x0001, DATA_TYPE_8BIT_UNSIGNED, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class AlarmVoltageMin : public ActionObject
    {

    public:

        AlarmVoltageMin(void) : ActionObject("alarmVoltageMin", CLUSTER_PERENIO, 0x0000, 0x0004, DATA_TYPE_16BIT_UNSIGNED, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class AlarmVoltageMax : public ActionObject
    {

    public:

        AlarmVoltageMax(void) : ActionObject("alarmVoltageMax", CLUSTER_PERENIO, 0x0000, 0x0005, DATA_TYPE_16BIT_UNSIGNED, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class AlarmPowerMax : public ActionObject
    {

    public:

        AlarmPowerMax(void) : ActionObject("alarmPowerMax", CLUSTER_PERENIO, 0x0000, 0x000B, DATA_TYPE_16BIT_UNSIGNED, true) {}
        QByteArray request(const QVariant &data) override;

    };

    class AlarmEnergyLimit : public ActionObject
    {

    public:

        AlarmEnergyLimit(void) : ActionObject("alarmEnergyLimit", CLUSTER_PERENIO, 0x0000, 0x000F, DATA_TYPE_16BIT_UNSIGNED, true) {}
        QByteArray request(const QVariant &data) override;

    };
}

#endif
