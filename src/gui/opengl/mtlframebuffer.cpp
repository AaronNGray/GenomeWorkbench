/*  $Id: mtlframebuffer.cpp 42756 2019-04-10 16:43:22Z katargir $
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

#include "mtlframebuffer.hpp"

#include <gui/opengl/mtldata.hpp>
#include <gui/opengl/mtldata.hpp>

BEGIN_NCBI_SCOPE

CMtlFrameBuffer::CMtlFrameBuffer(size_t dim)
    : m_FrameSize(dim)
    , m_TexMin(GL_LINEAR)
    , m_TexMag(GL_LINEAR)
    , m_ClearColorRed(1)
    , m_ClearColorGreen(1)
    , m_ClearColorBlue(1)
    , m_ClearColorAlpha(1)
{
}

void CMtlFrameBuffer::SetTextureFiltering(GLint min_filter, GLint mag_filter)
{
    m_TexMin = min_filter;
    m_TexMag = mag_filter;
}

void CMtlFrameBuffer::CreateFrameBuffer()
{
    if (m_Texture)
        return;

    @autoreleasepool {
        MTLTextureDescriptor *textureDescriptor = 
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                  width:m_FrameSize
                                  height:m_FrameSize
                                  mipmapped:YES];

        id<MTLTexture> texture = [Metal().GetMTLDevice() newTextureWithDescriptor:textureDescriptor];
        if (texture) {
            m_Texture.Reset(new CMtlTexture(texture));
            m_Texture->SetFilterMin(m_TexMin);
            m_Texture->SetFilterMag(m_TexMag);
            m_Texture->SetWrapS(GL_CLAMP);
            m_Texture->SetWrapT(GL_CLAMP);
        }
    }
}

void CMtlFrameBuffer::SetClearColor(float red, float green, float blue, float alpha)
{
    m_ClearColorRed = red;
    m_ClearColorGreen = green;
    m_ClearColorBlue = blue;
    m_ClearColorAlpha = alpha;
}

bool CMtlFrameBuffer::IsValid()
{
    return m_Texture;
}

void CMtlFrameBuffer::Render(std::function<void()> renderer)
{
    if (!m_Texture)
        return;

    @autoreleasepool {
        id<MTLCommandBuffer> commandBuffer = [Metal().GetCommandQueue() commandBuffer];
        commandBuffer.label = @"FrameBufferCommand";

        MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];

        if (renderPassDescriptor) {
            renderPassDescriptor.colorAttachments[0].texture = m_Texture->m_Texture;
            renderPassDescriptor.colorAttachments[0].clearColor =
                MTLClearColorMake(m_ClearColorRed, m_ClearColorGreen, m_ClearColorBlue, m_ClearColorAlpha);
            renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;

            // Create a render command encoder so we can render into something
            id<MTLRenderCommandEncoder> renderEncoder =
                [commandBuffer renderCommandEncoderWithDescriptor: renderPassDescriptor];
            renderEncoder.label = @"FrameBufferEncoder";

            [renderEncoder setRenderPipelineState: Metal().GetPipelineState(0)];

            id<MTLRenderCommandEncoder> renderEncoderSave = Metal().GetEncoder();
            simd::uint2 clientSizeSave = Metal().GetClientSize();
            Metal().SetClientSize({ (uint)m_FrameSize, (uint)m_FrameSize });
            Metal().SetEncoder(renderEncoder);
            renderer();
            Metal().SetEncoder(renderEncoderSave);
            Metal().SetClientSize(clientSizeSave);

            [renderEncoder endEncoding];
        }

        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            dispatch_semaphore_signal(semaphore);
        }];
        // Finalize rendering here & push the command buffer to the GPU
        [commandBuffer commit];

        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
        dispatch_release(semaphore);

        MTLRegion r1 = MTLRegionMake2D(0, 0, m_FrameSize, 1);
        MTLRegion r2 = MTLRegionMake2D(0, m_FrameSize - 1, m_FrameSize, 1);
        int bytesPerRow = m_FrameSize*4;
        unique_ptr<char[]> buffer(new char[bytesPerRow*2]);

        for (int i = 0; i < m_FrameSize/2; ++i) {
            [m_Texture->m_Texture
                getBytes:    buffer.get()
                bytesPerRow: bytesPerRow
                fromRegion:  r1
                mipmapLevel: 0];

            [m_Texture->m_Texture
                getBytes:    buffer.get() + bytesPerRow
                bytesPerRow: bytesPerRow
                fromRegion:  r2
                mipmapLevel: 0];

            [m_Texture->m_Texture
                replaceRegion: r1
                mipmapLevel:   0
                withBytes:     buffer.get() + bytesPerRow
                bytesPerRow:   bytesPerRow];

            [m_Texture->m_Texture
                replaceRegion: r2
                mipmapLevel:   0
                withBytes:     buffer.get()
                bytesPerRow:   bytesPerRow];

            ++r1.origin.y;
            --r2.origin.y;
        }
    }
}

void CMtlFrameBuffer::GenerateMipMaps()
{
    if (!m_Texture)
        return;

    @autoreleasepool {
        id<MTLCommandBuffer> commandBuffer = [Metal().GetCommandQueue() commandBuffer];
        commandBuffer.label = @"FrameBufferMipMaps";

        id<MTLBlitCommandEncoder> commandEncoder =  [commandBuffer blitCommandEncoder];
        [commandEncoder generateMipmapsForTexture: m_Texture->m_Texture];
        [commandEncoder endEncoding];
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            dispatch_semaphore_signal(semaphore);
        }];
        [commandBuffer commit];
        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
        dispatch_release(semaphore);
    }
}

I3DTexture* CMtlFrameBuffer::Get3DTexture()
{
    return m_Texture;
}

END_NCBI_SCOPE

#endif // NCBI_OS_DARWIN
