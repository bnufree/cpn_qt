/******************************************************************************
 *
 * Project:  OpenCPN
 * Authors:  David Register
 *           Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2014 by David S. Register                               *
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

#include <stdint.h>

#include "config.h"

#if defined( __UNIX__ ) && !defined(__WXOSX__)  // high resolution stopwatch for profiling
class OCPNStopWatch
{
public:
    OCPNStopWatch() { Reset(); }
    void Reset() { clock_gettime(CLOCK_REALTIME, &tp); }

    double GetTime() {
        timespec tp_end;
        clock_gettime(CLOCK_REALTIME, &tp_end);
        return (tp_end.tv_sec - tp.tv_sec) * 1.e3 + (tp_end.tv_nsec - tp.tv_nsec) / 1.e6;
    }

private:
    timespec tp;
};
#endif


//#if defined(__OCPN__ANDROID__)
//#include "androidUTIL.h"
//#elif defined(__WXQT__)
//#include <GL/wglext.h>
//#endif

#include "dychart.h"

#include "glChartCanvas.h"
#include "chcanv.h"
#include "s52plib.h"
#include "Quilt.h"
//#include "pluginmanager.h"
#include "chartbase.h"
#include "chartimg.h"
#include "ChInfoWin.h"
#include "thumbwin.h"
#include "chartdb.h"
//#include "navutil.h"
#include "TexFont.h"
#include "glTexCache.h"
#include "gshhs.h"
#include "OCPNPlatform.h"
//#include "tcmgr.h"
#include "compass.h"
#include "FontMgr.h"
#include "mipmap/mipmap.h"
#include "chartimg.h"
#include "mbtiles.h"
#include <vector>
#include <algorithm>
#include "styles.h"
#include "GL/glext.h"
#include "zchxconfig.h"
#include "CanvasConfig.h"
#include "emboss_data.h"

#include <QMessageBox>
#include <QProgressDialog>

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif

#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"

#include "lz4/lz4.h"

#ifdef __OCPN__ANDROID__
//  arm gcc compiler has a lot of trouble passing doubles as function aruments.
//  We don't really need double precision here, so fix with a (faster) macro.
extern "C" void glOrthof(float left,  float right,  float bottom,  float top,  float near,  float far);
#define glOrtho(a,b,c,d,e,f);     glOrthof(a,b,c,d,e,f);

#endif

#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"
#include "zchxmapmainwindow.h"

//extern bool GetMemoryStatus(int *mem_total, int *mem_used);

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#endif

extern zchxMapMainWindow *gFrame;
extern s52plib *ps52plib;
extern bool g_bDebugOGL;
extern bool g_bShowFPS;
extern bool g_bSoftwareGL;
extern bool g_btouch;
extern bool             g_bShowChartBar;
extern glTextureManager   *g_glTextureManager;
extern bool             b_inCompressAllCharts;
extern std::vector<int> g_quilt_noshow_index_array;

extern GLenum       g_texture_rectangle_format;
extern QString                  *pInit_Chart_Dir;
extern ChartGroupArray  *g_pGroupArray;
extern double           g_display_size_mm;
int                       g_mem_total, g_mem_used, g_mem_initial;
extern QString                             ChartListFileName;
extern QString                  gWorldMapLocation;
QString gDefaultWorldMapLocation;
extern int              g_iDistanceFormat;

extern int g_memCacheLimit;
extern ColorScheme global_color_scheme;
extern ThumbWin         *pthumbwin;
extern bool             g_bDisplayGrid;
extern int g_mipmap_max_level;

extern double           gLat, gLon, gCog, gSog, gHdt;

extern int              g_OwnShipIconType;
extern double           g_ownship_predictor_minutes;
extern double           g_ownship_HDTpredictor_miles;

extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;

extern ChartDB          *ChartData;

extern bool             b_inCompressAllCharts;
extern bool             g_bGLexpert;
extern bool             g_bcompression_wait;
extern bool             g_bresponsive;
extern float            g_ChartScaleFactorExp;
extern float            g_ShipScaleFactorExp;

float            g_GLMinSymbolLineWidth;
float            g_GLMinCartographicLineWidth;

extern bool             g_fog_overzoom;
extern double           g_overzoom_emphasis_base;
extern bool             g_oz_vector_scale;
extern unsigned int     g_canvasConfig;

extern zchxGLOptions g_GLOptions;

//    For VBO(s)
extern bool         g_b_EnableVBO;
bool         g_b_needFinish;  //Need glFinish() call on each frame?


PFNGLGENFRAMEBUFFERSEXTPROC         s_glGenFramebuffers;
PFNGLGENRENDERBUFFERSEXTPROC        s_glGenRenderbuffers;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    s_glFramebufferTexture2D;
PFNGLBINDFRAMEBUFFEREXTPROC         s_glBindFramebuffer;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC s_glFramebufferRenderbuffer;
PFNGLRENDERBUFFERSTORAGEEXTPROC     s_glRenderbufferStorage;
PFNGLBINDRENDERBUFFEREXTPROC        s_glBindRenderbuffer;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  s_glCheckFramebufferStatus;
PFNGLDELETEFRAMEBUFFERSEXTPROC      s_glDeleteFramebuffers;
PFNGLDELETERENDERBUFFERSEXTPROC     s_glDeleteRenderbuffers;

PFNGLCOMPRESSEDTEXIMAGE2DPROC s_glCompressedTexImage2D;
PFNGLGETCOMPRESSEDTEXIMAGEPROC s_glGetCompressedTexImage;

//      Vertex Buffer Object (VBO) support
PFNGLGENBUFFERSPROC                 s_glGenBuffers;
PFNGLBINDBUFFERPROC                 s_glBindBuffer;
PFNGLBUFFERDATAPROC                 s_glBufferData;
PFNGLDELETEBUFFERSPROC              s_glDeleteBuffers;


extern GLuint g_raster_format/* = GL_RGB*/;
long g_tex_mem_used;

bool            b_timeGL;
//wxStopWatch     g_glstopwatch;
double          g_gl_ms_per_frame;

int g_tile_size;
int g_uncompressed_tile_size;

bool glChartCanvas::s_b_useScissorTest;
bool glChartCanvas::s_b_useStencil;
bool glChartCanvas::s_b_useStencilAP;
bool glChartCanvas::s_b_useFBO;

//#define     GL_TEST


//static int s_nquickbind;


/* for debugging */
static void print_region(OCPNRegion &Region)
{
    OCPNRegionIterator upd ( Region );
    while ( upd.HaveRects() )
    {
        QRect rect = upd.GetRect();
        qDebug("[(%d, %d) (%d, %d)] ", rect.x(), rect.y(), rect.width(), rect.height());
        upd.NextRect();
    }
}

GLboolean QueryExtension( const char *extName )
{
    /*
     ** Search for extName in the extensions string. Use of strstr()
     ** is not sufficient because extension names can be prefixes of
     ** other extension names. Could use strtok() but the constant
     ** string returned by glGetString might be in read-only memory.
     */
    char *p;
    char *end;
    int extNameLen;

    extNameLen = strlen( extName );

    p = (char *) glGetString( GL_EXTENSIONS );
    if( NULL == p ) {
        return GL_FALSE;
    }

    end = p + strlen( p );

    while( p < end ) {
        int n = strcspn( p, " " );
        if( ( extNameLen == n ) && ( strncmp( extName, p, n ) == 0 ) ) {
            return GL_TRUE;
        }
        p += ( n + 1 );
    }
    return GL_FALSE;
}

typedef void (*GenericFunction)(void);

//#if defined(__WXMSW__)
//#define systemGetProcAddress(ADDR) wglGetProcAddress(ADDR)
//#elif defined(__WXOSX__)
//#include <dlfcn.h>
//#define systemGetProcAddress(ADDR) dlsym( RTLD_DEFAULT, ADDR)
//#elif defined(__OCPN__ANDROID__)
//#define systemGetProcAddress(ADDR) eglGetProcAddress(ADDR)
//#else
//#define systemGetProcAddress(ADDR) glXGetProcAddress((const GLubyte*)ADDR)
//#endif

#define systemGetProcAddress(ADDR) wglGetProcAddress(ADDR)

GenericFunction ocpnGetProcAddress(const char *addr, const char *extension)
{
    char addrbuf[256];
    if(!extension)
        return (GenericFunction)NULL;

#ifndef __OCPN__ANDROID__    
    //  If this is an extension entry point,
    //  We look explicitly in the extensions list to confirm
    //  that the request is actually supported.
    // This may be redundant, but is conservative, and only happens once per session.    
    if(extension && strlen(extension)){
        QString s_extension = QString::fromUtf8(&addr[2]);
        QString s_family = QString::fromUtf8(extension);
        s_extension.insert(0, "_");
        s_extension.insert(0, s_family);
        s_extension.insert(0, "GL_");
        
        if(!QueryExtension( s_extension.toUtf8().data() )){
            return (GenericFunction)NULL;
        }
    }
#endif    
    
    snprintf(addrbuf, sizeof addrbuf, "%s%s", addr, extension);
    return (GenericFunction)systemGetProcAddress(addrbuf);
    
}

bool  b_glEntryPointsSet;

void GetglEntryPoints( OCPN_GLCaps *pcaps )
{
    
    // the following are all part of framebuffer object,
    // according to opengl spec, we cannot mix EXT and ARB extensions
    // (I don't know that it could ever happen, but if it did, bad things would happen)

#ifndef __OCPN__ANDROID__
    const char *extensions[] = {"", "ARB", "EXT", 0 };
#else
    const char *extensions[] = {"OES", 0 };
#endif
    
    unsigned int n_ext = (sizeof extensions) / (sizeof *extensions);

    unsigned int i;
    for(i=0; i<n_ext; i++) {
        if((pcaps->m_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenFramebuffers", extensions[i])))
            break;
    }

    if(i<n_ext){
        pcaps->m_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenRenderbuffers", extensions[i]);
        pcaps->m_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
            ocpnGetProcAddress( "glFramebufferTexture2D", extensions[i]);
        pcaps->m_glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindFramebuffer", extensions[i]);
        pcaps->m_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glFramebufferRenderbuffer", extensions[i]);
        pcaps->m_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)
            ocpnGetProcAddress( "glRenderbufferStorage", extensions[i]);
        pcaps->m_glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindRenderbuffer", extensions[i]);
        pcaps->m_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)
            ocpnGetProcAddress( "glCheckFramebufferStatus", extensions[i]);
        pcaps->m_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteFramebuffers", extensions[i]);
        pcaps->m_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteRenderbuffers", extensions[i]);
            
        //VBO
        pcaps->m_glGenBuffers = (PFNGLGENBUFFERSPROC)
            ocpnGetProcAddress( "glGenBuffers", extensions[i]);
        pcaps->m_glBindBuffer = (PFNGLBINDBUFFERPROC)
            ocpnGetProcAddress( "glBindBuffer", extensions[i]);
        pcaps->m_glBufferData = (PFNGLBUFFERDATAPROC)
            ocpnGetProcAddress( "glBufferData", extensions[i]);
        pcaps->m_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)
            ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);

             
    }

    //  Retry VBO entry points with all extensions
    if(0 == pcaps->m_glGenBuffers){
        for( i=0; i<n_ext; i++) {
            if((pcaps->m_glGenBuffers = (PFNGLGENBUFFERSPROC)ocpnGetProcAddress( "glGenBuffers", extensions[i])) )
                break;
        }
        
        if( i < n_ext ){
            pcaps->m_glBindBuffer = (PFNGLBINDBUFFERPROC) ocpnGetProcAddress( "glBindBuffer", extensions[i]);
            pcaps->m_glBufferData = (PFNGLBUFFERDATAPROC) ocpnGetProcAddress( "glBufferData", extensions[i]);
            pcaps->m_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);
        }
    }
            

#ifndef __OCPN__ANDROID__            
    for(i=0; i<n_ext; i++) {
        if((pcaps->m_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)
            ocpnGetProcAddress( "glCompressedTexImage2D", extensions[i])))
            break;
    }

    if(i<n_ext){
        pcaps->m_glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)
            ocpnGetProcAddress( "glGetCompressedTexImage", extensions[i]);
    }
#else    
    pcaps->m_glCompressedTexImage2D =          glCompressedTexImage2D;
#endif
    
}





static void GetglEntryPoints( void )
{
    b_glEntryPointsSet = true;
    
    // the following are all part of framebuffer object,
    // according to opengl spec, we cannot mix EXT and ARB extensions
    // (I don't know that it could ever happen, but if it did, bad things would happen)

#ifndef __OCPN__ANDROID__
    const char *extensions[] = {"", "ARB", "EXT", 0 };
#else
    const char *extensions[] = {"OES", 0 };
#endif
    
    unsigned int n_ext = (sizeof extensions) / (sizeof *extensions);

    unsigned int i;
    for(i=0; i<n_ext; i++) {
        if((s_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenFramebuffers", extensions[i])))
            break;
    }

    if(i<n_ext){
        s_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glGenRenderbuffers", extensions[i]);
        s_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
            ocpnGetProcAddress( "glFramebufferTexture2D", extensions[i]);
        s_glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindFramebuffer", extensions[i]);
        s_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glFramebufferRenderbuffer", extensions[i]);
        s_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)
            ocpnGetProcAddress( "glRenderbufferStorage", extensions[i]);
        s_glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)
            ocpnGetProcAddress( "glBindRenderbuffer", extensions[i]);
        s_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)
            ocpnGetProcAddress( "glCheckFramebufferStatus", extensions[i]);
        s_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteFramebuffers", extensions[i]);
        s_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)
            ocpnGetProcAddress( "glDeleteRenderbuffers", extensions[i]);
            
        //VBO
        s_glGenBuffers = (PFNGLGENBUFFERSPROC)
            ocpnGetProcAddress( "glGenBuffers", extensions[i]);
        s_glBindBuffer = (PFNGLBINDBUFFERPROC)
            ocpnGetProcAddress( "glBindBuffer", extensions[i]);
        s_glBufferData = (PFNGLBUFFERDATAPROC)
            ocpnGetProcAddress( "glBufferData", extensions[i]);
        s_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)
            ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);

            
    }

    //  Retry VBO entry points with all extensions
    if(0 == s_glGenBuffers){
        for( i=0; i<n_ext; i++) {
            if((s_glGenBuffers = (PFNGLGENBUFFERSPROC)ocpnGetProcAddress( "glGenBuffers", extensions[i])) )
                break;
        }
        
        if( i < n_ext ){
            s_glBindBuffer = (PFNGLBINDBUFFERPROC) ocpnGetProcAddress( "glBindBuffer", extensions[i]);
            s_glBufferData = (PFNGLBUFFERDATAPROC) ocpnGetProcAddress( "glBufferData", extensions[i]);
            s_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) ocpnGetProcAddress( "glDeleteBuffers", extensions[i]);
        }
    }
            

#ifndef __OCPN__ANDROID__            
    for(i=0; i<n_ext; i++) {
        if((s_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)
            ocpnGetProcAddress( "glCompressedTexImage2D", extensions[i])))
            break;
    }

    if(i<n_ext){
        s_glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)
            ocpnGetProcAddress( "glGetCompressedTexImage", extensions[i]);
    }
#else    
    s_glCompressedTexImage2D =          glCompressedTexImage2D;
#endif
    
}

glChartCanvas::glChartCanvas(QWidget* parent) : QGLWidget(parent)
    , m_bsetup( false )
    , mFrameWork(new ChartFrameWork(this))
    , m_bQuiting(false)
    , m_bShowDepthUnits(false)
    , m_bDisplayGrid(false)
    , m_encDisplayCategory((int)STANDARD)
    , m_encShowLights(true)
    , m_encShowAnchor(true)
    , m_encShowDataQual(false)
    , mIsLeftDown(false)
    , m_MouseDragging(false)
{
    m_pcontext = this->context();
    m_pEM_Feet = NULL;
    m_pEM_Meters = NULL;
    m_pEM_Fathoms = NULL;
    m_pEM_OverZoom = NULL;
    m_overzoomTextWidth = 0;
    m_overzoomTextHeight = 0;
    m_cs = GLOBAL_COLOR_SCHEME_DAY;
    pWorldBackgroundChart = new GSHHSChart;
    
    m_cache_current_ch = NULL;

    m_bShowOutlines = false;

    m_b_paint_enable = false;
    m_in_glpaint = false;

    m_cache_tex[0] = m_cache_tex[1] = 0;
    m_cache_page = 0;

    m_b_BuiltFBO = false;
    m_b_DisableFBO = false;

    m_piano_tex = 0;
    
    m_binPinch = false;
    m_binPan = false;
    m_bpinchGuard = false;
    
    b_timeGL = true;
    m_last_render_time = -1;

    m_LRUtime = 0;
    setFocusPolicy(Qt::StrongFocus);
    

    if( !g_glTextureManager) g_glTextureManager = new glTextureManager;
    QTimer::singleShot(100, this, SLOT(slotStartLoadEcdis()));
    mDisplsyTimer = new QTimer(this);
    mDisplsyTimer->setInterval(1000);
    connect(mDisplsyTimer, SIGNAL(timeout()), this, SLOT(update()));
    mDisplsyTimer->start();
}

glChartCanvas::~glChartCanvas()
{
    if(m_pEM_Feet )delete m_pEM_Feet;
    if(m_pEM_Meters )delete m_pEM_Meters;
    if(m_pEM_Fathoms )delete m_pEM_Fathoms;
    if(m_pEM_OverZoom )delete m_pEM_OverZoom;
    if(pWorldBackgroundChart) delete pWorldBackgroundChart;
    if(pCursorLeft)delete pCursorLeft;
    if(pCursorRight)delete pCursorRight;
    if(pCursorUp)delete pCursorUp;
    if(pCursorDown)delete pCursorDown;
    if(pCursorArrow)delete pCursorArrow;
    if(pCursorPencil)delete pCursorPencil;
    if(pCursorCross)delete pCursorCross;
}

void glChartCanvas::FlushFBO( void ) 
{
    if(m_bsetup)
        BuildFBO();
}

void glChartCanvas::initializeGL()
{
    qDebug()<<"now initialized...";
#ifdef GL_TEST
    glClearColor(0.0, 0.2, 0.3, 1.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH);
#endif
}

