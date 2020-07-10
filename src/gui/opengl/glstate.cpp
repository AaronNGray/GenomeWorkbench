/*  $Id: glstate.cpp 44821 2020-03-23 16:08:14Z evgeniev $
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

#include <ncbi_pch.hpp>
#include <gui/opengl/glstate.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/i3dtexture.hpp>
#include <gui/opengl/irender.hpp>

BEGIN_NCBI_SCOPE


/// Defaults are set to be the stated openGL defaults for those values.
CGlState::CGlState()
: m_LineWidth(1.0f)
, m_LineWidthSet(false)
, m_LineJoinStyle(eMiteredJoin)
, m_LineJoinStyleSet(false)
, m_LineCapStyle(eButtCap)
, m_LineCapStyleSet(false)
, m_PdfShadeStyle(eGouraud)
, m_PdfShadeStyleSet(false)
, m_PointSize(1.0f)
, m_PointSizeSet(false)
, m_ShadeModel(GL_SMOOTH)
, m_ShadeModelSet(false)
, m_ScissorRect(0, 0, 0, 0)
, m_ScissorSet(false)
, m_Color(1.0f, 1.0f, 1.0f, 1.0f)
, m_ColorSet(false)
, m_ColorMaskRed(true)
, m_ColorMaskGreen(true)
, m_ColorMaskBlue(true)
, m_ColorMaskAlpha(true)
, m_ColorMaskSet(false)
, m_PolygonModeFace(GL_FRONT_AND_BACK)
, m_PolygonMode(GL_FILL)
, m_PolygonModeSet(false)
, m_LineStippleFactor(1)
, m_LineStipplePattern(0xFFFF)
, m_LineStippleSet(false)    
, m_PolygonStippleSet(false)
, m_SourceBlendFactor(GL_ONE)
, m_TargetBlendFactor(GL_ZERO)
, m_BlendFuncSet(false)
, m_BlendEquation(GL_FUNC_ADD)
, m_BlendEquationSet(false)
, m_BlendFuncSeparateSet(false)
, m_BlendColor(0.0f, 0.0f, 0.0f, 0.0f)
, m_BlendColorSet(false)
, m_ScaleInvarient(false)
, m_ScaleInvarientSet(false)
, m_ScaleFactor(TModelUnit(1), TModelUnit(1))
, m_Texture(nullptr)
, m_Dirty(true)
{
}

void CGlState::MakeCurrent(ERenderTarget target)
{
 //   if (!m_Dirty)
 //       return;
    m_Dirty = false;

    IRender& gl = GetGl();

    if (m_Texture != NULL)
        m_Texture->MakeCurrent();

    if (m_LineWidthSet)
        gl.LineWidth(m_LineWidth);

    if (m_PointSizeSet)
        gl.PointSize(m_PointSize);

    if (m_ShadeModelSet)
        glShadeModel(m_ShadeModel);   

    if (m_ScissorSet) 
        glScissor(m_ScissorRect.Left(), m_ScissorRect.Bottom(),
                  m_ScissorRect.Width(), m_ScissorRect.Height());

    if (m_ColorSet)
        gl.Color4fv(m_Color.GetColorArray());

    if (m_ColorMaskSet) 
        glColorMask(m_ColorMaskRed,
                    m_ColorMaskGreen,
                    m_ColorMaskBlue,
                    m_ColorMaskAlpha);

    if (m_PolygonModeSet)
        glPolygonMode(m_PolygonModeFace, m_PolygonMode);  

    if (m_LineStippleSet) 
        glLineStipple(m_LineStippleFactor, m_LineStipplePattern);  

    if (m_PolygonStippleSet)
        glPolygonStipple(m_PolygonStippleMask);

    size_t i;
    for (i=0; i<m_Enabled.size(); ++i) {
        gl.Enable(m_Enabled[i]);

        if (m_Enabled[i] == GL_BLEND) {
            gl.BlendFunc(m_SourceBlendFactor, m_TargetBlendFactor);
            if (target == eOpenGL20) {
                if (m_BlendEquationSet) {
                    glBlendEquation(m_BlendEquation);
                }

                if (m_BlendColorSet) {
                    glBlendColor(m_BlendColor.GetRed(),
                        m_BlendColor.GetGreen(),
                        m_BlendColor.GetBlue(),
                        m_BlendColor.GetAlpha());
                }

                // OpenGL 1.4
                if (m_BlendFuncSeparateSet) {
                    glBlendFuncSeparate(m_SourceRgbFactor,
                        m_TargetRgbFactor,
                        m_SourceAlpahFactor,
                        m_TargetAlphaFactor);
                }
            }
        }
    }

    for (i=0; i<m_Disabled.size(); ++i) {
        gl.Disable(m_Disabled[i]);
    }
}

/*
void CGlState::SetDefaultGLState()
{
}

void CGlState::ReadState()
{
}
*/

