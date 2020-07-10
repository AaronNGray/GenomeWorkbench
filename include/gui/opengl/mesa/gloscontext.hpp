#ifndef __GL_OS_RENDERER__HPP
#define __GL_OS_RENDERER__HPP
/*  $Id: gloscontext.hpp 34011 2015-10-20 14:48:52Z falkrb $
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
 *    CGlOsContext -- wrapper for Mesa3D's off-screen rendering extension
 */


#include <corelib/ncbiobj.hpp>
#include <util/image/image.hpp>

#include <GL/glew.h>

// GLEW undefines GLAPI and GLAPIENTRY which causes issues with mesa, which 
// normally includes gl.h first, and then expects these macros to continue
// to be defined afterward. More up-to-date versions of glew (1.5.2 and above) may
// resolve this issue and allow this code to be removed.
#ifdef NCBI_OS_UNIX
#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#ifndef GLAPI
#define GLAPI extern
#endif
#endif

#include <GL/osmesa.h>

BEGIN_NCBI_SCOPE


//
// class CGlOsContext defines a simple interface for off-screen rendering.
//
// The current implementation depends on using the Off-Screen Mesa
// implementation that comes standard with the Mesa3D package, available at
// http://www.mesa3d.org/.  This interface provides for a simple virtualized
// frame-buffer.  The user establishes the buffer on their own; Mesa3D then
// uses this image as its framebuffer.
//

class CGlOsContext : public CObject
{
public:
    // initialize without a framebuffer. Caller will have to pass
    // an image to MakeCurrent (allows one redering context to 
    // render to multiple framebuffers)
    CGlOsContext(unsigned stencilBits = 0);

    // initialize our renderer with a virtual frame buffer of a given size.
    // This frame buffer will always be initialized as an RGBA image.
    CGlOsContext(size_t width, size_t height, unsigned stencilBits = 0);

    // destructor for cleaning up our OSMesa context
    virtual ~CGlOsContext();

    // make the current frame buffer the active buffer for rendering
    bool MakeCurrent(void);
    bool MakeCurrent(size_t width, size_t height);

    // make context current with passed-in frame buffer
    bool MakeCurrent(CImage* img);

    // return the maximum size buffer that can be created
    void GetMaximumBufferSize(GLint& max_width, GLint& max_height);

    // Set pixel store options via OSMesaPixelStore
    void SetPixelStore(GLint pname, GLint value);

    // access the image we use for the virtual frame buffer
    const CImage& GetBuffer(void) const;
    CImage&       SetBuffer(void);

    /// Resize frame buffer size.
    void Resize(size_t width, size_t height);

private:
    void x_InitContext(unsigned stencilBits);

    // our image
    CRef<CImage> m_Image;

    // the OSMesa context.  This is a hook to Mesa3D's internals.
    OSMesaContext m_Ctx;
};


END_NCBI_SCOPE

#endif  // __GL_OS_RENDERER__HPP
