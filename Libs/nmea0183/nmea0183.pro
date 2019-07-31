include(../../Libs.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = nmea0183
TARGET = $$qtLibraryName($$TARGET)

QT += core

#根据不同的编译清空生成不同的输出路径
CONFIG(release, debug|release) {
  DEFINES *= RELEASE _RELEASE NDEBUG
  CONFIG_NAME = Release
} else {
  DEFINES *= DEBUG _DEBUG
  CONFIG_NAME = Debug
}

TargetRoot=$$dirname(PWD)
BINARIES_PATH = $$TargetRoot/out/$$CONFIG_NAME
DESTDIR = $$BINARIES_PATH

message("output:" + $$DESTDIR)
INCLUDEPATH += $${PWD}/include/gdal

Lib_install.path = $${OPENCPN_3RD_STATIC_LIB_PATH}/
Lib_install.files += $${DESTDIR}/$${TARGET}
INSTALLS += Lib_install;

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}/
Include_install.files += $${PWD}/src/*.h
INSTALLS += Include_install;

HEADERS += \
    src/apb.hpp \
    src/gga.hpp \
    src/gll.hpp \
    src/GPwpl.hpp \
    src/gsv.hpp \
    src/hdg.hpp \
    src/hdm.hpp \
    src/hdt.hpp \
    src/LatLong.hpp \
    src/nmea0183.h \
    src/nmea0183.hpp \
    src/Response.hpp \
    src/RMB.hpp \
    src/RMC.HPP \
    src/rte.hpp \
    src/Sentence.hpp \
    src/vtg.hpp \
    src/wpl.hpp \
    src/xte.hpp

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

