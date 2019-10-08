/***************************************************************************
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
#include "OCPNRegion.h"
#include "gshhs.h"
#include "CanvasConfig.h"
//#include "CanvasOptions.h"
#include "mbtiles.h"
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
#include "glChartCanvas.h"


extern float  g_ChartScaleFactorExp;
extern float  g_ShipScaleFactorExp;
QString                             g_chartListFileName;
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
extern QString          gWorldMapLocation;
extern QString          gDefaultWorldMapLocation;

//extern ConsoleCanvas    *console;

//extern zchxConfig         *pConfig;
//extern Select           *pSelect;
//extern ThumbWin         *pthumbwin;
//extern TCMgr            *ptcmgr;
extern bool             g_bConfirmObjectDelete;

//extern IDX_entry        *gpIDX;
extern int               gpIDXn;

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
static bool mouse_leftisdown;

bool g_brouteCreating;
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
extern unsigned int     g_canvasConfig;
extern QString         g_lastPluginMessage;

extern float            g_toolbar_scalefactor;
extern SENCThreadManager *g_SencThreadManager;
extern s57RegistrarMgr          *m_pRegistrarMan;

// "Curtain" mode parameters
QDialog                *g_pcurtain;
ChartFrameWork          *gChartFrameWork = 0;



// Define a constructor for my canvas
ChartFrameWork::ChartFrameWork( glChartCanvas *frame ) : QObject(0) , mGLCC(frame)
{
    gChartFrameWork = this;
    m_groupIndex = 0;
    m_bzooming = false;
    m_disable_edge_pan = false;
    m_bautofind = false;
    m_bFirstAuto = true;
    m_groupIndex = 0;
    m_singleChart = NULL;
    
    m_vLat = 35.7421999999;
    m_vLon = 127.52430000;


    m_zoom_factor = 1;

    mViewPoint.invalidate();
    m_focus_indicator_pix = 1;

    m_pCurrentStack = NULL;
    m_bpersistent_quilt = false;

    m_pQuilt = new Quilt( this );
    m_restore_dbindex = 0;
    SetQuiltMode(true);
    
    // Set some benign initial values


    mViewPoint.setLat(0);
    mViewPoint.setLon(0);
    mViewPoint.setViewScalePPM(1);
    mViewPoint.invalidate();
    m_canvas_scale_factor = 1.;
    m_canvas_width = 1000;
    m_pQuilt->EnableHighDefinitionZoom( true );

    //开启信号槽
    qRegisterMetaType<ArrayOfCDI>("ArrayOfCDI&");

    connect(this, SIGNAL(signalUpdateChartDatabase(ArrayOfCDI&,bool,QString)),
            this, SLOT(slotUpdateChartDatabase(ArrayOfCDI&,bool,QString)));
    this->moveToThread(&mWorkThread);
    mWorkThread.start();

}

void ChartFrameWork::slotInitEcidsAsDelayed()
{
    //读取配置文件中保存的地图数据目录
    ArrayOfCDI ChartDirArray;
    ZCHX_CFG_INS->LoadChartDirArray( ChartDirArray );


    if( !ChartDirArray.count() )
    {
        if(QFile::exists(g_chartListFileName )) QFile::remove(g_chartListFileName );
    }

    if(!ChartData)  ChartData = new ChartDB( );
    ChartData->LoadBinary(g_chartListFileName, ChartDirArray);
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
}


void ChartFrameWork::slotUpdateChartDatabase(ArrayOfCDI &DirArray, bool b_force, const QString &filename)
{
    // ..For each canvas...
    InvalidateQuilt();
    SetQuiltRefChart( -1 );
    m_singleChart = NULL;
    if(ChartData)   ChartData->PurgeCache();

    qDebug("Starting chart database Update...");
    QString gshhg_chart_loc = gWorldMapLocation;
    gWorldMapLocation.clear();
    ChartData->Update( DirArray, b_force, 0 );
    ChartData->SaveBinary(filename);
    qDebug("Finished chart database Update");
    if( gWorldMapLocation.isEmpty() ) { //Last resort. User might have deleted all GSHHG data, but we still might have the default dataset distributed with OpenCPN or from the package repository...
       gWorldMapLocation = gDefaultWorldMapLocation;
       gshhg_chart_loc.clear();
    }

    if( gWorldMapLocation != gshhg_chart_loc )
    {
        mGLCC->ResetWorldBackgroundChart();
    }
    ZCHX_CFG_INS->UpdateChartDirs( DirArray );
    DoCanvasUpdate();
    ReloadVP();                  // once more, and good to go

    emit signalUpdateChartArrayFinished();
}



ChartFrameWork::~ChartFrameWork()
{
    delete m_pQuilt;
}


void ChartFrameWork::CheckGroupValid( bool showMessage, bool switchGroup0)
{
    bool groupOK = CheckGroup( m_groupIndex );
    
    if(!groupOK){
        SetGroupIndex( m_groupIndex, true );
    }

}


//TODO
//extern bool     g_bLookAhead;
extern bool     g_bPreserveScaleOnX;
/*extern*/ ChartDummy *pDummyChart;
int      g_sticky_chart;

