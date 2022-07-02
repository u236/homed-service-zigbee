include(../homed-common/homed-common.pri)

HEADERS += \
    controller.h \
    device.h \
    zigbee.h \
    zstack.h

SOURCES += \
    controller.cpp \
    zigbee.cpp \
    zstack.cpp

QT += serialport
