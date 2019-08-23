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
#include "s52plib.h"
#include "OCPNPlatform.h"
#include "S57ClassRegistrar.h"
#include "s57RegistrarMgr.h"
#include "glChartCanvas.h"


//MyFrame                   *gFrame;

//ConsoleCanvas             *console;

//MyConfig                  *pConfig;

//ChartBase                 *Current_Vector_Ch;
ChartDB                   *ChartData = NULL;
arrayofCanvasPtr            g_canvasArray;
ColorScheme               global_color_scheme = GLOBAL_COLOR_SCHEME_DAY;
static int                Usercolortable_index;
static wxArrayPtrVoid     *UserColorTableArray;
static wxArrayPtrVoid     *UserColourHashTableArray;
static QColorHashMap     *pcurrent_user_color_hash;
SENCThreadManager *g_SencThreadManager;
s52plib                   *ps52plib;
QString                  g_csv_locn;
OCPNPlatform                *g_Platform;
QString                  g_SENCPrefix;
QString                  g_UserPresLibData;
S57ClassRegistrar         *g_poRegistrar;
s57RegistrarMgr           *m_pRegistrarMan;
zchxConfig                *pConfig;
extern zchxMapMainWindow*  gFrame;
int                       g_nDepthUnitDisplay;



zchxMapMainWindow::zchxMapMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::zchxMapMainWindow),
    mChartDB(new ChartDB()),
    mOptionDlg(0),
    mConfigObj(new zchxConfig),
    FrameTimer1(0)
{
    ChartData = mChartDB;
    ui->setupUi(this);
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

    mMonitorTimer = new QTimer();
    mMonitorTimer->setInterval(30000);
    connect(mMonitorTimer, SIGNAL(timeout()), this, SLOT(slotMemoryMonitor()));
    mMonitorTimer->start();

    //窗口刷新
    FrameTimer1 = new QTimer(this);
    FrameTimer1->setInterval(TIMER_GFRAME_1);
    connect(FrameTimer1, SIGNAL(timeout()), this, SLOT(slotOnFrameTimer1Out()));
}

zchxMapMainWindow::~zchxMapMainWindow()
{
    delete ui;
}


void zchxMapMainWindow::setApplicationName(const QString &name)
{
    mApplicationName = name;
}

quint64 zchxMapMainWindow::getProcessIDFromSystem()
{
#if 1
    return GetCurrentProcessId();
#else
    quint64 id = 0;
    HANDLE    hToolHelp32Snapshot;
    hToolHelp32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32    pe = { sizeof(PROCESSENTRY32) };
    BOOL  isSuccess = Process32First(hToolHelp32Snapshot, &pe);
    while (isSuccess)
    {
        size_t len = WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, wcslen(pe.szExeFile), NULL, 0, NULL, NULL);
        char *des = (char *)malloc(sizeof(char) * (len + 1));
        WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, wcslen(pe.szExeFile), des, len, NULL, NULL);
        des[len] = '\0';
        if (!strcmp(des, mApplicationName.toStdString().c_str()))
        {
            free(des);
            id = pe.th32ProcessID;
            break;
        }
        free(des);
        isSuccess = Process32Next(hToolHelp32Snapshot, &pe);
    }
    CloseHandle(hToolHelp32Snapshot);
    return id;
#endif
}

int zchxMapMainWindow::GetApplicationMemoryUse( void )
{
    int memsize = -1;
#if 0
    if(mApplicationName.size() == 0 && mProcessedID == 0)
    {
        qDebug()<<"application memsize cannot get for name not set yet.";
        return memsize;
    }
    if(mProcessedID == 0)
    {
        //先获取当前的进程号
        qDebug()<<"now start to get process id of application name:"<<mApplicationName;
        quint64 id = getProcessIDFromSystem();
        qDebug()<<"get process id from system is:"<<id;
        if(id <= 0) return memsize;
        mProcessedID = id;
    }


    HANDLE hProcess/* = GetCurrentProcess()*/;
    hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processID );
#else
    HANDLE hProcess = GetCurrentProcess();
