﻿/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  ChartBase Definition
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 **************************************************************************/

#ifndef _CHARTBASE_H_
#define _CHARTBASE_H_

#include "dychart.h"

#include "bbox.h"
#include "LLRegion.h"
#include "_def.h"
#include <QDateTime>

//----------------------------------------------------------------------------
//  Forward Declarations
//----------------------------------------------------------------------------
class ViewPort;
class QGLContext;
class OCPNRegion;

//----------------------------------------------------------------------------
// Constants. etc
//----------------------------------------------------------------------------

//    ChartBase::Init()  init_flags constants
typedef enum ChartInitFlag
{
      FULL_INIT = 0,
      HEADER_ONLY,
      THUMB_ONLY
}_ChartInitFlag;


typedef enum RenderTypeEnum
{
      DC_RENDER_ONLY = 0,
      DC_RENDER_RETURN_DIB,
      DC_RENDER_RETURN_IMAGE
}_RenderTypeEnum;

typedef enum InitReturn
{
      INIT_OK = 0,
      INIT_FAIL_RETRY,        // Init failed, retry suggested
      INIT_FAIL_REMOVE,       // Init failed, suggest remove from further use
      INIT_FAIL_NOERROR       // Init failed, request no explicit error message
}_InitReturn;



class ThumbData
{
public:
    ThumbData();
    virtual ~ThumbData();

      QBitmap    *pDIBThumb;
      int         ShipX;
      int         ShipY;
      int         Thumb_Size_X;
      int         Thumb_Size_Y;
};



typedef struct _Extent{
  float SLAT;
  float WLON;
  float NLAT;
  float ELON;
}Extent;

//          Depth unit type enum
typedef enum ChartDepthUnitType
{
    DEPTH_UNIT_UNKNOWN,
    DEPTH_UNIT_FEET,
    DEPTH_UNIT_METERS,
    DEPTH_UNIT_FATHOMS
}_ChartDepthUnitType;

//          Projection type enum
typedef enum OcpnProjType
{
      PROJECTION_UNKNOWN,
      PROJECTION_MERCATOR,
      PROJECTION_TRANSVERSE_MERCATOR,
      PROJECTION_POLYCONIC,

      PROJECTION_ORTHOGRAPHIC,
      PROJECTION_POLAR,
      PROJECTION_STEREOGRAPHIC,
      PROJECTION_GNOMONIC,
      PROJECTION_EQUIRECTANGULAR,
      PROJECTION_WEB_MERCATOR
}_OcpnProjType;



class Plypoint
{
      public:
            float ltp;
            float lnp;
};


// ----------------------------------------------------------------------------
// ChartBase
// ----------------------------------------------------------------------------

class ChartBase
{

public:
      ChartBase();
      virtual ~ChartBase() = 0;

      virtual InitReturn Init( const QString& name, ChartInitFlag init_flags) = 0;

      virtual void Activate(void) {}
      virtual void Deactivate(void) {}

//    Accessors
      virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon) = 0;
      virtual ThumbData *GetThumbData() = 0;
      virtual bool UpdateThumbData(double lat, double lon) = 0;

      virtual double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom) = 0;
      virtual double GetNormalScaleMax(double canvas_scale_factor, int canvas_width) = 0;

      virtual bool GetChartExtent(Extent *pext) = 0;


      virtual OcpnProjType GetChartProjectionType(){ return m_projection;}
      virtual QDateTime GetEditionDate(void){ return m_EdDate;}
      virtual QDateTime getModifyTime(void);
      virtual quint32   getFileSize(void);

      virtual QString GetPubDate(){ return m_PubYear;}
      virtual int GetNativeScale(){ return m_Chart_Scale;}
      QString GetFullPath() const { return m_FullPath;}
      QString GetHashKey() const;
      QString GetName(){ return m_Name;}
      QString GetDescription() { return m_Description;}
      QString GetID(){ return m_ID;}
      QString GetSE(){ return m_SE;}
      QString GetDepthUnits(){ return m_DepthUnits;}
      QString GetSoundingsDatum(){ return m_SoundingsDatum;}
      QString GetDatumString(){ return m_datum_str;}
      QString GetExtraInfo(){ return m_ExtraInfo; }
      double GetChart_Error_Factor(){ return Chart_Error_Factor; }
      ChartTypeEnum GetChartType(){ return m_ChartType;}
      ChartFamilyEnum GetChartFamily(){ return m_ChartFamily;}
      double GetChartSkew(){ return m_Chart_Skew; }


      virtual ChartDepthUnitType GetDepthUnitType(void) { return m_depth_unit_id;}

      virtual bool IsReadyToRender(){ return bReadyToRender;}
      virtual bool RenderRegionViewOnDC(QPainter* dc, const ViewPort& VPoint,
                                        const OCPNRegion &Region) = 0;

      virtual bool RenderRegionViewOnGL(const QGLContext &glc, const ViewPort& VPoint,
                                        const OCPNRegion &RectRegion, const LLRegion &Region) = 0;

      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed) = 0;

      virtual void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion) = 0;
      virtual LLRegion GetValidRegion() = 0;

      virtual void SetColorScheme(ColorScheme cs, bool bApplyImmediate = true ) = 0;

      virtual double GetNearestPreferredScalePPM(double target_scale_ppm) = 0;

      virtual int GetCOVREntries(){ return  m_nCOVREntries; }
      virtual int GetCOVRTablePoints(int iTable) { return m_pCOVRTablePoints[iTable]; }
      virtual int  GetCOVRTablenPoints(int iTable){ return m_pCOVRTablePoints[iTable]; }
      virtual float *GetCOVRTableHead(int iTable){ return m_pCOVRTable[iTable]; }

      virtual int GetNoCOVREntries(){ return  m_nNoCOVREntries; }
      virtual int GetNoCOVRTablePoints(int iTable) { return m_pNoCOVRTablePoints[iTable]; }
      virtual int  GetNoCOVRTablenPoints(int iTable){ return m_pNoCOVRTablePoints[iTable]; }
      virtual float *GetNoCOVRTableHead(int iTable){ return m_pNoCOVRTable[iTable]; }
      
