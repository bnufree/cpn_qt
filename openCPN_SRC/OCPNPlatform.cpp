/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Platform specific support utilities
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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
#include "dychart.h"
#include "OCPNPlatform.h"
//#include "chart1.h"
//#include "cutil.h"
//#include "styles.h"
//#include "navutil.h"
#include "FontMgr.h"
//#include "s52s57.h"
//#include "Select.h"
#include "_def.h"
#include <QDesktopWidget>
#include "GL/gl_private.h"
#include "zchxconfig.h"
#include <QDebug>

//#include "glChartCanvas.h"


#ifdef __MSVC__
#include <new.h>
#endif



#include <windows.h>
#include <winioctl.h>
#include <initguid.h>
#include "setupapi.h"                   // presently stored in opencpn/src

#include <cstdlib>
#include "zchxmapmainwindow.h"

void appendOSDirSlash( QString* pString );


//#ifndef __WXMSW__
//struct sigaction          sa_all;
//struct sigaction          sa_all_old;
//extern sigjmp_buf env;                    // the context saved by sigsetjmp();
//#endif


extern bool                      g_bFirstRun;
extern bool                      g_bUpgradeInProcess;

extern int                       quitflag;


extern bool                      g_bshowToolbar;
extern bool                      g_bBasicMenus;

extern bool                      g_bshowToolbar;
extern bool                      g_bBasicMenus;

extern bool                      g_bShowOutlines;
extern bool                      g_bShowDepthUnits;
extern bool                      g_bDisplayGrid;  // Flag indicating weather the lat/lon grid should be displayed
extern bool                      g_bShowChartBar;
extern bool                      g_bShowActiveRouteHighway;
extern int                       g_nNMEADebug;
extern int                       g_nAWDefault;
extern int                       g_nAWMax;
extern bool                      g_bPlayShipsBells;
extern bool                      g_bFullscreenToolbar;
extern bool                      g_bShowLayers;
extern bool                      g_bPermanentMOBIcon;
extern bool                      g_bTempShowMenuBar;
extern float                     g_toolbar_scalefactor;

extern int                       g_iSDMMFormat;
extern int                       g_iDistanceFormat;
extern int                       g_iSpeedFormat;

extern int                       g_iNavAidRadarRingsNumberVisible;
extern float                     g_fNavAidRadarRingsStep;
extern int                       g_pNavAidRadarRingsStepUnits;
extern int                       g_iWaypointRangeRingsNumber;
extern float                     g_fWaypointRangeRingsStep;
extern int                       g_iWaypointRangeRingsStepUnits;
extern QColor                       g_colourWaypointRangeRingsColour;
extern bool                      g_bWayPointPreventDragging;
extern bool                      g_bConfirmObjectDelete;

// AIS Global configuration
extern bool                      g_bCPAMax;
extern double                    g_CPAMax_NM;
extern bool                      g_bCPAWarn;
extern double                    g_CPAWarn_NM;
extern bool                      g_bTCPA_Max;
extern double                    g_TCPA_Max;
extern bool                      g_bMarkLost;
extern double                    g_MarkLost_Mins;
extern bool                      g_bRemoveLost;
extern double                    g_RemoveLost_Mins;
extern bool                      g_bShowCOG;
extern double                    g_ShowCOG_Mins;
extern bool                      g_bAISShowTracks;
extern double                    g_AISShowTracks_Mins;
extern bool                      g_bHideMoored;
extern double                    g_ShowMoored_Kts;
extern QString                  g_sAIS_Alert_Sound_File;
extern bool                      g_bAIS_CPA_Alert_Suppress_Moored;
extern bool                      g_bAIS_ACK_Timeout;
extern double                    g_AckTimeout_Mins;
extern bool                      g_bShowAreaNotices;
extern bool                      g_bDrawAISSize;
extern bool                      g_bShowAISName;
extern int                       g_Show_Target_Name_Scale;
extern bool                      g_bWplIsAprsPosition;

extern int                       gps_watchdog_timeout_ticks;
extern int                       sat_watchdog_timeout_ticks;

