#ifndef ZCHXCONFIG_H
#define ZCHXCONFIG_H

#include <QSettings>
#include "_def.h"

//配置文件定义
#define         COMMON_SEC                          "Settings"
#define         USE_MODERN_UI5                      "UseModernUI5"
#define         USE_OPEN_GL                         "UseOpenGL"
#define         OPENGL_EXPERT                       "OpenGLExpert"
#define         RASTER_FORMAT                       "RasterFormat"
#define         GLOBAL_TOOLBAR_CONFIG               "GlobalToolbarConfig"
#define         DISTANCE_FORMAT                     "DistanceFormat"
#define         SPEED_FORMAT                        "SpeedFormat"
#define         SHOW_SPEED_UNITS                    "ShowDepthUnits"
#define         DISPLAY_CATEGORY                    "DisplayCategory"
#define         LAST_APPLIED_TEMPLATE               "LastAppliedTemplate"
#define         CONFIG_VERSION_SETTING              "ConfigVersionString"
#define         CMD_SOUND_STRING                    "CmdSoundString"
#define         NAV_MESSAGE_SHOWN                   "NavMessageShown"
#define         INLAND_ECDIS                        "InlandEcdis"
#define         DARK_DECORATION                     "DarkDecorations"
#define         UI_EXPERT                           "UIexpert"
#define         SPACE_DROP_MARK                     "SpaceDropMark"
#define         SHOW_STATUS_BAR                     "ShowStatusBar"
#define         SHOW_MENU_BAR                       "ShowMenuBar"
#define         DEFAULT_FONT_SIZE                   "DefaultFontSize"
#define         FULL_SCREEN                         "FullscreeN"
#define         SHOW_COMPASS_WINDOW                 "ShowCompassWindow"
#define         SET_SYSTEM_TIME                     "SetSystemTime"
#define         SHOW_GRID                           "ShowGrid"
#define         PLAY_SHIP_BELL                      "PlayShipsBells"
#define         SOUND_DEVICE_INDEX                  "SoundDeviceIndex"
#define         FULL_SCREEN_TOOLBAR                 "FullscreenToolbar"
#define         TRANSPARENT_TOOLBAR                 "TransparentToolbar"
#define         PERMANENT_MOB_ICON                  "PermanentMOBIcon"
#define         SHOW_LAYERS                         "ShowLayers"
#define         AUTO_ANCHOR_DROP                    "AutoAnchorDrop"
#define         SHOW_CHART_OUTLINES                 "ShowChartOutlines"
#define         SHOW_ACTIVE_ROUTE_TOTAL             "ShowActiveRouteTotal"
#define         SHOW_ACTIVE_ROUTE_HIGHWAY           "ShowActiveRouteHighway"
#define         SDMM_FORMAT                         "SDMMFormat"
#define         MOST_RECENT_GPS_UPLOAD_CONN         "MostRecentGPSUploadConnection"
#define         SHOW_CHART_BAR                      "ShowChartBar"
#define         GUI_SCALE_FACTOR                    "GUIScaleFactor"
#define         CHART_OBJECT_SCALE_FACTOR           "ChartObjectScaleFactor"
#define         SHIP_SCALE_FACTOR                   "ShipScaleFactor"
#define         FILTER_NMEA_AVG                     "FilterNMEA_Avg"
#define         FILTER_NMEA_SEC                     "FilterNMEA_Sec"
#define         SHOW_TRUE                           "ShowTrue"
#define         SHOW_MAG                            "ShowMag"
#define         USER_MAG_VARIATION                  "UserMagVariation"
#define         CM93_DETAIL_FACTOR                  "CM93DetailFactor"
#define         CM93_DETAIL_ZOOM_POS_X              "CM93DetailZoomPosX"
#define         CM93_DETAIL_ZOOM_POS_Y              "CM93DetailZoomPosY"
#define         SHOW_CM93_DETAIL_SILDER             "ShowCM93DetailSlider"
#define         SKEW_TO_NORTH_UP                    "SkewToNorthUp"
#define         OPEN_GL                             "OpenGL"
#define         SOFTWARE_GL                         "SoftwareGL"
#define         SHOW_FPS                            "ShowFPS"
#define         ZOOM_DETAIL_FACTOR                  "ZoomDetailFactor"
#define         ZOOM_DETAIL_FACTOR_VECTOR           "ZoomDetailFactorVector"
#define         FOG_ON_OVERZOOM                     "FogOnOverzoom"
#define         OVERZOOM_VECTOR_SCALE               "OverzoomVectorScale"
#define         OVERZOOM_ENPHASIS_BASE              "OverzoomEmphasisBase"
#define         USE_ACCELERATED_PANNING             "UseAcceleratedPanning"
#define         GPU_TEXTURE_COMPRESSION             "GPUTextureCompression"
#define         GPU_TEXTURE_COMPRESSION_CACHING     "GPUTextureCompressionCaching"
#define         GPU_TEXTURE_DEMENSION               "GPUTextureDimension"
#define         GPU_TEXTURE_MEMSIZE                 "GPUTextureMemSize"
#define         POLYGON_SMOOTHING                   "PolygonSmoothing"
#define         LINE_SMOOTHING                      "LineSmoothing"
#define         SMOOTH_PAN_ZOOM                     "SmoothPanZoom"
#define         COURSE_UP_MODE                      "CourseUpMode"
#define         LOOK_AHEAD_MODE                     "LookAheadMode"
#define         COG_UP_AVG_SECONDS                  "COGUPAvgSeconds"
#define         USE_MAG_APB                         "UseMagAPB"
#define         OWN_SHIP_COG_PREDICTOR_MIN          "OwnshipCOGPredictorMinutes"
#define         OWM_SHIP_COG_PREDICTOR_WIDTH        "OwnshipCOGPredictorWidth"
#define         OWN_SHIP_HDT_PREDICTOR_MILES        "OwnshipHDTPredictorMiles"
#define         OWN_SHIP_ICON_TYPE                  "OwnShipIconType"
#define         OWN_SHIP_LENGTH                     "OwnShipLength"
#define         OWN_SHIP_WIDTH                      "OwnShipWidth"
#define         OWN_SHIP_GPS_OFFSET_X               "OwnShipGPSOffsetX"
#define         OWN_SHIP_GPS_OFFSET_Y               "OwnShipGPSOffsetY"
#define         OWN_SHIP_MIN_SIZE                   "OwnShipMinSize"
#define         OWN_SHIP_SOG_COG_CALC               "OwnShipSogCogCalc"
#define         OWN_SHIP_SOG_COG_CALC_DAMP_SEC      "OwnShipSogCogCalcDampSec"
#define         ROUTE_ARRIVAL_CIRCLE_RADIUS         "RouteArrivalCircleRadius"
#define         CHART_QUILTING                      "ChartQuilting"
#define         NMEA_LOG_WINDOWS_SIZE_X             "NMEALogWindowSizeX"
#define         NMEA_LOG_WINDOWS_SIZE_Y             "NMEALogWindowSizeY"
#define         NMEA_LOG_WINDOWS_POS_X              "NMEALogWindowPosX"
#define         NMEA_LOG_WINDOWS_POS_Y              "NMEALogWindowPosY"
#define         PRESERVE_SCALE_ON_X                 "PreserveScaleOnX"
#define         START_WITH_TRACK_ACTIVE             "StartWithTrackActive"
#define         AUTOMATIC_DAILY_TRACKS              "AutomaticDailyTracks"
#define         TRACK_ROTATE_AT                     "TrackRotateAt"
#define         TRACK_ROTATE_TIME_TYPE              "TrackRotateTimeType"
#define         HOGHLIGHT_TRACKS                    "HighlightTracks"
#define         INITIAL_STACK_INDEX                 "InitialStackIndex"
#define         INITIAL_DB_INDEX                    "InitialdBIndex"
#define         ACTIVE_CHART_GROUP                  "ActiveChartGroup"
#define         NMEA_APB_PRECISION                  "NMEAAPBPrecision"
#define         TALKER_ID_TEXT                      "TalkerIdText"
#define         ANCHOR_WATCH_1_GUID                 "AnchorWatch1GUID"
#define         ANCHOR_WATCH_2_GUID                 "AnchorWatch2GUID"
#define         I_ENC_TOOLBAR_X                     "iENCToolbarX"
#define         I_ENC_TOOLBAR_Y                     "iENCToolbarY="
#define         SHOW_DEPTH_UNITS                    "ShowDepthUnits"
#define         GPS_IDENT                           "GPSIdent"
#define         USE_GARMIN_HOST_UPLOAD              "UseGarminHostUpload"
#define         MOBILE_TOUCH                        "MobileTouch"
#define         RESPONSIVE_GRAPHICS                 "ResponsiveGraphics"
#define         AUTO_HIDE_TOOLBAR                   "AutoHideToolbar"
#define         AUTO_HIDE_TOOLBAR_SECS              "AutoHideToolbarSecs"
#define         DISPLAY_SIZE_MM                     "DisplaySizeMM"
#define         DISPLAY_SIZE_MANUAL                 "DisplaySizeManual"
#define         SELECTION_RADIUS_MM                 "SelectionRadiusMM"
#define         SELECTION_RADIUS_TOUCH_MM           "SelectionRadiusTouchMM"
#define         PLAN_SPEED                          "PlanSpeed"
#define         LOCALE                              "Locale"
#define         LOCALE_OVERRIDE                     "LocaleOverride"
#define         KEEP_NAV_OBJECT_BACKUP              "KeepNavobjBackups"
#define         LEGACY_INPUT_COM_PORT_FILTER_BEHAVIOUR  "LegacyInputCOMPortFilterBehaviour"
#define         ADVANCE_ROUTE_WAYPOINT_ON_ARRIVAL_ONLY  "AdvanceRouteWaypointOnArrivalOnly"
#define         LIVE_ETA                            "LiveETA"
#define         DEFAULT_BOAT_SPEED                  "DefaultBoatSpeed"
#define         ENABLE_ZOOM_TO_CURSOR               "EnableZoomToCursor"

