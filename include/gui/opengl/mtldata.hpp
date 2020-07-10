#ifndef GUI_OPENGL___MTL_DATA__HPP
#define GUI_OPENGL___MTL_DATA__HPP

/*  $Id: mtldata.hpp 43570 2019-08-01 15:39:31Z katargir $
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

#include <gui/opengl.h>
#include <gui/gui.hpp>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>

#include <simd/simd.h>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class NCBI_GUIOPENGL_EXPORT CMtlData
{
private:
    CMtlData() {}

public:
    static CMtlData& GetInstance();

    enum EPipelineState {
        kPipelineStateLine,
        kPipelineStateLineSmooth,
        kPipelineStateLineStipple,
        kPipelineStateTreeNode,
        kPipelineStateStippledPolygon
    };

 
    id<MTLDevice> GetMTLDevice();
    id<MTLCommandQueue> GetCommandQueue() { return m_CommandQueue; }

    id<MTLBuffer> CreateBuffer(int length);

    void SetEncoder(id<MTLRenderCommandEncoder> encoder);
    id<MTLRenderCommandEncoder> GetEncoder();

    void SetClientSize(simd::uint2 clientSize);
    simd::uint2 GetClientSize();

    bool InitPipelineStates(MTLPixelFormat colorPixelFormat);
    void DeletePipelineStates();
    id<MTLRenderPipelineState> GetPipelineState(size_t index);
    id<MTLRenderPipelineState> GetNamedPipelineState(EPipelineState state);

    id<MTLSamplerState> GetSampler();
    void TexParameter(GLenum pname, GLint param);
    void EnableBlending(bool enable) { m_BlendingEnabled = enable; }
    void BlendingState(int state);
    void EnableDepthTest(bool enable);
    void EnableTextureModulate(bool enable) { m_TextureModulate = enable; }
    bool GetTextureModulate() { return m_TextureModulate; }
    void EnableTextureAlpha(bool enable) { m_TextureAlpha = enable; }

    void  SetPointSize(float pointSize) { m_PointSize = pointSize; }
    float GetPointSize() { return m_PointSize; }

    void  SetLineWidth(float lineWidth) { m_LineWidth = lineWidth; }
    float GetLineWidth() { return m_LineWidth; }

    void  SetLineSmooth(bool lineSmooth) { m_LineSmooth = lineSmooth; }
    bool  GetLineSmooth() const { return m_LineSmooth; }

    void EnableLineStipple(bool enable) { m_EnableLineStipple = enable; }
    void SetLineStipple(short factor, short pattern)
    { 
        m_LineStippleiFactor = min<short>(256, max<short>(1, factor));
        m_LineStipplePattern = pattern;
    }

    void RenderLines(
            GLint viewport[4],
            const simd::float4x4& matrix,
            size_t vertexCount,
            const std::function<simd::float2(size_t index)>& coords,
            const std::function<simd::uchar4(size_t index)>& colors);

private:
    void x_FunctionNotFound(NSString* function);
    void x_FailedToCreatePipelineState(NSError* error);

    id<MTLDevice> m_Device = nil;

    // The command Queue from which we'll obtain command buffers
    id<MTLCommandQueue> m_CommandQueue = nil;

    id<MTLRenderCommandEncoder> m_Encoder = nil;
    simd::uint2 m_ClientSize = {0, 0};

    float m_PointSize = 1.0f;
    float m_LineWidth = 1.0f;
    bool  m_LineSmooth = false;

    bool  m_EnableLineStipple = false;
    short m_LineStippleiFactor = 1;
    short m_LineStipplePattern = 0xFFFF;

    vector<id<MTLRenderPipelineState> > m_pipelineStates;
    vector<id<MTLRenderPipelineState> > m_pipelineStatesBlend;
    vector<id<MTLRenderPipelineState> > m_pipelineStatesBlendDest;
    map<EPipelineState, id<MTLRenderPipelineState> > m_NamedPipelineStates;
    bool m_BlendingEnabled = false;
    int m_BlendingState = 0;
    bool m_TextureModulate = false;
    bool m_TextureAlpha = false;
    MTLSamplerDescriptor* m_SamplerDescriptor = nil;
    id<MTLDepthStencilState> m_EnableDepthTest = nil;
    id<MTLDepthStencilState> m_DisableDepthTest = nil;
};

CMtlData& Metal();

END_NCBI_SCOPE

/* @} */

#endif // GUI_OPENGL___MTL_DATA__HPP
