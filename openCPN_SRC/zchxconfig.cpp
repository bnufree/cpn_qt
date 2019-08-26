#include "zchxconfig.h"
#include <QTextCodec>
#include "GL/gl.h"
#include "s52plib.h"
#include "s52utils.h"
#include "_def.h"
#include <QDebug>
#include "zchxmapmainwindow.h"
#include "OCPNPlatform.h"
#include "CanvasConfig.h"
#include "chartdbs.h"
#include "styles.h"


zchxConfig* zchxConfig::minstance = 0;
zchxConfig::MGarbage zchxConfig::Garbage;

extern OCPNPlatform                 *g_Platform;
extern zchxMapMainWindow          *gFrame;

extern double           g_ChartNotRenderScaleFactor;
extern int              g_restore_stackindex;
extern int              g_restore_dbindex;
extern int              g_LayerIdx;
extern ArrayOfCDI       g_ChartDirArray;
extern double           vLat, vLon, gLat, gLon;
extern double           kLat, kLon;
extern double           initial_scale_ppm, initial_rotation;
extern ColorScheme      global_color_scheme;
extern int              g_nbrightness;
extern bool             g_bShowTrue, g_bShowMag;
extern double           g_UserVar;
extern bool             g_bShowStatusBar;
extern bool             g_bUIexpert;
extern bool             g_bFullscreen;
extern int              g_nDepthUnitDisplay;
extern QString         g_csv_locn;
extern QString         g_SENCPrefix;
extern QString         g_UserPresLibData;
extern QString         *pInit_Chart_Dir;
extern QString         gWorldMapLocation;
extern bool             s_bSetSystemTime;
extern bool             g_bDisplayGrid;         //Flag indicating if grid is to be displayed
extern bool             g_bPlayShipsBells;
extern int              g_iSoundDeviceIndex;
extern bool             g_bFullscreenToolbar;
extern bool             g_bShowLayers;
extern bool             g_bTransparentToolbar;
extern bool             g_bPermanentMOBIcon;

extern bool             g_bShowDepthUnits;
extern bool             g_bAutoAnchorMark;
extern bool             g_bskew_comp;
extern bool             g_bopengl;
extern bool             g_bdisable_opengl;
extern bool             g_bSoftwareGL;
extern bool             g_bShowFPS;
extern bool             g_bsmoothpanzoom;
extern bool             g_fog_overzoom;
extern double           g_overzoom_emphasis_base;
extern bool             g_oz_vector_scale;

extern bool             g_bShowOutlines;
extern bool             g_bShowActiveRouteHighway;
extern bool             g_bShowRouteTotal;
extern int              g_nNMEADebug;
extern int              g_nAWDefault;
extern int              g_nAWMax;
extern int              g_nTrackPrecision;

extern int              g_iSDMMFormat;
extern int              g_iDistanceFormat;
extern int              g_iSpeedFormat;

extern int              g_nframewin_x;
extern int              g_nframewin_y;
extern int              g_nframewin_posx;
extern int              g_nframewin_posy;
extern bool             g_bframemax;

extern double           g_PlanSpeed;
extern QString         g_VisibleLayers;
extern QString         g_InvisibleLayers;
extern QString         g_VisiNameinLayers;
extern QString         g_InVisiNameinLayers;
extern QRect           g_blink_rect;

extern QStringList    *pMessageOnceArray;

// LIVE ETA OPTION
extern bool             g_bShowLiveETA;
extern double           g_defaultBoatSpeed;
extern double           g_defaultBoatSpeedUserUnit;

//    AIS Global configuration
extern bool             g_bCPAMax;
extern double           g_CPAMax_NM;
extern bool             g_bCPAWarn;
extern double           g_CPAWarn_NM;
extern bool             g_bTCPA_Max;
extern double           g_TCPA_Max;
extern bool             g_bMarkLost;
extern double           g_MarkLost_Mins;
extern bool             g_bRemoveLost;
extern double           g_RemoveLost_Mins;
extern bool             g_bShowCOG;
extern double           g_ShowCOG_Mins;
extern bool             g_bAISShowTracks;
extern bool             g_bTrackCarryOver;
extern bool             g_bTrackDaily;
extern int              g_track_rotate_time;
extern int              g_track_rotate_time_type;
extern double           g_AISShowTracks_Mins;
extern double           g_AISShowTracks_Limit;
extern bool             g_bHideMoored;
extern double           g_ShowMoored_Kts;
extern bool             g_bAllowShowScaled;
extern bool             g_bShowScaled;
extern int              g_ShowScaled_Num;
extern bool             g_bAIS_CPA_Alert;
extern bool             g_bAIS_CPA_Alert_Audio;
extern int              g_ais_alert_dialog_x, g_ais_alert_dialog_y;
extern int              g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;
extern int              g_ais_query_dialog_x, g_ais_query_dialog_y;
extern QString         g_sAIS_Alert_Sound_File;
extern bool             g_bAIS_CPA_Alert_Suppress_Moored;
extern bool             g_bAIS_ACK_Timeout;
extern double           g_AckTimeout_Mins;
extern QString         g_AisTargetList_perspective;
extern int              g_AisTargetList_range;
extern int              g_AisTargetList_sortColumn;
extern bool             g_bAisTargetList_sortReverse;
extern QString         g_AisTargetList_column_spec;
extern QString         g_AisTargetList_column_order;
extern bool             g_bShowAreaNotices;
extern bool             g_bDrawAISSize;
extern bool             g_bShowAISName;
extern int              g_Show_Target_Name_Scale;
extern bool             g_bWplIsAprsPosition;
extern bool             g_benableAISNameCache;
extern bool             g_bUseOnlyConfirmedAISName;
extern int              g_ScaledNumWeightSOG;
extern int              g_ScaledNumWeightCPA;
extern int              g_ScaledNumWeightTCPA;
extern int              g_ScaledNumWeightRange;
extern int              g_ScaledNumWeightSizeOfT;
extern int              g_ScaledSizeMinimal;

extern int              g_S57_dialog_sx, g_S57_dialog_sy;
int                     g_S57_extradialog_sx, g_S57_extradialog_sy;

extern int              g_iNavAidRadarRingsNumberVisible;
extern float            g_fNavAidRadarRingsStep;
extern int              g_pNavAidRadarRingsStepUnits;
extern int              g_iWaypointRangeRingsNumber;
extern float            g_fWaypointRangeRingsStep;
extern int              g_iWaypointRangeRingsStepUnits;
extern QColor         g_colourWaypointRangeRingsColour;
extern bool             g_bWayPointPreventDragging;
extern bool             g_bConfirmObjectDelete;
extern QColor         g_colourOwnshipRangeRingsColour;
extern int              g_iWpt_ScaMin;
extern bool             g_bUseWptScaMin;
extern bool             g_bOverruleScaMin;
extern bool             g_bShowWptName;


extern bool             g_bEnableZoomToCursor;
extern QString         g_toolbarConfig;
extern QString         g_toolbarConfigSecondary;
extern double           g_TrackIntervalSeconds;
extern double           g_TrackDeltaDistance;
extern int              gps_watchdog_timeout_ticks;

extern int              g_nCacheLimit;
extern int              g_memCacheLimit;

extern bool             g_bGDAL_Debug;
extern bool             g_bDebugCM93;
extern bool             g_bDebugS57;

extern double           g_ownship_predictor_minutes;
extern double           g_ownship_HDTpredictor_miles;

extern bool             g_own_ship_sog_cog_calc;
extern int              g_own_ship_sog_cog_calc_damp_sec;

extern bool             g_bShowMenuBar;
extern bool             g_bShowCompassWin;

extern s52plib          *ps52plib;

extern int              g_cm93_zoom_factor;
extern bool             g_b_legacy_input_filter_behaviour;
extern bool             g_bShowDetailSlider;
extern int              g_detailslider_dialog_x, g_detailslider_dialog_y;

extern bool             g_bUseGreenShip;

extern bool             g_b_overzoom_x;                      // Allow high overzoom
extern int              g_OwnShipIconType;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;
extern double           g_n_gps_antenna_offset_y;
extern double           g_n_gps_antenna_offset_x;
extern int              g_n_ownship_min_mm;
extern double           g_n_arrival_circle_radius;

extern bool             g_bPreserveScaleOnX;
extern bool             g_bsimplifiedScalebar;

extern bool             g_bUseRMC;
extern bool             g_bUseGLL;

extern QString         g_locale;
extern QString         g_localeOverride;

extern bool             g_bUseRaster;
extern bool             g_bUseVector;
extern bool             g_bUseCM93;

extern bool             g_bCourseUp;
extern bool             g_bLookAhead;
extern int              g_COGAvgSec;
extern bool             g_bMagneticAPB;
extern bool             g_bShowChartBar;

extern int              g_MemFootSec;
extern int              g_MemFootMB;

extern int              g_nCOMPortCheck;

extern bool             g_bbigred;

extern QString         g_AW1GUID;
extern QString         g_AW2GUID;
extern int              g_BSBImgDebug;

extern int             n_NavMessageShown;
extern QString        g_config_version_string;

extern QString        g_CmdSoundString;

extern bool             g_bAISRolloverShowClass;
extern bool             g_bAISRolloverShowCOG;
extern bool             g_bAISRolloverShowCPA;

extern bool             g_bDebugGPSD;

extern bool             g_bfilter_cogsog;
extern int              g_COGFilterSec;
extern int              g_SOGFilterSec;

int                     g_navobjbackups;

extern bool             g_bQuiltEnable;
extern bool             g_bFullScreenQuilt;
extern bool             g_bQuiltStart;

extern int              g_SkewCompUpdatePeriod;

extern int              g_maintoolbar_x;
extern int              g_maintoolbar_y;
extern long             g_maintoolbar_orient;

extern int              g_GPU_MemSize;

extern int              g_lastClientRectx;
extern int              g_lastClientRecty;
extern int              g_lastClientRectw;
extern int              g_lastClientRecth;

extern bool             g_bHighliteTracks;
extern int              g_cog_predictor_width;
extern int              g_ais_cog_predictor_width;

extern int              g_route_line_width;
extern int              g_track_line_width;
extern QColor         g_colourTrackLineColour;
extern QString         g_default_wp_icon;
extern QString         g_default_routepoint_icon;

extern ChartGroupArray  *g_pGroupArray;
extern int              g_GroupIndex;

extern bool             g_bDebugOGL;
extern int              g_tcwin_scale;
extern QString         g_GPS_Ident;
extern bool             g_bGarminHostUpload;
extern QString         g_uploadConnection;

extern ocpnStyle::StyleManager* g_StyleManager;
extern QStringList    TideCurrentDataSet;
extern QString         g_TCData_Dir;

extern bool             g_btouch;
extern bool             g_bresponsive;

extern bool             bGPSValid;              // for track recording
extern bool             g_bGLexpert;

extern int              g_SENC_LOD_pixels;

extern int              g_chart_zoom_modifier;
extern int              g_chart_zoom_modifier_vector;

extern int              g_NMEAAPBPrecision;

extern QString         g_TalkerIdText;
extern int              g_maxWPNameLength;

extern bool             g_bAdvanceRouteWaypointOnArrivalOnly;
extern double           g_display_size_mm;
extern double           g_config_display_size_mm;
extern bool             g_config_display_size_manual;

extern float            g_selection_radius_mm;
extern float            g_selection_radius_touch_mm;

extern bool             g_benable_rotate;
extern bool             g_bEmailCrashReport;

extern int              g_default_font_size;

extern bool             g_bAutoHideToolbar;
extern int              g_nAutoHideToolbar;
extern int              g_GUIScaleFactor;
extern int              g_ChartScaleFactor;
extern float            g_ChartScaleFactorExp;
extern int              g_ShipScaleFactor;
extern float            g_ShipScaleFactorExp;

extern bool             g_bInlandEcdis;
extern int              g_iENCToolbarPosX;
extern int              g_iENCToolbarPosY;

extern bool             g_bSpaceDropMark;

extern bool             g_bShowTide;
extern bool             g_bShowCurrent;

extern bool             g_benableUDPNullHeader;

extern QString         g_uiStyle;
extern bool             g_useMUI;

int                     g_nCPUCount;

extern bool             g_bDarkDecorations;
extern unsigned int     g_canvasConfig;
extern arrayofCanvasConfigPtr g_canvasConfigArray;
extern QString         g_lastAppliedTemplateGUID;

extern int              g_route_prop_x, g_route_prop_y;
extern int              g_route_prop_sx, g_route_prop_sy;

QString                g_gpx_path;
bool                    g_bLayersLoaded;
extern zchxGLOptions g_GLOptions;

/*-------------------------------------------
 * 实例化
---------------------------------------------*/
zchxConfig *zchxConfig::instance()
{
    if ( minstance == 0)
    {
        minstance = new zchxConfig(zchxFuncUtil::getConfigFileName());
    }
    return minstance;
}

zchxConfig::zchxConfig( const QString &LocalFileName ) : QSettings(LocalFileName)
{
    this->setIniCodec(QTextCodec::codecForName("UTF-8"));
    m_bSkipChangeSetUpdate = false;
    loadMyConfig();
}

/*-------------------------------------------
 * 设置默认值
---------------------------------------------*/
void zchxConfig::setDefault(const QString & prefix,const QString &key, const QVariant &def)
{
    beginGroup(prefix);
    if(value(key).toString().isEmpty())
    {
        setValue(key, def);
    }
    endGroup();
}
/*-------------------------------------------
 * 设置配置文件值
---------------------------------------------*/
void zchxConfig::setCustomValue(const QString & prefix,const QString & key, const QVariant & value)
{
    beginGroup(prefix);
    {
        setValue(key, value);
    }
    endGroup();
}
/*-------------------------------------------
 * 返回值
---------------------------------------------*/
QVariant zchxConfig::getCustomValue(const QString& prefix,const QString &keys, const QVariant &defaultValue)
{
    return value(prefix+"/"+keys,defaultValue);
}

