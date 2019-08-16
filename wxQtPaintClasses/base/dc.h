/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dc.h
// Purpose:     wxDC class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     05/25/99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DC_H_BASE_
#define _WX_DC_H_BASE_

// ----------------------------------------------------------------------------
// headers which we must include here
// ----------------------------------------------------------------------------

//#include "wx/object.h"          // the base class

//#include "wx/intl.h"            // for wxLayoutDirection
//#include "wx/colour.h"          // we have member variables of these classes
//#include "wx/font.h"            // so we can't do without them
//#include "wx/bitmap.h"          // for QBitmap()
//#include "wx/brush.h"
//#include "wx/pen.h"
//#include "wx/palette.h"
//#include "wx/dynarray.h"
//#include "wx/math.h"
//#include "wx/image.h"
//#include "wx/region.h"
//#include "wx/affinematrix2d.h"

#include "../qt/_def.h"
#include "../bitmap.h"
#include <QFont>
#include <QPen>

#define wxUSE_NEW_DC 1

class  wxDC;
class  wxClientDC;
class  wxPaintDC;
class  wxWindowDC;
class  wxScreenDC;
class  wxMemoryDC;
class  wxPrinterDC;
class  wxPrintData;
class  wxWindow;
class  wxBitmap;

#if wxUSE_GRAPHICS_CONTEXT
class WXDLLIMPEXP_FWD_CORE wxGraphicsContext;
#endif

//  Logical ops
enum wxRasterOperationMode
{
    wxCLEAR,       // 0
    wxXOR,         // src XOR dst
    wxINVERT,      // NOT dst
    wxOR_REVERSE,  // src OR (NOT dst)
    wxAND_REVERSE, // src AND (NOT dst)
    wxCOPY,        // src
    wxAND,         // src AND dst
    wxAND_INVERT,  // (NOT src) AND dst
    wxNO_OP,       // dst
    wxNOR,         // (NOT src) AND (NOT dst)
    wxEQUIV,       // (NOT src) XOR dst
    wxSRC_INVERT,  // (NOT src)
    wxOR_INVERT,   // (NOT src) OR dst
    wxNAND,        // (NOT src) OR (NOT dst)
    wxOR,          // src OR dst
    wxSET          // 1
#if WXWIN_COMPATIBILITY_2_8
    ,wxROP_BLACK = wxCLEAR,
    wxBLIT_BLACKNESS = wxCLEAR,
    wxROP_XORPEN = wxXOR,
    wxBLIT_SRCINVERT = wxXOR,
    wxROP_NOT = wxINVERT,
    wxBLIT_DSTINVERT = wxINVERT,
    wxROP_MERGEPENNOT = wxOR_REVERSE,
    wxBLIT_00DD0228 = wxOR_REVERSE,
    wxROP_MASKPENNOT = wxAND_REVERSE,
    wxBLIT_SRCERASE = wxAND_REVERSE,
    wxROP_COPYPEN = wxCOPY,
    wxBLIT_SRCCOPY = wxCOPY,
    wxROP_MASKPEN = wxAND,
    wxBLIT_SRCAND = wxAND,
    wxROP_MASKNOTPEN = wxAND_INVERT,
    wxBLIT_00220326 = wxAND_INVERT,
    wxROP_NOP = wxNO_OP,
    wxBLIT_00AA0029 = wxNO_OP,
    wxROP_NOTMERGEPEN = wxNOR,
    wxBLIT_NOTSRCERASE = wxNOR,
    wxROP_NOTXORPEN = wxEQUIV,
    wxBLIT_00990066 = wxEQUIV,
    wxROP_NOTCOPYPEN = wxSRC_INVERT,
    wxBLIT_NOTSCRCOPY = wxSRC_INVERT,
    wxROP_MERGENOTPEN = wxOR_INVERT,
    wxBLIT_MERGEPAINT = wxOR_INVERT,
    wxROP_NOTMASKPEN = wxNAND,
    wxBLIT_007700E6 = wxNAND,
    wxROP_MERGEPEN = wxOR,
    wxBLIT_SRCPAINT = wxOR,
    wxROP_WHITE = wxSET,
    wxBLIT_WHITENESS = wxSET
#endif //WXWIN_COMPATIBILITY_2_8
};

//  Flood styles
enum wxFloodFillStyle
{
    wxFLOOD_SURFACE = 1,
    wxFLOOD_BORDER
};

//  Mapping modes
enum wxMappingMode
{
    wxMM_TEXT = 1,
    wxMM_METRIC,
    wxMM_LOMETRIC,
    wxMM_TWIPS,
    wxMM_POINTS
};

// Description of text characteristics.
struct wxFontMetrics
{
    wxFontMetrics()
    {
        height =
        ascent =
        descent =
        internalLeading =
        externalLeading =
        averageWidth = 0;
    }

    int height,             // Total character height.
        ascent,             // Part of the height above the baseline.
        descent,            // Part of the height below the baseline.
        internalLeading,    // Intra-line spacing.
        externalLeading,    // Inter-line spacing.
        averageWidth;       // Average font width, a.k.a. "x-width".
};

#if WXWIN_COMPATIBILITY_2_8

//-----------------------------------------------------------------------------
// wxDrawObject helper class
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxDrawObject
{
public:
    wxDEPRECATED_CONSTRUCTOR(wxDrawObject)()
        : m_isBBoxValid(false)
        , m_minX(0), m_minY(0), m_maxX(0), m_maxY(0)
    { }

    virtual ~wxDrawObject() { }

    virtual void Draw(wxDC&) const { }

    virtual void CalcBoundingBox(int x, int y)
    {
      if ( m_isBBoxValid )
      {
         if ( x < m_minX ) m_minX = x;
         if ( y < m_minY ) m_minY = y;
         if ( x > m_maxX ) m_maxX = x;
         if ( y > m_maxY ) m_maxY = y;
      }
      else
      {
         m_isBBoxValid = true;

         m_minX = x;
         m_minY = y;
         m_maxX = x;
         m_maxY = y;
      }
    }

    void ResetBoundingBox()
    {
        m_isBBoxValid = false;

        m_minX = m_maxX = m_minY = m_maxY = 0;
    }

    // Get the final bounding box of the PostScript or Metafile picture.

    int MinX() const { return m_minX; }
    int MaxX() const { return m_maxX; }
    int MinY() const { return m_minY; }
    int MaxY() const { return m_maxY; }

    //to define the type of object for derived objects
    virtual int GetType()=0;

protected:
    //for boundingbox calculation
    bool m_isBBoxValid:1;
    //for boundingbox calculation
    int m_minX, m_minY, m_maxX, m_maxY;
};

#endif // WXWIN_COMPATIBILITY_2_8


//-----------------------------------------------------------------------------
// wxDCFactory
//-----------------------------------------------------------------------------

class  wxDCImpl;

class  wxDCFactory
{
public:
    wxDCFactory() {}
    virtual ~wxDCFactory() {}

