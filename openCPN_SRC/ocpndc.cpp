/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Layer to perform QDC drawing using QDC or opengl
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Sean D'Epagnier                                 *
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
 */

#include "zchxLog.h"
#include "config.h"

#include "dychart.h"
#include "ocpn_plugin.h"

#ifdef __WIN32
#include <windows.h>
#endif

#include <vector>
#include "ocpndc.h"
#include "wx28compat.h"
#include "cutil.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
extern zchxGLOptions g_GLOptions;
#endif

extern float g_GLMinSymbolLineWidth;
QList<void*> gTesselatorVertices;

//----------------------------------------------------------------------------
/* pass the dc to the constructor, or NULL to use opengl */
ocpnDC::ocpnDC( glChartCanvas *canvas ) :
    glcanvas( canvas ), dc( NULL ), m_pen( QPen() ), m_brush( QBrush() )
{
#if QUSE_GRAPHICS_CONTEXT
    pgc = NULL;
#endif
#ifdef ocpnUSE_GL
    m_textforegroundcolour = QColor( 0, 0, 0 );
#endif   
    m_buseTex = /*GetLocaleCanonicalName().IsSameAs(_T("en_US"))*/true;
}

//ocpnDC::ocpnDC( QPainter* painter ) :
//        glcanvas( NULL ), dc( painter ), m_pen( QPen() ), m_brush( QBrush( )
//{
//#if QUSE_GRAPHICS_CONTEXT
//    pgc = NULL;
//    QMemoryDC *pmdc = QDynamicCast(dc, QMemoryDC);
//    if( pmdc ) pgc = QGraphicsContext::Create( *pmdc );
//    else {
//        QClientDC *pcdc = QDynamicCast(dc, QClientDC);
//        if( pcdc ) pgc = QGraphicsContext::Create( *pcdc );
//    }
//#endif
//    m_textforegroundcolour = QColour( 0, 0, 0 );
//    m_buseTex = /*GetLocaleCanonicalName().IsSameAs(_T("en_US"))*/true;
//}

ocpnDC::ocpnDC() :
    glcanvas( NULL ), dc( NULL ), m_pen( QPen() ), m_brush( QBrush() )
{
#if QUSE_GRAPHICS_CONTEXT
    pgc = NULL;
#endif
    m_buseTex = /*GetLocaleCanonicalName().IsSameAs(_T("en_US"))*/true;
}

ocpnDC::~ocpnDC()
{
#if QUSE_GRAPHICS_CONTEXT
    if( pgc ) delete pgc;
#endif
}

void ocpnDC::Clear()
{
    if( dc ) dc->eraseRect(0, 0, dc->device()->width(), dc->device()->height());
    else {
#ifdef ocpnUSE_GL
        QBrush tmpBrush = m_brush;
        int w = glcanvas->width(), h = glcanvas->height();
        SetBrush( QBrush( glcanvas->palette().background() ) );
        //        glcanvas->GetSize( &w, &h );
        DrawRectangle( 0, 0, w, h);
        SetBrush( tmpBrush );
#endif        
    }
}

void ocpnDC::SetBackground( const QBrush &brush )
{
    //    if( dc )
    //    {
    //        dc->SetBackground( brush );
    //    } else {
    //#ifdef ocpnUSE_GL
    QPalette pal = glcanvas->palette();
    pal.setBrush(QPalette::Window, brush);
    glcanvas->setPalette(pal);
    //        glcanvas->setBackgroundColor( brush.GetColour() );
    //#endif
    //    }
}

void ocpnDC::SetPen( const QPen &pen )
{
    if( dc ) {
        if( pen == QPen() )
        {
            dc->setPen(Qt::transparent );
        } else
        {
            dc->setPen( pen );
        }
    } else
        m_pen = pen;
}

void ocpnDC::SetBrush( const QBrush &brush )
{
    if( dc ) dc->setBrush( brush );
    else
        m_brush = brush;
}

void ocpnDC::SetTextForeground( const QColor &color )
{
    m_textforegroundcolour = color;
}

void ocpnDC::SetFont( const QFont& font )
{
    //    if( dc ) dc->SetFont( font );
    //    else
    m_font = font;
}

const QPen& ocpnDC::GetPen() const
{
    //    if( dc ) return dc->GetPen();
    return m_pen;
}

const QBrush& ocpnDC::GetBrush() const
{
    //    if( dc ) return dc->GetBrush();
    return m_brush;
}

const QFont& ocpnDC::GetFont() const
{
    //    if( dc ) return dc->GetFont();
    return m_font;
}

void ocpnDC::GetSize( uint *width, uint *height ) const
{
    *width = glcanvas->width();
    *height = glcanvas->height();
    //    if( dc )
    //        dc->GetSize( width, height );
    //    else {
    //#ifdef ocpnUSE_GL

    //        glcanvas->GetSize( width, height );
    //#endif
    //    }
}

