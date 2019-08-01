/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Route Manager
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

#ifndef __ROUTEMAN_H__
#define __ROUTEMAN_H__


#include "chart1.h"                 // for ColorScheme definition
#include "styles.h"
#include "Select.h"
#include "nmea0183.h"

#include <QBitmap>
#include "Route.h"
#include "RoutePoint.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

extern bool g_bPluginHandleAutopilotRoute;

//----------------------------------------------------------------------------
//    forward class declarations
//----------------------------------------------------------------------------

class Route;
class RoutePoint;

//    List definitions for Waypoint Manager Icons

class markicon_bitmap_list_type;
class markicon_key_list_type;
class markicon_description_list_type;
class MarkIcon;

typedef QList<MarkIcon*> SortedArrayOfMarkIcon;
typedef QList<MarkIcon*> ArrayOfMarkIcon;

//----------------------------------------------------------------------------
//   Routeman
//----------------------------------------------------------------------------

class Routeman : public QObject
{
    Q_OBJECT
public:
      Routeman(QObject *parent);
      ~Routeman();

      bool DeleteRoute(Route *pRoute);
      void DeleteAllRoutes(void);
      void DeleteAllTracks(void);

      void DeleteTrack(Track *pTrack);

      bool IsRouteValid(Route *pRoute);

      Route *FindRouteByGUID(const QString &guid);
      Track *FindTrackByGUID(const QString &guid);
      Route *FindRouteContainingWaypoint(RoutePoint *pWP);
      wxArrayPtrVoid *GetRouteArrayContaining(RoutePoint *pWP);
      bool DoesRouteContainSharedPoints( Route *pRoute );
      void RemovePointFromRoute( RoutePoint* point, Route* route, ChartCanvas *cc );
          
      bool ActivateRoute(Route *pRouteToActivate, RoutePoint *pStartPoint = NULL);
      bool ActivateRoutePoint(Route *pA, RoutePoint *pRP);
      bool ActivateNextPoint(Route *pr, bool skipped);
      RoutePoint *FindBestActivatePoint(Route *pR, double lat, double lon, double cog, double sog);

      bool UpdateProgress();
      bool UpdateAutopilot();
      bool DeactivateRoute( bool b_arrival = false );
      bool IsAnyRouteActive(void){ return (pActiveRoute != NULL); }
      void SetColorScheme(ColorScheme cs);

      Route *GetpActiveRoute(){ return pActiveRoute;}
      RoutePoint *GetpActivePoint(){ return pActivePoint;}
      double GetCurrentRngToActivePoint(){ return CurrentRngToActivePoint;}
      double GetCurrentBrgToActivePoint(){ return CurrentBrgToActivePoint;}
      double GetCurrentRngToActiveNormalArrival(){ return CurrentRangeToActiveNormalCrossing;}
      double GetCurrentXTEToActivePoint(){ return CurrentXTEToActivePoint;}
      void   ZeroCurrentXTEToActivePoint();
      double GetCurrentSegmentCourse(){ return CurrentSegmentCourse;}
      int   GetXTEDir(){ return XTEDir;}

      QPen   * GetRoutePen(void){return m_pRoutePen;}
      QPen   * GetTrackPen(void){return m_pTrackPen;}
      QPen   * GetSelectedRoutePen(void){return m_pSelectedRoutePen;}
      QPen   * GetActiveRoutePen(void){return m_pActiveRoutePen;}
      QPen   * GetActiveRoutePointPen(void){return m_pActiveRoutePointPen;}
      QPen   * GetRoutePointPen(void){return m_pRoutePointPen;}
      QBrush * GetRouteBrush(void){return m_pRouteBrush;}
      QBrush * GetSelectedRouteBrush(void){return m_pSelectedRouteBrush;}
      QBrush * GetActiveRouteBrush(void){return m_pActiveRouteBrush;}
      QBrush * GetActiveRoutePointBrush(void){return m_pActiveRoutePointBrush;}
      QBrush * GetRoutePointBrush(void){return m_pRoutePointBrush;}

      QString GetRouteReverseMessage(void);

      bool        m_bDataValid;

private:
      void DoAdvance(void);
    
