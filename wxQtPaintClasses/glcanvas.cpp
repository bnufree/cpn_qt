/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/glcanvas.cpp
// Author:      Sean D'Epagnier
// Copyright:   (c) Sean D'Epagnier 2014
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

//#include "wx/qt/private/winevent.h"
//#include "wx/glcanvas.h"

#include <QtOpenGL/QGLWidget>
#include "glcanvas.h"
#include <QDebug>


// ============================================================================
// implementation
// ============================================================================

void wxGLAttribsBase::AddAttribBits(int searchVal, int combineVal)
{
    // Search for searchVal
    QVector<int>::iterator it = m_GLValues.begin();
    while ( it != m_GLValues.end() && *it != searchVal )
        it++;
    // Have we searchVal?
    if ( it != m_GLValues.end() )
    {
        if ( ++it == m_GLValues.end() )
        {
            m_GLValues.push_back(combineVal);
        }
        else
        {
            *it |= combineVal;
        }
    }
    else
    {
        // Add the identifier and the bits
        m_GLValues.push_back(searchVal);
        m_GLValues.push_back(combineVal);
    }
}

// ============================================================================

wxGLCanvasBase::wxGLCanvasBase()
{
#if WXWIN_COMPATIBILITY_2_8
    m_glContext = NULL;
#endif

    // we always paint background entirely ourselves so prevent wx from erasing
    // it to avoid flicker
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

bool wxGLCanvasBase::SetCurrent(const wxGLContext& context) const
{
    // although on MSW it works even if the window is still hidden, it doesn't
    // work in other ports (notably X11-based ones) and documentation mentions
    // that SetCurrent() can only be called for a shown window, so check for it
    wxASSERT_MSG( IsShown(), wxT("can't make hidden GL canvas current") );


    return context.SetCurrent(*static_cast<const wxGLCanvas *>(this));
}

bool wxGLCanvasBase::SetColour(const QString& colour)
{
    QColor col;
    col.setNamedColor(colour);
    if ( !col.isValid() )  return false;

#ifdef wxHAS_OPENGL_ES
    wxGLAPI::glColor3f((GLfloat) (col.Red() / 256.), (GLfloat) (col.Green() / 256.),
                (GLfloat) (col.Blue() / 256.));
#else
    GLboolean isRGBA;
    glGetBooleanv(GL_RGBA_MODE, &isRGBA);
    if ( isRGBA )
    {
        glColor3f((GLfloat) (col.red() / 256.), (GLfloat) (col.green() / 256.), (GLfloat) (col.blue() / 256.));
    }
    else // indexed colour
    {
        GLint pix = GetColourIndex(col);
        if ( pix == -1 )
        {
            qDebug(_("Failed to allocate colour for OpenGL"));
            return false;
        }

        glIndexi(pix);
    }
#endif
    return true;
}

wxGLCanvasBase::~wxGLCanvasBase()
{
#if WXWIN_COMPATIBILITY_2_8
    delete m_glContext;
#endif // WXWIN_COMPATIBILITY_2_8
}

#if WXWIN_COMPATIBILITY_2_8

wxGLContext *wxGLCanvasBase::GetContext() const
{
    return m_glContext;
}

void wxGLCanvasBase::SetCurrent()
{
    if ( m_glContext )
        SetCurrent(*m_glContext);
}

void wxGLCanvasBase::OnSize(wxSizeEvent& WXUNUSED(event))
{
}

#endif // WXWIN_COMPATIBILITY_2_8

/* static */
bool wxGLCanvasBase::IsExtensionInList(const char *list, const char *extension)
{
    if ( !list )
        return false;

    for ( const char *p = list; *p; p++ )
    {
        // advance up to the next possible match
        p = wxStrstr(p, extension);
        if ( !p )
            break;

        // check that the extension appears at the beginning/ending of the list
        // or is preceded/followed by a space to avoid mistakenly finding
        // "glExtension" in a list containing some "glFunkyglExtension"
        if ( (p == list || p[-1] == ' ') )
        {
            char c = p[strlen(extension)];
            if ( c == '\0' || c == ' ' )
                return true;
        }
    }

    return false;
}

/* static */
bool wxGLCanvasBase::ParseAttribList(const int *attribList,
                                     wxGLAttributes& dispAttrs,
                                     wxGLContextAttrs* ctxAttrs)
{
    // Some attributes are usually needed
    dispAttrs.PlatformDefaults();
    if ( ctxAttrs )
        ctxAttrs->PlatformDefaults();

    if ( !attribList )
    {
        // Default visual attributes used in wx versions before wx3.1
        dispAttrs.AddDefaultsForWXBefore31();
        dispAttrs.EndList();
        if ( ctxAttrs )
            ctxAttrs->EndList();
        return true;
    }

    int src = 0;
    int minColo[4] = {-1, -1, -1, -1};
    int minAcum[4] = {-1, -1, -1, -1};
    int num = 0;
    while ( attribList[src] )
    {
        // Check a non zero-terminated list. This may help a bit with malformed lists.
        if ( ++num > 200 )
        {
            wxFAIL_MSG("The attributes list is not zero-terminated");
        }

        switch ( attribList[src++] )
        {
            // Pixel format attributes

            case WX_GL_RGBA:
                dispAttrs.RGBA();
                break;

            case WX_GL_BUFFER_SIZE:
                dispAttrs.BufferSize(attribList[src++]);
                break;

            case WX_GL_LEVEL:
                dispAttrs.Level(attribList[src++]);
                break;

            case WX_GL_DOUBLEBUFFER:
                dispAttrs.DoubleBuffer();
                break;

            case WX_GL_STEREO:
                dispAttrs.Stereo();
                break;

            case WX_GL_AUX_BUFFERS:
                dispAttrs.AuxBuffers(attribList[src++]);
                break;

            case WX_GL_MIN_RED:
                minColo[0] = attribList[src++];
                break;

            case WX_GL_MIN_GREEN:
                minColo[1] = attribList[src++];
                break;

            case WX_GL_MIN_BLUE:
                minColo[2] = attribList[src++];
                break;

            case WX_GL_MIN_ALPHA:
                minColo[3] = attribList[src++];
                break;

            case WX_GL_DEPTH_SIZE:
                dispAttrs.Depth(attribList[src++]);
                break;

            case WX_GL_STENCIL_SIZE:
                dispAttrs.Stencil(attribList[src++]);
                break;

            case WX_GL_MIN_ACCUM_RED:
                minAcum[0] = attribList[src++];
                break;

            case WX_GL_MIN_ACCUM_GREEN:
                minAcum[1] = attribList[src++];
                break;

            case WX_GL_MIN_ACCUM_BLUE:
                minAcum[2] = attribList[src++];
                break;

            case WX_GL_MIN_ACCUM_ALPHA:
                minAcum[3] = attribList[src++];
                break;

            case WX_GL_SAMPLE_BUFFERS:
                dispAttrs.SampleBuffers(attribList[src++]);
                break;

            case WX_GL_SAMPLES:
                dispAttrs.Samplers(attribList[src++]);
                break;

            case WX_GL_FRAMEBUFFER_SRGB:
                dispAttrs.FrameBuffersRGB();
                break;

            // Context attributes

            case WX_GL_CORE_PROFILE:
                if ( ctxAttrs )
                    ctxAttrs->CoreProfile();
                break;

            case WX_GL_MAJOR_VERSION:
                if ( ctxAttrs )
                    ctxAttrs->MajorVersion(attribList[src]);
                src++;
                break;

            case WX_GL_MINOR_VERSION:
                if ( ctxAttrs )
                    ctxAttrs->MinorVersion(attribList[src]);
                src++;
                break;

            case wx_GL_COMPAT_PROFILE:
                if ( ctxAttrs )
                    ctxAttrs->CompatibilityProfile();
                break;

            case WX_GL_FORWARD_COMPAT:
                if ( ctxAttrs )
                    ctxAttrs->ForwardCompatible();
                break;

            case WX_GL_ES2:
                if ( ctxAttrs )
                    ctxAttrs->ES2();
                break;

            case WX_GL_DEBUG:
                if ( ctxAttrs )
                    ctxAttrs->DebugCtx();
                break;

            case WX_GL_ROBUST_ACCESS:
                if ( ctxAttrs )
                    ctxAttrs->Robust();
                break;

            case WX_GL_NO_RESET_NOTIFY:
                if ( ctxAttrs )
                    ctxAttrs->NoResetNotify();
                break;

            case WX_GL_LOSE_ON_RESET:
                if ( ctxAttrs )
                    ctxAttrs->LoseOnReset();
                break;

            case WX_GL_RESET_ISOLATION:
                if ( ctxAttrs )
                    ctxAttrs->ResetIsolation();
                break;

            case WX_GL_RELEASE_FLUSH:
                if ( ctxAttrs )
                    ctxAttrs->ReleaseFlush(1);
                break;

            case WX_GL_RELEASE_NONE:
                if ( ctxAttrs )
                    ctxAttrs->ReleaseFlush(0);
                break;

            default:
                wxFAIL_MSG("Unexpected value in attributes list");
                return false;
        }
    }

    // Set color and accumulation
    if ( minColo[0] >= 0 || minColo[1] >= 0 || minColo[2] >= 0 || minColo[3] >= 0 )
        dispAttrs.MinRGBA(minColo[0], minColo[1], minColo[2], minColo[3]);
    if ( minAcum[0] >= 0 || minAcum[1] >= 0 || minAcum[2] >= 0 || minAcum[3] >= 0 )
        dispAttrs.MinAcumRGBA(minAcum[0], minAcum[1], minAcum[2], minAcum[3]);

    // The attributes lists must be zero-terminated
    dispAttrs.EndList();
    if ( ctxAttrs )
        ctxAttrs->EndList();

    return true;
}

// ============================================================================
// compatibility layer for OpenGL 3 and OpenGL ES
// ============================================================================

static wxGLAPI s_glAPI;

#if wxUSE_OPENGL_EMULATION

#include "wx/vector.h"

static GLenum s_mode;

static GLfloat s_currentTexCoord[2];
static GLfloat s_currentColor[4];
static GLfloat s_currentNormal[3];

// TODO move this into a different construct with locality for all attributes
// of a vertex

static wxVector<GLfloat> s_texCoords;
static wxVector<GLfloat> s_vertices;
static wxVector<GLfloat> s_normals;
static wxVector<GLfloat> s_colors;

static bool s_texCoordsUsed;
static bool s_colorsUsed;
static bool s_normalsUsed;

bool SetState( int flag, bool desired )
{
    bool former = glIsEnabled( flag );
    if ( former != desired )
    {
        if ( desired )
            glEnableClientState(flag);
        else
            glDisableClientState(flag);
    }
    return former;
}

void RestoreState( int flag, bool desired )
{
    if ( desired )
        glEnableClientState(flag);
    else
        glDisableClientState(flag);
}
#endif

wxGLAPI::wxGLAPI()
{
#if wxUSE_OPENGL_EMULATION
    s_mode = 0xFF;
#endif
}

wxGLAPI::~wxGLAPI()
{
}

void wxGLAPI::glFrustum(GLfloat left, GLfloat right, GLfloat bottom,
                            GLfloat top, GLfloat zNear, GLfloat zFar)
{
#if wxUSE_OPENGL_EMULATION
    ::glFrustumf(left, right, bottom, top, zNear, zFar);
#else
    ::glFrustum(left, right, bottom, top, zNear, zFar);
#endif
}

void wxGLAPI::glBegin(GLenum mode)
{
#if wxUSE_OPENGL_EMULATION
    if ( s_mode != 0xFF )
    {
        wxFAIL_MSG("nested glBegin");
    }

    s_mode = mode;
    s_texCoordsUsed = false;
    s_colorsUsed = false;
    s_normalsUsed = false;

    s_texCoords.clear();
    s_normals.clear();
    s_colors.clear();
    s_vertices.clear();
#else
    ::glBegin(mode);
#endif
}

void wxGLAPI::glTexCoord2f(GLfloat s, GLfloat t)
{
#if wxUSE_OPENGL_EMULATION
    if ( s_mode == 0xFF )
    {
        wxFAIL_MSG("glTexCoord2f called outside glBegin/glEnd");
    }

    else
    {
        s_texCoordsUsed = true;
        s_currentTexCoord[0] = s;
        s_currentTexCoord[1] = t;
    }
#else
    ::glTexCoord2f(s,t);
#endif
}

void wxGLAPI::glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
#if wxUSE_OPENGL_EMULATION
    if ( s_mode == 0xFF )
    {
        wxFAIL_MSG("glVertex3f called outside glBegin/glEnd");
    }
    else
    {
        s_texCoords.push_back(s_currentTexCoord[0]);
        s_texCoords.push_back(s_currentTexCoord[1]);

        s_normals.push_back(s_currentNormal[0]);
        s_normals.push_back(s_currentNormal[1]);
        s_normals.push_back(s_currentNormal[2]);

        s_colors.push_back(s_currentColor[0]);
        s_colors.push_back(s_currentColor[1]);
        s_colors.push_back(s_currentColor[2]);
        s_colors.push_back(s_currentColor[3]);

        s_vertices.push_back(x);
        s_vertices.push_back(y);
        s_vertices.push_back(z);
    }
#else
    ::glVertex3f(x,y,z);
#endif
}