void ocpnDC::SetGLAttrs( bool highQuality )
{
    // Enable anti-aliased polys, at best quality
    if( highQuality ) {
        if( g_GLOptions.m_GLLineSmoothing )
            glEnable( GL_LINE_SMOOTH );
        if( g_GLOptions.m_GLPolygonSmoothing )
            glEnable( GL_POLYGON_SMOOTH );
        glEnable( GL_BLEND );
    } else {
        glDisable(GL_LINE_SMOOTH);
        glDisable( GL_POLYGON_SMOOTH );
        glDisable( GL_BLEND );
    }
}

void ocpnDC::SetGLStipple() const
{
    switch( m_pen.style() ) {
    case Qt::DotLine: {
        glLineStipple( 1, 0x3333 );
        glEnable( GL_LINE_STIPPLE );
        break;
    }
    case Qt::DashLine: {
        glLineStipple( 1, 0xFFF8 );
        glEnable( GL_LINE_STIPPLE );
        break;
    }
    case Qt::DashDotLine: {
        glLineStipple( 1, 0x3F3F );
        glEnable( GL_LINE_STIPPLE );
        break;
    }
    case Qt::DashDotDotLine: {
        glLineStipple( 1, 0x8FF1 );
        glEnable( GL_LINE_STIPPLE );
        break;
    }
    default: break;
    }
}

#ifdef ocpnUSE_GL
/* draw a half circle using triangles */
void DrawEndCap(float x1, float y1, float t1, float angle)
{
    const int steps = 16;
    float xa, ya;
    bool first = true;
    for(int i = 0; i <= steps; i++) {
        float a = angle + M_PI/2 + M_PI/steps*i;

        float xb = x1 + t1 / 2 * cos( a );
        float yb = y1 + t1 / 2 * sin( a );
        if(first)
            first = false;
        else {
            glVertex2f( x1, y1 );
            glVertex2f( xa, ya );
            glVertex2f( xb, yb );
        }
        xa = xb, ya = yb;
    }
}
#endif

// Draws a line between (x1,y1) - (x2,y2) with a start thickness of t1
void DrawGLThickLine( float x1, float y1, float x2, float y2, QPen pen, bool b_hiqual )
{
#ifdef ocpnUSE_GL
    
    float angle = atan2f( y2 - y1, x2 - x1 );
    float t1 = pen.widthF();
    float t2sina1 = t1 / 2 * sinf( angle );
    float t2cosa1 = t1 / 2 * cosf( angle );

    glBegin( GL_TRIANGLES );

    //    n.b.  The dQDash interpretation for GL only allows for 2 elements in the dash table.
    //    The first is assumed drawn, second is assumed space
    QVector<qreal> dashes = pen.dashPattern();
    int n_dashes = dashes.size();
    if( n_dashes ) {
        float lpix = sqrtf( powf( (float) (x1 - x2), 2) + powf( (float) (y1 - y2), 2) );
        float lrun = 0.;
        float xa = x1;
        float ya = y1;
        float ldraw = t1 * (unsigned char)dashes[0];
        float lspace = t1 * (unsigned char)dashes[1];

        if((ldraw < 0) || (lspace < 0)){
            glEnd();
            return;
        }
        
        while( lrun < lpix ) {
            //    Dash
            float xb = xa + ldraw * cosf( angle );
            float yb = ya + ldraw * sinf( angle );

            if( ( lrun + ldraw ) >= lpix )         // last segment is partial draw
            {
                xb = x2;
                yb = y2;
            }

            glVertex2f( xa + t2sina1, ya - t2cosa1 );
            glVertex2f( xb + t2sina1, yb - t2cosa1 );
            glVertex2f( xb - t2sina1, yb + t2cosa1 );

            glVertex2f( xb - t2sina1, yb + t2cosa1 );
            glVertex2f( xa - t2sina1, ya + t2cosa1 );
            glVertex2f( xa + t2sina1, ya - t2cosa1 );

            xa = xb;
            ya = yb;
            lrun += ldraw;

            //    Space
            xb = xa + lspace * cos( angle );
            yb = ya + lspace * sin( angle );

            xa = xb;
            ya = yb;
            lrun += lspace;
        }
    } else {
        glVertex2f( x1 + t2sina1, y1 - t2cosa1 );
        glVertex2f( x2 + t2sina1, y2 - t2cosa1 );
        glVertex2f( x2 - t2sina1, y2 + t2cosa1 );

        glVertex2f( x2 - t2sina1, y2 + t2cosa1 );
        glVertex2f( x1 - t2sina1, y1 + t2cosa1 );
        glVertex2f( x1 + t2sina1, y1 - t2cosa1 );

        /* Q draws a nice rounded end in dc mode, so replicate
           this for opengl mode, should this be done for the dashed mode case? */
        if(pen.capStyle() == Qt::RoundCap) {
            DrawEndCap( x1, y1, t1, angle);
            DrawEndCap( x2, y2, t1, angle + M_PI);
        }

    }

    glEnd();
#endif    
}

