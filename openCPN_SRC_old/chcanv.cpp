﻿/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Canvas
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/
#include "config.h"
#include "chcanv.h"
#include "chartdbs.h"
#include "zchxconfig.h"
#include "chartdb.h"

#include "config.h"
#include "dychart.h"
#include "OCPNPlatform.h"

#ifdef __WXOSX__
#include "DarkMode.h"
#endif


#include "chcanv.h"
#include "geodesic.h"
#include "styles.h"
#include "thumbwin.h"
#include "chartdb.h"
#include "chartimg.h"
#include "cutil.h"
#include "ocpn_pixel.h"
#include "ocpndc.h"
#include "timers.h"
#include "glTextureDescriptor.h"
#include "ChInfoWin.h"
#include "Quilt.h"
#include "SelectItem.h"
#include "Select.h"
#include "FontMgr.h"
#include "SendToGpsDlg.h"
#include "compass.h"
#include "OCPNRegion.h"
#include "gshhs.h"
#include "wx28compat.h"
#include "CanvasConfig.h"
#include "CanvasOptions.h"
#include "mbtiles.h"
#include "glChartCanvas.h"
#include "zchxmapmainwindow.h"

#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"
#include "s52utils.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QMenu>
#include <QMessageBox>
#include "glwidget.h"
#include <QVBoxLayout>
#include <QProgressDialog>


extern float  g_ChartScaleFactorExp;
extern float  g_ShipScaleFactorExp;
QString                             ChartListFileName;
extern int                          g_restore_dbindex;
extern int                       g_memCacheLimit;

#include <vector>
//#include <wx-3.0/wx/aui/auibar.h>

#if defined(__MSVC__) &&  (_MSC_VER < 1700) 
#define  trunc(d) ((d>0) ? floor(d) : ceil(d))
#endif

//  Define to enable the invocation of a temporary menubar by pressing the Alt key.
//  Not implemented for Windows XP, as it interferes with Alt-Tab processing.
#define OCPN_ALT_MENUBAR 1


//    Profiling support
//#include "/usr/include/valgrind/callgrind.h"

// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------
extern bool G_FloatPtInPolygon ( MyFlPoint *rgpts, int wnumpts, float x, float y ) ;
extern void catch_signals(int signo);

extern void AlphaBlending( ocpnDC& dc, int x, int y, int size_x, int size_y, float radius,
                           QColor color, unsigned char transparency );

extern ChartBase        *Current_Vector_Ch;
extern double           g_ChartNotRenderScaleFactor;
extern double           gLat, gLon, gCog, gSog, gHdt;
//extern double           vLat, vLon;
extern ChartDB          *ChartData;
bool             bDBUpdateInProgress;
extern ColorScheme      global_color_scheme;
extern int              g_nbrightness;

//extern ConsoleCanvas    *console;

//extern zchxConfig         *pConfig;
//extern Select           *pSelect;
//extern ThumbWin         *pthumbwin;
//extern TCMgr            *ptcmgr;
extern bool             g_bConfirmObjectDelete;

//extern IDX_entry        *gpIDX;
extern int               gpIDXn;
extern int              g_iDistanceFormat;

//extern GoToPositionDialog *pGoToPositionDialog;
extern QString GetLayerName(int id);
extern bool             g_bsimplifiedScalebar;

extern bool             bDrawCurrentValues;

extern s52plib          *ps52plib;
//extern CM93OffsetDialog  *g_pCM93OffsetDialog;

extern bool             bGPSValid;
//extern bool             g_bShowOutlines;
//extern bool             g_bShowDepthUnits;
extern bool             g_bTempShowMenuBar;
extern bool             g_bShowMenuBar;
extern bool             g_bShowCompassWin;

extern bool             g_bShowAreaNotices;
extern int              g_Show_Target_Name_Scale;

extern zchxMapMainWindow          *gFrame;

extern int              g_iNavAidRadarRingsNumberVisible;
extern float            g_fNavAidRadarRingsStep;
extern int              g_pNavAidRadarRingsStepUnits;
extern bool             g_bWayPointPreventDragging;
extern bool             g_bEnableZoomToCursor;
extern bool             g_bShowChartBar;
extern bool             g_bInlandEcdis;

extern bool             g_bDarkDecorations;


extern int              g_S57_dialog_sx, g_S57_dialog_sy;

//extern PopUpDSlide       *pPopupDetailSlider;
extern bool             g_bShowDetailSlider;
extern int              g_detailslider_dialog_x, g_detailslider_dialog_y;
extern int              g_cm93_zoom_factor;

extern bool             g_b_overzoom_x;                      // Allow high overzoom
//extern bool             g_bDisplayGrid;

extern bool             g_bUseGreenShip;

extern int              g_OwnShipIconType;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;
extern double           g_n_gps_antenna_offset_y;
extern double           g_n_gps_antenna_offset_x;
extern int              g_n_ownship_min_mm;

extern bool             g_bUseRaster;
extern bool             g_bUseVector;
extern bool             g_bUseCM93;

double           g_COGAvg;               // only needed for debug....

extern int              g_click_stop;
extern double           g_ownship_predictor_minutes;
extern double           g_ownship_HDTpredictor_miles;

extern std::vector<int>      g_quilt_noshow_index_array;
extern bool              g_bquiting;

extern bool             g_bFullScreenQuilt;

extern bool             g_bsmoothpanzoom;

extern bool                    g_bDebugOGL;

extern bool             g_b_assume_azerty;

extern ChartGroupArray  *g_pGroupArray;

extern bool              g_bShowTrue, g_bShowMag;
extern bool              g_btouch;
extern bool              g_bresponsive;

extern QString         g_toolbarConfigSecondary;
extern zchxGLOptions g_GLOptions;

extern bool              g_bShowFPS;
extern double            g_gl_ms_per_frame;
extern bool              g_benable_rotate;

extern bool              g_bSpaceDropMark;
extern bool              g_bAutoHideToolbar;
extern int               g_nAutoHideToolbar;
extern bool              g_bDeferredInitDone;

//  TODO why are these static?
static int mouse_x;
static int mouse_y;
static bool mouse_leftisdown;

bool g_brouteCreating;

int r_gamma_mult;
int g_gamma_mult;
int b_gamma_mult;
int gamma_state;
bool g_brightness_init;
int   last_brightness;

extern int                     g_cog_predictor_width;
extern double           g_display_size_mm;


// LIVE ETA OPTION
extern bool                    g_bShowLiveETA;
extern double                  g_defaultBoatSpeed;
extern double                  g_defaultBoatSpeedUserUnit;

extern int              g_nAIS_activity_timer;
extern bool             g_bskew_comp;
extern float            g_compass_scalefactor;
extern int              g_COGAvgSec; // COG average period (sec.) for Course Up Mode

QGLContext             *g_pGLcontext;   //shared common context

extern bool             g_useMUI;
extern unsigned int     g_canvasConfig;
extern QString         g_lastPluginMessage;

extern ChartCanvas      *g_focusCanvas;
extern ChartCanvas      *g_overlayCanvas;

extern float            g_toolbar_scalefactor;
extern SENCThreadManager *g_SencThreadManager;

ChartCanvas                 *gMainCanvas = 0;
extern s57RegistrarMgr          *m_pRegistrarMan;

// "Curtain" mode parameters
QDialog                *g_pcurtain;
extern QString                  *pInit_Chart_Dir;

int                       g_mem_total, g_mem_used, g_mem_initial;
extern QString                  gWorldMapLocation;
QString gDefaultWorldMapLocation;

#define MIN_BRIGHT 10
#define MAX_BRIGHT 100



// Define a constructor for my canvas
ChartCanvas::ChartCanvas ( QWidget *frame, int canvasIndex ) : QWidget(frame)
{
    parent_frame = ( zchxMapMainWindow * ) frame;       // save a pointer to parent
    m_canvasIndex = canvasIndex;
    gMainCanvas = this;
    
    pscratch_bm = NULL;
    //    SetBackgroundColour ( QColor(0,0,0) );
    //    SetBackgroundStyle ( wxBG_STYLE_CUSTOM );  // on WXMSW, this prevents flashing on color scheme change

    m_groupIndex = 0;
    m_bDrawingRoute = false;
    m_bRouteEditing = false;
    m_bMarkEditing = false;
    m_bRoutePoinDragging = false;
    m_bIsInRadius = false;
    m_bMayToggleMenuBar = true;

    m_bFollow = false;
    m_bShowNavobjects = true;
    m_bAppendingRoute = false;          // was true in MSW, why??
    pThumbDIBShow = NULL;
    m_bzooming = false;
    m_b_paint_enable = false;
    m_routeState = 0;
    
    pss_overlay_bmp = NULL;
    pss_overlay_mask = NULL;
    m_bChartDragging = false;
    m_bMeasure_Active = false;
    m_bMeasure_DistCircle = false;
    m_bedge_pan = false;
    m_disable_edge_pan = false;
    m_dragoffsetSet = false;
    m_bautofind = false;
    m_bFirstAuto = true;
    m_groupIndex = 0;
    m_singleChart = NULL;
    m_bCourseUp = false;
    m_MouseDragging = false;
    
    m_vLat = 35.7421999999;
    m_vLon = 127.52430000;
    
    m_pCIWin = NULL;
    m_pFoundPoint                 = NULL;
    m_FinishRouteOnKillFocus = true;

    m_bsectors_shown              = false;
    
    m_bbrightdir = false;
    r_gamma_mult = 1;
    g_gamma_mult = 1;
    b_gamma_mult = 1;


    m_zoom_factor = 1;
    m_rotation_speed = 0;
    m_mustmove = 0;

    m_OSoffsetx = 0.;
    m_OSoffsety = 0.;

    VPoint.invalidate();

    m_glcc = NULL;
    m_focus_indicator_pix = 1;

    m_pCurrentStack = NULL;
    m_bpersistent_quilt = false;
    m_piano_ctx_menu = NULL;
    m_Compass = NULL;
    
    g_ChartNotRenderScaleFactor = 2.0;
    m_bShowScaleInStatusBar = true;
    m_bShowScaleInStatusBar = false;

    m_bShowOutlines = false;
    m_bDisplayGrid = false;
    m_bShowDepthUnits = true;
    m_encDisplayCategory = (int)STANDARD;

    m_encShowLights = true;
    m_encShowAnchor = true;
    m_encShowDataQual = false;
    m_bShowGPS = true;
    m_pQuilt = new Quilt( this );
    m_restore_dbindex = 0;
    SetQuiltMode(true);
    
    SetupGlCanvas( );
    singleClickEventIsValid = false;

    //    Build the cursors


    m_panx = m_pany = 0;
    m_panspeed = 0;

    m_curtrack_timer_msec = 10;

    m_wheelzoom_stop_oneshot = 0;
    m_last_wheel_dir = 0;
    
    m_rollover_popup_timer_msec = 20;
    
    m_b_rot_hidef = true;
    
    proute_bm = NULL;
    m_prot_bm = NULL;

    m_bCourseUp = false;
    m_bLookAhead = false;
    
    // Set some benign initial values

    m_cs = GLOBAL_COLOR_SCHEME_DAY;
    VPoint.setLat(0);
    VPoint.setLon(0);
    VPoint.setViewScalePPM(1);
    VPoint.invalidate();

    m_canvas_scale_factor = 1.;

    m_canvas_width = 1000;

    m_overzoomTextWidth = 0;
    m_overzoomTextHeight = 0;

    //    Create the default world chart
    pWorldBackgroundChart = new GSHHSChart;

    //    Create the default depth unit emboss maps
    m_pEM_Feet = NULL;
    m_pEM_Meters = NULL;
    m_pEM_Fathoms = NULL;



    m_pQuilt->EnableHighDefinitionZoom( true );

    m_pgridFont = FontMgr::Get().FindOrCreateFont( 8, "Microsoft Yahei", QFont::StyleNormal, QFont::Weight::Normal, false);

    m_bShowCompassWin = g_bShowCompassWin;

    m_Compass = new ocpnCompass(this);
    m_Compass->SetScaleFactor(g_compass_scalefactor);
    m_Compass->Show(m_bShowCompassWin);

    mDisplsyTimer = new QTimer(this);
    mDisplsyTimer->setInterval(1000);
    connect(mDisplsyTimer, SIGNAL(timeout()), this, SLOT(update()));
//    mDisplsyTimer->start();
//    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
    QTimer::singleShot(2000, this, SLOT(slotStartLoadEcdis()));
}

void ChartCanvas::slotStartLoadEcdis()
{
    buildStyle();
    initBeforeUpdateMap();
    slotInitEcidsAsDelayed();
}

void ChartCanvas::slotInitEcidsAsDelayed()
{
    //读取配置文件中保存的地图数据目录
    ArrayOfCDI ChartDirArray;
    ZCHX_CFG_INS->LoadChartDirArray( ChartDirArray );


    if( !ChartDirArray.count() )
    {
        if(QFile::exists(ChartListFileName )) QFile::remove(ChartListFileName );
    }

    if(!ChartData)  ChartData = new ChartDB( );
    ChartData->LoadBinary(ChartListFileName, ChartDirArray);
    //  Verify any saved chart database startup index
    if(g_restore_dbindex >= 0)
    {
        if(ChartData->GetChartTableEntries() == 0)
        {
            g_restore_dbindex = -1;
        } else if(g_restore_dbindex > (ChartData->GetChartTableEntries()-1))
        {
            g_restore_dbindex = 0;
        }
    }

    //  Apply the inital Group Array structure to the chart data base
    ChartData->ApplyGroupArray( g_pGroupArray );
    DoCanvasUpdate();
    ReloadVP();                  // once more, and good to go
    OCPNPlatform::Initialize_4( );
    GetglCanvas()->setUpdateAvailable(true);
    startUpdate();
}

void ChartCanvas::initBeforeUpdateMap()
{
    if(!pInit_Chart_Dir)pInit_Chart_Dir = new QString();
    g_pGroupArray = new ChartGroupArray;
    ZCHX_CFG_INS->loadMyConfig();
    StyleMgrIns->SetStyle("MUI_flat");
    if( !StyleMgrIns->IsOK() ) {
        QString logFile = QApplication::applicationDirPath() + QString("/log/opencpn.log");
        QString msg = ("Failed to initialize the user interface. ");
        msg.append("OpenCPN cannot start. ");
        msg.append("The necessary configuration files were not found. ");
        msg.append("See the log file at ").append(logFile).append(" for details.").append("\n\n");
        QMessageBox::warning(0, "Failed to initialize the user interface. ", msg);
        exit( EXIT_FAILURE );
    }
    if(g_useMUI){
        ocpnStyle::Style* style = StyleMgrIns->GetCurrentStyle();
        style->chartStatusWindowTransparent = true;
    }

    g_display_size_mm = fmax(100.0, OCPNPlatform::instance()->GetDisplaySizeMM());
    qDebug("Detected display size (horizontal): %d mm", (int) g_display_size_mm);
    // Instantiate and initialize the Config Manager
//    ConfigMgr::Get();

    //  Validate OpenGL functionality, if selected
    zchxFuncUtil::getMemoryStatus(&g_mem_total, &g_mem_initial);
    if( 0 == g_memCacheLimit ) g_memCacheLimit = (int) ( g_mem_total * 0.5 );
    g_memCacheLimit = fmin(g_memCacheLimit, 1024 * 1024); // math in kBytes, Max is 1 GB

//      Establish location and name of chart database
    ChartListFileName = ChartListFileName = QString("%1/CHRTLIST.DAT").arg(zchxFuncUtil::getDataDir());
//      Establish guessed location of chart tree
    if( pInit_Chart_Dir->isEmpty() )
    {
        pInit_Chart_Dir->append(zchxFuncUtil::getDataDir());
    }

//      Establish the GSHHS Dataset location
    gDefaultWorldMapLocation = "gshhs";
    gDefaultWorldMapLocation.insert(0, QString("%1/").arg(zchxFuncUtil::getDataDir()));
//    gDefaultWorldMapLocation.Append( wxFileName::GetPathSeparator() );
    if( gWorldMapLocation.isEmpty() || !(QDir(gWorldMapLocation).exists()) ) {
        gWorldMapLocation = gDefaultWorldMapLocation;
    }
    qDebug()<<gWorldMapLocation<<gDefaultWorldMapLocation;
    OCPNPlatform::instance()->Initialize_2();
    InitializeUserColors();
    //  Do those platform specific initialization things that need gFrame
    OCPNPlatform::instance()->Initialize_3();


    //  Clear the cache, and thus close all charts to avoid memory leaks
    if(ChartData) ChartData->PurgeCache();

    //更新视窗的配置
    canvasConfig *config = new canvasConfig();
    config->LoadFromLegacyConfig(ZCHX_CFG_INS);
    config->canvas = this;

    // Verify that glCanvas is ready, if necessary
    if(!GetglCanvas()) SetupGlCanvas();
    config->iLat = 37.123456;
    config->iLon = 127.123456;

    SetDisplaySizeMM(g_display_size_mm);

    ApplyCanvasConfig(config);

    //            cc->SetToolbarPosition(wxPoint( g_maintoolbar_x, g_maintoolbar_y ));
    SetColorScheme( global_color_scheme );
    GetCompass()->SetScaleFactor(g_compass_scalefactor);
    SetShowGPS( true );
}

void ChartCanvas::buildStyle()
{
    ocpnStyle::Style* style = StyleMgrIns->GetCurrentStyle();
    QImage ICursorLeft = style->GetIcon(("left") ).ConvertToImage();
    QImage ICursorRight = style->GetIcon( ("right") ).ConvertToImage();
    QImage ICursorUp = style->GetIcon( ("up") ).ConvertToImage();
    QImage ICursorDown = style->GetIcon( ("down") ).ConvertToImage();
    QImage ICursorPencil = style->GetIcon( ("pencil") ).ConvertToImage();
    QImage ICursorCross = style->GetIcon( ("cross") ).ConvertToImage();

    if( !ICursorLeft.isNull() ) {
        //        ICursorLeft.set( QImage_OPTION_CUR_HOTSPOT_X, 0 );
        //        ICursorLeft.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorLeft = new QCursor(QPixmap::fromImage(ICursorLeft) );
    } else
        pCursorLeft = new QCursor( Qt::ArrowCursor );

    if( !ICursorRight.isNull() ) {
        //        ICursorRight.SetOption( QImage_OPTION_CUR_HOTSPOT_X, 31 );
        //        ICursorRight.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorRight = new QCursor( QPixmap::fromImage(ICursorRight) );
    } else
        pCursorRight = new QCursor( Qt::ArrowCursor );

    if( !ICursorUp.isNull()  ) {
        //        ICursorUp.SetOption( QImage_OPTION_CUR_HOTSPOT_X, 15 );
        //        ICursorUp.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 0 );
        pCursorUp = new QCursor( QPixmap::fromImage(ICursorUp) );
    } else
        pCursorUp = new QCursor( Qt::ArrowCursor );

    if( !ICursorDown.isNull()  ) {
        //        ICursorDown.SetOption( QImage_OPTION_CUR_HOTSPOT_X, 15 );
        //        ICursorDown.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 31 );
        pCursorDown = new QCursor( QPixmap::fromImage(ICursorDown) );
    } else
        pCursorDown = new QCursor( Qt::ArrowCursor );

    if( !ICursorPencil.isNull() ) {
        //        ICursorPencil.SetOption( QImage_OPTION_CUR_HOTSPOT_X, 0 );
        //        ICursorPencil.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorPencil = new QCursor( QPixmap::fromImage(ICursorPencil) );
    } else
        pCursorPencil = new QCursor( Qt::ArrowCursor );

    if( !ICursorCross.isNull() ) {
        //        ICursorCross.SetOption( QImage_OPTION_CUR_HOTSPOT_X, 13 );
        //        ICursorCross.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 12 );
        pCursorCross = new QCursor( QPixmap::fromImage(ICursorCross) );
    } else
        pCursorCross = new QCursor( Qt::ArrowCursor );

    pCursorArrow = new QCursor( Qt::ArrowCursor );
    pPlugIn_Cursor = NULL;

    SetCursor( *pCursorArrow );

    CreateDepthUnitEmbossMaps( GLOBAL_COLOR_SCHEME_DAY );

    m_pEM_OverZoom = NULL;
    SetOverzoomFont();
    CreateOZEmbossMapData( GLOBAL_COLOR_SCHEME_DAY );



    //    Build Dusk/Night  ownship icons
    double factor_dusk = 0.5;
    double factor_night = 0.25;

    m_b_paint_enable = true;

}

void ChartCanvas::startUpdate()
{
    if(mDisplsyTimer) mDisplsyTimer->start();
}

void ChartCanvas::stopUpdate()
{
    if(mDisplsyTimer) mDisplsyTimer->stop();
}

ChartCanvas::~ChartCanvas()
{

    delete pThumbDIBShow;

    //    Delete Cursors
    delete pCursorLeft;
    delete pCursorRight;
    delete pCursorUp;
    delete pCursorDown;
    delete pCursorArrow;
    delete pCursorPencil;
    delete pCursorCross;

//    delete m_pBrightPopup;

    delete m_pCIWin;

    delete pscratch_bm;

    delete proute_bm;

    delete pWorldBackgroundChart;
    delete pss_overlay_bmp;

    delete m_pEM_Feet;
    delete m_pEM_Meters;
    delete m_pEM_Fathoms;

    delete m_pEM_OverZoom;
    //        delete m_pEM_CM93Offset;


    delete m_prot_bm;
    delete m_glcc;
//    delete g_pGLcontext;

    delete m_pQuilt;
}

void ChartCanvas::CanvasApplyLocale()
{
    CreateDepthUnitEmbossMaps( m_cs );
    CreateOZEmbossMapData( m_cs );
}

void ChartCanvas::SetupGlCanvas( )
{
    QLayout *lay = this->layout();
    if(!lay)
    {
        lay = new QVBoxLayout(this);
        this->setLayout(lay);
    }
    qDebug("Creating glChartCanvas");
    m_glcc = new glChartCanvas( this);
    lay->addWidget(m_glcc);

    // We use one context for all GL windows, so that textures etc will be automatically shared
    if(IsPrimaryCanvas()){
        QGLContext *pctx = m_glcc->context();
        g_pGLcontext = pctx;                // Save a copy of the common context
    } else{
        m_glcc->setContext(g_pGLcontext);   // If not primary canvas, use the saved common context
    }
}


void ChartCanvas::ApplyCanvasConfig(canvasConfig *pcc)
{
    SetViewPoint( pcc->iLat, pcc->iLon, pcc->iScale, 0., pcc->iRotation );
    m_vLat = pcc->iLat;
    m_vLon = pcc->iLon;

    m_restore_dbindex = pcc->DBindex;
    m_bFollow = pcc->bFollow;
    if ( pcc->GroupID < 0 )
        pcc->GroupID = 0;

    if( pcc->GroupID > (int) g_pGroupArray->count() )
        m_groupIndex = 0;
    else
        m_groupIndex = pcc->GroupID;
    
    if( pcc->bQuilt != GetQuiltMode() )
        ToggleCanvasQuiltMode();

    SetShowDepthUnits( pcc->bShowDepthUnits );
    SetShowGrid( pcc->bShowGrid );
    SetShowOutlines( pcc->bShowOutlines );
    
    // ENC options
    SetShowENCText( pcc->bShowENCText );
    m_encDisplayCategory = pcc->nENCDisplayCategory;
    m_encShowDepth = pcc->bShowENCDepths;
    m_encShowLightDesc = pcc->bShowENCLightDescriptions;
    m_encShowBuoyLabels = pcc->bShowENCBuoyLabels;
    m_encShowLights = pcc->bShowENCLights;
    
    m_bCourseUp = pcc->bCourseUp;
    m_bLookAhead = pcc->bLookahead;
    
    m_singleChart = NULL;

}

void ChartCanvas::ApplyGlobalSettings()
{
    // GPS compas window
    m_bShowCompassWin = g_bShowCompassWin;
    if(m_Compass){
        m_Compass->Show(m_bShowCompassWin);
        if(m_bShowCompassWin)
            m_Compass->UpdateStatus();
    }
}


void ChartCanvas::CheckGroupValid( bool showMessage, bool switchGroup0)
{
    bool groupOK = CheckGroup( m_groupIndex );
    
    if(!groupOK){
        SetGroupIndex( m_groupIndex, true );
    }

}

void ChartCanvas::SetShowGPS( bool bshow )
{
    if(m_bShowGPS != bshow){
        delete m_Compass;
        m_Compass = new ocpnCompass( this, bshow );
        m_Compass->SetScaleFactor(g_compass_scalefactor);
        m_Compass->Show(m_bShowCompassWin);
    }
    m_bShowGPS = bshow;
    
}

void ChartCanvas::SetShowGPSCompassWindow( bool bshow )
{
    if(m_Compass){
        m_Compass->Show(m_bShowCompassWin);
        if(m_bShowCompassWin)
            m_Compass->UpdateStatus();
    }

}

//TODO
//extern bool     g_bLookAhead;
extern bool     g_bPreserveScaleOnX;
/*extern*/ ChartDummy *pDummyChart;
extern int      g_sticky_chart;

void ChartCanvas::canvasRefreshGroupIndex( void )
{
    SetGroupIndex(m_groupIndex);
}

void ChartCanvas::SetGroupIndex( int index, bool autoSwitch )
{
    int new_index = index;
    if( index > (int) g_pGroupArray->count() )
        new_index = 0;
    
    bool bgroup_override = false;
    int old_group_index = new_index;
    
    if( !CheckGroup( new_index ) ) {
        new_index = 0;
        bgroup_override = true;
    }
    
    if(!autoSwitch && ( index <= (int) g_pGroupArray->count()))
        new_index = index;
    
    //    Get the currently displayed chart native scale, and the current ViewPort
    int current_chart_native_scale = GetCanvasChartNativeScale();
    ViewPort vp = GetVP();
    
    m_groupIndex = new_index;

    // Are there  ENCs in this group
    if(ChartData)
        m_bENCGroup = ChartData->IsENCInGroup( m_groupIndex );
    
    //  Update the MUIBar for ENC availability
    //    if(m_muiBar)
    //        m_muiBar->SetCanvasENCAvailable(m_bENCGroup);
    
    //  Invalidate the "sticky" chart on group change, since it might not be in the new group
    g_sticky_chart = -1;
    
    //    We need a chartstack and quilt to figure out which chart to open in the new group
    UpdateCanvasOnGroupChange();

    int dbi_now = -1;
    if(GetQuiltMode())
        dbi_now = GetQuiltReferenceChartIndex();
    
    int dbi_hint = FindClosestCanvasChartdbIndex( current_chart_native_scale );
    
    // If a new reference chart is indicated, set a good scale for it.
    if((dbi_now != dbi_hint) || !GetQuiltMode()){
        double best_scale = GetBestStartScale(dbi_hint, vp);
        SetVPScale( best_scale );
    }
    
    if(GetQuiltMode())
        dbi_hint = GetQuiltReferenceChartIndex();
    
    //    Refresh the canvas, selecting the "best" chart,
    //    applying the prior ViewPort exactly
    canvasChartsRefresh( dbi_hint );

    if(!autoSwitch && bgroup_override){
        // show a short timed message box
        QString msg( ("Group \"") );

        ChartGroup *pGroup = g_pGroupArray->at( new_index - 1 );
        msg += pGroup->m_group_name;

        msg += ("\" is empty.");

        QMessageBox::information(0, "OpenCPN Group Notice", msg);
        
        return;
    }
    
    
    //    Message box is deferred so that canvas refresh occurs properly before dialog
    if( bgroup_override ) {
        QString msg(("Group \"") );

        ChartGroup *pGroup = g_pGroupArray->at( old_group_index - 1 );
        msg += pGroup->m_group_name;

        msg += ("\" is empty, switching to \"All Active Charts\" group.");

        QMessageBox::information(0, "OpenCPN Group Notice", msg);
    }
}

bool ChartCanvas::CheckGroup( int igroup )
{
    if(!ChartData)
        return true;                            //  Not known yet...

    if( igroup == 0 )
        return true;              // "all charts" is always OK

    if ( igroup < 0 )               // negative group is an error
        return false;

    ChartGroup *pGroup = g_pGroupArray->at( igroup - 1 );
    
    if( pGroup->m_element_array.empty() )   //  truly empty group prompts a warning, and auto-shift to group 0
        return false;

    for( auto& elem : pGroup->m_element_array ) {
        for( unsigned int ic = 0; ic < (unsigned int) ChartData->GetChartTableEntries(); ic++ ) {
            ChartTableEntry *pcte = ChartData->GetpChartTableEntry( ic );
            QString chart_full_path = QString::fromUtf8(pcte->GetpFullPath() );

            if( chart_full_path.startsWith(elem->m_element_name ) )
                return true;
        }
    }

    //  If necessary, check for GSHHS
    for( auto& elem : pGroup->m_element_array ) {
        QString element_root = elem->m_element_name;
        QString test_string = ("GSHH");
        if(element_root.toUpper().contains(test_string))
            return true;
    }

    return false;
}


void ChartCanvas::canvasChartsRefresh( int dbi_hint )
{
    if( !ChartData )
        return;
    
    OCPNPlatform::instance()->ShowBusySpinner();
    
    double old_scale = GetVPScale();
    InvalidateQuilt();
    SetQuiltRefChart( -1 );
    
    m_singleChart = NULL;
    
    //delete m_pCurrentStack;
    //m_pCurrentStack = NULL;
    
    //    Build a new ChartStack
    if(!m_pCurrentStack){
        m_pCurrentStack = new ChartStack;
        ChartData->BuildChartStack( m_pCurrentStack, m_vLat, m_vLon, m_groupIndex );
    }
    
    if( -1 != dbi_hint ) {
        if( GetQuiltMode() ) {
            GetpCurrentStack()->SetCurrentEntryFromdbIndex( dbi_hint );
            SetQuiltRefChart( dbi_hint );
        } else {
            //      Open the saved chart
            ChartBase *pTentative_Chart;
            pTentative_Chart = ChartData->OpenChartFromDB( dbi_hint, FULL_INIT );
            
            if( pTentative_Chart ) {
                /* m_singleChart is always NULL here, (set above) should this go before that? */
                if( m_singleChart )
                    m_singleChart->Deactivate();
                
                m_singleChart = pTentative_Chart;
                m_singleChart->Activate();
                
                GetpCurrentStack()->CurrentStackEntry = ChartData->GetStackEntry( GetpCurrentStack(),
                                                                                  m_singleChart->GetFullPath() );
            }
            //else
            //SetChartThumbnail( dbi_hint );       // need to reset thumbnail on failed chart open
        }
        
        //refresh_Piano();
    } else {
        //    Select reference chart from the stack, as though clicked by user
        //    Make it the smallest scale chart on the stack
        GetpCurrentStack()->CurrentStackEntry = GetpCurrentStack()->nEntry - 1;
        int selected_index = GetpCurrentStack()->GetCurrentEntrydbIndex();
        SetQuiltRefChart( selected_index );
    }
    
    //    Validate the correct single chart, or set the quilt mode as appropriate
    SetupCanvasQuiltMode();
    if( !GetQuiltMode() && m_singleChart == 0) {
        // use a dummy like in DoChartUpdate
        if (NULL == pDummyChart )
            pDummyChart = new ChartDummy;
        m_singleChart = pDummyChart;
        SetVPScale( old_scale );
    }
    
    ReloadVP();

    UpdateGPSCompassStatusBox( true );
    
    SetCursor( Qt::ArrowCursor );
}