void wxGLAPI::glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
#if wxUSE_OPENGL_EMULATION
    if ( s_mode == 0xFF )
        ::glNormal3f(nx,ny,nz);
    else
    {
        s_normalsUsed = true;
        s_currentNormal[0] = nx;
        s_currentNormal[1] = ny;
        s_currentNormal[2] = nz;
    }
#else
    ::glNormal3f(nx,ny,nz);
#endif
}

void wxGLAPI::glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
#if wxUSE_OPENGL_EMULATION
    if ( s_mode == 0xFF )
        ::glColor4f(r,g,b,a);
    else
    {
        s_colorsUsed = true;
        s_currentColor[0] = r;
        s_currentColor[1] = g;
        s_currentColor[2] = b;
        s_currentColor[3] = a;
    }
#else
    ::glColor4f(r,g,b,a);
#endif
}

void wxGLAPI::glColor3f(GLfloat r, GLfloat g, GLfloat b)
{
#if wxUSE_OPENGL_EMULATION
    glColor4f(r,g,b,1.0);
#else
    ::glColor3f(r,g,b);
#endif
}

void wxGLAPI::glEnd()
{
#if wxUSE_OPENGL_EMULATION
    bool formerColors = SetState( GL_COLOR_ARRAY, s_colorsUsed );
    bool formerNormals = SetState( GL_NORMAL_ARRAY, s_normalsUsed );
    bool formerTexCoords = SetState( GL_TEXTURE_COORD_ARRAY, s_texCoordsUsed );
    bool formerVertex = glIsEnabled(GL_VERTEX_ARRAY);

    if( !formerVertex )
        glEnableClientState(GL_VERTEX_ARRAY);

    if ( s_colorsUsed )
        glColorPointer( 4, GL_FLOAT, 0, &s_colors[0] );

    if ( s_normalsUsed )
        glNormalPointer( GL_FLOAT, 0, &s_normals[0] );

    if ( s_texCoordsUsed )
        glTexCoordPointer( 2, GL_FLOAT, 0, &s_texCoords[0] );

    glVertexPointer(3, GL_FLOAT, 0, &s_vertices[0]);
    glDrawArrays( s_mode, 0, s_vertices.size() / 3 );

    if ( s_colorsUsed != formerColors )
        RestoreState( GL_COLOR_ARRAY, formerColors );

    if ( s_normalsUsed != formerNormals )
        RestoreState( GL_NORMAL_ARRAY, formerColors );

    if ( s_texCoordsUsed != formerTexCoords )
        RestoreState( GL_TEXTURE_COORD_ARRAY, formerColors );

    if( !formerVertex )
        glDisableClientState(GL_VERTEX_ARRAY);

    s_mode = 0xFF;
#else
    ::glEnd();
#endif
}




