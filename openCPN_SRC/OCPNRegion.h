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

//#if defined(__WXOSX__)
#define USE_NEW_REGION
//#endif


// ----------------------------------------------------------------------------
// OCPNRegion
// ----------------------------------------------------------------------------

class OCPNRegion : public
#ifdef USE_NEW_REGION
 QObject
#else
 QRegion
#endif
{
    Q_OBJECT
public:
    OCPNRegion() { }

    OCPNRegion( int x, int y, int w, int h );
    OCPNRegion( const QPoint& topLeft, const QPoint& bottomRight );
    OCPNRegion( const QRect& rect );
    OCPNRegion( const QRegion& region );
    OCPNRegion( size_t n, const QPoint *points, int fillStyle = Qt::OddEvenFill );
    
    virtual ~OCPNRegion();
    
    QRegion *GetNew_QRegion() const;
    
    
#ifdef USE_NEW_REGION    

    // common part of ctors for a rectangle region
    void InitRect(int x, int y, int w, int h);
 
     // operators
    // ---------
    bool operator==(const OCPNRegion& region) const { return ODoIsEqual(region); }
    bool operator!=(const OCPNRegion& region) const { return !(*this == region); }
    
    bool IsOk() const { return m_refData != NULL; }
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
    QRegionContain Contains(int x, int y) const
    { return ODoContainsPoint(x, y); }
    QRegionContain Contains(const QPoint& pt) const
    { return ODoContainsPoint(pt.x, pt.y); }
    QRegionContain Contains(int x, int y, int w, int h) const
    { return ODoContainsRect(QRect(x, y, w, h)); }
    QRegionContain Contains(const QRect& rect) const
    { return ODoContainsRect(rect); }
    
 // Is region equal (i.e. covers the same area as another one)?
 bool IsEqual(const OCPNRegion& region) const;
 
    // OCPNRegionBase methods
    virtual void Clear();
    virtual bool IsEmpty() const;
    bool Empty() const { return IsEmpty(); }
    
public:
//    OCPNRegion( OGdkRegion *region );

    void *GetRegion() const;

    bool Offset(int x, int y)   { return ODoOffset(x, y); }
    bool Offset(const QPoint& pt)      { return ODoOffset(pt.x, pt.y); }
    bool Intersect(const OCPNRegion& region) { return ODoIntersect(region); }
    bool Union(const OCPNRegion& region) { return ODoUnionWithRegion(region); }
            bool Union(int x, int y, int w, int h) { return ODoUnionWithRect(QRect(x, y, w, h)); }
    bool Union(const QRect& rect) { return ODoUnionWithRect(rect); }
    bool Subtract(const OCPNRegion& region) { return ODoSubtract(region); }
    
protected:
    // ref counting code
    virtual wxObjectRefData *CreateRefData() const;
    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;

    // QRegionBase pure virtuals
    virtual bool ODoIsEqual(const OCPNRegion& region) const;
    virtual bool ODoGetBox(int& x, int& y, int& w, int& h) const;
    virtual QRegionContain ODoContainsPoint(int x, int y) const;
    virtual QRegionContain ODoContainsRect(const QRect& rect) const;

    virtual bool ODoOffset(int x, int y);
    virtual bool ODoUnionWithRect(const QRect& rect);
    virtual bool ODoUnionWithRegion(const OCPNRegion& region);
    virtual bool ODoIntersect(const OCPNRegion& region);
    virtual bool ODoSubtract(const OCPNRegion& region);
//    virtual bool DoXor(const OCPNRegion& region);
    

#endif
    
private:
    DECLARE_DYNAMIC_CLASS(OCPNRegion);
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
#ifdef USE_NEW_REGION
    void Init();
    void CreateRects( const OCPNRegion& r );

    size_t   m_current;
    OCPNRegion m_region;

    QRect *m_rects;
    size_t  m_numRects;
#else
    QRegionIterator *m_ri;
#endif
};


#endif
// _OCPN_REGION_H_
