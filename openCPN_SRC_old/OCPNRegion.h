/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Portions Copyright (C) 2010 by David S. Register                      *
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
 */

/////////////////////////////////////////////////////////////////////////////
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _OCPN_REGION_H_
#define _OCPN_REGION_H_

#include <QRegion>
#include "_def.h"

// ----------------------------------------------------------------------------
// OCPNRegion
// ----------------------------------------------------------------------------
typedef enum
{
    OGDK_EVEN_ODD_RULE,
    OGDK_WINDING_RULE
} OGdkFillRule;

typedef enum
{
    OGDK_OVERLAP_RECTANGLE_IN,
    OGDK_OVERLAP_RECTANGLE_OUT,
    OGDK_OVERLAP_RECTANGLE_PART
} OGdkOverlapType;

#define EMPTY_REGION(pReg) pReg->numRects = 0
#define REGION_NOT_EMPTY(pReg) pReg->numRects

typedef struct _OGdkPoint
{
    int x;
    int y;
}OGdkPoint;

typedef struct _OGdkRectangle
{
    int x;
    int y;
    int width;
    int height;
}OGdkRectangle;

typedef struct _OGdkSegment
{
    int x1;
    int y1;
    int x2;
    int y2;
}OGdkSegment, OGdkRegionBox;

typedef struct _OGdkRegion
{
    long size;
    long numRects;
    OGdkRegionBox *rects;
    OGdkRegionBox extents;
}OGdkRegion;


class OCPNRegion
{
public:
    OCPNRegion() {m_region = 0;}

    OCPNRegion( int x, int y, int w, int h );
    OCPNRegion( const zchxPoint& topLeft, const zchxPoint& bottomRight );
    OCPNRegion( const QRect& rect );
    OCPNRegion( const QRegion& region );
    OCPNRegion( size_t n, const zchxPoint *points, int fillStyle = Qt::OddEvenFill );
    
    virtual ~OCPNRegion();
    
    QRegion *GetNew_QRegion() const;
    // common part of ctors for a rectangle region
    void InitRect(int x, int y, int w, int h);
 
     // operators
    // ---------
    bool operator==(const OCPNRegion& region) const { return ODoIsEqual(region); }
    bool operator!=(const OCPNRegion& region) const { return !(*this == region); }
    
    bool IsOk() const { return m_region != NULL; }
    bool Ok() const { return IsOk(); }
    
    // Get the bounding box
    bool GetBox(int& x, int& y, int& w, int& h) const
    { return ODoGetBox(x, y, w, h); }
    QRect GetBox() const
    {
        int x, y, w, h;
        return ODoGetBox(x, y, w, h) ? QRect(x, y, w, h) : QRect();
    }

    // Test if the given point or rectangle is inside this region
    bool Contains(int x, int y) const
    { return ODoContainsPoint(x, y); }
    bool Contains(const zchxPoint& pt) const
    { return ODoContainsPoint(pt.x, pt.y); }
    bool Contains(int x, int y, int w, int h) const
    { return ODoContainsRect(QRect(x, y, w, h)); }
    bool Contains(const QRect& rect) const
    { return ODoContainsRect(rect); }
    
    // Is region equal (i.e. covers the same area as another one)?
    bool IsEqual(const OCPNRegion& region) const;
 
    // OCPNRegionBase methods
    virtual void Clear();
    virtual bool IsEmpty() const;
    bool Empty() const { return IsEmpty(); }

    void copyFromOtherRegion(const OCPNRegion& region);
    
public:
//    OCPNRegion( OGdkRegion *region );

    void *GetRegion() const;

    bool Offset(int x, int y)   { return ODoOffset(x, y); }
    bool Offset(const zchxPoint& pt)      { return ODoOffset(pt.x, pt.y); }
    bool Intersect(const OCPNRegion& region) { return ODoIntersect(region); }
    bool Union(const OCPNRegion& region) { return ODoUnionWithRegion(region); }
            bool Union(int x, int y, int w, int h) { return ODoUnionWithRect(QRect(x, y, w, h)); }
    bool Union(const QRect& rect) { return ODoUnionWithRect(rect); }
    bool Subtract(const OCPNRegion& region) { return ODoSubtract(region); }
    
protected:
    // ref counting code
//    virtual wxObjectRefData *CreateRefData() const;
//    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;

    // QRegionBase pure virtuals
    virtual bool ODoIsEqual(const OCPNRegion& region) const;
    virtual bool ODoGetBox(int& x, int& y, int& w, int& h) const;
    virtual bool ODoContainsPoint(int x, int y) const;
    virtual bool ODoContainsRect(const QRect& rect) const;

    virtual bool ODoOffset(int x, int y);
    virtual bool ODoUnionWithRect(const QRect& rect);
    virtual bool ODoUnionWithRegion(const OCPNRegion& region);
    virtual bool ODoIntersect(const OCPNRegion& region);
    virtual bool ODoSubtract(const OCPNRegion& region);
//    virtual bool DoXor(const OCPNRegion& region);

private:
    OGdkRegion*             m_region;
    
private:
//    DECLARE_DYNAMIC_CLASS(OCPNRegion);
};

// ----------------------------------------------------------------------------
// OCPNRegionIterator: decomposes a region into rectangles
// ----------------------------------------------------------------------------

class  OCPNRegionIterator 
{
public:
    OCPNRegionIterator();
    OCPNRegionIterator(const OCPNRegion& region);
    virtual ~OCPNRegionIterator();

    void Reset();
    void Reset(const OCPNRegion& region);

    bool HaveRects() const;
    void NextRect(void);
    QRect GetRect() const;

private:
    void Init();
    void CreateRects( const OCPNRegion& r );

    size_t   m_current;
//    OCPNRegion m_region;

    QRect *m_rects;
    size_t  m_numRects;
};


#endif
// _OCPN_REGION_H_
