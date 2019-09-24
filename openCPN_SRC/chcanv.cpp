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
#include "dychart.h"
#include "OCPNPlatform.h"
#include "geodesic.h"
#include "styles.h"
#include "thumbwin.h"
#include "chartimg.h"
#include "cutil.h"
#include "ocpn_pixel.h"
#include "ocpndc.h"
#include "timers.h"
#include "glTextureDescriptor.h"
#include "Quilt.h"
#include "FontMgr.h"
#include "OCPNRegion.h"
#include "gshhs.h"
#include "wx28compat.h"
#include "CanvasConfig.h"
#include "CanvasOptions.h"
#include "mbtiles.h"
#include "zchxmapmainwindow.h"
#include "s52plib.h"
#include "s52utils.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QProgressDialog>
#include <stdint.h>
#include "chartbase.h"
#include "TexFont.h"
#include "glTexCache.h"
#include "mipmap/mipmap.h"
#include <vector>
#include <algorithm>
#include "glTextureManager.h"
#include "GL/glext.h"
#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "lz4/lz4.h"
#include <vector>

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif



//extern bool GetMemoryStatus(int *mem_total, int *mem_used);

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#endif

extern          zchxMapMainWindow               *gFrame;
extern          s52plib                         *ps52plib;
extern          bool                            g_bShowFPS;
extern          bool                            g_bSoftwareGL;
extern          glTextureManager                *g_glTextureManager;
extern          bool                            b_inCompressAllCharts;
extern          std::vector<int>                g_quilt_noshow_index_array;
extern          GLenum                          g_texture_rectangle_format;
extern          int                             g_memCacheLimit;
extern          ColorScheme                     global_color_scheme;
extern          bool                            g_bquiting;
extern          ThumbWin                        *pthumbwin;
extern          int                             g_mipmap_max_level;
extern          ChartDB                         *ChartData;
extern          bool                            g_bcompression_wait;
float                                           g_GLMinSymbolLineWidth;
float                                           g_GLMinCartographicLineWidth;
extern          bool                            g_fog_overzoom;
extern          double                          g_overzoom_emphasis_base;
extern          bool                            g_oz_vector_scale;
extern          unsigned int                    g_canvasConfig;
extern          zchxGLOptions                   g_GLOptions;
extern          bool                            g_b_EnableVBO;
bool                                            g_b_needFinish;  //Need glFinish() call on each frame?

PFNGLGENFRAMEBUFFERSEXTPROC         s_glGenFramebuffers;
PFNGLGENRENDERBUFFERSEXTPROC        s_glGenRenderbuffers;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    s_glFramebufferTexture2D;
PFNGLBINDFRAMEBUFFEREXTPROC         s_glBindFramebuffer;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC s_glFramebufferRenderbuffer;
PFNGLRENDERBUFFERSTORAGEEXTPROC     s_glRenderbufferStorage;
PFNGLBINDRENDERBUFFEREXTPROC        s_glBindRenderbuffer;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  s_glCheckFramebufferStatus;
PFNGLDELETEFRAMEBUFFERSEXTPROC      s_glDeleteFramebuffers;
PFNGLDELETERENDERBUFFERSEXTPROC     s_glDeleteRenderbuffers;
PFNGLCOMPRESSEDTEXIMAGE2DPROC       s_glCompressedTexImage2D;
PFNGLGETCOMPRESSEDTEXIMAGEPROC      s_glGetCompressedTexImage;
PFNGLGENBUFFERSPROC                 s_glGenBuffers;
PFNGLBINDBUFFERPROC                 s_glBindBuffer;
PFNGLBUFFERDATAPROC                 s_glBufferData;
PFNGLDELETEBUFFERSPROC              s_glDeleteBuffers;

extern GLuint g_raster_format/* = GL_RGB*/;
long g_tex_mem_used;
bool            b_timeGL;
double          g_gl_ms_per_frame;
int g_tile_size;
int g_uncompressed_tile_size;

bool ChartCanvas::s_b_useScissorTest;
bool ChartCanvas::s_b_useStencil;
bool ChartCanvas::s_b_useStencilAP;
bool ChartCanvas::s_b_useFBO;
QString                             ChartListFileName;
extern int                          g_restore_dbindex;
// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------
extern bool G_FloatPtInPolygon ( MyFlPoint *rgpts, int wnumpts, float x, float y ) ;
extern void catch_signals(int signo);
extern void AlphaBlending( ocpnDC& dc, int x, int y, int size_x, int size_y, float radius,  QColor color, unsigned char transparency );
extern double           g_ChartNotRenderScaleFactor;
bool             bDBUpdateInProgress;
extern int              g_nbrightness;
extern int              g_iDistanceFormat;
extern QString GetLayerName(int id);
extern bool             g_bEnableZoomToCursor;
extern bool             g_bInlandEcdis;
extern bool             g_bFullScreenQuilt;
extern bool             g_bsmoothpanzoom;
extern bool             g_b_assume_azerty;
extern ChartGroupArray  *g_pGroupArray;

//  TODO why are these static?
static int mouse_x;
static int mouse_y;
extern double           g_display_size_mm;
extern bool             g_bskew_comp;
extern SENCThreadManager *g_SencThreadManager;
ChartCanvas                 *gMainCanvas = 0;
extern QString                  *pInit_Chart_Dir;
int                       g_mem_total, g_mem_used, g_mem_initial;
extern QString                  gWorldMapLocation;
QString gDefaultWorldMapLocation;

#define MIN_BRIGHT 10
#define MAX_BRIGHT 100



// Define a constructor for my canvas
ChartCanvas::ChartCanvas ( QWidget *frame ) : QGLWidget(frame)
{
    parent_frame = ( zchxMapMainWindow * ) frame;       // save a pointer to parent
    gMainCanvas = this;    
    pscratch_bm = NULL;
    m_groupIndex = 0;
    m_bShowNavobjects = true;
    m_bzooming = false;
    m_b_paint_enable = false;
    m_disable_edge_pan = false;
    m_dragoffsetSet = false;
    m_bautofind = false;
    m_bFirstAuto = true;
    m_groupIndex = 0;
    m_singleChart = NULL;
    m_MouseDragging = false;
    
    m_vLat = 35.7421999999;
    m_vLon = 127.52430000;
    m_bbrightdir = false;

    m_zoom_factor = 1;
    m_rotation_speed = 0;
    m_mustmove = 0;

    VPoint.invalidate();
    m_focus_indicator_pix = 1;

    m_pCurrentStack = NULL;
    m_bpersistent_quilt = false;
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
    m_pQuilt = new Quilt( this );
    m_restore_dbindex = 0;
    SetQuiltMode(true);
    m_bsetup = false;
    mIsUpdateAvailable = false;
    m_pcontext = this->context();
    m_cache_current_ch = NULL;
    m_b_paint_enable = true;
    m_in_glpaint = false;
    m_cache_tex[0] = m_cache_tex[1] = 0;
    m_cache_page = 0;
    m_b_BuiltFBO = false;
    m_b_DisableFBO = false;
    m_binPinch = false;
    m_binPan = false;
    m_bpinchGuard = false;
    m_LRUtime = 0;

    if( !g_glTextureManager) g_glTextureManager = new glTextureManager;
    m_panx = m_pany = 0;
    m_panspeed = 0;
    
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
    mDisplsyTimer = new QTimer(this);
    mDisplsyTimer->setInterval(1000);
    connect(mDisplsyTimer, SIGNAL(timeout()), this, SLOT(update()));
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
    setUpdateAvailable(true);
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
    config->iLat = 37.123456;
    config->iLon = 127.123456;

    SetDisplaySizeMM(g_display_size_mm);

    ApplyCanvasConfig(config);

    //            cc->SetToolbarPosition(wxPoint( g_maintoolbar_x, g_maintoolbar_y ));
    SetColorScheme( global_color_scheme );
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

    //    Delete Cursors
    delete pCursorLeft;
    delete pCursorRight;
    delete pCursorUp;
    delete pCursorDown;
    delete pCursorArrow;
    delete pCursorPencil;
    delete pCursorCross;
    delete pscratch_bm;
    delete pWorldBackgroundChart;
    delete m_pEM_Feet;
    delete m_pEM_Meters;
    delete m_pEM_Fathoms;
    delete m_pEM_OverZoom;
    delete m_pQuilt;
}

void ChartCanvas::CanvasApplyLocale()
{
    CreateDepthUnitEmbossMaps( m_cs );
    CreateOZEmbossMapData( m_cs );
}




void ChartCanvas::ApplyCanvasConfig(canvasConfig *pcc)
{
    SetViewPoint( pcc->iLat, pcc->iLon, pcc->iScale, 0., pcc->iRotation );
    m_vLat = pcc->iLat;
    m_vLon = pcc->iLon;

    m_restore_dbindex = pcc->DBindex;
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
    m_singleChart = NULL;

}



void ChartCanvas::CheckGroupValid( bool showMessage, bool switchGroup0)
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
    if( m_pCurrentStack )      last_nEntry = m_pCurrentStack->nEntry;

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
    }
update_finish:
    m_bFirstAuto = false;                           // Auto open on program start
    
    //  If we need a Refresh(), do it here...
    //  But don't duplicate a Refresh() done by SetViewPoint()
    if( bNewChart && !bNewView )  Refresh( false );

#ifdef ocpnUSE_GL
    // If a new chart, need to invalidate gl viewport for refresh
    // so the fbo gets flushed
    if( bNewChart) Invalidate();
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
    Invalidate();
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
    if(m_modkeys == Qt::NoModifier)
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
        } else {
            PanCanvas( -panspeed, 0 );
        }
        b_handled = true;
        break;

    case Qt::Key_Up:
        PanCanvas( 0, -panspeed );
        b_handled = true;
        break;

    case Qt::Key_Right:
        if(  m_modkeys & Qt::ControlModifier ) this->DoCanvasStackDelta(1);
        else
            PanCanvas( panspeed, 0 );
        b_handled = true;
        
        break;

    case Qt::Key_Down:
        PanCanvas( 0, panspeed );
        b_handled = true;
        break;

    case Qt::Key_F2:
        break;

    case Qt::Key_F3: {
        SetShowENCText( !GetShowENCText() );
        Refresh(true);
        InvalidateGL();
        break;
    }
    case Qt::Key_F4:
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

                break;

            case 2:                      // Ctrl B
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
                    break;
            }

            case 32:            // Special needs use space bar
            {
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

void ChartCanvas::ToggleChartOutlines( void )
{
    m_bShowOutlines = !m_bShowOutlines;
    
    Refresh( false );

    InvalidateGL();
}


void ChartCanvas::StopMovement( )
{
    m_panx = m_pany = 0;
    m_panspeed = 0;
    m_zoom_factor = 1;
    m_rotation_speed = 0;
    m_mustmove = 0;
}



void ChartCanvas::SetColorScheme( ColorScheme cs )
{
    CreateDepthUnitEmbossMaps( cs );
    CreateOZEmbossMapData( cs );

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
    
    if(pWorldBackgroundChart) pWorldBackgroundChart->SetColorScheme( cs );
    if(g_glTextureManager) g_glTextureManager->ClearAllRasterTextures();

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

    if( !g_bsmoothpanzoom ) {
        if( m_modkeys == Qt::AltModifier )
            factor = pow(factor, .15);
        
        DoZoomCanvas( factor, can_zoom_to_cursor );
    }
}

void ChartCanvas::DoZoomCanvas( double factor,  bool can_zoom_to_cursor )
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
            SetVPScale( new_scale );
        }
    }
    
    m_bzooming = false;
    
}

void ChartCanvas::RotateCanvas( double dir )
{
    double speed = dir*10;
    if( m_modkeys == Qt::AltModifier)
        speed /= 20;
    DoRotateCanvas(VPoint.rotation() + PI/180 * speed);
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


void ChartCanvas::JumpToPosition( double lat, double lon, double scale_ppm )
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
            VPoint.setChartScale(m_canvas_scale_factor / ( scale_ppm ));
            AdjustQuiltRefChart();
        }
        SetViewPoint( lat, lon, scale_ppm, 0, GetVPRotation() );
    }
    
    ReloadVP();
}


bool ChartCanvas::PanCanvas( double dx, double dy )
{
    if( !ChartData )
        return false;

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
    Invalidate();
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

    m_pQuilt->SetQuiltParameters( m_canvas_scale_factor, m_canvas_width );

    //    Resize the current viewport

    VPoint.setPixWidth(m_canvas_width);
    VPoint.setPixHeight(m_canvas_height);

    // Resize the scratch BM
    delete pscratch_bm;
    pscratch_bm = new wxBitmap( VPoint.pixWidth(), VPoint.pixHeight(), -1 );

    //  Rescale again, to capture all the changes for new canvas size
    SetVPScale( GetVPScale() );
    //  Invalidate the whole window
    ReloadVP();
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



bool leftIsDown;




bool panleftIsDown;


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
    RenderChartOutline(dbIndex, vp);
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


bool ChartCanvas::SetCursor( const QCursor &c )
{
    setCursor( c );
    return true;
}

void ChartCanvas::Refresh( bool eraseBackground, const QRect *rect )
{
    if( g_bquiting )
        return;
    //  Keep the mouse position members up to date
    GetCanvasPixPoint( mouse_x, mouse_y, m_cursor_lat, m_cursor_lon );

    if( eraseBackground && UsingFBO() )    Invalidate();
}


void ChartCanvas::DrawEmboss( ocpnDC &dc, emboss_data *pemboss)
{
}

emboss_data *ChartCanvas::EmbossOverzoomIndicator( ocpnDC &dc )
{
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
    ped->y = 40;
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
    if( b_force_new )
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

#include <QLabel>
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




//#define     GL_TEST


//static int s_nquickbind;


/* for debugging */
static void print_region(OCPNRegion &Region)
{
    OCPNRegionIterator upd ( Region );
    while ( upd.HaveRects() )
    {
        QRect rect = upd.GetRect();
        qDebug("[(%d, %d) (%d, %d)] ", rect.x(), rect.y(), rect.width(), rect.height());
        upd.NextRect();
    }
}

GLboolean QueryExtension( const char *extName )
{
    /*
     ** Search for extName in the extensions string. Use of strstr()
     ** is not sufficient because extension names can be prefixes of
     ** other extension names. Could use strtok() but the constant
     ** string returned by glGetString might be in read-only memory.
     */
    char *p;
    char *end;
    int extNameLen;

    extNameLen = strlen( extName );

    p = (char *) glGetString( GL_EXTENSIONS );
    if( NULL == p ) {
        return GL_FALSE;
    }

    end = p + strlen( p );

    while( p < end ) {
        int n = strcspn( p, " " );
        if( ( extNameLen == n ) && ( strncmp( extName, p, n ) == 0 ) ) {
            return GL_TRUE;
        }
        p += ( n + 1 );
    }
    return GL_FALSE;
}

typedef void (*GenericFunction)(void);

//#if defined(__WXMSW__)
//#define systemGetProcAddress(ADDR) wglGetProcAddress(ADDR)
//#elif defined(__WXOSX__)
//#include <dlfcn.h>
//#define systemGetProcAddress(ADDR) dlsym( RTLD_DEFAULT, ADDR)
//#elif defined(__OCPN__ANDROID__)
//#define systemGetProcAddress(ADDR) eglGetProcAddress(ADDR)
//#else
//#define systemGetProcAddress(ADDR) glXGetProcAddress((const GLubyte*)ADDR)
//#endif

#define systemGetProcAddress(ADDR) wglGetProcAddress(ADDR)

GenericFunction ocpnGetProcAddress(const char *addr, const char *extension)
{
    char addrbuf[256];
    if(!extension)
        return (GenericFunction)NULL;

#ifndef __OCPN__ANDROID__
    //  If this is an extension entry point,
    //  We look explicitly in the extensions list to confirm
    //  that the request is actually supported.
    // This may be redundant, but is conservative, and only happens once per session.
    if(extension && strlen(extension)){
        QString s_extension = QString::fromUtf8(&addr[2]);
        QString s_family = QString::fromUtf8(extension);
        s_extension.insert(0, "_");
        s_extension.insert(0, s_family);
        s_extension.insert(0, "GL_");

        if(!QueryExtension( s_extension.toUtf8().data() )){
            return (GenericFunction)NULL;
        }
    }
#endif

    snprintf(addrbuf, sizeof addrbuf, "%s%s", addr, extension);
    return (GenericFunction)systemGetProcAddress(addrbuf);

}

bool  b_glEntryPointsSet;

void GetglEntryPoints( OCPN_GLCaps *pcaps )
{

    // the following are all part of framebuffer object,
    // according to opengl spec, we cannot mix EXT and ARB extensions
    // (I don't know that it could ever happen, but if it did, bad things would happen)

#ifndef __OCPN__ANDROID__
    const char *extensions[] = {"", "ARB", "EXT", 0 };
#else
    const char *extensions[] = {"OES", 0 };
#endif

    unsigned int n_ext = (sizeof extensions) / (sizeof *extensions);

    unsigned int i;
    for(i=0; i<n_ext; i++) {
        if((pcaps->m_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenFramebuffers", extensions[i])))
            break;
    }

    if(i<n_ext){
        pcaps->m_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenRenderbuffers", extensions[i]);
        pcaps->m_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
            ocpnGetProcAddress( "glFramebufferTexture2D", extensions[i]);
        pcaps->m_glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindFramebuffer", extensions[i]);
        pcaps->m_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glFramebufferRenderbuffer", extensions[i]);
        pcaps->m_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)
            ocpnGetProcAddress( "glRenderbufferStorage", extensions[i]);
        pcaps->m_glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindRenderbuffer", extensions[i]);
        pcaps->m_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)
            ocpnGetProcAddress( "glCheckFramebufferStatus", extensions[i]);
        pcaps->m_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteFramebuffers", extensions[i]);
        pcaps->m_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteRenderbuffers", extensions[i]);

        //VBO
        pcaps->m_glGenBuffers = (PFNGLGENBUFFERSPROC)
            ocpnGetProcAddress( "glGenBuffers", extensions[i]);
        pcaps->m_glBindBuffer = (PFNGLBINDBUFFERPROC)
            ocpnGetProcAddress( "glBindBuffer", extensions[i]);
        pcaps->m_glBufferData = (PFNGLBUFFERDATAPROC)
            ocpnGetProcAddress( "glBufferData", extensions[i]);
        pcaps->m_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)
            ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);


    }

    //  Retry VBO entry points with all extensions
    if(0 == pcaps->m_glGenBuffers){
        for( i=0; i<n_ext; i++) {
            if((pcaps->m_glGenBuffers = (PFNGLGENBUFFERSPROC)ocpnGetProcAddress( "glGenBuffers", extensions[i])) )
                break;
        }

        if( i < n_ext ){
            pcaps->m_glBindBuffer = (PFNGLBINDBUFFERPROC) ocpnGetProcAddress( "glBindBuffer", extensions[i]);
            pcaps->m_glBufferData = (PFNGLBUFFERDATAPROC) ocpnGetProcAddress( "glBufferData", extensions[i]);
            pcaps->m_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);
        }
    }


