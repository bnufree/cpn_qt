#include "_def.h"
#include "zchxmapmainwindow.h"
#include "GL/gl.h"
#include <windows.h>
#include <psapi.h>
#include <QDebug>
#include "styles.h"
#include "s52plib.h"
#include "chcanv.h"
#include "CanvasConfig.h"
#include "FontMgr.h"
#include "OCPNPlatform.h"
#include "chartdb.h"

 OCPNPlatform                   *g_Platform = NULL;
 zchxMapMainWindow              *gFrame = NULL;

 double                         g_ChartNotRenderScaleFactor;
 int                            g_restore_stackindex;
 int                            g_restore_dbindex;
 int              g_LayerIdx;
 ArrayOfCDI       g_ChartDirArray;
 double           vLat, vLon, gLat, gLon;
 double           kLat, kLon;
 double           initial_scale_ppm, initial_rotation;
 ColorScheme      global_color_scheme = GLOBAL_COLOR_SCHEME_DAY;
 int              g_nbrightness = 100;
 bool             g_bShowTrue, g_bShowMag;
 double           g_UserVar;
 bool             g_bShowStatusBar;
 bool             g_bUIexpert;
 bool             g_bFullscreen;
 int              g_nDepthUnitDisplay;
 QString         g_csv_locn;
 QString         g_SENCPrefix;
 QString         g_UserPresLibData;
 QString         *pInit_Chart_Dir = 0;
 QString         gWorldMapLocation;
 bool             s_bSetSystemTime;
 bool             g_bDisplayGrid;         //Flag indicating if grid is to be displayed
 bool             g_bPlayShipsBells;
 int              g_iSoundDeviceIndex;
 bool             g_bFullscreenToolbar;
 bool             g_bShowLayers;
 bool             g_bTransparentToolbar;
 bool             g_bPermanentMOBIcon;

 bool             g_bShowDepthUnits;
 bool             g_bAutoAnchorMark;
 bool             g_bskew_comp;
 bool             g_bopengl;
 bool             g_bdisable_opengl;
 bool             g_bSoftwareGL;
 bool             g_bShowFPS;
 bool             g_bsmoothpanzoom;
 bool             g_fog_overzoom;
 double           g_overzoom_emphasis_base;
 bool             g_oz_vector_scale;

 bool             g_bShowOutlines;
 bool             g_bShowActiveRouteHighway;
 bool             g_bShowRouteTotal;
 int              g_nNMEADebug;
 int              g_nAWDefault;
 int              g_nAWMax;
 int              g_nTrackPrecision;

 int              g_iSDMMFormat;
 int              g_iDistanceFormat;
 int              g_iSpeedFormat;

 int              g_nframewin_x;
 int              g_nframewin_y;
 int              g_nframewin_posx;
 int              g_nframewin_posy;
 bool             g_bframemax;

 double           g_PlanSpeed;
 QString         g_VisibleLayers;
 QString         g_InvisibleLayers;
 QString         g_VisiNameinLayers;
 QString         g_InVisiNameinLayers;
 QRect           g_blink_rect;

 QStringList    *pMessageOnceArray;

// LIVE ETA OPTION
 bool             g_bShowLiveETA;
 double           g_defaultBoatSpeed;
 double           g_defaultBoatSpeedUserUnit;