void glChartCanvas::resizeGL(int w, int h)
{
    qDebug()<<"now resized with:"<<w<<h;
#ifdef GL_TEST
    int side = qMin(w, h);
    glViewport((width() - side) / 2, (height() - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.2, 1.2, -1.2, 1.2, 5.0, 60.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -40.0);
#endif
    if(mFrameWork) mFrameWork->slotResize(w, h);
    if(m_bsetup)
    {
        BuildFBO();
    }
    QGLWidget::resizeGL(w, h);
}


void glChartCanvas::mouseDoubleClickEvent(QMouseEvent* event)
{
    QGLWidget::mouseDoubleClickEvent(event);
}

void glChartCanvas::mouseMoveEvent(QMouseEvent* e)
{
    mouse_x = e->pos().x();
    mouse_y = e->pos().y();
    updateCurrentLL();
    if(e->buttons() & Qt::LeftButton)
    {
        if(mIsLeftDown)
        {
            m_MouseDragging = true;
            //没有拖动的情况,将地图的中心移动到这里
            QPoint pos = e->pos();
            int dx  = pos.x() - last_drag_point.x();
            int dy  =  pos.y() - last_drag_point.y();
            if(abs(dx) > 2 || abs(dy) > 2)
            {
                Pan(-dx, -dy);
                last_drag_point = pos;
            }
        }
    }
    QGLWidget::mouseMoveEvent(e);
}

void glChartCanvas::mousePressEvent(QMouseEvent* e)
{
    m_MouseDragging = false;
    if(e->buttons() & Qt::LeftButton)
    {
        mouse_x = e->pos().x();
        mouse_y = e->pos().y();
        updateCurrentLL();
        mIsLeftDown = true;
        last_drag_point = e->pos();
    }
    qDebug()<<e->buttons()<<mIsLeftDown;
    QGLWidget::mousePressEvent(e);
}

void glChartCanvas::mouseReleaseEvent(QMouseEvent* e)
{
    mouse_x = e->pos().x();
    mouse_y = e->pos().y();
    updateCurrentLL();
    if(mIsLeftDown)
    {
        mIsLeftDown = false;
        if(!m_MouseDragging)
        {
            //没有拖动的情况,将地图的中心移动到这里
            QPoint pos = e->pos();
            Pan( pos.x() - GetVP().pixWidth() / 2, pos.y() - GetVP().pixHeight() / 2 );
        }
    }
    qDebug()<<e->buttons()<<mIsLeftDown;
    m_MouseDragging = false;

    QGLWidget::mouseReleaseEvent(e);
}

void glChartCanvas::wheelEvent(QWheelEvent * e)
{
    static uint time = 0;
    uint cur = QDateTime::currentDateTime().toTime_t();
    if(cur - time >= 1)
    {
        time = cur;
        if(e->delta() > 0)
        {
            Zoom(2, false);
        } else
        {
            Zoom(0.5, false);
        }
    }
}

void glChartCanvas::keyPressEvent(QKeyEvent *event)
{
    m_modkeys = event->modifiers();
    int key_char = event->key();
 //   qDebug()<<"mm:"<<m_modkeys<<" key:"<<key_char;

    bool b_handled = false;
    //处理旋转
    if( m_modkeys == Qt::NoModifier)
    {
        b_handled = true;
        switch( key_char )
        {
        case ']':
            RotateContinus( 1 );
            break;

        case '[':
            RotateContinus( -1 );
            break;

        case '\\':
            Rotate(0);
            break;
        default:
            b_handled = false;
            break;
        }
    }

    int panspeed = (m_modkeys == Qt::AltModifier ? 2 : 100);
    // HOTKEYS
    switch( key_char )
    {
    case Qt::Key_Left:
        Pan( -panspeed, 0 );
        b_handled = true;
        break;

    case Qt::Key_Up:
        Pan( 0, -panspeed );
        b_handled = true;
        break;

    case Qt::Key_Right:
        Pan( panspeed, 0 );
        b_handled = true;
        break;

    case Qt::Key_Down:
        Pan(0, panspeed );
        b_handled = true;
        break;
    case Qt::Key_F3:
    {
        SetShowENCText( !GetShowENCText() );
        Refresh(true);
        Invalidate();
        break;
    }

    case Qt::Key_F12: {
        ToggleChartOutlines();
        break;
    }
    case Qt::Key_PageUp:
        Zoom( 2.0, false );
        break;
        //    case Qt::Key_NUMPAD_SUBTRACT:   // '-' on NUM PAD
    case Qt::Key_PageDown:
        Zoom( .5, false );
        break;
    default:
        break;

    }
    QGLWidget::keyPressEvent(event);
}



void glChartCanvas::BuildFBO( )
{
    if(isVisible())        makeCurrent();
    
    if( m_b_BuiltFBO ) {
        glDeleteTextures( 2, m_cache_tex );
        ( s_glDeleteFramebuffers )( 1, &m_fb0 );
        ( s_glDeleteRenderbuffers )( 1, &m_renderbuffer );
        m_b_BuiltFBO = false;
    }

    if( m_b_DisableFBO)
        return;

    //  In CanvasPanning mode, we will build square POT textures for the FBO backing store
    //  We will make them as large as possible...
    if(g_GLOptions.m_bUseCanvasPanning){
        int rb_x = width();
        int rb_y = height();
        int i=1;
        while(i < rb_x) i <<= 1;
            rb_x = i;
            
        i=1;
        while(i < rb_y) i <<= 1;
            rb_y = i;
            
        m_cache_tex_x = fmax(rb_x, rb_y);
        m_cache_tex_y = fmax(rb_x, rb_y);
        m_cache_tex_x = fmax(2048, m_cache_tex_x);
        m_cache_tex_y = fmax(2048, m_cache_tex_y);
    } else {            
        m_cache_tex_x = width();
        m_cache_tex_y = height();
    }        
        
    ( s_glGenFramebuffers )( 1, &m_fb0 );
    ( s_glGenRenderbuffers )( 1, &m_renderbuffer );

    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );
    

    // initialize color textures
    glGenTextures( 2, m_cache_tex );
    for(int i=0; i<2; i++) {
        glBindTexture( g_texture_rectangle_format, m_cache_tex[i] );
        glTexParameterf( g_texture_rectangle_format, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( g_texture_rectangle_format, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexImage2D( g_texture_rectangle_format, 0, GL_RGBA, m_cache_tex_x, m_cache_tex_y, 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, NULL );
        
    }

    ( s_glBindRenderbuffer )( GL_RENDERBUFFER_EXT, m_renderbuffer );

    if( m_b_useFBOStencil ) {
        // initialize composite depth/stencil renderbuffer
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT,
                                         m_cache_tex_x, m_cache_tex_y );
        
        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );

        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );
    } else {
        
        GLenum depth_format = GL_DEPTH_COMPONENT24;
        
        //      Need to check for availability of 24 bit depth buffer extension on GLES
#ifdef ocpnUSE_GLES
        if( !QueryExtension("GL_OES_depth24") )
            depth_format = GL_DEPTH_COMPONENT16;
#endif        
        
        // initialize depth renderbuffer
        ( s_glRenderbufferStorage )( GL_RENDERBUFFER_EXT, depth_format,
                                         m_cache_tex_x, m_cache_tex_y );
        int err = glGetError();
        if(err){
            qDebug("    OpenGL-> Framebuffer Depth Buffer Storage error:  %08X", err );
        }
                
        ( s_glFramebufferRenderbuffer )( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_RENDERBUFFER_EXT, m_renderbuffer );
        
        err = glGetError();
        if(err){
            qDebug("    OpenGL-> Framebuffer Depth Buffer Attach error:  %08X", err );
        }
    }
    
    // Disable Render to FBO
    ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );

    /* invalidate cache */
    Invalidate();

    glClear( GL_COLOR_BUFFER_BIT );
    m_b_BuiltFBO = true;
}


void glChartCanvas::SetupOpenGL()
{
    char *str = (char *) glGetString( GL_RENDERER );
    if (str == NULL) {
        // perhaps we should edit the config and turn off opengl now
        qDebug("Failed to initialize OpenGL");
        exit(1);
    }
    
    char render_string[80];
    strncpy( render_string, str, 79 );
    m_renderer = QString::fromUtf8(render_string );

    QString msg;
    if(g_bSoftwareGL)
    {
        msg.sprintf("OpenGL-> Software OpenGL");
    }
    msg.sprintf("OpenGL-> Renderer String: " );
    msg += m_renderer;
    qDebug()<<msg;

    if( ps52plib ) ps52plib->SetGLRendererString( m_renderer );
    
    char version_string[80];
    strncpy( version_string, (char *) glGetString( GL_VERSION ), 79 );
    msg.sprintf("OpenGL-> Version reported:  ");
    m_version = QString::fromUtf8(version_string );
    msg += m_version;
    qDebug()<< msg;
    
    const GLubyte *ext_str = glGetString(GL_EXTENSIONS);
    m_extensions = QString::fromUtf8((const char *)ext_str );
//    qDebug("OpenGL extensions available: %s", m_extensions.toUtf8().data() );

    bool b_oldIntel = false;
    if( GetRendererString().toUpper().indexOf("INTEL")  != -1 ){
        if( GetRendererString().toUpper().indexOf("965")  != -1 ){
            qDebug("OpenGL-> Detected early Intel renderer, disabling some GL features" );
            b_oldIntel = true;
        }
    }
    
    //  Set the minimum line width
    GLint parms[2];
    glGetIntegerv( GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0] );
    g_GLMinSymbolLineWidth = fmax(parms[0], 1);
    g_GLMinCartographicLineWidth = fmax(parms[0], 1);
    
    //    Some GL renderers do a poor job of Anti-aliasing very narrow line widths.
    //    This is most evident on rendered symbols which have horizontal or vertical line segments
    //    Detect this case, and adjust the render parameters.
    
    if( m_renderer.toUpper().indexOf("MESA")  != -1 ){
        GLfloat parf;
        glGetFloatv(  GL_SMOOTH_LINE_WIDTH_GRANULARITY, &parf );
        
        g_GLMinSymbolLineWidth = fmax(((float)parms[0] + parf), 1);
    }
    
    s_b_useScissorTest = true;
    // the radeon x600 driver has buggy scissor test
    if( GetRendererString().toUpper().indexOf("RADEON X600")  != -1 )
        s_b_useScissorTest = false;

    if( GetRendererString().indexOf("GeForce")  != -1 )   //GeForce GTX 1070
        s_b_useScissorTest = false;
        
    //  This little hack fixes a problem seen with some Intel 945 graphics chips
    //  We need to not do anything that requires (some) complicated stencil operations.

    bool bad_stencil_code = false;
    if( GetRendererString().indexOf("Intel")  != -1 ) {
        qDebug("OpenGL-> Detected Intel renderer, disabling stencil buffer" );
        bad_stencil_code = true;
    }

    //      And for the lousy Unichrome drivers, too
    if( GetRendererString().indexOf("UniChrome")  != -1 )
        bad_stencil_code = true;

    //      And for the lousy Mali drivers, too
    if( GetRendererString().indexOf("Mali")  != -1 )
        bad_stencil_code = true;

    //XP  Generic Needs stencil buffer
    //W7 Generic Needs stencil buffer    
//      if( GetRendererString().Find( _T("Generic") ) != Q_INDEX_NOT_FOUND ) {
//          qDebug("OpenGL-> Detected Generic renderer, disabling stencil buffer") );
//          bad_stencil_code = true;
//      }
    
    //          Seen with intel processor on VBox Win7
    if( GetRendererString().indexOf("Chromium")  != -1 ) {
        qDebug("OpenGL-> Detected Chromium renderer, disabling stencil buffer" );
        bad_stencil_code = true;
    }
    
    //      Stencil buffer test
    glEnable( GL_STENCIL_TEST );
    GLboolean stencil = glIsEnabled( GL_STENCIL_TEST );
    int sb;
    glGetIntegerv( GL_STENCIL_BITS, &sb );
    //        printf("Stencil Buffer Available: %d\nStencil bits: %d\n", stencil, sb);
    glDisable( GL_STENCIL_TEST );

    s_b_useStencil = false;
    if( stencil && ( sb == 8 ) )
        s_b_useStencil = true;
     
    if( QueryExtension( "GL_ARB_texture_non_power_of_two" ) )
        g_texture_rectangle_format = GL_TEXTURE_2D;
    else if( QueryExtension( "GL_OES_texture_npot" ) )
        g_texture_rectangle_format = GL_TEXTURE_2D;
    else if( QueryExtension( "GL_ARB_texture_rectangle" ) )
        g_texture_rectangle_format = GL_TEXTURE_RECTANGLE_ARB;
    qDebug("OpenGL-> Texture rectangle format: %x",g_texture_rectangle_format);

#ifndef __OCPN__ANDROID__
        //      We require certain extensions to support FBO rendering
        if(!g_texture_rectangle_format)
            m_b_DisableFBO = true;
        
        if(!QueryExtension( "GL_EXT_framebuffer_object" ))
            m_b_DisableFBO = true;
#endif
 
#ifdef __OCPN__ANDROID__
         g_texture_rectangle_format = GL_TEXTURE_2D;
#endif
        
    GetglEntryPoints();
    
    if( !s_glGenFramebuffers  || !s_glGenRenderbuffers        || !s_glFramebufferTexture2D ||
        !s_glBindFramebuffer  || !s_glFramebufferRenderbuffer || !s_glRenderbufferStorage  ||
        !s_glBindRenderbuffer || !s_glCheckFramebufferStatus  || !s_glDeleteFramebuffers   ||
        !s_glDeleteRenderbuffers )
        m_b_DisableFBO = true;

    // VBO??
    
    g_b_EnableVBO = true;
    if( !s_glBindBuffer || !s_glBufferData || !s_glGenBuffers || !s_glDeleteBuffers )
        g_b_EnableVBO = false;

#if defined( __WXMSW__ ) || defined(__WXOSX__)
    if(b_oldIntel)    
        g_b_EnableVBO = false;
#endif

#ifdef __OCPN__ANDROID__
    g_b_EnableVBO = false;
#endif

    if(g_b_EnableVBO)
        qDebug("OpenGL-> Using Vertexbuffer Objects") ;
    else
        qDebug("OpenGL-> Vertexbuffer Objects unavailable");
    
    
    //      Can we use the stencil buffer in a FBO?
#ifdef ocpnUSE_GLES 
    m_b_useFBOStencil = QueryExtension( "GL_OES_packed_depth_stencil" );
#else
    m_b_useFBOStencil = QueryExtension( "GL_EXT_packed_depth_stencil" ) == GL_TRUE;
#endif

#ifdef __OCPN__ANDROID__
    m_b_useFBOStencil = false;
#endif
    
    //  On Intel Graphics platforms, don't use stencil buffer at all
    if( bad_stencil_code)    
        s_b_useStencil = false;
    
    g_GLOptions.m_bUseCanvasPanning = false;
#ifdef __OCPN__ANDROID__
    g_GLOptions.m_bUseCanvasPanning = isPlatformCapable(PLATFORM_CAP_FASTPAN);
#endif
        
    //      Maybe build FBO(s)

    BuildFBO();
    
    
    
    
#if 1   /* this test sometimes fails when the fbo still works */
        //  But we need to be ultra-conservative here, so run all the tests we can think of
    
    
    //  But we cannot even run this test on some platforms
    //  So we simply have to declare FBO unavailable
#ifdef __WXMSW__
    if( GetRendererString().Upper().Find( _T("INTEL") ) != Q_INDEX_NOT_FOUND ) {
        if(GetRendererString().Upper().Find( _T("MOBILE") ) != Q_INDEX_NOT_FOUND ){
            qDebug("OpenGL-> Detected Windows Intel Mobile renderer, disabling Frame Buffer Objects") );
            m_b_DisableFBO = true;
            BuildFBO();
        }
    }
#endif
    
    if( m_b_BuiltFBO ) {
        // Check framebuffer completeness at the end of initialization.
        ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );
        
        ( s_glFramebufferTexture2D )
        ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
          g_texture_rectangle_format, m_cache_tex[0], 0 );
        
        GLenum fb_status = ( s_glCheckFramebufferStatus )( GL_FRAMEBUFFER_EXT );
        ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );
        
        if( fb_status != GL_FRAMEBUFFER_COMPLETE_EXT ) {
            qDebug("    OpenGL-> Framebuffer Incomplete:  %08X", fb_status );
            m_b_DisableFBO = true;
            BuildFBO();
        }
    }
#endif

#ifdef __OCPN__ANDROID__
    g_GLOptions.m_bUseCanvasPanning = m_b_BuiltFBO;
    if(g_GLOptions.m_bUseCanvasPanning)
        qDebug("OpenGL-> Using FastCanvas Panning/Zooming") );
    
#endif

    if( m_b_BuiltFBO && !m_b_useFBOStencil )
        s_b_useStencil = false;

    //  If stencil seems to be a problem, force use of depth buffer clipping for Area Patterns
    s_b_useStencilAP = s_b_useStencil & !bad_stencil_code;

    if( m_b_BuiltFBO ) {
        qDebug("OpenGL-> Using Framebuffer Objects");

        if( m_b_useFBOStencil )
            qDebug("OpenGL-> Using FBO Stencil buffer" );
        else
            qDebug("OpenGL-> FBO Stencil buffer unavailable" );
    } else
        qDebug("OpenGL-> Framebuffer Objects unavailable" );

    if( s_b_useStencil ) qDebug("OpenGL-> Using Stencil buffer clipping" );
    else
        qDebug("OpenGL-> Using Depth buffer clipping" );

    if(s_b_useScissorTest && s_b_useStencil)
        qDebug("OpenGL-> Using Scissor Clipping" );

    /* we upload non-aligned memory */
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    MipMap_ResolveRoutines();
    SetupCompression();

    qDebug("OpenGL-> Minimum cartographic line width: %4.1f", g_GLMinCartographicLineWidth);
    qDebug("OpenGL-> Minimum symbol line width: %4.1f", g_GLMinSymbolLineWidth);
    
    m_benableFog = true;
    m_benableVScale = true;
#ifdef __OCPN__ANDROID__
    m_benableFog = false;
    m_benableVScale = false;
#endif    
        
    if(!g_bGLexpert)
        g_GLOptions.m_bUseAcceleratedPanning =  !m_b_DisableFBO && m_b_BuiltFBO;
    
    if(1)     // for now upload all levels
    {
        int max_level = 0;
        int tex_dim = g_GLOptions.m_iTextureDimension;
        for(int dim=tex_dim; dim>0; dim/=2)
            max_level++;
        g_mipmap_max_level = max_level - 1;
    } 
    
    s_b_useFBO = m_b_BuiltFBO;

    // Some older Intel GL drivers need a glFinish() call after each full frame render
    if(b_oldIntel)
        g_b_needFinish = true;
    
    //  Inform the S52 PLIB of options determined
    if(ps52plib)
        ps52plib->SetGLOptions(s_b_useStencil, s_b_useStencilAP, s_b_useScissorTest,  s_b_useFBO, g_b_EnableVBO, g_texture_rectangle_format);
    
    m_bsetup = true;
    
    SendJSONConfigMessage();    
}

void glChartCanvas::SendJSONConfigMessage()
{
//    if(g_pi_manager){
//        wxJSONValue v;
//        v[_T("setupComplete")] =  m_bsetup;
//        v[_T("useStencil")] =  s_b_useStencil;
//        v[_T("useStencilAP")] =  s_b_useStencilAP;
//        v[_T("useScissorTest")] =  s_b_useScissorTest;
//        v[_T("useFBO")] =  s_b_useFBO;
//        v[_T("useVBO")] =  g_b_EnableVBO;
//        v[_T("TextureRectangleFormat")] =  g_texture_rectangle_format;
//        QString msg_id( _T("OCPN_OPENGL_CONFIG") );
//        g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
//    }
}
void glChartCanvas::SetupCompression()
{
    int dim = g_GLOptions.m_iTextureDimension;

#ifdef __WXMSW__    
    if(!::IsProcessorFeaturePresent( PF_XMMI64_INSTRUCTIONS_AVAILABLE )) {
        qDebug("OpenGL-> SSE2 Instruction set not available") );
        goto no_compression;
    }
#endif

    g_uncompressed_tile_size = dim*dim*4; // stored as 32bpp in vram
    if(!g_GLOptions.m_bTextureCompression)
        goto no_compression;

    g_raster_format = GL_RGB;
    
    // On GLES, we prefer OES_ETC1 compression, if available
#ifdef ocpnUSE_GLES
    if(QueryExtension("GL_OES_compressed_ETC1_RGB8_texture") && s_glCompressedTexImage2D) {
        g_raster_format = GL_ETC1_RGB8_OES;
    
        qDebug("OpenGL-> Using oes etc1 compression") );
    }
#endif
    
    if(GL_RGB == g_raster_format) {
        /* because s3tc is patented, many foss drivers disable
           support by default, however the extension dxt1 allows
           us to load this texture type which is enough because we
           compress in software using libsquish for superior quality anyway */

        if((QueryExtension("GL_EXT_texture_compression_s3tc") ||
            QueryExtension("GL_EXT_texture_compression_dxt1")) &&
           s_glCompressedTexImage2D) {
            /* buggy opensource nvidia driver, renders incorrectly,
               workaround is to use format with alpha... */
            if(GetRendererString().indexOf("Gallium" ) != -1 &&
               GetRendererString().indexOf("NV" ) != -1 )
                g_raster_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            else
                g_raster_format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            
            qDebug("OpenGL-> Using s3tc dxt1 compression" );
        } else if(QueryExtension("GL_3DFX_texture_compression_FXT1") &&
                  s_glCompressedTexImage2D && s_glGetCompressedTexImage) {
            g_raster_format = GL_COMPRESSED_RGB_FXT1_3DFX;
            
            qDebug("OpenGL-> Using 3dfx fxt1 compression" );
        } else {
            qDebug("OpenGL-> No Useable compression format found" );
            goto no_compression;
        }
    }

#ifdef ocpnUSE_GLES /* gles doesn't have GetTexLevelParameter */
    g_tile_size = 512*512/2; /* 4bpp */
#else
    /* determine compressed size of a level 0 single tile */
    GLuint texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D( GL_TEXTURE_2D, 0, g_raster_format, dim, dim,
                  0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
                             GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &g_tile_size);
    glDeleteTextures(1, &texture);
#endif

    /* disable texture compression if the tile size is 0 */
    if(g_tile_size == 0)
        goto no_compression;

    qDebug("OpenGL-> Compressed tile size: %dkb (%d:1)",
                                    g_tile_size / 1024,
                                    g_uncompressed_tile_size / g_tile_size);
    return;

no_compression:
    g_GLOptions.m_bTextureCompression = false;

    g_tile_size = g_uncompressed_tile_size;
    qDebug("OpenGL-> Not Using compression");
}

void glChartCanvas::paintGL()
{
    QTime   t;
    t.start();
    if(!m_pcontext) return;
    makeCurrent();
    
    if( !m_bsetup ) {
        SetupOpenGL();
        m_bsetup = true;
    }
    if(!m_b_paint_enable) return;
    if( m_in_glpaint ) return;

    UpdateCanvasS52PLIBConfig();
    
    m_in_glpaint++;
    Render();
    m_in_glpaint--;
//    qDebug()<<"update end elaped:"<<t.elapsed()<<" ms";

}


//   These routines allow reusable coordinates
bool glChartCanvas::HasNormalizedViewPort(const ViewPort &vp)
{
    return vp.projectType() == PROJECTION_MERCATOR ||
        vp.projectType() == PROJECTION_POLAR ||
        vp.projectType() == PROJECTION_EQUIRECTANGULAR;
}

/* adjust the opengl transformation matrix so that
   points plotted using the identity viewport are correct.
   and all rotation translation and scaling is now done in opengl

   a central lat and lon of 0, 0 can be used, however objects on the far side of the world
   can be up to 3 meters off because limited floating point precision, and if the
   points cross 180 longitude then two passes will be required to render them correctly */