extern int                       gGPS_Watchdog;
extern bool                      bGPSValid;

extern int                       gHDx_Watchdog;
extern int                       gHDT_Watchdog;
extern int                       gVAR_Watchdog;
extern bool                      g_bHDT_Rx;
extern bool                      g_bVAR_Rx;

extern int                       gSAT_Watchdog;
extern int                       g_SatsInView;
extern bool                      g_bSatValid;

extern bool                      g_bDebugCM93;
extern bool                      g_bDebugS57;

extern bool                      g_bfilter_cogsog;
extern int                       g_COGFilterSec;
extern int                       g_SOGFilterSec;

extern int                       g_ChartUpdatePeriod;
extern int                       g_SkewCompUpdatePeriod;

extern int                       g_lastClientRectx;
extern int                       g_lastClientRecty;
extern int                       g_lastClientRectw;
extern int                       g_lastClientRecth;
extern double                    g_display_size_mm;

extern float                     g_selection_radius_mm;
extern float                     g_selection_radius_touch_mm;

extern bool                     g_bTrackDaily;
extern double                   g_PlanSpeed;
extern bool                     g_bFullScreenQuilt;
extern bool                     g_bQuiltEnable;
extern bool                     g_bskew_comp;
extern bool                     g_btouch;
extern bool                     g_bresponsive;
extern bool                     g_bShowStatusBar;
extern int                      g_cm93_zoom_factor;
extern int                      g_GUIScaleFactor;
extern bool                     g_fog_overzoom;
extern double                   g_overzoom_emphasis_base;
extern bool                     g_oz_vector_scale;
extern int                      g_nTrackPrecision;
extern QString                 g_toolbarConfig;
extern bool                     g_bPreserveScaleOnX;

//extern Select                    *pSelect;
//extern Select                    *pSelectTC;
//extern Select                    *pSelectAIS;

#ifdef ocpnUSE_GL
extern zchxGLOptions            g_GLOptions;
#endif
extern int                      g_default_font_size;
extern int                       options_lastPage;


OCPNPlatform* OCPNPlatform::minstance = 0;
OCPNPlatform::MGarbage OCPNPlatform::Garbage;


//  OCPN Platform implementation

OCPNPlatform* OCPNPlatform::instance()
{
    if(!minstance) minstance = new OCPNPlatform();
    return minstance;
}

OCPNPlatform::OCPNPlatform() : mOldShape(Qt::ArrowCursor)
{
    m_pt_per_pixel = 0;                 // cached value
    m_bdisableWindowsDisplayEnum = false;
    m_displaySize = QSize(0,0);
    m_displaySizeMM = QSize(0,0);
    m_displaySizeMMOverride = 0;
    initSystemInfo();
}

OCPNPlatform::~OCPNPlatform()
{
}






//  Called from MyApp() immediately before creation of MyFrame()
//  Config is known to be loaded and stable
//  Log is available
void OCPNPlatform::Initialize_2( void )
{
#ifdef __OCPN__ANDROID__
    ZCHX_LOGMSG(androidGetDeviceInfo());
#endif    
    
    //  Set a global toolbar scale factor
    g_toolbar_scalefactor = GetToolbarScaleFactor( g_GUIScaleFactor );
    
}

void OCPNPlatform::Initialize_3( void )
{
    
    bool bcapable = IsGLCapable();

#ifdef ocpnARM         // Boot arm* platforms (meaning rPI) without OpenGL on first run
    bcapable = false;
#endif    
    
    // Try to automatically switch to guaranteed usable GL mode on an OCPN upgrade or fresh install

    if( (g_bFirstRun || g_bUpgradeInProcess) && bcapable){
        // Set up visually nice options
        g_GLOptions.m_bUseAcceleratedPanning = true;
        g_GLOptions.m_bTextureCompression = true;
        g_GLOptions.m_bTextureCompressionCaching = true;

        g_GLOptions.m_iTextureDimension = 512;
        g_GLOptions.m_iTextureMemorySize = 64;
    
        g_GLOptions.m_GLPolygonSmoothing = true;
        g_GLOptions.m_GLLineSmoothing = true;

    }
}

