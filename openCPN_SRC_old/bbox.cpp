/////////////////////////////////////////////////////////////////////////////
// Name:        bbox.cpp
// Author:      Klaas Holwerda
// Created:     XX/XX/XX
// Copyright:   2000 (c) Klaas Holwerda
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////


// For compilers that support precompilation, includes "wx/wx.h".

#include "bbox.h"
#include <assert.h>

wxBoundingBox::wxBoundingBox()
{
    m_minx = m_miny = m_maxx =  m_maxy = 0.0;
    m_validbbox = false;
}


wxBoundingBox::wxBoundingBox(const wxBoundingBox &other)
{
    m_minx = other.m_minx;
    m_miny = other.m_miny;
    m_maxx = other.m_maxx;
    m_maxy = other.m_maxy;
    m_validbbox= other.m_validbbox;
}


wxBoundingBox::wxBoundingBox(const QPointF& a)
{
    m_minx = a.x();
    m_maxx = a.x();
    m_miny = a.y();
    m_maxy = a.y();
    m_validbbox = true;
}

wxBoundingBox::wxBoundingBox(double xmin, double ymin, double xmax, double ymax)
{
    m_minx = xmin;
    m_miny = ymin;
    m_maxx = xmax;
    m_maxy = ymax;
    m_validbbox = true;
}

wxBoundingBox::~wxBoundingBox()
{
}

// This function checks if two bboxes intersect
bool wxBoundingBox::And(wxBoundingBox *_bbox, double Marge)
{
    assert (m_validbbox == true);
    assert (_bbox->GetValid());
    m_minx = qMax(m_minx, _bbox->m_minx);
    m_maxx = qMin(m_maxx, _bbox->m_maxx);
    m_miny = qMax(m_miny, _bbox->m_miny);
    m_maxy = qMin(m_maxy, _bbox->m_maxy);
    return (bool)(
                ((m_minx - Marge) < (m_maxx + Marge)) &&
                ((m_miny - Marge) < (m_maxy + Marge))
                );
}

// Shrink the boundingbox with the given marge
void wxBoundingBox::Shrink(const double Marge)
{
    assert (m_validbbox == true);

    m_minx += Marge;
    m_maxx -= Marge;
    m_miny += Marge;
    m_maxy -= Marge;
}


// Expand the boundingbox with another boundingbox
void wxBoundingBox::Expand(const wxBoundingBox &other)
{
    if (!m_validbbox)
    {
        *this=other;
    }
    else
    {
        m_minx = qMin(m_minx, other.m_minx);
        m_maxx = qMax(m_maxx, other.m_maxx);
        m_miny = qMin(m_miny, other.m_miny);
        m_maxy = qMax(m_maxy, other.m_maxy);
    }
}


// Expand the boundingbox with a point
void wxBoundingBox::Expand(const QPointF& a_point)
{
    if (!m_validbbox)
    {
        m_minx = m_maxx = a_point.x();
        m_miny = m_maxy = a_point.y();
        m_validbbox = true;
    }
    else
    {
        m_minx = qMin(m_minx, a_point.x());
        m_maxx = qMax(m_maxx, a_point.x());
        m_miny = qMin(m_miny, a_point.y());
        m_maxy = qMax(m_maxy, a_point.y());
    }
}

// Expand the boundingbox with a point
void wxBoundingBox::Expand(double x,double y)
{
    if (!m_validbbox)
    {
        m_minx = m_maxx = x;
        m_miny = m_maxy = y;
        m_validbbox = true;
    }
    else
    {
        m_minx = qMin(m_minx, x);
        m_maxx = qMax(m_maxx, x);
        m_miny = qMin(m_miny, y);
        m_maxy = qMax(m_maxy, y);
    }
}


// Expand the boundingbox with two points
void wxBoundingBox::Expand(const QPointF& a, const QPointF& b)
{
    Expand(a);
    Expand(b);
}

// Enlarge the boundingbox with the given marge
void wxBoundingBox::EnLarge(const double marge)
{
    if (!m_validbbox)
    {
        m_minx = m_maxx = marge;
        m_miny = m_maxy = marge;
        m_validbbox = true;
    }
    else
    {
        m_minx -= marge;
        m_maxx += marge;
        m_miny -= marge;
        m_maxy += marge;
    }
}

