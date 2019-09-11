include(../common.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = iso8211
TARGET = $$qtLibraryName($$TARGET)

QT += core

SOURCES += \
    src/ddffield.cpp \
    src/ddffielddefn.cpp \
    src/ddfmodule.cpp \
    src/ddfrecord.cpp \
    src/ddfrecordindex.cpp \
    src/ddfsubfielddefn.cpp \
    src/ddfutils.cpp

INCLUDEPATH += $${PWD}/include/iso8211
INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}

Lib_install.path = $${LIB_INSTALL_PATH}
Lib_install.files += $${DESTDIR}/lib$${TARGET}.a
INSTALLS += Lib_install

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install




