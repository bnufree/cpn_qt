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

LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -lgdal
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -lnmea0183
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -ltinyxml
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -lsound
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -liso8211
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -lSQLiteCPP
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -lssl_sha1
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -ltexcmp
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -llz4
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -ltess2
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -lmipmap
LIBS += -lpsapi

QT       += core gui network positioning core_private opengl svg xml webkitwidgets multimedia

DEFINES += ocpnUSE_GL TIXML_USE_STL __WXQT__

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







HEADERS += \
#    bbox.h \
#    CanvasConfig.h \
#    CanvasOptions.h \
#    chart1.h \
#    chartbase.h \
    ChartDataInputStream.h \
    chartdb.h \
    chartdbs.h \
    chartimg.h \
    chartsymbols.h \
    chcanv.h \
#    ChInfoWin.h \
#    compass.h \
#    concanv.h \
#    ConfigMgr.h \
#    crashprint.h \
#    cutil.h \
#    DarkMode.h \
#    DetailSlider.h \
#    dsPortType.h \
#    dychart.h \
#    emboss_data.h \
#    FlexHash.h \
#    FontDesc.h \
#    FontMgr.h \
#    garmin_wrapper.h \
#    geodesic.h \
#    georef.h \
    glChartCanvas.h \
    glTexCache.h \
    glTextureDescriptor.h \
    glTextureManager.h \
#    GoToPositionDialog.h \
#    gshhs.h \
#    Hyperlink.h \
#    IDX_entry.h \
#    iENCToolbar.h \
#    kml.h \
#    Layer.h \
#    LinkPropDlg.h \
#    LLRegion.h \
#    macutils.h \
#    MarkIcon.h \
#    MUIBar.h \
    mygeom.h \
#    NavObjectCollection.h \
#    NMEALogWindow.h \
#    OCP_DataStreamInputhread.h \
#    ocpCursor.h \
#    OCPN_AUIManager.h \
    ocpn_pixel.h \
#    ocpn_types.h \
#    ocpndc.h \
#    OCPNListCtrl.h \
     OCPNPlatform.h \
    OCPNRegion.h \
    ogr_s57.h \
#    options.h \
    Osenc.h \
#    piano.h \
#    pluginmanager.h \
#    PositionParser.h \
#    printtable.h \
#    pugiconfig.hpp \
#    pugixml.hpp \
    Quilt.h \
#    RolloverWin.h \
#    Route.h \
#    routeman.h \
#    RoutePoint.h \
    s52plib.h \
    s52s57.h \
    s52utils.h \
    s57.h \
    s57chart.h \
    S57ClassRegistrar.h \
    S57Light.h \
    s57mgr.h \
    S57ObjectDesc.h \
    S57QueryDialog.h \
    s57RegistrarMgr.h \
    S57Sector.h \
#    scrollingdialog.h \
#    Select.h \
#    SelectItem.h \
#    SencManager.h \
#    SendToGpsDlg.h \
#    Station_Data.h \
#    styles.h \
#    TC_Error_Code.h \
#    TCDataFactory.h \
#    TCDataSource.h \
#    TCDS_Ascii_Harmonic.h \
#    TCDS_Binary_Harmonic.h \
#    tcmgr.h \
#    TCWin.h \
#    TexFont.h \
    thumbwin.h \
#    tide_time.h \
#    time_textbox.h \
#    timers.h \
#    toolbar.h \
#    Track.h \
#    trackprintout.h \
#    TTYScroll.h \
#    TTYWindow.h \
#    tzdata.h \
#    undo.h \
#    vector2D.h \
#    viewport.h \
#    wificlient.h \
#    WindowDestroyListener.h \
#    wx28compat.h \
    GL/gl.h \
    GL/gl_private.h \
    GL/glext.h \
    GL/glu.h \
    _def.h \
#    ocpn_plugin.h \
    opencpn_global.h \
    objectfactory.h \
    zchxoptionsdlg.h \
    zchxopengloptiondlg.h \
#    zchxchecklistwidget.h \
#    zchxframe.h \
    zchxmapmainwindow.h \
    zchxconfig.h \
    zchxs57listctrlbox.h \
    cm93.h \
    mbtiles.h \
    zchxpainter.h \
    bitmap.h