// Calculates if two boundingboxes intersect. If so, the function returns _ON.
// If they do not intersect, two scenario's are possible:
// other is outside this -> return _OUT
// other is inside this -> return _IN
OVERLAP wxBoundingBox::Intersect(const wxBoundingBox &other, double Marge) const
{
    assert (m_validbbox == true);

    // other boundingbox must exist
    assert (&other);

    if (((m_minx - Marge) > (other.m_maxx + Marge)) ||
         ((m_maxx + Marge) < (other.m_minx - Marge)) ||
         ((m_maxy + Marge) < (other.m_miny - Marge)) ||
         ((m_miny - Marge) > (other.m_maxy + Marge)))
        return _OUT;

    // Check if other.bbox is inside this bbox
    if ((m_minx <= other.m_minx) &&
         (m_maxx >= other.m_maxx) &&
         (m_maxy >= other.m_maxy) &&
         (m_miny <= other.m_miny))
        return _IN;

    // Boundingboxes intersect
    return _ON;
}


// Checks if a line intersects the boundingbox
bool wxBoundingBox::LineIntersect(const QPointF& begin, const QPointF& end ) const
{
    assert (m_validbbox == true);

    return (bool)
              !(((begin.y() > m_maxy) && (end.y() > m_maxy)) ||
                ((begin.y() < m_miny) && (end.y() < m_miny)) ||
                ((begin.x() > m_maxx) && (end.x() > m_maxx)) ||
                ((begin.x() < m_minx) && (end.x() < m_minx)));
}


// Is the given point in the boundingbox ??
bool wxBoundingBox::PointInBox(double x, double y, double Marge) const
{
    assert (m_validbbox == true);

    if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
            y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
            return true;
    return false;
}


//
// Is the given point in the boundingbox ??
//
bool wxBoundingBox::PointInBox(const QPointF& a, double Marge) const
{
    assert (m_validbbox == true);

    return PointInBox(a.x(), a.y(), Marge);
}


QPointF wxBoundingBox::GetMin() const
{
    assert (m_validbbox == true);

    return QPointF(m_minx, m_miny);
}


QPointF wxBoundingBox::GetMax() const
{
    assert (m_validbbox == true);

    return QPointF(m_maxx, m_maxy);
}

bool wxBoundingBox::GetValid() const
{
    return m_validbbox;
}

void wxBoundingBox::SetMin(double px, double py)
{
    m_minx = px;
    m_miny = py;
    if (!m_validbbox)
    {
        m_maxx = px;
        m_maxy = py;
        m_validbbox = true;
    }
}

void wxBoundingBox::SetMax(double px, double py)
{
    m_maxx = px;
    m_maxy = py;
    if (!m_validbbox)
    {
        m_minx = px;
        m_miny = py;
        m_validbbox = true;
    }
}

void wxBoundingBox::SetValid(bool value)
{
    m_validbbox = value;
}

// adds an offset to the boundingbox
// usage : a_boundingbox.Translate(a_point);
void wxBoundingBox::Translate(QPointF& offset)
{
    assert (m_validbbox == true);

    m_minx += offset.x();
    m_maxx += offset.x();
    m_miny += offset.y();
    m_maxy += offset.y();
}


// clears the bounding box settings
void wxBoundingBox::Reset()
{
    m_minx = 0.0;
    m_maxx = 0.0;
    m_miny = 0.0;
    m_maxy = 0.0;
    m_validbbox = true;
}


void wxBoundingBox::SetBoundingBox(const QPointF& a_point)
{
    m_minx = a_point.x();
    m_maxx = a_point.x();
    m_miny = a_point.y();
    m_maxy = a_point.y();

    m_validbbox = true;
}


// Expands the boundingbox with the given point
// usage : a_boundingbox = a_boundingbox + pointer_to_an_offset;
wxBoundingBox& wxBoundingBox::operator+(wxBoundingBox &other)
{
    assert (m_validbbox == true);
    assert (other.GetValid());

    Expand(other);
    return *this;
}


