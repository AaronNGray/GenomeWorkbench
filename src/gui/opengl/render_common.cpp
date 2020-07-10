/*  $Id: render_common.cpp 42977 2019-05-01 19:51:41Z katargir $
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

#include <gui/opengl/render_common.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/glmodel2d.hpp>


 // define this to cause OpenGL actual state to be updated immediately on state
 // update calls (useful if some code is using rendermanager and some isn't, and
 // state is shared between the two)
#define GL_STATE_UPDATE

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IRender& GetGl()
{
    return CGlResMgr::Instance().GetCurrentRenderer().GetObject();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


CRenderCommon::CRenderCommon()
    : m_MatrixMode(GL_NONE)
    , m_CurrentMode(GL_NONE)
    , m_CurrentFont(NULL)
{
    m_State.Reset(new CGlState());

    m_RenderNode.SetState(m_State.GetPointer());
}

void CRenderCommon::Clear()
{
    m_CurrentMode = GL_NONE;
    m_MatrixMode = GL_NONE;

    m_RenderNode.SetVBOGeom(nullptr);
}

void CRenderCommon::Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    if (m_CurrentMode == GL_NONE) {
        LOG_POST(Error << "Vertex*() called without first calling CGlRender::Begin");
        return;
    }

    m_VertexBuffer.push_back(CVect3<float>(x, y, z));

    // If quads, break up quads into triangle pairs.  Do this whenever
    // 4 new vertices have been added (ignore groups of 6 since they
    // are the previous triangles)    (we do this because quads are deprecated
    // in OpenGL 3+ and because PDF files do not support gouraud-shaded quads)    
    if (m_CurrentMode == GL_QUADS && m_VertexBuffer.size() % 6 == 4) {
        size_t v1 = m_VertexBuffer.size() - 4;
        size_t v3 = v1 + 2;

        // error condition:
        if (m_State->GetPolygonMode() == GL_LINE) {
            _TRACE("Warning: Rendering triangle pairs for quads in line mode");
        }

        if (m_ColorBuffer.size() > 0) {
            x_SyncAttribs(m_ColorBuffer);
            m_ColorBuffer.push_back(m_ColorBuffer[v1]);
            m_ColorBuffer.push_back(m_ColorBuffer[v3]);
        }

        if (m_TexCoord2DBuffer1.size() > 0) {
            x_SyncAttribs(m_TexCoord2DBuffer1);
            m_TexCoord2DBuffer1.push_back(m_TexCoord2DBuffer1[v1]);
            m_TexCoord2DBuffer1.push_back(m_TexCoord2DBuffer1[v3]);
        }

        // Do vertices last since x_SyncAttribs syncs size of attribute
        // buffers to current size of this (m_VertexBuffer) buffer.
        m_VertexBuffer.push_back(m_VertexBuffer[v1]);
        m_VertexBuffer.push_back(m_VertexBuffer[v3]);
    }
}

void CRenderCommon::Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "Rect*() called between Begin() and End()");
        return;
    }

    if (m_State->GetPolygonMode() == GL_LINE) {
        Begin(GL_LINE_STRIP);
        Vertex3f(x1, y1, 0.0f);
        Vertex3f(x2, y1, 0.0f);
        Vertex3f(x2, y2, 0.0f);
        Vertex3f(x1, y2, 0.0f);
        Vertex3f(x1, y1, 0.0f);
        End();
    }
    else {
        Begin(GL_TRIANGLE_STRIP);
        Vertex3f(x1, y1, 0.0f);
        Vertex3f(x2, y1, 0.0f);
        Vertex3f(x1, y2, 0.0f);
        Vertex3f(x2, y2, 0.0f);
        End();
    }
}

void CRenderCommon::ColorC(const CRgbaColor& c)
{
#ifdef GL_STATE_UPDATE
    glColor4f(c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha());
#endif

    // Color is the one command that can be called both between Begin()/End() and
    // outside of Begin()/End().  If we are outside, update the state. If we
    // are inside, update the attribute array too.    
    if (m_CurrentMode == GL_NONE) {
        m_State->ColorC(c);
        return;
    }

    size_t attrib_size = m_ColorBuffer.size();
    size_t vert_size = m_VertexBuffer.size();

    // If an attribute (color, normal, texcoord) is provided for any 
    // vertex, there most be an attribute for all vertices since buffers
    // must have same number of elements when rendered.  Attributes should
    // be provided ahead of vertices (it applies to the next vertex).

    // Just right 
    if (attrib_size == vert_size) {
        m_ColorBuffer.push_back(c);
    }
    // already have correct # of attributes - replace last element
    else if (attrib_size == vert_size + 1) {
        m_ColorBuffer.back() = c;
    }
    // too few attributes (could be any number missing up to vert_size).  This
    // is normal opengl usage - e.g. one color applies to all vertices for a tri
    if (attrib_size < vert_size) {
        x_SyncAttribs(m_ColorBuffer);
        m_ColorBuffer.push_back(c);
    }
    // this should never happen
    else if (attrib_size > vert_size + 1) {
        LOG_POST(Error << "Error - color attributes exceed expected count");
        m_ColorBuffer.back() = c;
    }

    // remember color state so that if not set before next glbegin/glend, this color will be used
    m_State->ColorC(c);
}


void CRenderCommon::TexCoord2f(GLfloat s, GLfloat t)
{
    CVect2<float> v(s, t);

    size_t attrib_size = m_TexCoord2DBuffer1.size();
    size_t vert_size = m_VertexBuffer.size();

    // If an attribute (color, normal, texcoord) is provided for any 
    // vertex, there most be an attribute for all vertices since buffers
    // must have same number of elements when rendered.  Attributes should
    // be provided ahead of vertices (it applies to the next vertex).

    // Just right 
    if (attrib_size == vert_size) {
        m_TexCoord2DBuffer1.push_back(v);
    }
    // already have correct # of attributes - replace last element
    else if (attrib_size == vert_size + 1) {
        m_TexCoord2DBuffer1.back() = v;
    }
    // too few attributes (could be any number missing up to vert_size).  This
    // is normal opengl usage - e.g. one color applies to all vertices for a tri
    if (attrib_size < vert_size) {
        x_SyncAttribs(m_TexCoord2DBuffer1);
        m_TexCoord2DBuffer1.push_back(v);
    }
    // this should never happen
    else if (attrib_size > vert_size + 1) {
        LOG_POST(Error << "Error - texcoord2 attributes exceed expected count");
        m_TexCoord2DBuffer1.back() = v;
    }
}

void CRenderCommon::BeginText(const CGlTextureFont* font, const CRgbaColor& color)
{
    m_CurrentFont = font;
    m_TextColor = color;

    m_CurrentFont->BeginText();
}

void CRenderCommon::BeginText(const CGlTextureFont* font)
{
    m_CurrentFont = font;
    m_TextColor = m_State->GetColor();

    m_CurrentFont->BeginText();
}

void CRenderCommon::EndText()
{
    m_CurrentFont->EndText();
    m_CurrentFont = NULL;
}

void CRenderCommon::WriteText(TModelUnit x, TModelUnit y,
    const char* text,
    TModelUnit rotate_degrees)
{
    if (m_CurrentFont == NULL) {
        LOG_POST(Error << "Unable to write text - must call BeginText() first");
        return;
    }

    m_CurrentFont->WriteText(x, y, text, m_TextColor.GetColorArray(), rotate_degrees);
}

void CRenderCommon::WriteText(TModelUnit x, TModelUnit y,
    TModelUnit width, TModelUnit height,
    const char* text,
    CGlTextureFont::TAlign align,
    CGlTextureFont::ETruncate trunc,
    TModelUnit rotate_degrees)
{
    if (m_CurrentFont == NULL) {
        LOG_POST(Error << "Unable to write text - must call BeginText() first");
        return;
    }

    m_CurrentFont->WriteText(x, y, width, height, text, m_TextColor.GetColorArray(), align, trunc, rotate_degrees);
}

TModelUnit CRenderCommon::GetMetric(const CGlTextureFont* font,
    IGlFont::EMetric metric,
    const char* text,
    int len) const
{
    return font->GetMetric(metric, text, len);
}

TModelUnit CRenderCommon::TextWidth(const CGlTextureFont* font, const char* text) const
{
    return font->TextWidth(text);
}

TModelUnit CRenderCommon::TextHeight(const CGlTextureFont* font) const
{
    return font->TextHeight();
}

TModelUnit CRenderCommon::GetFontDescender(const CGlTextureFont* font) const
{
    return font->GetFontDescender();
}

TModelUnit CRenderCommon::GetAdvance(const CGlTextureFont* font, char c) const
{
    return font->GetAdvance(c);
}

TModelUnit CRenderCommon::GetMaxWidth(const CGlTextureFont* font, int max_num) const
{
    return font->GetMaxWidth(max_num);
}

void CRenderCommon::Begin(GLenum mode)
{
    // If End wasn't called after last begin log an error (but continue so that subsequent
    // rendering will work)
    if (m_CurrentMode != GL_NONE) {
        LOG_POST(Error << "Begin() called without first calling CGlRender::End()");
    }

    m_CurrentMode = mode;

    if (m_CurrentMode == GL_POLYGON) {
        LOG_POST(Error << "Error - GL_POLYGON rendering not supported");
        return;
    }
    else if (m_CurrentMode == GL_QUAD_STRIP) {
        LOG_POST(Error << "Error - GL_QUAD_STRIP rendering not supported");
    }

    m_VertexBuffer.clear();
    m_ColorBuffer.clear();
    m_TexCoord2DBuffer1.clear();
}

void CRenderCommon::End()
{
    size_t vert_size = m_VertexBuffer.size();

    if (m_CurrentMode == GL_NONE) {
        LOG_POST(Error << "End() called without first calling CGlRender::Begin");
        // Can't render anything if we don't know what user wanted (tris? lines?)
        return;
    }

    if (vert_size == 0) {
        m_CurrentMode = GL_NONE;
        return;
    }

    // All attribute vectors that are in use must be the same 
    // size as the vertex buffer
    if (m_ColorBuffer.size() > 0 && m_ColorBuffer.size() < vert_size)
        x_SyncAttribs(m_ColorBuffer);

    if (m_TexCoord2DBuffer1.size() > 0 && m_TexCoord2DBuffer1.size() < vert_size)
        x_SyncAttribs(m_TexCoord2DBuffer1);

    // For quads we set the render mode passed to OpenGL to triangles since we
    // break down quads into tris.
    GLenum drawMode = (m_CurrentMode == GL_QUADS) ? GL_TRIANGLES : m_CurrentMode;

    if (!m_RenderNode.GetVBOGeom())
        m_RenderNode.SetVBOGeom(drawMode);
    else
        m_RenderNode.GetVBOGeom()->x_SetDrawMode(drawMode);

    m_CurrentMode = GL_NONE;

    // Sync all vertex and attribute buffers with the stl vectors. It would be more efficient
    // to copy directly into the vertex buffers but we can't do that with the begin/end api
    // since we don't know in advance how many elements there will be.   
    m_RenderNode.SetVertexBuffer3D(m_VertexBuffer);
    m_RenderNode.SetColorBuffer(m_ColorBuffer);
    if (m_TexCoord2DBuffer1.size() > 0)
        m_RenderNode.SetTexCoordBuffer(m_TexCoord2DBuffer1);

    /*
    CMatrix4<float>  mat;
    glGetFloatv(GL_MODELVIEW_MATRIX, mat.GetData());
    mat.Transpose();
    m_RenderNode.SetPosition(mat);
    */
    m_RenderNode.SetPosition(GetModelViewMatrix());

    x_RenderBuffer(&m_RenderNode);
}

