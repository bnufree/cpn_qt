/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/dc.cpp
// Author:      Peter Most, Javier Torres, Mariano Reingart
// Copyright:   (c) 2009 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include <QtGui/QBitmap>
#include <QtGui/QPen>
#include <QtGui/QPainter>

#include "base/dc.h"
#include "qt/dc.h"
//#include "qt/private/converter.h"
//#include "qt/private/utils.h"

#include <QtGui/QScreen>
#include <QtWidgets/QApplication>
#include <QDebug>
#include <QIcon>

static void SetPenColour( QPainter *qtPainter, QColor col )
{
    QPen p = qtPainter->pen();
    p.setColor( col );
    qtPainter->setPen( p );
}

static void SetBrushColour( QPainter *qtPainter, QColor col )
{
    QBrush b = qtPainter->brush();
    b.setColor( col );
    qtPainter->setBrush( b );
}

wxQtDCImpl::wxQtDCImpl( wxDC *owner )
    : wxDCImpl( owner )
{
    m_clippingRegion = new QRegion;
    m_qtImage = NULL;
    m_rasterColourOp = wxQtNONE;
    m_qtPenColor = new QColor;
    m_qtBrushColor = new QColor;
    m_ok = true;
}

wxQtDCImpl::~wxQtDCImpl()
{
    if ( m_qtPainter )
    {
        if( m_qtPainter->isActive() )
        {
            m_qtPainter->end();
        }
        delete m_qtPainter;
    }

    delete m_clippingRegion;
    delete m_qtPenColor;
    delete m_qtBrushColor;
}

void wxQtDCImpl::QtPreparePainter( )
{
    //Do here all QPainter initialization (called after each begin())
    if ( m_qtPainter == NULL )
    {
        qDebug("wxQtDCImpl::QtPreparePainter is NULL!!!");
    }
    else if ( m_qtPainter->isActive() )
    {
        m_qtPainter->setPen( QPen());
        m_qtPainter->setBrush( QBrush());
        m_qtPainter->setFont( QFont());

        if (m_clipping)
        {
            int size = m_clippingRegion->rects().size();
            bool append = false;
            int i = 0;
            while (i<size)
            {
                QRect r = m_clippingRegion->rects()[i];
                m_qtPainter->setClipRect( r.x(), r.y(), r.width(), r.height(),
                                          append ? Qt::IntersectClip : Qt::ReplaceClip );
                append = true;
                i++;
            }
        }
    }
    else
    {
//        wxLogDebug(wxT("wxQtDCImpl::QtPreparePainter not active!"));
    }
}

bool wxQtDCImpl::CanDrawBitmap() const
{
    return true;
}

bool wxQtDCImpl::CanGetTextExtent() const
{
    return true;
}

void wxQtDCImpl::DoGetSize(int *width, int *height) const
{
    if (width)  *width  = m_qtPainter->device()->width();
    if (height) *height = m_qtPainter->device()->height();
}

void wxQtDCImpl::DoGetSizeMM(int* width, int* height) const
{
    if (width)  *width  = m_qtPainter->device()->widthMM();
    if (height) *height = m_qtPainter->device()->heightMM();
}

int wxQtDCImpl::GetDepth() const
{
    return m_qtPainter->device()->depth();
}

QSize wxQtDCImpl::GetPPI() const
{
    QScreen *srn = QApplication::screens().at(0);
    if (!srn)
        return QSize(m_qtPainter->device()->logicalDpiX(), m_qtPainter->device()->logicalDpiY());
    qreal dotsPerInch = srn->logicalDotsPerInch();
    return QSize(round(dotsPerInch), round(dotsPerInch));
}

void wxQtDCImpl::SetFont(const QFont& font)
{
    m_font = font;

    if (m_qtPainter->isActive())
        m_qtPainter->setFont(font);
}

void wxQtDCImpl::SetPen(const QPen& pen)
{
    m_pen = pen;

    m_qtPainter->setPen(pen);

    ApplyRasterColourOp();
}

