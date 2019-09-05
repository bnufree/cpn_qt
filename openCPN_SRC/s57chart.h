﻿/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S57 Chart Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#ifndef __S57CHART_H__
#define __S57CHART_H__

#include "bbox.h"
#include "chartbase.h"
#include "gdal/ogrsf_frmts.h"

#include "iso8211/iso8211.h"

#include "gdal/gdal.h"
#include "s57RegistrarMgr.h"
#include "S57ClassRegistrar.h"
#include "S57Light.h"
#include "S57Sector.h"
#include "s52s57.h"                 //types
#include "OCPNRegion.h"
#include "ocpndc.h"
#include "viewport.h"
#include "SencManager.h"
#include <memory>
#include <QGLContext>

class ChartCanvas;
// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// S57 Utility Prototypes
// ----------------------------------------------------------------------------
extern "C" bool s57_GetChartExtent(const QString& FullPath, Extent *pext);

void s57_DrawExtendedLightSectors( ocpnDC& temp_dc, ViewPort& VPoint, std::vector<s57Sector_t>& sectorlegs );
bool s57_CheckExtendedLightSectors( ChartCanvas *cc, int mx, int my, ViewPort& VPoint, std::vector<s57Sector_t>& sectorlegs );

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------

enum
{
      BUILD_SENC_OK,
      BUILD_SENC_NOK_RETRY,
      BUILD_SENC_NOK_PERMANENT,
      BUILD_SENC_PENDING
};

//----------------------------------------------------------------------------
// Fwd Defns
//----------------------------------------------------------------------------

class ChartBase;
class ViewPort;
class ocpnBitmap;
class PixelCache;
class S57ObjectDesc;
class S57Reader;
class OGRS57DataSource;
class S57ClassRegistrar;
class S57Obj;
class VE_Element;
class VC_Element;
class connector_segment;


// Declare the Array of S57Obj
typedef QList<S57Obj> ArrayOfS57Obj;

// And also a list
typedef QList<S57Obj> ListOfS57Obj;


typedef QList<ObjRazRules> ListOfObjRazRules;

//----------------------------------------------------------------------------
// s57 Chart object class
//----------------------------------------------------------------------------
class s57chart : public ChartBase
{
public:
      s57chart();
      ~s57chart();

      virtual InitReturn Init( const QString& name, ChartInitFlag flags );

//    Accessors

      virtual ThumbData *GetThumbData(int tnx, int tny, float lat, float lon);
      virtual ThumbData *GetThumbData() {return pThumbData;}
      bool UpdateThumbData(double lat, double lon);

      virtual int GetNativeScale(){return m_Chart_Scale;}
      virtual double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
      virtual double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);

      void SetNativeScale(int s){m_Chart_Scale = s;}

      virtual bool RenderRegionViewOnDC(QPainter* dc, const ViewPort& VPoint, const OCPNRegion &Region);
      virtual bool RenderOverlayRegionViewOnDC(QPainter* dc, const ViewPort& VPoint, const OCPNRegion &Region);

      virtual bool RenderRegionViewOnDCNoText(QPainter* dc, const ViewPort& VPoint, const OCPNRegion &Region);
      virtual bool RenderRegionViewOnDCTextOnly(QPainter* dc, const ViewPort& VPoint, const OCPNRegion &Region);
      
      virtual void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion);
      virtual LLRegion GetValidRegion();

      virtual void GetPointPix(ObjRazRules *rzRules, float rlat, float rlon, zchxPoint *r);
      virtual void GetPointPix(ObjRazRules *rzRules, zchxPointF *en, zchxPoint *r, int nPoints);
      virtual void GetPixPoint(int pixx, int pixy, double *plat, double *plon, ViewPort *vpt);

      virtual void SetVPParms(const ViewPort &vpt);

      virtual bool AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed);