    virtual wxDCImpl* CreateWindowDC( wxWindowDC *owner, wxWindow *window ) = 0;
    virtual wxDCImpl* CreateClientDC( wxClientDC *owner, wxWindow *window ) = 0;
    virtual wxDCImpl* CreatePaintDC( wxPaintDC *owner, wxWindow *window ) = 0;
    virtual wxDCImpl* CreateMemoryDC( wxMemoryDC *owner ) = 0;
    virtual wxDCImpl* CreateMemoryDC( wxMemoryDC *owner, wxBitmap &bitmap ) = 0;
    virtual wxDCImpl* CreateMemoryDC( wxMemoryDC *owner, wxDC *dc ) = 0;
    virtual wxDCImpl* CreateScreenDC( wxScreenDC *owner ) = 0;
#if wxUSE_PRINTING_ARCHITECTURE
    virtual wxDCImpl* CreatePrinterDC( wxPrinterDC *owner, const wxPrintData &data  ) = 0;
#endif

    static void Set(wxDCFactory *factory);
    static wxDCFactory *Get();

private:
    static wxDCFactory *m_factory;
};

//-----------------------------------------------------------------------------
// wxNativeDCFactory
//-----------------------------------------------------------------------------

class wxNativeDCFactory: public wxDCFactory
{
public:
    wxNativeDCFactory() {}

    virtual wxDCImpl* CreateWindowDC( wxWindowDC *owner, wxWindow *window ) ;
    virtual wxDCImpl* CreateClientDC( wxClientDC *owner, wxWindow *window ) ;
    virtual wxDCImpl* CreatePaintDC( wxPaintDC *owner, wxWindow *window ) ;
    virtual wxDCImpl* CreateMemoryDC( wxMemoryDC *owner ) ;
    virtual wxDCImpl* CreateMemoryDC( wxMemoryDC *owner, wxBitmap &bitmap ) ;
    virtual wxDCImpl* CreateMemoryDC( wxMemoryDC *owner, wxDC *dc ) ;
    virtual wxDCImpl* CreateScreenDC( wxScreenDC *owner ) ;
#if wxUSE_PRINTING_ARCHITECTURE
    virtual wxDCImpl* CreatePrinterDC( wxPrinterDC *owner, const wxPrintData &data  ) ;
#endif
};

//-----------------------------------------------------------------------------
// wxDCImpl
//-----------------------------------------------------------------------------

class wxDCImpl/*: public wxObject*/
{
public:
    wxDCImpl( wxDC *owner );
    virtual ~wxDCImpl();

    wxDC *GetOwner() const { return m_owner; }

    wxWindow* GetWindow() const { return m_window; }

    virtual bool IsOk() const { return m_ok; }

    // query capabilities

    virtual bool CanDrawBitmap() const = 0;
    virtual bool CanGetTextExtent() const = 0;

    // get Cairo context
    virtual void* GetCairoContext() const
    {
        return NULL;
    }

    virtual void* GetHandle() const { return NULL; }
    
    // query dimension, colour deps, resolution

    virtual void DoGetSize(int *width, int *height) const = 0;
    void GetSize(int *width, int *height) const
    {
        DoGetSize(width, height);
        return ;
    }

    QSize GetSize() const
    {
        int w, h;
        DoGetSize(&w, &h);
        return QSize(w, h);
    }

    virtual void DoGetSizeMM(int* width, int* height) const = 0;

    virtual int GetDepth() const = 0;
    virtual QSize GetPPI() const = 0;

    // Right-To-Left (RTL) modes

    virtual void SetLayoutDirection(Qt::LayoutDirection dir) { }
    virtual Qt::LayoutDirection GetLayoutDirection() const  { return mDir; }

    // page and document

    virtual bool StartDoc(const QString& message) { return true; }
    virtual void EndDoc() { }

    virtual void StartPage() { }
    virtual void EndPage() { }

    // flushing the content of this dc immediately eg onto screen
    virtual void Flush() { }

    // bounding box

    virtual void CalcBoundingBox(int x, int y)
    {
      // Bounding box is internally stored in device units.
      x = LogicalToDeviceX(x);
      y = LogicalToDeviceY(y);
      if ( m_isBBoxValid )
      {
         if ( x < m_minX ) m_minX = x;
         if ( y < m_minY ) m_minY = y;
         if ( x > m_maxX ) m_maxX = x;
         if ( y > m_maxY ) m_maxY = y;
      }
      else
      {
         m_isBBoxValid = true;

         m_minX = x;
         m_minY = y;
         m_maxX = x;
         m_maxY = y;
      }
    }
    void ResetBoundingBox()
    {
        m_isBBoxValid = false;

        m_minX = m_maxX = m_minY = m_maxY = 0;
    }

    // Get bounding box in logical units.
    int MinX() const { return m_isBBoxValid ? DeviceToLogicalX(m_minX) : 0; }
    int MaxX() const { return m_isBBoxValid ? DeviceToLogicalX(m_maxX) : 0; }
    int MinY() const { return m_isBBoxValid ? DeviceToLogicalY(m_minY) : 0; }
    int MaxY() const { return m_isBBoxValid ? DeviceToLogicalY(m_maxY) : 0; }

    // setters and getters

    virtual void SetFont(const QFont& font) = 0;
    virtual const QFont& GetFont() const { return m_font; }

    virtual void SetPen(const QPen& pen) = 0;
    virtual const QPen& GetPen() const { return m_pen; }

    virtual void SetBrush(const QBrush& brush) = 0;
    virtual const QBrush& GetBrush() const { return m_brush; }

    virtual void SetBackground(const QBrush& brush) = 0;
    virtual const QBrush& GetBackground() const { return m_backgroundBrush; }

    virtual void SetBackgroundMode(int mode) = 0;
    virtual int GetBackgroundMode() const { return m_backgroundMode; }

    virtual void SetTextForeground(const QColor& colour)
        { m_textForegroundColour = colour; }
    virtual const QColor& GetTextForeground() const
        { return m_textForegroundColour; }

    virtual void SetTextBackground(const QColor& colour)
        { m_textBackgroundColour = colour; }
    virtual const QColor& GetTextBackground() const
        { return m_textBackgroundColour; }

#if wxUSE_PALETTE
    virtual void SetPalette(const wxPalette& palette) = 0;
#endif // wxUSE_PALETTE

    // inherit the DC attributes (font and colours) from the given window
    //
    // this is called automatically when a window, client or paint DC is
    // created
    virtual void InheritAttributes(wxWindow *win);


    // logical functions

    virtual void SetLogicalFunction(wxRasterOperationMode function) = 0;
    virtual wxRasterOperationMode GetLogicalFunction() const
                                      { return m_logicalFunction; }

    // text measurement

    virtual int GetCharHeight() const = 0;
    virtual int GetCharWidth() const = 0;

    // The derived classes should really override DoGetFontMetrics() to return
    // the correct values in the future but for now provide a default
    // implementation in terms of DoGetTextExtent() to avoid breaking the
    // compilation of all other ports as wxMSW is the only one to implement it.
    virtual void DoGetFontMetrics(int *height,
                                  int *ascent,
                                  int *descent,
                                  int *internalLeading,
                                  int *externalLeading,
                                  int *averageWidth) const;

