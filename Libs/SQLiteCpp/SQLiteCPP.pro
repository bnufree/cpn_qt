include(../common.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = SQLiteCPP
TARGET = $$qtLibraryName($$TARGET)

QT += core

INCLUDEPATH += $${PWD}/include

LIBS += -lsqlite3

Lib_install.path = $${LIB_INSTALL_PATH}/
Lib_install.files += $${DESTDIR}/lib$${TARGET}.a
INSTALLS += Lib_install

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}/
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install

SOURCES += \
    src/Backup.cpp \
    src/Column.cpp \
    src/Database.cpp \
    src/Exception.cpp \
    src/Statement.cpp \
    src/Transaction.cpp






