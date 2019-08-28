
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
#include "timers.h"
#include "emboss_data.h"
#include "S57Sector.h"
#include "gshhs.h"
#include <QTimer>
#include <QPainter>.
#include <QResizeEvent>
#include "bitmap.h"
#include <QWidget>
#include <QMenu>



class GSHHSChart;
//class IDX_entry;
class ocpnCompass;

//    Useful static routines
//void ShowAISTargetQueryDialog(wxWindow *parent, int mmsi);

//--------------------------------------------------------
//    Screen Brightness Control Support Routines
//
//--------------------------------------------------------

int InitScreenBrightness(void);
int RestoreScreenBrightness(void);
int SetScreenBrightness(int brightness);


//    Set up the preferred quilt type
#define QUILT_TYPE_2

//----------------------------------------------------------------------------
//    Forward Declarations
//----------------------------------------------------------------------------
      class SelectItem;
      class wxBoundingBox;
      class ocpnBitmap;
      class WVSChart;
      class ChartBaseBSB;
      class ChartBase;
      class S57ObjectTree;
      class S57ObjectDesc;
      class RolloverWin;
      class Quilt;
      class PixelCache;
      class ChInfoWin;
      class glChartCanvas;
      class ChartStack;
      class Piano;
      class canvasConfig;
      class MUIBar;

      class zchxMapMainWindow;

enum                                //  specify the render behaviour of SetViewPoint()
{
    CURRENT_RENDER,                 // use the current render type
    FORCE_SUBSAMPLE                 // force sub-sampled render, with re-render timer
};

//          Cursor region enumerator
enum
{
      CENTER,
      MID_RIGHT,
      MID_LEFT,
      MID_TOP,
      MID_BOT,
};

typedef enum ownship_state_t
{
      SHIP_NORMAL        = 0,
      SHIP_LOWACCURACY,
      SHIP_INVALID
}_ownship_state_t;

enum {
      ID_S57QUERYTREECTRL =            10000,
      ID_AISDIALOGOK
};

enum {
    ID_PIANO_DISABLE_QUILT_CHART = 32000, ID_PIANO_ENABLE_QUILT_CHART
};


//----------------------------------------------------------------------------
// ChartCanvas
//----------------------------------------------------------------------------
class ChartCanvas: public QWidget
{
    Q_OBJECT
     friend class glChartCanvas;
public:
      ChartCanvas(QWidget *frame, int canvasIndex);
      ~ChartCanvas();

      void SetupGlCanvas( );
      void PaintCleanup();
      void Scroll(int dx, int dy);
      void SetAlertString( QString str){ m_alertString = str;}
      QString GetAlertString(){ return m_alertString; }

protected:
      //    Methods
      void keyPressEvent(QKeyEvent *event);
      void keyReleaseEvent(QKeyEvent *event);
      void paintEvent(QPaintEvent* event);
      void focusInEvent(QFocusEvent *){}
      void focusOutEvent(QFocusEvent *){}
      void resizeEvent(QResizeEvent * event );
public slots:
//      void OnToolLeftClick();
      bool MouseEventOverlayWindows( QMouseEvent* event );
      bool MouseEventChartBar( QMouseEvent* event );
      bool MouseEventSetup( QMouseEvent* event, bool b_handle_dclick = true );
      bool MouseEventProcessObjects( QMouseEvent* event );
      bool MouseEventProcessCanvas( QMouseEvent* event );
      void SetCanvasCursor( QMouseEvent* event );
      
//      void PopupMenuHandler(wxCommandEvent& event);
      bool IsPrimaryCanvas(){ return (m_canvasIndex == 0); }
      
      double GetCanvasRangeMeters();
      void SetCanvasRangeMeters( double range );
      
      void EnablePaint(bool b_enable);
      virtual bool SetCursor(const QCursor &c);
      virtual void Refresh( bool eraseBackground = true, const QRect *rect = (const QRect *) NULL );
      virtual void Update();

//      void LostMouseCapture(wxMouseCaptureLostEvent& event);
      