void wxQtDCImpl::SetBrush(const QBrush& brush)
{
    m_brush = brush;

    if (brush.style() == Qt::TexturePattern)
    {
        // Use a monochrome mask: use foreground color for the mask
        QBrush b(brush);
        b.setColor(m_textForegroundColour);
        b.setTexture(b.texture().mask());
        m_qtPainter->setBrush(b);
    }
//    else if (brush.GetStyle() == QBrushSTYLE_STIPPLE)
//    {
//        //Don't use the mask
//        QBrush b(brush.GetHandle());

//        QPixmap p = b.texture();
//        p.setMask(QBitmap());
//        b.setTexture(p);

//        m_qtPainter->setBrush(b);
//    }
    else
    {
        m_qtPainter->setBrush(brush);
    }

    ApplyRasterColourOp();
}

void wxQtDCImpl::SetBackground(const QBrush& brush)
{
    m_backgroundBrush = brush;

    if (m_qtPainter->isActive())
        m_qtPainter->setBackground(brush);
}

void wxQtDCImpl::SetBackgroundMode(int mode)
{
    /* Do not change QPainter, as wx uses this background mode
     * only for drawing text, where Qt uses it for everything.
     * Always let QPainter mode to transparent, and change it
     * when needed */
    m_backgroundMode = mode;
}

#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtGui/QScreen>
#include <QtWidgets/QApplication>

#if wxUSE_PALETTE
void wxQtDCImpl::SetPalette(const wxPalette& WXUNUSED(palette))
{
    wxMISSING_IMPLEMENTATION(__FUNCTION__);
}
#endif // wxUSE_PALETTE

void wxQtDCImpl::SetLogicalFunction(wxRasterOperationMode function)
{
    m_logicalFunction = function;

    wxQtRasterColourOp rasterColourOp = wxQtNONE;
    switch ( function )
    {
        case wxCLEAR:       // 0
            m_qtPainter->setCompositionMode( QPainter::CompositionMode_SourceOver );
            rasterColourOp = wxQtBLACK;
            break;
        case wxXOR:         // src XOR dst
            m_qtPainter->setCompositionMode( QPainter::RasterOp_SourceXorDestination );
            break;
        case wxINVERT:      // NOT dst => dst XOR WHITE
            m_qtPainter->setCompositionMode( QPainter::RasterOp_SourceXorDestination );
            rasterColourOp = wxQtWHITE;
            break;
        case wxOR_REVERSE:  // src OR (NOT dst) => (NOT (NOT src)) OR (NOT dst)
            m_qtPainter->setCompositionMode( QPainter::RasterOp_NotSourceOrNotDestination );
            rasterColourOp = wxQtINVERT;
            break;
        case wxAND_REVERSE: // src AND (NOT dst)
            m_qtPainter->setCompositionMode( QPainter::RasterOp_SourceAndNotDestination );
            break;
        case wxCOPY:        // src
            m_qtPainter->setCompositionMode( QPainter::CompositionMode_SourceOver );
            break;
        case wxAND:         // src AND dst
            m_qtPainter->setCompositionMode( QPainter::RasterOp_SourceAndDestination );
            break;
        case wxAND_INVERT:  // (NOT src) AND dst
            m_qtPainter->setCompositionMode( QPainter::RasterOp_NotSourceAndDestination );
            break;
        case wxNO_OP:       // dst
            m_qtPainter->setCompositionMode( QPainter::CompositionMode_DestinationOver );
            break;
        case wxNOR:         // (NOT src) AND (NOT dst)
            m_qtPainter->setCompositionMode( QPainter::RasterOp_NotSourceAndNotDestination );
            break;
        case wxEQUIV:       // (NOT src) XOR dst
            m_qtPainter->setCompositionMode( QPainter::RasterOp_NotSourceXorDestination );
            break;
        case wxSRC_INVERT:  // (NOT src)
            m_qtPainter->setCompositionMode( QPainter::RasterOp_NotSource );
            break;
        case wxOR_INVERT:   // (NOT src) OR dst
            m_qtPainter->setCompositionMode( QPainter::RasterOp_SourceOrDestination );
            rasterColourOp = wxQtINVERT;
            break;
        case wxNAND:        // (NOT src) OR (NOT dst)
            m_qtPainter->setCompositionMode( QPainter::RasterOp_NotSourceOrNotDestination );
            break;
        case wxOR:          // src OR dst
            m_qtPainter->setCompositionMode( QPainter::RasterOp_SourceOrDestination );
            break;
        case wxSET:          // 1
            m_qtPainter->setCompositionMode( QPainter::CompositionMode_SourceOver );
            rasterColourOp = wxQtWHITE;
            break;
    }

    if ( rasterColourOp != m_rasterColourOp )
    {
        // Source colour mode changed
        m_rasterColourOp = rasterColourOp;

        // Restore original colours and apply new mode
        SetPenColour( m_qtPainter, *m_qtPenColor );
        SetBrushColour( m_qtPainter, *m_qtPenColor );

        ApplyRasterColourOp();
    }
}

