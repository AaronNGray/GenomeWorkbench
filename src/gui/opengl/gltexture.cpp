/*  $Id: gltexture.cpp 43126 2019-05-17 18:16:36Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <gui/opengl/gltexture.hpp>
#include <gui/opengl/glexception.hpp>
#include <util/image/image_io.hpp>
#include <gui/opengl.h>

BEGIN_NCBI_SCOPE


// default ctor
CGlTexture::CGlTexture()
    : m_TexId(0),
      m_Target(GL_TEXTURE_2D),
      m_WrapS(GL_CLAMP),
      m_WrapT(GL_CLAMP),
      m_FilterMin(GL_LINEAR),
      m_FilterMag(GL_LINEAR),
      m_TexEnv(GL_MODULATE)
{
}


//
// ctor: create an empty image
CGlTexture::CGlTexture(size_t w, size_t h, size_t depth)
    : m_TexId(0),
      m_Target(GL_TEXTURE_2D),
      m_WrapS(GL_CLAMP),
      m_WrapT(GL_CLAMP),
      m_FilterMin(GL_LINEAR),
      m_FilterMag(GL_LINEAR),
      m_TexEnv(GL_MODULATE)
{
    Init(w, h, depth);
}

// ctor:  create a texture with the texture id of a texture
// created elsewhere
CGlTexture::CGlTexture(GLuint tex_id,  size_t w, size_t h, GLenum target)
    : m_Width(w),
      m_Height(h),
      m_TexId(tex_id),
      m_Target(target),
      m_WrapS(GL_CLAMP),
      m_WrapT(GL_CLAMP),
      m_FilterMin(GL_LINEAR),
      m_FilterMag(GL_LINEAR),
      m_TexEnv(GL_MODULATE)
{
    // only works if it's really a texture
    if (!glIsTexture(tex_id))
        m_TexId = 0;
}


//
// ctor: create an image around an existing image
CGlTexture::CGlTexture(CImage* image)
    : m_TexId(0),
      m_Target(GL_TEXTURE_2D),
      m_WrapS(GL_CLAMP),
      m_WrapT(GL_CLAMP),
      m_FilterMin(GL_LINEAR),
      m_FilterMag(GL_LINEAR),
      m_TexEnv(GL_MODULATE)
{
    Swallow(image);
}


//
// get an image from a file
CGlTexture::CGlTexture(CNcbiIstream& istr, const string& tag)
    : m_ImageTag(tag),
      m_TexId(0),
      m_Target(GL_TEXTURE_2D),
      m_WrapS(GL_CLAMP),
      m_WrapT(GL_CLAMP),
      m_FilterMin(GL_LINEAR),
      m_FilterMag(GL_LINEAR),
      m_TexEnv(GL_MODULATE)
{
    m_Image.Reset(CImageIO::ReadImage(istr));
    if ( !m_Image ) {
        string msg("CGlTexture(): cannot read image");
        NCBI_THROW(COpenGLException, eTextureError, msg);
    }
}


//
// initialize an empty image
//
void CGlTexture::Init(size_t w, size_t h, size_t d)
{
    Clear();

    // initialize our image
    m_Image.Reset(new CImage(w, h, d));
}

//
// clear the current image
//
void CGlTexture::Clear(void)
{
    Unload();
    m_Image.Reset();
}


//
// "swallow" an image
//
void CGlTexture::Swallow(CImage *image)
{
    // clear the current image
    Clear();

    // assign the wrapped image
    m_Image.Reset(image);

    // load the texture
    Load();
}


bool CGlTexture::IsValid(void) const
{
    return glIsTexture(m_TexId) ? true : false;
}


//
// bind the image as a texture
//
void CGlTexture::MakeCurrent(void)
{
    // texture is still valid?
    if (!IsValid()) {
        Load();
    }

    // Texture environment is not saved in the texture object, so we set it
    // here
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_TexEnv);

    glEnable(m_Target);
    glBindTexture(m_Target, m_TexId);
}

//
// set (or reset) the opengl texture parameters that are saved in
// the texture object
// 
void CGlTexture::SetParams()
{
    if (!IsValid()) 
        return;

    MakeCurrent();
    glPixelStorei  (GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(m_Target, GL_TEXTURE_WRAP_S, m_WrapS);
    glTexParameteri(m_Target, GL_TEXTURE_WRAP_T, m_WrapT);
    glTexParameteri(m_Target, GL_TEXTURE_MAG_FILTER, m_FilterMag);
    glTexParameteri(m_Target, GL_TEXTURE_MIN_FILTER, m_FilterMin);
}


CImage* CGlTexture::GenerateImage()
{
    if (!glIsTexture(m_TexId) || m_Target != GL_TEXTURE_2D || 
        m_Width == 0 || m_Height == 0)
        return nullptr;

    CRef<CImage> img(new CImage(m_Width, m_Height, 3));

    MakeCurrent();
    GLint alignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(m_Target, 0, GL_RGB, GL_UNSIGNED_BYTE, img->SetData());
    glPixelStorei(GL_PACK_ALIGNMENT, alignment);

    return img.Release();
}


//
// create a representation of the texture for rendering
// this is either a display list or a texture object
//
void CGlTexture::Load(void)
{
    //_TRACE("CGlTexture::Load()");
    // unload the current texture
    Unload();

    // safety first!
    if ( !m_Image ) {
        _TRACE("  no image to load!");
        return;
    }

    glGenTextures(1, &m_TexId);
    glBindTexture(m_Target, m_TexId);

    // set texture params
    SetParams();

    // call glTexImage2D
    switch (m_Image->GetDepth()) {
    case 1:
        gluBuild2DMipmaps(m_Target, GL_ALPHA,
                          (GLint)m_Image->GetWidth(), (GLint)m_Image->GetHeight(),
                          GL_ALPHA, GL_UNSIGNED_BYTE,
                          m_Image->GetData());
        break;

    case 3:
        gluBuild2DMipmaps(m_Target, GL_RGB,
                          (GLint)m_Image->GetWidth(), (GLint)m_Image->GetHeight(),
                          GL_RGB, GL_UNSIGNED_BYTE,
                          m_Image->GetData());
        break;

    case 4:
        gluBuild2DMipmaps(m_Target, GL_RGBA,
                          (GLint)m_Image->GetWidth(), (GLint)m_Image->GetHeight(),
                          GL_RGBA, GL_UNSIGNED_BYTE,
                          m_Image->GetData());
        break;

    default:
        LOG_POST(Error << "CGlTexture::Load(): unhandled image depth");
        break;
    }
    m_Width  = m_Image->GetWidth();
    m_Height = m_Image->GetHeight();
}

void CGlTexture::Load1DRGBA(size_t width, float* data)
{
    Unload();

    m_Target = GL_TEXTURE_1D;
    glGenTextures(1, &m_TexId);
    glBindTexture(m_Target, m_TexId);
    SetParams();
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, width, 0, GL_RGBA, GL_FLOAT, (GLvoid*)data);
    m_Width = width;
    m_Height = 1;
}

//
// unload the image as a texture
void CGlTexture::Unload (void)
{

    if (glIsTexture (m_TexId))
        glDeleteTextures (1, &m_TexId);
    
    m_TexId = 0;

    m_Width  = 0;
    m_Height = 0;
}

END_NCBI_SCOPE