void ocpnDC::DrawLine( int x1, int y1, int x2, int y2, bool b_hiqual )
{
    if(!ConfigurePen() ) return;
    bool b_draw_thick = false;

    float pen_width = fmax(g_GLMinSymbolLineWidth, m_pen.width());

    //      Enable anti-aliased lines, at best quality
    if( b_hiqual ) {
        SetGLStipple();

#ifndef __QQT__
        glEnable( GL_BLEND );
        if( g_GLOptions.m_GLLineSmoothing )
            glEnable( GL_LINE_SMOOTH );
#endif            

        if( pen_width > 1.0 ) {
            GLint parms[2];
            glGetIntegerv( GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0] );
            if( pen_width > parms[1] ) b_draw_thick = true;
            else
                glLineWidth( pen_width );
        } else
            glLineWidth( pen_width );
    } else {
        if( pen_width > 1 ) {
            GLint parms[2];
            glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
            if( pen_width > parms[1] ) b_draw_thick = true;
            else
                glLineWidth( pen_width );
        } else
            glLineWidth( pen_width );
    }

    if( b_draw_thick ) DrawGLThickLine( x1, y1, x2, y2, m_pen, b_hiqual );
    else {
        QVector<qreal> dashes = m_pen.dashPattern();
        int n_dashes = dashes.size();
        if( n_dashes ) {
            float angle = atan2f( (float) ( y2 - y1 ), (float) ( x2 - x1 ) );
            float cosa = cosf( angle );
            float sina = sinf( angle );
            float t1 = m_pen.widthF();

            float lpix = sqrtf( powf(x1 - x2, 2) + powf(y1 - y2, 2) );
            float lrun = 0.;
            float xa = x1;
            float ya = y1;
            float ldraw = t1 * dashes[0];
            float lspace = t1 * dashes[1];

            ldraw = fmax(ldraw, 4.0);
            lspace = fmax(lspace, 4.0);
            lpix = fmin(lpix, 2000.0);

            glBegin( GL_LINES );
            while( lrun < lpix ) {
                //    Dash
                float xb = xa + ldraw * cosa;
                float yb = ya + ldraw * sina;

                if( ( lrun + ldraw ) >= lpix )         // last segment is partial draw
                {
                    xb = x2;
                    yb = y2;
                }

                glVertex2f( xa, ya );
                glVertex2f( xb, yb );

                xa = xa + ( lspace + ldraw ) * cosa;
                ya = ya + ( lspace + ldraw ) * sina;
                lrun += lspace + ldraw;

            }
            glEnd();
        } else                    // not dashed
        {
            glBegin( GL_LINES );
            glVertex2i( x1, y1 );
            glVertex2i( x2, y2 );
            glEnd();
        }
    }

    glDisable( GL_LINE_STIPPLE );

    if( b_hiqual ) {
        glDisable( GL_LINE_SMOOTH );
        glDisable( GL_BLEND );
    }
}