void wxQtDCImpl::ApplyRasterColourOp()
{
    // Save colours
    *m_qtPenColor = m_qtPainter->pen().color();
    *m_qtBrushColor = m_qtPainter->brush().color();

    // Apply op
    switch ( m_rasterColourOp )
    {
        case wxQtWHITE:
            SetPenColour( m_qtPainter, QColor( Qt::white ) );
            SetBrushColour( m_qtPainter, QColor( Qt::white ) );
            break;
        case wxQtBLACK:
            SetPenColour( m_qtPainter, QColor( Qt::black ) );
            SetBrushColour( m_qtPainter, QColor( Qt::black ) );
            break;
        case wxQtINVERT:
            SetPenColour( m_qtPainter, QColor( ~m_qtPenColor->rgb() ) );
            SetBrushColour( m_qtPainter, QColor( ~m_qtBrushColor->rgb() ) );
            break;
        case wxQtNONE:
            // No op
            break;
    }
}

int wxQtDCImpl::GetCharHeight() const
{
    QFontMetrics metrics(m_qtPainter->font());
    return int( metrics.height() );
}

int wxQtDCImpl::GetCharWidth() const
{
    //FIXME: Returning max width, instead of average
    QFontMetrics metrics(m_qtPainter->font());
    return int( metrics.maxWidth() );
}

void wxQtDCImpl::DoGetTextExtent(const QString& string,
                             int *x, int *y,
                             int *descent,
                             int *externalLeading,
                             const QFont *theFont ) const
{
    QFont f;
    if (theFont != NULL)
        f = *theFont;
    else
        f = m_font;

    QFontMetrics metrics(f);
    if (x != NULL || y != NULL)
    {
        // note that boundingRect doesn't return "advance width" for spaces
        if (x != NULL)
            *x = metrics.width(string);
        if (y != NULL)
            *y = metrics.height();
    }

    if (descent != NULL)
        *descent = metrics.descent();

    if (externalLeading != NULL)
        *externalLeading = metrics.leading();
}

void wxQtDCImpl::Clear()
{
    int width, height;
    DoGetSize(&width, &height);

    m_qtPainter->eraseRect(QRect(0, 0, width, height));
}

void wxQtDCImpl::DoSetClippingRegion(int x, int y,
                                 int width, int height)
{
    // Special case: Empty region -> DestroyClippingRegion()
    if ( width == 0 && height == 0 )
    {
        DestroyClippingRegion();
    }
    else
    {
        if (m_qtPainter->isActive())
        {
            // Set QPainter clipping (intersection if not the first one)
            m_qtPainter->setClipRect( x, y, width, height,
                                      m_clipping ? Qt::IntersectClip : Qt::ReplaceClip );
        }

        // Set internal state for getters
        /* Note: Qt states that QPainter::clipRegion() may be slow, so we
         * keep the region manually, which should be faster */
        if ( m_clipping )
            *m_clippingRegion = m_clippingRegion->united(QRect( x, y, width, height ) );
        else
            *m_clippingRegion = m_clippingRegion->intersected(QRect( x, y, width, height ) );

        QRect clipRect = m_clippingRegion->boundingRect();

        m_clipX1 = clipRect.left();
        m_clipX2 = clipRect.right();
        m_clipY1 = clipRect.top();
        m_clipY2 = clipRect.bottom();
        m_clipping = true;
    }
}

