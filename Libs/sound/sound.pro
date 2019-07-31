include(../../Libs.pri)

INCLUDEPATH += ../../include

QT += core multimedia

CONFIG += staticlib
TEMPLATE = lib
TARGET = sound
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

message("output:" + $$DESTDIR)
INCLUDEPATH += $${PWD}/include/


Lib_install.path = $${OPENCPN_3RD_STATIC_LIB_PATH}/
Lib_install.files += $${DESTDIR}/$${TARGET}
INSTALLS += Lib_install;

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}/
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install;

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



