
/*  $Id: mtldata.cpp 44586 2020-01-27 20:31:44Z katargir $
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

#include <gui/opengl/mtldata.hpp>

#include "mtl_shader_types.h"

BEGIN_NCBI_SCOPE

CMtlData& Metal()
{
    return CMtlData::GetInstance();
}

CMtlData& CMtlData::GetInstance()
{
    static CMtlData instance;
    return instance;
}

id<MTLDevice> CMtlData::GetMTLDevice()
{
    if (m_Device == nil)
        m_Device = MTLCreateSystemDefaultDevice();
    return m_Device;
}

id<MTLBuffer> CMtlData::CreateBuffer(int length)
{
    if (!m_Device || length <= 0)
        return nil;

    return [m_Device newBufferWithLength:length options:MTLResourceStorageModeShared];
}

void CMtlData::SetEncoder(id<MTLRenderCommandEncoder> encoder)
{
    m_Encoder = encoder;
}

id<MTLRenderCommandEncoder> CMtlData::GetEncoder()
{
    return m_Encoder;
}

void CMtlData::SetClientSize(simd::uint2 clientSize)
{
    m_ClientSize = clientSize;
}

simd::uint2 CMtlData::GetClientSize()
{
    return m_ClientSize;
}

id<MTLRenderPipelineState> CMtlData::GetPipelineState(size_t index)
{
    if (index >= m_pipelineStates.size())
        return nil;
    
    if (index == 2 && m_TextureAlpha)
        index = 3;

    if (!m_BlendingEnabled)
        return m_pipelineStates[index];

    if (m_BlendingState == 1)
        return m_pipelineStatesBlendDest[index];
    else
        return m_pipelineStatesBlend[index];
}

id<MTLRenderPipelineState> CMtlData::GetNamedPipelineState(EPipelineState state)
{
    auto it = m_NamedPipelineStates.find(state);
    if (it == m_NamedPipelineStates.end())
      return nil;
    return it->second;	
}

void CMtlData::x_FunctionNotFound(NSString* function)
{
    NSString* errMsg = [NSString stringWithFormat: @"Shader function %@ not found in default library.", function];
    LOG_POST(Error << [errMsg UTF8String]);
    DeletePipelineStates();
}

void CMtlData::x_FailedToCreatePipelineState(NSError* error)
{
    NSString* errMsg = [NSString stringWithFormat: @"Failed to create pipeline state, error %@", error];
    LOG_POST(Error << [errMsg UTF8String]);
    DeletePipelineStates();
}

bool CMtlData::InitPipelineStates(MTLPixelFormat colorPixelFormat)
{
    if (!m_pipelineStates.empty())
        return true;

    id<MTLDevice> device = GetMTLDevice();
    if (!device)
        return false;

    if (!m_CommandQueue) {
        m_CommandQueue = [device newCommandQueue];

        if (!m_CommandQueue)
            return false;
    }
    
    if(!m_SamplerDescriptor) {
        m_SamplerDescriptor = [MTLSamplerDescriptor new];
        m_SamplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
        m_SamplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
        
        m_SamplerDescriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
        m_SamplerDescriptor.normalizedCoordinates = YES;
    }

    @autoreleasepool {
        NSError *error = nil;

        id<MTLLibrary> defaultLibrary = [[device newDefaultLibrary] autorelease];
        if (!defaultLibrary) {
            NSString* errMsg = @"Failed to load default.metallib";
            LOG_POST(Error << [errMsg UTF8String]);
            return false;
        }

        NSArray* vertexShaders = @[
            @"vertexShaderPlain",
            @"vertexShaderColor",
            @"vertexShaderTexture",
            @"vertexShaderTextureAlpha",
            @"vertexShaderPlain2D",
            @"vertexShaderColor2D",
            @"vertexShaderColor2DUC",
            @"vertexShader2DTexture1D"
            ];

        NSArray* fragmentShaders = @[
            @"fragmentShader",
            @"fragmentShader",
            @"fragmentShaderTexture",
            @"fragmentShaderTextureAlpha",
            @"fragmentShader",
            @"fragmentShader",
            @"fragmentShader",
            @"fragmentShaderTexture",
            ];

        MTLRenderPipelineDescriptor *pipelineStateDescriptor =[[[MTLRenderPipelineDescriptor alloc] init] autorelease];
        pipelineStateDescriptor.label = @"Simple Pipeline";
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = colorPixelFormat;

        MTLRenderPipelineDescriptor *pipelineStateDescriptorBlending =[[[MTLRenderPipelineDescriptor alloc] init] autorelease];
        pipelineStateDescriptorBlending.label = @"Blending Pipeline";
        pipelineStateDescriptorBlending.colorAttachments[0].pixelFormat = colorPixelFormat;
        pipelineStateDescriptorBlending.colorAttachments[0].blendingEnabled = YES;
        pipelineStateDescriptorBlending.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        pipelineStateDescriptorBlending.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        pipelineStateDescriptorBlending.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        pipelineStateDescriptorBlending.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        pipelineStateDescriptorBlending.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pipelineStateDescriptorBlending.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

        MTLRenderPipelineDescriptor *pipelineStateDescriptorBlendingDest =[[[MTLRenderPipelineDescriptor alloc] init] autorelease];
        pipelineStateDescriptorBlendingDest.label = @"Blending Pipeline Destination";
        pipelineStateDescriptorBlendingDest.colorAttachments[0].pixelFormat = colorPixelFormat;
        pipelineStateDescriptorBlendingDest.colorAttachments[0].blendingEnabled = YES;
        pipelineStateDescriptorBlendingDest.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        pipelineStateDescriptorBlendingDest.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        pipelineStateDescriptorBlendingDest.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
        pipelineStateDescriptorBlendingDest.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOneMinusDestinationAlpha;
        pipelineStateDescriptorBlendingDest.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorDestinationAlpha;
        pipelineStateDescriptorBlendingDest.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorDestinationAlpha;

        for (int i = 0; i < vertexShaders.count; ++i) {
            id<MTLFunction> vertexFunction = [[defaultLibrary newFunctionWithName:vertexShaders[i]] autorelease];
            if (!vertexFunction) {
                x_FunctionNotFound(vertexShaders[i]);
                goto error;
            }

            id<MTLFunction> fragmentFunction = [[defaultLibrary newFunctionWithName:fragmentShaders[i]] autorelease];
            if (!fragmentFunction) {
                x_FunctionNotFound(fragmentShaders[i]);
                goto error;
            }

            pipelineStateDescriptor.vertexFunction = vertexFunction;
            pipelineStateDescriptor.fragmentFunction = fragmentFunction;

            id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
            if (!pipelineState) {
                x_FailedToCreatePipelineState(error);
                goto error;
            }
            m_pipelineStates.push_back(pipelineState);

            // Blending  SrcAlpha / (1 - SrcAlpha)

            pipelineStateDescriptorBlending.vertexFunction = vertexFunction;
            pipelineStateDescriptorBlending.fragmentFunction = fragmentFunction;

            pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptorBlending error:&error];
            if (!pipelineState) {
                x_FailedToCreatePipelineState(error);
                goto error;
            }
            m_pipelineStatesBlend.push_back(pipelineState);

            // Blending (1 - DestAlpha) / DestAlpha

            pipelineStateDescriptorBlendingDest.vertexFunction = vertexFunction;
            pipelineStateDescriptorBlendingDest.fragmentFunction = fragmentFunction;

            pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptorBlendingDest error:&error];
            if (!pipelineState) {
                x_FailedToCreatePipelineState(error);
                goto error;
            }
            m_pipelineStatesBlendDest.push_back(pipelineState);
        }


        {
            ///
            /// kPipelineStateLine
            ///

            NSString* vertexShaderLine = @"vertexShaderLine";
            id<MTLFunction> vertexFunction = [[defaultLibrary newFunctionWithName:vertexShaderLine] autorelease];
            if (!vertexFunction) {
                x_FunctionNotFound(vertexShaderLine);
                goto error;
            }

            NSString* fragmentShaderLine = @"fragmentShaderLine";
            id<MTLFunction> fragmentFunction = [[defaultLibrary newFunctionWithName:fragmentShaderLine] autorelease];
            if (!fragmentFunction) {
                x_FunctionNotFound(fragmentShaderLine);
                goto error;
            }

            MTLRenderPipelineDescriptor *pipelineStateDescriptor =[[[MTLRenderPipelineDescriptor alloc] init] autorelease];
            pipelineStateDescriptor.label = @"Line Pipeline";
            pipelineStateDescriptor.colorAttachments[0].pixelFormat = colorPixelFormat;
            pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
            pipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
            pipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
            pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

            pipelineStateDescriptor.vertexFunction = vertexFunction;
            pipelineStateDescriptor.fragmentFunction = fragmentFunction;

            id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
            if (!pipelineState) {
                x_FailedToCreatePipelineState(error);
                goto error;
            }
            m_NamedPipelineStates[kPipelineStateLine] = pipelineState;

            ///
            /// kPipelineStateLineSmooth
            ///

            NSString* vertexShaderLineSmooth = @"vertexShaderLineSmooth";
            vertexFunction = [[defaultLibrary newFunctionWithName:vertexShaderLineSmooth] autorelease];
            if (!vertexFunction) {
                x_FunctionNotFound(vertexShaderLineSmooth);
                goto error;
            }

            NSString* fragmentShaderLineSmooth = @"fragmentShaderLineSmooth";
            fragmentFunction = [[defaultLibrary newFunctionWithName:fragmentShaderLineSmooth] autorelease];
            if (!fragmentFunction) {
                x_FunctionNotFound(fragmentShaderLineSmooth);
                goto error;
            }

            pipelineStateDescriptor =[[[MTLRenderPipelineDescriptor alloc] init] autorelease];
            pipelineStateDescriptor.label = @"Line Smooth Pipeline";
            pipelineStateDescriptor.colorAttachments[0].pixelFormat = colorPixelFormat;
            pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
            pipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
            pipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
            pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

            pipelineStateDescriptor.vertexFunction = vertexFunction;
            pipelineStateDescriptor.fragmentFunction = fragmentFunction;

            pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
            if (!pipelineState) {
                x_FailedToCreatePipelineState(error);
                goto error;
            }
            m_NamedPipelineStates[kPipelineStateLineSmooth] = pipelineState;

            ///
            /// kPipelineStateLineStipple
            ///

            NSString* vertexShaderLineStipple = @"vertexShaderLineStipple";
            vertexFunction = [[defaultLibrary newFunctionWithName:vertexShaderLineStipple] autorelease];
            if (!vertexFunction) {
                x_FunctionNotFound(vertexShaderLineStipple);
                goto error;
            }

            NSString* fragmentShaderLineStipple = @"fragmentShaderLineStipple";
            fragmentFunction = [[defaultLibrary newFunctionWithName:fragmentShaderLineStipple] autorelease];
            if (!fragmentFunction) {
                x_FunctionNotFound(fragmentShaderLineStipple);
                goto error;
            }

            pipelineStateDescriptor =[[[MTLRenderPipelineDescriptor alloc] init] autorelease];
            pipelineStateDescriptor.label = @"Line Stipple Pipeline";
            pipelineStateDescriptor.colorAttachments[0].pixelFormat = colorPixelFormat;
            pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
            pipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
            pipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
            pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

            pipelineStateDescriptor.vertexFunction = vertexFunction;
            pipelineStateDescriptor.fragmentFunction = fragmentFunction;

            pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
            if (!pipelineState) {
                x_FailedToCreatePipelineState(error);
                goto error;
            }
            m_NamedPipelineStates[kPipelineStateLineStipple] = pipelineState;

            ///
            /// kPipelineStateTreeNode
            ///

            NSString* vertexShaderTreeNode = @"vertexShaderTreeNode";
            vertexFunction = [[defaultLibrary newFunctionWithName:vertexShaderTreeNode] autorelease];
            if (!vertexFunction) {
                x_FunctionNotFound(vertexShaderTreeNode);
                goto error;
            }

            NSString* fragmentShaderTreeNode = @"fragmentShader";
            fragmentFunction = [[defaultLibrary newFunctionWithName:fragmentShaderTreeNode] autorelease];
            if (!fragmentFunction) {
                x_FunctionNotFound(fragmentShaderTreeNode);
                goto error;
            }

            pipelineStateDescriptor.label = @"TreeNode Pipeline";
            pipelineStateDescriptor.colorAttachments[0].blendingEnabled = NO;

            pipelineStateDescriptor.vertexFunction = vertexFunction;
            pipelineStateDescriptor.fragmentFunction = fragmentFunction;

            pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
            if (!pipelineState) {
                x_FailedToCreatePipelineState(error);
                goto error;
            }
            m_NamedPipelineStates[kPipelineStateTreeNode] = pipelineState;

            ///
            /// kPipelineStateStippledPolygon
            ///

            NSString* vertexShaderStippledPolygon = @"vertexShaderStippledPolygon";
            vertexFunction = [[defaultLibrary newFunctionWithName:vertexShaderStippledPolygon] autorelease];
            if (!vertexFunction) {
                x_FunctionNotFound(vertexShaderStippledPolygon);
                goto error;
            }

            NSString* fragmentShaderStippledPolygon = @"fragmentShaderStippledPolygon";
            fragmentFunction = [[defaultLibrary newFunctionWithName:fragmentShaderStippledPolygon] autorelease];
            if (!fragmentFunction) {
                x_FunctionNotFound(fragmentShaderStippledPolygon);
                goto error;
            }

            pipelineStateDescriptor.label = @"StippledPolygon Pipeline";
            pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
            pipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
            pipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
            pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
            pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

            pipelineStateDescriptor.vertexFunction = vertexFunction;
            pipelineStateDescriptor.fragmentFunction = fragmentFunction;

            pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
            if (!pipelineState) {
                x_FailedToCreatePipelineState(error);
                goto error;
            }
            m_NamedPipelineStates[kPipelineStateStippledPolygon] = pipelineState;
        }

        return true;

error:
        DeletePipelineStates();
        return false;
    }
}

id<MTLSamplerState> CMtlData::GetSampler()
{
    @autoreleasepool {
        if (!m_Device || !m_SamplerDescriptor)
            return nil;
        return [m_Device newSamplerStateWithDescriptor:m_SamplerDescriptor];
    }
}

void CMtlData::TexParameter(GLenum pname, GLint param)
{
    if (!m_SamplerDescriptor)
        return;
    if (pname == GL_TEXTURE_WRAP_S) {
        switch(param) {
        case GL_CLAMP_TO_EDGE:
            m_SamplerDescriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
            break;
        case GL_MIRRORED_REPEAT:
            m_SamplerDescriptor.sAddressMode = MTLSamplerAddressModeMirrorRepeat;
            break;
        case GL_REPEAT:
            m_SamplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
            break;
        }
    }
    else if (pname == GL_TEXTURE_WRAP_T) {
        switch(param) {
        case GL_CLAMP_TO_EDGE:
            m_SamplerDescriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
            break;
        case GL_MIRRORED_REPEAT:
            m_SamplerDescriptor.tAddressMode = MTLSamplerAddressModeMirrorRepeat;
            break;
        case GL_REPEAT:
            m_SamplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
            break;
        }
    }
    else if (pname == GL_TEXTURE_MAG_FILTER) {
        switch(param) {
        case GL_NEAREST:
            m_SamplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
            break;
        case GL_LINEAR:
            m_SamplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
            break;
        }
    }
    else if (pname == GL_TEXTURE_MIN_FILTER) {
        switch(param) {
        case GL_NEAREST:
            m_SamplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
            break;
        case GL_LINEAR:
            m_SamplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
            break;
        }
    }
}

void CMtlData::BlendingState(int state)
{ 
    switch(state) {
    case 1:
        m_BlendingState = 1;
        break;
    default:
        m_BlendingState = 0;
        break;
    }
}

void CMtlData::DeletePipelineStates()
{
    for (auto& p : m_pipelineStates)
        [p release]; 
    m_pipelineStates.clear();

    for (auto& p : m_pipelineStatesBlend)
        [p release]; 
    m_pipelineStatesBlend.clear();

    for (auto& p : m_pipelineStatesBlendDest)
        [p release]; 
    m_pipelineStatesBlendDest.clear();

    for (auto& p : m_NamedPipelineStates)
      [p.second release];
    m_NamedPipelineStates.clear();

    if (m_CommandQueue) {
        [m_CommandQueue release];
        m_CommandQueue = nil;
    }
}

void CMtlData::EnableDepthTest(bool enable)
{
    @autoreleasepool {
        if (enable) {
            if (!m_EnableDepthTest) {
                MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor new] autorelease];
                depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
                depthStencilDescriptor.depthWriteEnabled = YES;
                m_EnableDepthTest = [m_Device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
            }
            [m_Encoder setDepthStencilState:m_EnableDepthTest];
        } else {
            if (!m_DisableDepthTest) {
                MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor new] autorelease];
                depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
                depthStencilDescriptor.depthWriteEnabled = NO;
                m_DisableDepthTest = [m_Device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
            }
            [m_Encoder setDepthStencilState:m_DisableDepthTest];
        }
    }
}

void CMtlData::RenderLines(
        GLint viewport[4],
        const simd::float4x4& matrix,
        size_t vertexCount,
        const std::function<simd::float2(size_t index)>& coords,
        const std::function<simd::uchar4(size_t index)>& colors)
{
    if (!m_Encoder)
        return;

    simd::float4x4 mv, projection;

    mv.columns[0] = { viewport[2]*.5f, 0, 0, 0 };
    mv.columns[1] = { 0, viewport[3]*.5f, 0, 0 };
    mv.columns[2] = { 0, 0, 1, 0 };
    mv.columns[3] = { viewport[2]*.5f + viewport[0], viewport[3]*.5f + viewport[1], 0, 1 };
    mv = mv*matrix;

    projection.columns[0] = { 2.f/viewport[2], 0, 0, 0 };
    projection.columns[1] = { 0, 2.f/viewport[3], 0, 0 };
    projection.columns[2] = { 0, 0, 1, 0 };
    projection.columns[3] = { - 2.f*viewport[0]/viewport[2] - 1, -2.f*viewport[1]/viewport[3] - 1, 0, 1 };

    UniformsLine uniforms;
    uniforms.mvMatrix = mv;
    uniforms.prMatrix = projection;
    uniforms.lineWidth = GetLineWidth();

    [m_Encoder setTriangleFillMode:MTLTriangleFillModeFill];

    [m_Encoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:0];

    if (m_EnableLineStipple) {
        UniformsStippleLine uniformsStipple;
        uniformsStipple.factor  = m_LineStippleiFactor;
        uniformsStipple.pattern = m_LineStipplePattern;
        [m_Encoder setFragmentBytes:&uniformsStipple length:sizeof(uniformsStipple) atIndex:0];

        [m_Encoder setRenderPipelineState:GetNamedPipelineState(kPipelineStateLineStipple)];

        simd::float2x2 m;
        m.columns[0] = mv.columns[0].xy;
        m.columns[1] = mv.columns[1].xy;

        try {
            int num_lines = vertexCount/2;
            constexpr size_t block = 4096/sizeof(LineStippleVertexIn)/6;
            LineStippleVertexIn b[block*6];

            simd::uchar4 c1, c2;
            simd::float2 v1, v2;

            LineStippleVertexIn* pv = &b[0];
            size_t vertex = 0, lines = 0;

            for (int i = 0; i < num_lines; ++i, ++lines, pv += 6) {
                if (lines == block) {
                    [m_Encoder setVertexBytes:&b[0] length:sizeof(LineStippleVertexIn)*6*lines atIndex:1];
                    [m_Encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6*lines];
                    pv = &b[0];
                    lines = 0;
                }

                v1 = coords(vertex);
                c1 = colors(vertex);
                ++vertex;
                v2 = coords(vertex);
                c2 = colors(vertex);
                ++vertex;

                simd::float2 t1 = v2 - v1, t2 = -t1;
                float length = simd::length(m*t1); 

                pv[0].position = v1;
                pv[0].tangent  = t1;
                pv[0].distance = 0;
                pv[0].color    = c1;

                pv[1].position = v1;
                pv[1].tangent  = t2;
                pv[1].distance = 0;
                pv[1].color    = c1;

                pv[2].position = v2;
                pv[2].tangent  = t1;
                pv[2].distance = length;
                pv[2].color    = c2;

                pv[3] = pv[2];
                pv[4] = pv[1];

                pv[5].position = v2;
                pv[5].tangent  = t2;
                pv[5].distance = length;
                pv[5].color    = c2;
            }

            if (lines) {
                [m_Encoder setVertexBytes:&b[0] length:sizeof(LineStippleVertexIn)*6*lines atIndex:1];
                [m_Encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6*lines];
            }
        } catch (const std::exception&) {}
    } else {

        [m_Encoder setFragmentBytes:&uniforms length:sizeof(uniforms) atIndex:0];


        if (m_LineSmooth) {
            [m_Encoder setRenderPipelineState:GetNamedPipelineState(kPipelineStateLineSmooth)];
        } else {
            [m_Encoder setRenderPipelineState:GetNamedPipelineState(kPipelineStateLine)];
        }


        int num_lines = vertexCount/2;
        try {
            constexpr size_t block = 4096/sizeof(LineVertexIn)/6;
            LineVertexIn b[block*6];

            simd::uchar4 c1, c2;
            simd::float2 v1, v2;

            LineVertexIn* pv = &b[0];
            size_t vertex = 0, lines = 0;

            for (int i = 0; i < num_lines; ++i, ++lines, pv += 6) {
                if (lines == block) {
                    [m_Encoder setVertexBytes:&b[0] length:sizeof(LineVertexIn)*6*lines atIndex:1];
                    [m_Encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6*lines];
                    pv = &b[0];
                    lines = 0;
                }

                v1 = coords(vertex);
                c1 = colors(vertex);
                ++vertex;
                v2 = coords(vertex);
                c2 = colors(vertex);
                ++vertex;

                simd::float2 t1 = v2 - v1, t2 = -t1;

                pv[0].position = v1;
                pv[0].tangent  = t1;
                pv[0].color    = c1;

                pv[1].position = v1;
                pv[1].tangent  = t2;
                pv[1].color    = c1;

                pv[2].position = v2;
                pv[2].tangent  = t1;
                pv[2].color    = c2;

                pv[3] = pv[2];
                pv[4] = pv[1];

                pv[5].position = v2;
                pv[5].tangent  = t2;
                pv[5].color    = c2;
            }

            if (lines) {
                [m_Encoder setVertexBytes:&b[0] length:sizeof(LineVertexIn)*6*lines atIndex:1];
                [m_Encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6*lines];
            }
        } catch (const std::exception&) {}
    }
}

END_NCBI_SCOPE

#endif // NCBI_OS_DARWIN