void wxQtDCImpl::DoSetDeviceClippingRegion(const QRegion& region)
{
    if ( region.isEmpty() )
    {
        DestroyClippingRegion();
    }
    else
    {
        QRegion qregion = region;
        // Save current origin / scale (logical coordinates)
        QTransform qtrans = m_qtPainter->worldTransform();
        // Reset transofrmation to match device coordinates
        m_qtPainter->setWorldTransform( QTransform() );
        // Set QPainter clipping (intersection if not the first one)
        m_qtPainter->setClipRegion( qregion,
                                 m_clipping ? Qt::IntersectClip : Qt::ReplaceClip );

        // Restore the transformation (translation / scale):
        m_qtPainter->setWorldTransform( qtrans );

        // Set internal state for getters
        /* Note: Qt states that QPainter::clipRegion() may be slow, so we
        * keep the region manually, which should be faster */
        if ( m_clipping )
            *m_clippingRegion = m_clippingRegion->united(region );
        else
            *m_clippingRegion = m_clippingRegion->intersected(region );

        QRect clipRect = m_clippingRegion->boundingRect();

        m_clipX1 = clipRect.left();
        m_clipX2 = clipRect.right();
        m_clipY1 = clipRect.top();
        m_clipY2 = clipRect.bottom();
        m_clipping = true;
    }
}

void wxQtDCImpl::DestroyClippingRegion()
{
    wxDCImpl::DestroyClippingRegion();
    QRegion null_region;
    m_clippingRegion->swap(null_region);

    if (m_qtPainter->isActive())
        m_qtPainter->setClipping( false );
}

bool wxQtDCImpl::DoFloodFill(int x, int y, const QColor& col,
                         wxFloodFillStyle style )
{
#if wxUSE_IMAGE
    extern bool wxDoFloodFill(wxDC *dc, int x, int y,
                              const QColor & col, wxFloodFillStyle style);

    return wxDoFloodFill( GetOwner(), x, y, col, style);
#else
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(col);
    Q_UNUSED(style);

    return false;
#endif
}

bool wxQtDCImpl::DoGetPixel(int x, int y, QColor *col) const
{
    if(! m_qtPainter->isActive())
    {
        qDebug( "Invalid wxDC" );
    }

    if ( col )
    {
        if( m_qtImage == NULL)
        {
            qDebug("This DC doesn't support GetPixel()" );
        }

        QColor pixel = m_qtImage->pixel( x, y );
        col->setRgb(pixel.red(), pixel.green(), pixel.blue(), pixel.alpha() );

        return true;
    }
    else
        return false;
}

void wxQtDCImpl::DoDrawPoint(int x, int y)
{
    m_qtPainter->drawPoint(x, y);
}

void wxQtDCImpl::DoDrawLine(int x1, int y1, int x2, int y2)
{
    m_qtPainter->drawLine(x1, y1, x2, y2);
}


void wxQtDCImpl::DoDrawArc(int x1, int y1,
                       int x2, int y2,
                       int xc, int yc)
{
    // Calculate the rectangle that contains the circle
    QLineF l1( xc, yc, x1, y1 );
    QLineF l2( xc, yc, x2, y2 );
    QPointF center( xc, yc );

    qreal penWidth = m_qtPainter->pen().width();
    qreal lenRadius = l1.length() - penWidth / 2;
    QPointF centerToCorner( lenRadius, lenRadius );

    QRect rectangle = QRectF( center - centerToCorner, center + centerToCorner ).toRect();

    // Calculate the angles
    int startAngle = (int)( l1.angle() * 16 );
    int endAngle = (int)( l2.angle() * 16 );
    int spanAngle = endAngle - startAngle;
    if ( spanAngle < 0 )
    {
        spanAngle = -spanAngle;
    }

    if ( spanAngle == 0 )
        m_qtPainter->drawEllipse( rectangle );
    else
        m_qtPainter->drawPie( rectangle, startAngle, spanAngle );
}

void wxQtDCImpl::DoDrawEllipticArc(int x, int y, int w, int h,
                               double sa, double ea)
{
    int penWidth = m_qtPainter->pen().width();
    x += penWidth / 2;
    y += penWidth / 2;
    w -= penWidth;
    h -= penWidth;

    double spanAngle = sa - ea;
    if (spanAngle < -180)
        spanAngle += 360;
    if (spanAngle > 180)
        spanAngle -= 360;

    if ( spanAngle == 0 )
        m_qtPainter->drawEllipse( x, y, w, h );
    else
        m_qtPainter->drawPie( x, y, w, h, (int)( sa * 16 ), (int)( ( ea - sa ) * 16 ) );
}

void wxQtDCImpl::DoDrawRectangle(int x, int y, int width, int height)
{
    int penWidth = m_qtPainter->pen().width();
    x += penWidth / 2;
    y += penWidth / 2;
    width -= penWidth;
    height -= penWidth;

    m_qtPainter->drawRect( x, y, width, height );
}