#define         SETTING_GLOBAL_STATE_SEC            "Settings/GlobalState"
#define         B_SHOW_S57_TEXT                     "BShowS57Text"
#define         B_SHOW_S57_IMPORTANT_TEXT_ONLY      "BShowS57ImportantTextOnly"
#define         N_DISPLAY_CATEGORY                  "nDisplayCategory"
#define         N_SYMBOL_STYLE                      "nSymbolStyle"
#define         N_BOUNDARY_STYLE                    "nBoundaryStyle"
#define         B_SHOW_SOUNDG                       "bShowSoundg"
#define         B_SHOW_META                         "bShowMeta"
#define         B_USE_SCAMIN                        "bUseSCAMIN"
#define         B_SHOW_ATON_TEXT                    "bShowAtonTexT"
#define         B_SHOW_LOGHT_DESCRIPTION            "bShowLightDescription"
#define         B_EXTEND_LIGHT_SECTORS              "bExtendLightSectors"
#define         B_DEL_CLUTTER_TEXT                  "bDeClutterText"
#define         B_SHOW_NATIONAL_TEXT                "bShowNationalText"
#define         S52_MAR_SAFETY_CONTOUR_S              "S52_MAR_SAFETY_CONTOUR"
#define         S52_MAR_SHALLOW_CONTOUR_S             "S52_MAR_SHALLOW_CONTOUR"
#define         S52_MAR_DEEP_CONTOUR_S                "S52_MAR_DEEP_CONTOUR"
#define         S52_MAR_TWO_SHADES_S                  "S52_MAR_TWO_SHADES"
#define         S52_DEPTH_UNIT_SHOW_S                 "S52_DEPTH_UNIT_SHOW"
#define         OWN_SHIO_LAT_LON                    "OwnShipLatLon"
#define         N_COLOR_SCHEME                      "nColorScheme"
#define         FRAMW_WIN_X                         "FrameWinX"
#define         FRAME_WIN_Y                         "FrameWinY"
#define         FRAME_WIN_POS_X                     "FrameWinPosX"
#define         FRAME_WIN_POS_Y                     "FrameWinPosY"
#define         FRAMW_MAX                           "FrameMax"
#define         CLIENT_POS_X                        "ClientPosX"
#define         CLIENT_POS_Y                        "ClientPosY"
#define         CLIENT_SIZE_X                       "ClientSzX"
#define         CLIENT_SIZE_Y                       "ClientSzY"
#define         ROUTE_DROP_SIZE_Z                   "RoutePropSizeX"
#define         ROUTE_DROP_SIZE_Y                   "RoutePropSizeY"
#define         ROUTE_DROP_POS_Y                    "RoutePropPosX"
#define         ROUTE_DROP_POS_Y                    "RoutePropPosY"

