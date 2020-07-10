/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <ncbi_pch.hpp>

#include <gui/opengl.h>
#ifdef USE_METAL

#include <cassert>
#include <string> // For memset

#include <FTGL/ftgl.h>

#include "MTLTextureGlyphImpl.h"
#include "MTLTextureGlyph.h"
#include "MTLTextureFontImpl.h"
#include "MTLTextureFont.h"

#include <gui/opengl/mtldata.hpp>
#include <gui/opengl/irender.hpp>

using namespace ncbi;

//
//  MTLTextureFont
//


MTLTextureFont::MTLTextureFont(char const *fontFilePath) :
    FTFont(new MTLTextureFontImpl(this, fontFilePath))
{}


MTLTextureFont::MTLTextureFont(const unsigned char *pBufferBytes,
                               size_t bufferSizeInBytes) :
    FTFont(new MTLTextureFontImpl(this, pBufferBytes, bufferSizeInBytes))
{}


MTLTextureFont::~MTLTextureFont()
{}


FTGlyph* MTLTextureFont::MakeGlyph(FT_GlyphSlot ftGlyph)
{
    MTLTextureFontImpl *myimpl = dynamic_cast<MTLTextureFontImpl *>(impl);
    if(!myimpl)
    {
        return NULL;
    }

    return myimpl->MakeGlyphImpl(ftGlyph);
}


//
//  MTLTextureFontImpl
//


static inline GLuint NextPowerOf2(GLuint in)
{
     in -= 1;

     in |= in >> 16;
     in |= in >> 8;
     in |= in >> 4;
     in |= in >> 2;
     in |= in >> 1;

     return in + 1;
}


MTLTextureFontImpl::MTLTextureFontImpl(FTFont *ftFont, const char* fontFilePath)
:   FTFontImpl(ftFont, fontFilePath),
    maximumGLTextureSize(16384),
    textureWidth(0),
    textureHeight(0),
    glyphHeight(0),
    glyphWidth(0),
    padding(3),
    xOffset(0),
    yOffset(0)
{
    load_flags = FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
    remGlyphs = numGlyphs = face.GlyphCount();
}


MTLTextureFontImpl::MTLTextureFontImpl(FTFont *ftFont,
                                       const unsigned char *pBufferBytes,
                                       size_t bufferSizeInBytes)
:   FTFontImpl(ftFont, pBufferBytes, bufferSizeInBytes),
    maximumGLTextureSize(16384),
    textureWidth(0),
    textureHeight(0),
    glyphHeight(0),
    glyphWidth(0),
    padding(3),
    xOffset(0),
    yOffset(0)
{
    load_flags = FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
    remGlyphs = numGlyphs = face.GlyphCount();
}


MTLTextureFontImpl::~MTLTextureFontImpl()
{
    for (auto t : textureList)
        [t release];
}


FTGlyph* MTLTextureFontImpl::MakeGlyphImpl(FT_GlyphSlot ftGlyph)
{
    glyphHeight = static_cast<int>(charSize.Height() + 0.5);
    glyphWidth = static_cast<int>(charSize.Width() + 0.5);

    if(glyphHeight < 1) glyphHeight = 1;
    if(glyphWidth < 1) glyphWidth = 1;

    if(textureList.empty())
    {
        textureList.push_back(CreateTexture());
        xOffset = yOffset = padding;
    }

    if(xOffset > (textureWidth - glyphWidth))
    {
        xOffset = padding;
        yOffset += glyphHeight;

        if(yOffset > (textureHeight - glyphHeight))
        {
            textureList.push_back(CreateTexture());
            yOffset = padding;
        }
    }

    MTLTextureGlyph* tempGlyph = new MTLTextureGlyph(ftGlyph, textureList[textureList.size() - 1],
                                                   xOffset, yOffset, textureWidth, textureHeight);
    xOffset += static_cast<int>(tempGlyph->BBox().Upper().X() - tempGlyph->BBox().Lower().X() + padding + 0.5);

    --remGlyphs;

    return tempGlyph;
}


void MTLTextureFontImpl::CalculateTextureSize()
{
    textureWidth = NextPowerOf2((remGlyphs * glyphWidth) + (padding * 2));
    textureWidth = textureWidth > maximumGLTextureSize ? maximumGLTextureSize : textureWidth;

    int h = static_cast<int>((textureWidth - (padding * 2)) / glyphWidth + 0.5);

    textureHeight = NextPowerOf2(((numGlyphs / h) + 1) * glyphHeight);
    textureHeight = textureHeight > maximumGLTextureSize ? maximumGLTextureSize : textureHeight;
}


id<MTLTexture> MTLTextureFontImpl::CreateTexture()
{
    CalculateTextureSize();

    int totalMemory = textureWidth * textureHeight * 4;
    unique_ptr<unsigned char[]> textureMemory(new unsigned char[totalMemory]);
    memset(&textureMemory[0], 0, totalMemory);

    id<MTLTexture> texture = nil;
    @autoreleasepool {
        MTLTextureDescriptor *textureDescriptor = 
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                  width:textureWidth
                                  height:textureHeight
                                  mipmapped:NO];

        texture = [Metal().GetMTLDevice() newTextureWithDescriptor:textureDescriptor];

        MTLRegion region = MTLRegionMake2D(0, 0, textureWidth, textureHeight);
        [texture replaceRegion:region
                 mipmapLevel:0
                 withBytes:&textureMemory[0]
                 bytesPerRow:4*textureWidth];
    }

    return texture;
}


bool MTLTextureFontImpl::FaceSize(const unsigned int size, const unsigned int res)
{
    if(!textureList.empty()) {
        for (auto t : textureList)
            [t release];
        textureList.clear();
        remGlyphs = numGlyphs = face.GlyphCount();
    }

    return FTFontImpl::FaceSize(size, res);
}


template <typename T>
inline FTPoint MTLTextureFontImpl::RenderI(const T* string, const int len,
                                           FTPoint position, FTPoint spacing,
                                           int renderMode)
{
    IRender& gl = GetGl();

    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.Enable(GL_BLEND);

    Metal().TexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP);
    Metal().TexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP);
    Metal().TexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Metal().TexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    Metal().EnableTextureAlpha(true);

    MTLTextureGlyphImpl::ResetActiveTexture();

    FTPoint tmp = FTFontImpl::Render(string, len,
                                     position, spacing, renderMode);

    return tmp;
}


FTPoint MTLTextureFontImpl::Render(const char * string, const int len,
                                   FTPoint position, FTPoint spacing,
                                   int renderMode)
{
    return RenderI(string, len, position, spacing, renderMode);
}


FTPoint MTLTextureFontImpl::Render(const wchar_t * string, const int len,
                                   FTPoint position, FTPoint spacing,
                                   int renderMode)
{
    return RenderI(string, len, position, spacing, renderMode);
}

#endif // USE_METAL
