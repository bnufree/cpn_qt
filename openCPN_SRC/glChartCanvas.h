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

 #define FORMAT_BITS           GL_RGB


class glTexFactory;
class ChartCanvas;

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

#include <QOpenGLWindow>
class glChartCanvas : public QGLWidget
{
    Q_OBJECT
public:
    glChartCanvas(QGLContext *ctx, ChartCanvas *parentCavas);
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
    void FastZoom(float factor);
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

    time_t m_last_render_time;

    int viewport[4];
    double mvmatrix[16], projmatrix[16];
public slots:
    void OnActivate ();
//    void OnSize (QSize size);
    void MouseEvent(QMouseEvent* event);

protected:
    void paintGL();
    void resizeGL(int w, int h);
    void initializeGL();
    void mouseDoubleClickEvent(QMouseEvent* evnt);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

protected:

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

    void DrawChartBar( ocpnDC &dc );
    void DrawQuiting();
    void DrawCloseMessage(QString msg);
    
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
};

extern void BuildCompressedCache();

#include "glTextureManager.h"
extern glTextureManager   *g_glTextureManager;

#endif