bool ChartCanvas::DoCanvasUpdate( void )
{
    
    double tLat, tLon;           // Chart Stack location
    double vpLat, vpLon;         // ViewPort location
    
    bool bNewChart = false;
    bool bNewView = false;
    bool bCanvasChartAutoOpen = true;                             // debugging
    
    bool bNewPiano = false;
    bool bOpenSpecified;
    ChartStack LastStack;
    ChartBase *pLast_Ch;
    
    ChartStack WorkStack;
    
    if( bDBUpdateInProgress )
        return false;
    if( !ChartData ) return false;
    
    if(ChartData->IsBusy())
        return false;
    
    int last_nEntry = -1;
    if( m_pCurrentStack )
        last_nEntry = m_pCurrentStack->nEntry;
    
    if( m_bFollow ) {
        tLat = gLat;
        tLon = gLon;

        // Set the ViewPort center based on the OWNSHIP offset
        double dx = m_OSoffsetx;
        double dy = m_OSoffsety;
        double d_east = dx / GetVP().viewScalePPM();
        double d_north = dy / GetVP().viewScalePPM();

        fromSM( d_east, d_north, gLat, gLon, &vpLat, &vpLon );

        // on lookahead mode, adjust the vp center point
        if( m_bLookAhead && bGPSValid && !m_MouseDragging ) {
            double angle = g_COGAvg + ( GetVPRotation() * 180. / PI );
            
            double pixel_deltay = fabs( cos( angle * PI / 180. ) ) * GetCanvasHeight() / 4;
            double pixel_deltax = fabs( sin( angle * PI / 180. ) ) * GetCanvasWidth() / 4;
            
            double pixel_delta_tent = sqrt(
                        ( pixel_deltay * pixel_deltay ) + ( pixel_deltax * pixel_deltax ) );
            
            double pixel_delta = 0;
            
            //    The idea here is to cancel the effect of LookAhead for slow gSog, to avoid
            //    jumping of the vp center point during slow maneuvering, or at anchor....
            if( !std::isnan(gSog) ) {
                if( gSog < 1.0 ) pixel_delta = 0.;
                else
                    if( gSog >= 3.0 ) pixel_delta = pixel_delta_tent;
                    else
                        pixel_delta = pixel_delta_tent * ( gSog - 1.0 ) / 2.0;
            }
            
            double meters_to_shift = cos( gLat * PI / 180. ) * pixel_delta / GetVPScale();
            
            double dir_to_shift = g_COGAvg;
            
            ll_gc_ll( gLat, gLon, dir_to_shift, meters_to_shift / 1852., &vpLat, &vpLon );
        }
        else if(m_bLookAhead && !bGPSValid){
            m_OSoffsetx = 0;            // center ownship on loss of GPS
            m_OSoffsety = 0;
            vpLat = gLat;
            vpLon = gLon;
        }

        
    } else {
        tLat = m_vLat;
        tLon = m_vLon;
        vpLat = m_vLat;
        vpLon = m_vLon;
        
    }
    
    // Calculate change in VP, in pixels, using a simple SM projection
    // if change in pixels is smaller than 2% of screen size, do not change the VP
    // This will avoid "jitters" at large scale.
    if(GetVP().viewScalePPM() > 1.0){
        double easting, northing;
        toSM( GetVP().lat(), GetVP().lon(), vpLat, vpLon,  &easting, &northing );
        if( (fabs(easting * GetVP().viewScalePPM()) < (GetVP().pixWidth() * 2 / 100)) ||
                (fabs(northing * GetVP().viewScalePPM()) < (GetVP().pixHeight() * 2 / 100)) ){
            vpLat = GetVP().lat();
            vpLon = GetVP().lon();
        }
    }
    
    
    if( GetQuiltMode() ) {
        int current_db_index = -1;
        if( m_pCurrentStack )
            current_db_index = m_pCurrentStack->GetCurrentEntrydbIndex(); // capture the currently selected Ref chart dbIndex
        else
            m_pCurrentStack = new ChartStack;

        //  This logic added to enable opening a chart when there is no
        //  previous chart indication, either from inital startup, or from adding new chart directory
        if( m_bautofind && (-1 == GetQuiltReferenceChartIndex()) && m_pCurrentStack ){
            if (m_pCurrentStack->nEntry) {
                int new_dbIndex = m_pCurrentStack->GetDBIndex(m_pCurrentStack->nEntry-1);    // smallest scale
                SelectQuiltRefdbChart(new_dbIndex, true);
                m_bautofind = false;
            }
        }

        ChartData->BuildChartStack( m_pCurrentStack, tLat, tLon, m_groupIndex );
        m_pCurrentStack->SetCurrentEntryFromdbIndex( current_db_index );

        if( m_bFirstAuto ) {
            double proposed_scale_onscreen = GetCanvasScaleFactor() / GetVPScale(); // as set from config load

            int initial_db_index = m_restore_dbindex;
            if( initial_db_index < 0 ) {
                if( m_pCurrentStack->nEntry ) {
//                    initial_db_index = m_pCurrentStack->GetDBIndex( m_pCurrentStack->nEntry - 1 );
                    initial_db_index = m_pCurrentStack->GetDBIndex( 0 );
                } else
                    m_bautofind = true; //initial_db_index = 0;
            }

            if( m_pCurrentStack->nEntry ) {

                int initial_type = ChartData->GetDBChartType( initial_db_index );

                //    Check to see if the target new chart is quiltable as a reference chart

                if( !IsChartQuiltableRef( initial_db_index ) ) {
                    // If it is not quiltable, then walk the stack up looking for a satisfactory chart
                    // i.e. one that is quiltable and of the same type
                    // XXX if there's none?
                    int stack_index = 0;

                    if ( stack_index >= 0 ){
                        while( ( stack_index < m_pCurrentStack->nEntry - 1 ) ) {
                            int test_db_index = m_pCurrentStack->GetDBIndex( stack_index );
                            if( IsChartQuiltableRef( test_db_index )
                                    && ( initial_type == ChartData->GetDBChartType( initial_db_index ) ) ) {
                                initial_db_index = test_db_index;
                                break;
                            }
                            stack_index++;
                        }
                    }
                }

                ChartBase *pc = ChartData->OpenChartFromDB( initial_db_index, FULL_INIT );
                if( pc ) {
                    SetQuiltRefChart( initial_db_index );
                    m_pCurrentStack->SetCurrentEntryFromdbIndex( initial_db_index );
                }

                // Check proposed scale, see how much underzoom results
                // Adjust as necessary to prevent slow loading on initial startup
                if(pc){
                    double chartScale = pc->GetNativeScale();
                    proposed_scale_onscreen = fmin(proposed_scale_onscreen, chartScale * 4);
                }
            }

            bNewView |= SetViewPoint( vpLat, vpLon,
                                      GetCanvasScaleFactor() / proposed_scale_onscreen, 0,
                                      GetVPRotation() );

        }
        // else
        bNewView |= SetViewPoint( vpLat, vpLon, GetVPScale(), 0, GetVPRotation() );

        goto update_finish;

    }
    
    //  Single Chart Mode from here....
    pLast_Ch = m_singleChart;
    ChartTypeEnum new_open_type;
    ChartFamilyEnum new_open_family;
    if( pLast_Ch ) {
        new_open_type = pLast_Ch->GetChartType();
        new_open_family = pLast_Ch->GetChartFamily();
    } else {
        new_open_type = CHART_TYPE_KAP;
        new_open_family = CHART_FAMILY_RASTER;
    }
    
    bOpenSpecified = m_bFirstAuto;
    
    //  Make sure the target stack is valid
    if( NULL == m_pCurrentStack )
        m_pCurrentStack = new ChartStack;

    // Build a chart stack based on tLat, tLon
    if( 0 == ChartData->BuildChartStack( &WorkStack, tLat, tLon, g_sticky_chart, m_groupIndex ) ) {      // Bogus Lat, Lon?
        if( NULL == pDummyChart ) {
            pDummyChart = new ChartDummy;
            bNewChart = true;
        }
        
        if( m_singleChart ) if( m_singleChart->GetChartType() != CHART_TYPE_DUMMY ) bNewChart = true;

        m_singleChart = pDummyChart;
        
        //    If the current viewpoint is invalid, set the default scale to something reasonable.
        double set_scale = GetVPScale();
        if( !GetVP().isValid() ) set_scale = 1. / 20000.;

        bNewView |= SetViewPoint( tLat, tLon, set_scale, 0, GetVPRotation() );

        //      If the chart stack has just changed, there is new status
        if(WorkStack.nEntry && m_pCurrentStack->nEntry){
            if( !ChartData->EqualStacks( &WorkStack, m_pCurrentStack ) ) {
                bNewPiano = true;
                bNewChart = true;
            }
        }

        //      Copy the new (by definition empty) stack into the target stack
        ChartData->CopyStack( m_pCurrentStack, &WorkStack );

        goto update_finish;
    }
    
    //              Check to see if Chart Stack has changed
    if( !ChartData->EqualStacks( &WorkStack, m_pCurrentStack ) ) {
        //      New chart stack, so...
        bNewPiano = true;
        
        //      Save a copy of the current stack
        ChartData->CopyStack( &LastStack, m_pCurrentStack );
        
        //      Copy the new stack into the target stack
        ChartData->CopyStack( m_pCurrentStack, &WorkStack );
        
        //  Is Current Chart in new stack?
        
        int tEntry = -1;
        if( NULL != m_singleChart )                                  // this handles startup case
            tEntry = ChartData->GetStackEntry( m_pCurrentStack, m_singleChart->GetFullPath() );
        
        if( tEntry != -1 ) {                // m_singleChart is in the new stack
            m_pCurrentStack->CurrentStackEntry = tEntry;
            bNewChart = false;
        }
        
        else                           // m_singleChart is NOT in new stack
        {                                       // So, need to open a new chart
            //      Find the largest scale raster chart that opens OK

            ChartBase *pProposed = NULL;

            if( bCanvasChartAutoOpen ) {
                bool search_direction = false;        // default is to search from lowest to highest
                int start_index = 0;

                //    A special case:  If panning at high scale, open largest scale chart first
                if( ( LastStack.CurrentStackEntry == LastStack.nEntry - 1 ) || ( LastStack.nEntry == 0 ) ) {
                    search_direction = true;
                    start_index = m_pCurrentStack->nEntry - 1;
                }
                
                //    Another special case, open specified index on program start
                if( bOpenSpecified ) {
                    search_direction = false;
                    start_index = 0;
                    if( ( start_index < 0 ) | ( start_index >= m_pCurrentStack->nEntry ) )
                        start_index = 0;

                    new_open_type = CHART_TYPE_DONTCARE;
                }
                
                pProposed = ChartData->OpenStackChartConditional( m_pCurrentStack, start_index,
                                                                  search_direction, new_open_type, new_open_family );
                
                //    Try to open other types/families of chart in some priority
                if( NULL == pProposed ) pProposed = ChartData->OpenStackChartConditional(
                            m_pCurrentStack, start_index, search_direction, CHART_TYPE_CM93COMP,
                            CHART_FAMILY_VECTOR );
                
                if( NULL == pProposed ) pProposed = ChartData->OpenStackChartConditional(
                            m_pCurrentStack, start_index, search_direction, CHART_TYPE_CM93COMP,
                            CHART_FAMILY_RASTER );
                
                bNewChart = true;
                
            }     // bCanvasChartAutoOpen

            else
                pProposed = NULL;

            //  If no go, then
            //  Open a Dummy Chart
            if( NULL == pProposed ) {
                if( NULL == pDummyChart ) {
                    pDummyChart = new ChartDummy;
                    bNewChart = true;
                }
                
                if( pLast_Ch ) if( pLast_Ch->GetChartType() != CHART_TYPE_DUMMY ) bNewChart = true;

                pProposed = pDummyChart;
            }
            
            // Arriving here, pProposed points to an opened chart, or NULL.
            if( m_singleChart ) m_singleChart->Deactivate();
            m_singleChart = pProposed;
            
            if( m_singleChart ) {
                m_singleChart->Activate();
                m_pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( m_pCurrentStack,
                                                                               m_singleChart->GetFullPath() );
            }
        }   // need new chart
        
        // Arriving here, m_singleChart is opened and OK, or NULL
        if( NULL != m_singleChart ) {
            
            //      Setup the view using the current scale
            double set_scale = GetVPScale();
            
            //    If the current viewpoint is invalid, set the default scale to something reasonable.
            if( !GetVP().isValid() )
                set_scale = 1. / 20000.;
            else {                                    // otherwise, match scale if elected.
                double proposed_scale_onscreen;
                
                if( m_bFollow ) {          // autoset the scale only if in autofollow
                    double new_scale_ppm = m_singleChart->GetNearestPreferredScalePPM( GetVPScale() );
                    proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;
                }
                else
                    proposed_scale_onscreen = GetCanvasScaleFactor() / set_scale;
                
                
                //  This logic will bring a new chart onscreen at roughly twice the true paper scale equivalent.
                //  Note that first chart opened on application startup (bOpenSpecified = true) will open at the config saved scale
                if( bNewChart && !g_bPreserveScaleOnX && !bOpenSpecified ) {
                    proposed_scale_onscreen = m_singleChart->GetNativeScale() / 2;
                    double equivalent_vp_scale = GetCanvasScaleFactor()
                            / proposed_scale_onscreen;
                    double new_scale_ppm = m_singleChart->GetNearestPreferredScalePPM(
                                equivalent_vp_scale );
                    proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;
                }

                if( m_bFollow ) {     // bounds-check the scale only if in autofollow
                    proposed_scale_onscreen =
                            fmin(proposed_scale_onscreen, m_singleChart->GetNormalScaleMax(GetCanvasScaleFactor(), GetCanvasWidth()));
                    proposed_scale_onscreen =
                            fmax(proposed_scale_onscreen, m_singleChart->GetNormalScaleMin(GetCanvasScaleFactor(), g_b_overzoom_x));
                }

                set_scale = GetCanvasScaleFactor() / proposed_scale_onscreen;
            }
            
            bNewView |= SetViewPoint( vpLat, vpLon, set_scale,
                                      m_singleChart->GetChartSkew() * PI / 180., GetVPRotation() );
            
        }
    }         // new stack
    
    else                                                                 // No change in Chart Stack
    {
        if( ( m_bFollow ) && m_singleChart )
            bNewView |= SetViewPoint( vpLat, vpLon, GetVPScale(), m_singleChart->GetChartSkew() * PI / 180., GetVPRotation() );
    }
    
update_finish:
    
    //    Ask for a new tool bar if the stack is going to or coming from only one entry.
    //     if(GetToolbar()){
    //         if(m_pCurrentStack){
    //            bool toolbar_scale_tools_show = m_pCurrentStack && m_pCurrentStack->b_valid && ( m_pCurrentStack->nEntry > 1 );
    //            bool scale_tools_shown = m_toolBar->m_toolbar_scale_tools_shown;

    //            if( toolbar_scale_tools_show != scale_tools_shown){
    //                if( !m_bFirstAuto )
    //                    RequestNewCanvasToolbar( false );
    //            }
    //         }
    //     }

    //TODO
    //     if( bNewPiano ) UpdateControlBar();

    //  Update the ownship position on thumbnail chart, if shown
    //    if( pthumbwin && pthumbwin->IsShown() ) {
    //        if( pthumbwin->pThumbChart ){
    //            if( pthumbwin->pThumbChart->UpdateThumbData( gLat, gLon ) )
    //                pthumbwin->Refresh( TRUE );
    //        }
    //    }
    
    m_bFirstAuto = false;                           // Auto open on program start
    
    //  If we need a Refresh(), do it here...
    //  But don't duplicate a Refresh() done by SetViewPoint()
    if( bNewChart && !bNewView )
        Refresh( false );

#ifdef ocpnUSE_GL
    // If a new chart, need to invalidate gl viewport for refresh
    // so the fbo gets flushed
    if(m_glcc && bNewChart)
        GetglCanvas()->Invalidate();
#endif

    return bNewChart | bNewView;
}

void ChartCanvas::SelectQuiltRefdbChart( int db_index, bool b_autoscale )
{
    if( m_pCurrentStack )
        m_pCurrentStack->SetCurrentEntryFromdbIndex( db_index );
    
    SetQuiltRefChart( db_index );
    if (ChartData) {
        ChartBase *pc = ChartData->OpenChartFromDB( db_index, FULL_INIT );
        if( pc ) {
            if(b_autoscale) {
                double best_scale_ppm = GetBestVPScale( pc );
                SetVPScale( best_scale_ppm );
            }
        }
        else
            SetQuiltRefChart( -1 );
    }
    else
        SetQuiltRefChart( -1 );
}

void ChartCanvas::SelectQuiltRefChart( int selected_index )
{
    std::vector<int>  piano_chart_index_array = GetQuiltExtendedStackdbIndexArray();
    int current_db_index = piano_chart_index_array[selected_index];
    
    SelectQuiltRefdbChart( current_db_index );
}

double ChartCanvas::GetBestVPScale( ChartBase *pchart )
{
    if( pchart ) {
        double proposed_scale_onscreen = GetCanvasScaleFactor() / GetVPScale();

        if( ( g_bPreserveScaleOnX ) || ( CHART_TYPE_CM93COMP == pchart->GetChartType() ) ) {
            double new_scale_ppm = GetVPScale();
            proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;
        } else {
            //  This logic will bring the new chart onscreen at roughly twice the true paper scale equivalent.
            proposed_scale_onscreen = pchart->GetNativeScale() / 2;
            double equivalent_vp_scale = GetCanvasScaleFactor() / proposed_scale_onscreen;
            double new_scale_ppm = pchart->GetNearestPreferredScalePPM( equivalent_vp_scale );
            proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;
        }

        // Do not allow excessive underzoom, even if the g_bPreserveScaleOnX flag is set.
        // Otherwise, we get severe performance problems on all platforms

        double max_underzoom_multiplier = 2.0;
        if(GetVP().quilt()){
            double scale_max = m_pQuilt->GetNomScaleMin(pchart->GetNativeScale(), pchart->GetChartType(), pchart->GetChartFamily());
            max_underzoom_multiplier = scale_max / pchart->GetNativeScale();
        }

        proposed_scale_onscreen =
                fmin(proposed_scale_onscreen,
                     pchart->GetNormalScaleMax(GetCanvasScaleFactor(), GetCanvasWidth()) * max_underzoom_multiplier);

        //  And, do not allow excessive overzoom either
        proposed_scale_onscreen =
                fmax(proposed_scale_onscreen, pchart->GetNormalScaleMin(GetCanvasScaleFactor(), false));

        return GetCanvasScaleFactor() / proposed_scale_onscreen;
    } else
        return 1.0;
}

void ChartCanvas::SetupCanvasQuiltMode( void )
{
    
    if( GetQuiltMode() )                               // going to quilt mode
    {
        ChartData->LockCache();
        
        //    Select the proper Ref chart
        int target_new_dbindex = -1;
        if( m_pCurrentStack ) {
            target_new_dbindex = GetQuiltReferenceChartIndex();    //m_pCurrentStack->GetCurrentEntrydbIndex();
            
            if(-1 != target_new_dbindex){
                if( !IsChartQuiltableRef( target_new_dbindex ) ){
                    
                    int proj = ChartData->GetDBChartProj(target_new_dbindex);
                    int type = ChartData->GetDBChartType(target_new_dbindex);
                    
                    // walk the stack up looking for a satisfactory chart
                    int stack_index = m_pCurrentStack->CurrentStackEntry;
                    
                    while((stack_index < m_pCurrentStack->nEntry-1) && (stack_index >= 0)) {
                        int proj_tent = ChartData->GetDBChartProj( m_pCurrentStack->GetDBIndex(stack_index));
                        int type_tent = ChartData->GetDBChartType( m_pCurrentStack->GetDBIndex(stack_index));
                        
                        if(IsChartQuiltableRef(m_pCurrentStack->GetDBIndex(stack_index))){
                            if((proj == proj_tent) && (type_tent == type)){
                                target_new_dbindex = m_pCurrentStack->GetDBIndex(stack_index);
                                break;
                            }
                        }
                        stack_index++;
                    }
                }
            }
        }
        
        if( IsChartQuiltableRef( target_new_dbindex ) )
            SelectQuiltRefdbChart( target_new_dbindex, false );        // Try not to allow a scale change
        else
            SelectQuiltRefdbChart( -1, false );

        m_singleChart = NULL;                  // Bye....

        //TODOSetChartThumbnail( -1 );            //Turn off thumbnails for sure

        //  Re-qualify the quilt reference chart selection
        AdjustQuiltRefChart(  );
        
        //  Restore projection type saved on last quilt mode toggle
        //TODO
        //             if(g_sticky_projection != -1)
        //                 GetVP().SetProjectionType(g_sticky_projection);
        //             else
        //                 GetVP().SetProjectionType(PROJECTION_MERCATOR);
        GetVP().setProjectionType(PROJECTION_UNKNOWN);

    }
    
    //    When shifting from quilt to single chart mode, select the "best" single chart to show
    if( !GetQuiltMode() ) {
        if( ChartData && ChartData->IsValid() ) {
            UnlockQuilt();
            
            double tLat, tLon;
            if( m_bFollow == true ) {
                tLat = gLat;
                tLon = gLon;
            } else {
                tLat = m_vLat;
                tLon = m_vLon;
            }
            
            if( !m_singleChart ) {
                
                // Build a temporary chart stack based on tLat, tLon
                ChartStack TempStack;
                ChartData->BuildChartStack( &TempStack, tLat, tLon, g_sticky_chart, m_groupIndex );
                
                //    Iterate over the quilt charts actually shown, looking for the largest scale chart that will be in the new chartstack....
                //    This will (almost?) always be the reference chart....
                
                ChartBase *Candidate_Chart = NULL;
                int cur_max_scale = (int) 1e8;
                
                ChartBase *pChart = GetFirstQuiltChart();
                while( pChart ) {
                    //  Is this pChart in new stack?
                    int tEntry = ChartData->GetStackEntry( &TempStack, pChart->GetFullPath() );
                    if( tEntry != -1 ) {
                        if( pChart->GetNativeScale() < cur_max_scale ) {
                            Candidate_Chart = pChart;
                            cur_max_scale = pChart->GetNativeScale();
                        }
                    }
                    pChart = GetNextQuiltChart();
                }
                
                m_singleChart = Candidate_Chart;
                
                //    If the quilt is empty, there is no "best" chart.
                //    So, open the smallest scale chart in the current stack
                if( NULL == m_singleChart ) {
                    m_singleChart = ChartData->OpenStackChartConditional( &TempStack,
                                                                          TempStack.nEntry - 1, true, CHART_TYPE_DONTCARE,
                                                                          CHART_FAMILY_DONTCARE );
                }
            }
            
            //  Invalidate all the charts in the quilt,
            // as any cached data may be region based and not have fullscreen coverage
            InvalidateAllQuiltPatchs();
            if( m_singleChart ) {
                GetVP().setProjectionType(m_singleChart->GetChartProjectionType());
            }
            
        }
        //    Invalidate the current stack so that it will be rebuilt on next tick
        if( m_pCurrentStack )
            m_pCurrentStack->b_valid = false;
    }
    
}


bool ChartCanvas::IsTempMenuBarEnabled()
{
    return true;
}

double ChartCanvas::GetCanvasRangeMeters()
{
    int wid = width();
    int hei = height();
    int minDimension =  fmin(wid, hei);
    
    double range  = (minDimension / GetVP().viewScalePPM())/2;
    range *= cos(GetVP().lat() *PI/180.);
    return range;
}

void ChartCanvas::SetCanvasRangeMeters( double range )
{
    int wid = width();
    int hei = height();
    int minDimension =  fmin(wid, hei);
    
    double scale_ppm = minDimension / (range / cos(GetVP().lat() *PI/180.));
    SetVPScale( scale_ppm / 2 );
    
}


void ChartCanvas::SetDisplaySizeMM( double size )
{
    m_display_size_mm = size;
    
    int sx = width(), sy = height();
    
    double max_physical = fmax(sx, sy);
    
    m_pix_per_mm = ( max_physical ) / ( (double) m_display_size_mm );
    m_canvas_scale_factor = ( max_physical ) / (m_display_size_mm /1000.);
    
    
    if( ps52plib )
        ps52plib->SetPPMM( m_pix_per_mm );
    
    qDebug("Metrics:  m_display_size_mm: %g     wxDisplaySize:  %d:%d   ", m_display_size_mm, sx, sy);
    
    m_focus_indicator_pix = std::round(1 * GetPixPerMM());

}

void ChartCanvas::InvalidateGL()
{
    if(!m_glcc)  return;
    m_glcc->Invalidate();
    if(m_Compass)   m_Compass->UpdateStatus( true );
}

int ChartCanvas::GetCanvasChartNativeScale()
{
    int ret = 1;
    if( !VPoint.quilt() ) {
        if( m_singleChart ) ret = m_singleChart->GetNativeScale();
    } else
        ret = (int) m_pQuilt->GetRefNativeScale();

    return ret;

}

ChartBase* ChartCanvas::GetChartAtCursor() {
    ChartBase* target_chart;
    if( m_singleChart && ( m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR ) )
        target_chart = m_singleChart;
    else
        if( VPoint.quilt() )
            target_chart = m_pQuilt->GetChartAtPix( VPoint, QPoint( mouse_x, mouse_y ) );
        else
            target_chart = NULL;
    return target_chart;
}

ChartBase* ChartCanvas::GetOverlayChartAtCursor() {
    ChartBase* target_chart;
    if( VPoint.quilt() )
        target_chart = m_pQuilt->GetOverlayChartAtPix( VPoint, QPoint( mouse_x, mouse_y ) );
    else
        target_chart = NULL;
    return target_chart;
}

int ChartCanvas::FindClosestCanvasChartdbIndex( int scale )
{
    int new_dbIndex = -1;
    if( !VPoint.quilt() ) {
        if( m_pCurrentStack ) {
            for( int i = 0; i < m_pCurrentStack->nEntry; i++ ) {
                int sc = ChartData->GetStackChartScale( m_pCurrentStack, i, NULL, 0 );
                if( sc >= scale ) {
                    new_dbIndex = m_pCurrentStack->GetDBIndex( i );
                    break;
                }
            }
        }
    } else {
        //    Using the current quilt, select a useable reference chart
        //    Said chart will be in the extended (possibly full-screen) stack,
        //    And will have a scale equal to or just greater than the stipulated value
        unsigned int im = m_pQuilt->GetExtendedStackIndexArray().size();
        if( im > 0 ) {
            for( unsigned int is = 0; is < im; is++ ) {
                const ChartTableEntry &m = ChartData->GetChartTableEntry(
                            m_pQuilt->GetExtendedStackIndexArray()[is] );
                if( ( m.Scale_ge(scale ) )/* && (m_reference_family == m.GetChartFamily())*/) {
                    new_dbIndex = m_pQuilt->GetExtendedStackIndexArray()[is];
                    break;
                }
            }
        }
    }

    return new_dbIndex;
}

void ChartCanvas::EnablePaint(bool b_enable)
{
    m_b_paint_enable = b_enable;
    if(m_glcc)
        m_glcc->EnablePaint(b_enable);
}

bool ChartCanvas::IsQuiltDelta()
{
    return m_pQuilt->IsQuiltDelta( VPoint );
}

void ChartCanvas::UnlockQuilt()
{
    m_pQuilt->UnlockQuilt();
}

std::vector<int>  ChartCanvas::GetQuiltIndexArray( void )
{
    return m_pQuilt->GetQuiltIndexArray();;
}

void ChartCanvas::SetQuiltMode( bool quilt )
{
    VPoint.setQuilt(quilt);
    VPoint.setFullScreenQuilt(g_bFullScreenQuilt);
}

bool ChartCanvas::GetQuiltMode( void )
{
    return VPoint.quilt();
}

int ChartCanvas::GetQuiltReferenceChartIndex(void)
{
    return m_pQuilt->GetRefChartdbIndex();
}

void ChartCanvas::InvalidateAllQuiltPatchs( void )
{
    m_pQuilt->InvalidateAllQuiltPatchs();
}

ChartBase *ChartCanvas::GetLargestScaleQuiltChart()
{
    return m_pQuilt->GetLargestScaleChart();
}

ChartBase *ChartCanvas::GetFirstQuiltChart()
{
    return m_pQuilt->GetFirstChart();
}

ChartBase *ChartCanvas::GetNextQuiltChart()
{
    return m_pQuilt->GetNextChart();
}

int ChartCanvas::GetQuiltChartCount()
{
    return m_pQuilt->GetnCharts();
}

void ChartCanvas::SetQuiltChartHiLiteIndex( int dbIndex )
{
    m_pQuilt->SetHiliteIndex( dbIndex );
}

std::vector<int>  ChartCanvas::GetQuiltCandidatedbIndexArray( bool flag1, bool flag2 )
{
    return m_pQuilt->GetCandidatedbIndexArray( flag1, flag2 );
}

int ChartCanvas::GetQuiltRefChartdbIndex( void )
{
    return m_pQuilt->GetRefChartdbIndex();
}

std::vector<int>  ChartCanvas::GetQuiltExtendedStackdbIndexArray()
{
    return m_pQuilt->GetExtendedStackIndexArray();
}

std::vector<int>  ChartCanvas::GetQuiltEclipsedStackdbIndexArray()
{
    return m_pQuilt->GetEclipsedStackIndexArray();
}

void ChartCanvas::InvalidateQuilt( void )
{
    return m_pQuilt->Invalidate();
}

double ChartCanvas::GetQuiltMaxErrorFactor()
{
    return m_pQuilt->GetMaxErrorFactor();
}

bool ChartCanvas::IsChartQuiltableRef( int db_index )
{
    return m_pQuilt->IsChartQuiltableRef( db_index );
}

bool ChartCanvas::IsChartLargeEnoughToRender( ChartBase* chart, ViewPort& vp )
{
    double chartMaxScale = chart->GetNormalScaleMax( GetCanvasScaleFactor(), GetCanvasWidth() );
    return ( chartMaxScale*g_ChartNotRenderScaleFactor > vp.chartScale() );
}

void ChartCanvas::StartMeasureRoute()
{
    if( !m_routeState ) {  // no measure tool if currently creating route
        m_bMeasure_Active = true;
        m_nMeasureState = 1;
        m_bDrawingRoute = false;

        SetCursor( *pCursorPencil );
        Refresh();
    }
}

void ChartCanvas::CancelMeasureRoute()
{
    m_bMeasure_Active = false;
    m_nMeasureState = 0;
    m_bDrawingRoute = false;
    SetCursor( *pCursorArrow );
}

ViewPort &ChartCanvas::GetVP()
{
    return VPoint;
}

void ChartCanvas::SetVP(ViewPort &vp)
{
    VPoint = vp;
}

// void ChartCanvas::SetFocus()
// {
//     printf("set %d\n", m_canvasIndex);
//     //wxWindow:SetFocus();
// }



void ChartCanvas::keyPressEvent(QKeyEvent *event)
{
    m_modkeys = event->modifiers();
    int key_char = event->key();

    bool b_handled = false;
    //处理旋转
    if(g_benable_rotate && m_modkeys == Qt::NoModifier)
    {
        b_handled = true;
        switch( key_char )
        {
        case ']':
            RotateCanvas( 1 );
            break;

        case '[':
            RotateCanvas( -1 );
            break;

        case '\\':
            DoRotateCanvas(0);
            break;
        default:
            b_handled = false;
            break;
        }
    }

    int panspeed = (m_modkeys == Qt::AltModifier ? 2 : 100);
    // HOTKEYS
    switch( key_char ) {

    case Qt::Key_Tab:
        //parent_frame->SwitchKBFocus( this );
        break;

    case Qt::Key_Menu:
        //        int x, y;
        //        event->po( &x, &y );
        //        m_FinishRouteOnKillFocus = false;
        //        CallPopupMenu(x, y);
        //        m_FinishRouteOnKillFocus = true;
        break;

    case Qt::Key_Alt:
        m_modkeys |= Qt::Key_Alt;
        break;

    case Qt::Key_Control:
        m_modkeys |= Qt::Key_Control;
        break;

    case Qt::Key_Left:
        if( m_modkeys & Qt::ControlModifier )
        {
            this->DoCanvasStackDelta(-1);
        } else if(g_bsmoothpanzoom)
        {
            StartTimedMovement();
            m_panx = -1;
        } else {
            PanCanvas( -panspeed, 0 );
        }
        b_handled = true;
        break;

    case Qt::Key_Up:
        if(g_bsmoothpanzoom) {
            StartTimedMovement();
            m_pany = -1;
        } else
            PanCanvas( 0, -panspeed );
        b_handled = true;
        break;

    case Qt::Key_Right:
        if(  m_modkeys & Qt::ControlModifier ) this->DoCanvasStackDelta(1);
        else if(g_bsmoothpanzoom) {
            StartTimedMovement();
            m_panx = 1;
        } else
            PanCanvas( panspeed, 0 );
        b_handled = true;
        
        break;

    case Qt::Key_Down:
        if(g_bsmoothpanzoom) {
            StartTimedMovement();
            m_pany = 1;
        } else
            PanCanvas( 0, panspeed );
        b_handled = true;
        break;

    case Qt::Key_F2:
        TogglebFollow();
        break;

    case Qt::Key_F3: {
        SetShowENCText( !GetShowENCText() );
        Refresh(true);
        InvalidateGL();
        break;
    }
    case Qt::Key_F4:
        if( !m_bMeasure_Active ) {
            if (m_modkeys == Qt::ShiftModifier)
                m_bMeasure_DistCircle = true;
            else
                m_bMeasure_DistCircle = false;

            StartMeasureRoute();
        }
        else{
            CancelMeasureRoute();
            
            SetCursor( *pCursorArrow );
            InvalidateGL();
            Refresh( false );
        }
        
        break;

    case Qt::Key_F5:
        parent_frame->ToggleColorScheme();
        gFrame->raise();
        break;

    case Qt::Key_F6: {
        int mod = m_modkeys & Qt::ShiftModifier;
        if( mod != m_brightmod ) {
            m_brightmod = mod;
            m_bbrightdir = !m_bbrightdir;
        }

        if( !m_bbrightdir ) {
            g_nbrightness -= 10;
            if( g_nbrightness <= MIN_BRIGHT ) {
                g_nbrightness = MIN_BRIGHT;
                m_bbrightdir = true;
            }
        } else {
            g_nbrightness += 10;
            if( g_nbrightness >= MAX_BRIGHT ) {
                g_nbrightness = MAX_BRIGHT;
                m_bbrightdir = false;
            }
        }

//        SetScreenBrightness( g_nbrightness );
        ShowBrightnessLevelTimedPopup( g_nbrightness / 10, 1, 10 );
        gFrame->raise();        // And reactivate the application main

        break;
    }

    case Qt::Key_F7:
        this->DoCanvasStackDelta(-1);
        break;

    case Qt::Key_F8:
        this->DoCanvasStackDelta(1);
        break;
    case Qt::Key_F9:
        ToggleCanvasQuiltMode();
        break;
        
    case Qt::Key_F11:
        //        parent_frame->ToggleFullScreen();
        b_handled = true;
        break;

    case Qt::Key_F12: {
        if( m_modkeys == Qt::AltModifier )
            m_nMeasureState = *(volatile int *)(0);     // generate a fault for testing

        ToggleChartOutlines();
        break;
    }

    case Qt::Key_Pause:                   // Drop MOB
        //         parent_frame->ActivateMOB();
        break;

        //NUMERIC PAD
        //    case Qt::Key_NUMPAD_ADD:              // '+' on NUM PAD
    case Qt::Key_PageUp:{
        ZoomCanvas( 2.0, false );
        break;
    }
        //    case Qt::Key_NUMPAD_SUBTRACT:   // '-' on NUM PAD
    case Qt::Key_PageDown:{
        ZoomCanvas( .5, false );
        break;
    }
        break;
    default:
        break;

    }

    if( event->key() < 128 )            //ascii
    {
        int key_char = event->key();

        //      Handle both QWERTY and AZERTY keyboard separately for a few control codes
        if( !g_b_assume_azerty ) {
            switch( key_char ) {
            case '+': case '=':
                ZoomCanvas( 2.0, false );
                break;

            case '-': case '_':
                ZoomCanvas( 0.5, false );
                break;

            }

        } else {   //AZERTY
            switch( key_char ) {
            case 43:
                ZoomCanvas( 2.0, false );
                break;

            case 54:                     // '-'  alpha/num pad
                //            case 56:                     // '_'  alpha/num pad
                ZoomCanvas( 0.5, false );
                break;
            }
        }


        if ( m_modkeys & Qt::ControlModifier )
            key_char -= 64;

        if (key_char >= '0' && key_char <= '9')
            SetGroupIndex( key_char - '0' );
        else

            switch( key_char ) {
            case 'A':
                SetShowENCAnchor(!GetShowENCAnchor());
                ReloadVP();

                break;

            case 'C':
                parent_frame->ToggleColorScheme();
                break;

            case 'D':
            {
                int x,y;
                //            event.GetPosition( &x, &y );
                ChartTypeEnum ChartType = CHART_TYPE_UNKNOWN;
                ChartFamilyEnum ChartFam = CHART_FAMILY_UNKNOWN;
                // First find out what kind of chart is being used
                //            if( !pPopupDetailSlider ) {
                //                if( VPoint.quilt() )
                //                    {
                //                            if (m_pQuilt->GetChartAtPix( VPoint, QPoint( x, y )) ) // = null if no chart loaded for this point
                //                            {
                //                                ChartType = m_pQuilt->GetChartAtPix( VPoint, QPoint( x, y ) )->GetChartType();
                //                                ChartFam = m_pQuilt->GetChartAtPix( VPoint, QPoint( x, y ) )->GetChartFamily();
                //                            }
                //                    }
                //                else
                //                    {
                //                        if (m_singleChart){
                //                            ChartType = m_singleChart->GetChartType();
                //                            ChartFam =  m_singleChart->GetChartFamily();
                //                        }
                //                    }
                //                    //If a charttype is found show the popupslider
                //                if ( (ChartType != CHART_TYPE_UNKNOWN) || (ChartFam != CHART_FAMILY_UNKNOWN) )
                //                    {
                //                        pPopupDetailSlider = new PopUpDSlide( this, -1, ChartType, ChartFam,
                //                            QPoint( g_detailslider_dialog_x, g_detailslider_dialog_y ));
                //                        if (pPopupDetailSlider)
                //                            pPopupDetailSlider->show();
                //                    }
                //                }
                //            else //( !pPopupDetailSlider ) close popupslider
                //            {
                //                if (pPopupDetailSlider) pPopupDetailSlider->close();
                //                pPopupDetailSlider = NULL;
                //            }
                break;
            }

            case 'L':
                SetShowENCLights(!GetShowENCLights());
                ReloadVP();

                break;

            case 'M':
                if (m_modkeys & Qt::ShiftModifier)
                    m_bMeasure_DistCircle = true;
                else
                    m_bMeasure_DistCircle = false;

                StartMeasureRoute();
                break;

            case 'N':
                if( g_bInlandEcdis && ps52plib){
                    SetENCDisplayCategory( (_DisCat)STANDARD );
                }
                break;

            case 'O':
                ToggleChartOutlines();
                break;

            case 'Q':
                ToggleCanvasQuiltMode();
                break;

            case 'P':
                //            parent_frame->ToggleTestPause();
                break;
#if 0
            case 'R':
                parent_frame->ToggleRocks();
                break;
#endif
            case 'S':
                SetShowENCDepth( !m_encShowDepth );
                ReloadVP();
                break;

            case 'T':
                SetShowENCText(!GetShowENCText());
                ReloadVP();
                break;

            case 'U':
                SetShowENCDataQual(!GetShowENCDataQual());
                ReloadVP();
                break;

            case 'V':
                m_bShowNavobjects = !m_bShowNavobjects;
                Refresh( true );
                break;

            case 1:                      // Ctrl A
                TogglebFollow();

                break;

            case 2:                      // Ctrl B
                if ( g_bShowMenuBar == false )
                    //                parent_frame->ToggleChartBar( this );
                    break;

            case 13:             // Ctrl M // Drop Marker at cursor
            {
                if(m_modkeys & Qt::ControlModifier)
                    //                gFrame->DropMarker(false);
                    break;
            }

            case 14:             // Ctrl N - Activate next waypoint in a route
            {

                break;
            }

            case 15:             // Ctrl O - Drop Marker at boat's position
            {
                if(!g_bShowMenuBar)
                    //                gFrame->DropMarker(true);
                    break;
            }

            case 32:            // Special needs use space bar
            {
                if ( g_bSpaceDropMark )
                    //                gFrame->DropMarker( true );
                    break;
            }

            case -32:                     // Ctrl Space            //    Drop MOB
            {
                //            if( m_modkeys == wxMOD_CONTROL ) parent_frame->ActivateMOB();

                break;
            }

            case -20:                       // Ctrl ,
            {
                //            parent_frame->DoSettings();
                break;
            }
            case 17:                       // Ctrl Q
                //            parent_frame->Close();
                return;

            case 18:                       // Ctrl R
                //            StartRoute();
                return;

            case 20:                       // Ctrl T
                //            if( NULL == pGoToPositionDialog ) // There is one global instance of the Go To Position Dialog
                //                pGoToPositionDialog = new GoToPositionDialog( this );
                //            pGoToPositionDialog->SetCanvas( this );
                //            pGoToPositionDialog->Show();
                break;

            case 25:                       // Ctrl Y
//                if( undo->AnythingToRedo() ) {
//                    undo->RedoNextAction();
//                    InvalidateGL();
//                    Refresh( false );
//                }
                break;

            case 26:
                //            if ( event.ShiftDown() ) { // Shift-Ctrl-Z
                //                if( undo->AnythingToRedo() ) {
                //                    undo->RedoNextAction();
                //                    InvalidateGL();
                //                    Refresh( false );
                //                }
                //            } else {                   // Ctrl Z
                //                if( undo->AnythingToUndo() ) {
                //                    undo->UndoLastAction();
                //                    InvalidateGL();
                //                    Refresh( false );
                //                }
                //            }
                break;

            case 27:
                // Generic break
                //            if( m_bMeasure_Active ) {
                //                CancelMeasureRoute();

                //                SetCursor( *pCursorArrow );

                //                SurfaceToolbar();
                //                gFrame->RefreshAllCanvas();
                //            }

                //            if( m_routeState )         // creating route?
                //            {
                //                FinishRoute();
                //                SurfaceToolbar();
                //                InvalidateGL();
                //                Refresh( false );
                //            }

                break;

            case 7:                       // Ctrl G
                switch( gamma_state ) {
                case ( 0 ):
                    r_gamma_mult = 0;
                    g_gamma_mult = 1;
                    b_gamma_mult = 0;
                    gamma_state = 1;
                    break;
                case ( 1 ):
                    r_gamma_mult = 1;
                    g_gamma_mult = 0;
                    b_gamma_mult = 0;
                    gamma_state = 2;
                    break;
                case ( 2 ):
                    r_gamma_mult = 1;
                    g_gamma_mult = 1;
                    b_gamma_mult = 1;
                    gamma_state = 0;
                    break;
                }
//                SetScreenBrightness( g_nbrightness );

                break;

            case 9:                      // Ctrl I
                //           if(event.ControlDown()){
                //                m_bShowCompassWin = !m_bShowCompassWin;
                //                SetShowGPSCompassWindow(m_bShowCompassWin);
                //                Refresh( false );
                //           }
                break;

            default:
                break;

            }           // switch
    }

    if(!b_handled)
        event->ignore();
}

