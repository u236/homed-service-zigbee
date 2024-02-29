#include <QtEndian>
#include "zcl.h"

QByteArray zclHeader(quint8 frameControl, quint8 transactionId, quint8 commandId, quint16 manufacturerCode)
{
    QByteArray header(1, static_cast <char> (manufacturerCode ? frameControl | FC_MANUFACTURER_SPECIFIC : frameControl));

    if (manufacturerCode)
    {
        manufacturerCode = qToLittleEndian(manufacturerCode);
        header.append(reinterpret_cast <char*> (&manufacturerCode), sizeof(manufacturerCode));
    }

    return header.append(1, static_cast <char> (transactionId)).append(1, static_cast <char> (commandId));
}

QByteArray readAttributesRequest(quint8 transactionId, quint16 manufacturerCode, QList <quint16> attributes)
{
    QByteArray request = zclHeader(FC_DISABLE_DEFAULT_RESPONSE, transactionId, CMD_READ_ATTRIBUTES, manufacturerCode);

    for (int i = 0; i < attributes.count(); i++)
    {
        quint16 attributeId = qToLittleEndian(attributes.at(i));
        request.append(reinterpret_cast <char*> (&attributeId), sizeof(attributeId));
    }

    return request;
}

QByteArray writeAttributeRequest(quint8 transactionId, quint16 manufacturerCode, quint16 attributeId, quint8 dataType, const QByteArray &data)
{
    writeArrtibutesStruct payload;

    payload.attributeId = qToLittleEndian(attributeId);
    payload.dataType = dataType;

    return zclHeader(FC_DISABLE_DEFAULT_RESPONSE, transactionId, CMD_WRITE_ATTRIBUTES, manufacturerCode).append(reinterpret_cast <char*> (&payload), sizeof(payload)).append(data);
}

quint8 zclDataSize(quint8 dataType)
{
    switch (dataType)
    {
        case DATA_TYPE_BOOLEAN:
        case DATA_TYPE_8BIT_BITMAP:
        case DATA_TYPE_8BIT_UNSIGNED:
        case DATA_TYPE_8BIT_SIGNED:
        case DATA_TYPE_8BIT_ENUM:
            return 1;

        case DATA_TYPE_16BIT_BITMAP:
        case DATA_TYPE_16BIT_UNSIGNED:
        case DATA_TYPE_16BIT_SIGNED:
        case DATA_TYPE_16BIT_ENUM:
            return 2;

        case DATA_TYPE_24BIT_BITMAP:
        case DATA_TYPE_24BIT_UNSIGNED:
        case DATA_TYPE_24BIT_SIGNED:
            return 3;

        case DATA_TYPE_32BIT_BITMAP:
        case DATA_TYPE_32BIT_UNSIGNED:
        case DATA_TYPE_32BIT_SIGNED:
        case DATA_TYPE_SINGLE_PRECISION:
            return 4;

        case DATA_TYPE_40BIT_BITMAP:
        case DATA_TYPE_40BIT_UNSIGNED:
        case DATA_TYPE_40BIT_SIGNED:
            return 5;

        case DATA_TYPE_48BIT_BITMAP:
        case DATA_TYPE_48BIT_UNSIGNED:
        case DATA_TYPE_48BIT_SIGNED:
            return 6;

        case DATA_TYPE_56BIT_BITMAP:
        case DATA_TYPE_56BIT_UNSIGNED:
        case DATA_TYPE_56BIT_SIGNED:
            return 7;

        case DATA_TYPE_64BIT_BITMAP:
        case DATA_TYPE_64BIT_UNSIGNED:
        case DATA_TYPE_64BIT_SIGNED:
        case DATA_TYPE_DOUBLE_PRECISION:
        case DATA_TYPE_IEEE_ADDRESS:
            return 8;
    }

    return 0;
}

quint8 zclDataSize(quint8 dataType, const QByteArray &data, quint8 *offset)
{
    switch (dataType)
    {
        case DATA_TYPE_OCTET_STRING:
        case DATA_TYPE_CHARACTER_STRING:
            return static_cast <quint8> (data.at((*offset)++));

        case DATA_TYPE_ARRAY:
        case DATA_TYPE_STRUCTURE:
            return static_cast <quint8> (data.length() - *offset);
    }

    return zclDataSize(dataType);
}
