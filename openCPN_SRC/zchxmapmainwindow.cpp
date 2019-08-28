#include "zchxmapmainwindow.h"
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
//#include "s52plib.h"
#include "OCPNPlatform.h"
//#include "S57ClassRegistrar.h"
//#include "s57RegistrarMgr.h"
//#include "glChartCanvas.h"
//#include "thumbwin.h"
//#include "styles.h"
#include <QVBoxLayout>
#include <QThread>
#include "config.h"


extern      ChartDB                     *ChartData;
extern      QThread                     *g_Main_thread;
//arrayofCanvasPtr            g_canvasArray;
extern      float                       g_compass_scalefactor;
extern      int                         g_GUIScaleFactor;
extern      OCPNPlatform                *g_Platform;
extern      ColorScheme                 global_color_scheme;


static wxArrayPtrVoid       *UserColorTableArray;
static wxArrayPtrVoid       *UserColourHashTableArray;
static QColorHashMap        *pcurrent_user_color_hash;
//SENCThreadManager           *g_SencThreadManager;
extern s52plib                   *ps52plib;
extern QString                  g_csv_locn;

extern QString                  g_SENCPrefix;
extern QString                  g_UserPresLibData;
//extern S57ClassRegistrar         *g_poRegistrar;
//s57RegistrarMgr           *m_pRegistrarMan = 0;
extern zchxMapMainWindow*  gFrame;
extern int                       g_nDepthUnitDisplay;

extern int                       g_nCacheLimit;
extern int                       g_memCacheLimit;
//extern ThumbWin                  *pthumbwin;
extern bool                      g_bopengl;
extern bool                      g_fog_overzoom;
extern double                    g_overzoom_emphasis_base;
extern bool                      g_oz_vector_scale;

extern QString                  ChartListFileName;
extern QString                  AISTargetNameFileName;
extern QString                  gWorldMapLocation, gDefaultWorldMapLocation;
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
extern bool                      g_bdisable_opengl;

//extern ChartGroupArray           *g_pGroupArray;
extern int                       g_GroupIndex;
extern bool                      g_bNeedDBUpdate;
extern bool                      g_bPreserveScaleOnX;
extern bool                      g_bFullscreen;
extern bool                      g_bFullScreenQuilt;
extern bool                      g_bQuiltEnable;
extern bool                      g_bQuiltStart;
extern bool                      g_bquiting;
//extern ocpnStyle::StyleManager*  g_StyleManager;
extern double                    g_ChartNotRenderScaleFactor;
std::vector<int>               g_quilt_noshow_index_array;
extern int                       g_nbrightness;
extern bool                      bDBUpdateInProgress;
extern bool                      bGPSValid;
extern int                       g_SatsInView;
extern bool                      g_bSatValid;
extern bool                        g_bSpaceDropMark;
extern QString                  *pInit_Chart_Dir;
extern ChartGroupArray            *g_pGroupArray;
bool                                g_bNeedDBUpdate;
QString                             ChartListFileName;
QString                             gDefaultWorldMapLocation;



zchxMapMainWindow::zchxMapMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::zchxMapMainWindow)
    , mEcdisWidget(0)
{

    ui->setupUi(this);
    gFrame = this;
    g_Main_thread = QThread::currentThread();
    //工具
    QMenu* tools = this->menuBar()->addMenu(tr("Tools"));
    addCustomAction(tools,tr("Options"),this, SLOT(slotOpenSettingDlg()));
    addCustomAction(tools, tr("Measure distance"), this, SLOT(slotMeasureDistance()));

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
    addCustomAction(view, tr("Show ENC Text"), this, SLOT(slotShowChartBar(bool)));
    addCustomAction(view, tr("Zoom In"), this, SLOT(slotShowENCText(bool)));
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
#if 1  //如果使用海图S7
    //开始初始化平台的相关信息
    QDateTime now = QDateTime::currentDateTime();
    qDebug()<<"start ecids now at:"<<now.toString("yyyy-MM-dd hh:mm:ss");
    int mem_used =0, mem_total = 0;
    zchxFuncUtil::getMemoryStatus(& mem_total, &mem_used);
    qDebug()<<"memory total(M):"<<mem_total<<"  app used(M):"<<mem_used;
    g_Platform = new OCPNPlatform;
    //设定标题
    this->setWindowTitle("ZCHX Ecdis");
    //获取当前版本
    qDebug()<<" currernt version is:"<<VERSION_FULL<<" date:"<<VERSION_DATE;
    //获取本地数据的范围
    qDebug()<<"local data dir is:"<<zchxFuncUtil::getDataDir();
    //加载默认的配置文件
    if(!ZCHX_CFG_INS->hasLoadConfig()) ZCHX_CFG_INS->loadMyConfig();
    //添加显示控件,地图数据在显示控件进行初始化,MainWindow只是提供外部操作的接口
//    mEcdisWidget = new ChartCanvas(this, 0);
//    if(!ui->centralwidget->layout())
//    {
//        ui->centralwidget->setLayout(new QVBoxLayout(ui->centralwidget));
//    }
//    ui->centralwidget->layout()->addWidget(mEcdisWidget);
#endif
    initEcdis();
}

zchxMapMainWindow::~zchxMapMainWindow()
{
    delete ui;
}

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

}

void zchxMapMainWindow::slotZoomOut()
{

}

void zchxMapMainWindow::slotLargeScaleChart()
{

}

void zchxMapMainWindow::slotSmallScaleChart()
{

}

void zchxMapMainWindow::slotEnableChartQuilting(bool sts)
{

}

