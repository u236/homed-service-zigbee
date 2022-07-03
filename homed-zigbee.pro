include(../homed-common/homed-common.pri)

HEADERS += \
    controller.h \
    device.h \
    property.h \
    zigbee.h \
    zstack.h

SOURCES += \
    controller.cpp \
    device.cpp \
    property.cpp \
    zigbee.cpp \
    zstack.cpp

QT += serialport