#ifndef __OCPN__ANDROID__
    for(i=0; i<n_ext; i++) {
        if((pcaps->m_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)
            ocpnGetProcAddress( "glCompressedTexImage2D", extensions[i])))
            break;
    }

    if(i<n_ext){
        pcaps->m_glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)
            ocpnGetProcAddress( "glGetCompressedTexImage", extensions[i]);
    }
#else
    pcaps->m_glCompressedTexImage2D =          glCompressedTexImage2D;
#endif

}





static void GetglEntryPoints( void )
{
    b_glEntryPointsSet = true;

    // the following are all part of framebuffer object,
    // according to opengl spec, we cannot mix EXT and ARB extensions
    // (I don't know that it could ever happen, but if it did, bad things would happen)

#ifndef __OCPN__ANDROID__
    const char *extensions[] = {"", "ARB", "EXT", 0 };
#else
    const char *extensions[] = {"OES", 0 };
#endif

    unsigned int n_ext = (sizeof extensions) / (sizeof *extensions);

    unsigned int i;
    for(i=0; i<n_ext; i++) {
        if((s_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenFramebuffers", extensions[i])))
            break;
    }

    if(i<n_ext){
        s_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenRenderbuffers", extensions[i]);
        s_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
            ocpnGetProcAddress( "glFramebufferTexture2D", extensions[i]);
        s_glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindFramebuffer", extensions[i]);
        s_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glFramebufferRenderbuffer", extensions[i]);
        s_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)
            ocpnGetProcAddress( "glRenderbufferStorage", extensions[i]);
        s_glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindRenderbuffer", extensions[i]);
        s_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)
            ocpnGetProcAddress( "glCheckFramebufferStatus", extensions[i]);
        s_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteFramebuffers", extensions[i]);
        s_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteRenderbuffers", extensions[i]);

        //VBO
        s_glGenBuffers = (PFNGLGENBUFFERSPROC)
            ocpnGetProcAddress( "glGenBuffers", extensions[i]);
        s_glBindBuffer = (PFNGLBINDBUFFERPROC)
            ocpnGetProcAddress( "glBindBuffer", extensions[i]);
        s_glBufferData = (PFNGLBUFFERDATAPROC)
            ocpnGetProcAddress( "glBufferData", extensions[i]);
        s_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)
            ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);


    }

    //  Retry VBO entry points with all extensions
    if(0 == s_glGenBuffers){
        for( i=0; i<n_ext; i++) {
            if((s_glGenBuffers = (PFNGLGENBUFFERSPROC)ocpnGetProcAddress( "glGenBuffers", extensions[i])) )
                break;
        }

        if( i < n_ext ){
            s_glBindBuffer = (PFNGLBINDBUFFERPROC) ocpnGetProcAddress( "glBindBuffer", extensions[i]);
            s_glBufferData = (PFNGLBUFFERDATAPROC) ocpnGetProcAddress( "glBufferData", extensions[i]);
            s_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);
        }
    }


#ifndef __OCPN__ANDROID__
    for(i=0; i<n_ext; i++) {
        if((s_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)
            ocpnGetProcAddress( "glCompressedTexImage2D", extensions[i])))
            break;
    }

    if(i<n_ext){
        s_glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)
            ocpnGetProcAddress( "glGetCompressedTexImage", extensions[i]);
    }
#else
    s_glCompressedTexImage2D =          glCompressedTexImage2D;
#endif

}


void ChartCanvas::FlushFBO( void )
{
    if(m_bsetup)
        BuildFBO();
}




void ChartCanvas::initializeGL()
{
    qDebug()<<"now initialized...";
#ifdef GL_TEST
    glClearColor(0.0, 0.2, 0.3, 1.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH);
#endif
}

void ChartCanvas::resizeGL(int w, int h)
{
    qDebug()<<"now resized with:"<<w<<h;
#ifdef GL_TEST
    int side = qMin(w, h);
    glViewport((width() - side) / 2, (height() - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.2, 1.2, -1.2, 1.2, 5.0, 60.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -40.0);
#endif
    if(m_bsetup)
    {
        BuildFBO();
    }
    QGLWidget::resizeGL(w, h);
}


void ChartCanvas::BuildFBO( )
{
    if(isVisible())        makeCurrent();

    if( m_b_BuiltFBO ) {
        glDeleteTextures( 2, m_cache_tex );
        ( s_glDeleteFramebuffers )( 1, &m_fb0 );
        ( s_glDeleteRenderbuffers )( 1, &m_renderbuffer );
        m_b_BuiltFBO = false;
    }

    if( m_b_DisableFBO)
        return;

    //  In CanvasPanning mode, we will build square POT textures for the FBO backing store
    //  We will make them as large as possible...
    if(g_GLOptions.m_bUseCanvasPanning){
        int rb_x = width();
        int rb_y = height();
        int i=1;
        while(i < rb_x) i <<= 1;
            rb_x = i;

        i=1;
        while(i < rb_y) i <<= 1;
            rb_y = i;

        m_cache_tex_x = fmax(rb_x, rb_y);
        m_cache_tex_y = fmax(rb_x, rb_y);
        m_cache_tex_x = fmax(2048, m_cache_tex_x);
        m_cache_tex_y = fmax(2048, m_cache_tex_y);
    } else {
        m_cache_tex_x = width();
        m_cache_tex_y = height();
    }

    ( s_glGenFramebuffers )( 1, &m_fb0 );
    ( s_glGenRenderbuffers )( 1, &m_renderbuffer );

    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );


    // initialize color textures
    glGenTextures( 2, m_cache_tex );
    for(int i=0; i<2; i++) {
        glBindTexture( g_texture_rectangle_format, m_cache_tex[i] );
        glTexParameterf( g_texture_rectangle_format, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( g_texture_rectangle_format, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexImage2D( g_texture_rectangle_format, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, NULL );

    }

    ( s_glBindRenderbuffer )( GL_RENDERBUFFER_EXT, m_renderbuffer );

    if( m_b_useFBOStencil ) {
        // initialize composite depth/stencil renderbuffer
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT,
                                         m_cache_tex_x, m_cache_tex_y );

        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );

        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );
    } else {

        GLenum depth_format = GL_DEPTH_COMPONENT24;

        //      Need to check for availability of 24 bit depth buffer extension on GLES
#ifdef ocpnUSE_GLES
        if( !QueryExtension("GL_OES_depth24") )
            depth_format = GL_DEPTH_COMPONENT16;
#endif

        // initialize depth renderbuffer
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER_EXT, depth_format,
                                         m_cache_tex_x, m_cache_tex_y );
        int err = glGetError();
        if(err){
            qDebug("    OpenGL-> Framebuffer Depth Buffer Storage error:  %08X", err );
        }

        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );

        err = glGetError();
        if(err){
            qDebug("    OpenGL-> Framebuffer Depth Buffer Attach error:  %08X", err );
        }
    }

    // Disable Render to FBO
    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );

    /* invalidate cache */
    Invalidate();

    glClear( GL_COLOR_BUFFER_BIT );
    m_b_BuiltFBO = true;
}


void ChartCanvas::SetupOpenGL()
{
    char *str = (char *) glGetString( GL_RENDERER );
    if (str == NULL) {
        // perhaps we should edit the config and turn off opengl now
        qDebug("Failed to initialize OpenGL");
        exit(1);
    }

    char render_string[80];
    strncpy( render_string, str, 79 );
    m_renderer = QString::fromUtf8(render_string );

    QString msg;
    if(g_bSoftwareGL)
    {
        msg.sprintf("OpenGL-> Software OpenGL");
    }
    msg.sprintf("OpenGL-> Renderer String: " );
    msg += m_renderer;
    qDebug()<<msg;

    if( ps52plib ) ps52plib->SetGLRendererString( m_renderer );

    char version_string[80];
    strncpy( version_string, (char *) glGetString( GL_VERSION ), 79 );
    msg.sprintf("OpenGL-> Version reported:  ");
    m_version = QString::fromUtf8(version_string );
    msg += m_version;
    qDebug()<< msg;

    const GLubyte *ext_str = glGetString(GL_EXTENSIONS);
    m_extensions = QString::fromUtf8((const char *)ext_str );
//    qDebug("OpenGL extensions available: %s", m_extensions.toUtf8().data() );

    bool b_oldIntel = false;
    if( GetRendererString().toUpper().indexOf("INTEL")  != -1 ){
        if( GetRendererString().toUpper().indexOf("965")  != -1 ){
            qDebug("OpenGL-> Detected early Intel renderer, disabling some GL features" );
            b_oldIntel = true;
        }
    }

    //  Set the minimum line width
    GLint parms[2];
    glGetIntegerv( GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0] );
    g_GLMinSymbolLineWidth = fmax(parms[0], 1);
    g_GLMinCartographicLineWidth = fmax(parms[0], 1);

    //    Some GL renderers do a poor job of Anti-aliasing very narrow line widths.
    //    This is most evident on rendered symbols which have horizontal or vertical line segments
    //    Detect this case, and adjust the render parameters.

    if( m_renderer.toUpper().indexOf("MESA")  != -1 ){
        GLfloat parf;
        glGetFloatv(  GL_SMOOTH_LINE_WIDTH_GRANULARITY, &parf );

        g_GLMinSymbolLineWidth = fmax(((float)parms[0] + parf), 1);
    }

    s_b_useScissorTest = true;
    // the radeon x600 driver has buggy scissor test
    if( GetRendererString().toUpper().indexOf("RADEON X600")  != -1 )
        s_b_useScissorTest = false;

    if( GetRendererString().indexOf("GeForce")  != -1 )   //GeForce GTX 1070
        s_b_useScissorTest = false;

    //  This little hack fixes a problem seen with some Intel 945 graphics chips
    //  We need to not do anything that requires (some) complicated stencil operations.

    bool bad_stencil_code = false;
    if( GetRendererString().indexOf("Intel")  != -1 ) {
        qDebug("OpenGL-> Detected Intel renderer, disabling stencil buffer" );
        bad_stencil_code = true;
    }

    //      And for the lousy Unichrome drivers, too
    if( GetRendererString().indexOf("UniChrome")  != -1 )
        bad_stencil_code = true;

    //      And for the lousy Mali drivers, too
    if( GetRendererString().indexOf("Mali")  != -1 )
        bad_stencil_code = true;

    //XP  Generic Needs stencil buffer
    //W7 Generic Needs stencil buffer
//      if( GetRendererString().Find( _T("Generic") ) != Q_INDEX_NOT_FOUND ) {
//          qDebug("OpenGL-> Detected Generic renderer, disabling stencil buffer") );
//          bad_stencil_code = true;
//      }

    //          Seen with intel processor on VBox Win7
    if( GetRendererString().indexOf("Chromium")  != -1 ) {
        qDebug("OpenGL-> Detected Chromium renderer, disabling stencil buffer" );
        bad_stencil_code = true;
    }

    //      Stencil buffer test
    glEnable( GL_STENCIL_TEST );
    GLboolean stencil = glIsEnabled( GL_STENCIL_TEST );
    int sb;
    glGetIntegerv( GL_STENCIL_BITS, &sb );
    //        printf("Stencil Buffer Available: %d\nStencil bits: %d\n", stencil, sb);
    glDisable( GL_STENCIL_TEST );

    s_b_useStencil = false;
    if( stencil && ( sb == 8 ) )
        s_b_useStencil = true;

    if( QueryExtension( "GL_ARB_texture_non_power_of_two" ) )
        g_texture_rectangle_format = GL_TEXTURE_2D;
    else if( QueryExtension( "GL_OES_texture_npot" ) )
        g_texture_rectangle_format = GL_TEXTURE_2D;
    else if( QueryExtension( "GL_ARB_texture_rectangle" ) )
        g_texture_rectangle_format = GL_TEXTURE_RECTANGLE_ARB;
    qDebug("OpenGL-> Texture rectangle format: %x",g_texture_rectangle_format);

#ifndef __OCPN__ANDROID__
        //      We require certain extensions to support FBO rendering
        if(!g_texture_rectangle_format)
            m_b_DisableFBO = true;

        if(!QueryExtension( "GL_EXT_framebuffer_object" ))
            m_b_DisableFBO = true;
#endif

#ifdef __OCPN__ANDROID__
         g_texture_rectangle_format = GL_TEXTURE_2D;
#endif

    GetglEntryPoints();

    if( !s_glGenFramebuffers  || !s_glGenRenderbuffers        || !s_glFramebufferTexture2D ||
        !s_glBindFramebuffer  || !s_glFramebufferRenderbuffer || !s_glRenderbufferStorage  ||
        !s_glBindRenderbuffer || !s_glCheckFramebufferStatus  || !s_glDeleteFramebuffers   ||
        !s_glDeleteRenderbuffers )
        m_b_DisableFBO = true;

    // VBO??

    g_b_EnableVBO = true;
    if( !s_glBindBuffer || !s_glBufferData || !s_glGenBuffers || !s_glDeleteBuffers )
        g_b_EnableVBO = false;

#if defined( __WXMSW__ ) || defined(__WXOSX__)
    if(b_oldIntel)
        g_b_EnableVBO = false;
#endif

#ifdef __OCPN__ANDROID__
    g_b_EnableVBO = false;
#endif

    if(g_b_EnableVBO)
        qDebug("OpenGL-> Using Vertexbuffer Objects") ;
    else
        qDebug("OpenGL-> Vertexbuffer Objects unavailable");


    //      Can we use the stencil buffer in a FBO?
#ifdef ocpnUSE_GLES
    m_b_useFBOStencil = QueryExtension( "GL_OES_packed_depth_stencil" );
#else
    m_b_useFBOStencil = QueryExtension( "GL_EXT_packed_depth_stencil" ) == GL_TRUE;
#endif

#ifdef __OCPN__ANDROID__
    m_b_useFBOStencil = false;