    virtual void DoGetTextExtent(const QString& string,
                                 int *x, int *y,
                                 int *descent = NULL,
                                 int *externalLeading = NULL,
                                 const QFont *theFont = NULL) const = 0;
    virtual void GetMultiLineTextExtent(const QString& string,
                                        int *width,
                                        int *height,
                                        int *heightLine = NULL,
                                        const QFont *font = NULL) const;
    virtual bool DoGetPartialTextExtents(const QString& text, QList<int>& widths) const;

    // clearing

    virtual void Clear() = 0;

    // clipping

    // Note that this pure virtual method has an implementation that updates
    // the values returned by DoGetClippingBox() and so can be called from the
    // derived class overridden version if it makes sense (i.e. if the clipping
    // box coordinates are not already updated in some other way).
    virtual void DoSetClippingRegion(int x, int y,
                                     int w, int h) = 0;

    // NB: this function works with device coordinates, not the logical ones!
    virtual void DoSetDeviceClippingRegion(const QRegion& region) = 0;

    // Method used to implement wxDC::GetClippingBox().
    //
    // Default implementation returns values stored in m_clip[XY][12] member
    // variables, so this method doesn't need to be overridden if they're kept
    // up to date.
    virtual bool DoGetClippingRect(QRect& rect) const;

#if WXWIN_COMPATIBILITY_3_0
    // This method is kept for backwards compatibility but shouldn't be used
    // nor overridden in the new code, implement DoGetClippingRect() above
    // instead.
    wxDEPRECATED_BUT_USED_INTERNALLY(
        virtual void DoGetClippingBox(int *x, int *y,
                                      int *w, int *h) const
    );
#endif // WXWIN_COMPATIBILITY_3_0

    virtual void DestroyClippingRegion() { ResetClipping(); }


    // coordinates conversions and transforms

    virtual int DeviceToLogicalX(int x) const;
    virtual int DeviceToLogicalY(int y) const;
    virtual int DeviceToLogicalXRel(int x) const;
    virtual int DeviceToLogicalYRel(int y) const;
    virtual int LogicalToDeviceX(int x) const;
    virtual int LogicalToDeviceY(int y) const;
    virtual int LogicalToDeviceXRel(int x) const;
    virtual int LogicalToDeviceYRel(int y) const;

    virtual void SetMapMode(wxMappingMode mode);
    virtual wxMappingMode GetMapMode() const { return m_mappingMode; }

    virtual void SetUserScale(double x, double y);
    virtual void GetUserScale(double *x, double *y) const
    {
        if ( x ) *x = m_userScaleX;
        if ( y ) *y = m_userScaleY;
    }

    virtual void SetLogicalScale(double x, double y);
    virtual void GetLogicalScale(double *x, double *y) const
    {
        if ( x ) *x = m_logicalScaleX;
        if ( y ) *y = m_logicalScaleY;
    }

    virtual void SetLogicalOrigin(int x, int y);
    virtual void DoGetLogicalOrigin(int *x, int *y) const
    {
        if ( x ) *x = m_logicalOriginX;
        if ( y ) *y = m_logicalOriginY;
    }

    virtual void SetDeviceOrigin(int x, int y);
    virtual void DoGetDeviceOrigin(int *x, int *y) const
    {
        if ( x ) *x = m_deviceOriginX;
        if ( y ) *y = m_deviceOriginY;
    }

#if wxUSE_DC_TRANSFORM_MATRIX
    // Transform matrix support is not available in most ports right now
    // (currently only wxMSW provides it) so do nothing in these methods by
    // default.
    virtual bool CanUseTransformMatrix() const
        { return false; }
    virtual bool SetTransformMatrix(const wxAffineMatrix2D& WXUNUSED(matrix))
        { return false; }
    virtual wxAffineMatrix2D GetTransformMatrix() const
        { return wxAffineMatrix2D(); }
    virtual void ResetTransformMatrix()
        { }
#endif // wxUSE_DC_TRANSFORM_MATRIX

    virtual void SetDeviceLocalOrigin( int x, int y );

    virtual void ComputeScaleAndOrigin();

    // this needs to overidden if the axis is inverted
    virtual void SetAxisOrientation(bool xLeftRight, bool yBottomUp);
    
    virtual double GetContentScaleFactor() const { return m_contentScaleFactor; }

#ifdef __WXMSW__
    // Native Windows functions using the underlying HDC don't honour GDI+
    // transformations which may be applied to it. Using this function we can
    // transform the coordinates manually before passing them to such functions
    // (as in e.g. wxRendererMSW code). It doesn't do anything if this is not a
    // wxGCDC.
    virtual QRect MSWApplyGDIPlusTransform(const QRect& r) const
    {
        return r;
    }
#endif // __WXMSW__


    // ---------------------------------------------------------
    // the actual drawing API

    virtual bool DoFloodFill(int x, int y, const QColor& col,
                             wxFloodFillStyle style = wxFLOOD_SURFACE) = 0;

    virtual void DoGradientFillLinear(const QRect& rect,
                                      const QColor& initialColour,
                                      const QColor& destColour,
                                      /*wxDirection nDirection = wxEAST*/int nDirection);

    virtual void DoGradientFillConcentric(const QRect& rect,
                                        const QColor& initialColour,
                                        const QColor& destColour,
                                        const zchxPoint& circleCenter);

    virtual bool DoGetPixel(int x, int y, QColor *col) const = 0;

    virtual void DoDrawPoint(int x, int y) = 0;
    virtual void DoDrawLine(int x1, int y1, int x2, int y2) = 0;

    virtual void DoDrawArc(int x1, int y1,
                           int x2, int y2,
                           int xc, int yc) = 0;
    virtual void DoDrawCheckMark(int x, int y,
                                 int width, int height);
    virtual void DoDrawEllipticArc(int x, int y, int w, int h,
                                   double sa, double ea) = 0;

    virtual void DoDrawRectangle(int x, int y, int width, int height) = 0;
    virtual void DoDrawRoundedRectangle(int x, int y,
                                        int width, int height,
                                        double radius) = 0;
    virtual void DoDrawEllipse(int x, int y,
                               int width, int height) = 0;

    virtual void DoCrossHair(int x, int y) = 0;

    virtual void DoDrawIcon(const QIcon& icon, int x, int y) = 0;
    virtual void DoDrawBitmap(const wxBitmap &bmp, int x, int y,
                              bool useMask = false) = 0;

    virtual void DoDrawText(const QString& text, int x, int y) = 0;
    virtual void DoDrawRotatedText(const QString& text,
                                   int x, int y, double angle) = 0;

    virtual bool DoBlit(int xdest, int ydest,
                        int width, int height,
                        wxDC *source,
                        int xsrc, int ysrc,
                        wxRasterOperationMode rop = wxCOPY,
                        bool useMask = false,
                        int xsrcMask = -1,
                        int ysrcMask = -1) = 0;

    virtual bool DoStretchBlit(int xdest, int ydest,
                               int dstWidth, int dstHeight,
                               wxDC *source,
                               int xsrc, int ysrc,
                               int srcWidth, int srcHeight,
                               wxRasterOperationMode rop = wxCOPY,
                               bool useMask = false,
                               int xsrcMask = -1,
                               int ysrcMask = -1);

    virtual wxBitmap DoGetAsBitmap(const QRect *subrect) const
        { return wxBitmap(); }