int  zchxConfig::getChildCount(const QString& prefix)
{
    return getChildKeys(prefix).size();
}

QStringList zchxConfig::getChildKeys(const QString& prefix)
{
    QStringList keys;
    beginGroup(prefix);
    keys = childKeys();
    endGroup();
    return keys;
}

//bool zchxConfig::isExpert()
//{
//    return value(COMMON_SEC, OPENGL_EXPERT, false).toBool();
//}

//bool zchxConfig::isUseOpenGL()
//{
//    return value(COMMON_SEC, USE_OPEN_GL, true).toBool();
//}

//quint32 zchxConfig::getRasterFormat()
//{
//    return value(COMMON_SEC, RASTER_FORMAT, GL_RGB).toUInt();
//}

int zchxConfig::loadMyConfig()
{
    g_useMUI = true;
    g_TalkerIdText = ("EC");
    g_maxWPNameLength = 6;
    g_NMEAAPBPrecision = 3;
    g_GLOptions.m_bUseAcceleratedPanning = true;
    g_GLOptions.m_GLPolygonSmoothing = true;
    g_GLOptions.m_GLLineSmoothing = true;
    g_GLOptions.m_iTextureDimension = 512;
    g_GLOptions.m_iTextureMemorySize = 128;
    if(!g_bGLexpert){
        g_GLOptions.m_iTextureMemorySize = qMax(128, g_GLOptions.m_iTextureMemorySize);
        g_GLOptions.m_bTextureCompressionCaching = g_GLOptions.m_bTextureCompression;
    }

    g_maintoolbar_orient = 0;
    g_iENCToolbarPosX = -1;
    g_iENCToolbarPosY = -1;
    g_restore_dbindex = -1;
    g_ChartNotRenderScaleFactor = 1.5;
    g_detailslider_dialog_x = 200L;
    g_detailslider_dialog_y = 200L;
    g_SENC_LOD_pixels = 2;
    g_SkewCompUpdatePeriod = 10;

    g_bShowStatusBar = 1;
    g_bShowCompassWin = 1;
    g_iSoundDeviceIndex = -1;
    g_bFullscreenToolbar = 1;
    g_bTransparentToolbar =  0;
    g_bShowLayers = 1;
    g_bShowDepthUnits = 1;
    g_bShowActiveRouteHighway = 1;
    g_bShowChartBar = 1;
    g_defaultBoatSpeed = 6.0;
    g_ownship_predictor_minutes = 5;
    g_cog_predictor_width = 3;
    g_ownship_HDTpredictor_miles = 1;
    g_n_ownship_min_mm = 1;
    g_own_ship_sog_cog_calc_damp_sec = 1;
    g_bFullScreenQuilt = 1;
    g_track_rotate_time_type =  3;
    g_bHighliteTracks = 1;
    g_bPreserveScaleOnX = 1;
    g_navobjbackups = 5;
    g_benableAISNameCache = true;
    g_n_arrival_circle_radius = 0.05;

    g_AISShowTracks_Mins = 20;
    g_AISShowTracks_Limit = 300.0;
    g_ShowScaled_Num = 10;
    g_ScaledNumWeightSOG = 50;
    g_ScaledNumWeightCPA = 60;
    g_ScaledNumWeightTCPA = 25;
    g_ScaledNumWeightRange = 75;
    g_ScaledNumWeightSizeOfT = 25;
    g_ScaledSizeMinimal = 50;
    g_Show_Target_Name_Scale = 250000;
    g_bWplIsAprsPosition = 1;
    g_ais_cog_predictor_width = 3;
    g_ais_alert_dialog_sx = 200;
    g_ais_alert_dialog_sy = 200;
    g_ais_alert_dialog_x = 200;
    g_ais_alert_dialog_y = 200;
    g_ais_query_dialog_x = 200;
    g_ais_query_dialog_y = 200;
    g_AisTargetList_range = 40;
    g_AisTargetList_sortColumn = 2; // Column #2 is MMSI
    g_S57_dialog_sx = 400;
    g_S57_dialog_sy = 400;
    g_S57_extradialog_sx = 400;
    g_S57_extradialog_sy = 400;

    //    Reasonable starting point
    vLat = START_LAT;                   // display viewpoint
    vLon = START_LON;
    gLat = START_LAT;                   // GPS position, as default
    gLon = START_LON;
    initial_scale_ppm = .0003;        // decent initial value
    initial_rotation = 0;

    g_iNavAidRadarRingsNumberVisible = 0;
    g_fNavAidRadarRingsStep = 1.0;
    g_pNavAidRadarRingsStepUnits = 0;
    g_colourOwnshipRangeRingsColour = QColor(Qt::red);
    g_iWaypointRangeRingsNumber = 0;
    g_fWaypointRangeRingsStep = 1.0;
    g_iWaypointRangeRingsStepUnits = 0;
    g_colourWaypointRangeRingsColour = QColor(Qt::red );
    g_bConfirmObjectDelete = true;

    g_TrackIntervalSeconds = 60.0;
    g_TrackDeltaDistance = 0.10;
    g_route_line_width = 2;
    g_track_line_width = 2;
    g_colourTrackLineColour = QColor( 243, 229, 47 );

    g_tcwin_scale = 100;
    g_default_wp_icon = ("triangle");
    g_default_routepoint_icon = ("diamond");

    g_nAWDefault = 50;
    g_nAWMax = 1852;

    // Load the raw value, with no defaults, and no processing
    int ret_Val = LoadMyConfigRaw();

    //  Perform any required post processing and validation
    if(!ret_Val){
        g_ChartScaleFactorExp = g_Platform->getChartScaleFactorExp( g_ChartScaleFactor );
        g_ShipScaleFactorExp = g_Platform->getChartScaleFactorExp( g_ShipScaleFactor );

        g_COGFilterSec = qMin(g_COGFilterSec, MAX_COGSOG_FILTER_SECONDS);
        g_COGFilterSec = qMax(g_COGFilterSec, 1);
        g_SOGFilterSec = g_COGFilterSec;

        if(!g_bShowTrue && !g_bShowMag)
            g_bShowTrue = true;
        g_COGAvgSec = qMin(g_COGAvgSec, MAX_COG_AVERAGE_SECONDS);        // Bound the array size

        if( g_bInlandEcdis )
            g_bLookAhead=1;

        if ( g_bdisable_opengl )
            g_bopengl = false;
        if(!g_bGLexpert){
            g_GLOptions.m_iTextureMemorySize = qMax(128, g_GLOptions.m_iTextureMemorySize);
            g_GLOptions.m_bTextureCompressionCaching = g_GLOptions.m_bTextureCompression;
        }

        g_chart_zoom_modifier = qMin(g_chart_zoom_modifier,5);
        g_chart_zoom_modifier = qMax(g_chart_zoom_modifier,-5);
        g_chart_zoom_modifier_vector = qMin(g_chart_zoom_modifier_vector,5);
        g_chart_zoom_modifier_vector = qMax(g_chart_zoom_modifier_vector,-5);
        g_cm93_zoom_factor = qMin(g_cm93_zoom_factor,CM93_ZOOM_FACTOR_MAX_RANGE);
        g_cm93_zoom_factor = qMax(g_cm93_zoom_factor,(-CM93_ZOOM_FACTOR_MAX_RANGE));

        if( ( g_detailslider_dialog_x < 0 ) )
            g_detailslider_dialog_x = 5;
        if( ( g_detailslider_dialog_y < 0 ) )
            g_detailslider_dialog_y =  5;

        g_defaultBoatSpeedUserUnit = zchxFuncUtil::toUsrSpeed(g_defaultBoatSpeed, -1);
        g_n_ownship_min_mm = qMax(g_n_ownship_min_mm, 1);
        if( g_navobjbackups > 99 ) g_navobjbackups = 99;
        if( g_navobjbackups < 0 ) g_navobjbackups = 0;

        g_selection_radius_mm = fmax(g_selection_radius_mm, 0.5);
        g_selection_radius_touch_mm = fmax(g_selection_radius_touch_mm, 1.0);

        g_Show_Target_Name_Scale = qMax( 5000, g_Show_Target_Name_Scale );
        if ( g_bInlandEcdis )
            global_color_scheme = GLOBAL_COLOR_SCHEME_DUSK; //startup in duskmode if inlandEcdis
    }

    return ret_Val;
}

QVariant zchxConfig::Read(const QString &key, ParamType type, void *ret, const QVariant& def)
{
    QVariant val = value(key, def);
    switch (type) {
    case PARAM_BOOL:
        *(bool*)ret = val.toBool();
        break;
    case PARAM_INT:
        *(int*)ret = val.toInt();
        break;
    case PARAM_DOUBLE:
        *(double*)ret = val.toDouble();
        break;
    case PARAM_FLOAT:
        *(float*)ret = val.toFloat();
        break;
    case PARAM_STRING:
        *(QString*)ret = val.toString();
        break;
    case PARAM_STRINGLIST:
        *(QStringList*)ret = val.toStringList();
        break;
    default:
        break;
    }

    return val;
}

