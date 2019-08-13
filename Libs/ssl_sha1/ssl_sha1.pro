include(../../Libs.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = ssl_sha1
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
INCLUDEPATH += $${PWD}/include

Lib_install.path = $${OPENCPN_3RD_STATIC_LIB_PATH}/
Lib_install.files += $${DESTDIR}/$${TARGET}
INSTALLS += Lib_install;

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}/
Include_install.files += $${PWD}/include/*.h
INSTALLS += Include_install;

SOURCES += \
    src/sha1.c

HEADERS += \
    include/ssl/sha1.h



