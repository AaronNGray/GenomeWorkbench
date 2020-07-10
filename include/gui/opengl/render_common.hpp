#ifndef GUI_OPENGL___GL_RENDER_COMMON__HPP
#define GUI_OPENGL___GL_RENDER_COMMON__HPP

/*  $Id: render_common.hpp 45024 2020-05-09 02:03:16Z evgeniev $
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

#include <gui/opengl/irender.hpp>

#include <gui/opengl/glvbonode.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIOPENGL_EXPORT CRenderCommon : public IRender
{
public:
    CRenderCommon();
    virtual ~CRenderCommon() {}

    virtual void SetIsGreyscale(bool /*b*/) {};

    virtual bool IsSimplified() const { return false; };

    virtual bool IsPrinterFriendly() const  { return false; };

    /// Clear/delete any graphics/OpenGL memory.  Call this if you delete
    /// the current OpenGL context
    virtual void Clear();

    /// Overridden in subclasses to return the supported rendering target
    virtual ERenderTarget GetApi() { return eApiUndefined; }

    /// Start rendering.  "mode" may be one of:
    /// GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES, GL_TRIANGLE_FAN, 
    /// GL_TRIANGLE_STRIP or GL_QUADS.
    /// Start rendering - use state in existing 'state' object.
    virtual void Begin(GLenum mode);

    /// Finish rendering (create buffer and send to renderer)
    virtual void End();

    /// @name Polygon rendering interface
    /// @{

    /// Explicit support not currently available for:
    ///     GLbyte, GlShort and GLint calls, e.g. glVertex2i(int x, int y);
    ///     Vertices with homgeneous coords, e.g.
    ///         glVertex4f(float x, float y, float z, float w);
    ///     3&4-dimensional texture coords, e.g. glTexCoord4f
    ///     Generic attributes and mulitple texture coords per vertex

    /// Vertices
    virtual void Vertex3f(GLfloat x, GLfloat y, GLfloat z);

    /// Rect() functions also do Begin() and End() (as in OpenGL)
    virtual void Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);

    /// Colors (also defined in IGlState since they can be set either before
    /// or inside of Begin()/End().  Other state must be set before.
    virtual void ColorC(const CRgbaColor& color);

    /// Texture coords
    virtual void TexCoord2f(GLfloat s, GLfloat t);

    /// @name Text interface
    /// @{

    /// Text is drawn is pixel coordinates.  Its position is 
    /// transformed by the current ModelView matrix.  The color and alpha for
    /// text are set in BeginText() and can be changed with SetTextColor().
    /// Other OpenGL state should not affect text (since BeginText sets 
    /// appropriate state)

    /// Set OpenGL state needed for writing text (call before DrawText)
    virtual void BeginText(const CGlTextureFont* font,
        const CRgbaColor& color);
    /// Same as above, but get color from m_RenderNode.m_State
    virtual void BeginText(const CGlTextureFont* font);
    /// Pops matrices and attributes after writing text
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

    /// Calls the standard font metric functions except for pdf in which case
    /// it first replaces any bitmap fonts with texture fnots
    virtual TModelUnit GetMetric(const CGlTextureFont* font,
        IGlFont::EMetric metric,
        const char* text = NULL,
        int len = -1) const;
    virtual TModelUnit TextWidth(const CGlTextureFont* font, const char* text) const;
    virtual TModelUnit TextHeight(const CGlTextureFont* font) const;
    virtual TModelUnit GetFontDescender(const CGlTextureFont* font) const;
    virtual TModelUnit GetAdvance(const CGlTextureFont* font, char c) const;
    virtual TModelUnit GetMaxWidth(const CGlTextureFont* font, int max_num) const;
    /// @}

    /// @name Viewports, matrices and transformations
    /// @{
    virtual void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
    virtual void GetViewport(GLint *params);
    /// @}

    /// Allow caller to access state directly
    virtual CRef<CGlState> GetState() { return m_State; }
    virtual void ResetState()
    {
        if (m_State)
            m_State->Reset();
    }

    virtual void BeginClippingRect(GLint x, GLint y, GLsizei width, GLsizei height) {};
    virtual void EndClippingRect() {};

    virtual void Render(CGlPane& pane, CGlModel2D* model);

protected:
    virtual void x_RenderBuffer(CGlVboNode* node);

    void x_SyncAttribs(vector<CRgbaColor>& colors);
    void x_SyncAttribs(vector<CVect3<float> >& vecs);
    void x_SyncAttribs(vector<CVect2<float> >& vecs);

    vector<CVect3<float> > m_VertexBuffer;
    vector<CRgbaColor> m_ColorBuffer;
    vector<CVect2<float> >  m_TexCoord2DBuffer1;

    /// vertex buffer node for rendering all Begin()/End() renders
    CGlVboNode m_RenderNode;
    /// state used by all nodes
    CRef<CGlState> m_State;

    /// current modelview matrix and matrix mode
    GLenum           m_MatrixMode;

    /// current projection set by Viewport()
    GLint m_Viewport[4];

    /// rendering mode from Begin()
    GLenum m_CurrentMode;

    /// text parameters
    const CGlTextureFont* m_CurrentFont;
    /// color and alpha for text only (other GL options do not apply to text)
    CRgbaColor m_TextColor;

private:
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_RENDER_COMMON__HPP
