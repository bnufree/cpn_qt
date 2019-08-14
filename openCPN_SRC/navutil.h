/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navigation Utility Functions
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#ifndef __NAVUTIL__
#define __NAVUTIL__


#ifdef __WXMSW__
#include <wx/msw/regconf.h>
#include <wx/msw/iniconf.h>
#endif

#include "bbox.h"
//#include "chcanv.h"
#include "chartdbs.h"
//nclude "RoutePoint.h"
#include "vector2D.h"
#include "SelectItem.h"
#include "ocpndc.h"
//#include "RoutePoint.h"
//#include "Track.h"
//#include "Route.h"

enum
{
    DISTANCE_NMI = 0,
    DISTANCE_MI,
    DISTANCE_KM,
    DISTANCE_M,
    DISTANCE_FT,
    DISTANCE_FA,
    DISTANCE_IN,
    DISTANCE_CM
};

enum
{
    SPEED_KTS = 0,
    SPEED_MPH,
    SPEED_KMH,
    SPEED_MS
};

struct zchxTimeSpan{
public:
    zchxTimeSpan(qint64 secs = 0) {mSecs = secs;}


    qint64 mSecs;
};

extern bool LogMessageOnce(const QString &msg);
extern double toUsrDistance( double nm_distance, int unit = -1 );
extern double fromUsrDistance( double usr_distance, int unit = -1 );
extern double toUsrSpeed( double kts_speed, int unit = -1 );
extern double fromUsrSpeed( double usr_speed, int unit = -1 );
extern QString getUsrDistanceUnit( int unit = -1 );
extern QString getUsrSpeedUnit( int unit = -1 );
extern QString toSDMM(int NEflag, double a, bool hi_precision = true);
extern QString FormatDistanceAdaptive( double distance );
extern QString formatTimeDelta(zchxTimeSpan span);
extern QString formatTimeDelta(QDateTime startTime, QDateTime endTime);
extern QString formatTimeDelta(qint64 secs);
extern QString formatAngle(double angle);

extern void AlphaBlending( ocpnDC& dc, int x, int y, int size_x, int size_y, float radius,
                                       QColor color, unsigned char transparency );

    //Central dimmer...
void DimeControl(QWidget* ctrl);
void DimeControl(QWidget* ctrl, QColor col, QColor col1, QColor back_color,QColor text_color,QColor uitext, QColor udkrd, QColor gridline);

extern double fromDMM(QString sdms);

class Route;
class NavObjectCollection;
class wxGenericProgressDialog;
class ocpnDC;
class NavObjectCollection1;
class NavObjectChanges;
class TrackPoint;
class canvasConfig;
class RoutePoint;
class Track;

//----------------------------------------------------------------------------
//    Static XML Helpers
//----------------------------------------------------------------------------

//RoutePoint *LoadGPXWaypoint (GpxWptElement *wptnode, QString def_symbol_name, bool b_fullviz = false );
//Route *LoadGPXRoute (GpxRteElement *rtenode, int routenum, bool b_fullviz = false );
//Route *LoadGPXTrack (GpxTrkElement *trknode, bool b_fullviz = false );
//void GPXLoadTrack ( GpxTrkElement *trknode, bool b_fullviz = false  );
//void GPXLoadRoute ( GpxRteElement *rtenode, int routenum, bool b_fullviz = false );
//void InsertRoute(Route *pTentRoute, int routenum);
//void UpdateRoute(Route *pTentRoute);

//GpxWptElement *CreateGPXWpt ( RoutePoint *pr, char * waypoint_type, bool b_props_explicit = false, bool b_props_minimal = false );
//GpxRteElement *CreateGPXRte ( Route *pRoute );
//GpxTrkElement *CreateGPXTrk ( Route *pRoute );

bool WptIsInRouteList(RoutePoint *pr);
RoutePoint *WaypointExists( const QString& name, double lat, double lon);
RoutePoint *WaypointExists( const QString& guid);
Route *RouteExists( const QString& guid);
Route *RouteExists( Route * pTentRoute );
Track *TrackExists( const QString& guid );
const char *ParseGPXDateTime( QDateTime &dt, const char *datetime );

void ExportGPX(QWidget* parent, bool bviz_only = false, bool blayer = false);
void UI_ImportGPX(QWidget* parent, bool islayer = false, QString dirpath = (""), bool isdirectory = true, bool isPersistent = false);
 
