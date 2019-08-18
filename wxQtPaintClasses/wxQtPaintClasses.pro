#-------------------------------------------------
#
# Project created by QtCreator 2019-08-16T13:04:45
#
#-------------------------------------------------

QT       += widgets opengl svg xml xmlpatterns

TARGET = wxQtPaintClasses
TEMPLATE = lib

DEFINES += WXQTPAINTCLASSES_LIBRARY __WXQT__


SOURCES += wxqtpaintclasses.cpp \
    bitmap.cpp \
    qt/dc.cpp \
    qt/dcclient.cpp \
    qt/dcmemory.cpp \
    qt/dcprint.cpp \
    qt/dcscreen.cpp \
    base/dcbase.cpp \
    base/dcbufcmn.cpp \
    base/dcgraph.cpp \
    base/dcsvg.cpp \
    qt/window.cpp

HEADERS += wxqtpaintclasses.h\
        wxqtpaintclasses_global.h \
    bitmap.h \
    qt/dc.h \
    qt/dcclient.h \
    qt/dcmemory.h \
    qt/dcprint.h \
    qt/dcscreen.h \
    base/dc.h \
    base/dcbuffer.h \
    base/dcclient.h \
    base/dcgraph.h \
    base/dcmemory.h \
    base/dcmirror.h \
    base/dcprint.h \
    base/dcps.h \
    base/dcscreen.h \
    base/dcsvg.h \
    qt/_def.h \
    qt/window.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
