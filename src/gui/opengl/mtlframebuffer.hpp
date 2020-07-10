#ifndef GUI_OPENGL___MTL_FRAMEBUFFER___HPP
#define GUI_OPENGL___MTL_FRAMEBUFFER___HPP


/*  $Id: mtlframebuffer.hpp 42756 2019-04-10 16:43:22Z katargir $
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


#include <corelib/ncbiobj.hpp>

#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/opengl/i3dframebuffer.hpp>

#include "mtltexture.hpp"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// CMtlFrameBuffer

class NCBI_GUIOPENGL_EXPORT CMtlFrameBuffer : public CObject, public I3DFrameBuffer
{
friend class CGlResMgr;

private:
    CMtlFrameBuffer(size_t dim);

public:
    /// Returns the size of the framebuffer (w==h, so only return 1 value)
    virtual size_t GetFrameSize() const { return m_FrameSize; }

    virtual void SetTextureFiltering(GLint min_filter, GLint mag_filter);

    virtual void CreateFrameBuffer();
    virtual void SetClearColor(float red, float green, float blue, float alpha);
    virtual bool IsValid();
    virtual void Render(std::function<void()> renderer);

    virtual void GenerateMipMaps();

    virtual I3DTexture* Get3DTexture();

    virtual void ReleaseTexture() {}

private:
    /// Size of framebuffer (in both dimensions).
    size_t  m_FrameSize;

    /// minification filter
    GLint m_TexMin;
    /// magnification filter
    GLint m_TexMag;

    CRef<CMtlTexture> m_Texture;

    float m_ClearColorRed;
    float m_ClearColorGreen;
    float m_ClearColorBlue;
    float m_ClearColorAlpha;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___MTL_TEXTURE___HPP
