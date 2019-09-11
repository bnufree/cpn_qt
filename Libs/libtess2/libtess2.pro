include(../common.pri)

CONFIG += staticlib
TEMPLATE = lib
TARGET = tess2
TARGET = $$qtLibraryName($$TARGET)

#DEFINES += TIXML_USE_STL



INCLUDEPATH += $${PWD}/include/tess2

HEADERS += \
    Source/bucketalloc.h \
    Source/dict.h \
    Source/geom.h \
    Source/mesh.h \
    Source/priorityq.h \
    Source/sweep.h \
    Source/tess.h

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

Lib_install.path = $${LIB_INSTALL_PATH}
Lib_install.files += $${DESTDIR}/lib$${TARGET}.a
INSTALLS += Lib_install

Include_install.path = $${OPENCPN_3RD_INCLUDE_PATH}
Include_install.files += $${PWD}/include/*
INSTALLS += Include_install