// makes a boundingbox same as the other
wxBoundingBox& wxBoundingBox::operator=( const wxBoundingBox &other)
{
    m_minx = other.m_minx;
    m_maxx = other.m_maxx;
    m_miny = other.m_miny;
    m_maxy = other.m_maxy;
    m_validbbox = other.m_validbbox;
    return *this;
}

#ifdef WX_
void wxBoundingBox::MapBbox( const QTransform& matrix)
{
    assert (m_validbbox == true);

    double x1,y1,x2,y2,x3,y3,x4,y4;

    matrix.TransformPoint( m_minx, m_miny, x1, y1 );
    matrix.TransformPoint( m_minx, m_maxy, x2, y2 );
    matrix.TransformPoint( m_maxx, m_maxy, x3, y3 );
    matrix.TransformPoint( m_maxx, m_miny, x4, y4 );

    double xmin = qMin(x1,x2);
    xmin = qMin(xmin,x3);
    xmin = qMin(xmin,x4);

    double xmax = qMax( x1, x2);
    xmax = qMax(xmax,x3);
    xmax = qMax(xmax,x4);

    double ymin = qMin(y1, y2);
    ymin = qMin(ymin,y3);
    ymin = qMin(ymin,y4);

    double ymax = qMax(y1,y2);
    ymax = qMax(ymax,y3);
    ymax = qMax(ymax,y4);

    // Use these min and max values to set the new boundingbox
    m_minx = xmin;
    m_miny = ymin;
    m_maxx = xmax;
    m_maxy = ymax;
}
#endif

//----------------------------------------------------------------
//    LLBBox Implementation
//----------------------------------------------------------------

void LLBBox::Set(double minlat, double minlon, double maxlat, double maxlon)
{
    m_minlat = minlat;
    m_minlon = minlon;
    m_maxlat = maxlat;
    m_maxlon = maxlon;
    m_valid = (minlat <= maxlat && minlon <= maxlon);
}

void LLBBox::SetFromSegment(double lat1, double lon1, double lat2, double lon2)
{
    m_minlat = qMin(lat1, lat2);
    m_maxlat = qMax(lat1, lat2);

    double minlon[3], maxlon[3];
    double lon[2][3] = {{lon1}, {lon2}};
    for(int i=0; i<2; i++) {
        if(lon[i][0] < 0) {
            lon[i][1] = lon[i][0] + 360;
            lon[i][2] = lon[i][0];
        } else {
            lon[i][1] = lon[i][0];
            lon[i][2] = lon[i][0] - 360;
        }
    }
    
    double d[3];
    for(int k=0; k<3; k++) {
        minlon[k] = qMin(lon[0][k], lon[1][k]);
        maxlon[k] = qMax(lon[0][k], lon[1][k]);

        double a = maxlon[k] + minlon[k];
        // eliminate cases where the average longitude falls outside of -180 to 180
        if(a <= -360 || a >= 360)
            d[k] = 360;
        else
            d[k] = maxlon[k] - minlon[k];
    }

    double epsilon = 1e-2;  // because floating point rounding favor... d1, then d2 then d3
    d[1] += epsilon, d[2] += 2*epsilon;
    int mink = 0;
    for(int k=1; k<3; k++)
        if(d[k] < d[mink])
            mink = k;

    m_minlon = minlon[mink];
    m_maxlon = maxlon[mink];

    m_valid = true;
}

