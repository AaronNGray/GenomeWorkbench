/*  $Id: glcanvas.cpp 43252 2019-06-05 19:21:13Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/glcanvas.hpp>

#include <gui/opengl.h>
#include <gui/opengl/glutils.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <wx/dcclient.h>


BEGIN_NCBI_SCOPE

class CGLContextCache
{
public:
    static CGLContextCache& GetInstance();
    void AddRef();
    void Release();
    wxGLContext* GetContext(wxGLCanvas* win);

#ifdef GLEW_MX
    GLEWContext* GetGlewContext() { return &m_GLEWContext; }
#endif

private:
    CGLContextCache() : m_GLContext(0), m_RefCnt(0) {}
    ~CGLContextCache()
    { 
        // We could try to delete the context here but, at least on windows,
        // by the time this dtor is called, the memory is already cleand up
        // so an exception is thrown when we try to delete the (already deleted)
        // wxGLContext
    }

private:
    wxGLContext* m_GLContext;
    int m_RefCnt;

#ifdef GLEW_MX
    GLEWContext  m_GLEWContext;
#endif
};


CGLContextCache& CGLContextCache::GetInstance()
{
    // Declare the static instance (singleton)
    static CGLContextCache instance;

#ifdef GLEW_MX
    // push the address of the glew context to the static instance of CGLGlewContext
    // since that is in the base opengl library which needs the (glew) context, but
    // does not have any dependencies on wxWidgets.
    if (CGLGlewContext::GetInstance().GetGlewContext() == NULL)
        CGLGlewContext::GetInstance().SetGlewContext(&instance.m_GLEWContext);
#endif

    return instance;
}

void CGLContextCache::AddRef()
{
    ++m_RefCnt;
}

void CGLContextCache::Release()
{
    --m_RefCnt;
    if (m_RefCnt == 0) {
        // Keep context - we share one context for all windows and will
        // reuse it when the next window opens.  Also, some classes (e.g. 
        // CGlTextureFont) may save gl objects like texture maps (fonts) 
        // between window instances so context must stay alive.
    }
}

wxGLContext* CGLContextCache::GetContext(wxGLCanvas* win)
{
    if (m_GLContext == 0) {
        m_GLContext = new wxGLContext(win);

        m_GLContext->SetCurrent(*win);
    
#ifdef GLEW_MX
        GLenum err = glewContextInit(&m_GLEWContext);
        if (GLEW_OK != err)
        {
            // Problem: Error creating glew context
            _TRACE("Error creating glew context");
        } 
        else {
            err = glewInit();      
            if (GLEW_OK != err)
            {
                // Problem: glewInit failed, something is seriously wrong.
                //   No extenstions will be available.
                _TRACE("Error loading opengl extensions");
            }
        }
#else
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            // Problem: glewInit failed, something is seriously wrong.
            //   No extenstions will be available.
            _TRACE("Error loading opengl extensions");
        }
#endif

    }
    return m_GLContext;
}


BEGIN_EVENT_TABLE(CGLCanvas, wxGLCanvas)
    EVT_PAINT(CGLCanvas::OnPaint)
END_EVENT_TABLE()

/// Not having all programs share the same window attributes has caused
/// crashing on Linux, so each window will use these attributes (this should
/// be the superset of attributes needed by program windows).

///
/// X2Go X-Server doesn't allow to specify number of bitplanes of alpha/stencil
/// To make gbench work with X2Go the following code
///

const int* CGLCanvas::x_GetAttribList()
{
    static const int* attrListPtr = nullptr;
    static bool _initialized = false;
    static const int _attrList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_MIN_ALPHA, 8, WX_GL_STENCIL_SIZE, 8, 0 };

    if (!_initialized) {
        if (wxGLCanvas::IsDisplaySupported(&_attrList[0]))
            attrListPtr = &_attrList[0];
        _initialized = true;
    }
    return attrListPtr;
}

CGLCanvas::CGLCanvas(wxWindow* parent, wxWindowID id,
                     const wxPoint& pos, const wxSize& size, long style) :

    wxGLCanvas(parent, id, x_GetAttribList(), pos, size, style),
    m_DoNotUpdate(false)
{
#ifdef GLEW_MX
    CGLContextCache::GetInstance().AddRef();
#endif

    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    // Check that we have an opengl level of at least 1.4 and display
    // a warning message if we don't
/*
    static bool opengl_level_checked = false;
    if (!opengl_level_checked) {
        opengl_level_checked = true;
        x_CheckGlVersion();
    }
*/

    m_ClearColor[0] = 1;
    m_ClearColor[1] = 1;
    m_ClearColor[2] = 1;
    m_ClearColor[3] = 0;
}

