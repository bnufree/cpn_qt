/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navigation Utility Functions
 * Authors:   David Register
 *            Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by David S. Register                               *
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

#ifndef __TRACK_H__
#define __TRACK_H__


#include "vector2D.h"

#include <vector>
#include <list>
#include <deque>
#include <QDateTime>
#include "Hyperlink.h"
#include "LLRegion.h"
#include "Route.h"

class ChartCanvas;
class ViewPort;

struct SubTrack
{
    SubTrack() {}

    LLBBox            m_box;
    double            m_scale;
};

class TrackPoint
{
public:
      TrackPoint(double lat, double lon, QString ts="");
      TrackPoint(double lat, double lon, QDateTime dt);
      TrackPoint( TrackPoint* orig );
      ~TrackPoint();

      QDateTime GetCreateTime(void);
      void SetCreateTime( QDateTime dt );
      void Draw(ChartCanvas *cc, ocpnDC& dc );
      const char *GetTimeString() { return m_timestring; }
      
      double            m_lat, m_lon;
      int               m_GPXTrkSegNo;
private:
      void SetCreateTime( QString ts );
      char             *m_timestring;
};

//----------------------------------------------------------------------------
//    Track
//----------------------------------------------------------------------------

class Track
{
public:
    Track();
    virtual ~Track();

    void Draw( ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box);
    int GetnPoints(void){ return TrackPoints.size(); }
    
    
    void SetVisible(bool visible = true) { m_bVisible = visible; }
    TrackPoint *GetPoint( int nWhichPoint );
    TrackPoint *GetLastPoint();
    void AddPoint( TrackPoint *pNewPoint );
    void AddPointFinalized( TrackPoint *pNewPoint );
    TrackPoint* AddNewPoint( vector2D point, QDateTime time );
    
    void SetListed(bool listed = true) { m_bListed = listed; }
    virtual bool IsRunning() { return false; }

    bool IsVisible() { return m_bVisible; }
    bool IsListed() { return m_bListed; }

    int GetCurrentTrackSeg(){ return m_CurrentTrackSeg; }
    void SetCurrentTrackSeg(int seg){ m_CurrentTrackSeg = seg; }

    double Length();
    int Simplify( double maxDelta );
    Route *RouteFromTrack(wxGenericProgressDialog *pprog);

    void ClearHighlights();
    
    QString GetName( bool auto_if_empty = false ) const {
        if( !auto_if_empty || !m_TrackNameString.isEmpty() ) {
            return m_TrackNameString;
        } else {
            QString name;
            TrackPoint *rp = NULL;
            if((int) TrackPoints.size() > 0)
                rp = TrackPoints[0];
            if( rp && rp->GetCreateTime().isValid() )
            {
                name = rp->GetCreateTime().toString("yyyy-MM-dd hh:mm:ss sss");
            }
            else
            {
                name = QObject::tr("(Unnamed Track)");
                return name;
            }
        }
    }
    void SetName( const QString name ) { m_TrackNameString = name; }
    
    QString    m_GUID;
    bool        m_bIsInLayer;
    int         m_LayerID;

    QString    m_TrackDescription;

    QString    m_TrackStartString;
    QString    m_TrackEndString;

    int         m_width;
    Qt::PenStyle  m_style;
    QString    m_Colour;

    bool m_bVisible;
    bool        m_bListed;
    bool        m_btemp;

    int               m_CurrentTrackSeg;

    HyperlinkList     *m_HyperlinkList;
    int m_HighlightedTrackPoint;

    void Clone( Track *psourcetrack, int start_nPoint, int end_nPoint, const QString & suffix);

protected:
    void Segments( ChartCanvas *cc, std::list< std::list<QPoint> > &pointlists, const LLBBox &box, double scale);
    void DouglasPeuckerReducer( std::vector<TrackPoint*>& list,
                                std::vector<bool> & keeplist,
                                int from, int to, double delta );
    double GetXTE(TrackPoint *fm1, TrackPoint *fm2, TrackPoint *to);
    double GetXTE( double fm1Lat, double fm1Lon, double fm2Lat, double fm2Lon, double toLat, double toLon  );
            
    std::vector<TrackPoint*>     TrackPoints;
    std::vector<std::vector <SubTrack> > SubTracks;

private:
    void GetPointLists(ChartCanvas *cc, std::list< std::list<QPoint> > &pointlists,
                       ViewPort &VP, const LLBBox &box );
    void Finalize();
    double ComputeScale(int left, int right);
    void InsertSubTracks(LLBBox &box, int level, int pos);

    void AddPointToList(ChartCanvas *cc, std::list< std::list<QPoint> > &pointlists, int n);
    void AddPointToLists(ChartCanvas *cc, std::list< std::list<QPoint> > &pointlists, int &last, int n);

    void Assemble( ChartCanvas *cc, std::list< std::list<QPoint> > &pointlists, const LLBBox &box, double scale, int &last, int level, int pos);
    
    QString    m_TrackNameString;
};

typedef QList<Track> TrackList; // establish class Route as list member

class Route;
class ActiveTrack : public QObject, public Track
{
    Q_OBJECT
public:
    ActiveTrack();
    ~ActiveTrack();

    void SetPrecision(int precision);

    void Start(void);
    void Stop(bool do_add_point = false);
    Track *DoExtendDaily();
    bool IsRunning(){ return m_bRunning; }

    void AdjustCurrentTrackPoint( TrackPoint *prototype );
public slots:
    void OnTimerTrack();

private:

    void AddPointNow(bool do_add_point = false);

    bool              m_bRunning;
    QTimer            *m_TimerTrack;

    int               m_nPrecision;
    double            m_TrackTimerSec;
    double            m_allowedMaxXTE;
    double            m_allowedMaxAngle;

    vector2D          m_lastAddedPoint;
    double            m_prev_dist;
    QDateTime        m_prev_time;

    TrackPoint        *m_lastStoredTP;
    TrackPoint        *m_removeTP;
    TrackPoint        *m_prevFixedTP;
    TrackPoint        *m_fixedTP;
    int               m_track_run;
    double            m_minTrackpoint_delta;

    enum eTrackPointState {
        firstPoint,
        secondPoint,
        potentialPoint
    } trackPointState;

    std::deque<vector2D> skipPoints;
    std::deque<QDateTime> skipTimes;
};

#endif