void ChartCanvas::keyReleaseEvent(QKeyEvent* event)
{
#if 0
    if(g_pi_manager && g_pi_manager->SendKeyEventToPlugins( event ))  return;                     // PlugIn did something, and does not want the canvas to do anything else

    switch( event->key() ) {
    case Qt::Key_Tab:
        parent_frame->SwitchKBFocus( this );
        break;

    case Qt::Key_Left:
    case Qt::Key_Right:
        m_panx = 0;
        if(!m_pany)
            m_panspeed = 0;
        break;

    case Qt::Key_Up:
    case Qt::Key_Down:
        m_pany = 0;
        if(!m_panx)
            m_panspeed = 0;
        break;

        //    case Qt::Key_NUMPAD_ADD:              // '+' on NUM PAD
        //    case Qt::Key_NUMPAD_SUBTRACT:   // '-' on NUM PAD
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        if(m_mustmove)
            DoMovement(m_mustmove);

        m_zoom_factor = 1;
        break;

    case Qt::Key_Alt:
        m_modkeys &= ~Qt::AltModifier;
#ifdef OCPN_ALT_MENUBAR        
#ifndef __WXOSX__
        // If the permanent menu bar is disabled, and we are not in the middle of another key combo,
        // then show the menu bar temporarily when Alt is released (or hide it if already visible).
        if ( IsTempMenuBarEnabled() && !g_bShowMenuBar  &&  m_bMayToggleMenuBar ) {
            g_bTempShowMenuBar = !g_bTempShowMenuBar;
            parent_frame->ApplyGlobalSettings(false);
        }
        m_bMayToggleMenuBar = true;
#endif
#endif        
        break;

    case Qt::Key_Control:
        m_modkeys &= ~Qt::ControlModifier;
        break;

    }

    if( event->key() < 128 )            //ascii
    {
        int key_char = event->key();

        //      Handle both QWERTY and AZERTY keyboard separately for a few control codes
        if( !g_b_assume_azerty ) {
            switch( key_char ) {
            case '+':     case '=':
            case '-':     case '_':
            case 54:    case 56:    // '_'  alpha/num pad
                DoMovement(m_mustmove);

                m_zoom_factor = 1;
                break;
            case '[': case ']':
                DoMovement(m_mustmove);
                m_rotation_speed = 0;
                break;
            }
        } else {
            switch( key_char ) {
            case 43:
            case 54:                     // '-'  alpha/num pad
            case 56:                     // '_'  alpha/num pad
                DoMovement(m_mustmove);

                m_zoom_factor = 1;
                break;
            }
        }
    }
    event.Skip();
#endif
}

void ChartCanvas::ToggleChartOutlines( void )
{
    m_bShowOutlines = !m_bShowOutlines;
    
    Refresh( false );

    InvalidateGL();
}


void ChartCanvas::ToggleLookahead( )
{
    m_bLookAhead = !m_bLookAhead;
    m_OSoffsetx = 0;            // center ownship
    m_OSoffsety = 0;

}


void ChartCanvas::ToggleCourseUp( )
{
    //    m_bCourseUp = !m_bCourseUp;
    
    //    if( m_bCourseUp ) {
    //        //    Stuff the COGAvg table in case COGUp is selected
    //        double stuff = 0;
    //        if( !std::isnan(gCog) )
    //            stuff = gCog;

    //        if( g_COGAvgSec > 0) {
    //            for( int i = 0; i < g_COGAvgSec; i++ )
    //                gFrame->COGTable[i] = stuff;
    //        }
    //        g_COGAvg = stuff;
    //        gFrame->FrameCOGTimer.Start( 100, QTimer_CONTINUOUS );
    //    } else {
    //        if ( !g_bskew_comp && (fabs(GetVPSkew()) > 0.0001))
    //            SetVPRotation(GetVPSkew());
    //        else
    //            SetVPRotation(0); /* reset to north up */
    //    }
    
    
    //    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
    //        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    //DoCOGSet();
    UpdateGPSCompassStatusBox( true );
    DoCanvasUpdate();
}

bool ChartCanvas::DoCanvasCOGSet( double cog )
{
    if( !m_bCourseUp )  return false;
    
    //    if (std::isnan(g_COGAvg))
    //        return true;
    
    double old_VPRotate = m_VPRotate;
    m_VPRotate = -cog * PI / 180.;
    
    SetVPRotation( m_VPRotate );
    bool bnew_chart = DoCanvasUpdate();
    
    if( ( bnew_chart ) || ( old_VPRotate != m_VPRotate ) )
        ReloadVP();
    
    return true;
}

void ChartCanvas::StopMovement( )
{
    m_panx = m_pany = 0;
    m_panspeed = 0;
    m_zoom_factor = 1;
    m_rotation_speed = 0;
    m_mustmove = 0;
#if 0    
#if !defined(__WXGTK__) && !defined(__WXQT__)
    SetFocus();
    gFrame->Raise();
#endif    
#endif
}

/* instead of integrating in timer callbacks
   (which do not always get called fast enough)
   we can perform the integration of movement
   at each render frame based on the time change */
bool ChartCanvas::StartTimedMovement( bool stoptimer )
{
    // Start/restart the stop movement timer
    //    if(stoptimer)
    //        pMovementStopTimer->Start( 1000, QTimer_ONE_SHOT );

    //    if(!pMovementTimer->IsRunning()){
    ////        printf("timer not running, starting\n");
    //        pMovementTimer->Start( 1, QTimer_ONE_SHOT );
    //    }
    
    //    if(m_panx || m_pany || m_zoom_factor!=1 || m_rotation_speed) {
    //        // already moving, gets called again because of key-repeat event
    //        return false;
    //    }

    m_last_movement_time = QDateTime::currentDateTime();

    /* jumpstart because paint gets called right away, if we want first frame to move */
    //    m_last_movement_time -= wxTimeSpan::Milliseconds(100);

    //    Refresh( false );

    return true;
}

void ChartCanvas::DoTimedMovement()
{
    if( m_pan_drag == zchxPoint(0, 0) && !m_panx && !m_pany && m_zoom_factor==1 && !m_rotation_speed)
        return; /* not moving */

    QDateTime now = QDateTime::currentDateTime();
    long dt = 0;
    if(m_last_movement_time.isValid())
        dt = m_last_movement_time.msecsTo(now );

    m_last_movement_time = now;

    if(dt > 500) /* if we are running very slow, don't integrate too fast */
        dt = 500;

    DoMovement(dt);
}

void ChartCanvas::DoMovement( long dt )
{
    /* if we get here quickly assume 1ms so that some movement occurs */
    if(dt == 0)
        dt = 1;

    m_mustmove -= dt;
    if(m_mustmove < 0)
        m_mustmove = 0;

    if(m_pan_drag.x || m_pan_drag.y) {
        PanCanvas( m_pan_drag.x, m_pan_drag.y );
        m_pan_drag.x = m_pan_drag.y = 0;
    }

    if(m_panx || m_pany) {
        const double slowpan = .1, maxpan = 2;
        if( m_modkeys & Qt::AltModifier )
            m_panspeed = slowpan;
        else {
            m_panspeed += (double)dt/500; /* apply acceleration */
            m_panspeed = fmin( maxpan, m_panspeed );
        }
        PanCanvas( m_panspeed * m_panx * dt, m_panspeed * m_pany * dt);
    }

    if(m_zoom_factor != 1) {
        double alpha = 400, beta = 1.5;
        double zoom_factor = (exp(dt / alpha) - 1) / beta + 1;

        if( m_modkeys & Qt::AltModifier )
            zoom_factor = pow(zoom_factor, .15);

        if(m_zoom_factor < 1)
            zoom_factor = 1/zoom_factor;

        //  Try to hit the zoom target exactly.
        if(m_wheelzoom_stop_oneshot > 0) {
            if(zoom_factor > 1){
                if(  VPoint.chartScale() / zoom_factor <= m_zoom_target)
                    zoom_factor = VPoint.chartScale() / m_zoom_target;
            }

            else if(zoom_factor < 1){
                if(  VPoint.chartScale() / zoom_factor >= m_zoom_target)
                    zoom_factor = VPoint.chartScale() / m_zoom_target;
            }
        }

        if( fabs(zoom_factor - 1) > 1e-4)
            DoZoomCanvas( zoom_factor, m_bzooming_to_cursor );

        if(m_wheelzoom_stop_oneshot > 0) {
            if(m_wheelstopwatch.elapsed() > m_wheelzoom_stop_oneshot){
                m_wheelzoom_stop_oneshot = 0;
                StopMovement( );
            }

            //      Don't overshoot the zoom target.
            if(zoom_factor > 1){
                if(  VPoint.chartScale() <= m_zoom_target){
                    m_wheelzoom_stop_oneshot = 0;
                    StopMovement( );
                }
            }
            else if(zoom_factor < 1){
                if(  VPoint.chartScale() >= m_zoom_target){
                    m_wheelzoom_stop_oneshot = 0;
                    StopMovement( );
                }
            }
        }
    }

    if( m_rotation_speed ) { /* in degrees per second */
        double speed = m_rotation_speed;
        if( m_modkeys & Qt::AltModifier)
            speed /= 10;
        DoRotateCanvas( VPoint.rotation() + speed * PI / 180 * dt / 1000.0);
    }
}

void ChartCanvas::SetColorScheme( ColorScheme cs )
{
    CreateDepthUnitEmbossMaps( cs );
    CreateOZEmbossMapData( cs );
    
    //  Set up fog effect base color
    m_fog_color = QColor( 170, 195, 240 );  // this is gshhs (backgound world chart) ocean color
    float dim = 1.0;
    switch( cs ){
    case GLOBAL_COLOR_SCHEME_DUSK:
        dim = 0.5;
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        dim = 0.25;
        break;
    default:
        break;
    }
    m_fog_color.setRgb(m_fog_color.red()*dim, m_fog_color.green()*dim, m_fog_color.blue()*dim );

    //    UpdateToolbarColorScheme( cs );
    
    m_Compass->SetColorScheme( cs );

    //    if(m_muiBar)
    //        m_muiBar->SetColorScheme( cs );
    
    if(pWorldBackgroundChart)
        pWorldBackgroundChart->SetColorScheme( cs );
    if( m_glcc ){
        m_glcc->SetColorScheme( cs );
        g_glTextureManager->ClearAllRasterTextures();
        //m_glcc->FlushFBO();
    }

    ReloadVP();

    m_cs = cs;
}

wxBitmap ChartCanvas::CreateDimBitmap( wxBitmap &Bitmap, double factor )
{
    QImage img = Bitmap.ConvertToImage();
    int sx = img.width();
    int sy = img.height();

    QImage new_img( img );

    for( int i = 0; i < sx; i++ ) {
        for( int j = 0; j < sy; j++ ) {
            QColor color  = img.pixelColor(i, j);
            bool is_transparent = color.alpha() == 0;
            if( !is_transparent ) {
                new_img.setPixelColor( i, j, QColor(color.red() * factor,  color.green() * factor, color.blue() * factor));
            }
        }
    }

    wxBitmap ret = wxBitmap( new_img );

    return ret;

}

void ChartCanvas::ShowBrightnessLevelTimedPopup( int brightness, int min, int max )
{
#if 0
    QFont pfont = FontMgr::Get().FindOrCreateFont( 40, "Microsoft YH", QFont::StyleNormal, QFont::Weight::Bold );

    if( !m_pBrightPopup ) {
        //    Calculate size
        int x, y;
        QFontMetrics mcs(pfont);
        x = mcs.width("MAX");
        y = mcs.height();
        m_pBrightPopup = new TimedPopupWin( 3, this);

        m_pBrightPopup->resize(x, y);
        m_pBrightPopup->move(120,120);
    }

    int bmpsx = m_pBrightPopup->size().width();
    int bmpsy = m_pBrightPopup->size().height();

    wxBitmap bmp( bmpsx, bmpsx );
    QPainter mdc( bmp.GetHandle());
    //    mdc.SetTextForeground( GetGlobalColor( ("GREEN4") ) );
    mdc.setBackground(QBrush( GetGlobalColor( ("UINFD") ) ) );
    mdc.setPen( QPen( QColor( 0, 0, 0 ) ) );
    mdc.setBrush( QBrush( GetGlobalColor(("UINFD") ) ) );
    //    mdc.Clear();

    mdc.drawRect( 0, 0, bmpsx, bmpsy );

    mdc.setFont( pfont );
    QPen pen = mdc.pen();
    pen.setColor(GetGlobalColor("GREEN4"));
    QString val;

    if( brightness == max ) val = ("MAX");
    else
        if( brightness == min ) val = ("MIN");
        else
            val.sprintf("%3d", brightness );

    mdc.drawText(0, 0, val );

    m_pBrightPopup->SetBitmap(QBitmap::fromImage(bmp.ConvertToImage()) );
    m_pBrightPopup->show();
#endif
}


void ChartCanvas::RotateTimerEvent( QTimerEvent& event )
{
    m_b_rot_hidef = true;
    ReloadVP();
}


void ChartCanvas::OnCursorTrackTimerEvent( QTimerEvent& event )
{
    if( s57_CheckExtendedLightSectors( this, mouse_x, mouse_y, VPoint, extendedSectorLegs ) ){
        if(!m_bsectors_shown) {
            ReloadVP( false );
            m_bsectors_shown = true;
        }
    }
    else {
        if( m_bsectors_shown ) {
            ReloadVP( false );
            m_bsectors_shown = false;
        }
    }

    //      This is here because GTK status window update is expensive..
    //            cairo using pango rebuilds the font every time so is very inefficient
    //      Anyway, only update the status bar when this timer expires
#if defined(__WXGTK__) || defined(__WXQT__)
    {
        //    Check the absolute range of the cursor position
        //    There could be a window wherein the chart geoereferencing is not valid....
        double cursor_lat, cursor_lon;
        GetCanvasPixPoint ( mouse_x, mouse_y, cursor_lat, cursor_lon );

        if((fabs(cursor_lat) < 90.) && (fabs(cursor_lon) < 360.))
        {
            while(cursor_lon < -180.)
                cursor_lon += 360.;

            while(cursor_lon > 180.)
                cursor_lon -= 360.;

            SetCursorStatus(cursor_lat, cursor_lon);
        }
    }
#endif
}

void ChartCanvas::SetCursorStatus( double cursor_lat, double cursor_lon )
{
#if 0
    //    if ( !parent_frame->m_pStatusBar )  return;

    QString s1;
    s1 += (" ");
    s1 += toSDMM(1, cursor_lat);
    s1 += ("   ");
    s1 += toSDMM(2, cursor_lon);
    
    if(STAT_FIELD_CURSOR_LL >= 0)
        //        parent_frame->SetStatusText ( s1, STAT_FIELD_CURSOR_LL );

        if( STAT_FIELD_CURSOR_BRGRNG < 0 )
            return;

    double brg, dist;
    QString s;
    DistanceBearingMercator(cursor_lat, cursor_lon, gLat, gLon, &brg, &dist);
    if( g_bShowMag )
        s.sprintf( QString("%03d°(M)  ", wxConvUTF8 ), (int)gFrame->GetMag( brg ) );
    else
        s.Printf( QString("%03d°  ", wxConvUTF8 ), (int)brg );
    
    s << FormatDistanceAdaptive( dist );
    
    // CUSTOMIZATION - LIVE ETA OPTION
    // -------------------------------------------------------
    // Calculate an "live" ETA based on route starting from the current
    // position of the boat and goes to the cursor of the mouse.
    // In any case, an standard ETA will be calculated with a default speed
    // of the boat to give an estimation of the route (in particular if GPS
    // is off).
    
    // Display only if option "live ETA" is selected in Settings > Display > General.
    if (g_bShowLiveETA)
    {

        float realTimeETA;
        float boatSpeed;
        float boatSpeedDefault = g_defaultBoatSpeed;
        
        // Calculate Estimate Time to Arrival (ETA) in minutes
        // Check before is value not closed to zero (it will make an very big number...)
        if (!std::isnan(gSog))
        {
            boatSpeed = gSog;
            if (boatSpeed < 0.5)
            {
                realTimeETA = 0;
            }
            else
            {
                realTimeETA = dist / boatSpeed * 60;
            }
        }
        else
        {
            realTimeETA = 0;
        }
        
        // Add space after distance display
        s << " ";
        // Display ETA
        s << minutesToHoursDays(realTimeETA);
        
        // In any case, display also an ETA with default speed at 6knts
        
        s << " [@";
        s << QString::Format(_T("%d"), (int)toUsrSpeed(boatSpeedDefault, -1));
        s << QString::Format(_T("%s"), getUsrSpeedUnit(-1));
        s << " ";
        s << minutesToHoursDays(dist/boatSpeedDefault*60);
        s << "]";

    }
    // END OF - LIVE ETA OPTION
    
    //    parent_frame->SetStatusText ( s, STAT_FIELD_CURSOR_BRGRNG );
#endif
}

// CUSTOMIZATION - FORMAT MINUTES
// -------------------------------------------------------
// New function to format minutes into a more readable format:
//  * Hours + minutes, or
//  * Days + hours.
QString minutesToHoursDays(float timeInMinutes)
{
    QStringList s;
    
    if (timeInMinutes == 0)
    {
        s << "--min";
    }
    
    // Less than 60min, keep time in minutes
    else if (timeInMinutes < 60 && timeInMinutes != 0)
    {
        s << QString::number((int)timeInMinutes);
        s << "min";
    }
    
    // Between 1h and less than 24h, display time in hours, minutes
    else if (timeInMinutes >= 60 && timeInMinutes < 24 * 60)
    {
        
        int hours;
        int min;
        hours = (int)timeInMinutes / 60;
        min = (int)timeInMinutes % 60;
        
        if (min == 0)
        {
            s << QString::number( hours );
            s << "h";
        }
        else
        {
            s << QString::number( hours );
            s << "h";
            s << QString::number( min );
            s << "min";
        }
        
    }
    
    // More than 24h, display time in days, hours
    else if (timeInMinutes > 24 * 60)
    {
        
        int days;
        int hours;
        days = (int)(timeInMinutes / 60) / 24;
        hours = (int)(timeInMinutes / 60) % 24;
        
        if (hours == 0)
        {
            s << QString::number(days );
            s << "d";
        }
        else
        {
            s << QString::number(days );
            s << "d";
            s << QString::number(hours );
            s << "h";
        }
        
    }
    
    return s.join("");
}

// END OF CUSTOMIZATION - FORMAT MINUTES
// Thanks open source code ;-)
// -------------------------------------------------------


void ChartCanvas::GetCursorLatLon( double *rlat, double *rlon )
{
    double lat, lon;
    GetCanvasPixPoint( mouse_x, mouse_y, lat, lon );
    *rlat = lat;
    *rlon = lon;
}

void ChartCanvas::GetDoubleCanvasPointPix( double rlat, double rlon, zchxPointF &r )
{
    return GetDoubleCanvasPointPixVP( GetVP(), rlat, rlon, r );
}

void ChartCanvas::GetDoubleCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, zchxPointF &r )
{
    r = vp.GetDoublePixFromLL( rlat, rlon );
}


// This routine might be deleted and all of the rendering improved
// to have floating point accuracy
bool ChartCanvas::GetCanvasPointPix( double rlat, double rlon, zchxPoint &r )
{
    return GetCanvasPointPixVP( GetVP(), rlat, rlon, r);
}

bool ChartCanvas::GetCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, zchxPoint &r )
{
    zchxPointF p;
    GetDoubleCanvasPointPixVP(vp, rlat, rlon, p);

    // some projections give nan values when invisible values (other side of world) are requested
    // we should stop using integer coordinates or return false here (and test it everywhere)
    if(std::isnan(p.x)) {
        r = zchxPoint(INVALID_COORD, INVALID_COORD);
        return false;
    }

    r = zchxPoint(qRound(p.x), qRound(p.y));
    return true;
}


void ChartCanvas::GetCanvasPixPoint( double x, double y, double &lat, double &lon )
{
    GetVP().GetLLFromPix( zchxPointF( x, y ), &lat, &lon );
}

void ChartCanvas::ZoomCanvas( double factor, bool can_zoom_to_cursor, bool stoptimer )
{
    m_bzooming_to_cursor = can_zoom_to_cursor && g_bEnableZoomToCursor;

    if( g_bsmoothpanzoom ) {
        if(StartTimedMovement(stoptimer)) {
            m_mustmove += 150; /* for quick presses register as 200 ms duration */
            m_zoom_factor = factor;
        }
        m_zoom_target =  VPoint.chartScale() / factor;
    } else {
        if( m_modkeys == Qt::AltModifier )
            factor = pow(factor, .15);
        
        DoZoomCanvas( factor, can_zoom_to_cursor );
    }

    extendedSectorLegs.clear();
}

void ChartCanvas::DoZoomCanvas( double factor,  bool can_zoom_to_cursor )
{
    // possible on startup
    if( !ChartData )
        return;
    if(!m_pCurrentStack)
        return;

    if(g_bShowCompassWin){
        m_bShowCompassWin = true;
        SetShowGPSCompassWindow( true );    // Cancel effects of Ctrl-I
    }

    /* TODO: queue the quilted loading code to a background thread
       so yield is never called from here, and also rendering is not delayed */

    //    Cannot allow Yield() re-entrancy here
    if( m_bzooming ) return;
    m_bzooming = true;

    double old_ppm = GetVP().viewScalePPM();

    //  Capture current cursor position for zoom to cursor
    double zlat = m_cursor_lat;
    double zlon = m_cursor_lon;

    double proposed_scale_onscreen = GetVP().chartScale() / factor; // GetCanvasScaleFactor() / ( GetVPScale() * factor );
    bool b_do_zoom = false;
    
    if(factor > 1)
    {
        b_do_zoom = true;

        double zoom_factor = factor;

        ChartBase *pc = NULL;

        if( !VPoint.quilt() ) {
            pc = m_singleChart;
        } else {
            int new_db_index = m_pQuilt->AdjustRefOnZoomIn( proposed_scale_onscreen );
            if( new_db_index >= 0 )
                pc = ChartData->OpenChartFromDB( new_db_index, FULL_INIT );

            if(m_pCurrentStack)
                m_pCurrentStack->SetCurrentEntryFromdbIndex( new_db_index ); // highlite the correct bar entry
        }

        if( pc ) {
            //             double target_scale_ppm = GetVPScale() * zoom_factor;
            //             proposed_scale_onscreen = GetCanvasScaleFactor() / target_scale_ppm;
            
            //  Query the chart to determine the appropriate zoom range
            double min_allowed_scale = 800;    // Roughly, latitude dependent for mercator charts
            
            if( proposed_scale_onscreen < min_allowed_scale ) {
                if( min_allowed_scale == GetCanvasScaleFactor() / ( GetVPScale() ) ) {
                    m_zoom_factor = 1; /* stop zooming */
                    b_do_zoom = false;
                } else
                    proposed_scale_onscreen = min_allowed_scale;
            }
            
        }
        else {
            proposed_scale_onscreen = fmax( proposed_scale_onscreen, 800.);
        }

        
    } else if(factor < 1) {
        double zoom_factor = 1/factor;

        b_do_zoom = true;

        ChartBase *pc = NULL;

        bool b_smallest = false;

        if( !VPoint.quilt() ) {             // not quilted
            pc = m_singleChart;

            if( pc ) {
                //      If m_singleChart is not on the screen, unbound the zoomout
                LLBBox viewbox = VPoint.getBBox();
                //                wxBoundingBox chart_box;
                int current_index = ChartData->FinddbIndex( pc->GetFullPath() );
                double max_allowed_scale;

                max_allowed_scale = GetCanvasScaleFactor() / m_absolute_min_scale_ppm;

                //  We can allow essentially unbounded zoomout in single chart mode
                //                if( ChartData->GetDBBoundingBox( current_index, &chart_box ) &&
                //                    !viewbox.IntersectOut( chart_box ) )
                //                    //  Clamp the minimum scale zoom-out to the value specified by the chart
                //                    max_allowed_scale = fmin(max_allowed_scale, 4.0 *
                //                                              pc->GetNormalScaleMax( GetCanvasScaleFactor(),
                //                                                                     GetCanvasWidth() ) );
                if(proposed_scale_onscreen > max_allowed_scale) {
                    m_zoom_factor = 1; /* stop zooming */
                    proposed_scale_onscreen = max_allowed_scale;
                }
            }

        } else {
            int new_db_index = m_pQuilt->AdjustRefOnZoomOut( proposed_scale_onscreen );
            if( new_db_index >= 0 ) pc = ChartData->OpenChartFromDB( new_db_index, FULL_INIT );

            if(m_pCurrentStack)
                m_pCurrentStack->SetCurrentEntryFromdbIndex( new_db_index ); // highlite the correct bar entry
            
            b_smallest = m_pQuilt->IsChartSmallestScale( new_db_index );

            if( b_smallest || (0 == m_pQuilt->GetExtendedStackCount()))
                proposed_scale_onscreen = fmin(proposed_scale_onscreen,
                                               GetCanvasScaleFactor() / m_absolute_min_scale_ppm);
        }

        //set a minimum scale
        if( ( GetCanvasScaleFactor() / proposed_scale_onscreen ) < m_absolute_min_scale_ppm )
            b_do_zoom = false;
    }

    double new_scale = GetVPScale() * (GetVP().chartScale() / proposed_scale_onscreen);
    if( b_do_zoom ) {
        if( can_zoom_to_cursor && g_bEnableZoomToCursor) {
            //  Arrange to combine the zoom and pan into one operation for smoother appearance
            SetVPScale( new_scale, false );   // adjust, but deferred refresh

            zchxPoint r;
            GetCanvasPointPix( zlat, zlon, r );
            PanCanvas( r.x - mouse_x, r.y - mouse_y );  // this will give the Refresh()

            //ClearbFollow();      // update the follow flag
        }
        else{
            if(m_bFollow){      //  Adjust the Viewpoint to keep ownship at the same pixel point on-screen
                double offx, offy;
                toSM(GetVP().lat(), GetVP().lon(), gLat, gLon, &offx, &offy);

                m_OSoffsetx = offx * old_ppm;
                m_OSoffsety = offy * old_ppm;

                double nlat, nlon;
                double dx = m_OSoffsetx;
                double dy = m_OSoffsety;
                double d_east = dx / new_scale;
                double d_north = dy / new_scale;

                fromSM( d_east, d_north, gLat, gLon, &nlat, &nlon );
                SetViewPoint( nlat, nlon, new_scale, GetVP().skew(), GetVP().rotation());
            }
            else
                SetVPScale( new_scale );
        }
    }
    
    m_bzooming = false;
    
}

void ChartCanvas::RotateCanvas( double dir )
{
    if(m_bCourseUp)
        m_bCourseUp = false;

    if(g_bsmoothpanzoom) {
        if(StartTimedMovement()) {
            m_mustmove += 150; /* for quick presses register as 200 ms duration */
            m_rotation_speed = dir*60;
        }
    } else {
        double speed = dir*10;
        if( m_modkeys == Qt::AltModifier)
            speed /= 20;
        DoRotateCanvas(VPoint.rotation() + PI/180 * speed);
    }
}

void ChartCanvas::DoRotateCanvas( double rotation )
{
    while(rotation < 0) rotation += 2*PI;
    while(rotation > 2*PI) rotation -= 2*PI;

    if(rotation == VPoint.rotation() || std::isnan(rotation))
        return;

    SetVPRotation( rotation );
    UpdateGPSCompassStatusBox( true );
    DoCanvasUpdate();
}

void ChartCanvas::DoRotateCanvasWithDegree(double rotation)
{
    DoRotateCanvas(PI/180 * rotation);
}

void ChartCanvas::DoTiltCanvas( double tilt )
{
    while(tilt < 0) tilt = 0;
    while(tilt > .95) tilt = .95;

    if(tilt == VPoint.tilt() || std::isnan(tilt))
        return;

    VPoint.setTilt(tilt);
    Refresh( false );
}

void ChartCanvas::TogglebFollow( void )
{
    if( !m_bFollow )
        SetbFollow();
    else
        ClearbFollow();
}

void ChartCanvas::ClearbFollow( void )
{
    m_bFollow = false;      // update the follow flag
    
    //    Center the screen on the GPS position, for lack of a better place
    m_vLat = gLat;
    m_vLon = gLon;
    
#ifdef __OCPN__ANDROID__
    androidSetFollowTool(false);
#endif

    //    if( m_toolBar )
    //        m_toolBar->GetToolbar()->ToggleTool( ID_FOLLOW, false );
    //    parent_frame->SetMenubarItemState( ID_MENU_NAV_FOLLOW, false );
    
    UpdateFollowButtonState();
    
    DoCanvasUpdate();
    ReloadVP();
    parent_frame->SetChartUpdatePeriod( );
    
}

void ChartCanvas::SetbFollow( void )
{
    JumpToPosition(gLat, gLon, GetVPScale());
    m_bFollow = true;
    
    //    if( m_toolBar )
    //        m_toolBar->GetToolbar()->ToggleTool( ID_FOLLOW, true );
    //    parent_frame->SetMenubarItemState( ID_MENU_NAV_FOLLOW, true );

    UpdateFollowButtonState();
    
#ifdef __OCPN__ANDROID__
    androidSetFollowTool(true);
#endif
    
    // Is the OWNSHIP on-screen?
    // If not, then reset the OWNSHIP offset to 0 (center screen)
    if( (fabs(m_OSoffsetx) > VPoint.pixWidth() / 2) || (fabs(m_OSoffsety) > VPoint.pixHeight() / 2) ){
        m_OSoffsetx = 0;
        m_OSoffsety = 0;
    }

    DoCanvasUpdate();
    ReloadVP();
    parent_frame->SetChartUpdatePeriod( );
}

void ChartCanvas::UpdateFollowButtonState( void )
{
    //   if(m_muiBar){
    //        if(!m_bFollow)
    //            m_muiBar->SetFollowButtonState( 0 );
    //        else{
    //            if(m_bLookAhead)
    //                m_muiBar->SetFollowButtonState( 2 );
    //            else
    //                m_muiBar->SetFollowButtonState( 1 );
    //        }
    //   }
}

void ChartCanvas::JumpToPosition( double lat, double lon, double scale_ppm )
{
    if (lon > 180.0)
        lon -= 360.0;
    m_vLat = lat;
    m_vLon = lon;
    StopMovement();
    m_bFollow = false;
    
    if( !GetQuiltMode() ) {
        double skew = 0;
        if(m_singleChart)
            skew = m_singleChart->GetChartSkew() * PI / 180.;
        SetViewPoint( lat, lon, scale_ppm, skew, GetVPRotation() );
    } else {
        if (scale_ppm != GetVPScale()) {
            // XXX should be done in SetViewPoint
            VPoint.setChartScale(m_canvas_scale_factor / ( scale_ppm ));
            AdjustQuiltRefChart();
        }
        SetViewPoint( lat, lon, scale_ppm, 0, GetVPRotation() );
    }
    
    ReloadVP();
    
    //    if( m_toolBar )
    //        m_toolBar->GetToolbar()->ToggleTool( ID_FOLLOW, false );

    UpdateFollowButtonState();
    
    //TODO
    //    if( g_pi_manager ) {
    //        g_pi_manager->SendViewPortToRequestingPlugIns( cc1->GetVP() );
    //    }
}


bool ChartCanvas::PanCanvas( double dx, double dy )
{
    if( !ChartData )
        return false;

    extendedSectorLegs.clear();

    double lat = VPoint.lat(), lon = VPoint.lon();
    double dlat, dlon;
    zchxPointF p(VPoint.pixWidth() / 2.0, VPoint.pixHeight() / 2.0);

    int iters = 0;
    for(;;) {
        GetCanvasPixPoint( p.x + trunc(dx), p.y + trunc(dy), dlat, dlon );

        if(iters++ > 5)
            return false;
        if(!std::isnan(dlat))
            break;

        dx *= .5, dy *= .5;
        if(fabs(dx) < 1 && fabs(dy) < 1)
            return false;
    }

    // avoid overshooting the poles
    if(dlat > 90)
        dlat = 90;
    else if(dlat < -90)
        dlat = -90;
    
    if( dlon > 360. ) dlon -= 360.;
    if( dlon < -360. ) dlon += 360.;

    //    This should not really be necessary, but round-trip georef on some charts is not perfect,
    //    So we can get creep on repeated unidimensional pans, and corrupt chart cacheing.......

    //    But this only works on north-up projections
    // TODO: can we remove this now?
    if( ( ( fabs( GetVP().skew() ) < .001 ) ) && ( fabs( GetVP().rotation() ) < .001 ) ) {

        if( dx == 0 ) dlon = lon;
        if( dy == 0 ) dlat = lat;
    }

    int cur_ref_dbIndex = m_pQuilt->GetRefChartdbIndex();

    SetViewPoint( dlat, dlon, VPoint.viewScalePPM(), VPoint.skew(), VPoint.rotation() );

    if( VPoint.quilt()) {
        int new_ref_dbIndex = m_pQuilt->GetRefChartdbIndex();
        if( ( new_ref_dbIndex != cur_ref_dbIndex ) && ( new_ref_dbIndex != -1 ) ) {
            //Tweak the scale slightly for a new ref chart
            ChartBase *pc = ChartData->OpenChartFromDB( new_ref_dbIndex, FULL_INIT );
            if( pc ) {
                double tweak_scale_ppm = pc->GetNearestPreferredScalePPM( VPoint.viewScalePPM() );
                SetVPScale( tweak_scale_ppm );
            }
        }
    }

    //  Turn off bFollow only if the ownship has left the screen
    double offx, offy;
    toSM(dlat, dlon, gLat, gLon, &offx, &offy);
    m_OSoffsetx = offx * VPoint.viewScalePPM();
    m_OSoffsety = offy * VPoint.viewScalePPM();
    
    if( m_bFollow && ((fabs(m_OSoffsetx) > VPoint.pixWidth() / 2) || (fabs(m_OSoffsety) > VPoint.pixHeight() / 2)) ){
        m_bFollow = false;      // update the follow flag
        //        if( m_toolBar )
        //            m_toolBar->GetToolbar()->ToggleTool( ID_FOLLOW, false );

        UpdateFollowButtonState();
    }
    
    Refresh( false );

    //    pCurTrackTimer->Start( m_curtrack_timer_msec, QTimer_ONE_SHOT );

    return true;
}

void ChartCanvas::ReloadVP( bool b_adjust )
{
//    if( g_brightness_init ) SetScreenBrightness( g_nbrightness );

    LoadVP( VPoint, b_adjust );
}

void ChartCanvas::LoadVP( ViewPort &vp, bool b_adjust )
{
    if( m_glcc ) {
        m_glcc->Invalidate();
        if( m_glcc->size() != size() ) {
            m_glcc->resize(size() );
        }
    }
    else
    {
        m_cache_vp.invalidate();
        m_bm_cache_vp.invalidate();
    }

    VPoint.invalidate();

    m_pQuilt->Invalidate();

    //  Make sure that the Selected Group is sensible...
    //    if( m_groupIndex > (int) g_pGroupArray->count() )
    //        m_groupIndex = 0;
    //    if( !CheckGroup( m_groupIndex ) )
    //        m_groupIndex = 0;
    
    SetViewPoint( vp.lat(), vp.lon(), vp.viewScalePPM(), vp.skew(), vp.rotation(), vp.projectType(), b_adjust );

}

void ChartCanvas::SetQuiltRefChart( int dbIndex )
{
    m_pQuilt->SetReferenceChart( dbIndex );
    VPoint.invalidate();
    m_pQuilt->Invalidate();
}

double ChartCanvas::GetBestStartScale(int dbi_hint, const ViewPort &vp)
{
    return m_pQuilt->GetBestStartScale(dbi_hint, vp);
}


//      Verify and adjust the current reference chart,
//      so that it will not lead to excessive overzoom or underzoom onscreen
int ChartCanvas::AdjustQuiltRefChart()
{
    int ret = -1;
    Q_ASSERT(m_pQuilt);

    Q_ASSERT(ChartData);
    ChartBase *pc = ChartData->OpenChartFromDB( m_pQuilt->GetRefChartdbIndex(), FULL_INIT );
    if( pc ) {
        double min_ref_scale = pc->GetNormalScaleMin( m_canvas_scale_factor, false );
        double max_ref_scale = pc->GetNormalScaleMax( m_canvas_scale_factor, m_canvas_width );

        if( VPoint.chartScale() < min_ref_scale )  {
            ret = m_pQuilt->AdjustRefOnZoomIn( VPoint.chartScale() );
        }
        else if( VPoint.chartScale() > max_ref_scale )  {
            ret = m_pQuilt->AdjustRefOnZoomOut( VPoint.chartScale() );
        }
        else {
            bool brender_ok = IsChartLargeEnoughToRender( pc, VPoint );

            int ref_family = pc->GetChartFamily();

            if( !brender_ok ) {
                unsigned int target_stack_index = 0;
                std::vector<int> reslist = m_pQuilt->GetExtendedStackIndexArray();
                int target_stack_index_check = -1;
                if(reslist.size() > m_pQuilt->GetRefChartdbIndex())
                {
                    target_stack_index_check = reslist[m_pQuilt->GetRefChartdbIndex()]; // Lookup
                }

                if( -1 != target_stack_index_check )
                    target_stack_index = target_stack_index_check;

                int extended_array_count = m_pQuilt->GetExtendedStackIndexArray().size();
                while( ( !brender_ok )  && ( (int)target_stack_index < ( extended_array_count - 1 ) ) ) {
                    target_stack_index++;
                    int test_db_index = m_pQuilt->GetExtendedStackIndexArray()[target_stack_index];
                    
                    if( ( ref_family == ChartData->GetDBChartFamily( test_db_index ) )
                            && IsChartQuiltableRef( test_db_index ) ) {
                        //    open the target, and check the min_scale
                        ChartBase *ptest_chart = ChartData->OpenChartFromDB( test_db_index, FULL_INIT );
                        if( ptest_chart ){
                            brender_ok = IsChartLargeEnoughToRender( ptest_chart, VPoint );
                        }
                    }
                }

                if(brender_ok){             // found a better reference chart
                    int new_db_index = m_pQuilt->GetExtendedStackIndexArray()[target_stack_index];
                    if( ( ref_family == ChartData->GetDBChartFamily( new_db_index ) )
                            && IsChartQuiltableRef( new_db_index ) ) {
                        m_pQuilt->SetReferenceChart( new_db_index );
                        ret = new_db_index;
                    }
                    else
                        ret =m_pQuilt->GetRefChartdbIndex();
                }
                else
                    ret = m_pQuilt->GetRefChartdbIndex();

            }
            else
                ret = m_pQuilt->GetRefChartdbIndex();
        }
    }
    else
        ret = -1;
    
    return ret;
}


