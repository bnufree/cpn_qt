﻿#include "zchxmapmainwindow.h"
#include "ui_zchxmapmainwindow.h"
#include <windows.h>
#include <psapi.h>
#include <QDebug>
#include <QTimer>
#include "_def.h"
#include "zchxoptionsdlg.h"
#include "zchxconfig.h"
#include "chartdb.h"
#include "chcanv.h"
#include "s52plib.h"
#include "OCPNPlatform.h"
#include "S57ClassRegistrar.h"
#include "s57RegistrarMgr.h"
#include "glChartCanvas.h"
#include "SencManager.h"
//#include "thumbwin.h"
#include "styles.h"
#include <QVBoxLayout>
#include <QThread>
#include "config.h"
#include <QProgressDialog>
#include <QLabel>
#include <QMessageBox>
#include "CanvasConfig.h"
#include "compass.h"
#include "glwidget.h"


//extern      ChartDB                     *ChartData;
extern      QThread                     *g_Main_thread;
//arrayofCanvasPtr            g_canvasArray;
extern      float                       g_compass_scalefactor;
extern      int                         g_GUIScaleFactor;
extern      ColorScheme                 global_color_scheme;


static QList<colTable*>       *UserColorTableArray = 0;
static QList<QColorHashMap*>       *UserColourHashTableArray = 0;
static QColorHashMap        *pcurrent_user_color_hash;
SENCThreadManager           *g_SencThreadManager = 0;
extern s52plib                   *ps52plib;
extern QString                  g_csv_locn;

extern QString                  g_SENCPrefix;
extern QString                  g_UserPresLibData;
extern S57ClassRegistrar         *g_poRegistrar;
s57RegistrarMgr           *m_pRegistrarMan = 0;
extern zchxMapMainWindow*  gFrame;
extern int                       g_nDepthUnitDisplay;

extern int                       g_nCacheLimit;
//extern ThumbWin                  *pthumbwin;
extern bool                      g_fog_overzoom;
extern double                    g_overzoom_emphasis_base;
extern bool                      g_oz_vector_scale;

extern QString                  ChartListFileName;
extern QString                  AISTargetNameFileName;
extern bool            g_bShowStatusBar;
extern bool            g_bShowMenuBar;
extern bool            g_bShowCompassWin;
extern bool            g_bShowChartBar;
extern double          g_display_size_mm;
extern double          g_config_display_size_mm;
extern bool            g_config_display_size_manual;
extern bool            g_bskew_comp;
extern bool            g_bresponsive;
extern bool            g_bAutoHideToolbar;
extern int             g_nAutoHideToolbar;
extern bool            g_bsmoothpanzoom;
extern bool            g_bShowTrue;
extern bool            g_bShowMag;
extern int             g_iSDMMFormat;
extern int             g_iDistanceFormat;
extern int             g_iSpeedFormat;
extern bool            g_bEnableZoomToCursor;
extern int             g_chart_zoom_modifier;
extern int             g_chart_zoom_modifier_vector;


extern int             g_ChartScaleFactor;
extern int             g_ShipScaleFactor;
extern float           g_ChartScaleFactorExp;
extern float           g_ShipScaleFactorExp;
extern int             g_cm93_zoom_factor;
extern bool                      g_bInlandEcdis;
bool                      g_b_assume_azerty;
extern bool                      g_benable_rotate;
extern bool                      g_b_overzoom_x; // Allow high overzoom
//ChartDummy                *pDummyChart = 0;
int               g_sticky_chart;
extern double                    gLat, gLon, gCog, gHdt, gHdm;
double gSog, gVar;
extern double                    g_UserVar;
extern double                    vLat, vLon;
extern double                    initial_scale_ppm, initial_rotation;
extern bool                      g_bDebugS57;
extern bool                      g_bGDAL_Debug;
extern double                    g_VPRotate; // Viewport rotation angle, used on "Course Up" mode
extern bool                      g_bCourseUp;
extern int                       g_COGAvgSec; // COG average period (sec.) for Course Up Mode
extern double                    g_COGAvg;
extern bool                      g_bLookAhead;
extern bool                      g_bFirstRun;
extern bool                      g_bUpgradeInProcess;
extern float                     g_selection_radius_mm;
extern float                     g_selection_radius_touch_mm;

extern int                       g_maintoolbar_x;
extern int                       g_maintoolbar_y;
extern long                      g_maintoolbar_orient;
extern float                     g_toolbar_scalefactor;

extern int                     g_nCPUCount;
extern bool                        g_bSoftwareGL;
extern bool                        g_bGLexpert;
//extern ChartCanvas      *g_focusCanvas;
//extern ChartCanvas      *g_overlayCanvas;
bool             b_inCompressAllCharts;
extern unsigned int     g_canvasConfig;
extern bool                      g_bcompression_wait;
extern QString                  g_locale;
extern QString                  g_localeOverride;
extern bool             g_btouch;
extern ChartGroupArray           *g_pGroupArray;
extern int                       g_GroupIndex;
extern bool                      g_bNeedDBUpdate;
extern bool                      g_bPreserveScaleOnX;
extern bool                      g_bFullscreen;
extern bool                      g_bFullScreenQuilt;
extern bool                      g_bQuiltEnable;
extern bool                      g_bQuiltStart;
extern bool                      g_bquiting;
extern double                    g_ChartNotRenderScaleFactor;
std::vector<int>               g_quilt_noshow_index_array;
extern int                       g_nbrightness;
extern bool                      bDBUpdateInProgress;
extern bool                      bGPSValid;
extern int                       g_SatsInView;
extern bool                      g_bSatValid;
extern bool                        g_bSpaceDropMark;

//extern ChartGroupArray            *g_pGroupArray;
bool                                g_bNeedDBUpdate;
extern bool                         g_useMUI;
extern bool                         g_bUIexpert;



zchxMapMainWindow::zchxMapMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::zchxMapMainWindow)
    , mEcdisWidget(0)
{

    ui->setupUi(this);
    setMouseTracking(true);
    //工具
    QMenu* tools = this->menuBar()->addMenu(tr("Tools"));
    addCustomAction(tools,tr("Options"),this, SLOT(slotOpenSettingDlg()));
    addCustomAction(tools, tr("Measure distance"), this, SLOT(slotMeasureDistance()));
    addCustomAction(tools, tr("Rotate"), this, SLOT(slotRotate()));

    //导航
    QMenu* navigation = this->menuBar()->addMenu(tr("Navigation"));
    addCustomAction(navigation, tr("North Up"), this, SLOT(slotNorthUp()));
    addCustomAction(navigation, tr("Course Up"), this, SLOT(slotAnyAngleUp()));
    addCustomAction(navigation, tr("Look Ahead Mode"), this, SLOT(slotLookAheadMode(bool)), true);
    addCustomAction(navigation, tr("Zoom In"), this, SLOT(slotZoomIn()));
    addCustomAction(navigation, tr("Zoom Out"), this, SLOT(slotZoomOut()));
    addCustomAction(navigation, tr("Large Scale Chart"), this, SLOT(slotLargeScaleChart()));
    addCustomAction(navigation, tr("Small Scale Chart"), this, SLOT(slotSmallScaleChart()));
    //视图
    QMenu* view = this->menuBar()->addMenu(tr("View"));
    addCustomAction(view, tr("Enable Chart Quilting"), this, SLOT(slotEnableChartQuilting(bool)));
    addCustomAction(view, tr("Show Chart Quilting"), this, SLOT(slotShowChartQuilting(bool)));
    addCustomAction(view, tr("Show ENC Text"), this, SLOT(slotShowENCText(bool)));
    addCustomAction(view, tr("Show ENC Lights"), this, SLOT(slotShowENCLights(bool)));
    addCustomAction(view, tr("Show ENC Soundings"), this, SLOT(slotShowENCSoundings(bool)));
    addCustomAction(view, tr("Show ENC Anchoring Info"), this, SLOT(slotShowENCAnchoringInfo(bool)));
    addCustomAction(view, tr("Show ENC Data Quality"), this, SLOT(slotShowENCDataQuality(bool)));
    addCustomAction(view, tr("Show Nav Objects"), this, SLOT(slotShowNavObjects(bool)));
    //颜色模式
    QMenu* color = view->addMenu(tr("Change Color Schem"));
    addCustomAction(color, tr("Day"), this, SLOT(slotChangeColorScheme()), false, ColorScheme::GLOBAL_COLOR_SCHEME_DAY);
    addCustomAction(color, tr("Dusk"), this, SLOT(slotChangeColorScheme()), false, ColorScheme::GLOBAL_COLOR_SCHEME_DUSK);
    addCustomAction(color, tr("Night"), this, SLOT(slotChangeColorScheme()), false, ColorScheme::GLOBAL_COLOR_SCHEME_NIGHT);

    addCustomAction(view, tr("Show Depth Unit"), this, SLOT(slotShowDepthUnit(bool)));
    addCustomAction(view, tr("Show Grid"), this, SLOT(slotShowGrid(bool)));
    addCustomAction(view, tr("Show Depth"), this, SLOT(slotShowDepth(bool)));
    addCustomAction(view, tr("Show Buoy Light Label"), this, SLOT(slotShowBuoyLightLabel(bool)));
    addCustomAction(view, tr("Show Light Discriptions"), this, SLOT(slotShowLightDiscriptions(bool)));
    //显示模式
    QMenu* display = view->addMenu(tr("Show Display Category"));
    addCustomAction(display, tr("Base"), this, SLOT(slotShowDisplayCategory()), false, ColorScheme::GLOBAL_COLOR_SCHEME_DAY);
    addCustomAction(display, tr("Standard"), this, SLOT(slotShowDisplayCategory()), false, ColorScheme::GLOBAL_COLOR_SCHEME_DUSK);
    addCustomAction(display, tr("All"), this, SLOT(slotShowDisplayCategory()), false, ColorScheme::GLOBAL_COLOR_SCHEME_NIGHT);

    //添加窗口
    mEcdisWidget = new ChartCanvas(this, 0);
    if(!ui->centralwidget->layout())
    {
        ui->centralwidget->setLayout(new QVBoxLayout(ui->centralwidget));
    }
    ui->centralwidget->layout()->addWidget(mEcdisWidget);

    g_Main_thread = QThread::currentThread();
    gFrame = this;

    //添加窗口时的初始化
//    initBeforeCreateCanvas();
    //添加窗口
//    CreateCanvasLayout();
    //窗口添加完成延时加载地图数据
//    QTimer::singleShot(1000, this, SLOT(slotInitEcidsAsDelayed()));
}