      void CancelMouseRoute();
      void SetDisplaySizeMM( double size );
      double GetDisplaySizeMM(){ return m_display_size_mm; }
      
      bool SetVPScale(double sc, bool b_refresh = true);
      bool SetVPProjection(int projection);
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

      void UpdateCanvasS52PLIBConfig();


      void ClearS52PLIBStateHash(){ m_s52StateHash = 0; }
      void SetupCanvasQuiltMode( void );
      void ApplyCanvasConfig(canvasConfig *pcc);
      
      void SetVPRotation(double angle){ VPoint.rotation = angle; }
      double GetVPRotation(void) { return GetVP().rotation; }
      double GetVPSkew(void) { return GetVP().skew; }
      double GetVPTilt(void) { return GetVP().tilt; }
      void ClearbFollow(void);
      void SetbFollow(void);
      void TogglebFollow( void );
      void JumpToPosition( double lat, double lon, double scale );
      void SetFirstAuto( bool b_auto ){m_bFirstAuto = b_auto; }
      
      void GetDoubleCanvasPointPix(double rlat, double rlon, zchxPointF& r);
      void GetDoubleCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, zchxPointF& r );
      bool GetCanvasPointPix( double rlat, double rlon, zchxPoint& r );
      bool GetCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, zchxPoint &r );
      
      void GetCanvasPixPoint(double x, double y, double &lat, double &lon);

      bool IsMeasureActive(){ return m_bMeasure_Active; }
//      wxBitmap &GetTideBitmap(){ return m_cTideBitmap; }
      
      void UnlockQuilt();
      void SetQuiltMode(bool b_quilt);
      bool GetQuiltMode(void);
      std::vector<int> GetQuiltIndexArray(void);
      bool IsQuiltDelta(void);
      void SetQuiltChartHiLiteIndex(int dbIndex);
      int GetQuiltReferenceChartIndex(void);
      double GetBestStartScale(int dbi_hint, const ViewPort &vp);
      void ConfigureChartBar();
      
//      int GetNextContextMenuId();
      bool StartTimedMovement( bool stoptimer=true );
      void DoTimedMovement( );
      void DoMovement( long dt );
      void StopMovement( );

      void SetColorScheme(ColorScheme cs);
      ColorScheme GetColorScheme(){ return m_cs;}

      void CanvasApplyLocale();

      //    Accessors
      int GetCanvasWidth(){ return m_canvas_width;}
      int GetCanvasHeight(){ return m_canvas_height;}
      float GetVPScale(){return GetVP().view_scale_ppm;}
      float GetVPChartScale(){return GetVP().chart_scale;}
      double GetCanvasScaleFactor(){return m_canvas_scale_factor;}
      double GetCanvasTrueScale(){return m_true_scale_ppm;}
      double GetAbsoluteMinScalePpm(){ return m_absolute_min_scale_ppm; }
      ViewPort &GetVP();
      ViewPort *GetpVP(){ return &VPoint; }
      void SetVP(ViewPort &);
      ChartBase* GetChartAtCursor();
      ChartBase* GetOverlayChartAtCursor();
      Piano *GetPiano(){ return m_Piano; }
      int GetPianoHeight();
      
      GSHHSChart* GetWorldBackgroundChart() { return pWorldBackgroundChart; }
      void ResetWorldBackgroundChart() { pWorldBackgroundChart->Reset(); }

      double GetPixPerMM(){ return m_pix_per_mm;}

      void SetCursorStatus( double cursor_lat, double cursor_lon );
      void GetCursorLatLon(double *lat, double *lon);

      bool PanCanvas(double dx, double dy);
//      void StopAutoPan(void);

      void ZoomCanvas(double factor, bool can_zoom_to_cursor=true, bool stoptimer=true );
      void DoZoomCanvas(double factor,  bool can_zoom_to_cursor = true);

      void RotateCanvas( double dir );
      void DoRotateCanvas( double rotation );
      void DoTiltCanvas( double tilt );