void ChartCanvas::UpdateCanvasOnGroupChange( void )
{
    delete m_pCurrentStack;
    m_pCurrentStack = NULL;
    m_pCurrentStack = new ChartStack;
    Q_ASSERT(ChartData);
    ChartData->BuildChartStack( m_pCurrentStack, VPoint.lat(), VPoint.lon(), m_groupIndex );

    m_pQuilt->Compose( VPoint );
}

bool ChartCanvas::SetViewPointByCorners( double latSW, double lonSW, double latNE, double lonNE )
{
    // Center Point
    double latc = (latSW + latNE)/2.0;
    double lonc = (lonSW + lonNE)/2.0;
    
    // Get scale in ppm (latitude)
    double ne_easting, ne_northing;
    toSM( latNE, lonNE, latc, lonc, &ne_easting, &ne_northing );
    
    double sw_easting, sw_northing;
    toSM( latSW, lonSW, latc, lonc, &sw_easting, &sw_northing );
    
    double scale_ppm = VPoint.pixHeight() / fabs(ne_northing - sw_northing);

    return SetViewPoint( latc, lonc, scale_ppm, VPoint.skew(), VPoint.rotation() );
}

bool ChartCanvas::SetVPScale( double scale, bool refresh )
{
    return SetViewPoint( VPoint.lat(), VPoint.lon(), scale, VPoint.skew(), VPoint.rotation(),
                         VPoint.projectType(), true, refresh );
}

bool ChartCanvas::SetViewPoint( double lat, double lon )
{
    return SetViewPoint( lat, lon, VPoint.viewScalePPM(), VPoint.skew(), VPoint.rotation() );
}

bool ChartCanvas::SetViewPoint( double lat, double lon, double scale_ppm, double skew,
                                double rotation, int projection, bool b_adjust, bool b_refresh )
{
    bool b_ret = false;

    if(skew > PI) /* so our difference tests work, put in range of +-Pi */
        skew -= 2*PI;

    //  Any sensible change?
    if (VPoint.isValid()) {
        if( ( fabs( VPoint.viewScalePPM() - scale_ppm )/ scale_ppm < 1e-5 )
                && ( fabs( VPoint.skew() - skew ) < 1e-9 )
                && ( fabs( VPoint.rotation() - rotation ) < 1e-9 )
                && ( fabs( VPoint.lat() - lat ) < 1e-9 )
                && ( fabs( VPoint.lon() - lon ) < 1e-9 )
                && (VPoint.projectType() == projection || projection == PROJECTION_UNKNOWN) )
            return false;
    }
    
    if(VPoint.projectType() != projection)
        VPoint.InvalidateTransformCache(); // invalidate

    //    Take a local copy of the last viewport
    ViewPort last_vp = VPoint;

    VPoint.setSkew(skew);
    VPoint.setLat(lat);
    VPoint.setLon(lon);
    VPoint.setViewScalePPM(scale_ppm);
    if(projection != PROJECTION_UNKNOWN)
        VPoint.setProjectionType(projection);
    else
        if(VPoint.projectType() == PROJECTION_UNKNOWN)
            VPoint.setProjectionType(PROJECTION_MERCATOR);

    // don't allow latitude above 88 for mercator (90 is infinity)
    if(VPoint.projectType() == PROJECTION_MERCATOR ||
            VPoint.projectType() == PROJECTION_TRANSVERSE_MERCATOR) {
        if(VPoint.lat() > 89.5) VPoint.setLat(89.5);
        else if(VPoint.lat() < -89.5) VPoint.setLat( -89.5);
    }

    // don't zoom out too far for transverse mercator polyconic until we resolve issues
    if(VPoint.projectType() == PROJECTION_POLYCONIC ||
            VPoint.projectType() == PROJECTION_TRANSVERSE_MERCATOR)
        VPoint.setViewScalePPM(fmax(VPoint.viewScalePPM(), 2e-4));

    SetVPRotation( rotation );

    if( ( VPoint.pixWidth() <= 0 ) || ( VPoint.pixHeight() <= 0 ) )    // Canvas parameters not yet set
        return false;

    VPoint.validate();                     // Mark this ViewPoint as OK

    //  Has the Viewport scale changed?  If so, invalidate the vp
    if( last_vp.viewScalePPM() != scale_ppm ) {
        m_cache_vp.invalidate();
        InvalidateGL();
    }

    //  A preliminary value, may be tweaked below
    VPoint.setChartScale(m_canvas_scale_factor / ( scale_ppm ));

    // recompute cursor position
    // and send to interested plugins if the mouse is actually in this window

    const zchxPoint pt(QCursor::pos());
    //获取当前窗口在屏幕坐标的位置
    QWidget* parent = this->parentWidget();
    QPoint widget_pos = this->pos();
    if(parent) widget_pos = parent->mapToGlobal(widget_pos);
    int mouseX = pt.x - widget_pos.x();
    int mouseY = pt.y - widget_pos.y();
    if( (mouseX > 0) && (mouseX < VPoint.pixWidth()) && (mouseY > 0) && (mouseY < VPoint.pixHeight())){
        double lat, lon;
        GetCanvasPixPoint( mouseX, mouseY, lat, lon );
        m_cursor_lat = lat;
        m_cursor_lon = lon;
//        if(g_pi_manager)
//            g_pi_manager->SendCursorLatLonToAllPlugIns( lat,lon );
    }
    
    if( !VPoint.quilt() && m_singleChart ) {

        VPoint.SetBoxes();

        //  Allow the chart to adjust the new ViewPort for performance optimization
        //  This will normally be only a fractional (i.e.sub-pixel) adjustment...
        if( b_adjust ) m_singleChart->AdjustVP( last_vp, VPoint );

        // If there is a sensible change in the chart render, refresh the whole screen
        if( ( !m_cache_vp.isValid() ) || ( m_cache_vp.viewScalePPM() != VPoint.viewScalePPM() ) ) {
            Refresh( false );
            b_ret = true;
        } else {
            zchxPoint cp_last, cp_this;
            GetCanvasPointPix( m_cache_vp.lat(), m_cache_vp.lon(), cp_last );
            GetCanvasPointPix( VPoint.lat(), VPoint.lon(), cp_this );

            if( cp_last != cp_this ) {
                Refresh( false );
                b_ret = true;
            }
        }
        //  Create the stack
        if( m_pCurrentStack ) {
            assert(ChartData != 0);
            int current_db_index;
            current_db_index = m_pCurrentStack->GetCurrentEntrydbIndex();       // capture the current

            ChartData->BuildChartStack( m_pCurrentStack, lat, lon, current_db_index, m_groupIndex);
            m_pCurrentStack->SetCurrentEntryFromdbIndex( current_db_index );
        }
    }

    //  Handle the quilted case
    if( VPoint.quilt()) {

        if( last_vp.viewScalePPM() != scale_ppm ) m_pQuilt->InvalidateAllQuiltPatchs();

        //  Create the quilt
        if( ChartData /*&& ChartData->IsValid()*/ ) {
            if( !m_pCurrentStack ) return false;

            int current_db_index;
            current_db_index = m_pCurrentStack->GetCurrentEntrydbIndex();       // capture the current

            ChartData->BuildChartStack( m_pCurrentStack, lat, lon, m_groupIndex );
            m_pCurrentStack->SetCurrentEntryFromdbIndex( current_db_index );

            //   Check to see if the current quilt reference chart is in the new stack
            int current_ref_stack_index = -1;
            for( int i = 0; i < m_pCurrentStack->nEntry; i++ ) {
                if( m_pQuilt->GetRefChartdbIndex() == m_pCurrentStack->GetDBIndex( i ) ) current_ref_stack_index =
                        i;
            }

            if( g_bFullScreenQuilt ) {
                current_ref_stack_index = m_pQuilt->GetRefChartdbIndex();
            }
            
            //We might need a new Reference Chart
            bool b_needNewRef = false;

            //    If the new stack does not contain the current ref chart....
            if( ( -1 == current_ref_stack_index ) && ( m_pQuilt->GetRefChartdbIndex() >= 0 ) )
                b_needNewRef = true;
            
            // Would the current Ref Chart be excessively underzoomed?
            // We need to check this here to be sure, since we cannot know where the reference chart was assigned.
            // For instance, the reference chart may have been selected from the config file,
            // or from a long jump with a chart family switch implicit.
            // Anyway, we check to be sure....
            bool renderable = true;
            ChartBase* referenceChart = ChartData->OpenChartFromDB( m_pQuilt->GetRefChartdbIndex(), FULL_INIT );
            if( referenceChart ) {
                double chartMaxScale = referenceChart->GetNormalScaleMax( GetCanvasScaleFactor(), GetCanvasWidth() );
                renderable = chartMaxScale * 64 >= VPoint.chartScale();
            }
            if( !renderable )
                b_needNewRef = true;
            


            //    Need new refchart?
            if( b_needNewRef ) {
                const ChartTableEntry &cte_ref = ChartData->GetChartTableEntry(
                            m_pQuilt->GetRefChartdbIndex() );
                int target_scale = cte_ref.GetScale();
                int target_type = cte_ref.GetChartType();
                int candidate_stack_index;

                //    reset the ref chart in a way that does not lead to excessive underzoom, for performance reasons
                //    Try to find a chart that is the same type, and has a scale of just smaller than the current ref chart

                candidate_stack_index = 0;
                while( candidate_stack_index <= m_pCurrentStack->nEntry - 1 ) {
                    const ChartTableEntry &cte_candidate = ChartData->GetChartTableEntry(
                                m_pCurrentStack->GetDBIndex( candidate_stack_index ) );
                    int candidate_scale = cte_candidate.GetScale();
                    int candidate_type = cte_candidate.GetChartType();

                    if( ( candidate_scale >= target_scale ) && ( candidate_type == target_type ) ){
                        bool renderable = true;
                        ChartBase* tentative_referenceChart = ChartData->OpenChartFromDB( m_pCurrentStack->GetDBIndex( candidate_stack_index ),
                                                                                          FULL_INIT );
                        if( tentative_referenceChart ) {
                            double chartMaxScale = tentative_referenceChart->GetNormalScaleMax( GetCanvasScaleFactor(), GetCanvasWidth() );
                            renderable = chartMaxScale*1.5 > VPoint.chartScale();
                        }
                        
                        if(renderable)
                            break;
                    }

                    candidate_stack_index++;
                }

                //    If that did not work, look for a chart of just larger scale and same type
                if( candidate_stack_index >= m_pCurrentStack->nEntry ) {
                    candidate_stack_index = m_pCurrentStack->nEntry - 1;
                    while( candidate_stack_index >= 0 ) {
                        int idx = m_pCurrentStack->GetDBIndex( candidate_stack_index );
                        if ( idx >= 0) {
                            const ChartTableEntry &cte_candidate = ChartData->GetChartTableEntry(idx);
                            int candidate_scale = cte_candidate.GetScale();
                            int candidate_type = cte_candidate.GetChartType();

                            if( ( candidate_scale <= target_scale ) && ( candidate_type == target_type ) )
                                break;
                        }
                        candidate_stack_index--;
                    }
                }

                // and if that did not work, chose stack entry 0
                if( ( candidate_stack_index >= m_pCurrentStack->nEntry )
                        || ( candidate_stack_index < 0 ) ) candidate_stack_index = 0;

                int new_ref_index = m_pCurrentStack->GetDBIndex( candidate_stack_index );

                m_pQuilt->SetReferenceChart( new_ref_index ); //maybe???

            }
            VPoint.SetBoxes();

            //    If this quilt will be a perceptible delta from the existing quilt, then refresh the entire screen
            if( m_pQuilt->IsQuiltDelta( VPoint ) ) {
                //  Allow the quilt to adjust the new ViewPort for performance optimization
                //  This will normally be only a fractional (i.e. sub-pixel) adjustment...
                if( b_adjust ) m_pQuilt->AdjustQuiltVP( last_vp, VPoint );

                //                ChartData->ClearCacheInUseFlags();
                //                unsigned long hash1 = m_pQuilt->GetXStackHash();

                //                wxStopWatch sw;
                m_pQuilt->Compose( VPoint );
                //                printf("comp time %ld\n", sw.Time());

                //      If the extended chart stack has changed, invalidate any cached render bitmap
                //                if(m_pQuilt->GetXStackHash() != hash1) {
                //                    m_bm_cache_vp.Invalidate();
                //                    InvalidateGL();
                //                }

                ChartData->PurgeCacheUnusedCharts( 0.7 );

                if(b_refresh)
                    Refresh( false );

                b_ret = true;
            }
        }

        VPoint.setSkew(0.);  // Quilting supports 0 Skew
    }
    //  Has the Viewport projection changed?  If so, invalidate the vp
    if( last_vp.projectType() != VPoint.projectType() ) {
        m_cache_vp.invalidate();
        InvalidateGL();
    }


    VPoint.setChartScale(1.0);           // fallback default value
    
    if( !VPoint.getBBox().GetValid() ) VPoint.SetBoxes();

    if( VPoint.getBBox().GetValid() ) {

        //      Update the viewpoint reference scale
        if( m_singleChart )
            VPoint.setRefScale(m_singleChart->GetNativeScale());
        else
            VPoint.setRefScale(m_pQuilt->GetRefNativeScale());

        //    Calculate the on-screen displayed actual scale
        //    by a simple traverse northward from the center point
        //    of roughly one eighth of the canvas height
        zchxPointF r, r1;

        double delta_check = (VPoint.pixHeight() / VPoint.viewScalePPM()) / (1852. * 60);
        delta_check /= 8.;
        
        double check_point = fmin(89., VPoint.lat());

        while((delta_check + check_point) > 90.)
            delta_check /= 2.;

        double rhumbDist;
        DistanceBearingMercator( check_point, VPoint.lon(),
                                 check_point + delta_check, VPoint.lon(),
                                 0, &rhumbDist );

        GetDoubleCanvasPointPix( check_point, VPoint.lon(), r1 );
        GetDoubleCanvasPointPix( check_point + delta_check, VPoint.lon(), r );
        double delta_p = sqrt( ((r1.y - r.y) * (r1.y - r.y)) + ((r1.x - r.x) * (r1.x - r.x)) );
        
        m_true_scale_ppm = delta_p / (rhumbDist * 1852);
        
        //        A fall back in case of very high zoom-out, giving delta_y == 0
        //        which can probably only happen with vector charts
        if( 0.0 == m_true_scale_ppm )
            m_true_scale_ppm = scale_ppm;

        //        Another fallback, for highly zoomed out charts
        //        This adjustment makes the displayed TrueScale correspond to the
        //        same algorithm used to calculate the chart zoom-out limit for ChartDummy.
        if( scale_ppm < 1e-4 )
            m_true_scale_ppm = scale_ppm;

        if( m_true_scale_ppm )
            VPoint.setChartScale(m_canvas_scale_factor / ( m_true_scale_ppm ));
        else
            VPoint.setChartScale(1.0);


        // Create a nice renderable string
        double round_factor = 100.;
        if(VPoint.chartScale() < 1000.)
            round_factor = 10.;
        else if (VPoint.chartScale() < 10000.)
            round_factor = 50.;

        double true_scale_display =  qRound(VPoint.chartScale() / round_factor ) * round_factor;
        QString text;

        m_displayed_scale_factor = VPoint.refScale() / VPoint.chartScale();

        if( m_displayed_scale_factor > 10.0 )
            text.sprintf( "%s %4.0f (%1.0fx)", "Scale", true_scale_display, m_displayed_scale_factor );
        else if( m_displayed_scale_factor > 1.0 )
            text.sprintf("%s %4.0f (%1.1fx)", "Scale", true_scale_display, m_displayed_scale_factor );
        else if( m_displayed_scale_factor > 0.1 ){
            double sfr = qRound(m_displayed_scale_factor * 10.) / 10.;
            text.sprintf("%s %4.0f (%1.2fx)", "Scale", true_scale_display, sfr );
        }
        else if( m_displayed_scale_factor > 0.01 ){
            double sfr = qRound(m_displayed_scale_factor * 100.) / 100.;
            text.sprintf("%s %4.0f (%1.2fx)", "Scale", true_scale_display, sfr );
        }
        else  {
            text.sprintf("%s %4.0f (---)", "Scale", true_scale_display );      // Generally, no chart, so no chart scale factor
        }

        if( g_bShowFPS){
            QString fps_str;
            double fps = 0.;
            if( g_gl_ms_per_frame > 0){
                fps = 1000./ g_gl_ms_per_frame;
                fps_str.sprintf("  %3d fps", (int)fps);
            }
            text += fps_str;
        }
        m_scaleValue = true_scale_display;
        m_scaleText = text;
//        if(m_muiBar)
//            m_muiBar->UpdateDynamicValues();
//        if( m_bShowScaleInStatusBar && parent_frame->GetStatusBar() && (parent_frame->GetStatusBar()->GetFieldsCount() > STAT_FIELD_SCALE) ) {
//            // Check to see if the text will fit in the StatusBar field...
//            bool b_noshow = false;
//            {
//                int w = 0;
//                int h;
//                wxClientDC dc(parent_frame->GetStatusBar());
//                if( dc.IsOk() ){
//                    QFont* templateFont = FontMgr::Get().GetFont( _("StatusBar"), 0 );
//                    dc.SetFont(*templateFont);
//                    dc.GetTextExtent(text, &w, &h);


//                    // If text is too long for the allocated field, try to reduce the text string a bit.
//                    wxRect rect;
//                    parent_frame->GetStatusBar()->GetFieldRect(STAT_FIELD_SCALE, rect);
//                    if(w && w > rect.width){
//                        text.Printf( _T("%s (%1.1fx)"), _("Scale"), m_displayed_scale_factor );
//                    }
                    
//                    //  Test again...if too big still, then give it up.
//                    dc.GetTextExtent(text, &w, &h);
                    
//                    if(w && w > rect.width){
//                        b_noshow = true;
//                    }
//                }
//            }

//            if(!b_noshow)
//                parent_frame->SetStatusText( text, STAT_FIELD_SCALE );
//        }


    }

    //  Maintain member vLat/vLon
    m_vLat = VPoint.lat();
    m_vLon = VPoint.lon();

    return b_ret;
}


//          Static Icon definitions for some symbols requiring scaling/rotation/translation
//          Very specific wxDC draw commands are necessary to properly render these icons...See the code in ShipDraw()

//      This icon was adapted and scaled from the S52 Presentation Library version 3_03.
//     Symbol VECGND02

static int s_png_pred_icon[] = { -10, -10, -10, 10, 10, 10, 10, -10 };

//      This ownship icon was adapted and scaled from the S52 Presentation Library version 3_03
//      Symbol OWNSHP05
static int s_ownship_icon[] = { 5, -42, 11, -28, 11, 42, -11, 42, -11, -28, -5, -42, -11, 0, 11, 0,
                                0, 42, 0, -42
                              };

QColor ChartCanvas::PredColor()
{ 
//    //  RAdjust predictor color change on LOW_ACCURACY ship state in interests of visibility.
//    if( SHIP_NORMAL == m_ownship_state )
//        return GetGlobalColor( _T ( "URED" ) );

//    else if( SHIP_LOWACCURACY == m_ownship_state )
//        return GetGlobalColor( _T ( "YELO1" ) );

//    return GetGlobalColor( _T ( "NODTA" ) );
}

QColor ChartCanvas::ShipColor()
{ 
    //      Establish ship color
//    //     It changes color based on GPS and Chart accuracy/availability

//    if( SHIP_NORMAL != m_ownship_state )
//        return GetGlobalColor( _T ( "GREY1" ) );

//    if( SHIP_LOWACCURACY == m_ownship_state )
//        return GetGlobalColor( _T ( "YELO1" ) );

//    return GetGlobalColor( _T ( "URED" ) );         // default is OK
}

#if 0
void ChartCanvas::ShipDrawLargeScale( ocpnDC& dc, zchxPoint lShipMidPoint )
{

    dc.SetPen( QPen( PredColor(), 2 ) );

    if( SHIP_NORMAL == m_ownship_state )
        dc.SetBrush( wxBrush( ShipColor(), wxBRUSHSTYLE_TRANSPARENT ) );
    else
        dc.SetBrush( wxBrush( GetGlobalColor( _T ( "YELO1" ) ) ) );

    dc.DrawEllipse( lShipMidPoint.x - 10, lShipMidPoint.y - 10, 20, 20 );
    dc.DrawEllipse( lShipMidPoint.x - 6, lShipMidPoint.y - 6, 12, 12 );

    dc.DrawLine( lShipMidPoint.x - 12, lShipMidPoint.y, lShipMidPoint.x + 12, lShipMidPoint.y );
    dc.DrawLine( lShipMidPoint.x, lShipMidPoint.y - 12, lShipMidPoint.x, lShipMidPoint.y + 12 );
}

void ChartCanvas::ShipIndicatorsDraw( ocpnDC& dc, int img_height,
                                      zchxPoint GPSOffsetPixels, zchxPoint lGPSPoint)
{
    // Develop a uniform length for course predictor line dash length, based on physical display size
    // Use this reference length to size all other graphics elements
    float ref_dim = m_display_size_mm / 24;
    ref_dim = fmin(ref_dim, 12);
    ref_dim = fmax(ref_dim, 6);
    
    QColor cPred = PredColor();
    
    //  Establish some graphic element line widths dependent on the platform display resolution
    //double nominal_line_width_pix = fmax(1.0, floor(g_Platform->GetDisplayDPmm() / 2));             //0.5 mm nominal, but not less than 1 pixel
    double nominal_line_width_pix = fmax(1.0, floor(m_pix_per_mm / 2));             //0.5 mm nominal, but not less than 1 pixel
    
    
    // If the calculated value is greater than the config file spec value, then use it.
    if(nominal_line_width_pix > g_cog_predictor_width)
        g_cog_predictor_width = nominal_line_width_pix;
    
    //    Calculate ownship Position Predictor
    zchxPoint lPredPoint, lHeadPoint;

    float pCog = std::isnan(gCog) ? 0 : gCog;
    float pSog = std::isnan(gSog) ? 0 : gSog;

    double pred_lat, pred_lon;
    ll_gc_ll( gLat, gLon, pCog, pSog * g_ownship_predictor_minutes / 60., &pred_lat, &pred_lon );
    GetCanvasPointPix( pred_lat, pred_lon, &lPredPoint );
    
    // test to catch the case where COG/HDG line crosses the screen
    LLBBox box;

    //    Should we draw the Head vector?
    //    Compare the points lHeadPoint and lPredPoint
    //    If they differ by more than n pixels, and the head vector is valid, then render the head vector


    float ndelta_pix = 10.;
    double hdg_pred_lat, hdg_pred_lon;
    bool b_render_hdt = false;
    if( !std::isnan( gHdt ) ) {
        //    Calculate ownship Heading pointer as a predictor
        ll_gc_ll( gLat, gLon, gHdt, g_ownship_HDTpredictor_miles, &hdg_pred_lat,
                  &hdg_pred_lon );
        GetCanvasPointPix( hdg_pred_lat, hdg_pred_lon, &lHeadPoint );
        float dist = sqrtf( powf(  (float) (lHeadPoint.x - lPredPoint.x), 2) +
                            powf(  (float) (lHeadPoint.y - lPredPoint.y), 2) );
        if( dist > ndelta_pix /*&& !std::isnan(gSog)*/ ) {
            box.SetFromSegment(gLat, gLon, hdg_pred_lat, hdg_pred_lon);
            if( !GetVP().getBBox().IntersectOut(box))
                b_render_hdt = true;
        }
    }

    // draw course over ground if they are longer than the ship
    zchxPoint lShipMidPoint;
    lShipMidPoint.x = lGPSPoint.x + GPSOffsetPixels.x;
    lShipMidPoint.y = lGPSPoint.y + GPSOffsetPixels.y;
    float lpp = sqrtf( powf( (float) (lPredPoint.x - lShipMidPoint.x), 2) +
                       powf( (float) (lPredPoint.y - lShipMidPoint.y), 2) );

    if(lpp >= img_height / 2) {
        box.SetFromSegment(gLat, gLon, pred_lat, pred_lon);
        if( !GetVP().getBBox().IntersectOut(box) && !std::isnan(gCog) && !std::isnan(gSog) ) {
            
            //      COG Predictor
            float dash_length = ref_dim;
            wxDash dash_long[2];
            dash_long[0] = (int) ( floor(g_Platform->GetDisplayDPmm() * dash_length) / g_cog_predictor_width );  // Long dash , in mm <---------+
            dash_long[1] = dash_long[0] / 2.0;                                                                   // Short gap
            
            // On ultra-hi-res displays, do not allow the dashes to be greater than 250, since it is defined as (char)
            if( dash_length > 250.){
                dash_long[0] = 250. /g_cog_predictor_width;
                dash_long[1] = dash_long[0] / 2;
            }

            QPen ppPen2( cPred, g_cog_predictor_width, QPenSTYLE_USER_DASH );
            ppPen2.SetDashes( 2, dash_long );
            dc.SetPen( ppPen2 );
            dc.StrokeLine( lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
                           lPredPoint.x + GPSOffsetPixels.x, lPredPoint.y + GPSOffsetPixels.y );

            if( g_cog_predictor_width > 1 ) {
                float line_width = g_cog_predictor_width/3.;

                wxDash dash_long3[2];
                dash_long3[0] = g_cog_predictor_width / line_width * dash_long[0];
                dash_long3[1] = g_cog_predictor_width / line_width * dash_long[1];

                QPen ppPen3( GetGlobalColor( _T ( "UBLCK" ) ), fmax(1, line_width), QPenSTYLE_USER_DASH );
                ppPen3.SetDashes( 2, dash_long3 );
                dc.SetPen( ppPen3 );
                dc.StrokeLine( lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
                               lPredPoint.x + GPSOffsetPixels.x, lPredPoint.y + GPSOffsetPixels.y );
            }


            // Prepare COG predictor endpoint icon
            double png_pred_icon_scale_factor = .4;
            if(g_ShipScaleFactorExp > 1.0)
                png_pred_icon_scale_factor *= (log(g_ShipScaleFactorExp) + 1.0) * 1.1;

            zchxPoint icon[4];

            float cog_rad = atan2f( (float) ( lPredPoint.y - lShipMidPoint.y ),
                                    (float) ( lPredPoint.x - lShipMidPoint.x ) );
            cog_rad += (float)PI;

            for( int i = 0; i < 4; i++ ) {
                int j = i * 2;
                double pxa = (double) ( s_png_pred_icon[j] );
                double pya = (double) ( s_png_pred_icon[j + 1] );

                pya *= png_pred_icon_scale_factor;
                pxa *= png_pred_icon_scale_factor;

                double px = ( pxa * sin( cog_rad ) ) + ( pya * cos( cog_rad ) );
                double py = ( pya * sin( cog_rad ) ) - ( pxa * cos( cog_rad ) );

                icon[i].x = (int) qRound( px ) + lPredPoint.x + GPSOffsetPixels.x;
                icon[i].y = (int) qRound( py ) + lPredPoint.y + GPSOffsetPixels.y;
            }

            // Render COG endpoint icon
            QPen ppPen1( GetGlobalColor( _T ( "UBLCK" ) ), g_cog_predictor_width/2, Qt::SolidLine );
            dc.SetPen( ppPen1 );
            dc.SetBrush( wxBrush( cPred ) );

            dc.StrokePolygon( 4, icon );
        }
    }

    //      HDT Predictor
    if( b_render_hdt ) {
        float hdt_dash_length = ref_dim * 0.4;

        float hdt_width = g_cog_predictor_width * 0.8;
        wxDash dash_short[2];
        dash_short[0] = (int) ( floor(g_Platform->GetDisplayDPmm() * hdt_dash_length) / hdt_width );  // Short dash , in mm <---------+
        dash_short[1] = (int) ( floor(g_Platform->GetDisplayDPmm() * hdt_dash_length * 0.9 ) / hdt_width );  // Short gap            |

        QPen ppPen2( cPred, hdt_width, QPenSTYLE_USER_DASH );
        ppPen2.SetDashes( 2, dash_short );

        dc.SetPen( ppPen2 );
        dc.StrokeLine( lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
                       lHeadPoint.x + GPSOffsetPixels.x, lHeadPoint.y + GPSOffsetPixels.y );

        QPen ppPen1( cPred, g_cog_predictor_width/3, Qt::SolidLine );
        dc.SetPen( ppPen1 );
        dc.SetBrush( wxBrush( GetGlobalColor( _T ( "GREY2" ) ) ) );

        double nominal_circle_size_pixels = fmax(4.0, floor(m_pix_per_mm * (ref_dim / 5.0)));    // not less than 4 pixel

        // Scale the circle to ChartScaleFactor, slightly softened....
        if(g_ShipScaleFactorExp > 1.0)
            nominal_circle_size_pixels *= (log(g_ShipScaleFactorExp) + 1.0) * 1.1;

        dc.StrokeCircle( lHeadPoint.x + GPSOffsetPixels.x, lHeadPoint.y + GPSOffsetPixels.y, nominal_circle_size_pixels/2 );
    }

    // Draw radar rings if activated
    if( g_iNavAidRadarRingsNumberVisible ) {
        double factor = 1.00;
        if( g_pNavAidRadarRingsStepUnits == 1 )          // nautical miles
            factor = 1 / 1.852;

        factor *= g_fNavAidRadarRingsStep;

        double tlat, tlon;
        zchxPoint r;
        ll_gc_ll( gLat, gLon, 0, factor, &tlat, &tlon );
        GetCanvasPointPix( tlat, tlon, &r );

        double lpp = sqrt( pow( (double) (lGPSPoint.x - r.x), 2) +
                           pow( (double) (lGPSPoint.y - r.y), 2 ) );
        int pix_radius = (int) lpp;

        extern wxColor GetDimColor(wxColor c);
        wxColor rangeringcolour = GetDimColor(g_colourOwnshipRangeRingsColour);

        QPen ppPen1( rangeringcolour, g_cog_predictor_width );

        dc.SetPen( ppPen1 );
        dc.SetBrush( wxBrush( rangeringcolour, wxBRUSHSTYLE_TRANSPARENT ) );

        for( int i = 1; i <= g_iNavAidRadarRingsNumberVisible; i++ )
            dc.StrokeCircle( lGPSPoint.x, lGPSPoint.y, i * pix_radius );
    }
}

void ChartCanvas::ComputeShipScaleFactor(float icon_hdt,
                                         int ownShipWidth, int ownShipLength,
                                         zchxPoint &lShipMidPoint,
                                         zchxPoint &GPSOffsetPixels, zchxPoint lGPSPoint,
                                         float &scale_factor_x, float &scale_factor_y)
{
    float screenResolution = (float) ::wxGetDisplaySize().x / g_display_size_mm;

    //  Calculate the true ship length in exact pixels
    double ship_bow_lat, ship_bow_lon;
    ll_gc_ll( gLat, gLon, icon_hdt, g_n_ownship_length_meters / 1852., &ship_bow_lat,
              &ship_bow_lon );
    zchxPoint lShipBowPoint;
    zchxPoint2DDouble b_point = GetVP().GetDoublePixFromLL( ship_bow_lat, ship_bow_lon );
    zchxPoint2DDouble a_point = GetVP().GetDoublePixFromLL( gLat, gLon );
    
    float shipLength_px = sqrtf( powf( (float) (b_point.m_x - a_point.m_x), 2) +
                                 powf( (float) (b_point.m_y - a_point.m_y), 2) );
    
    //  And in mm
    float shipLength_mm = shipLength_px / screenResolution;
    
    //  Set minimum ownship drawing size
    float ownship_min_mm = g_n_ownship_min_mm;
    ownship_min_mm = fmax(ownship_min_mm, 1.0);
    
    //  Calculate Nautical Miles distance from midships to gps antenna
    float hdt_ant = icon_hdt + 180.;
    float dy = ( g_n_ownship_length_meters / 2 - g_n_gps_antenna_offset_y ) / 1852.;
    float dx = g_n_gps_antenna_offset_x / 1852.;
    if( g_n_gps_antenna_offset_y > g_n_ownship_length_meters / 2 )      //reverse?
    {
        hdt_ant = icon_hdt;
        dy = -dy;
    }
    
    //  If the drawn ship size is going to be clamped, adjust the gps antenna offsets
    if( shipLength_mm < ownship_min_mm ) {
        dy /= shipLength_mm / ownship_min_mm;
        dx /= shipLength_mm / ownship_min_mm;
    }

    double ship_mid_lat, ship_mid_lon, ship_mid_lat1, ship_mid_lon1;
    
    ll_gc_ll( gLat, gLon, hdt_ant, dy, &ship_mid_lat, &ship_mid_lon );
    ll_gc_ll( ship_mid_lat, ship_mid_lon, icon_hdt - 90., dx, &ship_mid_lat1, &ship_mid_lon1 );

    GetCanvasPointPix( ship_mid_lat1, ship_mid_lon1, &lShipMidPoint );
    GPSOffsetPixels.x = lShipMidPoint.x - lGPSPoint.x;
    GPSOffsetPixels.y = lShipMidPoint.y - lGPSPoint.y;
    
    float scale_factor = shipLength_px / ownShipLength;
    
    //  Calculate a scale factor that would produce a reasonably sized icon
    float scale_factor_min = ownship_min_mm / ( ownShipLength / screenResolution );
    
    //  And choose the correct one
    scale_factor = fmax(scale_factor, scale_factor_min);
    
    scale_factor_y = scale_factor;
    scale_factor_x = scale_factor_y * ( (float) ownShipLength / ownShipWidth )
            / ( (float) g_n_ownship_length_meters / g_n_ownship_beam_meters );
}

void ChartCanvas::ShipDraw( ocpnDC& dc )
{
    if( !GetVP().IsValid() ) return;

    zchxPoint lGPSPoint, lShipMidPoint, GPSOffsetPixels(0,0);

    //  COG/SOG may be undefined in NMEA data stream
    float pCog = std::isnan(gCog) ? 0 : gCog;
    float pSog = std::isnan(gSog) ? 0 : gSog;

    GetCanvasPointPix( gLat, gLon, &lGPSPoint );
    lShipMidPoint = lGPSPoint;

    //  Draw the icon rotated to the COG
    //  or to the Hdt if available
    float icon_hdt = pCog;
    if( !std::isnan( gHdt ) ) icon_hdt = gHdt;

    //  COG may be undefined in NMEA data stream
    if( std::isnan(icon_hdt) ) icon_hdt = 0.0;

    //    Calculate the ownship drawing angle icon_rad using an assumed 10 minute predictor
    double osd_head_lat, osd_head_lon;
    zchxPoint osd_head_point;

    ll_gc_ll( gLat, gLon, icon_hdt, pSog * 10. / 60., &osd_head_lat, &osd_head_lon );

    GetCanvasPointPix( osd_head_lat, osd_head_lon, &osd_head_point );

    float icon_rad = atan2f( (float) ( osd_head_point.y - lShipMidPoint.y ),
                             (float) ( osd_head_point.x - lShipMidPoint.x ) );
    icon_rad += (float)PI;

    if (pSog < 0.2) icon_rad = ((icon_hdt + 90.) * PI / 180) + GetVP().rotation;

    //    Another draw test ,based on pixels, assuming the ship icon is a fixed nominal size
    //    and is just barely outside the viewport        ....
    wxBoundingBox bb_screen( 0, 0, GetVP().pixWidth(), GetVP().pixHeight() );

    // TODO: fix to include actual size of boat that will be rendered
    int img_height = 0;
    if( bb_screen.PointInBox( lShipMidPoint, 20 ) ) {
        if( GetVP().chartScale() > 300000 )             // According to S52, this should be 50,000
        {
            ShipDrawLargeScale(dc, lShipMidPoint);
            img_height = 20;
        } else {

            QImage pos_image;

            //      Substitute user ownship image if found
            if( m_pos_image_user )
                pos_image = m_pos_image_user->Copy();
            else if( SHIP_NORMAL == m_ownship_state )
                pos_image = m_pos_image_red->Copy();
            if( SHIP_LOWACCURACY == m_ownship_state )
                pos_image = m_pos_image_yellow->Copy();
            else if( SHIP_NORMAL != m_ownship_state )
                pos_image = m_pos_image_grey->Copy();


            //      Substitute user ownship image if found
            if( m_pos_image_user ) {
                pos_image = m_pos_image_user->Copy();
                
                if( SHIP_LOWACCURACY == m_ownship_state )
                    pos_image = m_pos_image_user_yellow->Copy();
                else if( SHIP_NORMAL != m_ownship_state )
                    pos_image = m_pos_image_user_grey->Copy();
            }

            img_height = pos_image.height();

            if( g_n_ownship_beam_meters > 0.0 &&
                    g_n_ownship_length_meters > 0.0 &&
                    g_OwnShipIconType > 0 ) // use large ship
            {
                int ownShipWidth = 22; // Default values from s_ownship_icon
                int ownShipLength= 84;
                if( g_OwnShipIconType == 1 ) {
                    ownShipWidth = pos_image.width();
                    ownShipLength= pos_image.height();
                }
                
                float scale_factor_x, scale_factor_y;
                ComputeShipScaleFactor
                        (icon_hdt, ownShipWidth, ownShipLength, lShipMidPoint,
                         GPSOffsetPixels, lGPSPoint, scale_factor_x, scale_factor_y);

                if( g_OwnShipIconType == 1 ) { // Scaled bitmap
                    pos_image.Rescale( ownShipWidth * scale_factor_x, ownShipLength * scale_factor_y,
                                       QImage_QUALITY_HIGH );
                    zchxPoint rot_ctr( pos_image.width() / 2, pos_image.height() / 2 );
                    QImage rot_image = pos_image.Rotate( -( icon_rad - ( PI / 2. ) ), rot_ctr, true );
                    
                    // Simple sharpening algorithm.....
                    for( int ip = 0; ip < rot_image.width(); ip++ )
                        for( int jp = 0; jp < rot_image.height(); jp++ )
                            if( rot_image.GetAlpha( ip, jp ) > 64 ) rot_image.SetAlpha( ip, jp, 255 );

                    wxBitmap os_bm( rot_image );

                    int w = os_bm.width();
                    int h = os_bm.height();
                    img_height = h;

                    dc.DrawBitmap( os_bm, lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2, true );
                    
                    // Maintain dirty box,, missing in __WXMSW__ library
                    dc.CalcBoundingBox( lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2 );
                    dc.CalcBoundingBox( lShipMidPoint.x - w / 2 + w, lShipMidPoint.y - h / 2 + h );
                }

                else if( g_OwnShipIconType == 2 ) { // Scaled Vector
                    zchxPoint ownship_icon[10];

                    for( int i = 0; i < 10; i++ ) {
                        int j = i * 2;
                        float pxa = (float) ( s_ownship_icon[j] );
                        float pya = (float) ( s_ownship_icon[j + 1] );
                        pya *= scale_factor_y;
                        pxa *= scale_factor_x;

                        float px = ( pxa * sinf( icon_rad ) ) + ( pya * cosf( icon_rad ) );
                        float py = ( pya * sinf( icon_rad ) ) - ( pxa * cosf( icon_rad ) );

                        ownship_icon[i].x = (int) ( px ) + lShipMidPoint.x;
                        ownship_icon[i].y = (int) ( py ) + lShipMidPoint.y;
                    }

                    QPen ppPen1( GetGlobalColor( _T ( "UBLCK" ) ), 1, Qt::SolidLine );
                    dc.SetPen( ppPen1 );
                    dc.SetBrush( wxBrush( ShipColor() ) );
                    
                    dc.StrokePolygon( 6, &ownship_icon[0], 0, 0 );
                    
                    //     draw reference point (midships) cross
                    dc.StrokeLine( ownship_icon[6].x, ownship_icon[6].y, ownship_icon[7].x,
                            ownship_icon[7].y );
                    dc.StrokeLine( ownship_icon[8].x, ownship_icon[8].y, ownship_icon[9].x,
                            ownship_icon[9].y );
                }

                img_height = ownShipLength * scale_factor_y;

                //      Reference point, where the GPS antenna is
                int circle_rad = 3;
                if( m_pos_image_user ) circle_rad = 1;
                
                dc.SetPen( QPen( GetGlobalColor( _T ( "UBLCK" ) ), 1 ) );
                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UIBCK" ) ) ) );
                dc.StrokeCircle( lGPSPoint.x, lGPSPoint.y, circle_rad );
            }
            else { // Fixed bitmap icon.
                /* non opengl, or suboptimal opengl via ocpndc: */
                zchxPoint rot_ctr( pos_image.width() / 2, pos_image.height() / 2 );
                QImage rot_image = pos_image.Rotate( -( icon_rad - ( PI / 2. ) ), rot_ctr, true );
                
                // Simple sharpening algorithm.....
                for( int ip = 0; ip < rot_image.width(); ip++ )
                    for( int jp = 0; jp < rot_image.height(); jp++ )
                        if( rot_image.GetAlpha( ip, jp ) > 64 ) rot_image.SetAlpha( ip, jp, 255 );
                
                wxBitmap os_bm( rot_image );
                
                if(g_ShipScaleFactorExp > 1){
                    QImage scaled_image = os_bm.ConvertToImage();
                    double factor = (log(g_ShipScaleFactorExp) + 1.0) * 1.0;   // soften the scale factor a bit
                    os_bm = wxBitmap(scaled_image.Scale(scaled_image.width() * factor,
                                                        scaled_image.height() * factor,
                                                        QImage_QUALITY_HIGH));
                }
                int w = os_bm.width();
                int h = os_bm.height();
                img_height = h;
                
                dc.DrawBitmap( os_bm, lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2, true );
                
                //      Reference point, where the GPS antenna is
                int circle_rad = 3;
                if( m_pos_image_user ) circle_rad = 1;
                
                dc.SetPen( QPen( GetGlobalColor( _T ( "UBLCK" ) ), 1 ) );
                dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UIBCK" ) ) ) );
                dc.StrokeCircle( lShipMidPoint.x, lShipMidPoint.y, circle_rad );
                
                // Maintain dirty box,, missing in __WXMSW__ library
                dc.CalcBoundingBox( lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2 );
                dc.CalcBoundingBox( lShipMidPoint.x - w / 2 + w, lShipMidPoint.y - h / 2 + h );
            }
        }        // ownship draw
    }

    ShipIndicatorsDraw(dc, img_height, GPSOffsetPixels, lGPSPoint);
}
#endif