#endif

    //  On Intel Graphics platforms, don't use stencil buffer at all
    if( bad_stencil_code)
        s_b_useStencil = false;

    g_GLOptions.m_bUseCanvasPanning = false;
#ifdef __OCPN__ANDROID__
    g_GLOptions.m_bUseCanvasPanning = isPlatformCapable(PLATFORM_CAP_FASTPAN);
#endif

    //      Maybe build FBO(s)

    BuildFBO();




#if 1   /* this test sometimes fails when the fbo still works */
        //  But we need to be ultra-conservative here, so run all the tests we can think of


    //  But we cannot even run this test on some platforms
    //  So we simply have to declare FBO unavailable
#ifdef __WXMSW__
    if( GetRendererString().Upper().Find( _T("INTEL") ) != Q_INDEX_NOT_FOUND ) {
        if(GetRendererString().Upper().Find( _T("MOBILE") ) != Q_INDEX_NOT_FOUND ){
            qDebug("OpenGL-> Detected Windows Intel Mobile renderer, disabling Frame Buffer Objects") );
            m_b_DisableFBO = true;
            BuildFBO();
        }
    }
#endif

    if( m_b_BuiltFBO ) {
        // Check framebuffer completeness at the end of initialization.
        ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );

        ( s_glFramebufferTexture2D )
        ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
          g_texture_rectangle_format, m_cache_tex[0], 0 );

        GLenum fb_status = ( s_glCheckFramebufferStatus )( GL_FRAMEBUFFER_EXT );
        ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );

        if( fb_status != GL_FRAMEBUFFER_COMPLETE_EXT ) {
            qDebug("    OpenGL-> Framebuffer Incomplete:  %08X", fb_status );
            m_b_DisableFBO = true;
            BuildFBO();
        }
    }
#endif

#ifdef __OCPN__ANDROID__
    g_GLOptions.m_bUseCanvasPanning = m_b_BuiltFBO;
    if(g_GLOptions.m_bUseCanvasPanning)
        qDebug("OpenGL-> Using FastCanvas Panning/Zooming") );

#endif

    if( m_b_BuiltFBO && !m_b_useFBOStencil )
        s_b_useStencil = false;

    //  If stencil seems to be a problem, force use of depth buffer clipping for Area Patterns
    s_b_useStencilAP = s_b_useStencil & !bad_stencil_code;

    if( m_b_BuiltFBO ) {
        qDebug("OpenGL-> Using Framebuffer Objects");

        if( m_b_useFBOStencil )
            qDebug("OpenGL-> Using FBO Stencil buffer" );
        else
            qDebug("OpenGL-> FBO Stencil buffer unavailable" );
    } else
        qDebug("OpenGL-> Framebuffer Objects unavailable" );

    if( s_b_useStencil ) qDebug("OpenGL-> Using Stencil buffer clipping" );
    else
        qDebug("OpenGL-> Using Depth buffer clipping" );

    if(s_b_useScissorTest && s_b_useStencil)
        qDebug("OpenGL-> Using Scissor Clipping" );

    /* we upload non-aligned memory */
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    MipMap_ResolveRoutines();
    SetupCompression();

    qDebug("OpenGL-> Minimum cartographic line width: %4.1f", g_GLMinCartographicLineWidth);
    qDebug("OpenGL-> Minimum symbol line width: %4.1f", g_GLMinSymbolLineWidth);

    m_benableFog = true;
    m_benableVScale = true;
#ifdef __OCPN__ANDROID__
    m_benableFog = false;
    m_benableVScale = false;
#endif

    g_GLOptions.m_bUseAcceleratedPanning =  !m_b_DisableFBO && m_b_BuiltFBO;

    if(1)     // for now upload all levels
    {
        int max_level = 0;
        int tex_dim = g_GLOptions.m_iTextureDimension;
        for(int dim=tex_dim; dim>0; dim/=2)
            max_level++;
        g_mipmap_max_level = max_level - 1;
    }

    s_b_useFBO = m_b_BuiltFBO;

    // Some older Intel GL drivers need a glFinish() call after each full frame render
    if(b_oldIntel)
        g_b_needFinish = true;

    //  Inform the S52 PLIB of options determined
    if(ps52plib)
        ps52plib->SetGLOptions(s_b_useStencil, s_b_useStencilAP, s_b_useScissorTest,  s_b_useFBO, g_b_EnableVBO, g_texture_rectangle_format);

    m_bsetup = true;
}


void ChartCanvas::SetupCompression()
{
    int dim = g_GLOptions.m_iTextureDimension;

#ifdef __WXMSW__
    if(!::IsProcessorFeaturePresent( PF_XMMI64_INSTRUCTIONS_AVAILABLE )) {
        qDebug("OpenGL-> SSE2 Instruction set not available") );
        goto no_compression;
    }
#endif

    g_uncompressed_tile_size = dim*dim*4; // stored as 32bpp in vram
    if(!g_GLOptions.m_bTextureCompression)
        goto no_compression;

    g_raster_format = GL_RGB;

    // On GLES, we prefer OES_ETC1 compression, if available
#ifdef ocpnUSE_GLES
    if(QueryExtension("GL_OES_compressed_ETC1_RGB8_texture") && s_glCompressedTexImage2D) {
        g_raster_format = GL_ETC1_RGB8_OES;

        qDebug("OpenGL-> Using oes etc1 compression") );
    }
#endif

    if(GL_RGB == g_raster_format) {
        /* because s3tc is patented, many foss drivers disable
           support by default, however the extension dxt1 allows
           us to load this texture type which is enough because we
           compress in software using libsquish for superior quality anyway */

        if((QueryExtension("GL_EXT_texture_compression_s3tc") ||
            QueryExtension("GL_EXT_texture_compression_dxt1")) &&
           s_glCompressedTexImage2D) {
            /* buggy opensource nvidia driver, renders incorrectly,
               workaround is to use format with alpha... */
            if(GetRendererString().indexOf("Gallium" ) != -1 &&
               GetRendererString().indexOf("NV" ) != -1 )
                g_raster_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            else
                g_raster_format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

            qDebug("OpenGL-> Using s3tc dxt1 compression" );
        } else if(QueryExtension("GL_3DFX_texture_compression_FXT1") &&
                  s_glCompressedTexImage2D && s_glGetCompressedTexImage) {
            g_raster_format = GL_COMPRESSED_RGB_FXT1_3DFX;

            qDebug("OpenGL-> Using 3dfx fxt1 compression" );
        } else {
            qDebug("OpenGL-> No Useable compression format found" );
            goto no_compression;
        }
    }

#ifdef ocpnUSE_GLES /* gles doesn't have GetTexLevelParameter */
    g_tile_size = 512*512/2; /* 4bpp */
#else
    /* determine compressed size of a level 0 single tile */
    GLuint texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D( GL_TEXTURE_2D, 0, g_raster_format, dim, dim,
                  0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                             GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &g_tile_size);
    glDeleteTextures(1, &texture);
#endif

    /* disable texture compression if the tile size is 0 */
    if(g_tile_size == 0)
        goto no_compression;

    qDebug("OpenGL-> Compressed tile size: %dkb (%d:1)",
                                    g_tile_size / 1024,
                                    g_uncompressed_tile_size / g_tile_size);
    return;

no_compression:
    g_GLOptions.m_bTextureCompression = false;

    g_tile_size = g_uncompressed_tile_size;
    qDebug("OpenGL-> Not Using compression");
}

void ChartCanvas::paintGL()
{
    QTime   t;
    t.start();
    if(!m_pcontext) return;
    if(!mIsUpdateAvailable) return;
    if(!m_b_paint_enable) return;
    makeCurrent();

    if( !m_bsetup ) {
        SetupOpenGL();
        m_bsetup = true;
    }
    //  If necessary, reconfigure the S52 PLIB
    UpdateCanvasS52PLIBConfig();
    Render();
    qDebug()<<"update end elaped:"<<t.elapsed()<<" ms";

}


//   These routines allow reusable coordinates
bool ChartCanvas::HasNormalizedViewPort(const ViewPort &vp)
{
    return vp.projectType() == PROJECTION_MERCATOR ||
        vp.projectType() == PROJECTION_POLAR ||
        vp.projectType() == PROJECTION_EQUIRECTANGULAR;
}

/* adjust the opengl transformation matrix so that
   points plotted using the identity viewport are correct.
   and all rotation translation and scaling is now done in opengl

   a central lat and lon of 0, 0 can be used, however objects on the far side of the world
   can be up to 3 meters off because limited floating point precision, and if the
   points cross 180 longitude then two passes will be required to render them correctly */
#define NORM_FACTOR 4096.0
void ChartCanvas::MultMatrixViewPort(ViewPort &vp, float lat, float lon)
{
    zchxPointF point;

    switch(vp.projectType()) {
    case PROJECTION_MERCATOR:
    case PROJECTION_EQUIRECTANGULAR:
    case PROJECTION_WEB_MERCATOR:
        //m_pParentCanvas->GetDoubleCanvasPointPixVP(vp, lat, lon, &point);
        point = vp.GetDoublePixFromLL(lat, lon);
        glTranslated(point.x, point.y, 0);
        glScaled(vp.viewScalePPM()/NORM_FACTOR, vp.viewScalePPM()/NORM_FACTOR, 1);
        break;

    case PROJECTION_POLAR:
        //m_pParentCanvas->GetDoubleCanvasPointPixVP(vp, vp.lat() > 0 ? 90 : -90, vp.lon(), &point);
        point = vp.GetDoublePixFromLL(vp.lat() > 0 ? 90 : -90, vp.lon());
        glTranslated(point.x, point.y, 0);
        glRotatef(vp.lon() - lon, 0, 0, vp.lat());
        glScalef(vp.viewScalePPM()/NORM_FACTOR, vp.viewScalePPM()/NORM_FACTOR, 1);
        glTranslatef(-vp.pixWidth()/2, -vp.pixHeight()/2, 0);
        break;

    default:
        qDebug("ERROR: Unhandled projection\n");
    }

    double rotation = vp.rotation();

    if (rotation)
        glRotatef(rotation*180/PI, 0, 0, 1);
}

ViewPort ChartCanvas::NormalizedViewPort(const ViewPort &vp, float lat, float lon)
{
    ViewPort cvp = vp;

    switch(vp.projectType()) {
    case PROJECTION_MERCATOR:
    case PROJECTION_EQUIRECTANGULAR:
    case PROJECTION_WEB_MERCATOR:
        cvp.setLat(lat);
        break;

    case PROJECTION_POLAR:
        cvp.setLat(vp.lat() > 0 ? 90 : -90); // either north or south polar
        break;

    default:
        printf("ERROR: Unhandled projection\n");
    }

    cvp.setLon(lon);
    cvp.setViewScalePPM(NORM_FACTOR);
    cvp.setRotation(0);
    cvp.setSkew(0);
    return cvp;
}

bool ChartCanvas::CanClipViewport(const ViewPort &vp)
{
    return vp.projectType() == PROJECTION_MERCATOR || vp.projectType() == PROJECTION_WEB_MERCATOR ||
        vp.projectType() == PROJECTION_EQUIRECTANGULAR;
}

ViewPort ChartCanvas::ClippedViewport(const ViewPort &vp, const LLRegion &region)
{
    if(!CanClipViewport(vp))
        return vp;

    ViewPort cvp = vp;
    LLBBox bbox = region.GetBox();

    if(!bbox.GetValid())
        return vp;

    /* region.GetBox() will always try to give coordinates from -180 to 180 but in
       the case where the viewport crosses the IDL, we actually want the clipped viewport
       to use coordinates outside this range to ensure the logic in the various rendering
       routines works the same here (with accelerated panning) as it does without, so we
       can adjust the coordinates here */

    if(bbox.GetMaxLon() < cvp.getBBox().GetMinLon()) {
        bbox.Set(bbox.GetMinLat(), bbox.GetMinLon() + 360,
                 bbox.GetMaxLat(), bbox.GetMaxLon() + 360);
        cvp.setBBoxDirect(bbox);
    } else if(bbox.GetMinLon() > cvp.getBBox().GetMaxLon()) {
        bbox.Set(bbox.GetMinLat(), bbox.GetMinLon() - 360,
                 bbox.GetMaxLat(), bbox.GetMaxLon() - 360);
        cvp.setBBoxDirect(bbox);
    } else
        cvp.setBBoxDirect(bbox);

    return cvp;
}




static void GetLatLonCurveDist(const ViewPort &vp, float &lat_dist, float &lon_dist)
{
    // This really could use some more thought, and possibly split at different
    // intervals based on chart skew and other parameters to optimize performance
    switch(vp.projectType()) {
    case PROJECTION_TRANSVERSE_MERCATOR:
        lat_dist = 4,   lon_dist = 1;        break;
    case PROJECTION_POLYCONIC:
        lat_dist = 2,   lon_dist = 1;        break;
    case PROJECTION_ORTHOGRAPHIC:
        lat_dist = 2,   lon_dist = 2;        break;
    case PROJECTION_POLAR:
        lat_dist = 180, lon_dist = 1;        break;
    case PROJECTION_STEREOGRAPHIC:
    case PROJECTION_GNOMONIC:
        lat_dist = 2, lon_dist = 1;          break;
    case PROJECTION_EQUIRECTANGULAR:
        // this is suboptimal because we don't care unless there is
        // a change in both lat AND lon (skewed chart)
        lat_dist = 2,   lon_dist = 360;      break;
    default:
        lat_dist = 180, lon_dist = 360;
    }
}

void ChartCanvas::RenderChartOutline( int dbIndex, ViewPort &vp )
{
    if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_PLUGIN &&
        !ChartData->IsChartAvailable( dbIndex ) )
        return;

    /* quick bounds check */
    LLBBox box;
    ChartData->GetDBBoundingBox( dbIndex, box );
    if(!box.GetValid())
        return;


    // Don't draw an outline in the case where the chart covers the entire world */
    if(box.GetLonRange() == 360)
        return;

    LLBBox vpbox = vp.getBBox();

    double lon_bias = 0;
    // chart is outside of viewport lat/lon bounding box
    if( box.IntersectOutGetBias( vp.getBBox(), lon_bias ) )
        return;

    float plylat, plylon;

    QColor color;

    if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_CM93 )
        color = GetGlobalColor( "YELO1"  );
    else if( ChartData->GetDBChartFamily( dbIndex ) == CHART_FAMILY_VECTOR )
        color = GetGlobalColor( "GREEN2" );
    else
        color = GetGlobalColor( "UINFR"  );

    if( g_GLOptions.m_GLLineSmoothing )
        glEnable( GL_LINE_SMOOTH );

    glColor3ub(color.red(), color.green(), color.blue());
    glLineWidth( g_GLMinSymbolLineWidth );

    float lat_dist, lon_dist;
    GetLatLonCurveDist(vp, lat_dist, lon_dist);

    //        Are there any aux ply entries?
    int nAuxPlyEntries = ChartData->GetnAuxPlyEntries( dbIndex ), nPly;
    int j=0;
    do {
        if(nAuxPlyEntries)
            nPly = ChartData->GetDBAuxPlyPoint( dbIndex, 0, j, 0, 0 );
        else
            nPly = ChartData->GetDBPlyPoint( dbIndex, 0, &plylat, &plylon );

        bool begin = false, sml_valid = false;
        double sml[2];
        float lastplylat = 0.0;
        float lastplylon = 0.0;
        // modulo is undefined for zero (compiler can use a div operation)
        int modulo = (nPly == 0)?1:nPly;
        for( int i = 0; i < nPly+1; i++ ) {
            if(nAuxPlyEntries)
                ChartData->GetDBAuxPlyPoint( dbIndex, i % modulo, j, &plylat, &plylon );
            else
                ChartData->GetDBPlyPoint( dbIndex, i % modulo, &plylat, &plylon );

            plylon += lon_bias;

            if(lastplylon - plylon > 180)
                lastplylon -= 360;
            else if(lastplylon - plylon < -180)
                lastplylon += 360;

            int splits;
            if(i==0)
                splits = 1;
            else {
                int lat_splits = floor(fabs(plylat-lastplylat) / lat_dist);
                int lon_splits = floor(fabs(plylon-lastplylon) / lon_dist);
                splits = fmax(lat_splits, lon_splits) + 1;
            }

            double smj[2];
            if(splits != 1) {
                // must perform border interpolation in mercator space as this is what the charts use
                toSM(plylat, plylon, 0, 0, smj+0, smj+1);
                if(!sml_valid)
                    toSM(lastplylat, lastplylon, 0, 0, sml+0, sml+1);
            }

            for(double c=0; c<splits; c++) {
                double lat, lon;
                if(c == splits - 1)
                    lat = plylat, lon = plylon;
                else {
                    double d = (double)(c+1) / splits;
                    fromSM(d*smj[0] + (1-d)*sml[0], d*smj[1] + (1-d)*sml[1], 0, 0, &lat, &lon);
                }

                zchxPointF s;
                GetDoubleCanvasPointPix( lat, lon, s );
                if(!std::isnan(s.x)) {
                    if(!begin) {
                        begin = true;
                        glBegin(GL_LINE_STRIP);
                    }
                    glVertex2f( s.x, s.y );
                } else if(begin) {
                    glEnd();
                    begin = false;
                }
            }
            if((sml_valid = splits != 1))
                memcpy(sml, smj, sizeof smj);
            lastplylat = plylat, lastplylon = plylon;
        }

        if(begin)
            glEnd();

    } while(++j < nAuxPlyEntries );                 // There are no aux Ply Point entries

    glDisable( GL_LINE_SMOOTH );
