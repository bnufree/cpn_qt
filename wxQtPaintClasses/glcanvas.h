/////////////////////////////////////////////////////////////////////////////
// Name:        include/wx/qt/glcanvas.cpp
// Author:      Sean D'Epagnier
// Copyright:   (c) Sean D'Epagnier 2014
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GLCANVAS_H_
#define _WX_GLCANVAS_H_

#include <GL/gl.h>
#include <QVector>

class QGLWidget;
class QGLContext;
class QGLFormat;
class wxGLCanvas;
class wxGLContext;

// ----------------------------------------------------------------------------
// Constants for attributes list
// ----------------------------------------------------------------------------

// Notice that not all implementation support options such as stereo, auxiliary
// buffers, alpha channel, and accumulator buffer, use IsDisplaySupported() to
// check for individual attributes support.
enum
{
    WX_GL_RGBA = 1,        // use true color palette (on if no attrs specified)
    WX_GL_BUFFER_SIZE,     // bits for buffer if not WX_GL_RGBA
    WX_GL_LEVEL,           // 0 for main buffer, >0 for overlay, <0 for underlay
    WX_GL_DOUBLEBUFFER,    // use double buffering (on if no attrs specified)
    WX_GL_STEREO,          // use stereoscopic display
    WX_GL_AUX_BUFFERS,     // number of auxiliary buffers
    WX_GL_MIN_RED,         // use red buffer with most bits (> MIN_RED bits)
    WX_GL_MIN_GREEN,       // use green buffer with most bits (> MIN_GREEN bits)
    WX_GL_MIN_BLUE,        // use blue buffer with most bits (> MIN_BLUE bits)
    WX_GL_MIN_ALPHA,       // use alpha buffer with most bits (> MIN_ALPHA bits)
    WX_GL_DEPTH_SIZE,      // bits for Z-buffer (0,16,32)
    WX_GL_STENCIL_SIZE,    // bits for stencil buffer
    WX_GL_MIN_ACCUM_RED,   // use red accum buffer with most bits (> MIN_ACCUM_RED bits)
    WX_GL_MIN_ACCUM_GREEN, // use green buffer with most bits (> MIN_ACCUM_GREEN bits)
    WX_GL_MIN_ACCUM_BLUE,  // use blue buffer with most bits (> MIN_ACCUM_BLUE bits)
    WX_GL_MIN_ACCUM_ALPHA, // use alpha buffer with most bits (> MIN_ACCUM_ALPHA bits)
    WX_GL_SAMPLE_BUFFERS,  // 1 for multisampling support (antialiasing)
    WX_GL_SAMPLES,         // 4 for 2x2 antialiasing supersampling on most graphics cards
    WX_GL_FRAMEBUFFER_SRGB,// capability for sRGB framebuffer
    // Context attributes
    WX_GL_CORE_PROFILE,    // use an OpenGL core profile
    WX_GL_MAJOR_VERSION,   // major OpenGL version of the core profile
    WX_GL_MINOR_VERSION,   // minor OpenGL version of the core profile
    wx_GL_COMPAT_PROFILE,  // use compatible profile (use all versions features)
    WX_GL_FORWARD_COMPAT,  // forward compatible context. OpenGL >= 3.0
    WX_GL_ES2,             // ES or ES2 context.
    WX_GL_DEBUG,           // create a debug context
    WX_GL_ROBUST_ACCESS,   // robustness.
    WX_GL_NO_RESET_NOTIFY, // never deliver notification of reset events
    WX_GL_LOSE_ON_RESET,   // if graphics reset, all context state is lost
    WX_GL_RESET_ISOLATION, // protect other apps or share contexts from reset side-effects
    WX_GL_RELEASE_FLUSH,   // on context release, flush pending commands
    WX_GL_RELEASE_NONE     // on context release, pending commands are not flushed
};

#define         wxGLCanvasName      "GLCanvas"

// ----------------------------------------------------------------------------
// wxGLAttribsBase: OpenGL rendering attributes
// ----------------------------------------------------------------------------

class  wxGLAttribsBase
{
public:
    wxGLAttribsBase() { Reset(); }

    // Setters
    void AddAttribute(int attribute) { m_GLValues.push_back(attribute); }
    // Search for searchVal and combine the next value with combineVal
    void AddAttribBits(int searchVal, int combineVal);
    // ARB functions necessity
    void SetNeedsARB(bool needsARB = true) { m_needsARB = needsARB; }

    // Delete contents
    void Reset()
    {
        m_GLValues.clear();
        m_needsARB = false;
    }

    // Accessors
    const int* GetGLAttrs() const
    {
        return (m_GLValues.empty() || !m_GLValues[0]) ? NULL : &*m_GLValues.begin();
    }

    int GetSize() const { return (int)(m_GLValues.size()); }

