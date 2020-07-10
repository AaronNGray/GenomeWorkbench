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

#include <math.h>

#include "FTGL/ftgl.h"

#include "MTLTextureGlyph.h"
#include "MTLTextureGlyphImpl.h"

#include <gui/opengl/mtldata.hpp>
#include <gui/opengl/irender.hpp>

//
//  MTLGLTextureGlyph
//

MTLTextureGlyph::MTLTextureGlyph(FT_GlyphSlot glyph, id<MTLTexture> texture, int xOffset,
                               int yOffset, int width, int height) :
    FTGlyph(new MTLTextureGlyphImpl(glyph, texture, xOffset, yOffset, width, height))
{}


MTLTextureGlyph::~MTLTextureGlyph()
{}


const FTPoint& MTLTextureGlyph::Render(const FTPoint& pen, int renderMode)
{
    MTLTextureGlyphImpl *myimpl = dynamic_cast<MTLTextureGlyphImpl *>(impl);
    return myimpl->RenderImpl(pen, renderMode);
}


using namespace ncbi;

//
//  FTGLTextureGlyphImpl
//


id<MTLTexture> MTLTextureGlyphImpl::activeTexture= nil;

MTLTextureGlyphImpl::MTLTextureGlyphImpl(FT_GlyphSlot glyph, id<MTLTexture> texture, int xOffset,
                                         int yOffset, int width, int height)
:   FTGlyphImpl(glyph),
    destWidth(0),
    destHeight(0),
    glTexture(texture)
{
    /* FIXME: need to propagate the render mode all the way down to
     * here in order to get FT_RENDER_MODE_MONO aliased fonts.
     */

    err = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
    if(err || glyph->format != ft_glyph_format_bitmap)
    {
        return;
    }

    advance = FTPoint(glyph->advance.x / 64.0f,
                      glyph->advance.y / 64.0f);

    FT_Bitmap      bitmap = glyph->bitmap;

    destWidth  = bitmap.width;
    destHeight = bitmap.rows;

    if(destWidth && destHeight)
    {
        size_t buffSize = destWidth*4*destHeight;
        std::unique_ptr<unsigned char[]> buff(new unsigned char[buffSize]);
        memset(&buff[0], 0, buffSize);

        unsigned char* p1 = &buff[0];
        unsigned char* p2 = bitmap.buffer;

        for (int i = 0; i < destHeight; ++i) {
            for (int j = 0; j < destWidth; ++j) {
                p1[j*4 + 3] = p2[j];
            }

            p1 += destWidth*4;
            p2 += destWidth;
        }

        MTLRegion region = MTLRegionMake2D(xOffset, yOffset, destWidth, destHeight);
        [glTexture replaceRegion:region mipmapLevel:0 withBytes:&buff[0] bytesPerRow:4*destWidth];
    }


//      0
//      +----+
//      |    |
//      |    |
//      |    |
//      +----+
//           1

    uv[0].X(static_cast<float>(xOffset) / static_cast<float>(width));
    uv[0].Y(static_cast<float>(yOffset) / static_cast<float>(height));
    uv[1].X(static_cast<float>(xOffset + destWidth) / static_cast<float>(width));
    uv[1].Y(static_cast<float>(yOffset + destHeight) / static_cast<float>(height));

    corner = FTPoint(glyph->bitmap_left, glyph->bitmap_top);
}


MTLTextureGlyphImpl::~MTLTextureGlyphImpl()
{}


const FTPoint& MTLTextureGlyphImpl::RenderImpl(const FTPoint& pen,
                                              int renderMode)
{
    float dx, dy;

    id<MTLRenderCommandEncoder> renderEncoder = Metal().GetEncoder();

    if (activeTexture != glTexture)
    {
        [renderEncoder setFragmentTexture:glTexture atIndex:0];
        activeTexture = glTexture;
    }

    dx = floor(pen.Xf() + corner.Xf());
    dy = floor(pen.Yf() + corner.Yf());

    IRender& gl = GetGl();

    gl.Begin(GL_TRIANGLE_STRIP);
        gl.TexCoord2f(uv[0].Xf(), uv[1].Yf());
        gl.Vertex2f(dx, dy - destHeight);

        gl.TexCoord2f(uv[0].Xf(), uv[0].Yf());
        gl.Vertex2f(dx, dy);

        gl.TexCoord2f(uv[1].Xf(), uv[1].Yf());
        gl.Vertex2f(dx + destWidth, dy - destHeight);

        gl.TexCoord2f(uv[1].Xf(), uv[0].Yf());
        gl.Vertex2f(dx + destWidth, dy);
    gl.End();

    return advance;
}

#endif // USE_METAL