void CGLCanvas::SetClearColor(float r, float g, float b, float a)
{
    m_ClearColor[0] = r;
    m_ClearColor[1] = g;
    m_ClearColor[2] = b;
    m_ClearColor[3] = a;
}

CGLCanvas::~CGLCanvas()
{
    CGLContextCache::GetInstance().Release();
}

void CGLCanvas::x_CheckGlVersion()
{
    x_SetupGLContext();
    string version = (const char*)glGetString(GL_VERSION);
    vector<string> arr;

    if (version != "")
        NStr::Split(version, ". \t", arr);

    bool version_passed = false;

    // Check major version
    int major_version = 0;
    if (arr.size() >= 1) {       
        try {
            major_version = NStr::StringToInt(arr[0]);
            if (major_version > 1) {
                version_passed = true;
            }
        }
        catch (CException& ) {
            //didn't parse as an int...
        }
    }

    // If major version is only 1, check minor version:
    if (arr.size() > 1 && major_version == 1) {
        try {
            int minor_version = NStr::StringToInt(arr[1]);
            if (minor_version > 4) {
                version_passed = true;
            }
        }
        catch (CException&) {
            //minor version didn't parse as an int
        }
    }

    string user_warning;
    if (!version_passed) {
        user_warning = "Warning - Your computer's graphics do not meet the\n";
        user_warning += "minimum level required by Genome Workbench. Visual or \n";
        user_warning += "stability issues may occur.\n\n";
        user_warning += "OpenGL version 1.5 or better is needed. Your graphics are:\n\n";
        user_warning += version;
        user_warning += "\n";
        user_warning += (const char*)glGetString(GL_RENDERER);

        NcbiWarningBox(user_warning, "Graphics Warning");
    }
}

void CGLCanvas::x_SetupGLContext()
{
    wxGLContext* context = CGLContextCache::GetInstance().GetContext(this);
    context->SetCurrent(*this);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void CGLCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    //LOG_POST("CGLCanvas::OnPaint()");

    // This is a dummy, to avoid an endless succession of paint messages.
    // OnPaint handlers must always create a wxPaintDC.
    wxPaintDC dc(this);

    // The hack is used for tooltip window
    // SwapBuffers ovewrites child windows on some systems
    // Ubuntu with Intel videocard (netbook).
    if (m_DoNotUpdate) {
        m_DoNotUpdate = false;
        return;
    }

    x_SetupGLContext();

    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    x_Render();

    // For some implementations of graphics card drivers,
    // the update of the stored image underneath the mouse
    // cursor is not sychronized with the udpate of OpenGL
    // window.  Here we force to update the stored image
    // after the frame butter gets updated.
    // Currently, This works on Windows only.
#if defined __WXMSW__
    ShowCursor(false);
#endif

    SwapBuffers();

#if defined __WXMSW__
    ShowCursor(true);
#endif
}

void CGLCanvas::x_Render()
{
    wxColour clr = GetBackgroundColour();
    glClearColor(clr.Red()/255.f, clr.Green()/255.f, clr.Blue()/255.f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

END_NCBI_SCOPE