zchxMapMainWindow::~zchxMapMainWindow()
{
    ZCHX_CFG_INS->UpdateSettings();
    delete ui;
}

//void zchxMapMainWindow::initBeforeCreateCanvas()
//{
//    gFrame = this;
//    g_Main_thread = QThread::currentThread();
//    g_Platform = new OCPNPlatform;
//    this->setWindowTitle("ZCHX Ecdis");
//    pInit_Chart_Dir = new QString();
//    g_pGroupArray = new ChartGroupArray;
//    ZCHX_CFG_INS->loadMyConfig();
//    g_Platform->applyExpertMode(g_bUIexpert);
//    g_StyleManager = new ocpnStyle::StyleManager();
//    g_StyleManager->SetStyle("MUI_flat");
//    if( !g_StyleManager->IsOK() ) {
//        QString logFile = QApplication::applicationDirPath() + QString("/log/opencpn.log");
//        QString msg = ("Failed to initialize the user interface. ");
//        msg.append("OpenCPN cannot start. ");
//        msg.append("The necessary configuration files were not found. ");
//        msg.append("See the log file at ").append(logFile).append(" for details.").append("\n\n");
//        QMessageBox::warning(0, "Failed to initialize the user interface. ", msg);
//        exit( EXIT_FAILURE );
//    }
//    if(g_useMUI){
//        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
//        style->chartStatusWindowTransparent = true;
//    }

//    g_display_size_mm = fmax(100.0, g_Platform->GetDisplaySizeMM());
//    qDebug("Detected display size (horizontal): %d mm", (int) g_display_size_mm);
//    // User override....
//    if((g_config_display_size_mm > 0) &&(g_config_display_size_manual)){
//        g_display_size_mm = g_config_display_size_mm;
//        qDebug("Display size (horizontal) config override: %d mm", (int) g_display_size_mm);
//        g_Platform->SetDisplaySizeMM(g_display_size_mm);
//    }

//    // Instantiate and initialize the Config Manager
////    ConfigMgr::Get();

//    //  Validate OpenGL functionality, if selected
//    g_bdisable_opengl = false;
//    if(g_bdisable_opengl) g_bopengl = false;
//    zchxFuncUtil::getMemoryStatus(&g_mem_total, &g_mem_initial);
//    if( 0 == g_memCacheLimit ) g_memCacheLimit = (int) ( g_mem_total * 0.5 );
//    g_memCacheLimit = fmin(g_memCacheLimit, 1024 * 1024); // math in kBytes, Max is 1 GB

////      Establish location and name of chart database
//    ChartListFileName = ChartListFileName = QString("%1/CHRTLIST.DAT").arg(zchxFuncUtil::getDataDir());
////      Establish guessed location of chart tree
//    if( pInit_Chart_Dir->isEmpty() )
//    {
//        pInit_Chart_Dir->append(zchxFuncUtil::getDataDir());
//    }

////      Establish the GSHHS Dataset location
//    gDefaultWorldMapLocation = "gshhs";
//    gDefaultWorldMapLocation.insert(0, QString("%1/").arg(zchxFuncUtil::getDataDir()));
////    gDefaultWorldMapLocation.Append( wxFileName::GetPathSeparator() );
//    if( gWorldMapLocation.isEmpty() || !(QDir(gWorldMapLocation).exists()) ) {
//        gWorldMapLocation = gDefaultWorldMapLocation;
//    }
//    qDebug()<<gWorldMapLocation<<gDefaultWorldMapLocation;
//    g_Platform->Initialize_2();
//    InitializeUserColors();
//    //  Do those platform specific initialization things that need gFrame
//    g_Platform->Initialize_3();
//}

//void zchxMapMainWindow::CreateCanvasLayout()
//{

//    mEcdisWidget = new ChartCanvas(this, 0);
//    if(!ui->centralwidget->layout())
//    {
//        ui->centralwidget->setLayout(new QVBoxLayout(ui->centralwidget));
//    }
//    ui->centralwidget->layout()->addWidget(mEcdisWidget);
//    return;

//    //  Clear the cache, and thus close all charts to avoid memory leaks
//    if(ChartData) ChartData->PurgeCache();

//    //更新视窗的配置
//    canvasConfig *config = new canvasConfig();
//    config->LoadFromLegacyConfig(ZCHX_CFG_INS);
//    config->canvas = mEcdisWidget;

//    // Verify that glCanvas is ready, if necessary
//    if(g_bopengl){
//        if(!mEcdisWidget->GetglCanvas())
//            mEcdisWidget->SetupGlCanvas();
//    }
//    config->iLat = 37.123456;
//    config->iLon = 127.123456;

//    mEcdisWidget->SetDisplaySizeMM(g_display_size_mm);

//    mEcdisWidget->ApplyCanvasConfig(config);

//    //            cc->SetToolbarPosition(wxPoint( g_maintoolbar_x, g_maintoolbar_y ));
//    mEcdisWidget->ConfigureChartBar();
//    mEcdisWidget->SetColorScheme( global_color_scheme );
//    mEcdisWidget->GetCompass()->SetScaleFactor(g_compass_scalefactor);
//    mEcdisWidget->SetShowGPS( true );
//}

void zchxMapMainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);

}


//void zchxMapMainWindow::slotInitEcidsAsDelayed()
//{
//    //   Build the initial chart dir array
//    ArrayOfCDI ChartDirArray;
//    ZCHX_CFG_INS->LoadChartDirArray( ChartDirArray );

//    if( !ChartDirArray.count() )
//    {
//        if(QFile::exists(ChartListFileName )) QFile::remove(ChartListFileName );
//    }

//    if(!ChartData)  ChartData = new ChartDB( );
//    if (!ChartData->LoadBinary(ChartListFileName, ChartDirArray))
//    {
//        g_bNeedDBUpdate = true;
//    }
//    //  Verify any saved chart database startup index
//    if(g_restore_dbindex >= 0)
//    {
//        if(ChartData->GetChartTableEntries() == 0)
//        {
//            g_restore_dbindex = -1;
//        } else if(g_restore_dbindex > (ChartData->GetChartTableEntries()-1))
//        {
//            g_restore_dbindex = 0;
//        }
//    }

