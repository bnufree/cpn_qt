include(../../Libs.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = texcmp
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
    squish/alpha.h \
    squish/clusterfit.h \
    squish/colourblock.h \
    squish/colourfit.h \
    squish/colourset.h \
    squish/config.h \
    squish/maths.h \
    squish/rangefit.h \
    squish/simd.h \
    squish/simd_float.h \
    squish/simd_sse.h \
    squish/simd_ve.h \
    squish/singlecolourfit.h \
    squish/singlecolourfitfast.h \
    squish/squish.h \
    squish/twocolourfitfast.h

SOURCES += \
    squish/alpha.cpp \
    squish/clusterfit.cpp \
    squish/colourblock.cpp \
    squish/colourfit.cpp \
    squish/colourset.cpp \
    squish/maths.cpp \
    squish/rangefit.cpp \
    squish/singlecolourfit.cpp \
    squish/singlecolourfitfast.cpp \
    squish/singlecolourlookup.inl \
    squish/squish.cpp \
    squish/twocolourfitfast.cpp \
    etcpak.cpp

