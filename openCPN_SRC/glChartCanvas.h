/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 */

#ifndef __GLCHARTCANVAS_H__
#define __GLCHARTCANVAS_H__

//#include <wx/glcanvas.h>

#include "dychart.h"
#include "OCPNRegion.h"
#include "LLRegion.h"
#include "viewport.h"
#include "TexFont.h"
#include <QGLWidget>
#include "gshhs.h"

 #define FORMAT_BITS           GL_RGB


class glTexFactory;
class ChartCanvas;
class GSHHSChart;

#define GESTURE_EVENT_TIMER 78334

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
class canvasConfig;

#include <QOpenGLWindow>
class glChartCanvas : public QGLWidget
{
    Q_OBJECT
public:
    glChartCanvas(/*QGLContext *ctx,*/ ChartCanvas *parentCavas);
    void setUpdateAvailable(bool sts) {mIsUpdateAvailable = sts;}
    static bool CanClipViewport(const ViewPort &vp);
    static ViewPort ClippedViewport(const ViewPort &vp, const LLRegion &region);

    static bool HasNormalizedViewPort(const ViewPort &vp);
    static void MultMatrixViewPort(ViewPort &vp, float lat=0, float lon=0);
    static ViewPort NormalizedViewPort(const ViewPort &vp, float lat=0, float lon=0);

    static void RotateToViewPort(const ViewPort &vp);
    static void DrawRegion( ViewPort &vp, const LLRegion &region);
    static void SetClipRegion( ViewPort &vp, const LLRegion &region);
    static void SetClipRect(const ViewPort &vp, const QRect &rect, bool g_clear=false);
    static void DisableClipRegion();
    void SetColorScheme(ColorScheme cs);
    
    static bool         s_b_useScissorTest;
    static bool         s_b_useStencil;
    static bool         s_b_useStencilAP;
    static bool         s_b_useFBO;
    
    void SendJSONConfigMessage();

    ~glChartCanvas();

    void SetContext(QGLContext *pcontext) { m_pcontext = pcontext; }
    void Render();    
    void FastPan(int dx, int dy);
    bool Pan(double dx, double dy);
    void FastZoom(float factor);
    void Zoom(double factor,  bool can_zoom_to_cursor = true);
    void Rotate(double rad);
    void RotateDegree(double degree);
    void RotateContinus(double dir);
    void RenderCanvasBackingChart( ocpnDC &dc, OCPNRegion chart_get_region);
    QString GetRendererString(){ return m_renderer; }
    QString GetVersionString(){ return m_version; }
    void EnablePaint(bool b_enable){ m_b_paint_enable = b_enable; }

    void Invalidate();
    void RenderRasterChartRegionGL(ChartBase *chart, ViewPort &vp, LLRegion &region);
    
    void DrawGLOverLayObjects(void);
    void GridDraw( );
    void FlushFBO( void );
    
    void DrawDynamicRoutesTracksAndWaypoints( ViewPort &vp );
    void DrawStaticRoutesTracksAndWaypoints( ViewPort &vp );
    
    void RenderAllChartOutlines( ocpnDC &dc, ViewPort &VP );
    void RenderChartOutline( int dbIndex, ViewPort &VP );

    void DrawEmboss( emboss_data *emboss );
    void ShipDraw(ocpnDC& dc);

    void SetupCompression();
    bool CanAcceleratePanning() { return m_b_BuiltFBO; }
    bool UsingFBO() { return m_b_BuiltFBO; }
    QColor GetBackGroundColor() const;
    //
    bool GetShowOutlines(){ return m_bShowOutlines; }
    void SetShowOutlines( bool show ){ m_bShowOutlines = show; }
    bool GetQuilting() {return m_bQuiting;}
    void SetQuilting(bool sts) {m_bQuiting = sts;}
    bool GetShowDepthUnits(){ return m_bShowDepthUnits; }
    void SetShowDepthUnits( bool show ){ m_bShowDepthUnits = show; }
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
    bool GetShowGrid(){ return m_bDisplayGrid; }
    void SetShowGrid( bool show ){ m_bDisplayGrid = show; }

    time_t m_last_render_time;

    int viewport[4];
    double mvmatrix[16], projmatrix[16];
    //
    void ToggleChartOutlines();
    void CanvasApplyLocale();
    ColorScheme GetColorScheme(){ return m_cs;}
    QColor GetFogColor(){ return m_fog_color; }
    GSHHSChart* GetWorldBackgroundChart() { return pWorldBackgroundChart; }
    void ResetWorldBackgroundChart() { pWorldBackgroundChart->Reset(); }
    int GetMinAvailableGshhgQuality() { return pWorldBackgroundChart->GetMinAvailableQuality(); }
    int GetMaxAvailableGshhgQuality() { return pWorldBackgroundChart->GetMaxAvailableQuality(); }

    void buildStyle();
    void initBeforeUpdateMap();