// Draws thick lines from triangles
void DrawGLThickLines( int n, zchxPoint points[],int xoffset,
                       int yoffset, QPen pen, bool b_hiqual )
{
#ifdef ocpnUSE_GL
    if(n < 2)
        return;

    /* for dashed case, for now just draw thick lines */
    QVector<qreal> dashes = pen.dashPattern();
    if( dashes.size() > 0 )
    {
        zchxPoint p0 = points[0];
        for( int i = 1; i < n; i++ ) {
            DrawGLThickLine( p0.x + xoffset, p0.y + yoffset, points[i].x + xoffset,
                             points[i].y + yoffset, pen, b_hiqual );
            p0 = points[i];
        }
        return;
    }

    /* cull zero segments */
    zchxPoint *cpoints = new zchxPoint[n];
    cpoints[0] = points[0];
    int c = 1;
    for( int i = 1; i < n; i++ ) {
        if(points[i].x != points[i-1].x || points[i].y != points[i-1].y)
            cpoints[c++] = points[i];
    }

    /* nicer than than rendering each segment separately, this is because thick
       line segments drawn as rectangles which have different angles have
       rectangles which overlap and also leave a gap.
       This code properly calculates vertexes for adjoining segments */
    float t1 = pen.widthF();

    float x0 = cpoints[0].x, y0 = cpoints[0].y, x1 = cpoints[1].x, y1 = cpoints[1].y;
    float a0 = atan2f( y1 - y0, x1 - x0 );

    // It is also possible to use triangle strip, (and triangle fan for endcap)
    // to reduce vertex count.. is it worth it?
    glBegin( GL_TRIANGLES );

    float t2sina0 = t1 / 2 * sinf( a0 );
    float t2cosa0 = t1 / 2 * cosf( a0 );

    for( int i = 1; i < c; i++ ) {
        float x2, y2;
        float a1;

        if(i < c - 1) {
            x2 = cpoints[i + 1].x, y2 = cpoints[i + 1].y;
            a1 = atan2f( y2 - y1, x2 - x1 );
        } else {
            x2 = x1, y2 = y1;
            a1 = a0;
        }

        float aa = (a0 + a1) / 2;
        float diff = fabsf(a0 - a1);
        if(diff > M_PI)
            diff -= 2 * (float)M_PI;
        float rad = t1 / 2 / fmax(cosf(diff / 2), .4);

        float t2sina1 = rad * sinf( aa );
        float t2cosa1 = rad * cosf( aa );

        glVertex2f( x1 + t2sina1, y1 - t2cosa1 );
        glVertex2f( x1 - t2sina1, y1 + t2cosa1 );
        glVertex2f( x0 + t2sina0, y0 - t2cosa0 );

        glVertex2f( x0 - t2sina0, y0 + t2cosa0 );
        glVertex2f( x0 + t2sina0, y0 - t2cosa0 );

        float dot = t2sina0 * t2sina1 + t2cosa0 * t2cosa1;
        if(dot > 0)
            glVertex2f( x1 - t2sina1, y1 + t2cosa1 );
        else
            glVertex2f( x1 + t2sina1, y1 - t2cosa1 );

        x0 = x1, x1 = x2;
        y0 = y1, y1 = y2;
        a0 = a1;
        t2sina0 = t2sina1, t2cosa0 = t2cosa1;
    }

    if(pen.capStyle() == Qt::RoundCap) {
        DrawEndCap( x0, y0, t1, a0);
        DrawEndCap( x0, y0, t1, a0 + M_PI);
    }

    glEnd();

    delete [] cpoints;

#endif
}

void ocpnDC::DrawLines( int n, zchxPoint points[], int xoffset, int yoffset, bool b_hiqual )
{
    if( ConfigurePen() ) {

        SetGLAttrs( b_hiqual );
        bool b_draw_thick = false;

        glDisable( GL_LINE_STIPPLE );
        SetGLStipple();

        //      Enable anti-aliased lines, at best quality
        if( b_hiqual ) {
            glEnable( GL_BLEND );
            if( m_pen.width() > 1 ) {
                GLint parms[2];
                glGetIntegerv( GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0] );
                if( m_pen.width() > parms[1] ) b_draw_thick = true;
                else
                    glLineWidth( fmax(g_GLMinSymbolLineWidth, m_pen.widthF()) );
            } else
                glLineWidth( fmax(g_GLMinSymbolLineWidth, 1) );
        } else {
            if( m_pen.width() > 1 ) {
                GLint parms[2];
                glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
                if( m_pen.width() > parms[1] ) b_draw_thick = true;
                else
                    glLineWidth( fmax(g_GLMinSymbolLineWidth, m_pen.width()) );
            } else
                glLineWidth( fmax(g_GLMinSymbolLineWidth, 1) );
        }

        if( b_draw_thick) {
            DrawGLThickLines( n, points, xoffset, yoffset, m_pen, b_hiqual );
        } else {

            if( b_hiqual ) {
                if( g_GLOptions.m_GLLineSmoothing )
                    glEnable( GL_LINE_SMOOTH );
                //                SetGLStipple(m_pen.GetStyle());
            }

            glBegin( GL_LINE_STRIP );
            for( int i = 0; i < n; i++ )
                glVertex2i( points[i].x + xoffset, points[i].y + yoffset );
            glEnd();
        }

        if( b_hiqual ) {
            glDisable( GL_LINE_STIPPLE );
            glDisable( GL_POLYGON_SMOOTH );
            glDisable( GL_BLEND );
        }
    }
}

void ocpnDC::StrokeLine( int x1, int y1, int x2, int y2 )
{
#if QUSE_GRAPHICS_CONTEXT
    if( pgc ) {
        pgc->SetPen( dc->GetPen() );
        pgc->StrokeLine( x1, y1, x2, y2 );

        dc->CalcBoundingBox( x1, y1 );
        dc->CalcBoundingBox( x2, y2 );
    } else
#endif
        DrawLine( x1, y1, x2, y2, true );
}

void ocpnDC::StrokeLines( int n, zchxPoint *points) {
    if(n < 2) /* optimization and also to avoid assertion in pgc->StrokeLines */
        return;

#if QUSE_GRAPHICS_CONTEXT
    if( pgc ) {
        QPoint2DDouble* dPoints = (QPoint2DDouble*) malloc( n * sizeof( QPoint2DDouble ) );
        for( int i=0; i<n; i++ ) {
            dPoints[i].m_x = points[i].x;
            dPoints[i].m_y = points[i].y;
        }
        pgc->SetPen( dc->GetPen() );
        pgc->StrokeLines( n, dPoints );
        free( dPoints );
    } else
#endif
        DrawLines( n, points, 0, 0, true );
}