SOURCES += \
#    bbox.cpp \
#    CanvasConfig.cpp \
#    CanvasOptions.cpp \
#    chart1.cpp \
    ChartDataInputStream.cpp \
    chartdb.cpp \
    chartdbs.cpp \
    chartimg.cpp \
    chartsymbols.cpp \
    chcanv.cpp \
#    ChInfoWin.cpp \
#    compass.cpp \
#    compasswin.cpp \
#    concanv.cpp \
#    ConfigMgr.cpp \
#    crashprint.cpp \
#    cutil.cpp \
#    DetailSlider.cpp \
#    FlexHash.cpp \
#    FontDesc.cpp \
#    FontMgr.cpp \
#    garmin_wrapper.cpp \
#    geodesic.cpp \
#    georef.cpp \
    glChartCanvas.cpp \
    glTexCache.cpp \
    glTextureDescriptor.cpp \
    glTextureManager.cpp \
#    GoToPositionDialog.cpp \
#    gshhs.cpp \
#    Hyperlink.cpp \
#    IDX_entry.cpp \
#    iENCToolbar.cpp \
#    kml.cpp \
#    Layer.cpp \
#    LinkPropDlg.cpp \
#    LLRegion.cpp \
#    MarkInfo.cpp \
#    MUIBar.cpp \
#    multiplexer.cpp \
    mygeom.cpp \
#    NavObjectCollection.cpp \
#    NMEALogWindow.cpp \
#    OCP_DataStreamInputhread.cpp \
#    ocpCursor.cpp \
#    OCPN_AUIManager.cpp \
    ocpn_pixel.cpp \
#    ocpndc.cpp \
#    OCPNListCtrl.cpp \
    OCPNPlatform.cpp \
    OCPNRegion.cpp \
    ogrs57datasource.cpp \
    ogrs57layer.cpp \
#    options.cpp \
    Osenc.cpp \
#    piano.cpp \
#    pluginmanager.cpp \
#    PositionParser.cpp \
#    printtable.cpp \
#    pugixml.cpp \
    Quilt.cpp \
#    RolloverWin.cpp \
#    Route.cpp \
#    routeman.cpp \
#    routemanagerdialog.cpp \
#    RoutePoint.cpp \
    s52cnsy.cpp \
    s52plib.cpp \
    s52utils.cpp \
    s57chart.cpp \
    s57classregistrar.cpp \
    s57featuredefns.cpp \
    s57mgr.cpp \
    s57obj.cpp \
    S57QueryDialog.cpp \
    s57reader.cpp \
    s57RegistrarMgr.cpp \
#    scrollingdialog.cpp \
#    Select.cpp \
#    SelectItem.cpp \
#    SencManager.cpp \
#    SendToGpsDlg.cpp \
#    Station_Data.cpp \
#    styles.cpp \
#    TCDataFactory.cpp \
#    TCDataSource.cpp \
#    TCDS_Ascii_Harmonic.cpp \
#    TCDS_Binary_Harmonic.cpp \
#    tcmgr.cpp \
#    TCWin.cpp \
#    TexFont.cpp \
    thumbwin.cpp \
#    toolbar.cpp \
#    Track.cpp \
#    trackprintout.cpp \
#    TrackPropDlg.cpp \
#    TTYScroll.cpp \
#    TTYWindow.cpp \
#    undo.cpp \
#    viewport.cpp \
#    wificlient.cpp \
#    macutils.c \
#    ocpnhelper.c \
    zchxoptionsdlg.cpp \
    zchxopengloptiondlg.cpp \
#    zchxchecklistwidget.cpp \
#    zchxframe.cpp \
    main.cpp \
    zchxmapmainwindow.cpp \
    zchxconfig.cpp \
    zchxs57listctrlbox.cpp \
    cm93.cpp \
    mbtiles.cpp \
    zchxpainter.cpp \
    bitmap.cpp

OBJECTIVE_SOURCES += \
    DarkMode.mm

FORMS += \
#    sendtogpsdlg.ui \
    zchxoptionsdlg.ui \
    zchxopengloptiondlg.ui \
    zchxmapmainwindow.ui