    void SetAlertString( QString str){ m_alertString = str;}
    QString GetAlertString(){ return m_alertString; }
    bool UpdateChartDatabaseInplace( ArrayOfCDI &DirArray, bool b_force, bool b_prog, const QString &ChartListFileName );
    void ClearS52PLIBStateHash(){ m_s52StateHash = 0; }
    void ApplyCanvasConfig(canvasConfig *pcc);
    void UpdateCanvasS52PLIBConfig();
    QRect GetScaleBarRect(){ return m_scaleBarRect; }
    ChartBase* GetChartAtCursor();
    ChartBase* GetOverlayChartAtCursor();
    void Refresh( bool eraseBackground = true, const QRect *rect = (const QRect *) NULL );
    void GetCursorLatLon(double *lat, double *lon);
    void updateCurrentLL();
    zchxPoint   getCurrentPos();
    zchxPointF  getCurrentLL();
    int      getCurPosX() {return mouse_x;}
    int      getCurPosY() {return mouse_y;}
    double      getCurLat() {return m_cursor_lat;}
    double      getCurLon() {return m_cursor_lon;}
    void        setCurLL(double lat, double lon);

public slots:

protected:
    void paintGL();
    void resizeGL(int w, int h);
    void initializeGL();
    void mouseDoubleClickEvent(QMouseEvent* evnt);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent *);
    void keyPressEvent(QKeyEvent *);
private:
    ViewPort GetVP();

protected:
    bool UpdateS52State();
    void DrawArrow( ocpnDC& dc, int x, int y, double rot_angle, double scale );
    void RenderQuiltViewGL( ViewPort &vp, const OCPNRegion &rect_region );
    void RenderQuiltViewGLText( ViewPort &vp, const OCPNRegion &rect_region );
    void    RenderGLAlertMessage();
    
    void BuildFBO();
    void SetupOpenGL();
    
//    void ComputeRenderQuiltViewGLRegion( ViewPort &vp, OCPNRegion &Region );
    void RenderCharts(ocpnDC &dc, const OCPNRegion &rect_region);
    void RenderNoDTA(ViewPort &vp, const LLRegion &region, int transparency = 255);
    void RenderNoDTA(ViewPort &vp, ChartBase *chart);
    void RenderWorldChart(ocpnDC &dc, ViewPort &vp, QRect &rect, bool &world_view);

    void DrawFloatingOverlayObjects( ocpnDC &dc );
    void DrawGroundedOverlayObjects(ocpnDC &dc, ViewPort &vp);
    void DrawQuiting();
    void DrawCloseMessage(QString msg);
    void GridDraw(ocpnDC& dc); // Display lat/lon Grid in chart display
    void ScaleBarDraw( ocpnDC& dc );

    emboss_data* EmbossOverzoomIndicator( ocpnDC &dc );
    emboss_data *EmbossDepthScale();
    void SetOverzoomFont();
    emboss_data *CreateEmbossMapData(QFont &font, int width, int height, const QString &str, ColorScheme cs);
    void CreateDepthUnitEmbossMaps(ColorScheme cs);
    void CreateOZEmbossMapData(ColorScheme cs);



    
    QGLContext       *m_pcontext;

    int max_texture_dimension;

    bool m_bsetup;

    QString m_renderer;
    QString m_version;
    QString m_extensions;    
    
    ViewPort    m_cache_vp;
    ChartBase   *m_cache_current_ch;
    
    bool        m_b_paint_enable;
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

//    GLuint      ownship_tex;
//    int         ownship_color;
//    QSize      ownship_size, ownship_tex_size;

    GLuint      m_piano_tex;
    
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

//    GLuint       m_tideTex;
//    GLuint       m_currentTex;
//    int          m_tideTexWidth;
//    int          m_tideTexHeight;
//    int          m_currentTexWidth;
//    int          m_currentTexHeight;
    
    ChartCanvas *m_pParentCanvas;
    bool            mIsUpdateAvailable;
    bool            m_bShowOutlines;
    bool            m_bQuiting;
    bool            m_bShowDepthUnits;
    bool            m_encShowText;
    bool            m_encShowDepth;
    bool            m_encShowLightDesc;
    bool            m_encShowBuoyLabels;
    int             m_encDisplayCategory;
    bool            m_encShowLights;
    bool            m_encShowAnchor;
    bool            m_encShowDataQual;
    bool            m_bDisplayGrid;

    //
    emboss_data     *m_pEM_Feet;                // maps for depth unit emboss pattern
    emboss_data     *m_pEM_Meters;
    emboss_data     *m_pEM_Fathoms;
    emboss_data     *m_pEM_OverZoom;
    QFont m_overzoomFont;
    int m_overzoomTextWidth;
    int m_overzoomTextHeight;

    ColorScheme m_cs;
    QColor    m_fog_color;

    GSHHSChart  *pWorldBackgroundChart;
    QCursor    *pCursorPencil;
    QCursor    *pCursorArrow;
    QCursor    *pCursorCross;
    QCursor    *pPlugIn_Cursor;
    QCursor    *pCursorLeft;
    QCursor    *pCursorRight;
    QCursor    *pCursorUp;
    QCursor    *pCursorDown;
    QCursor    *pCursorUpLeft;
    QCursor    *pCursorUpRight;
    QCursor    *pCursorDownLeft;
    QCursor    *pCursorDownRight;
    QString      m_alertString;
    long         m_s52StateHash;
    QRect       m_scaleBarRect;

    int         mouse_x;                //当前鼠标点的位置
    int         mouse_y;
    double      m_cursor_lon, m_cursor_lat;
    bool        m_bzooming;
    bool        m_MouseDragging;
    QPoint      last_drag_point;
    bool        mIsLeftDown;
    int         m_modkeys;
};

extern void BuildCompressedCache();

#include "glTextureManager.h"
extern glTextureManager   *g_glTextureManager;

#endif