//    AIS Global configuration
 bool             g_bCPAMax;
 double           g_CPAMax_NM;
 bool             g_bCPAWarn;
 double           g_CPAWarn_NM;
 bool             g_bTCPA_Max;
 double           g_TCPA_Max;
 bool             g_bMarkLost;
 double           g_MarkLost_Mins;
 bool             g_bRemoveLost;
 double           g_RemoveLost_Mins;
 bool             g_bShowCOG;
 double           g_ShowCOG_Mins;
 bool             g_bAISShowTracks;
 bool             g_bTrackCarryOver;
 bool             g_bTrackDaily;
 int              g_track_rotate_time;
 int              g_track_rotate_time_type;
 double           g_AISShowTracks_Mins;
 double           g_AISShowTracks_Limit;
 bool             g_bHideMoored;
 double           g_ShowMoored_Kts;
 bool             g_bAllowShowScaled;
 bool             g_bShowScaled;
 int              g_ShowScaled_Num;
 bool             g_bAIS_CPA_Alert;
 bool             g_bAIS_CPA_Alert_Audio;
 int              g_ais_alert_dialog_x, g_ais_alert_dialog_y;
 int              g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;
 int              g_ais_query_dialog_x, g_ais_query_dialog_y;
 QString         g_sAIS_Alert_Sound_File;
 bool             g_bAIS_CPA_Alert_Suppress_Moored;
 bool             g_bAIS_ACK_Timeout;
 double           g_AckTimeout_Mins;
 QString         g_AisTargetList_perspective;
 int              g_AisTargetList_range;
 int              g_AisTargetList_sortColumn;
 bool             g_bAisTargetList_sortReverse;
 QString         g_AisTargetList_column_spec;
 QString         g_AisTargetList_column_order;
 bool             g_bShowAreaNotices;
 bool             g_bDrawAISSize;
 bool             g_bShowAISName;
 int              g_Show_Target_Name_Scale;
 bool             g_bWplIsAprsPosition;
 bool             g_benableAISNameCache;
 bool             g_bUseOnlyConfirmedAISName;
 int              g_ScaledNumWeightSOG;
 int              g_ScaledNumWeightCPA;
 int              g_ScaledNumWeightTCPA;
 int              g_ScaledNumWeightRange;
 int              g_ScaledNumWeightSizeOfT;
 int              g_ScaledSizeMinimal;

 int              g_S57_dialog_sx, g_S57_dialog_sy;
int                     g_S57_extradialog_sx, g_S57_extradialog_sy;

 int              g_iNavAidRadarRingsNumberVisible;
 float            g_fNavAidRadarRingsStep;
 int              g_pNavAidRadarRingsStepUnits;
 int              g_iWaypointRangeRingsNumber;
 float            g_fWaypointRangeRingsStep;
 int              g_iWaypointRangeRingsStepUnits;
 QColor         g_colourWaypointRangeRingsColour;
 bool             g_bWayPointPreventDragging;
 bool             g_bConfirmObjectDelete;
 QColor         g_colourOwnshipRangeRingsColour;
 int              g_iWpt_ScaMin;
 bool             g_bUseWptScaMin;
 bool             g_bOverruleScaMin;
 bool             g_bShowWptName;


 bool             g_bEnableZoomToCursor;
 QString         g_toolbarConfig;
 QString         g_toolbarConfigSecondary;
 double           g_TrackIntervalSeconds;
 double           g_TrackDeltaDistance;
 int              gps_watchdog_timeout_ticks;

 int              g_nCacheLimit;
 int              g_memCacheLimit;

 bool             g_bGDAL_Debug;
 bool             g_bDebugCM93;
 bool             g_bDebugS57;

 double           g_ownship_predictor_minutes;
 double           g_ownship_HDTpredictor_miles;

 bool             g_own_ship_sog_cog_calc;
 int              g_own_ship_sog_cog_calc_damp_sec;

 bool             g_bShowMenuBar;
 bool             g_bShowCompassWin;

 s52plib          *ps52plib = 0;

 int              g_cm93_zoom_factor;
 bool             g_b_legacy_input_filter_behaviour;
 bool             g_bShowDetailSlider;
 int              g_detailslider_dialog_x, g_detailslider_dialog_y;

 bool             g_bUseGreenShip;

 bool             g_b_overzoom_x = true;                      // Allow high overzoom
 int              g_OwnShipIconType;
 double           g_n_ownship_length_meters;
 double           g_n_ownship_beam_meters;
 double           g_n_gps_antenna_offset_y;
 double           g_n_gps_antenna_offset_x;
 int              g_n_ownship_min_mm;
 double           g_n_arrival_circle_radius;

 bool             g_bPreserveScaleOnX;
 bool             g_bsimplifiedScalebar;

 bool             g_bUseRMC;
 bool             g_bUseGLL;

 QString         g_locale;
 QString         g_localeOverride;

 bool             g_bUseRaster;
 bool             g_bUseVector;
 bool             g_bUseCM93;

 bool             g_bCourseUp;
 bool             g_bLookAhead;
 int              g_COGAvgSec = 15;
 bool             g_bMagneticAPB;
 bool             g_bShowChartBar;

 int              g_MemFootSec;
 int              g_MemFootMB;

 int              g_nCOMPortCheck;

 bool             g_bbigred;

 QString         g_AW1GUID;
 QString         g_AW2GUID;
 int              g_BSBImgDebug;

 int             n_NavMessageShown;
 QString        g_config_version_string;

 QString        g_CmdSoundString;

 bool             g_bAISRolloverShowClass;
 bool             g_bAISRolloverShowCOG;
 bool             g_bAISRolloverShowCPA;

 bool             g_bDebugGPSD;

 bool             g_bfilter_cogsog;
 int              g_COGFilterSec;
 int              g_SOGFilterSec;

