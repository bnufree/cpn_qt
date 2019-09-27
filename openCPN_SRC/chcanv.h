
/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Canvas
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

#ifndef __CHCANV_H__
#define __CHCANV_H__

#include "bbox.h"
#include "ocpndc.h"
#include "ocpCursor.h"
#include "gshhs.h"
#include <QTimer>
#include <QResizeEvent>
#include "bitmap.h"
#include <QWidget>
#include <QThread>



//    Set up the preferred quilt type
#define QUILT_TYPE_2

//----------------------------------------------------------------------------
//    Forward Declarations
//----------------------------------------------------------------------------

class ChartBase;
class Quilt;
class glChartCanvas;
class ChartStack;


//----------------------------------------------------------------------------
// ChartCanvas
//----------------------------------------------------------------------------
class ChartFrameWork : public QObject
{
    Q_OBJECT
     friend class glChartCanvas;
public:
      ChartFrameWork(glChartCanvas* parent);
      ~ChartFrameWork();
      glChartCanvas* getGL() {return mGLCC;}

public slots:
      void slotInitEcidsAsDelayed();
      void slotResize(int width, int height);
      void slotUpdateChartDatabase(ArrayOfCDI &DirArray, bool b_force, const QString &filename );

signals:
      void signalUpdateChartDatabase(ArrayOfCDI& cd, bool force_new, const QString& fileName);
      void signalUpdateChartArrayFinished();
      void signalResize(int width, int height);


public slots:

      
      double GetCanvasRangeMeters();
      void SetCanvasRangeMeters( double range );

//      void LostMouseCapture(wxMouseCaptureLostEvent& event);

      void SetDisplaySizeMM( double size );
      double GetDisplaySizeMM(){ return m_display_size_mm; }
      
      bool SetVPScale(double sc, bool b_refresh = true);
      bool SetViewPoint ( double lat, double lon);
      bool SetViewPointByCorners( double latSW, double lonSW, double latNE, double lonNE );
      bool SetViewPoint(double lat, double lon, double scale_ppm, double skew, double rotation,
                        int projection = 0, bool b_adjust = true, bool b_refresh = true);
      void ReloadVP ( bool b_adjust = true );
      void LoadVP ( ViewPort &vp, bool b_adjust = true );

      ChartStack *GetpCurrentStack(){ return m_pCurrentStack; }
      void SetGroupIndex( int index, bool autoswitch = false );
      bool CheckGroup( int igroup );
      void canvasRefreshGroupIndex( void );
      void canvasChartsRefresh( int dbi_hint );
      
      void CheckGroupValid( bool showMessage = true, bool switchGroup0 = true);





      void SetupCanvasQuiltMode( void );

      
      void SetVPRotation(double angle){ GetVP().setRotation(angle); }
      double GetVPRotation(void) { return GetVP().rotation(); }
      double GetVPSkew(void) { return GetVP().skew(); }
      double GetVPTilt(void) { return GetVP().tilt(); }
      void JumpToPosition( double lat, double lon, double scale );
      void SetFirstAuto( bool b_auto ){m_bFirstAuto = b_auto; }
      
      void GetDoubleCanvasPointPix(double rlat, double rlon, zchxPointF& r);
      void GetDoubleCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, zchxPointF& r );
      bool GetCanvasPointPix( double rlat, double rlon, zchxPoint& r );
      bool GetCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, zchxPoint &r );
      
      void GetCanvasPixPoint(double x, double y, double &lat, double &lon);
