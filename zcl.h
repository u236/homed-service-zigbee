#ifndef ZCL_H
#define ZCL_H

#define FC_CLUSTER_SPECIFIC                         0x01
#define FC_MANUFACTURER_SPECIFIC                    0x04
#define FC_SERVER_TO_CLIENT                         0x08
#define FC_DISABLE_DEFAULT_RESPONSE                 0x10

#define CMD_READ_ATTRIBUTES                         0x00
#define CMD_READ_ATTRIBUTES_RESPONSE                0x01
#define CMD_WRITE_ATTRIBUTES                        0x02
#define CMD_WRITE_ATTRIBUTES_RESPONSE               0x04
#define CMD_CONFIGURE_REPORTING                     0x06
#define CMD_CONFIGURE_REPORTING_RESPONSE            0x07
#define CMD_REPORT_ATTRIBUTES                       0x0A
#define CMD_DEFAULT_RESPONSE                        0x0B

#define POWER_SOURCE_UNKNOWN                        0x00
#define POWER_SOURCE_MAINS                          0x01
#define POWER_SOURCE_BATTERY                        0x03
#define POWER_SOURCE_DC                             0x04

#define STATUS_SUCCESS                              0x00
#define STATUS_UNSUPPORTED_ATTRIBUTE                0x86
#define STATUS_INSUFFICIENT_SPACE                   0x89
#define STATUS_DUPLICATE_EXISTS                     0x8A
#define STATUS_NOT_FOUND                            0x8B
#define STATUS_NO_IMAGE_AVAILABLE                   0x98

#define DATA_TYPE_NO_DATA                           0x00
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
#define DATA_TYPE_8BIT_ENUM                         0x30
#define DATA_TYPE_16BIT_ENUM                        0x31
#define DATA_TYPE_SINGLE_PRECISION                  0x39
#define DATA_TYPE_DOUBLE_PRECISION                  0x3A
#define DATA_TYPE_OCTET_STRING                      0x41
#define DATA_TYPE_CHARACTER_STRING                  0x42
#define DATA_TYPE_ARRAY                             0x48
#define DATA_TYPE_STRUCTURE                         0x4C
#define DATA_TYPE_UTC_TIME                          0xE2
#define DATA_TYPE_IEEE_ADDRESS                      0xF0

#define CLUSTER_BASIC                               0x0000
#define CLUSTER_POWER_CONFIGURATION                 0x0001
#define CLUSTER_TEMPERATURE_CONFIGURATION           0x0002
#define CLUSTER_IDENTIFY                            0x0003
#define CLUSTER_GROUPS                              0x0004
#define CLUSTER_SCENES                              0x0005
#define CLUSTER_ON_OFF                              0x0006
#define CLUSTER_SWITCH_CONFIGURATION                0x0007
#define CLUSTER_LEVEL_CONTROL                       0x0008
#define CLUSTER_TIME                                0x000A
#define CLUSTER_ANALOG_INPUT                        0x000C
#define CLUSTER_ANALOG_OUTPUT                       0x000D
#define CLUSTER_BINARY_OUTPUT                       0x0010
#define CLUSTER_MULTISTATE_INPUT                    0x0012
#define CLUSTER_MULTISTATE_VALUE                    0x0014
#define CLUSTER_OTA_UPGRADE                         0x0019
#define CLUSTER_POWER_PROFILE                       0x001A
#define CLUSTER_POLL_CONTROL                        0x0020
#define CLUSTER_GREEN_POWER                         0x0021
#define CLUSTER_DOOR_LOCK                           0x0101
#define CLUSTER_WINDOW_COVERING                     0x0102
#define CLUSTER_THERMOSTAT                          0x0201
#define CLUSTER_FAN_CONTROL                         0x0202
#define CLUSTER_THERMOSTAT_UI_CONFIGURATION         0x0204
#define CLUSTER_COLOR_CONTROL                       0x0300
#define CLUSTER_ILLUMINANCE_MEASUREMENT             0x0400
#define CLUSTER_ILLUMINANCE_LEVEL_SENSING           0x0401
#define CLUSTER_TEMPERATURE_MEASUREMENT             0x0402
#define CLUSTER_PRESSURE_MEASUREMENT                0x0403
#define CLUSTER_HUMIDITY_MEASUREMENT                0x0405
#define CLUSTER_OCCUPANCY_SENSING                   0x0406
#define CLUSTER_MOISTURE_MEASUREMENT                0x0408
#define CLUSTER_PH_MEASUREMENT                      0x0409
#define CLUSTER_CO2_CONCENTRATION                   0x040D
#define CLUSTER_PM25_CONCENTRATION                  0x042A
#define CLUSTER_IAS_ZONE                            0x0500
#define CLUSTER_IAS_WD                              0x0502
#define CLUSTER_SMART_ENERGY_METERING               0x0702
#define CLUSTER_ELECTRICAL_MEASUREMENT              0x0B04
#define CLUSTER_TOUCHLINK                           0x1000