    virtual void DoDrawLines(int n, const zchxPoint points[],
                             int xoffset, int yoffset ) = 0;
    virtual void DrawLines(const QList<zchxPoint> *list,
                           int xoffset, int yoffset );

    virtual void DoDrawPolygon(int n, const zchxPoint points[],
                           int xoffset, int yoffset,
                           Qt::FillRule fillStyle = Qt::OddEvenFill) = 0;
    virtual void DoDrawPolyPolygon(int n, const int count[], const zchxPoint points[],
                               int xoffset, int yoffset,
                               Qt::FillRule fillStyle);
    void DrawPolygon(const QList<zchxPoint> *list,
                     int xoffset, int yoffset,
                     Qt::FillRule fillStyle );


#if wxUSE_SPLINES
    void DrawSpline(int x1, int y1,
                            int x2, int y2,
                            int x3, int y3);
    void DrawSpline(int n, const zchxPoint points[]);
    void DrawSpline(const zchxPointList *points) { DoDrawSpline(points); }

    virtual void DoDrawSpline(const zchxPointList *points);
#endif

    // ---------------------------------------------------------
    // wxMemoryDC Impl API

    virtual void DoSelect(const wxBitmap& bmp)
       { }

    virtual const wxBitmap& GetSelectedBitmap() const
        { return wxBitmap(); }
    virtual wxBitmap& GetSelectedBitmap()
        {
            wxBitmap bmp;
        return bmp; }

    // ---------------------------------------------------------
    // wxPrinterDC Impl API

    virtual QRect GetPaperRect() const
        { int w = 0; int h = 0; DoGetSize( &w, &h ); return QRect(0,0,w,h); }

    virtual int GetResolution() const
        { return -1; }

#if wxUSE_GRAPHICS_CONTEXT
    virtual wxGraphicsContext* GetGraphicsContext() const
        { return NULL; }
    virtual void SetGraphicsContext( wxGraphicsContext* WXUNUSED(ctx) )
        {}
#endif

private:
    wxDC       *m_owner;

protected:
    // This method exists for backwards compatibility only (while it's not
    // documented, there are derived classes using it outside wxWidgets
    // itself), don't use it in any new code and just call wxDCImpl version of
    // DestroyClippingRegion() to reset the clipping information instead.
    void ResetClipping()
    {
        m_clipping = false;

        m_clipX1 = m_clipX2 = m_clipY1 = m_clipY2 = 0;
    }

    // returns adjustment factor for converting QFont "point size"; in wx
    // it is point size on screen and needs to be multiplied by this value
    // for rendering on higher-resolution DCs such as printer ones
    static float GetFontPointSizeAdjustment(float dpi);

    // Return the number of pixels per mm in the horizontal and vertical
    // directions, respectively.
    //
    // If the physical size of the DC is not known, or doesn't make sense, as
    // for a SVG DC, for example, a fixed value corresponding to the standard
    // DPI is used.
    double GetMMToPXx() const;
    double GetMMToPXy() const;


    // window on which the DC draws or NULL
    wxWindow   *m_window;

    // flags
    bool m_colour:1;
    bool m_ok:1;
    bool m_clipping:1;
    bool m_isInteractive:1;
    bool m_isBBoxValid:1;

    // coordinate system variables

    int m_logicalOriginX, m_logicalOriginY;
    int m_deviceOriginX, m_deviceOriginY;           // Usually 0,0, can be change by user

    int m_deviceLocalOriginX, m_deviceLocalOriginY; // non-zero if native top-left corner
                                                        // is not at 0,0. This was the case under
                                                        // Mac's GrafPorts (coordinate system
                                                        // used toplevel window's origin) and
                                                        // e.g. for Postscript, where the native
                                                        // origin in the bottom left corner.
    double m_logicalScaleX, m_logicalScaleY;
    double m_userScaleX, m_userScaleY;
    double m_scaleX, m_scaleY;  // calculated from logical scale and user scale

    int m_signX, m_signY;  // Used by SetAxisOrientation() to invert the axes
    
    double m_contentScaleFactor; // used by high resolution displays (retina)

    // Pixel per mm in horizontal and vertical directions.
    //
    // These variables are computed on demand by GetMMToPX[xy]() functions,
    // don't access them directly other than for assigning to them.
    mutable double m_mm_to_pix_x,
                   m_mm_to_pix_y;

    // bounding and clipping boxes
    int m_minX, m_minY, m_maxX, m_maxY; // Bounding box is stored in device units.
    int m_clipX1, m_clipY1, m_clipX2, m_clipY2;  // Clipping box is stored in logical units.

    wxRasterOperationMode m_logicalFunction;
    int m_backgroundMode;
    wxMappingMode m_mappingMode;

    QPen             m_pen;
    QBrush           m_brush;
    QBrush           m_backgroundBrush;
    QColor          m_textForegroundColour;
    QColor          m_textBackgroundColour;
    QFont            m_font;

#if wxUSE_PALETTE
    wxPalette         m_palette;
    bool              m_hasCustomPalette;
#endif // wxUSE_PALETTE

private:
    // Return the full DC area in logical coordinates.
    QRect GetLogicalArea() const;
    Qt::LayoutDirection    mDir;

//    wxDECLARE_ABSTRACT_CLASS(wxDCImpl);
};


class wxDC/* : public wxObject*/
{
public:
    // copy attributes (font, colours and writing direction) from another DC
    void CopyAttributes(const wxDC& dc);

    virtual ~wxDC() { delete m_pimpl; }

    wxDCImpl *GetImpl()
        { return m_pimpl; }
    const wxDCImpl *GetImpl() const
        { return m_pimpl; }

    wxWindow *GetWindow() const
        { return m_pimpl->GetWindow(); }

    void *GetHandle() const
        { return m_pimpl->GetHandle(); }

    bool IsOk() const
        { return m_pimpl && m_pimpl->IsOk(); }

    // query capabilities

    bool CanDrawBitmap() const
        { return m_pimpl->CanDrawBitmap(); }
    bool CanGetTextExtent() const
        { return m_pimpl->CanGetTextExtent(); }

    // query dimension, colour deps, resolution

    void GetSize(int *width, int *height) const
        { m_pimpl->DoGetSize(width, height); }
    QSize GetSize() const
        { return m_pimpl->GetSize(); }

    void GetSizeMM(int* width, int* height) const
        { m_pimpl->DoGetSizeMM(width, height); }
    QSize GetSizeMM() const
    {
        int w, h;
        m_pimpl->DoGetSizeMM(&w, &h);
        return QSize(w, h);
    }

    int GetDepth() const
        { return m_pimpl->GetDepth(); }
    QSize GetPPI() const
        { return m_pimpl->GetPPI(); }

    virtual int GetResolution() const
        { return m_pimpl->GetResolution(); }

    double GetContentScaleFactor() const
        { return m_pimpl->GetContentScaleFactor(); }

    // Right-To-Left (RTL) modes

    void SetLayoutDirection(Qt::LayoutDirection dir)
        { m_pimpl->SetLayoutDirection( dir ); }
    Qt::LayoutDirection GetLayoutDirection() const
        { return m_pimpl->GetLayoutDirection(); }