#define NORM_FACTOR 4096.0
void glChartCanvas::MultMatrixViewPort(ViewPort &vp, float lat, float lon)
{
    zchxPointF point;

    switch(vp.projectType()) {
    case PROJECTION_MERCATOR:
    case PROJECTION_EQUIRECTANGULAR:
    case PROJECTION_WEB_MERCATOR:
        //mFrameWork->GetDoubleCanvasPointPixVP(vp, lat, lon, &point);
        point = vp.GetDoublePixFromLL(lat, lon);
        glTranslated(point.x, point.y, 0);
        glScaled(vp.viewScalePPM()/NORM_FACTOR, vp.viewScalePPM()/NORM_FACTOR, 1);
        break;

    case PROJECTION_POLAR:
        //mFrameWork->GetDoubleCanvasPointPixVP(vp, vp.lat() > 0 ? 90 : -90, vp.lon(), &point);
        point = vp.GetDoublePixFromLL(vp.lat() > 0 ? 90 : -90, vp.lon());
        glTranslated(point.x, point.y, 0);
        glRotatef(vp.lon() - lon, 0, 0, vp.lat());
        glScalef(vp.viewScalePPM()/NORM_FACTOR, vp.viewScalePPM()/NORM_FACTOR, 1);
        glTranslatef(-vp.pixWidth()/2, -vp.pixHeight()/2, 0);
        break;

    default:
        qDebug("ERROR: Unhandled projection\n");
    }

    double rotation = vp.rotation();

    if (rotation)
        glRotatef(rotation*180/PI, 0, 0, 1);
}

ViewPort glChartCanvas::NormalizedViewPort(const ViewPort &vp, float lat, float lon)
{
    ViewPort cvp = vp;

    switch(vp.projectType()) {
    case PROJECTION_MERCATOR:
    case PROJECTION_EQUIRECTANGULAR:
    case PROJECTION_WEB_MERCATOR:
        cvp.setLat(lat);
        break;

    case PROJECTION_POLAR:
        cvp.setLat(vp.lat() > 0 ? 90 : -90); // either north or south polar
        break;

    default:
        printf("ERROR: Unhandled projection\n");
    }

    cvp.setLon(lon);
    cvp.setViewScalePPM(NORM_FACTOR);
    cvp.setRotation(0);
    cvp.setSkew(0);
    return cvp;
}

bool glChartCanvas::CanClipViewport(const ViewPort &vp)
{
    return vp.projectType() == PROJECTION_MERCATOR || vp.projectType() == PROJECTION_WEB_MERCATOR ||
        vp.projectType() == PROJECTION_EQUIRECTANGULAR;
}

ViewPort glChartCanvas::ClippedViewport(const ViewPort &vp, const LLRegion &region)
{
    if(!CanClipViewport(vp))
        return vp;

    ViewPort cvp = vp;
    LLBBox bbox = region.GetBox();
    
    if(!bbox.GetValid())
        return vp;

    /* region.GetBox() will always try to give coordinates from -180 to 180 but in
       the case where the viewport crosses the IDL, we actually want the clipped viewport
       to use coordinates outside this range to ensure the logic in the various rendering
       routines works the same here (with accelerated panning) as it does without, so we
       can adjust the coordinates here */

    if(bbox.GetMaxLon() < cvp.getBBox().GetMinLon()) {
        bbox.Set(bbox.GetMinLat(), bbox.GetMinLon() + 360,
                 bbox.GetMaxLat(), bbox.GetMaxLon() + 360);
        cvp.setBBoxDirect(bbox);
    } else if(bbox.GetMinLon() > cvp.getBBox().GetMaxLon()) {
        bbox.Set(bbox.GetMinLat(), bbox.GetMinLon() - 360,
                 bbox.GetMaxLat(), bbox.GetMaxLon() - 360);
        cvp.setBBoxDirect(bbox);
    } else
        cvp.setBBoxDirect(bbox);

    return cvp;
}


void glChartCanvas::DrawStaticRoutesTracksAndWaypoints( ViewPort &vp )
{
#if 0
    if(!mFrameWork->m_bShowNavobjects)
        return;
    ocpnDC dc(*this);

    for(wxTrackListNode *node = pTrackList->GetFirst();
        node; node = node->GetNext() ) {
        Track *pTrackDraw = node->GetData();
                /* defer rendering active tracks until later */
        ActiveTrack *pActiveTrack = dynamic_cast<ActiveTrack *>(pTrackDraw);
        if(pActiveTrack && pActiveTrack->IsRunning() )
            continue;

        pTrackDraw->Draw( mFrameWork, dc, vp, vp.getBBox() );
    }
    
    for(wxRouteListNode *node = pRouteList->GetFirst();
        node; node = node->GetNext() ) {
        Route *pRouteDraw = node->GetData();

        if( !pRouteDraw )
            continue;
    
        /* defer rendering active routes until later */
        if( pRouteDraw->IsActive() || pRouteDraw->IsSelected() )
            continue;
    
        /* defer rendering routes being edited until later */
        if( pRouteDraw->m_bIsBeingEdited )
            continue;
    
        pRouteDraw->DrawGL( vp, mFrameWork );
    }
        
    /* Waypoints not drawn as part of routes, and not being edited */
    if( vp.getBBox().GetValid() && pWayPointMan) {
        for(wxRoutePointListNode *pnode = pWayPointMan->GetWaypointList()->GetFirst(); pnode; pnode = pnode->GetNext() ) {
            RoutePoint *pWP = pnode->GetData();
            if( pWP && (!pWP->m_bRPIsBeingEdited) &&(!pWP->m_bIsInRoute ) )
                if(vp.getBBox().ContainsMarge(pWP->m_lat, pWP->m_lon, .5))
                    pWP->DrawGL( vp, mFrameWork );
        }
    }
#endif
}

void glChartCanvas::DrawDynamicRoutesTracksAndWaypoints( ViewPort &vp )
{
#if 0
    ocpnDC dc(*this);

    for(wxTrackListNode *node = pTrackList->GetFirst();
        node; node = node->GetNext() ) {
        Track *pTrackDraw = node->GetData();
        ActiveTrack *pActiveTrack = dynamic_cast<ActiveTrack *>(pTrackDraw);
        if(pActiveTrack && pActiveTrack->IsRunning() )
            pTrackDraw->Draw( mFrameWork, dc, vp, vp.getBBox() );     // We need Track::Draw() to dynamically render last (ownship) point.
    }
    
    for(wxRouteListNode *node = pRouteList->GetFirst(); node; node = node->GetNext() ) {
        Route *pRouteDraw = node->GetData();
        
        int drawit = 0;
        if( !pRouteDraw )
            continue;
        
        /* Active routes */
        if( pRouteDraw->IsActive() || pRouteDraw->IsSelected() )
            drawit++;
                
        /* Routes being edited */
        if( pRouteDraw->m_bIsBeingEdited )
            drawit++;
        
        /* Routes Selected */
        if( pRouteDraw->IsSelected() )
            drawit++;
        
        if(drawit) {
            const LLBBox &vp_box = vp.getBBox(), &test_box = pRouteDraw->getBBox();
            if(!vp_box.IntersectOut(test_box))
                pRouteDraw->DrawGL( vp, mFrameWork );
        }
    }
    
    
    /* Waypoints not drawn as part of routes, which are being edited right now */
    if( vp.getBBox().GetValid() && pWayPointMan) {
        
        for(wxRoutePointListNode *pnode = pWayPointMan->GetWaypointList()->GetFirst(); pnode; pnode = pnode->GetNext() ) {
            RoutePoint *pWP = pnode->GetData();
            if( pWP && pWP->m_bRPIsBeingEdited && !pWP->m_bIsInRoute )
                pWP->DrawGL( vp, mFrameWork );
        }
    }
#endif
    
}

static void GetLatLonCurveDist(const ViewPort &vp, float &lat_dist, float &lon_dist)
{
    // This really could use some more thought, and possibly split at different
    // intervals based on chart skew and other parameters to optimize performance
    switch(vp.projectType()) {
    case PROJECTION_TRANSVERSE_MERCATOR:
        lat_dist = 4,   lon_dist = 1;        break;
    case PROJECTION_POLYCONIC:
        lat_dist = 2,   lon_dist = 1;        break;
    case PROJECTION_ORTHOGRAPHIC:
        lat_dist = 2,   lon_dist = 2;        break;
    case PROJECTION_POLAR:
        lat_dist = 180, lon_dist = 1;        break;
    case PROJECTION_STEREOGRAPHIC:
    case PROJECTION_GNOMONIC:
        lat_dist = 2, lon_dist = 1;          break;
    case PROJECTION_EQUIRECTANGULAR:
        // this is suboptimal because we don't care unless there is
        // a change in both lat AND lon (skewed chart)
        lat_dist = 2,   lon_dist = 360;      break;
    default:
        lat_dist = 180, lon_dist = 360;
    }
}

void glChartCanvas::RenderChartOutline( int dbIndex, ViewPort &vp )
{
    if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_PLUGIN &&
        !ChartData->IsChartAvailable( dbIndex ) )
        return;
        
    /* quick bounds check */
    LLBBox box;
    ChartData->GetDBBoundingBox( dbIndex, box );
    if(!box.GetValid())
        return;

    
    // Don't draw an outline in the case where the chart covers the entire world */
    if(box.GetLonRange() == 360)
        return;

    LLBBox vpbox = vp.getBBox();
    
    double lon_bias = 0;
    // chart is outside of viewport lat/lon bounding box
    if( box.IntersectOutGetBias( vp.getBBox(), lon_bias ) )
        return;

    float plylat, plylon;

    QColor color;

    if( ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_CM93 )
        color = GetGlobalColor( "YELO1"  );
    else if( ChartData->GetDBChartFamily( dbIndex ) == CHART_FAMILY_VECTOR )
        color = GetGlobalColor( "GREEN2" );
    else
        color = GetGlobalColor( "UINFR"  );

    if( g_GLOptions.m_GLLineSmoothing )
        glEnable( GL_LINE_SMOOTH );

    glColor3ub(color.red(), color.green(), color.blue());
    glLineWidth( g_GLMinSymbolLineWidth );

    float lat_dist, lon_dist;
    GetLatLonCurveDist(vp, lat_dist, lon_dist);

    //        Are there any aux ply entries?
    int nAuxPlyEntries = ChartData->GetnAuxPlyEntries( dbIndex ), nPly;
    int j=0;
    do {
        if(nAuxPlyEntries)
            nPly = ChartData->GetDBAuxPlyPoint( dbIndex, 0, j, 0, 0 );
        else
            nPly = ChartData->GetDBPlyPoint( dbIndex, 0, &plylat, &plylon );

        bool begin = false, sml_valid = false;
        double sml[2];
        float lastplylat = 0.0;
        float lastplylon = 0.0;
        // modulo is undefined for zero (compiler can use a div operation)
        int modulo = (nPly == 0)?1:nPly;
        for( int i = 0; i < nPly+1; i++ ) {
            if(nAuxPlyEntries)
                ChartData->GetDBAuxPlyPoint( dbIndex, i % modulo, j, &plylat, &plylon );
            else
                ChartData->GetDBPlyPoint( dbIndex, i % modulo, &plylat, &plylon );

            plylon += lon_bias;

            if(lastplylon - plylon > 180)
                lastplylon -= 360;
            else if(lastplylon - plylon < -180)
                lastplylon += 360;

            int splits;
            if(i==0)
                splits = 1;
            else {
                int lat_splits = floor(fabs(plylat-lastplylat) / lat_dist);
                int lon_splits = floor(fabs(plylon-lastplylon) / lon_dist);
                splits = fmax(lat_splits, lon_splits) + 1;
            }
                
            double smj[2];
            if(splits != 1) {
                // must perform border interpolation in mercator space as this is what the charts use
                toSM(plylat, plylon, 0, 0, smj+0, smj+1);
                if(!sml_valid)
                    toSM(lastplylat, lastplylon, 0, 0, sml+0, sml+1);
            }

            for(double c=0; c<splits; c++) {
                double lat, lon;
                if(c == splits - 1)
                    lat = plylat, lon = plylon;
                else {
                    double d = (double)(c+1) / splits;
                    fromSM(d*smj[0] + (1-d)*sml[0], d*smj[1] + (1-d)*sml[1], 0, 0, &lat, &lon);
                }

                zchxPointF s;
                mFrameWork->GetDoubleCanvasPointPix( lat, lon, s );
                if(!std::isnan(s.x)) {
                    if(!begin) {
                        begin = true;
                        glBegin(GL_LINE_STRIP);
                    }
                    glVertex2f( s.x, s.y );
                } else if(begin) {
                    glEnd();
                    begin = false;
                }
            }
            if((sml_valid = splits != 1))
                memcpy(sml, smj, sizeof smj);
            lastplylat = plylat, lastplylon = plylon;
        }

        if(begin)
            glEnd();

    } while(++j < nAuxPlyEntries );                 // There are no aux Ply Point entries

    glDisable( GL_LINE_SMOOTH );
//    glDisable( GL_BLEND );
}

ViewPort glChartCanvas::GetVP()
{
    return mFrameWork->GetVP();
}

extern void CalcGridSpacing( float WindowDegrees, float& MajorSpacing, float&MinorSpacing );
extern QString CalcGridText( float latlon, float spacing, bool bPostfix );
void glChartCanvas::GridDraw( )
{
    if( !m_bDisplayGrid ) return;

    ViewPort &vp = mFrameWork->GetVP();

    // TODO: make minor grid work all the time
    bool minorgrid = fabs( vp.rotation() ) < 0.0001 &&
        vp.projectType() == PROJECTION_MERCATOR;

    double nlat, elon, slat, wlon;
    float lat, lon;
    float gridlatMajor, gridlatMinor, gridlonMajor, gridlonMinor;
    int w, h;
    
    QColor GridColor = GetGlobalColor( "SNDG1"  );

    if(!m_gridfont.IsBuilt()){
        QFont dFont = FontMgr::Get().GetFont( ("ChartTexts"), 0 );
        QFont font = dFont;
        font.setPointSize(8);
        font.setWeight(QFont::Weight::Normal);
        
        m_gridfont.Build(font);
    }

    w = vp.pixWidth();
    h = vp.pixHeight();

    LLBBox llbbox = vp.getBBox();
    nlat = llbbox.GetMaxLat();
    slat = llbbox.GetMinLat();
    elon = llbbox.GetMaxLon();
    wlon = llbbox.GetMinLon();

    // calculate distance between latitude grid lines
    CalcGridSpacing( vp.viewScalePPM(), gridlatMajor, gridlatMinor );
    CalcGridSpacing( vp.viewScalePPM(), gridlonMajor, gridlonMinor );


    // if it is known the grid has straight lines it's a bit faster
    bool straight_latitudes =
        vp.projectType() == PROJECTION_MERCATOR ||
        vp.projectType() == PROJECTION_WEB_MERCATOR ||
        vp.projectType() == PROJECTION_EQUIRECTANGULAR;
    bool straight_longitudes =
        vp.projectType() == PROJECTION_MERCATOR ||
        vp.projectType() == PROJECTION_WEB_MERCATOR ||
        vp.projectType() == PROJECTION_POLAR ||
        vp.projectType() == PROJECTION_EQUIRECTANGULAR;

    double latmargin;
    if(straight_latitudes)
        latmargin = 0;
    else
        latmargin = gridlatMajor / 2; // don't draw on poles

    slat = fmax(slat, -90 + latmargin);
    nlat = fmin(nlat,  90 - latmargin);

    float startlat = ceil( slat / gridlatMajor ) * gridlatMajor;
    float startlon = ceil( wlon / gridlonMajor ) * gridlonMajor;
    float curved_step = fmin(sqrt(5e-3 / vp.viewScalePPM()), 3);

    ocpnDC gldc( this );
    QPen pen( GridColor, g_GLMinSymbolLineWidth, Qt::SolidLine );
    gldc.SetPen( pen );
    
    // Draw Major latitude grid lines and text

    // calculate position of first major latitude grid line
    float lon_step = elon - wlon;
    if(!straight_latitudes)
        lon_step /= ceil(lon_step / curved_step);

    for(lat = startlat; lat < nlat; lat += gridlatMajor) {
        zchxPointF r, s;
        s.x = NAN;

        for(lon = wlon; lon < elon+lon_step/2; lon += lon_step) {
            mFrameWork->GetDoubleCanvasPointPix( lat, lon, r );
            if(!std::isnan(s.x) && !std::isnan(r.x)) {
                gldc.DrawLine( s.x, s.y, r.x, r.y, true );
            }
            s = r;
        }
    }

    if(minorgrid) {
        // draw minor latitude grid lines
        for(lat = ceil( slat / gridlatMinor ) * gridlatMinor; lat < nlat; lat += gridlatMinor) {
        
            zchxPoint r;
            mFrameWork->GetCanvasPointPix( lat, ( elon + wlon ) / 2, r );
            gldc.DrawLine( 0, r.y, 10, r.y, true );
            gldc.DrawLine( w - 10, r.y, w, r.y, true );
            
            lat = lat + gridlatMinor;
        }
    }

    // draw major longitude grid lines
    float lat_step = nlat - slat;
    if(!straight_longitudes)
        lat_step /= ceil(lat_step / curved_step);

    for(lon = startlon; lon < elon; lon += gridlonMajor) {
        zchxPointF r, s;
        s.x = NAN;
        for(lat = slat; lat < nlat+lat_step/2; lat+=lat_step) {
            mFrameWork->GetDoubleCanvasPointPix( lat, lon, r );

            if(!std::isnan(s.x) && !std::isnan(r.x)) {
                gldc.DrawLine( s.x, s.y, r.x, r.y, true );
            }
            s = r;
        }
    }

    if(minorgrid) {
        // draw minor longitude grid lines
        for(lon = ceil( wlon / gridlonMinor ) * gridlonMinor; lon < elon; lon += gridlonMinor) {
            zchxPoint r;
            mFrameWork->GetCanvasPointPix( ( nlat + slat ) / 2, lon, r );
            gldc.DrawLine( r.x, 0, r.x, 10, true );
            gldc.DrawLine( r.x, h-10, r.x, h, true );
        }
    }

    
    // draw text labels
    glEnable(GL_TEXTURE_2D);
    glEnable( GL_BLEND );
    for(lat = startlat; lat < nlat; lat += gridlatMajor) {
        if( fabs( lat - qRound( lat ) ) < 1e-5 )
            lat = qRound( lat );

        QString st = CalcGridText( lat, gridlatMajor, true ); // get text for grid line
        int iy;
        m_gridfont.GetTextExtent(st, 0, &iy);

        if(straight_latitudes) {
            zchxPoint r, s;
            mFrameWork->GetCanvasPointPix( lat, elon, r );
            mFrameWork->GetCanvasPointPix( lat, wlon, s );
        
            float x = 0, y = -1;
            y = (float)(r.y*s.x - s.y*r.x) / (s.x - r.x);
            if(y < 0 || y > h) {
                y = h - iy;
                x = (float)(r.x*s.y - s.x*r.y + (s.x - r.x)*y) / (s.y - r.y);
            }

            m_gridfont.RenderString(st, x, y);
        } else {
            // iteratively attempt to find where the latitude line crosses x=0
            zchxPointF r;
            double y1, y2, lat1, lon1, lat2, lon2;

            y1 = 0, y2 = vp.pixHeight();
            double error = vp.pixWidth(), lasterror;
            int maxiters = 10;
            do {
                mFrameWork->GetCanvasPixPoint(0, y1, lat1, lon1);
                mFrameWork->GetCanvasPixPoint(0, y2, lat2, lon2);

                double y = y1 + (lat1 - lat) * (y2 - y1) / (lat1 - lat2);

                mFrameWork->GetDoubleCanvasPointPix( lat, lon1 + (y1 - y) * (lon2 - lon1) / (y1 - y2), r);

                if(fabs(y - y1) < fabs(y - y2))
                    y1 = y;
                else
                    y2 = y;

                lasterror = error;
                error = fabs(r.x);
                if(--maxiters == 0)
                    break;
            } while(error > 1 && error < lasterror);

            if(error < 1 && r.y >= 0 && r.y <= vp.pixHeight() - iy )
                r.x = 0;
            else
                // just draw at center longitude
                mFrameWork->GetDoubleCanvasPointPix( lat, vp.lon(), r);

            m_gridfont.RenderString(st, r.x, r.y);
        }
    }


    for(lon = startlon; lon < elon; lon += gridlonMajor) {
        if( fabs( lon - qRound( lon ) ) < 1e-5 )
            lon = qRound( lon );

        zchxPoint r, s;
        mFrameWork->GetCanvasPointPix( nlat, lon, r );
        mFrameWork->GetCanvasPointPix( slat, lon, s );

        float xlon = lon;
        if( xlon > 180.0 )
            xlon -= 360.0;
        else if( xlon <= -180.0 )
            xlon += 360.0;
        
        QString st = CalcGridText( xlon, gridlonMajor, false );
        int ix;
        m_gridfont.GetTextExtent(st, &ix, 0);

        if(straight_longitudes) {
            float x = -1, y = 0;
            x = (float)(r.x*s.y - s.x*r.y) / (s.y - r.y);
            if(x < 0 || x > w) {
                x = w - ix;
                y = (float)(r.y*s.x - s.y*r.x + (s.y - r.y)*x) / (s.x - r.x);
            }
            
            m_gridfont.RenderString(st, x, y);
        } else {
            // iteratively attempt to find where the latitude line crosses x=0
            zchxPointF r;
            double x1, x2, lat1, lon1, lat2, lon2;

            x1 = 0, x2 = vp.pixWidth();
            double error = vp.pixHeight(), lasterror;
            do {
                mFrameWork->GetCanvasPixPoint(x1, 0, lat1, lon1);
                mFrameWork->GetCanvasPixPoint(x2, 0, lat2, lon2);

                double x = x1 + (lon1 - lon) * (x2 - x1) / (lon1 - lon2);

                mFrameWork->GetDoubleCanvasPointPix( lat1 + (x1 - x) * (lat2 - lat1) / (x1 - x2), lon, r);

                if(fabs(x - x1) < fabs(x - x2))
                    x1 = x;
                else
                    x2 = x;

                lasterror = error;
                error = fabs(r.y);
            } while(error > 1 && error < lasterror);

            if(error < 1 && r.x >= 0 && r.x <= vp.pixWidth() - ix)
                r.y = 0;
            else
                // failure, instead just draw the text at center latitude
                mFrameWork->GetDoubleCanvasPointPix( fmin(fmax(vp.lat(), slat), nlat), lon, r);

            m_gridfont.RenderString(st, r.x, r.y);
        }
    }

    glDisable(GL_TEXTURE_2D);

    glDisable( GL_BLEND );
}