int zchxConfig::LoadMyConfigRaw( bool bAsTemplate )
{

    int read_int;
    QString val;

    beginGroup("Settings" );

    Read("LastAppliedTemplate", PARAM_STRING, &g_lastAppliedTemplateGUID );
    // Some undocumented values
    Read("ConfigVersionString", PARAM_STRING, &g_config_version_string );
    Read("NavMessageShown", PARAM_INT, &n_NavMessageShown );
    Read("UIexpert",PARAM_BOOL, &g_bUIexpert );
    Read("UIStyle", PARAM_STRING, &g_uiStyle  );
    Read("NCacheLimit", PARAM_INT, &g_nCacheLimit );

    Read("InlandEcdis",PARAM_BOOL, &g_bInlandEcdis );

    Read("DarkDecorations", PARAM_BOOL,&g_bDarkDecorations );

    Read("SpaceDropMark",PARAM_BOOL, &g_bSpaceDropMark );

    int mem_limit = 0;
    Read("MEMCacheLimit",PARAM_INT, &mem_limit );
    if(mem_limit > 0)
        g_memCacheLimit = mem_limit * 1024;       // convert from MBytes to kBytes

    Read("UseModernUI5",PARAM_BOOL, &g_useMUI );

    Read("NCPUCount", PARAM_INT,&g_nCPUCount);

    Read("DebugGDAL", PARAM_BOOL,&g_bGDAL_Debug );
    Read("DebugNMEA", PARAM_INT,&g_nNMEADebug );
    Read("AnchorWatchDefault",PARAM_INT, &g_nAWDefault );
    Read("AnchorWatchMax", PARAM_INT, &g_nAWMax );
    Read("GPSDogTimeout",PARAM_INT, &gps_watchdog_timeout_ticks );
    Read("DebugCM93", PARAM_BOOL, &g_bDebugCM93 );
    Read("DebugS57", PARAM_BOOL, &g_bDebugS57 );
    Read("DebugBSBImg", PARAM_INT, &g_BSBImgDebug );
    Read("DebugGPSD", PARAM_BOOL,&g_bDebugGPSD );

    Read("DefaultFontSize",PARAM_INT, &g_default_font_size );

    Read("UseGreenShipIcon",PARAM_BOOL, &g_bUseGreenShip );


    Read("AutoHideToolbar", PARAM_BOOL,&g_bAutoHideToolbar );
    Read("AutoHideToolbarSecs",PARAM_INT, &g_nAutoHideToolbar );

    Read("UseSimplifiedScalebar", PARAM_BOOL,&g_bsimplifiedScalebar );
    Read("ShowTide", PARAM_BOOL,&g_bShowTide );
    Read("ShowCurrent", PARAM_BOOL, &g_bShowCurrent );

    int size_mm = -1;
    Read("DisplaySizeMM", PARAM_INT, &size_mm );

    Read("SelectionRadiusMM", PARAM_FLOAT, &g_selection_radius_mm);
    Read("SelectionRadiusTouchMM", PARAM_FLOAT, &g_selection_radius_touch_mm);

    if(!bAsTemplate){
        if(size_mm > 0){
            g_config_display_size_mm = size_mm;
            if((size_mm > 100) && (size_mm < 2000)){
                g_display_size_mm = size_mm;
            }
        }
        Read("DisplaySizeManual", PARAM_BOOL, &g_config_display_size_manual );
    }

    Read("GUIScaleFactor", PARAM_INT, &g_GUIScaleFactor );

    Read("ChartObjectScaleFactor", PARAM_INT,  &g_ChartScaleFactor );
    Read("ShipScaleFactor",  PARAM_INT, &g_ShipScaleFactor );


    //  NMEA connection options.
    if( !bAsTemplate ){
        Read("FilterNMEA_Avg",  PARAM_BOOL, &g_bfilter_cogsog );
        Read("FilterNMEA_Sec",  PARAM_INT, &g_COGFilterSec );
        Read("GPSIdent",  PARAM_STRING, &g_GPS_Ident );
        Read("UseGarminHostUpload",   PARAM_BOOL, &g_bGarminHostUpload );
        Read("UseNMEA_GLL",  PARAM_BOOL, &g_bUseGLL );
        Read("UseMagAPB",  PARAM_BOOL, &g_bMagneticAPB );
    }

    Read("ShowTrue",  PARAM_BOOL, &g_bShowTrue );
    Read("ShowMag",  PARAM_BOOL, &g_bShowMag );


    QString umv;
    Read("UserMagVariation",  PARAM_STRING, &umv );
    if(umv.length()) g_UserVar = umv.toDouble();

    Read("ScreenBrightness",  PARAM_INT, &g_nbrightness );

    Read("MemFootprintTargetMB",  PARAM_INT, &g_MemFootMB );

    Read("WindowsComPortMax",  PARAM_INT, &g_nCOMPortCheck );

    Read("ChartQuilting",  PARAM_BOOL, &g_bQuiltEnable );
    Read("ChartQuiltingInitial",  PARAM_BOOL, &g_bQuiltStart );

    Read("CourseUpMode",  PARAM_BOOL, &g_bCourseUp );
    Read("COGUPAvgSeconds",   PARAM_INT, &g_COGAvgSec );
    Read("LookAheadMode",   PARAM_BOOL, &g_bLookAhead );
    Read("SkewToNorthUp",   PARAM_BOOL, &g_bskew_comp );

    Read("ShowFPS",   PARAM_BOOL, &g_bShowFPS );
    Read("ActiveChartGroup", PARAM_INT, &g_GroupIndex );
    Read("NMEAAPBPrecision",  PARAM_INT,  &g_NMEAAPBPrecision );

    Read("TalkerIdText",   PARAM_STRING, &g_TalkerIdText );
    Read("MaxWaypointNameLength",   PARAM_INT, &g_maxWPNameLength );

    /* opengl options */
#ifdef ocpnUSE_GL
    if(!bAsTemplate ){
        Read("OpenGLExpert",  PARAM_BOOL, &g_bGLexpert, false );
        Read("UseAcceleratedPanning", PARAM_BOOL, &g_GLOptions.m_bUseAcceleratedPanning, true );
        Read("GPUTextureCompression", PARAM_BOOL, &g_GLOptions.m_bTextureCompression);
        Read("GPUTextureCompressionCaching", PARAM_BOOL, &g_GLOptions.m_bTextureCompressionCaching);
        Read("PolygonSmoothing", PARAM_BOOL, &g_GLOptions.m_GLPolygonSmoothing);
        Read("LineSmoothing", PARAM_BOOL, &g_GLOptions.m_GLLineSmoothing);
        Read("GPUTextureDimension", PARAM_INT, &g_GLOptions.m_iTextureDimension );
        Read("GPUTextureMemSize", PARAM_INT, &g_GLOptions.m_iTextureMemorySize );
        Read("DebugOpenGL", PARAM_BOOL, &g_bDebugOGL );
        Read("OpenGL", PARAM_BOOL, &g_bopengl, true );
        Read("SoftwareGL", PARAM_BOOL, &g_bSoftwareGL, false );
    }
#endif

    Read("SmoothPanZoom", PARAM_BOOL, &g_bsmoothpanzoom );

    Read("ToolbarX", PARAM_INT, &g_maintoolbar_x );
    Read("ToolbarY", PARAM_INT, &g_maintoolbar_y );
    Read("ToolbarOrient",PARAM_INT,  &g_maintoolbar_orient );
    Read("GlobalToolbarConfig",PARAM_STRING,  &g_toolbarConfig );

    Read("iENCToolbarX", PARAM_INT, &g_iENCToolbarPosX );
    Read("iENCToolbarY", PARAM_INT, &g_iENCToolbarPosY );

    Read("AnchorWatch1GUID", PARAM_STRING, &g_AW1GUID );
    Read("AnchorWatch2GUID", PARAM_STRING, &g_AW2GUID );

    Read("InitialStackIndex", PARAM_INT, &g_restore_stackindex );
    Read("InitialdBIndex", PARAM_INT, &g_restore_dbindex );

    Read("ChartNotRenderScaleFactor", PARAM_DOUBLE, &g_ChartNotRenderScaleFactor );

    Read("MobileTouch", PARAM_BOOL, &g_btouch );
    Read("ResponsiveGraphics", PARAM_BOOL, &g_bresponsive );

    Read("ZoomDetailFactor", PARAM_INT, &g_chart_zoom_modifier );
    Read("ZoomDetailFactorVector", PARAM_INT, &g_chart_zoom_modifier_vector );

    Read("CM93DetailFactor", PARAM_INT, &g_cm93_zoom_factor );

    Read("CM93DetailZoomPosX", PARAM_INT, &g_detailslider_dialog_x );
    Read("CM93DetailZoomPosY", PARAM_INT, &g_detailslider_dialog_y );
    Read("ShowCM93DetailSlider", PARAM_BOOL, &g_bShowDetailSlider );

    Read("SENC_LOD_Pixels", PARAM_INT, &g_SENC_LOD_pixels );

    Read("SkewCompUpdatePeriod", PARAM_INT, &g_SkewCompUpdatePeriod );

    Read("SetSystemTime", PARAM_BOOL, &s_bSetSystemTime );
    Read("ShowStatusBar", PARAM_BOOL, &g_bShowStatusBar );
    Read("ShowMenuBar", PARAM_BOOL, &g_bShowMenuBar );
    Read("Fullscreen", PARAM_BOOL, &g_bFullscreen );
    Read("ShowCompassWindow", PARAM_BOOL, &g_bShowCompassWin );
    Read("ShowGrid", PARAM_BOOL, &g_bDisplayGrid );
    Read("PlayShipsBells", PARAM_BOOL, &g_bPlayShipsBells );
    Read("SoundDeviceIndex", PARAM_INT, &g_iSoundDeviceIndex );
    Read("FullscreenToolbar", PARAM_BOOL, &g_bFullscreenToolbar );
    Read("PermanentMOBIcon", PARAM_BOOL, &g_bPermanentMOBIcon );
    Read("ShowLayers", PARAM_BOOL, &g_bShowLayers );
    Read("ShowDepthUnits", PARAM_BOOL, &g_bShowDepthUnits );
    Read("AutoAnchorDrop", PARAM_BOOL, &g_bAutoAnchorMark );
    Read("ShowChartOutlines", PARAM_BOOL, &g_bShowOutlines );
    Read("ShowActiveRouteHighway", PARAM_BOOL, &g_bShowActiveRouteHighway );
    Read("ShowActiveRouteTotal", PARAM_BOOL, &g_bShowRouteTotal );
    Read("MostRecentGPSUploadConnection", PARAM_STRING, &g_uploadConnection);
    Read("ShowChartBar", PARAM_BOOL, &g_bShowChartBar );
    Read("SDMMFormat", PARAM_INT, &g_iSDMMFormat ); //0 = "Degrees, Decimal minutes"), 1 = "Decimal degrees", 2 = "Degrees,Minutes, Seconds"

    Read("DistanceFormat", PARAM_INT, &g_iDistanceFormat ); //0 = "Nautical miles"), 1 = "Statute miles", 2 = "Kilometers", 3 = "Meters"
    Read("SpeedFormat", PARAM_INT, &g_iSpeedFormat ); //0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"

    // LIVE ETA OPTION
    Read("LiveETA", PARAM_BOOL, &g_bShowLiveETA );
    Read("DefaultBoatSpeed", PARAM_DOUBLE, &g_defaultBoatSpeed );

    Read("OwnshipCOGPredictorMinutes", PARAM_DOUBLE, &g_ownship_predictor_minutes );
    Read("OwnshipCOGPredictorWidth", PARAM_INT,  &g_cog_predictor_width );
    Read("OwnshipHDTPredictorMiles", PARAM_DOUBLE, &g_ownship_HDTpredictor_miles );

    Read("OwnShipIconType", PARAM_INT, &g_OwnShipIconType );
    Read("OwnShipLength", PARAM_DOUBLE, &g_n_ownship_length_meters );
    Read("OwnShipWidth", PARAM_DOUBLE, &g_n_ownship_beam_meters );
    Read("OwnShipGPSOffsetX", PARAM_DOUBLE,&g_n_gps_antenna_offset_x );
    Read("OwnShipGPSOffsetY", PARAM_DOUBLE,&g_n_gps_antenna_offset_y );
    Read("OwnShipMinSize", PARAM_INT, &g_n_ownship_min_mm );
    Read("OwnShipSogCogCalc", PARAM_BOOL, &g_own_ship_sog_cog_calc );
    Read("OwnShipSogCogCalcDampSec", PARAM_INT, &g_own_ship_sog_cog_calc_damp_sec );

    QString racr;
    Read("RouteArrivalCircleRadius", PARAM_STRING, &racr );
    if(racr.length())  g_n_arrival_circle_radius = racr.toDouble();

    Read("FullScreenQuilt", PARAM_BOOL, &g_bFullScreenQuilt );

    Read("StartWithTrackActive", PARAM_BOOL, &g_bTrackCarryOver );
    Read("AutomaticDailyTracks", PARAM_BOOL, &g_bTrackDaily );
    Read("TrackRotateAt", PARAM_INT, &g_track_rotate_time );
    Read("TrackRotateTimeType", PARAM_INT, &g_track_rotate_time_type );
    Read("HighlightTracks", PARAM_BOOL, &g_bHighliteTracks );

    QString stps;
    Read("PlanSpeed", PARAM_STRING, &stps );
    if(!stps.isEmpty())  g_PlanSpeed = stps.toDouble();

    Read("VisibleLayers", PARAM_STRING, &g_VisibleLayers );
    Read("InvisibleLayers", PARAM_STRING, &g_InvisibleLayers );
    Read("VisNameInLayers", PARAM_STRING, &g_VisiNameinLayers );
    Read("InvisNameInLayers", PARAM_STRING, &g_InVisiNameinLayers );
    Read("PreserveScaleOnX", PARAM_BOOL, &g_bPreserveScaleOnX );

    Read("Locale", PARAM_STRING, &g_locale );
    Read("LocaleOverride", PARAM_STRING, &g_localeOverride );

    //We allow 0-99 backups ov navobj.xml
    Read("KeepNavobjBackups", PARAM_INT, &g_navobjbackups );

    // Boolean to cater for legacy Input COM Port filer behaviour, i.e. show msg filtered but put msg on bus.
    Read("LegacyInputCOMPortFilterBehaviour", PARAM_BOOL, &g_b_legacy_input_filter_behaviour );

    // Boolean to cater for sailing when not approaching waypoint
    Read("AdvanceRouteWaypointOnArrivalOnly", PARAM_BOOL,&g_bAdvanceRouteWaypointOnArrivalOnly);

    Read("EnableRotateKeys",  PARAM_BOOL, &g_benable_rotate );
    Read("EmailCrashReport",  PARAM_BOOL, &g_bEmailCrashReport );

    g_benableAISNameCache = true;
    Read("EnableAISNameCache",  PARAM_BOOL, &g_benableAISNameCache, true );

    Read("EnableUDPNullHeader", PARAM_BOOL, &g_benableUDPNullHeader );

    endGroup();

    beginGroup("Settings/GlobalState"  );

    Read("FrameWinX", PARAM_INT, &g_nframewin_x );
    Read("FrameWinY", PARAM_INT, &g_nframewin_y );
    Read("FrameWinPosX", PARAM_INT, &g_nframewin_posx );
    Read("FrameWinPosY", PARAM_INT, &g_nframewin_posy );
    Read("FrameMax", PARAM_BOOL, &g_bframemax );

    Read("ClientPosX", PARAM_INT, &g_lastClientRectx );
    Read("ClientPosY", PARAM_INT, &g_lastClientRecty );
    Read("ClientSzX", PARAM_INT, &g_lastClientRectw );
    Read("ClientSzY", PARAM_INT, &g_lastClientRecth );

    Read("RoutePropSizeX", PARAM_INT, &g_route_prop_sx );
    Read("RoutePropSizeY", PARAM_INT, &g_route_prop_sy );
    Read("RoutePropPosX", PARAM_INT, &g_route_prop_x );
    Read("RoutePropPosY", PARAM_INT, &g_route_prop_y );

    read_int = -1;
    Read("S52_DEPTH_UNIT_SHOW", PARAM_INT, &read_int );   // default is metres
    if(read_int >= 0){
        read_int = qMax(read_int, 0);        // qualify value
        read_int = qMin(read_int, 2);
        g_nDepthUnitDisplay = read_int;
    }

    endGroup();

    //    AIS
    QString s;
    beginGroup("Settings/AIS"  );

    g_bUseOnlyConfirmedAISName = false;
    Read("UseOnlyConfirmedAISName",  PARAM_BOOL, &g_bUseOnlyConfirmedAISName, false );
    Read("bNoCPAMax", PARAM_BOOL, &g_bCPAMax );
    Read("NoCPAMaxNMi", PARAM_DOUBLE, &g_CPAMax_NM );
    Read("bCPAWarn", PARAM_BOOL, &g_bCPAWarn );
    Read("CPAWarnNMi", PARAM_DOUBLE, &g_CPAWarn_NM );
    Read("bTCPAMax", PARAM_BOOL, &g_bTCPA_Max );
    Read("TCPAMaxMinutes", PARAM_DOUBLE, &g_TCPA_Max );
    Read("bMarkLostTargets", PARAM_BOOL, &g_bMarkLost );
    Read("MarkLost_Minutes", PARAM_DOUBLE, &g_MarkLost_Mins );
    Read("bRemoveLostTargets", PARAM_BOOL, &g_bRemoveLost );
    Read("RemoveLost_Minutes", PARAM_DOUBLE, &g_RemoveLost_Mins );
    Read("bShowCOGArrows", PARAM_BOOL, &g_bShowCOG );
    Read("CogArrowMinutes", PARAM_DOUBLE, &g_ShowCOG_Mins );
    Read("bShowTargetTracks", PARAM_BOOL, &g_bAISShowTracks );


    if( Read("TargetTracksLimit", &s ) ) {
        s.ToDouble( &g_AISShowTracks_Limit );
        g_AISShowTracks_Limit = qMax(300.0, g_AISShowTracks_Limit);
    }
    if( Read("TargetTracksMinutes", &s ) ) {
        s.ToDouble( &g_AISShowTracks_Mins );
        g_AISShowTracks_Mins = qMax(1.0, g_AISShowTracks_Mins);
        g_AISShowTracks_Mins = qMin(g_AISShowTracks_Limit, g_AISShowTracks_Mins);
    }

    Read("bHideMooredTargets", &g_bHideMoored );
    if(Read("MooredTargetMaxSpeedKnots", &s ))
        s.ToDouble( &g_ShowMoored_Kts );

    Read(_T ("bShowScaledTargets"), &g_bAllowShowScaled );
    Read("AISScaledNumber", &g_ShowScaled_Num );
    Read("AISScaledNumberWeightSOG", &g_ScaledNumWeightSOG );
    Read("AISScaledNumberWeightCPA", &g_ScaledNumWeightCPA );
    Read("AISScaledNumberWeightTCPA", &g_ScaledNumWeightTCPA );
    Read("AISScaledNumberWeightRange",& g_ScaledNumWeightRange );
    Read("AISScaledNumberWeightSizeOfTarget", &g_ScaledNumWeightSizeOfT );
    Read("AISScaledSizeMinimal", &g_ScaledSizeMinimal );
    Read(_T("AISShowScaled"), &g_bShowScaled );

    Read("bShowAreaNotices", &g_bShowAreaNotices );
    Read("bDrawAISSize", &g_bDrawAISSize );
    Read("bShowAISName", &g_bShowAISName );
    Read("bAISAlertDialog", &g_bAIS_CPA_Alert );
    Read("ShowAISTargetNameScale", &g_Show_Target_Name_Scale );
    Read("bWplIsAprsPositionReport", &g_bWplIsAprsPosition );
    Read("AISCOGPredictorWidth", &g_ais_cog_predictor_width );

    Read("bAISAlertAudio", &g_bAIS_CPA_Alert_Audio );
    Read("AISAlertAudioFile", &g_sAIS_Alert_Sound_File );
    Read("bAISAlertSuppressMoored", &g_bAIS_CPA_Alert_Suppress_Moored );

    Read("bAISAlertAckTimeout", &g_bAIS_ACK_Timeout );
    if(Read("AlertAckTimeoutMinutes", &s ))
        s.ToDouble( &g_AckTimeout_Mins );

    Read("AlertDialogSizeX", &g_ais_alert_dialog_sx );
    Read("AlertDialogSizeY", &g_ais_alert_dialog_sy );
    Read("AlertDialogPosX", &g_ais_alert_dialog_x );
    Read("AlertDialogPosY", &g_ais_alert_dialog_y );
    Read("QueryDialogPosX", &g_ais_query_dialog_x );
    Read("QueryDialogPosY", &g_ais_query_dialog_y );

    Read("AISTargetListPerspective", &g_AisTargetList_perspective );
    Read("AISTargetListRange", &g_AisTargetList_range );
    Read("AISTargetListSortColumn", &g_AisTargetList_sortColumn );
    Read("bAISTargetListSortReverse", &g_bAisTargetList_sortReverse );
    Read("AISTargetListColumnSpec", &g_AisTargetList_column_spec );
    Read("AISTargetListColumnOrder"), &g_AisTargetList_column_order);

    Read("bAISRolloverShowClass", &g_bAISRolloverShowClass );
    Read("bAISRolloverShowCOG", &g_bAISRolloverShowCOG );
    Read("bAISRolloverShowCPA", &g_bAISRolloverShowCPA );

    Read("S57QueryDialogSizeX", &g_S57_dialog_sx );
    Read("S57QueryDialogSizeY", &g_S57_dialog_sy );
    Read("S57QueryExtraDialogSizeX", &g_S57_extradialog_sx );
    Read("S57QueryExtraDialogSizeY", &g_S57_extradialog_sy );


    QString strpres("PresentationLibraryData" ) );
    QString valpres;
    SetPath("/Directories" ) );
    Read( strpres, &valpres );       // Get the File name
    if(!valpres.IsEmpty())
        g_UserPresLibData = valpres;

    QString strs("SENCFileLocation" ) );
    SetPath("/Directories" ) );
    QString vals;
    Read( strs, &vals );       // Get the Directory name
    if(!vals.IsEmpty())
        g_SENCPrefix = vals;

    SetPath("/Directories" ) );
    QString vald;
    Read("InitChartDir", &vald );    // Get the Directory name

    QString dirnamed( vald );
    if( !dirnamed.IsEmpty() ) {
        if( pInit_Chart_Dir->IsEmpty() )   // on second pass, don't overwrite
        {
            pInit_Chart_Dir->Clear();
            pInit_Chart_Dir->Append( vald );
        }
    }

    Read("GPXIODir", &g_gpx_path );    // Get the Directory name
    Read("TCDataDir", &g_TCData_Dir );    // Get the Directory name
    Read("BasemapDir"), &gWorldMapLocation );

    SetPath("/Settings/GlobalState" ) );

    if(Read("nColorScheme", &read_int ))
        global_color_scheme = (ColorScheme) read_int;

    if(! bAsTemplate ){
        SetPath("/Settings/NMEADataSource" ) );

        QString connectionconfigs;
        Read ("DataConnections",  &connectionconfigs );
        if(!connectionconfigs.IsEmpty()){
            QStringList confs = QStringTokenize(connectionconfigs, _T("|"));
            g_pConnectionParams->Clear();
            for (size_t i = 0; i < confs.Count(); i++)
            {
                ConnectionParams * prm = new ConnectionParams(confs[i]);
                if (!prm->Valid) {
                    ZCHX_LOGMSG("Skipped invalid DataStream config") );
                    delete prm;
                    continue;
                }
                g_pConnectionParams->Add(prm);
            }
        }
    }



    SetPath("/Settings/GlobalState" ) );
    QString st;

    double st_lat, st_lon;
    if( Read("VPLatLon", &st ) ) {
        sscanf( st.mb_str( wxConvUTF8, "%lf,%lf", &st_lat, &st_lon );

        //    Sanity check the lat/lon...both have to be reasonable.
        if( fabs( st_lon ) < 360. ) {
            while( st_lon < -180. )
                st_lon += 360.;

            while( st_lon > 180. )
                st_lon -= 360.;

            vLon = st_lon;
        }

        if( fabs( st_lat ) < 90.0 ) vLat = st_lat;

        s.Printf("Setting Viewpoint Lat/Lon %g, %g", vLat, vLon );
        ZCHX_LOGMSG( s );

    }

    double st_view_scale, st_rotation;
    if( Read( QString("VPScale" ), &st ) ) {
        sscanf( st.mb_str( wxConvUTF8, "%lf", &st_view_scale );
        //    Sanity check the scale
        st_view_scale = fmax ( st_view_scale, .001/32 );
        st_view_scale = fmin ( st_view_scale, 4 );
        initial_scale_ppm = st_view_scale;
    }

    if( Read( QString("VPRotation" ), &st ) ) {
        sscanf( st.mb_str( wxConvUTF8, "%lf", &st_rotation );
        //    Sanity check the rotation
        st_rotation = fmin ( st_rotation, 360 );
        st_rotation = fmax ( st_rotation, 0 );
        initial_rotation = st_rotation * PI / 180.;
    }

    QString sll;
    double lat, lon;
    if( Read("OwnShipLatLon", &sll ) ) {
        sscanf( sll.mb_str( wxConvUTF8, "%lf,%lf", &lat, &lon );

        //    Sanity check the lat/lon...both have to be reasonable.
        if( fabs( lon ) < 360. ) {
            while( lon < -180. )
                lon += 360.;

            while( lon > 180. )
                lon -= 360.;

            gLon = lon;
        }

        if( fabs( lat ) < 90.0 ) gLat = lat;

        s.Printf("Setting Ownship Lat/Lon %g, %g", gLat, gLon );
        ZCHX_LOGMSG( s );

    }

    //    Fonts

    //  Load the persistent Auxiliary Font descriptor Keys
    SetPath ("/Settings/AuxFontKeys" ) );

    QString strk;
    long dummyk;
    QString kval;
    bool bContk = GetFirstEntry( strk, dummyk );
    bool bNewKey = false;
    while( bContk ) {
        Read( strk, &kval );
        bNewKey = FontMgr::Get().AddAuxKey(kval);
        if(!bAsTemplate && !bNewKey) {
            DeleteEntry( strk );
            dummyk--;
        }
        bContk = GetNextEntry( strk, dummyk );
    }

#ifdef __WXX11__
    SetPath ("/Settings/X11Fonts" ) );
#endif

#ifdef __WXGTK__
    SetPath ("/Settings/GTKFonts" ) );
#endif

#ifdef __WXMSW__
    SetPath("/Settings/MSWFonts" ) );
