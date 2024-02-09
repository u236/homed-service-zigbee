#include "hue.h"

QByteArray ActionsHUE::IndicatorMode::request(const QString &, const QVariant &data)
{
    quint8 value = data.toString() == "on" ? 0x01 : 0x00;
    return writeAttribute(DATA_TYPE_BOOLEAN, &value, sizeof(value));
}

QByteArray ActionsHUE::SensivivityMode::request(const QString &, const QVariant &data)
{
    qint8 value = listIndex({"low", "medium", "high", "ultra", "max"}, data);
    return value < 0 ? QByteArray() : writeAttribute(DATA_TYPE_8BIT_UNSIGNED, &value, sizeof(value));
}