//  Called from MyApp() just before end of MyApp::OnInit()
void OCPNPlatform::Initialize_4( void )
{
#ifdef __OCPN__ANDROID__
    if(pSelect) pSelect->SetSelectPixelRadius(qMax( 25, 6.0 * getAndroidDPmm()) );
    if(pSelectTC) pSelectTC->SetSelectPixelRadius( qMax( 25, 6.0 * getAndroidDPmm()) );
    if(pSelectAIS) pSelectAIS->SetSelectPixelRadius( qMax( 25, 6.0 * getAndroidDPmm()) );
#endif

#ifdef __WXMAC__
    // A bit of a hack for Mojave MacOS 10.14.
    // Force the user to actively select "Display" tab to ensure initial rendering of
    // canvas layout select button.
    options_lastPage = 1;
#endif
    
}

void OCPNPlatform::OnExit_1( void ){
}
    
void OCPNPlatform::OnExit_2( void ){
    
#ifdef OCPN_USE_CRASHRPT
#ifndef _DEBUG
        // Uninstall Windows crash reporting
//    crUninstall();
#endif
#endif
    
}


bool OCPNPlatform::BuildGLCaps( void *pbuf )
{
#if 0
    // Investigate OpenGL capabilities
    QOpenGLWindow *tcanvas = new QOpenGLWindow(new QOpenGLContext);
    tcanvas->show();
    QThread::yieldCurrentThread();
    
    OCPN_GLCaps *pcaps = (OCPN_GLCaps *)pbuf;
    
    char *str = (char *) glGetString( GL_RENDERER );
    if (str == NULL){
        delete tcanvas;
        return false;
    }
    
    char render_string[80];
    strncpy( render_string, str, 79 );
    pcaps->Renderer = QString::fromUtf8(render_string);

    
    if( QueryExtension( "GL_ARB_texture_non_power_of_two" ) )
        pcaps->TextureRectangleFormat = GL_TEXTURE_2D;
    else if( QueryExtension( "GL_OES_texture_npot" ) )
        pcaps->TextureRectangleFormat = GL_TEXTURE_2D;
    else if( QueryExtension( "GL_ARB_texture_rectangle" ) )
        pcaps->TextureRectangleFormat = GL_TEXTURE_RECTANGLE_ARB;


    GetglEntryPoints( pcaps );
    
    if( pcaps->Renderer.toUpper().contains("INTEL") ){
        if( pcaps->Renderer.toUpper().contains("965") ){
            pcaps->bOldIntel = true;
        }
    }
 
    // Can we use VBO?
    pcaps->bCanDoVBO = true;
    if( !pcaps->m_glBindBuffer || !pcaps->m_glBufferData || !pcaps->m_glGenBuffers || !pcaps->m_glDeleteBuffers )
        pcaps->bCanDoVBO = false;

    if(pcaps->bOldIntel)
        pcaps->bCanDoVBO = false;
    
    // Can we use FBO?
    pcaps->bCanDoFBO = true;
    //  We need NPOT to support FBO rendering
    if(!pcaps->TextureRectangleFormat)
    {
        pcaps->bCanDoFBO = false;
    }

    //      We require certain extensions to support FBO rendering
    if(!QueryExtension( "GL_EXT_framebuffer_object" ))
    {
        pcaps->bCanDoFBO = false;
    }

    if( !pcaps->m_glGenFramebuffers  || !pcaps->m_glGenRenderbuffers        || !pcaps->m_glFramebufferTexture2D ||
            !pcaps->m_glBindFramebuffer  || !pcaps->m_glFramebufferRenderbuffer || !pcaps->m_glRenderbufferStorage  ||
            !pcaps->m_glBindRenderbuffer || !pcaps->m_glCheckFramebufferStatus  || !pcaps->m_glDeleteFramebuffers   ||
            !pcaps->m_glDeleteRenderbuffers )
        pcaps->bCanDoFBO = false;

    if( pcaps->Renderer.toUpper().contains("INTEL") ) {
        if(pcaps->Renderer.toUpper().contains("MOBILE") ){
            pcaps->bCanDoFBO = false;
        }
    }


    delete tcanvas;
#endif
    
    return true;
}

