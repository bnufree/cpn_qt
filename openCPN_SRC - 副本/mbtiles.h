/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  MBTiles Chart Support
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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


#ifndef _CHARTMBTILES_H_
#define _CHARTMBTILES_H_


#include "chartbase.h"
#include "georef.h"                 // for GeoRef type
#include "OCPNRegion.h"
#include "viewport.h"
#include "opencpn_global.h"


enum class MBTilesType : std::int8_t {BASE, OVERLAY};
enum class MBTilesScheme : std::int8_t {XYZ, TMS};

//-----------------------------------------------------------------------------
//    Constants, etc.
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//    Fwd Refs
//-----------------------------------------------------------------------------

class ViewPort;
class PixelCache;
class ocpnBitmap;
class mbTileZoomDescriptor;
class mbTileDescriptor;

 namespace SQLite {
   class Database;
 }
 
//-----------------------------------------------------------------------------
//    Helper classes
//-----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// ChartMBTiles
// ----------------------------------------------------------------------------

class ZCHX_OPENCPN_EXPORT ChartMBTiles     :public ChartBase
{
    public:
      //    Public methods

      ChartMBTiles();
      virtual ~ChartMBTiles();
 
      //    Accessors
      virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
      virtual ThumbData *GetThumbData();
      virtual bool UpdateThumbData(double lat, double lon);
      
      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);
      
      int GetNativeScale(){return m_Chart_Scale;}
      double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
      double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);

      virtual InitReturn Init( const QString& name, ChartInitFlag init_flags );


      bool RenderRegionViewOnDC(QPainter* dc, const ViewPort& VPoint, const OCPNRegion &Region);

      virtual bool RenderRegionViewOnGL(QGLContext *glc, const ViewPort& VPoint,
                                        const OCPNRegion &RectRegion, const LLRegion &Region);

      virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

      virtual void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion);
      virtual LLRegion GetValidRegion();

      virtual bool GetChartExtent(Extent *pext);

      void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

      double GetPPM(){ return m_ppm_avg;}
      double GetZoomFactor(){ return m_zoomScaleFactor; }
      
protected:
//    Methods
      bool RenderViewOnDC(QPainter* dc, const ViewPort& VPoint);
      InitReturn PreInit( const QString& name, ChartInitFlag init_flags, ColorScheme cs );
      InitReturn PostInit(void);

      void PrepareTiles();
      void PrepareTilesForZoom(int zoomFactor, bool bset_geom);
      bool getTileTexture( mbTileDescriptor *tile);
      void FlushTiles( void );
      void FlushTextures( void );
      bool RenderTile( mbTileDescriptor *tile, int zoomLevel, const ViewPort& VPoint);


//    Protected Data


      float       m_LonMax, m_LonMin, m_LatMax, m_LatMin;

      double      m_ppm_avg;              // Calculated true scale factor of the 1X chart,
                                        // pixels per meter

      int       m_b_cdebug;

      int       m_minZoom, m_maxZoom;
      mbTileZoomDescriptor      **m_tileArray;
      LLRegion  m_minZoomRegion;
//      wxBitmapType m_imageType;
//      Format	MIME type	Description
//      BMP	image/bmp	Windows Bitmap
//      GIF	image/gif	Graphic Interchange Format (optional)
//      JPG	image/jpeg	Joint Photographic Experts Group
//      PNG	image/png	Portable Network Graphics
//      PBM	image/x-portable-bitmap	Portable Bitmap
//      PGM	image/x-portable-graymap	Portable Graymap
//      PPM	image/x-portable-pixmap	Portable Pixmap
//      XBM	image/x-xbitmap	X11 Bitmap
//      XPM	image/x-xpixmap	X11 Pixmap
//      SVG	image/svg+xml	Scalable Vector Graphics
      QString    m_imageType;           //图像的类型
      
      double m_zoomScaleFactor;
    
      MBTilesType m_Type;
      MBTilesScheme m_Scheme;
      
      SQLite::Database  *m_pDB;
      int       m_nTiles;
      
private:
      void InitFromTiles( const QString& name );
      zchxPointF GetDoublePixFromLL( ViewPort& vp, double lat, double lon );

};



#endif
