include(../common.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = nmea0183
TARGET = $$qtLibraryName($$TARGET)

QT += core

INCLUDEPATH += $${PWD}/include/nmea0183

SOURCES += \
    src/apb.cpp \
    src/expid.cpp \
    src/gga.cpp \
    src/gll.cpp \
    src/GPwpl.cpp \
    src/gsv.cpp \
    src/hdg.cpp \
    src/hdm.cpp \
    src/hdt.cpp \
    src/hexvalue.cpp \
    src/lat.cpp \
    src/latlong.cpp \
    src/long.cpp \
    src/nmea0183.cpp \
    src/response.cpp \
    src/rmb.cpp \
    src/rmc.cpp \
    src/rte.cpp \
    src/sentence.cpp \
    src/talkerid.cpp \
    src/vtg.cpp \
    src/wpl.cpp \
    src/xte.cpp


Lib_install.path = $${LIB_INSTALL_PATH}
Lib_install.files += $${DESTDIR}/lib$${TARGET}.a
INSTALLS += Lib_install

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install