bool OCPNPlatform::IsGLCapable()
{
#if 0
    OCPN_GLCaps *pcaps = new OCPN_GLCaps;
    
    BuildGLCaps(pcaps);

    // and so we decide....
    
    // We insist on FBO support, since otherwise DC mode is always faster on canvas panning..
    if(!pcaps->bCanDoFBO)
        return false;
#endif
    
    return true;
}

//      Setup default global options when config file is unavailable,
//      as on initial startup after new install
//      The global config object (ZCHX_CFG_INS) is available, so direct updates are also allowed

void OCPNPlatform::SetDefaultOptions( void )
{
#if 0
    //  General options, applied to all platforms
    g_bShowOutlines = true;
    
    g_CPAMax_NM = 20.;
    g_CPAWarn_NM = 2.;
    g_TCPA_Max = 30.;
    g_bMarkLost = true;
    g_MarkLost_Mins = 8;
    g_bRemoveLost = true;
    g_RemoveLost_Mins = 10;
    g_bShowCOG = true;
    g_ShowCOG_Mins = 6;
    g_bHideMoored = false;
    g_ShowMoored_Kts = 0.2;
    g_bTrackDaily = false;
    g_PlanSpeed = 6.;
    g_bFullScreenQuilt = true;
    g_bQuiltEnable = true;
    g_bskew_comp = false;
    g_bShowAreaNotices = false;
    g_bDrawAISSize = false;
    g_bShowAISName = false;
    g_nTrackPrecision = 2;
    g_bPreserveScaleOnX = true;
    g_nAWDefault = 50;
    g_nAWMax = 1852;
    gps_watchdog_timeout_ticks = 6;
    
    
    // Initial S52/S57 options
    ZCHX_CFG_INS->beginGroup("Settings/GlobalState");
    ZCHX_CFG_INS->WriteDefault("bShowS57Text", true );
    ZCHX_CFG_INS->WriteDefault("bShowS57ImportantTextOnly", false );
    ZCHX_CFG_INS->WriteDefault("nDisplayCategory", (int)(_DisCat)STANDARD );
    ZCHX_CFG_INS->WriteDefault("nSymbolStyle", (int)(_LUPname)PAPER_CHART );
    ZCHX_CFG_INS->WriteDefault("nBoundaryStyle", (int)(_LUPname)PLAIN_BOUNDARIES );

    ZCHX_CFG_INS->WriteDefault("bShowSoundg", false );
    ZCHX_CFG_INS->WriteDefault("bShowMeta", false );
    ZCHX_CFG_INS->WriteDefault("bUseSCAMIN", true );
    ZCHX_CFG_INS->WriteDefault("bShowAtonText", false );
    ZCHX_CFG_INS->WriteDefault("bShowLightDescription", false );
    ZCHX_CFG_INS->WriteDefault("bExtendLightSectors", true );
    ZCHX_CFG_INS->WriteDefault("bDeClutterText", true );
    ZCHX_CFG_INS->WriteDefault("bShowNationalText", true );

    ZCHX_CFG_INS->WriteDefault("S52_MAR_SAFETY_CONTOUR", 3 );
    ZCHX_CFG_INS->WriteDefault("S52_MAR_SHALLOW_CONTOUR", 2 );
    ZCHX_CFG_INS->WriteDefault("S52_MAR_DEEP_CONTOUR", 6 );
    ZCHX_CFG_INS->WriteDefault("S52_MAR_TWO_SHADES", 0  );
    ZCHX_CFG_INS->WriteDefault("S52_DEPTH_UNIT_SHOW", 1 );
    ZCHX_CFG_INS->WriteDefault("ZoomDetailFactorVector", 3 );
    ZCHX_CFG_INS->endGroup();
#endif
}


    
QString OCPNPlatform::GetSupplementalLicenseString()
{
    QString lic;
#ifdef __OCPN__ANDROID__
    lic = androidGetSupplementalLicense();
#endif    
    return lic;
}
    
