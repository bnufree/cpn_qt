/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/dc.h
// Author:      Peter Most, Javier Torres, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_DC_H_
#define _WX_QT_DC_H_

#include "base/dc.h"

class QPainter;
class QImage;

class QRegion;

class wxQtDCImpl : public wxDCImpl
{
public:
    wxQtDCImpl( wxDC *owner );
    ~wxQtDCImpl();

    virtual bool CanDrawBitmap() const;
    virtual bool CanGetTextExtent() const;

    virtual void DoGetSize(int *width, int *height) const;
    virtual void DoGetSizeMM(int* width, int* height) const;

    virtual int GetDepth() const;
    virtual QSize GetPPI() const;

    virtual void SetFont(const QFont& font);
    virtual void SetPen(const QPen& pen);
    virtual void SetBrush(const QBrush& brush);
    virtual void SetBackground(const QBrush& brush);
    virtual void SetBackgroundMode(int mode);

#if wxUSE_PALETTE
    virtual void SetPalette(const wxPalette& palette);
#endif // wxUSE_PALETTE

    virtual void SetLogicalFunction(wxRasterOperationMode function);

    virtual int GetCharHeight() const;
    virtual int GetCharWidth() const;
    virtual void DoGetTextExtent(const QString& string,
                                 int *x, int *y,
                                 int *descent = NULL,
                                 int *externalLeading = NULL,
                                 const QFont *theFont = NULL) const;

    virtual void Clear();

    virtual void DoSetClippingRegion(int x, int y,
                                     int width, int height);

    virtual void DoSetDeviceClippingRegion(const QRegion& region);
    virtual void DestroyClippingRegion();

    virtual bool DoFloodFill(int x, int y, const QColor& col,
                             wxFloodFillStyle style = wxFLOOD_SURFACE);
    virtual bool DoGetPixel(int x, int y, QColor *col) const;

    virtual void DoDrawPoint(int x, int y);
    virtual void DoDrawLine(int x1, int y1, int x2, int y2);

    virtual void DoDrawArc(int x1, int y1,
                           int x2, int y2,
                           int xc, int yc);

    virtual void DoDrawEllipticArc(int x, int y, int w, int h,
                                   double sa, double ea);

    virtual void DoDrawRectangle(int x, int y, int width, int height);
    virtual void DoDrawRoundedRectangle(int x, int y,
                                        int width, int height,
                                        double radius);
    virtual void DoDrawEllipse(int x, int y,
                               int width, int height);

    virtual void DoCrossHair(int x, int y);

    virtual void DoDrawIcon(const QIcon& icon, int x, int y);
    virtual void DoDrawBitmap(const wxBitmap &bmp, int x, int y,
                              bool useMask = false);

    virtual void DoDrawText(const QString& text, int x, int y);
    virtual void DoDrawRotatedText(const QString& text,
                                   int x, int y, double angle);

    virtual bool DoBlit(int xdest, int ydest,
                        int width, int height,
                        wxDC *source,
                        int xsrc, int ysrc,
                        wxRasterOperationMode rop = wxCOPY,
                        bool useMask = false,
                        int xsrcMask = -1,
                        int ysrcMask = -1);

    virtual void DoDrawLines(int n, const zchxPoint points[],
                             int xoffset, int yoffset );

    virtual void DoDrawPolygon(int n, const zchxPoint points[],
                           int xoffset, int yoffset,
                           Qt::FillRule fillStyle = Qt::OddEvenFill);

    // Use Qt transformations, as they automatically scale pen widths, text...
    virtual void ComputeScaleAndOrigin();

    void QtPreparePainter();

    virtual void* GetHandle() const { return (void*) m_qtPainter; }

protected:
    virtual QImage *GetQImage() { return m_qtImage; }
    
    QPainter *m_qtPainter;
    QImage *m_qtImage;

    QRegion *m_clippingRegion;
private:
    enum wxQtRasterColourOp
    {
        wxQtNONE,
        wxQtWHITE,
        wxQtBLACK,
        wxQtINVERT
    };
    wxQtRasterColourOp m_rasterColourOp;
    QColor *m_qtPenColor;
    QColor *m_qtBrushColor;
    void ApplyRasterColourOp();
    
};

#endif // _WX_QT_DC_H_