void glChartCanvas::DrawEmboss( emboss_data *emboss  )
{
    if( !emboss ) return;
    
    int w = emboss->width, h = emboss->height;
    
    glEnable( GL_TEXTURE_2D );
    
    // render using opengl and alpha blending
    if( !emboss->gltexind ) { /* upload to texture */

        emboss->glwidth = NextPow2(emboss->width);
        emboss->glheight = NextPow2(emboss->height);
                
        /* convert to luminance alpha map */
        int size = emboss->glwidth * emboss->glheight;
        char *data = new char[2 * size];
        for( int i = 0; i < h; i++ ) {
            for( int j = 0; j < emboss->glwidth; j++ ) {
                if( j < w ) {
                    data[2 * ( ( i * emboss->glwidth ) + j )] =
                        (char) (emboss->pmap[( i * w ) + j] > 0 ? 0 : 255);
                    data[2 * ( ( i * emboss->glwidth ) + j ) + 1] = 
                        (char) abs( (emboss->pmap[( i * w ) + j]) );
                }
            }
        }

        glGenTextures( 1, &emboss->gltexind );
        glBindTexture( GL_TEXTURE_2D, emboss->gltexind );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, emboss->glwidth, emboss->glheight, 0,
                      GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        
        delete[] data;
    }
    
    glBindTexture( GL_TEXTURE_2D, emboss->gltexind );
    
    glEnable( GL_BLEND );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    
    const float factor = 200;
    glColor4f( 1, 1, 1, factor / 256 );
    
    int x = emboss->x, y = emboss->y;

    float wp = (float) w / emboss->glwidth;
    float hp = (float) h / emboss->glheight;
    
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 ), glVertex2i( x, y );
    glTexCoord2f( wp, 0 ), glVertex2i( x + w, y );
    glTexCoord2f( wp, hp ), glVertex2i( x + w, y + h );
    glTexCoord2f( 0, hp ), glVertex2i( x, y + h );
    glEnd();
    
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
}


void glChartCanvas::DrawFloatingOverlayObjects( ocpnDC &dc )
{
    ViewPort &vp = mFrameWork->GetVP();
    GridDraw( );
    ScaleBarDraw( dc );
}

void glChartCanvas::DrawQuiting()
{
    GLubyte pattern[8][8];
    for( int y = 0; y < 8; y++ )
        for( int x = 0; x < 8; x++ ) 
            pattern[y][x] = (y == x) * 255;

    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glBindTexture(GL_TEXTURE_2D, 0);

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 8, 8,
                  0, GL_ALPHA, GL_UNSIGNED_BYTE, pattern );
    glColor3f( 0, 0, 0 );

    float x = width(), y = height();
    float u = x / 8, v = y / 8;

    glBegin( GL_QUADS );
    glTexCoord2f(0, 0); glVertex2f( 0, 0 );
    glTexCoord2f(0, v); glVertex2f( 0, y );
    glTexCoord2f(u, v); glVertex2f( x, y );
    glTexCoord2f(u, 0); glVertex2f( x, 0 );
    glEnd();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void glChartCanvas::DrawCloseMessage(QString msg)
{
    if(1){
        
        QFont pfont = FontMgr::Get().FindOrCreateFont(12, "Microsoft YaHei", QFont::StyleNormal, QFont::Weight::Bold);
        
        TexFont texfont;
        
        texfont.Build(pfont);
        int w, h;
        texfont.GetTextExtent( msg, &w, &h);
        h += 2;
        int yp = mFrameWork->GetVP().pixHeight()/2;
        int xp = (mFrameWork->GetVP().pixWidth() - w)/2;
        
        glColor3ub( 243, 229, 47 );
        
        glBegin(GL_QUADS);
        glVertex2i(xp, yp);
        glVertex2i(xp+w, yp);
        glVertex2i(xp+w, yp+h);
        glVertex2i(xp, yp+h);
        glEnd();
        
        glEnable(GL_BLEND);
        
        glColor3ub( 0, 0, 0 );
        glEnable(GL_TEXTURE_2D);
        texfont.RenderString( msg, xp, yp);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);        
    }
}

void glChartCanvas::RotateToViewPort(const ViewPort &vp)
{
    float angle = vp.rotation();

//    qDebug()<<"current roate:"<<vp.rotation();
    if( fabs( angle ) > 0.0001 )
    {
        //    Rotations occur around 0,0, so translate to rotate around screen center
        float xt = vp.pixWidth() / 2.0, yt = vp.pixHeight() / 2.0;
        qDebug()<<"dxy:"<<xt<<yt;
        glTranslatef( xt, yt, 0 );
        glRotatef( angle * 180. / PI, 0, 0, 1 );
        glTranslatef( -xt, -yt, 0 );
    }
}

static std::list<double*> combine_work_data;
static void combineCallbackD(GLdouble coords[3],
                             GLdouble *vertex_data[4],
                             GLfloat weight[4], GLdouble **dataOut )
{
    double *vertex = new double[3];
    combine_work_data.push_back(vertex);
    memcpy(vertex, coords, 3*(sizeof *coords)); 
    *dataOut = vertex;
}

void vertexCallbackD(GLvoid *vertex)
{
    glVertex3dv( (GLdouble *)vertex);
}

void beginCallbackD( GLenum mode)
{
    glBegin( mode );
}

void endCallbackD()
{
    glEnd();
}

void glChartCanvas::DrawRegion(ViewPort &vp, const LLRegion &region)
{
    float lat_dist, lon_dist;
    GetLatLonCurveDist(vp, lat_dist, lon_dist);

    GLUtesselator *tobj = gluNewTess();

    gluTessCallback( tobj, GLU_TESS_VERTEX, (_GLUfuncptr) &vertexCallbackD  );
    gluTessCallback( tobj, GLU_TESS_BEGIN, (_GLUfuncptr) &beginCallbackD  );
    gluTessCallback( tobj, GLU_TESS_END, (_GLUfuncptr) &endCallbackD  );
    gluTessCallback( tobj, GLU_TESS_COMBINE, (_GLUfuncptr) &combineCallbackD );
    
    gluTessNormal( tobj, 0, 0, 1);
    
    gluTessBeginPolygon(tobj, NULL);
    for(std::list<poly_contour>::const_iterator i = region.contours.begin(); i != region.contours.end(); i++) {
        gluTessBeginContour(tobj);
        contour_pt l = *i->rbegin();
        double sml[2];
        bool sml_valid = false;
        for(poly_contour::const_iterator j = i->begin(); j != i->end(); j++) {
            int lat_splits = floor(fabs(j->y - l.y) / lat_dist);
            int lon_splits = floor(fabs(j->x - l.x) / lon_dist);
            int splits = fmax(lat_splits, lon_splits) + 1;

            double smj[2];
            if(splits != 1) {
                // must perform border interpolation in mercator space as this is what the charts use
                toSM(j->y, j->x, 0, 0, smj+0, smj+1);
                if(!sml_valid)
                    toSM(l.y, l.x, 0, 0, sml+0, sml+1);
            }

            for(int i = 0; i<splits; i++) {
                double lat, lon;
                if(i == splits - 1)
                    lat = j->y, lon = j->x;
                else {
                    double d = (double)(i+1) / splits;
                    fromSM(d*smj[0] + (1-d)*sml[0], d*smj[1] + (1-d)*sml[1], 0, 0, &lat, &lon);
                }
                zchxPointF q = vp.GetDoublePixFromLL(lat, lon);
                if(std::isnan(q.x))
                    continue;

                double *p = new double[6];
                p[0] = q.x, p[1] = q.y, p[2] = 0;
                gluTessVertex(tobj, p, p);
                combine_work_data.push_back(p);
            }
            l = *j;

            if((sml_valid = splits != 1))
                memcpy(sml, smj, sizeof smj);
        }
        gluTessEndContour(tobj);
    }
    gluTessEndPolygon(tobj);

    gluDeleteTess(tobj);

    for(std::list<double*>::iterator i = combine_work_data.begin(); i!=combine_work_data.end(); i++)
        delete [] *i;
    combine_work_data.clear();
}

/* set stencil buffer to clip in this region, and optionally clear using the current color */
void glChartCanvas::SetClipRegion(ViewPort &vp, const LLRegion &region)
{
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer

    if( s_b_useStencil ) {
        //    Create a stencil buffer for clipping to the region
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x1 );                 // write only into bit 0 of the stencil buffer
        glClear( GL_STENCIL_BUFFER_BIT );

        //    We are going to write "1" into the stencil buffer wherever the region is valid
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
    } else              //  Use depth buffer for clipping
    {
        glEnable( GL_DEPTH_TEST ); // to enable writing to the depth buffer
        glDepthFunc( GL_ALWAYS );  // to ensure everything you draw passes
        glDepthMask( GL_TRUE );    // to allow writes to the depth buffer

        glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start

        //    Decompose the region into rectangles, and draw as quads
        //    With z = 1
            // dep buffer clear = 1
            // 1 makes 0 in dep buffer, works
            // 0 make .5 in depth buffer
            // -1 makes 1 in dep buffer

            //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
            //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
            //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
        glTranslatef( 0, 0, .5 );
    }

    DrawRegion(vp, region);

    if( s_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    } else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
        glTranslatef( 0, 0, -.5 ); // reset translation
    }

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer
}

void glChartCanvas::SetClipRect(const ViewPort &vp, const QRect &rect, bool b_clear)
{
    /* for some reason this causes an occasional bug in depth mode, I cannot
       seem to solve it yet, so for now: */
    if(s_b_useStencil && s_b_useScissorTest) {
        QRect vp_rect(0, 0, vp.pixWidth(), vp.pixHeight());
        if(rect != vp_rect) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(rect.x(), vp.pixHeight() - rect.height() - rect.y(), rect.width(), rect.height());
        }

        if(b_clear) {
            glBegin(GL_QUADS);
            glVertex2i( rect.x(), rect.y() );
            glVertex2i( rect.x() + rect.width(), rect.y() );
            glVertex2i( rect.x() + rect.width(), rect.y() + rect.height() );
            glVertex2i( rect.x(), rect.y() + rect.height() );
            glEnd();
        }

        /* the code in s52plib depends on the depth buffer being
           initialized to this value, this code should go there instead and
           only a flag set here. */
        if(!s_b_useStencil) {
            glClearDepth( 0.25 );
            glDepthMask( GL_TRUE );    // to allow writes to the depth buffer
            glClear( GL_DEPTH_BUFFER_BIT );
            glDepthMask( GL_FALSE );
            glClearDepth( 1 ); // set back to default of 1
            glDepthFunc( GL_GREATER );                          // Set the test value
        }
        return;
    }

    // slower way if there is no scissor support
    if(!b_clear)
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer

    if( s_b_useStencil ) {
        //    Create a stencil buffer for clipping to the region
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x1 );                 // write only into bit 0 of the stencil buffer
        glClear( GL_STENCIL_BUFFER_BIT );

        //    We are going to write "1" into the stencil buffer wherever the region is valid
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
    } else              //  Use depth buffer for clipping
    {
        glEnable( GL_DEPTH_TEST ); // to enable writing to the depth buffer
        glDepthFunc( GL_ALWAYS );  // to ensure everything you draw passes
        glDepthMask( GL_TRUE );    // to allow writes to the depth buffer

        glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start

        //    Decompose the region into rectangles, and draw as quads
        //    With z = 1
            // dep buffer clear = 1
            // 1 makes 0 in dep buffer, works
            // 0 make .5 in depth buffer
            // -1 makes 1 in dep buffer

            //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
            //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
            //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
        glTranslatef( 0, 0, .5 );
    }

    glBegin(GL_QUADS);
    glVertex2i( rect.x(), rect.y() );
    glVertex2i( rect.x() + rect.width(), rect.y() );
    glVertex2i( rect.x() + rect.width(), rect.y() + rect.height() );
    glVertex2i( rect.x(), rect.y() + rect.height() );
    glEnd();
    
    if( s_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    } else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
        glTranslatef( 0, 0, -.5 ); // reset translation
    }

    if(!b_clear)
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer
}

void glChartCanvas::DisableClipRegion()
{
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_DEPTH_TEST );
}

void glChartCanvas::Invalidate()
{
    m_cache_vp.invalidate();

}

void glChartCanvas::RenderRasterChartRegionGL( ChartBase *chart, ViewPort &vp, LLRegion &region )
{
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( chart );
    if( !pBSBChart ) return;

    if(b_inCompressAllCharts) return; // don't want multiple texfactories to exist
    

    //    Look for the texture factory for this chart
    QString key = chart->GetHashKey();
    
    glTexFactory *pTexFact;
    ChartPathHashTexfactType &hash = g_glTextureManager->m_chart_texfactory_hash;
    ChartPathHashTexfactType::iterator ittf = hash.find( key );
    
    //    Not Found ?
    if( ittf == hash.end() ){
        hash[key] = new glTexFactory(chart, g_raster_format);
        hash[key]->SetHashKey(key);
    }
    
    pTexFact = hash[key];
    pTexFact->SetLRUTime(++m_LRUtime);
    
    // for small scales, don't use normalized coordinates for accuracy (difference is up to 3 meters error)
    bool use_norm_vp = glChartCanvas::HasNormalizedViewPort(vp) && pBSBChart->GetPPM() < 1;
    pTexFact->PrepareTiles(vp, use_norm_vp, pBSBChart);

    //    For underzoom cases, we will define the textures as having their base levels
    //    equivalent to a level "n" mipmap, where n is calculated, and is always binary
    //    This way we can avoid loading much texture memory

    int base_level;
    if(vp.projectType() == PROJECTION_MERCATOR &&
       chart->GetChartProjectionType() == PROJECTION_MERCATOR) {
        double scalefactor = pBSBChart->GetRasterScaleFactor(vp);
        base_level = log(scalefactor) / log(2.0);

        if(base_level < 0) /* for overzoom */
            base_level = 0;
        if(base_level > g_mipmap_max_level)
            base_level = g_mipmap_max_level;
    } else
        base_level = 0; // base level should be computed per tile, for now load all

    /* setup opengl parameters */
    glEnable( GL_TEXTURE_2D );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if(use_norm_vp) {
        glPushMatrix();
        double lat, lon;
        pTexFact->GetCenter(lat, lon);
        MultMatrixViewPort(vp, lat, lon);
    }

    LLBBox box = region.GetBox();
    int numtiles;
    int mem_used = 0;
    if (g_memCacheLimit > 0) {
        // GetMemoryStatus is slow on linux
        zchxFuncUtil::getMemoryStatus(0, &mem_used);
    }

    glTexTile **tiles = pTexFact->GetTiles(numtiles);
    for(int i = 0; i<numtiles; i++) {
        glTexTile *tile = tiles[i];
        if(region.IntersectOut(tile->box)) {
            
            /*   user setting is in MB while we count exact bytes */
            bool bGLMemCrunch = g_tex_mem_used > g_GLOptions.m_iTextureMemorySize * 1024 * 1024;
            if( bGLMemCrunch)
                pTexFact->DeleteTexture( tile->rect );
        } else {
            bool texture = pTexFact->PrepareTexture( base_level, tile->rect, global_color_scheme, mem_used );
            if(!texture) { // failed to load, draw red
                glDisable(GL_TEXTURE_2D);
                glColor3f(1, 0, 0);
            }

            float *coords;
            if(use_norm_vp)
                coords = tile->m_coords;
            else {
                coords = new float[2 * tile->m_ncoords];
                for(int i=0; i<tile->m_ncoords; i++) {
                    zchxPointF p = vp.GetDoublePixFromLL(tile->m_coords[2*i+0],
                                                              tile->m_coords[2*i+1]);
                    coords[2*i+0] = p.x;
                    coords[2*i+1] = p.y;
                }
            }

            glTexCoordPointer(2, GL_FLOAT, 2*sizeof(GLfloat), tile->m_texcoords);
            glVertexPointer(2, GL_FLOAT, 2*sizeof(GLfloat), coords);
            glDrawArrays(GL_QUADS, 0, tile->m_ncoords);

            if(!texture)
                glEnable(GL_TEXTURE_2D);

            if(!use_norm_vp)
                delete [] coords;
        }
    }

    glDisable(GL_TEXTURE_2D);

    if(use_norm_vp)
        glPopMatrix();

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void glChartCanvas::RenderQuiltViewGL( ViewPort &vp, const OCPNRegion &rect_region )
{
    if( !mFrameWork->m_pQuilt->GetnCharts() || mFrameWork->m_pQuilt->IsBusy() )
        return;

    //  render the quilt
        ChartBase *chart = mFrameWork->m_pQuilt->GetFirstChart();
        
    //  Check the first, smallest scale chart
    if(chart) {
        //            if( ! mFrameWork->IsChartLargeEnoughToRender( chart, vp ) )
//            chart = NULL;
    }

    LLRegion region = vp.GetLLRegion(rect_region);

    LLRegion rendered_region;
    while( chart ) {
            
        //  This test does not need to be done for raster charts, since
        //  we can assume that texture binding is acceptably fast regardless of the render region,
        //  and that the quilt zoom methods choose a reasonable reference chart.
        if(chart->GetChartFamily() != CHART_FAMILY_RASTER)
        {
            //                if( ! mFrameWork->IsChartLargeEnoughToRender( chart, vp ) ) {
                //                    chart = mFrameWork->m_pQuilt->GetNextChart();
//                    continue;
//                }
        }

        QuiltPatch *pqp = mFrameWork->m_pQuilt->GetCurrentPatch();
        if( pqp->b_Valid ) {
            LLRegion get_region = pqp->ActiveRegion;
            bool b_rendered = false;

            if( !pqp->b_overlay ) {
                get_region.Intersect( region );
                if( !get_region.Empty() ) {
                    if( chart->GetChartFamily() == CHART_FAMILY_RASTER ) {
                        
                        ChartBaseBSB *Patch_Ch_BSB = dynamic_cast<ChartBaseBSB*>( chart );
                        if (Patch_Ch_BSB) {
                            SetClipRegion(vp, pqp->ActiveRegion/*pqp->quilt_region*/);
                            RenderRasterChartRegionGL( chart, vp, get_region );
                            DisableClipRegion();
                            b_rendered = true;
                        }
                        else if(chart->GetChartType() == CHART_TYPE_MBTILES){
                            SetClipRegion(vp, pqp->ActiveRegion/*pqp->quilt_region*/);
                            chart->RenderRegionViewOnGL( m_pcontext, vp, rect_region, get_region );
                            DisableClipRegion();
                        }
                        
                    } else if(chart->GetChartFamily() == CHART_FAMILY_VECTOR ) {

                        if(chart->GetChartType() == CHART_TYPE_CM93COMP){
                           RenderNoDTA(vp, get_region);
                           chart->RenderRegionViewOnGL( m_pcontext, vp, rect_region, get_region );
                        }
                        else{
                            s57chart *Chs57 = dynamic_cast<s57chart*>( chart );
                            if(Chs57){
                                if(Chs57->m_RAZBuilt){
                                    RenderNoDTA(vp, get_region);
                                    Chs57->RenderRegionViewOnGLNoText(m_pcontext, vp, rect_region, get_region );
                                    DisableClipRegion();
                                }
                                else{
                                    // The SENC is quesed for building, so..
                                    // Show GSHHS with compatible color scheme in the meantime.
                                    ocpnDC gldc( this );
                                    const LLRegion &oregion = get_region;
                                    LLBBox box = oregion.GetBox();

                                    zchxPoint p1 = vp.GetPixFromLL( box.GetMaxLat(), box.GetMinLon());
                                    zchxPoint p2 = vp.GetPixFromLL( box.GetMaxLat(), box.GetMaxLon());
                                    zchxPoint p3 = vp.GetPixFromLL( box.GetMinLat(), box.GetMaxLon());
                                    zchxPoint p4 = vp.GetPixFromLL( box.GetMinLat(), box.GetMinLon());
                                        
                                    QRect srect(p1.x, p1.y, p3.x - p1.x, p4.y - p2.y);

                                    bool world = false;
                                    ViewPort cvp = ClippedViewport(vp, get_region);
                                    if( GetWorldBackgroundChart()){
                                        SetClipRegion(cvp, get_region);
                                        GetWorldBackgroundChart()->SetColorsDirect(GetGlobalColor( ( "LANDA" ) ), GetGlobalColor( ( "DEPMS" )));
                                        RenderWorldChart(gldc, cvp, srect, world);
                                        GetWorldBackgroundChart()->SetColorScheme(global_color_scheme);
                                        DisableClipRegion();
                                    }
                                }
                            }
                            else{
                                ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                                if(ChPI){
                                    RenderNoDTA(vp, get_region);
                                    ChPI->RenderRegionViewOnGLNoText(m_pcontext, vp, rect_region, get_region );
                                }
                                else{    
                                    RenderNoDTA(vp, get_region);
                                    chart->RenderRegionViewOnGL(m_pcontext, vp, rect_region, get_region );
                                }
                            }
                        }
                     }
                }
            }

            if(b_rendered) {
//                LLRegion get_region = pqp->ActiveRegion;
//                    get_region.Intersect( Region );  not technically required?
//                rendered_region.Union(get_region);
            }
        }


        chart = mFrameWork->m_pQuilt->GetNextChart();
    }

    //    Render any Overlay patches for s57 charts(cells)
    if( mFrameWork->m_pQuilt->HasOverlays() ) {
        ChartBase *pch = mFrameWork->m_pQuilt->GetFirstChart();
        while( pch ) {
            QuiltPatch *pqp = mFrameWork->m_pQuilt->GetCurrentPatch();
            if( pqp->b_Valid && pqp->b_overlay && pch->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                LLRegion get_region = pqp->ActiveRegion;

                get_region.Intersect( region );
                if( !get_region.Empty()  ) {
                    s57chart *Chs57 = dynamic_cast<s57chart*>( pch );
                    if( Chs57 )
                        Chs57->RenderOverlayRegionViewOnGL(m_pcontext, vp, rect_region, get_region );
                    else{
                        ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( pch );
                        if(ChPI){
                            ChPI->RenderRegionViewOnGL(m_pcontext, vp, rect_region, get_region );
                        }
                    }

                }
            }

            pch = mFrameWork->m_pQuilt->GetNextChart();
        }
    }

    // Hilite rollover patch
    LLRegion hiregion = mFrameWork->m_pQuilt->GetHiliteRegion();

    if( !hiregion.Empty() ) {
        glEnable( GL_BLEND );

        double hitrans;
        switch( global_color_scheme ) {
        case GLOBAL_COLOR_SCHEME_DAY:
            hitrans = .4;
            break;
        case GLOBAL_COLOR_SCHEME_DUSK:
            hitrans = .2;
            break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
            hitrans = .1;
            break;
        default:
            hitrans = .4;
            break;
        }

        glColor4f( (float) .8, (float) .4, (float) .4, (float) hitrans );

        DrawRegion(vp, hiregion);

        glDisable( GL_BLEND );
    }
    mFrameWork->m_pQuilt->SetRenderedVP( vp );

#if 0    
    if(m_bfogit) {
        double scale_factor = vp.ref_scale/vp.chart_scale;
        float fog = ((scale_factor - g_overzoom_emphasis_base) * 255.) / 20.;
        fog = fmin(fog, 200.);         // Don't blur completely
            
        if( !rendered_region.Empty() ) {
     
            int width = vp.pixWidth();
            int height = vp.pixHeight();
                
            // Use MipMap LOD tweaking to produce a blurred, downsampling effect at reasonable speed.

            if( (s_glGenerateMipmap) && (g_texture_rectangle_format == GL_TEXTURE_2D)){       //nPOT texture supported

                //          Capture the rendered screen image to a texture
                glReadBuffer( GL_BACK);
                        
                GLuint screen_capture;
                glGenTextures( 1, &screen_capture );
                        
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, screen_capture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
                glCopyTexSubImage2D(GL_TEXTURE_2D,  0,  0,  0, 0,  0,  width, height);
                    
                glClear(GL_DEPTH_BUFFER_BIT);
                glDisable(GL_DEPTH_TEST);
                        
                //  Build MipMaps 
                int max_mipmap = 3;
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_mipmap );
                        
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, max_mipmap);
                        
                glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
                        
                s_glGenerateMipmap(GL_TEXTURE_2D);

                // Render at reduced LOD (i.e. higher mipmap number)
                double bias = fog/70;
                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias);
                glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
                        

                glBegin(GL_QUADS);
                        
                glTexCoord2f(0 , 1 ); glVertex2i(0,     0);
                glTexCoord2f(0 , 0 ); glVertex2i(0,     height);
                glTexCoord2f(1 , 0 ); glVertex2i(width, height);
                glTexCoord2f(1 , 1 ); glVertex2i(width, 0);
                glEnd ();
                        
                glDeleteTextures(1, &screen_capture);

                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
                glDisable(GL_TEXTURE_2D);
            }
                    
            else if(scale_factor > 20){ 
                // Fogging by alpha blending                
                fog = ((scale_factor - 20) * 255.) / 20.;
            
                glEnable( GL_BLEND );
                    
                fog = fmin(fog, 150.);         // Don't fog out completely
                    
                QColor color = mFrameWork->GetFogColor();
                glColor4ub( color.Red(), color.Green(), color.Blue(), (int)fog );

                DrawRegion(vp, rendered_region);
                glDisable( GL_BLEND );
            }
        }
    }