//    //  Apply the inital Group Array structure to the chart data base
//    ChartData->ApplyGroupArray( g_pGroupArray );
//    DoChartUpdate();
//    mEcdisWidget->ReloadVP();                  // once more, and good to go
//    OCPNPlatform::Initialize_4( );
//    mEcdisWidget->GetglCanvas()->setUpdateAvailable(true);
//    mEcdisWidget->startUpdate();
//}

void zchxMapMainWindow::slotOpenSettingDlg()
{
    qDebug()<<"open settings windows now";
    zchxOptionsDlg* dlg = new zchxOptionsDlg(this);
    dlg->show();
}

void zchxMapMainWindow::slotMeasureDistance()
{

}

void zchxMapMainWindow::slotNorthUp()
{

}

void zchxMapMainWindow::slotAnyAngleUp()
{

}

void zchxMapMainWindow::slotLookAheadMode(bool sts)
{

}

void zchxMapMainWindow::slotZoomIn()
{
     if(mEcdisWidget) mEcdisWidget->ZoomCanvas( 2.0, false );
}

void zchxMapMainWindow::slotZoomOut()
{
     if(mEcdisWidget) mEcdisWidget->ZoomCanvas( 0.5, false );
}

void zchxMapMainWindow::slotLargeScaleChart()
{

}

void zchxMapMainWindow::slotSmallScaleChart()
{

}

void zchxMapMainWindow::slotEnableChartQuilting(bool sts)
{
    if(mEcdisWidget &&  mEcdisWidget->GetQuiltMode() != sts)
    {
        mEcdisWidget->ToggleCanvasQuiltMode();
    }
}

void zchxMapMainWindow::slotShowENCAnchoringInfo(bool sts)
{

}

void zchxMapMainWindow::slotShowBuoyLightLabel(bool sts)
{

}

void zchxMapMainWindow::slotShowChartQuilting(bool sts)
{
    if(!mEcdisWidget) return;
    if(mEcdisWidget->GetShowOutlines() == sts) return;
    mEcdisWidget->SetShowOutlines(sts);
    mEcdisWidget->Refresh(false);
    mEcdisWidget->InvalidateGL();
}

void zchxMapMainWindow::slotShowDepth(bool sts)
{
    slotShowENCSoundings(sts);
}

void zchxMapMainWindow::slotShowDepthUnit(bool sts)
{

}

void zchxMapMainWindow::slotShowDisplayCategory()
{

}

void zchxMapMainWindow::slotShowENCDataQuality(bool sts)
{

}

void zchxMapMainWindow::slotShowENCLights(bool sts)
{

}

void zchxMapMainWindow::slotShowENCSoundings(bool sts)
{
    if(mEcdisWidget)
    {
        if(mEcdisWidget->GetShowENCDepth() != sts)
        {
            mEcdisWidget->SetShowENCDepth(sts);
            mEcdisWidget->ReloadVP();
        }
    }
}

void zchxMapMainWindow::slotShowENCText(bool sts)
{
    if(!mEcdisWidget) return;
    if(mEcdisWidget->GetShowENCText() == sts) return;
    mEcdisWidget->SetShowENCText(sts);
    mEcdisWidget->ReloadVP();
}

void zchxMapMainWindow::slotShowGrid(bool sts)
{
    if(!mEcdisWidget) return;
    if(mEcdisWidget->GetShowGrid() == sts) return;
    mEcdisWidget->SetShowGrid(sts);
//    mEcdisWidget->ReloadVP();
}

void zchxMapMainWindow::slotShowLightDiscriptions(bool sts)
{

}

void zchxMapMainWindow::slotShowNavObjects(bool sts)
{

}

void zchxMapMainWindow::slotChangeColorScheme()
{

}

QAction* zchxMapMainWindow::addCustomAction(QMenu *menu, const QString &text, const QObject *receiver, const char *slot,  bool check, const QVariant &data)
{
    if(!menu || !slot || !receiver || strlen(slot) == 0) return 0;
    if(mActionMap.contains(text)) return 0;

    QAction *result = menu->addAction(text);
    if(QString(slot).contains("(bool)"))
    {
        connect(result, SIGNAL(toggled(bool)), receiver, slot);
        result->setCheckable(true);
        result->setChecked(check);
    } else
    {
        connect(result,SIGNAL(triggered()), receiver, slot);
        result->setCheckable(false);
    }

    result->setData(data);
    mActionMap[text] = result;
    return result;
}

void zchxMapMainWindow::setActionCheckSts(const QString &action, bool check)
{
    if(!mActionMap.contains(action)) return;
    if(!mActionMap[action]->isCheckable()) return;
    mActionMap[action]->setChecked(check);
}

void zchxMapMainWindow::setActionEnableSts(const QString &action, bool check)
{
    if(!mActionMap.contains(action)) return;
    mActionMap[action]->setEnabled(check);
}

bool zchxMapMainWindow::ProcessOptionsDialog( int rr, ArrayOfCDI *pNewDirArray )
{
    bool b_need_refresh = false;                // Do we need a full reload?

    if( ( rr & VISIT_CHARTS )
            && ( ( rr & CHANGE_CHARTS ) || ( rr & FORCE_UPDATE ) || ( rr & SCAN_UPDATE ) ) ) {
        if(mEcdisWidget){
            mEcdisWidget->UpdateChartDatabaseInplace( *pNewDirArray, ( ( rr & FORCE_UPDATE ) == FORCE_UPDATE ),  true, ChartListFileName );

            b_need_refresh = true;
        }
    }
    ZCHX_CFG_INS->UpdateSettings();
    if(mEcdisWidget) mEcdisWidget->canvasRefreshGroupIndex();
//    bool ztc = g_bEnableZoomToCursor;     // record the present state
//    g_bEnableZoomToCursor = false;        // since we don't want to pan to an unknown cursor position

//    //  This is needed to recognise changes in zoom-scale factors
//    if(mEcdisWidget) mEcdisWidget->DoZoomCanvas(1.0001);

//    g_bEnableZoomToCursor = ztc;

#if 0
    bool b_groupchange = false;
    if( ( ( rr & VISIT_CHARTS )
            && ( ( rr & CHANGE_CHARTS ) || ( rr & FORCE_UPDATE ) || ( rr & SCAN_UPDATE ) ) )
            || ( rr & GROUPS_CHANGED ) ) {
        b_groupchange = ScrubGroupArray();
        ChartData->ApplyGroupArray( g_pGroupArray );
        RefreshGroupIndices( );
    }

    if( rr & GROUPS_CHANGED || b_groupchange) {
        pConfig->DestroyConfigGroups();
        pConfig->CreateConfigGroups( g_pGroupArray );
    }

    if( rr & TIDES_CHANGED ) {
        LoadHarmonics();
    }

    //  S52_CHANGED is a byproduct of a change in the chart object render scale
    //  So, applies to RoutePoint icons also
    if( rr & S52_CHANGED){
        //  Reload Icons
        pWayPointMan->ReloadAllIcons( );
    }

    pConfig->UpdateSettings();

    if( g_pActiveTrack ) {
        g_pActiveTrack->SetPrecision( g_nTrackPrecision );
    }

//     if( ( bPrevQuilt != g_bQuiltEnable ) || ( bPrevFullScreenQuilt != g_bFullScreenQuilt ) ) {
//         GetPrimaryCanvas()->SetQuiltMode( g_bQuiltEnable );
//         GetPrimaryCanvas()->SetupCanvasQuiltMode();
//     }

#if 0
//TODO Not need with per-canvas CourseUp
    if( g_bCourseUp ) {
        //    Stuff the COGAvg table in case COGUp is selected
        double stuff = NAN;
        if( !std::isnan(gCog) ) stuff = gCog;
        if( g_COGAvgSec > 0 ) {
            for( int i = 0; i < g_COGAvgSec; i++ )
                COGTable[i] = stuff;
        }

        g_COGAvg = stuff;

        DoCOGSet();
    }
#endif

    g_pRouteMan->SetColorScheme(global_color_scheme);           // reloads pens and brushes

    //    Stuff the Filter tables
    double stuffcog = NAN;
    double stuffsog = NAN;
    if( !std::isnan(gCog) ) stuffcog = gCog;
    if( !std::isnan(gSog) ) stuffsog = gSog;

    for( int i = 0; i < MAX_COGSOG_FILTER_SECONDS; i++ ) {
        COGFilterTable[i] = stuffcog;
        SOGFilterTable[i] = stuffsog;
    }

    SetChartUpdatePeriod( );              // Pick up changes to skew compensator

    if(rr & GL_CHANGED){
        //    Refresh the chart display, after flushing cache.
        //      This will allow all charts to recognise new OpenGL configuration, if any
        b_need_refresh = true;
    }

    if(rr & S52_CHANGED){
        b_need_refresh = true;
    }

#ifdef ocpnUSE_GL
    if(rr & REBUILD_RASTER_CACHE){
        if(g_glTextureManager) {
            GetPrimaryCanvas()->setEnabled(false);
            g_glTextureManager->BuildCompressedCache();
            GetPrimaryCanvas()->setEnabled(true);
        }
    }
#endif

    if(g_config_display_size_mm > 0){
        g_display_size_mm = g_config_display_size_mm;
    }
    else{
        g_display_size_mm = wxMax(100, g_Platform->GetDisplaySizeMM());
    }

    for(unsigned int i=0 ; i < g_canvasArray.GetCount() ; i++){
        ChartCanvas *cc = g_canvasArray.Item(i);
        if(cc)
            cc->SetDisplaySizeMM( g_display_size_mm );
    }

    if(g_pi_manager){
        g_pi_manager->SendBaseConfigToAllPlugIns();
        int rrt = rr & S52_CHANGED;
        g_pi_manager->SendS52ConfigToAllPlugIns( rrt == S52_CHANGED);
    }


    if(g_MainToolbar){
        g_MainToolbar->SetAutoHide(g_bAutoHideToolbar);
        g_MainToolbar->SetAutoHideTimer(g_nAutoHideToolbar);
    }

    // Apply any needed updates to each canvas
    for(unsigned int i=0 ; i < g_canvasArray.GetCount() ; i++){
        ChartCanvas *cc = g_canvasArray.Item(i);
        if(cc)
            cc->ApplyGlobalSettings();
    }


    //    Do a full Refresh, trying to open the last open chart
//TODO  This got move up a level.  FIX ANDROID codepath
#if 0
    if(b_need_refresh){
        int index_hint = ChartData->FinddbIndex( chart_file_name );
        if( -1 == index_hint )
            b_autofind = true;
        ChartsRefresh( );
    }
#endif

    //  The zoom-scale factor may have changed
    //  so, trigger a recalculation of the reference chart

    bool ztc = g_bEnableZoomToCursor;     // record the present state
    g_bEnableZoomToCursor = false;        // since we don't want to pan to an unknown cursor position

    //  This is needed to recognise changes in zoom-scale factors
    GetPrimaryCanvas()->DoZoomCanvas(1.0001);

    g_bEnableZoomToCursor = ztc;


    return b_need_refresh;
#endif
}



