
/*  $Id: mtlrender.cpp 44565 2020-01-22 18:26:02Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <ncbiconf.h>

#ifdef NCBI_OS_DARWIN

#include "mtlrender.hpp"
#include "mtl_shader_types.h"

#include <gui/opengl/mtldata.hpp>

BEGIN_NCBI_SCOPE

CMtlRender::CMtlRender()
{
    m_ProjectionMatrix.push_front(simd::double4x4(matrix_identity_double4x4));
    m_ModelViewMatrix.push_front(simd::double4x4(matrix_identity_double4x4));
    memset(m_PolygonStipple, 0xFF, 128);
}

CMtlRender::~CMtlRender()
{
}

static void s_Double2Float4x4(const simd::double4x4& src, simd::float4x4& dst)
{
    const simd::double4 (&s)[4] = src.columns;
    simd::float4 (&d)[4] = dst.columns;

    d[0][0] = (float)s[0][0];
    d[0][1] = (float)s[0][1];
    d[0][2] = (float)s[0][2];
    d[0][3] = (float)s[0][3];

    d[1][0] = (float)s[1][0];
    d[1][1] = (float)s[1][1];
    d[1][2] = (float)s[1][2];
    d[1][3] = (float)s[1][3];

    d[2][0] = (float)s[2][0];
    d[2][1] = (float)s[2][1];
    d[2][2] = (float)s[2][2];
    d[2][3] = (float)s[2][3];

    d[3][0] = (float)s[3][0];
    d[3][1] = (float)s[3][1];
    d[3][2] = (float)s[3][2];
    d[3][3] = (float)s[3][3];
}

void CMtlRender::End()
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

    if (m_CurrentMode == GL_LINE_LOOP) {
        size_t v0 = 0;

        if (!m_ColorBuffer.empty()) {
            x_SyncAttribs(m_ColorBuffer);
            m_ColorBuffer.push_back(m_ColorBuffer[v0]);
        }

        if (!m_TexCoord2DBuffer1.empty()) {
            x_SyncAttribs(m_TexCoord2DBuffer1);
            m_TexCoord2DBuffer1.push_back(m_TexCoord2DBuffer1[v0]);
        }

        m_VertexBuffer.push_back(m_VertexBuffer[v0]);
    }

    // All attribute vectors that are in use must be the same 
    // size as the vertex buffer
    if (m_ColorBuffer.size() > 0 && m_ColorBuffer.size() < vert_size)
        x_SyncAttribs(m_ColorBuffer);

    if (m_TexCoord2DBuffer1.size() > 0 && m_TexCoord2DBuffer1.size() < vert_size)
        x_SyncAttribs(m_TexCoord2DBuffer1);

    if (m_CurrentMode == GL_LINES) {
        simd::float4x4 projection, mv;
        s_Double2Float4x4(m_ProjectionMatrix.front(), projection);
        s_Double2Float4x4(m_ModelViewMatrix.front(), mv);

        GLint viewport[4];
        GetViewport(viewport);

        auto v = [this](size_t index) -> simd::float2 {
            return { m_VertexBuffer[index].X(), m_VertexBuffer[index].Y() };
        };

        if (!m_ColorBuffer.empty()) {
            auto c = [this](size_t index) -> simd::uchar4 {
                CVect4<unsigned char> cUC = m_ColorBuffer[index].GetRgbaUC();
                return { cUC.X(), cUC.Y(), cUC.Z(), cUC.W() };
            };
            Metal().RenderLines(viewport, projection*mv, m_VertexBuffer.size(), v, c);
        } else {
            CVect4<unsigned char> cUC = GetState()->GetColor().GetRgbaUC();
            simd::uchar4 color = { cUC.X(), cUC.Y(), cUC.Z(), cUC.W() };
            auto c = [color](size_t) -> simd::uchar4 { return color; };
            Metal().RenderLines(viewport, projection*mv, m_VertexBuffer.size(), v, c);
        }
    }
    else
        x_RenderBuffers();

    m_CurrentMode = GL_NONE;
}

void CMtlRender::Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    if (m_CurrentMode == GL_TRIANGLE_FAN && m_VertexBuffer.size() >= 3) {
        size_t v0 = 0;
        size_t vp = m_VertexBuffer.size() - 1;

        if (!m_ColorBuffer.empty()) {
            x_SyncAttribs(m_ColorBuffer);
            m_ColorBuffer.push_back(m_ColorBuffer[v0]);
            m_ColorBuffer.push_back(m_ColorBuffer[vp]);
        }

        if (!m_TexCoord2DBuffer1.empty()) {
            x_SyncAttribs(m_TexCoord2DBuffer1);
            m_TexCoord2DBuffer1.push_back(m_TexCoord2DBuffer1[v0]);
            m_TexCoord2DBuffer1.push_back(m_TexCoord2DBuffer1[vp]);
        }

        m_VertexBuffer.push_back(m_VertexBuffer[v0]);
        m_VertexBuffer.push_back(m_VertexBuffer[vp]);
        m_VertexBuffer.push_back(CVect3<float>(x, y, z));
    }
    else
        CRenderCommon::Vertex3f(x, y, z);
}

void CMtlRender::x_GetScreenMatrices(simd::float4x4& projection, simd::float4x4& modelView)
{
    s_Double2Float4x4(m_ProjectionMatrix.front(), projection);
    s_Double2Float4x4(m_ModelViewMatrix.front(), modelView);

    GLint viewport[4];
    GetViewport(viewport);

    simd::float4x4 win;
    win.columns[0] = { viewport[2]*.5f, 0, 0, 0 };
    win.columns[1] = { 0, viewport[3]*.5f, 0, 0 };
    win.columns[2] = { 0, 0, 1, 0 };
    win.columns[3] = { viewport[2]*.5f + viewport[0], viewport[3]*.5f + viewport[1], 0, 1 };
    modelView = win*projection*modelView;
    
    projection.columns[0] = { 2.f/viewport[2], 0, 0, 0 };
    projection.columns[1] = { 0, 2.f/viewport[3], 0, 0 };
    projection.columns[2] = { 0, 0, 1, 0 };
    projection.columns[3] = { - 2.f*viewport[0]/viewport[2] - 1, -2.f*viewport[1]/viewport[3] - 1, 0, 1 };
}

void CMtlRender::x_RenderBuffers()
{
    id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
    if (!renderEncoder) return;

    MTLPrimitiveType primitiveType;
    switch(m_CurrentMode) {
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
    case GL_QUADS:
    case GL_TRIANGLES:
    case GL_TRIANGLE_FAN:
        if (m_PolygonStippleEnabled) {
            x_RenderStippledPolygon();
            return;
        }
        primitiveType = MTLPrimitiveTypeTriangle;
        break;
    case GL_TRIANGLE_STRIP:
        if (m_PolygonStippleEnabled) {
            x_RenderStippledPolygon();
            return;
        }
        primitiveType = MTLPrimitiveTypeTriangleStrip;
        break;
    default:
       return;
    }

    int elemSize = m_ColorBuffer.empty() ? sizeof(CVect3<float>) : sizeof(CRgbaColor);
    size_t groupSize = 1;

    switch(primitiveType) {
    case MTLPrimitiveTypeLine:
        groupSize = 2;
        break;
    case MTLPrimitiveTypeTriangle:
        groupSize = 3;
        break;
    case MTLPrimitiveTypeTriangleStrip:
        groupSize = 4;
        break;
    default:
        break;
    }

    try {
        size_t groupCount = 4096/(groupSize*elemSize);
        size_t blockSize = groupCount*groupSize;

        int pipeLineState = 0;

        if (!m_TexCoord2DBuffer1.empty())
            pipeLineState = 2;
        if (!m_ColorBuffer.empty())
            pipeLineState = 1;

        simd::float4x4 projectionMatrix;
        s_Double2Float4x4(m_ProjectionMatrix.front(), projectionMatrix);

        simd::float4x4 mv;
        s_Double2Float4x4(m_ModelViewMatrix.front(), mv);

        if (pipeLineState == 0) {
            UniformsPlain uniforms;
            uniforms.transfMatrix = projectionMatrix * mv;
            CRgbaColor c = m_State->GetColor();
            uniforms.color = (simd::float4){ c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha() };
            uniforms.pointSize = Metal().GetPointSize();
            uniforms.textureEnvMode = Metal().GetTextureModulate() ? 1 : 0;

            [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:0];
        }
        else if (pipeLineState == 1) {
            UniformsColor uniforms;
            uniforms.transfMatrix = projectionMatrix * mv;
            uniforms.pointSize = Metal().GetPointSize();

            [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:0];
        }
        else if (pipeLineState == 2) {
            UniformsPlain uniforms;
            uniforms.transfMatrix = projectionMatrix * mv;
            CRgbaColor c = m_State->GetColor();
            uniforms.color = (simd::float4){ c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha() };
            uniforms.pointSize = Metal().GetPointSize();
            uniforms.textureEnvMode = Metal().GetTextureModulate() ? 1 : 0;

            [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:0];

            id<MTLSamplerState> sampler = Metal().GetSampler();
            [renderEncoder setFragmentSamplerState:sampler atIndex:0];
            [sampler release];
        }

        [renderEncoder setRenderPipelineState:Metal().GetPipelineState(pipeLineState)];

        for (size_t i = 0;;) {
            size_t count = m_VertexBuffer.size() - i;
            if (count > blockSize)
                count = blockSize;

            [renderEncoder setVertexBytes:&m_VertexBuffer[i] length:sizeof(CVect3<float>)*count atIndex:1];

            if (pipeLineState == 1)
                [renderEncoder setVertexBytes:&m_ColorBuffer[i] length:sizeof(CRgbaColor)*count atIndex:2];
            else if (pipeLineState == 2)
                [renderEncoder setVertexBytes:&m_TexCoord2DBuffer1[i] length:sizeof(CVect2<float>)*count atIndex:2];

            [renderEncoder drawPrimitives:primitiveType vertexStart:0 vertexCount:count];

            i += count;
            if (i == m_VertexBuffer.size())
                break;

            if (primitiveType == MTLPrimitiveTypeLineStrip)
                i -= 1;
            else if (primitiveType == MTLPrimitiveTypeTriangleStrip)
                i -= 2;
        }
    }
    catch (const std::exception&) {}
}

void CMtlRender::x_RenderStippledPolygon()
{
    id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
    if (!renderEncoder) return;
    
    CRgbaColor c = m_State->GetColor();

    simd::float4x4 projection, mv;
    x_GetScreenMatrices(projection, mv);

    UniformsStippledPolygon uniforms;
    uniforms.mvMatrix = mv;
    uniforms.prMatrix = projection;

    [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:0];
    [renderEncoder setFragmentBytes:&m_PolygonStipple[0] length:sizeof(m_PolygonStipple) atIndex:0];
    [renderEncoder setRenderPipelineState:Metal().GetNamedPipelineState(CMtlData::kPipelineStateStippledPolygon)];
    [renderEncoder setTriangleFillMode:MTLTriangleFillModeFill];

    try {
        size_t groupSize = 3;
        MTLPrimitiveType primitiveType = MTLPrimitiveTypeTriangle;

        if (m_CurrentMode == GL_TRIANGLE_STRIP) {
            groupSize = 4;
            primitiveType = MTLPrimitiveTypeTriangleStrip;
        }
        
        size_t groupCount = 4096/(groupSize*sizeof(VertexInStippledPolygon));
        size_t blockSize = groupCount*groupSize;

        unique_ptr<VertexInStippledPolygon[]> b(new VertexInStippledPolygon[blockSize]);
        VertexInStippledPolygon* pv = &b[0];
        size_t group = 0;
        
        for (size_t i = 0; i < m_VertexBuffer.size(); ++i, ++group) {
            if (group == blockSize) {
                [renderEncoder setVertexBytes:&b[0] length:sizeof(VertexInStippledPolygon)*group atIndex:1];
                [renderEncoder drawPrimitives:primitiveType vertexStart:0 vertexCount:group];
                pv = &b[0];
                group = 0;
                
                if (primitiveType == MTLPrimitiveTypeTriangleStrip)
                    i -= 2;
            }
            
            pv->position = { m_VertexBuffer[i].X(), m_VertexBuffer[i].Y() };

            if (!m_ColorBuffer.empty())
                c = m_ColorBuffer[i];
            memcpy(&pv->color, c.GetColorArray(), sizeof(float)*4);
            ++pv;
        }

        if (group) {
            [renderEncoder setVertexBytes:&b[0] length:sizeof(VertexInStippledPolygon)*group atIndex:1];
            [renderEncoder drawPrimitives:primitiveType vertexStart:0 vertexCount:group];
        }
    }
    catch (const std::exception&) {}
}

void CMtlRender::Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    CRenderCommon::Viewport(x, y, width, height);

    id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
    if (!renderEncoder) return;

    simd::uint2 clientSize = Metal().GetClientSize();

    _ASSERT(y >= 0);
    _ASSERT(height > 0 && height <= clientSize.y - y);
    _ASSERT(x >= 0);
    _ASSERT(width > 0 && width <= clientSize.x - x);

    [renderEncoder setViewport:(MTLViewport){(double)x, (double)(clientSize.y - height - y), (double)width, (double)height, 0.0, 1.0}];
}

void CMtlRender::MatrixMode(GLenum mode)
{
    m_MatrixMode = mode;
}

void CMtlRender::PushMatrix()
{
    switch(m_MatrixMode) {
    case GL_PROJECTION :
        m_ProjectionMatrix.push_front(simd::double4x4(m_ProjectionMatrix.front()));
        break;
    case GL_MODELVIEW :
        m_ModelViewMatrix.push_front(simd::double4x4(m_ModelViewMatrix.front()));
        break;
    }
}

void CMtlRender::PopMatrix()
{
    switch(m_MatrixMode) {
    case GL_PROJECTION :
        if(m_ProjectionMatrix.size() > 1)
            m_ProjectionMatrix.pop_front();
        break;
    case GL_MODELVIEW :
        if(m_ModelViewMatrix.size() > 1)
            m_ModelViewMatrix.pop_front();
        break;
    }
}

void CMtlRender::LoadIdentity()
{
    x_LoadMatrix(matrix_identity_double4x4);
}

void CMtlRender::x_LoadMatrix(const simd::double4x4& m)
{
    switch(m_MatrixMode) {
    case GL_PROJECTION :
        m_ProjectionMatrix.front()= m;
        break;
    case GL_MODELVIEW :
        m_ModelViewMatrix.front() = m;
        break;
    }
}

void CMtlRender::x_ApplyTransf(const simd::double4x4& t)
{
    switch(m_MatrixMode) {
    case GL_PROJECTION :
        m_ProjectionMatrix.front() = m_ProjectionMatrix.front() * t;
        break;
    case GL_MODELVIEW :
        m_ModelViewMatrix.front() = m_ModelViewMatrix.front() * t;
        break;
    }
}

static void s_InitMatrixd(simd::double4x4& d, const GLdouble* s)
{
    d.columns[0][0] = s[0];
    d.columns[0][1] = s[1];
    d.columns[0][2] = s[2];
    d.columns[0][3] = s[3];
    d.columns[1][0] = s[4];
    d.columns[1][1] = s[5];
    d.columns[1][2] = s[6];
    d.columns[1][3] = s[7];
    d.columns[2][0] = s[8];
    d.columns[2][1] = s[9];
    d.columns[2][2] = s[10];
    d.columns[2][3] = s[11];
    d.columns[3][0] = s[12];
    d.columns[3][1] = s[13];
    d.columns[3][2] = s[14];
    d.columns[3][3] = s[15];
}

static void s_InitMatrixf(simd::double4x4& d, const GLfloat* s)
{
    d.columns[0][0] = s[0];
    d.columns[0][1] = s[1];
    d.columns[0][2] = s[2];
    d.columns[0][3] = s[3];
    d.columns[1][0] = s[4];
    d.columns[1][1] = s[5];
    d.columns[1][2] = s[6];
    d.columns[1][3] = s[7];
    d.columns[2][0] = s[8];
    d.columns[2][1] = s[9];
    d.columns[2][2] = s[10];
    d.columns[2][3] = s[11];
    d.columns[3][0] = s[12];
    d.columns[3][1] = s[13];
    d.columns[3][2] = s[14];
    d.columns[3][3] = s[15];
}

void CMtlRender::LoadMatrixf(const GLfloat *m)
{
    simd::double4x4 t;
    s_InitMatrixf(t, m);
    x_LoadMatrix(t);
}

void CMtlRender::LoadMatrixd(const GLdouble* m)
{
    simd::double4x4 t;
    s_InitMatrixd(t, m);
    x_LoadMatrix(t);
}

void CMtlRender::MultMatrixf(const GLfloat* m)
{
    simd::double4x4 t;
    s_InitMatrixf(t, m);
    x_ApplyTransf(t);
}

void CMtlRender::MultMatrixd(const GLdouble* m)
{
    simd::double4x4 t;
    s_InitMatrixd(t, m);
    x_ApplyTransf(t);
}

static CMatrix4<float> s_MatrixFromSimd(const simd::double4x4& d)
{
    const simd::double4 (&m)[4] = d.columns;
    return CMatrix4<float>(
            m[0][0], m[1][0], m[2][0], m[3][0],
            m[0][1], m[1][1], m[2][1], m[3][1],
            m[0][2], m[1][2], m[2][2], m[3][2],
            m[0][3], m[1][3], m[2][3], m[3][3]);
}

static void s_MatrixFromSimd(const simd::double4x4& d, GLdouble* res)
{
    const simd::double4 (&m)[4] = d.columns;
    res[0]  = m[0][0];
    res[1]  = m[0][1];
    res[2]  = m[0][2];
    res[3]  = m[0][3];
    res[4]  = m[1][0];
    res[5]  = m[1][1];
    res[6]  = m[1][2];
    res[7]  = m[1][3];
    res[8]  = m[2][0];
    res[9]  = m[2][1];
    res[10] = m[2][2];
    res[11] = m[2][3];
    res[12] = m[3][0];
    res[13] = m[3][1];
    res[14] = m[3][2];
    res[15] = m[3][3];
}

CMatrix4<float> CMtlRender::GetModelViewMatrix() const
{
    return s_MatrixFromSimd(m_ModelViewMatrix.front());
}

CMatrix4<float> CMtlRender::GetProjectionMatrix() const
{
    return s_MatrixFromSimd(m_ProjectionMatrix.front());
}

void CMtlRender::GetModelViewMatrix(GLdouble* m) const
{
    s_MatrixFromSimd(m_ModelViewMatrix.front(), m);
}

void CMtlRender::GetProjectionMatrix(GLdouble* m) const
{
    s_MatrixFromSimd(m_ProjectionMatrix.front(), m);
}

void CMtlRender::Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    double mod = sqrt(x*x + y*y + z*z);
    if (mod == 0)
        return;
    x /= mod; y /= mod; z /= mod;
    double r = angle*3.141592653589793238463/360.0;
    double c = cos(r), s = sin(r);

    //simd::quatd q = { s*x, s*y, s*z, c };
    //simd::double4x4 t(q);

    simd::double4 v = { s*x, s*y, s*z, c };
    simd::double4x4 t(
        (simd::double4){
            1 - 2*(v.y*v.y + v.z*v.z),
            2*(v.x*v.y + v.z*v.w),
            2*(v.x*v.z - v.y*v.w), 0 },
        (simd::double4){
            2*(v.x*v.y - v.z*v.w),
            1 - 2*(v.z*v.z + v.x*v.x),
            2*(v.y*v.z + v.x*v.w), 0 },
        (simd::double4){
            2*(v.z*v.x + v.y*v.w),
            2*(v.y*v.z - v.x*v.w),
            1 - 2*(v.y*v.y + v.x*v.x), 0 },
        (simd::double4){ 0, 0, 0, 1 });

    x_ApplyTransf(t);
}

void CMtlRender::Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    Rotated(angle, x, y, z);
}

void CMtlRender::Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    simd::double4x4 m = matrix_identity_double4x4;

    m.columns[0][0] = x;
    m.columns[1][1] = y;
    m.columns[2][2] = z;

    x_ApplyTransf(m);
}

void CMtlRender::Scalef(GLfloat x, GLfloat y, GLfloat z)
{
    Scaled(x, y, z);
}

void CMtlRender::Translated(GLdouble x, GLdouble y, GLdouble z)
{
    simd::double4x4 m = matrix_identity_double4x4;

    m.columns[3][0] = x;
    m.columns[3][1] = y;
    m.columns[3][2] = z;

    x_ApplyTransf(m);
}

void CMtlRender::Translatef(GLfloat x, GLfloat y, GLfloat z)
{
    Translated(x, y, z);
}

void CMtlRender::Ortho(GLdouble left, GLdouble right,
                       GLdouble bottom, GLdouble top,
                       GLdouble nearVal, GLdouble farVal)
{
    simd::double4x4 m = matrix_identity_double4x4;

    m.columns[0][0] = 2.0/(right - left);
    m.columns[1][1] = 2.0/(top - bottom);
    m.columns[2][2] = -2.0/(farVal - nearVal);

    m.columns[3][0] = -(right + left)/(right - left);
    m.columns[3][1] = -(top + bottom)/(top - bottom);
    m.columns[3][2] = -(farVal + nearVal)/(farVal - nearVal);

    x_ApplyTransf(m);
}
                       
void CMtlRender::Perspective(GLdouble fovy, GLdouble aspect, 
                       GLdouble zNear, GLdouble zFar)
{
}
                       
void CMtlRender::PushAttrib(GLbitfield mask)
{
}

void CMtlRender::PopAttrib()
{
}

void CMtlRender::Enable(GLenum glstate)
{
    if (glstate == GL_BLEND) {
        Metal().EnableBlending(true);
    }
    else if (glstate == GL_DEPTH_TEST) {
        Metal().EnableDepthTest(true);
    }
    else if(glstate == GL_POLYGON_STIPPLE) {
        m_PolygonStippleEnabled = true;
    }
    else if (glstate == GL_SCISSOR_TEST) {
/*
        id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
        if (!renderEncoder) return;

        MTLScissorRect rc = {
                             .x = static_cast<NSUInteger>(m_ScissorRectX),
                             .y = static_cast<NSUInteger>(m_ScissorRectY),
                             .width = static_cast<NSUInteger>(m_ScissorRectW),
                             .height = static_cast<NSUInteger>(m_ScissorRectH) };
        [renderEncoder setScissorRect:rc];
 */
    }
    else if (glstate == GL_LINE_STIPPLE) {
        Metal().EnableLineStipple(true);
    }
    else if (glstate == GL_LINE_SMOOTH) {
        Metal().SetLineSmooth(true);
    }
    else {
        //LOG_POST(Error << "CMtlRender::Enable(" << glstate << ") not implemented");
    }
}

