/*  $Id: mtltreenodevbo.cpp 43157 2019-05-22 17:37:24Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>
#include <ncbiconf.h>

#ifdef NCBI_OS_DARWIN

#include "mtltreenodevbo.hpp"

#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/mtldata.hpp>
#include "mtl_shader_types.h"

BEGIN_NCBI_SCOPE

///
/// CMtlTreeNodeVbo
///

CMtlTreeNodeVbo::CMtlTreeNodeVbo(size_t numSegments) : CTreeNodeVboBase(numSegments)
{
    if (numSegments < 4)
        return;

    vector <CVect2<float> > vertices;
    GetVertexBuffer2D(vertices);

    m_ColorBuffer.resize(vertices.size(), CRgbaColor(0, 0, 0, 0));
    m_ColorBuffer[0].SetAlpha(1.0f);

    m_VertexBuffer = Metal().CreateBuffer(sizeof(vertices[0])*vertices.size());
    if (!m_VertexBuffer)
        return;
    memcpy(m_VertexBuffer.contents, &vertices[0], sizeof(vertices[0])*vertices.size());

    m_SecondaryBuffer = Metal().CreateBuffer(sizeof(m_ColorBuffer[0])*m_ColorBuffer.size());
    if (!m_SecondaryBuffer)
        return;
    memcpy(m_SecondaryBuffer.contents, &m_ColorBuffer[0], sizeof(m_ColorBuffer[0])*m_ColorBuffer.size());

    int count = 3*m_NumSegments;
    m_IndexBuffer = Metal().CreateBuffer(sizeof(unsigned short)*count);
    if (!m_IndexBuffer)
        return;

    vector<unsigned short> index;
    index.reserve(count);

    for (size_t i = 0; i < m_NumSegments; ++i) {
        index.push_back(0);
        index.push_back(i + 1);
        index.push_back(i + 2);
    }
    memcpy(m_IndexBuffer.contents, &index[0], sizeof(index[0])*index.size());
}

CMtlTreeNodeVbo::~CMtlTreeNodeVbo()
{
    if (m_VertexBuffer)
        [m_VertexBuffer release];

    if (m_SecondaryBuffer)
        [m_SecondaryBuffer release];

    if (m_IndexBuffer)
        [m_IndexBuffer release];
}

void CMtlTreeNodeVbo::Render(const float* modelView)
{
    id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
    if (!renderEncoder) return;

    if (!m_VertexBuffer || !m_SecondaryBuffer || !m_IndexBuffer)
        return;

    [renderEncoder setVertexBuffer:m_VertexBuffer offset:0 atIndex:1];
    [renderEncoder setVertexBuffer:m_SecondaryBuffer offset:0 atIndex:2];

    IRender& gl = GetGl();

    CMatrix4<float> pM = gl.GetProjectionMatrix();
    const float* p = pM.GetData();

    simd::float4x4 projectionMatrix, mv;

    projectionMatrix.columns[0] = { p[0], p[4], p[8],  p[12] };
    projectionMatrix.columns[1] = { p[1], p[5], p[9],  p[13] };
    projectionMatrix.columns[2] = { p[2], p[6], p[10], p[14] };
    projectionMatrix.columns[3] = { p[3], p[7], p[11], p[15] };

    mv.columns[0] = { modelView[0],  modelView[1],  modelView[2],  modelView[3] };
    mv.columns[1] = { modelView[4],  modelView[5],  modelView[6],  modelView[7] };
    mv.columns[2] = { modelView[8],  modelView[9],  modelView[10], modelView[11] };
    mv.columns[3] = { modelView[12], modelView[13], modelView[14], modelView[15] };

    UniformsPlain uniforms;
    uniforms.transfMatrix = projectionMatrix * mv;
    CRgbaColor c = gl.GetState()->GetColor();
    uniforms.color = (simd::float4){ c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha() };
    uniforms.pointSize = Metal().GetPointSize();
    uniforms.textureEnvMode = Metal().GetTextureModulate() ? 1 : 0;

    [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:0];

    [renderEncoder setRenderPipelineState:Metal().GetNamedPipelineState(CMtlData::kPipelineStateTreeNode)];

    [renderEncoder
        drawIndexedPrimitives: MTLPrimitiveTypeTriangle
            indexCount:  3*m_NumSegments
            indexType:   MTLIndexTypeUInt16
            indexBuffer: m_IndexBuffer
            indexBufferOffset: 0
            instanceCount: 1
            baseVertex:    0
            baseInstance:  0];
}

END_NCBI_SCOPE

#endif // NCBI_OS_DARWIN
