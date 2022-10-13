include(../homed-common/homed-common.pri)
include(../homed-common/homed-gpio.pri)

HEADERS += \
    action.h \
    adapter.h \
    controller.h \
    device.h \
    ezsp.h \
    poll.h \
    property.h \
    reporting.h \
    zcl.h \
    zigbee.h \
    zstack.h

SOURCES += \
    action.cpp \
    adapter.cpp \
    controller.cpp \
    ezsp.cpp \
    poll.cpp \
    property.cpp \
    reporting.cpp \
    zcl.cpp \
    zigbee.cpp \
    zstack.cpp

DISTFILES += \
    package/data/usr/share/homed/zigbee.json

QT += serialport

deploy.files = $${DISTFILES}
deploy.path = /usr/share/homed

INSTALLS += deploy
