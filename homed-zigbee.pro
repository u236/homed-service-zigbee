include(../homed-common/homed-common.pri)
include(../homed-common/homed-gpio.pri)

HEADERS += \
    action.h \
    controller.h \
    device.h \
    poll.h \
    property.h \
    reporting.h \
    zcl.h \
    zigbee.h \
    zstack.h

SOURCES += \
    action.cpp \
    controller.cpp \
    device.cpp \
    property.cpp \
    zigbee.cpp \
    zstack.cpp

QT += serialport
