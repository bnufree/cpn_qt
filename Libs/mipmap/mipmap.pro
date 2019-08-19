include(../../Libs.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = mipmap
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
INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}
INCLUDEPATH += $${PWD}/include/mipmap

Lib_install.path = $${OPENCPN_3RD_STATIC_LIB_PATH}/
Lib_install.files += $${DESTDIR}/$${TARGET}
INSTALLS += Lib_install;

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}/
Include_install.files += $${PWD}/include/*.h
INSTALLS += Include_install;

SOURCES += \
    src/mipmap.c \
    src/mipmap_avx2.c \
    src/mipmap_neon.c \
    src/mipmap_sse.c \
    src/mipmap_sse2.c \
    src/mipmap_ssse3.c

HEADERS += \
    include/mipmap/mipmap.h