void LLBBox::Expand(const LLBBox& other)
{
    if(!m_valid) {
        *this = other;
        return;
    }

    m_minlat = qMin(m_minlat, other.m_minlat);
    m_maxlat = qMax(m_maxlat, other.m_maxlat);

    double minlons[2][3] = {{m_minlon}, {other.m_minlon}};
    double maxlons[2][3] = {{m_maxlon}, {other.m_maxlon}};

    for(int i=0; i<2; i++) {
        if(minlons[i][0] < 0) {
            minlons[i][1] = minlons[i][0] + 360;
            maxlons[i][1] = maxlons[i][0] + 360;
        } else {
            minlons[i][1] = minlons[i][0];
            maxlons[i][1] = maxlons[i][0];
        }

        if(maxlons[i][0] > 0) {
            minlons[i][2] = minlons[i][0] - 360;
            maxlons[i][2] = maxlons[i][0] - 360;
        } else {
            minlons[i][2] = minlons[i][0];
            maxlons[i][2] = maxlons[i][0];
        }
    }

    double d[3];
    double minlon[3], maxlon[3];

    for(int k=0; k<3; k++) {
        minlon[k] = qMin(minlons[0][k], minlons[1][k]);
        maxlon[k] = qMax(maxlons[0][k], maxlons[1][k]);

        double a = maxlon[k] + minlon[k];
        // eliminate cases where the average longitude falls outside of -180 to 180
        if(a <= -360 || a >= 360)
            d[k] = 360;
        else
            d[k] = maxlon[k] - minlon[k];
    }

    double epsilon = 1e-2;  // because floating point rounding favor... d1, then d2 then d3
    d[1] += epsilon, d[2] += 2*epsilon;
    int mink = 0;
    for(int k=1; k<3; k++)
        if(d[k] < d[mink])
            mink = k;

    m_minlon = minlon[mink];
    m_maxlon = maxlon[mink];

}

bool LLBBox::Contains(double lat, double lon) const
{
    if(lat < m_minlat || lat > m_maxlat )
        return false;

//    Box is centered in East lon, crossing IDL
    if(m_maxlon > 180.) {
        if( lon < m_maxlon - 360.)
            lon +=  360.;
    }
      //    Box is centered in Wlon, crossing IDL
    else if(m_minlon < -180.)
    {
        if(lon > m_minlon + 360.)
            lon -= 360.;
    }

    return lon >= m_minlon && lon <= m_maxlon;
}

bool LLBBox::ContainsMarge(double lat, double lon, double Marge) const
{
    if(lat < (m_minlat - Marge) || lat > (m_maxlat + Marge) )
        return false;

//    Box is centered in East lon, crossing IDL
    if(m_maxlon > 180.) {
        if( lon < m_maxlon - 360.)
            lon +=  360.;
    }
      //    Box is centered in Wlon, crossing IDL
    else if(m_minlon < -180.)
    {
        if(lon > m_minlon + 360.)
            lon -= 360.;
    }

    return  lon >= (m_minlon - Marge) && lon <= (m_maxlon + Marge);
}

bool LLBBox::IntersectIn( const LLBBox &other ) const
{
    if( !GetValid() || !other.GetValid() )
        return false;

    if((m_maxlat <= other.m_maxlat) || (m_minlat >= other.m_minlat))
        return false;
    
    double minlon = m_minlon, maxlon = m_maxlon;
    if(m_maxlon < other.m_minlon)
        minlon += 360, maxlon += 360;
    else if(m_minlon > other.m_maxlon)
        minlon -= 360, maxlon -= 360;

    return (other.m_minlon > minlon) && (other.m_maxlon < maxlon);
}

bool LLBBox::IntersectOutGetBias( const LLBBox &other, double bias ) const
{
    // allow -180 to 180 or 0 to 360
    if( !GetValid() || !other.GetValid() )
        return true;
    
    if((m_maxlat < other.m_minlat) || (m_minlat > other.m_maxlat))
        return true;

    if(m_maxlon < other.m_minlon)
        bias = 360;
    else if(m_minlon > other.m_maxlon)
        bias = -360;
    else
        bias = 0;

    return (m_minlon + bias > other.m_maxlon) || (m_maxlon + bias < other.m_minlon);
}

#if 0 // use if needed...
OVERLAP LLBox::Intersect( const LLBBox &other) const
{
    if(IntersectOut(other))
        return _OUT;

    if(IntersectIn(other))
        return _IN;

    // Boundingboxes intersect
    return _ON;
}
#endif

void LLBBox::EnLarge(const double marge)
{
    if (!m_valid)
    {
        m_minlon = m_maxlon = marge;
        m_minlat = m_maxlat = marge;
        m_valid = true;
    }
    else
    {
        m_minlon -= marge;
        m_maxlon += marge;
        m_minlat -= marge;
        m_maxlat += marge;
    }
}
