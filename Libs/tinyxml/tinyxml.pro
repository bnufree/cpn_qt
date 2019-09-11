include(../common.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = tinyxml
TARGET = $$qtLibraryName($$TARGET)


DEFINES += TIXML_USE_STL

SOURCES += \
    src/tinyxml.cpp \
    src/tinyxmlerror.cpp \
    src/tinyxmlparser.cpp

INCLUDEPATH += $${PWD}/include/tinyxml

Lib_install.path = $${LIB_INSTALL_PATH}
Lib_install.files += $${DESTDIR}/lib$${TARGET}.a
INSTALLS += Lib_install

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install

