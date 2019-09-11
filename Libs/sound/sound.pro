include(../common.pri)

INCLUDEPATH += ../../include

QT += core multimedia

CONFIG += staticlib
TEMPLATE = lib
TARGET = sound
TARGET = $$qtLibraryName($$TARGET)

INCLUDEPATH += $${PWD}/include/sound



DEFINES += TIXML_USE_STL
CONFIG += C++11

SOURCES += \
    src/AndroidSound.cpp \
    src/MswSound.cpp \
    src/OCPN_Sound.cpp \
    src/OcpnWxSound.cpp \
    src/SoundFactory.cpp \
    src/SoundFileLoader.cpp \
    src/SoundLoaderFactory.cpp \
    src/SystemCmdSound.cpp

Lib_install.path = $${LIB_INSTALL_PATH}/
Lib_install.files += $${DESTDIR}/lib$${TARGET}.a
INSTALLS += Lib_install

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}/
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install




