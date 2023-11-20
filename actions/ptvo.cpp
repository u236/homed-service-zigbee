#include <QtEndian>
#include "ptvo.h"

QByteArray ActionsPTVO::Status::request(const QString &, const QVariant &data)
{
    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, data.toBool() ? 0x01 : 0x00);
}

QByteArray ActionsPTVO::AnalogInput::request(const QString &, const QVariant &data)
{
    float value = qToLittleEndian <float> (data.toFloat() * option(QString(m_name).append("Divider"), 1).toDouble());
    return writeAttribute(DATA_TYPE_SINGLE_PRECISION, &value, sizeof(value));
}

QByteArray ActionsPTVO::SerialData::request(const QString &, const QVariant &data)
{
    QByteArray value = QByteArray::fromHex(data.toString().toUtf8());
    return value.length() > 0x7F ? QByteArray() : writeAttributeRequest(m_transactionId++, m_manufacturerCode, 0x000E, DATA_TYPE_CHARACTER_STRING, QByteArray(1, static_cast <char> (value.length())).append(value));
}