void CGlState::MergeStates(CGlState& s, GLbitfield mask)
{
    // Merge any state information not included in the mask
    if (mask == GL_ALL_ATTRIB_BITS)
        return;

    // now go one attribute at a time and if none of the mask
    // variables refer to that item, merge it.
    
    if (!(mask|GL_LINE_BIT)) {
        m_LineWidth = s.m_LineWidth;
        m_LineWidthSet = s.m_LineWidthSet;

        // put pdf attributes under most apprpriate opengl bits
        m_LineJoinStyle = s.m_LineJoinStyle;
        m_LineJoinStyleSet = s.m_LineJoinStyleSet;

        /// Cap (line terminator) style (pdf only)
        m_LineCapStyle = s.m_LineCapStyle;
        m_LineCapStyleSet = s.m_LineCapStyleSet;
    }

    if (!(mask|GL_POINT_BIT)) {
        m_PointSize = s.m_PointSize;
        m_PointSizeSet = s.m_PointSizeSet;
    }

    if (!(mask|GL_LIGHTING_BIT)) {
        m_ShadeModel = s.m_ShadeModel;
        m_ShadeModelSet = s.m_ShadeModelSet;
    }
   
    if (!(mask|GL_SCISSOR_BIT)) {
        m_ScissorRect = s.m_ScissorRect;
        m_ScissorSet = s.m_ScissorSet;
        // GL_SCISSOR_TEST can be masked by GL_SCISSOR_BIT or GL_ENABLE_BIT
    }
    
    /// Current color (glColor())
    if (!(mask|GL_CURRENT_BIT)) {
        m_Color = s.m_Color;
        m_ColorSet = s.m_ColorSet;
    }

    /// ColorMask ???
    if (!(mask|GL_SCISSOR_BIT)) {
        m_ColorMaskRed = s.m_ColorMaskRed;
        m_ColorMaskGreen = s.m_ColorMaskGreen;
        m_ColorMaskBlue = s.m_ColorMaskBlue;
        m_ColorMaskAlpha = s.m_ColorMaskAlpha;
        m_ColorMaskSet = s.m_ColorMaskSet;
    }

    /// parameters for glPolygonMode.  We only support one entry - can't specify
    /// different modes for GL_FRONT and GL_BACK (should use GL_FRONT_AND_BACK)
    if (!(mask|GL_POLYGON_BIT)) {
        m_PolygonModeFace = s.m_PolygonModeFace;
        m_PolygonMode = s.m_PolygonMode;
        m_PolygonModeSet = s.m_PolygonModeSet;
        // GL_POLYGON_STIPPLE can be masked by GL_POLYGON_BIT or GL_ENABLE_BIT
        // but not GL_POLYGON_STIPPLE_BIT
        // GL_CULL_FACE is  masked by GL_POLYGON_BIT and GL_ENABLE_BIT
    }

    /// glLineStipple(factor, pattern)  Deprecated in OpenGL 3+
    if (!(mask|GL_LINE_BIT)) {
        m_LineStippleFactor = s.m_LineStippleFactor;
        m_LineStipplePattern = s.m_LineStipplePattern;
        m_LineStippleSet = s.m_LineStippleSet;
        // GL_LINE_STIPPLE enable can be masked by GL_LINE_BIT or GL_ENABLE_BIT        
    }

    if (!(mask|GL_POLYGON_STIPPLE_BIT)) {
        memcpy(m_PolygonStippleMask, s.m_PolygonStippleMask, sizeof(GLubyte)*32*32);  
        m_PolygonStippleSet = s.m_PolygonStippleSet;
        // GL_POLYGON_STIPPLE is not masked by GL_POLYGON_STIPPLE_BIT
    }

    /// Blend factors for glBlendFunc()
    if (!(mask|GL_COLOR_BUFFER_BIT)) {
        m_SourceBlendFactor = s.m_SourceBlendFactor;
        m_TargetBlendFactor = s.m_TargetBlendFactor;
        m_BlendFuncSet = s.m_BlendFuncSet;

        m_SourceRgbFactor = s.m_SourceRgbFactor;
        m_TargetRgbFactor = s.m_TargetRgbFactor;
        m_SourceAlpahFactor = s.m_SourceAlpahFactor;
        m_TargetAlphaFactor = s.m_TargetAlphaFactor;
        m_BlendFuncSeparateSet = s.m_BlendFuncSeparateSet;
    
        /// Optional color to be used in blending (glBlendColor())
        m_BlendColor = s.m_BlendColor;
        m_BlendColorSet = s.m_BlendColorSet;
    }

    // copy any flags that were set/unset and were not covered
    // by some other flag
    if (!(mask|GL_ENABLE_BIT)) {

        // Iterate over all flags and if they are not specifically covered
        // by another flag, set them in the merged (this) state
        size_t i;
        for (i=0; i<m_Enabled.size(); ++i) {
            if (x_MergeFlag(m_Enabled[i], mask)) {
                Enable(m_Enabled[i]);
            }
        }
        for (i=0; i<m_Disabled.size(); ++i) {
            if (x_MergeFlag(m_Disabled[i], mask)) {
                Disable(m_Disabled[i]);
            }
        }
    }
}