int                     g_navobjbackups;

 bool             g_bQuiltEnable;
 bool             g_bFullScreenQuilt = true;
 bool             g_bQuiltStart;

 int              g_SkewCompUpdatePeriod;

 int              g_maintoolbar_x;
 int              g_maintoolbar_y;
 long             g_maintoolbar_orient;

 int              g_GPU_MemSize;

 int              g_lastClientRectx;
 int              g_lastClientRecty;
 int              g_lastClientRectw;
 int              g_lastClientRecth;

 bool             g_bHighliteTracks;
 int              g_cog_predictor_width;
 int              g_ais_cog_predictor_width;

 int              g_route_line_width;
 int              g_track_line_width;
 QColor         g_colourTrackLineColour;
 QString         g_default_wp_icon;
 QString         g_default_routepoint_icon;

 ChartGroupArray  *g_pGroupArray = NULL;
 int              g_GroupIndex;

 bool             g_bDebugOGL;
 int              g_tcwin_scale;
 QString         g_GPS_Ident;
 bool             g_bGarminHostUpload;
 QString         g_uploadConnection;

 ocpnStyle::StyleManager* g_StyleManager = NULL;
 QStringList    TideCurrentDataSet;
 QString         g_TCData_Dir;

 bool             g_btouch;
 bool             g_bresponsive;

 bool             bGPSValid;              // for track recording
 bool             g_bGLexpert;

 int              g_SENC_LOD_pixels;

 int              g_chart_zoom_modifier;
 int              g_chart_zoom_modifier_vector;

 int              g_NMEAAPBPrecision;

 QString         g_TalkerIdText;
 int              g_maxWPNameLength;

 bool             g_bAdvanceRouteWaypointOnArrivalOnly;
 double           g_display_size_mm;
 double           g_config_display_size_mm;
 bool             g_config_display_size_manual;

 float            g_selection_radius_mm = 2.0;
 float            g_selection_radius_touch_mm = 10.0;

 bool             g_benable_rotate;
 bool             g_bEmailCrashReport;

 int              g_default_font_size;

 bool             g_bAutoHideToolbar;
 int              g_nAutoHideToolbar;
 int              g_GUIScaleFactor;
 int              g_ChartScaleFactor;
 float            g_ChartScaleFactorExp;
 int              g_ShipScaleFactor;
 float            g_ShipScaleFactorExp;

 bool             g_bInlandEcdis;
 int              g_iENCToolbarPosX;
 int              g_iENCToolbarPosY;

 bool             g_bSpaceDropMark;

 bool             g_bShowTide;
 bool             g_bShowCurrent;

 bool             g_benableUDPNullHeader;

 QString         g_uiStyle;
 bool             g_useMUI;

 int                     g_nCPUCount;

 bool             g_bDarkDecorations;
 unsigned int     g_canvasConfig;
 arrayofCanvasConfigPtr g_canvasConfigArray;
 QString         g_lastAppliedTemplateGUID;

 int              g_route_prop_x, g_route_prop_y;
 int              g_route_prop_sx, g_route_prop_sy;

QString                g_gpx_path;
bool                    g_bLayersLoaded;
 zchxGLOptions g_GLOptions;
 ChartDB                   *ChartData = NULL;
 QThread                   *g_Main_thread = 0;
 float                     g_compass_scalefactor;