void ocpnDC::DrawRectangle( int x, int y, uint w, uint h )
{
    if( ConfigureBrush() ) {
        glBegin( GL_QUADS );
        glVertex2i( x, y );
        glVertex2i( x + w, y );
        glVertex2i( x + w, y + h );
        glVertex2i( x, y + h );
        glEnd();
    }

    if( ConfigurePen() ) {
        glBegin( GL_LINE_LOOP );
        glVertex2i( x, y );
        glVertex2i( x + w, y );
        glVertex2i( x + w, y + h );
        glVertex2i( x, y + h );
        glEnd();
    }
}

/* draw the arc along corners */
static void drawrrhelper( int x0, int y0, int r, int quadrant, int steps )
{
    float step = 1.0/steps, rs = 2.0*r*step, rss = rs*step, x, y, dx, dy, ddx, ddy;
    switch(quadrant) {
    case 0: x =  r, y =  0, dx =   0, dy = -rs, ddx = -rss, ddy =  rss; break;
    case 1: x =  0, y = -r, dx = -rs, dy =   0, ddx =  rss, ddy =  rss; break;
    case 2: x = -r, y =  0, dx =   0, dy =  rs, ddx =  rss, ddy = -rss; break;
    case 3: x =  0, y =  r, dx =  rs, dy =   0, ddx = -rss, ddy = -rss; break;
    default: return; // avoid unitialized compiler warnings
    }

    for(int i=0; i<steps; i++) {
        glVertex2i( x0 + floor(x), y0 + floor(y) );
        x += dx+ddx/2,  y += dy+ddy/2;
        dx += ddx,      dy += ddy;
    }
    glVertex2i( x0 + floor(x), y0 + floor(y) );
}

void ocpnDC::DrawRoundedRectangle( int x, int y, uint w, uint h, int r )
{
        r++;
        int steps = ceil(sqrt((float)r));

        int x1 = x + r, x2 = x + w - r;
        int y1 = y + r, y2 = y + h - r;
        if( ConfigureBrush() ) {
            glBegin( GL_TRIANGLE_FAN );
            drawrrhelper( x2, y1, r, 0, steps );
            drawrrhelper( x1, y1, r, 1, steps );
            drawrrhelper( x1, y2, r, 2, steps );
            drawrrhelper( x2, y2, r, 3, steps );
            glEnd();
        }

        if( ConfigurePen() ) {
            glBegin( GL_LINE_LOOP );
            drawrrhelper( x2, y1, r, 0, steps );
            drawrrhelper( x1, y1, r, 1, steps );
            drawrrhelper( x1, y2, r, 2, steps );
            drawrrhelper( x2, y2, r, 3, steps );
            glEnd();
        }
}

void ocpnDC::DrawCircle( int x, int y, int radius )
{
    DrawEllipse( x - radius, y - radius, 2 * radius, 2 * radius );
}

void ocpnDC::StrokeCircle( int x, int y, int radius )
{
#if QUSE_GRAPHICS_CONTEXT
    if( pgc ) {
        QGraphicsPath gpath = pgc->CreatePath();
        gpath.AddCircle( x, y, radius );

        pgc->SetPen( GetPen() );
        pgc->SetBrush( GetBrush() );
        pgc->DrawPath( gpath );

        // keep dc dirty box up-to-date
        dc->CalcBoundingBox( x + radius + 2, y + radius + 2 );
        dc->CalcBoundingBox( x - radius - 2, y - radius - 2 );
    } else
#endif
        DrawCircle( x, y, radius );
}

void ocpnDC::DrawEllipse( int x, int y, int width, int height )
{
        float r1 = width / 2, r2 = height / 2;
        float cx = x + r1, cy = y + r2;

        //      Enable anti-aliased lines, at best quality
        glEnable( GL_BLEND );

        /* formula for variable step count to produce smooth ellipse */
        float steps = floorf(fmax(sqrtf(sqrtf((float)(width*width + height*height))), 1) * M_PI);

        if( ConfigureBrush() ) {
            glBegin( GL_TRIANGLE_FAN );
            glVertex2f( cx, cy );
            for( float a = 0; a <= 2 * M_PI + M_PI/steps; a += 2 * M_PI / steps )
                glVertex2f( cx + r1 * sinf( a ), cy + r2 * cosf( a ) );
            glEnd();
        }

        if( ConfigurePen() ) {
            glBegin( GL_LINE_LOOP );
            for( float a = 0; a < 2 * M_PI - M_PI/steps; a += 2 * M_PI / steps )
                glVertex2f( cx + r1 * sinf( a ), cy + r2 * cosf( a ) );
            glEnd();
        }

        glDisable( GL_BLEND );
}