bool CGlState::x_MergeFlag(GLenum e, GLbitfield mask)
{
    if (e==GL_BLEND && (mask|GL_COLOR_BUFFER_BIT))
        return false;
    if (e==GL_ALPHA_TEST && (mask|GL_COLOR_BUFFER_BIT))
        return false;
    if (e==GL_DITHER && (mask|GL_COLOR_BUFFER_BIT))
        return false;
    if (e==GL_COLOR_LOGIC_OP && (mask|GL_COLOR_BUFFER_BIT))
        return false;
    if (e==GL_INDEX_LOGIC_OP && (mask|GL_COLOR_BUFFER_BIT))
        return false;

    if (e==GL_DEPTH_TEST && (mask|GL_DEPTH_BUFFER_BIT))
        return false;

    if (e==GL_FOG && (mask|GL_FOG_BIT))
        return false;

    if (e==GL_COLOR_MATERIAL && (mask|GL_LIGHTING_BIT))
        return false;
    if (e==GL_LIGHTING && (mask|GL_LIGHTING_BIT))
        return false;
    if (e==GL_LIGHT0 && (mask|GL_LIGHTING_BIT))
        return false;
    if (e==GL_LIGHT1 && (mask|GL_LIGHTING_BIT))
        return false;
    if (e==GL_LIGHT2 && (mask|GL_LIGHTING_BIT))
        return false;
    if (e==GL_LIGHT3 && (mask|GL_LIGHTING_BIT))
        return false;
    if (e==GL_SHADE_MODEL && (mask|GL_LIGHTING_BIT))
        return false;

    if (e==GL_LINE_SMOOTH && (mask|GL_LINE_BIT))
        return false;
    if (e==GL_LINE_STIPPLE && (mask|GL_LINE_BIT))
        return false;

    if (e==GL_POINT_SMOOTH && (mask|GL_POINT_BIT))
        return false;

    if (e==GL_CULL_FACE && (mask|GL_POLYGON_BIT))
        return false;
    if (e==GL_POLYGON_STIPPLE && (mask|GL_POLYGON_BIT))
        return false;

    if (e==GL_SCISSOR_TEST && (mask|GL_SCISSOR_BIT))
        return false;

    if (e==GL_STENCIL_TEST && (mask|GL_STENCIL_BUFFER_BIT))
        return false;

    if (e==GL_TEXTURE_1D && (mask|GL_TEXTURE_BIT))
        return false;
    if (e==GL_TEXTURE_2D && (mask|GL_TEXTURE_BIT))
        return false;
    if (e==GL_TEXTURE_3D && (mask|GL_TEXTURE_BIT))
        return false;

    if (e==GL_NORMALIZE && (mask|GL_TRANSFORM_BIT))
        return false;
    if (e==GL_RESCALE_NORMAL && (mask|GL_TRANSFORM_BIT))
        return false;

    return true;
}

void CGlState::Enable(GLenum glstate)
{
    vector<GLenum>::iterator iter = std::find(m_Disabled.begin(), m_Disabled.end(), glstate);
    if (iter != m_Disabled.end()) {
        m_Disabled.erase(iter);
        m_Dirty = true;
    }

    iter = std::find(m_Enabled.begin(), m_Enabled.end(), glstate);
    if (iter == m_Enabled.end()) {
        m_Enabled.push_back(glstate);
        m_Dirty = true;
    }
}