int                Usercolortable_index;
bool                g_bOpenGL = true;
bool                                g_bGlExpert = false;
float                     g_toolbar_scalefactor;
bool                    g_bFirstRun;
bool                     g_bUpgradeInProcess;
QString s_locale;
 bool             g_bcompression_wait;
 bool               g_bquiting;
ChartCanvas      *g_focusCanvas = 0;
 ChartCanvas      *g_overlayCanvas = 0;

  bool g_b_EnableVBO = false;
  GLenum  g_texture_rectangle_format;


double zchxFuncUtil::m_pt_per_pixel = 0.0;

bool zchxFuncUtil::isDirExist(const QString& name)
{
    QDir dir(name);
    return dir.exists();
}

bool zchxFuncUtil::isFileExist(const QString& name)
{
    return QFile::exists(name);
}

QTextCodec* zchxFuncUtil::codesOfName(const QString& name)
{
    return QTextCodec::codecForName(name.toLatin1().data());
}

QString zchxFuncUtil::convertCodesStringToUtf8(const char* str, const QString& codes)
{
    QTextCodec *src_codes = codesOfName(codes);
    QTextCodec *utf8 = codesOfName("UTF-8");
    QString unicode = src_codes->toUnicode(str);
    return QString::fromUtf8(utf8->fromUnicode(unicode));
}

bool zchxFuncUtil::isSameAs(const QString& p1, const QString& p2, bool caseSensitive )
{
    Qt::CaseSensitivity val = (caseSensitive == true ?  Qt::CaseSensitive : Qt::CaseInsensitive);
    return QString::compare(p1, p2, val) == 0;
}

bool zchxFuncUtil::renameFileExt(QString& newPath, const QString& oldFile, const QString& newExt)
{
    QFile file(oldFile);
    if(file.exists())
    {
        int last_index = oldFile.lastIndexOf(".");
        if(last_index >= 0)
        {
            QString ext = oldFile.mid(last_index+1);
            QString newname = oldFile;
            newname.replace(last_index+1, ext.size(), newExt);
            if(file.rename(newname))
            {
                newPath = newname;
                return true;
            }
        }
    }

    return false;
}

QString zchxFuncUtil::getNewFileNameWithExt(const QString& oldName, const QString& newExt)
{
    int last_index = oldName.lastIndexOf(".");
    if(last_index >= 0)
    {
        QString ext = oldName.mid(last_index+1);
        QString newname = oldName;
        newname.replace(last_index+1, ext.size(), newExt);
        return newname;
    }
    QString temp = oldName;
    return temp.append(".").append(newExt);
}


QString zchxFuncUtil::getFileName(const QString& fullName)
{
    int index = fullName.lastIndexOf(".");
    return fullName.left(index);
}

QString zchxFuncUtil::getFileExt(const QString& fullname)
{
    int index = fullname.lastIndexOf(".");
    return fullname.mid(index+1);
}

QString zchxFuncUtil::getTempDir()
{
    QString path = QApplication::applicationDirPath();
    if(path.right(1) != zchxFuncUtil::separator()) path.append(zchxFuncUtil::separator());
    QString temp_path = QString("%1__temp").arg(path);
    QDir dir(temp_path);
    if(!dir.exists()) dir.mkpath(temp_path);
    return temp_path;
}

bool zchxFuncUtil::isImageTransparent(const QImage& img, int x, int y, int alpha)
{
    QColor color = img.pixelColor(x, y);
    if(color.alpha() <= alpha) return true;
    return false;
}

QColor zchxFuncUtil::getNewRGBColor(const QColor& rgb, double level)
{
    QColor hsv = rgb.toHsv();
    int h = hsv.hsvHue();
    int s = hsv.hsvSaturation();
    int v = hsv.value();
    int new_v = (int)( v * level);
    hsv.setHsv(h, s, new_v);
    return hsv.toRgb();
}

bool zchxFuncUtil::IsLeftMouseDown(QMouseEvent* e)
{
    if(e->type() != QEvent::MouseButtonPress) return false;
    if(e->button() != Qt::LeftButton) return false;
    return true;
}