//      virtual bool IsRenderDelta(ViewPort &vp_last, ViewPort &vp_proposed);

      virtual double GetNearestPreferredScalePPM(double target_scale_ppm){ return target_scale_ppm; }

      void SetFullExtent(Extent& ext);
      bool GetChartExtent(Extent *pext);

      void SetColorScheme(ColorScheme cs, bool bApplyImmediate = true);
      virtual void UpdateLUPs(s57chart *pOwner);

      int _insertRules(S57Obj *obj, LUPrec *LUP, s57chart *pOwner);

      virtual ListOfObjRazRules *GetObjRuleListAtLatLon(float lat, float lon, float select_radius, 
                                                        ViewPort *VPoint, int selection_mask = MASK_ALL);
      bool DoesLatLonSelectObject(float lat, float lon, float select_radius, S57Obj *obj);
      bool IsPointInObjArea(float lat, float lon, float select_radius, S57Obj *obj);
      QString GetObjectAttributeValueAsString( S57Obj *obj, int iatt, QString curAttrName );
      static QString GetAttributeValueAsString( S57attVal *pAttrVal, QString AttrName );
      static bool CompareLights( const S57Light* l1, const S57Light* l2 );
      QString CreateObjDescriptions( ListOfObjRazRules* rule);
      static QString GetAttributeDecode(QString& att, int ival);

      int BuildRAZFromSENCFile(const QString& SENCPath);
      static void GetChartNameFromTXT(const QString& FullPath, QString &Name);
      QString buildSENCName( const QString& name);
      
      //    DEPCNT VALDCO array access
      bool GetNearestSafeContour(double safe_cnt, double &next_safe_cnt);

      virtual ListOfS57Obj *GetAssociatedObjects(S57Obj *obj);

      virtual VE_Hash&  Get_ve_hash(void){ return m_ve_hash; }
      virtual VC_Hash&  Get_vc_hash(void){ return m_vc_hash; }

      virtual void ForceEdgePriorityEvaluate(void);

      float *GetLineVertexBuffer( void ){ return m_line_vertex_buffer; }
      
      void ClearRenderedTextCache();
      
      double GetCalculatedSafetyContour(void){ return m_next_safe_cnt; }

      virtual bool RenderRegionViewOnGL(QGLContext *glc, const ViewPort& VPoint,
                                        const OCPNRegion &RectRegion, const LLRegion &Region);
      virtual bool RenderOverlayRegionViewOnGL(QGLContext *glc, const ViewPort& VPoint,
                                               const OCPNRegion &RectRegion, const LLRegion &Region);
      virtual bool RenderRegionViewOnGLNoText(QGLContext *glc, const ViewPort& VPoint,
                                        const OCPNRegion &RectRegion, const LLRegion &Region);
      virtual bool RenderViewOnGLTextOnly(QGLContext *glc, const ViewPort& VPoint);
      
// Public data
//Todo Accessors here
      //  Object arrays used by S52PLIB TOPMAR rendering logic
      wxArrayPtrVoid *pFloatingATONArray;
      wxArrayPtrVoid *pRigidATONArray;

      double        ref_lat, ref_lon;             // Common reference point, derived from FullExtent
      double        m_LOD_meters;
      Extent        m_FullExtent;
      bool          m_bExtentSet;
      bool          m_bLinePrioritySet;

      //  SM Projection parms, stored as convenience to expedite pixel conversions
      double    m_easting_vp_center, m_northing_vp_center;
      double    m_pixx_vp_center, m_pixy_vp_center;
      double    m_view_scale_ppm;

      //    Last ViewPort succesfully rendered, stored as an aid to calculating pixel cache address offsets and regions
      ViewPort    m_last_vp;
      OCPNRegion    m_last_Region;

      virtual bool IsCacheValid(){ return (pDIB != nullptr); }
      virtual void InvalidateCache();
      virtual bool RenderViewOnDC(QPainter* dc, const ViewPort& VPoint);

      virtual void ClearDepthContourArray(void);
      virtual void BuildDepthContourArray(void);
      int ValidateAndCountUpdates( const QFileInfo& file000, const QString CopyDir,
                                   QString &LastUpdateDate, bool b_copyfiles);
      static int GetUpdateFileArray(const QFileInfo& file000, QStringList *UpFiles,
                                    QDateTime date000, QString edtn000 );
      QString GetISDT(void);
      InitReturn PostInit( ChartInitFlag flags, ColorScheme cs );

      char GetUsageChar(void){ return m_usage_char; }
      static bool IsCellOverlayType(char *pFullPath);

      bool        m_b2pointLUPS;
      bool        m_b2lineLUPS;
      bool        m_RAZBuilt;
      
      struct _chart_context     *m_this_chart_context;

      int FindOrCreateSenc( const QString& name, bool b_progress = true );
      void DisableBackgroundSENC(){ m_disableBackgroundSENC = true; }
      void EnableBackgroundSENC(){ m_disableBackgroundSENC = false; }
      
      SENCThreadStatus m_SENCthreadStatus;