//    glDisable( GL_BLEND );
}

extern void CalcGridSpacing( float WindowDegrees, float& MajorSpacing, float&MinorSpacing );
extern QString CalcGridText( float latlon, float spacing, bool bPostfix );
void ChartCanvas::GridDraw( )
{
    if( !m_bDisplayGrid ) return;

    ViewPort &vp = GetVP();

    // TODO: make minor grid work all the time
    bool minorgrid = fabs( vp.rotation() ) < 0.0001 &&
        vp.projectType() == PROJECTION_MERCATOR;

    double nlat, elon, slat, wlon;
    float lat, lon;
    float gridlatMajor, gridlatMinor, gridlonMajor, gridlonMinor;
    int w, h;

    QColor GridColor = GetGlobalColor( "SNDG1"  );

    if(!m_gridfont.IsBuilt()){
        QFont dFont = FontMgr::Get().GetFont( ("ChartTexts"), 0 );
        QFont font = dFont;
        font.setPointSize(8);
        font.setWeight(QFont::Weight::Normal);

        m_gridfont.Build(font);
    }

    w = vp.pixWidth();
    h = vp.pixHeight();

    LLBBox llbbox = vp.getBBox();
    nlat = llbbox.GetMaxLat();
    slat = llbbox.GetMinLat();
    elon = llbbox.GetMaxLon();
    wlon = llbbox.GetMinLon();

    // calculate distance between latitude grid lines
    CalcGridSpacing( vp.viewScalePPM(), gridlatMajor, gridlatMinor );
    CalcGridSpacing( vp.viewScalePPM(), gridlonMajor, gridlonMinor );


    // if it is known the grid has straight lines it's a bit faster
    bool straight_latitudes =
        vp.projectType() == PROJECTION_MERCATOR ||
        vp.projectType() == PROJECTION_WEB_MERCATOR ||
        vp.projectType() == PROJECTION_EQUIRECTANGULAR;
    bool straight_longitudes =
        vp.projectType() == PROJECTION_MERCATOR ||
        vp.projectType() == PROJECTION_WEB_MERCATOR ||
        vp.projectType() == PROJECTION_POLAR ||
        vp.projectType() == PROJECTION_EQUIRECTANGULAR;

    double latmargin;
    if(straight_latitudes)
        latmargin = 0;
    else
        latmargin = gridlatMajor / 2; // don't draw on poles

    slat = fmax(slat, -90 + latmargin);
    nlat = fmin(nlat,  90 - latmargin);

    float startlat = ceil( slat / gridlatMajor ) * gridlatMajor;
    float startlon = ceil( wlon / gridlonMajor ) * gridlonMajor;
    float curved_step = fmin(sqrt(5e-3 / vp.viewScalePPM()), 3);

    ocpnDC gldc( this );
    QPen pen( GridColor, g_GLMinSymbolLineWidth, Qt::SolidLine );
    gldc.SetPen( pen );

    // Draw Major latitude grid lines and text

    // calculate position of first major latitude grid line
    float lon_step = elon - wlon;
    if(!straight_latitudes)
        lon_step /= ceil(lon_step / curved_step);

    for(lat = startlat; lat < nlat; lat += gridlatMajor) {
        zchxPointF r, s;
        s.x = NAN;

        for(lon = wlon; lon < elon+lon_step/2; lon += lon_step) {
            m_pParentCanvas->GetDoubleCanvasPointPix( lat, lon, r );
            if(!std::isnan(s.x) && !std::isnan(r.x)) {
                gldc.DrawLine( s.x, s.y, r.x, r.y, true );
            }
            s = r;
        }
    }

    if(minorgrid) {
        // draw minor latitude grid lines
        for(lat = ceil( slat / gridlatMinor ) * gridlatMinor; lat < nlat; lat += gridlatMinor) {

            zchxPoint r;
            m_pParentCanvas->GetCanvasPointPix( lat, ( elon + wlon ) / 2, r );
            gldc.DrawLine( 0, r.y, 10, r.y, true );
            gldc.DrawLine( w - 10, r.y, w, r.y, true );

            lat = lat + gridlatMinor;
        }
    }

    // draw major longitude grid lines
    float lat_step = nlat - slat;
    if(!straight_longitudes)
        lat_step /= ceil(lat_step / curved_step);

    for(lon = startlon; lon < elon; lon += gridlonMajor) {
        zchxPointF r, s;
        s.x = NAN;
        for(lat = slat; lat < nlat+lat_step/2; lat+=lat_step) {
            m_pParentCanvas->GetDoubleCanvasPointPix( lat, lon, r );

            if(!std::isnan(s.x) && !std::isnan(r.x)) {
                gldc.DrawLine( s.x, s.y, r.x, r.y, true );
            }
            s = r;
        }
    }

    if(minorgrid) {
        // draw minor longitude grid lines
        for(lon = ceil( wlon / gridlonMinor ) * gridlonMinor; lon < elon; lon += gridlonMinor) {
            zchxPoint r;
            m_pParentCanvas->GetCanvasPointPix( ( nlat + slat ) / 2, lon, r );
            gldc.DrawLine( r.x, 0, r.x, 10, true );
            gldc.DrawLine( r.x, h-10, r.x, h, true );
        }
    }


    // draw text labels
    glEnable(GL_TEXTURE_2D);
    glEnable( GL_BLEND );
    for(lat = startlat; lat < nlat; lat += gridlatMajor) {
        if( fabs( lat - qRound( lat ) ) < 1e-5 )
            lat = qRound( lat );

        QString st = CalcGridText( lat, gridlatMajor, true ); // get text for grid line
        int iy;
        m_gridfont.GetTextExtent(st, 0, &iy);

        if(straight_latitudes) {
            zchxPoint r, s;
            m_pParentCanvas->GetCanvasPointPix( lat, elon, r );
            m_pParentCanvas->GetCanvasPointPix( lat, wlon, s );

            float x = 0, y = -1;
            y = (float)(r.y*s.x - s.y*r.x) / (s.x - r.x);
            if(y < 0 || y > h) {
                y = h - iy;
                x = (float)(r.x*s.y - s.x*r.y + (s.x - r.x)*y) / (s.y - r.y);
            }

            m_gridfont.RenderString(st, x, y);
        } else {
            // iteratively attempt to find where the latitude line crosses x=0
            zchxPointF r;
            double y1, y2, lat1, lon1, lat2, lon2;

            y1 = 0, y2 = vp.pixHeight();
            double error = vp.pixWidth(), lasterror;
            int maxiters = 10;
            do {
                m_pParentCanvas->GetCanvasPixPoint(0, y1, lat1, lon1);
                m_pParentCanvas->GetCanvasPixPoint(0, y2, lat2, lon2);

                double y = y1 + (lat1 - lat) * (y2 - y1) / (lat1 - lat2);

                m_pParentCanvas->GetDoubleCanvasPointPix( lat, lon1 + (y1 - y) * (lon2 - lon1) / (y1 - y2), r);

                if(fabs(y - y1) < fabs(y - y2))
                    y1 = y;
                else
                    y2 = y;

                lasterror = error;
                error = fabs(r.x);
                if(--maxiters == 0)
                    break;
            } while(error > 1 && error < lasterror);

            if(error < 1 && r.y >= 0 && r.y <= vp.pixHeight() - iy )
                r.x = 0;
            else
                // just draw at center longitude
                m_pParentCanvas->GetDoubleCanvasPointPix( lat, vp.lon(), r);

            m_gridfont.RenderString(st, r.x, r.y);
        }
    }


    for(lon = startlon; lon < elon; lon += gridlonMajor) {
        if( fabs( lon - qRound( lon ) ) < 1e-5 )
            lon = qRound( lon );

        zchxPoint r, s;
        m_pParentCanvas->GetCanvasPointPix( nlat, lon, r );
        m_pParentCanvas->GetCanvasPointPix( slat, lon, s );

        float xlon = lon;
        if( xlon > 180.0 )
            xlon -= 360.0;
        else if( xlon <= -180.0 )
            xlon += 360.0;

        QString st = CalcGridText( xlon, gridlonMajor, false );
        int ix;
        m_gridfont.GetTextExtent(st, &ix, 0);

        if(straight_longitudes) {
            float x = -1, y = 0;
            x = (float)(r.x*s.y - s.x*r.y) / (s.y - r.y);
            if(x < 0 || x > w) {
                x = w - ix;
                y = (float)(r.y*s.x - s.y*r.x + (s.y - r.y)*x) / (s.x - r.x);
            }

            m_gridfont.RenderString(st, x, y);
        } else {
            // iteratively attempt to find where the latitude line crosses x=0
            zchxPointF r;
            double x1, x2, lat1, lon1, lat2, lon2;

            x1 = 0, x2 = vp.pixWidth();
            double error = vp.pixHeight(), lasterror;
            do {
                m_pParentCanvas->GetCanvasPixPoint(x1, 0, lat1, lon1);
                m_pParentCanvas->GetCanvasPixPoint(x2, 0, lat2, lon2);

                double x = x1 + (lon1 - lon) * (x2 - x1) / (lon1 - lon2);

                m_pParentCanvas->GetDoubleCanvasPointPix( lat1 + (x1 - x) * (lat2 - lat1) / (x1 - x2), lon, r);

                if(fabs(x - x1) < fabs(x - x2))
                    x1 = x;
                else
                    x2 = x;

                lasterror = error;
                error = fabs(r.y);
            } while(error > 1 && error < lasterror);

            if(error < 1 && r.x >= 0 && r.x <= vp.pixWidth() - ix)
                r.y = 0;
            else
                // failure, instead just draw the text at center latitude
                m_pParentCanvas->GetDoubleCanvasPointPix( fmin(fmax(vp.lat(), slat), nlat), lon, r);

            m_gridfont.RenderString(st, r.x, r.y);
        }
    }

    glDisable(GL_TEXTURE_2D);

    glDisable( GL_BLEND );
}


void ChartCanvas::DrawEmboss( emboss_data *emboss  )
{
    if( !emboss ) return;

    int w = emboss->width, h = emboss->height;

    glEnable( GL_TEXTURE_2D );

    // render using opengl and alpha blending
    if( !emboss->gltexind ) { /* upload to texture */

        emboss->glwidth = NextPow2(emboss->width);
        emboss->glheight = NextPow2(emboss->height);

        /* convert to luminance alpha map */
        int size = emboss->glwidth * emboss->glheight;
        char *data = new char[2 * size];
        for( int i = 0; i < h; i++ ) {
            for( int j = 0; j < emboss->glwidth; j++ ) {
                if( j < w ) {
                    data[2 * ( ( i * emboss->glwidth ) + j )] =
                        (char) (emboss->pmap[( i * w ) + j] > 0 ? 0 : 255);
                    data[2 * ( ( i * emboss->glwidth ) + j ) + 1] =
                        (char) abs( (emboss->pmap[( i * w ) + j]) );
                }
            }
        }

        glGenTextures( 1, &emboss->gltexind );
        glBindTexture( GL_TEXTURE_2D, emboss->gltexind );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, emboss->glwidth, emboss->glheight, 0,
                      GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

        delete[] data;
    }

    glBindTexture( GL_TEXTURE_2D, emboss->gltexind );

    glEnable( GL_BLEND );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    const float factor = 200;
    glColor4f( 1, 1, 1, factor / 256 );

    int x = emboss->x, y = emboss->y;

    float wp = (float) w / emboss->glwidth;
    float hp = (float) h / emboss->glheight;

    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 ), glVertex2i( x, y );
    glTexCoord2f( wp, 0 ), glVertex2i( x + w, y );
    glTexCoord2f( wp, hp ), glVertex2i( x + w, y + h );
    glTexCoord2f( 0, hp ), glVertex2i( x, y + h );
    glEnd();

    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
}


void ChartCanvas::DrawFloatingOverlayObjects( ocpnDC &dc )
{
    ViewPort &vp = m_pParentCanvas->GetVP();
    GridDraw( );

    g_overlayCanvas = m_pParentCanvas;
    m_pParentCanvas->ScaleBarDraw( dc );
    s57_DrawExtendedLightSectors( dc, m_pParentCanvas->VPoint, m_pParentCanvas->extendedSectorLegs );
}