/* @ChartCanvas::CalcGridSpacing
 **
 ** Calculate the major and minor spacing between the lat/lon grid
 **
 ** @param [r] WindowDegrees [float] displayed number of lat or lan in the window
 ** @param [w] MajorSpacing [float &] Major distance between grid lines
 ** @param [w] MinorSpacing [float &] Minor distance between grid lines
 ** @return [void]
 */
void CalcGridSpacing( float view_scale_ppm, float& MajorSpacing, float&MinorSpacing )
{
    // table for calculating the distance between the grids
    // [0] view_scale ppm
    // [1] spacing between major grid lines in degrees
    // [2] spacing between minor grid lines in degrees
    const float lltab[][3] =
    { {  0.0f, 90.0f, 30.0f },
      { .000001f, 45.0f, 15.0f },
      { .0002f,   30.0f, 10.0f },
      { .0003f,   10.0f, 2.0f  },
      { .0008f,   5.0f, 1.0f },
      { .001f,    2.0f,          30.0f / 60.0f },
      { .003f,    1.0f,          20.0f / 60.0f },
      { .006f,    0.5f,          10.0f / 60.0f },
      { .03f,     15.0f / 60.0f, 5.0f / 60.0f },
      { .01f,     10.0f / 60.0f, 2.0f / 60.0f },
      { .06f,     5.0f / 60.0f,  1.0f / 60.0f },
      { .1f,      2.0f / 60.0f,  1.0f / 60.0f },
      { .4f,      1.0f / 60.0f,  0.5f / 60.0f },
      { .6f,      0.5f / 60.0f,  0.1f / 60.0f },
      { 1.0f,     0.2f / 60.0f,  0.1f / 60.0f },
      { 1e10f,    0.1f / 60.0f,  0.05f / 60.0f }
    };

    unsigned int tabi;
    for( tabi = 0; tabi < ((sizeof lltab) / (sizeof *lltab)) -1; tabi++ )
        if( view_scale_ppm < lltab[tabi][0] )
            break;
    MajorSpacing = lltab[tabi][1]; // major latitude distance
    MinorSpacing = lltab[tabi][2]; // minor latitude distance
    return;
}
/* @ChartCanvas::CalcGridText *************************************
 **
 ** Calculates text to display at the major grid lines
 **
 ** @param [r] latlon [float] latitude or longitude of grid line
 ** @param [r] spacing [float] distance between two major grid lines
 ** @param [r] bPostfix [bool] true for latitudes, false for longitudes
 **
 ** @return
 */

QString CalcGridText( float latlon, float spacing, bool bPostfix )
{
    int deg = (int) fabs( latlon ); // degrees
    float min = fabs( ( fabs( latlon ) - deg ) * 60.0 ); // Minutes
    char postfix;
    
    // calculate postfix letter (NSEW)
    if( latlon > 0.0 ) {
        if( bPostfix ) {
            postfix = 'N';
        } else {
            postfix = 'E';
        }
    } else if( latlon < 0.0 ) {
        if( bPostfix ) {
            postfix = 'S';
        } else {
            postfix = 'W';
        }
    } else {
        postfix = ' '; // no postfix for equator and greenwich
    }
    // calculate text, display minutes only if spacing is smaller than one degree

    QString ret;
    if( spacing >= 1.0 ) {
        ret.sprintf( "%3d%c %c", deg, 0x00b0, postfix );
    } else if( spacing >= ( 1.0 / 60.0 ) ) {
        ret.sprintf( "%3d%c%02.0f %c", deg, 0x00b0, min, postfix );
    } else {
        ret.sprintf( "%3d%c%02.2f %c", deg, 0x00b0, min, postfix );
    }

    return ret;
}

/* @ChartCanvas::GridDraw *****************************************
 **
 ** Draws major and minor Lat/Lon Grid on the chart
 ** - distance between Grid-lm ines are calculated automatic
 ** - major grid lines will be across the whole chart window
 ** - minor grid lines will be 10 pixel at each edge of the chart window.
 **
 ** @param [w] dc [wxDC&] the wx drawing context
 **
 ** @return [void]
 ************************************************************************/
void ChartCanvas::GridDraw( ocpnDC& dc )
{
    if( !( m_bDisplayGrid && ( fabs( GetVP().rotation() ) < 1e-5 ) ) )
        return;

    double nlat, elon, slat, wlon;
    float lat, lon;
    float dlat, dlon;
    float gridlatMajor, gridlatMinor, gridlonMajor, gridlonMinor;
    int w, h;
    QPen GridPen( GetGlobalColor(  "SNDG1"  ), 1, Qt::SolidLine );
    dc.SetPen( GridPen );
    dc.SetFont( m_pgridFont );
    dc.SetTextForeground( GetGlobalColor( ( "SNDG1" ) ) );

    w = m_canvas_width;
    h = m_canvas_height;

    GetCanvasPixPoint( 0, 0, nlat, wlon ); // get lat/lon of upper left point of the window
    GetCanvasPixPoint( w, h, slat, elon ); // get lat/lon of lower right point of the window
    dlat = nlat - slat; // calculate how many degrees of latitude are shown in the window
    dlon = elon - wlon; // calculate how many degrees of longitude are shown in the window
    if( dlon < 0.0 ) // concider datum border at 180 degrees longitude
    {
        dlon = dlon + 360.0;
    }
    // calculate distance between latitude grid lines
    CalcGridSpacing( GetVP().viewScalePPM(), gridlatMajor, gridlatMinor );

    // calculate position of first major latitude grid line
    lat = ceil( slat / gridlatMajor ) * gridlatMajor;

    // Draw Major latitude grid lines and text
    while( lat < nlat ) {
        zchxPoint r;
        QString st = CalcGridText( lat, gridlatMajor, true ); // get text for grid line
        GetCanvasPointPix( lat, ( elon + wlon ) / 2, r );
        dc.DrawLine( 0, r.y, w, r.y, false );                             // draw grid line
        dc.drawText( st, 0, r.y ); // draw text
        lat = lat + gridlatMajor;

        if( fabs( lat - qRound( lat ) ) < 1e-5 ) lat = qRound( lat );
    }

    // calculate position of first minor latitude grid line
    lat = ceil( slat / gridlatMinor ) * gridlatMinor;

    // Draw minor latitude grid lines
    while( lat < nlat ) {
        zchxPoint r;
        GetCanvasPointPix( lat, ( elon + wlon ) / 2, r );
        dc.DrawLine( 0, r.y, 10, r.y, false );
        dc.DrawLine( w - 10, r.y, w, r.y, false );
        lat = lat + gridlatMinor;
    }

    // calculate distance between grid lines
    CalcGridSpacing( GetVP().viewScalePPM(), gridlonMajor, gridlonMinor );

    // calculate position of first major latitude grid line
    lon = ceil( wlon / gridlonMajor ) * gridlonMajor;

    // draw major longitude grid lines
    for( int i = 0, itermax = (int) ( dlon / gridlonMajor ); i <= itermax; i++ ) {
        zchxPoint r;
        QString st = CalcGridText( lon, gridlonMajor, false );
        GetCanvasPointPix( ( nlat + slat ) / 2, lon, r );
        dc.DrawLine( r.x, 0, r.x, h, false );
        dc.drawText( st, r.x, 0 );
        lon = lon + gridlonMajor;
        if( lon > 180.0 ) {
            lon = lon - 360.0;
        }

        if( fabs( lon - qRound( lon ) ) < 1e-5 ) lon = qRound( lon );

    }

    // calculate position of first minor longitude grid line
    lon = ceil( wlon / gridlonMinor ) * gridlonMinor;
    // draw minor longitude grid lines
    for( int i = 0, itermax = (int) ( dlon / gridlonMinor ); i <= itermax; i++ ) {
        zchxPoint r;
        GetCanvasPointPix( ( nlat + slat ) / 2, lon, r );
        dc.DrawLine( r.x, 0, r.x, 10, false );
        dc.DrawLine( r.x, h - 10, r.x, h, false );
        lon = lon + gridlonMinor;
        if( lon > 180.0 ) {
            lon = lon - 360.0;
        }
    }
}

void ChartCanvas::ScaleBarDraw( ocpnDC& dc )
{
    if(0 /*!g_bsimplifiedScalebar*/){
        double blat, blon, tlat, tlon;
        zchxPoint r;

        int x_origin = m_bDisplayGrid ? 60 : 20;
        int y_origin = m_canvas_height - 50;

        float dist;
        int count;
        QPen pen1, pen2;

        if( GetVP().chartScale() > 80000 )        // Draw 10 mile scale as SCALEB11
        {
            dist = 10.0;
            count = 5;
            pen1 = QPen( GetGlobalColor( ( "SNDG2" ) ), 3, Qt::SolidLine );
            pen2 = QPen( GetGlobalColor( ( "SNDG1" ) ), 3, Qt::SolidLine );
        } else                                // Draw 1 mile scale as SCALEB10
        {
            dist = 1.0;
            count = 10;
            pen1 = QPen( GetGlobalColor(  ( "SCLBR" ) ), 3, Qt::SolidLine );
            pen2 = QPen( GetGlobalColor(  ( "CHGRD" ) ), 3, Qt::SolidLine );
        }

        GetCanvasPixPoint( x_origin, y_origin, blat, blon );
        double rotation = -VPoint.rotation();

        ll_gc_ll( blat, blon, rotation * 180 / PI, dist, &tlat, &tlon );
        GetCanvasPointPix( tlat, tlon, r );
        int l1 = ( y_origin - r.y ) / count;

        for( int i = 0; i < count; i++ ) {
            int y = l1 * i;
            if( i & 1 ) dc.SetPen( pen1 );
            else
                dc.SetPen( pen2 );
            
            dc.DrawLine( x_origin, y_origin - y, x_origin, y_origin - ( y + l1 ) );
        }
    }
    else {
        double blat, blon, tlat, tlon;

        int x_origin = 5.0 * GetPixPerMM();
        //         ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        //         if (style->chartStatusWindowTransparent)
        //             chartbar_height = 0;
        int y_origin = m_canvas_height  - 25;

        GetCanvasPixPoint( x_origin, y_origin, blat, blon );
        GetCanvasPixPoint( x_origin + m_canvas_width, y_origin, tlat, tlon );

        double d;
        ll_gc_ll_reverse(blat, blon, tlat, tlon, 0, &d);
        d /= 2;

        int unit = g_iDistanceFormat;
        if( d < .5 && ( unit == DISTANCE_KM || unit == DISTANCE_MI || unit == DISTANCE_NMI ) )
            unit = ( unit == DISTANCE_MI ) ? DISTANCE_FT : DISTANCE_M;
        
        // nice number
        float dist = zchxFuncUtil::toUsrDistance( d, unit );
        float logdist = log(dist) / log(10.F);
        float places = floor(logdist), rem = logdist - places;
        dist = pow(10, places);

        if(rem < .2)
            dist /= 5;
        else if(rem < .5)
            dist /= 2;

        QString s = QString("").sprintf("%g ", dist) + zchxFuncUtil::getUsrDistanceUnit( unit );
        QColor black = GetGlobalColor( "UBLCK"  );
        QPen pen1 = QPen( black , 1, Qt::SolidLine );
        double rotation = -VPoint.rotation();

        ll_gc_ll( blat, blon, rotation * 180 / PI + 90,
                  zchxFuncUtil::fromUsrDistance(dist, unit), &tlat, &tlon );
        zchxPoint r;
        GetCanvasPointPix( tlat, tlon, r );
        int l1 = r.x - x_origin;
        
        m_scaleBarRect = QRect(x_origin, y_origin- 12, l1, 12);        // Store this for later reference

        dc.SetPen(pen1);
        
        dc.DrawLine( x_origin, y_origin, x_origin, y_origin - 12);
        dc.DrawLine( x_origin, y_origin, x_origin + l1, y_origin);
        dc.DrawLine( x_origin + l1, y_origin, x_origin + l1, y_origin - 12);

        dc.SetFont( m_pgridFont );
        dc.SetTextForeground( black );
        int w, h;
        dc.GetTextExtent(s, &w, &h);
        dc.drawText( s, x_origin + l1/2 - w/2, y_origin - h - 1 );
    }
}

void ChartCanvas::JaggyCircle( ocpnDC &dc, QPen pen, int x, int y, int radius )
{
    //    Constants?
    double da_min = 2.;
    double da_max = 6.;
    double ra_min = 0.;
    double ra_max = 40.;

    QPen pen_save = dc.GetPen();

    QDateTime now = QDateTime::currentDateTime();

    dc.SetPen( pen );

    int x0, y0, x1, y1;

    x0 = x1 = x + radius;                    // Start point
    y0 = y1 = y;
    double angle = 0.;
    int i = 0;

    while( angle < 360. ) {
        double da = da_min + ( ( (double) rand() / RAND_MAX ) * ( da_max - da_min ) );
        angle += da;

        if( angle > 360. ) angle = 360.;

        double ra = ra_min + ( ( (double) rand() / RAND_MAX ) * ( ra_max - ra_min ) );

        double r;
        if( i & 1 ) r = radius + ra;
        else
            r = radius - ra;

        x1 = (int) ( x + cos( angle * PI / 180. ) * r );
        y1 = (int) ( y + sin( angle * PI / 180. ) * r );

        dc.DrawLine( x0, y0, x1, y1 );

        x0 = x1;
        y0 = y1;

        i++;

    }

    dc.DrawLine( x + radius, y, x1, y1 );             // closure

    dc.SetPen( pen_save );
}

static bool bAnchorSoundPlaying = false;

static void onSoundFinished( void* ptr )
{
    bAnchorSoundPlaying = false;
}




//void ChartCanvas::OnActivate( wxActivateEvent& event )
//{
//    ReloadVP();
//}

void ChartCanvas::resizeEvent(QResizeEvent * event )
{
    m_canvas_width = event->size().width();
    m_canvas_height = event->size().height();
    //    Get some canvas metrics

    //          Rescale to current value, in order to rebuild VPoint data structures
    //          for new canvas size
    SetVPScale( GetVPScale() );

    m_absolute_min_scale_ppm = m_canvas_width / ( 1.2 * WGS84_semimajor_axis_meters * PI ); // something like 180 degrees

    //  Inform the parent Frame that I am being resized...
    UpdateGPSCompassStatusBox(true);
    //    Set up the scroll margins
    xr_margin = m_canvas_width * 95 / 100;
    xl_margin = m_canvas_width * 5 / 100;
    yt_margin = m_canvas_height * 5 / 100;
    yb_margin = m_canvas_height * 95 / 100;

    m_pQuilt->SetQuiltParameters( m_canvas_scale_factor, m_canvas_width );

    //    Resize the current viewport

    VPoint.setPixWidth(m_canvas_width);
    VPoint.setPixHeight(m_canvas_height);

    // Resize the scratch BM
    delete pscratch_bm;
    pscratch_bm = new wxBitmap( VPoint.pixWidth(), VPoint.pixHeight(), -1 );

    // Resize the Route Calculation BM
//    m_dc_route.SelectObject( wxNullBitmap );
//    delete proute_bm;
//    proute_bm = new wxBitmap( VPoint.pixWidth(), VPoint.pixHeight(), -1 );
//    m_dc_route.SelectObject( *proute_bm );

    //  Resize the saved Bitmap
    m_cached_chart_bm.Create( VPoint.pixWidth(), VPoint.pixHeight(), -1 );

    //  Resize the working Bitmap
    m_working_bm.Create( VPoint.pixWidth(), VPoint.pixHeight(), -1 );

    //  Rescale again, to capture all the changes for new canvas size
    SetVPScale( GetVPScale() );
    if( m_glcc ) {
        m_glcc->resize(event->size() );
    }
    //  Invalidate the whole window
    ReloadVP();
}

void ChartCanvas::ProcessNewGUIScale()
{
//    m_muiBar->Hide();
//    delete m_muiBar;
//    m_muiBar = 0;
    
//    CreateMUIBar();
}


//void ChartCanvas::CreateMUIBar()
//{
//    if(g_useMUI && !m_muiBar){                          // rebuild if necessary
        
//        // We need to update the m_bENCGroup flag, at least for the initial creation of a MUIBar
//        if(ChartData)
//            m_bENCGroup = ChartData->IsENCInGroup( m_groupIndex );

//        m_muiBar = new MUIBar(this, wxHORIZONTAL, g_toolbar_scalefactor);
//        m_muiBar->SetColorScheme( m_cs );
//        m_muiBarHOSize = m_muiBar->GetSize();
//    }
    
    
//    if(m_muiBar){
//        SetMUIBarPosition();
//        UpdateFollowButtonState();
//        m_muiBar->SetCanvasENCAvailable( m_bENCGroup );
//        m_muiBar->Raise();
//    }
    
//}


//void ChartCanvas::SetMUIBarPosition()
//{
//    //  if MUIBar is active, size the bar
//    if(m_muiBar){
//        // We precalculate the piano width based on the canvas width
//        int pianoWidth = GetClientSize().x * (g_btouch ? 0.7f : 0.6f);
//        //        if(m_Piano)
//        //            pianoWidth = m_Piano->width();
        
//        if((m_muiBar->GetOrientation() == wxHORIZONTAL)){
//            if(m_muiBarHOSize.x > (GetClientSize().x - pianoWidth)){
//                delete m_muiBar;
//                m_muiBar = new MUIBar(this, wxVERTICAL, g_toolbar_scalefactor);
//            }
//        }
        
//        if((m_muiBar->GetOrientation() == wxVERTICAL)){
//            if(m_muiBarHOSize.x < (GetClientSize().x - pianoWidth)){
//                delete m_muiBar;
//                m_muiBar = new MUIBar(this, wxHORIZONTAL, g_toolbar_scalefactor);
//            }
//        }
        
//        m_muiBar->SetBestPosition();
//    }
//}

//void ChartCanvas::DestroyMuiBar()
//{
//    if(m_muiBar){
//        m_muiBar->Destroy();
//        m_muiBar = NULL;
//    }
//}

void ChartCanvas::PanTimerEvent( QTimerEvent& event )
{
//    wxMouseEvent ev( wxEVT_MOTION );
//    ev.m_x = mouse_x;
//    ev.m_y = mouse_y;
//    ev.m_leftDown = mouse_leftisdown;

//    wxEvtHandler *evthp = GetEventHandler();

//    ::wxPostEvent( evthp, ev );

}

void ChartCanvas::MovementTimerEvent( QTimerEvent& )
{
    DoTimedMovement();
}

void ChartCanvas::MovementStopTimerEvent( QTimerEvent& )
{
    StopMovement( );
}

bool ChartCanvas::CheckEdgePan( int x, int y, bool bdragging, int margin, int delta )
{
    if(m_disable_edge_pan)
        return false;
    
    bool bft = false;
    int pan_margin = m_canvas_width * margin / 100;
    int pan_timer_set = 200;
    double pan_delta = GetVP().pixWidth() * delta / 100;
    int pan_x = 0;
    int pan_y = 0;

    if( x > m_canvas_width - pan_margin ) {
        bft = true;
        pan_x = pan_delta;
    }

    else if( x < pan_margin ) {
        bft = true;
        pan_x = -pan_delta;
    }

    if( y < pan_margin ) {
        bft = true;
        pan_y = -pan_delta;
    }

    else if( y > m_canvas_height - pan_margin ) {
        bft = true;
        pan_y = pan_delta;
    }

    //    Of course, if dragging, and the mouse left button is not down, we must stop the event injection
//    if( bdragging ) {
//        if( !g_btouch )
//        {
//            wxMouseState state = ::wxGetMouseState();
//            if( !state.LeftDown() )
//                bft = false;
//        }
//    }
//    if( ( bft ) && !pPanTimer->IsRunning() ) {
//        PanCanvas( pan_x, pan_y );
//        pPanTimer->Start( pan_timer_set, QTimer_ONE_SHOT );
//        return true;
//    }

//    //    This mouse event must not be due to pan timer event injector
//    //    Mouse is out of the pan zone, so prevent any orphan event injection
//    if( ( !bft ) && pPanTimer->IsRunning() ) {
//        pPanTimer->Stop();
//    }

    return ( false );
}

// Look for waypoints at the current position.
// Used to determine what a mouse event should act on.

void ChartCanvas::MouseTimedEvent( QTimerEvent& event )
{
//    if( singleClickEventIsValid ) MouseEvent( singleClickEvent );
//    singleClickEventIsValid = false;
//    m_DoubleClickTimer->Stop();
}

bool leftIsDown;


bool ChartCanvas::MouseEventOverlayWindows(  QMouseEvent* event  )
{
//    if (!m_bChartDragging && !m_bDrawingRoute) {
//        if(m_Compass && m_Compass->IsShown() && m_Compass->GetRect().contains(event->pos())) {
//            if (m_Compass->MouseEvent( event )) {
//                cursor_region = CENTER;
//                if( !g_btouch )
//                    SetCanvasCursor( event );
//                return true;
//            }
//        }

//        if(MouseEventChartBar( event ))
//            return true;
//    }
    return false;
}


bool ChartCanvas::MouseEventChartBar( QMouseEvent* event )
{
//    if(!g_bShowChartBar)  return false;

//    if (! m_Piano->MouseEvent(event) )    return false;

//    cursor_region = CENTER;
//    if( !g_btouch )  SetCanvasCursor( event );
//    return true;
}

bool ChartCanvas::MouseEventSetup( QMouseEvent* event,  bool b_handle_dclick )
{
    return true;
#if 0
    int x, y;
    int mx, my;

    bool bret = false;
    
    event.GetPosition( &x, &y );
    
    m_MouseDragging = event.Dragging();
    //  Some systems produce null drag events, where the pointer position has not changed from the previous value.
    //  Detect this case, and abort further processing (FS#1748)
#ifdef __WXMSW__    
    if(event.Dragging()){
        if((x == mouse_x) && (y == mouse_y))
            return true;
    }
#endif    
    
    mouse_x = x;
    mouse_y = y;
    mouse_leftisdown = event.LeftDown();
    mx = x;
    my = y;
    GetCanvasPixPoint( x, y, m_cursor_lat, m_cursor_lon );

    //  Establish the event region
    cursor_region = CENTER;
    
    int chartbar_height = m_Piano->GetHeight();

    if( m_Compass && m_Compass->IsShown() &&
            m_Compass->GetRect().Contains(event.GetPosition())) {
        cursor_region = CENTER;
    } else if( x > xr_margin ) {
        cursor_region = MID_RIGHT;
    } else if( x < xl_margin ) {
        cursor_region = MID_LEFT;
    } else if( y > yb_margin - chartbar_height &&
               y < m_canvas_height - chartbar_height) {
        cursor_region = MID_TOP;
    } else if( y < yt_margin ) {
        cursor_region = MID_BOT;
    } else {
        cursor_region = CENTER;
    }
    
    
    if( !g_btouch )
        SetCanvasCursor( event );
    
    
    // Protect from leftUp's coming from event handlers in child
    // windows who return focus to the canvas.
    leftIsDown = event.LeftDown();

    
#ifndef __WXOSX__
    if (event.LeftDown()) {
        if ( g_bShowMenuBar == false && g_bTempShowMenuBar == true ) {
            // The menu bar is temporarily visible due to alt having been pressed.
            // Clicking will hide it, and do nothing else.
            g_bTempShowMenuBar = false;
            parent_frame->ApplyGlobalSettings(false);
            return(true);
        }
    }
#endif
    
    // Update modifiers here; some window managers never send the key event
    m_modkeys = 0;
    if(event.ControlDown())
        m_modkeys |= wxMOD_CONTROL;
    if(event.AltDown())
        m_modkeys |= wxMOD_ALT;

#ifdef __WXMSW__
    //TODO Test carefully in other platforms, remove ifdef....
    if( event.ButtonDown() && !HasCapture() ) CaptureMouse();
    if( event.ButtonUp() && HasCapture() ) ReleaseMouse();
#endif
    
    if(g_pi_manager)
        if(g_pi_manager->SendMouseEventToPlugins( event ))
            return(true);                     // PlugIn did something, and does not want the canvas to do anything else


    // Capture LeftUp's and time them, unless it already came from the timer.

    if( b_handle_dclick && event.LeftUp() && !singleClickEventIsValid ) {

        // Ignore the second LeftUp after the DClick.
        if( m_DoubleClickTimer->IsRunning() ) {
            m_DoubleClickTimer->Stop();
            return(true);
        }

        // Save the event for later running if there is no DClick.
        m_DoubleClickTimer->Start( 350, QTimer_ONE_SHOT );
        singleClickEvent = event;
        singleClickEventIsValid = true;
        return(true);
    }

    //  This logic is necessary on MSW to handle the case where
    //  a context (right-click) menu is dismissed without action
    //  by clicking on the chart surface.
    //  We need to avoid an unintentional pan by eating some clicks...
#ifdef __WXMSW__
    if( event.LeftDown() || event.LeftUp() || event.Dragging() ) {
        if( g_click_stop > 0 ) {
            g_click_stop--;
            return(true);
        }
    }
#endif



    //  Kick off the Rotation control timer
    if( m_bCourseUp ) {
        m_b_rot_hidef = false;
        pRotDefTimer->Start( 500, QTimer_ONE_SHOT );
    } else
        pRotDefTimer->Stop();


    //      Retrigger the route leg / AIS target popup timer
    if( !g_btouch )
    {
        if( (m_pRouteRolloverWin && m_pRouteRolloverWin->IsActive()) ||
                (m_pTrackRolloverWin && m_pTrackRolloverWin->IsActive()) ||
                (m_pAISRolloverWin && m_pAISRolloverWin->IsActive()) )
            m_RolloverPopupTimer.Start( 10, QTimer_ONE_SHOT );               // faster response while the rollover is turned on
        else
            m_RolloverPopupTimer.Start( m_rollover_popup_timer_msec, QTimer_ONE_SHOT );
    }

    //  Retrigger the cursor tracking timer
    pCurTrackTimer->Start( m_curtrack_timer_msec, QTimer_ONE_SHOT );

    //      Show cursor position on Status Bar, if present
    //      except for GTK, under which status bar updates are very slow
    //      due to Update() call.
    //      In this case, as a workaround, update the status window
    //      after an interval timer (pCurTrackTimer) pops, which will happen
    //      whenever the mouse has stopped moving for specified interval.
    //      See the method OnCursorTrackTimerEvent()
#if !defined(__WXGTK__) && !defined(__WXQT__)
    SetCursorStatus(m_cursor_lat, m_cursor_lon);
#endif

    //  Send the current cursor lat/lon to all PlugIns requesting it
    if( g_pi_manager ){
        //  Occasionally, MSW will produce nonsense events on right click....
        //  This results in an error in cursor geo position, so we skip this case
        if( (x >= 0) && (y >= 0) )
            g_pi_manager->SendCursorLatLonToAllPlugIns( m_cursor_lat, m_cursor_lon );
    }
    
    
    if(!g_btouch){
        if( ( m_bMeasure_Active && ( m_nMeasureState >= 2 ) ) || ( m_routeState > 1 ) )
        {
            zchxPoint p = ClientToScreen( zchxPoint( x, y ) );
            
            // Submerge the toolbar if necessary
            if( m_toolBar ) {
                wxRect rect = m_toolBar->GetScreenRect();
                rect.Inflate( 20 );
                if( rect.Contains( p.x, p.y ) )
                    m_toolBar->Submerge();
            }
        }
    }
    
    if(1/*!g_btouch*/ ){
        //    Route Creation Rubber Banding
        if( m_routeState >= 2 ) {
            r_rband.x = x;
            r_rband.y = y;
            m_bDrawingRoute = true;
            
            if(!g_btouch )
                CheckEdgePan( x, y, event.Dragging(), 5, 2 );
            Refresh( false );
        }
        
        
        //    Measure Tool Rubber Banding
        if( m_bMeasure_Active && ( m_nMeasureState >= 2 ) ) {
            r_rband.x = x;
            r_rband.y = y;
            m_bDrawingRoute = true;
            
            if(!g_btouch )
                CheckEdgePan( x, y, event.Dragging(), 5, 2 );
            Refresh( false );
        }
    }
    return bret;
#endif

}

