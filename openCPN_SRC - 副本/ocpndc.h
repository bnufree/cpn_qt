/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Layer to use QDC or opengl
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Sean D'Epagnier                                 *
 *   sean at depagnier dot com                                             *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 *f
 */


#ifndef __OCPNDC_H__
#define __OCPNDC_H__

#include <vector>

#include "TexFont.h"
#include <QPen>
#include "bitmap.h"
#include "_def.h"


void DrawGLThickLine( float x1, float y1, float x2, float y2, QPen pen, bool b_hiqual );

//----------------------------------------------------------------------------
// ocpnDC
//----------------------------------------------------------------------------

class glChartCanvas;

class ocpnDC
{
public:
     ocpnDC(glChartCanvas* canvas);
     ocpnDC();

     ~ocpnDC();
     QPainter* GetDC() {return 0;}

     void SetBackground( const QBrush &brush );
     void SetPen( const QPen &pen);
     void SetBrush( const QBrush &brush);
     void SetTextForeground(const QColor &colour);
     void SetFont(const QFont& font);
     static void SetGLAttrs( bool highQuality ); 
     void SetGLStipple() const;

     const QPen& GetPen() const;
     const QBrush& GetBrush() const;
     const QFont& GetFont() const;

     void GetSize(uint *width, uint *height) const;

     void DrawLine( int x1, int y1, int x2, int y2, bool b_hiqual = true);
     void DrawLines( int n, zchxPoint points[], int xoffset = 0, int yoffset = 0, bool b_hiqual = true);

     void StrokeLine( int x1, int y1, int x2, int y2);
     void StrokeLine( QPoint a, QPoint b) { StrokeLine(a.x(), a.y(), b.x(), b.y()); }
     void StrokeLines( int n, zchxPoint *points);

     void Clear();
     void DrawRectangle( int x, int y, uint w, uint h );
     void DrawRoundedRectangle( int x, int y, uint w, uint h, int rr );
     void DrawCircle(int x, int y, int radius);
     void DrawCircle(const QPoint &pt, int radius) { DrawCircle(pt.x(), pt.y(), radius); }
     void StrokeCircle(int x, int y, int radius);

     void DrawEllipse(int x, int y, int width, int height);
     void DrawPolygon(int n, zchxPoint points[], int xoffset = 0, int yoffset = 0, float scale =1.0);
     void DrawPolygonTessellated(int n, zchxPoint points[], int xoffset = 0, int yoffset = 0);
     void StrokePolygon(int n, zchxPoint points[], int xoffset = 0, int yoffset = 0, float scale = 1.0);

     void DrawBitmap(const wxBitmap &bitmap, int x, int y, bool usemask);

     void drawText(const QString &text, int x, int y);
     void GetTextExtent(const QString &string, int *w, int *h, int *descent = NULL,
                        int *externalLeading = NULL, QFont *font = NULL);

     void ResetBoundingBox();
     void CalcBoundingBox(int x, int y);

     void DestroyClippingRegion() {}

//     QPainter *GetDC() const { return dc; }

protected:
     bool ConfigurePen();
     bool ConfigureBrush();

     void GLDrawBlendData(int x, int y, int w, int h,
                          int format, const unsigned char *data);

     glChartCanvas *glcanvas;
     QPainter *dc;
     QPen m_pen;
     QBrush m_brush;
     QColor m_textforegroundcolour;
     QFont m_font;

     bool m_buseTex;

#if  QUSE_GRAPHICS_CONTEXT
     QGraphicsContext *pgc;
#endif
};

#endif
