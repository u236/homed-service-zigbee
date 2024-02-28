include(../homed-common/homed-color.pri)
include(../homed-common/homed-common.pri)
include(../homed-common/homed-endpoint.pri)
include(../homed-common/homed-gpio.pri)

HEADERS += \
    action.h \
    actions/common.h \
    actions/custom.h \
    actions/efekta.h \
    actions/lumi.h \
    actions/modkam.h \
    actions/other.h \
    actions/ptvo.h \
    actions/tuya.h \
    adapter.h \
    binding.h \
    controller.h \
    device.h \
    ezsp.h \
    poll.h \
    properties/byun.h \
    properties/common.h \
    properties/custom.h \
    properties/efekta.h \
    properties/hue.h \
    properties/ias.h \
    properties/ikea.h \
    properties/lumi.h \
    properties/modkam.h \
    properties/other.h \
    properties/ptvo.h \
    properties/tuya.h \
    property.h \
    reporting.h \
    zcl.h \
    zigate.h \
    zigbee.h \
    zstack.h

SOURCES += \
    action.cpp \
    actions/common.cpp \
    actions/custom.cpp \
    actions/efekta.cpp \
    actions/hue.cpp \
    actions/hue.h \
    actions/lumi.cpp \
    actions/modkam.cpp \
    actions/other.cpp \
    actions/ptvo.cpp \
    actions/tuya.cpp \
    adapter.cpp \
    binding.cpp \
    controller.cpp \
    device.cpp \
    ezsp.cpp \
    poll.cpp \
    properties/byun.cpp \
    properties/common.cpp \
    properties/custom.cpp \
    properties/efekta.cpp \
    properties/hue.cpp \
    properties/ias.cpp \
    properties/ikea.cpp \
    properties/lumi.cpp \
    properties/modkam.cpp \
    properties/other.cpp \
    properties/ptvo.cpp \
    properties/tuya.cpp \
    property.cpp \
    reporting.cpp \
    zcl.cpp \
    zigate.cpp \
    zigbee.cpp \
    zstack.cpp

DISTFILES += \
    deploy/data/usr/share/homed-zigbee/efekta.json \
    deploy/data/usr/share/homed-zigbee/expose.json \
    deploy/data/usr/share/homed-zigbee/gledopto.json \
    deploy/data/usr/share/homed-zigbee/gs.json \
    deploy/data/usr/share/homed-zigbee/homed.json \
    deploy/data/usr/share/homed-zigbee/hue.json \
    deploy/data/usr/share/homed-zigbee/ikea.json \
    deploy/data/usr/share/homed-zigbee/konke.json \
    deploy/data/usr/share/homed-zigbee/lifecontrol.json \
    deploy/data/usr/share/homed-zigbee/lumi.json \
    deploy/data/usr/share/homed-zigbee/modkam.json \
    deploy/data/usr/share/homed-zigbee/orvibo.json \
    deploy/data/usr/share/homed-zigbee/other.json \
    deploy/data/usr/share/homed-zigbee/perenio.json \
    deploy/data/usr/share/homed-zigbee/sonoff.json \
    deploy/data/usr/share/homed-zigbee/tuya.json

QT += serialport

deploy.files = $${DISTFILES}
deploy.path = /usr/share/homed-zigbee

INSTALLS += deploy