void ChartFrameWork::canvasRefreshGroupIndex( void )
{
    SetGroupIndex(m_groupIndex);
}

void ChartFrameWork::SetGroupIndex( int index, bool autoSwitch )
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

bool ChartFrameWork::CheckGroup( int igroup )
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


void ChartFrameWork::canvasChartsRefresh( int dbi_hint )
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
}


bool ChartFrameWork::DoCanvasUpdate( void )
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
    

    tLat = m_vLat;
    tLon = m_vLon;
    vpLat = m_vLat;
    vpLon = m_vLon;
    
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

                set_scale = GetCanvasScaleFactor() / proposed_scale_onscreen;
            }
            
            bNewView |= SetViewPoint( vpLat, vpLon, set_scale,
                                      m_singleChart->GetChartSkew() * PI / 180., GetVPRotation() );
            
        }
    }         // new stack
    
update_finish:
    m_bFirstAuto = false;                           // Auto open on program start
    
    //  If we need a Refresh(), do it here...
    //  But don't duplicate a Refresh() done by SetViewPoint()
    if( bNewChart && !bNewView )
    {
        if(mGLCC) mGLCC->Refresh( false );
    }
    if(mGLCC && bNewChart) mGLCC->Invalidate();

    return bNewChart | bNewView;
}

void ChartFrameWork::SelectQuiltRefdbChart( int db_index, bool b_autoscale )
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

void ChartFrameWork::SelectQuiltRefChart( int selected_index )
{
    std::vector<int>  piano_chart_index_array = GetQuiltExtendedStackdbIndexArray();
    int current_db_index = piano_chart_index_array[selected_index];
    
    SelectQuiltRefdbChart( current_db_index );
}

double ChartFrameWork::GetBestVPScale( ChartBase *pchart )
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

void ChartFrameWork::SetupCanvasQuiltMode( void )
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
            tLat = m_vLat;
            tLon = m_vLon;
            
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

double ChartFrameWork::GetCanvasRangeMeters()
{
    int minDimension =  fmin(m_canvas_width, m_canvas_height);
    
    double range  = (minDimension / GetVP().viewScalePPM())/2;
    range *= cos(GetVP().lat() *PI/180.);
    return range;
}

void ChartFrameWork::SetCanvasRangeMeters( double range )
{
    int minDimension =  fmin(m_canvas_width, m_canvas_height);
    
    double scale_ppm = minDimension / (range / cos(GetVP().lat() *PI/180.));
    SetVPScale( scale_ppm / 2 );
    
}


void ChartFrameWork::SetDisplaySizeMM( double size )
{
    m_display_size_mm = size;
    
    int sx = m_canvas_width, sy = m_canvas_height;
    
    double max_physical = fmax(sx, sy);
    
    m_pix_per_mm = ( max_physical ) / ( (double) m_display_size_mm );
    m_canvas_scale_factor = ( max_physical ) / (m_display_size_mm /1000.);
    
    
    if( ps52plib )
        ps52plib->SetPPMM( m_pix_per_mm );
    
    qDebug("Metrics:  m_display_size_mm: %g     wxDisplaySize:  %d:%d   ", m_display_size_mm, sx, sy);
    
    m_focus_indicator_pix = std::round(1 * GetPixPerMM());

}

int ChartFrameWork::GetCanvasChartNativeScale()
{
    int ret = 1;
    if( !mViewPoint.quilt() ) {
        if( m_singleChart ) ret = m_singleChart->GetNativeScale();
    } else
        ret = (int) m_pQuilt->GetRefNativeScale();

    return ret;

}

