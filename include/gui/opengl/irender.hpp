#ifndef GUI_OPENGL___IRENDER__HPP
#define GUI_OPENGL___IRENDER__HPP

/*  $Id: irender.hpp 45024 2020-05-09 02:03:16Z evgeniev $
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

#include <gui/opengl/glstate.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/gltypes.hpp>


/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CGlModel2D;
class CGlPane;

class IRender
{
public:
    virtual ~IRender() {}

    /// Clear/delete any graphics/OpenGL memory.  Call this if you delete
    /// the current OpenGL context
    virtual void Clear() = 0;

    virtual ERenderTarget GetApi() = 0;

    virtual void SetIsGreyscale(bool b) = 0;

    virtual bool IsSimplified() const = 0;

    virtual bool IsPrinterFriendly() const = 0;

    /// @name former (no more deriving from an interface) IGlState interface
    /// @{
    /// glEnable()
    virtual void Enable(GLenum glstate) = 0;
    /// glDisable()
    virtual void Disable(GLenum glstate) = 0;

    virtual void Hint(GLenum target, GLenum mode) = 0;

    /// Set line width for drawing: glLineWidth()
    virtual void LineWidth(GLfloat w) = 0;

    /// Set point size for drawing: glPointSize()
    virtual void PointSize(GLfloat size) = 0;

    /// Set shade model for default lighting: glShadeModel(GL_FLAT or GL_SMOOTH)
    virtual void ShadeModel(GLenum mode) = 0;

    /// Set clipping window: glScissor(x,y,width,height)
    virtual void Scissor(GLint x, GLint y,
        GLsizei width, GLsizei height) = 0;

    /// Set current color (glColor{3,4}{f,d}{v,})    
    virtual void ColorC(const CRgbaColor& c) = 0;

    void Color3f(GLfloat r, GLfloat g, GLfloat b) { ColorC(CRgbaColor(r, g, b, 1.0f)); }
    void Color3fv(const GLfloat* v) { ColorC(CRgbaColor(v[0], v[1], v[2], 1.0f)); }
    void Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { ColorC(CRgbaColor(r, g, b, a)); }
    void Color4fv(const GLfloat* v) { ColorC(CRgbaColor(v[0], v[1], v[2], v[3])); }

    void Color3d(GLdouble r, GLdouble g, GLdouble b) { ColorC(CRgbaColor(float(r), float(g), float(b), 1.0f)); }
    void Color3dv(const GLdouble* v) { ColorC(CRgbaColor(float(v[0]), float(v[1]), float(v[2]), 1.0f)); }
    void Color4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a) { ColorC(CRgbaColor(float(r), float(g), float(b), float(a))); }
    void Color4dv(const GLdouble* v) { ColorC(CRgbaColor(float(v[0]), float(v[1]), float(v[2]), float(v[3]))); }

    /// Set the color mask (glColorMask)
    virtual void ColorMask(GLboolean red,
        GLboolean green,
        GLboolean blue,
        GLboolean alpha) = 0;

    /// Set the polygon rasterization mode.  For the first parameter, GL_FRONT
    /// and GL_BACK are deprecated so better to use GL_FRONT_AND_BACK
    virtual void PolygonMode(GLenum face, GLenum mode) = 0;

    /// Set line stipple pattern: glLineStipple().  Deprecated in gl 3.2+
    virtual void LineStipple(GLint factor, GLushort pattern) = 0;

    /// Set polygon stipple pattern: glPolygonStipple().  Deprecated in gl 3.2+
    virtual void PolygonStipple(GLubyte* mask) = 0;


    ///
    /// Options to be used when GL_BLEND is enabled. 
    /// The options used at render time will be those most recently 
    /// given by SetBlendFunc() OR SetBlendFuncSeparate() prior
    /// to calling MakeCurrent().
    ///

    /// Options for glBlendFunc. Enable via Enable(GL_BLEND);
    virtual void BlendFunc(GLenum sfactor, GLenum dfactor) = 0;

    virtual void TexEnvi(GLenum target, GLenum pname, GLint param) = 0;

    ///
    /// PDF-specific rendering state.  Will not have any effect on OpenGL output
    /// Note that in PDF files, some or all of these options may be settable 
    /// pre-element (attributes), not just per-object (uniform)
    ///

    /// Set path joing style for lines (pdf-only)
    virtual void LineJoinStyle(IGlState::ELineJoinStyle s) = 0;

    /// Set line cap ending style (pdf only)
    virtual void LineCapStyle(IGlState::ELineCapStyle c) = 0;

    /// Set (override defualt) shading style for polygons
    virtual void PdfShadeStyle(IGlState::EPdfShadeStyle s) = 0;
    /// @}

    virtual void PushAttrib(GLbitfield mask) = 0;
    virtual void PopAttrib() = 0;

    virtual CRef<CGlState> GetState() = 0;
    virtual void ResetState() = 0;


    /// Start rendering.  "mode" may be one of:
    /// GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES, GL_TRIANGLE_FAN, 
    /// GL_TRIANGLE_STRIP or GL_QUADS.
    /// Start rendering - use state in existing 'state' object.
    virtual void Begin(GLenum mode) = 0;

    /// Finish rendering (create buffer and send to renderer)
    virtual void End() = 0;

    /// For shaders. Only works with OpenGL 2.0+
    virtual void UseProgram(GLuint program) = 0;

    /// @name Polygon rendering interface
    /// @{

    /// Explicit support not currently available for:
    ///     GLbyte, GlShort and GLint calls, e.g. glVertex2i(int x, int y);
    ///     Vertices with homgeneous coords, e.g.
    ///         glVertex4f(float x, float y, float z, float w);
    ///     3&4-dimensional texture coords, e.g. glTexCoord4f
    ///     Generic attributes and mulitple texture coords per vertex

    /// Vertices
    virtual void Vertex3f(GLfloat x, GLfloat y, GLfloat z) = 0;

    void Vertex2f(GLfloat x, GLfloat y) { Vertex3f(x, y, 0.0f); }
    void Vertex2fv(const GLfloat* v) { Vertex3f(v[0], v[1], 0.0f); }
    void Vertex3fv(const GLfloat* v) { Vertex3f(v[0], v[1], v[2]); }
    void Vertex2d(GLdouble x, GLdouble y) { Vertex3f(float(x), float(y), 0.0f); }
    void Vertex2dv(const GLdouble* v) { Vertex3f(float(v[0]), float(v[1]), 0.0f); }
    void Vertex3d(GLdouble x, GLdouble y, GLdouble z) { Vertex3f(float(x), float(y), float(z)); }
    void Vertex3dv(const GLdouble* v) { Vertex3f(float(v[0]), float(v[1]), float(v[2])); }

    /// Rect() functions also do Begin() and End() (as in OpenGL)
    virtual void Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) = 0;

    void Rectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
    {
        Rectf(float(x1), float(y1), float(x2), float(y2));
    }
    void RectC(const TVPRect& rc)
    {
        Rectf(float(rc.Left()), float(rc.Bottom()), float(rc.Right()), float(rc.Top()));
    }
    void RectC(const TModelRect& rc)
    {
        Rectf(float(rc.Left()), float(rc.Bottom()), float(rc.Right()), float(rc.Top()));
    }

    /// Colors (also defined in IGlState since they can be set either before
    /// or inside of Begin()/End().  Other state must be set before.

    /// Texture coords
    virtual void TexCoord2f(GLfloat s, GLfloat t) = 0;

    void TexCoord2fv(const GLfloat* v) { TexCoord2f(v[0], v[1]); }
    void TexCoord2d(GLdouble s, GLdouble t) { TexCoord2f(float(s), float(t)); }
    void TexCoord2dv(const GLdouble* v) { TexCoord2f(float(v[0]), float(v[1])); }
    /// @}

    /// @name Text interface
    /// @{

    /// Text is drawn is pixel coordinates.  Its position is 
    /// transformed by the current ModelView matrix.  The color and alpha for
    /// text are set in BeginText() and can be changed with SetTextColor().
    /// Other OpenGL state should not affect text (since BeginText sets 
    /// appropriate state)

    /// Set OpenGL state needed for writing text (call before DrawText)
    virtual void BeginText(const CGlTextureFont* font,
        const CRgbaColor& color) = 0;
    /// Same as above, but get color from m_RenderNode.m_State
    virtual void BeginText(const CGlTextureFont* font) = 0;
    /// Pops matrices and attributes after writing text
    virtual void EndText() = 0;

    /// Write text at specified model coords
    virtual void WriteText(TModelUnit x, TModelUnit y,
        const char* text,
        TModelUnit rotate_degrees = 0.0) = 0;
    /// Write text at specified model coords inside box defined by
    /// (x,y,width,height) with specified alignment, truncation and rotation
    virtual void WriteText(TModelUnit x, TModelUnit y,
        TModelUnit width, TModelUnit height,
        const char* text,
        CGlTextureFont::TAlign align = CGlTextureFont::eAlign_Center,
        CGlTextureFont::ETruncate trunc = CGlTextureFont::eTruncate_Ellipsis,
        TModelUnit rotate_degrees = 0.0f) = 0;

    /// Calls the standard font metric functions except for pdf in which case
    /// it first replaces any bitmap fonts with texture fnots
    virtual TModelUnit GetMetric(const CGlTextureFont* font,
        IGlFont::EMetric metric,
        const char* text = NULL,
        int len = -1) const = 0;
    virtual TModelUnit TextWidth(const CGlTextureFont* font, const char* text) const = 0;
    virtual TModelUnit TextHeight(const CGlTextureFont* font) const = 0;
    virtual TModelUnit GetMaxWidth(const CGlTextureFont* font, int max_num) const = 0;
    /// @}


    /// @name Viewports, matrices and transformations
    /// @{
    virtual void Viewport(GLint x, GLint y, GLsizei width, GLsizei height) = 0;
    virtual void GetViewport(GLint *params) = 0;

    virtual void MatrixMode(GLenum mode) = 0;

    virtual void PushMatrix() = 0;
    virtual void PopMatrix() = 0;
    virtual void LoadIdentity() = 0;
    virtual void LoadMatrixf(const GLfloat *m) = 0;
    virtual void LoadMatrixd(const GLdouble* m) = 0;
    virtual CMatrix4<float> GetModelViewMatrix() const = 0;
    virtual CMatrix4<float> GetProjectionMatrix() const = 0;
    virtual void GetModelViewMatrix(GLdouble* m) const = 0;
    virtual void GetProjectionMatrix(GLdouble* m) const = 0;

    virtual void Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) = 0;
    virtual void Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) = 0;
    virtual void Scalef(GLfloat x, GLfloat y, GLfloat z) = 0;
    virtual void Translated(GLdouble x, GLdouble y, GLdouble z) = 0;
    virtual void Translatef(GLfloat x, GLfloat y, GLfloat z) = 0;

    virtual void Ortho(GLdouble left, GLdouble right,
        GLdouble bottom, GLdouble top,
        GLdouble nearVal, GLdouble farVal) = 0;
    /// @}

    // Clipping rectangles (implemented in SVG only at the moment)
    virtual void BeginClippingRect(GLint x, GLint y, GLsizei width, GLsizei height) = 0;
    virtual void EndClippingRect() = 0;

    virtual void Render(CGlPane& pane, CGlModel2D* model) = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// convenience function for getting current render manager 
///
NCBI_GUIOPENGL_EXPORT IRender& GetGl();


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___IRENDER__HPP
