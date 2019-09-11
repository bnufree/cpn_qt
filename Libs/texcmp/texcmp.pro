include(../common.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = texcmp
TARGET = $$qtLibraryName($$TARGET)


DEFINES += TIXML_USE_STL
INCLUDEPATH += $${PWD}/include/texcmp

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

Lib_install.path = $${LIB_INSTALL_PATH}
Lib_install.files += $${DESTDIR}/lib$${TARGET}.a
INSTALLS += Lib_install

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install