      QObject       *m_pparent_app;
      Route       *pActiveRoute;
      RoutePoint  *pActivePoint;
      double       RouteBrgToActivePoint;        //TODO all these need to be doubles
      double       CurrentSegmentBeginLat;
      double       CurrentSegmentBeginLon;
      double       CurrentRngToActivePoint;
      double       CurrentBrgToActivePoint;
      double       CurrentXTEToActivePoint;
      double       CourseToRouteSegment;
      double       CurrentRangeToActiveNormalCrossing;
      RoutePoint  *pActiveRouteSegmentBeginPoint;
      RoutePoint  *pRouteActivatePoint;
      double       CurrentSegmentCourse;
      int         XTEDir;
      bool        m_bArrival;
      QPen       *m_pRoutePen;
      QPen       *m_pTrackPen;
      QPen       *m_pSelectedRoutePen;
      QPen       *m_pActiveRoutePen;
      QPen       *m_pActiveRoutePointPen;
      QPen       *m_pRoutePointPen;
      QBrush     *m_pRouteBrush;
      QBrush     *m_pSelectedRouteBrush;
      QBrush     *m_pActiveRouteBrush;
      QBrush     *m_pActiveRoutePointBrush;
      QBrush     *m_pRoutePointBrush;

      NMEA0183    m_NMEA0183;                         // For autopilot output
      
      double      m_arrival_min;
      int         m_arrival_test;
      

};


//----------------------------------------------------------------------------
//   WayPointman
//----------------------------------------------------------------------------
typedef QList<QImage>       QImageList;

class WayPointman
{
public:
      WayPointman();
      ~WayPointman();
      QBitmap *GetIconBitmap(const QString& icon_key);
      bool GetIconPrescaled( const QString& icon_key );
      unsigned int GetIconTexture( const QBitmap *pmb, int &glw, int &glh );
      int GetIconIndex(const QBitmap *pbm);
      int GetIconImageListIndex(const QBitmap *pbm);
      int GetXIconImageListIndex(const QBitmap *pbm);
      int GetNumIcons(void){ return m_pIconArray->count(); }
      QString CreateGUID(RoutePoint *pRP);
      RoutePoint *GetNearbyWaypoint(double lat, double lon, double radius_meters);
      RoutePoint *GetOtherNearbyWaypoint(double lat, double lon, double radius_meters, const QString &guid);
      void SetColorScheme(ColorScheme cs);
      bool SharedWptsExist();
      void DeleteAllWaypoints(bool b_delete_used);
      RoutePoint *FindRoutePointByGUID(const QString &guid);
      void DestroyWaypoint(RoutePoint *pRp, bool b_update_changeset = true);
      void ClearRoutePointFonts(void);
      void ProcessIcons( ocpnStyle::Style* style );
      void ProcessDefaultIcons();
      void ReloadAllIcons();
      void ReloadRoutepointIcons();
      
      bool DoesIconExist(const QString & icon_key) const;
      QBitmap GetIconBitmapForList(int index, int height);
      QString *GetIconDescription(int index);
      QString *GetIconKey(int index);

      QImageList *Getpmarkicon_image_list( int nominal_height );
      
      bool AddRoutePoint(RoutePoint *prp);
      bool RemoveRoutePoint(RoutePoint *prp);
      RoutePointList *GetWaypointList(void) { return m_pWayPointList; }

      MarkIcon *ProcessIcon(QBitmap pimage, const QString & key, const QString & description);
private:
      MarkIcon *ProcessLegacyIcon( QString fileName, const QString & key, const QString & description);
      MarkIcon *ProcessExtendedIcon(QImage &image, const QString & key, const QString & description);
      QRect CropImageOnAlpha(QImage &image);
      QImage CreateDimImage( QImage &image, double factor );
      
      void ProcessUserIcons( ocpnStyle::Style* style );
      RoutePointList    *m_pWayPointList;
      QBitmap *CreateDimBitmap(QBitmap *pBitmap, double factor);

      QImageList       *pmarkicon_image_list;        // Current QImageList, updated on colorscheme change
      int               m_markicon_image_list_base_count;
      ArrayOfMarkIcon    *m_pIconArray;

      int         m_nGUID;
      double      m_iconListScale;
      
      SortedArrayOfMarkIcon    *m_pLegacyIconArray;
      SortedArrayOfMarkIcon    *m_pExtendedIconArray;
      
      int         m_bitmapSizeForList;
      int         m_iconListHeight;
      ColorScheme m_cs;
};

#endif