//const           QString APP_DIR = QApplication::applicationDirPath();
//const           QString MAP_DIR = QString("%1/mapdata").arg(APP_DIR);
//const           QString PLUGIN_DIR = QString("%1/plugins").arg(MAP_DIR);



QString OCPNPlatform::NormalizePath(const QString &full_path) {
    return full_path;
}

QString OCPNPlatform::GetConfigFileName()
{
    return QString("%1/opencpn.ini").arg(zchxFuncUtil::getDataDir());
}


bool OCPNPlatform::DoFileSelectorDialog( QWidget *parent, QString *file_spec, QString Title, QString initDir, QString suggestedName, QString wildcard)
{
    QString file;
    bool save = (suggestedName.length() ? true : false);
    QString dir = initDir;
    if(suggestedName.length() > 0)
    {
        if(dir.right(1) == "/")
        {
            dir.append(suggestedName);
        } else
        {
            dir.append("/").append(suggestedName);
        }
    }
    if(save)
    {
        file = QFileDialog::getSaveFileName(parent, Title, dir, wildcard);
    } else
    {
        file = QFileDialog::getOpenFileName(parent, Title, dir, wildcard);
    }

    if(file.length() > 0)
    {
        *file_spec = file;
    }
    return file.length() > 0;
}

int OCPNPlatform::DoDirSelectorDialog( QWidget *parent, QString *file_spec, QString Title, QString initDir)
{
    QString dir = QFileDialog::getExistingDirectory(parent, Title, initDir);
    if(dir.length() > 0) *file_spec = dir;
    return dir.length() > 0;
}


//--------------------------------------------------------------------------
//      Platform Display Support
//--------------------------------------------------------------------------

QCursor OCPNPlatform::ShowBusySpinner(  Qt::CursorShape old )
{
    mOldShape = old;
    return QCursor(Qt::BusyCursor);
}

QCursor OCPNPlatform::HideBusySpinner( void )
{
    return QCursor(mOldShape);
}

double OCPNPlatform::GetDisplayDensityFactor()
{
#ifdef __OCPN__ANDROID__
    return getAndroidDisplayDensity();
#else
    return 1.0;
#endif
}
    

int OCPNPlatform::GetStatusBarFieldCount()
{
    return 5;            // default

}


double OCPNPlatform::getFontPointsperPixel( void )
{
    double pt_per_pixel = 1.0;
    if(m_pt_per_pixel == 0)
    {
        QFont f = FontMgr::Get().FindOrCreateFont( 12, "Microsoft YaHei", QFont::StyleNormal, QFont::Bold, false);
        double width = QFontMetricsF(f).width("H");
        double height = QFontMetricsF(f).height();
        if(height > 0) m_pt_per_pixel = 12.0 / height;
    }
    if(m_pt_per_pixel > 0) pt_per_pixel = m_pt_per_pixel;
    
    return pt_per_pixel;
    
}

QSize OCPNPlatform::getDisplaySize()
{
    if(m_displaySize.width() < 10)
        m_displaySize = QApplication::desktop()->screenGeometry().size();               // default, for most platforms
    return m_displaySize;

}

double  OCPNPlatform::GetDisplaySizeMM()
{
    if(m_displaySizeMMOverride > 0) return m_displaySizeMMOverride;

    if(m_displaySizeMM.width() < 1) m_displaySizeMM = QSize(QApplication::desktop()->screen()->widthMM(), QApplication::desktop()->screen()->heightMM());

    double ret = m_displaySizeMM.width();
    return ret * 1.0;
}

void OCPNPlatform::SetDisplaySizeMM( double sizeMM)
{
    m_displaySizeMMOverride = sizeMM;
}


double OCPNPlatform::GetDisplayDPmm()
{
    double r = getDisplaySize().width();            // dots
    return r / GetDisplaySizeMM();
}
                    
unsigned int OCPNPlatform::GetSelectRadiusPix()
{
    return GetDisplayDPmm() * (g_btouch ? g_selection_radius_touch_mm : g_selection_radius_mm);
}