protected:

      int               m_Chart_Scale;
      ChartTypeEnum     m_ChartType;
      ChartFamilyEnum   m_ChartFamily;

      QString          m_FullPath;
      QString          m_Name;
      QString          m_Description;
      QString          m_ID;
      QString          m_SE;
      QString          m_SoundingsDatum;
      QString          m_datum_str;
      QString          m_ExtraInfo;
      QString          m_PubYear;
      QString          m_DepthUnits;

      OcpnProjType      m_projection;
      ChartDepthUnitType m_depth_unit_id;

      QDateTime        m_EdDate;

      ThumbData         *pThumbData;

      ColorScheme       m_global_color_scheme;
      bool              bReadyToRender;

      double            Chart_Error_Factor;

      double            m_lon_datum_adjust;             // Add these numbers to WGS84 position to obtain internal chart position
      double            m_lat_datum_adjust;

      double            m_Chart_Skew;


      //    Chart region coverage information
      //    Charts may have multiple valid regions within the lat/lon box described by the chart extent
      //    The following table structure contains this embedded information

      //    Typically, BSB charts will contain only one entry, corresponding to the PLY information in the chart header
      //    ENC charts often contain multiple entries

      int         m_nCOVREntries;                       // number of coverage table entries
      int         *m_pCOVRTablePoints;                  // int table of number of points in each coverage table entry
      float       **m_pCOVRTable;                       // table of pointers to list of floats describing valid COVR

      int         m_nNoCOVREntries;                       // number of NoCoverage table entries
      int         *m_pNoCOVRTablePoints;                  // int table of number of points in each NoCoverage table entry
      float       **m_pNoCOVRTable;                       // table of pointers to list of floats describing valid NOCOVR

};


// ----------------------------------------------------------------------------
// ChartDummy
// ----------------------------------------------------------------------------

class ChartDummy : public ChartBase
{

public:
      ChartDummy();
      virtual ~ChartDummy();

      virtual InitReturn Init( const QString& name, ChartInitFlag init_flags );

//    Accessors
      virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
      virtual ThumbData *GetThumbData() {return pThumbData;}
      virtual bool UpdateThumbData(double lat, double lon);

      double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom){return 1.0;}
      double GetNormalScaleMax(double canvas_scale_factor, int canvas_width){ return 2.0e7;}

      virtual bool GetChartExtent(Extent *pext);

      virtual bool RenderRegionViewOnDC(QPainter* dc, const ViewPort& VPoint,
                                        const OCPNRegion &Region);

      virtual bool RenderRegionViewOnGL(const QGLContext &glc, const ViewPort& VPoint,
                                        const OCPNRegion &RectRegion, const LLRegion &Region);

      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);

      virtual void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion);
      virtual LLRegion GetValidRegion();

      virtual void SetColorScheme(ColorScheme cs, bool bApplyImmediate);

      virtual double GetNearestPreferredScalePPM(double target_scale_ppm){ return target_scale_ppm; }

private:
      bool RenderViewOnDC(QPainter* dc, const ViewPort& VPoint);

      QBitmap    *m_pBM;
};


#endif