bool CGlState::IsEnabled(GLenum glstate) const
{
    vector<GLenum>::const_iterator iter = std::find(m_Enabled.begin(), m_Enabled.end(), glstate);
    if (iter != m_Enabled.end())
        return true;
    else
        return false;
}

void CGlState::Disable(GLenum glstate)
{
    vector<GLenum>::iterator iter = std::find(m_Enabled.begin(), m_Enabled.end(), glstate);
    if (iter != m_Enabled.end()) {
        m_Enabled.erase(iter);
        m_Dirty = true;
    }

    iter = std::find(m_Disabled.begin(), m_Disabled.end(), glstate);
    if (iter == m_Disabled.end()) {
        m_Disabled.push_back(glstate);
        m_Dirty = true;
    }
}

bool CGlState::IsDisabled(GLenum glstate) const
{
    vector<GLenum>::const_iterator iter = std::find(m_Disabled.begin(), m_Disabled.end(), glstate);
    if (iter != m_Disabled.end())
        return true;
    else
        return false;
}

void CGlState::SetTexture(I3DTexture* tex)
{
    m_Texture = tex;
    m_Dirty = true;
}

void CGlState::LineWidth(GLfloat w) 
{ 
    if (!m_LineWidthSet || w != m_LineWidth) {
        m_LineWidth = w; 
        m_LineWidthSet = true;
        m_Dirty = true;
    }
}

void CGlState::PointSize(GLfloat s) 
{ 
    if (!m_PointSizeSet || s != m_PointSize) {
        m_PointSize = s; 
        m_PointSizeSet = true;
        m_Dirty = true;
    }
}

void CGlState::ShadeModel(GLenum mode)
{
    if (!m_ShadeModelSet || mode != m_ShadeModel) {
        m_ShadeModel = mode;
        m_ShadeModelSet = true;
        m_Dirty = true;
    }
}

void CGlState::Scissor(GLint x, GLint y,
                       GLsizei width, GLsizei height)
{
    TVPRect s(x, y, x+width, y+height);

    if (!m_ScissorSet || s != m_ScissorRect) {
        m_ScissorRect = s;
        m_ScissorSet = true;
        m_Dirty = true;
    }
}

void CGlState::ColorC(const CRgbaColor& c) 
{ 
    m_Color = c; 
    m_ColorSet = true;
    m_Dirty = true;
}

void CGlState::Color3f(GLfloat r, GLfloat g, GLfloat b)
{
    ColorC(CRgbaColor(r, g, b));
}

void CGlState::Color3fv(const GLfloat* v)
{
    ColorC(CRgbaColor(v[0], v[1], v[2]));
}

void CGlState::Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    ColorC(CRgbaColor(r, g, b, a));
}

void CGlState::Color4fv(const GLfloat* v)
{
    ColorC(CRgbaColor(v[0], v[1], v[2], v[3]));
}

void CGlState::Color3d(GLdouble r, GLdouble g, GLdouble b)
{
    ColorC(CRgbaColor(float(r), float(g), float(b)));
}

void CGlState::Color3dv(const GLdouble* v)
{
    ColorC(CRgbaColor(float(v[0]), float(v[1]), float(v[2])));
}

void CGlState::Color4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a)
{
    ColorC(CRgbaColor(float(r), float(g), float(b), float(a)));
}

void CGlState::Color4dv(const GLdouble* v)
{
    ColorC(CRgbaColor(float(v[0]), float(v[1]), float(v[2]), float(v[3])));
}

void CGlState::ColorMask(GLboolean red, 
                         GLboolean green, 
                         GLboolean blue, 
                         GLboolean alpha)
{
    m_ColorMaskRed = red;
    m_ColorMaskGreen = green;
    m_ColorMaskBlue = blue;
    m_ColorMaskAlpha = alpha;
    
    m_ColorMaskSet = true;
    m_Dirty = true;
}

void CGlState::GetColorMask(GLboolean& red, 
                            GLboolean& green, 
                            GLboolean& blue, 
                            GLboolean& alpha)
{
    red = m_ColorMaskRed;
    green = m_ColorMaskGreen;
    blue = m_ColorMaskBlue;
    alpha = m_ColorMaskAlpha;
    m_Dirty = true;
}

void CGlState::PolygonMode(GLenum face, GLenum mode)
{
    if (!m_PolygonModeSet || 
        face != m_PolygonModeFace ||
        mode != m_PolygonMode) {
            m_PolygonModeFace = face;
            m_PolygonMode = mode;
            m_PolygonModeSet = true;
            m_Dirty = true;
    }
}

