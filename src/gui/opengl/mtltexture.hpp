#ifndef GUI_OPENGL___MTL_TEXTURE___HPP
#define GUI_OPENGL___MTL_TEXTURE___HPP


/*  $Id: mtltexture.hpp 43126 2019-05-17 18:16:36Z katargir $
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

#include <gui/opengl/i3dtexture.hpp>
#include <util/image/image.hpp>

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


//
// class CMtlTexture defines a specialization of CImage that is useful for
// textures.
//
// This class holds a CImage as well as its texture information.
//
class CMtlTexture : public CObject, public I3DTexture
{
    friend class CGlResMgr;
    friend class CMtlFrameBuffer;

private:
    CMtlTexture();
    CMtlTexture(id<MTLTexture> texture);
    explicit CMtlTexture(CImage* image);

    // create a texture from a file
    explicit CMtlTexture(CNcbiIstream& istr, const string& tag);

public:
    virtual ~CMtlTexture();

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

    //
    // OpenGL specifics
    //

    // filtering - magnification / minification
    virtual void   SetFilterMin(GLenum f) { m_FilterMin = f; }
    virtual void   SetFilterMag(GLenum f) { m_FilterMag = f; }

    // texture wrapping - in two directions, S and T
    virtual void   SetWrapS(GLenum e) { m_WrapS = e; }
    virtual void   SetWrapT(GLenum e) { m_WrapT = e; }

    // texture environment mode
    virtual void   SetTexEnv(GLenum e) { m_TexEnv = e; }

    virtual CImage* GenerateImage() { return nullptr; }

protected:
    // our filename (may be empty)
    string m_ImageTag;

    // the CImage we wrap
    CRef<CImage>    m_Image;

    // OpenGL stuff:

    // texture wrapping (S and T directions)
    GLenum m_WrapS;
    GLenum m_WrapT;

    // texture filtering for magnification / minification
    GLenum m_FilterMin;
    GLenum m_FilterMag;

    // texture environment (modulation vs. decal, for example)
    GLint  m_TexEnv;

    id<MTLTexture> m_Texture;

private:
    // forbidden
    CMtlTexture (const CMtlTexture&);
    CMtlTexture& operator= (const CMtlTexture&);
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___MTL_TEXTURE___HPP