void zchxMapMainWindow::slotShowENCAnchoringInfo(bool sts)
{

}

void zchxMapMainWindow::slotShowBuoyLightLabel(bool sts)
{

}

void zchxMapMainWindow::slotShowChartBar(bool sts)
{

}

void zchxMapMainWindow::slotShowChartQuilting(bool sts)
{

}

void zchxMapMainWindow::slotShowDepth(bool sts)
{

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

}

void zchxMapMainWindow::slotShowENCText(bool sts)
{

}

void zchxMapMainWindow::slotShowGrid(bool sts)
{

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
#if 0
    bool b_need_refresh = false;                // Do we need a full reload?

    if( ( rr & VISIT_CHARTS )
            && ( ( rr & CHANGE_CHARTS ) || ( rr & FORCE_UPDATE ) || ( rr & SCAN_UPDATE ) ) ) {
        if(pNewDirArray){
            UpdateChartDatabaseInplace( *pNewDirArray, ( ( rr & FORCE_UPDATE ) == FORCE_UPDATE ),
                true, ChartListFileName );

            b_need_refresh = true;
        }
    }

    if(  rr & STYLE_CHANGED  ) {
        OCPNMessageBox(NULL, _("Please restart OpenCPN to activate language or style changes."),
                _("OpenCPN Info"), wxOK | wxICON_INFORMATION );
    }

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
            GetPrimaryCanvas()->Disable();
            g_glTextureManager->BuildCompressedCache();
            GetPrimaryCanvas()->Enable();
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


bool zchxMapMainWindow::UpdateChartDatabaseInplace( ArrayOfCDI &DirArray, bool b_force, bool b_prog, const QString &ChartListFileName )
{
#if 0
    bool b_run = false;
    if(FrameTimer1)
    {
        b_run = FrameTimer1->isActive();
        FrameTimer1->stop();                  // stop other asynchronous activity
    }

    // ..For each canvas...
    for(unsigned int i=0 ; i < g_canvasArray.GetCount() ; i++){
        ChartCanvas *cc = g_canvasArray.Item(i);
        if( cc ) {
            cc->InvalidateQuilt();
            cc->SetQuiltRefChart( -1 );
            cc->m_singleChart = NULL;
        }
    }
    if(ChartData)   ChartData->PurgeCache();

//TODO
//     delete pCurrentStack;
//     pCurrentStack = NULL;

    OCPNPlatform::ShowBusySpinner();

    wxGenericProgressDialog *pprog = nullptr;
    if( b_prog ) {
        QString longmsg = _("OpenCPN Chart Update");
        longmsg += _T("..........................................................................");

        pprog = new wxGenericProgressDialog();

        wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
        pprog->SetFont( *qFont );

        pprog->Create( _("OpenCPN Chart Update"), longmsg, 100,
                                          gFrame, wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );


        DimeControl( pprog );
        pprog->Show();
    }

    ZCHX_LOGMSG( _T("   ") );
    ZCHX_LOGMSG( _T("Starting chart database Update...") );
    QString gshhg_chart_loc = gWorldMapLocation;
    gWorldMapLocation = wxEmptyString;
    ChartData->Update( DirArray, b_force, pprog );
    ChartData->SaveBinary(ChartListFileName);
    ZCHX_LOGMSG( _T("Finished chart database Update") );
    ZCHX_LOGMSG( _T("   ") );
    if( gWorldMapLocation.empty() ) { //Last resort. User might have deleted all GSHHG data, but we still might have the default dataset distributed with OpenCPN or from the package repository...
       gWorldMapLocation = gDefaultWorldMapLocation;
       gshhg_chart_loc = wxEmptyString;
    }

    if( gWorldMapLocation != gshhg_chart_loc ){
    // ..For each canvas...
        for(unsigned int i=0 ; i < g_canvasArray.GetCount() ; i++){
            ChartCanvas *cc = g_canvasArray.Item(i);
            if( cc )
                cc->ResetWorldBackgroundChart();
        }
    }


    delete pprog;

    OCPNPlatform::HideBusySpinner();

    pConfig->UpdateChartDirs( DirArray );

    if( b_run ) FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );
#endif

    return true;
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
//    if(mEcdisWidget) return_val = mEcdisWidget->DoCanvasUpdate();
    return return_val;

}

void zchxMapMainWindow::UpdateRotationState( double rotation )
{
//    //  If rotated manually, we switch to NORTHUP
//    g_bCourseUp = false;

//    if(fabs(rotation) > .001){
//        SetMenubarItemState( ID_MENU_CHART_COGUP, false );
//        SetMenubarItemState( ID_MENU_CHART_NORTHUP, true );
//        if(m_pMenuBar){
//            m_pMenuBar->SetLabel( ID_MENU_CHART_NORTHUP, _("Rotated Mode") );
//        }
//    }
//    else{
//        SetMenubarItemState( ID_MENU_CHART_COGUP, g_bCourseUp );
//        SetMenubarItemState( ID_MENU_CHART_NORTHUP, !g_bCourseUp );
//        if(m_pMenuBar){
//            m_pMenuBar->SetLabel( ID_MENU_CHART_NORTHUP, _("North Up Mode") );
//        }
//    }

//    UpdateGPSCompassStatusBoxes( true );
    DoChartUpdate();
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
    g_compass_scalefactor = g_Platform->GetCompassScaleFactor( g_GUIScaleFactor );

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
#if 0
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
#endif

    return ret_color;
}