void ChartCanvas::DrawQuiting()
{
    GLubyte pattern[8][8];
    for( int y = 0; y < 8; y++ )
        for( int x = 0; x < 8; x++ )
            pattern[y][x] = (y == x) * 255;

    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glBindTexture(GL_TEXTURE_2D, 0);

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 8, 8,
                  0, GL_ALPHA, GL_UNSIGNED_BYTE, pattern );
    glColor3f( 0, 0, 0 );

    float x = width(), y = height();
    float u = x / 8, v = y / 8;

    glBegin( GL_QUADS );
    glTexCoord2f(0, 0); glVertex2f( 0, 0 );
    glTexCoord2f(0, v); glVertex2f( 0, y );
    glTexCoord2f(u, v); glVertex2f( x, y );
    glTexCoord2f(u, 0); glVertex2f( x, 0 );
    glEnd();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void ChartCanvas::DrawCloseMessage(QString msg)
{
    if(1){

        QFont pfont = FontMgr::Get().FindOrCreateFont(12, "Microsoft YaHei", QFont::StyleNormal, QFont::Weight::Bold);

        TexFont texfont;

        texfont.Build(pfont);
        int w, h;
        texfont.GetTextExtent( msg, &w, &h);
        h += 2;
        int yp = m_pParentCanvas->GetVP().pixHeight()/2;
        int xp = (m_pParentCanvas->GetVP().pixWidth() - w)/2;

        glColor3ub( 243, 229, 47 );

        glBegin(GL_QUADS);
        glVertex2i(xp, yp);
        glVertex2i(xp+w, yp);
        glVertex2i(xp+w, yp+h);
        glVertex2i(xp, yp+h);
        glEnd();

        glEnable(GL_BLEND);

        glColor3ub( 0, 0, 0 );
        glEnable(GL_TEXTURE_2D);
        texfont.RenderString( msg, xp, yp);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
}

void ChartCanvas::RotateToViewPort(const ViewPort &vp)
{
    float angle = vp.rotation();

    qDebug()<<"current roate:"<<vp.rotation();
    if( fabs( angle ) > 0.0001 )
    {
        //    Rotations occur around 0,0, so translate to rotate around screen center
        float xt = vp.pixWidth() / 2.0, yt = vp.pixHeight() / 2.0;
        qDebug()<<"dxy:"<<xt<<yt;
        glTranslatef( xt, yt, 0 );
        glRotatef( angle * 180. / PI, 0, 0, 1 );
        glTranslatef( -xt, -yt, 0 );
    }
}

static std::list<double*> combine_work_data;
static void combineCallbackD(GLdouble coords[3],
                             GLdouble *vertex_data[4],
                             GLfloat weight[4], GLdouble **dataOut )
{
    double *vertex = new double[3];
    combine_work_data.push_back(vertex);
    memcpy(vertex, coords, 3*(sizeof *coords));
    *dataOut = vertex;
}

void vertexCallbackD(GLvoid *vertex)
{
    glVertex3dv( (GLdouble *)vertex);
}

void beginCallbackD( GLenum mode)
{
    glBegin( mode );
}

void endCallbackD()
{
    glEnd();
}

void ChartCanvas::DrawRegion(ViewPort &vp, const LLRegion &region)
{
    float lat_dist, lon_dist;
    GetLatLonCurveDist(vp, lat_dist, lon_dist);

    GLUtesselator *tobj = gluNewTess();

    gluTessCallback( tobj, GLU_TESS_VERTEX, (_GLUfuncptr) &vertexCallbackD  );
    gluTessCallback( tobj, GLU_TESS_BEGIN, (_GLUfuncptr) &beginCallbackD  );
    gluTessCallback( tobj, GLU_TESS_END, (_GLUfuncptr) &endCallbackD  );
    gluTessCallback( tobj, GLU_TESS_COMBINE, (_GLUfuncptr) &combineCallbackD );

    gluTessNormal( tobj, 0, 0, 1);

    gluTessBeginPolygon(tobj, NULL);
    for(std::list<poly_contour>::const_iterator i = region.contours.begin(); i != region.contours.end(); i++) {
        gluTessBeginContour(tobj);
        contour_pt l = *i->rbegin();
        double sml[2];
        bool sml_valid = false;
        for(poly_contour::const_iterator j = i->begin(); j != i->end(); j++) {
            int lat_splits = floor(fabs(j->y - l.y) / lat_dist);
            int lon_splits = floor(fabs(j->x - l.x) / lon_dist);
            int splits = fmax(lat_splits, lon_splits) + 1;

            double smj[2];
            if(splits != 1) {
                // must perform border interpolation in mercator space as this is what the charts use
                toSM(j->y, j->x, 0, 0, smj+0, smj+1);
                if(!sml_valid)
                    toSM(l.y, l.x, 0, 0, sml+0, sml+1);
            }

            for(int i = 0; i<splits; i++) {
                double lat, lon;
                if(i == splits - 1)
                    lat = j->y, lon = j->x;
                else {
                    double d = (double)(i+1) / splits;
                    fromSM(d*smj[0] + (1-d)*sml[0], d*smj[1] + (1-d)*sml[1], 0, 0, &lat, &lon);
                }
                zchxPointF q = vp.GetDoublePixFromLL(lat, lon);
                if(std::isnan(q.x))
                    continue;

                double *p = new double[6];
                p[0] = q.x, p[1] = q.y, p[2] = 0;
                gluTessVertex(tobj, p, p);
                combine_work_data.push_back(p);
            }
            l = *j;

            if((sml_valid = splits != 1))
                memcpy(sml, smj, sizeof smj);
        }
        gluTessEndContour(tobj);
    }
    gluTessEndPolygon(tobj);

    gluDeleteTess(tobj);

    for(std::list<double*>::iterator i = combine_work_data.begin(); i!=combine_work_data.end(); i++)
        delete [] *i;
    combine_work_data.clear();
}

/* set stencil buffer to clip in this region, and optionally clear using the current color */
void ChartCanvas::SetClipRegion(ViewPort &vp, const LLRegion &region)
{
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer

    if( s_b_useStencil ) {
        //    Create a stencil buffer for clipping to the region
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x1 );                 // write only into bit 0 of the stencil buffer
        glClear( GL_STENCIL_BUFFER_BIT );

        //    We are going to write "1" into the stencil buffer wherever the region is valid
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
    } else              //  Use depth buffer for clipping
    {
        glEnable( GL_DEPTH_TEST ); // to enable writing to the depth buffer
        glDepthFunc( GL_ALWAYS );  // to ensure everything you draw passes
        glDepthMask( GL_TRUE );    // to allow writes to the depth buffer

        glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start

        //    Decompose the region into rectangles, and draw as quads
        //    With z = 1
            // dep buffer clear = 1
            // 1 makes 0 in dep buffer, works
            // 0 make .5 in depth buffer
            // -1 makes 1 in dep buffer

            //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
            //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
            //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
        glTranslatef( 0, 0, .5 );
    }

    DrawRegion(vp, region);

    if( s_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    } else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
        glTranslatef( 0, 0, -.5 ); // reset translation
    }

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer
}

void ChartCanvas::SetClipRect(const ViewPort &vp, const QRect &rect, bool b_clear)
{
    /* for some reason this causes an occasional bug in depth mode, I cannot
       seem to solve it yet, so for now: */
    if(s_b_useStencil && s_b_useScissorTest) {
        QRect vp_rect(0, 0, vp.pixWidth(), vp.pixHeight());
        if(rect != vp_rect) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(rect.x(), vp.pixHeight() - rect.height() - rect.y(), rect.width(), rect.height());
        }

        if(b_clear) {
            glBegin(GL_QUADS);
            glVertex2i( rect.x(), rect.y() );
            glVertex2i( rect.x() + rect.width(), rect.y() );
            glVertex2i( rect.x() + rect.width(), rect.y() + rect.height() );
            glVertex2i( rect.x(), rect.y() + rect.height() );
            glEnd();
        }

        /* the code in s52plib depends on the depth buffer being
           initialized to this value, this code should go there instead and
           only a flag set here. */
        if(!s_b_useStencil) {
            glClearDepth( 0.25 );
            glDepthMask( GL_TRUE );    // to allow writes to the depth buffer
            glClear( GL_DEPTH_BUFFER_BIT );
            glDepthMask( GL_FALSE );
            glClearDepth( 1 ); // set back to default of 1
            glDepthFunc( GL_GREATER );                          // Set the test value
        }
        return;
    }

    // slower way if there is no scissor support
    if(!b_clear)
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer

    if( s_b_useStencil ) {
        //    Create a stencil buffer for clipping to the region
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x1 );                 // write only into bit 0 of the stencil buffer
        glClear( GL_STENCIL_BUFFER_BIT );

        //    We are going to write "1" into the stencil buffer wherever the region is valid
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
    } else              //  Use depth buffer for clipping
    {
        glEnable( GL_DEPTH_TEST ); // to enable writing to the depth buffer
        glDepthFunc( GL_ALWAYS );  // to ensure everything you draw passes
        glDepthMask( GL_TRUE );    // to allow writes to the depth buffer

        glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start

        //    Decompose the region into rectangles, and draw as quads
        //    With z = 1
            // dep buffer clear = 1
            // 1 makes 0 in dep buffer, works
            // 0 make .5 in depth buffer
            // -1 makes 1 in dep buffer

            //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
            //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
            //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
        glTranslatef( 0, 0, .5 );
    }

    glBegin(GL_QUADS);
    glVertex2i( rect.x(), rect.y() );
    glVertex2i( rect.x() + rect.width(), rect.y() );
    glVertex2i( rect.x() + rect.width(), rect.y() + rect.height() );
    glVertex2i( rect.x(), rect.y() + rect.height() );
    glEnd();

    if( s_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    } else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
        glTranslatef( 0, 0, -.5 ); // reset translation
    }

    if(!b_clear)
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer
}

void ChartCanvas::DisableClipRegion()
{
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_DEPTH_TEST );
}

void ChartCanvas::Invalidate()
{
    /* should probably use a different flag for this */

    m_pParentCanvas->m_glcc->m_cache_vp.invalidate();

}

void ChartCanvas::RenderRasterChartRegionGL( ChartBase *chart, ViewPort &vp, LLRegion &region )
{
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( chart );
    if( !pBSBChart ) return;

    if(b_inCompressAllCharts) return; // don't want multiple texfactories to exist


    //    Look for the texture factory for this chart
    QString key = chart->GetHashKey();

    glTexFactory *pTexFact;
    ChartPathHashTexfactType &hash = g_glTextureManager->m_chart_texfactory_hash;
    ChartPathHashTexfactType::iterator ittf = hash.find( key );

    //    Not Found ?
    if( ittf == hash.end() ){
        hash[key] = new glTexFactory(chart, g_raster_format);
        hash[key]->SetHashKey(key);
    }

    pTexFact = hash[key];
    pTexFact->SetLRUTime(++m_LRUtime);

    // for small scales, don't use normalized coordinates for accuracy (difference is up to 3 meters error)
    bool use_norm_vp = ChartCanvas::HasNormalizedViewPort(vp) && pBSBChart->GetPPM() < 1;
    pTexFact->PrepareTiles(vp, use_norm_vp, pBSBChart);

    //    For underzoom cases, we will define the textures as having their base levels
    //    equivalent to a level "n" mipmap, where n is calculated, and is always binary
    //    This way we can avoid loading much texture memory

    int base_level;
    if(vp.projectType() == PROJECTION_MERCATOR &&
       chart->GetChartProjectionType() == PROJECTION_MERCATOR) {
        double scalefactor = pBSBChart->GetRasterScaleFactor(vp);
        base_level = log(scalefactor) / log(2.0);

        if(base_level < 0) /* for overzoom */
            base_level = 0;
        if(base_level > g_mipmap_max_level)
            base_level = g_mipmap_max_level;
    } else
        base_level = 0; // base level should be computed per tile, for now load all

    /* setup opengl parameters */
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if(use_norm_vp) {
        glPushMatrix();
        double lat, lon;
        pTexFact->GetCenter(lat, lon);
        MultMatrixViewPort(vp, lat, lon);
    }

    LLBBox box = region.GetBox();
    int numtiles;
    int mem_used = 0;
    if (g_memCacheLimit > 0) {
        // GetMemoryStatus is slow on linux
        zchxFuncUtil::getMemoryStatus(0, &mem_used);
    }

    glTexTile **tiles = pTexFact->GetTiles(numtiles);
    for(int i = 0; i<numtiles; i++) {
        glTexTile *tile = tiles[i];
        if(region.IntersectOut(tile->box)) {

            /*   user setting is in MB while we count exact bytes */
            bool bGLMemCrunch = g_tex_mem_used > g_GLOptions.m_iTextureMemorySize * 1024 * 1024;
            if( bGLMemCrunch)
                pTexFact->DeleteTexture( tile->rect );
        } else {
            bool texture = pTexFact->PrepareTexture( base_level, tile->rect, global_color_scheme, mem_used );
            if(!texture) { // failed to load, draw red
                glDisable(GL_TEXTURE_2D);
                glColor3f(1, 0, 0);
            }

            float *coords;
            if(use_norm_vp)
                coords = tile->m_coords;
            else {
                coords = new float[2 * tile->m_ncoords];
                for(int i=0; i<tile->m_ncoords; i++) {
                    zchxPointF p = vp.GetDoublePixFromLL(tile->m_coords[2*i+0],
                                                              tile->m_coords[2*i+1]);
                    coords[2*i+0] = p.x;
                    coords[2*i+1] = p.y;
                }
            }

            glTexCoordPointer(2, GL_FLOAT, 2*sizeof(GLfloat), tile->m_texcoords);
            glVertexPointer(2, GL_FLOAT, 2*sizeof(GLfloat), coords);
            glDrawArrays(GL_QUADS, 0, tile->m_ncoords);

            if(!texture)
                glEnable(GL_TEXTURE_2D);

            if(!use_norm_vp)
                delete [] coords;
        }
    }

    glDisable(GL_TEXTURE_2D);

    if(use_norm_vp)
        glPopMatrix();

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void ChartCanvas::RenderQuiltViewGL( ViewPort &vp, const OCPNRegion &rect_region )
{
    if( !m_pParentCanvas->m_pQuilt->GetnCharts() || m_pParentCanvas->m_pQuilt->IsBusy() )
        return;

    //  render the quilt
        ChartBase *chart = m_pParentCanvas->m_pQuilt->GetFirstChart();

    //  Check the first, smallest scale chart
    if(chart) {
        //            if( ! m_pParentCanvas->IsChartLargeEnoughToRender( chart, vp ) )
//            chart = NULL;
    }

    LLRegion region = vp.GetLLRegion(rect_region);

    LLRegion rendered_region;
    while( chart ) {

        //  This test does not need to be done for raster charts, since
        //  we can assume that texture binding is acceptably fast regardless of the render region,
        //  and that the quilt zoom methods choose a reasonable reference chart.
        if(chart->GetChartFamily() != CHART_FAMILY_RASTER)
        {
            //                if( ! m_pParentCanvas->IsChartLargeEnoughToRender( chart, vp ) ) {
                //                    chart = m_pParentCanvas->m_pQuilt->GetNextChart();
//                    continue;
//                }
        }

        QuiltPatch *pqp = m_pParentCanvas->m_pQuilt->GetCurrentPatch();
        if( pqp->b_Valid ) {
            LLRegion get_region = pqp->ActiveRegion;
            bool b_rendered = false;

            if( !pqp->b_overlay ) {
                get_region.Intersect( region );
                if( !get_region.Empty() ) {
                    if( chart->GetChartFamily() == CHART_FAMILY_RASTER ) {

                        ChartBaseBSB *Patch_Ch_BSB = dynamic_cast<ChartBaseBSB*>( chart );
                        if (Patch_Ch_BSB) {
                            SetClipRegion(vp, pqp->ActiveRegion/*pqp->quilt_region*/);
                            RenderRasterChartRegionGL( chart, vp, get_region );
                            DisableClipRegion();
                            b_rendered = true;
                        }
                        else if(chart->GetChartType() == CHART_TYPE_MBTILES){
                            SetClipRegion(vp, pqp->ActiveRegion/*pqp->quilt_region*/);
                            chart->RenderRegionViewOnGL( m_pcontext, vp, rect_region, get_region );
                            DisableClipRegion();
                        }

                    } else if(chart->GetChartFamily() == CHART_FAMILY_VECTOR ) {

                        if(chart->GetChartType() == CHART_TYPE_CM93COMP){
                           RenderNoDTA(vp, get_region);
                           chart->RenderRegionViewOnGL( m_pcontext, vp, rect_region, get_region );
                        }
                        else{
                            s57chart *Chs57 = dynamic_cast<s57chart*>( chart );
                            if(Chs57){
                                if(Chs57->m_RAZBuilt){
                                    RenderNoDTA(vp, get_region);
                                    Chs57->RenderRegionViewOnGLNoText(m_pcontext, vp, rect_region, get_region );
                                    DisableClipRegion();
                                }
                                else{
                                    // The SENC is quesed for building, so..
                                    // Show GSHHS with compatible color scheme in the meantime.
                                    ocpnDC gldc( this );
                                    const LLRegion &oregion = get_region;
                                    LLBBox box = oregion.GetBox();

                                    zchxPoint p1 = vp.GetPixFromLL( box.GetMaxLat(), box.GetMinLon());
                                    zchxPoint p2 = vp.GetPixFromLL( box.GetMaxLat(), box.GetMaxLon());
                                    zchxPoint p3 = vp.GetPixFromLL( box.GetMinLat(), box.GetMaxLon());
                                    zchxPoint p4 = vp.GetPixFromLL( box.GetMinLat(), box.GetMinLon());

                                    QRect srect(p1.x, p1.y, p3.x - p1.x, p4.y - p2.y);

                                    bool world = false;
                                    ViewPort cvp = ClippedViewport(vp, get_region);
                                    if( m_pParentCanvas->GetWorldBackgroundChart()){
                                        SetClipRegion(cvp, get_region);
                                        m_pParentCanvas->GetWorldBackgroundChart()->SetColorsDirect(GetGlobalColor( ( "LANDA" ) ), GetGlobalColor( ( "DEPMS" )));
                                        RenderWorldChart(gldc, cvp, srect, world);
                                        m_pParentCanvas->GetWorldBackgroundChart()->SetColorScheme(global_color_scheme);
                                        DisableClipRegion();
                                    }
                                }
                            }
                            else{
                                ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                                if(ChPI){
                                    RenderNoDTA(vp, get_region);
                                    ChPI->RenderRegionViewOnGLNoText(m_pcontext, vp, rect_region, get_region );
                                }
                                else{
                                    RenderNoDTA(vp, get_region);
                                    chart->RenderRegionViewOnGL(m_pcontext, vp, rect_region, get_region );
                                }
                            }
                        }
                     }
                }
            }

            if(b_rendered) {
//                LLRegion get_region = pqp->ActiveRegion;
//                    get_region.Intersect( Region );  not technically required?
//                rendered_region.Union(get_region);
            }
        }


        chart = m_pParentCanvas->m_pQuilt->GetNextChart();
    }

    //    Render any Overlay patches for s57 charts(cells)
    if( m_pParentCanvas->m_pQuilt->HasOverlays() ) {
        ChartBase *pch = m_pParentCanvas->m_pQuilt->GetFirstChart();
        while( pch ) {
            QuiltPatch *pqp = m_pParentCanvas->m_pQuilt->GetCurrentPatch();
            if( pqp->b_Valid && pqp->b_overlay && pch->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                LLRegion get_region = pqp->ActiveRegion;

                get_region.Intersect( region );
                if( !get_region.Empty()  ) {
                    s57chart *Chs57 = dynamic_cast<s57chart*>( pch );
                    if( Chs57 )
                        Chs57->RenderOverlayRegionViewOnGL(m_pcontext, vp, rect_region, get_region );
                    else{
                        ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( pch );
                        if(ChPI){
                            ChPI->RenderRegionViewOnGL(m_pcontext, vp, rect_region, get_region );
                        }
                    }

                }
            }

            pch = m_pParentCanvas->m_pQuilt->GetNextChart();
        }
    }

    // Hilite rollover patch
    LLRegion hiregion = m_pParentCanvas->m_pQuilt->GetHiliteRegion();

    if( !hiregion.Empty() ) {
        glEnable( GL_BLEND );

        double hitrans;
        switch( global_color_scheme ) {
        case GLOBAL_COLOR_SCHEME_DAY:
            hitrans = .4;
            break;
        case GLOBAL_COLOR_SCHEME_DUSK:
            hitrans = .2;
            break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
            hitrans = .1;
            break;
        default:
            hitrans = .4;
            break;
        }

        glColor4f( (float) .8, (float) .4, (float) .4, (float) hitrans );

        DrawRegion(vp, hiregion);

        glDisable( GL_BLEND );
    }
    m_pParentCanvas->m_pQuilt->SetRenderedVP( vp );

#if 0
    if(m_bfogit) {
        double scale_factor = vp.ref_scale/vp.chart_scale;
        float fog = ((scale_factor - g_overzoom_emphasis_base) * 255.) / 20.;
        fog = fmin(fog, 200.);         // Don't blur completely

        if( !rendered_region.Empty() ) {

            int width = vp.pixWidth();
            int height = vp.pixHeight();

            // Use MipMap LOD tweaking to produce a blurred, downsampling effect at reasonable speed.

            if( (s_glGenerateMipmap) && (g_texture_rectangle_format == GL_TEXTURE_2D)){       //nPOT texture supported

                //          Capture the rendered screen image to a texture
                glReadBuffer( GL_BACK);

                GLuint screen_capture;
                glGenTextures( 1, &screen_capture );

                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, screen_capture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
                glCopyTexSubImage2D(GL_TEXTURE_2D,  0,  0,  0, 0,  0,  width, height);

                glClear(GL_DEPTH_BUFFER_BIT);
                glDisable(GL_DEPTH_TEST);

                //  Build MipMaps
                int max_mipmap = 3;
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_mipmap );

                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, max_mipmap);

                glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

                s_glGenerateMipmap(GL_TEXTURE_2D);

                // Render at reduced LOD (i.e. higher mipmap number)
                double bias = fog/70;
                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias);
                glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );


                glBegin(GL_QUADS);

                glTexCoord2f(0 , 1 ); glVertex2i(0,     0);
                glTexCoord2f(0 , 0 ); glVertex2i(0,     height);
                glTexCoord2f(1 , 0 ); glVertex2i(width, height);
                glTexCoord2f(1 , 1 ); glVertex2i(width, 0);
                glEnd ();

                glDeleteTextures(1, &screen_capture);

                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
                glDisable(GL_TEXTURE_2D);
            }

            else if(scale_factor > 20){
                // Fogging by alpha blending
                fog = ((scale_factor - 20) * 255.) / 20.;

                glEnable( GL_BLEND );

                fog = fmin(fog, 150.);         // Don't fog out completely

                QColor color = m_pParentCanvas->GetFogColor();
                glColor4ub( color.Red(), color.Green(), color.Blue(), (int)fog );

                DrawRegion(vp, rendered_region);
                glDisable( GL_BLEND );
            }
        }
    }
