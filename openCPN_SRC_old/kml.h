/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Read and write KML Format (http://en.wikipedia.org/wiki/Keyhole_Markup_Language)
 * Author:   Jesper Weissglas
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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
 ***************************************************************************
 *
 *
 */

#ifndef KML_H
#define KML_H

#define KML_INSERT_EXTRADATA true // For QtVlm Routing.

enum KmlPastebufferType {
    KML_PASTE_WAYPOINT,
    KML_PASTE_ROUTE,
    KML_PASTE_TRACK,
    KML_PASTE_ROUTE_TRACK,
    KML_PASTE_INVALID,
    KML_COPY_STANDARD,
    KML_COPY_EXTRADATA
};

class dPoint {
public:
    double x,y,z;
};

typedef std::vector<dPoint> dPointList;

class Kml {
public:
    Kml();
    ~Kml();
    KmlPastebufferType ParsePasteBuffer();
    Route* GetParsedRoute() { return parsedRoute; }
    Track* GetParsedTrack() { return parsedTrack; }
    RoutePoint* GetParsedRoutePoint() { return parsedRoutePoint; }

    static QString MakeKmlFromRoute( Route* route, bool insertSeqNames = false );
    static QString MakeKmlFromTrack( Track* track );
    static QString MakeKmlFromWaypoint( RoutePoint* routepoint );
    static void CopyWaypointToClipboard( RoutePoint* routepoint );
    static void CopyRouteToClipboard( Route* route );
    static void CopyTrackToClipboard( Track* route );

private:
    KmlPastebufferType ParseOnePlacemarkPoint( TiXmlNode* node, QString& name );
    KmlPastebufferType ParseTrack( TiXmlNode* node, QString& name );
    int ParseCoordinates( TiXmlNode* node, dPointList& points );
    static TiXmlElement* StandardHead( TiXmlDocument& xmlDoc, QString name );
    static std::string PointPlacemark(  TiXmlElement* document, RoutePoint* routepoint );

    QString kmlText;
    RoutePoint* parsedRoutePoint;
    Route* parsedRoute;
    Track* parsedTrack;
    static bool insertQtVlmExtendedData;
    static int seqCounter;
};

//---------------------------------------------------------------------------
#include <QWidget>
#include <QButtonGroup>

class KmlFormatDialog : public QWidget{
    Q_OBJECT
private:
    QButtonGroup* choices;

public:
    KmlFormatDialog( QWidget* parent );
    int GetSelectedFormat();
};

#endif