#endif    
}

void glChartCanvas::RenderQuiltViewGLText( ViewPort &vp, const OCPNRegion &rect_region )
{
    if( !mFrameWork->m_pQuilt->GetnCharts() || mFrameWork->m_pQuilt->IsBusy() )
        return;
    
    //  render the quilt
        ChartBase *chart = mFrameWork->m_pQuilt->GetLargestScaleChart();
        
        LLRegion region = vp.GetLLRegion(rect_region);
        
        LLRegion rendered_region;
        while( chart ) {
            
            QuiltPatch *pqp = mFrameWork->m_pQuilt->GetCurrentPatch();
            if( pqp->b_Valid ) {
                LLRegion get_region = pqp->ActiveRegion;
                bool b_rendered = false;
                
                if( !pqp->b_overlay ) {
                    if(chart->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                        
                        s57chart *Chs57 = dynamic_cast<s57chart*>( chart );
                        if(Chs57){
                            Chs57->RenderViewOnGLTextOnly(m_pcontext, vp);
                        }
                        else{
                            ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                            if(ChPI){
                                ChPI->RenderRegionViewOnGLTextOnly(m_pcontext, vp, rect_region);
                            }
                        }
                    }    
                }
            }
            
            
            chart = mFrameWork->m_pQuilt->GetNextSmallerScaleChart();
        }

/*        
        //    Render any Overlay patches for s57 charts(cells)
        if( mFrameWork->m_pQuilt->HasOverlays() ) {
            ChartBase *pch = mFrameWork->m_pQuilt->GetFirstChart();
            while( pch ) {
                QuiltPatch *pqp = mFrameWork->m_pQuilt->GetCurrentPatch();
                if( pqp->b_Valid && pqp->b_overlay && pch->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                    LLRegion get_region = pqp->ActiveRegion;
                    
                    get_region.Intersect( region );
                    if( !get_region.Empty()  ) {
                        s57chart *Chs57 = dynamic_cast<s57chart*>( pch );
                        if( Chs57 )
                            Chs57->RenderOverlayRegionViewOnGL( *m_pcontext, vp, rect_region, get_region );
                    }
                }
                
                pch = mFrameWork->m_pQuilt->GetNextChart();
            }
        }
*/        
}

void glChartCanvas::RenderCharts(ocpnDC &dc, const OCPNRegion &rect_region)
{
    ViewPort &vp = mFrameWork->mViewPoint;

    // Only for cm93 (not quilted), SetVPParms can change the valid region of the chart
    // we need to know this before rendering the chart so we can compute the background region
    // and nodta regions correctly.  I would prefer to just perform this here (or in SetViewPoint)
    // for all vector charts instead of in their render routine, but how to handle quilted cases?
    if(!vp.quilt() && mFrameWork->m_singleChart->GetChartType() == CHART_TYPE_CM93COMP)
    {
//        static_cast<cm93compchart*>( mFrameWork->m_singleChart )->SetVPParms( vp );
    }
        
    LLRegion chart_region;
    if( !vp.quilt() && (mFrameWork->m_singleChart->GetChartType() == CHART_TYPE_PLUGIN) ){
        if(mFrameWork->m_singleChart->GetChartFamily() == CHART_FAMILY_RASTER){
            // We do this the hard way, since PlugIn Raster charts do not understand LLRegion yet...
            double ll[8];
            ChartPlugInWrapper *cpw = dynamic_cast<ChartPlugInWrapper*> ( mFrameWork->m_singleChart );
            if( !cpw) return;
            
            cpw->chartpix_to_latlong(0,                     0,              ll+0, ll+1);
            cpw->chartpix_to_latlong(0,                     cpw->GetSize_Y(), ll+2, ll+3);
            cpw->chartpix_to_latlong(cpw->GetSize_X(),      cpw->GetSize_Y(), ll+4, ll+5);
            cpw->chartpix_to_latlong(cpw->GetSize_X(),      0,              ll+6, ll+7);
            
            // for now don't allow raster charts to cross both 0 meridian and IDL (complicated to deal with)
            for(int i=1; i<6; i+=2)
                if(fabs(ll[i] - ll[i+2]) > 180) {
                    // we detect crossing idl here, make all longitudes positive
                    for(int i=1; i<8; i+=2)
                        if(ll[i] < 0)
                            ll[i] += 360;
                    break;
                }
                
            chart_region = LLRegion(4, ll);
        }
        else{
            Extent ext;
            mFrameWork->m_singleChart->GetChartExtent(&ext);
            
            double ll[8] = {ext.SLAT, ext.WLON,
            ext.SLAT, ext.ELON,
            ext.NLAT, ext.ELON,
            ext.NLAT, ext.WLON};
            chart_region = LLRegion(4, ll);
        }
    }
    else
        chart_region = vp.quilt() ? mFrameWork->m_pQuilt->GetFullQuiltRegion() : mFrameWork->m_singleChart->GetValidRegion();

    bool world_view = false;
    for(OCPNRegionIterator upd ( rect_region ); upd.HaveRects(); upd.NextRect()) {
        QRect rect = upd.GetRect();
        LLRegion background_region = vp.GetLLRegion(rect);
        //    Remove the valid chart area to find the region NOT covered by the charts
        background_region.Subtract(chart_region);

        if(!background_region.Empty()) {
            ViewPort cvp = ClippedViewport(vp, background_region);
            RenderWorldChart(dc, cvp, rect, world_view);
        }
    }

    if(vp.quilt())
        RenderQuiltViewGL( vp, rect_region );
    else {
        LLRegion region = vp.GetLLRegion(rect_region);
        if( mFrameWork->m_singleChart->GetChartFamily() == CHART_FAMILY_RASTER ){
            if(mFrameWork->m_singleChart->GetChartType() == CHART_TYPE_MBTILES)
                mFrameWork->m_singleChart->RenderRegionViewOnGL(m_pcontext, vp, rect_region, region );
            else
                RenderRasterChartRegionGL( mFrameWork->m_singleChart, vp, region );
        }
        else if( mFrameWork->m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR ) {
            chart_region.Intersect(region);
            RenderNoDTA(vp, chart_region);
            mFrameWork->m_singleChart->RenderRegionViewOnGL(m_pcontext, vp, rect_region, region );
        } 
    }
        
}

void glChartCanvas::RenderNoDTA(ViewPort &vp, const LLRegion &region, int transparency)
{
    QColor color = GetGlobalColor( ( "NODTA" ) );
    if( color.isValid() )
        glColor4ub( color.red(), color.green(), color.blue(), transparency );
    else
        glColor4ub( 163, 180, 183, transparency );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DrawRegion(vp, region);
    
    glDisable(GL_BLEND);
}

void glChartCanvas::RenderNoDTA(ViewPort &vp, ChartBase *chart)
{
//TODO stack from parent    
#if 0    
    QColor color = GetGlobalColor( _T ( "NODTA" ) );
    if( color.IsOk() )
        glColor3ub( color.Red(), color.Green(), color.Blue() );
    else
        glColor3ub( 163, 180, 183 );

    int index = -1;
    ChartTableEntry *pt;
    for(int i=0; i<pCurrentStack->nEntry; i++) {
#if 0
        ChartBase *c = OpenChartFromStack(pCurrentStack, i, HEADER_ONLY);
        if(c == chart) {
            index = pCurrentStack->GetDBIndex(i);
            pt = (ChartTableEntry *) &ChartData->GetChartTableEntry( index );
            break;
        }
#else
        int j = pCurrentStack->GetDBIndex(i);
        assert (j >= 0);
        pt = (ChartTableEntry *) &ChartData->GetChartTableEntry( j );
        if(pt->GetpsFullPath()->IsSameAs(chart->GetFullPath())){
            index = j;
            break;
        }
#endif
    }

    if(index == -1)
        return;

    if( ChartData->GetDBChartType( index ) != CHART_TYPE_CM93COMP ) {
        // Maybe it's a good idea to cache the glutesselator results to improve performance
        LLRegion region(pt->GetnPlyEntries(), pt->GetpPlyTable());
        DrawRegion(vp, region);
    } else {
        QRect rect(0, 0, vp.pixWidth(), vp.pixHeight());
        int x1 = rect.x, y1 = rect.y, x2 = x1 + rect.width, y2 = y1 + rect.height;
        glBegin( GL_QUADS );
        glVertex2i( x1, y1 );
        glVertex2i( x2, y1 );
        glVertex2i( x2, y2 );
        glVertex2i( x1, y2 );
        glEnd();
    }
#endif    
}

/* render world chart, but only in this rectangle */
void glChartCanvas::RenderWorldChart(ocpnDC &dc, ViewPort &vp, QRect &rect, bool &world_view)
{
    // set gl color to water
    QColor water = pWorldBackgroundChart->water;
    glColor3ub(water.red(), water.green(), water.blue());

    // clear background
    if(!world_view) {
        if(vp.projectType() == PROJECTION_ORTHOGRAPHIC) {
            // for this projection, if zoomed out far enough that the earth does
            // not fill the viewport we need to first clear the screen black and
            // draw a blue circle representing the earth

            ViewPort tvp = vp;
            tvp.setLat(0);
            tvp.setLon(0);
            tvp.setRotation(0);
            zchxPointF p = tvp.GetDoublePixFromLL( 89.99, 0);
            float w = ((float)tvp.pixWidth())/2, h = ((float)tvp.pixHeight())/2;
            double world_r = h - p.y;
            const float pi_ovr100 = float(M_PI)/100;
            if(world_r*world_r < w*w + h*h) {
                glClear( GL_COLOR_BUFFER_BIT );

                glBegin(GL_TRIANGLE_FAN);
                float w = ((float)vp.pixWidth())/2, h = ((float)vp.pixHeight())/2;
                for(float theta = 0; theta < 2*M_PI+.01f; theta+=pi_ovr100)
                    glVertex2f(w + world_r*sinf(theta), h + world_r*cosf(theta));
                glEnd();

                world_view = true;
            }
        } else if(vp.projectType() == PROJECTION_EQUIRECTANGULAR) {
            // for this projection we will draw black outside of the earth (beyond the pole)
            glClear( GL_COLOR_BUFFER_BIT );

            zchxPointF p[4] = {
                vp.GetDoublePixFromLL( 90, vp.lon() - 170 ),
                vp.GetDoublePixFromLL( 90, vp.lon() + 170 ),
                vp.GetDoublePixFromLL(-90, vp.lon() + 170 ),
                vp.GetDoublePixFromLL(-90, vp.lon() - 170 )};

            glBegin(GL_QUADS);
            for(int i = 0; i<4; i++)
                glVertex2f(p[i].x, p[i].y);
            glEnd();

            world_view = true;
        }

        if(!world_view) {
            int x1 = rect.x(), y1 = rect.y(), x2 = x1 + rect.width(), y2 = y1 + rect.height();
            glBegin( GL_QUADS );
            glVertex2i( x1, y1 );
            glVertex2i( x2, y1 );
            glVertex2i( x2, y2 );
            glVertex2i( x1, y2 );
            glEnd();
        }
    }

    pWorldBackgroundChart->RenderViewOnDC( dc, vp );
}

/* these are the overlay objects which move with the charts and
   are not frequently updated (not ships etc..) 

   many overlay objects are fixed to a geographical location or
   grounded as opposed to the floating overlay objects. */
void glChartCanvas::DrawGroundedOverlayObjects(ocpnDC &dc, ViewPort &vp)
{
    RenderAllChartOutlines( dc, vp );

    DrawStaticRoutesTracksAndWaypoints( vp );

    DisableClipRegion();
}

void glChartCanvas::RenderAllChartOutlines(ocpnDC &dc, ViewPort &vp)
{
    if( !m_bShowOutlines ) return;
    int nEntry = ChartData->GetChartTableEntries();

    for( int i = 0; i < nEntry; i++ ) {
        RenderChartOutline(i, vp);
    }
}