    // page and document

    bool StartDoc(const QString& message)
        { return m_pimpl->StartDoc(message); }
    void EndDoc()
        { m_pimpl->EndDoc(); }

    void StartPage()
        { m_pimpl->StartPage(); }
    void EndPage()
        { m_pimpl->EndPage(); }

    // bounding box

    void CalcBoundingBox(int x, int y)
        { m_pimpl->CalcBoundingBox(x,y); }
    void ResetBoundingBox()
        { m_pimpl->ResetBoundingBox(); }

    int MinX() const
        { return m_pimpl->MinX(); }
    int MaxX() const
        { return m_pimpl->MaxX(); }
    int MinY() const
        { return m_pimpl->MinY(); }
    int MaxY() const
        { return m_pimpl->MaxY(); }

    // setters and getters

    void SetFont(const QFont& font)
        { m_pimpl->SetFont( font ); }
    const QFont&   GetFont() const
        { return m_pimpl->GetFont(); }

    void SetPen(const QPen& pen)
        { m_pimpl->SetPen( pen ); }
    const QPen&    GetPen() const
        { return m_pimpl->GetPen(); }

    void SetBrush(const QBrush& brush)
        { m_pimpl->SetBrush( brush ); }
    const QBrush&  GetBrush() const
        { return m_pimpl->GetBrush(); }

    void SetBackground(const QBrush& brush)
        { m_pimpl->SetBackground( brush ); }
    const QBrush&  GetBackground() const
        { return m_pimpl->GetBackground(); }

    void SetBackgroundMode(int mode)
        { m_pimpl->SetBackgroundMode( mode ); }
    int GetBackgroundMode() const
        { return m_pimpl->GetBackgroundMode(); }

    void SetTextForeground(const QColor& colour)
        { m_pimpl->SetTextForeground(colour); }
    const QColor& GetTextForeground() const
        { return m_pimpl->GetTextForeground(); }

    void SetTextBackground(const QColor& colour)
        { m_pimpl->SetTextBackground(colour); }
    const QColor& GetTextBackground() const
        { return m_pimpl->GetTextBackground(); }

#if wxUSE_PALETTE
    void SetPalette(const wxPalette& palette)
        { m_pimpl->SetPalette(palette); }
#endif // wxUSE_PALETTE

    // logical functions

    void SetLogicalFunction(wxRasterOperationMode function)
        { m_pimpl->SetLogicalFunction(function); }
    wxRasterOperationMode GetLogicalFunction() const
        { return m_pimpl->GetLogicalFunction(); }

    // text measurement

    int GetCharHeight() const
        { return m_pimpl->GetCharHeight(); }
    int GetCharWidth() const
        { return m_pimpl->GetCharWidth(); }

    wxFontMetrics GetFontMetrics() const
    {
        wxFontMetrics fm;
        m_pimpl->DoGetFontMetrics(&fm.height, &fm.ascent, &fm.descent,
                                  &fm.internalLeading, &fm.externalLeading,
                                  &fm.averageWidth);
        return fm;
    }

    void GetTextExtent(const QString& string,
                       int *x, int *y,
                       int *descent = NULL,
                       int *externalLeading = NULL,
                       const QFont *theFont = NULL) const
        { m_pimpl->DoGetTextExtent(string, x, y, descent, externalLeading, theFont); }

    QSize GetTextExtent(const QString& string) const
    {
        int w, h;
        m_pimpl->DoGetTextExtent(string, &w, &h);
        return QSize(w, h);
    }

    void GetMultiLineTextExtent(const QString& string,
                                        int *width,
                                        int *height,
                                        int *heightLine = NULL,
                                        const QFont *font = NULL) const
        { m_pimpl->GetMultiLineTextExtent( string, width, height, heightLine, font ); }

    QSize GetMultiLineTextExtent(const QString& string) const
    {
        int w, h;
        m_pimpl->GetMultiLineTextExtent(string, &w, &h);
        return QSize(w, h);
    }

    bool GetPartialTextExtents(const QString& text, QList<int>& widths) const
        { return m_pimpl->DoGetPartialTextExtents(text, widths); }

    // clearing

    void Clear()
        { m_pimpl->Clear(); }

    // clipping

    void SetClippingRegion(int x, int y, int width, int height)
        { m_pimpl->DoSetClippingRegion(x, y, width, height); }
    void SetClippingRegion(const zchxPoint& pt, const QSize& sz)
        { m_pimpl->DoSetClippingRegion(pt.x, pt.y, sz.width(), sz.height()); }
    void SetClippingRegion(const QRect& rect)
        { m_pimpl->DoSetClippingRegion(rect.x(), rect.y(), rect.width(), rect.height()); }

    // unlike the functions above, the coordinates of the region used in this
    // one are in device coordinates, not the logical ones
    void SetDeviceClippingRegion(const QRegion& region)
        { m_pimpl->DoSetDeviceClippingRegion(region); }

    // this function is deprecated because its name is confusing: you may
    // expect it to work with logical coordinates but, in fact, it does exactly
    // the same thing as SetDeviceClippingRegion()
    //
    // please review the code using it and either replace it with calls to
    // SetDeviceClippingRegion() or correct it if it was [wrongly] passing
    // logical coordinates to this function
    void SetClippingRegion(const QRegion& region)
    {
        SetDeviceClippingRegion(region);
    }

    void DestroyClippingRegion()
        { m_pimpl->DestroyClippingRegion(); }

    bool GetClippingBox(int *x, int *y, int *w, int *h) const
    {
        QRect r;
        const bool clipping = m_pimpl->DoGetClippingRect(r);
        if ( x )
            *x = r.x();
        if ( y )
            *y = r.y();
        if ( w )
            *w = r.width();
        if ( h )
            *h = r.height();
        return clipping;
    }
    bool GetClippingBox(QRect& rect) const
        { return m_pimpl->DoGetClippingRect(rect); }

    // coordinates conversions and transforms

    int DeviceToLogicalX(int x) const
        { return m_pimpl->DeviceToLogicalX(x); }
    int DeviceToLogicalY(int y) const
        { return m_pimpl->DeviceToLogicalY(y); }
    int DeviceToLogicalXRel(int x) const
        { return m_pimpl->DeviceToLogicalXRel(x); }
    int DeviceToLogicalYRel(int y) const
        { return m_pimpl->DeviceToLogicalYRel(y); }
    int LogicalToDeviceX(int x) const
        { return m_pimpl->LogicalToDeviceX(x); }
    int LogicalToDeviceY(int y) const
        { return m_pimpl->LogicalToDeviceY(y); }
    int LogicalToDeviceXRel(int x) const
        { return m_pimpl->LogicalToDeviceXRel(x); }
    int LogicalToDeviceYRel(int y) const
        { return m_pimpl->LogicalToDeviceYRel(y); }

    void SetMapMode(wxMappingMode mode)
        { m_pimpl->SetMapMode(mode); }
    wxMappingMode GetMapMode() const
        { return m_pimpl->GetMapMode(); }

    void SetUserScale(double x, double y)
        { m_pimpl->SetUserScale(x,y); }
    void GetUserScale(double *x, double *y) const
        { m_pimpl->GetUserScale( x, y ); }

