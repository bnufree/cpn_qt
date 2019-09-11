include(../common.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = ssl_sha1
TARGET = $$qtLibraryName($$TARGET)

QT += core

#INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}
INCLUDEPATH += $${PWD}/include/ssl_sha1

Lib_install.path = $${LIB_INSTALL_PATH}/
Lib_install.files += $${DESTDIR}/lib$${TARGET}.a
INSTALLS += Lib_install

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}/
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install

SOURCES += \
    src/sha1.c