#endif
    if( NULL == hProcess ) return 0;
    PROCESS_MEMORY_COUNTERS pmc;

    if( GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) ) {
        memsize = pmc.WorkingSetSize / 1024;
    }

    CloseHandle( hProcess );
    return memsize;
}

void  zchxMapMainWindow::getMemoryStatus(int* total, int* used)
{
    mMemUsed = GetApplicationMemoryUse();
    if(mMemTotal == 0)
    {
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof( statex );
        GlobalMemoryStatusEx( &statex );
        mMemTotal = statex.ullTotalPhys / 1024;
    }
    if(total) *total = mMemTotal;
    if(used) *used = mMemUsed;
    qDebug()<<"memory total:"<<mMemTotal<<"  app used:"<<mMemUsed;
}

void zchxMapMainWindow::slotOpenSettingDlg()
{
    qDebug()<<"open settings windows now";
    if(!mOptionDlg) mOptionDlg = new zchxOptionsDlg(this);
    mOptionDlg->show();
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

void zchxMapMainWindow::startFrameTimer1()
{
    if(FrameTimer1) FrameTimer1->start();
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
                wxString msg;
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
                wxString day = lognow.FormatISODate();
                wxString utc = lognow.FormatISOTime();
                wxString navmsg = _T("LOGBOOK:  ");
                navmsg += day;
                navmsg += _T(" ");
                navmsg += utc;
                navmsg += _T(" UTC ");

                if( bGPSValid ) {
                    wxString data;
                    data.Printf( _T(" GPS Lat %10.5f Lon %10.5f "), gLat, gLon );
                    navmsg += data;

                    wxString cog;
                    if( std::isnan(gCog) ) cog.Printf( _T("COG ----- ") );
                    else
                        cog.Printf( _T("COG %10.5f "), gCog );

                    wxString sog;
                    if( std::isnan(gSog) ) sog.Printf( _T("SOG -----  ") );
                    else
                        sog.Printf( _T("SOG %6.2f ") + getUsrSpeedUnit(), toUsrSpeed( gSog ) );

                    navmsg += cog;
                    navmsg += sog;
                } else {
                    wxString data;
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
            wxString sogcog( _T("SOG --- ") + getUsrSpeedUnit() + + _T("     ") + _T(" COG ---\u00B0") );
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
        wxString longmsg = _("OpenCPN Chart Update");
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
    wxString gshhg_chart_loc = gWorldMapLocation;
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


void zchxMapMainWindow::DoStackDown( ChartCanvas *cc )
{
    DoStackDelta( cc, -1 );
}

void zchxMapMainWindow::DoStackUp( ChartCanvas *cc )
{
    DoStackDelta( cc, 1 );
}

void zchxMapMainWindow::DoStackDelta( ChartCanvas *cc, int direction )
{
    if(cc){
        cc->DoCanvasStackDelta( direction );
    }
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

    // ..For each canvas...
    for(unsigned int i=0 ; i < g_canvasArray.count() ; i++){
        ChartCanvas *cc = g_canvasArray.at(i);
        if(cc)
            return_val |= cc->DoCanvasUpdate();
    }

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
    if(g_canvasArray.count() > 0)
        return g_canvasArray.at(0);
    else
        return NULL;
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

double zchxMapMainWindow::GetBestVPScale( ChartBase *pchart )
{
    return GetPrimaryCanvas()->GetBestVPScale( pchart );
}

void zchxMapMainWindow::RefreshAllCanvas( bool bErase)
{
    // For each canvas
    for(unsigned int i=0 ; i < g_canvasArray.count() ; i++){
        ChartCanvas *cc = g_canvasArray.at(i);
        if(cc){
            cc->Refresh( bErase );
        }
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

QColor GetGlobalColor(QString colorName)
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


void LoadS57()
{
    if(ps52plib) // already loaded?
        return;

    //  Start a SENC Thread manager
    g_SencThreadManager = new SENCThreadManager();

//      Set up a useable CPL library error handler for S57 stuff
    CPLSetErrorHandler( MyCPLErrorHandler );

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
     wxString GetUserDataDir() const
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

        pConfig->LoadS57Config();
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
}

