
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
#include "dychart.h"
#include "OCPNRegion.h"
#include "LLRegion.h"
#include "viewport.h"
#include "TexFont.h"
#include <QGLWidget>

 #define FORMAT_BITS           GL_RGB


class glTexFactory;
class GSHHSChart;
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
class canvasConfig;
class zchxMapMainWindow;

int InitScreenBrightness(void);
int RestoreScreenBrightness(void);
int SetScreenBrightness(int brightness);
QString minutesToHoursDays(float timeInMinutes);
extern void BuildCompressedCache();


//    Set up the preferred quilt type
#define QUILT_TYPE_2

//----------------------------------------------------------------------------
//    Forward Declarations
//----------------------------------------------------------------------------


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

typedef class{
  public:
    QString Renderer;
    GLenum TextureRectangleFormat;

    bool bOldIntel;
    bool bCanDoVBO;
    bool bCanDoFBO;

    //      Vertex Buffer Object (VBO) support
    PFNGLGENBUFFERSPROC                 m_glGenBuffers;
    PFNGLBINDBUFFERPROC                 m_glBindBuffer;
    PFNGLBUFFERDATAPROC                 m_glBufferData;
    PFNGLDELETEBUFFERSPROC              m_glDeleteBuffers;

    //      Frame Buffer Object (FBO) support
    PFNGLGENFRAMEBUFFERSEXTPROC         m_glGenFramebuffers;
    PFNGLGENRENDERBUFFERSEXTPROC        m_glGenRenderbuffers;
    PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    m_glFramebufferTexture2D;
    PFNGLBINDFRAMEBUFFEREXTPROC         m_glBindFramebuffer;
    PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC m_glFramebufferRenderbuffer;
    PFNGLRENDERBUFFERSTORAGEEXTPROC     m_glRenderbufferStorage;
    PFNGLBINDRENDERBUFFEREXTPROC        m_glBindRenderbuffer;
    PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  m_glCheckFramebufferStatus;
    PFNGLDELETEFRAMEBUFFERSEXTPROC      m_glDeleteFramebuffers;
    PFNGLDELETERENDERBUFFERSEXTPROC     m_glDeleteRenderbuffers;

    PFNGLCOMPRESSEDTEXIMAGE2DPROC       m_glCompressedTexImage2D;
    PFNGLGETCOMPRESSEDTEXIMAGEPROC      m_glGetCompressedTexImage;


}OCPN_GLCaps;

void GetglEntryPoints( OCPN_GLCaps *pcaps );
GLboolean QueryExtension( const char *extName );

class ocpnDC;
class emboss_data;
class Route;
class ChartBaseBSB;
class ChartBase;


