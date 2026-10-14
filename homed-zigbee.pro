include(../homed-common/homed-color.pri)
include(../homed-common/homed-common.pri)
include(../homed-common/homed-endpoint.pri)
include(../homed-common/homed-gpio.pri)
include(../homed-common/homed-parser.pri)

HEADERS += \
    action.h \
    actions/common.h \
    actions/ias.h \
    actions/lumi.h \
    actions/other.h \
    actions/ptvo.h \
    actions/tuya.h \
    adapter.h \
    binding.h \
    controller.h \
    device.h \
    ezsp.h \
    poll.h \
    properties/common.h \
    properties/ias.h \
    properties/lumi.h \
    properties/other.h \
    properties/ptvo.h \
    properties/tuya.h \
    property.h \
    reporting.h \
    zcl.h \
    zigate.h \
    zigbee.h \
    zstack.h \
    zboss.h

SOURCES += \
    action.cpp \
    actions/common.cpp \
    actions/ias.cpp \
    actions/lumi.cpp \
    actions/other.cpp \
    actions/ptvo.cpp \
    actions/tuya.cpp \
    adapter.cpp \
    binding.cpp \
    controller.cpp \
    device.cpp \
    ezsp.cpp \
    poll.cpp \
    properties/common.cpp \
    properties/ias.cpp \
    properties/lumi.cpp \
    properties/other.cpp \
    properties/ptvo.cpp \
    properties/tuya.cpp \
    property.cpp \
    reporting.cpp \
    zcl.cpp \
    zigate.cpp \
    zigbee.cpp \
    zstack.cpp \
    zboss.cpp

DISTFILES += $$files(deploy/data/usr/share/homed-zigbee/*, true)
QT += serialport

deploy.files = deploy/data/usr/share/homed-zigbee/*
deploy.path = /usr/share/homed-zigbee

INSTALLS += deploy