void ChartCanvas::CallPopupMenu(int x, int y)
{
#if 0
    int mx, my;
    mx = x;
    my = y;

    last_drag.x = mx;
    last_drag.y = my;
    if( m_routeState ) {                    // creating route?
        InvokeCanvasMenu(x, y, SELTYPE_ROUTECREATE);
        return;
    }
    // General Right Click
    // Look for selectable objects
    double slat, slon;
    slat = m_cursor_lat;
    slon = m_cursor_lon;
    
#if defined(__WXMAC__) || defined(__OCPN__ANDROID__)
    wxScreenDC sdc;
    ocpnDC dc( sdc );
#else
    wxClientDC cdc( GetParent() );
    ocpnDC dc( cdc );
#endif
    
    SelectItem *pFindAIS;
    SelectItem *pFindRP;
    SelectItem *pFindRouteSeg;
    SelectItem *pFindTrackSeg;
    SelectItem *pFindCurrent = NULL;
    SelectItem *pFindTide = NULL;
    
    //    Deselect any current objects
    if( m_pSelectedRoute ) {
        m_pSelectedRoute->m_bRtIsSelected = false;        // Only one selection at a time
        m_pSelectedRoute->DeSelectRoute();
#ifdef ocpnUSE_GL
        if(g_bopengl && m_glcc){
            InvalidateGL();
            Update();
        }
        else
#endif
            m_pSelectedRoute->Draw( dc, this, GetVP().getBBox() );
    }
    
    if( m_pFoundRoutePoint ) {
        m_pFoundRoutePoint->m_bPtIsSelected = false;
        m_pFoundRoutePoint->Draw( dc, this );
        RefreshRect( m_pFoundRoutePoint->CurrentRect_in_DC );
    }

    /**in touch mode a route point could have been selected and draghandle icon shown so clear the selection*/
    if (g_btouch && m_pRoutePointEditTarget) {
        m_pRoutePointEditTarget->m_bRPIsBeingEdited = false;
        m_pRoutePointEditTarget->m_bPtIsSelected = false;
        m_pRoutePointEditTarget->EnableDragHandle(false);
    }
    
    //      Get all the selectable things at the cursor
    pFindAIS = pSelectAIS->FindSelection( this, slat, slon, SELTYPE_AISTARGET );
    pFindRP = pSelect->FindSelection( this, slat, slon, SELTYPE_ROUTEPOINT );
    pFindRouteSeg = pSelect->FindSelection( this, slat, slon, SELTYPE_ROUTESEGMENT );
    pFindTrackSeg = pSelect->FindSelection( this, slat, slon, SELTYPE_TRACKSEGMENT );
    
    if( m_bShowCurrent ) pFindCurrent = pSelectTC->FindSelection( this, slat, slon, SELTYPE_CURRENTPOINT );
    
    if( m_bShowTide )                                // look for tide stations
        pFindTide = pSelectTC->FindSelection( this, slat, slon, SELTYPE_TIDEPOINT );
    
    int seltype = 0;
    
    //    Try for AIS targets first
    if( pFindAIS ) {
        m_FoundAIS_MMSI = pFindAIS->GetUserData();
        
        //      Make sure the target data is available
        if( g_pAIS->Get_Target_Data_From_MMSI( m_FoundAIS_MMSI ) ) seltype |=
                SELTYPE_AISTARGET;
    }
    
    //    Now the various Route Parts
    
    m_pFoundRoutePoint = NULL;
    if( pFindRP ) {
        RoutePoint *pFirstVizPoint = NULL;
        RoutePoint *pFoundActiveRoutePoint = NULL;
        RoutePoint *pFoundVizRoutePoint = NULL;
        Route *pSelectedActiveRoute = NULL;
        Route *pSelectedVizRoute = NULL;
        
        //There is at least one routepoint, so get the whole list
        SelectableItemList SelList = pSelect->FindSelectionList( this, slat, slon, SELTYPE_ROUTEPOINT );
        wxSelectableItemListNode *node = SelList.GetFirst();
        while( node ) {
            SelectItem *pFindSel = node->GetData();
            
            RoutePoint *prp = (RoutePoint *) pFindSel->m_pData1;        //candidate
            
            //    Get an array of all routes using this point
            wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining( prp );
            
            // Use route array (if any) to determine actual visibility for this point
            bool brp_viz = false;
            if( proute_array ) {
                for( unsigned int ir = 0; ir < proute_array->count(); ir++ ) {
                    Route *pr = (Route *) proute_array->at( ir );
                    if( pr->IsVisible() ) {
                        brp_viz = true;
                        break;
                    }
                }
                if( !brp_viz  && prp->m_bKeepXRoute)    // is not visible as part of route, but still exists as a waypoint
                    brp_viz = prp->IsVisible();         //  so treat as isolated point

            } else
                brp_viz = prp->IsVisible();               // isolated point

            if( ( NULL == pFirstVizPoint ) && brp_viz ) pFirstVizPoint = prp;

            // Use route array to choose the appropriate route
            // Give preference to any active route, otherwise select the first visible route in the array for this point
            m_pSelectedRoute = NULL;
            if( proute_array ) {
                for( unsigned int ir = 0; ir < proute_array->count(); ir++ ) {
                    Route *pr = (Route *) proute_array->at( ir );
                    if( pr->m_bRtIsActive ) {
                        pSelectedActiveRoute = pr;
                        pFoundActiveRoutePoint = prp;
                        break;
                    }
                }
                
                if( NULL == pSelectedVizRoute ) {
                    for( unsigned int ir = 0; ir < proute_array->count(); ir++ ) {
                        Route *pr = (Route *) proute_array->at( ir );
                        if( pr->IsVisible() ) {
                            pSelectedVizRoute = pr;
                            pFoundVizRoutePoint = prp;
                            break;
                        }
                    }
                }
                
                delete proute_array;
            }
            
            node = node->GetNext();
        }
        
        //      Now choose the "best" selections
        if( pFoundActiveRoutePoint ) {
            m_pFoundRoutePoint = pFoundActiveRoutePoint;
            m_pSelectedRoute = pSelectedActiveRoute;
        } else if( pFoundVizRoutePoint ) {
            m_pFoundRoutePoint = pFoundVizRoutePoint;
            m_pSelectedRoute = pSelectedVizRoute;
        } else
            // default is first visible point in list
            m_pFoundRoutePoint = pFirstVizPoint;
        
        if( m_pSelectedRoute ) {
            if( m_pSelectedRoute->IsVisible() ) seltype |= SELTYPE_ROUTEPOINT;
        } else if( m_pFoundRoutePoint ) seltype |= SELTYPE_MARKPOINT;
        
        //      Highlite the selected point, to verify the proper right click selection
        if( m_pFoundRoutePoint) {
            m_pFoundRoutePoint->m_bPtIsSelected = true;
            wxRect wp_rect;
            m_pFoundRoutePoint->CalculateDCRect( m_dc_route, this, &wp_rect );
            RefreshRect( wp_rect, true );
        }
        
    }
    
    // Note here that we use SELTYPE_ROUTESEGMENT to select tracks as well as routes
    // But call the popup handler with identifier appropriate to the type
    if( pFindRouteSeg )                  // there is at least one select item
    {
        SelectableItemList SelList = pSelect->FindSelectionList( this, slat, slon, SELTYPE_ROUTESEGMENT );
        
        if( NULL == m_pSelectedRoute )  // the case where a segment only is selected
        {
            //  Choose the first visible route containing segment in the list
            wxSelectableItemListNode *node = SelList.GetFirst();
            while( node ) {
                SelectItem *pFindSel = node->GetData();
                
                Route *pr = (Route *) pFindSel->m_pData3;
                if( pr->IsVisible() ) {
                    m_pSelectedRoute = pr;
                    break;
                }
                node = node->GetNext();
            }
        }
        
        if( m_pSelectedRoute ) {
            if( NULL == m_pFoundRoutePoint )
                m_pFoundRoutePoint =   (RoutePoint *) pFindRouteSeg->m_pData1;
            
            m_pSelectedRoute->m_bRtIsSelected = !(seltype & SELTYPE_ROUTEPOINT);
            if( m_pSelectedRoute->m_bRtIsSelected ){
#ifdef ocpnUSE_GL
                if(g_bopengl && m_glcc){
                    InvalidateGL();
                    Update();
                }
                else
#endif
                    m_pSelectedRoute->Draw( dc, this, GetVP().getBBox() );
            }
            
            seltype |= SELTYPE_ROUTESEGMENT;
        }
        
    }
    
    if( pFindTrackSeg ) {
        m_pSelectedTrack = NULL;
        SelectableItemList SelList = pSelect->FindSelectionList( this, slat, slon, SELTYPE_TRACKSEGMENT );
        
        //  Choose the first visible track containing segment in the list
        wxSelectableItemListNode *node = SelList.GetFirst();
        while( node ) {
            SelectItem *pFindSel = node->GetData();
            
            Track *pt = (Track *) pFindSel->m_pData3;
            if( pt->IsVisible() ) {
                m_pSelectedTrack = pt;
                break;
            }
            node = node->GetNext();
        }
        
        if( m_pSelectedTrack ) seltype |= SELTYPE_TRACKSEGMENT;
    }
    
    bool bseltc = false;
    //                      if(0 == seltype)
    {
        if( pFindCurrent ) {
            // There may be multiple current entries at the same point.
            // For example, there often is a current substation (with directions specified)
            // co-located with its master.  We want to select the substation, so that
            // the direction will be properly indicated on the graphic.
            // So, we search the select list looking for IDX_type == 'c' (i.e substation)
            IDX_entry *pIDX_best_candidate;
            
            SelectItem *pFind = NULL;
            SelectableItemList SelList = pSelectTC->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_CURRENTPOINT );
            
            //      Default is first entry
            wxSelectableItemListNode *node = SelList.GetFirst();
            pFind = node->GetData();
            pIDX_best_candidate = (IDX_entry *) ( pFind->m_pData1 );
            
            if( SelList.count() > 1 ) {
                node = node->GetNext();
                while( node ) {
                    pFind = node->GetData();
                    IDX_entry *pIDX_candidate = (IDX_entry *) ( pFind->m_pData1 );
                    if( pIDX_candidate->IDX_type == 'c' ) {
                        pIDX_best_candidate = pIDX_candidate;
                        break;
                    }
                    
                    node = node->GetNext();
                }       // while (node)
            } else {
                wxSelectableItemListNode *node = SelList.GetFirst();
                pFind = node->GetData();
                pIDX_best_candidate = (IDX_entry *) ( pFind->m_pData1 );
            }
            
            m_pIDXCandidate = pIDX_best_candidate;
            
            if( 0 == seltype ) {
                DrawTCWindow( x, y, (void *) pIDX_best_candidate );
                Refresh( false );
                bseltc = true;
            } else
                seltype |= SELTYPE_CURRENTPOINT;
        }
        
        else if( pFindTide ) {
            m_pIDXCandidate = (IDX_entry *) pFindTide->m_pData1;
            
            if( 0 == seltype ) {
                DrawTCWindow( x, y, (void *) pFindTide->m_pData1 );
                Refresh( false );
                bseltc = true;
            } else
                seltype |= SELTYPE_TIDEPOINT;
        }
    }
    
    if( 0 == seltype )
        seltype |= SELTYPE_UNKNOWN;
    
    if( !bseltc ){
        InvokeCanvasMenu(x, y, seltype);
        
        // Clean up if not deleted in InvokeCanvasMenu
        if( m_pSelectedRoute && g_pRouteMan->IsRouteValid(m_pSelectedRoute) ) {
            m_pSelectedRoute->m_bRtIsSelected = false;
        }
        
        m_pSelectedRoute = NULL;
        
        if( m_pFoundRoutePoint ) {
            if (pSelect->IsSelectableRoutePointValid(m_pFoundRoutePoint))
                m_pFoundRoutePoint->m_bPtIsSelected = false;
        }
        m_pFoundRoutePoint = NULL;
        
        Refresh( true );
        
    }
    
    // Seth: Is this refresh needed?
    Refresh( false );            // needed for MSW, not GTK  Why??
#endif
}

bool ChartCanvas::MouseEventProcessObjects( QMouseEvent* event )
{
    return true;
    #if 0
    // For now just bail out completely if the point clicked is not on the chart
    if(std::isnan(m_cursor_lat))
        return false;

    //          Mouse Clicks
    bool ret = false;        // return true if processed
    
    int x, y, mx, my;
    event.GetPosition( &x, &y );
    mx = x;
    my = y;
    
    //    Calculate meaningful SelectRadius
    float SelectRadius;
    SelectRadius = g_Platform->GetSelectRadiusPix() / ( m_true_scale_ppm * 1852 * 60 );  // Degrees, approximately

    ///
    // We start with Double Click processing. The first left click just starts a timer and
    // is remembered, then we actually do something if there is a LeftDClick.
    // If there is, the two single clicks are ignored.
    
    if( event.LeftDClick() && ( cursor_region == CENTER ) ) {
        
        m_DoubleClickTimer->Start();
        singleClickEventIsValid = false;
        
        double zlat, zlon;
        GetCanvasPixPoint( x, y, zlat, zlon );
        
        if(m_bShowAIS){
            SelectItem *pFindAIS;
            pFindAIS = pSelectAIS->FindSelection( this, zlat, zlon, SELTYPE_AISTARGET );

            if( pFindAIS ) {
                m_FoundAIS_MMSI = pFindAIS->GetUserData();
                if( g_pAIS->Get_Target_Data_From_MMSI( m_FoundAIS_MMSI ) ) {
                    wxWindow *pwin = wxDynamicCast(this, wxWindow);
                    ShowAISTargetQueryDialog( pwin, m_FoundAIS_MMSI );
                }
                return true;
            }
        }
        
        SelectableItemList rpSelList = pSelect->FindSelectionList( this, zlat, zlon, SELTYPE_ROUTEPOINT );
        wxSelectableItemListNode *node = rpSelList.GetFirst();
        bool b_onRPtarget = false;
        while( node ) {
            SelectItem *pFind = node->GetData();
            RoutePoint *frp = (RoutePoint *) pFind->m_pData1;
            if(m_pRoutePointEditTarget && (frp == m_pRoutePointEditTarget) ){
                b_onRPtarget = true;
                break;
            }
            node = node->GetNext();
        }
        
        //      Double tap with selected RoutePoint or Mark
        bool bt1 = m_bMarkEditing;
        RoutePoint *pp = m_pRoutePointEditTarget;
        

        else{
            node = rpSelList.GetFirst();
            if( node ) {
                SelectItem *pFind = node->GetData();
                RoutePoint *frp = (RoutePoint *) pFind->m_pData1;
                if( frp ){
                    wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining( frp );

                    // Use route array (if any) to determine actual visibility for this point
                    bool brp_viz = false;
                    if( proute_array ){
                        for( unsigned int ir = 0; ir < proute_array->count(); ir++ )
                        {
                            Route *pr = (Route *) proute_array->at( ir );
                            if( pr->IsVisible() )
                            {
                                brp_viz = true;
                                break;
                            }
                        }
                        if( !brp_viz && frp->m_bKeepXRoute ) // is not visible as part of route, but still exists as a waypoint
                            brp_viz = frp->IsVisible(); // so treat as isolated point
                    } else
                        brp_viz = frp->IsVisible(); // isolated point

                    if( brp_viz ){
                        ShowMarkPropertiesDialog( frp );
                        return true;
                    }
                }
            }
        }
        
        
        
        SelectItem* cursorItem;
        cursorItem = pSelect->FindSelection( this, zlat, zlon, SELTYPE_ROUTESEGMENT );
        
        if( cursorItem ) {
            Route *pr = (Route *) cursorItem->m_pData3;
            if( pr->IsVisible() ) {
                ShowRoutePropertiesDialog( _("Route Properties"), pr );
                return true;
            }
        }
        
        cursorItem = pSelect->FindSelection( this, zlat, zlon, SELTYPE_TRACKSEGMENT );
        
        if( cursorItem ) {
            Track *pt = (Track *) cursorItem->m_pData3;
            if( pt->IsVisible() ) {
                ShowTrackPropertiesDialog( pt );
                return true;
            }
        }
        
        // Found no object to act on, so show chart info.
        
        ShowObjectQueryWindow( x, y, zlat, zlon );
        return true;
    }
    
    
    
    ///
    if( event.LeftDown() ) {
        //  This really should not be needed, but....
        //  on Windows, when using wxAUIManager, sometimes the focus is lost
        //  when clicking into another pane, e.g.the AIS target list, and then back to this pane.
        //  Oddly, some mouse events are not lost, however.  Like this one....
        SetFocus();
        
        last_drag.x = mx;
        last_drag.y = my;
        leftIsDown = true;
        
        if(!g_btouch){
            if( m_routeState )                  // creating route?
            {
                double rlat, rlon;
                
                SetCursor( *pCursorPencil );
                rlat = m_cursor_lat;
                rlon = m_cursor_lon;
                
                m_bRouteEditing = true;
                
                if( m_routeState == 1 ) {
                    m_pMouseRoute = new Route();
                    pRouteList->Append( m_pMouseRoute );
                    r_rband.x = x;
                    r_rband.y = y;
                }
                
                //    Check to see if there is a nearby point which may be reused
                RoutePoint *pMousePoint = NULL;
                
                //    Calculate meaningful SelectRadius
                double nearby_radius_meters = g_Platform->GetSelectRadiusPix() / m_true_scale_ppm;
                
                RoutePoint *pNearbyPoint = pWayPointMan->GetNearbyWaypoint( rlat, rlon,
                                                                            nearby_radius_meters );
                if( pNearbyPoint && ( pNearbyPoint != m_prev_pMousePoint )
                        && !pNearbyPoint->m_bIsInLayer && pNearbyPoint->IsVisible() )
                {
                    wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining( pNearbyPoint );

                    // Use route array (if any) to determine actual visibility for this point
                    bool brp_viz = false;
                    if( proute_array ){
                        for( unsigned int ir = 0; ir < proute_array->count(); ir++ ){
                            Route *pr = (Route *) proute_array->at( ir );
                            if( pr->IsVisible() ) {
                                brp_viz = true;
                                break;
                            }
                        }

                        if( !brp_viz && pNearbyPoint->m_bKeepXRoute ) // is not visible as part of route, but still exists as a waypoint
                            brp_viz = pNearbyPoint->IsVisible(); // so treat as isolated point
                    }
                    else
                        brp_viz = pNearbyPoint->IsVisible(); // isolated point
                    
                    
                    if( brp_viz ){
                        m_FinishRouteOnKillFocus = false;              // Avoid route finish on focus change for message dialog
                        int dlg_return = OCPNMessageBox( this, _("Use nearby waypoint?"),
                                                         _("OpenCPN Route Create"),
                                                         (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );
                        m_FinishRouteOnKillFocus = true;

                        if( dlg_return == wxID_YES ) {
                            pMousePoint = pNearbyPoint;

                            // Using existing waypoint, so nothing to delete for undo.
                            if( m_routeState > 1 )
                                undo->BeforeUndoableAction( Undo_AppendWaypoint, pMousePoint, Undo_HasParent, NULL );

                            // check all other routes to see if this point appears in any other route
                            // If it appears in NO other route, then it should e considered an isolated mark
                            if( !g_pRouteMan->FindRouteContainingWaypoint( pMousePoint ) )
                                pMousePoint->m_bKeepXRoute = true;
                        }
                    }
                }
                
                if( NULL == pMousePoint ) {                 // need a new point
                    pMousePoint = new RoutePoint( rlat, rlon, g_default_routepoint_icon, _T(""), wxEmptyString );
                    pMousePoint->SetNameShown( false );
                    
                    pConfig->AddNewWayPoint( pMousePoint, -1 );    // use auto next num
                    pSelect->AddSelectableRoutePoint( rlat, rlon, pMousePoint );
                    
                    if( m_routeState > 1 )
                        undo->BeforeUndoableAction( Undo_AppendWaypoint, pMousePoint, Undo_IsOrphanded, NULL );
                }
                
                if(m_pMouseRoute){
                    if( m_routeState == 1 ) {
                        // First point in the route.
                        m_pMouseRoute->AddPoint( pMousePoint );
                    } else {
                        if( m_pMouseRoute->m_NextLegGreatCircle ) {
                            double rhumbBearing, rhumbDist, gcBearing, gcDist;
                            DistanceBearingMercator( rlat, rlon, m_prev_rlat, m_prev_rlon, &rhumbBearing, &rhumbDist );
                            Geodesic::GreatCircleDistBear( m_prev_rlon, m_prev_rlat, rlon, rlat, &gcDist, &gcBearing, NULL );
                            double gcDistNM = gcDist / 1852.0;
                            
                            // Empirically found expression to get reasonable route segments.
                            int segmentCount = (3.0 + (rhumbDist - gcDistNM)) / pow(rhumbDist-gcDistNM-1, 0.5 );
                            
                            QString msg;
                            msg << _("For this leg the Great Circle route is ")
                                << FormatDistanceAdaptive( rhumbDist - gcDistNM ) << _(" shorter than rhumbline.\n\n")
                                << _("Would you like include the Great Circle routing points for this leg?");
                            
                            m_FinishRouteOnKillFocus = false;
                            m_disable_edge_pan = true;  // This helps on OS X if MessageBox does not fully capture mouse
                            
                            int answer = OCPNMessageBox( this, msg, _("OpenCPN Route Create"), wxYES_NO | wxNO_DEFAULT );
                            
                            m_disable_edge_pan = false;
                            m_FinishRouteOnKillFocus = true;
                            
                            if( answer == wxID_YES ) {
                                RoutePoint* gcPoint;
                                RoutePoint* prevGcPoint = m_prev_pMousePoint;
                                wxRealPoint gcCoord;
                                
                                for( int i = 1; i <= segmentCount; i++ ) {
                                    double fraction = (double) i * ( 1.0 / (double) segmentCount );
                                    Geodesic::GreatCircleTravel( m_prev_rlon, m_prev_rlat, gcDist * fraction,
                                                                 gcBearing, &gcCoord.x, &gcCoord.y, NULL );
                                    
                                    if( i < segmentCount ) {
                                        gcPoint = new RoutePoint( gcCoord.y, gcCoord.x, _T("xmblue"), _T(""),
                                                                  wxEmptyString );
                                        gcPoint->SetNameShown( false );
                                        pConfig->AddNewWayPoint( gcPoint, -1 );
                                        pSelect->AddSelectableRoutePoint( gcCoord.y, gcCoord.x, gcPoint );
                                    } else {
                                        gcPoint = pMousePoint; // Last point, previously exsisting!
                                    }
                                    
                                    m_pMouseRoute->AddPoint( gcPoint );
                                    pSelect->AddSelectableRouteSegment( prevGcPoint->m_lat, prevGcPoint->m_lon,
                                                                        gcPoint->m_lat, gcPoint->m_lon, prevGcPoint, gcPoint, m_pMouseRoute );
                                    prevGcPoint = gcPoint;
                                }
                                
                                undo->CancelUndoableAction( true );
                                
                            } else {
                                m_pMouseRoute->AddPoint( pMousePoint );
                                pSelect->AddSelectableRouteSegment( m_prev_rlat, m_prev_rlon,
                                                                    rlat, rlon, m_prev_pMousePoint, pMousePoint, m_pMouseRoute );
                                undo->AfterUndoableAction( m_pMouseRoute );
                            }
                        } else {
                            // Ordinary rhumblinesegment.
                            m_pMouseRoute->AddPoint( pMousePoint );
                            pSelect->AddSelectableRouteSegment( m_prev_rlat, m_prev_rlon,
                                                                rlat, rlon, m_prev_pMousePoint, pMousePoint, m_pMouseRoute );
                            undo->AfterUndoableAction( m_pMouseRoute );
                        }
                    }
                }
                
                m_prev_rlat = rlat;
                m_prev_rlon = rlon;
                m_prev_pMousePoint = pMousePoint;
                if(m_pMouseRoute)
                    m_pMouseRoute->m_lastMousePointIndex = m_pMouseRoute->GetnPoints();
                
                m_routeState++;
                gFrame->RefreshAllCanvas();
                ret = true;
            }
            
            else if( m_bMeasure_Active && m_nMeasureState )   // measure tool?
            {
                double rlat, rlon;
                
                SetCursor( *pCursorPencil );
                rlat = m_cursor_lat;
                rlon = m_cursor_lon;
                
                if( m_nMeasureState == 1 ) {
                    m_pMeasureRoute = new Route();
                    pRouteList->Append( m_pMeasureRoute );
                    r_rband.x = x;
                    r_rband.y = y;
                }
                
                RoutePoint *pMousePoint = new RoutePoint( m_cursor_lat, m_cursor_lon,
                                                          QString( _T ( "circle" ) ), wxEmptyString, wxEmptyString );
                pMousePoint->m_bShowName = false;
                
                m_pMeasureRoute->AddPoint( pMousePoint );
                
                m_prev_rlat = m_cursor_lat;
                m_prev_rlon = m_cursor_lon;
                m_prev_pMousePoint = pMousePoint;
                m_pMeasureRoute->m_lastMousePointIndex = m_pMeasureRoute->GetnPoints();
                
                m_nMeasureState++;
                gFrame->RefreshAllCanvas();
                ret = true;
            }
            
            else {
                FindRoutePointsAtCursor( SelectRadius, true );    // Not creating Route
            }
        }  // !g_btouch
        else {                  // g_btouch

            if(( m_bMeasure_Active && m_nMeasureState ) || ( m_routeState )){

                // if near screen edge, pan with injection
                //                if( CheckEdgePan( x, y, true, 5, 10 ) ) {
                //                    return;
                //                }

            }
        }
        
        if(ret)
            return true;
    }
    
    if( event.Dragging() ) {
        
        //in touch screen mode ensure the finger/cursor is on the selected point's radius to allow dragging
        if( g_btouch ) {
            if( m_pRoutePointEditTarget && !m_bIsInRadius ) {
                SelectItem *pFind = NULL;
                SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_ROUTEPOINT );
                wxSelectableItemListNode *node = SelList.GetFirst();
                while( node ) {
                    pFind = node->GetData();
                    RoutePoint *frp = (RoutePoint *) pFind->m_pData1;
                    if( m_pRoutePointEditTarget == frp )
                        m_bIsInRadius = true;
                    node = node->GetNext();
                }
            }
            
            // Check for use of dragHandle
            if(m_pRoutePointEditTarget && m_pRoutePointEditTarget->IsDragHandleEnabled()){
                SelectItem *pFind = NULL;
                SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_DRAGHANDLE );
                wxSelectableItemListNode *node = SelList.GetFirst();
                while( node ) {
                    pFind = node->GetData();
                    RoutePoint *frp = (RoutePoint *) pFind->m_pData1;
                    if( m_pRoutePointEditTarget == frp ){
                        m_bIsInRadius = true;
                        break;
                    }
                    node = node->GetNext();
                }
                
                if(!m_dragoffsetSet){
                    m_pRoutePointEditTarget->PresetDragOffset(this, mouse_x, mouse_y);
                    m_dragoffsetSet = true;
                }
            }
            
        }
        
        
        if( m_bRouteEditing && m_pRoutePointEditTarget ) {
            
            bool DraggingAllowed = g_btouch ? m_bIsInRadius : true;
            
            if( NULL == g_pMarkInfoDialog ) {
                if( g_bWayPointPreventDragging ) DraggingAllowed = false;
            } else if( !g_pMarkInfoDialog->IsShown() && g_bWayPointPreventDragging )
                DraggingAllowed = false;
            
            if( m_pRoutePointEditTarget && ( m_pRoutePointEditTarget->GetIconName() == _T("mob") ) )
                DraggingAllowed = false;
            
            if( m_pRoutePointEditTarget->m_bIsInLayer )
                DraggingAllowed = false;

            if( DraggingAllowed ) {

                if( !undo->InUndoableAction() ) {
                    undo->BeforeUndoableAction( Undo_MoveWaypoint, m_pRoutePointEditTarget,
                                                Undo_NeedsCopy, m_pFoundPoint );
                }

                // Get the update rectangle for the union of the un-edited routes
                wxRect pre_rect;

                if( !g_bopengl && m_pEditRouteArray ) {
                    for( unsigned int ir = 0; ir < m_pEditRouteArray->count(); ir++ ) {
                        Route *pr = (Route *) m_pEditRouteArray->at( ir );
                        //      Need to validate route pointer
                        //      Route may be gone due to drgging close to ownship with
                        //      "Delete On Arrival" state set, as in the case of
                        //      navigating to an isolated waypoint on a temporary route
                        if( g_pRouteMan->IsRouteValid(pr) ) {
                            wxRect route_rect;
                            pr->CalculateDCRect( m_dc_route, this, &route_rect );
                            pre_rect.Union( route_rect );
                        }
                    }
                }

                double new_cursor_lat = m_cursor_lat;
                double new_cursor_lon = m_cursor_lon;

                if( CheckEdgePan( x, y, true, 5, 2 ) )
                    GetCanvasPixPoint( x, y, new_cursor_lat, new_cursor_lon );

                // update the point itself
                if( g_btouch ) {
                    //m_pRoutePointEditTarget->SetPointFromDraghandlePoint(VPoint, new_cursor_lat, new_cursor_lon);
                    m_pRoutePointEditTarget->SetPointFromDraghandlePoint(this, mouse_x, mouse_y);
                    // update the Drag Handle entry in the pSelect list
                    pSelect->ModifySelectablePoint( new_cursor_lat, new_cursor_lon, m_pRoutePointEditTarget, SELTYPE_DRAGHANDLE );
                    m_pFoundPoint->m_slat = m_pRoutePointEditTarget->m_lat;             // update the SelectList entry
                    m_pFoundPoint->m_slon = m_pRoutePointEditTarget->m_lon;
                }
                else{
                    m_pRoutePointEditTarget->m_lat = new_cursor_lat;    // update the RoutePoint entry
                    m_pRoutePointEditTarget->m_lon = new_cursor_lon;
                    m_pFoundPoint->m_slat = new_cursor_lat;             // update the SelectList entry
                    m_pFoundPoint->m_slon = new_cursor_lon;
                }


                //    Update the MarkProperties Dialog, if currently shown
                if( ( NULL != g_pMarkInfoDialog ) && ( g_pMarkInfoDialog->IsShown() ) ) {
                    if( m_pRoutePointEditTarget == g_pMarkInfoDialog->GetRoutePoint() ) g_pMarkInfoDialog->UpdateProperties( true );
                }

                if(g_bopengl) {
                    //InvalidateGL();
                    Refresh( false );
                } else {
                    // Get the update rectangle for the edited route
                    wxRect post_rect;

                    if( m_pEditRouteArray ) {
                        for( unsigned int ir = 0; ir < m_pEditRouteArray->count(); ir++ ) {
                            Route *pr = (Route *) m_pEditRouteArray->at( ir );
                            if( g_pRouteMan->IsRouteValid(pr) ) {
                                wxRect route_rect;
                                pr->CalculateDCRect( m_dc_route, this, &route_rect );
                                post_rect.Union( route_rect );
                            }
                        }
                    }

                    //    Invalidate the union region
                    pre_rect.Union( post_rect );
                    RefreshRect( pre_rect, false );
                }
                gFrame->RefreshCanvasOther( this );
                m_bRoutePoinDragging = true;
            }
            ret = true;
        }     // if Route Editing
        
        else if( m_bMarkEditing && m_pRoutePointEditTarget ) {
            
            bool DraggingAllowed = g_btouch ? m_bIsInRadius : true;
            
            if( NULL == g_pMarkInfoDialog ) {
                if( g_bWayPointPreventDragging )
                    DraggingAllowed = false;
            } else if( !g_pMarkInfoDialog->IsShown() && g_bWayPointPreventDragging )
                DraggingAllowed = false;
            
            if( m_pRoutePointEditTarget
                    && ( m_pRoutePointEditTarget->GetIconName() == _T("mob") ) )
                DraggingAllowed = false;
            
            if( m_pRoutePointEditTarget->m_bIsInLayer )
                DraggingAllowed = false;

            if( DraggingAllowed ) {
                if( !undo->InUndoableAction() ) {
                    undo->BeforeUndoableAction( Undo_MoveWaypoint, m_pRoutePointEditTarget,
                                                Undo_NeedsCopy, m_pFoundPoint );
                }

                //      The mark may be an anchorwatch
                double lpp1 = 0.;
                double lpp2 = 0.;
                double lppmax;

                if( pAnchorWatchPoint1 == m_pRoutePointEditTarget ) {
                    lpp1 = fabs( GetAnchorWatchRadiusPixels( pAnchorWatchPoint1 ) );

                }
                if( pAnchorWatchPoint2 == m_pRoutePointEditTarget ) {
                    lpp2 = fabs( GetAnchorWatchRadiusPixels( pAnchorWatchPoint2 ) );
                }
                lppmax = fmax(lpp1 + 10, lpp2 + 10);         // allow for cruft

                // Get the update rectangle for the un-edited mark
                wxRect pre_rect;
                if(!g_bopengl) {
                    m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &pre_rect );
                    if( ( lppmax > pre_rect.width / 2 ) || ( lppmax > pre_rect.height / 2 ) )
                        pre_rect.Inflate( (int) ( lppmax - ( pre_rect.width / 2 ) ), (int) ( lppmax - ( pre_rect.height / 2 ) ) );
                }

                // update the point itself
                if( g_btouch ) {
                    //                            m_pRoutePointEditTarget->SetPointFromDraghandlePoint(VPoint, m_cursor_lat, m_cursor_lon);
                    m_pRoutePointEditTarget->SetPointFromDraghandlePoint(this, mouse_x, mouse_y);
                    // update the Drag Handle entry in the pSelect list
                    pSelect->ModifySelectablePoint( m_cursor_lat, m_cursor_lon, m_pRoutePointEditTarget, SELTYPE_DRAGHANDLE );
                    m_pFoundPoint->m_slat = m_pRoutePointEditTarget->m_lat;             // update the SelectList entry
                    m_pFoundPoint->m_slon = m_pRoutePointEditTarget->m_lon;
                }
                else{
                    m_pRoutePointEditTarget->m_lat = m_cursor_lat;    // update the RoutePoint entry
                    m_pRoutePointEditTarget->m_lon = m_cursor_lon;
                    m_pFoundPoint->m_slat = m_cursor_lat;             // update the SelectList entry
                    m_pFoundPoint->m_slon = m_cursor_lon;
                }



                //    Update the MarkProperties Dialog, if currently shown
                if( ( NULL != g_pMarkInfoDialog ) && ( g_pMarkInfoDialog->IsShown() ) ) {
                    if( m_pRoutePointEditTarget == g_pMarkInfoDialog->GetRoutePoint() )
                        g_pMarkInfoDialog->UpdateProperties( true );
                }

                //    Invalidate the union region
                if(g_bopengl) {
                    if(!g_btouch)
                        InvalidateGL();
                    Refresh( false );
                } else {
                    // Get the update rectangle for the edited mark
                    wxRect post_rect;
                    m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &post_rect );
                    if( ( lppmax > post_rect.width / 2 ) || ( lppmax > post_rect.height / 2 ) )
                        post_rect.Inflate((int) ( lppmax - ( post_rect.width / 2 ) ),
                                          (int) ( lppmax - ( post_rect.height / 2 ) ) );

                    //    Invalidate the union region
                    pre_rect.Union( post_rect );
                    RefreshRect( pre_rect, false );
                }
                gFrame->RefreshCanvasOther( this );
                m_bRoutePoinDragging = true;
            }
            ret = true;

        }
        
        if(ret)
            return true;
    }       //dragging
    
    if( event.LeftUp() ) {
        bool b_startedit_route = false;
        bool b_startedit_mark = false;
        m_dragoffsetSet = false;

        if(g_btouch) {
            m_bChartDragging = false;
            m_bIsInRadius = false;
            
            if( m_routeState )                  // creating route?
            {
                if(m_bedge_pan){
                    m_bedge_pan = false;
                    return false;
                }
                
                double rlat, rlon;
                
                rlat = m_cursor_lat;
                rlon = m_cursor_lon;
                
                if( m_pRoutePointEditTarget) {
                    m_pRoutePointEditTarget->m_bRPIsBeingEdited = false;
                    m_pRoutePointEditTarget->m_bPtIsSelected = false;
                    wxRect wp_rect;
                    m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &wp_rect );
                    RefreshRect( wp_rect, true );
                    m_pRoutePointEditTarget = NULL;
                }
                m_bRouteEditing = true;
                
                if( m_routeState == 1 ) {
                    m_pMouseRoute = new Route();
                    m_pMouseRoute->SetHiLite(50);
                    pRouteList->Append( m_pMouseRoute );
                    r_rband.x = x;
                    r_rband.y = y;
                }
                
                
                //    Check to see if there is a nearby point which may be reused
                RoutePoint *pMousePoint = NULL;
                
                //    Calculate meaningful SelectRadius
                double nearby_radius_meters = g_Platform->GetSelectRadiusPix() / m_true_scale_ppm;
                
                RoutePoint *pNearbyPoint = pWayPointMan->GetNearbyWaypoint( rlat, rlon,
                                                                            nearby_radius_meters );
                if( pNearbyPoint && ( pNearbyPoint != m_prev_pMousePoint )
                        && !pNearbyPoint->m_bIsInLayer && pNearbyPoint->IsVisible() )
                {
                    int dlg_return;
#ifndef __WXOSX__
                    m_FinishRouteOnKillFocus = false;  // Avoid route finish on focus change for message dialog
                    dlg_return = OCPNMessageBox( this, _("Use nearby waypoint?"),
                                                 _("OpenCPN Route Create"),
                                                 (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );
                    m_FinishRouteOnKillFocus = true;
#else
                    dlg_return = wxID_YES;
#endif
                    if( dlg_return == wxID_YES ) {
                        pMousePoint = pNearbyPoint;

                        // Using existing waypoint, so nothing to delete for undo.
                        if( m_routeState > 1 )
                            undo->BeforeUndoableAction( Undo_AppendWaypoint, pMousePoint, Undo_HasParent, NULL );

                        // check all other routes to see if this point appears in any other route
                        // If it appears in NO other route, then it should e considered an isolated mark
                        if( !g_pRouteMan->FindRouteContainingWaypoint( pMousePoint ) ) pMousePoint->m_bKeepXRoute =
                                true;
                    }
                }
                
                if( NULL == pMousePoint ) {                 // need a new point
                    pMousePoint = new RoutePoint( rlat, rlon, g_default_routepoint_icon, _T(""), wxEmptyString );
                    pMousePoint->SetNameShown( false );
                    
                    pConfig->AddNewWayPoint( pMousePoint, -1 );    // use auto next num
                    pSelect->AddSelectableRoutePoint( rlat, rlon, pMousePoint );
                    
                    if( m_routeState > 1 )
                        undo->BeforeUndoableAction( Undo_AppendWaypoint, pMousePoint, Undo_IsOrphanded, NULL );
                }
                
                if( m_routeState == 1 ) {
                    // First point in the route.
                    m_pMouseRoute->AddPoint( pMousePoint );
                } else {
                    if( m_pMouseRoute->m_NextLegGreatCircle ) {
                        double rhumbBearing, rhumbDist, gcBearing, gcDist;
                        DistanceBearingMercator( rlat, rlon, m_prev_rlat, m_prev_rlon, &rhumbBearing, &rhumbDist );
                        Geodesic::GreatCircleDistBear( m_prev_rlon, m_prev_rlat, rlon, rlat, &gcDist, &gcBearing, NULL );
                        double gcDistNM = gcDist / 1852.0;
                        
                        // Empirically found expression to get reasonable route segments.
                        int segmentCount = (3.0 + (rhumbDist - gcDistNM)) / pow(rhumbDist-gcDistNM-1, 0.5 );
                        
                        QString msg;
                        msg << _("For this leg the Great Circle route is ")
                            << FormatDistanceAdaptive( rhumbDist - gcDistNM ) << _(" shorter than rhumbline.\n\n")
                            << _("Would you like include the Great Circle routing points for this leg?");
                        
#ifndef __WXOSX__
                        m_FinishRouteOnKillFocus = false;
                        int answer = OCPNMessageBox( this, msg, _("OpenCPN Route Create"), wxYES_NO | wxNO_DEFAULT );
                        m_FinishRouteOnKillFocus = true;
#else
                        int answer = wxID_NO;
#endif
                        
                        if( answer == wxID_YES ) {
                            RoutePoint* gcPoint;
                            RoutePoint* prevGcPoint = m_prev_pMousePoint;
                            wxRealPoint gcCoord;
                            
                            for( int i = 1; i <= segmentCount; i++ ) {
                                double fraction = (double) i * ( 1.0 / (double) segmentCount );
                                Geodesic::GreatCircleTravel( m_prev_rlon, m_prev_rlat, gcDist * fraction,
                                                             gcBearing, &gcCoord.x, &gcCoord.y, NULL );
                                
                                if( i < segmentCount ) {
                                    gcPoint = new RoutePoint( gcCoord.y, gcCoord.x, _T("xmblue"), _T(""),
                                                              wxEmptyString );
                                    gcPoint->SetNameShown( false );
                                    pConfig->AddNewWayPoint( gcPoint, -1 );
                                    pSelect->AddSelectableRoutePoint( gcCoord.y, gcCoord.x, gcPoint );
                                } else {
                                    gcPoint = pMousePoint; // Last point, previously exsisting!
                                }
                                
                                m_pMouseRoute->AddPoint( gcPoint );
                                pSelect->AddSelectableRouteSegment( prevGcPoint->m_lat, prevGcPoint->m_lon,
                                                                    gcPoint->m_lat, gcPoint->m_lon, prevGcPoint, gcPoint, m_pMouseRoute );
                                prevGcPoint = gcPoint;
                            }
                            
                            undo->CancelUndoableAction( true );
                            
                        } else {
                            m_pMouseRoute->AddPoint( pMousePoint );
                            pSelect->AddSelectableRouteSegment( m_prev_rlat, m_prev_rlon,
                                                                rlat, rlon, m_prev_pMousePoint, pMousePoint, m_pMouseRoute );
                            undo->AfterUndoableAction( m_pMouseRoute );
                        }
                    } else {
                        // Ordinary rhumblinesegment.
                        m_pMouseRoute->AddPoint( pMousePoint );
                        pSelect->AddSelectableRouteSegment( m_prev_rlat, m_prev_rlon,
                                                            rlat, rlon, m_prev_pMousePoint, pMousePoint, m_pMouseRoute );
                        undo->AfterUndoableAction( m_pMouseRoute );
                    }
                }
                
                m_prev_rlat = rlat;
                m_prev_rlon = rlon;
                m_prev_pMousePoint = pMousePoint;
                m_pMouseRoute->m_lastMousePointIndex = m_pMouseRoute->GetnPoints();
                
                m_routeState++;
                Refresh( true );
                ret = true;
            }
            else if( m_bMeasure_Active && m_nMeasureState )   // measure tool?
            {
                if(m_bedge_pan){
                    m_bedge_pan = false;
                    return false;
                }
                
                double rlat, rlon;
                
                rlat = m_cursor_lat;
                rlon = m_cursor_lon;
                
                if( m_nMeasureState == 1 ) {
                    m_pMeasureRoute = new Route();
                    pRouteList->Append( m_pMeasureRoute );
                    r_rband.x = x;
                    r_rband.y = y;
                }
                
                
                RoutePoint *pMousePoint = new RoutePoint( m_cursor_lat, m_cursor_lon,
                                                          QString( _T ( "circle" ) ), wxEmptyString, wxEmptyString );
                pMousePoint->m_bShowName = false;

                m_pMeasureRoute->AddPoint( pMousePoint );

                m_prev_rlat = m_cursor_lat;
                m_prev_rlon = m_cursor_lon;
                m_prev_pMousePoint = pMousePoint;
                m_pMeasureRoute->m_lastMousePointIndex = m_pMeasureRoute->GetnPoints();

                m_nMeasureState++;

                Refresh( true );
                ret = true;
            }
            else {
                
                bool bSelectAllowed = true;
                if( NULL == g_pMarkInfoDialog ) {
                    if( g_bWayPointPreventDragging ) bSelectAllowed = false;
                } else if( !g_pMarkInfoDialog->IsShown() && g_bWayPointPreventDragging )
                    bSelectAllowed = false;

                /*if this left up happens at the end of a route point dragging and if the cursor/thumb is on the
                draghandle icon, not on the point iself a new selection will select nothing and the drag will never
                be ended, so the legs around this point never selectable. At this step we don't need a new selection,
                just keep the previoulsly selected and dragged point */
                if (m_bRoutePoinDragging)
                    bSelectAllowed = false;
                
                if(bSelectAllowed){
                    
                    bool b_was_editing_mark = m_bMarkEditing;
                    bool b_was_editing_route = m_bRouteEditing;
                    FindRoutePointsAtCursor( SelectRadius, true );    // Possibly selecting a point in a route for later dragging

                    /*route and a mark points in layer can't be dragged so should't be selected and no draghandle icon*/
                    if (m_pRoutePointEditTarget && m_pRoutePointEditTarget->m_bIsInLayer)
                        m_pRoutePointEditTarget = NULL;

                    if( !b_was_editing_route ) {
                        if( m_pEditRouteArray ) {
                            b_startedit_route = true;


                            //  Hide the track and route rollover during route point edit, not needed, and may be confusing
                            if( m_pTrackRolloverWin && m_pTrackRolloverWin->IsActive()  ) {
                                m_pTrackRolloverWin->IsActive( false );
                            }
                            if( m_pRouteRolloverWin && m_pRouteRolloverWin->IsActive()  ) {
                                m_pRouteRolloverWin->IsActive( false );
                            }

                            wxRect pre_rect;
                            for( unsigned int ir = 0; ir < m_pEditRouteArray->count(); ir++ ) {
                                Route *pr = (Route *) m_pEditRouteArray->at( ir );
                                //      Need to validate route pointer
                                //      Route may be gone due to drgging close to ownship with
                                //      "Delete On Arrival" state set, as in the case of
                                //      navigating to an isolated waypoint on a temporary route
                                if( g_pRouteMan->IsRouteValid(pr) ) {
                                    //                                pr->SetHiLite(50);
                                    wxRect route_rect;
                                    pr->CalculateDCRect( m_dc_route, this, &route_rect );
                                    pre_rect.Union( route_rect );
                                }
                            }
                            RefreshRect( pre_rect, true );
                        }
                    }
                    else {
                        b_startedit_route = false;
                    }


                    //  Mark editing
                    if( m_pRoutePointEditTarget ) {

                        if(b_was_editing_mark || b_was_editing_route) {            // kill previous hilight
                            if( m_lastRoutePointEditTarget) {
                                m_lastRoutePointEditTarget->m_bRPIsBeingEdited = false;
                                m_lastRoutePointEditTarget->m_bPtIsSelected = false;
                                m_lastRoutePointEditTarget->EnableDragHandle( false );
                                pSelect->DeleteSelectablePoint( m_lastRoutePointEditTarget, SELTYPE_DRAGHANDLE );

                            }
                        }

                        if( m_pRoutePointEditTarget) {
                            m_pRoutePointEditTarget->m_bRPIsBeingEdited = true;
                            m_pRoutePointEditTarget->m_bPtIsSelected = true;
                            m_pRoutePointEditTarget->EnableDragHandle( true );
                            zchxPoint2DDouble dragHandlePoint = m_pRoutePointEditTarget->GetDragHandlePoint(this);
                            pSelect->AddSelectablePoint(dragHandlePoint.m_y, dragHandlePoint.m_x, m_pRoutePointEditTarget, SELTYPE_DRAGHANDLE);

                        }
                    }
                    else {                  // Deselect everything
                        if( m_lastRoutePointEditTarget) {
                            m_lastRoutePointEditTarget->m_bRPIsBeingEdited = false;
                            m_lastRoutePointEditTarget->m_bPtIsSelected = false;
                            m_lastRoutePointEditTarget->EnableDragHandle( false );
                            pSelect->DeleteSelectablePoint( m_lastRoutePointEditTarget, SELTYPE_DRAGHANDLE );

                            //  Clear any routes being edited, probably orphans
                            wxArrayPtrVoid *lastEditRouteArray = g_pRouteMan->GetRouteArrayContaining( m_lastRoutePointEditTarget );
                            if( lastEditRouteArray ) {
                                for( unsigned int ir = 0; ir < lastEditRouteArray->count(); ir++ ) {
                                    Route *pr = (Route *) lastEditRouteArray->at( ir );
                                    if( g_pRouteMan->IsRouteValid(pr) ) {
                                        pr->m_bIsBeingEdited = false;
                                    }
                                }
                            }
                        }
                    }

                    //  Do the refresh

                    if(g_bopengl) {
                        InvalidateGL();
                        Refresh( false );
                    } else {
                        if( m_lastRoutePointEditTarget) {
                            wxRect wp_rect;
                            m_lastRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &wp_rect );
                            RefreshRect( wp_rect, true );
                        }
                        
                        if( m_pRoutePointEditTarget) {
                            wxRect wp_rect;
                            m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &wp_rect );
                            RefreshRect( wp_rect, true );
                        }
                    }
                }
            }       //  bSelectAllowed
            
            //      Check to see if there is a route or AIS target under the cursor
            //      If so, start the rollover timer which creates the popup
            bool b_start_rollover = false;
            if( g_pAIS && g_pAIS->GetNumTargets() && m_bShowAIS ) {
                SelectItem *pFind = pSelectAIS->FindSelection( this, m_cursor_lat, m_cursor_lon, SELTYPE_AISTARGET );
                if( pFind )
                    b_start_rollover = true;
            }
            
            if(!b_start_rollover && !b_startedit_route){
                SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_ROUTESEGMENT );
                wxSelectableItemListNode *node = SelList.GetFirst();
                while( node ) {
                    SelectItem *pFindSel = node->GetData();
                    
                    Route *pr = (Route *) pFindSel->m_pData3;        //candidate
                    
                    if( pr && pr->IsVisible() ){
                        b_start_rollover = true;
                        break;
                    }
                    node = node->GetNext();
                }       // while
            }
            
            if(!b_start_rollover && !b_startedit_route){
                SelectableItemList SelList = pSelect->FindSelectionList( this, m_cursor_lat, m_cursor_lon, SELTYPE_TRACKSEGMENT );
                wxSelectableItemListNode *node = SelList.GetFirst();
                while( node ) {
                    SelectItem *pFindSel = node->GetData();

                    Track *tr = (Track *) pFindSel->m_pData3;        //candidate

                    if( tr && tr->IsVisible() ){
                        b_start_rollover = true;
                        break;
                    }
                    node = node->GetNext();
                }       // while
            }

            if( b_start_rollover )
                m_RolloverPopupTimer.Start( m_rollover_popup_timer_msec, QTimer_ONE_SHOT );
            
            
            if( m_bRouteEditing/* && !b_startedit_route*/) {            // End of RoutePoint drag
                if( m_pRoutePointEditTarget ) {
                    pSelect->UpdateSelectableRouteSegments( m_pRoutePointEditTarget );

                    if( m_pEditRouteArray ) {
                        for( unsigned int ir = 0; ir < m_pEditRouteArray->count(); ir++ ) {
                            Route *pr = (Route *) m_pEditRouteArray->at( ir );
                            if( g_pRouteMan->IsRouteValid(pr) ) {
                                pr->FinalizeForRendering();
                                pr->UpdateSegmentDistances();
                                if( m_bRoutePoinDragging ) pConfig->UpdateRoute( pr );
                            }
                        }
                    }

                    //    Update the RouteProperties Dialog, if currently shown
                    if( pRoutePropDialog && pRoutePropDialog->IsShown() ) {
                        if( m_pEditRouteArray ) {
                            for( unsigned int ir = 0; ir < m_pEditRouteArray->count(); ir++ ) {
                                Route *pr = (Route *) m_pEditRouteArray->at( ir );
                                if( g_pRouteMan->IsRouteValid(pr) ) {
                                    if( pRoutePropDialog->GetRoute() == pr ) {
                                        pRoutePropDialog->SetRouteAndUpdate( pr, true );
                                    }
                                    /* cannot edit track points anyway
                                else if ( ( NULL != pTrackPropDialog ) && ( pTrackPropDialog->IsShown() ) && pTrackPropDialog->m_pTrack == pr ) {
                                    pTrackPropDialog->SetTrackAndUpdate( pr );
                                }
*/
                                }
                            }
                        }
                    }

                }
            }
            
            else if(  m_bMarkEditing ) {				// End of way point drag
                if( m_pRoutePointEditTarget )
                    if( m_bRoutePoinDragging ) pConfig->UpdateWayPoint( m_pRoutePointEditTarget );
            }

            if( m_pRoutePointEditTarget )
                undo->AfterUndoableAction( m_pRoutePointEditTarget );
            
            if(!m_pRoutePointEditTarget){
                delete m_pEditRouteArray;
                m_pEditRouteArray = NULL;
                m_bRouteEditing = false;
            }
            m_bRoutePoinDragging = false;
        }       // g_btouch
        
        
        else{                   // !g_btouch
            if( m_bRouteEditing ) {            // End of RoutePoint drag
                if( m_pRoutePointEditTarget ) {
                    pSelect->UpdateSelectableRouteSegments( m_pRoutePointEditTarget );
                    m_pRoutePointEditTarget->m_bBlink = false;

                    if( m_pEditRouteArray ) {
                        for( unsigned int ir = 0; ir < m_pEditRouteArray->count(); ir++ ) {
                            Route *pr = (Route *) m_pEditRouteArray->at( ir );
                            if( g_pRouteMan->IsRouteValid(pr) ) {
                                pr->FinalizeForRendering();
                                pr->UpdateSegmentDistances();
                                pr->m_bIsBeingEdited = false;

                                if( m_bRoutePoinDragging ) pConfig->UpdateRoute( pr );

                                pr->SetHiLite( 0 );
                            }
                        }
                        Refresh( false );
                    }

                    //    Update the RouteProperties Dialog, if currently shown
                    if( pRoutePropDialog && pRoutePropDialog->IsShown() ) {
                        if( m_pEditRouteArray ) {
                            for( unsigned int ir = 0; ir < m_pEditRouteArray->count(); ir++ ) {
                                Route *pr = (Route *) m_pEditRouteArray->at( ir );
                                if( g_pRouteMan->IsRouteValid(pr) ) {
                                    if( pRoutePropDialog->GetRoute() == pr ) {
                                        pRoutePropDialog->SetRouteAndUpdate( pr, true );
                                    }
                                }
                            }
                        }
                    }

                    m_pRoutePointEditTarget->m_bPtIsSelected = false;
                    m_pRoutePointEditTarget->m_bRPIsBeingEdited = false;

                    delete m_pEditRouteArray;
                    m_pEditRouteArray = NULL;
                    undo->AfterUndoableAction( m_pRoutePointEditTarget );
                }

                InvalidateGL();
                m_bRouteEditing = false;
                m_pRoutePointEditTarget = NULL;

                if( m_toolBar && !m_toolBar->IsToolbarShown())
                    SurfaceToolbar();
                ret = true;
            }

            else if( m_bMarkEditing) {         // end of Waypoint drag
                if( m_pRoutePointEditTarget ) {
                    if( m_bRoutePoinDragging ) pConfig->UpdateWayPoint( m_pRoutePointEditTarget );
                    undo->AfterUndoableAction( m_pRoutePointEditTarget );
                    m_pRoutePointEditTarget->m_bRPIsBeingEdited = false;
                    wxRect wp_rect;
                    m_pRoutePointEditTarget->CalculateDCRect( m_dc_route, this, &wp_rect );
                    m_pRoutePointEditTarget->m_bPtIsSelected = false;
                    RefreshRect( wp_rect, true );

                }
                m_pRoutePointEditTarget = NULL;
                m_bMarkEditing = false;
                if( m_toolBar && !m_toolBar->IsToolbarShown())
                    SurfaceToolbar();
                ret = true;
            }

            else if( leftIsDown ) {  // left click for chart center
                leftIsDown = false;
                ret = false;

                if( !g_btouch ){
                    if( !m_bChartDragging && !m_bMeasure_Active ) {
                    } else {
                        m_bChartDragging = false;
                    }
                }

            }
            m_bRoutePoinDragging = false;
        }       // !btouch
        
        if(ret)
            return true;
    }           // left up
    
    if( event.RightDown() ) {
        SetFocus();           //  This is to let a plugin know which canvas is right-clicked
        last_drag.x = mx;
        last_drag.y = my;
        
        if(g_btouch ){
            //            if( m_pRoutePointEditTarget )
            //                return false;
        }
        
        ret = true;
        m_FinishRouteOnKillFocus = false;
        CallPopupMenu(mx , my);
        m_FinishRouteOnKillFocus = true;
    }   //Right down

    return ret;