    // ARB function (e.g. wglCreateContextAttribsARB) is needed
    bool NeedsARB() const { return m_needsARB; }

private:
    QVector<int> m_GLValues;
    bool m_needsARB;
};

// ----------------------------------------------------------------------------
// wxGLContextAttrs: OpenGL rendering context attributes
// ----------------------------------------------------------------------------

class  wxGLContextAttrs : public wxGLAttribsBase
{
public:
    // Setters, allowing chained calls
    wxGLContextAttrs& CoreProfile();
    wxGLContextAttrs& MajorVersion(int val);
    wxGLContextAttrs& MinorVersion(int val);
    wxGLContextAttrs& OGLVersion(int vmayor, int vminor)
        { return MajorVersion(vmayor).MinorVersion(vminor); }
    wxGLContextAttrs& CompatibilityProfile();
    wxGLContextAttrs& ForwardCompatible();
    wxGLContextAttrs& ES2();
    wxGLContextAttrs& DebugCtx();
    wxGLContextAttrs& Robust();
    wxGLContextAttrs& NoResetNotify();
    wxGLContextAttrs& LoseOnReset();
    wxGLContextAttrs& ResetIsolation();
    wxGLContextAttrs& ReleaseFlush(int val = 1); //'int' allows future values
    wxGLContextAttrs& PlatformDefaults();
    void EndList(); // No more values can be chained

    // Currently only used for X11 context creation
    bool x11Direct; // X11 direct render
    bool renderTypeRGBA;
};

// ----------------------------------------------------------------------------
// wxGLAttributes: canvas configuration
// ----------------------------------------------------------------------------

class  wxGLAttributes : public wxGLAttribsBase
{
public:
    // Setters, allowing chained calls
    wxGLAttributes& RGBA();
    wxGLAttributes& BufferSize(int val);
    wxGLAttributes& Level(int val);
    wxGLAttributes& DoubleBuffer();
    wxGLAttributes& Stereo();
    wxGLAttributes& AuxBuffers(int val);
    wxGLAttributes& MinRGBA(int mRed, int mGreen, int mBlue, int mAlpha);
    wxGLAttributes& Depth(int val);
    wxGLAttributes& Stencil(int val);
    wxGLAttributes& MinAcumRGBA(int mRed, int mGreen, int mBlue, int mAlpha);
    wxGLAttributes& PlatformDefaults();
    wxGLAttributes& Defaults();
    wxGLAttributes& SampleBuffers(int val);
    wxGLAttributes& Samplers(int val);
    wxGLAttributes& FrameBuffersRGB();
    void EndList(); // No more values can be chained

    // This function is undocumented and can not be chained on purpose!
    // To keep backwards compatibility with versions before wx3.1 we add here
    // the default values used in those versions for the case of NULL list.
    void AddDefaultsForWXBefore31();
};

// ----------------------------------------------------------------------------
// wxGLContextBase: OpenGL rendering context
// ----------------------------------------------------------------------------

class  wxGLContextBase
{
public:

//  The derived class should provide a ctor with this signature:
//
//  wxGLContext(wxGLCanvas *win,
//              const wxGLContext *other = NULL,
//              const wxGLContextAttrs *ctxAttrs = NULL);

    // set this context as the current one
    virtual bool SetCurrent(const wxGLCanvas& win) const = 0;

    bool IsOK() { return m_isOk; }

protected:
    bool m_isOk;
};

// ----------------------------------------------------------------------------
// wxGLCanvasBase: window which can be used for OpenGL rendering
// ----------------------------------------------------------------------------

class  wxGLCanvasBase
{
public:
    // default ctor doesn't initialize the window, use Create() later
    wxGLCanvasBase();

    virtual ~wxGLCanvasBase();


    /*
       The derived class should provide a ctor with this signature:

    wxGLCanvas(wxWindow *parent,
               const wxGLAttributes& dispAttrs,
               wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const QString& name = wxGLCanvasName,
               const wxPalette& palette = wxNullPalette);
     */

    // operations
    // ----------

    // set the given context associated with this window as the current one
    bool SetCurrent(const wxGLContext& context) const;

    // flush the back buffer (if we have it)
    virtual bool SwapBuffers() = 0;


    // accessors
    // ---------

    // check if the given attributes are supported without creating a canvas
    static bool IsDisplaySupported(const wxGLAttributes& dispAttrs);
    static bool IsDisplaySupported(const int *attribList);

#if wxUSE_PALETTE
    const wxPalette *GetPalette() const { return &m_palette; }
#endif // wxUSE_PALETTE

    // miscellaneous helper functions
    // ------------------------------

    // call glcolor() for the colour with the given name, return false if
    // colour not found
    bool SetColour(const QString& colour);

