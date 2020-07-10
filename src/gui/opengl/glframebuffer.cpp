/*  $Id: glframebuffer.cpp 43452 2019-07-02 14:42:27Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include <gui/opengl/glframebuffer.hpp>
#include <gui/opengl/gltexture.hpp>
#include <gui/opengl/glutils.hpp>


BEGIN_NCBI_SCOPE

enum EGlDiagMode {
    eUndefined,
    eIgnore,
    eLogPost,
    eThrow,
    eAbort
};

CGLFrameBuffer::CGLFrameBuffer(size_t dim) 
: m_Fb(0)
, m_DepthRb(0)
, m_FbTex(0)
, m_FrameSize(dim) 
, m_TexMin(GL_LINEAR)
, m_TexMag(GL_LINEAR)
, m_WrapS(GL_CLAMP)
, m_WrapT(GL_CLAMP)
, m_TextureReleased(false)
, m_ClearColorRed(1)
, m_ClearColorGreen(1)
, m_ClearColorBlue(1)
, m_ClearColorAlpha(1)
{
}

CGLFrameBuffer::~CGLFrameBuffer()
{
    Clear();
}

bool CGLFrameBuffer::CheckFBOError()
{
    GLenum status;
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status == GL_FRAMEBUFFER_COMPLETE_EXT)
        return false;

    static EGlDiagMode mode = eUndefined;
    if ( mode == eUndefined ) {
        const char* value = getenv("NCBI_GBENCH_GLERROR");
        if ( !value ) {
            mode = eIgnore;
        } else if (strcmp(value, "ABORT") == 0) {
            mode = eAbort;
        } else if (strcmp(value, "LOGPOST") == 0) {
            mode = eLogPost;
        } else if (strcmp(value, "THROW") == 0) {
            mode = eThrow;
        } else {
            mode = eIgnore;
        }
    }

    std::string msg;

    switch(status) { 
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            msg = "Framebuffer: Incomplete attachment";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
             msg = "Framebuffer: Incomplete missing attachment";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
             msg = "Framebuffer: Incomplete dimensions";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
             msg = "Framebuffer: Incomplete formats";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
             msg = "Framebuffer: Incomplete draw buffer";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
             msg = "Framebuffer: Incomplete read buffer";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
             msg = "Framebuffer: Unsupported framebuffer";
            break;
        default:
             msg = "Framebuffer: Unknown error";
            break;
    }

    switch (mode) {
    case eUndefined:
    case eIgnore:
    default:
        break;

    case eLogPost:
        LOG_POST(Error << msg);
        break;

    case eAbort:
        // abort
        LOG_POST(Error << msg);
        Abort();
        break;

    //case eThrow:
    //    // throw
    //    NCBI_THROW(COpenGLException, eGlError, msg);
    //    break;
    }

    return true;
}

void CGLFrameBuffer::Clear()
{
    if (glewIsSupported("GL_EXT_framebuffer_object")) {
        if (glIsTexture(m_FbTex) && !m_TextureReleased)
            glDeleteTextures(1, &m_FbTex);
        if (glIsRenderbufferEXT(m_DepthRb))
            glDeleteRenderbuffersEXT(1, &m_DepthRb);
        if (glIsFramebufferEXT(m_Fb))
            glDeleteFramebuffersEXT(1, &m_Fb);
    }

    m_Fb = 0;
    m_FbTex = 0;
    m_DepthRb = 0;
}
    
void CGLFrameBuffer::SetTextureWrap(GLint wraps, GLint wrapt)
{
    m_WrapS = wraps;
    m_WrapT = wrapt;
}


void CGLFrameBuffer::SetTextureFiltering(GLint min_filter, GLint mag_filter)
{
    m_TexMin = min_filter;
    m_TexMag = mag_filter;
}


void CGLFrameBuffer::CreateFrameBuffer()
{
    if (!glewIsSupported("GL_EXT_framebuffer_object")) 
        return;

    Clear();

    glGenFramebuffersEXT(1, &m_Fb);// frame buffer             
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_Fb);

    // initialize texture
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &m_FbTex); // texture

    glBindTexture(GL_TEXTURE_2D, m_FbTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_TexMag );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_TexMin );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_WrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_WrapT);
    // may need to generate mipmaps here to fix ati bug (shoudn't be required
    // r.e. standard)
    // http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=249408
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)m_FrameSize, (GLsizei)m_FrameSize, 0,
                 GL_RGBA, GL_FLOAT, NULL);
    // attach texture to framebuffercolor buffer
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_FbTex, 0);
 
    // initialize depth/stencil renderbuffer 
    glGenRenderbuffersEXT(1, &m_DepthRb);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_DepthRb);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL_EXT, (GLsizei)m_FrameSize, (GLsizei)m_FrameSize);
    // attach renderbuffer to framebufferdepth buffer
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_DepthRb);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_DepthRb);

    if (CheckFBOError()) {
        Clear();
        m_Fb = 0;
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        return;
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    // On mac at least, it seems you need to generate the mipmaps before you
    // first use the framebuffer.  (otherwise result is blank on first
    // usage).    
    //m_TexMag can only be GL_NEAREST or GL_LINEAR (no mipmaps used in magnification)
    if (m_TexMin == GL_NEAREST_MIPMAP_NEAREST ||
        m_TexMin == GL_LINEAR_MIPMAP_NEAREST ||
        m_TexMin == GL_NEAREST_MIPMAP_LINEAR ||
        m_TexMin == GL_LINEAR_MIPMAP_LINEAR )
    {
        GenerateMipMaps();
    }    

    CGlUtils::CheckGlError();
}

void CGLFrameBuffer::SetClearColor(float red, float green, float blue, float alpha)
{
    m_ClearColorRed = red;
    m_ClearColorGreen = green;
    m_ClearColorBlue = blue;
    m_ClearColorAlpha = alpha;
}

std::vector<int> CGLFrameBuffer::checkFramebufferAvailability(int start_size, int count)
{
    int current_size = start_size;

    std::vector<int> sizes;

    if (!glewIsSupported("GL_EXT_framebuffer_object")) 
        return sizes;

    GLint max_tex_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);

    for (int i=0; i<count; ++i) {
        if (current_size > max_tex_size)
            break;

        sizes.push_back(current_size);
        current_size *= 2;
    }    
    
    return sizes;
}


bool CGLFrameBuffer::IsValid()
{
    if (m_Fb > 0)
        return true;

    return false;
}

void CGLFrameBuffer::MakeCurrent(bool b)
{
    if (IsValid()) {

        if (b) {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_Fb);

            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);         
            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
        }
        else {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

            glDrawBuffer(GL_BACK);
            glReadBuffer(GL_BACK);
        }
    }
}

void CGLFrameBuffer::Render(std::function<void()> renderer)
{
    if (!IsValid())
        return;

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_Fb);

    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

    glClearColor(m_ClearColorRed, m_ClearColorGreen, m_ClearColorBlue, m_ClearColorAlpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer();

    glFinish();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);
}


void CGLFrameBuffer::GenerateMipMaps()
{
    if (IsValid()) {
        MakeCurrent(false);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_FbTex);

        // Had been using glGenerateMipmap(GL_TEXTURE_2D) but a bug
        // with MESA made that crash (it didn't try to get the pointer
        // to the function).  This seems to work everywhere.
        glGenerateMipmapEXT(GL_TEXTURE_2D);
    }
}

I3DTexture* CGLFrameBuffer::Get3DTexture()
{
    return new CGlTexture(m_FbTex, m_FrameSize, m_FrameSize);
}

END_NCBI_SCOPE