#endif

}

bool panleftIsDown;

bool ChartCanvas::MouseEventProcessCanvas( QMouseEvent* event )
{
#if 0
    int x, y;
    event.GetPosition( &x, &y );
    
    //        Check for wheel rotation
    // ideally, should be just longer than the time between
    // processing accumulated mouse events from the event queue
    // as would happen during screen redraws.
    int wheel_dir = event.GetWheelRotation();
    
    if( wheel_dir ) {
        int mouse_wheel_oneshot = abs(wheel_dir)*4;                  //msec
        wheel_dir = wheel_dir > 0 ? 1 : -1; // normalize
        
        double factor = 2.0;
        if(wheel_dir < 0)
            factor = 1/factor;
        
        if(g_bsmoothpanzoom){
            if( (m_wheelstopwatch.elapsed() < m_wheelzoom_stop_oneshot) ) {
                if( wheel_dir == m_last_wheel_dir ) {
                    m_wheelzoom_stop_oneshot += mouse_wheel_oneshot;
                    //                    m_zoom_target /= factor;
                }
                else
                    StopMovement( );
            }
            else {
                m_wheelzoom_stop_oneshot = mouse_wheel_oneshot;
                m_wheelstopwatch.start();
                //                m_zoom_target =  VPoint.chartScale() / factor;
            }
        }
        
        m_last_wheel_dir = wheel_dir;
        
        
        ZoomCanvas( factor, true, false );
        
    }

    if( event.LeftDown() ) {
        // Skip the first left click if it will cause a canvas focus shift
        if( (GetCanvasCount() > 1) &&  (this != g_focusCanvas) ){
            //printf("focus shift\n");
            return false;
        }
        
        last_drag.x = x, last_drag.y = y;
        panleftIsDown = true;
    }
    
    if( event.LeftUp() ) {
        if( panleftIsDown ) {  // leftUp for chart center, but only after a leftDown seen here.
            panleftIsDown = false;
            
            if( !g_btouch ){
                if( !m_bChartDragging && !m_bMeasure_Active ) {
                    switch( cursor_region ){
                    case MID_RIGHT: {
                        PanCanvas( 100, 0 );
                        break;
                    }
                        
                    case MID_LEFT: {
                        PanCanvas( -100, 0 );
                        break;
                    }
                        
                    case MID_TOP: {
                        PanCanvas( 0, 100 );
                        break;
                    }
                        
                    case MID_BOT: {
                        PanCanvas( 0, -100 );
                        break;
                    }
                        
                    case CENTER: {
                        PanCanvas( x - GetVP().pixWidth() / 2, y - GetVP().pixHeight() / 2 );
                        break;
                    }
                    }
                } else {
                    m_bChartDragging = false;
                }
            }
        }
    }
    
    if( event.Dragging() && event.LeftIsDown()){
        /*
             * fixed dragging.
             * On my Surface Pro 3 running Arch Linux there is no mouse down event before the drag event.
             * Hence, as there is no mouse down event, last_drag is not reset before the drag.
             * And that results in one single drag session, meaning you cannot drag the map a few miles
             * north, lift your finger, and the go even further north. Instead, the map resets itself
             * always to the very first drag start (since there is not reset of last_drag).
             *
             * Besides, should not left down and dragging be enough of a situation to start a drag procedure?
             *
             * Anyways, guarded it to be active in touch situations only.
             */
        if( g_btouch ) {
            if(false == m_bChartDragging) {
                last_drag.x = x, last_drag.y = y;
                m_bChartDragging = true;
            }
        }


        if( ( last_drag.x != x ) || ( last_drag.y != y ) ) {
            m_bChartDragging = true;
            StartTimedMovement();
            m_pan_drag.x += last_drag.x - x;
            m_pan_drag.y += last_drag.y - y;

            last_drag.x = x, last_drag.y = y;

            if( g_btouch ) {
                if(( m_bMeasure_Active && m_nMeasureState ) || ( m_routeState )){
                    //deactivate next LeftUp to ovoid creating an unexpected point
                    m_DoubleClickTimer->Start();
                    singleClickEventIsValid = false;
                }
            }

        }
    }

#endif

    return true;
    

}

void ChartCanvas::MouseEvent( QMouseEvent& event )
{
#if 0
    if (MouseEventOverlayWindows( event ))
        return;

    if(MouseEventSetup( event ))
        return;              // handled, no further action required
    
    if(!MouseEventProcessObjects( event ))
        MouseEventProcessCanvas( event );
#endif
}

void ChartCanvas::mousePressEvent(QMouseEvent *e)
{
    qDebug()<<" mouse presss now";
    last_drag_point = e->pos();

}

void ChartCanvas::mouseMoveEvent(QMouseEvent *e)
{
    qDebug()<<" mouse move now"<<hasMouseTracking();
    m_MouseDragging = true;
    //没有拖动的情况,将地图的中心移动到这里
    QPoint pos = e->pos();
    int dx  = pos.x() - last_drag_point.x();
    int dy  =  pos.y() - last_drag_point.y();
    if(abs(dx) > 10 || abs(dy) > 10)
    PanCanvas(-dx, -dy);
    last_drag_point = pos;
}

void ChartCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    if(!m_MouseDragging)
    {
        //没有拖动的情况,将地图的中心移动到这里
        QPoint pos = e->pos();
        PanCanvas( pos.x() - GetVP().pixWidth() / 2, pos.y() - GetVP().pixHeight() / 2 );
    }
    m_MouseDragging = false;
}

void ChartCanvas::wheelEvent(QWheelEvent * e)
{
    qDebug()<<"wheel event now:";
    QTime t;
    t.start();
    static uint time = 0;
    uint cur = QDateTime::currentDateTime().toTime_t();
    if(cur - time >= 2)
    {
        qDebug()<<"start wheel:"<<QDateTime::currentDateTime();
        time = cur;
        if(e->delta() > 0)
        {
            ZoomCanvas(2, false);
        } else
        {
            ZoomCanvas(0.5, false);
        }

        qDebug()<<"end wheel:"<<QDateTime::currentDateTime();

    }

    qDebug()<<"wheel end with elaped time:"<<t.elapsed();

}


void ChartCanvas::SetCanvasCursor( QMouseEvent* event )
{
    //    Switch to the appropriate cursor on mouse movement

    QCursor *ptarget_cursor = pCursorArrow;
    if( !pPlugIn_Cursor ) {
        ptarget_cursor = pCursorArrow;
        if( ( !m_routeState )
                && ( !m_bMeasure_Active ) /*&& ( !m_bCM93MeasureOffset_Active )*/) {
            
            if( cursor_region == MID_RIGHT ) {
                ptarget_cursor = pCursorRight;
            } else if( cursor_region == MID_LEFT ) {
                ptarget_cursor = pCursorLeft;
            } else if( cursor_region == MID_TOP ) {
                ptarget_cursor = pCursorDown;
            } else if( cursor_region == MID_BOT ) {
                ptarget_cursor = pCursorUp;
            } else {
                ptarget_cursor = pCursorArrow;
            }
        } else if( m_bMeasure_Active || m_routeState ) // If Measure tool use Pencil Cursor
            ptarget_cursor = pCursorPencil;
    }
    else {
        ptarget_cursor = pPlugIn_Cursor;
    }
    

    SetCursor( *ptarget_cursor );

}





//void ChartCanvas::LostMouseCapture( wxMouseCaptureLostEvent& event )
//{
//    SetCursor( *pCursorArrow );
//}



void ChartCanvas::ShowObjectQueryWindow( int x, int y, float zlat, float zlon )
{
#if 0
    ChartPlugInWrapper *target_plugin_chart = NULL;
    s57chart *Chs57 = NULL;

    ChartBase *target_chart = GetChartAtCursor();
    if( target_chart ){
        if( (target_chart->GetChartType() == CHART_TYPE_PLUGIN) && (target_chart->GetChartFamily() == CHART_FAMILY_VECTOR) )
            target_plugin_chart = dynamic_cast<ChartPlugInWrapper *>(target_chart);
        else
            Chs57 = dynamic_cast<s57chart*>( target_chart );
    }

    std::vector<Ais8_001_22*> area_notices;



    if( target_plugin_chart || Chs57 || !area_notices.empty() ) {
        // Go get the array of all objects at the cursor lat/lon
        int sel_rad_pix = 5;
        float SelectRadius = sel_rad_pix / ( GetVP().viewScalePPM() * 1852 * 60 );

        // Make sure we always get the lights from an object, even if we are currently
        // not displaying lights on the chart.

        SetCursor( QCursor_WAIT );
        bool lightsVis = m_encShowLights; //gFrame->ToggleLights( false );
        if( !lightsVis ) SetShowENCLights( true );
        ;

        ListOfObjRazRules* rule_list = NULL;
        ListOfPI_S57Obj* pi_rule_list = NULL;
        if( Chs57 )
            rule_list = Chs57->GetObjRuleListAtLatLon( zlat, zlon, SelectRadius, &GetVP() );
        else if( target_plugin_chart )
            pi_rule_list = g_pi_manager->GetPlugInObjRuleListAtLatLon( target_plugin_chart, zlat, zlon, SelectRadius, GetVP() );

        ListOfObjRazRules* overlay_rule_list = NULL;
        ChartBase *overlay_chart = GetOverlayChartAtCursor();
        s57chart *CHs57_Overlay = dynamic_cast<s57chart*>( overlay_chart );

        if( CHs57_Overlay ) {
            overlay_rule_list =
                    CHs57_Overlay->GetObjRuleListAtLatLon( zlat, zlon, SelectRadius, &GetVP() );
        }

        if( !lightsVis ) SetShowENCLights( false );

        QString objText;
        QFont *dFont = FontMgr::Get().GetFont( _("ObjectQuery") );
        QString face = dFont->GetFaceName();

        if( NULL == g_pObjectQueryDialog ) {
            g_pObjectQueryDialog = new S57QueryDialog(this, -1, _( "Object Query" ), wxDefaultPosition, wxSize( g_S57_dialog_sx, g_S57_dialog_sy ));
        }

        wxColor bg = g_pObjectQueryDialog->GetBackgroundColour();
        wxColor fg = FontMgr::Get().GetFontColor( _("ObjectQuery") );

        objText.Printf( _T("<html><body bgcolor=#%02x%02x%02x><font color=#%02x%02x%02x>"),
                        bg.Red(), bg.Green(), bg.Blue(), fg.Red(), fg.Green(), fg.Blue() );

#ifdef __WXOSX__
        int points = dFont->GetPointSize();
#else
        int points = dFont->GetPointSize() + 1;
#endif

        int sizes[7];
        for ( int i=-2; i<5; i++ ) {
            sizes[i+2] = points + i + (i>0?i:0);
        }
        g_pObjectQueryDialog->m_phtml->SetFonts(face, face, sizes);

        if(QFontSTYLE_ITALIC == dFont->GetStyle())
            objText += _T("<i>");
        
        if( overlay_rule_list && CHs57_Overlay) {
            objText << CHs57_Overlay->CreateObjDescriptions( overlay_rule_list );
            objText << _T("<hr noshade>");
        }

        for( std::vector< Ais8_001_22* >::iterator an = area_notices.begin(); an != area_notices.end(); ++an ) {
            objText << _T( "<b>AIS Area Notice:</b> " );
            objText << ais8_001_22_notice_names[( *an )->notice_type];
            for( std::vector< Ais8_001_22_SubArea >::iterator sa = ( *an )->sub_areas.begin(); sa != ( *an )->sub_areas.end(); ++sa )
                if( !sa->text.empty() )
                    objText << sa->text;
            objText << _T( "<br>expires: " ) << ( *an )->expiry_time.Format();
            objText << _T( "<hr noshade>" );
        }

        if( Chs57 )
            objText << Chs57->CreateObjDescriptions( rule_list );
        else if( target_plugin_chart )
            objText << g_pi_manager->CreateObjDescriptions( target_plugin_chart, pi_rule_list );

        objText << _T("</font>");
        if(QFontSTYLE_ITALIC == dFont->GetStyle())
            objText << _T("</i>");
        
        objText << _T("</body></html>");
        
        g_pObjectQueryDialog->SetHTMLPage( objText );

        g_pObjectQueryDialog->Show();

        if( rule_list )
            rule_list->Clear();
        delete rule_list;

        if( overlay_rule_list )
            overlay_rule_list->Clear();
        delete overlay_rule_list;

        if( pi_rule_list )
            pi_rule_list->Clear();
        delete pi_rule_list;

        SetCursor( QCursor_ARROW );
    }
    #endif
}



bool ChartCanvas::InvokeCanvasMenu(int x, int y, int seltype)
{
#if 0
    m_canvasMenu = new CanvasMenuHandler(this, m_pSelectedRoute, m_pSelectedTrack,
                                         m_pFoundRoutePoint, m_FoundAIS_MMSI, m_pIDXCandidate);
    
    Connect(  wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction) (wxEventFunction) &ChartCanvas::PopupMenuHandler );

    m_canvasMenu->CanvasPopupMenu( x, y, seltype );

    Disconnect(  wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction) (wxEventFunction) &ChartCanvas::PopupMenuHandler );

    delete m_canvasMenu;
    m_canvasMenu = NULL;

#ifdef __WXQT__
    gFrame->SurfaceToolbar();
    //g_MainToolbar->Raise();
#endif
#endif
    
    return true;
}

//void ChartCanvas::PopupMenuHandler( wxCommandEvent& event )
//{
//    //  Pass menu events from the canvas to the menu handler
//    //  This is necessarily in ChartCanvas since that is the menu's parent.
//    if(m_canvasMenu){
//        m_canvasMenu->PopupMenuHandler( event );
//    }
//    return;
    
//}

void ChartCanvas::HideGlobalToolbar()
{
    if(m_canvasIndex == 0){
//        m_last_TBviz = gFrame->SetGlobalToolbarViz( false );
    }
}

void ChartCanvas::ShowGlobalToolbar()
{
    if(m_canvasIndex == 0){
//        if(m_last_TBviz)
//            gFrame->SetGlobalToolbarViz( true );
    }
}





void ChartCanvas::RenderAllChartOutlines( ocpnDC &dc, ViewPort& vp )
{
    if( !m_bShowOutlines ) return;

    int nEntry = ChartData->GetChartTableEntries();

    for( int i = 0; i < nEntry; i++ ) {
        ChartTableEntry *pt = (ChartTableEntry *) &ChartData->GetChartTableEntry( i );

        //    Check to see if the candidate chart is in the currently active group
        bool b_group_draw = false;
        if( m_groupIndex > 0 ) {
            for( unsigned int ig = 0; ig < pt->GetGroupArray().size(); ig++ ) {
                int index = pt->GetGroupArray()[ig];
                if( m_groupIndex == index ) {
                    b_group_draw = true;
                    break;
                }
            }
        } else
            b_group_draw = true;

        if( b_group_draw ) RenderChartOutline( dc, i, vp );
    }
#if 0
    //        On CM93 Composite Charts, draw the outlines of the next smaller scale cell
    cm93compchart *pcm93 = NULL;
    if( VPoint.quilt() ) {
        for(ChartBase *pch = GetFirstQuiltChart(); pch; pch = GetNextQuiltChart())
            if( pch->GetChartType() == CHART_TYPE_CM93COMP ) {
                pcm93 = (cm93compchart *)pch;
                break;
            }
    } else
        if ( m_singleChart && ( m_singleChart->GetChartType() == CHART_TYPE_CM93COMP ) )
            pcm93 = (cm93compchart *) m_singleChart;

    if( pcm93 ) {
        double chart_native_ppm = m_canvas_scale_factor / pcm93->GetNativeScale();
        double zoom_factor = GetVP().viewScalePPM() / chart_native_ppm;

        if( zoom_factor > 8.0 ) {
            QPen mPen( GetGlobalColor( ("UINFM") ), 2, Qt::DashLine );
            dc.SetPen( mPen );
        } else {
            QPen mPen( GetGlobalColor( ("UINFM") ), 1, Qt::SolidLine );
            dc.SetPen( mPen );
        }
        
        pcm93->RenderNextSmallerCellOutlines( dc, vp, this );
    }
#endif
}

void ChartCanvas::RenderChartOutline( ocpnDC &dc, int dbIndex, ViewPort& vp )
{
    if(!m_glcc) return;
    /* opengl version specially optimized */
    m_glcc->RenderChartOutline(dbIndex, vp);
    return;
}

static void RouteLegInfo( ocpnDC &dc, zchxPoint ref_point, const QString &first, const QString &second )
{
    QFont dFont = FontMgr::Get().GetFont( ("RouteLegInfoRollover") );
    dc.SetFont( dFont );

    int w1, h1;
    int w2 = 0;
    int h2 = 0;
    int h, w;
    
    int xp, yp;
    int hilite_offset = 3;
#ifdef __WXMAC__
    wxScreenDC sdc;
    sdc.GetTextExtent(first, &w1, &h1, NULL, NULL, dFont);
    if(second.Len())
        sdc.GetTextExtent(second, &w2, &h2, NULL, NULL, dFont);
#else
    dc.GetTextExtent( first, &w1, &h1 );
    if(second.length())
        dc.GetTextExtent( second, &w2, &h2 );
#endif

    w = fmax(w1, w2);
    h = h1 + h2;
    
    xp = ref_point.x - w;
    yp = ref_point.y;
    yp += hilite_offset;

    zchxFuncUtil::AlphaBlending(xp, yp, w, h, 0.0, GetGlobalColor( ( "YELO1" ) ), 172 );
    
    dc.SetPen( QPen( GetGlobalColor( ( "UBLCK" ) ) ) );
    dc.SetTextForeground( FontMgr::Get().GetFontColor( ("RouteLegInfoRollover") ) );
    dc.drawText( first, xp, yp );
    if(second.length())
        dc.drawText( second, xp, yp + h1 );
}


int s_msg;

void ChartCanvas::UpdateCanvasS52PLIBConfig()
{
    if(!ps52plib)
        return;
    
    if( VPoint.quilt() ){          // quilted
        if( !m_pQuilt->IsComposed() )
            return;  // not ready

        if(m_pQuilt->IsQuiltVector()){
            if(ps52plib->GetStateHash() != m_s52StateHash){
                UpdateS52State();
                m_s52StateHash = ps52plib->GetStateHash();
            }
        }
    }
    else{
        if(ps52plib->GetStateHash() != m_s52StateHash){
            UpdateS52State();
            m_s52StateHash = ps52plib->GetStateHash();
        }
    }
    
    // Plugin charts
    bool bSendPlibState = true;
    if( VPoint.quilt() ){          // quilted
        if(!m_pQuilt->DoesQuiltContainPlugins())
            bSendPlibState = false;
    }

    if(bSendPlibState){
        QVariantMap v;
        v[("OpenCPN Version Major")] = VERSION_MAJOR;
        v[("OpenCPN Version Minor")] = VERSION_MINOR;
        v[("OpenCPN Version Patch")] = VERSION_PATCH;
        v[("OpenCPN Version Date")] = VERSION_DATE;
        v[("OpenCPN Version Full")] = VERSION_FULL;
        
        //  S52PLIB state
        v[("OpenCPN S52PLIB ShowText")] = GetShowENCText();
        v[("OpenCPN S52PLIB ShowSoundings")] = GetShowENCDepth();
        v[("OpenCPN S52PLIB ShowLights")] = GetShowENCLights();
        v[("OpenCPN S52PLIB ShowAnchorConditions")] = m_encShowAnchor; //ps52plib->GetAnchorOn();
        v[("OpenCPN S52PLIB ShowQualityOfData")] = GetShowENCDataQual(); //ps52plib->GetQualityOfDataOn();
        v[("OpenCPN S52PLIB ShowATONLabel")] = GetShowENCBuoyLabels();
        v[("OpenCPN S52PLIB ShowLightDescription")] = GetShowENCLightDesc();

        v[("OpenCPN S52PLIB DisplayCategory")] = GetENCDisplayCategory();
        
        // Global options
        /*
        v[_T("OpenCPN S52PLIB MetaDisplay")] = ps52plib->m_bShowMeta;
        v[_T("OpenCPN S52PLIB DeclutterText")] = ps52plib->m_bDeClutterText;
        v[_T("OpenCPN S52PLIB ShowNationalText")] = ps52plib->m_bShowNationalTexts;
        v[_T("OpenCPN S52PLIB ShowImportantTextOnly")] = ps52plib->m_bShowS57ImportantTextOnly;
        v[_T("OpenCPN S52PLIB UseSCAMIN")] = ps52plib->m_bUseSCAMIN;
        v[_T("OpenCPN S52PLIB SymbolStyle")] = ps52plib->m_nSymbolStyle;
        v[_T("OpenCPN S52PLIB BoundaryStyle")] = ps52plib->m_nBoundaryStyle;
        v[_T("OpenCPN S52PLIB ColorShades")] = S52_getMarinerParam( S52_MAR_TWO_SHADES );
*/
        QString out = QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(v)).toJson());
        
//        if(!g_lastPluginMessage.IsSameAs(out)){
//            //printf("message %d  %d\n", s_msg++, m_canvasIndex);
//            g_pi_manager->SendMessageToAllPlugins(QString(_T("OpenCPN Config")), out);
//        }
    }
}
int spaint;
int s_in_update;
void ChartCanvas::paintEvent(QPaintEvent *event)
{

    //GetToolbar()->Show( m_bToolbarEnable );
    
    //  Paint updates may have been externally disabled (temporarily, to avoid Yield() recursion performance loss)
    //  It is important that the wxPaintDC is built, even if we elect to not process this paint message.
    //  Otherwise, the paint message may not be removed from the message queue, esp on Windows. (FS#1213)
    //  This would lead to a deadlock condition in ::wxYield()
    if(!m_b_paint_enable){
        return;
    }

    
    //  If necessary, reconfigure the S52 PLIB
    UpdateCanvasS52PLIBConfig();

    if( m_glcc ) {
        if( !s_in_update ) {          // no recursion allowed, seen on lo-spec Mac
            s_in_update++;
            m_glcc->update();
            s_in_update--;
        }

        return;
    }



}

void ChartCanvas::PaintCleanup()
{
    //    Handle the current graphic window, if present




    // Start movement timer, this runs nearly immediately.
    // the reason we cannot simply call it directly is the
    // refresh events it emits may be blocked from this paint event
//    pMovementTimer->Start( 1, QTimer_ONE_SHOT );
}

#if 0
QColor GetErrorGraphicColor(double val)
{
    /*
     double valm = fmin(val_max, val);

     unsigned char green = (unsigned char)(255 * (1 - (valm/val_max)));
     unsigned char red   = (unsigned char)(255 * (valm/val_max));

     QImage::HSVValue hv = QImage::RGBtoHSV(QImage::RGBValue(red, green, 0));

     hv.saturation = 1.0;
     hv.value = 1.0;

     QImage::RGBValue rv = QImage::HSVtoRGB(hv);
     return QColor(rv.red, rv.green, rv.blue);
     */

    //    HTML colors taken from NOAA WW3 Web representation
    QColor c;
    if((val > 0) && (val < 1)) c.Set(_T("#002ad9"));
    else if((val >= 1) && (val < 2)) c.Set(_T("#006ed9"));
    else if((val >= 2) && (val < 3)) c.Set(_T("#00b2d9"));
    else if((val >= 3) && (val < 4)) c.Set(_T("#00d4d4"));
    else if((val >= 4) && (val < 5)) c.Set(_T("#00d9a6"));
    else if((val >= 5) && (val < 7)) c.Set(_T("#00d900"));
    else if((val >= 7) && (val < 9)) c.Set(_T("#95d900"));
    else if((val >= 9) && (val < 12)) c.Set(_T("#d9d900"));
    else if((val >= 12) && (val < 15)) c.Set(_T("#d9ae00"));
    else if((val >= 15) && (val < 18)) c.Set(_T("#d98300"));
    else if((val >= 18) && (val < 21)) c.Set(_T("#d95700"));
    else if((val >= 21) && (val < 24)) c.Set(_T("#d90000"));
    else if((val >= 24) && (val < 27)) c.Set(_T("#ae0000"));
    else if((val >= 27) && (val < 30)) c.Set(_T("#8c0000"));
    else if((val >= 30) && (val < 36)) c.Set(_T("#870000"));
    else if((val >= 36) && (val < 42)) c.Set(_T("#690000"));
    else if((val >= 42) && (val < 48)) c.Set(_T("#550000"));
    else if( val >= 48) c.Set(_T("#410000"));

    return c;
}