void wxQtDCImpl::DoDrawRoundedRectangle(int x, int y,
                                    int width, int height,
                                    double radius)
{
    int penWidth = m_qtPainter->pen().width();
    x += penWidth / 2;
    y += penWidth / 2;
    width -= penWidth;
    height -= penWidth;

    m_qtPainter->drawRoundedRect( x, y, width, height, radius, radius );
}

void wxQtDCImpl::DoDrawEllipse(int x, int y,
                           int width, int height)
{
    QBrush savedBrush;
    int penWidth = m_qtPainter->pen().width();
    x += penWidth / 2;
    y += penWidth / 2;
    width -= penWidth;
    height -= penWidth;

    if ( m_pen.color() == Qt::transparent )
    {
        // Save pen/brush
        savedBrush = m_qtPainter->brush();
        // Fill with text background color ("no fill" like in wxGTK):
        m_qtPainter->setBrush(QBrush(m_textBackgroundColour));
    }

    // Draw
    m_qtPainter->drawEllipse( x, y, width, height );

    if ( m_pen.color() == Qt::transparent)
    {
        //Restore saved settings
        m_qtPainter->setBrush(savedBrush);
    }
}

void wxQtDCImpl::DoCrossHair(int x, int y)
{
    int w, h;
    DoGetSize( &w, &h );

    // Map width and height back (inverted transform)
    QTransform inv = m_qtPainter->transform().inverted();
    int left, top, right, bottom;
    inv.map( w, h, &right, &bottom );
    inv.map( 0, 0, &left, &top );

    m_qtPainter->drawLine( left, y, right, y );
    m_qtPainter->drawLine( x, top, x, bottom );
}

void wxQtDCImpl::DoDrawIcon(const QIcon& icon, int x, int y)
{
//    DoDrawBitmap( icon.pixmap(icon), x, y, true );
}

void wxQtDCImpl::DoDrawBitmap(const wxBitmap &bmp, int x, int y,
                          bool useMask )
{
    QPixmap pix = *bmp.GetHandle();
    if (pix.depth() == 1) {
        //Monochrome bitmap, draw using text fore/background

        //Save pen/brush
        QBrush savedBrush = m_qtPainter->background();
        QPen savedPen = m_qtPainter->pen();

        //Use text colors
        m_qtPainter->setBackground(QBrush(m_textBackgroundColour));
        m_qtPainter->setPen(QPen(m_textForegroundColour));

        //Draw
        m_qtPainter->drawPixmap(x, y, pix);

        //Restore saved settings
        m_qtPainter->setBackground(savedBrush);
        m_qtPainter->setPen(savedPen);
    }
    else
    {
            if ( useMask && bmp.GetMask() && bmp.GetMask()->GetHandle() )
                pix.setMask(*bmp.GetMask()->GetHandle());
            m_qtPainter->drawPixmap(x, y, pix);
    }
}

void wxQtDCImpl::DoDrawText(const QString& text, int x, int y)
{
    QPen savedPen = m_qtPainter->pen();
    m_qtPainter->setPen(QPen(m_textForegroundColour));

    // Disable logical function
    QPainter::CompositionMode savedOp = m_qtPainter->compositionMode();
    m_qtPainter->setCompositionMode( QPainter::CompositionMode_SourceOver );

    if (m_backgroundMode == Qt::SolidPattern)
    {
        m_qtPainter->setBackgroundMode(Qt::OpaqueMode);

        //Save pen/brush
        QBrush savedBrush = m_qtPainter->background();

        //Use text colors
        m_qtPainter->setBackground(QBrush(m_textBackgroundColour));

        //Draw
        m_qtPainter->drawText(x, y, 1, 1, Qt::TextDontClip, text);

        //Restore saved settings
        m_qtPainter->setBackground(savedBrush);


        m_qtPainter->setBackgroundMode(Qt::TransparentMode);
    }
    else
        m_qtPainter->drawText(x, y, 1, 1, Qt::TextDontClip, text);

    m_qtPainter->setPen(savedPen);
    m_qtPainter->setCompositionMode( savedOp );
}