#endif

#ifdef __WXMAC__
    SetPath ("/Settings/MacFonts" ) );
#endif

#ifdef __WXQT__
    SetPath ("/Settings/QTFonts" ) );
#endif

    QString str;
    long dummy;
    QString *pval = new QString;
    QStringList deleteList;

    bool bCont = GetFirstEntry( str, dummy );
    while( bCont ) {
        Read( str, pval );

        if( str.StartsWith( _T("Font") ) ) {
            // Convert pre 3.1 setting. Can't delete old entries from inside the
            // GetNextEntry() loop, so we need to save those and delete outside.
            deleteList.Add( str );
            QString oldKey = pval->BeforeFirst( _T(':') );
            str = FontMgr::GetFontConfigKey( oldKey );
        }

        if( pval->IsEmpty() || pval->StartsWith(_T(":")) ) {
            deleteList.Add( str );
        }
        else
            FontMgr::Get().LoadFontNative( &str, pval );

        bCont = GetNextEntry( str, dummy );
    }

    for( unsigned int i=0; i<deleteList.Count(); i++ ) {
        DeleteEntry( deleteList[i] );
    }
    deleteList.Clear();
    delete pval;

    //  Tide/Current Data Sources
    SetPath("/TideCurrentDataSources" ) );
    if( GetNumberOfEntries() ) {
        TideCurrentDataSet.Clear();
        QString str, val;
        long dummy;
        int iDir = 0;
        bool bCont = GetFirstEntry( str, dummy );
        while( bCont ) {
            Read( str, &val );       // Get a file name
            TideCurrentDataSet.Add(val);
            bCont = GetNextEntry( str, dummy );
        }
    }



    //    Groups
    LoadConfigGroups( g_pGroupArray );

    //     //    Multicanvas Settings
    //     LoadCanvasConfigs();

    SetPath("/Settings/Others" ) );

    // Radar rings
    Read("RadarRingsNumberVisible", &val );
    if( val.Length() > 0 ) g_iNavAidRadarRingsNumberVisible = atoi( val.mb_str() );

    Read("RadarRingsStep", &val );
    if( val.Length() > 0 ) g_fNavAidRadarRingsStep = atof( val.mb_str() );

    Read("RadarRingsStepUnits", &g_pNavAidRadarRingsStepUnits );

    QString l_wxsOwnshipRangeRingsColour;
    Read("RadarRingsColour", &l_wxsOwnshipRangeRingsColour );
    if(l_wxsOwnshipRangeRingsColour.Length()) g_colourOwnshipRangeRingsColour.Set( l_wxsOwnshipRangeRingsColour );

    // Waypoint Radar rings
    Read("WaypointRangeRingsNumber", &val );
    if( val.Length() > 0 ) g_iWaypointRangeRingsNumber = atoi( val.mb_str() );

    Read("WaypointRangeRingsStep", &val );
    if( val.Length() > 0 ) g_fWaypointRangeRingsStep = atof( val.mb_str() );

    Read("WaypointRangeRingsStepUnits", &g_iWaypointRangeRingsStepUnits );

    QString l_wxsWaypointRangeRingsColour;
    Read("WaypointRangeRingsColour", &l_wxsWaypointRangeRingsColour );
    g_colourWaypointRangeRingsColour.Set( l_wxsWaypointRangeRingsColour );

    if ( !Read( _T("WaypointUseScaMin"), &g_bUseWptScaMin ) ) g_bUseWptScaMin = false;
    if ( !Read( _T("WaypointScaMinValue"), &g_iWpt_ScaMin ) ) g_iWpt_ScaMin = 2147483646;
    if ( !Read( _T("WaypointUseScaMinOverrule"), &g_bOverruleScaMin ) ) g_bOverruleScaMin = false;
    if ( !Read( _T("WaypointsShowName"), &g_bShowWptName ) ) g_bShowWptName = true;



    //  Support Version 3.0 and prior config setting for Radar Rings
    bool b300RadarRings= true;
    if(Read ("ShowRadarRings", &b300RadarRings )){
        if(!b300RadarRings)
            g_iNavAidRadarRingsNumberVisible = 0;
    }

    Read("ConfirmObjectDeletion", &g_bConfirmObjectDelete );

    // Waypoint dragging with mouse
    g_bWayPointPreventDragging = false;
    Read("WaypointPreventDragging", &g_bWayPointPreventDragging );

    g_bEnableZoomToCursor = false;
    Read("EnableZoomToCursor", &g_bEnableZoomToCursor );

    val.Clear();
    Read("TrackIntervalSeconds", &val );
    if( val.Length() > 0 ) {
        double tval = atof( val.mb_str() );
        if( tval >= 2. ) g_TrackIntervalSeconds = tval;
    }

    val.Clear();
    Read("TrackDeltaDistance", &val );
    if( val.Length() > 0 ) {
        double tval = atof( val.mb_str() );
        if( tval >= 0.05 ) g_TrackDeltaDistance = tval;
    }

    Read("TrackPrecision", &g_nTrackPrecision );

    Read("NavObjectFileName", m_sNavObjSetFile );

    Read("RouteLineWidth", &g_route_line_width );
    Read("TrackLineWidth", &g_track_line_width );

    QString l_wxsTrackLineColour;
    if(Read("TrackLineColour", &l_wxsTrackLineColour ))
        g_colourTrackLineColour.Set( l_wxsTrackLineColour );

    Read("TideCurrentWindowScale", &g_tcwin_scale );
    Read("DefaultWPIcon", &g_default_wp_icon );
    Read("DefaultRPIcon", &g_default_routepoint_icon );

    SetPath("/MMSIProperties" ) );
    int iPMax = GetNumberOfEntries();
    if( iPMax ) {
        g_MMSI_Props_Array.Empty();
        QString str, val;
        long dummy;
        int iDir = 0;
        bool bCont = pConfig->GetFirstEntry( str, dummy );
        while( bCont ) {
            pConfig->Read( str, &val );       // Get an entry

            MMSIProperties *pProps = new MMSIProperties( val );
            g_MMSI_Props_Array.Add(pProps);

            bCont = pConfig->GetNextEntry( str, dummy );

        }
    }

    return ( 0 );
}