ChartBase* ChartFrameWork::GetChartAtPixel(int x, int y)
{
    ChartBase* target_chart;
    if( m_singleChart && ( m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR ) )
    {
        target_chart = m_singleChart;
    }
    else
    {
        if( mViewPoint.quilt() )
        {
            target_chart = m_pQuilt->GetChartAtPix( mViewPoint, QPoint( x, y ) );
        }
        else
        {
            target_chart = NULL;
        }
    }
    return target_chart;
}

ChartBase* ChartFrameWork::GetOverlayChartAtPixel(int x, int y)
{
    ChartBase* target_chart;
    if( mViewPoint.quilt() )
    {
        target_chart = m_pQuilt->GetOverlayChartAtPix( mViewPoint, QPoint( x, y ) );
    }
    else
    {
        target_chart = NULL;
    }
    return target_chart;
}

int ChartFrameWork::FindClosestCanvasChartdbIndex( int scale )
{
    int new_dbIndex = -1;
    if( !mViewPoint.quilt() ) {
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


bool ChartFrameWork::IsQuiltDelta()
{
    return m_pQuilt->IsQuiltDelta( mViewPoint );
}

void ChartFrameWork::UnlockQuilt()
{
    m_pQuilt->UnlockQuilt();
}

std::vector<int>  ChartFrameWork::GetQuiltIndexArray( void )
{
    return m_pQuilt->GetQuiltIndexArray();;
}

void ChartFrameWork::SetQuiltMode( bool quilt )
{
    mViewPoint.setQuilt(quilt);
    mViewPoint.setFullScreenQuilt(g_bFullScreenQuilt);
}

bool ChartFrameWork::GetQuiltMode( void )
{
    return mViewPoint.quilt();
}

int ChartFrameWork::GetQuiltReferenceChartIndex(void)
{
    return m_pQuilt->GetRefChartdbIndex();
}

void ChartFrameWork::InvalidateAllQuiltPatchs( void )
{
    m_pQuilt->InvalidateAllQuiltPatchs();
}

ChartBase *ChartFrameWork::GetLargestScaleQuiltChart()
{
    return m_pQuilt->GetLargestScaleChart();
}

ChartBase *ChartFrameWork::GetFirstQuiltChart()
{
    return m_pQuilt->GetFirstChart();
}

ChartBase *ChartFrameWork::GetNextQuiltChart()
{
    return m_pQuilt->GetNextChart();
}

int ChartFrameWork::GetQuiltChartCount()
{
    return m_pQuilt->GetnCharts();
}

void ChartFrameWork::SetQuiltChartHiLiteIndex( int dbIndex )
{
    m_pQuilt->SetHiliteIndex( dbIndex );
}

std::vector<int>  ChartFrameWork::GetQuiltCandidatedbIndexArray( bool flag1, bool flag2 )
{
    return m_pQuilt->GetCandidatedbIndexArray( flag1, flag2 );
}

int ChartFrameWork::GetQuiltRefChartdbIndex( void )
{
    return m_pQuilt->GetRefChartdbIndex();
}

std::vector<int>  ChartFrameWork::GetQuiltExtendedStackdbIndexArray()
{
    return m_pQuilt->GetExtendedStackIndexArray();
}

std::vector<int>  ChartFrameWork::GetQuiltEclipsedStackdbIndexArray()
{
    return m_pQuilt->GetEclipsedStackIndexArray();
}

void ChartFrameWork::InvalidateQuilt( void )
{
    return m_pQuilt->Invalidate();
}

double ChartFrameWork::GetQuiltMaxErrorFactor()
{
    return m_pQuilt->GetMaxErrorFactor();
}

bool ChartFrameWork::IsChartQuiltableRef( int db_index )
{
    return m_pQuilt->IsChartQuiltableRef( db_index );
}

bool ChartFrameWork::IsChartLargeEnoughToRender( ChartBase* chart, ViewPort& vp )
{
    double chartMaxScale = chart->GetNormalScaleMax( GetCanvasScaleFactor(), GetCanvasWidth() );
    return ( chartMaxScale*g_ChartNotRenderScaleFactor > vp.chartScale() );
}


ViewPort &ChartFrameWork::GetVP()
{
    return mViewPoint;
}

void ChartFrameWork::SetVP(ViewPort &vp)
{
    mViewPoint = vp;
}

// void ChartFrameWork::SetFocus()
// {
//     printf("set %d\n", m_canvasIndex);
//     //wxWindow:SetFocus();
// }







void ChartFrameWork::StopMovement( )
{
    m_zoom_factor = 1;
}


wxBitmap ChartFrameWork::CreateDimBitmap( wxBitmap &Bitmap, double factor )
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




void ChartFrameWork::GetDoubleCanvasPointPix( double rlat, double rlon, zchxPointF &r )
{
    return GetDoubleCanvasPointPixVP( GetVP(), rlat, rlon, r );
}

void ChartFrameWork::GetDoubleCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, zchxPointF &r )
{
    r = vp.GetDoublePixFromLL( rlat, rlon );
}


// This routine might be deleted and all of the rendering improved
// to have floating point accuracy
bool ChartFrameWork::GetCanvasPointPix( double rlat, double rlon, zchxPoint &r )
{
    return GetCanvasPointPixVP( GetVP(), rlat, rlon, r);
}

bool ChartFrameWork::GetCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, zchxPoint &r )
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


