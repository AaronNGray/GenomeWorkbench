/*  $Id: mtltexture.cpp 42756 2019-04-10 16:43:22Z katargir $
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

#include "mtltexture.hpp"

#include <gui/opengl/mtldata.hpp>
#include <gui/opengl/glexception.hpp>
#include <util/image/image_io.hpp>

BEGIN_NCBI_SCOPE

CMtlTexture::CMtlTexture()
    : m_WrapS(GL_CLAMP)
    , m_WrapT(GL_CLAMP)
    , m_FilterMin(GL_LINEAR)
    , m_FilterMag(GL_LINEAR)
    , m_Texture(nil)
{
}

CMtlTexture::CMtlTexture(id<MTLTexture> texture)
    : m_WrapS(GL_CLAMP)
    , m_WrapT(GL_CLAMP)
    , m_FilterMin(GL_LINEAR)
    , m_FilterMag(GL_LINEAR)
    , m_Texture(texture)
{
}

//
// get an image from a file
CMtlTexture::CMtlTexture(CNcbiIstream& istr, const string& tag)
    : m_ImageTag(tag)
    , m_WrapS(GL_CLAMP)
    , m_WrapT(GL_CLAMP)
    , m_FilterMin(GL_LINEAR)
    , m_FilterMag(GL_LINEAR)
    , m_Texture(nil)
{
    m_Image.Reset(CImageIO::ReadImage(istr));
    if ( !m_Image ) {
        string msg("CMtlTexture(): cannot read image");
        NCBI_THROW(COpenGLException, eTextureError, msg);
    }
}

CMtlTexture::CMtlTexture(CImage* image)
    : m_WrapS(GL_CLAMP)
    , m_WrapT(GL_CLAMP)
    , m_FilterMin(GL_LINEAR)
    , m_FilterMag(GL_LINEAR)
    , m_Texture(nil)
{
    Clear();
    m_Image.Reset(image);
    Load();
}

CMtlTexture::~CMtlTexture()
{
    Unload();
}

//
// initialize an empty image
//
void CMtlTexture::Init(size_t w, size_t h, size_t d)
{
    Clear();

    // initialize our image
    m_Image.Reset(new CImage(w, h, d));
}


//
// clear the current image
//
void CMtlTexture::Clear(void)
{
    Unload();
    m_Image.Reset();
}


//
// "swallow" an image
//
void CMtlTexture::Swallow(CImage *image)
{
    // clear the current image
    Clear();

    // assign the wrapped image
    m_Image.Reset(image);

    // load the texture
    Load();
}


bool CMtlTexture::IsValid(void) const
{
    return (m_Texture != nil);
}

//
// bind the image as a texture
//
void CMtlTexture::MakeCurrent(void)
{
    // texture is still valid?
    if (!IsValid()) {
        Load();
    }

    if (!m_Texture)
        return;

    id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();
    if (!renderEncoder)
        return;

    Metal().EnableTextureModulate(m_TexEnv == GL_MODULATE);
    Metal().EnableTextureAlpha(false);

    [renderEncoder setFragmentTexture:m_Texture atIndex:0];
}

//
// set (or reset) the opengl texture parameters that are saved in
// the texture object
// 
void CMtlTexture::SetParams()
{
    if (!IsValid()) 
        return;

    MakeCurrent();

    Metal().TexParameter(GL_TEXTURE_WRAP_S, m_WrapS);
    Metal().TexParameter(GL_TEXTURE_WRAP_T, m_WrapT);
    Metal().TexParameter(GL_TEXTURE_MAG_FILTER, m_FilterMag);
    Metal().TexParameter(GL_TEXTURE_MIN_FILTER, m_FilterMin);
}

//
// create a representation of the texture for rendering
// this is either a display list or a texture object
//
void CMtlTexture::Load(void)
{
    //_TRACE("CMtlTexture::Load()");
    // unload the current texture
    Unload();

    // safety first!
    if ( !m_Image ) {
        _TRACE("  no image to load!");
        return;
    }

    switch (m_Image->GetDepth()) {
    case 4:
        break;
    default:
        LOG_POST(Error << "CMtlTexture::Load(): unhandled image depth");
        return;
    }

    @autoreleasepool {
        MTLTextureDescriptor *textureDescriptor = 
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                  width:m_Image->GetWidth()
                                  height:m_Image->GetHeight()
                                  mipmapped:NO];
        m_Texture = [Metal().GetMTLDevice() newTextureWithDescriptor:textureDescriptor];

        MTLRegion region = MTLRegionMake2D(0, 0, m_Image->GetWidth(), m_Image->GetHeight());
        [m_Texture replaceRegion:region
                   mipmapLevel:0
                   withBytes:m_Image->GetData() 
                   bytesPerRow:4*m_Image->GetWidth()];
    }
}

void CMtlTexture::Load1DRGBA(size_t width, float* data)
{
    Unload();

    @autoreleasepool {
        MTLTextureDescriptor *textureDescriptor = 
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA32Float
                                  width:width
                                  height:1
                                  mipmapped:NO];
        m_Texture = [Metal().GetMTLDevice() newTextureWithDescriptor:textureDescriptor];

        MTLRegion region = MTLRegionMake2D(0, 0, width, 1);
        [m_Texture replaceRegion:region
                   mipmapLevel:0
                   withBytes:data 
                   bytesPerRow:4*width*sizeof(float)];
    }

    SetParams();
}

//
// unload the image as a texture
void CMtlTexture::Unload (void)
{
    if (m_Texture) {
        [m_Texture release];
        m_Texture = nil;
    }
}

END_NCBI_SCOPE

#endif // NCBI_OS_DARWIN