//      void ShowGoToPosition(void);
      void HideGlobalToolbar();
      void ShowGlobalToolbar();

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
      void ToggleCourseUp( );
      void ToggleLookahead( );
      void SetShowGPS( bool show );
 
      void ShowObjectQueryWindow( int x, int y, float zlat, float zlon);
      void UpdateGPSCompassStatusBox( bool b_force_new );
      ocpnCompass *GetCompass(){ return m_Compass; }
      
      QColor GetFogColor(){ return m_fog_color; }
      
      void ShowChartInfoWindow(int x, int dbIndex);
      void HideChartInfoWindow(void);
    
      void StartMeasureRoute();
      void CancelMeasureRoute();

      bool DoCanvasUpdate( void );
      void SelectQuiltRefdbChart( int db_index, bool b_autoscale = true );
      void SelectQuiltRefChart( int selected_index );
      double GetBestVPScale( ChartBase *pchart );
      void selectCanvasChartDisplay( int type, int family);
      void RemoveChartFromQuilt( int dbIndex );
      
      void HandlePianoClick( int selected_index, int selected_dbIndex );
      void HandlePianoRClick( int x, int y, int selected_index, int selected_dbIndex );
      void HandlePianoRollover( int selected_index, int selected_dbIndex );
      void UpdateCanvasControlBar( void );
      void FormatPianoKeys( void );
      void PianoPopupMenu ( int x, int y, int selected_index, int selected_dbIndex );

public slots:
      void OnPianoMenuDisableChart();
      void OnPianoMenuEnableChart();
public:
      bool IsPianoContextMenuActive(){ return m_piano_ctx_menu != 0; }
      void SetCanvasToolbarItemState( int tool_id, bool state );
      bool DoCanvasCOGSet( double cog );
      void UpdateFollowButtonState( void );
      void ApplyGlobalSettings();
      void SetShowGPSCompassWindow( bool bshow );

      
      //Todo build more accessors
      bool        m_bFollow;
      QCursor    *pCursorPencil;
      QCursor    *pCursorArrow;
      QCursor    *pCursorCross;
      QCursor    *pPlugIn_Cursor;
      wxBitmap    *pscratch_bm;
      bool        m_brepaint_piano;
      double      m_cursor_lon, m_cursor_lat;
//      Undo        *undo;
      QPoint     r_rband;
      double      m_prev_rlat;
      double      m_prev_rlon;
      Quilt       *m_pQuilt;
      bool        m_bShowNavobjects;
      int         m_canvasIndex;
      int         m_groupIndex;
      int          m_routeState;
      ChartBase   *m_singleChart;
      bool        m_bCourseUp;
      bool        m_bLookAhead;
      double      m_VPRotate;

      
      void InvalidateGL();
      
#ifdef ocpnUSE_GL
      glChartCanvas *GetglCanvas(){ return m_glcc; }
#endif      

      void JaggyCircle(ocpnDC &dc, QPen pen, int x, int y, int radius);
      
      bool CheckEdgePan( int x, int y, bool bdragging, int margin, int delta );
      bool        m_FinishRouteOnKillFocus;
      bool        m_bMeasure_Active;
      bool        m_bMeasure_DistCircle;
      QString    m_active_upload_port;
      bool        m_bAppendingRoute;
      int         m_nMeasureState;
      zchxMapMainWindow     *parent_frame;