void zchxMapMainWindow::slotOnFrameTimer1Out()
{
#if 0
    CheckToolbarPosition();

        if( ! g_bPauseTest && (g_unit_test_1 || g_unit_test_2) ) {
    //            if((0 == ut_index) && GetQuiltMode())
    //                  ToggleQuiltMode();

            // We use only one canvas for the unit tests, so far...
            ChartCanvas *cc = GetPrimaryCanvas();

            cc->m_bFollow = false;
            if( g_MainToolbar && g_MainToolbar->GetToolbar() )
                g_MainToolbar->GetToolbar()->ToggleTool( ID_FOLLOW, cc->m_bFollow );
            int ut_index_max = ( ( g_unit_test_1 > 0 ) ? ( g_unit_test_1 - 1 ) : INT_MAX );

            if( ChartData ) {
                if( g_GroupIndex > 0 ) {
                    while (ut_index < ChartData->GetChartTableEntries() && !ChartData->IsChartInGroup( ut_index, g_GroupIndex ) ) {
                        ut_index++;
                    }
                }
                if( ut_index < ChartData->GetChartTableEntries() ) {
                    // printf("%d / %d\n", ut_index, ChartData->GetChartTableEntries());
                    const ChartTableEntry *cte = &ChartData->GetChartTableEntry( ut_index );

                    double clat = ( cte->GetLatMax() + cte->GetLatMin() ) / 2;
                    double clon = ( cte->GetLonMax() + cte->GetLonMin() ) / 2;

                    vLat = clat;
                    vLon = clon;

                    cc->SetViewPoint( clat, clon );

                    if( cc->GetQuiltMode() ) {
                        if( cc->IsChartQuiltableRef( ut_index ) )
                            cc->SelectQuiltRefdbChart( ut_index );
                    } else
                        cc->SelectdbChart( ut_index );

                    double ppm; // final ppm scale to use
                    if (g_unit_test_1) {
                        ppm = cc->GetCanvasScaleFactor() / cte->GetScale();
                        ppm /= 2;
                    }
                    else {
                        double rw, rh; // width, height
                        int ww, wh;    // chart window width, height

                        // width in nm
                        DistanceBearingMercator( cte->GetLatMin(), cte->GetLonMin(), cte->GetLatMin(),
                                  cte->GetLonMax(), NULL, &rw );

                        // height in nm
                        DistanceBearingMercator( cte->GetLatMin(), cte->GetLonMin(), cte->GetLatMax(),
                                 cte->GetLonMin(), NULL, &rh );

                        cc->GetSize( &ww, &wh );
                        ppm = wxMin(ww/(rw*1852), wh/(rh*1852)) * ( 100 - fabs( clat ) ) / 90;
                        ppm = wxMin(ppm, 1.0);
                    }
                    cc->SetVPScale( ppm );

                    cc->ReloadVP();

                    ut_index++;
                    if( ut_index > ut_index_max )
                        exit(0);
                }
                else {
                    _exit(0);
                }
            }
        }
        g_tick++;

    //      Listen for quitflag to be set, requesting application close
        if( quitflag ) {
            ZCHX_LOGMSG( _T("Got quitflag from SIGNAL") );
            FrameTimer1.Stop();
            Close();
            return;
        }

        if( bDBUpdateInProgress ) return;

        FrameTimer1.Stop();

        //  If tracking carryover was found in config file, enable tracking as soon as
        //  GPS become valid
        if(g_bDeferredStartTrack){
            if(!g_bTrackActive){
                if(bGPSValid){
                    gFrame->TrackOn();
                    g_bDeferredStartTrack = false;
                }
            }
            else {                                  // tracking has been manually activated
                g_bDeferredStartTrack = false;
            }
        }

    //  Update and check watchdog timer for GPS data source
        gGPS_Watchdog--;
        if( gGPS_Watchdog <= 0 ) {
            bGPSValid = false;
            if( gGPS_Watchdog == 0  ){
                QString msg;
                msg.Printf( _T("   ***GPS Watchdog timeout at Lat:%g   Lon: %g"), gLat, gLon );
                ZCHX_LOGMSG(msg);
            }
            gSog = NAN;
            gCog = NAN;
        }

    //  Update and check watchdog timer for Mag Heading data source
        gHDx_Watchdog--;
        if( gHDx_Watchdog <= 0 ) {
            gHdm = NAN;
            if( g_nNMEADebug && ( gHDx_Watchdog == 0 ) ) ZCHX_LOGMSG(
                    _T("   ***HDx Watchdog timeout...") );
        }

    //  Update and check watchdog timer for True Heading data source
        gHDT_Watchdog--;
        if( gHDT_Watchdog <= 0 ) {
            g_bHDT_Rx = false;
            gHdt = NAN;
            if( g_nNMEADebug && ( gHDT_Watchdog == 0 ) ) ZCHX_LOGMSG(
                    _T("   ***HDT Watchdog timeout...") );
        }

        //  Update and check watchdog timer for Magnetic Variation data source
        gVAR_Watchdog--;
        if( gVAR_Watchdog <= 0 ) {
            g_bVAR_Rx = false;
            if( g_nNMEADebug && ( gVAR_Watchdog == 0 ) ) ZCHX_LOGMSG(
                _T("   ***VAR Watchdog timeout...") );
        }
        //  Update and check watchdog timer for GSV (Satellite data)
        gSAT_Watchdog--;
        if( gSAT_Watchdog <= 0 ) {
            g_bSatValid = false;
            g_SatsInView = 0;
            if( g_nNMEADebug && ( gSAT_Watchdog == 0 ) ) ZCHX_LOGMSG(
                    _T("   ***SAT Watchdog timeout...") );
        }

        //    Build and send a Position Fix event to PlugIns
        if( g_pi_manager )
        {
            GenericPosDatEx GPSData;
            GPSData.kLat = gLat;
            GPSData.kLon = gLon;
            GPSData.kCog = gCog;
            GPSData.kSog = gSog;
            GPSData.kVar = gVar;
            GPSData.kHdm = gHdm;
            GPSData.kHdt = gHdt;
            GPSData.nSats = g_SatsInView;

            GPSData.FixTime = m_fixtime;

            g_pi_manager->SendPositionFixToAllPlugIns( &GPSData );
        }

        //   Check for anchorwatch alarms                                 // pjotrc 2010.02.15
        if( pAnchorWatchPoint1 ) {
            double dist;
            double brg;
            DistanceBearingMercator( pAnchorWatchPoint1->m_lat, pAnchorWatchPoint1->m_lon, gLat, gLon,
                    &brg, &dist );
            double d = g_nAWMax;
            ( pAnchorWatchPoint1->GetName() ).ToDouble( &d );
            d = AnchorDistFix( d, AnchorPointMinDist, g_nAWMax );
            bool toofar = false;
            bool tooclose = false;
            if( d >= 0.0 ) toofar = ( dist * 1852. > d );
            if( d < 0.0 ) tooclose = ( dist * 1852 < -d );

            if( tooclose || toofar )
                AnchorAlertOn1 = true;
            else
                AnchorAlertOn1 = false;
        } else
            AnchorAlertOn1 = false;

        if( pAnchorWatchPoint2 ) {
            double dist;
            double brg;
            DistanceBearingMercator( pAnchorWatchPoint2->m_lat, pAnchorWatchPoint2->m_lon, gLat, gLon,
                    &brg, &dist );

            double d = g_nAWMax;
            ( pAnchorWatchPoint2->GetName() ).ToDouble( &d );
            d = AnchorDistFix( d, AnchorPointMinDist, g_nAWMax );
            bool toofar = false;
            bool tooclose = false;
            if( d >= 0 ) toofar = ( dist * 1852. > d );
            if( d < 0 ) tooclose = ( dist * 1852 < -d );

            if( tooclose || toofar ) AnchorAlertOn2 = true;
            else
                AnchorAlertOn2 = false;
        } else
            AnchorAlertOn2 = false;

        if( (pAnchorWatchPoint1 || pAnchorWatchPoint2) && !bGPSValid )
            AnchorAlertOn1 = true;

    //  Send current nav status data to log file on every half hour   // pjotrc 2010.02.09

        wxDateTime lognow = wxDateTime::Now();   // pjotrc 2010.02.09
        int hourLOC = lognow.GetHour();
        int minuteLOC = lognow.GetMinute();
        lognow.MakeGMT();
        int minuteUTC = lognow.GetMinute();
        int second = lognow.GetSecond();

        wxTimeSpan logspan = lognow.Subtract( g_loglast_time );
        if( ( logspan.IsLongerThan( wxTimeSpan( 0, 30, 0, 0 ) ) ) || ( minuteUTC == 0 )
                || ( minuteUTC == 30 ) ) {
            if( logspan.IsLongerThan( wxTimeSpan( 0, 1, 0, 0 ) ) ) {
                QString day = lognow.FormatISODate();
                QString utc = lognow.FormatISOTime();
                QString navmsg = _T("LOGBOOK:  ");
                navmsg += day;
                navmsg += _T(" ");
                navmsg += utc;
                navmsg += _T(" UTC ");

                if( bGPSValid ) {
                    QString data;
                    data.Printf( _T(" GPS Lat %10.5f Lon %10.5f "), gLat, gLon );
                    navmsg += data;

                    QString cog;
                    if( std::isnan(gCog) ) cog.Printf( _T("COG ----- ") );
                    else
                        cog.Printf( _T("COG %10.5f "), gCog );

                    QString sog;
                    if( std::isnan(gSog) ) sog.Printf( _T("SOG -----  ") );
                    else
                        sog.Printf( _T("SOG %6.2f ") + getUsrSpeedUnit(), toUsrSpeed( gSog ) );

                    navmsg += cog;
                    navmsg += sog;
                } else {
                    QString data;
                    data.Printf( _T(" DR Lat %10.5f Lon %10.5f"), gLat, gLon );
                    navmsg += data;
                }
                ZCHX_LOGMSG( navmsg );
                g_loglast_time = lognow;

                int bells = ( hourLOC % 4 ) * 2;     // 2 bells each hour
                if( minuteLOC != 0 ) bells++;       // + 1 bell on 30 minutes
                if( !bells ) bells = 8;     // 0 is 8 bells

                if( g_bPlayShipsBells && ( ( minuteLOC == 0 ) || ( minuteLOC == 30 ) ) ) {
                    m_BellsToPlay = bells;
                    wxCommandEvent ev(BELLS_PLAYED_EVTYPE);
                    wxPostEvent(this, ev);
                }
            }
        }

        if( ShouldRestartTrack() )
            TrackDailyRestart();

        if(g_bSleep){
            FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );
            return;
        }

    //      Update the Toolbar Status windows and lower status bar the first time watchdog times out
        if( ( gGPS_Watchdog == 0 ) || ( gSAT_Watchdog == 0 ) ) {
            QString sogcog( _T("SOG --- ") + getUsrSpeedUnit() + + _T("     ") + _T(" COG ---\u00B0") );
            if( GetStatusBar() ) SetStatusText( sogcog, STAT_FIELD_SOGCOG );

            gCog = 0.0;                                 // say speed is zero to kill ownship predictor
        }

    //TODO
    //  Not needed?
    #if 0
    #if !defined(__WXGTK__) && !defined(__WXQT__)
        {
            double cursor_lat, cursor_lon;
            GetPrimaryCanvas()->GetCursorLatLon( &cursor_lat, &cursor_lon );
            GetPrimaryCanvas()->SetCursorStatus(cursor_lat, cursor_lon);
        }
    #endif
    #endif

    //      Update the chart database and displayed chart
        bool bnew_view = false;

    //    Do the chart update based on the global update period currently set
    //    If in COG UP mode, the chart update is handled by COG Update timer
        if( /*!g_bCourseUp &&*/ (0 != g_ChartUpdatePeriod ) ) {
            if (0 == m_ChartUpdatePeriod--) {
                bnew_view = DoChartUpdate();
                m_ChartUpdatePeriod = g_ChartUpdatePeriod;
            }
        }

        nBlinkerTick++;

        // For each canvas....
        for(unsigned int i=0 ; i < g_canvasArray.GetCount() ; i++){
            ChartCanvas *cc = g_canvasArray.Item(i);
            if(cc){

                cc->DrawBlinkObjects();

    //      Update the active route, if any
                if( g_pRouteMan->UpdateProgress() ) {
            //    This RefreshRect will cause any active routepoint to blink
                    if( g_pRouteMan->GetpActiveRoute() )
                        cc->RefreshRect( g_blink_rect, false );
                }

    //  Force own-ship drawing parameters
                cc->SetOwnShipState( SHIP_NORMAL );

                if( cc->GetQuiltMode() ) {
                    double erf = cc->GetQuiltMaxErrorFactor();
                    if( erf > 0.02 )
                        cc->SetOwnShipState( SHIP_LOWACCURACY );
                } else {
                    if( cc->m_singleChart ) {
                        if( cc->m_singleChart->GetChart_Error_Factor() > 0.02 )
                            cc->SetOwnShipState( SHIP_LOWACCURACY );
                    }
                }

                if( !bGPSValid )
                    cc->SetOwnShipState( SHIP_INVALID );

                if( bGPSValid != m_last_bGPSValid ) {
                    if(!g_bopengl)
                        cc->UpdateShips();

                    bnew_view = true;                  // force a full Refresh()
                }
            }
        }

        m_last_bGPSValid = bGPSValid;

        //    If any PlugIn requested dynamic overlay callbacks, force a full canvas refresh
        //    thus, ensuring at least 1 Hz. callback.
        bool brq_dynamic = false;
        if( g_pi_manager ) {
            ArrayOfPlugIns *pplugin_array = g_pi_manager->GetPlugInArray();
            for( unsigned int i = 0; i < pplugin_array->GetCount(); i++ ) {
                PlugInContainer *pic = pplugin_array->Item( i );
                if( pic->m_bEnabled && pic->m_bInitState ) {
                    if( pic->m_cap_flag & WANTS_DYNAMIC_OPENGL_OVERLAY_CALLBACK ) {
                        brq_dynamic = true;
                        break;
                    }
                }
            }

            if( brq_dynamic )
                bnew_view = true;
        }


        //  Make sure we get a redraw and alert sound on AnchorWatch excursions.
        if(AnchorAlertOn1 || AnchorAlertOn2)
            bnew_view = true;

        // For each canvas....
        for(unsigned int i=0 ; i < g_canvasArray.GetCount() ; i++){
                ChartCanvas *cc = g_canvasArray.Item(i);
                if(cc){

                    if(g_bopengl) {
    #ifdef ocpnUSE_GL
                        if (cc->GetglCanvas()) {
                            if (m_fixtime - cc->GetglCanvas()->m_last_render_time > 0)
                                bnew_view = true;
                        }

                        if( AnyAISTargetsOnscreen( cc, cc->GetVP() ) )
                            bnew_view = true;

                        if(bnew_view) /* full frame in opengl mode */
                            cc->Refresh(false);
    #endif
                    } else {
    //  Invalidate the ChartCanvas window appropriately
    //    In non-follow mode, invalidate the rectangles containing the AIS targets and the ownship, etc...
    //    In follow mode, if there has already been a full screen refresh, there is no need to check ownship or AIS,
    //       since they will be always drawn on the full screen paint.

                    if( ( !cc->m_bFollow ) || cc->m_bCourseUp ) {
                        cc->UpdateShips();
                        cc->UpdateAIS();
                        cc->UpdateAlerts();
                    } else {
                        if( !bnew_view ) {                   // There has not been a Refresh() yet.....
                            cc->UpdateAIS();
                            cc->UpdateAlerts();
                        }
                    }
                }
            }
        }

        if( g_pais_query_dialog_active && g_pais_query_dialog_active->IsShown() )
            g_pais_query_dialog_active->UpdateText();

        // Refresh AIS target list every 5 seconds to avoid blinking
        if( g_pAISTargetList && ( 0 == ( g_tick % ( 5 ) ) ) )
            g_pAISTargetList->UpdateAISTargetList();

        //  Pick up any change Toolbar status displays
        UpdateGPSCompassStatusBoxes();
        UpdateAISTool();

        if( console && console->IsShown() ) {
    //            console->Raise();
            console->RefreshConsoleData();
        }

        //  This little hack fixes a problem seen with some UniChrome OpenGL drivers
        //  We need a deferred resize to get glDrawPixels() to work right.
        //  So we set a trigger to generate a resize after 5 seconds....
        //  See the "UniChrome" hack elsewhere
        if( m_bdefer_resize ) {
            if( 0 == ( g_tick % ( 5 ) ) ) {
                printf( "___RESIZE\n" );
                SetSize( m_defer_size );
                g_pauimgr->Update();
                m_bdefer_resize = false;
            }
        }
        if (g_unit_test_2)
            FrameTimer1.Start( TIMER_GFRAME_1*3, wxTIMER_CONTINUOUS );
        else
            FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );
