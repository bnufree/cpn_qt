include(../../Libs.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = lz4
TARGET = $$qtLibraryName($$TARGET)

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

DEFINES += TIXML_USE_STL



INCLUDEPATH += $${PWD}/include/

Lib_install.path = $${OPENCPN_3RD_STATIC_LIB_PATH}/
Lib_install.files += $${DESTDIR}/$${TARGET}
INSTALLS += Lib_install;

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}/
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install;

HEADERS += \
    src/lz4.h \
    src/lz4hc.h

SOURCES += \
    src/lz4.c \
    src/lz4hc.c