//      CanvasMenuHandler  *m_canvasMenu;
      int GetMinAvailableGshhgQuality() { return pWorldBackgroundChart->GetMinAvailableQuality(); }
      int GetMaxAvailableGshhgQuality() { return pWorldBackgroundChart->GetMaxAvailableQuality(); }

      
      void SelectChartFromStack(int index,  bool bDir = false,  ChartTypeEnum New_Type = CHART_TYPE_DONTCARE, ChartFamilyEnum New_Family = CHART_FAMILY_DONTCARE);
      void SelectdbChart( int dbindex );

      
      void DoCanvasStackDelta( int direction );

      void ProcessNewGUIScale();
      
      bool GetShowDepthUnits(){ return m_bShowDepthUnits; }
      void SetShowDepthUnits( bool show ){ m_bShowDepthUnits = show; }
      bool GetShowGrid(){ return m_bDisplayGrid; }
      void SetShowGrid( bool show ){ m_bDisplayGrid = show; }
      bool GetShowOutlines(){ return m_bShowOutlines; }
      void SetShowOutlines( bool show ){ m_bShowOutlines = show; }
      bool GetShowChartbar(){ return true; }
      
      
      void ToggleChartOutlines(void);
      void ToggleCanvasQuiltMode( void );
      
      QString GetScaleText(){ return m_scaleText; }
      int GetScaleValue(){ return m_scaleValue; }
      
      bool        m_b_paint_enable;

      bool GetShowENCText(){ return m_encShowText; }
      void SetShowENCText( bool show );
      
      bool GetShowENCDepth(){ return m_encShowDepth; }
      void SetShowENCDepth( bool show );
      
      bool GetShowENCLightDesc(){ return m_encShowLightDesc; }
      void SetShowENCLightDesc( bool show );
      
      bool GetShowENCBuoyLabels(){ return m_encShowBuoyLabels; }
      void SetShowENCBuoyLabels( bool show );

      bool GetShowENCLights(){ return m_encShowLights; }
      void SetShowENCLights( bool show );
      
      int GetENCDisplayCategory(){ return m_encDisplayCategory; }
      void SetENCDisplayCategory( int category );

      bool GetShowENCAnchor(){ return m_encShowAnchor; }
      void SetShowENCAnchor( bool show );

      bool GetShowENCDataQual(){ return m_encShowDataQual; }
      void SetShowENCDataQual( bool show );
      
      bool GetCourseUP(){ return m_bCourseUp; }
      bool GetLookahead(){ return m_bLookAhead; }

      
      QRect GetScaleBarRect(){ return m_scaleBarRect; }
      void RenderAlertMessage(QPainter* dc, const ViewPort &vp);

private:
      int AdjustQuiltRefChart();

      bool UpdateS52State();
      
      void CallPopupMenu( int x, int y );
      
      bool IsTempMenuBarEnabled();
      bool InvokeCanvasMenu(int x, int y, int seltype);
      
      ViewPort    VPoint;
      void        PositionConsole(void);
      
      QColor PredColor();
      QColor ShipColor();

//      void ComputeShipScaleFactor(float icon_hdt,
//                                  int ownShipWidth, int ownShipLength,
//                                  QPoint &lShipMidPoint,
//                                  QPoint &GpsOffsetPixels, QPoint lGPSPoint,
//                                  float &scale_factor_x, float &scale_factor_y);

