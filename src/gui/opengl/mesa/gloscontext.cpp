/*  $Id: gloscontext.cpp 40252 2018-01-17 20:02:10Z shkeda $
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

#include <ncbi_pch.hpp>
#include <gui/opengl/mesa/gloscontext.hpp>

BEGIN_NCBI_SCOPE

//
// CGlOsContext ctor
// initialize our renderer without a frame buffer.  This allows
// multiple frame buffers to be used. 
CGlOsContext::CGlOsContext(unsigned stencilBits)
{
    x_InitContext(stencilBits);
}

//
// CGlOsContext ctor
// initialize our renderer with a virtual frame buffer of a given size.
// This frame buffer will always be initialized as an RGBA image.
CGlOsContext::CGlOsContext(size_t width, size_t height, unsigned stencilBits)
    : m_Image(new CImage(width, height, 4))
{
    x_InitContext(stencilBits);
}

//
// Called by ctors for context initialization
void CGlOsContext::x_InitContext(unsigned stencilBits)
{
    // create our off-screen mesa rendering context
    // this context will use the screen image we've defined
#if OSMESA_MAJOR_VERSION * 100 + OSMESA_MINOR_VERSION >= 305
    m_Ctx = OSMesaCreateContextExt( OSMESA_RGBA, 16, stencilBits, 0, NULL );
#else
    m_Ctx = OSMesaCreateContext( OSMESA_RGBA, NULL );
#endif

    if ( !m_Ctx ) {
        LOG_POST(Error << "CGlOsContext(): "
                 "Failed to create off-screen rendering context");
    }
}

// destructor for cleaning up our OSMesa context
CGlOsContext::~CGlOsContext()
{
    if (m_Ctx) {
        OSMesaDestroyContext(m_Ctx);
    }
}


// make the current frame buffer the active buffer for rendering
bool CGlOsContext::MakeCurrent(void)
{
    if ( !m_Ctx ) {
        LOG_POST(Error << "CGlOsContext::MakeCurrent(): "
                 "Attempt to make an invalid context current");
        return false;
    }

    if ( !OSMesaMakeCurrent(m_Ctx, m_Image->SetData(), GL_UNSIGNED_BYTE,
                            m_Image->GetWidth(), m_Image->GetHeight()) ) {
        LOG_POST(Error << "CGlOsContext::MakeCurrent(): "
                 "Failed to make image surface current (w:" << m_Image->GetWidth() << ", h:" << m_Image->GetHeight() << ")");
        return false;
    }

    return true;
}

// make the current frame buffer the active buffer for rendering
bool CGlOsContext::MakeCurrent(size_t width, size_t height)
{
    m_Image->Init(width, height, 4);

    if ( !m_Ctx ) {
        LOG_POST(Error << "CGlOsContext::MakeCurrent(): "
                 "Attempt to make an invalid context current");
        return false;
    }

    if ( !OSMesaMakeCurrent(m_Ctx, m_Image->SetData(), GL_UNSIGNED_BYTE,
                            m_Image->GetWidth(), m_Image->GetHeight()) ) {
        LOG_POST(Error << "CGlOsContext::MakeCurrent(): "
                 "Failed to make image surface current");
        return false;
    }

    return true;
}

// make the current frame buffer the active buffer for rendering
bool CGlOsContext::MakeCurrent(CImage* img)
{
    if ( !m_Ctx ) {
        LOG_POST(Error << "CGlOsContext::MakeCurrent(): "
                 "Attempt to make an invalid context current");
        return false;
    }

    m_Image.Reset(img);

    if ( !OSMesaMakeCurrent(m_Ctx, m_Image->SetData(), GL_UNSIGNED_BYTE,
                            m_Image->GetWidth(), m_Image->GetHeight()) ) {
        LOG_POST(Error << "CGlOsContext::MakeCurrent(): "
                 "Failed to make image surface current");
        return false;
    }

    return true;
}

void CGlOsContext::GetMaximumBufferSize(GLint& max_width, GLint& max_height) 
{
    OSMesaGetIntegerv( OSMESA_MAX_WIDTH, &max_width );
    OSMesaGetIntegerv( OSMESA_MAX_HEIGHT, &max_height );
}

void CGlOsContext::SetPixelStore(GLint pname, GLint value)
{
    OSMesaPixelStore(pname, value);
}

// access the image we use for the virtual frame buffer
const CImage& CGlOsContext::GetBuffer(void) const
{
    return *m_Image;
}


// access the image we use for the virtual frame buffer
CImage& CGlOsContext::SetBuffer(void)
{
    return *m_Image;
}

void CGlOsContext::Resize(size_t width, size_t height)
{
    m_Image.Reset(new CImage(width, height, 4));
}


END_NCBI_SCOPE