void ChartCanvas::RenderGeorefErrorMap( wxMemoryDC *pmdc, ViewPort *vp)
{
    QImage gr_image(vp->pixWidth(), vp->pixHeight());
    gr_image.InitAlpha();

    double maxval = -10000;
    double minval = 10000;

    double rlat, rlon;
    double glat, glon;

    GetCanvasPixPoint(0, 0, rlat, rlon);

    for(int i=1; i < vp->pixHeight()-1; i++)
    {
        for(int j=0; j < vp->pixWidth(); j++)
        {
            // Reference mercator value
            //                  vp->GetMercatorLLFromPix(zchxPoint(j, i), &rlat, &rlon);

            // Georef value
            GetCanvasPixPoint(j, i, glat, glon);

            maxval = fmax(maxval, (glat - rlat));
            minval = fmin(minval, (glat - rlat));

        }
        rlat = glat;
    }

    GetCanvasPixPoint(0, 0, rlat, rlon);
    for(int i=1; i < vp->pixHeight()-1; i++)
    {
        for(int j=0; j < vp->pixWidth(); j++)
        {
            // Reference mercator value
            //                  vp->GetMercatorLLFromPix(zchxPoint(j, i), &rlat, &rlon);

            // Georef value
            GetCanvasPixPoint(j, i, glat, glon);

            double f = ((glat - rlat)-minval)/(maxval - minval);

            double dy = (f * 40);

            QColor c = GetErrorGraphicColor(dy);
            unsigned char r = c.Red();
            unsigned char g = c.Green();
            unsigned char b = c.Blue();

            gr_image.SetRGB(j, i, r,g,b);
            if((glat - rlat )!= 0)
                gr_image.SetAlpha(j, i, 128);
            else
                gr_image.SetAlpha(j, i, 255);

        }
        rlat = glat;
    }

    //    Create a Bitmap
    wxBitmap *pbm = new wxBitmap(gr_image);
    wxMask *gr_mask = new wxMask(*pbm, QColor(0,0,0));
    pbm->SetMask(gr_mask);

    pmdc->DrawBitmap(*pbm, 0,0);

    delete pbm;

}

#endif

void ChartCanvas::CancelMouseRoute()
{
    m_routeState = 0;
//    m_pMouseRoute = NULL;
    m_bDrawingRoute = false;
}

//int ChartCanvas::GetNextContextMenuId()
//{
//    return CanvasMenuHandler::GetNextContextMenuId();
//}

bool ChartCanvas::SetCursor( const QCursor &c )
{
    if(m_glcc ) m_glcc->setCursor( c );
    return true;
}

void ChartCanvas::Refresh( bool eraseBackground, const QRect *rect )
{
    if( g_bquiting )
        return;
    //  Keep the mouse position members up to date
    GetCanvasPixPoint( mouse_x, mouse_y, m_cursor_lat, m_cursor_lon );

    //      Retrigger the route leg popup timer
    //      This handles the case when the chart is moving in auto-follow mode, but no user mouse input is made.
    //      The timer handler may Hide() the popup if the chart moved enough
    //      n.b.  We use slightly longer oneshot value to allow this method's Refresh() to complete before
    //      potentially getting another Refresh() in the popup timer handler.

    if( m_glcc ) {
        
        //      We need to invalidate the FBO cache to ensure repaint of "grounded" overlay objects.
        if( eraseBackground && m_glcc->UsingFBO() )    m_glcc->Invalidate();
        

//        m_glcc->Refresh( eraseBackground, NULL ); // We always are going to render the entire screen anyway, so make
        // sure that the window managers understand the invalid area
        // is actually the entire client area.

        //  We need to selectively Refresh some child windows, if they are visible.
        //  Note that some children are refreshed elsewhere on timer ticks, so don't need attention here.

        //      Thumbnail chart
//        if( pthumbwin && pthumbwin->IsShown() ) {
//            pthumbwin->Raise();
//            pthumbwin->Refresh( false );
//        }

        //      ChartInfo window
        if( m_pCIWin && !m_pCIWin->isHidden()) {
            m_pCIWin->raise();
//            m_pCIWin->Refresh( false );
        }
        
        //        if(g_MainToolbar)
        //            g_MainToolbar->UpdateRecoveryWindow(g_bshowToolbar);
        
    }

}

void ChartCanvas::Update()
{
    if( m_glcc ) m_glcc->update();
}

void ChartCanvas::DrawEmboss( ocpnDC &dc, emboss_data *pemboss)
{
#if 0
    if( !pemboss ) return;
    int x = pemboss->x, y = pemboss->y;
    const double factor = 200;

    wxASSERT_MSG( dc.GetDC(), wxT ( "DrawEmboss has no dc (opengl?)" ) );
    wxMemoryDC *pmdc = dynamic_cast<wxMemoryDC*>( dc.GetDC() );
    wxASSERT_MSG ( pmdc, wxT ( "dc to EmbossCanvas not a memory dc" ) );
    
    //Grab a snipped image out of the chart
    wxMemoryDC snip_dc;
    wxBitmap snip_bmp( pemboss->width, pemboss->height, -1 );
    snip_dc.SelectObject( snip_bmp );
    
    snip_dc.Blit( 0, 0, pemboss->width, pemboss->height, pmdc, x, y );
    snip_dc.SelectObject( wxNullBitmap );
    
    QImage snip_img = snip_bmp.ConvertToImage();
    
    //  Apply Emboss map to the snip image
    unsigned char* pdata = snip_img.GetData();
    if( pdata ) {
        for( int y = 0; y < pemboss->height; y++ ) {
            int map_index = ( y * pemboss->width );
            for( int x = 0; x < pemboss->width; x++ ) {
                double val = ( pemboss->pmap[map_index] * factor ) / 256.;
                
                int nred = (int) ( ( *pdata ) + val );
                nred = nred > 255 ? 255 : ( nred < 0 ? 0 : nred );
                *pdata++ = (unsigned char) nred;
                
                int ngreen = (int) ( ( *pdata ) + val );
                ngreen = ngreen > 255 ? 255 : ( ngreen < 0 ? 0 : ngreen );
                *pdata++ = (unsigned char) ngreen;
                
                int nblue = (int) ( ( *pdata ) + val );
                nblue = nblue > 255 ? 255 : ( nblue < 0 ? 0 : nblue );
                *pdata++ = (unsigned char) nblue;
                
                map_index++;
            }
        }
    }
    
    //  Convert embossed snip to a bitmap
    wxBitmap emb_bmp( snip_img );

    //  Map to another memoryDC
    wxMemoryDC result_dc;
    result_dc.SelectObject( emb_bmp );
    
    //  Blit to target
    pmdc->Blit( x, y, pemboss->width, pemboss->height, &result_dc, 0, 0 );
    
    result_dc.SelectObject( wxNullBitmap );
#endif
}

emboss_data *ChartCanvas::EmbossOverzoomIndicator( ocpnDC &dc )
{
#if 0
    double zoom_factor = GetVP().ref_scale / GetVP().chartScale();
    
    if( GetQuiltMode() ) {

        // disable Overzoom indicator for MBTiles
        int refIndex = GetQuiltRefChartdbIndex();
        if(refIndex >= 0){
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( refIndex );
            ChartTypeEnum current_type = (ChartTypeEnum) cte.GetChartType();
            if( current_type == CHART_TYPE_MBTILES){
                ChartBase *pChart = m_pQuilt->GetRefChart();
                ChartMBTiles *ptc = dynamic_cast<ChartMBTiles *>( pChart );
                if(ptc){
                    zoom_factor = ptc->GetZoomFactor();
                }
            }
        }
        
        if( zoom_factor <= 3.9 )
            return NULL;
    } else {
        if( m_singleChart ) {
            if( zoom_factor <= 3.9 )
                return NULL;
        }
        else
            return NULL;
    }

    if(m_pEM_OverZoom){
        m_pEM_OverZoom->x = 4;
        m_pEM_OverZoom->y = 0;
        if(g_MainToolbar && IsPrimaryCanvas()){
            wxRect masterToolbarRect = g_MainToolbar->GetRect();
            m_pEM_OverZoom->x = masterToolbarRect.width + 4;
        }
    }
#endif
    return m_pEM_OverZoom;
}


emboss_data *ChartCanvas::EmbossDepthScale()
{
    if( !m_bShowDepthUnits ) return NULL;

    int depth_unit_type = DEPTH_UNIT_UNKNOWN;

    if( GetQuiltMode() ) {
        QString s = m_pQuilt->GetQuiltDepthUnit();
        s.toUpper();
        if( s == ("FEET") ) depth_unit_type = DEPTH_UNIT_FEET;
        else if( s.startsWith( ("FATHOMS") ) ) depth_unit_type = DEPTH_UNIT_FATHOMS;
        else if( s.startsWith( ("METERS") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.startsWith( ("METRES") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.startsWith( ("METRIC") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.startsWith( ("METER") ) ) depth_unit_type = DEPTH_UNIT_METERS;

    } else {
        if( m_singleChart ) {
            depth_unit_type = m_singleChart->GetDepthUnitType();
            if( m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR ) depth_unit_type =
                    ps52plib->m_nDepthUnitDisplay + 1;
        }
    }

    emboss_data *ped = NULL;
    switch( depth_unit_type ) {
    case DEPTH_UNIT_FEET:
        ped = m_pEM_Feet;
        break;
    case DEPTH_UNIT_METERS:
        ped = m_pEM_Meters;
        break;
    case DEPTH_UNIT_FATHOMS:
        ped = m_pEM_Fathoms;
        break;
    default:
        return NULL;
    }

    ped->x = ( GetVP().pixWidth() - ped->width );

    if(m_Compass && m_bShowCompassWin){
        QRect r = m_Compass->GetRect();
        ped->y = r.y() + r.height();
    }
    else{
        ped->y = 40;
    }
    return ped;
}

void ChartCanvas::CreateDepthUnitEmbossMaps( ColorScheme cs )
{
    ocpnStyle::Style* style = StyleMgrIns->GetCurrentStyle();
    QFont font;
    if( style->embossFont.isEmpty() ){
        QFont dFont = FontMgr::Get().GetFont( ("Dialog"), 0 );
        font = dFont;
        font.setPointSize(60);
        font.setWeight(QFont::Weight::Bold);
    }
    else
    {
//        font = QFont( "Micorosoft YH", style->embossHeigh(), QFont::Weight::Bold);
    }


    int emboss_width = 500;
    int emboss_height = 200;

    // Free any existing emboss maps
    delete m_pEM_Feet;
    delete m_pEM_Meters;
    delete m_pEM_Fathoms;

    // Create the 3 DepthUnit emboss map structures
    m_pEM_Feet = CreateEmbossMapData( font, emboss_width, emboss_height, ("Feet"), cs );
    m_pEM_Meters = CreateEmbossMapData( font, emboss_width, emboss_height, ("Meters"), cs );
    m_pEM_Fathoms = CreateEmbossMapData( font, emboss_width, emboss_height, ("Fathoms"), cs );
}

#define OVERZOOM_TEXT ("OverZoom")

void ChartCanvas::SetOverzoomFont()
{
    ocpnStyle::Style* style = StyleMgrIns->GetCurrentStyle();
    int w, h;

    QFont font;
    if( style->embossFont.isEmpty() ){
        QFont dFont = FontMgr::Get().GetFont( ("Dialog"), 0 );
        font = dFont;
        font.setPointSize(40);
        font.setWeight(QFont::Weight::Bold);
    }
    else
    {
//        font = QFont( "Micorosoft YH", style->embossHeigh, QFont::Weight::Bold);
    }
    
//    wxClientDC dc( this );
//    dc.SetFont( font );
//    dc.GetTextExtent( OVERZOOM_TEXT, &w, &h );

//    while( font.GetPointSize() > 10 && (w > 500 || h > 100) )
//    {
//        font.SetPointSize( font.GetPointSize() - 1 );
//        dc.SetFont( font );
//        dc.GetTextExtent( OVERZOOM_TEXT, &w, &h );
//    }
    m_overzoomFont = font;
    m_overzoomTextWidth = w;
    m_overzoomTextHeight = h;
}

void ChartCanvas::CreateOZEmbossMapData( ColorScheme cs )
{
    delete m_pEM_OverZoom;

    if( m_overzoomTextWidth > 0 && m_overzoomTextHeight > 0 )
        m_pEM_OverZoom = CreateEmbossMapData( m_overzoomFont, m_overzoomTextWidth + 10, m_overzoomTextHeight + 10, OVERZOOM_TEXT, cs );
}

emboss_data *ChartCanvas::CreateEmbossMapData( QFont &font, int width, int height,
                                               const QString &str, ColorScheme cs )
{
#if 0
    int *pmap;

    //  Create a temporary bitmap
    wxBitmap bmp( width, height, -1 );

    // Create a memory DC
    wxMemoryDC temp_dc;
    temp_dc.SelectObject( bmp );

    //  Paint on it
    temp_dc.SetBackground( *wxWHITE_BRUSH );
    temp_dc.SetTextBackground( *wxWHITE );
    temp_dc.SetTextForeground( *wxBLACK );

    temp_dc.Clear();

    temp_dc.SetFont( font );

    int str_w, str_h;
    temp_dc.GetTextExtent( str, &str_w, &str_h );
    //    temp_dc.DrawText( str, width - str_w - 10, 10 );
    temp_dc.DrawText( str, 1, 1 );
    
    //  Deselect the bitmap
    temp_dc.SelectObject( wxNullBitmap );

    //  Convert bitmap the QImage for manipulation
    QImage img = bmp.ConvertToImage();

    int image_width = str_w * 105 / 100;
    int image_height = str_h * 105 / 100;
    wxRect r(0,0, fmin(image_width, img.width()), fmin(image_height, img.height()));
    QImage imgs = img.GetSubImage(r);
    
    double val_factor;
    switch( cs ) {
    case GLOBAL_COLOR_SCHEME_DAY:
    default:
        val_factor = 1;
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
        val_factor = .5;
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        val_factor = .25;
        break;
    }

    int val;
    int index;
    const int w = imgs.width();
    const int h = imgs.height();
    pmap = (int *) calloc( w *  h * sizeof(int), 1 );
    //  Create emboss map by differentiating the emboss image
    //  and storing integer results in pmap
    //  n.b. since the image is B/W, it is sufficient to check
    //  one channel (i.e. red) only
    for( int y = 1; y < h - 1; y++ ) {
        for( int x = 1; x < w - 1; x++ ) {
            val = img.GetRed( x + 1, y + 1 ) - img.GetRed( x - 1, y - 1 );  // range +/- 256
            val = (int) ( val * val_factor );
            index = ( y * w ) + x;
            pmap[index] = val;

        }
    }

    emboss_data *pret = new emboss_data;
    pret->pmap = pmap;
    pret->width = w;
    pret->height = h;

    return pret;
#endif
    return 0;
}

#define NUM_CURRENT_ARROW_POINTS 9
static zchxPoint CurrentArrowArray[NUM_CURRENT_ARROW_POINTS] = { zchxPoint( 0, 0 ), zchxPoint( 0, -10 ),
                                                               zchxPoint( 55, -10 ), zchxPoint( 55, -25 ), zchxPoint( 100, 0 ), zchxPoint( 55, 25 ), zchxPoint( 55,
                                                               10 ), zchxPoint( 0, 10 ), zchxPoint( 0, 0 )
                                                             };

void ChartCanvas::DrawArrow( ocpnDC& dc, int x, int y, double rot_angle, double scale )
{
    if( scale > 1e-2 ) {

        float sin_rot = sin( rot_angle * PI / 180. );
        float cos_rot = cos( rot_angle * PI / 180. );

        // Move to the first point

        float xt = CurrentArrowArray[0].x;
        float yt = CurrentArrowArray[0].y;

        float xp = ( xt * cos_rot ) - ( yt * sin_rot );
        float yp = ( xt * sin_rot ) + ( yt * cos_rot );
        int x1 = (int) ( xp * scale );
        int y1 = (int) ( yp * scale );

        // Walk thru the point list
        for( int ip = 1; ip < NUM_CURRENT_ARROW_POINTS; ip++ ) {
            xt = CurrentArrowArray[ip].x;
            yt = CurrentArrowArray[ip].y;

            float xp = ( xt * cos_rot ) - ( yt * sin_rot );
            float yp = ( xt * sin_rot ) + ( yt * cos_rot );
            int x2 = (int) ( xp * scale );
            int y2 = (int) ( yp * scale );

            dc.DrawLine( x1 + x, y1 + y, x2 + x, y2 + y );

            x1 = x2;
            y1 = y2;
        }
    }
}


void ChartCanvas::ToggleCanvasQuiltMode( void )
{
    bool cur_mode = GetQuiltMode();

    if( !GetQuiltMode() )
        SetQuiltMode( true );
    else
        if( GetQuiltMode() ) {
            SetQuiltMode( false );
            g_sticky_chart = GetQuiltReferenceChartIndex();
        }


    if( cur_mode != GetQuiltMode() ){
        SetupCanvasQuiltMode();
        DoCanvasUpdate();
        InvalidateGL();
        Refresh();
    }
    //  TODO What to do about this?
    //g_bQuiltEnable = GetQuiltMode();

    // Recycle the S52 PLIB so that vector charts will flush caches and re-render
    if(ps52plib)
        ps52plib->GenerateStateHash();

//    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
//        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();
}

void ChartCanvas::DoCanvasStackDelta( int direction )
{
    if( !GetQuiltMode() ) {
        int current_stack_index = GetpCurrentStack()->CurrentStackEntry;
        if( (current_stack_index + direction) >= GetpCurrentStack()->nEntry )
            return;
        if( (current_stack_index + direction) < 0 )
            return;
        
        if( m_bpersistent_quilt /*&& g_bQuiltEnable*/ ) {
            int new_dbIndex = GetpCurrentStack()->GetDBIndex(current_stack_index + direction );
            
            if( IsChartQuiltableRef( new_dbIndex ) ) {
                ToggleCanvasQuiltMode();
                SelectQuiltRefdbChart( new_dbIndex );
                m_bpersistent_quilt = false;
            }
        }
        else {
            SelectChartFromStack( current_stack_index + direction );
        }
    } else {
        std::vector<int>  piano_chart_index_array = GetQuiltExtendedStackdbIndexArray();
        int refdb = GetQuiltRefChartdbIndex();
        
        //      Find the ref chart in the stack
        int current_index = -1;
        for(unsigned int i=0 ; i < piano_chart_index_array.size() ; i++){
            if(refdb == piano_chart_index_array[i]){
                current_index = i;
                break;
            }
        }
        if(current_index == -1)
            return;
        
        const ChartTableEntry &ctet = ChartData->GetChartTableEntry( refdb );
        int target_family= ctet.GetChartFamily();
        
        int new_index = -1;
        int check_index = current_index + direction;
        bool found = false;
        int check_dbIndex = -1;
        int new_dbIndex = -1;
        
        //      When quilted. switch within the same chart family
        while(!found && (unsigned int)check_index < piano_chart_index_array.size() && (check_index >= 0)){
            check_dbIndex = piano_chart_index_array[check_index];
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( check_dbIndex );
            if(target_family == cte.GetChartFamily()){
                found = true;
                new_index = check_index;
                new_dbIndex = check_dbIndex;
                break;
            }
            
            check_index += direction;
        }
        
        if(!found)
            return;
        
        
        if( !IsChartQuiltableRef( new_dbIndex ) ) {
            ToggleCanvasQuiltMode();
            SelectdbChart( new_dbIndex );
            m_bpersistent_quilt = true;
        } else {
            SelectQuiltRefChart( new_index );
        }
    }
    
//    gFrame->UpdateGlobalMenuItems(); // update the state of the menu items (checkmarks etc)
    SetQuiltChartHiLiteIndex( -1 );
    
    ReloadVP();
}


//--------------------------------------------------------------------------------------------------------
//
//      Toolbar support
//
//--------------------------------------------------------------------------------------------------------

//void ChartCanvas::OnToolLeftClick( wxCommandEvent& event )
//{
//    //  Handle the per-canvas toolbar clicks here
    
//    switch( event.GetId() ){

//    case ID_ZOOMIN: {
//        ZoomCanvas( 2.0, false );
//        break;
//    }
        
//    case ID_ZOOMOUT: {
//        ZoomCanvas( 0.5, false );
//        break;
//    }
        
//    case ID_STKUP:
//        DoCanvasStackDelta( 1 );
//        DoCanvasUpdate();
//        break;

//    case ID_STKDN:
//        DoCanvasStackDelta( -1 );
//        DoCanvasUpdate();
//        break;

//    case ID_FOLLOW: {
//        TogglebFollow();
//        break;
//    }
        
//    case ID_CURRENT: {
//        ShowCurrents( !GetbShowCurrent() );
//        ReloadVP();
//        Refresh( false );
//        break;

//    }
        
//    case ID_TIDE: {
//        ShowTides( !GetbShowTide() );
//        ReloadVP();
//        Refresh( false );
//        break;

//    }
        
//    case ID_ROUTE: {
//        if( 0 == m_routeState ){
//            StartRoute();
//        }
//        else {
//            FinishRoute();
//        }

//#ifdef __OCPN__ANDROID__
//        androidSetRouteAnnunciator(m_routeState == 1);
//#endif
//        break;
//    }
        
//    case ID_AIS: {
//        SetAISCanvasDisplayStyle(-1);
//        break;
//    }
        
//    case ID_TBSTATBOX: {
//        ToggleCourseUp();
//        break;
//    }
        
//    default:
//        break;
//    }

//    //  And then let  gFrame handle the rest....
//    event.Skip();
//}




//      Update inplace the current toolbar with bitmaps corresponding to the current color scheme

extern bool    g_bAllowShowScaled;




//---------------------------------------------------------------------------------
//
//      Compass/GPS status icon support
//
//---------------------------------------------------------------------------------

void ChartCanvas::UpdateGPSCompassStatusBox( bool b_force_new )
{
    //    Look for change in overlap or positions
    bool b_update = false;
    int cc1_edge_comp = 2;
    QRect rect = m_Compass->GetRect();
    QSize parent_size = size();

    // check to see if it would overlap if it was in its home position (upper right)
    zchxPoint tentative_pt(parent_size.width() - rect.width() - cc1_edge_comp, StyleMgrIns->GetCurrentStyle()->GetCompassYOffset());
    QRect tentative_rect( tentative_pt.toPoint(), rect.size());
    
    // No toolbar, so just place compass in upper right.
    m_Compass->Move( tentative_pt );
    if( m_Compass && m_Compass->IsShown())
        m_Compass->UpdateStatus( b_force_new | b_update );
    
    if( b_force_new | b_update )
        Refresh();

}



void ChartCanvas::SelectChartFromStack( int index, bool bDir, ChartTypeEnum New_Type,
                                        ChartFamilyEnum New_Family )
{
    if( !GetpCurrentStack() ) return;
    if( !ChartData ) return;
    
    if( index < GetpCurrentStack()->nEntry ) {
        //      Open the new chart
        ChartBase *pTentative_Chart;
        pTentative_Chart = ChartData->OpenStackChartConditional( GetpCurrentStack(), index, bDir,
                                                                 New_Type, New_Family );
        
        if( pTentative_Chart ) {
            if( m_singleChart ) m_singleChart->Deactivate();
            
            m_singleChart = pTentative_Chart;
            m_singleChart->Activate();
            
            GetpCurrentStack()->CurrentStackEntry = ChartData->GetStackEntry( GetpCurrentStack(), m_singleChart->GetFullPath() );
        }
        //else
        //    SetChartThumbnail( -1 );   // need to reset thumbnail on failed chart open

        //      Setup the view
        double zLat, zLon;
        if( m_bFollow ) {
            zLat = gLat;
            zLon = gLon;
        } else {
            zLat = m_vLat;
            zLon = m_vLon;
        }
        
        double best_scale_ppm = GetBestVPScale( m_singleChart );
        double rotation = GetVPRotation();
        double oldskew = GetVPSkew();
        double newskew = m_singleChart->GetChartSkew() * PI / 180.0;
        
        if (!g_bskew_comp && !m_bCourseUp) {
            if (fabs(oldskew) > 0.0001)
                rotation = 0.0;
            if (fabs(newskew) > 0.0001)
                rotation = newskew;
        }
        
        SetViewPoint( zLat, zLon, best_scale_ppm, newskew, rotation );
        
        
        UpdateGPSCompassStatusBox( true );           // Pick up the rotation
        
    }
    
    //  refresh Piano
    int idx = GetpCurrentStack()->GetCurrentEntrydbIndex();
    if (idx < 0)
        return;
}

void ChartCanvas::SelectdbChart( int dbindex )
{
    if( !GetpCurrentStack() ) return;
    if( !ChartData ) return;
    
    if( dbindex >= 0 ) {
        //      Open the new chart
        ChartBase *pTentative_Chart;
        pTentative_Chart = ChartData->OpenChartFromDB( dbindex, FULL_INIT );
        
        if( pTentative_Chart ) {
            if( m_singleChart ) m_singleChart->Deactivate();
            
            m_singleChart = pTentative_Chart;
            m_singleChart->Activate();
            
            GetpCurrentStack()->CurrentStackEntry = ChartData->GetStackEntry( GetpCurrentStack(),  m_singleChart->GetFullPath() );
        }
        //else
        //    SetChartThumbnail( -1 );       // need to reset thumbnail on failed chart open

        //      Setup the view
        double zLat, zLon;
        if( m_bFollow ) {
            zLat = gLat;
            zLon = gLon;
        } else {
            zLat = m_vLat;
            zLon = m_vLon;
        }
        
        double best_scale_ppm = GetBestVPScale( m_singleChart );
        
        if( m_singleChart )
            SetViewPoint( zLat, zLon, best_scale_ppm, m_singleChart->GetChartSkew() * PI / 180., GetVPRotation() );

        //SetChartUpdatePeriod( );
        
        //UpdateGPSCompassStatusBox();           // Pick up the rotation
        
    }
    
    // TODO refresh_Piano();
}


void ChartCanvas::selectCanvasChartDisplay( int type, int family)
{
    double target_scale = GetVP().viewScalePPM();
    
    if( !GetQuiltMode() ) {
        if(GetpCurrentStack()){
            int stack_index = -1;
            for(int i = 0; i < GetpCurrentStack()->nEntry ; i++){
                int check_dbIndex = GetpCurrentStack()->GetDBIndex( i );
                if (check_dbIndex < 0)
                    continue;
                const ChartTableEntry &cte = ChartData->GetChartTableEntry( check_dbIndex );
                if(type == cte.GetChartType()){
                    stack_index = i;
                    break;
                }
                else if(family == cte.GetChartFamily()){
                    stack_index = i;
                    break;
                }
            }
            
            if(stack_index >= 0){
                SelectChartFromStack( stack_index );
            }
        }
    } else {
        int sel_dbIndex = -1;
        std::vector<int>  piano_chart_index_array = GetQuiltExtendedStackdbIndexArray();
        for(unsigned int i = 0; i < piano_chart_index_array.size() ; i++){
            int check_dbIndex = piano_chart_index_array[i];
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( check_dbIndex );
            if(type == cte.GetChartType()){
                if( IsChartQuiltableRef( check_dbIndex ) ) {
                    sel_dbIndex = check_dbIndex;
                    break;
                }
            }
            else if(family == cte.GetChartFamily()){
                if( IsChartQuiltableRef( check_dbIndex ) ) {
                    sel_dbIndex = check_dbIndex;
                    break;
                }
            }
        }
        
        if(sel_dbIndex >= 0){
            SelectQuiltRefdbChart( sel_dbIndex, false );  // no autoscale
            //  Re-qualify the quilt reference chart selection
            AdjustQuiltRefChart(  );
        }
        
        //  Now reset the scale to the target...
        SetVPScale(target_scale);
        
        
        
        
    }
    
    SetQuiltChartHiLiteIndex( -1 );
    
    ReloadVP();
}


void ChartCanvas::RemoveChartFromQuilt( int dbIndex )
{
    //    Remove the item from the list (if it appears) to avoid multiple addition
    for( unsigned int i = 0; i < g_quilt_noshow_index_array.size(); i++ ) {
        if( g_quilt_noshow_index_array[i] == dbIndex ) // chart is already in the noshow list
        {
            g_quilt_noshow_index_array.erase(g_quilt_noshow_index_array.begin() + i );
            break;
        }
    }
    
    g_quilt_noshow_index_array.push_back( dbIndex );
    
}



bool ChartCanvas::UpdateS52State()
{
    bool retval = false;
    //    printf("    update %d\n", IsPrimaryCanvas());
    
    if(ps52plib){
        ps52plib->SetShowS57Text( m_encShowText );
        ps52plib->SetDisplayCategory( (DisCat) m_encDisplayCategory );
        ps52plib->m_bShowSoundg = m_encShowDepth;
        ps52plib->m_bShowAtonText = m_encShowBuoyLabels;
        ps52plib->m_bShowLdisText = m_encShowLightDesc;
        
        // Lights
        if(!m_encShowLights)                     // On, going off
            ps52plib->AddObjNoshow("LIGHTS");
        else                                   // Off, going on
            ps52plib->RemoveObjNoshow("LIGHTS");
        ps52plib->SetLightsOff( !m_encShowLights );
        ps52plib->m_bExtendLightSectors = true;
        
        // TODO ps52plib->m_bShowAtons = m_encShowBuoys;
        ps52plib->SetAnchorOn( m_encShowAnchor );
        ps52plib->SetQualityOfData( m_encShowDataQual );
    }
    
    return retval;
}

void ChartCanvas::SetShowENCDataQual( bool show )
{
    m_encShowDataQual = show;
//    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
//        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}


void ChartCanvas::SetShowENCText( bool show )
{ 
    m_encShowText = show;
//    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
//        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

void ChartCanvas::SetENCDisplayCategory( int category )
{
    m_encDisplayCategory = category;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}


void ChartCanvas::SetShowENCDepth( bool show )
{
    m_encShowDepth = show;
//    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
//        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

void ChartCanvas::SetShowENCLightDesc( bool show )
{
    m_encShowLightDesc = show;
//    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
//        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

void ChartCanvas::SetShowENCBuoyLabels( bool show )
{
    m_encShowBuoyLabels = show;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

void ChartCanvas::SetShowENCLights( bool show )
{
    m_encShowLights = show;
//    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
//        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

void ChartCanvas::SetShowENCAnchor( bool show )
{
    m_encShowAnchor = show;
//    if( GetMUIBar() && GetMUIBar()->GetCanvasOptions())
//        GetMUIBar()->GetCanvasOptions()->RefreshControlValues();

    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

bool ChartCanvas::UpdateChartDatabaseInplace( ArrayOfCDI &DirArray, bool b_force, bool b_prog, const QString &ChartListFileName )
{

    bool b_run = false;
    // ..For each canvas...
    InvalidateQuilt();
    SetQuiltRefChart( -1 );
    m_singleChart = NULL;
    if(ChartData)   ChartData->PurgeCache();
    setCursor(OCPNPlatform::instance()->ShowBusySpinner());

    QProgressDialog *pprog = nullptr;
    if( b_prog  && DirArray.count() > 0) {
        pprog = new QProgressDialog(0);
        pprog->setRange(0, 100);
        pprog->setAttribute(Qt::WA_DeleteOnClose);
        pprog->setWindowTitle("OpenCPN Chart Update");
        pprog->setLabel(new QLabel(QString("%1\n%2").arg("OpenCPN Chart Update").arg("..........................................................................")));
        pprog->setCancelButtonText(tr("取消"));
        pprog->show();
    }
    qDebug("Starting chart database Update...");
    QString gshhg_chart_loc = gWorldMapLocation;
    gWorldMapLocation.clear();
    ChartData->Update( DirArray, b_force, pprog );
    ChartData->SaveBinary(ChartListFileName);
    qDebug("Finished chart database Update");
    if( gWorldMapLocation.isEmpty() ) { //Last resort. User might have deleted all GSHHG data, but we still might have the default dataset distributed with OpenCPN or from the package repository...
       gWorldMapLocation = gDefaultWorldMapLocation;
       gshhg_chart_loc.clear();
    }

    if( gWorldMapLocation != gshhg_chart_loc )
    {
        ResetWorldBackgroundChart();
    }


//    delete pprog;

    setCursor(OCPNPlatform::instance()->HideBusySpinner());
    ZCHX_CFG_INS->UpdateChartDirs( DirArray );
    if(pprog)
    {
        delete pprog;
    }

//    if( b_run ) FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

    return true;
}


//wxRect ChartCanvas::GetMUIBarRect()
//{
//    wxRect rv;
//    if(m_muiBar){
//        rv = m_muiBar->GetRect();
//    }
    
//    return rv;
//}


//--------------------------------------------------------------------------------------------------------
//    Screen Brightness Control Support Routines
//
//--------------------------------------------------------------------------------------------------------

#ifdef __UNIX__
#define BRIGHT_XCALIB
#define __OPCPN_USEICC__
#endif


#ifdef __OPCPN_USEICC__
int CreateSimpleICCProfileFile(const char *file_name, double co_red, double co_green, double co_blue);

QString temp_file_name;
#endif

#if 0
class ocpnCurtain: public wxDialog
{
    DECLARE_CLASS( ocpnCurtain )
    DECLARE_EVENT_TABLE()

    public:
        ocpnCurtain( wxWindow *parent, zchxPoint position, wxSize size, long wstyle );
    ~ocpnCurtain( );
    bool ProcessEvent(wxEvent& event);

};

IMPLEMENT_CLASS ( ocpnCurtain, wxDialog )

BEGIN_EVENT_TABLE(ocpnCurtain, wxDialog)
END_EVENT_TABLE()

ocpnCurtain::ocpnCurtain( wxWindow *parent, zchxPoint position, wxSize size, long wstyle )
{
    wxDialog::Create( parent, -1, _T("ocpnCurtain"), position, size, wxNO_BORDER | wxSTAY_ON_TOP );
}

ocpnCurtain::~ocpnCurtain()
{
}

bool ocpnCurtain::ProcessEvent(wxEvent& event)
{
    GetParent()->GetEventHandler()->SetEvtHandlerEnabled(true);
    return GetParent()->GetEventHandler()->ProcessEvent(event);
}
#endif

#ifdef _WIN32
#include <windows.h>

HMODULE hGDI32DLL;
typedef BOOL (WINAPI *SetDeviceGammaRamp_ptr_type)( HDC hDC, LPVOID lpRampTable );
typedef BOOL (WINAPI *GetDeviceGammaRamp_ptr_type)( HDC hDC, LPVOID lpRampTable );
SetDeviceGammaRamp_ptr_type g_pSetDeviceGammaRamp;            // the API entry points in the dll
GetDeviceGammaRamp_ptr_type g_pGetDeviceGammaRamp;

WORD *g_pSavedGammaMap;

#endif

#ifdef __OPCPN_USEICC__

#define MLUT_TAG     0x6d4c5554L
#define VCGT_TAG     0x76636774L

int GetIntEndian(unsigned char *s)
{
    int ret;
    unsigned char *p;
    int i;

    p = (unsigned char *)&ret;

    if(1)
        for(i=sizeof(int)-1; i>-1; --i)
            *p++ = s[i];
    else
        for(i=0; i<(int)sizeof(int); ++i)
            *p++ = s[i];

    return ret;
}

unsigned short GetShortEndian(unsigned char *s)
{
    unsigned short ret;
    unsigned char *p;
    int i;

    p = (unsigned char *)&ret;

    if(1)
        for(i=sizeof(unsigned short)-1; i>-1; --i)
            *p++ = s[i];
    else
        for(i=0; i<(int)sizeof(unsigned short); ++i)
            *p++ = s[i];

    return ret;
}

//    Create a very simple Gamma correction file readable by xcalib
int CreateSimpleICCProfileFile(const char *file_name, double co_red, double co_green, double co_blue)
{
    FILE *fp;

    if(file_name)
    {
        fp = fopen(file_name, "wb");
        if(!fp)
            return -1; /* file can not be created */
    }
    else
        return -1; /* filename char pointer not valid */

    //    Write header
    char header[128];
    for(int i=0; i< 128; i++)
        header[i] = 0;

    fwrite(header, 128, 1, fp);

    //    Num tags
    int numTags0 = 1;
    int numTags = GetIntEndian((unsigned char *)&numTags0);
    fwrite(&numTags, 1, 4, fp);

    int tagName0 = VCGT_TAG;
    int tagName = GetIntEndian((unsigned char *)&tagName0);
    fwrite(&tagName, 1, 4, fp);

    int tagOffset0 = 128 + 4 * sizeof(int);
    int tagOffset = GetIntEndian((unsigned char *)&tagOffset0);
    fwrite(&tagOffset, 1, 4, fp);

    int tagSize0 = 1;
    int tagSize = GetIntEndian((unsigned char *)&tagSize0);
    fwrite(&tagSize, 1, 4, fp);

    fwrite(&tagName, 1, 4, fp);// another copy of tag

    fwrite(&tagName, 1, 4, fp);// dummy

    //  Table type

    /* VideoCardGammaTable (The simplest type) */
    int gammatype0 = 0;
    int gammatype = GetIntEndian((unsigned char *)&gammatype0);
    fwrite(&gammatype, 1, 4, fp);

    int numChannels0 = 3;
    unsigned short numChannels = GetShortEndian((unsigned char *)&numChannels0);
    fwrite(&numChannels, 1, 2, fp);

    int numEntries0 = 256;
    unsigned short numEntries = GetShortEndian((unsigned char *)&numEntries0);
    fwrite(&numEntries, 1, 2, fp);

    int entrySize0 = 1;
    unsigned short entrySize = GetShortEndian((unsigned char *)&entrySize0);
    fwrite(&entrySize, 1, 2, fp);

    unsigned char ramp[256];

    //    Red ramp
    for(int i=0; i< 256; i++)
        ramp[i] = i * co_red/100.;
    fwrite(ramp, 256, 1, fp);

    //    Green ramp
    for(int i=0; i< 256; i++)
        ramp[i] = i * co_green/100.;
    fwrite(ramp, 256, 1, fp);

    //    Blue ramp
    for(int i=0; i< 256; i++)
        ramp[i] = i * co_blue/100.;
    fwrite(ramp, 256, 1, fp);

    fclose(fp);

    return 0;
}
#endif // __OPCPN_USEICC__