class wxQtGLWidget : public wxQtEventSignalHandler< QGLWidget, wxGLCanvas >
{
public:
    wxQtGLWidget(wxWindow *parent, wxGLCanvas *handler, QGLFormat format)
        : wxQtEventSignalHandler<QGLWidget,wxGLCanvas>(parent, handler)
        {
            setFormat(format);
            setAutoBufferSwap( false );
        }

protected:
    virtual void showEvent ( QShowEvent * event );
    virtual void hideEvent ( QHideEvent * event );
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void paintEvent ( QPaintEvent * event );

    virtual void resizeGL(int w, int h);
    virtual void paintGL();
};

void wxQtGLWidget::showEvent ( QShowEvent * event )
{
    QGLWidget::showEvent( event );
}

void wxQtGLWidget::hideEvent ( QHideEvent * event )
{
    QGLWidget::hideEvent( event );
}

void wxQtGLWidget::resizeEvent ( QResizeEvent * event )
{
    QGLWidget::resizeEvent(event);
}

void wxQtGLWidget::paintEvent ( QPaintEvent * event )
{
    QGLWidget::paintEvent(event);
}

void wxQtGLWidget::resizeGL(int w, int h)
{
    wxSizeEvent event( wxSize(w, h) );
    EmitEvent(event);
}