void CGlState::LineStipple(GLint factor, GLushort pattern)
{
    m_LineStippleFactor = factor;
    m_LineStipplePattern = pattern;
    m_LineStippleSet = true;
    m_Dirty = true;
}

void CGlState::GetLineStipple(GLint& factor, GLushort& pattern) const
{
    factor = m_LineStippleFactor;
    pattern = m_LineStipplePattern;
}

void CGlState::PolygonStipple(GLubyte* mask)
{
    memcpy(m_PolygonStippleMask, mask, 32*32*sizeof(GLubyte));
    m_PolygonStippleSet = true;
    m_Dirty = true;
}

void CGlState::BlendColor(const CRgbaColor& c) 
{ 
    m_BlendColor = c; 
    m_BlendColorSet = true;
    m_Dirty = true;
}

void CGlState::BlendFunc(GLenum sfactor, GLenum dfactor) 
{ 
    if (!m_PolygonModeSet || 
        sfactor != m_SourceBlendFactor ||
        dfactor != m_TargetBlendFactor) {
            m_SourceBlendFactor = sfactor; 
            m_TargetBlendFactor = dfactor; 

            m_BlendFuncSet = true;
            m_Dirty = true;
    }
}

void CGlState::BlendEquation(GLenum mode)
{
    m_BlendEquation = mode;
    m_BlendEquationSet = true;
}

void CGlState::BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
                                 GLenum srcAlpha, GLenum dstAlpha)
{
    m_SourceRgbFactor = srcRGB;
    m_TargetRgbFactor = dstRGB;
    m_SourceAlpahFactor = srcAlpha;
    m_TargetAlphaFactor = dstAlpha;

    m_BlendFuncSeparateSet = true;
    m_Dirty = true;
}

void CGlState::ScaleInvarient(bool b, CVect2<TModelUnit> scale)
{
    m_ScaleInvarient = b;
    m_ScaleFactor = scale;
    m_ScaleInvarientSet = true;

    /// Scale change does not require OpenGL state to be set (m_Dirty invarient)
}

void CGlState::LineJoinStyle(ELineJoinStyle s)
{
    m_LineJoinStyle = s;
    m_LineJoinStyleSet = true;

    // pdf only options do not require state change (m_Dirty invarient)    
}

void CGlState::LineCapStyle(ELineCapStyle c)
{
    m_LineCapStyle = c;
    m_LineCapStyleSet = true;

    // pdf only options do not require state change (m_Dirty invarient)
}

void CGlState::PdfShadeStyle(EPdfShadeStyle s)
{
    m_PdfShadeStyle = s;
    m_PdfShadeStyleSet = true;

    // pdf only options do not require state change (m_Dirty invarient)
}

void CGlState::Reset()
{
    m_LineWidth = 1.0f;
    m_LineWidthSet = false;
    m_LineJoinStyle = eMiteredJoin;
    m_LineJoinStyleSet = false;
    m_LineCapStyle = eButtCap;
    m_LineCapStyleSet = false;
    m_PdfShadeStyle = eGouraud;
    m_PdfShadeStyleSet = false;
    m_PointSize = 1.0f;
    m_PointSizeSet = false;
    m_ShadeModel = GL_SMOOTH;
    m_ShadeModelSet = false;
    m_ScissorRect = TVPRect(0, 0, 0, 0);
    m_ScissorSet = false;
    m_Color = CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_ColorSet = false;
    m_ColorMaskRed = true;
    m_ColorMaskGreen = true;
    m_ColorMaskBlue = true;
    m_ColorMaskAlpha = true;
    m_ColorMaskSet = false;
    m_PolygonModeFace = GL_FRONT_AND_BACK;
    m_PolygonMode = GL_FILL;
    m_PolygonModeSet = false;
    m_LineStippleFactor = 1;
    m_LineStipplePattern = 0xFFFF;
    m_LineStippleSet = false;
    m_PolygonStippleSet = false;
    m_SourceBlendFactor = GL_ONE;
    m_TargetBlendFactor = GL_ZERO;
    m_BlendFuncSet = false;
    m_BlendEquation = GL_FUNC_ADD;
    m_BlendEquationSet = false;
    m_BlendFuncSeparateSet = false;
    m_BlendColor = CRgbaColor(0.0f, 0.0f, 0.0f, 0.0f);
    m_BlendColorSet = false;
    m_ScaleInvarient = false;
    m_ScaleInvarientSet = false;
    m_ScaleFactor = { TModelUnit(1), TModelUnit(1) };
    m_Texture = nullptr;
    m_Dirty = true;
}

END_NCBI_SCOPE