    void SetLogicalScale(double x, double y)
        { m_pimpl->SetLogicalScale( x, y ); }
    void GetLogicalScale(double *x, double *y) const
        { m_pimpl->GetLogicalScale( x, y ); }

    void SetLogicalOrigin(int x, int y)
        { m_pimpl->SetLogicalOrigin(x,y); }
    void GetLogicalOrigin(int *x, int *y) const
        { m_pimpl->DoGetLogicalOrigin(x, y); }
    zchxPoint GetLogicalOrigin() const
        { int x, y; m_pimpl->DoGetLogicalOrigin(&x, &y); return zchxPoint(x, y); }

    void SetDeviceOrigin(int x, int y)
        { m_pimpl->SetDeviceOrigin( x, y); }
    void GetDeviceOrigin(int *x, int *y) const
        { m_pimpl->DoGetDeviceOrigin(x, y); }
    zchxPoint GetDeviceOrigin() const
        { int x, y; m_pimpl->DoGetDeviceOrigin(&x, &y); return zchxPoint(x, y); }

    void SetAxisOrientation(bool xLeftRight, bool yBottomUp)
        { m_pimpl->SetAxisOrientation(xLeftRight, yBottomUp); }

#if wxUSE_DC_TRANSFORM_MATRIX
    bool CanUseTransformMatrix() const
        { return m_pimpl->CanUseTransformMatrix(); }

    bool SetTransformMatrix(const wxAffineMatrix2D &matrix)
        { return m_pimpl->SetTransformMatrix(matrix); }

    wxAffineMatrix2D GetTransformMatrix() const
        { return m_pimpl->GetTransformMatrix(); }

    void ResetTransformMatrix()
        { m_pimpl->ResetTransformMatrix(); }
#endif // wxUSE_DC_TRANSFORM_MATRIX

    // mostly internal
    void SetDeviceLocalOrigin( int x, int y )
        { m_pimpl->SetDeviceLocalOrigin( x, y ); }


    // -----------------------------------------------
    // the actual drawing API

    bool FloodFill(int x, int y, const QColor& col,
                   wxFloodFillStyle style = wxFLOOD_SURFACE)
        { return m_pimpl->DoFloodFill(x, y, col, style); }
    bool FloodFill(const zchxPoint& pt, const QColor& col,
                   wxFloodFillStyle style = wxFLOOD_SURFACE)
        { return m_pimpl->DoFloodFill(pt.x, pt.y, col, style); }

    // fill the area specified by rect with a radial gradient, starting from
    // initialColour in the centre of the cercle and fading to destColour.
    void GradientFillConcentric(const QRect& rect,
                                const QColor& initialColour,
                                const QColor& destColour)
        { m_pimpl->DoGradientFillConcentric( rect, initialColour, destColour,
                                             zchxPoint(rect.width() / 2,
                                                     rect.height() / 2)); }

    void GradientFillConcentric(const QRect& rect,
                                const QColor& initialColour,
                                const QColor& destColour,
                                const zchxPoint& circleCenter)
        { m_pimpl->DoGradientFillConcentric(rect, initialColour, destColour, circleCenter); }

    // fill the area specified by rect with a linear gradient
    void GradientFillLinear(const QRect& rect,
                            const QColor& initialColour,
                            const QColor& destColour,
                            /*wxDirection nDirection = wxEAST*/int nDirection)
        { m_pimpl->DoGradientFillLinear(rect, initialColour, destColour, nDirection); }

    bool GetPixel(int x, int y, QColor *col) const
        { return m_pimpl->DoGetPixel(x, y, col); }
    bool GetPixel(const zchxPoint& pt, QColor *col) const
        { return m_pimpl->DoGetPixel(pt.x, pt.y, col); }

    void DrawLine(int x1, int y1, int x2, int y2)
        { m_pimpl->DoDrawLine(x1, y1, x2, y2); }
    void DrawLine(const zchxPoint& pt1, const zchxPoint& pt2)
        { m_pimpl->DoDrawLine(pt1.x, pt1.y, pt2.x, pt2.y); }

    void CrossHair(int x, int y)
        { m_pimpl->DoCrossHair(x, y); }
    void CrossHair(const zchxPoint& pt)
        { m_pimpl->DoCrossHair(pt.x, pt.y); }

    void DrawArc(int x1, int y1, int x2, int y2,
                 int xc, int yc)
        { m_pimpl->DoDrawArc(x1, y1, x2, y2, xc, yc); }
    void DrawArc(const zchxPoint& pt1, const zchxPoint& pt2, const zchxPoint& centre)
        { m_pimpl->DoDrawArc(pt1.x, pt1.y, pt2.x, pt2.y, centre.x, centre.y); }

    void DrawCheckMark(int x, int y,
                       int width, int height)
        { m_pimpl->DoDrawCheckMark(x, y, width, height); }
    void DrawCheckMark(const QRect& rect)
        { m_pimpl->DoDrawCheckMark(rect.x(), rect.y(), rect.width(), rect.height()); }

    void DrawEllipticArc(int x, int y, int w, int h,
                         double sa, double ea)
        { m_pimpl->DoDrawEllipticArc(x, y, w, h, sa, ea); }
    void DrawEllipticArc(const zchxPoint& pt, const QSize& sz,
                         double sa, double ea)
        { m_pimpl->DoDrawEllipticArc(pt.x, pt.y, sz.width(), sz.height(), sa, ea); }

    void DrawPoint(int x, int y)
        { m_pimpl->DoDrawPoint(x, y); }
    void DrawPoint(const zchxPoint& pt)
        { m_pimpl->DoDrawPoint(pt.x, pt.y); }

    void DrawLines(int n, const zchxPoint points[],
                   int xoffset = 0, int yoffset = 0)
        { m_pimpl->DoDrawLines(n, points, xoffset, yoffset); }
    void DrawLines(const QList<zchxPoint> *list,
                   int xoffset = 0, int yoffset = 0)
        { m_pimpl->DrawLines( list, xoffset, yoffset ); }
#if WXWIN_COMPATIBILITY_2_8
    wxDEPRECATED( void DrawLines(const wxList *list,
                                 int xoffset = 0, int yoffset = 0) );
#endif  // WXWIN_COMPATIBILITY_2_8

    void DrawPolygon(int n, const zchxPoint points[],
                     int xoffset = 0, int yoffset = 0,
                     Qt::FillRule fillStyle = Qt::OddEvenFill)
        { m_pimpl->DoDrawPolygon(n, points, xoffset, yoffset, fillStyle); }
    void DrawPolygon(const QList<zchxPoint> *list,
                     int xoffset = 0, int yoffset = 0,
                     Qt::FillRule fillStyle = Qt::OddEvenFill)
        { m_pimpl->DrawPolygon( list, xoffset, yoffset, fillStyle ); }
    void DrawPolyPolygon(int n, const int count[], const zchxPoint points[],
                         int xoffset = 0, int yoffset = 0,
                         Qt::FillRule fillStyle = Qt::OddEvenFill)
        { m_pimpl->DoDrawPolyPolygon(n, count, points, xoffset, yoffset, fillStyle); }
#if WXWIN_COMPATIBILITY_2_8
    wxDEPRECATED( void DrawPolygon(const wxList *list,
                     int xoffset = 0, int yoffset = 0,
                     Qt::FillRule fillStyle = Qt::OddEvenFill) );
#endif  // WXWIN_COMPATIBILITY_2_8

