#ifndef GUI_OPENGL___GL_RENDER_MANAGER__HPP
#define GUI_OPENGL___GL_RENDER_MANAGER__HPP

/*  $Id: glrender.hpp 43136 2019-05-20 18:37:04Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/opengl/render_common.hpp>

#include <gui/opengl/globject.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/utils/matrix4.hpp>
#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/utils/vect4.hpp>


/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


//////////////////////////////////////////////////////////////////////////////
/// CGlRender
///
/// Purpose:
/// 
/// The purpose of CGlRender and related classes (CGlRenderNode, CGlVboNode, 
/// CGlVboGeom, CGlState etc) is to provide a flexible rendering 
/// interface supporting both PDF output and different versions of OpenGL:
///   1.  Support standard OpenGL rendering with minimal changes to existing code
///   2.  Support rendering to PDF files without adding PDF-specific code
///   3.  Run on platforms that only support OpenGL 1.1
///   4.  Run on OpenGL platforms which may not backward compatible to OpenGL 1.1 
///       (e.g. OpenGL ES or OpenGL 3.2+) without changing user code (work would
///        be needed though to add a new target to the API)
///
/// Usage:
/// CGlRender mimics to the extent possible the OpenGL 1.1 API.  
/// Functions are provided for both state-setting, e.g. LineWidth(float)
/// and rendering e.g. Begin(), Vertex3f(), End().  Not every
/// OpenGl function is supported, particularly those which are 
/// infrequently used or have alternatives.  For example, the short and 
/// integer versions of the glVertex*() commands, e.g. glVertex2i(int x, int y)
/// and glVertex2s(short x, short y), are not supported since you can use 
/// the floating point or double versions instead.
///
/// There is always a current singleton instance of CGlRender available 
/// for rendering.  The singleton, which may be an instance of CGlRenderPdf 
/// or the current OpenGL version, is managed by CGlResMgr, which is itself 
/// a singleton. The current instance can always be accessed by the function:
///
/// CGlRender& GetGl();  // declared in this file, CGlRender.hpp
///
/// To make it easy to convert code and to make code that looks like 
/// OpenGL 1.1, we usually use it in this manner:
///
/// CGlRender& gl = GetGl();
///
/// gl.Enable(GL_BLEND);
/// gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
/// 
/// gl.Begin(GL_TRIANGLES);
///   gl.Color3f(1.0f, 0.0f, 0.0f);
///   gl.Vertex3f(1.0f, 1.0f, 0.0f);
///    …
/// gl.End();
///
/// Since all standard OpenGL functions have the name format gl[A-Z]*, e.g.
/// glVertex3f(), all member functions of CGlRender that implement these have 
/// the same name minus the “gl”.  This way, to convert to CGlRender,
/// get the current CGlRender object with GetGl(), and then add a “.” after 
/// “gl” in all the OpenGL calls in the function.
///
/// Unlike OpenGL, the CGlRender interface also supports text output, using 
/// the CGlTextureFont class.  If you want to support PDF output, you should
/// use this interface rather than using CGlTextureFont directly.  The format 
/// for writing text has a begin/end style similar to OpenGL:
///
///    gl.BeginText(&m_Font, m_TextColor);
///        gl.WriteText(x1, y1, “I’m Text!”, rotate_degrees);
///    gl.EndText();
///
/// When possible, it’s more efficient to put multiple WriteText() commands 
/// between BeginText() and EndText() since it reduces state-setting.
///
/// The final important point in using the API is setup – if you are rendering
/// to PDF you have to set a CGlRenderPdf object as the current rendering 
/// singleton, and if rendering to OpenGL a CGlRender11/20/Debug object
/// has to be current.  Making sure the OpenGL CGlRender object is current
/// can be done like this:
///
/// CRef<CGlRender>  mgr = CGlResMgr::Instance().GetRenderer(CGlResMgr::Instance().GetApiLevel());
/// if (mgr.IsNull()) {
///    LOG_POST(Error << "CGlRender object not available.");
///    return;        
/// }
/// CGlResMgr::Instance().SetCurrentRenderer(mgr);
///
/// For PDF rendering, no additional work needs to be done after setting a 
/// CGlRenderPdf object as the current rendering singleton.  There are 
/// however a few PDF only rendering states defined in IGlState (and 
/// inherited by CGlRender).  These will not affect OpenGL output, 
/// but can make PDFs look nicer, e.g. rounding corners where edges join.
/// And there are significant OpenGL features not supported by PDF, 
/// including texture mapping and any 3D operations that require a depth 
/// buffer.
///
/// Design:
///
///   IGlState
///     ^    ^
///     |    |__________
///     |               |
///     |             CGlState----(blend mode, current color, polygon mode etc. etc)
///     |              
///     |              
///   CGlRender -------m_RenderNode  (CGlRenderNode does rendering, may be more than 1)
///   ^ ^ ^ ^   -------m_State (CGlState used by render nodes - IGlState calls forwarded here)
///   | | | |
///   | | | |__________
///   | | |            |
///   | | |          CGlRenderPdf (uses CPdf to write buffers to pdf file)
///   | | |_____________
///   | |               |
///   | |            CGlRenderDebug (uses only OpenGL 1.1 immediate mode for debugging)
///   | |_______________
///   |                 |
///   |              CGlRender11 (uses OpenGL 1.1 with vertex arrays)
///   |
///  CGlRender20 (OpenGL with vertex buffers)
///
/// Known Issues:
///
/// The CGlRender objects, like the CFtglFontManager manager, retain memory in 
/// buffers (or textures) owned by OpenGL.  Because these are singletons, 
/// they will hang around until the program ends.  So if at any point during 
/// the program you destroy the OpenGL context, you should also call the Clear()
/// function on these objects to prevent them from hanging on to stale OpenGL
/// memory.
///
/// The other thing to watch out for with the new library is the glPushAttrib()
/// and glPopAttrib() may no longer work in a consistent manner. Because
/// state is saved in the m_State object, options may be reset after glPopAttrib().
/// The best approach for using this library is to remove calls to glPushAttrib
/// and glPopAttrib.  These functions are in any case deprecated in newer versions
/// of OpenGL (and they are not replaced with anything similar).
///

class NCBI_GUIOPENGL_EXPORT CGlRender : public CObject, public CRenderCommon
{
public:
    CGlRender();
    virtual ~CGlRender() {}

    /// Clear/delete any graphics/OpenGL memory.  Call this if you delete
    /// the current OpenGL context
    virtual void Clear();

    /// For shaders. Only works with OpenGL 2.0+
    virtual void UseProgram(GLuint program) {}
    
    /// @name Viewports, matrices and transformations
    /// @{
    virtual void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
    
    virtual void MatrixMode(GLenum mode);
    
    virtual void PushMatrix();
    virtual void PopMatrix();
    virtual void LoadIdentity();
    virtual void LoadMatrixf(const GLfloat *m);
    virtual void LoadMatrixd(const GLdouble* m);
    virtual void MultMatrixf(const GLfloat* m);
    virtual void MultMatrixd(const GLdouble* m);
    virtual CMatrix4<float> GetModelViewMatrix() const { return m_ModelView; }
    virtual CMatrix4<float> GetProjectionMatrix() const;
    virtual void GetModelViewMatrix(GLdouble* m) const;
    virtual void GetProjectionMatrix(GLdouble* m) const;

    virtual void Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    virtual void Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    virtual void Scaled(GLdouble x, GLdouble y, GLdouble z);
    virtual void Scalef(GLfloat x, GLfloat y, GLfloat z);
    virtual void Translated(GLdouble x, GLdouble y, GLdouble z);
    virtual void Translatef(GLfloat x, GLfloat y, GLfloat z);

    virtual void Ortho(GLdouble left, GLdouble right,
                       GLdouble bottom, GLdouble top,
                       GLdouble nearVal, GLdouble farVal);
    virtual void Perspective(GLdouble fovy, GLdouble aspect, 
                       GLdouble zNear, GLdouble zFar);
    /// @}
   
    /// @name IGlState interface implementation
    /// @{

    /// 
    /// These commands just forward their parameters to the render managers
    /// CGlState object m_State.  State can also be updated (replaced) via
    /// Begin(GLenum mode, const CGlState& state);
    ///
    virtual void PushAttrib(GLbitfield mask);
    virtual void PopAttrib();

    virtual void Enable(GLenum glstate);
    virtual void Disable(GLenum glstate);
    virtual void LineWidth(GLfloat w);
    virtual void PointSize(GLfloat s);
    virtual void ShadeModel(GLenum mode);
    virtual void Scissor(GLint x, GLint y,
                         GLsizei width, GLsizei height);
    /// Color{3,4}{f,d}{v} commands are defined above....
    virtual void ColorMask(GLboolean red, 
                   GLboolean green, 
                   GLboolean blue, 
                   GLboolean alpha);
    virtual void PolygonMode(GLenum face, GLenum mode);
    virtual void LineStipple(GLint factor, GLushort pattern);
    virtual void PolygonStipple(GLubyte* mask);
    virtual void BlendFunc(GLenum sfactor, GLenum dfactor);
    virtual void TexEnvi(GLenum target, GLenum pname, GLint param);
    virtual void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
                                   GLenum srcAlpha, GLenum dstAlpha);
    virtual void BlendColor(const CRgbaColor& c);
    virtual void ScaleInvarient(bool b, CVect2<TModelUnit> scale = 
        CVect2<TModelUnit>(TModelUnit(1), TModelUnit(1)));
    virtual void ScaleFactor(const CVect2<TModelUnit>& scale);
    virtual void LineJoinStyle(IGlState::ELineJoinStyle s);
    virtual void LineCapStyle(IGlState::ELineCapStyle c);
    virtual void PdfShadeStyle(IGlState::EPdfShadeStyle s);
    /// @}

protected:
    /// state pushed/poped by PushAttrib/PopAttrib and the corresponding bitmask
    vector<pair<GLbitfield,CGlState> > m_PushedState;

private:
    /// current modelview matrix and matrix mode
    CMatrix4<float>  m_ModelView;
};


///////////////////////////////////////////////////////////////////////////////
/// CGlRender20
///
/// This is the renderer that should be (automatically) created if the OpenGL
/// level is 2.0 or above.  It uses vertex buffers rather than vertex arrays
/// for rendering.
class NCBI_GUIOPENGL_EXPORT CGlRender20 : public CGlRender
{
public:
    CGlRender20() {}
    virtual ~CGlRender20() {}

    /// Set OpenGL shader program. Other subclasses will not implement this.
    virtual void UseProgram(GLuint program);

    virtual ERenderTarget GetApi() { return eOpenGL20; }

    virtual void Hint(GLenum target, GLenum mode);
};

///////////////////////////////////////////////////////////////////////////////
/// CGlRender11
///
/// This is the renderer that should be (automatically) created if the OpenGL
/// level below 2.0.  It uses vertex arrays for rendering. Note that this
/// does not use the immediate mode commands glBegin(), glEnd().  By using
/// vertex arrays, the code for CGlRender11 and CGlRender20 can be almost
/// identical.
class NCBI_GUIOPENGL_EXPORT CGlRender11 : public CGlRender
{
public:
    CGlRender11() {}
    virtual ~CGlRender11() {}

    virtual ERenderTarget GetApi() { return eOpenGL11; }

    virtual void Hint(GLenum target, GLenum mode);
};

///////////////////////////////////////////////////////////////////////////////
/// CGlRenderDebug
///
/// Create this renderer and install it if you think the other ones are not
/// giving the correct result.  This renderer simply passes all commands 
/// directly through to their corresponding OpenGL command of the same name,
/// when applicable.  (so obviously not for the text functions)
class NCBI_GUIOPENGL_EXPORT CGlRenderDebug : public CGlRender
{
public:
    CGlRenderDebug() {}
    virtual ~CGlRenderDebug() {}

    virtual void Render(CGlPane& pane, CGlModel2D* model) {}

    /// Start rendering.  "mode" may be one of:
    /// GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES, GL_TRIANGLE_FAN, 
    /// GL_TRIANGLE_STRIP or GL_QUADS.
    /// Start rendering - use state in existing 'state' object.
    virtual void Begin(GLenum mode);

    /// Finish rendering (create buffer and send to renderer)
    virtual void End();

    ///
    /// Polygon rendering interface - render similar to matching OpenGL calls
    /// 
    /// Explicit support not currently available for:
    ///     GLbyte, GlShort and GLint calls, e.g. glVertex2i(int x, int y);
    ///     Vertices with homgeneous coords, e.g.
    ///         glVertex4f(float x, float y, float z, float w);
    ///     3&4-dimensional texture coords, e.g. glTexCoord4f
    ///     Generic attributes and mulitple texture coords per vertex
    ///

    virtual void Vertex3f(GLfloat x, GLfloat y, GLfloat z);
    virtual void Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
    virtual void RectC(const TModelRect& rc);
    virtual void Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    virtual void TexCoord2f(GLfloat s, GLfloat t);
 
    ///
    /// Text interface
    /// 
    /// Text is drawn is pixel coordinates.  Its position is 
    /// transformed by the current ModelView matrix.  The color and alpha for
    /// text are set in BeginText() and can be changed with SetTextColor().
    /// Other OpenGL state should not affect text (since BeginText sets 
    /// appropriate state)
    ///

    /// Set OpenGL state needed for writing text (call before DrawText)
    virtual void BeginText(const CGlTextureFont* font, 
                           const CRgbaColor& color);
    /// Same as above, but get color from m_RenderNode.m_State
    virtual void BeginText(const CGlTextureFont* font);
    virtual void EndText();

    /// Update text color and alpha
    void SetTextColor(const CRgbaColor& color) { m_TextColor = color; }
    CRgbaColor GetTextColor() const { return m_TextColor; }

    /// Write text at specified model coords
    virtual void WriteText(TModelUnit x, TModelUnit y, 
                           const char* text,
                           TModelUnit rotate_degrees = 0.0);
    /// Write text at specified model coords inside box defined by
    /// (x,y,width,height) with specified alignment, truncation and rotation
    virtual void WriteText(TModelUnit x, TModelUnit y, 
                           TModelUnit width, TModelUnit height,
                           const char* text,
                           CGlTextureFont::TAlign align = CGlTextureFont::eAlign_Center,
                           CGlTextureFont::ETruncate trunc = CGlTextureFont::eTruncate_Ellipsis,
                           TModelUnit rotate_degrees = 0.0f);   


    /// Overridden in subclasses to return the supported rendering target
    virtual ERenderTarget GetApi() { return eRenderDebug; }

    ///
    /// Rendering state interface
    /// 
    /// These commands just forward their parameters to the render managers
    /// CGlState object.
    ///
    
    virtual void PushAttrib(GLbitfield mask);
    virtual void PopAttrib();

    virtual void Enable(GLenum glstate);
    virtual void Disable(GLenum glstate);
    virtual void Hint(GLenum target, GLenum mode);
    virtual void LineWidth(GLfloat w);
    virtual void PointSize(GLfloat s);
    virtual void ShadeModel(GLenum mode);
    virtual void Scissor(GLint x, GLint y,
                         GLsizei width, GLsizei height);
    /// Color{3,4}{f,d}{v} commands are defined above....
    virtual void ColorMask(GLboolean red, 
                   GLboolean green, 
                   GLboolean blue, 
                   GLboolean alpha);
    virtual void PolygonMode(GLenum face, GLenum mode);
    virtual void LineStipple(GLint factor, GLushort pattern);
    virtual void PolygonStipple(GLubyte* mask);
    virtual void BlendFunc(GLenum sfactor, GLenum dfactor);
    virtual void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
                                   GLenum srcAlpha, GLenum dstAlpha);
    virtual void BlendColor(const CRgbaColor& c);
    virtual void ScaleInvarient(bool b, CVect2<TModelUnit> scale = 
        CVect2<TModelUnit>(TModelUnit(1), TModelUnit(1)));
    virtual void ScaleFactor(const CVect2<TModelUnit>& scale);
    virtual void LineJoinStyle(IGlState::ELineJoinStyle s);
    virtual void LineCapStyle(IGlState::ELineCapStyle c);
    virtual void PdfShadeStyle(IGlState::EPdfShadeStyle s);

protected:
    virtual void x_RenderBuffer(CGlVboNode* node) {}

    //CGlBitmapFont m_BitmapFont;
};



END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_RENDER_MANAGER__HPP