void ChartFrameWork::GetCanvasPixPoint( double x, double y, double &lat, double &lon )
{
    GetVP().GetLLFromPix( zchxPointF( x, y ), &lat, &lon );
}

bool ChartFrameWork::Pan( double dx, double dy )
{
    if( !ChartData ) return false;

    double lat = GetVP().lat(), lon = GetVP().lon();
    double dlat, dlon;
    zchxPointF p(GetVP().pixWidth() / 2.0, GetVP().pixHeight() / 2.0);

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

    SetViewPoint( dlat, dlon, GetVP().viewScalePPM(), GetVP().skew(), GetVP().rotation() );

    if( GetVP().quilt()) {
        int new_ref_dbIndex = m_pQuilt->GetRefChartdbIndex();
        if( ( new_ref_dbIndex != cur_ref_dbIndex ) && ( new_ref_dbIndex != -1 ) ) {
            //Tweak the scale slightly for a new ref chart
            ChartBase *pc = ChartData->OpenChartFromDB( new_ref_dbIndex, FULL_INIT );
            if( pc ) {
                double tweak_scale_ppm = pc->GetNearestPreferredScalePPM( GetVP().viewScalePPM() );
                SetVPScale( tweak_scale_ppm );
            }
        }
    }
    mGLCC->Refresh( false );
    return true;
}