#endif
}





void zchxMapMainWindow::ToggleColorScheme()
{
//    ColorScheme s = GetColorScheme();
//    int is = (int) s;
//    is++;
//    s = (ColorScheme) is;
//    if( s == N_COLOR_SCHEMES ) s = GLOBAL_COLOR_SCHEME_RGB;

//    SetAndApplyColorScheme( s );
}

bool zchxMapMainWindow::DoChartUpdate( void )
{
    bool return_val = false;
    if(mEcdisWidget) return_val = mEcdisWidget->DoCanvasUpdate();
    return return_val;

}

ChartCanvas *zchxMapMainWindow::GetPrimaryCanvas()
{
    return mEcdisWidget;
}

void zchxMapMainWindow::SetChartUpdatePeriod( )
{
//    //    Set the chart update period based upon chart skew and skew compensator

//    g_ChartUpdatePeriod = 0;            // General default

//    // In non-GL, singlele-chart mode, rotation of skewed charts is very slow
//    //  So we need to use a slower update time constant to preserve adequate UI performance
//    bool bskewdc = false;
//        for(unsigned int i=0 ; i < g_canvasArray.GetCount() ; i++){
//            ChartCanvas *cc = g_canvasArray.Item(i);
//            if(cc){
//                if( !g_bopengl && !cc->GetVP().b_quilt){
//                    if ( fabs(cc->GetVP().skew) > 0.0001)
//                        bskewdc = true;
//                }
//                if(cc->m_bFollow)
//                    g_ChartUpdatePeriod = 1;
//            }
//        }

//    if (bskewdc)
//        g_ChartUpdatePeriod = g_SkewCompUpdatePeriod;


//    m_ChartUpdatePeriod = g_ChartUpdatePeriod;
}