//----------------------------------------------------------------------------
// ChartCanvas
//----------------------------------------------------------------------------
class ChartCanvas: public QGLWidget
{
    Q_OBJECT
public:
      ChartCanvas(QWidget *frame);
      ~ChartCanvas();
      void              SetAlertString( QString str){ m_alertString = str;}
      QString           GetAlertString(){ return m_alertString; }
      bool              UpdateChartDatabaseInplace( ArrayOfCDI &DirArray, bool b_force, bool b_prog, const QString &ChartListFileName );
      double            GetCanvasRangeMeters();
      void              SetCanvasRangeMeters( double range );
      void              EnablePaint(bool b_enable);
      virtual bool      SetCursor(const QCursor &c);
      virtual void      Refresh( bool eraseBackground = true, const QRect *rect = (const QRect *) NULL );
      void              SetDisplaySizeMM( double size );
      double            GetDisplaySizeMM(){ return m_display_size_mm; }
      bool              SetVPScale(double sc, bool b_refresh = true);
      bool              SetViewPoint ( double lat, double lon);
      bool              SetViewPointByCorners( double latSW, double lonSW, double latNE, double lonNE );
      bool              SetViewPoint(double lat, double lon, double scale_ppm, double skew, double rotation, int projection = 0, bool b_adjust = true, bool b_refresh = true);
      void              ReloadVP ( bool b_adjust = true );
      void              LoadVP ( ViewPort &vp, bool b_adjust = true );
      ChartStack*       GetpCurrentStack(){ return m_pCurrentStack; }
      void              SetGroupIndex( int index, bool autoswitch = false );
      bool              CheckGroup( int igroup );
      void              canvasRefreshGroupIndex( void );
      void              canvasChartsRefresh( int dbi_hint );
      void              CheckGroupValid( bool showMessage = true, bool switchGroup0 = true);
      void              UpdateCanvasS52PLIBConfig();
      void              ClearS52PLIBStateHash(){ m_s52StateHash = 0; }
      void              SetupCanvasQuiltMode( void );
      void              ApplyCanvasConfig(canvasConfig *pcc);
      void              SetVPRotation(double angle){ GetVP().setRotation(angle); }
      double            GetVPRotation(void) { return GetVP().rotation(); }
      double            GetVPSkew(void) { return GetVP().skew(); }
      double            GetVPTilt(void) { return GetVP().tilt(); }
      void              JumpToPosition( double lat, double lon, double scale );
      void              SetFirstAuto( bool b_auto ){m_bFirstAuto = b_auto; }
      void              GetDoubleCanvasPointPix(double rlat, double rlon, zchxPointF& r);
      void              GetDoubleCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, zchxPointF& r );
      bool              GetCanvasPointPix( double rlat, double rlon, zchxPoint& r );
      bool              GetCanvasPointPixVP( ViewPort &vp, double rlat, double rlon, zchxPoint &r );
      void              GetCanvasPixPoint(double x, double y, double &lat, double &lon);
      void              UnlockQuilt();
      void              SetQuiltMode(bool b_quilt);
      bool              GetQuiltMode(void);
      std::vector<int>  GetQuiltIndexArray(void);
      bool              IsQuiltDelta(void);
      void              SetQuiltChartHiLiteIndex(int dbIndex);
      int               GetQuiltReferenceChartIndex(void);
      double            GetBestStartScale(int dbi_hint, const ViewPort &vp);
      void              StopMovement( );
      void              SetColorScheme(ColorScheme cs);
      ColorScheme       GetColorScheme(){ return m_cs;}
      void              CanvasApplyLocale();
      int               GetCanvasWidth(){ return m_canvas_width;}
      int               GetCanvasHeight(){ return m_canvas_height;}
      float             GetVPScale(){return GetVP().viewScalePPM();}
      float             GetVPChartScale(){return GetVP().chartScale();}
      double            GetCanvasScaleFactor(){return m_canvas_scale_factor;}
      double            GetCanvasTrueScale(){return m_true_scale_ppm;}
      double            GetAbsoluteMinScalePpm(){ return m_absolute_min_scale_ppm; }
      ViewPort&         GetVP();
      ViewPort*         GetpVP(){ return &VPoint; }
      void              SetVP(ViewPort &);
      ChartBase*        GetChartAtCursor();
      ChartBase*        GetOverlayChartAtCursor();
      GSHHSChart*       GetWorldBackgroundChart() { return pWorldBackgroundChart; }
      void              ResetWorldBackgroundChart() { pWorldBackgroundChart->Reset(); }
      double            GetPixPerMM(){ return m_pix_per_mm;}
      void              GetCursorLatLon(double *lat, double *lon);
      bool              PanCanvas(double dx, double dy);
      void              ZoomCanvas(double factor, bool can_zoom_to_cursor=true, bool stoptimer=true );
      void              DoZoomCanvas(double factor,  bool can_zoom_to_cursor = true);
      void              RotateCanvas( double dir );
      void              DoRotateCanvas( double rotation );
      void              DoRotateCanvasWithDegree(double rotate);
      void              DoTiltCanvas( double tilt );
      ChartBase*        GetLargestScaleQuiltChart();
      ChartBase*        GetFirstQuiltChart();
      ChartBase*        GetNextQuiltChart();
      int               GetQuiltChartCount();
      void              InvalidateAllQuiltPatchs(void);
      void              SetQuiltRefChart(int dbIndex);
      std::vector<int>  GetQuiltCandidatedbIndexArray(bool flag1 = true, bool flag2 = true);
      std::vector<int>  GetQuiltExtendedStackdbIndexArray();
      std::vector<int>  GetQuiltEclipsedStackdbIndexArray();
      int               GetQuiltRefChartdbIndex(void);
      void              InvalidateQuilt(void);
      double            GetQuiltMaxErrorFactor();
      bool              IsChartQuiltableRef(int db_index);
      bool              IsChartLargeEnoughToRender( ChartBase* chart, ViewPort& vp );
      int               GetCanvasChartNativeScale();
      int               FindClosestCanvasChartdbIndex(int scale);
      void              UpdateCanvasOnGroupChange(void);
      void              UpdateGPSCompassStatusBox( bool b_force_new );
      bool              DoCanvasUpdate( void );
      void              SelectQuiltRefdbChart( int db_index, bool b_autoscale = true );
      void              SelectQuiltRefChart( int selected_index );
      double            GetBestVPScale( ChartBase *pchart );
      void              selectCanvasChartDisplay( int type, int family);
      void              RemoveChartFromQuilt( int dbIndex );
      void              setUpdateAvailable(bool sts) {mIsUpdateAvailable = sts;}
      static bool       CanClipViewport(const ViewPort &vp);
      static ViewPort   ClippedViewport(const ViewPort &vp, const LLRegion &region);

      static bool       HasNormalizedViewPort(const ViewPort &vp);
      static void       MultMatrixViewPort(ViewPort &vp, float lat=0, float lon=0);
      static ViewPort   NormalizedViewPort(const ViewPort &vp, float lat=0, float lon=0);

      static void       RotateToViewPort(const ViewPort &vp);
      static void       DrawRegion( ViewPort &vp, const LLRegion &region);
      static void       SetClipRegion( ViewPort &vp, const LLRegion &region);
      static void       SetClipRect(const ViewPort &vp, const QRect &rect, bool g_clear=false);
      static void       DisableClipRegion();
      void              Render();
      void              FastPan(int dx, int dy);
      void              FastZoom(float factor);
      void              RenderCanvasBackingChart( ocpnDC &dc, OCPNRegion chart_get_region);
      QString           GetRendererString(){ return m_renderer; }
      QString           GetVersionString(){ return m_version; }