//      void ShipDrawLargeScale( ocpnDC& dc, QPoint lShipMidPoint );
//      void ShipIndicatorsDraw( ocpnDC& dc, int img_height,
//                               QPoint GPSOffsetPixels, QPoint lGPSPoint);
                               
      ChInfoWin   *m_pCIWin;

      int         cursor_region;
      QString    m_scaleText;
      int         m_scaleValue;
      bool        m_bShowScaleInStatusBar;
      QRect      bbRect;

      QPoint     LastShipPoint;
      QPoint     LastPredPoint;
      bool        m_bDrawingRoute;
      bool        m_bRouteEditing;
      bool        m_bMarkEditing;
	  bool		  m_bRoutePoinDragging;
      bool        m_bIsInRadius;
      bool        m_bMayToggleMenuBar;

      SelectItem  *m_pFoundPoint;
      bool        m_bChartDragging;

      int         m_FoundAIS_MMSI;

      QCursor    *pCursorLeft;
      QCursor    *pCursorRight;
      QCursor    *pCursorUp;
      QCursor    *pCursorDown;

      QCursor    *pCursorUpLeft;
      QCursor    *pCursorUpRight;
      QCursor    *pCursorDownLeft;
      QCursor    *pCursorDownRight;

      int         popx, popy;

      wxBitmap    *pThumbDIBShow;
      wxBitmap    *pThumbShowing;


      double       m_canvas_scale_factor;    // converter....
                                             // useage....
                                             // true_chart_scale_on_display = m_canvas_scale_factor / pixels_per_meter of displayed chart
                                             // also may be considered as the "pixels-per-meter" of the canvas on-screen
      double      m_pix_per_mm;     // pixels per millimeter on the screen
      double      m_display_size_mm;
      
      double      m_absolute_min_scale_ppm;

      bool singleClickEventIsValid;
      QMouseEvent* singleClickEvent;

      std::vector<s57Sector_t> extendedSectorLegs;
      QFont m_overzoomFont;
      int m_overzoomTextWidth;
      int m_overzoomTextHeight;

      //    Methods
      void OnActivate(/*wxActivateEvent& event*/);
      void OnSize(QResizeEvent& event);
      void MouseTimedEvent(QTimerEvent& event);
      void MouseEvent(QMouseEvent& event);
//      void ShipDraw(ocpnDC& dc);
      void DrawArrow(ocpnDC& dc, int x, int y, double rot_angle, double scale);
      void RotateTimerEvent(QTimerEvent& event);
      void PanTimerEvent(QTimerEvent& event);
      void MovementTimerEvent(QTimerEvent& );
      void MovementStopTimerEvent( QTimerEvent& );
      void OnCursorTrackTimerEvent(QTimerEvent& event);



      void RenderAllChartOutlines(ocpnDC &dc, ViewPort& vp);
      void RenderChartOutline(ocpnDC &dc, int dbIndex, ViewPort& vp);

      void GridDraw(ocpnDC& dc); // Display lat/lon Grid in chart display
      void ScaleBarDraw( ocpnDC& dc );


      emboss_data *EmbossDepthScale();
      emboss_data *CreateEmbossMapData(QFont &font, int width, int height, const QString &str, ColorScheme cs);
      void CreateDepthUnitEmbossMaps(ColorScheme cs);
      wxBitmap CreateDimBitmap(wxBitmap &Bitmap, double factor);

      void CreateOZEmbossMapData(ColorScheme cs);
      emboss_data *EmbossOverzoomIndicator ( ocpnDC &dc);
      void SetOverzoomFont();

//      void CreateCM93OffsetEmbossMapData(ColorScheme cs);
//      void EmbossCM93Offset ( QPainter *pdc);

      void DrawEmboss ( ocpnDC &dc, emboss_data *pemboss );

 
      void ShowBrightnessLevelTimedPopup( int brightness, int min, int max );
      
      //    Data
      int         m_canvas_width, m_canvas_height;

      int         xr_margin;                          // chart scroll margins, control cursor, etc.
      int         xl_margin;
      int         yt_margin;
      int         yb_margin;


      QPoint     last_drag;

      QPainter  *pmemdc;


//      QTimer     *pPanTimer;       // This timer used for auto panning on route creation and edit
//      QTimer     *pMovementTimer;       // This timer used for smooth movement in non-opengl mode
//      QTimer     *pMovementStopTimer; // This timer used to stop movement if a keyup event is lost
//      QTimer     *pCurTrackTimer;  // This timer used to update the status window on mouse idle
//      QTimer     *pRotDefTimer;    // This timer used to control rotaion rendering on mouse moves
//      QTimer     *m_DoubleClickTimer;
//      QTimer      m_routeFinishTimer;
      