void CMtlRender::Disable(GLenum glstate)
{
    if (glstate == GL_BLEND) {
        Metal().EnableBlending(false);
    }
    else if (glstate == GL_DEPTH_TEST) {
        Metal().EnableDepthTest(false);
    }
    else if(glstate == GL_POLYGON_STIPPLE) {
        m_PolygonStippleEnabled = false;
    }
    else if (glstate == GL_SCISSOR_TEST) {
        id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
        if (!renderEncoder) return;
        
        simd::uint2 clientSize = Metal().GetClientSize();
        MTLScissorRect rc = { 0, 0, clientSize.x, clientSize.y };
        [renderEncoder setScissorRect:rc];
    }
    else if (glstate == GL_LINE_STIPPLE) {
        Metal().EnableLineStipple(false);
    }
    else if (glstate == GL_LINE_SMOOTH) {
        Metal().SetLineSmooth(false);
    }
    else {
        //LOG_POST(Error << "CMtlRender::DIsable(" << glstate << ") not implemented");
    }
}

void CMtlRender::Hint(GLenum target, GLenum mode)
{
}

void CMtlRender::LineWidth(GLfloat w)
{
    Metal().SetLineWidth(w);
}

void CMtlRender::PointSize(GLfloat s)
{
    Metal().SetPointSize(s);
}