//      void              EnablePaint(bool b_enable){ m_b_paint_enable = b_enable; }
      void              Invalidate();
      void              RenderRasterChartRegionGL(ChartBase *chart, ViewPort &vp, LLRegion &region);
      void              GridDraw( );
      void              FlushFBO( void );
//      void              RenderAllChartOutlines( ocpnDC &dc, ViewPort &VP );
      void              RenderChartOutline( int dbIndex, ViewPort &VP );
      void              DrawEmboss( emboss_data *emboss );
      void              SetupCompression();
      bool              CanAcceleratePanning() { return m_b_BuiltFBO; }
      bool              UsingFBO() { return m_b_BuiltFBO; }
      QColor            GetBackGroundColor() const;
      void              InvalidateGL();
      void              JaggyCircle(ocpnDC &dc, QPen pen, int x, int y, int radius);
      bool              CheckEdgePan( int x, int y, bool bdragging, int margin, int delta );
      int               GetMinAvailableGshhgQuality() { return pWorldBackgroundChart->GetMinAvailableQuality(); }
      int               GetMaxAvailableGshhgQuality() { return pWorldBackgroundChart->GetMaxAvailableQuality(); }
      void              SelectChartFromStack(int index,  bool bDir = false,  ChartTypeEnum New_Type = CHART_TYPE_DONTCARE, ChartFamilyEnum New_Family = CHART_FAMILY_DONTCARE);
      void              SelectdbChart( int dbindex );
      void              DoCanvasStackDelta( int direction );
      bool              GetShowDepthUnits(){ return m_bShowDepthUnits; }
      void              SetShowDepthUnits( bool show ){ m_bShowDepthUnits = show; }
      bool              GetShowGrid(){ return m_bDisplayGrid; }
      void              SetShowGrid( bool show ){ m_bDisplayGrid = show; }
      bool              GetShowOutlines(){ return m_bShowOutlines; }
      void              SetShowOutlines( bool show ){ m_bShowOutlines = show; }
      bool              GetShowChartbar(){ return true; }
      void              ToggleChartOutlines(void);
      void              ToggleCanvasQuiltMode( void );
      QString           GetScaleText(){ return m_scaleText; }
      int               GetScaleValue(){ return m_scaleValue; }
      bool              GetShowENCText(){ return m_encShowText; }
      void              SetShowENCText( bool show );
      bool              GetShowENCDepth(){ return m_encShowDepth; }
      void              SetShowENCDepth( bool show );
      bool              GetShowENCLightDesc(){ return m_encShowLightDesc; }
      void              SetShowENCLightDesc( bool show );
      bool              GetShowENCBuoyLabels(){ return m_encShowBuoyLabels; }
      void              SetShowENCBuoyLabels( bool show );
      bool              GetShowENCLights(){ return m_encShowLights; }
      void              SetShowENCLights( bool show );
      int               GetENCDisplayCategory(){ return m_encDisplayCategory; }
      void              SetENCDisplayCategory( int category );
      bool              GetShowENCAnchor(){ return m_encShowAnchor; }
      void              SetShowENCAnchor( bool show );
      bool              GetShowENCDataQual(){ return m_encShowDataQual; }
      void              SetShowENCDataQual( bool show );
      QRect             GetScaleBarRect(){ return m_scaleBarRect; }
      void              RenderAlertMessage(QPainter* dc, const ViewPort &vp);