void ChartFrameWork::Zoom( double factor,  bool can_zoom_to_cursor )
{
    // possible on startup
    if( !ChartData )
        return;
    if(!m_pCurrentStack)
        return;


    /* TODO: queue the quilted loading code to a background thread
       so yield is never called from here, and also rendering is not delayed */

    //    Cannot allow Yield() re-entrancy here
    if( m_bzooming ) return;
//    if( m_modkeys == Qt::AltModifier )
//        factor = pow(factor, .15);
    m_bzooming = true;

    double old_ppm = GetVP().viewScalePPM();

    //  Capture current cursor position for zoom to cursor
    double zlat = mGLCC->getCurLat();
    double zlon = mGLCC->getCurLon();

    double proposed_scale_onscreen = GetVP().chartScale() / factor; // GetCanvasScaleFactor() / ( GetVPScale() * factor );
    bool b_do_zoom = false;
    
    if(factor > 1)
    {
        b_do_zoom = true;

        double zoom_factor = factor;

        ChartBase *pc = NULL;

        if( !mViewPoint.quilt() ) {
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

        if( !mViewPoint.quilt() ) {             // not quilted
            pc = m_singleChart;

            if( pc ) {
                //      If m_singleChart is not on the screen, unbound the zoomout
                LLBBox viewbox = mViewPoint.getBBox();
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
            Pan( r.x - mGLCC->getCurPosX(), r.y - mGLCC->getCurPosY() );  // this will give the Refresh()

            //ClearbFollow();      // update the follow flag
        }
        else{
            SetVPScale( new_scale );
        }
    }
    
    m_bzooming = false;
    
}

void ChartFrameWork::RotateContinus( double dir )
{

    double speed = dir*10;
//    if( m_modkeys == Qt::AltModifier)
//        speed /= 20;
    Rotate(mViewPoint.rotation() + PI/180 * speed);
}

void ChartFrameWork::Rotate( double rotation )
{
    while(rotation < 0) rotation += 2*PI;
    while(rotation > 2*PI) rotation -= 2*PI;

    if(rotation == mViewPoint.rotation() || std::isnan(rotation))
        return;

    SetVPRotation( rotation );
    UpdateGPSCompassStatusBox( true );
    DoCanvasUpdate();
}

void ChartFrameWork::RotateDegree(double rotation)
{
    Rotate(PI/180 * rotation);
}

void ChartFrameWork::DoTiltCanvas( double tilt )
{
    while(tilt < 0) tilt = 0;
    while(tilt > .95) tilt = .95;

    if(tilt == mViewPoint.tilt() || std::isnan(tilt))
        return;

    mViewPoint.setTilt(tilt);
    mGLCC->Refresh( false );
}

void ChartFrameWork::JumpToPosition( double lat, double lon, double scale_ppm )
{
    if (lon > 180.0)
        lon -= 360.0;
    m_vLat = lat;
    m_vLon = lon;
    StopMovement();
    
    if( !GetQuiltMode() ) {
        double skew = 0;
        if(m_singleChart)
            skew = m_singleChart->GetChartSkew() * PI / 180.;
        SetViewPoint( lat, lon, scale_ppm, skew, GetVPRotation() );
    } else {
        if (scale_ppm != GetVPScale()) {
            // XXX should be done in SetViewPoint
            mViewPoint.setChartScale(m_canvas_scale_factor / ( scale_ppm ));
            AdjustQuiltRefChart();
        }
        SetViewPoint( lat, lon, scale_ppm, 0, GetVPRotation() );
    }
    
    ReloadVP();
}



void ChartFrameWork::ReloadVP( bool b_adjust )
{

    LoadVP( mViewPoint, b_adjust );
}

void ChartFrameWork::LoadVP( ViewPort &vp, bool b_adjust )
{
    if( mGLCC ) {
        mGLCC->Invalidate();
    }
    else
    {
        m_cache_vp.invalidate();
    }

    mViewPoint.invalidate();
    m_pQuilt->Invalidate();
    SetViewPoint( vp.lat(), vp.lon(), vp.viewScalePPM(), vp.skew(), vp.rotation(), vp.projectType(), b_adjust );

}

void ChartFrameWork::SetQuiltRefChart( int dbIndex )
{
    m_pQuilt->SetReferenceChart( dbIndex );
    mViewPoint.invalidate();
    m_pQuilt->Invalidate();
}

double ChartFrameWork::GetBestStartScale(int dbi_hint, const ViewPort &vp)
{
    return m_pQuilt->GetBestStartScale(dbi_hint, vp);
}


//      Verify and adjust the current reference chart,
//      so that it will not lead to excessive overzoom or underzoom onscreen
int ChartFrameWork::AdjustQuiltRefChart()
{
    int ret = -1;
    Q_ASSERT(m_pQuilt);

    Q_ASSERT(ChartData);
    ChartBase *pc = ChartData->OpenChartFromDB( m_pQuilt->GetRefChartdbIndex(), FULL_INIT );
    if( pc ) {
        double min_ref_scale = pc->GetNormalScaleMin( m_canvas_scale_factor, false );
        double max_ref_scale = pc->GetNormalScaleMax( m_canvas_scale_factor, m_canvas_width );

        if( mViewPoint.chartScale() < min_ref_scale )  {
            ret = m_pQuilt->AdjustRefOnZoomIn( mViewPoint.chartScale() );
        }
        else if( mViewPoint.chartScale() > max_ref_scale )  {
            ret = m_pQuilt->AdjustRefOnZoomOut( mViewPoint.chartScale() );
        }
        else {
            bool brender_ok = IsChartLargeEnoughToRender( pc, mViewPoint );

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
                            brender_ok = IsChartLargeEnoughToRender( ptest_chart, mViewPoint );
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


void ChartFrameWork::UpdateCanvasOnGroupChange( void )
{
    delete m_pCurrentStack;
    m_pCurrentStack = NULL;
    m_pCurrentStack = new ChartStack;
    Q_ASSERT(ChartData);
    ChartData->BuildChartStack( m_pCurrentStack, mViewPoint.lat(), mViewPoint.lon(), m_groupIndex );

    m_pQuilt->Compose( mViewPoint );
}

bool ChartFrameWork::SetViewPointByCorners( double latSW, double lonSW, double latNE, double lonNE )
{
    // Center Point
    double latc = (latSW + latNE)/2.0;
    double lonc = (lonSW + lonNE)/2.0;
    
    // Get scale in ppm (latitude)
    double ne_easting, ne_northing;
    toSM( latNE, lonNE, latc, lonc, &ne_easting, &ne_northing );
    
    double sw_easting, sw_northing;
    toSM( latSW, lonSW, latc, lonc, &sw_easting, &sw_northing );
    
    double scale_ppm = mViewPoint.pixHeight() / fabs(ne_northing - sw_northing);

    return SetViewPoint( latc, lonc, scale_ppm, mViewPoint.skew(), mViewPoint.rotation() );
}

bool ChartFrameWork::SetVPScale( double scale, bool refresh )
{
    return SetViewPoint( mViewPoint.lat(), mViewPoint.lon(), scale, mViewPoint.skew(), mViewPoint.rotation(),
                         mViewPoint.projectType(), true, refresh );
}

bool ChartFrameWork::SetViewPoint( double lat, double lon )
{
    return SetViewPoint( lat, lon, mViewPoint.viewScalePPM(), mViewPoint.skew(), mViewPoint.rotation() );
}

bool ChartFrameWork::SetViewPoint( double lat, double lon, double scale_ppm, double skew,
                                double rotation, int projection, bool b_adjust, bool b_refresh )
{
    bool b_ret = false;

    if(skew > PI) /* so our difference tests work, put in range of +-Pi */
        skew -= 2*PI;

    //  Any sensible change?
    if (mViewPoint.isValid()) {
        if( ( fabs( mViewPoint.viewScalePPM() - scale_ppm )/ scale_ppm < 1e-5 )
                && ( fabs( mViewPoint.skew() - skew ) < 1e-9 )
                && ( fabs( mViewPoint.rotation() - rotation ) < 1e-9 )
                && ( fabs( mViewPoint.lat() - lat ) < 1e-9 )
                && ( fabs( mViewPoint.lon() - lon ) < 1e-9 )
                && (mViewPoint.projectType() == projection || projection == PROJECTION_UNKNOWN) )
            return false;
    }
    
    if(mViewPoint.projectType() != projection)
        mViewPoint.InvalidateTransformCache(); // invalidate

    //    Take a local copy of the last viewport
    ViewPort last_vp = mViewPoint;

    mViewPoint.setSkew(skew);
    mViewPoint.setLat(lat);
    mViewPoint.setLon(lon);
    mViewPoint.setViewScalePPM(scale_ppm);
    if(projection != PROJECTION_UNKNOWN)
        mViewPoint.setProjectionType(projection);
    else
        if(mViewPoint.projectType() == PROJECTION_UNKNOWN)
            mViewPoint.setProjectionType(PROJECTION_MERCATOR);

    // don't allow latitude above 88 for mercator (90 is infinity)
    if(mViewPoint.projectType() == PROJECTION_MERCATOR ||
            mViewPoint.projectType() == PROJECTION_TRANSVERSE_MERCATOR) {
        if(mViewPoint.lat() > 89.5) mViewPoint.setLat(89.5);
        else if(mViewPoint.lat() < -89.5) mViewPoint.setLat( -89.5);
    }

    // don't zoom out too far for transverse mercator polyconic until we resolve issues
    if(mViewPoint.projectType() == PROJECTION_POLYCONIC ||
            mViewPoint.projectType() == PROJECTION_TRANSVERSE_MERCATOR)
        mViewPoint.setViewScalePPM(fmax(mViewPoint.viewScalePPM(), 2e-4));

    SetVPRotation( rotation );

    if( ( mViewPoint.pixWidth() <= 0 ) || ( mViewPoint.pixHeight() <= 0 ) )    // Canvas parameters not yet set
        return false;

    mViewPoint.validate();                     // Mark this ViewPoint as OK

    //  Has the Viewport scale changed?  If so, invalidate the vp
    if( last_vp.viewScalePPM() != scale_ppm ) {
        m_cache_vp.invalidate();
        if(mGLCC) mGLCC->Invalidate();
    }

    //  A preliminary value, may be tweaked below
    mViewPoint.setChartScale(m_canvas_scale_factor / ( scale_ppm ));

    // recompute cursor position
    // and send to interested plugins if the mouse is actually in this window

    const zchxPoint pt(QCursor::pos());
    //获取当前窗口在屏幕坐标的位置
    QWidget* parent = mGLCC->parentWidget();
    QPoint widget_pos = mGLCC->pos();
    if(parent) widget_pos = parent->mapToGlobal(widget_pos);
    int mouseX = pt.x - widget_pos.x();
    int mouseY = pt.y - widget_pos.y();
    if( (mouseX > 0) && (mouseX < mViewPoint.pixWidth()) && (mouseY > 0) && (mouseY < mViewPoint.pixHeight())){
        double lat, lon;
        GetCanvasPixPoint( mouseX, mouseY, lat, lon );
        mGLCC->setCurLL(lat, lon);
    }
    
    if( !mViewPoint.quilt() && m_singleChart ) {

        mViewPoint.SetBoxes();

        //  Allow the chart to adjust the new ViewPort for performance optimization
        //  This will normally be only a fractional (i.e.sub-pixel) adjustment...
        if( b_adjust ) m_singleChart->AdjustVP( last_vp, mViewPoint );

        // If there is a sensible change in the chart render, refresh the whole screen
        if( ( !m_cache_vp.isValid() ) || ( m_cache_vp.viewScalePPM() != mViewPoint.viewScalePPM() ) ) {
            mGLCC->Refresh( false );
            b_ret = true;
        } else {
            zchxPoint cp_last, cp_this;
            GetCanvasPointPix( m_cache_vp.lat(), m_cache_vp.lon(), cp_last );
            GetCanvasPointPix( mViewPoint.lat(), mViewPoint.lon(), cp_this );

            if( cp_last != cp_this ) {
                mGLCC->Refresh( false );
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
    if( mViewPoint.quilt()) {

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
                renderable = chartMaxScale * 64 >= mViewPoint.chartScale();
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
                            renderable = chartMaxScale*1.5 > mViewPoint.chartScale();
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
            mViewPoint.SetBoxes();

            //    If this quilt will be a perceptible delta from the existing quilt, then refresh the entire screen
            if( m_pQuilt->IsQuiltDelta( mViewPoint ) ) {
                //  Allow the quilt to adjust the new ViewPort for performance optimization
                //  This will normally be only a fractional (i.e. sub-pixel) adjustment...
                if( b_adjust ) m_pQuilt->AdjustQuiltVP( last_vp, mViewPoint );

                //                ChartData->ClearCacheInUseFlags();
                //                unsigned long hash1 = m_pQuilt->GetXStackHash();

                //                wxStopWatch sw;
                m_pQuilt->Compose( mViewPoint );
                //                printf("comp time %ld\n", sw.Time());

                //      If the extended chart stack has changed, invalidate any cached render bitmap
                //                if(m_pQuilt->GetXStackHash() != hash1) {
                //                    m_bm_cache_vp.Invalidate();
                //                    InvalidateGL();
                //                }

                ChartData->PurgeCacheUnusedCharts( 0.7 );

                if(b_refresh)
                    mGLCC->Refresh( false );

                b_ret = true;
            }
        }

        mViewPoint.setSkew(0.);  // Quilting supports 0 Skew
    }
    //  Has the Viewport projection changed?  If so, invalidate the vp
    if( last_vp.projectType() != mViewPoint.projectType() ) {
        m_cache_vp.invalidate();
        if(mGLCC) mGLCC->Invalidate();
    }


    mViewPoint.setChartScale(1.0);           // fallback default value
    
    if( !mViewPoint.getBBox().GetValid() ) mViewPoint.SetBoxes();

    if( mViewPoint.getBBox().GetValid() ) {

        //      Update the viewpoint reference scale
        if( m_singleChart )
            mViewPoint.setRefScale(m_singleChart->GetNativeScale());
        else
            mViewPoint.setRefScale(m_pQuilt->GetRefNativeScale());

        //    Calculate the on-screen displayed actual scale
        //    by a simple traverse northward from the center point
        //    of roughly one eighth of the canvas height
        zchxPointF r, r1;

        double delta_check = (mViewPoint.pixHeight() / mViewPoint.viewScalePPM()) / (1852. * 60);
        delta_check /= 8.;
        
        double check_point = fmin(89., mViewPoint.lat());

        while((delta_check + check_point) > 90.)
            delta_check /= 2.;

        double rhumbDist;
        DistanceBearingMercator( check_point, mViewPoint.lon(),
                                 check_point + delta_check, mViewPoint.lon(),
                                 0, &rhumbDist );

        GetDoubleCanvasPointPix( check_point, mViewPoint.lon(), r1 );
        GetDoubleCanvasPointPix( check_point + delta_check, mViewPoint.lon(), r );
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
            mViewPoint.setChartScale(m_canvas_scale_factor / ( m_true_scale_ppm ));
        else
            mViewPoint.setChartScale(1.0);


        // Create a nice renderable string
        double round_factor = 100.;
        if(mViewPoint.chartScale() < 1000.)
            round_factor = 10.;
        else if (mViewPoint.chartScale() < 10000.)
            round_factor = 50.;

        double true_scale_display =  qRound(mViewPoint.chartScale() / round_factor ) * round_factor;
        QString text;

        m_displayed_scale_factor = mViewPoint.refScale() / mViewPoint.chartScale();

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
    }

    //  Maintain member vLat/vLon
    m_vLat = mViewPoint.lat();
    m_vLon = mViewPoint.lon();

    return b_ret;
}



/* @ChartFrameWork::CalcGridSpacing
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
/* @ChartFrameWork::CalcGridText *************************************
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

/* @ChartFrameWork::GridDraw *****************************************
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


void ChartFrameWork::JaggyCircle( ocpnDC &dc, QPen pen, int x, int y, int radius )
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


void ChartFrameWork::slotResize(int width, int height)
{
    m_canvas_width = width;
    m_canvas_height = height;
    //    Get some canvas metrics

    //          Rescale to current value, in order to rebuild mViewPoint data structures
    //          for new canvas size
    SetVPScale( GetVPScale() );

    m_absolute_min_scale_ppm = m_canvas_width / ( 1.2 * WGS84_semimajor_axis_meters * PI ); // something like 180 degrees

    //  Inform the parent Frame that I am being resized...
    UpdateGPSCompassStatusBox(true);

    m_pQuilt->SetQuiltParameters( m_canvas_scale_factor, m_canvas_width );

    //    Resize the current viewport

    mViewPoint.setPixWidth(m_canvas_width);
    mViewPoint.setPixHeight(m_canvas_height);
    //  Rescale again, to capture all the changes for new canvas size
    SetVPScale( GetVPScale() );
    ReloadVP();
}




bool ChartFrameWork::CheckEdgePan( int x, int y, bool bdragging, int margin, int delta )
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

    return ( false );
}


void ChartFrameWork::ToggleCanvasQuiltMode( void )
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
        if(mGLCC) mGLCC->Invalidate();
        mGLCC->Refresh();
    }
    //  TODO What to do about this?
    //g_bQuiltEnable = GetQuiltMode();

    // Recycle the S52 PLIB so that vector charts will flush caches and re-render
    if(ps52plib)
        ps52plib->GenerateStateHash();
}

void ChartFrameWork::DoCanvasStackDelta( int direction )
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

//void ChartFrameWork::OnToolLeftClick( wxCommandEvent& event )
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

void ChartFrameWork::UpdateGPSCompassStatusBox( bool b_force_new )
{
    
    if( b_force_new)
        mGLCC->Refresh();

}



void ChartFrameWork::SelectChartFromStack( int index, bool bDir, ChartTypeEnum New_Type,
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
        zLat = m_vLat;
            zLon = m_vLon;
        
        double best_scale_ppm = GetBestVPScale( m_singleChart );
        double rotation = GetVPRotation();
        double oldskew = GetVPSkew();
        double newskew = m_singleChart->GetChartSkew() * PI / 180.0;
        
        if (!g_bskew_comp) {
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

void ChartFrameWork::SelectdbChart( int dbindex )
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
        zLat = m_vLat;
        zLon = m_vLon;
        
        double best_scale_ppm = GetBestVPScale( m_singleChart );
        
        if( m_singleChart )
            SetViewPoint( zLat, zLon, best_scale_ppm, m_singleChart->GetChartSkew() * PI / 180., GetVPRotation() );

        //SetChartUpdatePeriod( );
        
        //UpdateGPSCompassStatusBox();           // Pick up the rotation
        
    }
    
    // TODO refresh_Piano();
}


void ChartFrameWork::selectCanvasChartDisplay( int type, int family)
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


void ChartFrameWork::RemoveChartFromQuilt( int dbIndex )
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










//wxRect ChartFrameWork::GetMUIBarRect()
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