#endif
}

void ChartCanvas::RenderQuiltViewGLText( ViewPort &vp, const OCPNRegion &rect_region )
{
    if( !m_pParentCanvas->m_pQuilt->GetnCharts() || m_pParentCanvas->m_pQuilt->IsBusy() )
        return;

    //  render the quilt
        ChartBase *chart = m_pParentCanvas->m_pQuilt->GetLargestScaleChart();

        LLRegion region = vp.GetLLRegion(rect_region);

        LLRegion rendered_region;
        while( chart ) {

            QuiltPatch *pqp = m_pParentCanvas->m_pQuilt->GetCurrentPatch();
            if( pqp->b_Valid ) {
                LLRegion get_region = pqp->ActiveRegion;
                bool b_rendered = false;

                if( !pqp->b_overlay ) {
                    if(chart->GetChartFamily() == CHART_FAMILY_VECTOR ) {

                        s57chart *Chs57 = dynamic_cast<s57chart*>( chart );
                        if(Chs57){
                            Chs57->RenderViewOnGLTextOnly(m_pcontext, vp);
                        }
                        else{
                            ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                            if(ChPI){
                                ChPI->RenderRegionViewOnGLTextOnly(m_pcontext, vp, rect_region);
                            }
                        }
                    }
                }
            }


            chart = m_pParentCanvas->m_pQuilt->GetNextSmallerScaleChart();
        }

/*
        //    Render any Overlay patches for s57 charts(cells)
        if( m_pParentCanvas->m_pQuilt->HasOverlays() ) {
            ChartBase *pch = m_pParentCanvas->m_pQuilt->GetFirstChart();
            while( pch ) {
                QuiltPatch *pqp = m_pParentCanvas->m_pQuilt->GetCurrentPatch();
                if( pqp->b_Valid && pqp->b_overlay && pch->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                    LLRegion get_region = pqp->ActiveRegion;

                    get_region.Intersect( region );
                    if( !get_region.Empty()  ) {
                        s57chart *Chs57 = dynamic_cast<s57chart*>( pch );
                        if( Chs57 )
                            Chs57->RenderOverlayRegionViewOnGL( *m_pcontext, vp, rect_region, get_region );
                    }
                }

                pch = m_pParentCanvas->m_pQuilt->GetNextChart();
            }
        }
*/
}

void ChartCanvas::RenderCharts(ocpnDC &dc, const OCPNRegion &rect_region)
{
    ViewPort &vp = m_pParentCanvas->VPoint;

    // Only for cm93 (not quilted), SetVPParms can change the valid region of the chart
    // we need to know this before rendering the chart so we can compute the background region
    // and nodta regions correctly.  I would prefer to just perform this here (or in SetViewPoint)
    // for all vector charts instead of in their render routine, but how to handle quilted cases?
    if(!vp.quilt() && m_pParentCanvas->m_singleChart->GetChartType() == CHART_TYPE_CM93COMP)
    {
//        static_cast<cm93compchart*>( m_pParentCanvas->m_singleChart )->SetVPParms( vp );
    }

    LLRegion chart_region;
    if( !vp.quilt() && (m_pParentCanvas->m_singleChart->GetChartType() == CHART_TYPE_PLUGIN) ){
        if(m_pParentCanvas->m_singleChart->GetChartFamily() == CHART_FAMILY_RASTER){
            // We do this the hard way, since PlugIn Raster charts do not understand LLRegion yet...
            double ll[8];
            ChartPlugInWrapper *cpw = dynamic_cast<ChartPlugInWrapper*> ( m_pParentCanvas->m_singleChart );
            if( !cpw) return;

            cpw->chartpix_to_latlong(0,                     0,              ll+0, ll+1);
            cpw->chartpix_to_latlong(0,                     cpw->GetSize_Y(), ll+2, ll+3);
            cpw->chartpix_to_latlong(cpw->GetSize_X(),      cpw->GetSize_Y(), ll+4, ll+5);
            cpw->chartpix_to_latlong(cpw->GetSize_X(),      0,              ll+6, ll+7);

            // for now don't allow raster charts to cross both 0 meridian and IDL (complicated to deal with)
            for(int i=1; i<6; i+=2)
                if(fabs(ll[i] - ll[i+2]) > 180) {
                    // we detect crossing idl here, make all longitudes positive
                    for(int i=1; i<8; i+=2)
                        if(ll[i] < 0)
                            ll[i] += 360;
                    break;
                }

            chart_region = LLRegion(4, ll);
        }
        else{
            Extent ext;
            m_pParentCanvas->m_singleChart->GetChartExtent(&ext);

            double ll[8] = {ext.SLAT, ext.WLON,
            ext.SLAT, ext.ELON,
            ext.NLAT, ext.ELON,
            ext.NLAT, ext.WLON};
            chart_region = LLRegion(4, ll);
        }
    }
    else
        chart_region = vp.quilt() ? m_pParentCanvas->m_pQuilt->GetFullQuiltRegion() : m_pParentCanvas->m_singleChart->GetValidRegion();

    bool world_view = false;
    for(OCPNRegionIterator upd ( rect_region ); upd.HaveRects(); upd.NextRect()) {
        QRect rect = upd.GetRect();
        LLRegion background_region = vp.GetLLRegion(rect);
        //    Remove the valid chart area to find the region NOT covered by the charts
        background_region.Subtract(chart_region);

        if(!background_region.Empty()) {
            ViewPort cvp = ClippedViewport(vp, background_region);
            RenderWorldChart(dc, cvp, rect, world_view);
        }
    }

    if(vp.quilt())
        RenderQuiltViewGL( vp, rect_region );
    else {
        LLRegion region = vp.GetLLRegion(rect_region);
        if( m_pParentCanvas->m_singleChart->GetChartFamily() == CHART_FAMILY_RASTER ){
            if(m_pParentCanvas->m_singleChart->GetChartType() == CHART_TYPE_MBTILES)
                m_pParentCanvas->m_singleChart->RenderRegionViewOnGL(m_pcontext, vp, rect_region, region );
            else
                RenderRasterChartRegionGL( m_pParentCanvas->m_singleChart, vp, region );
        }
        else if( m_pParentCanvas->m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR ) {
            chart_region.Intersect(region);
            RenderNoDTA(vp, chart_region);
            m_pParentCanvas->m_singleChart->RenderRegionViewOnGL(m_pcontext, vp, rect_region, region );
        }
    }

}

void ChartCanvas::RenderNoDTA(ViewPort &vp, const LLRegion &region, int transparency)
{
    QColor color = GetGlobalColor( ( "NODTA" ) );
    if( color.isValid() )
        glColor4ub( color.red(), color.green(), color.blue(), transparency );
    else
        glColor4ub( 163, 180, 183, transparency );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DrawRegion(vp, region);

    glDisable(GL_BLEND);
}

void ChartCanvas::RenderNoDTA(ViewPort &vp, ChartBase *chart)
{
//TODO stack from parent
#if 0
    QColor color = GetGlobalColor( _T ( "NODTA" ) );
    if( color.IsOk() )
        glColor3ub( color.Red(), color.Green(), color.Blue() );
    else
        glColor3ub( 163, 180, 183 );

    int index = -1;
    ChartTableEntry *pt;
    for(int i=0; i<pCurrentStack->nEntry; i++) {
#if 0
        ChartBase *c = OpenChartFromStack(pCurrentStack, i, HEADER_ONLY);
        if(c == chart) {
            index = pCurrentStack->GetDBIndex(i);
            pt = (ChartTableEntry *) &ChartData->GetChartTableEntry( index );
            break;
        }
#else
        int j = pCurrentStack->GetDBIndex(i);
        assert (j >= 0);
        pt = (ChartTableEntry *) &ChartData->GetChartTableEntry( j );
        if(pt->GetpsFullPath()->IsSameAs(chart->GetFullPath())){
            index = j;
            break;
        }
#endif
    }

    if(index == -1)
        return;

    if( ChartData->GetDBChartType( index ) != CHART_TYPE_CM93COMP ) {
        // Maybe it's a good idea to cache the glutesselator results to improve performance
        LLRegion region(pt->GetnPlyEntries(), pt->GetpPlyTable());
        DrawRegion(vp, region);
    } else {
        QRect rect(0, 0, vp.pixWidth(), vp.pixHeight());
        int x1 = rect.x, y1 = rect.y, x2 = x1 + rect.width, y2 = y1 + rect.height;
        glBegin( GL_QUADS );
        glVertex2i( x1, y1 );
        glVertex2i( x2, y1 );
        glVertex2i( x2, y2 );
        glVertex2i( x1, y2 );
        glEnd();
    }
#endif
}

/* render world chart, but only in this rectangle */
void ChartCanvas::RenderWorldChart(ocpnDC &dc, ViewPort &vp, QRect &rect, bool &world_view)
{
    // set gl color to water
    QColor water = m_pParentCanvas->pWorldBackgroundChart->water;
    glColor3ub(water.red(), water.green(), water.blue());

    // clear background
    if(!world_view) {
        if(vp.projectType() == PROJECTION_ORTHOGRAPHIC) {
            // for this projection, if zoomed out far enough that the earth does
            // not fill the viewport we need to first clear the screen black and
            // draw a blue circle representing the earth

            ViewPort tvp = vp;
            tvp.setLat(0);
            tvp.setLon(0);
            tvp.setRotation(0);
            zchxPointF p = tvp.GetDoublePixFromLL( 89.99, 0);
            float w = ((float)tvp.pixWidth())/2, h = ((float)tvp.pixHeight())/2;
            double world_r = h - p.y;
            const float pi_ovr100 = float(M_PI)/100;
            if(world_r*world_r < w*w + h*h) {
                glClear( GL_COLOR_BUFFER_BIT );

                glBegin(GL_TRIANGLE_FAN);
                float w = ((float)vp.pixWidth())/2, h = ((float)vp.pixHeight())/2;
                for(float theta = 0; theta < 2*M_PI+.01f; theta+=pi_ovr100)
                    glVertex2f(w + world_r*sinf(theta), h + world_r*cosf(theta));
                glEnd();

                world_view = true;
            }
        } else if(vp.projectType() == PROJECTION_EQUIRECTANGULAR) {
            // for this projection we will draw black outside of the earth (beyond the pole)
            glClear( GL_COLOR_BUFFER_BIT );

            zchxPointF p[4] = {
                vp.GetDoublePixFromLL( 90, vp.lon() - 170 ),
                vp.GetDoublePixFromLL( 90, vp.lon() + 170 ),
                vp.GetDoublePixFromLL(-90, vp.lon() + 170 ),
                vp.GetDoublePixFromLL(-90, vp.lon() - 170 )};

            glBegin(GL_QUADS);
            for(int i = 0; i<4; i++)
                glVertex2f(p[i].x, p[i].y);
            glEnd();

            world_view = true;
        }

        if(!world_view) {
            int x1 = rect.x(), y1 = rect.y(), x2 = x1 + rect.width(), y2 = y1 + rect.height();
            glBegin( GL_QUADS );
            glVertex2i( x1, y1 );
            glVertex2i( x2, y1 );
            glVertex2i( x2, y2 );
            glVertex2i( x1, y2 );
            glEnd();
        }
    }

    m_pParentCanvas->pWorldBackgroundChart->RenderViewOnDC( dc, vp );
}

/* these are the overlay objects which move with the charts and
   are not frequently updated (not ships etc..)

   many overlay objects are fixed to a geographical location or
   grounded as opposed to the floating overlay objects. */
void ChartCanvas::DrawGroundedOverlayObjects(ocpnDC &dc, ViewPort &vp)
{
    m_pParentCanvas->RenderAllChartOutlines( dc, vp );

    DrawStaticRoutesTracksAndWaypoints( vp );

    DisableClipRegion();
}