void ocpnDC::DrawPolygon( int n, zchxPoint points[], int xoffset, int yoffset, float scale )
{
        
#ifdef __QQT__
        SetGLAttrs( false );            // Some QT platforms (Android) have trouble with GL_BLEND / GL_LINE_SMOOTH
#else
        SetGLAttrs( true );
#endif        

        if( ConfigureBrush() ) {
            if( g_GLOptions.m_GLPolygonSmoothing )
                glEnable( GL_POLYGON_SMOOTH );
            glBegin( GL_POLYGON );
            for( int i = 0; i < n; i++ )
                glVertex2f( (points[i].x * scale) + xoffset, (points[i].y * scale) + yoffset );
            glEnd();
            glDisable( GL_POLYGON_SMOOTH );
        }

        if( ConfigurePen() ) {
            if( g_GLOptions.m_GLLineSmoothing )
                glEnable( GL_LINE_SMOOTH );
            glBegin( GL_LINE_LOOP );
            for( int i = 0; i < n; i++ )
                glVertex2f( (points[i].x * scale) + xoffset, (points[i].y * scale) + yoffset );
            glEnd();
            glDisable( GL_LINE_SMOOTH );
        }

        SetGLAttrs( false );
}

#ifdef ocpnUSE_GL

typedef union {
    GLdouble data[6];
    struct sGLvertex {
        GLdouble x;
        GLdouble y;
        GLdouble z;
        GLdouble r;
        GLdouble g;
        GLdouble b;
    } info;
} GLvertex;

void APIENTRY ocpnDCcombineCallback( GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4],
GLdouble **dataOut )
{
    GLvertex *vertex;

    vertex = new GLvertex();
    gTesselatorVertices.append(vertex );

    vertex->info.x = coords[0];
    vertex->info.y = coords[1];
    vertex->info.z = coords[2];

    for( int i = 3; i < 6; i++ ) {
        vertex->data[i] = weight[0] * vertex_data[0][i] + weight[1] * vertex_data[1][i];
    }

    *dataOut = &(vertex->data[0]);
}

void APIENTRY ocpnDCvertexCallback( GLvoid* arg )
{
    GLvertex* vertex;
    vertex = (GLvertex*) arg;
    glVertex2f( (float)vertex->info.x, (float)vertex->info.y );
}

void APIENTRY ocpnDCerrorCallback( GLenum errorCode )
{
    const GLubyte *estring;
    estring = gluErrorString(errorCode);
    qDebug("OpenGL Tessellation Error: %s", (char *)estring);
}

void APIENTRY ocpnDCbeginCallback( GLenum type )
{
    glBegin( type );
}

void APIENTRY ocpnDCendCallback()
{
    glEnd();
}
#endif          //#ifdef ocpnUSE_GL

void ocpnDC::DrawPolygonTessellated( int n, zchxPoint points[], int xoffset, int yoffset )
{
# ifndef ocpnUSE_GLES  // tessalator in glues is broken
        if( n < 5 )
# endif
        {
            DrawPolygon( n, points, xoffset, yoffset );
            return;
        }

        static GLUtesselator *tobj = NULL;
        if( ! tobj ) tobj = gluNewTess();

        gluTessCallback( tobj, GLU_TESS_VERTEX, (_GLUfuncptr) &ocpnDCvertexCallback );
        gluTessCallback( tobj, GLU_TESS_BEGIN, (_GLUfuncptr) &ocpnDCbeginCallback );
        gluTessCallback( tobj, GLU_TESS_END, (_GLUfuncptr) &ocpnDCendCallback );
        gluTessCallback( tobj, GLU_TESS_COMBINE, (_GLUfuncptr) &ocpnDCcombineCallback );
        gluTessCallback( tobj, GLU_TESS_ERROR, (_GLUfuncptr) &ocpnDCerrorCallback );

        gluTessNormal( tobj, 0, 0, 1);
        gluTessProperty( tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO );

        if( ConfigureBrush() ) {
            gluTessBeginPolygon( tobj, NULL );
            gluTessBeginContour( tobj );

            for( int i = 0; i < n; i++ ) {
                GLvertex* vertex = new GLvertex();
                gTesselatorVertices.append(vertex );
                vertex->info.x = (GLdouble) points[i].x;
                vertex->info.y = (GLdouble) points[i].y;
                vertex->info.z = (GLdouble) 0.0;
                vertex->info.r = (GLdouble) 0.0;
                vertex->info.g = (GLdouble) 0.0;
                vertex->info.b = (GLdouble) 0.0;
                gluTessVertex( tobj, (GLdouble*)vertex, (GLdouble*)vertex );
            }
            gluTessEndContour( tobj );
            gluTessEndPolygon( tobj );
        }

        for( unsigned int i=0; i<gTesselatorVertices.count(); i++ )
            delete (GLvertex*)gTesselatorVertices[i];
        gTesselatorVertices.clear();
}

