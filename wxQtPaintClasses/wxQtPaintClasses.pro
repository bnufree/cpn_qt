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
    glcanvas.cpp

HEADERS += wxqtpaintclasses.h\
        wxqtpaintclasses_global.h \
    bitmap.h \
    _def.h \
    glcanvas.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