int n_render;
void ChartCanvas::Render()
{
    if( !m_bsetup ||
        ( m_pParentCanvas->VPoint.quilt() && !m_pParentCanvas->m_pQuilt->IsComposed() ) ||
        ( !m_pParentCanvas->VPoint.quilt() && !m_pParentCanvas->m_singleChart ) ) {
#ifdef __WXGTK__  // for some reason in gtk, a swap is needed here to get an initial screen update
            SwapBuffers();
#endif
            return;
        }

    m_last_render_time = QDateTime::currentDateTime().toTime_t();

    // we don't care about jobs that are now off screen
    // clear out and it will be repopulated during render
    if(g_GLOptions.m_bTextureCompression && !g_GLOptions.m_bTextureCompressionCaching)
        g_glTextureManager->ClearJobList();

    if(b_timeGL && g_bShowFPS){
        if(n_render % 10){
            glFinish();
//            g_glstopwatch.Start();
        }
    }
//    wxPaintDC( this );

    //  If we are in the middle of a fast pan, we don't want the FBO coordinates to be reset
    if(m_binPinch || m_binPan)
        return;

    ViewPort VPoint = m_pParentCanvas->VPoint;
    ocpnDC gldc( this );

    int gl_width = width(), gl_height = height();
//    GetClientSize( &gl_width, &gl_height );

#ifdef __WXOSX__
    gl_height = m_pParentCanvas->GetClientSize().y;
#endif

    OCPNRegion screen_region(QRect(0, 0, VPoint.pixWidth(), VPoint.pixHeight()));

    glViewport( 0, 0, (GLint) gl_width, (GLint) gl_height );
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    glOrtho( 0, (GLint) gl_width, (GLint) gl_height, 0, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }

    // set opengl settings that don't normally change
    // this should be able to go in SetupOpenGL, but it's
    // safer here incase a plugin mangles these
    if( g_GLOptions.m_GLLineSmoothing )
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    if( g_GLOptions.m_GLPolygonSmoothing )
        glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    //  Delete any textures known to the GPU that
    //  belong to charts which will not be used in this render
    //  This is done chart-by-chart...later we will scrub for unused textures
    //  that belong to charts which ARE used in this render, if we need to....

    g_glTextureManager->TextureCrunch(0.8);

    //  If we plan to post process the display, don't use accelerated panning
    double scale_factor = VPoint.refScale()/VPoint.chartScale();

    m_bfogit = m_benableFog && g_fog_overzoom && (scale_factor > g_overzoom_emphasis_base) && VPoint.quilt();
    bool scale_it  =  m_benableVScale && g_oz_vector_scale && (scale_factor > g_overzoom_emphasis_base) && VPoint.quilt();

    bool bpost_hilite = !m_pParentCanvas->m_pQuilt->GetHiliteRegion( ).Empty();
    bool useFBO = false;
    int sx = gl_width;
    int sy = gl_height;
    qDebug()<<"gl size:"<<sx<<sy<<" tex size<<"<<m_cache_tex_x<<m_cache_tex_y;

    // Try to use the framebuffer object's cache of the last frame
    // to accelerate drawing this frame (if overlapping)
    if(m_b_BuiltFBO && !m_bfogit && !scale_it && !bpost_hilite
       //&& VPoint.tilt == 0 // disabling fbo in tilt mode gives better quality but slower
        ) {
        //  Is this viewpoint the same as the previously painted one?
        bool b_newview = true;

        // If the view is the same we do no updates,
        // cached texture to the framebuffer
        qDebug()<<"vp roate:"<<m_cache_vp.rotation()<<VPoint.rotation();
        if(    m_cache_vp.viewScalePPM() == VPoint.viewScalePPM()
               && m_cache_vp.rotation() == VPoint.rotation()
               && m_cache_vp.lat() == VPoint.lat()
               && m_cache_vp.lon() == VPoint.lon()
               && m_cache_vp.isValid()
               && m_cache_vp.pixHeight() == VPoint.pixHeight()
               && m_cache_current_ch == m_pParentCanvas->m_singleChart ) {
            b_newview = false;
        }

        qDebug()<<"new flag = "<<b_newview;
        if( b_newview ) {

            bool busy = false;
            if(VPoint.quilt() && m_pParentCanvas->m_pQuilt->IsQuiltVector() &&
                ( m_cache_vp.viewScalePPM() != VPoint.viewScalePPM() || m_cache_vp.rotation() != VPoint.rotation()))
            {
                    OCPNPlatform::instance()->ShowBusySpinner();
                    busy = true;
            }

            // enable rendering to texture in framebuffer object
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );

            int dx, dy;
            bool accelerated_pan = false;
            if( g_GLOptions.m_bUseAcceleratedPanning && m_cache_vp.isValid()
                && ( VPoint.projectType() == PROJECTION_MERCATOR
                || VPoint.projectType() == PROJECTION_EQUIRECTANGULAR )
                && m_cache_vp.pixHeight() == VPoint.pixHeight() )
            {
                qDebug()<<"!!!!!!!!!!!!!!";
                zchxPointF c_old = VPoint.GetDoublePixFromLL( VPoint.lat(), VPoint.lon() );
                zchxPointF c_new = m_cache_vp.GetDoublePixFromLL( VPoint.lat(), VPoint.lon() );

//                printf("diff: %f %f\n", c_new.y - c_old.y, c_new.x - c_old.x);
                dy = qRound(c_new.y - c_old.y);
                dx = qRound(c_new.x - c_old.x);

                //   The math below using the previous frame's texture does not really
                //   work for sub-pixel pans.
                //   TODO is to rethink this.
                //   Meanwhile, require the accelerated pans to be whole pixel multiples only.
                //   This is not as bad as it sounds.  Keyboard and mouse pans are whole_pixel moves.
                //   However, autofollow at large scale is certainly not.

                double deltax = c_new.x - c_old.x;
                double deltay = c_new.y - c_old.y;

                bool b_whole_pixel = true;
                if( ( fabs( deltax - dx ) > 1e-2 ) || ( fabs( deltay - dy ) > 1e-2 ) )
                    b_whole_pixel = false;
                accelerated_pan = b_whole_pixel && abs(dx) < m_cache_tex_x && abs(dy) < m_cache_tex_y
                                  && sx == m_cache_tex_x && sy == m_cache_tex_y;

                qDebug()<<b_whole_pixel<<dx<<m_cache_tex_x<<dy<<m_cache_tex_y<<sx<<sy<<accelerated_pan;
            }

            // do we allow accelerated panning?  can we perform it here?
            if(accelerated_pan && !g_GLOptions.m_bUseCanvasPanning) {
                qDebug()<<"???????????????????";
                if((dx != 0) || (dy != 0)){   // Anything to do?
                    m_cache_page = !m_cache_page; /* page flip */

                    /* perform accelerated pan rendering to the new framebuffer */
                    ( s_glFramebufferTexture2D )
                        ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                        g_texture_rectangle_format, m_cache_tex[m_cache_page], 0 );

                    //calculate the new regions to render
                    // add an extra pixel avoid coorindate rounding issues
                    OCPNRegion update_region;

                    if( dy > 0 && dy < VPoint.pixHeight())
                        update_region.Union(QRect( 0, VPoint.pixHeight() - dy, VPoint.pixWidth(), dy ));
                    else if(dy < 0)
                        update_region.Union(QRect( 0, 0, VPoint.pixWidth(), -dy ));

                    if( dx > 0 && dx < VPoint.pixWidth() )
                        update_region.Union(QRect( VPoint.pixWidth() - dx, 0, dx, VPoint.pixHeight() ));
                    else if (dx < 0)
                        update_region.Union(QRect( 0, 0, -dx, VPoint.pixHeight() ));

                    RenderCharts(gldc, update_region);

                    // using the old framebuffer
                    glBindTexture( g_texture_rectangle_format, m_cache_tex[!m_cache_page] );
                    glEnable( g_texture_rectangle_format );
                    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

                    //    Render the reuseable portion of the cached texture
                    // Render the cached texture as quad to FBO(m_blit_tex) with offsets
                    int x1, x2, y1, y2;

                    int ow = VPoint.pixWidth() - abs( dx );
                    int oh = VPoint.pixHeight() - abs( dy );
                    if( dx > 0 )
                        x1 = dx,  x2 = 0;
                    else
                        x1 = 0,   x2 = -dx;

                    if( dy > 0 )
                        y1 = dy,  y2 = 0;
                    else
                        y1 = 0,   y2 = -dy;

                    // normalize to texture coordinates range from 0 to 1
                    float tx1 = x1, tx2 = x1 + ow, ty1 = sy - y1, ty2 = sy - (y1 + oh);
                    if( GL_TEXTURE_RECTANGLE_ARB != g_texture_rectangle_format )
                        tx1 /= sx, tx2 /= sx, ty1 /= sy, ty2 /= sy;

                    glBegin( GL_QUADS );
                    glTexCoord2f( tx1, ty1 );  glVertex2f( x2, y2 );
                    glTexCoord2f( tx2, ty1 );  glVertex2f( x2 + ow, y2 );
                    glTexCoord2f( tx2, ty2 );  glVertex2f( x2 + ow, y2 + oh );
                    glTexCoord2f( tx1, ty2 );  glVertex2f( x2, y2 + oh );
                    glEnd();

                    //   Done with cached texture "blit"
                    glDisable( g_texture_rectangle_format );
                }

            } else { // must redraw the entire screen
                qDebug()<<"***********************";
                    ( s_glFramebufferTexture2D )( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                                g_texture_rectangle_format,
                                                m_cache_tex[m_cache_page], 0 );

                    if(g_GLOptions.m_bUseCanvasPanning) {
                        qDebug()<<"$$$$$$$$$$$$$$$$$$";
                        bool b_reset = false;
                        if( (m_fbo_offsetx < 50) ||
                            ((m_cache_tex_x - (m_fbo_offsetx + sx)) < 50) ||
                            (m_fbo_offsety < 50) ||
                            ((m_cache_tex_y - (m_fbo_offsety + sy)) < 50))
                            b_reset = true;

                        if(m_cache_vp.viewScalePPM() != VPoint.viewScalePPM() )
                            b_reset = true;
                        if(!m_cache_vp.isValid())
                            b_reset = true;

                        if( b_reset ){
                            m_fbo_offsetx = (m_cache_tex_x - width())/2;
                            m_fbo_offsety = (m_cache_tex_y - height())/2;
                            m_fbo_swidth = sx;
                            m_fbo_sheight = sy;

                            m_canvasregion = OCPNRegion( m_fbo_offsetx, m_fbo_offsety, sx, sy );

//                             if(m_cache_vp.viewScalePPM() != VPoint.viewScalePPM() )
//                                 OCPNPlatform::ShowBusySpinner();

                            RenderCanvasBackingChart(gldc, m_canvasregion);
                        }



                        glPushMatrix();

                        glViewport( m_fbo_offsetx, m_fbo_offsety, (GLint) sx, (GLint) sy );
                        RenderCharts(gldc, screen_region);

                        glPopMatrix();

                        glViewport( 0, 0, (GLint) sx, (GLint) sy );
                    }
                    else{
                        qDebug()<<"^^^^^^^^^^^^^^^";
#if 0
                        // Should not really need to clear the screen, but consider this case:
                        // A previous installation of OCPN loaded some PlugIn charts, so setting their coverage in the database.
                        // In this installation, the PlugIns are not available, for whatever reason.
                        // As a result, some areas on screen will not be rendered by the PlugIn, and
                        // The backing chart is eclipsed by the unrendered chart coverage region.
                        //  Result: leftover garbage in the unrendered regions.

                        QColor color = GetGlobalColor( _T ( "NODTA" ) );
                        if( color.IsOk() )
                            glClearColor( color.Red() / 256., color.Green()/256., color.Blue()/256., 1.0 );
                        else
                            glClearColor(0, 0., 0, 1.0);
                        glClear(GL_COLOR_BUFFER_BIT);
#endif

                        m_fbo_offsetx = 0;
                        m_fbo_offsety = 0;
                        m_fbo_swidth = sx;
                        m_fbo_sheight = sy;
                        QRect rect(m_fbo_offsetx, m_fbo_offsety, (GLint) sx, (GLint) sy);
                        qDebug()<<"render new chart";
                        RenderCharts(gldc, screen_region);
                    }

                }
            // Disable Render to FBO
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );

            if(busy)
                OCPNPlatform::instance()->HideBusySpinner();

        } // newview

        useFBO = true;
    }

    if(VPoint.tilt()) {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(2*180/PI*atan2((double)gl_height, (double)gl_width), (GLfloat) gl_width/(GLfloat) gl_height, 1, gl_width);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glScalef(1, -1, 1);
        glTranslatef(-gl_width/2, -gl_height/2, -gl_width/2);
        glRotated(VPoint.tilt()*180/PI, 1, 0, 0);

        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
    }

    if(useFBO) {
        // Render the cached texture as quad to screen
        glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
        glEnable( g_texture_rectangle_format );

        float tx, ty, tx0, ty0, divx, divy;

        //  Normalize, or not?
        if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format ){
            divx = divy = 1.0f;
         }
        else{
            divx = m_cache_tex_x;
            divy = m_cache_tex_y;
        }

        tx0 = m_fbo_offsetx/divx;
        ty0 = m_fbo_offsety/divy;
        tx =  (m_fbo_offsetx + m_fbo_swidth)/divx;
        ty =  (m_fbo_offsety + m_fbo_sheight)/divy;

        glBegin( GL_QUADS );
        glTexCoord2f( tx0, ty );  glVertex2f( 0,  0 );
        glTexCoord2f( tx,  ty );  glVertex2f( sx, 0 );
        glTexCoord2f( tx,  ty0 ); glVertex2f( sx, sy );
        glTexCoord2f( tx0, ty0 ); glVertex2f( 0,  sy );
        glEnd();

        glDisable( g_texture_rectangle_format );

        m_cache_vp = VPoint;
        m_cache_current_ch = m_pParentCanvas->m_singleChart;

        if(VPoint.quilt())
            m_pParentCanvas->m_pQuilt->SetRenderedVP( VPoint );

    } else          // useFBO
        RenderCharts(gldc, screen_region);

       //  Render the decluttered Text overlay for quilted vector charts, except for CM93 Composite
    if( VPoint.quilt() ) {
        if(m_pParentCanvas->m_pQuilt->IsQuiltVector() && ps52plib && ps52plib->GetShowS57Text()){

            ChartBase *chart = m_pParentCanvas->m_pQuilt->GetRefChart();
            if(chart && (chart->GetChartType() != CHART_TYPE_CM93COMP)){
                //        Clear the text Global declutter list
                if(chart){
                    ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                    if(ChPI)
                        ChPI->ClearPLIBTextList();
                    else
                        ps52plib->ClearTextList();
                }

                // Grow the ViewPort a bit laterally, to minimize "jumping" of text elements at left side of screen
                ViewPort vpx = VPoint;
                vpx.BuildExpandedVP(VPoint.pixWidth() * 12 / 10, VPoint.pixHeight());

                OCPNRegion screen_region(QRect(0, 0, VPoint.pixWidth(), VPoint.pixHeight()));
                RenderQuiltViewGLText( vpx, screen_region );
            }
        }
    }



    // Render MBTiles as overlay
    std::vector<int> stackIndexArray = m_pParentCanvas->m_pQuilt->GetExtendedStackIndexArray();
    unsigned int im = stackIndexArray.size();
    // XXX should
    // assert(!VPoint.quilt() && im == 0)
    if( VPoint.quilt() && im > 0 ) {
        bool regionVPBuilt = false;
        OCPNRegion screen_region;
        LLRegion screenLLRegion;
        LLBBox screenBox;
        ViewPort vp;

        std::vector<int> tiles_to_show;
        for( unsigned int is = 0; is < im; is++ ) {
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( stackIndexArray[is] );
            if(std::find(g_quilt_noshow_index_array.begin(), g_quilt_noshow_index_array.end(), stackIndexArray[is]) != g_quilt_noshow_index_array.end()) {
                continue;
            }
            if(cte.GetChartType() == CHART_TYPE_MBTILES){
                tiles_to_show.push_back(stackIndexArray[is]);
                if(!regionVPBuilt){
                    screen_region = OCPNRegion(QRect(0, 0, VPoint.pixWidth(), VPoint.pixHeight()));
                    screenLLRegion = VPoint.GetLLRegion( screen_region );
                    screenBox = screenLLRegion.GetBox();

                    vp = VPoint;
                    zchxPoint p;
                    p.x = VPoint.pixWidth() / 2;  p.y = VPoint.pixHeight() / 2;
                    double lat = 0.0, lon = 0.0;
                    VPoint.GetLLFromPix( p, &lat, &lon);
                    vp.setLat(lat);
                    vp.setLon(lon);

                    regionVPBuilt = true;
                }

            }
        }
        // We need to show the tilesets in reverse order to have the largest scale on top
        for(std::vector<int>::reverse_iterator rit = tiles_to_show.rbegin();
                            rit != tiles_to_show.rend(); ++rit) {
            ChartBase *chart = ChartData->OpenChartFromDBAndLock(*rit, FULL_INIT);
            ChartMBTiles *pcmbt = dynamic_cast<ChartMBTiles*>( chart );
            if(pcmbt){
                pcmbt->RenderRegionViewOnGL(m_pcontext, vp, screen_region, screenLLRegion);
            }
        }
    }




    // Render static overlay objects , 这里就是绿色的外框
    for(OCPNRegionIterator upd ( screen_region ); upd.HaveRects(); upd.NextRect()) {
         LLRegion region = VPoint.GetLLRegion(upd.GetRect());
         ViewPort cvp = ClippedViewport(VPoint, region);
         DrawGroundedOverlayObjects(gldc, cvp);
    }


    // If multi-canvas, indicate which canvas has keyboard focus
    // by drawing a simple blue bar at the top.
    if(g_canvasConfig != 0){             // multi-canvas?
        if( m_pParentCanvas->hasFocus()){
            g_focusCanvas = m_pParentCanvas;

            QColor colour = GetGlobalColor("BLUE4");
            glColor3ub(colour.red(), colour.green(), colour.blue() );

            float rect_pix = m_pParentCanvas->m_focus_indicator_pix;

            int xw = m_pParentCanvas->width();
            glBegin(GL_QUADS);
            glVertex2i(0, 0);
            glVertex2i(xw, 0);
            glVertex2i(xw, rect_pix);
            glVertex2i(0, rect_pix);
            glEnd();
        }
    }


    DrawDynamicRoutesTracksAndWaypoints( VPoint );

    // Now draw all the objects which normally move around and are not
    // cached from the previous frame
    DrawFloatingOverlayObjects( gldc );

    // from this point on don't use perspective
    if(VPoint.tilt()) {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        glOrtho( 0, (GLint) gl_width, (GLint) gl_height, 0, -1, 1 );
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    DrawEmboss(m_pParentCanvas->EmbossDepthScale() );
    DrawEmboss(m_pParentCanvas->EmbossOverzoomIndicator( gldc ) );

    //  On some platforms, the opengl context window is always on top of any standard DC windows,
    //  so we need to draw the Chart Info Window and the Thumbnail as overlayed bmps.

#ifdef __WXOSX__
        if(m_pParentCanvas->m_pCIWin && m_pParentCanvas->m_pCIWin->IsShown()) {
        int x, y, width, height;
        m_pParentCanvas->m_pCIWin->GetClientSize( &width, &height );
        m_pParentCanvas->m_pCIWin->GetPosition( &x, &y );
        wxBitmap bmp(width, height, -1);
        wxMemoryDC dc(bmp);
        if(bmp.IsOk()){
            dc.SetBackground( wxBrush(GetGlobalColor( _T ( "UIBCK" ) ) ));
            dc.Clear();

            dc.SetTextBackground( GetGlobalColor( _T ( "UIBCK" ) ) );
            dc.SetTextForeground( GetGlobalColor( _T ( "UITX1" ) ) );

            int yt = 0;
            int xt = 0;
            QString s = m_pParentCanvas->m_pCIWin->GetString();
            int h = m_pParentCanvas->m_pCIWin->GetCharHeight();

            QStringTokenizer tkz( s, _T("\n") );
            QString token;

            while(tkz.HasMoreTokens()) {
                token = tkz.GetNextToken();
                dc.DrawText(token, xt, yt);
                yt += h;
            }
            dc.SelectObject(wxNullBitmap);

            gldc.DrawBitmap( bmp, x, y, false);
        }
    }

    if( pthumbwin && pthumbwin->IsShown()) {
        int thumbx, thumby;
        pthumbwin->GetPosition( &thumbx, &thumby );
        if( pthumbwin->GetBitmap().IsOk())
            gldc.DrawBitmap( pthumbwin->GetBitmap(), thumbx, thumby, false);
    }

    if(g_MainToolbar && g_MainToolbar->m_pRecoverwin ){
        int recoverx, recovery;
        g_MainToolbar->m_pRecoverwin->GetPosition( &recoverx, &recovery );
        if( g_MainToolbar->m_pRecoverwin->GetBitmap().IsOk())
            gldc.DrawBitmap( g_MainToolbar->m_pRecoverwin->GetBitmap(), recoverx, recovery, true);
    }


#endif

    if (m_pParentCanvas->m_Compass)
        m_pParentCanvas->m_Compass->Paint(gldc);

    RenderGLAlertMessage();

    //quiting?
    if( g_bquiting )
        DrawQuiting();
    if( g_bcompression_wait)
        DrawCloseMessage("Waiting for raster chart compression thread exit.");



     //  Some older MSW OpenGL drivers are generally very unstable.
     //  This helps...

    if(g_b_needFinish)
        glFinish();

//    swapBuffers();
    if(b_timeGL && g_bShowFPS){
        if(n_render % 10){
            glFinish();

            double filter = .05;

        // Simple low pass filter
//            g_gl_ms_per_frame = g_gl_ms_per_frame * (1. - filter) + ((double)(g_glstopwatch.Time()) * filter);
//            if(g_gl_ms_per_frame > 0)
            //                printf(" OpenGL frame time: %3.0f  %3.0f\n", g_gl_ms_per_frame, 1000./ g_gl_ms_per_frame);
        }
    }

    g_glTextureManager->TextureCrunch(0.8);
    g_glTextureManager->FactoryCrunch(0.6);

    m_pParentCanvas->PaintCleanup();
    //OCPNPlatform::HideBusySpinner();

    n_render++;
}