int n_render;
void glChartCanvas::Render()
{
    if( !m_bsetup ||
        ( mFrameWork->mViewPoint.quilt() && !mFrameWork->m_pQuilt->IsComposed() ) ||
        ( !mFrameWork->mViewPoint.quilt() && !mFrameWork->m_singleChart ) ) {
#ifdef __WXGTK__  // for some reason in gtk, a swap is needed here to get an initial screen update
            SwapBuffers();
#endif
            return;
        }

    m_last_render_time = QDateTime::currentDateTime().toTime_t();

    // we don't care about jobs that are now off screen
    // clear out and it will be repopulated during render
    if(g_GLOptions.m_bTextureCompression && !g_GLOptions.m_bTextureCompressionCaching)
        g_glTextureManager->ClearJobList();

    if(b_timeGL && g_bShowFPS){
        if(n_render % 10){
            glFinish();   
//            g_glstopwatch.Start();
        }
    }
//    wxPaintDC( this );

    //  If we are in the middle of a fast pan, we don't want the FBO coordinates to be reset
    if(m_binPinch || m_binPan)
        return;
        
    ViewPort VPoint = mFrameWork->mViewPoint;
    ocpnDC gldc( this );

    int gl_width = width(), gl_height = height();
//    GetClientSize( &gl_width, &gl_height );

#ifdef __WXOSX__    
    gl_height = mFrameWork->GetClientSize().y;
#endif    
    
    OCPNRegion screen_region(QRect(0, 0, VPoint.pixWidth(), VPoint.pixHeight()));

    glViewport( 0, 0, (GLint) gl_width, (GLint) gl_height );
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    glOrtho( 0, (GLint) gl_width, (GLint) gl_height, 0, -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }

    // set opengl settings that don't normally change
    // this should be able to go in SetupOpenGL, but it's
    // safer here incase a plugin mangles these
    if( g_GLOptions.m_GLLineSmoothing )
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    if( g_GLOptions.m_GLPolygonSmoothing )
        glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    //  Delete any textures known to the GPU that
    //  belong to charts which will not be used in this render
    //  This is done chart-by-chart...later we will scrub for unused textures
    //  that belong to charts which ARE used in this render, if we need to....

    g_glTextureManager->TextureCrunch(0.8);

    //  If we plan to post process the display, don't use accelerated panning
    double scale_factor = VPoint.refScale()/VPoint.chartScale();
    
    m_bfogit = m_benableFog && g_fog_overzoom && (scale_factor > g_overzoom_emphasis_base) && VPoint.quilt();
    bool scale_it  =  m_benableVScale && g_oz_vector_scale && (scale_factor > g_overzoom_emphasis_base) && VPoint.quilt();
    
    bool bpost_hilite = !mFrameWork->m_pQuilt->GetHiliteRegion( ).Empty();
    bool useFBO = false;
    int sx = gl_width;
    int sy = gl_height;
//    qDebug()<<"gl size:"<<sx<<sy<<" tex size<<"<<m_cache_tex_x<<m_cache_tex_y;

    // Try to use the framebuffer object's cache of the last frame
    // to accelerate drawing this frame (if overlapping)
    if(m_b_BuiltFBO && !m_bfogit && !scale_it && !bpost_hilite
       //&& VPoint.tilt == 0 // disabling fbo in tilt mode gives better quality but slower
        ) {
        //  Is this viewpoint the same as the previously painted one?
        bool b_newview = true;

        // If the view is the same we do no updates, 
        // cached texture to the framebuffer
//        qDebug()<<"vp roate:"<<m_cache_vp.rotation()<<VPoint.rotation();
        if(    m_cache_vp.viewScalePPM() == VPoint.viewScalePPM()
               && m_cache_vp.rotation() == VPoint.rotation()
               && m_cache_vp.lat() == VPoint.lat()
               && m_cache_vp.lon() == VPoint.lon()
               && m_cache_vp.isValid()
               && m_cache_vp.pixHeight() == VPoint.pixHeight()
               && m_cache_current_ch == mFrameWork->m_singleChart ) {
            b_newview = false;
        }

//        qDebug()<<"new flag = "<<b_newview;
        if( b_newview ) {

            bool busy = false;
            if(VPoint.quilt() && mFrameWork->m_pQuilt->IsQuiltVector() &&
                ( m_cache_vp.viewScalePPM() != VPoint.viewScalePPM() || m_cache_vp.rotation() != VPoint.rotation()))
            {
                    OCPNPlatform::instance()->ShowBusySpinner();
                    busy = true;
            }
            
            // enable rendering to texture in framebuffer object
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, m_fb0 );

            int dx, dy;
            bool accelerated_pan = false;
            if( g_GLOptions.m_bUseAcceleratedPanning && m_cache_vp.isValid()
                && ( VPoint.projectType() == PROJECTION_MERCATOR
                || VPoint.projectType() == PROJECTION_EQUIRECTANGULAR )
                && m_cache_vp.pixHeight() == VPoint.pixHeight() )
            {
                qDebug()<<"!!!!!!!!!!!!!!";
                zchxPointF c_old = VPoint.GetDoublePixFromLL( VPoint.lat(), VPoint.lon() );
                zchxPointF c_new = m_cache_vp.GetDoublePixFromLL( VPoint.lat(), VPoint.lon() );

//                printf("diff: %f %f\n", c_new.y - c_old.y, c_new.x - c_old.x);
                dy = qRound(c_new.y - c_old.y);
                dx = qRound(c_new.x - c_old.x);

                //   The math below using the previous frame's texture does not really
                //   work for sub-pixel pans.
                //   TODO is to rethink this.
                //   Meanwhile, require the accelerated pans to be whole pixel multiples only.
                //   This is not as bad as it sounds.  Keyboard and mouse pans are whole_pixel moves.
                //   However, autofollow at large scale is certainly not.
                
                double deltax = c_new.x - c_old.x;
                double deltay = c_new.y - c_old.y;
                
                bool b_whole_pixel = true;
                if( ( fabs( deltax - dx ) > 1e-2 ) || ( fabs( deltay - dy ) > 1e-2 ) )
                    b_whole_pixel = false;
                accelerated_pan = b_whole_pixel && abs(dx) < m_cache_tex_x && abs(dy) < m_cache_tex_y
                                  && sx == m_cache_tex_x && sy == m_cache_tex_y;

                qDebug()<<b_whole_pixel<<dx<<m_cache_tex_x<<dy<<m_cache_tex_y<<sx<<sy<<accelerated_pan;
            }

            // do we allow accelerated panning?  can we perform it here?
            if(accelerated_pan && !g_GLOptions.m_bUseCanvasPanning) {
                qDebug()<<"???????????????????";
                if((dx != 0) || (dy != 0)){   // Anything to do?
                    m_cache_page = !m_cache_page; /* page flip */

                    /* perform accelerated pan rendering to the new framebuffer */
                    ( s_glFramebufferTexture2D )
                        ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                        g_texture_rectangle_format, m_cache_tex[m_cache_page], 0 );

                    //calculate the new regions to render
                    // add an extra pixel avoid coorindate rounding issues
                    OCPNRegion update_region;

                    if( dy > 0 && dy < VPoint.pixHeight())
                        update_region.Union(QRect( 0, VPoint.pixHeight() - dy, VPoint.pixWidth(), dy ));
                    else if(dy < 0)
                        update_region.Union(QRect( 0, 0, VPoint.pixWidth(), -dy ));
                            
                    if( dx > 0 && dx < VPoint.pixWidth() )
                        update_region.Union(QRect( VPoint.pixWidth() - dx, 0, dx, VPoint.pixHeight() ));
                    else if (dx < 0)
                        update_region.Union(QRect( 0, 0, -dx, VPoint.pixHeight() ));

                    RenderCharts(gldc, update_region);

                    // using the old framebuffer
                    glBindTexture( g_texture_rectangle_format, m_cache_tex[!m_cache_page] );
                    glEnable( g_texture_rectangle_format );
                    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
                                
                    //    Render the reuseable portion of the cached texture
                    // Render the cached texture as quad to FBO(m_blit_tex) with offsets
                    int x1, x2, y1, y2;

                    int ow = VPoint.pixWidth() - abs( dx );
                    int oh = VPoint.pixHeight() - abs( dy );
                    if( dx > 0 )
                        x1 = dx,  x2 = 0;
                    else
                        x1 = 0,   x2 = -dx;
                            
                    if( dy > 0 )
                        y1 = dy,  y2 = 0;
                    else
                        y1 = 0,   y2 = -dy;

                    // normalize to texture coordinates range from 0 to 1
                    float tx1 = x1, tx2 = x1 + ow, ty1 = sy - y1, ty2 = sy - (y1 + oh);
                    if( GL_TEXTURE_RECTANGLE_ARB != g_texture_rectangle_format )
                        tx1 /= sx, tx2 /= sx, ty1 /= sy, ty2 /= sy;

                    glBegin( GL_QUADS );
                    glTexCoord2f( tx1, ty1 );  glVertex2f( x2, y2 );
                    glTexCoord2f( tx2, ty1 );  glVertex2f( x2 + ow, y2 );
                    glTexCoord2f( tx2, ty2 );  glVertex2f( x2 + ow, y2 + oh );
                    glTexCoord2f( tx1, ty2 );  glVertex2f( x2, y2 + oh );
                    glEnd();

                    //   Done with cached texture "blit"
                    glDisable( g_texture_rectangle_format );
                }

            } else { // must redraw the entire screen
//                qDebug()<<"***********************";
                    ( s_glFramebufferTexture2D )( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                                g_texture_rectangle_format,
                                                m_cache_tex[m_cache_page], 0 );
                    
                    if(g_GLOptions.m_bUseCanvasPanning) {
                        qDebug()<<"$$$$$$$$$$$$$$$$$$";
                        bool b_reset = false;
                        if( (m_fbo_offsetx < 50) ||
                            ((m_cache_tex_x - (m_fbo_offsetx + sx)) < 50) ||
                            (m_fbo_offsety < 50) ||
                            ((m_cache_tex_y - (m_fbo_offsety + sy)) < 50))
                            b_reset = true;
    
                        if(m_cache_vp.viewScalePPM() != VPoint.viewScalePPM() )
                            b_reset = true;
                        if(!m_cache_vp.isValid())
                            b_reset = true;
                            
                        if( b_reset ){
                            m_fbo_offsetx = (m_cache_tex_x - width())/2;
                            m_fbo_offsety = (m_cache_tex_y - height())/2;
                            m_fbo_swidth = sx;
                            m_fbo_sheight = sy;
                            
                            m_canvasregion = OCPNRegion( m_fbo_offsetx, m_fbo_offsety, sx, sy );
                            
//                             if(m_cache_vp.viewScalePPM() != VPoint.viewScalePPM() )
//                                 OCPNPlatform::ShowBusySpinner();
                            
                            RenderCanvasBackingChart(gldc, m_canvasregion);
                        }
                        
                        

                        glPushMatrix();
                        
                        glViewport( m_fbo_offsetx, m_fbo_offsety, (GLint) sx, (GLint) sy );
                        RenderCharts(gldc, screen_region);
                        
                        glPopMatrix();

                        glViewport( 0, 0, (GLint) sx, (GLint) sy );
                    }
                    else{
//                        qDebug()<<"^^^^^^^^^^^^^^^";
#if 0                        
                        // Should not really need to clear the screen, but consider this case:
                        // A previous installation of OCPN loaded some PlugIn charts, so setting their coverage in the database.
                        // In this installation, the PlugIns are not available, for whatever reason.
                        // As a result, some areas on screen will not be rendered by the PlugIn, and
                        // The backing chart is eclipsed by the unrendered chart coverage region.
                        //  Result: leftover garbage in the unrendered regions.

                        QColor color = GetGlobalColor( _T ( "NODTA" ) );
                        if( color.IsOk() )
                            glClearColor( color.Red() / 256., color.Green()/256., color.Blue()/256., 1.0 );
                        else
                            glClearColor(0, 0., 0, 1.0);
                        glClear(GL_COLOR_BUFFER_BIT);
#endif

                        m_fbo_offsetx = 0;
                        m_fbo_offsety = 0;
                        m_fbo_swidth = sx;
                        m_fbo_sheight = sy;
                        QRect rect(m_fbo_offsetx, m_fbo_offsety, (GLint) sx, (GLint) sy);
//                        qDebug()<<"render new chart";
                        RenderCharts(gldc, screen_region);
                    }
                    
                } 
            // Disable Render to FBO
            ( s_glBindFramebuffer )( GL_FRAMEBUFFER_EXT, 0 );

            if(busy)
                OCPNPlatform::instance()->HideBusySpinner();
        
        } // newview

        useFBO = true;
    }

    if(VPoint.tilt()) {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(2*180/PI*atan2((double)gl_height, (double)gl_width), (GLfloat) gl_width/(GLfloat) gl_height, 1, gl_width);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glScalef(1, -1, 1);
        glTranslatef(-gl_width/2, -gl_height/2, -gl_width/2);
        glRotated(VPoint.tilt()*180/PI, 1, 0, 0);

        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
    }

    if(useFBO) {
        // Render the cached texture as quad to screen
        glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
        glEnable( g_texture_rectangle_format );

        float tx, ty, tx0, ty0, divx, divy;
        
        //  Normalize, or not?
        if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format ){
            divx = divy = 1.0f;
         }
        else{
            divx = m_cache_tex_x;
            divy = m_cache_tex_y;
        }

        tx0 = m_fbo_offsetx/divx;
        ty0 = m_fbo_offsety/divy;
        tx =  (m_fbo_offsetx + m_fbo_swidth)/divx;
        ty =  (m_fbo_offsety + m_fbo_sheight)/divy;
        
        glBegin( GL_QUADS );
        glTexCoord2f( tx0, ty );  glVertex2f( 0,  0 );
        glTexCoord2f( tx,  ty );  glVertex2f( sx, 0 );
        glTexCoord2f( tx,  ty0 ); glVertex2f( sx, sy );
        glTexCoord2f( tx0, ty0 ); glVertex2f( 0,  sy );
        glEnd();

        glDisable( g_texture_rectangle_format );

        m_cache_vp = VPoint;
        m_cache_current_ch = mFrameWork->m_singleChart;

        if(VPoint.quilt())
            mFrameWork->m_pQuilt->SetRenderedVP( VPoint );
        
    } else          // useFBO
        RenderCharts(gldc, screen_region);

       //  Render the decluttered Text overlay for quilted vector charts, except for CM93 Composite
    if( VPoint.quilt() ) {
        if(mFrameWork->m_pQuilt->IsQuiltVector() && ps52plib && ps52plib->GetShowS57Text()){

            ChartBase *chart = mFrameWork->m_pQuilt->GetRefChart();
            if(chart && (chart->GetChartType() != CHART_TYPE_CM93COMP)){
                //        Clear the text Global declutter list
                if(chart){
                    ChartPlugInWrapper *ChPI = dynamic_cast<ChartPlugInWrapper*>( chart );
                    if(ChPI)
                        ChPI->ClearPLIBTextList();
                    else
                        ps52plib->ClearTextList();
                }
                
                // Grow the ViewPort a bit laterally, to minimize "jumping" of text elements at left side of screen
                ViewPort vpx = VPoint;
                vpx.BuildExpandedVP(VPoint.pixWidth() * 12 / 10, VPoint.pixHeight());
                
                OCPNRegion screen_region(QRect(0, 0, VPoint.pixWidth(), VPoint.pixHeight()));
                RenderQuiltViewGLText( vpx, screen_region );
            }
        }
    }


    
    // Render MBTiles as overlay
    std::vector<int> stackIndexArray = mFrameWork->m_pQuilt->GetExtendedStackIndexArray();
    unsigned int im = stackIndexArray.size();
    // XXX should
    // assert(!VPoint.quilt() && im == 0)
    if( VPoint.quilt() && im > 0 ) {
        bool regionVPBuilt = false;
        OCPNRegion screen_region;
        LLRegion screenLLRegion;
        LLBBox screenBox;
        ViewPort vp;

        std::vector<int> tiles_to_show;
        for( unsigned int is = 0; is < im; is++ ) {
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( stackIndexArray[is] );
            if(std::find(g_quilt_noshow_index_array.begin(), g_quilt_noshow_index_array.end(), stackIndexArray[is]) != g_quilt_noshow_index_array.end()) {
                continue;
            }
            if(cte.GetChartType() == CHART_TYPE_MBTILES){
                tiles_to_show.push_back(stackIndexArray[is]);
                if(!regionVPBuilt){
                    screen_region = OCPNRegion(QRect(0, 0, VPoint.pixWidth(), VPoint.pixHeight()));
                    screenLLRegion = VPoint.GetLLRegion( screen_region );
                    screenBox = screenLLRegion.GetBox();

                    vp = VPoint;
                    zchxPoint p;
                    p.x = VPoint.pixWidth() / 2;  p.y = VPoint.pixHeight() / 2;
                    double lat = 0.0, lon = 0.0;
                    VPoint.GetLLFromPix( p, &lat, &lon);
                    vp.setLat(lat);
                    vp.setLon(lon);
                    
                    regionVPBuilt = true;
                }

            }
        }
        // We need to show the tilesets in reverse order to have the largest scale on top
        for(std::vector<int>::reverse_iterator rit = tiles_to_show.rbegin();
                            rit != tiles_to_show.rend(); ++rit) {
            ChartBase *chart = ChartData->OpenChartFromDBAndLock(*rit, FULL_INIT);
            ChartMBTiles *pcmbt = dynamic_cast<ChartMBTiles*>( chart );
            if(pcmbt){
                pcmbt->RenderRegionViewOnGL(m_pcontext, vp, screen_region, screenLLRegion);
            }
        }
    }
 


     
    // Render static overlay objects , 这里就是绿色的外框
    for(OCPNRegionIterator upd ( screen_region ); upd.HaveRects(); upd.NextRect()) {
         LLRegion region = VPoint.GetLLRegion(upd.GetRect());
         ViewPort cvp = ClippedViewport(VPoint, region);
         DrawGroundedOverlayObjects(gldc, cvp);
    }

    DrawDynamicRoutesTracksAndWaypoints( VPoint );
        
    // Now draw all the objects which normally move around and are not
    // cached from the previous frame
    DrawFloatingOverlayObjects( gldc );

    // from this point on don't use perspective
    if(VPoint.tilt()) {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        glOrtho( 0, (GLint) gl_width, (GLint) gl_height, 0, -1, 1 );
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    DrawEmboss(EmbossDepthScale() );
    DrawEmboss(EmbossOverzoomIndicator( gldc ) );

    //  On some platforms, the opengl context window is always on top of any standard DC windows,
    //  so we need to draw the Chart Info Window and the Thumbnail as overlayed bmps.

#ifdef __WXOSX__    
        if(mFrameWork->m_pCIWin && mFrameWork->m_pCIWin->IsShown()) {
        int x, y, width, height;
        mFrameWork->m_pCIWin->GetClientSize( &width, &height );
        mFrameWork->m_pCIWin->GetPosition( &x, &y );
        wxBitmap bmp(width, height, -1);
        wxMemoryDC dc(bmp);
        if(bmp.IsOk()){
            dc.SetBackground( wxBrush(GetGlobalColor( _T ( "UIBCK" ) ) ));
            dc.Clear();
 
            dc.SetTextBackground( GetGlobalColor( _T ( "UIBCK" ) ) );
            dc.SetTextForeground( GetGlobalColor( _T ( "UITX1" ) ) );
            
            int yt = 0;
            int xt = 0;
            QString s = mFrameWork->m_pCIWin->GetString();
            int h = mFrameWork->m_pCIWin->GetCharHeight();
            
            QStringTokenizer tkz( s, _T("\n") );
            QString token;
            
            while(tkz.HasMoreTokens()) {
                token = tkz.GetNextToken();
                dc.DrawText(token, xt, yt);
                yt += h;
            }
            dc.SelectObject(wxNullBitmap);
            
            gldc.DrawBitmap( bmp, x, y, false);
        }
    }

    if( pthumbwin && pthumbwin->IsShown()) {
        int thumbx, thumby;
        pthumbwin->GetPosition( &thumbx, &thumby );
        if( pthumbwin->GetBitmap().IsOk())
            gldc.DrawBitmap( pthumbwin->GetBitmap(), thumbx, thumby, false);
    }
    
    if(g_MainToolbar && g_MainToolbar->m_pRecoverwin ){
        int recoverx, recovery;
        g_MainToolbar->m_pRecoverwin->GetPosition( &recoverx, &recovery );
        if( g_MainToolbar->m_pRecoverwin->GetBitmap().IsOk())
            gldc.DrawBitmap( g_MainToolbar->m_pRecoverwin->GetBitmap(), recoverx, recovery, true);
    }
    
    
#endif
    
    RenderGLAlertMessage();

    //quiting?
    if( m_bQuiting )     DrawQuiting();
    if( g_bcompression_wait)
        DrawCloseMessage("Waiting for raster chart compression thread exit.");

    
    
     //  Some older MSW OpenGL drivers are generally very unstable.
     //  This helps...   

    if(g_b_needFinish)
        glFinish();
    
//    swapBuffers();
    if(b_timeGL && g_bShowFPS){
        if(n_render % 10){
            glFinish();
        
            double filter = .05;
        
        // Simple low pass filter
//            g_gl_ms_per_frame = g_gl_ms_per_frame * (1. - filter) + ((double)(g_glstopwatch.Time()) * filter);
//            if(g_gl_ms_per_frame > 0)
            //                printf(" OpenGL frame time: %3.0f  %3.0f\n", g_gl_ms_per_frame, 1000./ g_gl_ms_per_frame);
        }
    }

    g_glTextureManager->TextureCrunch(0.8);
    g_glTextureManager->FactoryCrunch(0.6);
    
    n_render++;
}



void glChartCanvas::RenderCanvasBackingChart( ocpnDC &dc, OCPNRegion valid_region)
{
 
    glPushMatrix();

    glLoadIdentity();
    
    glOrtho( 0, m_cache_tex_x, m_cache_tex_y, 0, -1, 1 );
    glViewport( 0, 0, (GLint) m_cache_tex_x, (GLint) m_cache_tex_y );
    
    // strategies:
    
    // 1:  Simple clear to color
    
    OCPNRegion texr( 0, 0, m_cache_tex_x,  m_cache_tex_y );
    texr.Subtract(valid_region);
   
    glViewport( 0, 0, (GLint) m_cache_tex_x, (GLint) m_cache_tex_y );
    
    glColor3ub(0, 250, 250);
    
    OCPNRegionIterator upd ( texr );
    while ( upd.HaveRects() )
    {
        QRect rect = upd.GetRect();
        
        glBegin( GL_QUADS );
        glVertex2f( rect.x(),                     rect.y() );
        glVertex2f( rect.x() + rect.width(),        rect.y() );
        glVertex2f( rect.x() + rect.width(),        rect.y() + rect.height() );
        glVertex2f( rect.x(),                     rect.y() + rect.height() );
        glEnd();
        
        upd.NextRect();
    }        

    // 2:  Render World Background chart
    
    QRect rtex( 0, 0, m_cache_tex_x,  m_cache_tex_y );
    ViewPort cvp = mFrameWork->GetVP().BuildExpandedVP( m_cache_tex_x,  m_cache_tex_y );

    bool world_view = false;
    RenderWorldChart(dc, cvp, rtex, world_view);

/*    
    dc.SetPen(wxPen(wxColour(254,254,0), 3));
    dc.DrawLine( 0, 0, m_cache_tex_x, m_cache_tex_y);
    
    dc.DrawLine( 0, 0, 0, m_cache_tex_y);
    dc.DrawLine( 0, m_cache_tex_y, m_cache_tex_x, m_cache_tex_y);
    dc.DrawLine( m_cache_tex_x, m_cache_tex_y, m_cache_tex_x, 0);
    dc.DrawLine( m_cache_tex_x, 0, 0, 0);
*/    
  
    // 3:  Use largest scale chart in the current quilt candidate list (which is identical to chart bar)
    //          which covers the entire canvas

    glPopMatrix();

}


void glChartCanvas::FastPan(int dx, int dy)
{
    int sx = width();
    int sy = height();
    
    //   ViewPort VPoint = mFrameWork->VPoint;
    //   ViewPort svp = VPoint;
    //   svp.pixWidth() = svp.rv_rect.width;
    //   svp.pixHeight() = svp.rv_rect.height;
    
    //   OCPNRegion chart_get_region( 0, 0, mFrameWork->VPoint.rv_rect.width, mFrameWork->VPoint.rv_rect.height );
    
    //    ocpnDC gldc( *this );
    
    int w = width(), h = height();
//    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );
    
    glLoadIdentity();
    glOrtho( 0, (GLint) w, (GLint) h, 0, -1, 1 );
    
    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }
    
    float vx0 = 0;
    float vy0 = 0;
    float vy = sy;
    float vx = sx;
    
    glBindTexture( g_texture_rectangle_format, 0);
    
    /*   glColor3ub(0, 0, 120);
     *    glBegin( GL_QUADS );
     *    glVertex2f( 0,  0 );
     *    glVertex2f( w, 0 );
     *    glVertex2f( w, h );
     *    glVertex2f( 0,  h );
     *    glEnd();
     */
    
    
    
    float tx, ty, tx0, ty0;
    //if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format )
    //  tx = sx, ty = sy;
    //else
    tx = 1, ty = 1;
    
    tx0 = ty0 = 0.;

    m_fbo_offsety += dy;
    m_fbo_offsetx += dx;
    
    tx0 = m_fbo_offsetx;
    ty0 = m_fbo_offsety;
    tx =  m_fbo_offsetx + sx;
    ty =  m_fbo_offsety + sy;
    
    
    if((m_fbo_offsety ) < 0){
        ty0 = 0;
        ty  =  m_fbo_offsety + sy;
        
        vy0 = 0;
        vy = sy + m_fbo_offsety;
        
        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( 0,  vy );
        glVertex2f( w, vy );
        glVertex2f( w, h );
        glVertex2f( 0,  h );
        glEnd();
        
    }
    else if((m_fbo_offsety + sy) > m_cache_tex_y){
        ty0 = m_fbo_offsety;
        ty  =  m_cache_tex_y;
        
        vy = sy;
        vy0 = (m_fbo_offsety + sy - m_cache_tex_y);
        
        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( 0,  0 );
        glVertex2f( w, 0 );
        glVertex2f( w, vy0 );
        glVertex2f( 0, vy0 );
        glEnd();
    }
    
    
    
    if((m_fbo_offsetx) < 0){
        tx0 = 0;
        tx  =  m_fbo_offsetx + sx;
        
        vx0 = -m_fbo_offsetx;
        vx = sx;
        
        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( 0,  0 );
        glVertex2f( vx0, 0 );
        glVertex2f( vx0, h );
        glVertex2f( 0,  h );
        glEnd();
    }
    else if((m_fbo_offsetx + sx) > m_cache_tex_x){
        tx0 = m_fbo_offsetx;
        tx  = m_cache_tex_x;
        
        vx0 = 0;
        vx = m_cache_tex_x - m_fbo_offsetx;
        
        glColor3ub(80, 0, 0);
        glBegin( GL_QUADS );
        glVertex2f( vx,  0 );
        glVertex2f( w, 0 );
        glVertex2f( w, h );
        glVertex2f( vx,  h );
        glEnd();
    }
    

    // Render the cached texture as quad to screen
    glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
    glEnable( g_texture_rectangle_format );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    
    glBegin( GL_QUADS );
    glTexCoord2f( tx0/m_cache_tex_x, ty/m_cache_tex_y );  glVertex2f( vx0,  vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty/m_cache_tex_y );  glVertex2f( vx,   vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty0/m_cache_tex_y ); glVertex2f( vx,   vy );
    glTexCoord2f( tx0/m_cache_tex_x, ty0/m_cache_tex_y ); glVertex2f( vx0,  vy );
    glEnd();
    
    glDisable( g_texture_rectangle_format );
    glBindTexture( g_texture_rectangle_format, 0);
    
    m_canvasregion.Union(tx0, ty0, sx, sy);
}

