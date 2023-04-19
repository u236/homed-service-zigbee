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
    meta.h \
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
    meta.cpp \
    poll.cpp \
    property.cpp \
    reporting.cpp \
    zcl.cpp \
    zigate.cpp \
    zigbee.cpp \
    zstack.cpp

DISTFILES += \
    deploy/data/usr/share/homed-zigbee/efekta.json \
    deploy/data/usr/share/homed-zigbee/gledopto.json \
    deploy/data/usr/share/homed-zigbee/gs.json \
    deploy/data/usr/share/homed-zigbee/homed.json \
    deploy/data/usr/share/homed-zigbee/ikea.json \
    deploy/data/usr/share/homed-zigbee/konke.json \
    deploy/data/usr/share/homed-zigbee/lifecontrol.json \
    deploy/data/usr/share/homed-zigbee/lumi.json \
    deploy/data/usr/share/homed-zigbee/orvibo.json \
    deploy/data/usr/share/homed-zigbee/other.json \
    deploy/data/usr/share/homed-zigbee/perenio.json \
    deploy/data/usr/share/homed-zigbee/sonoff.json \
    deploy/data/usr/share/homed-zigbee/tuya.json

QT += serialport

deploy.files = $${DISTFILES}
deploy.path = /usr/share/homed-zigbee

INSTALLS += deploy