private slots:
      void              buildStyle();
      void              initBeforeUpdateMap();
      void              slotInitEcidsAsDelayed();

protected:
      void              RenderQuiltViewGL( ViewPort &vp, const OCPNRegion &rect_region );
      void              RenderQuiltViewGLText( ViewPort &vp, const OCPNRegion &rect_region );
      void              RenderGLAlertMessage();
      void              BuildFBO();
      void              SetupOpenGL();
      void              RenderCharts(ocpnDC &dc, const OCPNRegion &rect_region);
      void              RenderNoDTA(ViewPort &vp, const LLRegion &region, int transparency = 255);
      void              RenderNoDTA(ViewPort &vp, ChartBase *chart);
      void              RenderWorldChart(ocpnDC &dc, ViewPort &vp, QRect &rect, bool &world_view);
      void              DrawFloatingOverlayObjects( ocpnDC &dc );
      void              DrawGroundedOverlayObjects(ocpnDC &dc, ViewPort &vp);
      void              DrawQuiting();
      void              DrawCloseMessage(QString msg);
      void              keyPressEvent(QKeyEvent *event);
//      void              paintEvent(QPaintEvent* event);
      void              resizeEvent(QResizeEvent * event );
      void              mousePressEvent(QMouseEvent *e);
      void              mouseMoveEvent(QMouseEvent *e);
      void              mouseReleaseEvent(QMouseEvent* e);
      void              wheelEvent(QWheelEvent *);
      void              paintGL();
      void              resizeGL(int w, int h);
      void              initializeGL();

public slots:
      void              startUpdate();
      void              stopUpdate();
      void              slotStartLoadEcdis();
private:
      int               AdjustQuiltRefChart();
      bool              UpdateS52State();
      void              DrawArrow(ocpnDC& dc, int x, int y, double rot_angle, double scale);
      void              RenderAllChartOutlines(ocpnDC &dc, ViewPort& vp);
      void              RenderChartOutline(ocpnDC &dc, int dbIndex, ViewPort& vp);
      void              GridDraw(ocpnDC& dc); // Display lat/lon Grid in chart display
      void              ScaleBarDraw( ocpnDC& dc );
      emboss_data*      EmbossDepthScale();
      emboss_data*      CreateEmbossMapData(QFont &font, int width, int height, const QString &str, ColorScheme cs);
      void              CreateDepthUnitEmbossMaps(ColorScheme cs);
      wxBitmap          CreateDimBitmap(wxBitmap &Bitmap, double factor);
      void              CreateOZEmbossMapData(ColorScheme cs);
      emboss_data*      EmbossOverzoomIndicator ( ocpnDC &dc);
      void              SetOverzoomFont();
      void              DrawEmboss ( ocpnDC &dc, emboss_data *pemboss );
      void              ShowBrightnessLevelTimedPopup( int brightness, int min, int max );