//      wxBitmap &GetTideBitmap(){ return m_cTideBitmap; }
      
      void UnlockQuilt();
      void SetQuiltMode(bool b_quilt);
      bool GetQuiltMode(void);
      std::vector<int> GetQuiltIndexArray(void);
      bool IsQuiltDelta(void);
      void SetQuiltChartHiLiteIndex(int dbIndex);
      int GetQuiltReferenceChartIndex(void);
      double GetBestStartScale(int dbi_hint, const ViewPort &vp);
      void StopMovement( );





      //    Accessors
      int GetCanvasWidth(){ return m_canvas_width;}
      int GetCanvasHeight(){ return m_canvas_height;}
      float GetVPScale(){return GetVP().viewScalePPM();}
      float GetVPChartScale(){return GetVP().chartScale();}
      double GetCanvasScaleFactor(){return m_canvas_scale_factor;}
      double GetCanvasTrueScale(){return m_true_scale_ppm;}
      double GetAbsoluteMinScalePpm(){ return m_absolute_min_scale_ppm; }
      ViewPort &GetVP();
      ViewPort *GetpVP(){ return &mViewPoint; }
      void SetVP(ViewPort &);
      ChartBase* GetChartAtPixel(int x, int y);
      ChartBase* GetOverlayChartAtPixel(int x, int y);
      


      double GetPixPerMM(){ return m_pix_per_mm;}



      bool Pan(double dx, double dy);
      void Zoom(double factor,  bool can_zoom_to_cursor = true);
      bool isZoomNow() const {return m_bzooming;}
      void Rotate(double rad);
      void RotateDegree(double deg);
      void RotateContinus( double dir );


      void DoTiltCanvas( double tilt );

      ChartBase *GetLargestScaleQuiltChart();
      ChartBase *GetFirstQuiltChart();
      ChartBase *GetNextQuiltChart();
      int GetQuiltChartCount();
      void InvalidateAllQuiltPatchs(void);
      void SetQuiltRefChart(int dbIndex);
      std::vector<int> GetQuiltCandidatedbIndexArray(bool flag1 = true, bool flag2 = true);
      std::vector<int> GetQuiltExtendedStackdbIndexArray();
      std::vector<int> GetQuiltEclipsedStackdbIndexArray();
      int GetQuiltRefChartdbIndex(void);
      void InvalidateQuilt(void);
      double GetQuiltMaxErrorFactor();
      bool IsChartQuiltableRef(int db_index);
      bool IsChartLargeEnoughToRender( ChartBase* chart, ViewPort& vp );
      int GetCanvasChartNativeScale();
      int FindClosestCanvasChartdbIndex(int scale);
      void UpdateCanvasOnGroupChange(void);

      void UpdateGPSCompassStatusBox( bool b_force_new );

      bool DoCanvasUpdate( void );
      void SelectQuiltRefdbChart( int db_index, bool b_autoscale = true );
      void SelectQuiltRefChart( int selected_index );
      double GetBestVPScale( ChartBase *pchart );
      void selectCanvasChartDisplay( int type, int family);
      void RemoveChartFromQuilt( int dbIndex );

public slots:

public:
      Quilt         *m_pQuilt;
      int           m_groupIndex;
      ChartBase     *m_singleChart;
      double        m_VPRotate;
      void JaggyCircle(ocpnDC &dc, QPen pen, int x, int y, int radius);
      
      bool CheckEdgePan( int x, int y, bool bdragging, int margin, int delta );
      void SelectChartFromStack(int index,  bool bDir = false,  ChartTypeEnum New_Type = CHART_TYPE_DONTCARE, ChartFamilyEnum New_Family = CHART_FAMILY_DONTCARE);
      void SelectdbChart( int dbindex );
      void DoCanvasStackDelta( int direction );
      void ToggleCanvasQuiltMode( void );
      QString GetScaleText(){ return m_scaleText; }
      int GetScaleValue(){ return m_scaleValue; }


private:
      int AdjustQuiltRefChart();
      ViewPort    mViewPoint;
      QString    m_scaleText;
      int         m_scaleValue;

      double       m_canvas_scale_factor;    // converter....
                                             // useage....
                                             // true_chart_scale_on_display = m_canvas_scale_factor / pixels_per_meter of displayed chart
                                             // also may be considered as the "pixels-per-meter" of the canvas on-screen
      double      m_pix_per_mm;     // pixels per millimeter on the screen
      double      m_display_size_mm;
      
      double      m_absolute_min_scale_ppm;





      wxBitmap CreateDimBitmap(wxBitmap &Bitmap, double factor);
      
      //    Data
      int         m_canvas_width, m_canvas_height;





      double      m_true_scale_ppm;




      
      ViewPort    m_cache_vp;



      bool        m_bzooming;
      double      m_displayed_scale_factor;
      double      m_zoom_factor;
      

      bool        m_disable_edge_pan;
      
      bool        m_bautofind;
      bool        m_bFirstAuto;
      double      m_vLat, m_vLon;
      ChartStack  *m_pCurrentStack;
      bool        m_bpersistent_quilt;
      int          m_restore_dbindex;

      float        m_focus_indicator_pix;
      bool         m_bENCGroup;
      glChartCanvas*  mGLCC;

      //线程化
      QThread           mWorkThread;




};

QString minutesToHoursDays(float timeInMinutes);


#endif