void ocpnDC::StrokePolygon( int n, zchxPoint points[], int xoffset, int yoffset, float scale )
{
#if QUSE_GRAPHICS_CONTEXT
    if( pgc ) {
        QGraphicsPath gpath = pgc->CreatePath();
        gpath.MoveToPoint( points[0].x * scale + xoffset, points[0].y  * scale + yoffset );
        for( int i = 1; i < n; i++ )
            gpath.AddLineToPoint( points[i].x * scale + xoffset, points[i].y * scale + yoffset );
        gpath.AddLineToPoint( points[0].x * scale + xoffset, points[0].y * scale + yoffset );

        pgc->SetPen( GetPen() );
        pgc->SetBrush( GetBrush() );
        pgc->DrawPath( gpath );

        for( int i = 0; i < n; i++ )
            dc->CalcBoundingBox( points[i].x * scale + xoffset, points[i].y * scale + yoffset );
    } else
#endif
        DrawPolygon( n, points, xoffset, yoffset, scale );
}

void ocpnDC::DrawBitmap( const wxBitmap &bitmap, int x, int y, bool usemask )
{
    wxBitmap bmp;
    if( x < 0 || y < 0 ) {
        int dx = ( x < 0 ? -x : 0 );
        int dy = ( y < 0 ? -y : 0 );
        int w = bitmap.GetWidth() - dx;
        int h = bitmap.GetHeight() - dy;
        /* picture is out of viewport */
        if( w <= 0 || h <= 0 ) return;
        wxBitmap newBitmap = bitmap.GetSubBitmap( QRect( dx, dy, w, h ) );
        x += dx;
        y += dy;
        bmp = newBitmap;
    } else {
        bmp = bitmap;
    }

#ifdef ocpnUSE_GLES  // Do not attempt to do anything with glDrawPixels if using opengles
        return; // this should not be hit anymore ever anyway
#endif
        QImage image = bmp.ConvertToImage();
        int w = image.width(), h = image.height();

        if( usemask ) {
            unsigned char *d = image.bits();
            unsigned char *a = image.alphaChannel().bits();

#ifdef __QOSX__
            if(image.HasMask())
                a=0;
#endif
            unsigned char mr, mg, mb;
//            if( !a && !image.GetOrFindMaskColour( &mr, &mg, &mb ) ){
//                printf("trying to use mask to draw a bitmap without alpha or mask\n" );
//            }

            
            unsigned char *e = new unsigned char[4 * w * h];
            if(e && d){
                for( int y = 0; y < h; y++ )
                    for( int x = 0; x < w; x++ ) {
                        unsigned char r, g, b;
                        int off = ( y * w + x );
                        r = d[off * 3 + 0];
                        g = d[off * 3 + 1];
                        b = d[off * 3 + 2];

                        e[off * 4 + 0] = r;
                        e[off * 4 + 1] = g;
                        e[off * 4 + 2] = b;

                        e[off * 4 + 3] =
                                a ? a[off] : ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                        //                        e[off * 4 + 3] = ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                    }
            }

            glColor4f( 1, 1, 1, 1 );
            GLDrawBlendData( x, y, w, h, GL_RGBA, e );
            delete[] ( e );
        } else {
            glRasterPos2i( x, y );
            glPixelZoom( 1, -1 ); /* draw data from top to bottom */
            if(image.bits())
                glDrawPixels( w, h, GL_RGB, GL_UNSIGNED_BYTE, image.bits() );
            glPixelZoom( 1, 1 );
        }
}

void ocpnDC::DrawText( const QString &text, int x, int y )
{

        int w = 0;
        int h = 0;

        if(m_buseTex){
            TexFont *texfont = GetTexFont( &m_font );

            texfont->GetTextExtent(text, &w, &h);
            
            if( w && h ) {
                
                glEnable( GL_BLEND );
                glEnable( GL_TEXTURE_2D );
                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

                glPushMatrix();
                glTranslatef(x, y, 0);
                
                glColor3ub( m_textforegroundcolour.red(), m_textforegroundcolour.green(),
                            m_textforegroundcolour.blue() );
                

                texfont->RenderString(text);
                glPopMatrix();

                glDisable( GL_TEXTURE_2D );
                glDisable( GL_BLEND );

            }
        }
#if 0
        else{
            QScreenDC sdc;
            sdc.SetFont(m_font);
            sdc.GetTextExtent(text, &w, &h, NULL, NULL, &m_font);
            
            /* create bitmap of appropriate size and select it */
            QBitmap bmp( w, h );
            QMemoryDC temp_dc;
            temp_dc.SelectObject( bmp );

            /* fill bitmap with black */
            temp_dc.SetBackground( QBrush( QColour( 0, 0, 0 ) ) );
            temp_dc.Clear();

            /* draw the text white */
            temp_dc.SetFont( m_font );
            temp_dc.SetTextForeground( QColour( 255, 255, 255 ) );
            temp_dc.DrawText( text, 0, 0 );
            temp_dc.SelectObject( QNullBitmap );

            /* use the data in the bitmap for alpha channel,
             and set the color to text foreground */
            QImage image = bmp.ConvertToImage();
            if( x < 0 || y < 0 ) { // Allow Drawing text which is offset to start off screen
                int dx = ( x < 0 ? -x : 0 );
                int dy = ( y < 0 ? -y : 0 );
                w = bmp.GetWidth() - dx;
                h = bmp.GetHeight() - dy;
                /* picture is out of viewport */
                if( w <= 0 || h <= 0 ) return;
                image = image.GetSubImage( QRect( dx, dy, w, h ) );
                x += dx;
                y += dy;
            }

            unsigned char *data = new unsigned char[w * h * 4];
            unsigned char *im = image.GetData();
            
            
            if(im){
                unsigned int r = m_textforegroundcolour.Red();
                unsigned int g = m_textforegroundcolour.Green();
                unsigned int b = m_textforegroundcolour.Blue();
                for( int i = 0; i < h; i++ ){
                    for(int j=0 ; j < w ; j++){
                        unsigned int index = ((i*w) + j) * 4;
                        data[index] = r;
                        data[index+1] = g;
                        data[index+2] = b;
                        data[index+3] = im[((i*w) + j) * 3];
                    }
                }
            }
#if 0
            glColor4ub( 255, 255, 255, 255 );
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glRasterPos2i( x, y );
            glPixelZoom( 1, -1 );
            glDrawPixels( w, h, GL_RGBA, GL_UNSIGNED_BYTE, data );
            glPixelZoom( 1, 1 );
            glDisable( GL_BLEND );
#else
            unsigned int texobj;
            
            glGenTextures(1, &texobj);
            glBindTexture(GL_TEXTURE_2D, texobj);
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            
            int TextureWidth = NextPow2(w);
            int TextureHeight = NextPow2(h);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
            
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            
            glColor3ub(0,0,0);
            
            float u = (float)w/TextureWidth, v = (float)h/TextureHeight;
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex2f(x, y);
            glTexCoord2f(u, 0); glVertex2f(x+w, y);
            glTexCoord2f(u, v); glVertex2f(x+w, y+h);
            glTexCoord2f(0, v); glVertex2f(x, y+h);
            glEnd();
            
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            
            glDeleteTextures(1, &texobj);
#endif            
            delete[] data;
        }
#endif
}

void ocpnDC::GetTextExtent( const QString &string, int *w, int *h, int *descent,
                            int *externalLeading, QFont *font )
{
    //  Give at least reasonable results on failure.
    if(w) *w = 100;
    if(h) *h = 100;

        QFont f = m_font;
        if( font ) f = *font;

        if(m_buseTex){
            TexFont *texfont = GetTexFont(&f);

            texfont->GetTextExtent(string, w, h);
        }

    //  Sometimes GetTextExtent returns really wrong, uninitialized results.
    //  Dunno why....
    if( w && (*w > 500) ) *w = 500;
    if( h && (*h > 500) ) *h = 500;
}

void ocpnDC::ResetBoundingBox()
{
//    if( dc ) dc->ResetBoundingBox();
}

void ocpnDC::CalcBoundingBox( int x, int y )
{
//    if( dc ) dc->CalcBoundingBox( x, y );
}

bool ocpnDC::ConfigurePen()
{
    if( m_pen.color() == Qt::transparent) return false;

    QColor c = m_pen.color();
    int width = m_pen.width();
#ifdef ocpnUSE_GL
    glColor4ub( c.red(), c.green(), c.blue(), c.alpha() );
    glLineWidth( fmax(g_GLMinSymbolLineWidth, width) );
#endif    
    return true;
}

bool ocpnDC::ConfigureBrush()
{
    if(m_brush.color() == Qt::transparent ) return false;
#ifdef ocpnUSE_GL
    QColor c = m_brush.color();
    glColor4ub( c.red(), c.green(), c.blue(), c.alpha() );
#endif    
    return true;
}

void ocpnDC::GLDrawBlendData( int x, int y, int w, int h, int format,
                              const unsigned char *data )
{
#ifdef ocpnUSE_GL
    glEnable( GL_BLEND );
    glRasterPos2i( x, y );
    glPixelZoom( 1, -1 );
    glDrawPixels( w, h, format, GL_UNSIGNED_BYTE, data );
    glPixelZoom( 1, 1 );
    glDisable( GL_BLEND );
#endif
}