QColor glChartCanvas::GetBackGroundColor()const
{
    QPalette pal = this->palette();
    QBrush brush = pal.background();
    return brush.color();
}

void glChartCanvas::RenderGLAlertMessage()
{
    if(!GetAlertString().isEmpty())
    {
        QString msg = GetAlertString();

        QFont pfont("Micorosoft Yahei", 10, QFont::Weight::Normal);
        TexFont texfont;
        texfont.Build(pfont);

        int w, h;
        texfont.GetTextExtent( msg, &w, &h);
        h += 2;
        w += 4;
        int yp = mFrameWork->mViewPoint.pixHeight() - 20 - h;

        QRect sbr = GetScaleBarRect();
        int xp = sbr.x()+sbr.width() + 5;

        glColor3ub( 243, 229, 47 );

        glBegin(GL_QUADS);
        glVertex2i(xp, yp);
        glVertex2i(xp+w, yp);
        glVertex2i(xp+w, yp+h);
        glVertex2i(xp, yp+h);
        glEnd();

        glEnable(GL_BLEND);
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        glColor3ub( 0, 0, 0 );
        glEnable(GL_TEXTURE_2D);
        texfont.RenderString( msg, xp, yp);
        glDisable(GL_TEXTURE_2D);

    }
}



void glChartCanvas::FastZoom(float factor)
{
    int sx = width();
    int sy = height();
   
    if(factor > 1.0f){
        int fbo_ctr_x = m_fbo_offsetx + (m_fbo_swidth / 2);
        int fbo_ctr_y = m_fbo_offsety + (m_fbo_sheight / 2);
        
        m_fbo_swidth  = m_fbo_swidth / factor;
        m_fbo_sheight = m_fbo_sheight / factor;
        
        m_fbo_offsetx = fbo_ctr_x - (m_fbo_swidth / 2.);
        m_fbo_offsety = fbo_ctr_y - (m_fbo_sheight / 2.);
        
    }
    
    if(factor < 1.0f){
        int fbo_ctr_x = m_fbo_offsetx + (m_fbo_swidth / 2);
        int fbo_ctr_y = m_fbo_offsety + (m_fbo_sheight / 2);
        
        m_fbo_swidth  = m_fbo_swidth / factor;
        m_fbo_sheight = m_fbo_sheight / factor;
        
        m_fbo_offsetx = fbo_ctr_x - (m_fbo_swidth / 2.);
        m_fbo_offsety = fbo_ctr_y - (m_fbo_sheight / 2.);
        
    }
    
    
    float tx, ty, tx0, ty0;
    //if( GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format )
    //  tx = sx, ty = sy;
    //else
    tx = 1, ty = 1;
    
    tx0 = ty0 = 0.;
    
    tx0 = m_fbo_offsetx;
    ty0 = m_fbo_offsety;
    tx =  m_fbo_offsetx + m_fbo_swidth;
    ty =  m_fbo_offsety + m_fbo_sheight;
    
    
    int w = width(), h = height();
//    GetClientSize( &w, &h );
    glViewport( 0, 0, (GLint) w, (GLint) h );
    
    glLoadIdentity();
    glOrtho( 0, (GLint) w, (GLint) h, 0, -1, 1 );
    
    if( s_b_useStencil ) {
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0xff );
        glClear( GL_STENCIL_BUFFER_BIT );
        glDisable( GL_STENCIL_TEST );
    }

    
    float vx0 = 0;
    float vy0 = 0;
    float vy = sy;
    float vx = sx;
    
    glBindTexture( g_texture_rectangle_format, 0);
    
    // Render the cached texture as quad to screen
    glBindTexture( g_texture_rectangle_format, m_cache_tex[m_cache_page]);
    glEnable( g_texture_rectangle_format );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    
    
    glBegin( GL_QUADS );
    glTexCoord2f( tx0/m_cache_tex_x, ty/m_cache_tex_y );  glVertex2f( vx0,  vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty/m_cache_tex_y );  glVertex2f( vx,   vy0 );
    glTexCoord2f( tx/m_cache_tex_x,  ty0/m_cache_tex_y ); glVertex2f( vx,   vy );
    glTexCoord2f( tx0/m_cache_tex_x, ty0/m_cache_tex_y ); glVertex2f( vx0,  vy );
    glEnd();
    
    glDisable( g_texture_rectangle_format );
    glBindTexture( g_texture_rectangle_format, 0);

    //  When zooming out, if we go too far, then the frame buffer is repeated on-screen due
    //  to address wrapping in the frame buffer.
    //  Detect this case, and render some simple solid covering quads to avoid a confusing display.
    if( (m_fbo_sheight > m_cache_tex_y) || (m_fbo_swidth > m_cache_tex_x) ){
        QColor color = GetGlobalColor("GREY1");
        glColor3ub(color.red(), color.green(), color.blue());
        
        if( m_fbo_sheight > m_cache_tex_y ){
            float h1 = sy * (1.0 - m_cache_tex_y/m_fbo_sheight) / 2.;
            
            glBegin( GL_QUADS );
            glVertex2f( 0,  0 );
            glVertex2f( w,  0 );
            glVertex2f( w, h1 );
            glVertex2f( 0, h1 );
            glEnd();

            glBegin( GL_QUADS );
            glVertex2f( 0,  sy );
            glVertex2f( w,  sy );
            glVertex2f( w, sy - h1 );
            glVertex2f( 0, sy - h1 );
            glEnd();
            
        }
 
         // horizontal axis
         if( m_fbo_swidth > m_cache_tex_x ){
             float w1 = sx * (1.0 - m_cache_tex_x/m_fbo_swidth) / 2.;
             
             glBegin( GL_QUADS );
             glVertex2f( 0,  0 );
             glVertex2f( w1,  0 );
             glVertex2f( w1, sy );
             glVertex2f( 0, sy );
             glEnd();
             
             glBegin( GL_QUADS );
             glVertex2f( sx,  0 );
             glVertex2f( sx - w1,  0 );
             glVertex2f( sx - w1, sy );
             glVertex2f( sx, sy );
             glEnd();
             
         }
    }
}

void glChartCanvas::ToggleChartOutlines()
{
    m_bShowOutlines = !m_bShowOutlines;
    Refresh( false );
    Invalidate();
}

emboss_data *glChartCanvas::EmbossOverzoomIndicator( ocpnDC &dc )
{
    double zoom_factor = mFrameWork->GetVP().refScale() / mFrameWork->GetVP().chartScale();

    if( mFrameWork->GetQuiltMode() ) {

        // disable Overzoom indicator for MBTiles
        int refIndex = mFrameWork->GetQuiltRefChartdbIndex();
        if(refIndex >= 0){
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( refIndex );
            ChartTypeEnum current_type = (ChartTypeEnum) cte.GetChartType();
            if( current_type == CHART_TYPE_MBTILES){
                ChartBase *pChart = mFrameWork->m_pQuilt->GetRefChart();
                ChartMBTiles *ptc = dynamic_cast<ChartMBTiles *>( pChart );
                if(ptc){
                    zoom_factor = ptc->GetZoomFactor();
                }
            }
        }

        if( zoom_factor <= 3.9 )
            return NULL;
    } else {
        if( mFrameWork->m_singleChart ) {
            if( zoom_factor <= 3.9 )
                return NULL;
        }
        else
            return NULL;
    }

    if(m_pEM_OverZoom){
        m_pEM_OverZoom->x = 4;
        m_pEM_OverZoom->y = 0;
    }
    return m_pEM_OverZoom;
}