void CRenderCommon::Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    m_Viewport[0] = x;
    m_Viewport[1] = y;
    m_Viewport[2] = width;
    m_Viewport[3] = height;
}

void CRenderCommon::GetViewport(GLint *params)
{
    // Don't get from opengl since that is limited to max buffer size but viewport, 
    // when used for other targets, e.g. pdf, may be larger.
    params[0] = m_Viewport[0];
    params[1] = m_Viewport[1];
    params[2] = m_Viewport[2];
    params[3] = m_Viewport[3];
}

void CRenderCommon::x_SyncAttribs(vector<CRgbaColor>& colors)
{
    // OpenGL color default  
    CRgbaColor default_color = m_State->GetColor();

    // But if we already have attributes, most recent one is default:
    if (colors.size() > 0) {
        default_color = colors.back();
    }

    // Copy default value into array until length equals vertex buffer length
    colors.insert(colors.end(),
        m_VertexBuffer.size() - colors.size(),
        default_color);
}

void CRenderCommon::x_SyncAttribs(vector<CVect3<float> >& vecs)
{
    // default
    CVect3<float> default_value(0.0f, 0.0f, 0.0f);

    // But if we already have attributes, most recent one is default:
    if (vecs.size() > 0) {
        default_value = vecs.back();
    }

    // Copy default value into array until length equals vertex buffer length
    vecs.insert(vecs.end(),
        m_VertexBuffer.size() - vecs.size(),
        default_value);
}

void CRenderCommon::x_SyncAttribs(vector<CVect2<float> >& vecs)
{
    // default
    CVect2<float> default_value(0.0f, 0.0f);

    // But if we already have attributes, most recent one is default:
    if (vecs.size() > 0) {
        default_value = vecs.back();
    }

    // Copy default value into array until length equals vertex buffer length
    vecs.insert(vecs.end(),
        m_VertexBuffer.size() - vecs.size(),
        default_value);
}

void CRenderCommon::Render(CGlPane& pane, CGlModel2D* model)
{
    if (model != NULL) {
        model->Render(pane);
    }
}

void CRenderCommon::x_RenderBuffer(CGlVboNode* node)
{
    node->Render();
}

END_NCBI_SCOPE
