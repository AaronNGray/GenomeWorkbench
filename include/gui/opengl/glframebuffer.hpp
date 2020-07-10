#ifndef GUI_WX_GL_FRAMEBUFFER__HPP
#define GUI_WX_GL_FRAMEBUFFER__HPP

/*  $Id: glframebuffer.hpp 42756 2019-04-10 16:43:22Z katargir $
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
 * A wrapper for a simple framebuffer that can be set as a rendering target
 * using the opengl FrameBuffer extension
 */

#include <corelib/ncbiobj.hpp>
#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/opengl/i3dframebuffer.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CGLFrameBuffer

class NCBI_GUIOPENGL_EXPORT CGLFrameBuffer : public CObject, public I3DFrameBuffer
{
public:
    CGLFrameBuffer(size_t dim);
    ~CGLFrameBuffer();

    /// Deletes the current framebuffer, if any
    void Clear();

    ///
    /// Set texture paramters, if needed. Call before calling CreateFrameBuffer
    //

    /// Set texture wrap parameters (default: GL_CLAMP_TO_EDGE)
    void SetTextureWrap(GLint wraps, GLint wrapt);
    GLint GetWraps() const { return m_WrapS; }
    GLint GetWrapT() const { return m_WrapT; }
    /// Set texture filtering parameters (default: GL_LINEAR)
    virtual void SetTextureFiltering(GLint min_filter, GLint mag_filter);
    GLint GetTexMin() const { return m_TexMin; }
    GLint GetTexMag() const { return m_TexMag; }

    ///
    /// Create, set and validate framebuffer
    ///

    /// Creates a new framebuffer, deleting the old one if present
    virtual void CreateFrameBuffer();
    virtual void SetClearColor(float red, float green, float blue, float alpha);

    /// Return an arrray of valid image output sizes
    static std::vector<int> checkFramebufferAvailability(int start_size,
                                                         int count);
    /// Returns true if framebuffer was created successfully
    virtual bool IsValid();
    /// Returns true and writes error message if framebuffer is invalid
    static bool CheckFBOError();
    /// Generates mipmaps for the texture rendering target
    virtual void GenerateMipMaps();
    /// Makes this framebuffer the current rendering target if b==true,
    /// and if b==false, makes the rendering target the default buffer.
    void MakeCurrent(bool b);

    virtual void Render(std::function<void()> renderer);

    virtual I3DTexture* Get3DTexture();
    GLuint GetTexture() { return m_FbTex; }

    /// Returns the size of the framebuffer (w==h, so only return 1 value)
    virtual size_t GetFrameSize() const { return m_FrameSize; }
    /// Releases ownership of texture so it isn't deleted with framebuffer
    virtual void ReleaseTexture() { m_TextureReleased = true; }

protected:
    ///
    /// OpenGL IDs
    ///

    /// Framebuffer id
    GLuint  m_Fb;
    /// Depth renderbuffer id
    GLuint  m_DepthRb;
    /// Texture target id
    GLuint  m_FbTex;    
    /// Size of framebuffer (in both dimensions).
    size_t  m_FrameSize;
   
    ///
    /// Texture parameters
    ///

    /// minification filter
    GLint m_TexMin;
    /// magnification filter
    GLint m_TexMag;
    /// wrap s
    GLint m_WrapS;
    /// wrap t
    GLint m_WrapT;
    /// if true, the texture is not deleted with the framebuffer
    bool m_TextureReleased;

    float m_ClearColorRed;
    float m_ClearColorGreen;
    float m_ClearColorBlue;
    float m_ClearColorAlpha;
};

END_NCBI_SCOPE

#endif // GUI_WX_GL_FRAMEBUFFER__HPP