bool zchxFuncUtil::IsLeftMouseUp(QMouseEvent* e)
{
    if(e->type() != QEvent::MouseButtonRelease) return false;
    if(e->button() != Qt::LeftButton) return false;
    return true;
}

bool zchxFuncUtil::IsRightMouseDown(QMouseEvent* e)
{
    if(e->type() != QEvent::MouseButtonPress) return false;
    if(e->button() != Qt::RightButton) return false;
    return true;
}

bool zchxFuncUtil::IsRightMouseUp(QMouseEvent* e)
{
    if(e->type() != QEvent::MouseButtonRelease) return false;
    if(e->button() != Qt::RightButton) return false;
    return true;
}

bool zchxFuncUtil::IsMouseUp(QMouseEvent* e)
{
    if(e->type() != QEvent::MouseButtonRelease) return false;
    return true;
}

double zchxFuncUtil::toUsrDistance( double nm_distance, int unit  )
{
    double ret = NAN;
    if ( unit == -1 )
        unit = g_iDistanceFormat;
    switch( unit ){
    case DISTANCE_NMI: //Nautical miles
        ret = nm_distance;
        break;
    case DISTANCE_MI: //Statute miles
        ret = nm_distance * 1.15078;
        break;
    case DISTANCE_KM:
        ret = nm_distance * 1.852;
        break;
    case DISTANCE_M:
        ret = nm_distance * 1852;
        break;
    case DISTANCE_FT:
        ret = nm_distance * 6076.12;
        break;
    case DISTANCE_FA:
        ret = nm_distance * 1012.68591;
        break;
    case DISTANCE_IN:
        ret = nm_distance * 72913.4;
        break;
    case DISTANCE_CM:
        ret = nm_distance * 185200;
        break;
    }
    return ret;
}

QString zchxFuncUtil::getUsrDistanceUnit( int unit )
{
    QString ret;
    if ( unit == -1 )
        unit = g_iDistanceFormat;
    switch( unit ){
        case DISTANCE_NMI: //Nautical miles
            ret = ("NMi");
            break;
        case DISTANCE_MI: //Statute miles
            ret = ("mi");
            break;
        case DISTANCE_KM:
            ret = ("km");
            break;
        case DISTANCE_M:
            ret = ("m");
            break;
        case DISTANCE_FT:
            ret = ("ft");
            break;
        case DISTANCE_FA:
            ret = ("fa");
            break;
        case DISTANCE_IN:
            ret = ("in");
        break;
        case DISTANCE_CM:
            ret = ("cm");
            break;
    }
    return ret;
}