#define         WMM_SEC                             "Settings/WMM"
#define         SHOW_ICON                           "ShowIcon"
#define         SHOW_LIVE_ICON                      "ShowLiveIcon"


//#define         OBJECT_FILTER_SEC                   "Settings/ObjectFilter"
//#define         VIZ_ADMARE                          "vizADMARE"
//#define         VIZ_AIRARE                          "vizAIRARE"
//#define
//vizACHBRT=0
//vizACHPNT=0
//vizACHARE=0
//vizBCNCAR=0
//vizBCNISD=0
//vizBCNLAT=0
//vizBCNSAW=0
//vizBCNSPP=0
//vizBERTHS=0
//vizBRIDGE=1
//vizBUISGL=0
//vizBUIREL=0
//vizBUAARE=0
//vizBOYCAR=0
//vizBOYINB=0
//vizBOYISD=0
//vizBOYLAT=0
//vizBOYSAW=0
//vizBOYSPP=0
//vizCBLARE=0
//vizCBLOHD=1
//vizCBLSUB=0
//vizCANALS=1
//vizCANBNK=0
//vizCTSARE=0
//vizCAUSWY=0
//vizCTNARE=0
//vizCHNWIR=0
//vizCHKPNT=0
//vizCGUSTA=0
//vizCOALNE=1
//vizCONZNE=0
//vizCOSARE=0
//vizCTRPNT=0
//vizCONVYR=1
//vizCRANES=0
//vizCURENT=0
//vizCUSZNE=0
//vizDAMCON=1
//vizDAYMAR=0
//vizDWRTCL=1
//vizDWRTPT=0
//vizDEPARE=1
//vizDEPCNT=0
//vizDISMAR=0
//vizDOCARE=1
//vizDRGARE=0
//vizDRYDOC=0
//vizDMPGRD=0
//vizDYKCON=0
//vizEXEZNE=0
//vizFAIRWY=0
//vizFNCLNE=0
//vizFERYRT=0
//vizFSHZNE=0
//vizFSHFAC=0
//vizFSHGRD=0
//vizFLODOC=1
//vizFOGSIG=0
//vizFORSTC=0
//vizFRPARE=0
//vizGATCON=1
//vizGRIDRN=0
//vizHRBARE=0
//vizHRBFAC=0
//vizHULKES=1
//vizICEARE=1
//vizICNARE=0
//vizISTZNE=0
//vizLAKARE=0
//vizLAKSHR=0
//vizLNDARE=1
//vizLNDELV=0
//vizLNDRGN=0
//vizLNDMRK=0
//vizLIGHTS=0
//viz_extgn=0
//vizLITFLT=0
//vizLITVES=0
//vizLOCMAG=0
//vizLOKBSN=1
//vizLOGPON=1
//vizMAGVAR=0
//vizMARCUL=0
//vizMIPARE=0
//vizMONUMT=0
//vizMORFAC=1
//vizNAVLNE=0
//vizOBSTRN=1
//vizOFSPLF=1
//vizOSPARE=0
//vizOILBAR=1
//vizPILPNT=0
//vizPILBOP=0
//vizPIPARE=0
//vizPIPOHD=1
//vizPIPSOL=0
//vizPONTON=1
//vizPRCARE=0
//vizPRDARE=0
//vizPYLONS=1
//vizRADLNE=0
//vizRADRNG=0
//vizRADRFL=0
//vizRADSTA=0
//vizRTPBCN=0
//vizRDOCAL=0
//vizRDOSTA=0
//vizRAILWY=0
//vizRAPIDS=0
//vizRCRTCL=0
//vizRECTRC=0
//vizRCTLPT=0
//vizRSCSTA=0
//vizRESARE=0
//vizRETRFL=0
//vizRIVERS=0
//vizRIVBNK=0
//vizROADWY=0
//vizRUNWAY=0
//vizSNDWAV=0
//vizSEAARE=0
//vizSPLARE=0
//vizSBDARE=0
//vizSLCONS=1
//vizSISTAT=0
//vizSISTAW=0
//vizSILTNK=0
//vizSLOTOP=0
//vizSLOGRD=0
//vizSMCFAC=0
//vizSOUNDG=0
//vizSPRING=0
//vizSQUARE=0
//vizSTSLNE=0
//vizSUBTLN=0
//vizSWPARE=0
//vizTESARE=0
//viz_texto=0
//vizTS_PRH=0
//vizTS_PNH=0
//vizTS_PAD=0
//vizTS_TIS=0
//vizT_HMON=0
//vizT_NHMN=0
//vizT_TIMS=0
//vizTIDEWY=0
//vizTOPMAR=0
//vizTOWERS=0
//vizTSELNE=0
//vizTSSBND=0
//vizTSSCRS=0
//vizTSSLPT=0
//vizTSSRON=0
//vizTSEZNE=0
//vizTUNNEL=0
//vizTWRTPT=0
//vizUWTROC=0
//vizUNSARE=0
//vizVEGATN=0
//vizWATTUR=0
//vizWATFAL=0
//vizWEDKLP=0
//vizWRECKS=0
//vizZEMCNT=0
//vizTS_FEB=0
//vizNEWOBJ=0
//vizM_ACCY=0
//vizM_CSCL=0
//vizM_COVR=0
//vizM_HDAT=0
//vizM_HOPA=0
//vizM_NPUB=0
//vizM_NSYS=0
//vizM_PROD=0
//vizM_QUAL=0
//vizM_SDAT=0
//vizM_SREL=0
//vizM_UNIT=0
//vizM_VDAT=0
//vizC_AGGR=0
//vizC_ASSO=0
//vizC_STAC=0
//viz\$AREAS=1
//viz\$LINES=0
//viz\$CSYMB=0
//viz\$COMPS=0
//viz\$TEXTS=0
//viznotmrk=1
//vizwtwaxs=0
//vizwtwprf=0
//vizbrgare=0
//vizbunsta=0
//vizcomare=0
//vizhrbbsn=1
//vizlokare=0
//vizlkbspt=1
//vizprtare=0
//vizbcnwtw=0
//vizboywtw=0
//vizrefdmp=0
//vizrtplpt=0
//viztermnl=0
//viztrnbsn=0
//vizwtware=0
//vizwtwgag=0
//viztisdge=0
//vizvehtrf=0
//vizexcnst=1
//vizlg_sdm=0
//vizlg_vsp=0
//vizANNOTA=0
//vizRESTRC=0
//vizTRFLNE=0
//vizGENNAV=0
//[PlugIns]
//PluginOrder=
//[PlugIns/chartdldr_pi.dll]
//bEnabled=1
//[PlugIns/wmm_pi.dll]
//bEnabled=1
//[ChartDirectories]
//ChartDir1=G:\\OpenCPN\\build\\Debug\\charts^9856882731248534143
//[Directories]
//S57DataLocation=
//InitChartDir=G:\\OpenCPN\\build\\Debug
//GPXIODir=
//TCDataDir=
//BasemapDir=gshhs\\
//[Canvas]
//CanvasConfig=0
//[Canvas/CanvasConfig1]
//canvasVPLatLon="   35.7238,  126.5989"
//canvasVPScale=0.000288115
//canvasVPRotation=0
//canvasInitialdBIndex=0
//canvasbFollow=0
//ActiveChartGroup=0
//canvasToolbarConfig=
//canvasShowToolbar=0
//canvasQuilt=1
//canvasShowGrid=0
//canvasShowOutlines=1
//canvasShowDepthUnits=1
//canvasShowAIS=1
//canvasAttenAIS=0
//canvasShowTides=0
//canvasShowCurrents=0
//canvasShowENCText=0
//canvasENCDisplayCategory=83
//canvasENCShowDepths=1
//canvasENCShowBuoyLabels=1
//canvasENCShowLightDescriptions=0
//canvasENCShowLights=1
//canvasCourseUp=0
//canvasLookahead=0
//canvasSizeX=1326
//canvasSizeY=663

