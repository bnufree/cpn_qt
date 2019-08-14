include(../../Libs.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = tess2
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

DEFINES += TIXML_USE_STL



INCLUDEPATH += $${PWD}/include/

Lib_install.path = $${OPENCPN_3RD_STATIC_LIB_PATH}/
Lib_install.files += $${DESTDIR}/$${TARGET}
INSTALLS += Lib_install;

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}/
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install;

HEADERS += \
    Source/bucketalloc.h \
    Source/dict.h \
    Source/geom.h \
    Source/mesh.h \
    Source/priorityq.h \
    Source/sweep.h \
    Source/tess.h \
    Include/Adjacency.h \
    Include/CustomArray.h \
    Include/RevisitedRadix.h \
    Include/Striper.h \
    Include/StripStdafx.h \
    Include/tesselator.h

SOURCES += \
    Source/Adjacency.cpp \
    Source/CustomArray.cpp \
    Source/RevisitedRadix.cpp \
    Source/Striper.cpp \
    Source/bucketalloc.c \
    Source/dict.c \
    Source/geom.c \
    Source/mesh.c \
    Source/priorityq.c \
    Source/sweep.c \
    Source/tess.c