public:
      static bool         s_b_useScissorTest;
      static bool         s_b_useStencil;
      static bool         s_b_useStencilAP;
      static bool         s_b_useFBO;
      int               viewport[4];
      double            mvmatrix[16], projmatrix[16];
      bool        m_b_paint_enable;
      QCursor    *pCursorPencil;
      QCursor    *pCursorArrow;
      QCursor    *pCursorCross;
      wxBitmap    *pscratch_bm;
      double      m_cursor_lon, m_cursor_lat;
      Quilt       *m_pQuilt;
      bool        m_bShowNavobjects;
      int         m_groupIndex;
      ChartBase   *m_singleChart;
      double      m_VPRotate;
      zchxMapMainWindow     *parent_frame;
      QGLContext       *m_pcontext;

      int max_texture_dimension;

      bool m_bsetup;

      QString m_renderer;
      QString m_version;
      QString m_extensions;

      ViewPort    m_cache_vp;
      ChartBase   *m_cache_current_ch;

//      bool        m_b_paint_enable;
      int         m_in_glpaint;

      //    For FBO(s)
      bool         m_b_DisableFBO;
      bool         m_b_BuiltFBO;
      bool         m_b_useFBOStencil;
      GLuint       m_fb0;
      GLuint       m_renderbuffer;

      GLuint       m_cache_tex[2];
      GLuint       m_cache_page;
      int          m_cache_tex_x;
      int          m_cache_tex_y;

      float       m_fbo_offsetx;
      float       m_fbo_offsety;
      float       m_fbo_swidth;
      float       m_fbo_sheight;
      bool        m_binPinch;
      bool        m_binPan;
      bool        m_bfogit;
      bool        m_benableFog;
      bool        m_benableVScale;
      bool        m_bgestureGuard;
      bool        m_bpinchGuard;

      OCPNRegion  m_canvasregion;
      TexFont     m_gridfont;

      int		m_LRUtime;
      bool            mIsUpdateAvailable;


      
      ViewPort    VPoint;
      int         cursor_region;
      QString    m_scaleText;
      int         m_scaleValue;
      bool        m_bShowScaleInStatusBar;
      QRect      bbRect;


      QCursor    *pCursorLeft;
      QCursor    *pCursorRight;
      QCursor    *pCursorUp;
      QCursor    *pCursorDown;

      QCursor    *pCursorUpLeft;
      QCursor    *pCursorUpRight;
      QCursor    *pCursorDownLeft;
      QCursor    *pCursorDownRight;

      int         popx, popy;



      double       m_canvas_scale_factor;    // converter....
                                             // useage....
                                             // true_chart_scale_on_display = m_canvas_scale_factor / pixels_per_meter of displayed chart
                                             // also may be considered as the "pixels-per-meter" of the canvas on-screen
      double      m_pix_per_mm;     // pixels per millimeter on the screen
      double      m_display_size_mm;
      
      double      m_absolute_min_scale_ppm;
      QFont m_overzoomFont;
      int m_overzoomTextWidth;
      int m_overzoomTextHeight;


      //    Data
      int         m_canvas_width, m_canvas_height;

      GSHHSChart  *pWorldBackgroundChart;

      emboss_data *m_pEM_Feet;                // maps for depth unit emboss pattern
      emboss_data *m_pEM_Meters;
      emboss_data *m_pEM_Fathoms;

      emboss_data *m_pEM_OverZoom;



      double      m_true_scale_ppm;


      ColorScheme m_cs;

      
//      ViewPort    m_cache_vp;
      ViewPort    m_bm_cache_vp;
      bool        m_bbrightdir;
      int         m_brightmod;

      bool        m_bzooming, m_bzooming_to_cursor;


      //Smooth movement member variables
      zchxPoint      m_pan_drag;
      int         m_panx, m_pany, m_modkeys;
      double      m_panspeed;
      bool        m_bmouse_key_mod;
      double      m_zoom_factor, m_rotation_speed;
      int         m_mustmove;


      QDateTime m_last_movement_time;


      double      m_displayed_scale_factor;
      bool        m_disable_edge_pan;
      QFont      m_pgridFont;
      
      bool        m_dragoffsetSet;
      
      bool        m_bautofind;
      bool        m_bFirstAuto;
      double      m_vLat, m_vLon;
      ChartStack  *m_pCurrentStack;
      bool        m_bpersistent_quilt;
      int          m_restore_dbindex;
      
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
      bool         m_MouseDragging;
      QPoint       last_drag_point;
      QRect       m_scaleBarRect;
      QString      m_alertString;
      QTimer        *mDisplsyTimer;
};



#endif