protected:
      void AssembleLineGeometry( void );

      ObjRazRules *razRules[PRIO_NUM][LUPNAME_NUM];
    
private:
      int GetLineFeaturePointArray(S57Obj *obj, void **ret_array);
      void SetSafetyContour(void);
    
      bool DoRenderViewOnDC(QPainter* dc, const ViewPort& VPoint, RenderTypeEnum option, bool force_new_view);

      bool DoRenderRegionViewOnDC(QPainter* dc, const ViewPort& VPoint, const OCPNRegion &Region, bool b_overlay);

      int DCRenderRect(QPainter* dcinput, const ViewPort& vp, QRect *rect);
      bool DCRenderLPB(QPainter* dcinput, const ViewPort& vp, QRect* rect);
      bool DCRenderText(QPainter* dcinput, const ViewPort& vp);
      

      int BuildSENCFile(const QString& FullPath000, const QString& SENCFileName, bool b_progress = true);
      
      void SetLinePriorities(void);

      bool BuildThumbnail(const QString &bmpname);
      bool CreateHeaderDataFromENC(void);
      bool CreateHeaderDataFromSENC(void);
      bool CreateHeaderDataFromoSENC(void);
      bool GetBaseFileAttr( const QString& file000 );
      
      void ResetPointBBoxes(const ViewPort &vp_last, const ViewPort &vp_this);

           //    Access to raw ENC DataSet
      bool InitENCMinimal( const QString& FullPath );
      int GetENCScale();
      OGRFeature *GetChartFirstM_COVR(int &catcov);
      OGRFeature *GetChartNextM_COVR(int &catcov);

      void FreeObjectsAndRules();
      const char *getName(OGRFeature *feature);

      bool DoRenderOnGL(QGLContext *glc, const ViewPort& VPoint);
      bool DoRenderOnGLText(QGLContext *glc, const ViewPort& VPoint);
      bool DoRenderRegionViewOnGL(QGLContext *glc, const ViewPort& VPoint,
                                  const OCPNRegion &RectRegion, const LLRegion &Region, bool b_overlay);

      void BuildLineVBO( void );
      
      void ChangeThumbColor(ColorScheme cs);
      void LoadThumb();
      
 // Private Data
      char        *hdr_buf;
      char        *mybuf_ptr;
      int         hdr_len;
      QString    m_SENCFileName;


      QStringList *m_tmpup_array;
      std::unique_ptr<PixelCache> pDIB;

      std::unique_ptr<wxBitmap> m_pCloneBM;

      bool         bGLUWarningSent;

      wxBitmap    *m_pDIBThumbDay;
      wxBitmap    *m_pDIBThumbDim;
      wxBitmap    *m_pDIBThumbOrphan;
      bool        m_bneed_new_thumbnail;

      bool        m_bbase_file_attr_known;
      QDateTime  m_date000;                    // extracted from DSID:ISDT
      QString    m_edtn000;                    // extracted from DSID:EDTN
      int         m_nGeoRecords;                // extracted from DSSI:NOGR
      int         m_native_scale;               // extracted from DSPM:CSCL


//  Raw ENC DataSet members
      std::unique_ptr<OGRS57DataSource> m_pENCDS;

//  DEPCNT VALDCO array members
      int         m_nvaldco;
      int         m_nvaldco_alloc;
      double       *m_pvaldco_array;

      
      float      *m_line_vertex_buffer;
      size_t      m_vbo_byte_length;
      
      bool        m_blastS57TextRender;
      QString    m_lastColorScheme;
      QRect      m_last_vprect;
      long        m_plib_state_hash;
      bool        m_btex_mem;
      char        m_usage_char;
      
      double      m_next_safe_cnt;

      int         m_LineVBO_name;
      
      VE_Hash     m_ve_hash;
      VC_Hash     m_vc_hash;
      std::vector<connector_segment *> m_pcs_vector;
      std::vector<VE_Element *> m_pve_vector;
      
      QString    m_TempFilePath;
      bool        m_disableBackgroundSENC;
protected:      
      sm_parms    vp_transform;
      
};


#endif
