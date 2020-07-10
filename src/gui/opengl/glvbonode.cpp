/*  $Id: glvbonode.cpp 43891 2019-09-16 13:50:00Z evgeniev $
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
#include <gui/opengl/glvbonode.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glexception.hpp>
#include <gui/opengl/glresmgr.hpp>

BEGIN_NCBI_SCOPE

CGlVboNode::CGlVboNode()
{
}

// Draw mode is one of: GL_{POINTS, LINES, LINE_STRIP, LINE_LOOP, 
// TRIANGLES, TRIANGLE_STRIP TRIANGLE_FAN, QUADS, QUAD_STRIP}
CGlVboNode::CGlVboNode(GLenum draw_mode)
{
    SetVBOGeom(draw_mode);
}

CGlVboNode::~CGlVboNode()
{    
    m_VBOGeom.Reset();
}

void CGlVboNode::SetVBOGeom(GLenum drawMode)
{
    m_VBOGeom.Reset(CGlResMgr::Instance().CreateVboGeom(drawMode, ""));
}

IVboGeom::EVertexFormat CGlVboNode::GetVertexFormat() const
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->GetVertexFormat();
    else NCBI_THROW(COpenGLException, eGenericError, "NULL VBO object");
}

IVboGeom::ESecondaryFormat CGlVboNode::GetSecondaryFormat() const
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->GetSecondaryFormat();
    else NCBI_THROW(COpenGLException, eGenericError, "NULL VBO object");
}

void CGlVboNode::SetVertexBuffer2D(const vector<CVect2<float> >& data)
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->SetVertexBuffer2D(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::GetVertexBuffer2D(vector<CVect2<float> >& data) const
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->GetVertexBuffer2D(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::SetVertexBuffer3D(const vector<CVect3<float> >& data)
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->SetVertexBuffer3D(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::GetVertexBuffer3D(vector<CVect3<float> >& data) const
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->GetVertexBuffer3D(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::SetColorBuffer(const vector<CRgbaColor>& data)
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->SetColorBuffer(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::GetColorBuffer(vector<CRgbaColor>& data) const
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->GetColorBuffer(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::SetColorBufferUC(const vector<CVect4<unsigned char> >& data)
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->SetColorBufferUC(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::GetColorBufferUC(vector<CVect4<unsigned char> >& data) const
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->GetColorBufferUC(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::SetTexCoordBuffer1D(const vector<float>& data)
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->SetTexCoordBuffer1D(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::GetTexCoordBuffer1D(vector<float>& data) const
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->GetTexCoordBuffer1D(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::SetTexCoordBuffer(const vector<CVect2<float> >& data)
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->SetTexCoordBuffer(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::GetTexCoordBuffer(vector<CVect2<float> >& data) const
{
    if (m_VBOGeom != NULL)
        return m_VBOGeom->GetTexCoordBuffer(data);
    else NCBI_THROW(COpenGLException, eGenericError, "NULL Texture Coord 1D Buffer");
}

void CGlVboNode::Get2DVertexBuffer(vector<CVect2<float> >& verts) const
{
    verts.clear();
    vector<CVect3<float> > verts3d;

    switch (GetVertexFormat())
    {
    case IVboGeom::kVertexFormatVertex2D:
        GetVertexBuffer2D(verts);
        break;
    case IVboGeom::kVertexFormatVertex3D:
        GetVertexBuffer3D(verts3d);
        verts.resize(verts3d.size());
        for (size_t i = 0; i < verts3d.size(); ++i)
            verts[i].Set(verts3d[i].X(), verts3d[i].Y());
        break;
    default:
        break;
    }
}

bool CGlVboNode::GetColors(vector<CRgbaColor>& colors, bool is_greyscale) const
{
    bool has_color = false;
    IVboGeom::ESecondaryFormat colorFormat = GetSecondaryFormat();
    if (colorFormat == IVboGeom::kSecondaryFormatColorFloat) {
        has_color = true;
        GetColorBuffer(colors);
    }
    else if (colorFormat == IVboGeom::kSecondaryFormatColorUChar) {
        has_color = true;
        vector<CVect4<unsigned char> > colors_uc;
        colors.clear();
        GetColorBufferUC(colors_uc);
        for (size_t i = 0; i < colors_uc.size(); ++i)
            colors.push_back(CRgbaColor(colors_uc[i][0], colors_uc[i][1], colors_uc[i][2], colors_uc[i][3]));
    }

    // If we are in grey scale mode, make sure all colors are greyscale. 
    if (has_color && is_greyscale) {
        for (size_t i = 0; i < colors.size(); ++i) {
            colors[i] = colors[i].GetGreyscale();
        }
    }

    return has_color;
}

bool CGlVboNode::GetDefaultColor(CRgbaColor& default_color, bool is_greyscale) const
{
    default_color = GetState().GetColor();

    if (is_greyscale) {
        default_color = default_color.GetGreyscale();
    }

    return GetState().ColorSet();
}

void CGlVboNode::x_Render()
{      
    if (m_VBOGeom != NULL) {
        for (size_t i=0; i<m_Positions.size(); ++i) {
            CMatrix4<float> mat = GetTransformedPosition(i);
            mat.Transpose();
            m_VBOGeom->Render(mat.GetData());
        }
    }
}


END_NCBI_SCOPE