void CMtlRender::ShadeModel(GLenum mode)
{
}

void CMtlRender::Scissor(GLint x, GLint y,
                         GLsizei width, GLsizei height)
{
    id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
    if (!renderEncoder) return;

    simd::uint2 clientSize = Metal().GetClientSize();

    m_ScissorRectX = x;
    m_ScissorRectY = clientSize.y - height - y;
    m_ScissorRectW = width;
    m_ScissorRectH = height;
    MTLScissorRect rc = {
                         .x = static_cast<NSUInteger>(m_ScissorRectX),
                         .y = static_cast<NSUInteger>(m_ScissorRectY),
                         .width = static_cast<NSUInteger>(m_ScissorRectW),
                         .height = static_cast<NSUInteger>(m_ScissorRectH) };
    [renderEncoder setScissorRect:rc];
}
                         
void CMtlRender::ColorMask(GLboolean red, 
                   GLboolean green, 
                   GLboolean blue, 
                   GLboolean alpha)
{
}
                   
void CMtlRender::PolygonMode(GLenum face, GLenum mode)
{
    id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
    if (!renderEncoder) return;

    switch(mode) {
    case GL_FILL:
        [renderEncoder setTriangleFillMode:MTLTriangleFillModeFill];
        break;
    case GL_LINE:
        [renderEncoder setTriangleFillMode:MTLTriangleFillModeLines];
        break;
    default:
        return;
    }

    m_State->PolygonMode(face, mode);
}

