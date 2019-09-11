include(../common.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = lz4
TARGET = $$qtLibraryName($$TARGET)



#DEFINES += TIXML_USE_STL



INCLUDEPATH += $${PWD}/include/lz4

SOURCES += \
    src/lz4.c \
    src/lz4hc.c

Lib_install.path = $${LIB_INSTALL_PATH}
Lib_install.files += $${DESTDIR}/lib$${TARGET}.a
INSTALLS += Lib_install

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install