void zchxConfig::LoadS57Config()
{
    if( !mS52LibObj )  return;
    mS52LibObj->SetShowS57Text( !(getCustomValue("Settings/GlobalStat", "bShowS57Text", 0).toInt() == 0 ) );
    mS52LibObj->SetShowS57ImportantTextOnly( !(getCustomValue("Settings/GlobalStat", "bShowS57ImportantTextOnly", 0).toInt() == 0 ) );
    mS52LibObj->SetShowLdisText( !(getCustomValue("Settings/GlobalStat", "bShowLightDescription", 0).toInt() == 0 ) );
    mS52LibObj->SetExtendLightSectors( !(getCustomValue("Settings/GlobalStat", "bExtendLightSectors", 0).toInt() == 0 ));
    mS52LibObj->SetDisplayCategory((enum _DisCat) getCustomValue("Settings/GlobalStat", "nDisplayCategory", (enum _DisCat) STANDARD).toInt() );
    mS52LibObj->m_nSymbolStyle = (LUPname) getCustomValue("Settings/GlobalStat", "nSymbolStyle", (enum _LUPname) PAPER_CHART).toInt();
    mS52LibObj->m_nBoundaryStyle = (LUPname) getCustomValue("Settings/GlobalStat", "nBoundaryStyle", PLAIN_BOUNDARIES).toInt();
    mS52LibObj->m_bShowSoundg = !( getCustomValue("Settings/GlobalStat", "bShowSoundg", 1).toInt() == 0 );
    mS52LibObj->m_bShowMeta = !( getCustomValue("Settings/GlobalStat", "bShowMeta", 0).toInt() == 0 );
    mS52LibObj->m_bUseSCAMIN = !( getCustomValue("Settings/GlobalStat", "bUseSCAMIN", 1).toInt() == 0 );
    mS52LibObj->m_bShowAtonText = !( getCustomValue("Settings/GlobalStat", "bShowAtonText", 1).toInt() == 0 );
    mS52LibObj->m_bDeClutterText = !( getCustomValue("Settings/GlobalStat", "bDeClutterText", 0).toInt() == 0 );
    mS52LibObj->m_bShowNationalTexts = !( getCustomValue("Settings/GlobalStat", "bShowNationalText", 0).toInt() == 0 );

    double dval = getCustomValue("Settings/GlobalStat", "S52_MAR_SAFETY_CONTOUR", 5.0).toDouble();
    S52_setMarinerParam( S52_MAR_SAFETY_CONTOUR, dval );
    S52_setMarinerParam( S52_MAR_SAFETY_DEPTH, dval ); // Set safety_contour and safety_depth the same

    dval = getCustomValue("Settings/GlobalStat", "S52_MAR_SHALLOW_CONTOUR", 3.0).toDouble();
    S52_setMarinerParam( S52_MAR_SHALLOW_CONTOUR, dval );

    dval = getCustomValue("Settings/GlobalStat", "S52_MAR_DEEP_CONTOUR", 10.0).toDouble();
    S52_setMarinerParam(S52_MAR_DEEP_CONTOUR, dval );

    dval = getCustomValue("Settings/GlobalStat", "S52_MAR_TWO_SHADES", 0.0).toDouble();
    S52_setMarinerParam(S52_MAR_TWO_SHADES, dval );

    mS52LibObj->UpdateMarinerParams();

    int read_int = getCustomValue("Settings/GlobalState", "S52_DEPTH_UNIT_SHOW", 1).toInt();
    read_int = qMax(read_int, 0);        // qualify value
    read_int = qMin(read_int, 2);
    mS52LibObj->m_nDepthUnitDisplay = read_int;
    g_nDepthUnitDisplay = read_int;

    //    S57 Object Class Visibility

    OBJLElement *pOLE;
    QString section = "Settings/ObjectFilter";
    int iOBJMax = getChildCount(section);
    if( iOBJMax ) {
        QStringList keys = getChildKeys(section);
        foreach (QString key, keys) {
            long val = getCustomValue(section, key).toLongLong();
            bool bNeedNew = false;
            QString sObj;
            if(key.startsWith("viz"))
            {
                sObj = key.mid(3);
                for( unsigned int iPtr = 0; iPtr <  mS52LibObj->pOBJLArray->count(); iPtr++ ) {
                    pOLE = (OBJLElement *) (  mS52LibObj->pOBJLArray->at( iPtr ) );
                    if( !strncmp( pOLE->OBJLName, sObj.toUtf8().data(), 6 ) ) {
                        pOLE->nViz = val;
                        bNeedNew = false;
                        break;
                    }
                }
                if( bNeedNew ) {
                    pOLE = (OBJLElement *) calloc( sizeof(OBJLElement), 1 );
                    memcpy( pOLE->OBJLName, sObj.toUtf8().data(), OBJL_NAME_LEN );
                    pOLE->nViz = 1;
                    mS52LibObj->pOBJLArray->append((void *) pOLE );
                }
            }
        }
    }
}

#if 0
bool zchxConfig::LoadLayers(QString &path)
{
    QStringList file_array;
    wxDir dir;
    Layer *l;
    dir.Open( path );
    if( dir.IsOpened() ) {
        QString filename;
        bool cont = dir.GetFirst( &filename );
        while( cont ) {
            file_array.Clear();
            filename.Prepend( wxFileName::GetPathSeparator() );
            filename.Prepend( path );
            wxFileName f( filename );
            size_t nfiles = 0;
            if( f.GetExt().IsSameAs( wxT("gpx") ) )
                file_array.Add( filename); // single-gpx-file layer
            else{
                if(wxDir::Exists( filename ) ){
                    wxDir dir( filename );
                    if( dir.IsOpened() ){
                        nfiles = dir.GetAllFiles( filename, &file_array, wxT("*.gpx") );      // layers subdirectory set
                    }
                }
            }

            if( file_array.GetCount() ){
                l = new Layer();
                l->m_LayerID = ++g_LayerIdx;
                l->m_LayerFileName = file_array[0];
                if( file_array.GetCount() <= 1 )
                    wxFileName::SplitPath( file_array[0], NULL, NULL, &( l->m_LayerName, NULL, NULL );
                else
                    wxFileName::SplitPath( filename, NULL, NULL, &( l->m_LayerName, NULL, NULL );

                bool bLayerViz = g_bShowLayers;

                if( g_VisibleLayers.Contains( l->m_LayerName ) )
                    bLayerViz = true;
                if( g_InvisibleLayers.Contains( l->m_LayerName ) )
                    bLayerViz = false;

                l->m_bHasVisibleNames = wxCHK_UNDETERMINED;
                if (g_VisiNameinLayers.Contains(l->m_LayerName))
                    l->m_bHasVisibleNames = wxCHK_CHECKED;
                if (g_InVisiNameinLayers.Contains(l->m_LayerName))
                    l->m_bHasVisibleNames = wxCHK_UNCHECKED;

                l->m_bIsVisibleOnChart = bLayerViz;

                QString laymsg;
                laymsg.Printf( wxT("New layer %d: %s"), l->m_LayerID, l->m_LayerName.c_str() );
                ZCHX_LOGMSG( laymsg );

                pLayerList->Insert( l );

                //  Load the entire file array as a single layer

                for( unsigned int i = 0; i < file_array.GetCount(); i++ ) {
                    QString file_path = file_array[i];

                    if( ::wxFileExists( file_path ) ) {
                        NavObjectCollection1 *pSet = new NavObjectCollection1;
                        pSet->load_file(file_path.fn_str());
                        long nItems = pSet->LoadAllGPXObjectsAsLayer(l->m_LayerID, bLayerViz, l->m_bHasVisibleNames);
                        l->m_NoOfItems += nItems;
                        l->m_LayerType = _("Persistent");

                        QString objmsg;
                        objmsg.Printf( wxT("Loaded GPX file %s with %ld items."), file_path.c_str(), nItems );
                        ZCHX_LOGMSG( objmsg );

                        delete pSet;
                    }
                }
            }

            cont = dir.GetNext( &filename );
        }
    }
    g_bLayersLoaded = true;

    return true;
}

bool zchxConfig::LoadChartDirArray( ArrayOfCDI &ChartDirArray )
{
    //    Chart Directories
    SetPath("/ChartDirectories" ) );
    int iDirMax = GetNumberOfEntries();
    if( iDirMax ) {
        ChartDirArray.Empty();
        QString str, val;
        long dummy;
        int nAdjustChartDirs = 0;
        int iDir = 0;
        bool bCont = pConfig->GetFirstEntry( str, dummy );
        while( bCont ) {
            pConfig->Read( str, &val );       // Get a Directory name

            QString dirname( val );
            if( !dirname.IsEmpty() ) {

                /*     Special case for first time run after Windows install with sample chart data...
   We desire that the sample configuration file opencpn.ini should not contain any
   installation dependencies, so...
   Detect and update the sample [ChartDirectories] entries to point to the Shared Data directory
   For instance, if the (sample) opencpn.ini file should contain shortcut coded entries like:

   [ChartDirectories]
   ChartDir1=SampleCharts\\MaptechRegion7

   then this entry will be updated to be something like:
   ChartDir1=c:\Program Files\opencpn\SampleCharts\\MaptechRegion7

   */
                if( dirname.Find("SampleCharts" ) ) == 0 ) // only update entries starting with "SampleCharts"
                {
                    nAdjustChartDirs++;

                    pConfig->DeleteEntry( str );
                    QString new_dir = dirname.Mid( dirname.Find("SampleCharts" ) ) );
                    new_dir.Prepend( g_Platform->GetSharedDataDir() );
                    dirname = new_dir;
                }

                ChartDirInfo cdi;
                cdi.fullpath = dirname.BeforeFirst( '^' );
                cdi.magic_number = dirname.AfterFirst( '^' );

                ChartDirArray.Add( cdi );
                iDir++;
            }

            bCont = pConfig->GetNextEntry( str, dummy );
        }

        if( nAdjustChartDirs ) pConfig->UpdateChartDirs( ChartDirArray );
    }

    return true;
}





bool zchxConfig::UpdateChartDirs( ArrayOfCDI& dir_array )
{
    QString key, dir;
    QString str_buf;

    SetPath("/ChartDirectories" ) );
    int iDirMax = GetNumberOfEntries();
    if( iDirMax ) {

        long dummy;

        for( int i = 0; i < iDirMax; i++ ) {
            GetFirstEntry( key, dummy );
            DeleteEntry( key, false );
        }
    }

    iDirMax = dir_array.GetCount();

    for( int iDir = 0; iDir < iDirMax; iDir++ ) {
        ChartDirInfo cdi = dir_array[iDir];

        QString dirn = cdi.fullpath;
        dirn.Append( _T("^") );
        dirn.Append( cdi.magic_number );

        str_buf.Printf("ChartDir%d", iDir + 1 );

        Write( str_buf, dirn );

    }

    Flush();
    return true;
}

void zchxConfig::CreateConfigGroups( ChartGroupArray *pGroupArray )
{
    if( !pGroupArray ) return;

    SetPath("/Groups" ) );
    Write("GroupCount", (int) pGroupArray->GetCount() );

    for( unsigned int i = 0; i < pGroupArray->GetCount(); i++ ) {
        ChartGroup *pGroup = pGroupArray->Item( i );
        QString s;
        s.Printf( _T("Group%d"), i + 1 );
        s.Prepend("/Groups/" ) );
        SetPath( s );

        Write("GroupName", pGroup->m_group_name );
        Write("GroupItemCount", (int) pGroup->m_element_array.size() );

        for( unsigned int j = 0; j < pGroup->m_element_array.size(); j++ ) {
            QString sg;
            sg.Printf( _T("Group%d/Item%d"), i + 1, j );
            sg.Prepend("/Groups/" ) );
            SetPath( sg );
            Write("IncludeItem", pGroup->m_element_array[j]->m_element_name );

            QString t;
            QStringList u = pGroup->m_element_array[j]->m_missing_name_array;
            if( u.GetCount() ) {
                for( unsigned int k = 0; k < u.GetCount(); k++ ) {
                    t += u[k];
                    t += _T(";");
                }
                Write("ExcludeItems", t );
            }
        }
    }
}

void zchxConfig::DestroyConfigGroups( void )
{
    DeleteGroup("/Groups" ) );  //zap
}

void zchxConfig::LoadConfigGroups( ChartGroupArray *pGroupArray )
{
    SetPath("/Groups" ) );
    unsigned int group_count;
    Read("GroupCount", (int *) &group_count, 0 );

    for( unsigned int i = 0; i < group_count; i++ ) {
        ChartGroup *pGroup = new ChartGroup;
        QString s;
        s.Printf( _T("Group%d"), i + 1 );
        s.Prepend("/Groups/" ) );
        SetPath( s );

        QString t;
        Read("GroupName", &t );
        pGroup->m_group_name = t;

        unsigned int item_count;
        Read("GroupItemCount", (int *) &item_count );
        for( unsigned int j = 0; j < item_count; j++ ) {
            QString sg;
            sg.Printf( _T("Group%d/Item%d"), i + 1, j );
            sg.Prepend("/Groups/" ) );
            SetPath( sg );

            QString v;
            Read("IncludeItem", &v );
            ChartGroupElement *pelement = new ChartGroupElement{v};
            pGroup->m_element_array.emplace_back( pelement );

            QString u;
            if( Read("ExcludeItems", &u ) ) {
                if( !u.IsEmpty() ) {
                    QStringTokenizer tk( u, _T(";") );
                    while( tk.HasMoreTokens() ) {
                        QString token = tk.GetNextToken();
                        pelement->m_missing_name_array.Add( token );
                    }
                }
            }
        }
        pGroupArray->Add( pGroup );
    }

}

