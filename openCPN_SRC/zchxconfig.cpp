#include "zchxconfig.h"
#include <QTextCodec>
#include "GL/gl.h"

//全局定义,方便使用
zchxGLOptions                       g_GLOptions;
bool                                g_bGlExpert;
bool                                g_bOpenGL;
GLuint                              g_raster_format;
bool                                g_bShowFPS;

zchxConfig::zchxConfig( const QString &LocalFileName ) : QSettings(LocalFileName)
{
    this->setIniCodec(QTextCodec::codecForName("UTF8"));
    m_bSkipChangeSetUpdate = false;
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

void zchxConfig::initDefaultValue()
{
    setDefault(COMMON_SEC, USE_MODERN_UI5, true);
    setDefault(COMMON_SEC, TALKER_ID_TEXT, "EC");
    setDefault(COMMON_SEC, NMEA_APB_PRECISION, 3);
    setDefault(COMMON_SEC, USE_OPEN_GL, true);
    setDefault(COMMON_SEC, USE_ACCELERATED_PANNING, true);
    setDefault(COMMON_SEC, POLYGON_SMOOTHING, true);
    setDefault(COMMON_SEC, LINE_SMOOTHING, true);
    setDefault(COMMON_SEC, GPU_TEXTURE_DEMENSION, 512);
    setDefault(COMMON_SEC, GPU_TEXTURE_MEMSIZE, 128);
    setDefault(COMMON_SEC, OPENGL_EXPERT, false);
    setDefault(COMMON_SEC, GPU_TEXTURE_COMPRESSION, true);
    setDefault(COMMON_SEC, GPU_TEXTURE_COMPRESSION_CACHING, true);
    setDefault(COMMON_SEC, RASTER_FORMAT, GL_RGB);



    g_bGlExpert = getCustomValue(COMMON_SEC, OPENGL_EXPERT, false).toBool();
    g_bOpenGL = getCustomValue(COMMON_SEC, USE_OPEN_GL, true).toBool();
    g_raster_format = (GLuint)(getCustomValue(COMMON_SEC, RASTER_FORMAT, GL_RGB).toUInt());
    g_bShowFPS = getCustomValue(COMMON_SEC, SHOW_FPS, false).toBool();

}

#if 0
int zchxConfig::LoadMyConfig()
{
    //设定默认的设置

    int display_width, display_height;
    wxDisplaySize( &display_width, &display_height );

    g_GLOptions.m_iTextureDimension = 512;
    g_GLOptions.m_iTextureMemorySize = 128;
    if(!g_bGLexpert){
        g_GLOptions.m_iTextureMemorySize = wxMax(128, g_GLOptions.m_iTextureMemorySize);
        g_GLOptions.m_bTextureCompressionCaching = g_GLOptions.m_bTextureCompression;
    }

    g_maintoolbar_orient = wxTB_HORIZONTAL;
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
    g_track_rotate_time_type =  TIME_TYPE_COMPUTER;
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
    g_colourOwnshipRangeRingsColour = *wxRED;
    g_iWaypointRangeRingsNumber = 0;
    g_fWaypointRangeRingsStep = 1.0;
    g_iWaypointRangeRingsStepUnits = 0;
    g_colourWaypointRangeRingsColour = wxColour( *wxRED );
    g_bConfirmObjectDelete = true;

    g_TrackIntervalSeconds = 60.0;
    g_TrackDeltaDistance = 0.10;
    g_route_line_width = 2;
    g_track_line_width = 2;
    g_colourTrackLineColour = wxColour( 243, 229, 47 );

    g_tcwin_scale = 100;
    g_default_wp_icon = _T("triangle");
    g_default_routepoint_icon = _T("diamond");

    g_nAWDefault = 50;
    g_nAWMax = 1852;

    // Load the raw value, with no defaults, and no processing
    int ret_Val = LoadMyConfigRaw();

    //  Perform any required post processing and validation
    if(!ret_Val){
        g_ChartScaleFactorExp = g_Platform->getChartScaleFactorExp( g_ChartScaleFactor );
        g_ShipScaleFactorExp = g_Platform->getChartScaleFactorExp( g_ShipScaleFactor );

        g_COGFilterSec = wxMin(g_COGFilterSec, MAX_COGSOG_FILTER_SECONDS);
        g_COGFilterSec = wxMax(g_COGFilterSec, 1);
        g_SOGFilterSec = g_COGFilterSec;

        if(!g_bShowTrue && !g_bShowMag)
            g_bShowTrue = true;
        g_COGAvgSec = wxMin(g_COGAvgSec, MAX_COG_AVERAGE_SECONDS);        // Bound the array size

        if( g_bInlandEcdis )
            g_bLookAhead=1;

        if ( g_bdisable_opengl )
            g_bopengl = false;

        #ifdef ocpnUSE_GL
            if(!g_bGLexpert){
            g_GLOptions.m_iTextureMemorySize = wxMax(128, g_GLOptions.m_iTextureMemorySize);
            g_GLOptions.m_bTextureCompressionCaching = g_GLOptions.m_bTextureCompression;
        }
        #endif

        g_chart_zoom_modifier = wxMin(g_chart_zoom_modifier,5);
        g_chart_zoom_modifier = wxMax(g_chart_zoom_modifier,-5);
        g_chart_zoom_modifier_vector = wxMin(g_chart_zoom_modifier_vector,5);
        g_chart_zoom_modifier_vector = wxMax(g_chart_zoom_modifier_vector,-5);
        g_cm93_zoom_factor = wxMin(g_cm93_zoom_factor,CM93_ZOOM_FACTOR_MAX_RANGE);
        g_cm93_zoom_factor = wxMax(g_cm93_zoom_factor,(-CM93_ZOOM_FACTOR_MAX_RANGE));

        if( ( g_detailslider_dialog_x < 0 ) || ( g_detailslider_dialog_x > display_width ) )
            g_detailslider_dialog_x = 5;
        if( ( g_detailslider_dialog_y < 0 ) || ( g_detailslider_dialog_y > display_height ) )
            g_detailslider_dialog_y =  5;

        g_defaultBoatSpeedUserUnit = toUsrSpeed(g_defaultBoatSpeed, -1);
        g_n_ownship_min_mm = wxMax(g_n_ownship_min_mm, 1);
        if( g_navobjbackups > 99 ) g_navobjbackups = 99;
        if( g_navobjbackups < 0 ) g_navobjbackups = 0;
        g_n_arrival_circle_radius = wxClip(g_n_arrival_circle_radius, 0.001, 0.6);

        g_selection_radius_mm = wxMax(g_selection_radius_mm, 0.5);
        g_selection_radius_touch_mm = wxMax(g_selection_radius_touch_mm, 1.0);

        g_Show_Target_Name_Scale = wxMax( 5000, g_Show_Target_Name_Scale );

        if( ( g_ais_alert_dialog_x < 0 ) || ( g_ais_alert_dialog_x > display_width ) )
            g_ais_alert_dialog_x = 5;
        if( ( g_ais_alert_dialog_y < 0 ) || ( g_ais_alert_dialog_y > display_height ) )
            g_ais_alert_dialog_y = 5;
        if( ( g_ais_query_dialog_x < 0 ) || ( g_ais_query_dialog_x > display_width ) )
            g_ais_query_dialog_x = 5;
        if( ( g_ais_query_dialog_y < 0 ) || ( g_ais_query_dialog_y > display_height ) )
            g_ais_query_dialog_y = 5;

        SwitchInlandEcdisMode( g_bInlandEcdis );
        if ( g_bInlandEcdis )
            global_color_scheme = GLOBAL_COLOR_SCHEME_DUSK; //startup in duskmode if inlandEcdis

        //    Multicanvas Settings
        LoadCanvasConfigs();
    }

    return ret_Val;
}

int zchxConfig::LoadMyConfigRaw( bool bAsTemplate )
{

    int read_int;
    wxString val;

    int display_width, display_height;
    wxDisplaySize( &display_width, &display_height );

//    Global options and settings
    SetPath( _T ( "/Settings" ) );

    Read( _T ( "LastAppliedTemplate" ), &g_lastAppliedTemplateGUID );

    // Some undocumented values
    Read( _T ( "ConfigVersionString" ), &g_config_version_string );
#ifdef SYSTEM_SOUND_CMD
    Read(_T("CmdSoundString"), &g_CmdSoundString, wxString(SYSTEM_SOUND_CMD) );
    if ( wxIsEmpty( g_CmdSoundString ) )
        g_CmdSoundString = wxString( SYSTEM_SOUND_CMD );
#endif /* SYSTEM_SOUND_CMD */
    Read( _T ( "NavMessageShown" ), &n_NavMessageShown );

    Read( _T ( "UIexpert" ), &g_bUIexpert );

    Read( _T ( "UIStyle" ), &g_uiStyle  );

    Read( _T ( "NCacheLimit" ), &g_nCacheLimit );

    Read( _T ( "InlandEcdis" ), &g_bInlandEcdis );// First read if in iENC mode as this will override some config settings

    Read( _T ("DarkDecorations" ), &g_bDarkDecorations );

    Read( _T( "SpaceDropMark" ), &g_bSpaceDropMark );

    int mem_limit = 0;
    Read( _T ( "MEMCacheLimit" ), &mem_limit );
    if(mem_limit > 0)
        g_memCacheLimit = mem_limit * 1024;       // convert from MBytes to kBytes

    Read( _T ( "UseModernUI5" ), &g_useMUI );

    Read( _T( "NCPUCount" ), &g_nCPUCount);

    Read( _T ( "DebugGDAL" ), &g_bGDAL_Debug );
    Read( _T ( "DebugNMEA" ), &g_nNMEADebug );
    Read( _T ( "AnchorWatchDefault" ), &g_nAWDefault );
    Read( _T ( "AnchorWatchMax" ), &g_nAWMax );
    Read( _T ( "GPSDogTimeout" ), &gps_watchdog_timeout_ticks );
    Read( _T ( "DebugCM93" ), &g_bDebugCM93 );
    Read( _T ( "DebugS57" ), &g_bDebugS57 );         // Show LUP and Feature info in object query
    Read( _T ( "DebugBSBImg" ), &g_BSBImgDebug );
    Read( _T ( "DebugGPSD" ), &g_bDebugGPSD );

    Read( _T ( "DefaultFontSize"), &g_default_font_size );

    Read( _T ( "UseGreenShipIcon" ), &g_bUseGreenShip );


    Read( _T ( "AutoHideToolbar" ), &g_bAutoHideToolbar );
    Read( _T ( "AutoHideToolbarSecs" ), &g_nAutoHideToolbar );

    Read( _T ( "UseSimplifiedScalebar" ), &g_bsimplifiedScalebar );
    Read( _T ( "ShowTide" ), &g_bShowTide );
    Read( _T ( "ShowCurrent" ), &g_bShowCurrent );

    int size_mm = -1;
    Read( _T ( "DisplaySizeMM" ), &size_mm );

    Read( _T ( "SelectionRadiusMM" ), &g_selection_radius_mm);
    Read( _T ( "SelectionRadiusTouchMM" ), &g_selection_radius_touch_mm);

    if(!bAsTemplate){
        if(size_mm > 0){
            g_config_display_size_mm = size_mm;
            if((size_mm > 100) && (size_mm < 2000)){
                g_display_size_mm = size_mm;
            }
        }
        Read( _T ( "DisplaySizeManual" ), &g_config_display_size_manual );
    }

    Read( _T ( "GUIScaleFactor" ), &g_GUIScaleFactor );

    Read( _T ( "ChartObjectScaleFactor" ), &g_ChartScaleFactor );
    Read( _T ( "ShipScaleFactor" ), &g_ShipScaleFactor );


    //  NMEA connection options.
    if( !bAsTemplate ){
        Read( _T ( "FilterNMEA_Avg" ), &g_bfilter_cogsog );
        Read( _T ( "FilterNMEA_Sec" ), &g_COGFilterSec );
        Read( _T ( "GPSIdent" ), &g_GPS_Ident );
        Read( _T ( "UseGarminHostUpload" ),  &g_bGarminHostUpload );
        Read( _T ( "UseNMEA_GLL" ), &g_bUseGLL );
        Read( _T ( "UseMagAPB" ), &g_bMagneticAPB );
    }

    Read( _T ( "ShowTrue" ), &g_bShowTrue );
    Read( _T ( "ShowMag" ), &g_bShowMag );

    wxString umv;
    Read( _T ( "UserMagVariation" ), &umv );
    if(umv.Len()) umv.ToDouble( &g_UserVar );

    Read( _T ( "ScreenBrightness" ), &g_nbrightness );

    Read( _T ( "MemFootprintTargetMB" ), &g_MemFootMB );

    Read( _T ( "WindowsComPortMax" ), &g_nCOMPortCheck );

    Read( _T ( "ChartQuilting" ), &g_bQuiltEnable );
    Read( _T ( "ChartQuiltingInitial" ), &g_bQuiltStart );

    Read( _T ( "CourseUpMode" ), &g_bCourseUp );
    Read( _T ( "COGUPAvgSeconds" ), &g_COGAvgSec );
    Read( _T ( "LookAheadMode" ), &g_bLookAhead );
    Read( _T ( "SkewToNorthUp" ), &g_bskew_comp );

    Read( _T ( "ShowFPS" ), &g_bShowFPS );

    Read( _T ( "ActiveChartGroup" ), &g_GroupIndex );

    Read( _T( "NMEAAPBPrecision" ), &g_NMEAAPBPrecision );

    Read( _T( "TalkerIdText" ), &g_TalkerIdText );
    Read( _T( "MaxWaypointNameLength" ), &g_maxWPNameLength );

    /* opengl options */
#ifdef ocpnUSE_GL
    if(!bAsTemplate ){
        Read( _T ( "OpenGLExpert" ), &g_bGLexpert, false );
        Read( _T ( "UseAcceleratedPanning" ), &g_GLOptions.m_bUseAcceleratedPanning, true );
        Read( _T ( "GPUTextureCompression" ), &g_GLOptions.m_bTextureCompression);
        Read( _T ( "GPUTextureCompressionCaching" ), &g_GLOptions.m_bTextureCompressionCaching);
        Read( _T ( "PolygonSmoothing" ), &g_GLOptions.m_GLPolygonSmoothing);
        Read( _T ( "LineSmoothing" ), &g_GLOptions.m_GLLineSmoothing);
        Read( _T ( "GPUTextureDimension" ), &g_GLOptions.m_iTextureDimension );
        Read( _T ( "GPUTextureMemSize" ), &g_GLOptions.m_iTextureMemorySize );
        Read( _T ( "DebugOpenGL" ), &g_bDebugOGL );
        Read( _T ( "OpenGL" ), &g_bopengl );
        Read( _T ( "SoftwareGL" ), &g_bSoftwareGL );
    }
#endif

    Read( _T ( "SmoothPanZoom" ), &g_bsmoothpanzoom );

    Read( _T ( "ToolbarX"), &g_maintoolbar_x );
    Read( _T ( "ToolbarY" ), &g_maintoolbar_y );
    Read( _T ( "ToolbarOrient" ), &g_maintoolbar_orient );
    Read( _T ( "GlobalToolbarConfig" ), &g_toolbarConfig );

    Read( _T ( "iENCToolbarX"), &g_iENCToolbarPosX );
    Read( _T ( "iENCToolbarY"), &g_iENCToolbarPosY );

    Read( _T ( "AnchorWatch1GUID" ), &g_AW1GUID );
    Read( _T ( "AnchorWatch2GUID" ), &g_AW2GUID );

    Read( _T ( "InitialStackIndex" ), &g_restore_stackindex );
    Read( _T ( "InitialdBIndex" ), &g_restore_dbindex );

    Read( _T ( "ChartNotRenderScaleFactor" ), &g_ChartNotRenderScaleFactor );

    Read( _T ( "MobileTouch" ), &g_btouch );
    Read( _T ( "ResponsiveGraphics" ), &g_bresponsive );

    Read( _T ( "ZoomDetailFactor" ), &g_chart_zoom_modifier );
    Read( _T ( "ZoomDetailFactorVector" ), &g_chart_zoom_modifier_vector );

    Read( _T ( "CM93DetailFactor" ), &g_cm93_zoom_factor );

    Read( _T ( "CM93DetailZoomPosX" ), &g_detailslider_dialog_x );
    Read( _T ( "CM93DetailZoomPosY" ), &g_detailslider_dialog_y );
    Read( _T ( "ShowCM93DetailSlider" ), &g_bShowDetailSlider );

    Read( _T ( "SENC_LOD_Pixels" ), &g_SENC_LOD_pixels );

    Read( _T ( "SkewCompUpdatePeriod" ), &g_SkewCompUpdatePeriod );

    Read( _T ( "SetSystemTime" ), &s_bSetSystemTime );
    Read( _T ( "ShowStatusBar" ), &g_bShowStatusBar );
#ifndef __WXOSX__
    Read( _T ( "ShowMenuBar" ), &g_bShowMenuBar );
#endif
    Read( _T ( "Fullscreen" ), &g_bFullscreen );
    Read( _T ( "ShowCompassWindow" ), &g_bShowCompassWin );
    Read( _T ( "ShowGrid" ), &g_bDisplayGrid );
    Read( _T ( "PlayShipsBells" ), &g_bPlayShipsBells );
    Read( _T ( "SoundDeviceIndex" ), &g_iSoundDeviceIndex );
    Read( _T ( "FullscreenToolbar" ), &g_bFullscreenToolbar );
    //Read( _T ( "TransparentToolbar" ), &g_bTransparentToolbar );
    Read( _T ( "PermanentMOBIcon" ), &g_bPermanentMOBIcon );
    Read( _T ( "ShowLayers" ), &g_bShowLayers );
    Read( _T ( "ShowDepthUnits" ), &g_bShowDepthUnits );
    Read( _T ( "AutoAnchorDrop" ), &g_bAutoAnchorMark );
    Read( _T ( "ShowChartOutlines" ), &g_bShowOutlines );
    Read( _T ( "ShowActiveRouteHighway" ), &g_bShowActiveRouteHighway );
    Read( _T ( "ShowActiveRouteTotal" ), &g_bShowRouteTotal );
    Read( _T ( "MostRecentGPSUploadConnection" ), &g_uploadConnection);
    Read( _T ( "ShowChartBar" ), &g_bShowChartBar );
    Read( _T ( "SDMMFormat" ), &g_iSDMMFormat ); //0 = "Degrees, Decimal minutes"), 1 = "Decimal degrees", 2 = "Degrees,Minutes, Seconds"

    Read( _T ( "DistanceFormat" ), &g_iDistanceFormat ); //0 = "Nautical miles"), 1 = "Statute miles", 2 = "Kilometers", 3 = "Meters"
    Read( _T ( "SpeedFormat" ), &g_iSpeedFormat ); //0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"

    // LIVE ETA OPTION
    Read( _T ( "LiveETA" ), &g_bShowLiveETA );
    Read( _T ( "DefaultBoatSpeed" ), &g_defaultBoatSpeed );

    Read( _T ( "OwnshipCOGPredictorMinutes" ), &g_ownship_predictor_minutes );
    Read( _T ( "OwnshipCOGPredictorWidth" ), &g_cog_predictor_width );
    Read( _T ( "OwnshipHDTPredictorMiles" ), &g_ownship_HDTpredictor_miles );

    Read( _T ( "OwnShipIconType" ), &g_OwnShipIconType );
    Read( _T ( "OwnShipLength" ), &g_n_ownship_length_meters );
    Read( _T ( "OwnShipWidth" ), &g_n_ownship_beam_meters );
    Read( _T ( "OwnShipGPSOffsetX" ), &g_n_gps_antenna_offset_x );
    Read( _T ( "OwnShipGPSOffsetY" ), &g_n_gps_antenna_offset_y );
    Read( _T ( "OwnShipMinSize" ), &g_n_ownship_min_mm );
    Read( _T ( "OwnShipSogCogCalc" ), &g_own_ship_sog_cog_calc );
    Read( _T ( "OwnShipSogCogCalcDampSec"), &g_own_ship_sog_cog_calc_damp_sec );

    wxString racr;
    Read( _T ( "RouteArrivalCircleRadius" ), &racr );
    if(racr.Len())
        racr.ToDouble( &g_n_arrival_circle_radius);

    Read( _T ( "FullScreenQuilt" ), &g_bFullScreenQuilt );

    Read( _T ( "StartWithTrackActive" ), &g_bTrackCarryOver );
    Read( _T ( "AutomaticDailyTracks" ), &g_bTrackDaily );
    Read( _T ( "TrackRotateAt" ), &g_track_rotate_time );
    Read( _T ( "TrackRotateTimeType" ), &g_track_rotate_time_type );
    Read( _T ( "HighlightTracks" ), &g_bHighliteTracks );

    wxString stps;
    Read( _T ( "PlanSpeed" ), &stps );
    if(!stps.IsEmpty())
        stps.ToDouble( &g_PlanSpeed );

    Read( _T ( "VisibleLayers" ), &g_VisibleLayers );
    Read( _T ( "InvisibleLayers" ), &g_InvisibleLayers );
    Read( _T ( "VisNameInLayers" ), &g_VisiNameinLayers );
    Read( _T ( "InvisNameInLayers" ), &g_InVisiNameinLayers );

    Read( _T ( "PreserveScaleOnX" ), &g_bPreserveScaleOnX );

    Read( _T ( "Locale" ), &g_locale );
    Read( _T ( "LocaleOverride" ), &g_localeOverride );

    //We allow 0-99 backups ov navobj.xml
    Read( _T ( "KeepNavobjBackups" ), &g_navobjbackups );

    NMEALogWindow::Get().SetSize(Read(_T("NMEALogWindowSizeX"), 600L), Read(_T("NMEALogWindowSizeY"), 400L));
    NMEALogWindow::Get().SetPos(Read(_T("NMEALogWindowPosX"), 10L), Read(_T("NMEALogWindowPosY"), 10L));
    NMEALogWindow::Get().CheckPos(display_width, display_height);

    // Boolean to cater for legacy Input COM Port filer behaviour, i.e. show msg filtered but put msg on bus.
    Read( _T ( "LegacyInputCOMPortFilterBehaviour" ), &g_b_legacy_input_filter_behaviour );

    // Boolean to cater for sailing when not approaching waypoint
    Read( _T( "AdvanceRouteWaypointOnArrivalOnly" ), &g_bAdvanceRouteWaypointOnArrivalOnly);

    Read( _T ( "EnableRotateKeys" ),  &g_benable_rotate );
    Read( _T ( "EmailCrashReport" ),  &g_bEmailCrashReport );

    g_benableAISNameCache = true;
    Read( _T ( "EnableAISNameCache" ),  &g_benableAISNameCache );

    Read( _T ( "EnableUDPNullHeader" ),  &g_benableUDPNullHeader );

    SetPath( _T ( "/Settings/GlobalState" ) );

    Read( _T ( "FrameWinX" ), &g_nframewin_x );
    Read( _T ( "FrameWinY" ), &g_nframewin_y );
    Read( _T ( "FrameWinPosX" ), &g_nframewin_posx );
    Read( _T ( "FrameWinPosY" ), &g_nframewin_posy );
    Read( _T ( "FrameMax" ), &g_bframemax );

    Read( _T ( "ClientPosX" ), &g_lastClientRectx );
    Read( _T ( "ClientPosY" ), &g_lastClientRecty );
    Read( _T ( "ClientSzX" ), &g_lastClientRectw );
    Read( _T ( "ClientSzY" ), &g_lastClientRecth );

    Read( _T( "RoutePropSizeX" ), &g_route_prop_sx );
    Read( _T( "RoutePropSizeY" ), &g_route_prop_sy );
    Read( _T( "RoutePropPosX" ), &g_route_prop_x );
    Read( _T( "RoutePropPosY" ), &g_route_prop_y );

    read_int = -1;
    Read( _T ( "S52_DEPTH_UNIT_SHOW" ), &read_int );   // default is metres
    if(read_int >= 0){
        read_int = wxMax(read_int, 0);                      // qualify value
        read_int = wxMin(read_int, 2);
        g_nDepthUnitDisplay = read_int;
    }

    //    AIS
    wxString s;
    SetPath( _T ( "/Settings/AIS" ) );

    g_bUseOnlyConfirmedAISName = false;
    Read( _T ( "UseOnlyConfirmedAISName" ),  &g_bUseOnlyConfirmedAISName );

    Read( _T ( "bNoCPAMax" ), &g_bCPAMax );

    Read( _T ( "NoCPAMaxNMi" ), &s );
    s.ToDouble( &g_CPAMax_NM );

    Read( _T ( "bCPAWarn" ), &g_bCPAWarn );

    Read( _T ( "CPAWarnNMi" ), &s );
    s.ToDouble( &g_CPAWarn_NM );

    Read( _T ( "bTCPAMax" ), &g_bTCPA_Max );

    Read( _T ( "TCPAMaxMinutes" ), &s );
    s.ToDouble( &g_TCPA_Max );

    Read( _T ( "bMarkLostTargets" ), &g_bMarkLost );

    Read( _T ( "MarkLost_Minutes" ), &s );
    s.ToDouble( &g_MarkLost_Mins );

    Read( _T ( "bRemoveLostTargets" ), &g_bRemoveLost );

    Read( _T ( "RemoveLost_Minutes" ), &s );
    s.ToDouble( &g_RemoveLost_Mins );

    Read( _T ( "bShowCOGArrows" ), &g_bShowCOG );

    Read( _T ( "CogArrowMinutes" ), &s );
    s.ToDouble( &g_ShowCOG_Mins );

    Read( _T ( "bShowTargetTracks" ), &g_bAISShowTracks );


    if( Read( _T ( "TargetTracksLimit" ), &s ) ) {
        s.ToDouble( &g_AISShowTracks_Limit );
        g_AISShowTracks_Limit = wxMax(300.0, g_AISShowTracks_Limit);
    }
    if( Read( _T ( "TargetTracksMinutes" ), &s ) ) {
        s.ToDouble( &g_AISShowTracks_Mins );
        g_AISShowTracks_Mins = wxMax(1.0, g_AISShowTracks_Mins);
        g_AISShowTracks_Mins = wxMin(g_AISShowTracks_Limit, g_AISShowTracks_Mins);
    }

    Read( _T ( "bHideMooredTargets" ), &g_bHideMoored );
    if(Read( _T ( "MooredTargetMaxSpeedKnots" ), &s ))
        s.ToDouble( &g_ShowMoored_Kts );

    Read(_T ("bShowScaledTargets"), &g_bAllowShowScaled );
    Read( _T ( "AISScaledNumber" ), &g_ShowScaled_Num );
    Read( _T ( "AISScaledNumberWeightSOG" ), &g_ScaledNumWeightSOG );
    Read( _T ( "AISScaledNumberWeightCPA" ), &g_ScaledNumWeightCPA );
    Read( _T ( "AISScaledNumberWeightTCPA" ), &g_ScaledNumWeightTCPA );
    Read( _T ( "AISScaledNumberWeightRange" ),& g_ScaledNumWeightRange );
    Read( _T ( "AISScaledNumberWeightSizeOfTarget" ), &g_ScaledNumWeightSizeOfT );
    Read( _T ( "AISScaledSizeMinimal" ), &g_ScaledSizeMinimal );
    Read(_T("AISShowScaled"), &g_bShowScaled );

    Read( _T ( "bShowAreaNotices" ), &g_bShowAreaNotices );
    Read( _T ( "bDrawAISSize" ), &g_bDrawAISSize );
    Read( _T ( "bShowAISName" ), &g_bShowAISName );
    Read( _T ( "bAISAlertDialog" ), &g_bAIS_CPA_Alert );
    Read( _T ( "ShowAISTargetNameScale" ), &g_Show_Target_Name_Scale );
    Read( _T ( "bWplIsAprsPositionReport" ), &g_bWplIsAprsPosition );
    Read( _T ( "AISCOGPredictorWidth" ), &g_ais_cog_predictor_width );

    Read( _T ( "bAISAlertAudio" ), &g_bAIS_CPA_Alert_Audio );
    Read( _T ( "AISAlertAudioFile" ), &g_sAIS_Alert_Sound_File );
    Read( _T ( "bAISAlertSuppressMoored" ), &g_bAIS_CPA_Alert_Suppress_Moored );

    Read( _T ( "bAISAlertAckTimeout" ), &g_bAIS_ACK_Timeout );
    if(Read( _T ( "AlertAckTimeoutMinutes" ), &s ))
        s.ToDouble( &g_AckTimeout_Mins );

    Read( _T ( "AlertDialogSizeX" ), &g_ais_alert_dialog_sx );
    Read( _T ( "AlertDialogSizeY" ), &g_ais_alert_dialog_sy );
    Read( _T ( "AlertDialogPosX" ), &g_ais_alert_dialog_x );
    Read( _T ( "AlertDialogPosY" ), &g_ais_alert_dialog_y );
    Read( _T ( "QueryDialogPosX" ), &g_ais_query_dialog_x );
    Read( _T ( "QueryDialogPosY" ), &g_ais_query_dialog_y );

    Read( _T ( "AISTargetListPerspective" ), &g_AisTargetList_perspective );
    Read( _T ( "AISTargetListRange" ), &g_AisTargetList_range );
    Read( _T ( "AISTargetListSortColumn" ), &g_AisTargetList_sortColumn );
    Read( _T ( "bAISTargetListSortReverse" ), &g_bAisTargetList_sortReverse );
    Read( _T ( "AISTargetListColumnSpec" ), &g_AisTargetList_column_spec );
    Read( _T ("AISTargetListColumnOrder"), &g_AisTargetList_column_order);

    Read( _T ( "bAISRolloverShowClass" ), &g_bAISRolloverShowClass );
    Read( _T ( "bAISRolloverShowCOG" ), &g_bAISRolloverShowCOG );
    Read( _T ( "bAISRolloverShowCPA" ), &g_bAISRolloverShowCPA );

    Read( _T ( "S57QueryDialogSizeX" ), &g_S57_dialog_sx );
    Read( _T ( "S57QueryDialogSizeY" ), &g_S57_dialog_sy );
    Read( _T ( "S57QueryExtraDialogSizeX" ), &g_S57_extradialog_sx );
    Read( _T ( "S57QueryExtraDialogSizeY" ), &g_S57_extradialog_sy );


    wxString strpres( _T ( "PresentationLibraryData" ) );
    wxString valpres;
    SetPath( _T ( "/Directories" ) );
    Read( strpres, &valpres );              // Get the File name
    if(!valpres.IsEmpty())
        g_UserPresLibData = valpres;

    wxString strs( _T ( "SENCFileLocation" ) );
    SetPath( _T ( "/Directories" ) );
    wxString vals;
    Read( strs, &vals );              // Get the Directory name
    if(!vals.IsEmpty())
        g_SENCPrefix = vals;

    SetPath( _T ( "/Directories" ) );
    wxString vald;
    Read( _T ( "InitChartDir" ), &vald );           // Get the Directory name

    wxString dirnamed( vald );
    if( !dirnamed.IsEmpty() ) {
        if( pInit_Chart_Dir->IsEmpty() )   // on second pass, don't overwrite
        {
            pInit_Chart_Dir->Clear();
            pInit_Chart_Dir->Append( vald );
        }
    }

    Read( _T ( "GPXIODir" ), &g_gpx_path );           // Get the Directory name
    Read( _T ( "TCDataDir" ), &g_TCData_Dir );           // Get the Directory name
    Read( _T ( "BasemapDir"), &gWorldMapLocation );

    SetPath( _T ( "/Settings/GlobalState" ) );

    if(Read( _T ( "nColorScheme" ), &read_int ))
        global_color_scheme = (ColorScheme) read_int;

    if(! bAsTemplate ){
        SetPath( _T ( "/Settings/NMEADataSource" ) );

        wxString connectionconfigs;
        Read ( _T( "DataConnections" ),  &connectionconfigs );
        if(!connectionconfigs.IsEmpty()){
            wxArrayString confs = wxStringTokenize(connectionconfigs, _T("|"));
            g_pConnectionParams->Clear();
            for (size_t i = 0; i < confs.Count(); i++)
            {
                ConnectionParams * prm = new ConnectionParams(confs[i]);
                if (!prm->Valid) {
                    ZCHX_LOGMSG( _T( "Skipped invalid DataStream config") );
                    delete prm;
                    continue;
                }
                g_pConnectionParams->Add(prm);
            }
        }
    }



    SetPath( _T ( "/Settings/GlobalState" ) );
    wxString st;

    double st_lat, st_lon;
    if( Read( _T ( "VPLatLon" ), &st ) ) {
        sscanf( st.mb_str( wxConvUTF8 ), "%lf,%lf", &st_lat, &st_lon );

        //    Sanity check the lat/lon...both have to be reasonable.
        if( fabs( st_lon ) < 360. ) {
            while( st_lon < -180. )
                st_lon += 360.;

            while( st_lon > 180. )
                st_lon -= 360.;

            vLon = st_lon;
        }

        if( fabs( st_lat ) < 90.0 ) vLat = st_lat;

        s.Printf( _T ( "Setting Viewpoint Lat/Lon %g, %g" ), vLat, vLon );
        ZCHX_LOGMSG( s );

    }

    double st_view_scale, st_rotation;
    if( Read( wxString( _T ( "VPScale" ) ), &st ) ) {
        sscanf( st.mb_str( wxConvUTF8 ), "%lf", &st_view_scale );
//    Sanity check the scale
        st_view_scale = fmax ( st_view_scale, .001/32 );
        st_view_scale = fmin ( st_view_scale, 4 );
        initial_scale_ppm = st_view_scale;
    }

    if( Read( wxString( _T ( "VPRotation" ) ), &st ) ) {
        sscanf( st.mb_str( wxConvUTF8 ), "%lf", &st_rotation );
//    Sanity check the rotation
        st_rotation = fmin ( st_rotation, 360 );
        st_rotation = fmax ( st_rotation, 0 );
        initial_rotation = st_rotation * PI / 180.;
    }

    wxString sll;
    double lat, lon;
    if( Read( _T ( "OwnShipLatLon" ), &sll ) ) {
        sscanf( sll.mb_str( wxConvUTF8 ), "%lf,%lf", &lat, &lon );

        //    Sanity check the lat/lon...both have to be reasonable.
        if( fabs( lon ) < 360. ) {
            while( lon < -180. )
                lon += 360.;

            while( lon > 180. )
                lon -= 360.;

            gLon = lon;
        }

        if( fabs( lat ) < 90.0 ) gLat = lat;

        s.Printf( _T ( "Setting Ownship Lat/Lon %g, %g" ), gLat, gLon );
        ZCHX_LOGMSG( s );

    }

//    Fonts

    //  Load the persistent Auxiliary Font descriptor Keys
    SetPath ( _T ( "/Settings/AuxFontKeys" ) );

    wxString strk;
    long dummyk;
    wxString kval;
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
    SetPath ( _T ( "/Settings/X11Fonts" ) );
#endif

#ifdef __WXGTK__
    SetPath ( _T ( "/Settings/GTKFonts" ) );
#endif

#ifdef __WXMSW__
    SetPath( _T ( "/Settings/MSWFonts" ) );
#endif

#ifdef __WXMAC__
    SetPath ( _T ( "/Settings/MacFonts" ) );
#endif

#ifdef __WXQT__
    SetPath ( _T ( "/Settings/QTFonts" ) );
#endif

    wxString str;
    long dummy;
    wxString *pval = new wxString;
    wxArrayString deleteList;

    bool bCont = GetFirstEntry( str, dummy );
    while( bCont ) {
        Read( str, pval );

        if( str.StartsWith( _T("Font") ) ) {
            // Convert pre 3.1 setting. Can't delete old entries from inside the
            // GetNextEntry() loop, so we need to save those and delete outside.
            deleteList.Add( str );
            wxString oldKey = pval->BeforeFirst( _T(':') );
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
    SetPath( _T ( "/TideCurrentDataSources" ) );
    if( GetNumberOfEntries() ) {
        TideCurrentDataSet.Clear();
        wxString str, val;
        long dummy;
        int iDir = 0;
        bool bCont = GetFirstEntry( str, dummy );
        while( bCont ) {
            Read( str, &val );              // Get a file name
            TideCurrentDataSet.Add(val);
            bCont = GetNextEntry( str, dummy );
        }
    }



    //    Groups
    LoadConfigGroups( g_pGroupArray );

//     //    Multicanvas Settings
//     LoadCanvasConfigs();

    SetPath( _T ( "/Settings/Others" ) );

    // Radar rings
    Read( _T ( "RadarRingsNumberVisible" ), &val );
    if( val.Length() > 0 ) g_iNavAidRadarRingsNumberVisible = atoi( val.mb_str() );

    Read( _T ( "RadarRingsStep" ), &val );
    if( val.Length() > 0 ) g_fNavAidRadarRingsStep = atof( val.mb_str() );

    Read( _T ( "RadarRingsStepUnits" ), &g_pNavAidRadarRingsStepUnits );

    wxString l_wxsOwnshipRangeRingsColour;
    Read( _T ( "RadarRingsColour" ), &l_wxsOwnshipRangeRingsColour );
    if(l_wxsOwnshipRangeRingsColour.Length()) g_colourOwnshipRangeRingsColour.Set( l_wxsOwnshipRangeRingsColour );

    // Waypoint Radar rings
    Read( _T ( "WaypointRangeRingsNumber" ), &val );
    if( val.Length() > 0 ) g_iWaypointRangeRingsNumber = atoi( val.mb_str() );

    Read( _T ( "WaypointRangeRingsStep" ), &val );
    if( val.Length() > 0 ) g_fWaypointRangeRingsStep = atof( val.mb_str() );

    Read( _T ( "WaypointRangeRingsStepUnits" ), &g_iWaypointRangeRingsStepUnits );

    wxString l_wxsWaypointRangeRingsColour;
    Read( _T( "WaypointRangeRingsColour" ), &l_wxsWaypointRangeRingsColour );
    g_colourWaypointRangeRingsColour.Set( l_wxsWaypointRangeRingsColour );

    if ( !Read( _T("WaypointUseScaMin"), &g_bUseWptScaMin ) ) g_bUseWptScaMin = false;
    if ( !Read( _T("WaypointScaMinValue"), &g_iWpt_ScaMin ) ) g_iWpt_ScaMin = 2147483646;
    if ( !Read( _T("WaypointUseScaMinOverrule"), &g_bOverruleScaMin ) ) g_bOverruleScaMin = false;
    if ( !Read( _T("WaypointsShowName"), &g_bShowWptName ) ) g_bShowWptName = true;



    //  Support Version 3.0 and prior config setting for Radar Rings
    bool b300RadarRings= true;
    if(Read ( _T ( "ShowRadarRings" ), &b300RadarRings )){
        if(!b300RadarRings)
            g_iNavAidRadarRingsNumberVisible = 0;
    }

    Read( _T ( "ConfirmObjectDeletion" ), &g_bConfirmObjectDelete );

    // Waypoint dragging with mouse
    g_bWayPointPreventDragging = false;
    Read( _T ( "WaypointPreventDragging" ), &g_bWayPointPreventDragging );

    g_bEnableZoomToCursor = false;
    Read( _T ( "EnableZoomToCursor" ), &g_bEnableZoomToCursor );

    val.Clear();
    Read( _T ( "TrackIntervalSeconds" ), &val );
    if( val.Length() > 0 ) {
        double tval = atof( val.mb_str() );
        if( tval >= 2. ) g_TrackIntervalSeconds = tval;
    }

    val.Clear();
    Read( _T ( "TrackDeltaDistance" ), &val );
    if( val.Length() > 0 ) {
        double tval = atof( val.mb_str() );
        if( tval >= 0.05 ) g_TrackDeltaDistance = tval;
    }

    Read( _T ( "TrackPrecision" ), &g_nTrackPrecision );

    Read( _T ( "NavObjectFileName" ), m_sNavObjSetFile );

    Read( _T ( "RouteLineWidth" ), &g_route_line_width );
    Read( _T ( "TrackLineWidth" ), &g_track_line_width );

    wxString l_wxsTrackLineColour;
    if(Read( _T( "TrackLineColour" ), &l_wxsTrackLineColour ))
        g_colourTrackLineColour.Set( l_wxsTrackLineColour );

    Read( _T ( "TideCurrentWindowScale" ), &g_tcwin_scale );
    Read( _T ( "DefaultWPIcon" ), &g_default_wp_icon );
    Read( _T ( "DefaultRPIcon" ), &g_default_routepoint_icon );

    SetPath( _T ( "/MMSIProperties" ) );
    int iPMax = GetNumberOfEntries();
    if( iPMax ) {
        g_MMSI_Props_Array.Empty();
        wxString str, val;
        long dummy;
        int iDir = 0;
        bool bCont = pConfig->GetFirstEntry( str, dummy );
        while( bCont ) {
            pConfig->Read( str, &val );              // Get an entry

            MMSIProperties *pProps = new MMSIProperties( val );
            g_MMSI_Props_Array.Add(pProps);

            bCont = pConfig->GetNextEntry( str, dummy );

        }
    }

    return ( 0 );
}

void zchxConfig::LoadS57Config()
{
    if( !ps52plib )
        return;

    int read_int;
    double dval;
    SetPath( _T ( "/Settings/GlobalState" ) );

    Read( _T ( "bShowS57Text" ), &read_int, 0 );
    ps52plib->SetShowS57Text( !( read_int == 0 ) );

    Read( _T ( "bShowS57ImportantTextOnly" ), &read_int, 0 );
    ps52plib->SetShowS57ImportantTextOnly( !( read_int == 0 ) );

    Read( _T ( "bShowLightDescription" ), &read_int, 0 );
    ps52plib->SetShowLdisText( !( read_int == 0 ) );

    Read( _T ( "bExtendLightSectors" ), &read_int, 0 );
    ps52plib->SetExtendLightSectors( !( read_int == 0 ) );

    Read( _T ( "nDisplayCategory" ), &read_int, (enum _DisCat) STANDARD );
    ps52plib->SetDisplayCategory((enum _DisCat) read_int );

    Read( _T ( "nSymbolStyle" ), &read_int, (enum _LUPname) PAPER_CHART );
    ps52plib->m_nSymbolStyle = (LUPname) read_int;

    Read( _T ( "nBoundaryStyle" ), &read_int, PLAIN_BOUNDARIES );
    ps52plib->m_nBoundaryStyle = (LUPname) read_int;

    Read( _T ( "bShowSoundg" ), &read_int, 1 );
    ps52plib->m_bShowSoundg = !( read_int == 0 );

    Read( _T ( "bShowMeta" ), &read_int, 0 );
    ps52plib->m_bShowMeta = !( read_int == 0 );

    Read( _T ( "bUseSCAMIN" ), &read_int, 1 );
    ps52plib->m_bUseSCAMIN = !( read_int == 0 );

    Read( _T ( "bShowAtonText" ), &read_int, 1 );
    ps52plib->m_bShowAtonText = !( read_int == 0 );

    Read( _T ( "bDeClutterText" ), &read_int, 0 );
    ps52plib->m_bDeClutterText = !( read_int == 0 );

    Read( _T ( "bShowNationalText" ), &read_int, 0 );
    ps52plib->m_bShowNationalTexts = !( read_int == 0 );

    if( Read( _T ( "S52_MAR_SAFETY_CONTOUR" ), &dval, 5.0 ) ) {
        S52_setMarinerParam( S52_MAR_SAFETY_CONTOUR, dval );
        S52_setMarinerParam( S52_MAR_SAFETY_DEPTH, dval ); // Set safety_contour and safety_depth the same
    }

    if( Read( _T ( "S52_MAR_SHALLOW_CONTOUR" ), &dval, 3.0 ) ) S52_setMarinerParam(
        S52_MAR_SHALLOW_CONTOUR, dval );

    if( Read( _T ( "S52_MAR_DEEP_CONTOUR" ), &dval, 10.0 ) ) S52_setMarinerParam(
        S52_MAR_DEEP_CONTOUR, dval );

    if( Read( _T ( "S52_MAR_TWO_SHADES" ), &dval, 0.0 ) ) S52_setMarinerParam(
        S52_MAR_TWO_SHADES, dval );

    ps52plib->UpdateMarinerParams();

    SetPath( _T ( "/Settings/GlobalState" ) );
    Read( _T ( "S52_DEPTH_UNIT_SHOW" ), &read_int, 1 );   // default is metres
    read_int = wxMax(read_int, 0);                      // qualify value
    read_int = wxMin(read_int, 2);
    ps52plib->m_nDepthUnitDisplay = read_int;
    g_nDepthUnitDisplay = read_int;

//    S57 Object Class Visibility

    OBJLElement *pOLE;

    SetPath( _T ( "/Settings/ObjectFilter" ) );

    int iOBJMax = GetNumberOfEntries();
    if( iOBJMax ) {

        wxString str;
        long val;
        long dummy;

        wxString sObj;

        bool bCont = pConfig->GetFirstEntry( str, dummy );
        while( bCont ) {
            pConfig->Read( str, &val );              // Get an Object Viz

            bool bNeedNew = true;

            if( str.StartsWith( _T ( "viz" ), &sObj ) ) {
                for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
                    pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
                    if( !strncmp( pOLE->OBJLName, sObj.mb_str(), 6 ) ) {
                        pOLE->nViz = val;
                        bNeedNew = false;
                        break;
                    }
                }

                if( bNeedNew ) {
                    pOLE = (OBJLElement *) calloc( sizeof(OBJLElement), 1 );
                    memcpy( pOLE->OBJLName, sObj.mb_str(), OBJL_NAME_LEN );
                    pOLE->nViz = 1;

                    ps52plib->pOBJLArray->Add( (void *) pOLE );
                }
            }
            bCont = pConfig->GetNextEntry( str, dummy );
        }
    }
}


bool zchxConfig::LoadLayers(wxString &path)
{
    wxArrayString file_array;
    wxDir dir;
    Layer *l;
    dir.Open( path );
    if( dir.IsOpened() ) {
        wxString filename;
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
                    wxFileName::SplitPath( file_array[0], NULL, NULL, &( l->m_LayerName ), NULL, NULL );
                else
                    wxFileName::SplitPath( filename, NULL, NULL, &( l->m_LayerName ), NULL, NULL );

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

                wxString laymsg;
                laymsg.Printf( wxT("New layer %d: %s"), l->m_LayerID, l->m_LayerName.c_str() );
                ZCHX_LOGMSG( laymsg );

                pLayerList->Insert( l );

                //  Load the entire file array as a single layer

                for( unsigned int i = 0; i < file_array.GetCount(); i++ ) {
                    wxString file_path = file_array[i];

                    if( ::wxFileExists( file_path ) ) {
                        NavObjectCollection1 *pSet = new NavObjectCollection1;
                        pSet->load_file(file_path.fn_str());
                        long nItems = pSet->LoadAllGPXObjectsAsLayer(l->m_LayerID, bLayerViz, l->m_bHasVisibleNames);
                        l->m_NoOfItems += nItems;
                        l->m_LayerType = _("Persistent");

                        wxString objmsg;
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
    SetPath( _T ( "/ChartDirectories" ) );
    int iDirMax = GetNumberOfEntries();
    if( iDirMax ) {
        ChartDirArray.Empty();
        wxString str, val;
        long dummy;
        int nAdjustChartDirs = 0;
        int iDir = 0;
        bool bCont = pConfig->GetFirstEntry( str, dummy );
        while( bCont ) {
            pConfig->Read( str, &val );              // Get a Directory name

            wxString dirname( val );
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
                if( dirname.Find( _T ( "SampleCharts" ) ) == 0 ) // only update entries starting with "SampleCharts"
                        {
                    nAdjustChartDirs++;

                    pConfig->DeleteEntry( str );
                    wxString new_dir = dirname.Mid( dirname.Find( _T ( "SampleCharts" ) ) );
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
    wxString key, dir;
    wxString str_buf;

    SetPath( _T ( "/ChartDirectories" ) );
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

        wxString dirn = cdi.fullpath;
        dirn.Append( _T("^") );
        dirn.Append( cdi.magic_number );

        str_buf.Printf( _T ( "ChartDir%d" ), iDir + 1 );

        Write( str_buf, dirn );

    }

    Flush();
    return true;
}

void zchxConfig::CreateConfigGroups( ChartGroupArray *pGroupArray )
{
    if( !pGroupArray ) return;

    SetPath( _T ( "/Groups" ) );
    Write( _T ( "GroupCount" ), (int) pGroupArray->GetCount() );

    for( unsigned int i = 0; i < pGroupArray->GetCount(); i++ ) {
        ChartGroup *pGroup = pGroupArray->Item( i );
        wxString s;
        s.Printf( _T("Group%d"), i + 1 );
        s.Prepend( _T ( "/Groups/" ) );
        SetPath( s );

        Write( _T ( "GroupName" ), pGroup->m_group_name );
        Write( _T ( "GroupItemCount" ), (int) pGroup->m_element_array.size() );

        for( unsigned int j = 0; j < pGroup->m_element_array.size(); j++ ) {
            wxString sg;
            sg.Printf( _T("Group%d/Item%d"), i + 1, j );
            sg.Prepend( _T ( "/Groups/" ) );
            SetPath( sg );
            Write( _T ( "IncludeItem" ), pGroup->m_element_array[j]->m_element_name );

            wxString t;
            wxArrayString u = pGroup->m_element_array[j]->m_missing_name_array;
            if( u.GetCount() ) {
                for( unsigned int k = 0; k < u.GetCount(); k++ ) {
                    t += u[k];
                    t += _T(";");
                }
                Write( _T ( "ExcludeItems" ), t );
            }
        }
    }
}

void zchxConfig::DestroyConfigGroups( void )
{
    DeleteGroup( _T ( "/Groups" ) );                //zap
}

void zchxConfig::LoadConfigGroups( ChartGroupArray *pGroupArray )
{
    SetPath( _T ( "/Groups" ) );
    unsigned int group_count;
    Read( _T ( "GroupCount" ), (int *) &group_count, 0 );

    for( unsigned int i = 0; i < group_count; i++ ) {
        ChartGroup *pGroup = new ChartGroup;
        wxString s;
        s.Printf( _T("Group%d"), i + 1 );
        s.Prepend( _T ( "/Groups/" ) );
        SetPath( s );

        wxString t;
        Read( _T ( "GroupName" ), &t );
        pGroup->m_group_name = t;

        unsigned int item_count;
        Read( _T ( "GroupItemCount" ), (int *) &item_count );
        for( unsigned int j = 0; j < item_count; j++ ) {
            wxString sg;
            sg.Printf( _T("Group%d/Item%d"), i + 1, j );
            sg.Prepend( _T ( "/Groups/" ) );
            SetPath( sg );

            wxString v;
            Read( _T ( "IncludeItem" ), &v );
            ChartGroupElement *pelement = new ChartGroupElement{v};
            pGroup->m_element_array.emplace_back( pelement );

            wxString u;
            if( Read( _T ( "ExcludeItems" ), &u ) ) {
                if( !u.IsEmpty() ) {
                    wxStringTokenizer tk( u, _T(";") );
                    while( tk.HasMoreTokens() ) {
                        wxString token = tk.GetNextToken();
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
    wxString s;
    canvasConfig *pcc;

    SetPath( _T ( "/Canvas" ) );

    //  If the canvas config has never been set/persisted, use the global settings
    if(!HasEntry( _T ( "CanvasConfig" ))){

        pcc = new canvasConfig(0);
        pcc->LoadFromLegacyConfig( this );
        g_canvasConfigArray.Add(pcc);

        return;
    }

    Read( _T ( "CanvasConfig" ), (int *)&g_canvasConfig, 0 );

    // Do not recreate canvasConfigs when applying config dynamically
    if(g_canvasConfigArray.GetCount() == 0){            // This is initial load from startup
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
    } else {                                            // This is a dynamic (i.e. Template) load
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
    wxString st;
    double st_lat, st_lon;

    if(!bApplyAsTemplate){
        //    Reasonable starting point
        cConfig->iLat = START_LAT;                   // display viewpoint
        cConfig->iLon = START_LON;

        if( Read( _T ( "canvasVPLatLon" ), &st ) ) {
            sscanf( st.mb_str( wxConvUTF8 ), "%lf,%lf", &st_lat, &st_lon );

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

        cConfig->iScale = .0003;        // decent initial value
        cConfig->iRotation = 0;

        double st_view_scale;
        if( Read( wxString( _T ( "canvasVPScale" ) ), &st ) ) {
            sscanf( st.mb_str( wxConvUTF8 ), "%lf", &st_view_scale );
            //    Sanity check the scale
            st_view_scale = fmax ( st_view_scale, .001/32 );
            st_view_scale = fmin ( st_view_scale, 4 );
            cConfig->iScale = st_view_scale;
        }

        double st_rotation;
        if( Read( wxString( _T ( "canvasVPRotation" ) ), &st ) ) {
            sscanf( st.mb_str( wxConvUTF8 ), "%lf", &st_rotation );
            //    Sanity check the rotation
            st_rotation = fmin ( st_rotation, 360 );
            st_rotation = fmax ( st_rotation, 0 );
            cConfig->iRotation = st_rotation * PI / 180.;
        }

        Read( _T ( "canvasInitialdBIndex" ), &cConfig->DBindex, 0 );
        Read( _T ( "canvasbFollow" ), &cConfig->bFollow, 0 );

        Read( _T ( "canvasCourseUp" ), &cConfig->bCourseUp, 0 );
        Read( _T ( "canvasLookahead" ), &cConfig->bLookahead, 0 );
    }

    Read( _T ( "ActiveChartGroup" ), &cConfig->GroupID, 0 );

    // Special check for group selection when applied as template
    if(cConfig->GroupID && bApplyAsTemplate){
        if( cConfig->GroupID > (int) g_pGroupArray->GetCount() )
            cConfig->GroupID = 0;
    }

    Read( _T ( "canvasShowTides" ), &cConfig->bShowTides, 0 );
    Read( _T ( "canvasShowCurrents" ), &cConfig->bShowCurrents, 0 );


    Read( _T ( "canvasQuilt" ), &cConfig->bQuilt, 1 );
    Read( _T ( "canvasShowGrid" ), &cConfig->bShowGrid, 0 );
    Read( _T ( "canvasShowOutlines" ), &cConfig->bShowOutlines, 0 );
    Read( _T ( "canvasShowDepthUnits" ), &cConfig->bShowDepthUnits, 0 );

    Read( _T ( "canvasShowAIS" ), &cConfig->bShowAIS, 1 );
    Read( _T ( "canvasAttenAIS" ), &cConfig->bAttenAIS, 0 );

    // ENC options
    Read( _T ( "canvasShowENCText" ), &cConfig->bShowENCText, 1 );
    Read( _T ( "canvasENCDisplayCategory" ), &cConfig->nENCDisplayCategory, STANDARD );
    Read( _T ( "canvasENCShowDepths" ), &cConfig->bShowENCDepths, 1 );
    Read( _T ( "canvasENCShowBuoyLabels" ), &cConfig->bShowENCBuoyLabels, 1 );
    Read( _T ( "canvasENCShowLightDescriptions" ), &cConfig->bShowENCLightDescriptions, 1 );
    Read( _T ( "canvasENCShowLights" ), &cConfig->bShowENCLights, 1 );


    int sx, sy;
    Read( _T ( "canvasSizeX" ), &sx, 0 );
    Read( _T ( "canvasSizeY" ), &sy, 0 );
    cConfig->canvasSize = wxSize(sx, sy);

#endif


}


void zchxConfig::SaveCanvasConfigs( )
{
#if 0
    SetPath( _T ( "/Canvas" ) );
    Write( _T ( "CanvasConfig" ), (int )g_canvasConfig );

    wxString s;
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
    wxString st1;

    if(cConfig->canvas){
        ViewPort vp = cConfig->canvas->GetVP();

        if( vp.IsValid() ) {
            st1.Printf( _T ( "%10.4f,%10.4f" ), vp.clat, vp.clon );
            Write( _T ( "canvasVPLatLon" ), st1 );
            st1.Printf( _T ( "%g" ), vp.view_scale_ppm );
            Write( _T ( "canvasVPScale" ), st1 );
            st1.Printf( _T ( "%i" ), ((int)(vp.rotation * 180 / PI)) % 360 );
            Write( _T ( "canvasVPRotation" ), st1 );
        }

        int restore_dbindex = 0;
        ChartStack *pcs = cConfig->canvas->GetpCurrentStack();
        if(pcs)
            restore_dbindex = pcs->GetCurrentEntrydbIndex();
        if( cConfig->canvas->GetQuiltMode())
            restore_dbindex = cConfig->canvas->GetQuiltReferenceChartIndex();
        Write( _T ( "canvasInitialdBIndex" ), restore_dbindex );

        Write( _T ( "canvasbFollow" ), cConfig->canvas->m_bFollow );
        Write( _T ( "ActiveChartGroup" ), cConfig->canvas->m_groupIndex );

        Write( _T ( "canvasToolbarConfig" ), cConfig->canvas->GetToolbarConfigString() );
        Write( _T ( "canvasShowToolbar" ), 0 );  //cConfig->canvas->GetToolbarEnable() );

        Write( _T ( "canvasQuilt" ), cConfig->canvas->GetQuiltMode() );
        Write( _T ( "canvasShowGrid" ), cConfig->canvas->GetShowGrid() );
        Write( _T ( "canvasShowOutlines" ), cConfig->canvas->GetShowOutlines() );
        Write( _T ( "canvasShowDepthUnits" ), cConfig->canvas->GetShowDepthUnits() );

        Write( _T ( "canvasShowAIS" ), cConfig->canvas->GetShowAIS() );
        Write( _T ( "canvasAttenAIS" ), cConfig->canvas->GetAttenAIS() );

        Write( _T ( "canvasShowTides" ), cConfig->canvas->GetbShowTide() );
        Write( _T ( "canvasShowCurrents" ), cConfig->canvas->GetbShowCurrent() );

        // ENC options
        Write( _T ( "canvasShowENCText" ), cConfig->canvas->GetShowENCText() );
        Write( _T ( "canvasENCDisplayCategory" ), cConfig->canvas->GetENCDisplayCategory() );
        Write( _T ( "canvasENCShowDepths" ), cConfig->canvas->GetShowENCDepth() );
        Write( _T ( "canvasENCShowBuoyLabels" ), cConfig->canvas->GetShowENCBuoyLabels() );
        Write( _T ( "canvasENCShowLightDescriptions" ), cConfig->canvas->GetShowENCLightDesc() );
        Write( _T ( "canvasENCShowLights" ), cConfig->canvas->GetShowENCLights() );

        Write( _T ( "canvasCourseUp" ), cConfig->canvas->GetCourseUP() );
        Write( _T ( "canvasLookahead" ), cConfig->canvas->GetLookahead() );


        int width = cConfig->canvas->GetSize().x;
        //         if(cConfig->canvas->IsPrimaryCanvas()){
//             width = wxMax(width, gFrame->GetClientSize().x / 10);
//         }
//         else{
//             width = wxMin(width, gFrame->GetClientSize().x  * 9 / 10);
//         }

        Write( _T ( "canvasSizeX" ), width );
        Write( _T ( "canvasSizeY" ), cConfig->canvas->GetSize().y );

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
    SetPath( _T ( "/Settings" ) );

    Write( _T ( "LastAppliedTemplate" ), g_lastAppliedTemplateGUID );

    Write( _T ( "ConfigVersionString" ), g_config_version_string );
#ifdef SYSTEM_SOUND_CMD
    if ( wxIsEmpty( g_CmdSoundString ) )
        g_CmdSoundString = wxString( SYSTEM_SOUND_CMD );
    Write( _T( "CmdSoundString" ), g_CmdSoundString );
#endif /* SYSTEM_SOUND_CMD */
    Write( _T ( "NavMessageShown" ), n_NavMessageShown );
    Write( _T ( "InlandEcdis" ), g_bInlandEcdis );

    Write( _T ( "DarkDecorations"), g_bDarkDecorations );

    Write( _T ( "UIexpert" ), g_bUIexpert );
    Write( _T( "SpaceDropMark" ), g_bSpaceDropMark );
//    Write( _T ( "UIStyle" ), g_StyleManager->GetStyleNextInvocation() );      //Not desired for O5 MUI

    Write( _T ( "ShowStatusBar" ), g_bShowStatusBar );
#ifndef __WXOSX__
    Write( _T ( "ShowMenuBar" ), g_bShowMenuBar );
#endif
    Write( _T ( "DefaultFontSize" ), g_default_font_size );

    Write( _T ( "Fullscreen" ), g_bFullscreen );
    Write( _T ( "ShowCompassWindow" ), g_bShowCompassWin );
    Write( _T ( "SetSystemTime" ), s_bSetSystemTime );
    Write( _T ( "ShowGrid" ), g_bDisplayGrid );
    Write( _T ( "PlayShipsBells" ), g_bPlayShipsBells );
    Write( _T ( "SoundDeviceIndex" ), g_iSoundDeviceIndex );
    Write( _T ( "FullscreenToolbar" ), g_bFullscreenToolbar );
    Write( _T ( "TransparentToolbar" ), g_bTransparentToolbar );
    Write( _T ( "PermanentMOBIcon" ), g_bPermanentMOBIcon );
    Write( _T ( "ShowLayers" ), g_bShowLayers );
    Write( _T ( "AutoAnchorDrop" ), g_bAutoAnchorMark );
    Write( _T ( "ShowChartOutlines" ), g_bShowOutlines );
    Write( _T ( "ShowActiveRouteTotal" ), g_bShowRouteTotal );
    Write( _T ( "ShowActiveRouteHighway" ), g_bShowActiveRouteHighway );
    Write( _T ( "SDMMFormat" ), g_iSDMMFormat );
    Write( _T ( "MostRecentGPSUploadConnection" ), g_uploadConnection );
    Write( _T ( "ShowChartBar" ), g_bShowChartBar );

    Write( _T ( "GUIScaleFactor" ), g_GUIScaleFactor );
    Write( _T ( "ChartObjectScaleFactor" ), g_ChartScaleFactor );
    Write( _T ( "ShipScaleFactor" ), g_ShipScaleFactor );

    Write( _T ( "FilterNMEA_Avg" ), g_bfilter_cogsog );
    Write( _T ( "FilterNMEA_Sec" ), g_COGFilterSec );

    Write( _T ( "ShowTrue" ), g_bShowTrue );
    Write( _T ( "ShowMag" ), g_bShowMag );
    Write( _T ( "UserMagVariation" ), wxString::Format( _T("%.2f"), g_UserVar ) );

    Write( _T ( "CM93DetailFactor" ), g_cm93_zoom_factor );
    Write( _T ( "CM93DetailZoomPosX" ), g_detailslider_dialog_x );
    Write( _T ( "CM93DetailZoomPosY" ), g_detailslider_dialog_y );
    Write( _T ( "ShowCM93DetailSlider" ), g_bShowDetailSlider );

    Write( _T ( "SkewToNorthUp" ), g_bskew_comp );
    Write( _T ( "OpenGL" ), g_bopengl );
    Write( _T ( "SoftwareGL" ), g_bSoftwareGL );
    Write( _T ( "ShowFPS" ), g_bShowFPS );

    Write( _T ( "ZoomDetailFactor" ), g_chart_zoom_modifier );
    Write( _T ( "ZoomDetailFactorVector" ), g_chart_zoom_modifier_vector );

    Write( _T ( "FogOnOverzoom" ), g_fog_overzoom );
    Write( _T ( "OverzoomVectorScale" ), g_oz_vector_scale );
    Write( _T ( "OverzoomEmphasisBase" ), g_overzoom_emphasis_base );

#ifdef ocpnUSE_GL
    /* opengl options */
    Write( _T ( "UseAcceleratedPanning" ), g_GLOptions.m_bUseAcceleratedPanning );

    Write( _T ( "GPUTextureCompression" ), g_GLOptions.m_bTextureCompression);
    Write( _T ( "GPUTextureCompressionCaching" ), g_GLOptions.m_bTextureCompressionCaching);
    Write( _T ( "GPUTextureDimension" ), g_GLOptions.m_iTextureDimension );
    Write( _T ( "GPUTextureMemSize" ), g_GLOptions.m_iTextureMemorySize );
    Write( _T ( "PolygonSmoothing" ), g_GLOptions.m_GLPolygonSmoothing);
    Write( _T ( "LineSmoothing" ), g_GLOptions.m_GLLineSmoothing);
#endif
    Write( _T ( "SmoothPanZoom" ), g_bsmoothpanzoom );

    Write( _T ( "CourseUpMode" ), g_bCourseUp );
    if (!g_bInlandEcdis ) Write( _T ( "LookAheadMode" ), g_bLookAhead );
    Write( _T ( "COGUPAvgSeconds" ), g_COGAvgSec );
    Write( _T ( "UseMagAPB" ), g_bMagneticAPB );

    Write( _T ( "OwnshipCOGPredictorMinutes" ), g_ownship_predictor_minutes );
    Write( _T ( "OwnshipCOGPredictorWidth" ), g_cog_predictor_width );
    Write( _T ( "OwnshipHDTPredictorMiles" ), g_ownship_HDTpredictor_miles );
    Write( _T ( "OwnShipIconType" ), g_OwnShipIconType );
    Write( _T ( "OwnShipLength" ), g_n_ownship_length_meters );
    Write( _T ( "OwnShipWidth" ), g_n_ownship_beam_meters );
    Write( _T ( "OwnShipGPSOffsetX" ), g_n_gps_antenna_offset_x );
    Write( _T ( "OwnShipGPSOffsetY" ), g_n_gps_antenna_offset_y );
    Write( _T ( "OwnShipMinSize" ), g_n_ownship_min_mm );
    Write( _T ( "OwnShipSogCogCalc" ), g_own_ship_sog_cog_calc );
    Write( _T ( "OwnShipSogCogCalcDampSec"), g_own_ship_sog_cog_calc_damp_sec );

    wxString racr;
 //   racr.Printf( _T ( "%g" ), g_n_arrival_circle_radius );
 //   Write( _T ( "RouteArrivalCircleRadius" ), racr );
    Write( _T ( "RouteArrivalCircleRadius" ), wxString::Format( _T("%.2f"), g_n_arrival_circle_radius ));

    Write( _T ( "ChartQuilting" ), g_bQuiltEnable );

    Write( _T ( "NMEALogWindowSizeX" ), NMEALogWindow::Get().GetSizeW());
    Write( _T ( "NMEALogWindowSizeY" ), NMEALogWindow::Get().GetSizeH());
    Write( _T ( "NMEALogWindowPosX" ), NMEALogWindow::Get().GetPosX());
    Write( _T ( "NMEALogWindowPosY" ), NMEALogWindow::Get().GetPosY());

    Write( _T ( "PreserveScaleOnX" ), g_bPreserveScaleOnX );

    Write( _T ( "StartWithTrackActive" ), g_bTrackCarryOver );
    Write( _T ( "AutomaticDailyTracks" ), g_bTrackDaily );
    Write( _T ( "TrackRotateAt" ), g_track_rotate_time );
    Write( _T ( "TrackRotateTimeType" ), g_track_rotate_time_type );
    Write( _T ( "HighlightTracks" ), g_bHighliteTracks );

    Write( _T ( "InitialStackIndex" ), g_restore_stackindex );
    Write( _T ( "InitialdBIndex" ), g_restore_dbindex );
    Write( _T ( "ActiveChartGroup" ), g_GroupIndex );

    Write( _T( "NMEAAPBPrecision" ), g_NMEAAPBPrecision );

    Write( _T("TalkerIdText"), g_TalkerIdText );

    Write( _T ( "AnchorWatch1GUID" ), g_AW1GUID );
    Write( _T ( "AnchorWatch2GUID" ), g_AW2GUID );

    //Write( _T ( "ToolbarX" ), g_maintoolbar_x );
    //Write( _T ( "ToolbarY" ), g_maintoolbar_y );
    //Write( _T ( "ToolbarOrient" ), g_maintoolbar_orient );

    Write( _T ( "iENCToolbarX" ), g_iENCToolbarPosX );
    Write( _T ( "iENCToolbarY" ), g_iENCToolbarPosY );

    if ( !g_bInlandEcdis ){
        Write( _T ( "GlobalToolbarConfig" ), g_toolbarConfig );
        Write( _T ( "DistanceFormat" ), g_iDistanceFormat );
        Write( _T ( "SpeedFormat" ), g_iSpeedFormat );
        Write( _T ( "ShowDepthUnits" ), g_bShowDepthUnits );
    }
    Write( _T ( "GPSIdent" ), g_GPS_Ident );
    Write( _T ( "UseGarminHostUpload" ), g_bGarminHostUpload );

    Write( _T ( "MobileTouch" ), g_btouch );
    Write( _T ( "ResponsiveGraphics" ), g_bresponsive );

    Write( _T ( "AutoHideToolbar" ), g_bAutoHideToolbar );
    Write( _T ( "AutoHideToolbarSecs" ), g_nAutoHideToolbar );

    Write( _T ( "DisplaySizeMM" ), g_config_display_size_mm );
    Write( _T ( "DisplaySizeManual" ), g_config_display_size_manual );

    Write( _T ( "SelectionRadiusMM" ), g_selection_radius_mm );
    Write( _T ( "SelectionRadiusTouchMM" ), g_selection_radius_touch_mm );

    wxString st0;
    st0.Printf( _T ( "%g" ), g_PlanSpeed );
    Write( _T ( "PlanSpeed" ), st0 );

    if(g_bLayersLoaded){
        wxString vis, invis, visnames, invisnames;
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
        Write( _T ( "VisibleLayers" ), vis );
        Write( _T ( "InvisibleLayers" ), invis );
        Write( _T ( "VisNameInLayers" ), visnames);
        Write( _T ( "InvisNameInLayers" ), invisnames);
    }
    Write( _T ( "Locale" ), g_locale );
    Write( _T ( "LocaleOverride" ), g_localeOverride );

    Write( _T ( "KeepNavobjBackups" ), g_navobjbackups );
    Write( _T ( "LegacyInputCOMPortFilterBehaviour" ), g_b_legacy_input_filter_behaviour );
    Write( _T( "AdvanceRouteWaypointOnArrivalOnly" ), g_bAdvanceRouteWaypointOnArrivalOnly);

    // LIVE ETA OPTION
    Write( _T( "LiveETA" ), g_bShowLiveETA);
    Write( _T( "DefaultBoatSpeed" ), g_defaultBoatSpeed);

//    S57 Object Filter Settings

    SetPath( _T ( "/Settings/ObjectFilter" ) );

    if( ps52plib ) {
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );

            wxString st1( _T ( "viz" ) );
            char name[7];
            strncpy( name, pOLE->OBJLName, 6 );
            name[6] = 0;
            st1.Append( wxString( name, wxConvUTF8 ) );
            Write( st1, pOLE->nViz );
        }
    }

//    Global State

    SetPath( _T ( "/Settings/GlobalState" ) );

    wxString st1;

//     if( cc1 ) {
//         ViewPort vp = cc1->GetVP();
//
//         if( vp.IsValid() ) {
//             st1.Printf( _T ( "%10.4f,%10.4f" ), vp.clat, vp.clon );
//             Write( _T ( "VPLatLon" ), st1 );
//             st1.Printf( _T ( "%g" ), vp.view_scale_ppm );
//             Write( _T ( "VPScale" ), st1 );
//             st1.Printf( _T ( "%i" ), ((int)(vp.rotation * 180 / PI)) % 360 );
//             Write( _T ( "VPRotation" ), st1 );
//         }
//     }

    st1.Printf( _T ( "%10.4f, %10.4f" ), gLat, gLon );
    Write( _T ( "OwnShipLatLon" ), st1 );

    //    Various Options
    SetPath( _T ( "/Settings/GlobalState" ) );
    if ( !g_bInlandEcdis ) Write( _T ( "nColorScheme" ), (int) gFrame->GetColorScheme() );

    Write( _T ( "FrameWinX" ), g_nframewin_x );
    Write( _T ( "FrameWinY" ), g_nframewin_y );
    Write( _T ( "FrameWinPosX" ), g_nframewin_posx );
    Write( _T ( "FrameWinPosY" ), g_nframewin_posy );
    Write( _T ( "FrameMax" ), g_bframemax );

    Write( _T ( "ClientPosX" ), g_lastClientRectx );
    Write( _T ( "ClientPosY" ), g_lastClientRecty );
    Write( _T ( "ClientSzX" ), g_lastClientRectw );
    Write( _T ( "ClientSzY" ), g_lastClientRecth );

    Write( _T ( "S52_DEPTH_UNIT_SHOW" ), g_nDepthUnitDisplay );

    Write( _T( "RoutePropSizeX" ), g_route_prop_sx );
    Write( _T( "RoutePropSizeY" ), g_route_prop_sy );
    Write( _T( "RoutePropPosX" ), g_route_prop_x );
    Write( _T( "RoutePropPosY" ), g_route_prop_y );

    //    AIS
    SetPath( _T ( "/Settings/AIS" ) );

    Write( _T ( "bNoCPAMax" ), g_bCPAMax );
    Write( _T ( "NoCPAMaxNMi" ), g_CPAMax_NM );
    Write( _T ( "bCPAWarn" ), g_bCPAWarn );
    Write( _T ( "CPAWarnNMi" ), g_CPAWarn_NM );
    Write( _T ( "bTCPAMax" ), g_bTCPA_Max );
    Write( _T ( "TCPAMaxMinutes" ), g_TCPA_Max );
    Write( _T ( "bMarkLostTargets" ), g_bMarkLost );
    Write( _T ( "MarkLost_Minutes" ), g_MarkLost_Mins );
    Write( _T ( "bRemoveLostTargets" ), g_bRemoveLost );
    Write( _T ( "RemoveLost_Minutes" ), g_RemoveLost_Mins );
    Write( _T ( "bShowCOGArrows" ), g_bShowCOG );
    Write( _T ( "CogArrowMinutes" ), g_ShowCOG_Mins );
    Write( _T ( "bShowTargetTracks" ), g_bAISShowTracks );
    Write( _T ( "TargetTracksMinutes" ), g_AISShowTracks_Mins );

    Write( _T ( "bHideMooredTargets" ), g_bHideMoored );
    Write( _T ( "MooredTargetMaxSpeedKnots" ), g_ShowMoored_Kts );

    Write( _T ( "bAISAlertDialog" ), g_bAIS_CPA_Alert );
    Write( _T ( "bAISAlertAudio" ), g_bAIS_CPA_Alert_Audio );
    Write( _T ( "AISAlertAudioFile" ), g_sAIS_Alert_Sound_File );
    Write( _T ( "bAISAlertSuppressMoored" ), g_bAIS_CPA_Alert_Suppress_Moored );
    Write( _T ( "bShowAreaNotices" ), g_bShowAreaNotices );
    Write( _T ( "bDrawAISSize" ), g_bDrawAISSize );
    Write( _T ( "bShowAISName" ), g_bShowAISName );
    Write( _T ( "ShowAISTargetNameScale" ), g_Show_Target_Name_Scale );
    Write( _T ( "bWplIsAprsPositionReport" ), g_bWplIsAprsPosition );
    Write( _T ( "AISCOGPredictorWidth" ), g_ais_cog_predictor_width );
    Write( _T ( "bShowScaledTargets" ), g_bAllowShowScaled );
    Write( _T ( "AISScaledNumber" ), g_ShowScaled_Num );
    Write( _T ( "AISScaledNumberWeightSOG" ), g_ScaledNumWeightSOG );
    Write( _T ( "AISScaledNumberWeightCPA" ), g_ScaledNumWeightCPA );
    Write( _T ( "AISScaledNumberWeightTCPA" ), g_ScaledNumWeightTCPA );
    Write( _T ( "AISScaledNumberWeightRange" ), g_ScaledNumWeightRange );
    Write( _T ( "AISScaledNumberWeightSizeOfTarget" ), g_ScaledNumWeightSizeOfT );
    Write( _T ( "AISScaledSizeMinimal" ), g_ScaledSizeMinimal );
    Write( _T ( "AISShowScaled"), g_bShowScaled);

    Write( _T ( "AlertDialogSizeX" ), g_ais_alert_dialog_sx );
    Write( _T ( "AlertDialogSizeY" ), g_ais_alert_dialog_sy );
    Write( _T ( "AlertDialogPosX" ), g_ais_alert_dialog_x );
    Write( _T ( "AlertDialogPosY" ), g_ais_alert_dialog_y );
    Write( _T ( "QueryDialogPosX" ), g_ais_query_dialog_x );
    Write( _T ( "QueryDialogPosY" ), g_ais_query_dialog_y );
    Write( _T ( "AISTargetListPerspective" ), g_AisTargetList_perspective );
    Write( _T ( "AISTargetListRange" ), g_AisTargetList_range );
    Write( _T ( "AISTargetListSortColumn" ), g_AisTargetList_sortColumn );
    Write( _T ( "bAISTargetListSortReverse" ), g_bAisTargetList_sortReverse );
    Write( _T ( "AISTargetListColumnSpec" ), g_AisTargetList_column_spec );
    Write( _T ("AISTargetListColumnOrder"), g_AisTargetList_column_order);

    Write( _T ( "S57QueryDialogSizeX" ), g_S57_dialog_sx );
    Write( _T ( "S57QueryDialogSizeY" ), g_S57_dialog_sy );
    Write( _T ( "S57QueryExtraDialogSizeX" ), g_S57_extradialog_sx );
    Write( _T ( "S57QueryExtraDialogSizeY" ), g_S57_extradialog_sy );

    Write( _T ( "bAISRolloverShowClass" ), g_bAISRolloverShowClass );
    Write( _T ( "bAISRolloverShowCOG" ), g_bAISRolloverShowCOG );
    Write( _T ( "bAISRolloverShowCPA" ), g_bAISRolloverShowCPA );

    Write( _T ( "bAISAlertAckTimeout" ), g_bAIS_ACK_Timeout );
    Write( _T ( "AlertAckTimeoutMinutes" ), g_AckTimeout_Mins );

    SetPath( _T ( "/Settings/GlobalState" ) );
    if( ps52plib ) {
        Write( _T ( "bShowS57Text" ), ps52plib->GetShowS57Text() );
        Write( _T ( "bShowS57ImportantTextOnly" ), ps52plib->GetShowS57ImportantTextOnly() );
        if ( !g_bInlandEcdis ) Write( _T ( "nDisplayCategory" ), (long) ps52plib->GetDisplayCategory() );
        Write( _T ( "nSymbolStyle" ), (int) ps52plib->m_nSymbolStyle );
        Write( _T ( "nBoundaryStyle" ), (int) ps52plib->m_nBoundaryStyle );

        Write( _T ( "bShowSoundg" ), ps52plib->m_bShowSoundg );
        Write( _T ( "bShowMeta" ), ps52plib->m_bShowMeta );
        Write( _T ( "bUseSCAMIN" ), ps52plib->m_bUseSCAMIN );
        Write( _T ( "bShowAtonText" ), ps52plib->m_bShowAtonText );
        Write( _T ( "bShowLightDescription" ), ps52plib->m_bShowLdisText );
        Write( _T ( "bExtendLightSectors" ), ps52plib->m_bExtendLightSectors );
        Write( _T ( "bDeClutterText" ), ps52plib->m_bDeClutterText );
        Write( _T ( "bShowNationalText" ), ps52plib->m_bShowNationalTexts );

        Write( _T ( "S52_MAR_SAFETY_CONTOUR" ), S52_getMarinerParam( S52_MAR_SAFETY_CONTOUR ) );
        Write( _T ( "S52_MAR_SHALLOW_CONTOUR" ), S52_getMarinerParam( S52_MAR_SHALLOW_CONTOUR ) );
        Write( _T ( "S52_MAR_DEEP_CONTOUR" ), S52_getMarinerParam( S52_MAR_DEEP_CONTOUR ) );
        Write( _T ( "S52_MAR_TWO_SHADES" ), S52_getMarinerParam( S52_MAR_TWO_SHADES ) );
        Write( _T ( "S52_DEPTH_UNIT_SHOW" ), ps52plib->m_nDepthUnitDisplay );
    }
    SetPath( _T ( "/Directories" ) );
    Write( _T ( "S57DataLocation" ), _T("") );
//    Write( _T ( "SENCFileLocation" ), _T("") );

    SetPath( _T ( "/Directories" ) );
    Write( _T ( "InitChartDir" ), *pInit_Chart_Dir );
    Write( _T ( "GPXIODir" ), g_gpx_path );
    Write( _T ( "TCDataDir" ), g_TCData_Dir );
    Write( _T ( "BasemapDir" ), g_Platform->NormalizePath(gWorldMapLocation) );

    SetPath( _T ( "/Settings/NMEADataSource" ) );
    wxString connectionconfigs;
    for (size_t i = 0; i < g_pConnectionParams->Count(); i++)
    {
        if (i > 0)
            connectionconfigs.Append(_T("|"));
        connectionconfigs.Append(g_pConnectionParams->Item(i)->Serialize());
    }
    Write ( _T ( "DataConnections" ), connectionconfigs );

    //    Fonts

    //  Store the persistent Auxiliary Font descriptor Keys
    SetPath( _T ( "/Settings/AuxFontKeys" ) );

    wxArrayString keyArray = FontMgr::Get().GetAuxKeyArray();
    for(unsigned int i=0 ; i <  keyArray.GetCount() ; i++){
        wxString key;
        key.Printf(_T("Key%i"), i);
        wxString keyval = keyArray[i];
        Write( key, keyval );
    }

    wxString font_path;
#ifdef __WXX11__
    font_path = ( _T ( "/Settings/X11Fonts" ) );
#endif

#ifdef __WXGTK__
    font_path = ( _T ( "/Settings/GTKFonts" ) );
#endif

#ifdef __WXMSW__
    font_path = ( _T ( "/Settings/MSWFonts" ) );
#endif

#ifdef __WXMAC__
    font_path = ( _T ( "/Settings/MacFonts" ) );
#endif

#ifdef __WXQT__
    font_path = ( _T ( "/Settings/QTFonts" ) );
#endif

    DeleteGroup(font_path);

    SetPath( font_path );

    int nFonts = FontMgr::Get().GetNumFonts();

    for( int i = 0; i < nFonts; i++ ) {
        wxString cfstring(FontMgr::Get().GetConfigString(i));
        wxString valstring = FontMgr::Get().GetFullConfigDesc( i );
        Write( cfstring, valstring );
    }

    //  Tide/Current Data Sources
    DeleteGroup( _T ( "/TideCurrentDataSources" ) );
    SetPath( _T ( "/TideCurrentDataSources" ) );
    unsigned int iDirMax = TideCurrentDataSet.Count();
    for( unsigned int id = 0 ; id < iDirMax ; id++ ) {
        wxString key;
        key.Printf(_T("tcds%d"), id);
        Write( key, TideCurrentDataSet[id] );
    }

    SetPath( _T ( "/Settings/Others" ) );

    // Radar rings
    Write( _T ( "ShowRadarRings" ), (bool)(g_iNavAidRadarRingsNumberVisible > 0) );  //3.0.0 config support
    Write( _T ( "RadarRingsNumberVisible" ), g_iNavAidRadarRingsNumberVisible );
    Write( _T ( "RadarRingsStep" ), g_fNavAidRadarRingsStep );
    Write( _T ( "RadarRingsStepUnits" ), g_pNavAidRadarRingsStepUnits );
    Write( _T ( "RadarRingsColour" ), g_colourOwnshipRangeRingsColour.GetAsString( wxC2S_HTML_SYNTAX ) );
    Write( _T( "WaypointUseScaMin" ), g_bUseWptScaMin );
    Write( _T( "WaypointScaMinValue" ), g_iWpt_ScaMin );
    Write( _T( "WaypointUseScaMinOverrule" ), g_bOverruleScaMin );
    Write( _T("WaypointsShowName"), g_bShowWptName );

    // Waypoint Radar rings
    Write( _T ( "WaypointRangeRingsNumber" ), g_iWaypointRangeRingsNumber );
    Write( _T ( "WaypointRangeRingsStep" ), g_fWaypointRangeRingsStep );
    Write( _T ( "WaypointRangeRingsStepUnits" ), g_iWaypointRangeRingsStepUnits );
    Write( _T ( "WaypointRangeRingsColour" ), g_colourWaypointRangeRingsColour.GetAsString( wxC2S_HTML_SYNTAX ) );

    Write( _T ( "ConfirmObjectDeletion" ), g_bConfirmObjectDelete );

    // Waypoint dragging with mouse; toh, 2009.02.24
    Write( _T ( "WaypointPreventDragging" ), g_bWayPointPreventDragging );

    Write( _T ( "EnableZoomToCursor" ), g_bEnableZoomToCursor );

    Write( _T ( "TrackIntervalSeconds" ), g_TrackIntervalSeconds );
    Write( _T ( "TrackDeltaDistance" ), g_TrackDeltaDistance );
    Write( _T ( "TrackPrecision" ), g_nTrackPrecision );

    Write( _T ( "RouteLineWidth" ), g_route_line_width );
    Write( _T ( "TrackLineWidth" ), g_track_line_width );
    Write( _T ( "TrackLineColour" ), g_colourTrackLineColour.GetAsString( wxC2S_HTML_SYNTAX ) );
    Write( _T ( "DefaultWPIcon" ), g_default_wp_icon );
    Write( _T ( "DefaultRPIcon" ), g_default_routepoint_icon );

    DeleteGroup(_T ( "/MMSIProperties" ));
    SetPath( _T ( "/MMSIProperties" ) );
    for(unsigned int i=0 ; i < g_MMSI_Props_Array.GetCount() ; i++){
        wxString p;
        p.Printf(_T("Props%d"), i);
        Write( p, g_MMSI_Props_Array[i]->Serialize() );
    }

    SaveCanvasConfigs();

    Flush();
#endif
}
#endif