#define CLUSTER_BYUN                                0x040A
#define CLUSTER_PERENIO                             0xFC7B
#define CLUSTER_LUMI                                0xFCC0

#define CLUSTER_TUYA_DATA                           0xEF00
#define CLUSTER_TUYA_SWITCH_MODE                    0xE001
#define CLUSTER_TUYA_IR_DATA                        0xED00
#define CLUSTER_TUYA_IR_CONTROL                     0xE004

#define TUYA_TYPE_RAW                               0x00
#define TUYA_TYPE_BOOL                              0x01
#define TUYA_TYPE_VALUE                             0x02
#define TUYA_TYPE_ENUM                              0x04

#define MANUFACTURER_CODE_SILABS                    0x1049
#define MANUFACTURER_CODE_LUMI                      0x115F

#include <QList>

#pragma pack(push, 1)

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
    quint64 valueChange;
};

struct defaultResponseStruct
{
    quint8  commandId;
    quint8  status;
};

struct groupControlResponseStruct
{
    quint8  status;
    quint16 groupId;
};

struct recallSceneStruct
{
    quint16 groupId;
    quint8  sceneId;
};

struct moveToLevelStruct
{
    quint8  level;
    quint16 time;
};

struct moveLevelStruct
{
    quint8  mode;
    quint8  rate;
};

struct stepLevelStruct
{
    quint8  mode;
    quint8  size;
    quint16 time;
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
    quint8  maxDataSize;
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

struct moveHueStruct
{
    quint8  mode;
    quint8  rate;
};

struct moveSaturationStruct
{
    quint8  mode;
    quint8  rate;
};

struct moveToColorHSStruct
{
    quint8  colorH;
    quint8  colorS;
    quint16 time;
};

struct moveToColorXYStruct
{
    quint16 colorX;
    quint16 colorY;
    quint16 time;
};

struct moveToColorTemperatureStruct
{
    quint16 colorTemperature;
    quint16 time;
};

struct moveColorTemperatureStruct
{
    quint8  mode;
    quint16 rate;
    quint16 minMireds;
    quint16 maxMireds;
};

struct stepColorTemperatureStruct
{
    quint8  mode;
    quint16 size;
    quint16 time;
    quint16 minMireds;
    quint16 maxMireds;
};

struct iasZoneEnrollResponseStruct
{
    quint8  responseCode;
    quint8  zoneId;
};

struct iasStartWarningStruct
{
    quint8  warning;
    quint16 duration;
    quint8  dutyCycle;
    quint8  strobeLevel;
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

struct tuyaTimeStruct
{
    quint16 payloadSize;
    quint32 utcTimestamp;
    quint32 localTimestamp;
};

#pragma pack(pop)

QByteArray zclHeader(quint8 frameControl, quint8 transactionId, quint8 commandId, quint16 manufacturerCode = 0);
QByteArray readAttributesRequest(quint8 transactionId, quint16 manufacturerCode, QList <quint16> attributes);
QByteArray writeAttributeRequest(quint8 transactionId, quint16 manufacturerCode, quint16 attributeId, quint8 dataType, const QByteArray &data);

quint8 zclDataSize(quint8 dataType);
quint8 zclDataSize(quint8 dataType, const QByteArray &data, quint8 *offset);

#endif