//double zchxMapMainWindow::GetBestVPScale( ChartBase *pchart )
//{
//    return GetPrimaryCanvas()->GetBestVPScale( pchart );
//}

void zchxMapMainWindow::RefreshAllCanvas( bool bErase)
{
    if(mEcdisWidget)
    {
        mEcdisWidget->DoCanvasUpdate();
        mEcdisWidget->ReloadVP();                  // once more, and good to go
    }
    // For each canvas
//    for(unsigned int i=0 ; i < g_canvasArray.count() ; i++){
//        ChartCanvas *cc = g_canvasArray.at(i);
//        if(cc){
//            cc->Refresh( bErase );
//        }
//    }
}

void zchxMapMainWindow::SetGPSCompassScale()
{
    g_compass_scalefactor = OCPNPlatform::instance()->GetCompassScaleFactor( g_GUIScaleFactor );

}

double zchxMapMainWindow::GetMag(double a)
{
    if(!std::isnan(gVar)){
        if((a - gVar) >360.)
            return (a - gVar - 360.);
        else
            return ((a - gVar) >= 0.) ? (a - gVar) : (a - gVar + 360.);
    }
    else{
        if((a - g_UserVar) >360.)
            return (a - g_UserVar - 360.);
        else
            return ((a - g_UserVar) >= 0.) ? (a - g_UserVar) : (a - g_UserVar + 360.);
    }
}



ColorScheme GetColorScheme()
{
    return global_color_scheme;
}

/*************************************************************************
 * Global color management routines
 *
 *************************************************************************/

QColor GetGlobalColor(const QString& colorName)
{
    QColor ret_color;
    //    Use the S52 Presentation library if present
    if( ps52plib ) ret_color = ps52plib->getQColor( colorName );
    if( !ret_color.isValid() && pcurrent_user_color_hash )
        ret_color = ( *pcurrent_user_color_hash )[colorName];

    //    Default
    if( !ret_color.isValid() ) {
        ret_color.setRgb(128, 128, 128 );  // Simple Grey
        qDebug("Warning: Color not found %s ", colorName.toUtf8().data());
        // Avoid duplicate warnings:
        if (pcurrent_user_color_hash)
            ( *pcurrent_user_color_hash )[colorName] = ret_color;
    }

    return ret_color;
}