void LoadS57()
{
#if 0
    if(ps52plib) // already loaded?
        return;

    //  Start a SENC Thread manager
    g_SencThreadManager = new SENCThreadManager();

//      Set up a useable CPL library error handler for S57 stuff
//    CPLSetErrorHandler( MyCPLErrorHandler );

//      Init the s57 chart object, specifying the location of the required csv files
    g_csv_locn = g_Platform->GetDataDir();
    g_csv_locn.append(QDir::separator()).append("s57data");
//      If the config file contains an entry for SENC file prefix, use it.
//      Otherwise, default to PrivateDataDir
    if( g_SENCPrefix.isEmpty() ) {
        g_SENCPrefix = g_Platform->GetDataDir();
        g_SENCPrefix.append(QDir::separator());
        g_SENCPrefix.append("SENC");
    }

//      If the config file contains an entry for PresentationLibraryData, use it.
//      Otherwise, default to conditionally set spot under g_pcsv_locn
    QString plib_data;
    bool b_force_legacy = false;

    if( g_UserPresLibData.isEmpty() ) {
        plib_data = g_csv_locn;
        plib_data.append(QDir::separator());
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
        look_data_dir.append( g_Platform->GetAppDir());
        look_data_dir.append(QDir::separator());
        QString tentative_SData_Locn = look_data_dir;
        look_data_dir.append("s57data");

        plib_data = look_data_dir;
        plib_data.append(QDir::separator());
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
        look_data_dir = g_Platform->GetDataDir();
        look_data_dir.append(QDir::separator());
        look_data_dir.append("s57data" );

        plib_data = look_data_dir;
        plib_data.append(QDir::separator());
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
        extern GLenum  g_texture_rectangle_format;\
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
#endif
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

void zchxMapMainWindow::initEcdis()
{
    qDebug()<<"start init db data now";
    if(!pInit_Chart_Dir) pInit_Chart_Dir = new QString(zchxFuncUtil::getDataDir());
    qDebug()<<"init chart dir:"<<pInit_Chart_Dir;
    if(!g_pGroupArray) g_pGroupArray = new ChartGroupArray;
    //获取数据文件
    ChartListFileName = QString("%1/CHRTLIST.DAT").arg(zchxFuncUtil::getDataDir());
    qDebug()<<"chart list file name:"<<ChartListFileName;
    //      Establish the GSHHS Dataset location
    gDefaultWorldMapLocation = QString("%1/gshhs/").arg(zchxFuncUtil::getDataDir());
    qDebug()<<"default world map location:"<<gDefaultWorldMapLocation;
    if( gWorldMapLocation.isEmpty() ) {
        qDebug()<<"word map location is empty, reset to default.";
        gWorldMapLocation = gDefaultWorldMapLocation;
    }
    qDebug()<<"world map location:"<<gWorldMapLocation;
    //   Build the initial chart dir array
    ArrayOfCDI ChartDirArray;
    ZCHX_CFG_INS->LoadChartDirArray( ChartDirArray );

    if( !ChartDirArray.count() )
    {
        if(QFile::exists(ChartListFileName )) QFile::remove(ChartListFileName );
    }

//    ChartData = new ChartDB( );
//    if (!ChartData->LoadBinary(ChartListFileName, ChartDirArray)) {
//        g_bNeedDBUpdate = true;
//    }

#if 0
    gFrame = this;
    g_Main_thread = QThread::currentThread();
    // Instantiate the global OCPNPlatform class
    g_Platform = new OCPNPlatform;

    //  Perform first stage initialization
    OCPNPlatform::Initialize_1( );

#if wxCHECK_VERSION(3,0,0)
    // Set the name of the app as displayed to the user.
    // This is necessary at least on OS X, for the capitalisation to be correct in the system menus.
    MyApp::SetAppDisplayName("OpenCPN");
#endif


    //  Seed the random number generator
    wxDateTime x = wxDateTime::UNow();
    long seed = x.GetMillisecond();
    seed *= x.GetTicks();
    srand(seed);


//Fulup: force floating point to use dot as separation.
// This needs to be set early to catch numerics in config file.
    setlocale( LC_NUMERIC, "C" );



    g_start_time = wxDateTime::Now();

    g_loglast_time = g_start_time;
    g_loglast_time.MakeGMT();
    g_loglast_time.Subtract( wxTimeSpan( 0, 29, 0, 0 ) ); // give 1 minute for GPS to get a fix

    AnchorPointMinDist = 5.0;


//      Init the private memory manager
    malloc_max = 0;

    //      Record initial memory status
    GetMemoryStatus( &g_mem_total, &g_mem_initial );


// Set up default FONT encoding, which should have been done by wxWidgets some time before this......
    wxFont temp_font( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, FALSE, wxString( _T("") ),
            wxFONTENCODING_SYSTEM );
    temp_font.SetDefaultEncoding( wxFONTENCODING_SYSTEM );


    //      Establish Log File location
    if(!g_Platform->InitializeLogFile())
        return false;


#ifdef __WXMSW__

    //  Un-comment the following to establish a separate console window as a target for printf() in Windows
    //     RedirectIOToConsole();

#endif

//      Send init message
    ZCHX_LOGMSG( _T("\n\n________\n") );

    wxDateTime date_now = wxDateTime::Now();

    wxString imsg = date_now.FormatISODate();
    ZCHX_LOGMSG( imsg );

    imsg = _T(" ------- Starting OpenCPN -------");
    ZCHX_LOGMSG( imsg.c_str());

    wxString version = VERSION_FULL;
    wxString vs = version.Trim( true );
    vs = vs.Trim( false );
    ZCHX_LOGMSG( vs );

    wxString wxver(wxVERSION_STRING);
    wxver.Prepend( _T("wxWidgets version: ") );

    wxPlatformInfo platforminfo = wxPlatformInfo::Get();

    wxString os_name;
#ifndef __OCPN__ANDROID__
    os_name = platforminfo.GetOperatingSystemIdName();
#else
    os_name = platforminfo.GetOperatingSystemFamilyName();
#endif

    wxString platform = os_name + _T(" ") +
    platforminfo.GetArchName()+ _T(" ") +
    platforminfo.GetPortIdName();

    ZCHX_LOGMSG( wxver + _T(" ") + platform );

    ::wxGetOsVersion(&osMajor, &osMinor);
    wxString osVersionMsg;
    osVersionMsg.Printf(_T("OS Version reports as:  %d.%d"), osMajor, osMinor);
    ZCHX_LOGMSG(osVersionMsg);

    ZCHX_LOGMSG(wxString::Format( _T("MemoryStatus:  mem_total: %d mb,  mem_initial: %d mb"), g_mem_total / 1024,
            g_mem_initial / 1024 ));

    //    Initialize embedded PNG icon graphics
    ::wxInitAllImageHandlers();


    imsg = _T("SData_Locn is ");
    imsg += g_Platform->GetSharedDataDir();
    ZCHX_LOGMSG( imsg );

#ifdef __OCPN__ANDROID__
    //  Now we can load a Qt StyleSheet, if present
    wxString style_file = g_Platform->GetSharedDataDir();
    style_file += _T("styles");
    appendOSDirSlash( &style_file );
    style_file += _T("qtstylesheet.qss");
    if(LoadQtStyleSheet(style_file)){
        wxString smsg = _T("Loaded Qt Stylesheet: ") + style_file;
        ZCHX_LOGMSG( smsg );
    }
    else{
        wxString smsg = _T("Qt Stylesheet not found: ") + style_file;
        ZCHX_LOGMSG( smsg );
    }
#endif

    //      Create some static strings
    pInit_Chart_Dir = new wxString();

    //  Establish an empty ChartCroupArray
    g_pGroupArray = new ChartGroupArray;


    imsg = _T("PrivateDataDir is ");
    imsg += g_Platform->GetPrivateDataDir();
    ZCHX_LOGMSG( imsg );


//      Create an array string to hold repeating messages, so they don't
//      overwhelm the log
    pMessageOnceArray = new wxArrayString;

//      Init the Route Manager
    g_pRouteMan = new Routeman( this );

    //      Init the Selectable Route Items List
    pSelect = new Select();
    pSelect->SetSelectPixelRadius( 12 );

    //      Init the Selectable Tide/Current Items List
    pSelectTC = new Select();
    //  Increase the select radius for tide/current stations
    pSelectTC->SetSelectPixelRadius( 25 );

    //      Init the Selectable AIS Target List
    pSelectAIS = new Select();
    pSelectAIS->SetSelectPixelRadius( 12 );

//      Initially AIS display is always on
    g_bShowAIS = true;
    g_pais_query_dialog_active = NULL;

//      Who am I?
    phost_name = new wxString( ::wxGetHostName() );

//      Initialize connection parameters array
    g_pConnectionParams = new wxArrayOfConnPrm();

//      Initialize some lists
    //    Layers
    pLayerList = new LayerList;
    //  Routes
    pRouteList = new RouteList;
    // Tracks
    pTrackList = new TrackList;


//      (Optionally) Capture the user and file(effective) ids
//  Some build environments may need root privileges for hardware
//  port I/O, as in the NMEA data input class.  Set that up here.

#ifndef __WXMSW__
#ifdef PROBE_PORTS__WITH_HELPER
    user_user_id = getuid ();
    file_user_id = geteuid ();
#endif
#endif


    bool b_initial_load = false;

    wxFileName config_test_file_name( g_Platform->GetConfigFileName() );
    if( config_test_file_name.FileExists() ) ZCHX_LOGMSG(
        _T("Using existing Config_File: ") + g_Platform->GetConfigFileName() );
    else {
        {
            ZCHX_LOGMSG( _T("Creating new Config_File: ") + g_Platform->GetConfigFileName() );

            b_initial_load = true;

            if( true != config_test_file_name.DirExists( config_test_file_name.GetPath() ) )
                if( !config_test_file_name.Mkdir(config_test_file_name.GetPath() ) )
                    ZCHX_LOGMSG( _T("Cannot create config file directory for ") + g_Platform->GetConfigFileName() );
        }
    }

    //      Open/Create the Config Object
    pConfig = g_Platform->GetConfigObject();
    pConfig->LoadMyConfig();

    //  Override for some safe and nice default values if the config file was created from scratch
    if(b_initial_load)
        g_Platform->SetDefaultOptions();

    g_Platform->applyExpertMode(g_bUIexpert);

    // Now initialize UI Style.
    g_StyleManager = new ocpnStyle::StyleManager();

//     if(g_useMUI)
//         g_uiStyle = _T("MUI_flat");

    g_StyleManager->SetStyle( _T("MUI_flat") );
    if( !g_StyleManager->IsOK() ) {
        wxString msg = _("Failed to initialize the user interface. ");
        msg << _("OpenCPN cannot start. ");
        msg << _("The necessary configuration files were not found. ");
        msg << _("See the log file at ") << g_Platform->GetLogFileName() << _(" for details.") << _T("\n\n");
        msg << g_Platform->GetSharedDataDir();

        wxMessageDialog w( NULL, msg, _("Failed to initialize the user interface. "),
                           wxCANCEL | wxICON_ERROR );
        w.ShowModal();
        exit( EXIT_FAILURE );
    }

    if(g_useMUI){
        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        style->chartStatusWindowTransparent = true;
    }


    //      Init the WayPoint Manager
    pWayPointMan = NULL;

    g_display_size_mm = wxMax(100, g_Platform->GetDisplaySizeMM());
    wxString msg;
    msg.Printf(_T("Detected display size (horizontal): %d mm"), (int) g_display_size_mm);
    ZCHX_LOGMSG(msg);

    // User override....
    if((g_config_display_size_mm > 0) &&(g_config_display_size_manual)){
        g_display_size_mm = g_config_display_size_mm;
        wxString msg;
        msg.Printf(_T("Display size (horizontal) config override: %d mm"), (int) g_display_size_mm);
        ZCHX_LOGMSG(msg);
        g_Platform->SetDisplaySizeMM(g_display_size_mm);
    }

    if(g_btouch){
        int SelectPixelRadius = 50;

        pSelect->SetSelectPixelRadius(SelectPixelRadius);
        pSelectTC->SetSelectPixelRadius( wxMax(25, SelectPixelRadius) );
        pSelectAIS->SetSelectPixelRadius(SelectPixelRadius);
    }


    //        Is this the first run after a clean install?
    if( !n_NavMessageShown ) g_bFirstRun = true;

    //  Now we can set the locale
    //    using wxWidgets/gettext methodology....


#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)
    if( lang_list[0] ) {};                 // silly way to avoid compiler warnings


    //  Where are the opencpn.mo files?
    g_Platform->SetLocaleSearchPrefixes();

    wxString def_lang_canonical = g_Platform->GetDefaultSystemLocale();

    imsg = _T("System default Language:  ") + def_lang_canonical;
    ZCHX_LOGMSG( imsg );

    wxString cflmsg = _T("Config file language:  ") + g_locale;
    ZCHX_LOGMSG( cflmsg );

    //  Make any adjustments necessary
    g_locale = g_Platform->GetAdjustedAppLocale();
    cflmsg = _T("Adjusted App language:  ") + g_locale;
    ZCHX_LOGMSG( cflmsg );


    // Set the desired locale
    g_Platform->ChangeLocale(g_locale, plocale_def_lang, &plocale_def_lang);

    imsg = _T("Opencpn language set to:  ");
    imsg += g_locale;
    ZCHX_LOGMSG( imsg );

    //  French language locale is assumed to include the AZERTY keyboard
    //  This applies to either the system language, or to OpenCPN language selection
    if( g_locale == _T("fr_FR") ) g_b_assume_azerty = true;
#else
    ZCHX_LOGMSG( _T("wxLocale support not available") );
#endif

    // Instantiate and initialize the Config Manager
    ConfigMgr::Get();

    // Is this an upgrade?
    g_bUpgradeInProcess = (vs != g_config_version_string);

//  Send the Welcome/warning message if it has never been sent before,
//  or if the version string has changed at all
//  We defer until here to allow for localization of the message
    if( !n_NavMessageShown || ( vs != g_config_version_string ) ) {
        if( wxID_CANCEL == ShowNavWarning() )
            return false;
        n_NavMessageShown = 1;
    }

    g_config_version_string = vs;

    //  log deferred log restart message, if it exists.
    if( !g_Platform->GetLargeLogMessage().IsEmpty() )
        ZCHX_LOGMSG( g_Platform->GetLargeLogMessage() );

    //  Validate OpenGL functionality, if selected
#ifdef ocpnUSE_GL

#ifdef __WXMSW__
#if !wxCHECK_VERSION(2, 9, 0)           // The OpenGL test app only runs on wx 2.8, unavailable on wx3.x

    if( /*g_bopengl &&*/ !g_bdisable_opengl ) {
        wxFileName fn(g_Platform->GetExePath());
        bool b_test_result = TestGLCanvas(fn.GetPathWithSep() );

        if( !b_test_result )
            ZCHX_LOGMSG( _T("OpenGL disabled due to test app failure.") );

        g_bdisable_opengl = !b_test_result;
    }
#endif
#endif

#else
    g_bdisable_opengl = true;;
#endif

    if(g_bdisable_opengl)
        g_bopengl = false;

#if defined(__UNIX__) && !defined(__OCPN__ANDROID__) && !defined(__WXOSX__)
    if(g_bSoftwareGL)
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
#endif

    g_bTransparentToolbarInOpenGLOK = isTransparentToolbarInOpenGLOK();

    // On Windows platforms, establish a default cache managment policy
    // as allowing OpenCPN a percentage of available physical memory,
    // not to exceed 1 GB
    // Note that this logic implies that Windows platforms always use
    // the memCacheLimit policy, and never use the fallback nCacheLimit policy
#ifdef __WXMSW__
    if( 0 == g_memCacheLimit )
        g_memCacheLimit = (int) ( g_mem_total * 0.5 );
    g_memCacheLimit = wxMin(g_memCacheLimit, 1024 * 1024); // math in kBytes, Max is 1 GB
#else
    if( 0 ==  g_nCacheLimit && 0 == g_memCacheLimit ){
        g_memCacheLimit = (int) ( (g_mem_total - g_mem_initial) * 0.5 );
        g_memCacheLimit = wxMin(g_memCacheLimit, 1024 * 1024); // Max is 1 GB if unspecified
#ifdef __WXMAC__
        if( g_mem_total > 8192 * 1024) {
            g_memCacheLimit = 1024 * 1024;
        } else if( g_mem_total > 4096 * 1024) {
            g_memCacheLimit = 600 * 1024;
        } else {
            g_memCacheLimit = 400 * 1024;
        }
#endif
    }
#endif
    if( 0 ==  g_nCacheLimit)
        g_nCacheLimit = CACHE_N_LIMIT_DEFAULT;
#ifdef __OCPN__ANDROID__
    g_memCacheLimit = 100 * 1024;
#endif

//      Establish location and name of chart database
    ChartListFileName = newPrivateFileName(g_Platform->GetPrivateDataDir(), "chartlist.dat", "CHRTLIST.DAT");

//      Establish location and name of AIS MMSI -> Target Name mapping
    AISTargetNameFileName = newPrivateFileName(g_Platform->GetPrivateDataDir(), "mmsitoname.csv", "MMSINAME.CSV");

//      Establish guessed location of chart tree
    if( pInit_Chart_Dir->IsEmpty() ) {
        wxStandardPaths& std_path = g_Platform->GetStdPaths();

        if( !g_bportable )
#ifndef __OCPN__ANDROID__
        pInit_Chart_Dir->Append( std_path.GetDocumentsDir() );
#else
        pInit_Chart_Dir->Append( g_Platform->GetPrivateDataDir() );
#endif
    }

//      Establish the GSHHS Dataset location
    gDefaultWorldMapLocation = "gshhs";
    gDefaultWorldMapLocation.Prepend( g_Platform->GetSharedDataDir() );
    gDefaultWorldMapLocation.Append( wxFileName::GetPathSeparator() );
    if( gWorldMapLocation == wxEmptyString ) {
        gWorldMapLocation = gDefaultWorldMapLocation;
    }

    //  Check the global Tide/Current data source array
    //  If empty, preset one default (US) Ascii data source
    wxString default_tcdata =  ( g_Platform->GetSharedDataDir() + _T("tcdata") +
             wxFileName::GetPathSeparator() + _T("HARMONIC.IDX"));

    if(!TideCurrentDataSet.GetCount()) {
        TideCurrentDataSet.Add(g_Platform->NormalizePath(default_tcdata) );
    }
    else {
        wxString first_tide = TideCurrentDataSet[0];
        wxFileName ft(first_tide);
        if(!ft.FileExists()){
            TideCurrentDataSet.RemoveAt(0);
            TideCurrentDataSet.Insert(g_Platform->NormalizePath(default_tcdata), 0 );
        }
    }


    //  Check the global AIS alarm sound file
    //  If empty, preset default
    if(g_sAIS_Alert_Sound_File.IsEmpty()) {
        wxString default_sound =  ( g_Platform->GetSharedDataDir() + _T("sounds") +
        wxFileName::GetPathSeparator() +
        _T("2bells.wav"));
        g_sAIS_Alert_Sound_File = g_Platform->NormalizePath(default_sound);
    }

    gpIDX = NULL;
    gpIDXn = 0;

    g_Platform->Initialize_2();

//  Set up the frame initial visual parameters
//      Default size, resized later
    wxSize new_frame_size( -1, -1 );
    int cx, cy, cw, ch;
    ::wxClientDisplayRect( &cx, &cy, &cw, &ch );

    InitializeUserColors();

    if( ( g_nframewin_x > 100 ) && ( g_nframewin_y > 100 ) && ( g_nframewin_x <= cw )
            && ( g_nframewin_y <= ch ) ) new_frame_size.Set( g_nframewin_x, g_nframewin_y );
    else
        new_frame_size.Set( cw * 7 / 10, ch * 7 / 10 );

    //  Try to detect any change in physical screen configuration
    //  This can happen when drivers are changed, for instance....
    //  and can confuse the WUI layout perspective stored in the config file.
    //  If detected, force a nominal window size and position....
    if( ( g_lastClientRectx != cx ) || ( g_lastClientRecty != cy ) || ( g_lastClientRectw != cw )
            || ( g_lastClientRecth != ch ) ) {
        new_frame_size.Set( cw * 7 / 10, ch * 7 / 10 );
        g_bframemax = false;
    }

    g_lastClientRectx = cx;
    g_lastClientRecty = cy;
    g_lastClientRectw = cw;
    g_lastClientRecth = ch;

    //  Validate config file position
    wxPoint position( 0, 0 );
    wxSize dsize = wxGetDisplaySize();

#ifdef __WXMAC__
    g_nframewin_posy = wxMax(g_nframewin_posy, 22);
#endif

    if( ( g_nframewin_posx < dsize.x ) && ( g_nframewin_posy < dsize.y ) ) position = wxPoint(
            g_nframewin_posx, g_nframewin_posy );

#ifdef __WXMSW__
    //  Support MultiMonitor setups which an allow negative window positions.
    RECT frame_rect;
    frame_rect.left = position.x;
    frame_rect.top = position.y;
    frame_rect.right = position.x + new_frame_size.x;
    frame_rect.bottom = position.y + new_frame_size.y;

    //  If the requested frame window does not intersect any installed monitor,
    //  then default to simple primary monitor positioning.
    if( NULL == MonitorFromRect( &frame_rect, MONITOR_DEFAULTTONULL ) ) position = wxPoint( 10,
            10 );
#endif

#ifdef __OCPN__ANDROID__
    wxSize asz = getAndroidDisplayDimensions();
    ch = asz.y;
    cw = asz.x;
//    qDebug() << cw << ch;

    if((cw > 200) && (ch > 200) )
        new_frame_size.Set( cw, ch );
    else
        new_frame_size.Set( 800, 400 );
#endif

    //  For Windows and GTK, provide the expected application Minimize/Close bar
    long app_style = wxDEFAULT_FRAME_STYLE;
    app_style |= wxWANTS_CHARS;

// Create the main frame window
    wxString myframe_window_title = wxString::Format(wxT("OpenCPN %s"),
            VERSION_FULL); //Gunther

    if( g_bportable ) {
        myframe_window_title += _(" -- [Portable(-p) executing from ");
        myframe_window_title += g_Platform->GetHomeDir();
        myframe_window_title += _T("]");
    }

    wxString fmsg;
    fmsg.Printf(_T("Creating MyFrame...size(%d, %d)  position(%d, %d)"), new_frame_size.x, new_frame_size.y, position.x, position.y);
    ZCHX_LOGMSG(fmsg);

    gFrame = new MyFrame( NULL, myframe_window_title, position, new_frame_size, app_style ); //Gunther

    //  Do those platform specific initialization things that need gFrame
    g_Platform->Initialize_3();


//  Initialize the Plugin Manager
    g_pi_manager = new PlugInManager( gFrame );

    //g_pauimgr = new wxAuiManager;
    g_pauimgr = new OCPN_AUIManager;
    g_pauidockart= new wxAuiDefaultDockArt;
    g_pauimgr->SetArtProvider(g_pauidockart);
    g_pauimgr->SetDockSizeConstraint(.9, .9);

    //g_pauimgr->SetFlags(g_pauimgr->GetFlags() | wxAUI_MGR_LIVE_RESIZE);

    g_grad_default = g_pauidockart->GetMetric(wxAUI_DOCKART_GRADIENT_TYPE);
    g_border_color_default = g_pauidockart->GetColour(wxAUI_DOCKART_BORDER_COLOUR );
    g_border_size_default = g_pauidockart->GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE );
    g_sash_size_default = g_pauidockart->GetMetric(wxAUI_DOCKART_SASH_SIZE);
    g_caption_color_default = g_pauidockart->GetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR);
    g_sash_color_default = g_pauidockart->GetColour(wxAUI_DOCKART_SASH_COLOUR );
    g_background_color_default = g_pauidockart->GetColour(wxAUI_DOCKART_BACKGROUND_COLOUR );



// tell wxAuiManager to manage the frame
    g_pauimgr->SetManagedWindow( gFrame );

    gFrame->CreateCanvasLayout();

    //gFrame->RequestNewMasterToolbar( true );

    gFrame->SetChartUpdatePeriod();             // Reasonable default

    gFrame->Enable();

    gFrame->GetPrimaryCanvas()->SetFocus();

    pthumbwin = new ThumbWin( gFrame->GetPrimaryCanvas() );

    gFrame->ApplyGlobalSettings( false );               // done once on init with resize


    gFrame->SetAllToolbarScale();


// Show the frame

    gFrame->Show( TRUE );

#ifdef __OCPN__ANDROID__
    androidShowBusyIcon();
#endif

    gFrame->SetAndApplyColorScheme( global_color_scheme );

    if( g_bframemax ) gFrame->Maximize( true );

    if( g_bresponsive  && ( gFrame->GetPrimaryCanvas()->GetPixPerMM() > 4.0))
        gFrame->Maximize( true );

    //  Yield to pick up the OnSize() calls that result from Maximize()
    Yield();

    bool b_SetInitialPoint = false;

    //   Build the initial chart dir array
    ArrayOfCDI ChartDirArray;
    pConfig->LoadChartDirArray( ChartDirArray );

    //  Windows installer may have left hints regarding the initial chart dir selection
#ifdef __WXMSW__
    if( g_bFirstRun && (ChartDirArray.GetCount() == 0) ) {
        int ndirs = 0;

        wxRegKey RegKey( wxString( _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\OpenCPN") ) );
        if( RegKey.Exists() ) {
            ZCHX_LOGMSG( _("Retrieving initial Chart Directory set from Windows Registry") );
            wxString dirs;
            RegKey.QueryValue( wxString( _T("ChartDirs") ), dirs );

            wxStringTokenizer tkz( dirs, _T(";") );
            while( tkz.HasMoreTokens() ) {
                wxString token = tkz.GetNextToken();

                ChartDirInfo cdi;
                cdi.fullpath = token.Trim();
                cdi.magic_number = _T("");

                ChartDirArray.Add( cdi );
                ndirs++;
            }

        }

        if (g_bportable)
        {
            ChartDirInfo cdi;
            cdi.fullpath =_T("charts");
            cdi.fullpath.Prepend(g_Platform->GetSharedDataDir());
            cdi.magic_number = _T("");
            ChartDirArray.Add(cdi);
            ndirs++;
        }

        if( ndirs ) pConfig->UpdateChartDirs( ChartDirArray );

        //    As a favor to new users, poll the database and
        //    move the initial viewport so that a chart will come up.
        if( ndirs ) b_SetInitialPoint = true;

    }
#endif

//    If the ChartDirArray is empty at this point, any existing chart database file must be declared invalid,
//    So it is best to simply delete it if present.
//    TODO  There is a possibility of recreating the dir list from the database itself......

    if( !ChartDirArray.GetCount() )
        if(::wxFileExists( ChartListFileName ))
            ::wxRemoveFile( ChartListFileName );

//      Try to load the current chart list Data file
    ChartData = new ChartDB( );
    if (!ChartData->LoadBinary(ChartListFileName, ChartDirArray)) {
        g_bNeedDBUpdate = true;
    }


    //  Verify any saved chart database startup index
    if(g_restore_dbindex >= 0){
        if(ChartData->GetChartTableEntries() == 0)
            g_restore_dbindex = -1;

        else if(g_restore_dbindex > (ChartData->GetChartTableEntries()-1))
            g_restore_dbindex = 0;
    }

    //  Apply the inital Group Array structure to the chart data base
    ChartData->ApplyGroupArray( g_pGroupArray );

//      All set to go.....

    // Process command line option to rebuild cache
#ifdef ocpnUSE_GL
extern ocpnGLOptions g_GLOptions;

    if(g_rebuild_gl_cache && g_bopengl &&
        g_GLOptions.m_bTextureCompression && g_GLOptions.m_bTextureCompressionCaching ) {

        gFrame->ReloadAllVP();                  //  Get a nice chart background loaded

        //      Turn off the toolbar as a clear signal that the system is busy right now.
        // Note: I commented this out because the toolbar never comes back for me
        // and is unusable until I restart opencpn without generating the cache
//        if( g_MainToolbar )
//            g_MainToolbar->Hide();

        if(g_glTextureManager)
            g_glTextureManager->BuildCompressedCache();

    }
#endif

    if(g_parse_all_enc )
        ParseAllENC(gFrame);

//      establish GPS timeout value as multiple of frame timer
//      This will override any nonsense or unset value from the config file
    if( ( gps_watchdog_timeout_ticks > 60 ) || ( gps_watchdog_timeout_ticks <= 0 ) ) gps_watchdog_timeout_ticks =
            ( GPS_TIMEOUT_SECONDS * 1000 ) / TIMER_GFRAME_1;

    wxString dogmsg;
    dogmsg.Printf( _T("GPS Watchdog Timeout is: %d sec."), gps_watchdog_timeout_ticks );
    ZCHX_LOGMSG( dogmsg );

    sat_watchdog_timeout_ticks = 12;

    gGPS_Watchdog = 2;
    gHDx_Watchdog = 2;
    gHDT_Watchdog = 2;
    gSAT_Watchdog = 2;
    gVAR_Watchdog = 2;

    //  Most likely installations have no ownship heading information
    g_bHDT_Rx = false;
    g_bVAR_Rx = false;

//  Start up a new track if enabled in config file
    if( g_bTrackCarryOver )
        g_bDeferredStartTrack = true;

    pAnchorWatchPoint1 = NULL;
    pAnchorWatchPoint2 = NULL;

    Yield();

    gFrame->DoChartUpdate();

    FontMgr::Get().ScrubList(); // Clean the font list, removing nonsensical entries


    gFrame->ReloadAllVP();                  // once more, and good to go


    gFrame->Refresh( false );
    gFrame->Raise();

    gFrame->GetPrimaryCanvas()->Enable();
    gFrame->GetPrimaryCanvas()->SetFocus();

    //  This little hack fixes a problem seen with some UniChrome OpenGL drivers
    //  We need a deferred resize to get glDrawPixels() to work right.
    //  So we set a trigger to generate a resize after 5 seconds....
    //  See the "UniChrome" hack elsewhere
#ifdef ocpnUSE_GL
    if ( !g_bdisable_opengl )
    {
        glChartCanvas *pgl = (glChartCanvas *) gFrame->GetPrimaryCanvas()->GetglCanvas();
        if( pgl && ( pgl->GetRendererString().Find( _T("UniChrome") ) != wxNOT_FOUND ) )
        {
            gFrame->m_defer_size = gFrame->GetSize();
            gFrame->SetSize( gFrame->m_defer_size.x - 10, gFrame->m_defer_size.y );
            g_pauimgr->Update();
            gFrame->m_bdefer_resize = true;
        }
    }
#endif

    if ( g_start_fullscreen )
        gFrame->ToggleFullScreen();

#ifdef __OCPN__ANDROID__
    //  We need a resize to pick up height adjustment after building android ActionBar
    if(pConfig->m_bShowMenuBar)
        gFrame->SetSize(getAndroidDisplayDimensions());
    androidSetFollowTool(gFrame->GetPrimaryCanvas()->m_bFollow);
#endif

    gFrame->Raise();
    gFrame->GetPrimaryCanvas()->Enable();
    gFrame->GetPrimaryCanvas()->SetFocus();

#ifdef __WXQT__
    if(gFrame->GetPrimaryCanvas() && gFrame->GetPrimaryCanvas()->GetToolbar())
        gFrame->GetPrimaryCanvas()->GetToolbar()->Raise();
#endif

    // Setup Tides/Currents to settings present at last shutdown
// TODO
//     gFrame->ShowTides( g_bShowTide );
//     gFrame->ShowCurrents( g_bShowCurrent );

//      Start up the ticker....
    gFrame->FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

//      Start up the ViewPort Rotation angle Averaging Timer....
    gFrame->FrameCOGTimer.Start( 10, wxTIMER_CONTINUOUS );

    // Start delayed initialization chain after 100 milliseconds
    gFrame->InitTimer.Start( 100, wxTIMER_CONTINUOUS );

    ZCHX_LOGMSG( wxString::Format(_("OpenCPN Initialized in %ld ms."), init_sw.Time() ) );

    OCPNPlatform::Initialize_4( );

    if( n_NavMessageShown == 1 ) {
        //In case the user accepted the "not for navigation" nag, persist it here...
        pConfig->UpdateSettings();
    }
#ifdef __OCPN__ANDROID__
    androidHideBusyIcon();
#endif

    g_pauimgr->Update();

    return TRUE;
#endif
}

