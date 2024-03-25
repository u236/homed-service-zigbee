#include <QtEndian>
#include "ias.h"

QByteArray ActionsIAS::Warning::request(const QString &name, const QVariant &data)
{
    const Property &property = endpointProperty();
    QMap <QString, QVariant> map = property->value().toMap();
    int sirenMode, sirenLevel, strobeLevel, strobe;
    iasStartWarningStruct payload;

    if (!map.contains("sirenMode"))
        map.insert("sirenMode", "stop");

    if (!map.contains("sirenLevel"))
        map.insert("sirenLevel", "low");

    if (!map.contains("strobe"))
        map.insert("strobe", false);

    if (!map.contains("strobeLevel"))
        map.insert("strobeLevel", "low");

    if (!map.contains("dutyCycle"))
        map.insert("dutyCycle", 50);

    if (!map.contains("duration"))
        map.insert("duration", 5);

    map.insert(name, data);

    sirenMode = enumIndex("sirenMode", map.value("sirenMode"));
    sirenLevel = enumIndex("sirenLevel", map.value("sirenLevel"));
    strobeLevel = enumIndex("strobeLevel", map.value("strobeLevel"));
    strobe = map.value("strobe").toBool() ? 0x01 : 0x00;

    if (sirenMode < 0)
    {
        map.insert("sirenMode", "stop");
        sirenMode = 0;
    }

    if (sirenLevel < 0)
    {
        map.insert("sirenLevel", "low");
        sirenLevel = 0;
    }

    if (strobeLevel < 0)
    {
        map.insert("strobeLevel", "low");
        strobeLevel = 0;
    }

    payload.warning = sirenMode << 4 |  strobe << 2 | sirenLevel;
    payload.duration = qToLittleEndian <quint16> (map.value("duration").toInt());
    payload.dutyCycle = static_cast <quint8> (map.value("dutyCycle").toInt());
    payload.strobeLevel = static_cast <quint8> (strobeLevel);

    property->setValue(map);

    if (sirenMode || strobe)
    {
        property->setTimeout(qFromLittleEndian(payload.duration));
        property->setTime(QDateTime::currentSecsSinceEpoch());
    }

    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, 0x00).append(reinterpret_cast <char*> (&payload), sizeof(payload));
}