class zchxConfig : public QSettings
{
    Q_OBJECT
public:
    zchxConfig(const QString &LocalFileName);
    void setDefault(const QString & prefix,const QString &key, const QVariant &value);
    void setCustomValue(const QString & prefix,const QString & key, const QVariant & value);
    QVariant getCustomValue(const QString& prefix,const QString &keys, const QVariant &defaultValue = QVariant());
    int  getChildCount(const QString& prefix);
    QStringList getChildKeys(const QString& prefix);
    void LoadS57Config();


    //    int LoadMyConfig();
    //
    //    void LoadNavObjects();
    //    virtual void LoadCanvasConfigs( bool bApplyAsTemplate = false );
    //    virtual void LoadConfigCanvas( canvasConfig *cConfig, bool bApplyAsTemplate );
    //    virtual void SaveCanvasConfigs( );
    //    virtual void SaveConfigCanvas( canvasConfig *cc );

    //    virtual bool UpdateChartDirs(ArrayOfCDI &dirarray);
    //    virtual bool LoadChartDirArray(ArrayOfCDI &ChartDirArray);
    //    virtual void UpdateSettings();
    //    bool LoadLayers(QString &path);
    //    int LoadMyConfigRaw( bool bAsTemplate = false );
    //    virtual void UpdateNavObj();
private:
    void                    initDefaultValue();
    bool                    m_bSkipChangeSetUpdate;
};

#endif // ZCHXCONFIG_H
