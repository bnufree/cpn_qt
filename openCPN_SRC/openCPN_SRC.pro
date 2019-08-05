#-------------------------------------------------
#
# Project created by QtCreator 2018-12-03T13:46:36
#
#-------------------------------------------------

include(../Libs.pri)

INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}
INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}/nmea0183
INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}/tinyxml
INCLUDEPATH += $${OPENCPN_3RD_INCLUDE_PATH}/sound
INCLUDEPATH += $${PWD}/iso8211
INCLUDEPATH += ../include

LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -l libgdal
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -l libnmea0183
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -l libtinyxml
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -l libsound
LIBS += -L $${OPENCPN_3RD_STATIC_LIB_PATH}\ -l libiso8211

QT       += core gui network positioning core_private opengl svg xml webkitwidgets

DEFINES += ocpnUSE_GL

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = zchx_ecdis
TARGET = $$qtLibraryName($$TARGET)
TEMPLATE = lib
CONFIG += shared dll c++11

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
    bbox.h \
    CanvasConfig.h \
    CanvasOptions.h \
    chart1.h \
    chartbase.h \
    ChartDataInputStream.h \
    chartdb.h \
    chartdbs.h \
    chartimg.h \
    chartsymbols.h \
    chcanv.h \
    ChInfoWin.h \
    cm93.h \
    compass.h \
    concanv.h \
    ConfigMgr.h \
    crashprint.h \
    cutil.h \
    DarkMode.h \
    DetailSlider.h \
    dsPortType.h \
    dychart.h \
    emboss_data.h \
    FlexHash.h \
    FontDesc.h \
    FontMgr.h \
    garmin_wrapper.h \
    geodesic.h \
    georef.h \
    glChartCanvas.h \
    glTexCache.h \
    glTextureDescriptor.h \
    glTextureManager.h \
    GoToPositionDialog.h \
    gshhs.h \
    Hyperlink.h \
    IDX_entry.h \
    iENCToolbar.h \
    kml.h \
    Layer.h \
    LinkPropDlg.h \
    LLRegion.h \
    macutils.h \
    MarkIcon.h \
    mbtiles.h \
    MUIBar.h \
    mygeom.h \
    NavObjectCollection.h \
    navutil.h \
    NMEALogWindow.h \
    OCP_DataStreamInputhread.h \
    ocpCursor.h \
    OCPN_AUIManager.h \
    ocpn_pixel.h \
    ocpn_types.h \
    ocpndc.h \
    OCPNListCtrl.h \
    OCPNPlatform.h \
    OCPNRegion.h \
    ogr_s57.h \
    options.h \
    Osenc.h \
    piano.h \
    pluginmanager.h \
    PositionParser.h \
    printtable.h \
    pugiconfig.hpp \
    pugixml.hpp \
    Quilt.h \
    RolloverWin.h \
    Route.h \
    routeman.h \
    RoutePoint.h \
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
    scrollingdialog.h \
    Select.h \
    SelectItem.h \
    SencManager.h \
    SendToGpsDlg.h \
    Station_Data.h \
    styles.h \
    TC_Error_Code.h \
    TCDataFactory.h \
    TCDataSource.h \
    TCDS_Ascii_Harmonic.h \
    TCDS_Binary_Harmonic.h \
    tcmgr.h \
    TCWin.h \
    TexFont.h \
    thumbwin.h \
    tide_time.h \
    time_textbox.h \
    timers.h \
    toolbar.h \
    Track.h \
    trackprintout.h \
    TTYScroll.h \
    TTYWindow.h \
    tzdata.h \
    undo.h \
    vector2D.h \
    viewport.h \
    wificlient.h \
    WindowDestroyListener.h \
    wx28compat.h \
    GL/gl.h \
    GL/gl_private.h \
    GL/glext.h \
    GL/glu.h \
    _def.h \
    ocpn_plugin.h \
    iso8211/iso8211.h \
    opencpn_global.h \
    objectfactory.h \
    zchxoptionsdlg.h \
    zchxopengloptiondlg.h

SOURCES += \
    bbox.cpp \
    CanvasConfig.cpp \
    CanvasOptions.cpp \
    chart1.cpp \
    ChartDataInputStream.cpp \
    chartdb.cpp \
    chartdbs.cpp \
    chartimg.cpp \
    chartsymbols.cpp \
    chcanv.cpp \
    ChInfoWin.cpp \
    cm93.cpp \
    compass.cpp \
    compasswin.cpp \
    concanv.cpp \
    ConfigMgr.cpp \
    crashprint.cpp \
    cutil.cpp \
    DetailSlider.cpp \
    FlexHash.cpp \
    FontDesc.cpp \
    FontMgr.cpp \
    garmin_wrapper.cpp \
    geodesic.cpp \
    georef.cpp \
    glChartCanvas.cpp \
    glTexCache.cpp \
    glTextureDescriptor.cpp \
    glTextureManager.cpp \
    GoToPositionDialog.cpp \
    gshhs.cpp \
    Hyperlink.cpp \
    IDX_entry.cpp \
    iENCToolbar.cpp \
    kml.cpp \
    Layer.cpp \
    LinkPropDlg.cpp \
    LLRegion.cpp \
    MarkInfo.cpp \
    mbtiles.cpp \
    MUIBar.cpp \
    multiplexer.cpp \
    mygeom.cpp \
    NavObjectCollection.cpp \
    navutil.cpp \
    NMEALogWindow.cpp \
    OCP_DataStreamInputhread.cpp \
    ocpCursor.cpp \
    OCPN_AUIManager.cpp \
    ocpn_pixel.cpp \
    ocpndc.cpp \
    OCPNListCtrl.cpp \
    OCPNPlatform.cpp \
    OCPNRegion.cpp \
    ogrs57datasource.cpp \
    ogrs57layer.cpp \
    options.cpp \
    Osenc.cpp \
    piano.cpp \
    pluginmanager.cpp \
    PositionParser.cpp \
    printtable.cpp \
    pugixml.cpp \
    Quilt.cpp \
    RolloverWin.cpp \
    Route.cpp \
    routeman.cpp \
    routemanagerdialog.cpp \
    RoutePoint.cpp \
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
    scrollingdialog.cpp \
    Select.cpp \
    SelectItem.cpp \
    SencManager.cpp \
    SendToGpsDlg.cpp \
    Station_Data.cpp \
    styles.cpp \
    TCDataFactory.cpp \
    TCDataSource.cpp \
    TCDS_Ascii_Harmonic.cpp \
    TCDS_Binary_Harmonic.cpp \
    tcmgr.cpp \
    TCWin.cpp \
    TexFont.cpp \
    thumbwin.cpp \
    toolbar.cpp \
    Track.cpp \
    trackprintout.cpp \
    TrackPropDlg.cpp \
    TTYScroll.cpp \
    TTYWindow.cpp \
    undo.cpp \
    viewport.cpp \
    wificlient.cpp \
    macutils.c \
    ocpnhelper.c \
    iso8211/ddffield.cpp \
    iso8211/ddffielddefn.cpp \
    iso8211/ddfmodule.cpp \
    iso8211/ddfrecord.cpp \
    iso8211/ddfrecordindex.cpp \
    iso8211/ddfsubfielddefn.cpp \
    iso8211/ddfutils.cpp \
    zchxoptionsdlg.cpp \
    zchxopengloptiondlg.cpp

OBJECTIVE_SOURCES += \
    DarkMode.mm

FORMS += \
    sendtogpsdlg.ui \
    zchxoptionsdlg.ui \
    zchxopengloptiondlg.ui

