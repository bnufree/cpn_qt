include(../common.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = mipmap
TARGET = $$qtLibraryName($$TARGET)

QT += core

INCLUDEPATH += $${PWD}/include/mipmap


SOURCES += \
    src/mipmap.c \
    src/mipmap_avx2.c \
    src/mipmap_neon.c \
    src/mipmap_sse.c \
    src/mipmap_sse2.c \
    src/mipmap_ssse3.c

QMAKE_CFLAGS += -mavx2

Lib_install.path = $${LIB_INSTALL_PATH}
Lib_install.files += $${DESTDIR}/lib$${TARGET}.a
INSTALLS += Lib_install

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install




