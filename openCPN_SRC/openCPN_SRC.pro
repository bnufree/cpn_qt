#-------------------------------------------------
#
# Project created by QtCreator 2018-12-03T13:46:36
#
#-------------------------------------------------

include(../Libs.pri)

INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}

INCLUDEPATH += ../include

LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -lgdal$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -lnmea0183$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -ltinyxml$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -lsound$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -liso8211$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -lSQLiteCPP$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -lssl_sha1$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -ltexcmp$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -llz4$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -ltess2$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}/$${CONFIG_NAME}\ -lmipmap$${EXT_NAME}
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -llzma
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -lsqlite3
LIBS += -lpsapi

QT       += core gui network positioning core_private opengl svg xml webkitwidgets multimedia

DEFINES += ocpnUSE_GL TIXML_USE_STL __WXQT__ ocpnUSE_SVG

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = zchx_cpn
TARGET = $$qtLibraryName($$TARGET)
TEMPLATE = app
#CONFIG += shared dll c++11
CONFIG += c++11
TargetRoot = $$dirname(PWD)/
BINARIES_PATH = $$TargetRoot/out/$$CONFIG_NAME
DESTDIR = $$BINARIES_PATH

message("rebuild please")

TRANSLATIONS += $$PWD/translations/zchx_ecdis_zh_CN.ts
exists($$PWD/translations/zchx_ecdis_zh_CN.ts){
    mkpath($$BINARIES_PATH/translations)
    mkpath($$BINARIES_PATH/resources)
    system(lrelease $$PWD/translations/zchx_ecdis_zh_CN.ts -qm $$BINARIES_PATH/translations/zchx_ecdis_zh_CN.qm)
    system(rcc $$PWD/res/resources.qrc --binary -o $$BINARIES_PATH/resources/zchx_ecdis.rcc)
}

DEFINES *= ZCHX_OPENCPN_PLUGIN ocpnUSE_GL

SOURCES += \
    main.cpp \
    _def.cpp \
    zchxs57listctrlbox.cpp \
    zchxoptionsdlg.cpp \
    zchxconfig.cpp \
    zchxopengloptiondlg.cpp \
    zchxmapmainwindow.cpp \
    glTexCache.cpp \
    glTextureDescriptor.cpp \
    glTextureManager.cpp \
    s52plib.cpp \
    s52utils.cpp \
    OCPNPlatform.cpp \
    FontDesc.cpp \
    FontMgr.cpp \
    chartdb.cpp \
    chartdbs.cpp \
    chartimg.cpp \
    chartsymbols.cpp \
    OCPNRegion.cpp \
    georef.cpp \
    vector2d.cpp \
    viewport.cpp \
    bitmap.cpp \
    glChartCanvas.cpp \
    bbox.cpp \
    chcanv.cpp \
    Quilt.cpp \
    SencManager.cpp \
    s57RegistrarMgr.cpp \
    s57chart.cpp \
    mygeom.cpp \
    cutil.cpp \
    s57obj.cpp \
    s52cnsy.cpp \
    TexFont.cpp \
    styles.cpp \
    mbtiles.cpp \
    FlexHash.cpp \
    LLRegion.cpp \
    ocpn_pixel.cpp \
    ChartDataInputStream.cpp \
    pugixml.cpp \
    ocpndc.cpp \
    gshhs.cpp \
    piano.cpp \
    compass.cpp \
    ChInfoWin.cpp \
    Osenc.cpp \
    s57classregistrar.cpp \
    ogrs57datasource.cpp \
    s57reader.cpp \
    s57featuredefns.cpp \
    ogrs57layer.cpp \
    CanvasOptions.cpp \
    glwidget.cpp \
    CanvasConfig.cpp \
    zchxmaploadworker.cpp




win32: QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
win32: QMAKE_CFLAGS_RELEASE -= -Zc:strictStrings
win32: QMAKE_CFLAGS -= -Zc:strictStrings
win32: QMAKE_CXXFLAGS -= -Zc:strictStrings

HEADERS += \
    _def.h \
    GL/gl.h \
    GL/gl_private.h \
    GL/glext.h \
    GL/glu.h \
    zchxs57listctrlbox.h \
    zchxoptionsdlg.h \
    zchxconfig.h \
    zchxopengloptiondlg.h \
    zchxmapmainwindow.h \
    glTexCache.h \
    glTextureDescriptor.h \
    glTextureManager.h \
    s52plib.h \
    s52s57.h \
    s52utils.h \
    OCPNPlatform.h \
    FontDesc.h \
    FontMgr.h \
    chartdb.h \
    chartdbs.h \
    chartimg.h \
    chartsymbols.h \
    OCPNRegion.h \
    georef.h \
    vector2D.h \
    viewport.h \
    bitmap.h \
    glChartCanvas.h \
    bbox.h \
    chcanv.h \
    Quilt.h \
    SencManager.h \
    s57RegistrarMgr.h \
    s57chart.h \
    mygeom.h \
    cutil.h \
    TexFont.h \
    styles.h \
    mbtiles.h \
    FlexHash.h \
    LLRegion.h \
    ocpn_pixel.h \
    ChartDataInputStream.h \
    pugixml.hpp \
    ocpndc.h \
    gshhs.h \
    piano.h \
    compass.h \
    ChInfoWin.h \
    Osenc.h \
    S57ClassRegistrar.h \
    CanvasOptions.h \
    glwidget.h \
    CanvasConfig.h \
    chartbase.h \
    zchxmaploadworker.h

FORMS += \
    zchxoptionsdlg.ui \
    zchxopengloptiondlg.ui \
    zchxmapmainwindow.ui