void wxQtGLWidget::paintGL()
{
    wxPaintEvent event( GetHandler()->GetId() );
    EmitEvent(event);
}

wxGLContextAttrs& wxGLContextAttrs::CoreProfile()
{
//    AddAttribBits(GLX_CONTEXT_PROFILE_MASK_ARB,
//                  GLX_CONTEXT_CORE_PROFILE_BIT_ARB);
    SetNeedsARB();
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::MajorVersion(int val)
{
    if ( val > 0 )
    {
        if ( val >= 3 )
            SetNeedsARB();
    }
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::MinorVersion(int val)
{
    if ( val >= 0 )
    {
    }
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::CompatibilityProfile()
{
    SetNeedsARB();
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::ForwardCompatible()
{
    SetNeedsARB();
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::ES2()
{
    SetNeedsARB();
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::DebugCtx()
{
    SetNeedsARB();
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::Robust()
{
    SetNeedsARB();
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::NoResetNotify()
{
    SetNeedsARB();
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::LoseOnReset()
{
    SetNeedsARB();
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::ResetIsolation()
{
    SetNeedsARB();
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::ReleaseFlush(int val)
{
    SetNeedsARB();
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::PlatformDefaults()
{
    renderTypeRGBA = true;
    return *this;
}

void wxGLContextAttrs::EndList()
{
//    AddAttribute(None);
}

// ----------------------------------------------------------------------------
// wxGLAttributes: Visual/FBconfig attributes
// ----------------------------------------------------------------------------
// GLX specific values

//   Different versions of GLX API use rather different attributes lists, see
//   the following URLs:
//
//   - <= 1.2: http://www.opengl.org/sdk/docs/man/xhtml/glXChooseVisual.xml
//   - >= 1.3: http://www.opengl.org/sdk/docs/man/xhtml/glXChooseFBConfig.xml
//
//   Notice in particular that
//   - GLX_RGBA is boolean attribute in the old version of the API but a
//     value of GLX_RENDER_TYPE in the new one
//   - Boolean attributes such as GLX_DOUBLEBUFFER don't take values in the
//     old version but must be followed by True or False in the new one.

wxGLAttributes& wxGLAttributes::RGBA()
{
    return *this;
}

wxGLAttributes& wxGLAttributes::BufferSize(int val)
{
    if ( val >= 0 )
    {
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::Level(int val)
{
//    AddAttribute(GLX_LEVEL);
    AddAttribute(val);
    return *this;
}

wxGLAttributes& wxGLAttributes::DoubleBuffer()
{
    return *this;
}

wxGLAttributes& wxGLAttributes::Stereo()
{
    return *this;
}

wxGLAttributes& wxGLAttributes::AuxBuffers(int val)
{
    if ( val >= 0 )
    {
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::MinRGBA(int mRed, int mGreen, int mBlue, int mAlpha)
{
    if ( mRed >= 0)
    {
    }
    if ( mGreen >= 0)
    {
    }
    if ( mBlue >= 0)
    {
    }
    if ( mAlpha >= 0)
    {
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::Depth(int val)
{
    if ( val >= 0 )
    {
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::Stencil(int val)
{
    if ( val >= 0 )
    {
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::MinAcumRGBA(int mRed, int mGreen, int mBlue, int mAlpha)
{
    if ( mRed >= 0)
    {
    }
    if ( mGreen >= 0)
    {
    }
    if ( mBlue >= 0)
    {
    }
    if ( mAlpha >= 0)
    {
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::SampleBuffers(int val)
{
#ifdef GLX_SAMPLE_BUFFERS_ARB
    if ( val >= 0 && wxGLCanvasX11::IsGLXMultiSampleAvailable() )
    {
        AddAttribute(GLX_SAMPLE_BUFFERS_ARB);
        AddAttribute(val);
    }
#endif
    return *this;
}

wxGLAttributes& wxGLAttributes::Samplers(int val)
{
#ifdef GLX_SAMPLES_ARB
    if ( val >= 0 && wxGLCanvasX11::IsGLXMultiSampleAvailable() )
    {
        AddAttribute(GLX_SAMPLES_ARB);
        AddAttribute(val);
    }
#endif
    return *this;
}

wxGLAttributes& wxGLAttributes::FrameBuffersRGB()
{
//    AddAttribute(GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB);
//    AddAttribute(True);
    return *this;
}

void wxGLAttributes::EndList()
{
}

wxGLAttributes& wxGLAttributes::PlatformDefaults()
{
    // No GLX specific values
    return *this;
}

wxGLAttributes& wxGLAttributes::Defaults()
{
    RGBA().DoubleBuffer();
//    if ( wxGLCanvasX11::GetGLXVersion() < 13 )
//        Depth(1).MinRGBA(1, 1, 1, 0);
//    else
        Depth(16).SampleBuffers(1).Samplers(4);
    return *this;
}

void wxGLAttributes::AddDefaultsForWXBefore31()
{
    Defaults();
}

//---------------------------------------------------------------------------
// wxGlContext
//---------------------------------------------------------------------------

wxIMPLEMENT_CLASS(wxGLContext, wxWindow);

wxGLContext::wxGLContext(wxGLCanvas *WXUNUSED(win), const wxGLContext* WXUNUSED(other), const wxGLContextAttrs *WXUNUSED(ctxAttrs))
{
//    m_glContext = win->GetHandle()->context();
}

bool wxGLContext::SetCurrent(const wxGLCanvas&) const
{
// I think I must destroy and recreate the QGLWidget to change the context?
//    win->GetHandle()->makeCurrent();
    return false;
}

//---------------------------------------------------------------------------
// wxGlCanvas
//---------------------------------------------------------------------------

wxIMPLEMENT_CLASS(wxGLCanvas, wxWindow);

wxGLCanvas::wxGLCanvas(wxWindow *parent,
                       const wxGLAttributes& dispAttrs,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const QString& name,
                       const wxPalette& palette)
{
    Create(parent, dispAttrs, id, pos, size, style, name, palette);
}

wxGLCanvas::wxGLCanvas(wxWindow *parent,
                       wxWindowID id,
                       const int *attribList,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const QString& name,
                       const wxPalette& palette)
{
    Create(parent, id, pos, size, style, name, attribList, palette);
}

bool wxGLCanvas::Create(wxWindow *parent,
                        const wxGLAttributes& dispAttrs,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const QString& name,
                        const wxPalette& palette)
{
    wxLogError("Missing implementation of " + QString(__FUNCTION__));
    return false;
}

bool wxGLCanvas::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const QString& name,
                        const int *attribList,
                        const wxPalette& palette)
{
#if wxUSE_PALETTE
    wxASSERT_MSG( !palette.IsOk(), wxT("palettes not supported") );
#endif // wxUSE_PALETTE
    wxUnusedVar(palette); // Unused when wxDEBUG_LEVEL==0

    QGLFormat format;
    if (!wxGLCanvas::ConvertWXAttrsToQtGL(attribList, format))
        return false;

    m_qtWindow = new wxQtGLWidget(parent, this, format);

    return wxWindow::Create( parent, id, pos, size, style, name );
}

bool wxGLCanvas::SwapBuffers()
{
    static_cast<QGLWidget *>(m_qtWindow)->swapBuffers();
    return true;
}

/* static */
bool wxGLCanvas::ConvertWXAttrsToQtGL(const int *wxattrs, QGLFormat &format)
{
    if (!wxattrs) return true;
    return true;

    // set default parameters to false
    format.setDoubleBuffer(false);
    format.setDepth(false);
    format.setAlpha(false);
    format.setStencil(false);

    for ( int arg = 0; wxattrs[arg] != 0; arg++ )
    {
        // indicates whether we have a boolean attribute
        bool isBoolAttr = false;

        int v = wxattrs[arg+1];
        switch ( wxattrs[arg] )
        {
            case WX_GL_BUFFER_SIZE:
                format.setRgba(false);
                // I do not know how to set the buffer size, so fail
                return false;

            case WX_GL_LEVEL:
                format.setPlane(v);
                break;

            case WX_GL_RGBA:
                format.setRgba(true);
                isBoolAttr = true;
                break;

            case WX_GL_DOUBLEBUFFER:
                format.setDoubleBuffer(true);
                isBoolAttr = true;
                break;

            case WX_GL_STEREO:
                format.setStereo(true);
                isBoolAttr = true;
                break;

            case WX_GL_AUX_BUFFERS:
                // don't know how to implement
                return false;

            case WX_GL_MIN_RED:
                format.setRedBufferSize(v*8);
                break;

            case WX_GL_MIN_GREEN:
                format.setGreenBufferSize(v);
                break;

            case WX_GL_MIN_BLUE:
                format.setBlueBufferSize(v);
                break;

            case WX_GL_MIN_ALPHA:
                format.setAlpha(true);
                format.setAlphaBufferSize(v);
                break;

            case WX_GL_DEPTH_SIZE:
                format.setDepth(true);
                format.setDepthBufferSize(v);
                break;

            case WX_GL_STENCIL_SIZE:
                format.setStencil(true);
                format.setStencilBufferSize(v);
                break;

            case WX_GL_MIN_ACCUM_RED:
            case WX_GL_MIN_ACCUM_GREEN:
            case WX_GL_MIN_ACCUM_BLUE:
            case WX_GL_MIN_ACCUM_ALPHA:
                format.setAccumBufferSize(v);
                break;

            case WX_GL_SAMPLE_BUFFERS:
                format.setSampleBuffers(v);
                // can we somehow indicate if it's not supported?
                break;

            case WX_GL_SAMPLES:
                format.setSamples(v);
                // can we somehow indicate if it's not supported?
                break;

            default:
                wxLogDebug(wxT("Unsupported OpenGL attribute %d"),
                           wxattrs[arg]);
                continue;
        }

        if ( !isBoolAttr ) {
            if ( !v )
                return false; // zero parameter
            arg++;
        }
    }

    return true;
}

/* static */
bool
wxGLCanvasBase::IsDisplaySupported(const int *attribList)
{
    QGLFormat format;

    if (!wxGLCanvas::ConvertWXAttrsToQtGL(attribList, format))
        return false;

    return QGLWidget(format).isValid();
}

/* static */
bool
wxGLCanvasBase::IsDisplaySupported(const wxGLAttributes& dispAttrs)
{
    wxLogError("Missing implementation of " + QString(__FUNCTION__));
    return false;
}

// ----------------------------------------------------------------------------
// wxGLApp
// ----------------------------------------------------------------------------

bool wxGLApp::InitGLVisual(const int *attribList)
{
    wxLogError("Missing implementation of " + QString(__FUNCTION__));
    return false;
}