    // return true if the extension with given name is supported
    //
    // notice that while this function is implemented for all of GLX, WGL and
    // AGL the extensions names are usually not the same for different
    // platforms and so the code using it still usually uses conditional
    // compilation
    static bool IsExtensionSupported(const char *extension);

    // Get the wxGLContextAttrs object filled with the context-related values
    // of the list of attributes passed at ctor when no wxGLAttributes is used
    // as a parameter
    wxGLContextAttrs& GetGLCTXAttrs() { return m_GLCTXAttrs; }

    // deprecated methods using the implicit wxGLContext
#if WXWIN_COMPATIBILITY_2_8
    wxDEPRECATED( wxGLContext* GetContext() const );

    wxDEPRECATED( void SetCurrent() );

    wxDEPRECATED( void OnSize(wxSizeEvent& event) );
#endif // WXWIN_COMPATIBILITY_2_8

#ifdef __WXUNIVERSAL__
    // resolve the conflict with wxWindowUniv::SetCurrent()
    virtual bool SetCurrent(bool doit) { return wxWindow::SetCurrent(doit); }
#endif

protected:
    // override this to implement SetColour() in GL_INDEX_MODE
    // (currently only implemented in wxX11 and wxMotif ports)
    virtual int GetColourIndex(const QColor& col) { return -1; }

    // check if the given extension name is present in the space-separated list
    // of extensions supported by the current implementation such as returned
    // by glXQueryExtensionsString() or glGetString(GL_EXTENSIONS)
    static bool IsExtensionInList(const char *list, const char *extension);

    // For the case of "int* attribList" at ctor is != 0
    wxGLContextAttrs m_GLCTXAttrs;

    // Extract pixel format and context attributes.
    // Return false if an unknown attribute is found.
    static bool ParseAttribList(const int* attribList,
                                wxGLAttributes& dispAttrs,
                                wxGLContextAttrs* ctxAttrs = NULL);

#if wxUSE_PALETTE
    // create default palette if we're not using RGBA mode
    // (not supported in most ports)
    virtual wxPalette CreateDefaultPalette() { return wxNullPalette; }

    wxPalette m_palette;
#endif // wxUSE_PALETTE

#if WXWIN_COMPATIBILITY_2_8
    wxGLContext *m_glContext;
#endif // WXWIN_COMPATIBILITY_2_8
};



// ----------------------------------------------------------------------------
// wxGLAPI: an API wrapper that allows the use of 'old' APIs even on OpenGL
// platforms that don't support it natively anymore, if the APIs are available
// it's a mere redirect
// ----------------------------------------------------------------------------

#ifndef wxUSE_OPENGL_EMULATION
    #define wxUSE_OPENGL_EMULATION 0
#endif

class  wxGLAPI
{
public:
    wxGLAPI();
    ~wxGLAPI();

    static void glFrustum(GLfloat left, GLfloat right, GLfloat bottom,
                            GLfloat top, GLfloat zNear, GLfloat zFar);
    static void glBegin(GLenum mode);
    static void glTexCoord2f(GLfloat s, GLfloat t);
    static void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
    static void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
    static void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    static void glColor3f(GLfloat r, GLfloat g, GLfloat b);
    static void glEnd();
};



class wxGLContext : public wxGLContextBase
{
public:
    wxGLContext(wxGLCanvas *win,
                const wxGLContext *other = NULL,
                const wxGLContextAttrs *ctxAttrs = NULL);
///    virtual ~wxGLContext();

    virtual bool SetCurrent(const wxGLCanvas& win) const;

private:
    QGLContext *m_glContext;
};

// ----------------------------------------------------------------------------
// wxGLCanvas: OpenGL output window
// ----------------------------------------------------------------------------

class  wxGLCanvas : public wxGLCanvasBase
{
public:
    explicit // avoid implicitly converting a wxWindow* to wxGLCanvas
    wxGLCanvas(wxWindow *parent,
               const wxGLAttributes& dispAttrs,
               wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const QString& name = wxGLCanvasName,
               const wxPalette& palette = wxNullPalette);

    explicit
    wxGLCanvas(wxWindow *parent,
               wxWindowID id = wxID_ANY,
               const int *attribList = NULL,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const QString& name = wxGLCanvasName,
               const wxPalette& palette = wxNullPalette);

    bool Create(wxWindow *parent,
                const wxGLAttributes& dispAttrs,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const QString& name = wxGLCanvasName,
                const wxPalette& palette = wxNullPalette);

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const QString& name = wxGLCanvasName,
                const int *attribList = NULL,
                const wxPalette& palette = wxNullPalette);

    virtual bool SwapBuffers();

    static bool ConvertWXAttrsToQtGL(const int *wxattrs, QGLFormat &format);

private:

};

#endif // _WX_GLCANVAS_H_