void CMtlRender::LineStipple(GLint factor, GLushort pattern)
{
    Metal().SetLineStipple(factor, pattern);
}

void CMtlRender::PolygonStipple(GLubyte* mask)
{
    memcpy(m_PolygonStipple, mask, 128);
}

void CMtlRender::BlendFunc(GLenum sfactor, GLenum dfactor)
{
    if (sfactor == GL_SRC_ALPHA && dfactor == GL_ONE_MINUS_SRC_ALPHA) {
        Metal().BlendingState(0);
        return;
    }

    if (sfactor == GL_ONE_MINUS_DST_ALPHA && dfactor == GL_DST_ALPHA) {
        Metal().BlendingState(1);
        return;
    }

    LOG_POST(Error << "CMtlRender::BlendFunc(" << sfactor << ", " << dfactor << ") not implemented");
}

void CMtlRender::TexEnvi(GLenum target, GLenum pname, GLint param)
{
    if (target != GL_TEXTURE_ENV || pname != GL_TEXTURE_ENV_MODE) {
        LOG_POST(Error << "CMtlRender::TexEnvi(" << target << ", " << pname << ", " << param << ") not implemented");
        return;
    }
    Metal().EnableTextureModulate(param == GL_MODULATE);
}

void CMtlRender::BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
                                   GLenum srcAlpha, GLenum dstAlpha)
{
}
                                   
void CMtlRender::BlendColor(const CRgbaColor& c)
{
}

void CMtlRender::ScaleInvarient(bool b, CVect2<TModelUnit> scale)
{
}
        
void CMtlRender::ScaleFactor(const CVect2<TModelUnit>& scale)
{
}

void CMtlRender::LineJoinStyle(IGlState::ELineJoinStyle s)
{
}

void CMtlRender::LineCapStyle(IGlState::ELineCapStyle c)
{
}

void CMtlRender::PdfShadeStyle(IGlState::EPdfShadeStyle s)
{
}

END_NCBI_SCOPE

#endif // NCBI_OS_DARWIN



