#include "zcl.h"

quint8 zclDataSize(quint8 dataType, const QByteArray &data, quint8 *offset)
{
    switch (dataType)
    {
        case DATA_TYPE_BOOLEAN:
        case DATA_TYPE_8BIT_BITMAP:
        case DATA_TYPE_8BIT_UNSIGNED:
        case DATA_TYPE_8BIT_SIGNED:
            return 1;

        case DATA_TYPE_16BIT_BITMAP:
        case DATA_TYPE_16BIT_UNSIGNED:
        case DATA_TYPE_16BIT_SIGNED:
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
            return 8;

        case DATA_TYPE_OCTET_STRING:
        case DATA_TYPE_CHARACTER_STRING:
            return static_cast <quint8> (data.at((*offset)++));

        case DATA_TYPE_STRUCTURE:
            return static_cast <quint8> (data.length() - *offset);
    }

    return 0;
}