    void DrawRectangle(int x, int y, int width, int height)
        { m_pimpl->DoDrawRectangle(x, y, width, height); }
    void DrawRectangle(const zchxPoint& pt, const QSize& sz)
        { m_pimpl->DoDrawRectangle(pt.x, pt.y, sz.width(), sz.height()); }
    void DrawRectangle(const QRect& rect)
        { m_pimpl->DoDrawRectangle(rect.x(), rect.y(), rect.width(), rect.height()); }

    void DrawRoundedRectangle(int x, int y, int width, int height,
                              double radius)
        { m_pimpl->DoDrawRoundedRectangle(x, y, width, height, radius); }
    void DrawRoundedRectangle(const zchxPoint& pt, const QSize& sz,
                             double radius)
        { m_pimpl->DoDrawRoundedRectangle(pt.x, pt.y, sz.width(), sz.height(), radius); }
    void DrawRoundedRectangle(const QRect& r, double radius)
        { m_pimpl->DoDrawRoundedRectangle(r.x(), r.y(), r.width(), r.height(), radius); }

    void DrawCircle(int x, int y, int radius)
        { m_pimpl->DoDrawEllipse(x - radius, y - radius, 2*radius, 2*radius); }
    void DrawCircle(const zchxPoint& pt, int radius)
        { m_pimpl->DoDrawEllipse(pt.x - radius, pt.y - radius, 2*radius, 2*radius); }

    void DrawEllipse(int x, int y, int width, int height)
        { m_pimpl->DoDrawEllipse(x, y, width, height); }
    void DrawEllipse(const zchxPoint& pt, const QSize& sz)
        { m_pimpl->DoDrawEllipse(pt.x, pt.y, sz.width(), sz.height()); }
    void DrawEllipse(const QRect& rect)
        { m_pimpl->DoDrawEllipse(rect.x(), rect.y(), rect.width(), rect.height()); }

    void DrawIcon(const QIcon& icon, int x, int y)
        { m_pimpl->DoDrawIcon(icon, x, y); }
    void DrawIcon(const QIcon& icon, const zchxPoint& pt)
        { m_pimpl->DoDrawIcon(icon, pt.x, pt.y); }

    void DrawBitmap(const wxBitmap &bmp, int x, int y,
                    bool useMask = false)
        { m_pimpl->DoDrawBitmap(bmp, x, y, useMask); }
    void DrawBitmap(const wxBitmap &bmp, const zchxPoint& pt,
                    bool useMask = false)
        { m_pimpl->DoDrawBitmap(bmp, pt.x, pt.y, useMask); }

    void DrawText(const QString& text, int x, int y)
        { m_pimpl->DoDrawText(text, x, y); }
    void DrawText(const QString& text, const zchxPoint& pt)
        { m_pimpl->DoDrawText(text, pt.x, pt.y); }

    void DrawRotatedText(const QString& text, int x, int y, double angle)
        { m_pimpl->DoDrawRotatedText(text, x, y, angle); }
    void DrawRotatedText(const QString& text, const zchxPoint& pt, double angle)
        { m_pimpl->DoDrawRotatedText(text, pt.x, pt.y, angle); }

    // this version puts both optional bitmap and the text into the given
    // rectangle and aligns is as specified by alignment parameter; it also
    // will emphasize the character with the given index if it is != -1 and
    // return the bounding rectangle if required
    void DrawLabel(const QString& text,
                           const wxBitmap& image,
                           const QRect& rect,
                           int alignment = Qt::AlignLeft | Qt::AlignTop,
                           int indexAccel = -1,
                           QRect *rectBounding = NULL);

    void DrawLabel(const QString& text, const QRect& rect,
                   int alignment = Qt::AlignLeft | Qt::AlignTop,
                   int indexAccel = -1)
        { DrawLabel(text, QBitmap(), rect, alignment, indexAccel); }

    bool Blit(int xdest, int ydest, int width, int height,
              wxDC *source, int xsrc, int ysrc,
              wxRasterOperationMode rop = wxCOPY, bool useMask = false,
              int xsrcMask = -1, int ysrcMask = -1)
    {
        return m_pimpl->DoBlit(xdest, ydest, width, height,
                      source, xsrc, ysrc, rop, useMask, xsrcMask, ysrcMask);
    }
    bool Blit(const zchxPoint& destPt, const QSize& sz,
              wxDC *source, const zchxPoint& srcPt,
              wxRasterOperationMode rop = wxCOPY, bool useMask = false,
              const zchxPoint& srcPtMask = zchxPoint(-1, -1))
    {
        return m_pimpl->DoBlit(destPt.x, destPt.y, sz.width(), sz.height(),
                      source, srcPt.x, srcPt.y, rop, useMask, srcPtMask.x, srcPtMask.y);
    }

    bool StretchBlit(int dstX, int dstY,
                     int dstWidth, int dstHeight,
                     wxDC *source,
                     int srcX, int srcY,
                     int srcWidth, int srcHeight,
                     wxRasterOperationMode rop = wxCOPY, bool useMask = false,
                     int srcMaskX = -1, int srcMaskY = -1)
    {
        return m_pimpl->DoStretchBlit(dstX, dstY, dstWidth, dstHeight,
                      source, srcX, srcY, srcWidth, srcHeight, rop, useMask, srcMaskX, srcMaskY);
    }
    bool StretchBlit(const zchxPoint& dstPt, const QSize& dstSize,
                     wxDC *source, const zchxPoint& srcPt, const QSize& srcSize,
                     wxRasterOperationMode rop = wxCOPY, bool useMask = false,
                     const zchxPoint& srcMaskPt = zchxPoint(-1, -1))
    {
        return m_pimpl->DoStretchBlit(dstPt.x, dstPt.y, dstSize.width(), dstSize.height(),
                      source, srcPt.x, srcPt.y, srcSize.width(), srcSize.height(), rop, useMask, srcMaskPt.x, srcMaskPt.y);
    }

    wxBitmap GetAsBitmap(const QRect *subrect = (const QRect *) NULL) const
    {
        return m_pimpl->DoGetAsBitmap(subrect);
    }

#if wxUSE_SPLINES
    void DrawSpline(int x1, int y1,
                    int x2, int y2,
                    int x3, int y3)
        { m_pimpl->DrawSpline(x1,y1,x2,y2,x3,y3); }
    void DrawSpline(int n, const zchxPoint points[])
        { m_pimpl->DrawSpline(n,points); }
    void DrawSpline(const zchxPointList *points)
        { m_pimpl->DrawSpline(points); }
#endif // wxUSE_SPLINES


#if WXWIN_COMPATIBILITY_2_8
    // for compatibility with the old code when int was long everywhere
    wxDEPRECATED( void GetTextExtent(const QString& string,
                       long *x, long *y,
                       long *descent = NULL,
                       long *externalLeading = NULL,
                       const QFont *theFont = NULL) const );
    wxDEPRECATED( void GetLogicalOrigin(long *x, long *y) const );
    wxDEPRECATED( void GetDeviceOrigin(long *x, long *y) const );
    wxDEPRECATED( void GetClippingBox(long *x, long *y, long *w, long *h) const );

