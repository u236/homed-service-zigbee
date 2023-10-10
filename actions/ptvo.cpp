#include <QtEndian>
#include "ptvo.h"

QByteArray ActionsPTVO::Status::request(const QString &, const QVariant &data)
{
    return zclHeader(FC_CLUSTER_SPECIFIC, m_transactionId++, data.toBool() ? 0x01 : 0x00);
}

QByteArray ActionsPTVO::AnalogInput::request(const QString &, const QVariant &data)
{
    float value = qToLittleEndian(data.toFloat());
    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, m_attributes.at(0), DATA_TYPE_SINGLE_PRECISION, QByteArray(reinterpret_cast <char*> (&value), sizeof(value)));
}

QByteArray ActionsPTVO::SerialData::request(const QString &, const QVariant &data)
{
    QByteArray value = QByteArray::fromHex(data.toString().toUtf8());

    if (value.length() > 0x7F)
        return QByteArray();

    return writeAttributeRequest(m_transactionId++, m_manufacturerCode, 0x000E, DATA_TYPE_OCTET_STRING, QByteArray(1, static_cast <char> (value.length())).append(value));
}