emboss_data *glChartCanvas::EmbossDepthScale()
{
    if( !m_bShowDepthUnits ) return NULL;
    int depth_unit_type = DEPTH_UNIT_UNKNOWN;

    if( mFrameWork->GetQuiltMode() ) {
        QString s = mFrameWork->m_pQuilt->GetQuiltDepthUnit();
        s.toUpper();
        if( s == ("FEET") ) depth_unit_type = DEPTH_UNIT_FEET;
        else if( s.startsWith( ("FATHOMS") ) ) depth_unit_type = DEPTH_UNIT_FATHOMS;
        else if( s.startsWith( ("METERS") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.startsWith( ("METRES") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.startsWith( ("METRIC") ) ) depth_unit_type = DEPTH_UNIT_METERS;
        else if( s.startsWith( ("METER") ) ) depth_unit_type = DEPTH_UNIT_METERS;

    } else {
        if( mFrameWork->m_singleChart ) {
            depth_unit_type = mFrameWork->m_singleChart->GetDepthUnitType();
            if( mFrameWork->m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR )
                depth_unit_type = ps52plib->m_nDepthUnitDisplay + 1;
        }
    }

    emboss_data *ped = NULL;
    switch( depth_unit_type ) {
    case DEPTH_UNIT_FEET:
        ped = m_pEM_Feet;
        break;
    case DEPTH_UNIT_METERS:
        ped = m_pEM_Meters;
        break;
    case DEPTH_UNIT_FATHOMS:
        ped = m_pEM_Fathoms;
        break;
    default:
        return NULL;
    }

    ped->x = ( mFrameWork->GetVP().pixWidth() - ped->width );
    ped->y = 40;
    return ped;
}

#define OVERZOOM_TEXT ("OverZoom")

void glChartCanvas::SetOverzoomFont()
{
    ocpnStyle::Style* style = StyleMgrIns->GetCurrentStyle();
    int w, h;

    QFont font;
    if( style->embossFont.isEmpty() ){
        QFont dFont = FontMgr::Get().GetFont( ("Dialog"), 0 );
        font = dFont;
        font.setPointSize(40);
        font.setWeight(QFont::Weight::Bold);
    }
    else
    {
        font = QFont( "Micorosoft YaHei", 40, QFont::Weight::Bold);
    }

    QFontMetrics m(font);
    w = m.width(OVERZOOM_TEXT);
    h = m.height();

    while( font.pointSize() > 10 && (w > 500 || h > 100) )
    {
        font.setPointSize( font.pointSize() - 1 );
        QFontMetrics s(font);
        w = s.width(OVERZOOM_TEXT);
        h = s.height();
    }
    m_overzoomFont = font;
    m_overzoomTextWidth = w;
    m_overzoomTextHeight = h;
}

emboss_data *glChartCanvas::CreateEmbossMapData( QFont &font, int width, int height, const QString &str, ColorScheme cs )
{
    int *pmap;

    //  Create a temporary bitmap
    wxBitmap bmp( width, height, -1 );

    // Create a memory DC
    QPainter temp_dc;
    temp_dc.begin(bmp.GetHandle());
    temp_dc.setBackground(QBrush(Qt::white));
    QPen pen = temp_dc.pen();
    pen.setColor(Qt::black);
    temp_dc.setPen(pen);
    temp_dc.eraseRect(QRect(0, 0, temp_dc.device()->width(), temp_dc.device()->height()));
    temp_dc.setFont(font );

    int str_w = temp_dc.fontMetrics().width(str), str_h = temp_dc.fontMetrics().height();
    temp_dc.drawText(1, 1, str );
    temp_dc.end();

    //  Convert bitmap the QImage for manipulation
    QImage img = bmp.ConvertToImage();

    int image_width = str_w * 105 / 100;
    int image_height = str_h * 105 / 100;
    QRect r(0,0, fmin(image_width, img.width()), fmin(image_height, img.height()));
    QImage imgs = img.copy(r);

    double val_factor;
    switch( cs ) {
    case GLOBAL_COLOR_SCHEME_DAY:
    default:
        val_factor = 1;
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
        val_factor = .5;
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        val_factor = .25;
        break;
    }

    int val;
    int index;
    const int w = imgs.width();
    const int h = imgs.height();
    pmap = (int *) calloc( w *  h * sizeof(int), 1 );
    //  Create emboss map by differentiating the emboss image
    //  and storing integer results in pmap
    //  n.b. since the image is B/W, it is sufficient to check
    //  one channel (i.e. red) only
    for( int y = 1; y < h - 1; y++ ) {
        for( int x = 1; x < w - 1; x++ ) {
            val = img.pixelColor( x + 1, y + 1 ).red() - img.pixelColor( x - 1, y - 1 ).red();  // range +/- 256
            val = (int) ( val * val_factor );
            index = ( y * w ) + x;
            pmap[index] = val;

        }
    }

    emboss_data *pret = new emboss_data;
    pret->pmap = pmap;
    pret->width = w;
    pret->height = h;

    return pret;
}

void glChartCanvas::CreateDepthUnitEmbossMaps( ColorScheme cs )
{
    ocpnStyle::Style* style = StyleMgrIns->GetCurrentStyle();
    QFont font;
    if( style->embossFont.isEmpty() ){
        QFont dFont = FontMgr::Get().GetFont( ("Dialog"), 0 );
        font = dFont;
        font.setPointSize(60);
        font.setWeight(QFont::Weight::Bold);
    }
    else
    {
        font = QFont( "Micorosoft YH", 60, QFont::Weight::Bold);
    }


    int emboss_width = 500;
    int emboss_height = 200;

    // Free any existing emboss maps
    delete m_pEM_Feet;
    delete m_pEM_Meters;
    delete m_pEM_Fathoms;

    // Create the 3 DepthUnit emboss map structures
    m_pEM_Feet = CreateEmbossMapData( font, emboss_width, emboss_height, ("Feet"), cs );
    m_pEM_Meters = CreateEmbossMapData( font, emboss_width, emboss_height, ("Meters"), cs );
    m_pEM_Fathoms = CreateEmbossMapData( font, emboss_width, emboss_height, ("Fathoms"), cs );
}

void glChartCanvas::CreateOZEmbossMapData( ColorScheme cs )
{
    delete m_pEM_OverZoom;

    if( m_overzoomTextWidth > 0 && m_overzoomTextHeight > 0 )
        m_pEM_OverZoom = CreateEmbossMapData( m_overzoomFont, m_overzoomTextWidth + 10, m_overzoomTextHeight + 10, OVERZOOM_TEXT, cs );
}

void glChartCanvas::CanvasApplyLocale()
{
    CreateDepthUnitEmbossMaps( m_cs );
    CreateOZEmbossMapData( m_cs );
}

void glChartCanvas::SetColorScheme( ColorScheme cs )
{
    CreateDepthUnitEmbossMaps( cs );
    CreateOZEmbossMapData( cs );

    //  Set up fog effect base color
    m_fog_color = QColor( 170, 195, 240 );  // this is gshhs (backgound world chart) ocean color
    float dim = 1.0;
    switch( cs ){
    case GLOBAL_COLOR_SCHEME_DUSK:
        dim = 0.5;
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        dim = 0.25;
        break;
    default:
        break;
    }
    m_fog_color.setRgb(m_fog_color.red()*dim, m_fog_color.green()*dim, m_fog_color.blue()*dim );

    if(pWorldBackgroundChart) pWorldBackgroundChart->SetColorScheme( cs );
    g_glTextureManager->ClearAllRasterTextures();
    mFrameWork->ReloadVP();

    m_cs = cs;
}

void glChartCanvas::buildStyle()
{
    ocpnStyle::Style* style = StyleMgrIns->GetCurrentStyle();
    QImage ICursorLeft = style->GetIcon(("left") ).ConvertToImage();
    QImage ICursorRight = style->GetIcon( ("right") ).ConvertToImage();
    QImage ICursorUp = style->GetIcon( ("up") ).ConvertToImage();
    QImage ICursorDown = style->GetIcon( ("down") ).ConvertToImage();
    QImage ICursorPencil = style->GetIcon( ("pencil") ).ConvertToImage();
    QImage ICursorCross = style->GetIcon( ("cross") ).ConvertToImage();

    if( !ICursorLeft.isNull() ) {
        //        ICursorLeft.set( QImage_OPTION_CUR_HOTSPOT_X, 0 );
        //        ICursorLeft.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorLeft = new QCursor(QPixmap::fromImage(ICursorLeft) );
    } else
        pCursorLeft = new QCursor( Qt::ArrowCursor );

    if( !ICursorRight.isNull() ) {
        //        ICursorRight.SetOption( QImage_OPTION_CUR_HOTSPOT_X, 31 );
        //        ICursorRight.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorRight = new QCursor( QPixmap::fromImage(ICursorRight) );
    } else
        pCursorRight = new QCursor( Qt::ArrowCursor );

    if( !ICursorUp.isNull()  ) {
        //        ICursorUp.SetOption( QImage_OPTION_CUR_HOTSPOT_X, 15 );
        //        ICursorUp.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 0 );
        pCursorUp = new QCursor( QPixmap::fromImage(ICursorUp) );
    } else
        pCursorUp = new QCursor( Qt::ArrowCursor );

    if( !ICursorDown.isNull()  ) {
        //        ICursorDown.SetOption( QImage_OPTION_CUR_HOTSPOT_X, 15 );
        //        ICursorDown.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 31 );
        pCursorDown = new QCursor( QPixmap::fromImage(ICursorDown) );
    } else
        pCursorDown = new QCursor( Qt::ArrowCursor );

    if( !ICursorPencil.isNull() ) {
        //        ICursorPencil.SetOption( QImage_OPTION_CUR_HOTSPOT_X, 0 );
        //        ICursorPencil.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 15 );
        pCursorPencil = new QCursor( QPixmap::fromImage(ICursorPencil) );
    } else
        pCursorPencil = new QCursor( Qt::ArrowCursor );

    if( !ICursorCross.isNull() ) {
        //        ICursorCross.SetOption( QImage_OPTION_CUR_HOTSPOT_X, 13 );
        //        ICursorCross.SetOption( QImage_OPTION_CUR_HOTSPOT_Y, 12 );
        pCursorCross = new QCursor( QPixmap::fromImage(ICursorCross) );
    } else
        pCursorCross = new QCursor( Qt::ArrowCursor );

    pCursorArrow = new QCursor( Qt::ArrowCursor );
    pPlugIn_Cursor = NULL;

    setCursor( *pCursorArrow );

    CreateDepthUnitEmbossMaps( GLOBAL_COLOR_SCHEME_DAY );
    SetOverzoomFont();
    CreateOZEmbossMapData( GLOBAL_COLOR_SCHEME_DAY );



    //    Build Dusk/Night  ownship icons
    double factor_dusk = 0.5;
    double factor_night = 0.25;

    m_b_paint_enable = true;

}

void glChartCanvas::initBeforeUpdateMap()
{
    if(!pInit_Chart_Dir)pInit_Chart_Dir = new QString();
    g_pGroupArray = new ChartGroupArray;
    ZCHX_CFG_INS->loadMyConfig();
    StyleMgrIns->SetStyle("MUI_flat");
    if( !StyleMgrIns->IsOK() ) {
        QString logFile = QApplication::applicationDirPath() + QString("/log/opencpn.log");
        QString msg = ("Failed to initialize the user interface. ");
        msg.append("OpenCPN cannot start. ");
        msg.append("The necessary configuration files were not found. ");
        msg.append("See the log file at ").append(logFile).append(" for details.").append("\n\n");
        QMessageBox::warning(0, "Failed to initialize the user interface. ", msg);
        exit( EXIT_FAILURE );
    }

    g_display_size_mm = fmax(100.0, OCPNPlatform::instance()->GetDisplaySizeMM());
    qDebug("Detected display size (horizontal): %d mm", (int) g_display_size_mm);
    // Instantiate and initialize the Config Manager
//    ConfigMgr::Get();

    //  Validate OpenGL functionality, if selected
    zchxFuncUtil::getMemoryStatus(&g_mem_total, &g_mem_initial);
    if( 0 == g_memCacheLimit ) g_memCacheLimit = (int) ( g_mem_total * 0.5 );
    g_memCacheLimit = fmin(g_memCacheLimit, 1024 * 1024); // math in kBytes, Max is 1 GB

//      Establish location and name of chart database
    ChartListFileName = QString("%1/CHRTLIST.DAT").arg(zchxFuncUtil::getDataDir());
//      Establish guessed location of chart tree
    if( pInit_Chart_Dir->isEmpty() )
    {
        pInit_Chart_Dir->append(zchxFuncUtil::getDataDir());
    }

//      Establish the GSHHS Dataset location
    gDefaultWorldMapLocation = "gshhs";
    gDefaultWorldMapLocation.insert(0, QString("%1/").arg(zchxFuncUtil::getDataDir()));
//    gDefaultWorldMapLocation.Append( wxFileName::GetPathSeparator() );
    if( gWorldMapLocation.isEmpty() || !(QDir(gWorldMapLocation).exists()) ) {
        gWorldMapLocation = gDefaultWorldMapLocation;
    }
    qDebug()<<gWorldMapLocation<<gDefaultWorldMapLocation;
    OCPNPlatform::instance()->Initialize_2();
    InitializeUserColors();
    //  Do those platform specific initialization things that need gFrame
    OCPNPlatform::instance()->Initialize_3();


    //  Clear the cache, and thus close all charts to avoid memory leaks
    if(ChartData) ChartData->PurgeCache();

    //更新视窗的配置
    canvasConfig *config = new canvasConfig();
    config->LoadFromLegacyConfig(ZCHX_CFG_INS);
    config->canvas = this;

    // Verify that glCanvas is ready, if necessary
    config->iLat = 37.123456;
    config->iLon = 127.123456;

    mFrameWork->SetDisplaySizeMM(g_display_size_mm);

    ApplyCanvasConfig(config);
    SetColorScheme( global_color_scheme );
}

bool glChartCanvas::UpdateChartDatabaseInplace( ArrayOfCDI &DirArray, bool b_force, bool b_prog, const QString &ChartListFileName )
{
    bool b_run = false;
    QProgressDialog *pprog = nullptr;
    if( b_prog  && DirArray.count() > 0) {
        pprog = new QProgressDialog(0);
//        pprog->setRange(0, 100);
        pprog->setAttribute(Qt::WA_DeleteOnClose);
        pprog->setWindowTitle(tr("数据更新"));
        pprog->setLabel(new QLabel(tr("正在更新地图数据,请稍候...")));
        pprog->setCancelButtonText(tr("取消"));
        pprog->show();
    }

    // ..For each canvas...
    mFrameWork->InvalidateQuilt();
    mFrameWork->SetQuiltRefChart( -1 );
    mFrameWork->m_singleChart = NULL;
    if(ChartData)   ChartData->PurgeCache();
    setCursor(OCPNPlatform::instance()->ShowBusySpinner());


    qDebug("Starting chart database Update...");
    QString gshhg_chart_loc = gWorldMapLocation;
    gWorldMapLocation.clear();
    ChartData->Update( DirArray, b_force, 0 );
    ChartData->SaveBinary(ChartListFileName);
    qDebug("Finished chart database Update");
    if( gWorldMapLocation.isEmpty() ) { //Last resort. User might have deleted all GSHHG data, but we still might have the default dataset distributed with OpenCPN or from the package repository...
       gWorldMapLocation = gDefaultWorldMapLocation;
       gshhg_chart_loc.clear();
    }

    if( gWorldMapLocation != gshhg_chart_loc )
    {
        ResetWorldBackgroundChart();
    }


//    delete pprog;

    setCursor(OCPNPlatform::instance()->HideBusySpinner());
    ZCHX_CFG_INS->UpdateChartDirs( DirArray );
    if(pprog)
    {
        delete pprog;
    }

    mFrameWork->DoCanvasUpdate();
    mFrameWork->ReloadVP();                  // once more, and good to go
    Zoom(1.0001);
    return true;
}

void glChartCanvas::ApplyCanvasConfig(canvasConfig *pcc)
{
    mFrameWork->SetViewPoint( pcc->iLat, pcc->iLon, pcc->iScale, 0., pcc->iRotation );
    mFrameWork->m_vLat = pcc->iLat;
    mFrameWork->m_vLon = pcc->iLon;

    mFrameWork->m_restore_dbindex = pcc->DBindex;
    if ( pcc->GroupID < 0 )
        pcc->GroupID = 0;

    if( pcc->GroupID > (int) g_pGroupArray->count() )
        mFrameWork->m_groupIndex = 0;
    else
        mFrameWork->m_groupIndex = pcc->GroupID;

    if( pcc->bQuilt != mFrameWork->GetQuiltMode() )
        mFrameWork->ToggleCanvasQuiltMode();

    SetShowDepthUnits( pcc->bShowDepthUnits );
    SetShowGrid( pcc->bShowGrid );
    SetShowOutlines( pcc->bShowOutlines );

    // ENC options
    SetShowENCText( pcc->bShowENCText );
    m_encDisplayCategory = pcc->nENCDisplayCategory;
    m_encShowDepth = pcc->bShowENCDepths;
    m_encShowLightDesc = pcc->bShowENCLightDescriptions;
    m_encShowBuoyLabels = pcc->bShowENCBuoyLabels;
    m_encShowLights = pcc->bShowENCLights;
    mFrameWork->m_singleChart = NULL;

}

bool glChartCanvas::UpdateS52State()
{
    bool retval = false;
    if(ps52plib){
        ps52plib->SetShowS57Text( m_encShowText );
        ps52plib->SetDisplayCategory( (DisCat) m_encDisplayCategory );
        ps52plib->m_bShowSoundg = m_encShowDepth;
        ps52plib->m_bShowAtonText = m_encShowBuoyLabels;
        ps52plib->m_bShowLdisText = m_encShowLightDesc;

        // Lights
        if(!m_encShowLights)                     // On, going off
            ps52plib->AddObjNoshow("LIGHTS");
        else                                   // Off, going on
            ps52plib->RemoveObjNoshow("LIGHTS");
        ps52plib->SetLightsOff( !m_encShowLights );
        ps52plib->m_bExtendLightSectors = true;

        // TODO ps52plib->m_bShowAtons = m_encShowBuoys;
        ps52plib->SetAnchorOn( m_encShowAnchor );
        ps52plib->SetQualityOfData( m_encShowDataQual );
    }

    return retval;
}

void glChartCanvas::SetShowENCDataQual( bool show )
{
    m_encShowDataQual = show;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

void glChartCanvas::SetShowOutlines(bool show)
{
    m_bShowOutlines = show;
    Refresh(false);
    Invalidate();
}


void glChartCanvas::SetShowENCText( bool show )
{
    m_encShowText = show;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure    
    mFrameWork->ReloadVP();
}

void glChartCanvas::SetENCDisplayCategory( int category )
{
    m_encDisplayCategory = category;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}


void glChartCanvas::SetShowENCDepth( bool show )
{
    m_encShowDepth = show;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure    
    mFrameWork->ReloadVP();
}

void glChartCanvas::SetShowENCLightDesc( bool show )
{
    m_encShowLightDesc = show;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

void glChartCanvas::SetShowENCBuoyLabels( bool show )
{
    m_encShowBuoyLabels = show;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

void glChartCanvas::SetShowENCLights( bool show )
{
    m_encShowLights = show;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

void glChartCanvas::SetShowENCAnchor( bool show )
{
    m_encShowAnchor = show;
    m_s52StateHash = 0;         // Force a S52 PLIB re-configure
}

void glChartCanvas::UpdateCanvasS52PLIBConfig()
{
    if(!ps52plib) return;

    if( mFrameWork->mViewPoint.quilt() ){          // quilted
        if( !mFrameWork->m_pQuilt->IsComposed() )
            return;  // not ready

        if(mFrameWork->m_pQuilt->IsQuiltVector()){
            if(ps52plib->GetStateHash() != m_s52StateHash){
                UpdateS52State();
                m_s52StateHash = ps52plib->GetStateHash();
            }
        }
    }
    else{
        if(ps52plib->GetStateHash() != m_s52StateHash){
            UpdateS52State();
            m_s52StateHash = ps52plib->GetStateHash();
        }
    }
}

void glChartCanvas::GridDraw( ocpnDC& dc )
{
    if( !( m_bDisplayGrid && ( fabs( GetVP().rotation() ) < 1e-5 ) ) )
        return;

    double nlat, elon, slat, wlon;
    float lat, lon;
    float dlat, dlon;
    float gridlatMajor, gridlatMinor, gridlonMajor, gridlonMinor;
    int w, h;
    QPen GridPen( GetGlobalColor(  "SNDG1"  ), 1, Qt::SolidLine );
    dc.SetPen( GridPen );
    QFont m_pgridFont = FontMgr::Get().FindOrCreateFont( 8, "Microsoft Yahei", QFont::StyleNormal, QFont::Weight::Normal, false);
    dc.SetFont( m_pgridFont );
    dc.SetTextForeground( GetGlobalColor( ( "SNDG1" ) ) );

    w = mFrameWork->m_canvas_width;
    h = mFrameWork->m_canvas_height;

    mFrameWork->GetCanvasPixPoint( 0, 0, nlat, wlon ); // get lat/lon of upper left point of the window
    mFrameWork->GetCanvasPixPoint( w, h, slat, elon ); // get lat/lon of lower right point of the window
    dlat = nlat - slat; // calculate how many degrees of latitude are shown in the window
    dlon = elon - wlon; // calculate how many degrees of longitude are shown in the window
    if( dlon < 0.0 ) // concider datum border at 180 degrees longitude
    {
        dlon = dlon + 360.0;
    }
    // calculate distance between latitude grid lines
    CalcGridSpacing( GetVP().viewScalePPM(), gridlatMajor, gridlatMinor );

    // calculate position of first major latitude grid line
    lat = ceil( slat / gridlatMajor ) * gridlatMajor;

    // Draw Major latitude grid lines and text
    while( lat < nlat ) {
        zchxPoint r;
        QString st = CalcGridText( lat, gridlatMajor, true ); // get text for grid line
        mFrameWork->GetCanvasPointPix( lat, ( elon + wlon ) / 2, r );
        dc.DrawLine( 0, r.y, w, r.y, false );                             // draw grid line
        dc.drawText( st, 0, r.y ); // draw text
        lat = lat + gridlatMajor;

        if( fabs( lat - qRound( lat ) ) < 1e-5 ) lat = qRound( lat );
    }

    // calculate position of first minor latitude grid line
    lat = ceil( slat / gridlatMinor ) * gridlatMinor;

    // Draw minor latitude grid lines
    while( lat < nlat ) {
        zchxPoint r;
        mFrameWork->GetCanvasPointPix( lat, ( elon + wlon ) / 2, r );
        dc.DrawLine( 0, r.y, 10, r.y, false );
        dc.DrawLine( w - 10, r.y, w, r.y, false );
        lat = lat + gridlatMinor;
    }

    // calculate distance between grid lines
    CalcGridSpacing( GetVP().viewScalePPM(), gridlonMajor, gridlonMinor );

    // calculate position of first major latitude grid line
    lon = ceil( wlon / gridlonMajor ) * gridlonMajor;

    // draw major longitude grid lines
    for( int i = 0, itermax = (int) ( dlon / gridlonMajor ); i <= itermax; i++ ) {
        zchxPoint r;
        QString st = CalcGridText( lon, gridlonMajor, false );
        mFrameWork->GetCanvasPointPix( ( nlat + slat ) / 2, lon, r );
        dc.DrawLine( r.x, 0, r.x, h, false );
        dc.drawText( st, r.x, 0 );
        lon = lon + gridlonMajor;
        if( lon > 180.0 ) {
            lon = lon - 360.0;
        }

        if( fabs( lon - qRound( lon ) ) < 1e-5 ) lon = qRound( lon );

    }

    // calculate position of first minor longitude grid line
    lon = ceil( wlon / gridlonMinor ) * gridlonMinor;
    // draw minor longitude grid lines
    for( int i = 0, itermax = (int) ( dlon / gridlonMinor ); i <= itermax; i++ ) {
        zchxPoint r;
        mFrameWork->GetCanvasPointPix( ( nlat + slat ) / 2, lon, r );
        dc.DrawLine( r.x, 0, r.x, 10, false );
        dc.DrawLine( r.x, h - 10, r.x, h, false );
        lon = lon + gridlonMinor;
        if( lon > 180.0 ) {
            lon = lon - 360.0;
        }
    }
}

void glChartCanvas::ScaleBarDraw( ocpnDC& dc )
{
    if(0 /*!g_bsimplifiedScalebar*/){
        double blat, blon, tlat, tlon;
        zchxPoint r;

        int x_origin = m_bDisplayGrid ? 60 : 20;
        int y_origin = mFrameWork->m_canvas_height - 50;

        float dist;
        int count;
        QPen pen1, pen2;

        if( GetVP().chartScale() > 80000 )        // Draw 10 mile scale as SCALEB11
        {
            dist = 10.0;
            count = 5;
            pen1 = QPen( GetGlobalColor( ( "SNDG2" ) ), 3, Qt::SolidLine );
            pen2 = QPen( GetGlobalColor( ( "SNDG1" ) ), 3, Qt::SolidLine );
        } else                                // Draw 1 mile scale as SCALEB10
        {
            dist = 1.0;
            count = 10;
            pen1 = QPen( GetGlobalColor(  ( "SCLBR" ) ), 3, Qt::SolidLine );
            pen2 = QPen( GetGlobalColor(  ( "CHGRD" ) ), 3, Qt::SolidLine );
        }

        mFrameWork->GetCanvasPixPoint( x_origin, y_origin, blat, blon );
        double rotation = -mFrameWork->mViewPoint.rotation();

        ll_gc_ll( blat, blon, rotation * 180 / PI, dist, &tlat, &tlon );
        mFrameWork->GetCanvasPointPix( tlat, tlon, r );
        int l1 = ( y_origin - r.y ) / count;

        for( int i = 0; i < count; i++ ) {
            int y = l1 * i;
            if( i & 1 ) dc.SetPen( pen1 );
            else
                dc.SetPen( pen2 );

            dc.DrawLine( x_origin, y_origin - y, x_origin, y_origin - ( y + l1 ) );
        }
    }
    else {
        double blat, blon, tlat, tlon;

        int x_origin = 5.0 * mFrameWork->GetPixPerMM();
        //         ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        //         if (style->chartStatusWindowTransparent)
        //             chartbar_height = 0;
        int y_origin = mFrameWork->m_canvas_height  - 25;

        mFrameWork->GetCanvasPixPoint( x_origin, y_origin, blat, blon );
        mFrameWork->GetCanvasPixPoint( x_origin + mFrameWork->m_canvas_width, y_origin, tlat, tlon );

        double d;
        ll_gc_ll_reverse(blat, blon, tlat, tlon, 0, &d);
        d /= 2;

        int unit = g_iDistanceFormat;
        if( d < .5 && ( unit == DISTANCE_KM || unit == DISTANCE_MI || unit == DISTANCE_NMI ) )
            unit = ( unit == DISTANCE_MI ) ? DISTANCE_FT : DISTANCE_M;

        // nice number
        float dist = zchxFuncUtil::toUsrDistance( d, unit );
        float logdist = log(dist) / log(10.F);
        float places = floor(logdist), rem = logdist - places;
        dist = pow(10, places);

        if(rem < .2)
            dist /= 5;
        else if(rem < .5)
            dist /= 2;

        QString s = QString("").sprintf("%g ", dist) + zchxFuncUtil::getUsrDistanceUnit( unit );
        QColor black = GetGlobalColor( "UBLCK"  );
        QPen pen1 = QPen( black , 1, Qt::SolidLine );
        double rotation = -GetVP().rotation();

        ll_gc_ll( blat, blon, rotation * 180 / PI + 90,
                  zchxFuncUtil::fromUsrDistance(dist, unit), &tlat, &tlon );
        zchxPoint r;
        mFrameWork->GetCanvasPointPix( tlat, tlon, r );
        int l1 = r.x - x_origin;

        m_scaleBarRect = QRect(x_origin, y_origin- 12, l1, 12);        // Store this for later reference

        dc.SetPen(pen1);

        dc.DrawLine( x_origin, y_origin, x_origin, y_origin - 12);
        dc.DrawLine( x_origin, y_origin, x_origin + l1, y_origin);
        dc.DrawLine( x_origin + l1, y_origin, x_origin + l1, y_origin - 12);
        QFont m_pgridFont = FontMgr::Get().FindOrCreateFont( 8, "Microsoft Yahei", QFont::StyleNormal, QFont::Weight::Normal, false);
        dc.SetFont( m_pgridFont );
        dc.SetTextForeground( black );
        int w, h;
        dc.GetTextExtent(s, &w, &h);
        dc.drawText( s, x_origin + l1/2 - w/2, y_origin - h - 1 );
    }
}

ChartBase* glChartCanvas::GetChartAtCursor()
{
    return mFrameWork->GetChartAtPixel(mouse_x, mouse_y);
}

ChartBase* glChartCanvas::GetOverlayChartAtCursor()
{
    return mFrameWork->GetOverlayChartAtPixel(mouse_x, mouse_y);
}

void glChartCanvas::Refresh( bool eraseBackground, const QRect *rect )
{
    if(GetQuilting())  return;
    //  Keep the mouse position members up to date
    mFrameWork->GetCanvasPixPoint( mouse_x, mouse_y, m_cursor_lat, m_cursor_lon );
    if( eraseBackground && UsingFBO() )    Invalidate();
}

#define NUM_CURRENT_ARROW_POINTS 9
static zchxPoint CurrentArrowArray[NUM_CURRENT_ARROW_POINTS] = { zchxPoint( 0, 0 ), zchxPoint( 0, -10 ),
                                                               zchxPoint( 55, -10 ), zchxPoint( 55, -25 ), zchxPoint( 100, 0 ), zchxPoint( 55, 25 ), zchxPoint( 55,
                                                               10 ), zchxPoint( 0, 10 ), zchxPoint( 0, 0 )
                                                             };

void glChartCanvas::DrawArrow( ocpnDC& dc, int x, int y, double rot_angle, double scale )
{
    if( scale > 1e-2 ) {

        float sin_rot = sin( rot_angle * PI / 180. );
        float cos_rot = cos( rot_angle * PI / 180. );

        // Move to the first point

        float xt = CurrentArrowArray[0].x;
        float yt = CurrentArrowArray[0].y;

        float xp = ( xt * cos_rot ) - ( yt * sin_rot );
        float yp = ( xt * sin_rot ) + ( yt * cos_rot );
        int x1 = (int) ( xp * scale );
        int y1 = (int) ( yp * scale );

        // Walk thru the point list
        for( int ip = 1; ip < NUM_CURRENT_ARROW_POINTS; ip++ ) {
            xt = CurrentArrowArray[ip].x;
            yt = CurrentArrowArray[ip].y;

            float xp = ( xt * cos_rot ) - ( yt * sin_rot );
            float yp = ( xt * sin_rot ) + ( yt * cos_rot );
            int x2 = (int) ( xp * scale );
            int y2 = (int) ( yp * scale );

            dc.DrawLine( x1 + x, y1 + y, x2 + x, y2 + y );

            x1 = x2;
            y1 = y2;
        }
    }
}

void glChartCanvas::GetCursorLatLon( double *rlat, double *rlon )
{
    double lat, lon;
    mFrameWork->GetCanvasPixPoint( mouse_x, mouse_y, lat, lon );
    *rlat = lat;
    *rlon = lon;
}

void glChartCanvas::updateCurrentLL()
{
    mFrameWork->GetCanvasPixPoint( mouse_x, mouse_y, m_cursor_lat, m_cursor_lon );
}

zchxPoint glChartCanvas::getCurrentPos()
{
    return zchxPoint(mouse_x, mouse_y);
}

zchxPointF glChartCanvas::getCurrentLL()
{
    return zchxPointF(m_cursor_lon, m_cursor_lat);
}

void glChartCanvas::setCurLL(double lat, double lon)
{
    m_cursor_lat = lat;
    m_cursor_lon = lon;
    double old_x = mouse_x;
    double old_y = mouse_y;
    mFrameWork->GetCanvasPixPoint(mouse_x, mouse_y, lat, lon);
}

bool glChartCanvas::GetQuiltMode()
{
    return mFrameWork->GetQuiltMode();
}

void glChartCanvas::ToggleCanvasQuiltMode()
{
    mFrameWork->ToggleCanvasQuiltMode();
}

double glChartCanvas::GetPixPerMM()
{
    return mFrameWork->GetPixPerMM();
}

bool glChartCanvas::Pan( double dx, double dy )
{
    return mFrameWork->Pan(dx, dy);
}

void glChartCanvas::Zoom( double factor,  bool can_zoom_to_cursor )
{
    if(mFrameWork->isZoomNow()) return;
    mFrameWork->Zoom(factor, can_zoom_to_cursor);
}

void glChartCanvas::Rotate(double rad)
{
    mFrameWork->Rotate(rad);
}

void glChartCanvas::RotateDegree(double degree)
{
    mFrameWork->RotateDegree(degree);
}

void glChartCanvas::RotateContinus(double dir)
{
    mFrameWork->RotateContinus(dir);
}

void glChartCanvas::slotStartLoadEcdis()
{
    buildStyle();
    initBeforeUpdateMap();
    mFrameWork->slotInitEcidsAsDelayed();
}