void zchxConfig::LoadCanvasConfigs( bool bApplyAsTemplate )
{
    int n_canvas;
    QString s;
    canvasConfig *pcc;

    SetPath("/Canvas" ) );

    //  If the canvas config has never been set/persisted, use the global settings
    if(!HasEntry("CanvasConfig" ))){

        pcc = new canvasConfig(0);
        pcc->LoadFromLegacyConfig( this );
        g_canvasConfigArray.Add(pcc);

        return;
    }

    Read("CanvasConfig", (int *)&g_canvasConfig, 0 );

    // Do not recreate canvasConfigs when applying config dynamically
    if(g_canvasConfigArray.GetCount() == 0){     // This is initial load from startup
        s.Printf( _T("/Canvas/CanvasConfig%d"), 1 );
        SetPath( s );
        canvasConfig *pcca = new canvasConfig(0);
        LoadConfigCanvas(pcca, bApplyAsTemplate);
        g_canvasConfigArray.Add(pcca);

        s.Printf( _T("/Canvas/CanvasConfig%d"), 2 );
        SetPath( s );
        pcca = new canvasConfig(1);
        LoadConfigCanvas(pcca, bApplyAsTemplate);
        g_canvasConfigArray.Add(pcca);
    } else {         // This is a dynamic (i.e. Template) load
        canvasConfig *pcca = g_canvasConfigArray[0];
        s.Printf( _T("/Canvas/CanvasConfig%d"), 1 );
        SetPath( s );
        LoadConfigCanvas(pcca, bApplyAsTemplate);

        if(g_canvasConfigArray.GetCount() > 1){
            canvasConfig *pcca = g_canvasConfigArray[1];
            s.Printf( _T("/Canvas/CanvasConfig%d"), 2 );
            SetPath( s );
            LoadConfigCanvas(pcca, bApplyAsTemplate);
        } else {
            s.Printf( _T("/Canvas/CanvasConfig%d"), 2 );
            SetPath( s );
            pcca = new canvasConfig(1);
            LoadConfigCanvas(pcca, bApplyAsTemplate);
            g_canvasConfigArray.Add(pcca);
        }
    }
}