static const char *usercolors[] = { "Table:DAY", "GREEN1;120;255;120;", "GREEN2; 45;150; 45;",
        "GREEN3;200;220;200;", "GREEN4;  0;255;  0;", "BLUE1; 170;170;255;", "BLUE2;  45; 45;170;",
        "BLUE3;   0;  0;255;", "GREY1; 200;200;200;", "GREY2; 230;230;230;", "RED1;  220;200;200;",
        "UBLCK;   0;  0;  0;", "UWHIT; 255;255;255;", "URED;  255;  0;  0;", "UGREN;   0;255;  0;",
        "YELO1; 243;229; 47;", "YELO2; 128; 80;  0;", "TEAL1;   0;128;128;", "GREEN5;170;254;  0;",
        "COMPT; 245;247;244",
#ifdef __WXOSX__
        "DILG0; 255;255;255;",              // Dialog Background white
#else
        "DILG0; 238;239;242;",              // Dialog Background white
#endif
        "DILG1; 212;208;200;",              // Dialog Background
        "DILG2; 255;255;255;",              // Control Background
        "DILG3;   0;  0;  0;",              // Text
        "UITX1;   0;  0;  0;",              // Menu Text, derived from UINFF

        "CHGRF; 163; 180; 183;",
        "UINFM; 197;  69; 195;",
        "UINFG; 104; 228;  86;",
        "UINFF; 125; 137; 140;",
        "UINFR; 241;  84; 105;",
        "SHIPS;   7;   7;   7;",
        "CHYLW; 244; 218;  72;",
        "CHWHT; 212; 234; 238;",

        "UDKRD; 124; 16;  0;",
        "UARTE; 200;  0;  0;",              // Active Route, Grey on Dusk/Night

        "NODTA; 163; 180; 183;",
        "CHBLK;   7;   7;   7;",
        "SNDG1; 125; 137; 140;",
        "SNDG2;   7;   7;   7;",
        "SCLBR; 235; 125;  54;",
        "UIBDR; 125; 137; 140;",
        "UINFB;  58; 120; 240;",
        "UINFD;   7;   7;   7;",
        "UINFO; 235; 125;  54;",
        "PLRTE; 220;  64;  37;",
        "CHMGD; 197; 69; 195;",
        "UIBCK; 212; 234; 238;",

        "DASHB; 255;255;255;",              // Dashboard Instr background
        "DASHL; 190;190;190;",              // Dashboard Instr Label
        "DASHF;  50; 50; 50;",              // Dashboard Foreground
        "DASHR; 200;  0;  0;",              // Dashboard Red
        "DASHG;   0;200;  0;",              // Dashboard Green
        "DASHN; 200;120;  0;",              // Dashboard Needle
        "DASH1; 204;204;255;",              // Dashboard Illustrations
        "DASH2; 122;131;172;",              // Dashboard Illustrations
        "COMP1; 211;211;211;",              // Compass Window Background

        "GREY3;  40; 40; 40;",              // MUIBar/TB background
        "BLUE4; 100;100;200;",              // Canvas Focus Bar
        "VIO01; 171; 33;141;",
        "VIO02; 209;115;213;",



        "Table:DUSK", "GREEN1; 60;128; 60;", "GREEN2; 22; 75; 22;", "GREEN3; 80;100; 80;",
        "GREEN4;  0;128;  0;", "BLUE1;  80; 80;160;", "BLUE2;  30; 30;120;", "BLUE3;   0;  0;128;",
        "GREY1; 100;100;100;", "GREY2; 128;128;128;", "RED1;  150;100;100;", "UBLCK;   0;  0;  0;",
        "UWHIT; 255;255;255;", "URED;  120; 54; 11;", "UGREN;  35;110; 20;", "YELO1; 120;115; 24;",
        "YELO2;  64; 40;  0;", "TEAL1;   0; 64; 64;", "GREEN5; 85;128; 0;",
        "COMPT; 124;126;121",

        "CHGRF;  41; 46; 46;",
        "UINFM;  58; 20; 57;",
        "UINFG;  35; 76; 29;",
        "UINFF;  41; 46; 46;",
        "UINFR;  80; 28; 35;",
        "SHIPS;  71; 78; 79;",
        "CHYLW;  81; 73; 24;",
        "CHWHT;  71; 78; 79;",

        "DILG0; 110;110;110;",              // Dialog Background
        "DILG1; 110;110;110;",              // Dialog Background
        "DILG2;   0;  0;  0;",              // Control Background
        "DILG3; 130;130;130;",              // Text
        "UITX1;  41; 46; 46;",              // Menu Text, derived from UINFF
        "UDKRD;  80;  0;  0;",
        "UARTE;  64; 64; 64;",              // Active Route, Grey on Dusk/Night

        "NODTA;  41;  46;  46;",
        "CHBLK;  54;  60;  61;",
        "SNDG1;  41;  46;  46;",
        "SNDG2;  71;  78;  79;",
        "SCLBR;  75;  38;  19;",
        "UIBDR;  54;  60;  61;",
        "UINFB;  19;  40;  80;",
        "UINFD;  71;  78;  79;",
        "UINFO;  75;  38;  19;",
        "PLRTE;  73;  21;  12;",
        "CHMGD; 74; 58; 81;",
        "UIBCK; 7; 7; 7;",

        "DASHB;  77; 77; 77;",              // Dashboard Instr background
        "DASHL;  54; 54; 54;",              // Dashboard Instr Label
        "DASHF;   0;  0;  0;",              // Dashboard Foreground
        "DASHR;  58; 21; 21;",              // Dashboard Red
        "DASHG;  21; 58; 21;",              // Dashboard Green
        "DASHN; 100; 50;  0;",              // Dashboard Needle
        "DASH1;  76; 76;113;",              // Dashboard Illustrations
        "DASH2;  48; 52; 72;",              // Dashboard Illustrations
        "COMP1; 107;107;107;",              // Compass Window Background

        "GREY3;  20; 20; 20;",              // MUIBar/TB background
        "BLUE4;  80; 80;160;",              // Canvas Focus Bar
        "VIO01; 128; 25;108;",
        "VIO02; 171; 33;141;",

        "Table:NIGHT", "GREEN1; 30; 80; 30;", "GREEN2; 15; 60; 15;", "GREEN3; 12; 23;  9;",
        "GREEN4;  0; 64;  0;", "BLUE1;  60; 60;100;", "BLUE2;  22; 22; 85;", "BLUE3;   0;  0; 40;",
        "GREY1;  48; 48; 48;", "GREY2;  32; 32; 32;", "RED1;  100; 50; 50;", "UWHIT; 255;255;255;",
        "UBLCK;   0;  0;  0;", "URED;   60; 27;  5;", "UGREN;  17; 55; 10;", "YELO1;  60; 65; 12;",
        "YELO2;  32; 20;  0;", "TEAL1;   0; 32; 32;", "GREEN5; 44; 64; 0;",
        "COMPT;  48; 49; 51",
        "DILG0;  80; 80; 80;",              // Dialog Background
        "DILG1;  80; 80; 80;",              // Dialog Background
        "DILG2;   0;  0;  0;",              // Control Background
        "DILG3;  65; 65; 65;",              // Text
        "UITX1;  31; 34; 35;",              // Menu Text, derived from UINFF
        "UDKRD;  50;  0;  0;",
        "UARTE;  64; 64; 64;",              // Active Route, Grey on Dusk/Night

        "CHGRF;  16; 18; 18;",
        "UINFM;  52; 18; 52;",
        "UINFG;  22; 24;  7;",
        "UINFF;  31; 34; 35;",
        "UINFR;  59; 17; 10;",
        "SHIPS;  37; 41; 41;",
        "CHYLW;  31; 33; 10;",
        "CHWHT;  37; 41; 41;",

        "NODTA;   7;   7;   7;",
        "CHBLK;  31;  34;  35;",
        "SNDG1;  31;  34;  35;",
        "SNDG2;  43;  48;  48;",
        "SCLBR;  52;  28;  12;",
        "UIBDR;  31;  34;  35;",
        "UINFB;  21;  29;  69;",
        "UINFD;  43;  48;  58;",
        "UINFO;  52;  28;  12;",
        "PLRTE;  66;  19;  11;",
        "CHMGD; 52; 18; 52;",
        "UIBCK; 7; 7; 7;",

        "DASHB;   0;  0;  0;",              // Dashboard Instr background
        "DASHL;  20; 20; 20;",              // Dashboard Instr Label
        "DASHF;  64; 64; 64;",              // Dashboard Foreground
        "DASHR;  70; 15; 15;",              // Dashboard Red
        "DASHG;  15; 70; 15;",              // Dashboard Green
        "DASHN;  17; 80; 56;",              // Dashboard Needle
        "DASH1;  48; 52; 72;",              // Dashboard Illustrations
        "DASH2;  36; 36; 53;",              // Dashboard Illustrations
        "COMP1;  24; 24; 24;",              // Compass Window Background

        "GREY3;  10; 10; 10;",              // MUIBar/TB background
        "BLUE4;  70; 70;140;",              // Canvas Focus Bar
        "VIO01;  85; 16; 72;",
        "VIO02; 128; 25;108;",

        "*****" };

int get_static_line( char *d, const char **p, int index, int n )
{
    if( !strcmp( p[index], "*****" ) ) return 0;

    strncpy( d, p[index], n );
    return strlen( d );
}


