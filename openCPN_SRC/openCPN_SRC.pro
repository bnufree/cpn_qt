#-------------------------------------------------
#
# Project created by QtCreator 2018-12-03T13:46:36
#
#-------------------------------------------------

include(../Libs.pri)

INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}

#INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}/gdal
#INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}/nmea0183
#INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}/tinyxml
#INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}/sound
#INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}/iso8211
#INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}/SQLiteCpp
#INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}/ssl
#INCLUDEPATH += $${PWD}/texcmp/squish
#INCLUDEPATH += $${PWD}/lz4
#INCLUDEPATH += $${PWD}/mipmap
#INCLUDEPATH += $${PWD}/nmea0183
#INCLUDEPATH += $${PWD}/tinyxml
#INCLUDEPATH += $${PWD}/sound
INCLUDEPATH += ../include

LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -lgdal
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -lnmea0183
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -ltinyxml
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -lsound
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -liso8211
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -lSQLiteCPP
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -lssl_sha1
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -ltexcmp
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -llz4
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -ltess2
LIBS += -L$${OPENCPN_3RD_STATIC_LIB_PATH}\ -lmipmap
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
warning("dest:" + $$DESTDIR)
#
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
    CanvasConfig.cpp




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
    chartbase.h

FORMS += \
    zchxoptionsdlg.ui \
    zchxopengloptiondlg.ui \
    zchxmapmainwindow.ui