void zchxConfig::LoadConfigCanvas( canvasConfig *cConfig, bool bApplyAsTemplate )
{
#if 0
    QString st;
    double st_lat, st_lon;

    if(!bApplyAsTemplate){
        //    Reasonable starting point
        cConfig->iLat = START_LAT;     // display viewpoint
        cConfig->iLon = START_LON;

        if( Read("canvasVPLatLon", &st ) ) {
            sscanf( st.mb_str( wxConvUTF8, "%lf,%lf", &st_lat, &st_lon );

            //    Sanity check the lat/lon...both have to be reasonable.
            if( fabs( st_lon ) < 360. ) {
                while( st_lon < -180. )
                    st_lon += 360.;

                while( st_lon > 180. )
                    st_lon -= 360.;

                cConfig->iLon = st_lon;
            }

            if( fabs( st_lat ) < 90.0 )
                cConfig->iLat = st_lat;
        }

        cConfig->iScale = .0003; // decent initial value
        cConfig->iRotation = 0;

        double st_view_scale;
        if( Read( QString("canvasVPScale" ), &st ) ) {
            sscanf( st.mb_str( wxConvUTF8, "%lf", &st_view_scale );
            //    Sanity check the scale
            st_view_scale = fmax ( st_view_scale, .001/32 );
            st_view_scale = fmin ( st_view_scale, 4 );
            cConfig->iScale = st_view_scale;
        }

        double st_rotation;
        if( Read( QString("canvasVPRotation" ), &st ) ) {
            sscanf( st.mb_str( wxConvUTF8, "%lf", &st_rotation );
            //    Sanity check the rotation
            st_rotation = fmin ( st_rotation, 360 );
            st_rotation = fmax ( st_rotation, 0 );
            cConfig->iRotation = st_rotation * PI / 180.;
        }

        Read("canvasInitialdBIndex", &cConfig->DBindex, 0 );
        Read("canvasbFollow", &cConfig->bFollow, 0 );

        Read("canvasCourseUp", &cConfig->bCourseUp, 0 );
        Read("canvasLookahead", &cConfig->bLookahead, 0 );
    }

    Read("ActiveChartGroup", &cConfig->GroupID, 0 );

    // Special check for group selection when applied as template
    if(cConfig->GroupID && bApplyAsTemplate){
        if( cConfig->GroupID > (int) g_pGroupArray->GetCount() )
            cConfig->GroupID = 0;
    }

    Read("canvasShowTides", &cConfig->bShowTides, 0 );
    Read("canvasShowCurrents", &cConfig->bShowCurrents, 0 );


    Read("canvasQuilt", &cConfig->bQuilt, 1 );
    Read("canvasShowGrid", &cConfig->bShowGrid, 0 );
    Read("canvasShowOutlines", &cConfig->bShowOutlines, 0 );
    Read("canvasShowDepthUnits", &cConfig->bShowDepthUnits, 0 );

    Read("canvasShowAIS", &cConfig->bShowAIS, 1 );
    Read("canvasAttenAIS", &cConfig->bAttenAIS, 0 );

    // ENC options
    Read("canvasShowENCText", &cConfig->bShowENCText, 1 );
    Read("canvasENCDisplayCategory", &cConfig->nENCDisplayCategory, STANDARD );
    Read("canvasENCShowDepths", &cConfig->bShowENCDepths, 1 );
    Read("canvasENCShowBuoyLabels", &cConfig->bShowENCBuoyLabels, 1 );
    Read("canvasENCShowLightDescriptions", &cConfig->bShowENCLightDescriptions, 1 );
    Read("canvasENCShowLights", &cConfig->bShowENCLights, 1 );


    int sx, sy;
    Read("canvasSizeX", &sx, 0 );
    Read("canvasSizeY", &sy, 0 );
    cConfig->canvasSize = wxSize(sx, sy);

#endif


}


void zchxConfig::SaveCanvasConfigs( )
{
#if 0
    SetPath("/Canvas" ) );
    Write("CanvasConfig", (int )g_canvasConfig );

    QString s;
    canvasConfig *pcc;

    switch( g_canvasConfig ){

    case 0:
    default:

        s.Printf( _T("/Canvas/CanvasConfig%d"), 1 );
        SetPath( s );

        if(g_canvasConfigArray.GetCount() > 0 ){
            pcc = g_canvasConfigArray.Item(0);
            if(pcc){
                SaveConfigCanvas(pcc);
            }
        }
        break;

    case 1:

        if(g_canvasConfigArray.GetCount() > 1 ){

            s.Printf( _T("/Canvas/CanvasConfig%d"), 1 );
            SetPath( s );
            pcc = g_canvasConfigArray.Item(0);
            if(pcc){
                SaveConfigCanvas(pcc);
            }

            s.Printf( _T("/Canvas/CanvasConfig%d"), 2 );
            SetPath( s );
            pcc = g_canvasConfigArray.Item(1);
            if(pcc){
                SaveConfigCanvas(pcc);
            }
        }
        break;

    }
#endif
}


void zchxConfig::SaveConfigCanvas( canvasConfig *cConfig )
{
#if 0
    QString st1;

    if(cConfig->canvas){
        ViewPort vp = cConfig->canvas->GetVP();

        if( vp.IsValid() ) {
            st1.Printf("%10.4f,%10.4f", vp.clat, vp.clon );
            Write("canvasVPLatLon", st1 );
            st1.Printf("%g", vp.view_scale_ppm );
            Write("canvasVPScale", st1 );
            st1.Printf("%i", ((int)(vp.rotation * 180 / PI)) % 360 );
            Write("canvasVPRotation", st1 );
        }

        int restore_dbindex = 0;
        ChartStack *pcs = cConfig->canvas->GetpCurrentStack();
        if(pcs)
            restore_dbindex = pcs->GetCurrentEntrydbIndex();
        if( cConfig->canvas->GetQuiltMode())
            restore_dbindex = cConfig->canvas->GetQuiltReferenceChartIndex();
        Write("canvasInitialdBIndex", restore_dbindex );

        Write("canvasbFollow", cConfig->canvas->m_bFollow );
        Write("ActiveChartGroup", cConfig->canvas->m_groupIndex );

        Write("canvasToolbarConfig", cConfig->canvas->GetToolbarConfigString() );
        Write("canvasShowToolbar", 0 );  //cConfig->canvas->GetToolbarEnable() );

        Write("canvasQuilt", cConfig->canvas->GetQuiltMode() );
        Write("canvasShowGrid", cConfig->canvas->GetShowGrid() );
        Write("canvasShowOutlines", cConfig->canvas->GetShowOutlines() );
        Write("canvasShowDepthUnits", cConfig->canvas->GetShowDepthUnits() );

        Write("canvasShowAIS", cConfig->canvas->GetShowAIS() );
        Write("canvasAttenAIS", cConfig->canvas->GetAttenAIS() );

        Write("canvasShowTides", cConfig->canvas->GetbShowTide() );
        Write("canvasShowCurrents", cConfig->canvas->GetbShowCurrent() );

        // ENC options
        Write("canvasShowENCText", cConfig->canvas->GetShowENCText() );
        Write("canvasENCDisplayCategory", cConfig->canvas->GetENCDisplayCategory() );
        Write("canvasENCShowDepths", cConfig->canvas->GetShowENCDepth() );
        Write("canvasENCShowBuoyLabels", cConfig->canvas->GetShowENCBuoyLabels() );
        Write("canvasENCShowLightDescriptions", cConfig->canvas->GetShowENCLightDesc() );
        Write("canvasENCShowLights", cConfig->canvas->GetShowENCLights() );

        Write("canvasCourseUp", cConfig->canvas->GetCourseUP() );
        Write("canvasLookahead", cConfig->canvas->GetLookahead() );


        int width = cConfig->canvas->GetSize().x;
        //  if(cConfig->canvas->IsPrimaryCanvas()){
        //      width = qMax(width, gFrame->GetClientSize().x / 10);
        //  }
        //  else{
        //      width = qMin(width, gFrame->GetClientSize().x  * 9 / 10);
        //  }

        Write("canvasSizeX", width );
        Write("canvasSizeY", cConfig->canvas->GetSize().y );

    }
#endif
}



void zchxConfig::UpdateSettings()
{
#if 0
    //  Temporarily suppress logging of trivial non-fatal wxLogSysError() messages provoked by Android security...
#ifdef __OCPN__ANDROID__
    wxLogNull logNo;
#endif


    //    Global options and settings
    SetPath("/Settings" ) );

    Write("LastAppliedTemplate", g_lastAppliedTemplateGUID );

    Write("ConfigVersionString", g_config_version_string );
#ifdef SYSTEM_SOUND_CMD
    if ( wxIsEmpty( g_CmdSoundString ) )
        g_CmdSoundString = QString( SYSTEM_SOUND_CMD );
    Write("CmdSoundString", g_CmdSoundString );
#endif /* SYSTEM_SOUND_CMD */
    Write("NavMessageShown", n_NavMessageShown );
    Write("InlandEcdis", g_bInlandEcdis );

    Write("DarkDecorations"), g_bDarkDecorations );

    Write("UIexpert", g_bUIexpert );
    Write("SpaceDropMark", g_bSpaceDropMark );
    //    Write("UIStyle", g_StyleManager->GetStyleNextInvocation() );      //Not desired for O5 MUI

    Write("ShowStatusBar", g_bShowStatusBar );
#ifndef __WXOSX__
    Write("ShowMenuBar", g_bShowMenuBar );
#endif
    Write("DefaultFontSize", g_default_font_size );

    Write("Fullscreen", g_bFullscreen );
    Write("ShowCompassWindow", g_bShowCompassWin );
    Write("SetSystemTime", s_bSetSystemTime );
    Write("ShowGrid", g_bDisplayGrid );
    Write("PlayShipsBells", g_bPlayShipsBells );
    Write("SoundDeviceIndex", g_iSoundDeviceIndex );
    Write("FullscreenToolbar", g_bFullscreenToolbar );
    Write("TransparentToolbar", g_bTransparentToolbar );
    Write("PermanentMOBIcon", g_bPermanentMOBIcon );
    Write("ShowLayers", g_bShowLayers );
    Write("AutoAnchorDrop", g_bAutoAnchorMark );
    Write("ShowChartOutlines", g_bShowOutlines );
    Write("ShowActiveRouteTotal", g_bShowRouteTotal );
    Write("ShowActiveRouteHighway", g_bShowActiveRouteHighway );
    Write("SDMMFormat", g_iSDMMFormat );
    Write("MostRecentGPSUploadConnection", g_uploadConnection );
    Write("ShowChartBar", g_bShowChartBar );

    Write("GUIScaleFactor", g_GUIScaleFactor );
    Write("ChartObjectScaleFactor", g_ChartScaleFactor );
    Write("ShipScaleFactor", g_ShipScaleFactor );

    Write("FilterNMEA_Avg", g_bfilter_cogsog );
    Write("FilterNMEA_Sec", g_COGFilterSec );

    Write("ShowTrue", g_bShowTrue );
    Write("ShowMag", g_bShowMag );
    Write("UserMagVariation", QString::Format( _T("%.2f"), g_UserVar ) );

    Write("CM93DetailFactor", g_cm93_zoom_factor );
    Write("CM93DetailZoomPosX", g_detailslider_dialog_x );
    Write("CM93DetailZoomPosY", g_detailslider_dialog_y );
    Write("ShowCM93DetailSlider", g_bShowDetailSlider );

    Write("SkewToNorthUp", g_bskew_comp );
    Write("OpenGL", g_bopengl );
    Write("SoftwareGL", g_bSoftwareGL );
    Write("ShowFPS", g_bShowFPS );

    Write("ZoomDetailFactor", g_chart_zoom_modifier );
    Write("ZoomDetailFactorVector", g_chart_zoom_modifier_vector );

    Write("FogOnOverzoom", g_fog_overzoom );
    Write("OverzoomVectorScale", g_oz_vector_scale );
    Write("OverzoomEmphasisBase", g_overzoom_emphasis_base );

#ifdef ocpnUSE_GL
    /* opengl options */
    Write("UseAcceleratedPanning", g_GLOptions.m_bUseAcceleratedPanning );

    Write("GPUTextureCompression", g_GLOptions.m_bTextureCompression);
    Write("GPUTextureCompressionCaching", g_GLOptions.m_bTextureCompressionCaching);
    Write("GPUTextureDimension", g_GLOptions.m_iTextureDimension );
    Write("GPUTextureMemSize", g_GLOptions.m_iTextureMemorySize );
    Write("PolygonSmoothing", g_GLOptions.m_GLPolygonSmoothing);
    Write("LineSmoothing", g_GLOptions.m_GLLineSmoothing);
#endif
    Write("SmoothPanZoom", g_bsmoothpanzoom );

    Write("CourseUpMode", g_bCourseUp );
    if (!g_bInlandEcdis ) Write("LookAheadMode", g_bLookAhead );
    Write("COGUPAvgSeconds", g_COGAvgSec );
    Write("UseMagAPB", g_bMagneticAPB );

    Write("OwnshipCOGPredictorMinutes", g_ownship_predictor_minutes );
    Write("OwnshipCOGPredictorWidth", g_cog_predictor_width );
    Write("OwnshipHDTPredictorMiles", g_ownship_HDTpredictor_miles );
    Write("OwnShipIconType", g_OwnShipIconType );
    Write("OwnShipLength", g_n_ownship_length_meters );
    Write("OwnShipWidth", g_n_ownship_beam_meters );
    Write("OwnShipGPSOffsetX", g_n_gps_antenna_offset_x );
    Write("OwnShipGPSOffsetY", g_n_gps_antenna_offset_y );
    Write("OwnShipMinSize", g_n_ownship_min_mm );
    Write("OwnShipSogCogCalc", g_own_ship_sog_cog_calc );
    Write("OwnShipSogCogCalcDampSec"), g_own_ship_sog_cog_calc_damp_sec );

    QString racr;
    //   racr.Printf("%g", g_n_arrival_circle_radius );
    //   Write("RouteArrivalCircleRadius", racr );
    Write("RouteArrivalCircleRadius", QString::Format( _T("%.2f"), g_n_arrival_circle_radius ));

    Write("ChartQuilting", g_bQuiltEnable );

    Write("NMEALogWindowSizeX", NMEALogWindow::Get().GetSizeW());
    Write("NMEALogWindowSizeY", NMEALogWindow::Get().GetSizeH());
    Write("NMEALogWindowPosX", NMEALogWindow::Get().GetPosX());
    Write("NMEALogWindowPosY", NMEALogWindow::Get().GetPosY());

    Write("PreserveScaleOnX", g_bPreserveScaleOnX );

    Write("StartWithTrackActive", g_bTrackCarryOver );
    Write("AutomaticDailyTracks", g_bTrackDaily );
    Write("TrackRotateAt", g_track_rotate_time );
    Write("TrackRotateTimeType", g_track_rotate_time_type );
    Write("HighlightTracks", g_bHighliteTracks );

    Write("InitialStackIndex", g_restore_stackindex );
    Write("InitialdBIndex", g_restore_dbindex );
    Write("ActiveChartGroup", g_GroupIndex );

    Write("NMEAAPBPrecision", g_NMEAAPBPrecision );

    Write( _T("TalkerIdText"), g_TalkerIdText );

    Write("AnchorWatch1GUID", g_AW1GUID );
    Write("AnchorWatch2GUID", g_AW2GUID );

    //Write("ToolbarX", g_maintoolbar_x );
    //Write("ToolbarY", g_maintoolbar_y );
    //Write("ToolbarOrient", g_maintoolbar_orient );

    Write("iENCToolbarX", g_iENCToolbarPosX );
    Write("iENCToolbarY", g_iENCToolbarPosY );

    if ( !g_bInlandEcdis ){
        Write("GlobalToolbarConfig", g_toolbarConfig );
        Write("DistanceFormat", g_iDistanceFormat );
        Write("SpeedFormat", g_iSpeedFormat );
        Write("ShowDepthUnits", g_bShowDepthUnits );
    }
    Write("GPSIdent", g_GPS_Ident );
    Write("UseGarminHostUpload", g_bGarminHostUpload );

    Write("MobileTouch", g_btouch );
    Write("ResponsiveGraphics", g_bresponsive );

    Write("AutoHideToolbar", g_bAutoHideToolbar );
    Write("AutoHideToolbarSecs", g_nAutoHideToolbar );

    Write("DisplaySizeMM", g_config_display_size_mm );
    Write("DisplaySizeManual", g_config_display_size_manual );

    Write("SelectionRadiusMM", g_selection_radius_mm );
    Write("SelectionRadiusTouchMM", g_selection_radius_touch_mm );

    QString st0;
    st0.Printf("%g", g_PlanSpeed );
    Write("PlanSpeed", st0 );

    if(g_bLayersLoaded){
        QString vis, invis, visnames, invisnames;
        LayerList::iterator it;
        int index = 0;
        for( it = ( *pLayerList ).begin(); it != ( *pLayerList ).end(); ++it, ++index ) {
            Layer *lay = (Layer *) ( *it );
            if( lay->IsVisibleOnChart() ) vis += ( lay->m_LayerName ) + _T(";");
            else
                invis += ( lay->m_LayerName ) + _T(";");

            if( lay->HasVisibleNames() == wxCHK_CHECKED ) {
                visnames += ( lay->m_LayerName) + _T(";");
            } else if( lay->HasVisibleNames() == wxCHK_UNCHECKED ) {
                invisnames += ( lay->m_LayerName) + _T(";");
            }
        }
        Write("VisibleLayers", vis );
        Write("InvisibleLayers", invis );
        Write("VisNameInLayers", visnames);
        Write("InvisNameInLayers", invisnames);
    }
    Write("Locale", g_locale );
    Write("LocaleOverride", g_localeOverride );

    Write("KeepNavobjBackups", g_navobjbackups );
    Write("LegacyInputCOMPortFilterBehaviour", g_b_legacy_input_filter_behaviour );
    Write("AdvanceRouteWaypointOnArrivalOnly", g_bAdvanceRouteWaypointOnArrivalOnly);

    // LIVE ETA OPTION
    Write("LiveETA", g_bShowLiveETA);
    Write("DefaultBoatSpeed", g_defaultBoatSpeed);

    //    S57 Object Filter Settings

    SetPath("/Settings/ObjectFilter" ) );

    if(  mS52LibObj ) {
        for( unsigned int iPtr = 0; iPtr <  mS52LibObj->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) (  mS52LibObj->pOBJLArray->Item( iPtr ) );

            QString st1("viz" ) );
            char name[7];
            strncpy( name, pOLE->OBJLName, 6 );
            name[6] = 0;
            st1.Append( QString( name, wxConvUTF8 ) );
            Write( st1, pOLE->nViz );
        }
    }

    //    Global State

    SetPath("/Settings/GlobalState" ) );

    QString st1;

    //     if( cc1 ) {
    //  ViewPort vp = cc1->GetVP();
    //
    //  if( vp.IsValid() ) {
    //      st1.Printf("%10.4f,%10.4f", vp.clat, vp.clon );
    //      Write("VPLatLon", st1 );
    //      st1.Printf("%g", vp.view_scale_ppm );
    //      Write("VPScale", st1 );
    //      st1.Printf("%i", ((int)(vp.rotation * 180 / PI)) % 360 );
    //      Write("VPRotation", st1 );
    //  }
    //     }

    st1.Printf("%10.4f, %10.4f", gLat, gLon );
    Write("OwnShipLatLon", st1 );

    //    Various Options
    SetPath("/Settings/GlobalState" ) );
    if ( !g_bInlandEcdis ) Write("nColorScheme", (int) gFrame->GetColorScheme() );

    Write("FrameWinX", g_nframewin_x );
    Write("FrameWinY", g_nframewin_y );
    Write("FrameWinPosX", g_nframewin_posx );
    Write("FrameWinPosY", g_nframewin_posy );
    Write("FrameMax", g_bframemax );

    Write("ClientPosX", g_lastClientRectx );
    Write("ClientPosY", g_lastClientRecty );
    Write("ClientSzX", g_lastClientRectw );
    Write("ClientSzY", g_lastClientRecth );

    Write("S52_DEPTH_UNIT_SHOW", g_nDepthUnitDisplay );

    Write("RoutePropSizeX", g_route_prop_sx );
    Write("RoutePropSizeY", g_route_prop_sy );
    Write("RoutePropPosX", g_route_prop_x );
    Write("RoutePropPosY", g_route_prop_y );

    //    AIS
    SetPath("/Settings/AIS" ) );

    Write("bNoCPAMax", g_bCPAMax );
    Write("NoCPAMaxNMi", g_CPAMax_NM );
    Write("bCPAWarn", g_bCPAWarn );
    Write("CPAWarnNMi", g_CPAWarn_NM );
    Write("bTCPAMax", g_bTCPA_Max );
    Write("TCPAMaxMinutes", g_TCPA_Max );
    Write("bMarkLostTargets", g_bMarkLost );
    Write("MarkLost_Minutes", g_MarkLost_Mins );
    Write("bRemoveLostTargets", g_bRemoveLost );
    Write("RemoveLost_Minutes", g_RemoveLost_Mins );
    Write("bShowCOGArrows", g_bShowCOG );
    Write("CogArrowMinutes", g_ShowCOG_Mins );
    Write("bShowTargetTracks", g_bAISShowTracks );
    Write("TargetTracksMinutes", g_AISShowTracks_Mins );

    Write("bHideMooredTargets", g_bHideMoored );
    Write("MooredTargetMaxSpeedKnots", g_ShowMoored_Kts );

    Write("bAISAlertDialog", g_bAIS_CPA_Alert );
    Write("bAISAlertAudio", g_bAIS_CPA_Alert_Audio );
    Write("AISAlertAudioFile", g_sAIS_Alert_Sound_File );
    Write("bAISAlertSuppressMoored", g_bAIS_CPA_Alert_Suppress_Moored );
    Write("bShowAreaNotices", g_bShowAreaNotices );
    Write("bDrawAISSize", g_bDrawAISSize );
    Write("bShowAISName", g_bShowAISName );
    Write("ShowAISTargetNameScale", g_Show_Target_Name_Scale );
    Write("bWplIsAprsPositionReport", g_bWplIsAprsPosition );
    Write("AISCOGPredictorWidth", g_ais_cog_predictor_width );
    Write("bShowScaledTargets", g_bAllowShowScaled );
    Write("AISScaledNumber", g_ShowScaled_Num );
    Write("AISScaledNumberWeightSOG", g_ScaledNumWeightSOG );
    Write("AISScaledNumberWeightCPA", g_ScaledNumWeightCPA );
    Write("AISScaledNumberWeightTCPA", g_ScaledNumWeightTCPA );
    Write("AISScaledNumberWeightRange", g_ScaledNumWeightRange );
    Write("AISScaledNumberWeightSizeOfTarget", g_ScaledNumWeightSizeOfT );
    Write("AISScaledSizeMinimal", g_ScaledSizeMinimal );
    Write("AISShowScaled"), g_bShowScaled);

    Write("AlertDialogSizeX", g_ais_alert_dialog_sx );
    Write("AlertDialogSizeY", g_ais_alert_dialog_sy );
    Write("AlertDialogPosX", g_ais_alert_dialog_x );
    Write("AlertDialogPosY", g_ais_alert_dialog_y );
    Write("QueryDialogPosX", g_ais_query_dialog_x );
    Write("QueryDialogPosY", g_ais_query_dialog_y );
    Write("AISTargetListPerspective", g_AisTargetList_perspective );
    Write("AISTargetListRange", g_AisTargetList_range );
    Write("AISTargetListSortColumn", g_AisTargetList_sortColumn );
    Write("bAISTargetListSortReverse", g_bAisTargetList_sortReverse );
    Write("AISTargetListColumnSpec", g_AisTargetList_column_spec );
    Write("AISTargetListColumnOrder"), g_AisTargetList_column_order);

    Write("S57QueryDialogSizeX", g_S57_dialog_sx );
    Write("S57QueryDialogSizeY", g_S57_dialog_sy );
    Write("S57QueryExtraDialogSizeX", g_S57_extradialog_sx );
    Write("S57QueryExtraDialogSizeY", g_S57_extradialog_sy );

    Write("bAISRolloverShowClass", g_bAISRolloverShowClass );
    Write("bAISRolloverShowCOG", g_bAISRolloverShowCOG );
    Write("bAISRolloverShowCPA", g_bAISRolloverShowCPA );

    Write("bAISAlertAckTimeout", g_bAIS_ACK_Timeout );
    Write("AlertAckTimeoutMinutes", g_AckTimeout_Mins );

    SetPath("/Settings/GlobalState" ) );
    if(  mS52LibObj ) {
        Write("bShowS57Text",  mS52LibObj->GetShowS57Text() );
        Write("bShowS57ImportantTextOnly",  mS52LibObj->GetShowS57ImportantTextOnly() );
        if ( !g_bInlandEcdis ) Write("nDisplayCategory", (long)  mS52LibObj->GetDisplayCategory() );
        Write("nSymbolStyle", (int)  mS52LibObj->m_nSymbolStyle );
        Write("nBoundaryStyle", (int)  mS52LibObj->m_nBoundaryStyle );

        Write("bShowSoundg",  mS52LibObj->m_bShowSoundg );
        Write("bShowMeta",  mS52LibObj->m_bShowMeta );
        Write("bUseSCAMIN",  mS52LibObj->m_bUseSCAMIN );
        Write("bShowAtonText",  mS52LibObj->m_bShowAtonText );
        Write("bShowLightDescription",  mS52LibObj->m_bShowLdisText );
        Write("bExtendLightSectors",  mS52LibObj->m_bExtendLightSectors );
        Write("bDeClutterText",  mS52LibObj->m_bDeClutterText );
        Write("bShowNationalText",  mS52LibObj->m_bShowNationalTexts );

        Write("S52_MAR_SAFETY_CONTOUR", S52_getMarinerParam( S52_MAR_SAFETY_CONTOUR ) );
        Write("S52_MAR_SHALLOW_CONTOUR", S52_getMarinerParam( S52_MAR_SHALLOW_CONTOUR ) );
        Write("S52_MAR_DEEP_CONTOUR", S52_getMarinerParam( S52_MAR_DEEP_CONTOUR ) );
        Write("S52_MAR_TWO_SHADES", S52_getMarinerParam( S52_MAR_TWO_SHADES ) );
        Write("S52_DEPTH_UNIT_SHOW",  mS52LibObj->m_nDepthUnitDisplay );
    }
    SetPath("/Directories" ) );
    Write("S57DataLocation", _T("") );
    //    Write("SENCFileLocation", _T("") );

    SetPath("/Directories" ) );
    Write("InitChartDir", *pInit_Chart_Dir );
    Write("GPXIODir", g_gpx_path );
    Write("TCDataDir", g_TCData_Dir );
    Write("BasemapDir", g_Platform->NormalizePath(gWorldMapLocation) );

    SetPath("/Settings/NMEADataSource" ) );
    QString connectionconfigs;
    for (size_t i = 0; i < g_pConnectionParams->Count(); i++)
    {
        if (i > 0)
            connectionconfigs.Append(_T("|"));
        connectionconfigs.Append(g_pConnectionParams->Item(i)->Serialize());
    }
    Write ("DataConnections", connectionconfigs );

    //    Fonts

    //  Store the persistent Auxiliary Font descriptor Keys
    SetPath("/Settings/AuxFontKeys" ) );

    QStringList keyArray = FontMgr::Get().GetAuxKeyArray();
    for(unsigned int i=0 ; i <  keyArray.GetCount() ; i++){
        QString key;
        key.Printf(_T("Key%i"), i);
        QString keyval = keyArray[i];
        Write( key, keyval );
    }

    QString font_path;
