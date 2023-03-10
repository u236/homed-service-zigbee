include(../homed-common/homed-color.pri)
include(../homed-common/homed-common.pri)
include(../homed-common/homed-gpio.pri)

HEADERS += \
    action.h \
    adapter.h \
    binding.h \
    controller.h \
    device.h \
    expose.h \
    ezsp.h \
    poll.h \
    property.h \
    reporting.h \
    zcl.h \
    zigate.h \
    zigbee.h \
    zstack.h

SOURCES += \
    action.cpp \
    adapter.cpp \
    binding.cpp \
    controller.cpp \
    device.cpp \
    expose.cpp \
    ezsp.cpp \
    poll.cpp \
    property.cpp \
    reporting.cpp \
    zcl.cpp \
    zigate.cpp \
    zigbee.cpp \
    zstack.cpp

DISTFILES += \
    deploy/data/usr/share/homed/zigbee.json

QT += serialport

deploy.files = $${DISTFILES}
deploy.path = /usr/share/homed

INSTALLS += deploy
