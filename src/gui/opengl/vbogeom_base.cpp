/*  $Id: vbogeom_base.cpp 43891 2019-09-16 13:50:00Z evgeniev $
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

#include "vbogeom_base.hpp"

BEGIN_NCBI_SCOPE

///
///  CVboGeomBase
///

void CVboGeomBase::x_SetDrawMode(GLenum drawMode)
{
    m_DrawMode = drawMode;

    m_VertexFormat = kVertexFormatNone;
    m_SecondaryFormat = kSecondaryFormatNone;
    m_SecondaryCount = m_VertexCount = 0;
}

IVboGeom::EVertexFormat CVboGeomBase::GetVertexFormat() const
{
    return m_VertexFormat;
}

IVboGeom::ESecondaryFormat CVboGeomBase::GetSecondaryFormat() const
{
    return m_SecondaryFormat;
}

size_t CVboGeomBase::GetVertexCount() const
{
    return m_VertexCount;
}

void CVboGeomBase::SetVertexBuffer2D(const vector<CVect2<float> >& data)
{
    if (data.empty()) {
        m_VertexFormat = kVertexFormatNone;
        m_VertexCount = 0;
    }
    else {
        m_VertexFormat = kVertexFormatVertex2D;
        m_VertexCount = data.size();
        x_SetBufferData(0, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::GetVertexBuffer2D(vector<CVect2<float> >& data) const
{
    data.clear();

    if (m_VertexFormat == kVertexFormatVertex2D) {
        data.resize(m_VertexCount);
        x_GetBufferData(0, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::SetVertexBuffer3D(const vector<CVect3<float> >& data)
{
    if (data.empty()) {
        m_VertexFormat = kVertexFormatNone;
        m_VertexCount = 0;
    }
    else {
        m_VertexFormat = kVertexFormatVertex3D;
        m_VertexCount = data.size();
        x_SetBufferData(0, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::GetVertexBuffer3D(vector<CVect3<float> >& data) const
{
    data.clear();

    if (m_VertexFormat == kVertexFormatVertex3D) {
        data.resize(m_VertexCount);
        x_GetBufferData(0, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::SetColorBuffer(const vector<CRgbaColor>& data)
{
    if (data.empty()) {
        m_SecondaryFormat = kSecondaryFormatNone;
        m_SecondaryCount = 0;
    }
    else {
        m_SecondaryFormat = kSecondaryFormatColorFloat;
        m_SecondaryCount = data.size();
        x_SetBufferData(1, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::GetColorBuffer(vector<CRgbaColor>& data) const
{
    data.clear();

    if (m_SecondaryFormat == kSecondaryFormatColorFloat) {
        data.resize(m_VertexCount);
        x_GetBufferData(1, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::SetColorBufferUC(const vector<CVect4<unsigned char> >& data) 
{
    if (data.empty()) {
        m_SecondaryFormat = kSecondaryFormatNone;
        m_SecondaryCount = 0;
    }
    else {
        m_SecondaryFormat = kSecondaryFormatColorUChar;
        m_SecondaryCount = data.size();
        x_SetBufferData(1, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::GetColorBufferUC(vector<CVect4<unsigned char> >& data) const
{
    data.clear();

    if (m_SecondaryFormat == kSecondaryFormatColorUChar) {
        data.resize(m_VertexCount);
        x_GetBufferData(1, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::SetTexCoordBuffer1D(const vector<float>& data)
{
    if (data.empty()) {
        m_SecondaryFormat = kSecondaryFormatNone;
        m_SecondaryCount = 0;
    }
    else {
        m_SecondaryFormat = kSecondaryFormatTexture1D;
        m_SecondaryCount = data.size();
        x_SetBufferData(1, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::GetTexCoordBuffer1D(vector<float>& data) const
{
    data.clear();

    if (m_SecondaryFormat == kSecondaryFormatTexture1D) {
        data.resize(m_VertexCount);
        x_GetBufferData(1, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::SetTexCoordBuffer(const vector<CVect2<float> >& data)
{
    if (data.empty()) {
        m_SecondaryFormat = kSecondaryFormatNone;
        m_SecondaryCount = 0;
    }
    else {
        m_SecondaryFormat = kSecondaryFormatTexture2D;
        m_SecondaryCount = data.size();
        x_SetBufferData(1, &data[0], data.size() * sizeof(data[0]));
    }
}

void CVboGeomBase::GetTexCoordBuffer(vector<CVect2<float> >& data) const
{
    data.clear();

    if (m_SecondaryFormat == kSecondaryFormatTexture2D) {
        data.resize(m_VertexCount);
        x_GetBufferData(1, &data[0], data.size() * sizeof(data[0]));
    }
}

///
/// CNarrowTreeEdgeVboGeomBase
///

void CNarrowTreeEdgeVboGeomBase::x_SetDrawMode(GLenum)
{
    _ASSERT(false);
}

size_t CNarrowTreeEdgeVboGeomBase::GetVertexCount() const
{
    return m_EdgeGeom ? m_EdgeGeom->GetVertexCount() : 0;
}

void CNarrowTreeEdgeVboGeomBase::SetVertexBuffer2D(const vector<CVect2<float> >& data)
{
    _ASSERT(false);
}

void CNarrowTreeEdgeVboGeomBase::GetVertexBuffer2D(vector<CVect2<float> >& data) const
{
    if (m_EdgeGeom)
        m_EdgeGeom->GetVertexBuffer2D(data);
    else
        data.clear();
}

void CNarrowTreeEdgeVboGeomBase::SetVertexBuffer3D(const vector<CVect3<float> >& data)
{
    _ASSERT(false);
}

void CNarrowTreeEdgeVboGeomBase::GetVertexBuffer3D(vector<CVect3<float> >& data) const
{
    data.clear();
    _ASSERT(false);
}

void CNarrowTreeEdgeVboGeomBase::SetColorBuffer(const vector<CRgbaColor>& data)
{
    _ASSERT(false);
}

void CNarrowTreeEdgeVboGeomBase::GetColorBuffer(vector<CRgbaColor>& data) const
{
    data.clear();
    _ASSERT(false);
}

void CNarrowTreeEdgeVboGeomBase::SetColorBufferUC(const vector<CVect4<unsigned char> >& data)
{
    _ASSERT(false);
}

void CNarrowTreeEdgeVboGeomBase::GetColorBufferUC(vector<CVect4<unsigned char> >& data) const
{
    data.clear();
    _ASSERT(false);
}

void CNarrowTreeEdgeVboGeomBase::SetTexCoordBuffer1D(const vector<float>& data)
{
    _ASSERT(false);
}

void CNarrowTreeEdgeVboGeomBase::GetTexCoordBuffer1D(vector<float>& data) const
{
    data.clear();
    _ASSERT(false);
}

void CNarrowTreeEdgeVboGeomBase::SetTexCoordBuffer(const vector<CVect2<float> >& data)
{
    _ASSERT(false);
}

void CNarrowTreeEdgeVboGeomBase::GetTexCoordBuffer(vector<CVect2<float> >& data) const
{
    data.clear();
    _ASSERT(false);
}

///
/// CFillerPointVboGeomBase
///

void CFillerPointVboGeomBase::x_SetDrawMode(GLenum)
{
    _ASSERT(false);
}

size_t CFillerPointVboGeomBase::GetVertexCount() const
{
    return m_EdgeGeom ? m_EdgeGeom->GetVertexCount() / 4 : 0;
}

void CFillerPointVboGeomBase::SetVertexBuffer2D(const vector<CVect2<float> >& data)
{
    _ASSERT(false);
}

void CFillerPointVboGeomBase::GetVertexBuffer2D(vector<CVect2<float> >& data) const
{
    data.clear();

    if (m_EdgeGeom) {
        vector<CVect2<float> > tmp;
        m_EdgeGeom->GetVertexBuffer2D(tmp);
        for (int i = 2; i < tmp.size() - 1; i += 4)
            data.push_back(tmp[i]);
    }
}

void CFillerPointVboGeomBase::SetVertexBuffer3D(const vector<CVect3<float> >& data)
{
    _ASSERT(false);
}

void CFillerPointVboGeomBase::GetVertexBuffer3D(vector<CVect3<float> >& data) const
{
    data.clear();
    _ASSERT(false);
}

void CFillerPointVboGeomBase::SetColorBuffer(const vector<CRgbaColor>& data)
{
    _ASSERT(false);
}

void CFillerPointVboGeomBase::GetColorBuffer(vector<CRgbaColor>& data) const
{
    data.clear();
    _ASSERT(false);
}

void CFillerPointVboGeomBase::SetColorBufferUC(const vector<CVect4<unsigned char> >& data)
{
    _ASSERT(false);
}

void CFillerPointVboGeomBase::GetColorBufferUC(vector<CVect4<unsigned char> >& data) const
{
    data.clear();

    if (m_EdgeGeom) {
        vector<CVect4<unsigned char> > tmp;
        m_EdgeGeom->GetColorBufferUC(tmp);
        for (int i = 2; i < tmp.size() - 1; i += 4)
            data.push_back(tmp[i]);
    }
}

void CFillerPointVboGeomBase::SetTexCoordBuffer1D(const vector<float>& data)
{
    _ASSERT(false);
}

void CFillerPointVboGeomBase::GetTexCoordBuffer1D(vector<float>& data) const
{
    data.clear();
    _ASSERT(false);
}

void CFillerPointVboGeomBase::SetTexCoordBuffer(const vector<CVect2<float> >& data)
{
    _ASSERT(false);
}

void CFillerPointVboGeomBase::GetTexCoordBuffer(vector<CVect2<float> >& data) const
{
    data.clear();
    _ASSERT(false);
}

END_NCBI_SCOPE