//      QTimer     m_RolloverPopupTimer;

      int         m_wheelzoom_stop_oneshot;
      int         m_last_wheel_dir;
      QTime       m_wheelstopwatch;
      double      m_zoom_target;
      
      int         m_curtrack_timer_msec;
      int         m_rollover_popup_timer_msec;

      GSHHSChart  *pWorldBackgroundChart;

      ChartBaseBSB *pCBSB;
      wxBitmap    *pss_overlay_bmp;
      wxBitmap      *pss_overlay_mask;

      QRect      ship_draw_rect;
      QRect      ship_draw_last_rect;
      QRect      ais_draw_rect;
      QRect      alert_draw_rect;          // pjotrc 2010.02.22

      wxBitmap    *proute_bm;          // a bitmap and dc used to calculate route bounding box


      emboss_data *m_pEM_Feet;                // maps for depth unit emboss pattern
      emboss_data *m_pEM_Meters;
      emboss_data *m_pEM_Fathoms;

      emboss_data *m_pEM_OverZoom;
//      emboss_data *m_pEM_CM93Offset;	// Flav



      double      m_true_scale_ppm;


      ColorScheme m_cs;

      
      ViewPort    m_cache_vp;
      wxBitmap    *m_prot_bm;
      QPoint     m_roffset;

      bool        m_b_rot_hidef;

      double      m_wheel_lat, m_wheel_lon;
      int         m_wheel_x,m_wheel_y;

      ViewPort    m_bm_cache_vp;
      wxBitmap    m_working_bm;           // Used to build quilt in OnPaint()
      wxBitmap    m_cached_chart_bm;      // A cached copy of the fully drawn quilt

      bool        m_bbrightdir;
      int         m_brightmod;

      bool        m_bzooming, m_bzooming_to_cursor;
//      IDX_entry   *m_pIDXCandidate;
      glChartCanvas *m_glcc;


      //Smooth movement member variables
      zchxPoint      m_pan_drag;
      int         m_panx, m_pany, m_modkeys;
      double      m_panspeed;
      bool        m_bmouse_key_mod;
      double      m_zoom_factor, m_rotation_speed;
      int         m_mustmove;


      QDateTime m_last_movement_time;


      bool        m_bsectors_shown;
      bool        m_bedge_pan;
      double      m_displayed_scale_factor;
      
      QColor    m_fog_color;
      bool        m_disable_edge_pan;
      QFont      m_pgridFont;
      
      bool        m_dragoffsetSet;
      
      bool        m_bautofind;
      bool        m_bFirstAuto;
      double      m_vLat, m_vLon;
      ChartStack  *m_pCurrentStack;
      Piano       *m_Piano;
      bool        m_bpersistent_quilt;
      
      QMenu      *m_piano_ctx_menu;
      int         menu_selected_dbIndex, menu_selected_index;
      
      ocpnCompass *m_Compass;
      bool         m_bShowGPS;
      
      QRect       m_mainlast_tb_rect;
      int          m_restore_dbindex;
      int          m_restore_group;
      
      bool         m_bShowOutlines;
      bool         m_bDisplayGrid;
      bool         m_bShowDepthUnits;
      
      // S52PLib state storage
      long         m_s52StateHash;
      bool         m_encShowText;
      bool         m_encShowDepth;
      bool         m_encShowLightDesc;
      bool         m_encShowBuoyLabels;
      int          m_encDisplayCategory;
      bool         m_encShowLights;
      bool         m_encShowAnchor;
      bool         m_encShowDataQual;
      
      QTimer      m_deferredFocusTimer;
      float        m_focus_indicator_pix;
      bool         m_bENCGroup;
      bool         m_last_TBviz;
      
      double       m_OSoffsetx, m_OSoffsety;
      bool         m_MouseDragging;
      QRect       m_scaleBarRect;
      bool         m_bShowCompassWin;
      QString      m_alertString;
};

//typedef QList<ChartCanvas*> arrayofCanvasPtr;

// CUSTOMIZATION - FORMAT MINUTES

QString minutesToHoursDays(float timeInMinutes);

// END OF CUSTOMIZATION - FORMAT MINUTES

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif


#endif