void InitializeUserColors( void )
{
    const char **p = usercolors;
    char buf[81];
    int index = 0;
    char TableName[20];
    colTable *ctp = 0;
    colTable *ct = 0;
    int colIdx = 0;
    int R, G, B;

    UserColorTableArray = new QList<colTable*>;
    UserColourHashTableArray = new QList<QColorHashMap*>;

    //    Create 3 color table entries
    ct = new colTable;
    ct->tableName = "DAY";
    ct->color = new QList<S52color*>;
    UserColorTableArray->append(ct );

    ct = new colTable;
    ct->tableName = "DUSK";
    ct->color = new QList<S52color*>;
    UserColorTableArray->append( ct );

    ct = new colTable;
    ct->tableName = "NIGHT";
    ct->color = new QList<S52color*>;
    UserColorTableArray->append( ct );

    while( ( get_static_line( buf, p, index, sizeof(buf) - 1 ) ) ) {
        if( !strncmp( buf, "Table", 5 ) ) {
            sscanf( buf, "Table:%s", TableName );

            for( unsigned int it = 0; it < UserColorTableArray->count(); it++ ) {
                ctp =  UserColorTableArray->at( it ) ;
                if( !strcmp( TableName, ctp->tableName.toUtf8().data() ) ) {
                    ct = ctp;
                    colIdx = 0;
                    break;
                }
            }

        } else {
            char name[21];
            int j = 0;
            while( buf[j] != ';' && j < 20 ) {
                name[j] = buf[j];
                j++;
            }
            name[j] = 0;

            S52color *c = new S52color;
            strcpy( c->colName, name );

            sscanf( &buf[j], ";%i;%i;%i", &R, &G, &B );
            c->R = (char) R;
            c->G = (char) G;
            c->B = (char) B;

            ct->color->append( c );

        }

        index++;
    }

    //    Now create the Hash tables

    for( unsigned int its = 0; its < UserColorTableArray->count(); its++ ) {
        QColorHashMap *phash = new QColorHashMap;
        UserColourHashTableArray->append( phash );
        colTable *ctp = UserColorTableArray->at( its );
        for( unsigned int ic = 0; ic < ctp->color->count(); ic++ ) {
            S52color *c2 = ctp->color->at( ic );
            QColor c( c2->R, c2->G, c2->B );
            QString key = QString::fromUtf8(c2->colName);
            phash->insert(key, c);
        }
    }

    //    Establish a default hash table pointer
    //    in case a color is needed before ColorScheme is set
    if(UserColourHashTableArray->size() > 0)
    {
        pcurrent_user_color_hash = UserColourHashTableArray->at( 0 );
    } else
    {
        pcurrent_user_color_hash = 0;
    }
}

void DeInitializeUserColors( void )
{
    unsigned int i;
    if(UserColorTableArray)
    {
        for( i = 0; i < UserColorTableArray->count(); i++ ) {
            colTable *ct = UserColorTableArray->at( i );
            for( unsigned int j = 0; j < ct->color->count(); j++ ) {
                S52color *c = ct->color->at( j );
                delete c;                     //color
            }

            //        delete ct->tableName;               // wxString
            delete ct->color;                   // wxArrayPtrVoid

            delete ct;                          // colTable
        }

        delete UserColorTableArray;
    }
    if(UserColourHashTableArray)
    {

        for( i = 0; i < UserColourHashTableArray->count(); i++ ) {
            QColorHashMap *phash = UserColourHashTableArray->at( i );
            delete phash;
        }

        delete UserColourHashTableArray;
    }

}



void LoadS57()
{
    if(ps52plib) // already loaded?
        return;

    //  Start a SENC Thread manager
    g_SencThreadManager = new SENCThreadManager();

//      Set up a useable CPL library error handler for S57 stuff
//    CPLSetErrorHandler( MyCPLErrorHandler );

//      Init the s57 chart object, specifying the location of the required csv files
    g_csv_locn = zchxFuncUtil::getDataDir();
    g_csv_locn.append(zchxFuncUtil::separator()).append("s57data");
//      If the config file contains an entry for SENC file prefix, use it.
//      Otherwise, default to PrivateDataDir
    if( g_SENCPrefix.isEmpty() ) {
        g_SENCPrefix = zchxFuncUtil::getDataDir();
        g_SENCPrefix.append(zchxFuncUtil::separator());
        g_SENCPrefix.append("SENC");
    }

//      If the config file contains an entry for PresentationLibraryData, use it.
//      Otherwise, default to conditionally set spot under g_pcsv_locn
    QString plib_data;
    bool b_force_legacy = false;

    if( g_UserPresLibData.isEmpty() ) {
        plib_data = g_csv_locn;
        plib_data.append(zchxFuncUtil::separator());
        plib_data.append("S52RAZDS.RLE");
    } else {
        plib_data = g_UserPresLibData;
        b_force_legacy = true;
    }

    ps52plib = new s52plib( plib_data, b_force_legacy );

    //  If the library load failed, try looking for the s57 data elsewhere

    //  First, look in UserDataDir
    /*    From wxWidgets documentation

     wxStandardPaths::GetUserDataDir
     QString GetUserDataDir() const
     Return the directory for the user-dependent application data files:
     * Unix: ~/.appname
     * Windows: C:\Documents and Settings\username\Application Data\appname
     * Mac: ~/Library/Application Support/appname
     */

    if( !ps52plib->m_bOK ) {
        delete ps52plib;
        QString look_data_dir;
        look_data_dir.append( zchxFuncUtil::getAppDir());
        look_data_dir.append(zchxFuncUtil::separator());
        QString tentative_SData_Locn = look_data_dir;
        look_data_dir.append("s57data");

        plib_data = look_data_dir;
        plib_data.append(zchxFuncUtil::separator());
        plib_data.append("S52RAZDS.RLE");

        qDebug("Looking for s57data in %s", look_data_dir.toUtf8().data() );
        ps52plib = new s52plib( plib_data );

        if( ps52plib->m_bOK ) {
            g_csv_locn = look_data_dir;
        }
    }

    //  And if that doesn't work, look again in the original SData Location
    //  This will cover the case in which the .ini file entry is corrupted or moved

    if( !ps52plib->m_bOK ) {
        delete ps52plib;

        QString look_data_dir;
        look_data_dir = zchxFuncUtil::getDataDir();
        look_data_dir.append(zchxFuncUtil::separator());
        look_data_dir.append("s57data" );

        plib_data = look_data_dir;
        plib_data.append(zchxFuncUtil::separator());
        plib_data.append( ("S52RAZDS.RLE") );

        qDebug("Looking for s57data in %s", look_data_dir.toUtf8().data() );
        ps52plib = new s52plib( plib_data );

        if( ps52plib->m_bOK ) g_csv_locn = look_data_dir;
    }

    if( ps52plib->m_bOK ) {
        qDebug("Using s57data in %s",  g_csv_locn.toUtf8().data() );
        m_pRegistrarMan = new s57RegistrarMgr( g_csv_locn );


            //    Preset some object class visibilites for "User Standard" disply category
            //  They may be overridden in LoadS57Config
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->count(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->at( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "DEPARE", 6 ) ) pOLE->nViz = 1;
            if( !strncmp( pOLE->OBJLName, "LNDARE", 6 ) ) pOLE->nViz = 1;
            if( !strncmp( pOLE->OBJLName, "COALNE", 6 ) ) pOLE->nViz = 1;
        }

        ZCHX_CFG_INS->LoadS57Config();
        ps52plib->SetPLIBColorScheme( global_color_scheme );

        if(gFrame->GetPrimaryCanvas() )
            ps52plib->SetPPMM( gFrame->GetPrimaryCanvas()->GetPixPerMM() );

#ifdef ocpnUSE_GL

        // Setup PLIB OpenGL options, if enabled
        extern bool g_b_EnableVBO;
        extern GLenum  g_texture_rectangle_format;
        ps52plib->SetGLOptions(glChartCanvas::s_b_useStencil,
                               glChartCanvas::s_b_useStencilAP,
                               glChartCanvas::s_b_useScissorTest,
                               glChartCanvas::s_b_useFBO,
                               g_b_EnableVBO,
                               g_texture_rectangle_format);
#endif


    } else {
        qDebug("   S52PLIB Initialization failed, disabling Vector charts." );
        delete ps52plib;
        ps52plib = NULL;
    }
}


void zchxMapMainWindow::InvalidateAllGL()
{
    if(mEcdisWidget)
    {
//        mEcdisWidget->InvalidateGL();
//        mEcdisWidget->Refresh();
    }
}

ColorScheme zchxMapMainWindow::GetColorScheme()
{
    return global_color_scheme;
}


void zchxMapMainWindow::slotRotateDegree(double angle)
{
    if(mEcdisWidget) mEcdisWidget->DoRotateCanvasWithDegree(angle);
}

void zchxMapMainWindow::slotRoateRad(double rad)
{
    if(mEcdisWidget) mEcdisWidget->DoRotateCanvas(rad);
}

void zchxMapMainWindow::slotRotate()
{
    static double rotate = 60;
    static double coeff = -1.0;
    slotRotateDegree(rotate);
    rotate += coeff * 60.0;
    if(fabs(rotate) > 60)
    {
        coeff *= (-1);
        rotate = 0;
    }
}