double zchxFuncUtil::fromUsrDistance( double usr_distance, int unit )
{
    double ret = NAN;
    if ( unit == -1 )
        unit =  g_iDistanceFormat;
    switch( unit ){
        case DISTANCE_NMI: //Nautical miles
            ret = usr_distance;
            break;
        case DISTANCE_MI: //Statute miles
            ret = usr_distance / 1.15078;
            break;
        case DISTANCE_KM:
            ret = usr_distance / 1.852;
            break;
        case DISTANCE_M:
            ret = usr_distance / 1852;
            break;
        case DISTANCE_FT:
            ret = usr_distance / 6076.12;
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Converts the speed to the units selected by user              */
/**************************************************************************/
double zchxFuncUtil::toUsrSpeed( double kts_speed, int unit )
{
    double ret = NAN;
    if ( unit == -1 )
        unit = g_iSpeedFormat;
    switch( unit )
    {
        case SPEED_KTS: //kts
            ret = kts_speed;
            break;
        case SPEED_MPH: //mph
            ret = kts_speed * 1.15078;
            break;
        case SPEED_KMH: //km/h
            ret = kts_speed * 1.852;
            break;
        case SPEED_MS: //m/s
            ret = kts_speed * 0.514444444;
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Converts the speed from the units selected by user to knots   */
/**************************************************************************/
double fromUsrSpeed( double usr_speed, int unit  )
{
    double ret = NAN;
    if ( unit == -1 )
        unit = g_iSpeedFormat;
    switch( unit )
    {
        case SPEED_KTS: //kts
            ret = usr_speed;
            break;
        case SPEED_MPH: //mph
            ret = usr_speed / 1.15078;
            break;
        case SPEED_KMH: //km/h
            ret = usr_speed / 1.852;
            break;
        case SPEED_MS: //m/s
            ret = usr_speed / 0.514444444;
            break;
    }
    return ret;
}

QString zchxFuncUtil::getUsrSpeedUnit( int unit  )
{
    QString ret;
    if ( unit == -1 )
        unit = g_iSpeedFormat;
    switch( unit ){
        case SPEED_KTS: //kts
            ret = ("kts");
            break;
        case SPEED_MPH: //mph
            ret = ("mph");
            break;
        case SPEED_KMH:
            ret = ("km/h");
            break;
        case SPEED_MS:
            ret = ("m/s");
            break;
    }
    return ret;
}


QString zchxFuncUtil::FormatDistanceAdaptive( double distance ) {
    QString result;
    int unit = g_iDistanceFormat;
    double usrDistance = toUsrDistance( distance, unit);
    if( usrDistance < 0.1 &&
      ( unit == DISTANCE_KM || unit == DISTANCE_MI || unit == DISTANCE_NMI ) ) {
        unit = ( unit == DISTANCE_MI ) ? DISTANCE_FT : DISTANCE_M;
        usrDistance = toUsrDistance( distance, unit );
    }
    QString format;
    if( usrDistance < 5.0 ) {
        format = ("%1.2f ");
    } else if( usrDistance < 100.0 ) {
        format = ("%2.1f ");
    } else if( usrDistance < 1000.0 ) {
        format = ("%3.0f ");
    } else {
        format = ("%4.0f ");
    }
    result.append(QString("").sprintf(format.toUtf8().data(), usrDistance ))
            .append(getUsrDistanceUnit( unit ));
    return result;
}

/**************************************************************************/
/*          Formats the coordinates to string                             */
/**************************************************************************/
QString zchxFuncUtil::toSDMM( int NEflag, double a,  bool hi_precision )
{
    QString s;
    double mpy;
    short neg = 0;
    int d;
    long m;
    double ang = a;
    char c = 'N';

    if( a < 0.0 ) {
        a = -a;
        neg = 1;
    }
    d = (int) a;
    if( neg ) d = -d;
    if( NEflag ) {
        if( NEflag == 1 ) {
            c = 'N';

            if( neg ) {
                d = -d;
                c = 'S';
            }
        } else
            if( NEflag == 2 ) {
                c = 'E';

                if( neg ) {
                    d = -d;
                    c = 'W';
                }
            }
    }

    switch( g_iSDMMFormat ){
        case 0:
            mpy = 600.0;
            if( hi_precision ) mpy = mpy * 1000;

            m = (long) qRound( ( a - (double) d ) * mpy );

            if( !NEflag || NEflag < 1 || NEflag > 2 ) //Does it EVER happen?
                    {
                if( hi_precision ) s.sprintf( ( "%d\u00B0 %02ld.%04ld'" ), d, m / 10000, m % 10000 );
                else
                    s.sprintf( ( "%d\u00B0 %02ld.%01ld'" ), d, m / 10, m % 10 );
            } else {
                if( hi_precision )
                    if (NEflag == 1)
                        s.sprintf( ( "%02d\u00B0 %02ld.%04ld' %c" ), d, m / 10000, ( m % 10000 ), c );
                    else
                        s.sprintf(( "%03d\u00B0 %02ld.%04ld' %c" ), d, m / 10000, ( m % 10000 ), c );
                else
                    if (NEflag == 1)
                        s.sprintf( ( "%02d\u00B0 %02ld.%01ld' %c" ), d, m / 10, ( m % 10 ), c );
                    else
                        s.sprintf(( "%03d\u00B0 %02ld.%01ld' %c" ), d, m / 10, ( m % 10 ), c );
            }
            break;
        case 1:
            if( hi_precision ) s.sprintf( ( "%03.6f" ), ang ); //cca 11 cm - the GPX precision is higher, but as we use hi_precision almost everywhere it would be a little too much....
            else
                s.sprintf(( "%03.4f" ), ang ); //cca 11m
            break;
        case 2:
            m = (long) ( ( a - (double) d ) * 60 );
            mpy = 10.0;
            if( hi_precision ) mpy = mpy * 100;
            long sec = (long) ( ( a - (double) d - ( ( (double) m ) / 60 ) ) * 3600 * mpy );

            if( !NEflag || NEflag < 1 || NEflag > 2 ) //Does it EVER happen?
                    {
                if( hi_precision ) s.sprintf(( "%d\u00B0 %ld'%ld.%ld\"" ), d, m, sec / 1000,
                        sec % 1000 );
                else
                    s.sprintf( ( "%d\u00B0 %ld'%ld.%ld\"" ), d, m, sec / 10, sec % 10 );
            } else {
                if( hi_precision )
                    if (NEflag == 1)
                        s.sprintf( ( "%02d\u00B0 %02ld' %02ld.%03ld\" %c" ), d, m, sec / 1000, sec % 1000, c );
                    else
                        s.sprintf(( "%03d\u00B0 %02ld' %02ld.%03ld\" %c" ), d, m, sec / 1000, sec % 1000, c );
                else
                    if (NEflag == 1)
                        s.sprintf(( "%02d\u00B0 %02ld' %02ld.%ld\" %c" ), d, m, sec / 10, sec % 10, c );
                    else
                        s.sprintf(( "%03d\u00B0 %02ld' %02ld.%ld\" %c" ), d, m, sec / 10, sec % 10, c );
            }
            break;
    }
    return s;
}


double zchxFuncUtil::fromDMM( QString sdms )
{
    wchar_t buf[64];
    char narrowbuf[64];
    int i, len, top = 0;
    double stk[32], sign = 1;

    //First round of string modifications to accomodate some known strange formats
    QString replhelper = QString::fromUtf8("´·" ); //UKHO PDFs
    sdms.replace( replhelper, (".") );
    replhelper = QString::fromUtf8("\"·" ); //Don't know if used, but to make sure
    sdms.replace( replhelper, (".") );
    replhelper = QString::fromUtf8( "·" );
    sdms.replace( replhelper, (".") );

    replhelper = QString::fromUtf8("s. š." ); //Another example: cs.wikipedia.org (someone was too active translating...)
    sdms.replace( replhelper, ("N") );
    replhelper = QString::fromUtf8( "j. š." );
    sdms.replace( replhelper, ("S") );
    sdms.replace( ("v. d."), ("E") );
    sdms.replace( ("z. d."), ("W") );

    //If the string contains hemisphere specified by a letter, then '-' is for sure a separator...
    sdms = sdms.toUpper();
    if( sdms.contains(("N") ) || sdms.contains(("S") ) || sdms.contains( ("E") )
            || sdms.contains( ("W") ) ) sdms.replace( ("-"), (" ") );

    wcsncpy( buf, sdms.toStdWString().data(), 63 );
    buf[63] = 0;
    len = qMin( wcslen( buf ), sizeof(narrowbuf)-1);;

    for( i = 0; i < len; i++ ) {
        wchar_t c = buf[i];
        if( ( c >= '0' && c <= '9' ) || c == '-' || c == '.' || c == '+' ) {
            narrowbuf[i] = c;
            continue; /* Digit characters are cool as is */
        }
        if( c == ',' ) {
            narrowbuf[i] = '.'; /* convert to decimal dot */
            continue;
        }
        if( ( c | 32 ) == 'w' || ( c | 32 ) == 's' ) sign = -1; /* These mean "negate" (note case insensitivity) */
        narrowbuf[i] = 0; /* Replace everything else with nuls */
    }

    /* Build a stack of doubles */
    stk[0] = stk[1] = stk[2] = 0;
    for( i = 0; i < len; i++ ) {
        while( i < len && narrowbuf[i] == 0 )
            i++;
        if( i != len ) {
            stk[top++] = atof( narrowbuf + i );
            i += strlen( narrowbuf + i );
        }
    }

    return sign * ( stk[0] + ( stk[1] + stk[2] / 60 ) / 60 );
}

// int g_iDistanceFormat;
// int g_iSpeedFormat;
// bool g_bShowMag;
// bool g_bShowTrue;
// zchxMapMainWindow *gFrame;
// int g_iSDMMFormat;

QString zchxFuncUtil::formatAngle(double angle, double mag, bool show_mag, bool show_true)
{
    QString out;
    if( show_mag && show_true ) {
        out.sprintf("%.0f \u00B0T (%.0f \u00B0M)", angle, /*gFrame->GetMag(angle)*/mag);
    } else if( show_true ) {
        out.sprintf("%.0f \u00B0T", angle);
    } else {
        out.sprintf("%.0f \u00B0M", /*gFrame->GetMag(angle)*/mag);
    }
    return out;
}

/* render a rectangle at a given color and transparency */
void zchxFuncUtil::AlphaBlending(int x, int y, int size_x, int size_y,
                                 float radius, QColor color,
                                    unsigned char transparency )
{

    glEnable( GL_BLEND );
    glColor4ub( color.red(), color.green(), color.blue(), transparency );
    glBegin( GL_QUADS );
    glVertex2i( x, y );
    glVertex2i( x + size_x, y );
    glVertex2i( x + size_x, y + size_y );
    glVertex2i( x, y + size_y );
    glEnd();
    glDisable( GL_BLEND );
}


qint64 zchxFuncUtil::getProcessIDFromSystem()
{
    return GetCurrentProcessId();
}

qint64 zchxFuncUtil::getApplicationMemoryUse( )
{
    int memsize = -1;
    HANDLE hProcess = GetCurrentProcess();
    if( NULL == hProcess ) return 0;
    PROCESS_MEMORY_COUNTERS pmc;

    if( GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) ) {
        memsize = pmc.WorkingSetSize / 1024;
    }

    CloseHandle( hProcess );
    return memsize;
}

void  zchxFuncUtil::getMemoryStatus(int* total, int* used)
{
    int mem_used = getApplicationMemoryUse();
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof( statex );
    GlobalMemoryStatusEx( &statex );
    int mem_total = statex.ullTotalPhys / 1024;
    if(total) *total = mem_total;
    if(used) *used = mem_used;
//    qDebug()<<"memory total:"<<mem_total<<"  app used:"<<mem_used;
}


QString zchxFuncUtil::getAppDir()
{
    return QApplication::applicationDirPath();
}

QString zchxFuncUtil::getDataDir()
{
    QString data_dir = QString("%1/map_data").arg(getAppDir());
    QDir dir(data_dir);
    if(!dir.exists()) dir.mkpath(data_dir);
    return data_dir;
}

QString zchxFuncUtil::separator()
{
    return "/";
}



QString zchxFuncUtil::getPluginDir()
{
    QString plugin_dir = QString("%1/plugin").arg(getDataDir());
    QDir dir(plugin_dir);
    if(!dir.exists()) dir.mkpath(plugin_dir);
    return plugin_dir;
}


QString zchxFuncUtil::getConfigFileName()
{
    return QString("%1/opencpn.ini").arg(getDataDir());
}

float zchxFuncUtil::getChartScaleFactorExp( float scale_linear )
{
    double factor = 1.0;
#ifndef __OCPN__ANDROID__
    factor =  exp( scale_linear * (log(3.0) / 5.0) );

#else
    // the idea here is to amplify the scale factor for higher density displays, in a measured way....
    factor =  exp( scale_linear * (0.693 / 5.0) * getAndroidDisplayDensity());
#endif

    factor = fmax(factor, .5);
    factor = fmin(factor, 4.);


    return factor;
}

double zchxFuncUtil::getFontPointsperPixel( void )
{
    if(m_pt_per_pixel == 0)
    {
        QFont f = FontMgr::Get().FindOrCreateFont( 12, "Microsoft YaHei", QFont::StyleNormal, QFont::Bold, false);
        double width = QFontMetricsF(f).width("H");
        double height = QFontMetricsF(f).height();
        if(height > 0) m_pt_per_pixel = 12.0 / height;
    }
    return m_pt_per_pixel;

}