#ifdef __WXX11__
    font_path = ("/Settings/X11Fonts" ) );
#endif

#ifdef __WXGTK__
    font_path = ("/Settings/GTKFonts" ) );
#endif

#ifdef __WXMSW__
    font_path = ("/Settings/MSWFonts" ) );
#endif

#ifdef __WXMAC__
    font_path = ("/Settings/MacFonts" ) );
#endif

#ifdef __WXQT__
    font_path = ("/Settings/QTFonts" ) );
#endif

    DeleteGroup(font_path);

    SetPath( font_path );

    int nFonts = FontMgr::Get().GetNumFonts();

    for( int i = 0; i < nFonts; i++ ) {
        QString cfstring(FontMgr::Get().GetConfigString(i));
        QString valstring = FontMgr::Get().GetFullConfigDesc( i );
        Write( cfstring, valstring );
    }

    //  Tide/Current Data Sources
    DeleteGroup("/TideCurrentDataSources" ) );
    SetPath("/TideCurrentDataSources" ) );
    unsigned int iDirMax = TideCurrentDataSet.Count();
    for( unsigned int id = 0 ; id < iDirMax ; id++ ) {
        QString key;
        key.Printf(_T("tcds%d"), id);
        Write( key, TideCurrentDataSet[id] );
    }

    SetPath("/Settings/Others" ) );

    // Radar rings
    Write("ShowRadarRings", (bool)(g_iNavAidRadarRingsNumberVisible > 0) );  //3.0.0 config support
    Write("RadarRingsNumberVisible", g_iNavAidRadarRingsNumberVisible );
    Write("RadarRingsStep", g_fNavAidRadarRingsStep );
    Write("RadarRingsStepUnits", g_pNavAidRadarRingsStepUnits );
    Write("RadarRingsColour", g_colourOwnshipRangeRingsColour.GetAsString( wxC2S_HTML_SYNTAX ) );
    Write("WaypointUseScaMin", g_bUseWptScaMin );
    Write("WaypointScaMinValue", g_iWpt_ScaMin );
    Write("WaypointUseScaMinOverrule", g_bOverruleScaMin );
    Write( _T("WaypointsShowName"), g_bShowWptName );

    // Waypoint Radar rings
    Write("WaypointRangeRingsNumber", g_iWaypointRangeRingsNumber );
    Write("WaypointRangeRingsStep", g_fWaypointRangeRingsStep );
    Write("WaypointRangeRingsStepUnits", g_iWaypointRangeRingsStepUnits );
    Write("WaypointRangeRingsColour", g_colourWaypointRangeRingsColour.GetAsString( wxC2S_HTML_SYNTAX ) );

    Write("ConfirmObjectDeletion", g_bConfirmObjectDelete );

    // Waypoint dragging with mouse; toh, 2009.02.24
    Write("WaypointPreventDragging", g_bWayPointPreventDragging );

    Write("EnableZoomToCursor", g_bEnableZoomToCursor );

    Write("TrackIntervalSeconds", g_TrackIntervalSeconds );
    Write("TrackDeltaDistance", g_TrackDeltaDistance );
    Write("TrackPrecision", g_nTrackPrecision );

    Write("RouteLineWidth", g_route_line_width );
    Write("TrackLineWidth", g_track_line_width );
    Write("TrackLineColour", g_colourTrackLineColour.GetAsString( wxC2S_HTML_SYNTAX ) );
    Write("DefaultWPIcon", g_default_wp_icon );
    Write("DefaultRPIcon", g_default_routepoint_icon );

    DeleteGroup(_T ( "/MMSIProperties" ));
    SetPath("/MMSIProperties" ) );
    for(unsigned int i=0 ; i < g_MMSI_Props_Array.GetCount() ; i++){
        QString p;
        p.Printf(_T("Props%d"), i);
        Write( p, g_MMSI_Props_Array[i]->Serialize() );
    }

    SaveCanvasConfigs();

    Flush();
#endif
}
#endif


void zchxConfig::setShowStatusBar(bool sts)
{
    mShowChartBar = sts;
    setCustomValue(COMMON_SEC, SHOW_STATUS_BAR, sts);
}

void zchxConfig::setShowMenuBar(bool sts)
{
    mShowMenuBar = sts;
    setCustomValue(COMMON_SEC, SHOW_MENU_BAR, sts);
}

void zchxConfig::setShowCompassWin(bool sts)
{
    mShowCompassWin = sts;
    setCustomValue(COMMON_SEC, SHOW_COMPASS_WINDOW, sts);
}

void zchxConfig::setShowChartBar(bool sts)
{
    mShowChartBar = sts;
    setCustomValue(COMMON_SEC, SHOW_CHART_BAR, sts);
}

void zchxConfig::setDiplaySizeMM(double val)
{
    mDisplaySizeMM = val;
    //    setCustomValue(COMMON_SEC, DISPLAY_SIZE_MM, val);
}

void zchxConfig::setConfigDisplaySizeMM(double sts)
{
    mConfigDisplaySizeMM = sts;
    setCustomValue(COMMON_SEC, DISPLAY_SIZE_MM, sts);
}

void zchxConfig::setConfigDisplaySizeManual(bool sts)
{
    mConfigDisplaySizeManual = sts;
    setCustomValue(COMMON_SEC, DISPLAY_SIZE_MANUAL, sts);
}

void zchxConfig::setSkewComp(bool sts)
{
    mSkewComp = sts;
    setCustomValue(COMMON_SEC, SKEW_TO_NORTH_UP, sts);
}

void zchxConfig::setResponsive(bool sts)
{
    mResponsive = sts;
    setCustomValue(COMMON_SEC, RESPONSIVE_GRAPHICS, sts);
}

void zchxConfig::setAutoHideToolBar(bool sts)
{
    mAutoHideToolbar = sts;
    setCustomValue(COMMON_SEC, AUTO_HIDE_TOOLBAR, sts);
}

void zchxConfig::setAutoHideToolBarSecs(int sts)
{
    mAutoHideToolbarSecs = sts;
    setCustomValue(COMMON_SEC, AUTO_HIDE_TOOLBAR_SECS, sts);
}

void zchxConfig::setSmoothPanZoom(bool sts)
{
    mSmoothPanZoom = sts;
    setCustomValue(COMMON_SEC, SMOOTH_PAN_ZOOM, sts);
}

void zchxConfig::setCogAvgSec(int sts)
{
    mCOGAvgSec = sts;
    setCustomValue(COMMON_SEC, COG_UP_AVG_SECONDS, sts);
}

void zchxConfig::setShowTrue(bool sts)
{
    mShowTrue = sts;
    setCustomValue(COMMON_SEC, SHOW_TRUE, sts);
}

void zchxConfig::setShowMag(bool sts)
{
    mShowMag = sts;
    setCustomValue(COMMON_SEC, SHOW_MAG, sts);
}

void zchxConfig::setSDMMFormat(int sts)
{
    mSDMMFormat = sts;
    setCustomValue(COMMON_SEC, SDMM_FORMAT, sts);
}

void zchxConfig::setDistanceFormat(int sts)
{
    mDistanceFormat = sts;
    setCustomValue(COMMON_SEC, DISTANCE_FORMAT, sts);
}

void zchxConfig::setSpeedFormat(int sts)
{
    mSpeedFormat = sts;
    setCustomValue(COMMON_SEC, SPEED_FORMAT, sts);
}

void zchxConfig::setEnableZoomToCursor(bool sts)
{
    mEnableZoomToCursor = sts;
    setCustomValue(COMMON_SEC, ENABLE_ZOOM_TO_CURSOR, sts);
}

void zchxConfig::setChartZoomModifier(int sts)
{
    mChartZoomModifier = sts;
    setCustomValue(COMMON_SEC, ZOOM_DETAIL_FACTOR, sts);
}

void zchxConfig::setChartZoomModifierVector(int sts)
{
    mChartZoomModifierVector = sts;
    setCustomValue(COMMON_SEC, ZOOM_DETAIL_FACTOR_VECTOR, sts);
}

void zchxConfig::setGUIScaleFactor(int sts)
{
    mGUIScaleFactor = sts;
    setCustomValue(COMMON_SEC, GUI_SCALE_FACTOR, sts);
}

void zchxConfig::setChartScaleFactor(int sts)
{
    mChartScaleFactor = sts;
    setCustomValue(COMMON_SEC, CHART_OBJECT_SCALE_FACTOR, sts);
}

void zchxConfig::setShipScaleFactor(int sts)
{
    mShipScaleFactor = sts;
    setCustomValue(COMMON_SEC, SHIP_SCALE_FACTOR, sts);
}

void zchxConfig::setChartScaleFactorExp(float sts)
{
    mChartScaleFactorExp = sts;
}

void zchxConfig::setShipScaleFactorExp(float sts)
{
    mShipScaleFactorExp = sts;
}

void zchxConfig::setOpenGL(bool sts)
{
    mOpenGL = sts;
    setCustomValue(COMMON_SEC, OPEN_GL, sts);
}

void zchxConfig:: setCm93ZoomFactor(int sts)
{
    mCm93ZoomFactor = sts;
    setCustomValue(COMMON_SEC, CM93_DETAIL_FACTOR, sts);
}

void zchxConfig::setDepthUnitDisplay(int sts)
{
    mDepthUnitDisplay = sts;
    setCustomValue(COMMON_SEC, S52_DEPTH_UNIT_SHOW_S, sts);
}

void zchxConfig::setGLLineSmoothing(bool sts)
{
    mGLOptions.m_GLLineSmoothing = sts;
    setCustomValue(COMMON_SEC, LINE_SMOOTHING, sts);
}

void  zchxConfig::setGLPolygonSmoothing(bool sts)
{
    mGLOptions.m_GLPolygonSmoothing = sts;
    setCustomValue(COMMON_SEC, POLYGON_SMOOTHING, sts);
}

void zchxConfig::setGLUseAcceleratedPanning(bool sts)
{
    mGLOptions.m_bUseAcceleratedPanning = sts;
    setCustomValue(COMMON_SEC, USE_ACCELERATED_PANNING, sts);
}

void zchxConfig::setGLTextureCompression(bool sts)
{
    mGLOptions.m_bTextureCompression = sts;
    setCustomValue(COMMON_SEC, GPU_TEXTURE_COMPRESSION, sts);
}

void zchxConfig::setGLTextureCompressionCaching(bool sts)
{
    mGLOptions.m_bTextureCompressionCaching = sts;
    setCustomValue(COMMON_SEC, GPU_TEXTURE_COMPRESSION_CACHING, sts);
}

void zchxConfig::setGLTextureDimension(int val)
{
    mGLOptions.m_iTextureDimension = val;
    setCustomValue(COMMON_SEC, GPU_TEXTURE_DEMENSION, val);
}
void zchxConfig::setGLTextureMemorySize(int val)
{
    mGLOptions.m_iTextureMemorySize = val;
    setCustomValue(COMMON_SEC, GPU_TEXTURE_MEMSIZE, val);
}

void zchxConfig::setGLExpert(bool sts)
{
    mGLExpert = sts;
    setCustomValue(COMMON_SEC, OPENGL_EXPERT, sts);
}

void zchxConfig::setShowFPS(bool sts)
{
    mShowFPS = sts;
    setCustomValue(COMMON_SEC, SHOW_FPS, sts);
}

void zchxConfig::setSoftwareGL(bool sts)
{
    mSoftwareGL = sts;
    setCustomValue(COMMON_SEC, SOFTWARE_GL, sts);
}

zchxConfig::~zchxConfig()
{
    qDebug()<<"now destruct here";
}






