#ifndef GUI_UTILS__SVG_RENDERER_HPP
#define GUI_UTILS__SVG_RENDERER_HPP

/*  $Id: svg_renderer.hpp 45024 2020-05-09 02:03:16Z evgeniev $
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
 * Authors:  Vladislav Evgeniev
 *
 * File Description: SVG file rendering
 *
 */

#include <stack>
#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/opengl/render_vector.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/print/svg.hpp>


/** @addtogroup GUI_PRINT
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class NCBI_GUIPRINT_EXPORT CSVGRenderer : public CObject, public CRenderVector
{
public:       
    virtual bool IsSimplified() const { return m_Simplified; };
    void SetSimplified(bool simplified = true) { m_Simplified = simplified; };

    virtual bool IsPrinterFriendly() const { return true; };

    virtual void SetIsGreyscale(bool b) { m_IsGreyscale = b; };

    virtual ERenderTarget GetApi() { return eRenderSVG; }

    virtual void Hint(GLenum, GLenum) {}

    virtual void Initialize(const TVPRect& viewport);
    virtual void Finalize();

    virtual void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);

    virtual void Ortho(GLdouble left, GLdouble right,
        GLdouble bottom, GLdouble top,
        GLdouble nearVal, GLdouble farVal);

    /// Set OpenGL state needed for writing text (call before DrawText)
    virtual void BeginText(const CGlTextureFont* font, 
                           const CRgbaColor& color);
    /// Same as above, but get color from m_RenderNode.m_State
    virtual void BeginText(const CGlTextureFont* font);
    /// Pops matrices and attributes after writing text
    virtual void EndText();
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
    
    void Write(CNcbiOstream& ostrm);

    virtual void BeginClippingRect(GLint x, GLint y, GLsizei width, GLsizei height);
    virtual void EndClippingRect();
    
protected:
    virtual void x_RenderBuffer(CGlVboNode *node);
    void x_PrintLineBuffer(CGlVboNode &node);
    void x_PrintPointBuffer(CGlVboNode &node);
    void x_PrintTriBuffer(CGlVboNode &node);
    void x_PrintQuadBuffer(CGlVboNode &node);

    void x_ProjectVertex(CVect2<float>& vertex);
    void x_ProjectVertices(vector<CVect2<float>>& vertices);
    string x_PolygonStippleToPattern(const GLubyte* stipple, const CRgbaColor& fill_rgba_color);
    string x_LinearGradient(std::vector<CRgbaColor> const& colors);
    string x_ClippingRect(GLint x, GLint y, GLsizei width, GLsizei height);
    void x_GetGradientColors(vector<CVect2<float>> const& vertices, vector<CRgbaColor> const &colors, vector<CRgbaColor>& gradient);
    void x_GetLineStyle(CGlVboNode &node, GLushort &pattern, GLint &factor, double &width, svg::Stroke::ELineCapStyle &lcap, svg::Stroke::ELineJoinStyle &ljoin);

private:
    bool        m_Simplified = false;
    bool        m_IsGreyscale = false;
    int         m_Height;
    map<string, string> m_PolygonStipplePatterns;
    map<string, string> m_LinearGradients;
    map<string, string> m_ClippingRects;
    unique_ptr<svg::SVG>    m_SVG;
    std::string             m_ClippingId;
    std::stack<std::string> m_ClippingStack;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS__SVG_RENDERER_HPP