    wxDEPRECATED( void DrawObject(wxDrawObject* drawobject) );
#endif  // WXWIN_COMPATIBILITY_2_8

#ifdef __WXMSW__
    // GetHDC() is the simplest way to retrieve an HDC From a wxDC but only
    // works if this wxDC is GDI-based and fails for GDI+ contexts (and
    // anything else without HDC, e.g. wxPostScriptDC)
    WXHDC GetHDC() const;

    // don't use these methods manually, use GetTempHDC() instead
    virtual WXHDC AcquireHDC() { return GetHDC(); }
    virtual void ReleaseHDC(WXHDC WXUNUSED(hdc)) { }

    // helper class holding the result of GetTempHDC() with std::auto_ptr<>-like
    // semantics, i.e. it is moved when copied
    class TempHDC
    {
    public:
        TempHDC(wxDC& dc)
            : m_dc(dc),
              m_hdc(dc.AcquireHDC())
        {
        }

        TempHDC(const TempHDC& thdc)
            : m_dc(thdc.m_dc),
              m_hdc(thdc.m_hdc)
        {
            const_cast<TempHDC&>(thdc).m_hdc = 0;
        }

        ~TempHDC()
        {
            if ( m_hdc )
                m_dc.ReleaseHDC(m_hdc);
        }

        WXHDC GetHDC() const { return m_hdc; }

    private:
        wxDC& m_dc;
        WXHDC m_hdc;

        wxDECLARE_NO_ASSIGN_CLASS(TempHDC);
    };

    // GetTempHDC() also works for wxGCDC (but still not for wxPostScriptDC &c)
    TempHDC GetTempHDC() { return TempHDC(*this); }
#endif // __WXMSW__

#if wxUSE_GRAPHICS_CONTEXT
    virtual wxGraphicsContext* GetGraphicsContext() const
    {
        return m_pimpl->GetGraphicsContext();
    }
    virtual void SetGraphicsContext( wxGraphicsContext* ctx )
    {
        m_pimpl->SetGraphicsContext(ctx);
    }
#endif

protected:
    // ctor takes ownership of the pointer
    wxDC(wxDCImpl *pimpl) : m_pimpl(pimpl) { }

    wxDCImpl * const m_pimpl;

private:
    wxDECLARE_ABSTRACT_CLASS(wxDC);
    wxDECLARE_NO_COPY_CLASS(wxDC);
};

// ----------------------------------------------------------------------------
// helper class: you can use it to temporarily change the DC text colour and
// restore it automatically when the object goes out of scope
// ----------------------------------------------------------------------------

class  wxDCTextColourChanger
{
public:
    wxDCTextColourChanger(wxDC& dc) : m_dc(dc), m_colFgOld() { }

    wxDCTextColourChanger(wxDC& dc, const QColor& col) : m_dc(dc)
    {
        Set(col);
    }

    ~wxDCTextColourChanger()
    {
        if ( m_colFgOld.isValid() )
            m_dc.SetTextForeground(m_colFgOld);
    }

    void Set(const QColor& col)
    {
        if ( !m_colFgOld.isValid() )
            m_colFgOld = m_dc.GetTextForeground();
        m_dc.SetTextForeground(col);
    }

private:
    wxDC& m_dc;

    QColor m_colFgOld;

//    wxDECLARE_NO_COPY_CLASS(wxDCTextColourChanger);
};

// ----------------------------------------------------------------------------
// helper class: you can use it to temporarily change the DC pen and
// restore it automatically when the object goes out of scope
// ----------------------------------------------------------------------------

class wxDCPenChanger
{
public:
    wxDCPenChanger(wxDC& dc, const QPen& pen) : m_dc(dc), m_penOld(dc.GetPen())
    {
        m_dc.SetPen(pen);
    }

    ~wxDCPenChanger()
    {
            m_dc.SetPen(m_penOld);
    }

private:
    wxDC& m_dc;

    QPen m_penOld;

    wxDECLARE_NO_COPY_CLASS(wxDCPenChanger);
};

// ----------------------------------------------------------------------------
// helper class: you can use it to temporarily change the DC brush and
// restore it automatically when the object goes out of scope
// ----------------------------------------------------------------------------

class wxDCBrushChanger
{
public:
    wxDCBrushChanger(wxDC& dc, const QBrush& brush) : m_dc(dc), m_brushOld(dc.GetBrush())
    {
        m_dc.SetBrush(brush);
    }

    ~wxDCBrushChanger()
    {
//        if ( m_brushOld.IsOk() )
            m_dc.SetBrush(m_brushOld);
    }

private:
    wxDC& m_dc;

    QBrush m_brushOld;

    wxDECLARE_NO_COPY_CLASS(wxDCBrushChanger);
};

// ----------------------------------------------------------------------------
// another small helper class: sets the clipping region in its ctor and
// destroys it in the dtor
// ----------------------------------------------------------------------------

class  wxDCClipper
{
public:
    wxDCClipper(wxDC& dc, const QRegion& r) : m_dc(dc)
    {
        Init(r.boundingRect());
    }
    wxDCClipper(wxDC& dc, const QRect& r) : m_dc(dc)
    {
        Init(r);
    }
    wxDCClipper(wxDC& dc, int x, int y, int w, int h) : m_dc(dc)
    {
        Init(QRect(x, y, w, h));
    }

    ~wxDCClipper()
    {
        m_dc.DestroyClippingRegion();
        if ( m_restoreOld )
            m_dc.SetClippingRegion(m_oldClipRect);
    }

private:
    // Common part of all ctors.
    void Init(const QRect& r)
    {
        m_restoreOld = m_dc.GetClippingBox(m_oldClipRect);
        m_dc.SetClippingRegion(r);
    }

    wxDC& m_dc;
    QRect m_oldClipRect;
    bool m_restoreOld;

    wxDECLARE_NO_COPY_CLASS(wxDCClipper);
};

// ----------------------------------------------------------------------------
// helper class: you can use it to temporarily change the DC font and
// restore it automatically when the object goes out of scope
// ----------------------------------------------------------------------------

class  wxDCFontChanger
{
public:
    wxDCFontChanger(wxDC& dc)
        : m_dc(dc), m_fontOld()
    {
    }

    wxDCFontChanger(wxDC& dc, const QFont& font)
        : m_dc(dc), m_fontOld(dc.GetFont())
    {
        m_dc.SetFont(font);
    }

    void Set(const QFont& font)
    {
//        if ( !m_fontOld.IsOk() )
            m_fontOld = m_dc.GetFont();
        m_dc.SetFont(font);
    }

    ~wxDCFontChanger()
    {
//        if ( m_fontOld.IsOk() )
            m_dc.SetFont(m_fontOld);
    }

private:
    wxDC& m_dc;

    QFont m_fontOld;

    wxDECLARE_NO_COPY_CLASS(wxDCFontChanger);
};


#endif // _WX_DC_H_BASE_