void OCPNPlatform::onStagedResizeFinal()
{
#ifdef __OCPN__ANDROID__
    androidConfirmSizeCorrection();
#endif
    
}

bool OCPNPlatform::GetFullscreen()
{
    bool bret = false;
    return bret;
}

bool OCPNPlatform::SetFullscreen( bool bFull )
{
    bool bret = false;
    return bret;
}

double OCPNPlatform::GetToolbarScaleFactor( int GUIScaleFactor )
{
    double rv = 1.0;
    double premult = 1.0;
#if 0
    if(g_bresponsive){
    //  Get the basic size of a tool icon
        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        QSize style_tool_size = style->GetToolSize();
        double tool_size = style_tool_size.width();

    // unless overridden by user, we declare the "best" tool size
    // to be roughly 9 mm square.
        double target_size = 9.0;                // mm

        double basic_tool_size_mm = tool_size / GetDisplayDPmm();
        premult = target_size / basic_tool_size_mm;
    }
#endif

    //Adjust the scale factor using the global GUI scale parameter
    double postmult =  exp( GUIScaleFactor * (0.693 / 5.0) );       //  exp(2)
    
    
    rv = premult * postmult;
    rv = qMin(rv, 3.0);      //  Clamp at 3.0
    rv = qMax(rv, 0.5);      //  and at 0.5
    return rv;
}

double OCPNPlatform::GetCompassScaleFactor( int GUIScaleFactor )
{
    double rv = 1.0;
    double premult = 1.0;
#if 0
    if(g_bresponsive ){
        
        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        QSize style_tool_size = style->GetToolSize();
        double compass_size = style_tool_size.width();

        // We declare the "best" tool size to be roughly 6 mm.
        double target_size = 6.0;                // mm
        
        double basic_tool_size_mm = compass_size / GetDisplayDPmm();
        premult = target_size / basic_tool_size_mm;
    }
#endif
        
    double postmult =  exp( GUIScaleFactor * (0.693 / 5.0) );       //  exp(2)

    rv = premult * postmult;
    rv = qMin(rv, 3.0);      //  Clamp at 3.0
    rv = qMax(rv, 0.5);
    return rv;
}

float OCPNPlatform::getChartScaleFactorExp( float scale_linear )
{
    double factor = 1.0;
    factor =  exp( scale_linear * (log(3.0) / 5.0) );
    factor = qMax(factor, .5);
    factor = qMin(factor, 4.);

    return factor;
}        



bool OCPNPlatform::GetWindowsMonitorSize( int &width, int &height)
{
    if(QApplication::desktop()->screenCount() <= 0) return false;
    width = QApplication::desktop()->screenGeometry().width();
    height = QApplication::desktop()->screenGeometry().height();
    return true;
}

void OCPNPlatform::initSystemInfo()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    mCpuCoreNum = sysInfo.dwNumberOfProcessors;
    qDebug()<<"total Cpu Number is:"<<mCpuCoreNum;
#if 0

    m_pageSize->setText(QString("分页大小:\t%1").arg(sysInfo.dwPageSize));
    char buff[32];
    sprintf(buff, "%p", sysInfo.lpMinimumApplicationAddress);
    m_minAddress->setText(QString("最小寻址:\t%1").arg(buff));
    sprintf(buff, "%p", sysInfo.lpMaximumApplicationAddress);
    m_maxAddress->setText(QString("最大寻址:\t%1").arg(buff));
    m_mask->setText(QString("掩码:\t\t%1").arg(sysInfo.dwActiveProcessorMask));
    m_processorNum->setText(QString("处理器个数:\t%1").arg(sysInfo.dwNumberOfProcessors));
    m_processorType->setText(QString("类型:\t\t%1").arg(sysInfo.dwProcessorType));
    m_processorLevel->setText(QString("等级:\t\t%1").arg(sysInfo.wProcessorLevel));
    m_processorVersion->setText(QString("版本:\t\t%1").arg(sysInfo.wProcessorRevision));
#endif
}













    
    



