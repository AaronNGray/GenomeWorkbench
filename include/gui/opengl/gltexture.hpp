#ifndef GUI_OPENGL___GL_TEX_IMAGE___HPP
#define GUI_OPENGL___GL_TEX_IMAGE___HPP


/*  $Id: gltexture.hpp 43126 2019-05-17 18:16:36Z katargir $
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


#include <util/image/image.hpp>
#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/opengl/i3dtexture.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


//
// class CGlTexture defines a specialization of CImage that is useful for
// textures.
//
// This class holds a CImage as well as its texture information.
//
class NCBI_GUIOPENGL_EXPORT CGlTexture : public CObject, public I3DTexture
{
    friend class CGlResMgr;

public:
    // create a texture from a file
    explicit CGlTexture(CNcbiIstream& istr, const string& tag);

public:
    // default ctor
    CGlTexture();

    // create a texture around an existing image
    explicit CGlTexture(CImage* image);

    // create a texture of a given dimension
    CGlTexture(size_t w, size_t h, size_t d);

    // wrap an existing texture given only texture id
    CGlTexture(GLuint tex_id, size_t w, size_t h, GLenum target=GL_TEXTURE_2D);

    // initialize our image.
    // Here, depth is in channels (3 = 24-bit, 4 = 32-bit)
    void Init(size_t w, size_t h, size_t depth);

    // reset our internal structures
    void Clear();

    // "swallow" an image - explicitly assigns the image we wrap
    void Swallow(CImage *image);

    // make this texture the current texture for OpenGL work
    virtual void MakeCurrent();

    // check if texture is valid
    virtual bool IsValid(void) const;

    /// set the gl texture parameters (filtering, etc) for texture
    virtual void SetParams();

    // load an image.  this creates a new display list or tex object
    virtual void Load();

    // Loads 1D texture with RGBA float data
    void Load1DRGBA(size_t width, float* data);

    // unload an image.  this destroys the current binding
    virtual void Unload();

    // access the filename
    const string&   GetImageTag(void) const;

    // access our image
    const CImage*   GetImage(void) const;
    CImage*         SetImage(void);

    //
    // OpenGL specifics
    //

    // filtering - magnification / minification
    virtual void   SetFilterMin(GLenum f);
    virtual void   SetFilterMag(GLenum f);
    GLenum GetFilterMin(void) const;
    GLenum GetFilterMag(void) const;

    // texture wrapping - in two directions, S and T
    virtual void   SetWrapS(GLenum e);
    virtual void   SetWrapT(GLenum e);
    GLenum GetWrapS(void) const;
    GLenum GetWrapT(void) const;

    // texture environment mode
    virtual void   SetTexEnv(GLenum e);
    GLenum GetTexEnv(void) const;

    virtual CImage* GenerateImage();

protected:
    size_t m_Width = 0;
    size_t m_Height = 0;

    // our filename (may be empty)
    string m_ImageTag;

    // the CImage we wrap
    CRef<CImage> m_Image;

    // OpenGL stuff:

    // the texture id - either object or display list
    GLuint m_TexId;

    // GL_TEXTURE_2D, GL_TEXTURE_1D ...
    GLenum m_Target;

    // texture wrapping (S and T directions)
    GLenum m_WrapS;
    GLenum m_WrapT;

    // texture filtering for magnification / minification
    GLenum m_FilterMin;
    GLenum m_FilterMag;

    // texture environment (modulation vs. decal, for example)
    GLint  m_TexEnv;

private:
    // forbidden
    CGlTexture (const CGlTexture&);
    CGlTexture& operator= (const CGlTexture&);
};


//
//
// inline accessors
//
//


inline
CImage* CGlTexture::SetImage(void)
{
    return m_Image;
}


inline
const CImage* CGlTexture::GetImage(void) const
{
    return m_Image;
}


inline
const string& CGlTexture::GetImageTag(void) const
{
    return m_ImageTag;
}


inline
void CGlTexture::SetWrapS(GLenum e)
{
    m_WrapS = e;
}


inline
void CGlTexture::SetWrapT(GLenum e)
{
    m_WrapT = e;
}


inline
void CGlTexture::SetFilterMin(GLenum e)
{
    m_FilterMin = e;
}


inline
void CGlTexture::SetFilterMag(GLenum e)
{
    m_FilterMag = e;
}


inline
void CGlTexture::SetTexEnv(GLenum e)
{
    m_TexEnv = e;
}


inline
GLenum CGlTexture::GetWrapS(void) const
{
    return m_WrapS;
}


inline
GLenum CGlTexture::GetWrapT(void) const
{
    return m_WrapT;
}


inline
GLenum CGlTexture::GetFilterMin(void) const
{
    return m_FilterMin;
}


inline
GLenum CGlTexture::GetFilterMag(void) const
{
    return m_FilterMag;
}


inline
GLenum CGlTexture::GetTexEnv(void) const
{
    return m_TexEnv;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_TEX_IMAGE___HPP