void ChartCanvas::RenderCanvasBackingChart( ocpnDC &dc, OCPNRegion valid_region)
{

    glPushMatrix();

    glLoadIdentity();

    glOrtho( 0, m_cache_tex_x, m_cache_tex_y, 0, -1, 1 );
    glViewport( 0, 0, (GLint) m_cache_tex_x, (GLint) m_cache_tex_y );

    // strategies:

    // 1:  Simple clear to color

    OCPNRegion texr( 0, 0, m_cache_tex_x,  m_cache_tex_y );
    texr.Subtract(valid_region);

    glViewport( 0, 0, (GLint) m_cache_tex_x, (GLint) m_cache_tex_y );

    glColor3ub(0, 250, 250);

    OCPNRegionIterator upd ( texr );
    while ( upd.HaveRects() )
    {
        QRect rect = upd.GetRect();

        glBegin( GL_QUADS );
        glVertex2f( rect.x(),                     rect.y() );
        glVertex2f( rect.x() + rect.width(),        rect.y() );
        glVertex2f( rect.x() + rect.width(),        rect.y() + rect.height() );
        glVertex2f( rect.x(),                     rect.y() + rect.height() );
        glEnd();

        upd.NextRect();
    }

    // 2:  Render World Background chart

    QRect rtex( 0, 0, m_cache_tex_x,  m_cache_tex_y );
    ViewPort cvp = m_pParentCanvas->GetVP().BuildExpandedVP( m_cache_tex_x,  m_cache_tex_y );

    bool world_view = false;
    RenderWorldChart(dc, cvp, rtex, world_view);

/*
    dc.SetPen(wxPen(wxColour(254,254,0), 3));
    dc.DrawLine( 0, 0, m_cache_tex_x, m_cache_tex_y);

    dc.DrawLine( 0, 0, 0, m_cache_tex_y);
    dc.DrawLine( 0, m_cache_tex_y, m_cache_tex_x, m_cache_tex_y);
    dc.DrawLine( m_cache_tex_x, m_cache_tex_y, m_cache_tex_x, 0);
    dc.DrawLine( m_cache_tex_x, 0, 0, 0);
*/

    // 3:  Use largest scale chart in the current quilt candidate list (which is identical to chart bar)
    //          which covers the entire canvas

    glPopMatrix();

}


void ChartCanvas::FastPan(int dx, int dy)
{
    int sx = width();
    int sy = height();

    //   ViewPort VPoint = m_pParentCanvas->VPoint;
    //   ViewPort svp = VPoint;
    //   svp.pixWidth() = svp.rv_rect.width;
    //   svp.pixHeight() = svp.rv_rect.height;

    //   OCPNRegion chart_get_region( 0, 0, m_pParentCanvas->VPoint.rv_rect.width, m_pParentCanvas->VPoint.rv_rect.height );

    //    ocpnDC gldc( *this );

    int w = width(), h = height();
//    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );

    glLoadIdentity();
    glOrtho( 0, (GLint) w, (GLint) h, 0, -1, 1 );

    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }

    float vx0 = 0;
    float vy0 = 0;
    float vy = sy;
    float vx = sx;

    glBindTexture( g_texture_rectangle_format, 0);

    /*   glColor3ub(0, 0, 120);
     *    glBegin( GL_QUADS );
     *    glVertex2f( 0,  0 );
     *    glVertex2f( w, 0 );
     *    glVertex2f( w, h );
     *    glVertex2f( 0,  h );
     *    glEnd();
     */



    float tx, ty, tx0, ty0;
    //if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format )
    //  tx = sx, ty = sy;
    //else
    tx = 1, ty = 1;

    tx0 = ty0 = 0.;

    m_fbo_offsety += dy;
    m_fbo_offsetx += dx;

    tx0 = m_fbo_offsetx;
    ty0 = m_fbo_offsety;
    tx =  m_fbo_offsetx + sx;
    ty =  m_fbo_offsety + sy;


    if((m_fbo_offsety ) < 0){
        ty0 = 0;
        ty  =  m_fbo_offsety + sy;

        vy0 = 0;
        vy = sy + m_fbo_offsety;

        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( 0,  vy );
        glVertex2f( w, vy );
        glVertex2f( w, h );
        glVertex2f( 0,  h );
        glEnd();

    }
    else if((m_fbo_offsety + sy) > m_cache_tex_y){
        ty0 = m_fbo_offsety;
        ty  =  m_cache_tex_y;

        vy = sy;
        vy0 = (m_fbo_offsety + sy - m_cache_tex_y);

        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( 0,  0 );
        glVertex2f( w, 0 );
        glVertex2f( w, vy0 );
        glVertex2f( 0, vy0 );
        glEnd();
    }



    if((m_fbo_offsetx) < 0){
        tx0 = 0;
        tx  =  m_fbo_offsetx + sx;

        vx0 = -m_fbo_offsetx;
        vx = sx;

        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( 0,  0 );
        glVertex2f( vx0, 0 );
        glVertex2f( vx0, h );
        glVertex2f( 0,  h );
        glEnd();
    }
    else if((m_fbo_offsetx + sx) > m_cache_tex_x){
        tx0 = m_fbo_offsetx;
        tx  = m_cache_tex_x;

        vx0 = 0;
        vx = m_cache_tex_x - m_fbo_offsetx;

        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( vx,  0 );
        glVertex2f( w, 0 );
        glVertex2f( w, h );
        glVertex2f( vx,  h );
        glEnd();
    }


    // Render the cached texture as quad to screen
    glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
    glEnable( g_texture_rectangle_format );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    glBegin( GL_QUADS );
    glTexCoord2f( tx0/m_cache_tex_x, ty/m_cache_tex_y );  glVertex2f( vx0,  vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty/m_cache_tex_y );  glVertex2f( vx,   vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty0/m_cache_tex_y ); glVertex2f( vx,   vy );
    glTexCoord2f( tx0/m_cache_tex_x, ty0/m_cache_tex_y ); glVertex2f( vx0,  vy );
    glEnd();

    glDisable( g_texture_rectangle_format );
    glBindTexture( g_texture_rectangle_format, 0);

    m_canvasregion.Union(tx0, ty0, sx, sy);
}

QColor ChartCanvas::GetBackGroundColor()const
{
    QPalette pal = this->palette();
    QBrush brush = pal.background();
    return brush.color();
}

void ChartCanvas::RenderGLAlertMessage()
{
    if(!m_pParentCanvas->GetAlertString().isEmpty())
    {
        QString msg = m_pParentCanvas->GetAlertString();

        QFont pfont("Micorosoft Yahei", 10, QFont::Weight::Normal);
        TexFont texfont;
        texfont.Build(pfont);

        int w, h;
        texfont.GetTextExtent( msg, &w, &h);
        h += 2;
        w += 4;
        int yp = m_pParentCanvas->VPoint.pixHeight() - 20 - h;

        QRect sbr = m_pParentCanvas->GetScaleBarRect();
        int xp = sbr.x()+sbr.width() + 5;

        glColor3ub( 243, 229, 47 );

        glBegin(GL_QUADS);
        glVertex2i(xp, yp);
        glVertex2i(xp+w, yp);
        glVertex2i(xp+w, yp+h);
        glVertex2i(xp, yp+h);
        glEnd();

        glEnable(GL_BLEND);
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glColor3ub( 0, 0, 0 );
        glEnable(GL_TEXTURE_2D);
        texfont.RenderString( msg, xp, yp);
        glDisable(GL_TEXTURE_2D);

    }
}



void ChartCanvas::FastZoom(float factor)
{
    int sx = width();
    int sy = height();

    if(factor > 1.0f){
        int fbo_ctr_x = m_fbo_offsetx + (m_fbo_swidth / 2);
        int fbo_ctr_y = m_fbo_offsety + (m_fbo_sheight / 2);

        m_fbo_swidth  = m_fbo_swidth / factor;
        m_fbo_sheight = m_fbo_sheight / factor;

        m_fbo_offsetx = fbo_ctr_x - (m_fbo_swidth / 2.);
        m_fbo_offsety = fbo_ctr_y - (m_fbo_sheight / 2.);

    }

    if(factor < 1.0f){
        int fbo_ctr_x = m_fbo_offsetx + (m_fbo_swidth / 2);
        int fbo_ctr_y = m_fbo_offsety + (m_fbo_sheight / 2);

        m_fbo_swidth  = m_fbo_swidth / factor;
        m_fbo_sheight = m_fbo_sheight / factor;

        m_fbo_offsetx = fbo_ctr_x - (m_fbo_swidth / 2.);
        m_fbo_offsety = fbo_ctr_y - (m_fbo_sheight / 2.);

    }


    float tx, ty, tx0, ty0;
    //if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format )
    //  tx = sx, ty = sy;
    //else
    tx = 1, ty = 1;

    tx0 = ty0 = 0.;

    tx0 = m_fbo_offsetx;
    ty0 = m_fbo_offsety;
    tx =  m_fbo_offsetx + m_fbo_swidth;
    ty =  m_fbo_offsety + m_fbo_sheight;


    int w = width(), h = height();
//    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );

    glLoadIdentity();
    glOrtho( 0, (GLint) w, (GLint) h, 0, -1, 1 );

    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }


    float vx0 = 0;
    float vy0 = 0;
    float vy = sy;
    float vx = sx;

    glBindTexture( g_texture_rectangle_format, 0);

    // Render the cached texture as quad to screen
    glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
    glEnable( g_texture_rectangle_format );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );


    glBegin( GL_QUADS );
    glTexCoord2f( tx0/m_cache_tex_x, ty/m_cache_tex_y );  glVertex2f( vx0,  vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty/m_cache_tex_y );  glVertex2f( vx,   vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty0/m_cache_tex_y ); glVertex2f( vx,   vy );
    glTexCoord2f( tx0/m_cache_tex_x, ty0/m_cache_tex_y ); glVertex2f( vx0,  vy );
    glEnd();

    glDisable( g_texture_rectangle_format );
    glBindTexture( g_texture_rectangle_format, 0);

    //  When zooming out, if we go too far, then the frame buffer is repeated on-screen due
    //  to address wrapping in the frame buffer.
    //  Detect this case, and render some simple solid covering quads to avoid a confusing display.
    if( (m_fbo_sheight > m_cache_tex_y) || (m_fbo_swidth > m_cache_tex_x) ){
        QColor color = GetGlobalColor("GREY1");
        glColor3ub(color.red(), color.green(), color.blue());

        if( m_fbo_sheight > m_cache_tex_y ){
            float h1 = sy * (1.0 - m_cache_tex_y/m_fbo_sheight) / 2.;

            glBegin( GL_QUADS );
            glVertex2f( 0,  0 );
            glVertex2f( w,  0 );
            glVertex2f( w, h1 );
            glVertex2f( 0, h1 );
            glEnd();

            glBegin( GL_QUADS );
            glVertex2f( 0,  sy );
            glVertex2f( w,  sy );
            glVertex2f( w, sy - h1 );
            glVertex2f( 0, sy - h1 );
            glEnd();

        }

         // horizontal axis
         if( m_fbo_swidth > m_cache_tex_x ){
             float w1 = sx * (1.0 - m_cache_tex_x/m_fbo_swidth) / 2.;

             glBegin( GL_QUADS );
             glVertex2f( 0,  0 );
             glVertex2f( w1,  0 );
             glVertex2f( w1, sy );
             glVertex2f( 0, sy );
             glEnd();

             glBegin( GL_QUADS );
             glVertex2f( sx,  0 );
             glVertex2f( sx - w1,  0 );
             glVertex2f( sx - w1, sy );
             glVertex2f( sx, sy );
             glEnd();

         }
    }
}


