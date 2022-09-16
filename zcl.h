#ifndef ZCL_H
#define ZCL_H

#define FC_CLUSTER_SPECIFIC                         0x01
#define FC_MANUFACTURER_SPECIFIC                    0x04
#define FC_SERVER_TO_CLIENT                         0x08
#define FC_DISABLE_DEFAULT_RESPONSE                 0x10

#define CMD_READ_ATTRIBUTES                         0x00
#define CMD_READ_ATTRIBUTES_RESPONSE                0x01
#define CMD_WRITE_ATTRIBUTES                        0x02
#define CMD_CONFIGURE_REPORTING                     0x06
#define CMD_CONFIGURE_REPORTING_RESPONSE            0x07
#define CMD_REPORT_ATTRIBUTES                       0x0A
#define CMD_DEFAULT_RESPONSE                        0x0B

#define DATA_TYPE_BOOLEAN                           0x10
#define DATA_TYPE_8BIT_BITMAP                       0x18
#define DATA_TYPE_16BIT_BITMAP                      0x19
#define DATA_TYPE_24BIT_BITMAP                      0x1A
#define DATA_TYPE_32BIT_BITMAP                      0x1B
#define DATA_TYPE_40BIT_BITMAP                      0x1C
#define DATA_TYPE_48BIT_BITMAP                      0x1D
#define DATA_TYPE_56BIT_BITMAP                      0x1E
#define DATA_TYPE_64BIT_BITMAP                      0x1F
#define DATA_TYPE_8BIT_UNSIGNED                     0x20
#define DATA_TYPE_16BIT_UNSIGNED                    0x21
#define DATA_TYPE_24BIT_UNSIGNED                    0x22
#define DATA_TYPE_32BIT_UNSIGNED                    0x23
#define DATA_TYPE_40BIT_UNSIGNED                    0x24
#define DATA_TYPE_48BIT_UNSIGNED                    0x25
#define DATA_TYPE_56BIT_UNSIGNED                    0x26
#define DATA_TYPE_64BIT_UNSIGNED                    0x27
#define DATA_TYPE_8BIT_SIGNED                       0x28
#define DATA_TYPE_16BIT_SIGNED                      0x29
#define DATA_TYPE_24BIT_SIGNED                      0x2A
#define DATA_TYPE_32BIT_SIGNED                      0x2B
#define DATA_TYPE_40BIT_SIGNED                      0x2C
#define DATA_TYPE_48BIT_SIGNED                      0x2D
#define DATA_TYPE_56BIT_SIGNED                      0x2E
#define DATA_TYPE_64BIT_SIGNED                      0x2F
#define DATA_TYPE_SINGLE_PRECISION                  0x39
#define DATA_TYPE_DOUBLE_PRECISION                  0x3A
#define DATA_TYPE_OCTET_STRING                      0x41
#define DATA_TYPE_CHARACTER_STRING                  0x42
#define DATA_TYPE_STRUCTURE                         0x4C

#define CLUSTER_BASIC                               0x0000
#define CLUSTER_POWER_CONFIGURATION                 0x0001
#define CLUSTER_IDENTIFY                            0x0003
#define CLUSTER_ON_OFF                              0x0006
#define CLUSTER_LEVEL_CONTROL                       0x0008
#define CLUSTER_TIME                                0x000A
#define CLUSTER_ANALOG_INPUT                        0x000C
#define CLUSTER_MULTISTATE_INPUT                    0x0012
#define CLUSTER_OTA_UPGRADE                         0x0019
#define CLUSTER_COLOR_CONTROL                       0x0300
#define CLUSTER_ILLUMINANCE_MEASUREMENT             0x0400
#define CLUSTER_TEMPERATURE_MEASUREMENT             0x0402
#define CLUSTER_RELATIVE_HUMIDITY                   0x0405
#define CLUSTER_OCCUPANCY_SENSING                   0x0406
#define CLUSTER_IAS_ZONE                            0x0500
#define CLUSTER_IAS_ACE                             0x0501
#define CLUSTER_IAS_WD                              0x0502
#define CLUSTER_SMART_ENERGY_METERING               0x0702
#define CLUSTER_ELECTRICAL_MEASUREMENT              0x0B04
#define CLUSTER_TOUCHLINK                           0x1000

#define CLUSTER_TUYA                                0xEF00
#define CLUSTER_LUMI                                0xFCC0

#include <QByteArray>

#pragma pack(push, 1)

struct zclHeaderStruct
{
    quint8  frameControl;
    quint8  transactionId;
    quint8  commandId;
};

struct writeArrtibutesStruct
{
    quint16 attributeId;
    quint8  dataType;
};

struct configureReportingStruct
{
    quint8  direction;
    quint16 attributeId;
    quint8  dataType;
    quint16 minInterval;
    quint16 maxInterval;
    quint16 valueChange;
};

struct defaultResponseStruct
{
    quint8  commandId;
    quint8  status;
};

struct otaFileHeaderStruct
{
    quint32 fileIdentifier;
    quint16 headerVersion;
    quint16 headerLength;
    quint16 headerFieldControl;
    quint16 manufacturerCode;
    quint16 imageType;
    quint32 fileVersion;
    quint16 stackVersion;
    quint8  headerString[32];
    quint32 imageSize;
};

struct otaImageNotifyStruct
{
    quint8  type;
    quint8  jitter;
};

struct otaNextImageRequestStruct
{
    quint8  fieldControl;
    quint16 manufacturerCode;
    quint16 imageType;
    quint32 fileVersion;
};

struct otaNextImageResponseStruct
{
    quint8  status;
    quint16 manufacturerCode;
    quint16 imageType;
    quint32 fileVersion;
    quint32 imageSize;
};

struct otaImageBlockRequestStruct
{
    quint8  fieldControl;
    quint16 manufacturerCode;
    quint16 imageType;
    quint32 fileVersion;
    quint32 fileOffset;
    quint8  dataSizeMax;
};

struct otaImageBlockResponseStruct
{
    quint8  status;
    quint16 manufacturerCode;
    quint16 imageType;
    quint32 fileVersion;
    quint32 fileOffset;
    quint8  dataSize;
};

struct otaUpgradeEndRequestStruct
{
    quint8  status;
    quint16 manufacturerCode;
    quint16 imageType;
    quint32 fileVersion;
};

struct otaUpgradeEndResponseStruct
{
    quint16 manufacturerCode;
    quint16 imageType;
    quint32 fileVersion;
    quint32 currentTime;
    quint32 upgradeTime;
};

struct touchLinkScanStruct
{
    quint32 transactionId;
    quint8  zigBeeInformation;
    quint8  touchLinkInformation;
};

struct tuyaHeaderStruct
{
    quint8  status;
    quint8  transactionId;
    quint8  dataPoint;
    quint8  dataType;
    quint8  function;
    quint8  length;
};

#pragma pack(pop)

quint8 zclDataSize(quint8 dataType, const QByteArray &data, quint8 *offset);

#endif
