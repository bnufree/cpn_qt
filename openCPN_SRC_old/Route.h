/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "wx28compat.h"

#include "viewport.h"
#include "RoutePoint.h"
#include "Hyperlink.h"
#include <QDateTime>
#define WIDTH_UNDEFINED -1

#define ROUTE_DEFAULT_SPEED 5.0
#define RTE_TIME_DISP_UTC "UTC"
#define RTE_TIME_DISP_PC "PC"
#define RTE_TIME_DISP_LOCAL "LOCAL"
#define RTE_UNDEF_DEPARTURE wxInvalidDateTime

const QString GpxxColorNames[] = {"Black", "DarkRed", "DarkGreen", "DarkYellow", "DarkBlue", "DarkMagenta", "DarkCyan", "LightGray", "DarkGray", "Red", "Green", "Yellow", "Blue", "Magenta", "Cyan", "White" };//The last color defined by Garmin is transparent - we ignore it
const QColor GpxxColors[] = { QColor(0x00, 0x00, 0x00), QColor(0x60, 0x00, 0x00), QColor(0x00, 0x60, 0x00), QColor(0x80, 0x80, 0x00), QColor(0x00, 0x00, 0x60), QColor(0x60, 0x00, 0x60), QColor(  0x00, 0x80, 0x80), QColor(0xC0, 0xC0, 0xC0), QColor(0x60, 0x60, 0x60), QColor(0xFF, 0x00, 0x00), QColor(0x00, 0xFF, 0x00), QColor(0xF0, 0xF0, 0x00), QColor(0x00, 0x00, 0xFF), QColor(0xFE, 0x00, 0xFE), QColor(0x00, 0xFF, 0xFF), QColor(0xFF, 0xFF, 0xFF) };
const int StyleValues[] = { -1, Qt::SolidLine, Qt::DotLine, Qt::DashLine, Qt::DashDotDotLine, Qt::DashDotLine };
const int WidthValues[] = { -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

class ocpnDC;
class ChartCanvas;

class Route : public QObject
{
    Q_OBJECT
public:
      Route(QObject* parent = 0);
      ~Route();

      virtual void Draw(ocpnDC& dc, ChartCanvas *canvas, const LLBBox &box);
      virtual int GetnPoints(void) { return pRoutePointList->size(); }
      
      void AddPoint(RoutePoint *pNewPoint,
                    bool b_rename_in_sequence = true,
                    bool b_deferBoxCalc = false);

      RoutePoint *GetPoint(int nPoint);
      RoutePoint *GetPoint ( const QString &guid );
      int GetIndexOf(RoutePoint *prp);
      RoutePoint *InsertPointBefore(RoutePoint *pRP, double rlat, double rlon, bool bRenamePoints = false);
      RoutePoint *InsertPointAfter(RoutePoint *pRP, double rlat, double rlon, bool bRenamePoints = false);
      
      void DrawPointWhich(ocpnDC& dc, ChartCanvas *canvas, int iPoint, QPoint *rpn);
      void DrawSegment(ocpnDC& dc, ChartCanvas *canvas, QPoint *rp1, QPoint *rp2, ViewPort &vp, bool bdraw_arrow);
      
      void DrawGLLines( ViewPort &vp, ocpnDC *dc, ChartCanvas *canvas );
      void DrawGL( ViewPort &vp, ChartCanvas *canvas );
      void DrawGLRouteLines( ViewPort &vp, ChartCanvas *canvas );
      
      RoutePoint *GetLastPoint();
      void DeletePoint(RoutePoint *rp, bool bRenamePoints = false);
      void RemovePoint(RoutePoint *rp, bool bRenamePoints = false);
      void DeSelectRoute();
      void FinalizeForRendering();
      void UpdateSegmentDistance( RoutePoint *prp0, RoutePoint *prp, double planspeed = -1.0 );
      void UpdateSegmentDistances(double planspeed = -1.0);
      void CalculateDCRect(wxDC& dc_route, ChartCanvas *canvas, QRect *prect);
      LLBBox &GetBBox();
      void SetHiLite( int width ) {m_hiliteWidth = width; }
      void Reverse(bool bRenamePoints = false);
      void RebuildGUIDList(void);
      void RenameRoutePoints();
      void ReloadRoutePointIcons();
      QString GetNewMarkSequenced(void);
      void AssembleRoute();
      bool IsEqualTo(Route *ptargetroute);
      void CloneRoute(Route *psourceroute, int start_nPoint, int end_nPoint, const QString & suffix, const bool duplicate_first_point = false);
      void ClearHighlights(void);
      void RenderSegment(ocpnDC& dc, int xa, int ya, int xb, int yb, ViewPort &vp, bool bdraw_arrow, int hilite_width = 0);
      void RenderSegmentArrowsGL( int xa, int ya, int xb, int yb, ViewPort &vp);

      void SetVisible(bool visible = true, bool includeWpts = true);
      void SetListed(bool visible = true);
      bool IsVisible() { return m_bVisible; }
      bool IsListed() { return m_bListed; }
      bool IsActive() { return m_bRtIsActive; }
      bool IsSelected() { return m_bRtIsSelected; }

      int SendToGPS(const QString & com_name, bool bsend_waypoints, QProgressBar *pProgress);

      double GetRouteArrivalRadius(void){ return m_ArrivalRadius;}
      void SetRouteArrivalRadius(double radius){m_ArrivalRadius = radius;}
      void SetDepartureDate(const QDateTime &dt) { if( dt.isValid() ) m_PlannedDeparture = dt; }
    
      QString GetName() const { return m_RouteNameString; }
      QString GetTo() const { return m_RouteEndString; }

      int         m_ConfigRouteNum;
      bool        m_bRtIsSelected;
      bool        m_bRtIsActive;
      RoutePoint  *m_pRouteActivePoint;
      bool        m_bIsBeingCreated;
      bool        m_bIsBeingEdited;
      double      m_route_length;
      double      m_route_time;
      QString    m_RouteNameString;
      QString    m_RouteStartString;
      QString    m_RouteEndString;
      QString    m_RouteDescription;
      bool        m_bDeleteOnArrival;
      QString    m_GUID;
      bool        m_bIsInLayer;
      int         m_LayerID;
      int         m_width;
      wxPenStyle  m_style;
      int         m_lastMousePointIndex;
      bool        m_NextLegGreatCircle;
      double      m_PlannedSpeed;
      QDateTime  m_PlannedDeparture;
      QString    mimeDisplayFormat;

      RoutePointList     *pRoutePointList;

      QRect      active_pt_rect;
      QString    m_Colour;
      bool        m_btemp;
      int         m_hiliteWidth;
      HyperlinkList *m_HyperlinkList;
      
private:
      LLBBox     RBBox;

      int         m_nm_sequence;
      bool        m_bVisible; // should this route be drawn?
      bool        m_bListed;
      double      m_ArrivalRadius;
};

typedef QList<Route>    RouteList; // establish class Route as list member

#endif