//bool ExportGPXRoutes(QWidget* parent, RouteList *pRoutes, const QString suggestedName = ("routes"));
//bool ExportGPXTracks(QWidget* parent, TrackList *pRoutes, const QString suggestedName = ("tracks"));
//bool ExportGPXWaypoints(QWidget* parent, RoutePointList *pRoutePoints, const QString suggestedName = ("waypoints"));

////----------------------------------------------------------------------------
////    Config
////----------------------------------------------------------------------------
//#include <QSettings>
//class MyConfig : public QSettings
//{
//public:

//      MyConfig(const QString &LocalFileName);

//      int LoadMyConfig();
//      void LoadS57Config();
//      void LoadNavObjects();
//      virtual void AddNewRoute(Route *pr);
//      virtual void UpdateRoute(Route *pr);
//      virtual void DeleteConfigRoute(Route *pr);

//      virtual void AddNewTrack(Track *pt);
//      virtual void UpdateTrack(Track *pt);
//      virtual void DeleteConfigTrack(Track *pt);

//      virtual void AddNewWayPoint(RoutePoint *pWP, int ConfigRouteNum = -1);
//      virtual void UpdateWayPoint(RoutePoint *pWP);
//      virtual void DeleteWayPoint(RoutePoint *pWP);
//      virtual void AddNewTrackPoint( TrackPoint *pWP, const QString& parent_GUID );

//      virtual void CreateConfigGroups ( ChartGroupArray *pGroupArray );
//      virtual void DestroyConfigGroups ( void );
//      virtual void LoadConfigGroups ( ChartGroupArray *pGroupArray );

//      virtual void LoadCanvasConfigs( bool bApplyAsTemplate = false );
//      virtual void LoadConfigCanvas( canvasConfig *cConfig, bool bApplyAsTemplate );

//      virtual void SaveCanvasConfigs( );
//      virtual void SaveConfigCanvas( canvasConfig *cc );
      
//      virtual bool UpdateChartDirs(ArrayOfCDI &dirarray);
//      virtual bool LoadChartDirArray(ArrayOfCDI &ChartDirArray);
//      virtual void UpdateSettings();

//      bool LoadLayers(QString &path);
//      int LoadMyConfigRaw( bool bAsTemplate = false );
      
//      void CreateRotatingNavObjBackup();
//      virtual void UpdateNavObj();
      
//      QString                m_sNavObjSetFile;
//      QString                m_sNavObjSetChangesFile;

//      NavObjectChanges        *m_pNavObjectChangesSet;
//      NavObjectCollection1    *m_pNavObjectInputSet;
//      bool                    m_bSkipChangeSetUpdate;
      
//};

void SwitchInlandEcdisMode( bool Switch );

/*
 * X11FontPicker DIALOG
 */

class MyFontPreviewer;

/*
enum
{
      wxID_FONT_UNDERLINE = 3000,
      wxID_FONT_STYLE,
      wxID_FONT_WEIGHT,
      wxID_FONT_FAMILY,
      wxID_FONT_COLOUR,
      wxID_FONT_SIZE
};
*/
#include "opencpn_global.h"
#include <QFontDialog>
#include <QComboBox>
#include <QCheckBox>

class ZCHX_OPENCPN_EXPORT X11FontPicker : public QFontDialog
{
    Q_OBJECT
public:
    X11FontPicker() : QFontDialog(0) { Init(); }
    X11FontPicker(const QFont &data, QWidget *parent = Q_NULLPTR)  : QFontDialog(data, parent) { Init(); }
    virtual ~X11FontPicker();

    virtual int ShowModal();
    virtual void CreateWidgets();
    virtual void InitializeFont();
protected:
    void closeEvent(QCloseEvent*);

public slots:
    void OnChangeFont();
    void OnChangeFace();
    void OnCloseWindow();




protected:
    // common part of all ctors
    void Init();

    virtual bool DoCreate(QWidget *parent);
    void InitializeAllAvailableFonts();
    void SetChoiceOptionsFromFacename(const QString &facename);
    void DoFontChange(void);

    QFont dialogFont;

    QFont    *familyChoice;
    QComboBox    *styleChoice;
    QComboBox    *weightChoice;
    QComboBox    *colourChoice;
    QCheckBox  *underLineCheckBox;
    QComboBox    *pointSizeChoice;

    MyFontPreviewer *m_previewer;
    bool        m_useEvents;

    QStringList     *pFaceNameArray;

    QFont            *pPreviewFont;
};


class GpxDocument
{
public:
    static QString GetUUID(void);
    static void SeedRandom();
private:
    static int GetRandomNumber(int min, int max);
};

#endif
