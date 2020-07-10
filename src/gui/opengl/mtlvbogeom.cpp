/*  $Id: mtlvbogeom.cpp 43596 2019-08-06 20:31:10Z katargir $
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

#include "mtlvbogeom.hpp"
#include <gui/opengl/glmodel2d.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/mtldata.hpp>
#include "mtl_shader_types.h"

BEGIN_NCBI_SCOPE


///
/// CMtlVboGeom
///

CMtlVboGeom::~CMtlVboGeom()
{
    if (m_VertexBuffer)
        [m_VertexBuffer release];

    if (m_SecondaryBuffer)
        [m_SecondaryBuffer release];
}

void CMtlVboGeom::x_SetBufferData(int index, const void* data, size_t size)
{
    if (index != 0 && index != 1) {
        _ASSERT(false);
        return;
    }

    id<MTLBuffer> buffer = nil;

    if (index == 0) {
        if (m_VertexBuffer && m_VertexBuffer.length < size) {
            [m_VertexBuffer release];
            m_VertexBuffer = nil;
        }
        if (!m_VertexBuffer)
            m_VertexBuffer = Metal().CreateBuffer(size);
        buffer = m_VertexBuffer;
    }
    else {
        if (m_SecondaryBuffer && m_SecondaryBuffer.length < size) {
            [m_SecondaryBuffer release];
            m_SecondaryBuffer = nil;
        }
        if (!m_SecondaryBuffer)
            m_SecondaryBuffer = Metal().CreateBuffer(size);
        buffer = m_SecondaryBuffer;
    }

    if (!buffer)
        return;

    memcpy(buffer.contents, data, size);
}

void CMtlVboGeom::x_GetBufferData(int index, void* data, size_t size) const
{
    if (index != 0 && index != 1) {
        _ASSERT(false);
        return;
    }

    id<MTLBuffer> buffer = (index == 0) ? m_VertexBuffer : m_SecondaryBuffer;
    if (!buffer)
        return;

    if (buffer.length < size)
        return;

    memcpy(data, buffer.contents, size);
}

static void s_GetMatrices(const float* modelView, simd::float4x4& projectionMatrix, simd::float4x4& mv)
{
    IRender& gl = GetGl();

    CMatrix4<float> pM = gl.GetProjectionMatrix();
    const float* p = pM.GetData();

    projectionMatrix.columns[0] = { p[0], p[4], p[8],  p[12] };
    projectionMatrix.columns[1] = { p[1], p[5], p[9],  p[13] };
    projectionMatrix.columns[2] = { p[2], p[6], p[10], p[14] };
    projectionMatrix.columns[3] = { p[3], p[7], p[11], p[15] };

    mv.columns[0] = { modelView[0],  modelView[1],  modelView[2],  modelView[3] };
    mv.columns[1] = { modelView[4],  modelView[5],  modelView[6],  modelView[7] };
    mv.columns[2] = { modelView[8],  modelView[9],  modelView[10], modelView[11] };
    mv.columns[3] = { modelView[12], modelView[13], modelView[14], modelView[15] };

    GLint viewport[4];
    gl.GetViewport(viewport);
    
    simd::float4x4 win;
    win.columns[0] = { viewport[2]*.5f, 0, 0, 0 };
    win.columns[1] = { 0, viewport[3]*.5f, 0, 0 };
    win.columns[2] = { 0, 0, 1, 0 };
    win.columns[3] = { viewport[2]*.5f + viewport[0], viewport[3]*.5f + viewport[1], 0, 1 };
    mv = win*projectionMatrix*mv;
    
    projectionMatrix.columns[0] = { 2.f/viewport[2], 0, 0, 0 };
    projectionMatrix.columns[1] = { 0, 2.f/viewport[3], 0, 0 };
    projectionMatrix.columns[2] = { 0, 0, 1, 0 };
    projectionMatrix.columns[3] = { - 2.f*viewport[0]/viewport[2] - 1, -2.f*viewport[1]/viewport[3] - 1, 0, 1 };
}

void CMtlVboGeom::Render(const float* modelView)
{
    if (m_DrawMode == GL_LINES) {
        _ASSERT(false);
        return;
    }

    id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
    if (!renderEncoder) return;

    if (m_VertexFormat != kVertexFormatVertex2D)
        return;

    if (m_SecondaryFormat != kSecondaryFormatNone)
        _ASSERT(m_VertexCount == m_SecondaryCount);

    if (!m_VertexBuffer)
        return;

    IRender& gl = GetGl();

    int pipeLineState = 4;

    [renderEncoder setVertexBuffer:m_VertexBuffer offset:0 atIndex:1];

    if (m_SecondaryFormat == kSecondaryFormatColorFloat) {
        pipeLineState = 5;
        [renderEncoder setVertexBuffer:m_SecondaryBuffer offset:0 atIndex:2];
    } else if (m_SecondaryFormat == kSecondaryFormatColorUChar) {
        pipeLineState = 6;
        [renderEncoder setVertexBuffer:m_SecondaryBuffer offset:0 atIndex:2];
    } else if (m_SecondaryFormat == kSecondaryFormatTexture2D) {
        pipeLineState = 2;
        [renderEncoder setVertexBuffer:m_SecondaryBuffer offset:0 atIndex:2];
    } else if (m_SecondaryFormat == kSecondaryFormatTexture1D) {
        pipeLineState = 7;
        [renderEncoder setVertexBuffer:m_SecondaryBuffer offset:0 atIndex:2];
    }
    
    simd::float4x4 projectionMatrix, mv;
    s_GetMatrices(modelView, projectionMatrix, mv);

    if (pipeLineState == 0 || pipeLineState == 4) {
        UniformsPlain uniforms;
        uniforms.transfMatrix = projectionMatrix * mv;
        CRgbaColor c = gl.GetState()->GetColor();
        uniforms.color = (simd::float4){ c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha() };
        uniforms.pointSize = Metal().GetPointSize();
        uniforms.textureEnvMode = Metal().GetTextureModulate() ? 1 : 0;

        [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:0];
    }
    else if (pipeLineState == 1 || pipeLineState == 5 || pipeLineState == 6) {
        UniformsColor uniforms;
        uniforms.transfMatrix = projectionMatrix * mv;
        uniforms.pointSize = Metal().GetPointSize();

        [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:0];
    }
    else if (pipeLineState == 2 || pipeLineState == 7) {
        UniformsPlain uniforms;
        uniforms.transfMatrix = projectionMatrix * mv;
        CRgbaColor c = gl.GetState()->GetColor();
        uniforms.color = (simd::float4){ c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha() };
        uniforms.pointSize = Metal().GetPointSize();
        uniforms.textureEnvMode = Metal().GetTextureModulate() ? 1 : 0;

        [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:0];

        id<MTLSamplerState> sampler = Metal().GetSampler();
        [renderEncoder setFragmentSamplerState:sampler atIndex:0];
        [sampler release];
    }

    [renderEncoder setRenderPipelineState:Metal().GetPipelineState(pipeLineState)];

    MTLPrimitiveType primitiveType;
    switch(m_DrawMode) {
    case GL_POINTS:
        primitiveType = MTLPrimitiveTypePoint;
        break;
    case GL_LINES:
        primitiveType = MTLPrimitiveTypeLine;
        break;
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
        primitiveType = MTLPrimitiveTypeLineStrip;
        break;
    case GL_TRIANGLES:
        primitiveType = MTLPrimitiveTypeTriangle;
        break;
    case GL_TRIANGLE_FAN:
        return;
    case GL_TRIANGLE_STRIP:
        primitiveType = MTLPrimitiveTypeTriangleStrip;
        break;
    default:
       return;
    }

    [renderEncoder drawPrimitives:primitiveType vertexStart:0 vertexCount:m_VertexCount];
}

///
/// CMtlNarrowTreeEdgeVboGeom
///

void CMtlNarrowTreeEdgeVboGeom::Render(const float* modelView)
{
    CMtlVboGeomLines* sharedGeom = dynamic_cast<CMtlVboGeomLines*>(m_EdgeGeom.GetNCPointerOrNull());
    if (!sharedGeom)
        return;

    if (sharedGeom->m_Vertices.empty())
        return;

    IRender& gl = GetGl();

    simd::float4x4 projection, mv;

    CMatrix4<float> pM = gl.GetProjectionMatrix();
    const float* p = pM.GetData();

    projection.columns[0] = { p[0], p[4], p[8],  p[12] };
    projection.columns[1] = { p[1], p[5], p[9],  p[13] };
    projection.columns[2] = { p[2], p[6], p[10], p[14] };
    projection.columns[3] = { p[3], p[7], p[11], p[15] };

    mv.columns[0] = { modelView[0],  modelView[1],  modelView[2],  modelView[3] };
    mv.columns[1] = { modelView[4],  modelView[5],  modelView[6],  modelView[7] };
    mv.columns[2] = { modelView[8],  modelView[9],  modelView[10], modelView[11] };
    mv.columns[3] = { modelView[12], modelView[13], modelView[14], modelView[15] };

    GLint viewport[4];
    gl.GetViewport(viewport);

    auto v = [sharedGeom](size_t index) -> simd::float2
        { return { sharedGeom->m_Vertices[index].X(), sharedGeom->m_Vertices[index].Y() }; };

    CVect4<unsigned char> cUC = gl.GetState()->GetColor().GetRgbaUC();
    simd::uchar4 color = { cUC.X(), cUC.Y(), cUC.Z(), cUC.W() };
    auto c = [color](size_t) -> simd::uchar4 { return color; };
    Metal().RenderLines(viewport, projection*mv, sharedGeom->m_Vertices.size(), v, c);
}

///
/// CMtlFillerPointVboGeom
///

void CMtlFillerPointVboGeom::Render(const float* modelView)
{
    CMtlVboGeomLines* sharedGeom = dynamic_cast<CMtlVboGeomLines*>(m_EdgeGeom.GetNCPointerOrNull());
    if (!sharedGeom)
        return;

    const vector<CVect2<float> >& vertices = sharedGeom->m_Vertices;
    const vector<CVect4<unsigned char> >& colors = sharedGeom->m_Colors;

    if (vertices.empty())
        return;

    if (vertices.size() != colors.size()) {
        _ASSERT(false);
        return;
    }

    id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
    if (!renderEncoder)
        return;

    simd::float4x4 projectionMatrix, mv;
    s_GetMatrices(modelView, projectionMatrix, mv);

    UniformsColor uniforms;
    uniforms.transfMatrix = projectionMatrix * mv;
    uniforms.pointSize = Metal().GetPointSize();

    [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:0];

    [renderEncoder setRenderPipelineState:Metal().GetPipelineState(6)];

    constexpr size_t vblock = 4096/sizeof(float)/2, cblock = 4096/sizeof(unsigned char)/4;
    constexpr size_t block = vblock < cblock ? vblock : cblock;
    float vb[block*2];
    unsigned char cb[block*4];

    size_t count = 0;
    float* pv = &vb[0];
    unsigned char* pc = &cb[0];

    for (size_t i = 2; i < vertices.size(); ++count, i += 4) {
        if (count == block) {
            [renderEncoder setVertexBytes:&vb[0] length:sizeof(float)*2*count atIndex:1];
            [renderEncoder setVertexBytes:&cb[0] length:sizeof(unsigned char)*4*count atIndex:2];
            [renderEncoder drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:count];
            pv = &vb[0];
            pc = &cb[0];
            count = 0;
        }

        *pv++ = vertices[i][0];
        *pv++ = vertices[i][1];

        *pc++ = colors[i][0];
        *pc++ = colors[i][1];
        *pc++ = colors[i][2];
        *pc++ = colors[i][3];
    }

    if (count) {
        [renderEncoder setVertexBytes:&vb[0] length:sizeof(float)*2*count atIndex:1];
        [renderEncoder setVertexBytes:&cb[0] length:sizeof(unsigned char)*4*count atIndex:2];
        [renderEncoder drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:count];
    }
}

///
/// CMtlVboGeomLines
///

void CMtlVboGeomLines::Render(const float* modelView)
{
    if (m_Vertices.empty())
        return;

    if (!m_Colors.empty())
        _ASSERT(m_Vertices.size() == m_Colors.size());

    simd::float4x4 projection, mv;

    IRender& gl = GetGl();

    CMatrix4<float> pM = gl.GetProjectionMatrix();
    const float* p = pM.GetData();

    projection.columns[0] = { p[0], p[4], p[8],  p[12] };
    projection.columns[1] = { p[1], p[5], p[9],  p[13] };
    projection.columns[2] = { p[2], p[6], p[10], p[14] };
    projection.columns[3] = { p[3], p[7], p[11], p[15] };

    mv.columns[0] = { modelView[0],  modelView[1],  modelView[2],  modelView[3] };
    mv.columns[1] = { modelView[4],  modelView[5],  modelView[6],  modelView[7] };
    mv.columns[2] = { modelView[8],  modelView[9],  modelView[10], modelView[11] };
    mv.columns[3] = { modelView[12], modelView[13], modelView[14], modelView[15] };

    GLint viewport[4];
    gl.GetViewport(viewport);

    auto v = [this](size_t index) -> simd::float2 {
        return  { m_Vertices[index].X(), m_Vertices[index].Y() };
    };

    if (!m_Colors.empty()) {
        auto c = [this](size_t index) -> simd::uchar4 {
            return { m_Colors[index].X(),
                     m_Colors[index].Y(),
                     m_Colors[index].Z(),
                     m_Colors[index].W() };
        };

        Metal().RenderLines(viewport, projection*mv, m_Vertices.size(), v, c);
    } else {
        CVect4<unsigned char> cUC = gl.GetState()->GetColor().GetRgbaUC();
        simd::uchar4 color = { cUC.X(), cUC.Y(), cUC.Z(), cUC.W() };
        auto c = [color](size_t) -> simd::uchar4 { return color; };
        Metal().RenderLines(viewport, projection*mv, m_Vertices.size(), v, c);
    }
}

END_NCBI_SCOPE

#endif // NCBI_OS_DARWIN