void wxQtDCImpl::DoDrawRotatedText(const QString& text,
                               int x, int y, double angle)
{
    if (m_backgroundMode == Qt::SolidPattern)
        m_qtPainter->setBackgroundMode(Qt::OpaqueMode);

    //Move and rotate (reverse angle direction in Qt and wx)
    m_qtPainter->translate(x, y);
    m_qtPainter->rotate(-angle);

    QPen savedPen = m_qtPainter->pen();
    m_qtPainter->setPen(QPen(m_textForegroundColour));

    // Disable logical function
    QPainter::CompositionMode savedOp = m_qtPainter->compositionMode();
    m_qtPainter->setCompositionMode( QPainter::CompositionMode_SourceOver );

    if (m_backgroundMode == Qt::SolidPattern)
    {
        m_qtPainter->setBackgroundMode(Qt::OpaqueMode);

        //Save pen/brush
        QBrush savedBrush = m_qtPainter->background();

        //Use text colors
        m_qtPainter->setBackground(QBrush(m_textBackgroundColour));

        //Draw
        m_qtPainter->drawText(x, y, 1, 1, Qt::TextDontClip, text);

        //Restore saved settings
        m_qtPainter->setBackground(savedBrush);

        m_qtPainter->setBackgroundMode(Qt::TransparentMode);
    }
    else
        m_qtPainter->drawText(x, y, 1, 1, Qt::TextDontClip, text);

    //Reset to default
    ComputeScaleAndOrigin();
    m_qtPainter->setPen(savedPen);
    m_qtPainter->setCompositionMode( savedOp );
}

bool wxQtDCImpl::DoBlit(int xdest, int ydest,
                    int width, int height,
                    wxDC *source,
                    int xsrc, int ysrc,
                    wxRasterOperationMode rop,
                    bool useMask,
                    int xsrcMask,
                    int ysrcMask )
{
    wxQtDCImpl *implSource = (wxQtDCImpl*)source->GetImpl();

    QImage *qtSource = implSource->GetQImage();

    // Not a CHECK on purpose
    if ( !qtSource )
        return false;

    QImage qtSourceConverted = *qtSource;
    if ( !useMask )
        qtSourceConverted = qtSourceConverted.convertToFormat( QImage::Format_RGB32 );

    // Change logical function
    wxRasterOperationMode savedMode = GetLogicalFunction();
    SetLogicalFunction( rop );

    m_qtPainter->drawImage( QRect( xdest, ydest, width, height ),
                           qtSourceConverted,
                           QRect( xsrc, ysrc, width, height ) );

    SetLogicalFunction( savedMode );

    return true;
}

void wxQtDCImpl::DoDrawLines(int n, const zchxPoint points[],
                         int xoffset, int yoffset )
{
    if (n > 0)
    {
        QPointF pf = QPointF(points[0].toPoint());
        QPainterPath path(pf);
        for (int i = 1; i < n; i++)
        {
            path.lineTo(points[i].toPoint());
        }

        m_qtPainter->translate(xoffset, yoffset);

        QBrush savebrush = m_qtPainter->brush();
        m_qtPainter->setBrush(Qt::NoBrush);
        m_qtPainter->drawPath(path);
        m_qtPainter->setBrush(savebrush);

        // Reset transform
        ComputeScaleAndOrigin();
    }
}

void wxQtDCImpl::DoDrawPolygon(int n, const zchxPoint points[],
                       int xoffset, int yoffset,
                       Qt::FillRule fillStyle )
{
    QPolygon qtPoints;
    for (int i = 0; i < n; i++) {
        qtPoints << points[i].toPoint();
    }

//    Qt::FillRule fill = (fillStyle == wxWINDING_RULE) ? Qt::WindingFill : Qt::OddEvenFill;

    m_qtPainter->translate(xoffset, yoffset);
    m_qtPainter->drawPolygon(qtPoints, fillStyle);
    // Reset transform
    ComputeScaleAndOrigin();
}

void wxQtDCImpl::ComputeScaleAndOrigin()
{
    QTransform t;

    // First apply device origin
    t.translate( m_deviceOriginX + m_deviceLocalOriginX,
                 m_deviceOriginY + m_deviceLocalOriginY );

    // Second, scale
    m_scaleX = m_logicalScaleX * m_userScaleX;
    m_scaleY = m_logicalScaleY * m_userScaleY;
    t.scale( m_scaleX * m_signX, m_scaleY * m_signY );

    // Finally, logical origin
    t.translate( m_logicalOriginX, m_logicalOriginY );

    // Apply transform to QPainter, overwriting the previous one
    m_qtPainter->setWorldTransform(t, false);
}
