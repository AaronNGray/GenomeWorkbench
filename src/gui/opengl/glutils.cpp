/*  $Id: glutils.cpp 39087 2017-07-25 20:39:56Z falkrb $
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
 *    OpenGL utility functions
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbi_system.hpp>
#include <gui/opengl.h>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glexception.hpp>

#ifdef GLEW_MX
GLEWContext*  glewGetContext()  
{ 
    return NCBI_NS_NCBI::CGLGlewContext::GetInstance().GetGlewContext();
}
#endif

BEGIN_NCBI_SCOPE

// static accelerated state - default to 'not determined'
CGlUtils::EAccelState CGlUtils::m_Accel = CGlUtils::eNotDetermined;



#ifdef GLEW_MX
CGLGlewContext& CGLGlewContext::GetInstance()
{
    static CGLGlewContext instance;
    return instance;
}
#endif


//
// access the accelerated flag
// this defaults to autodetect, and is user-overridable
//
CGlUtils::EAccelState CGlUtils::GetAccelerated(void)
{
    if ( m_Accel != eNotDetermined ) {
        return m_Accel;
    }

    const char* str = reinterpret_cast<const char*> (glGetString(GL_RENDERER));
    if ( !str ) {
        return eNotDetermined;
    }

    _TRACE("GL_VERSION = "    << glGetString(GL_VERSION));
    _TRACE("GL_RENDERER = "   << glGetString(GL_RENDERER));
    _TRACE("GL_EXTENSIONS = " << glGetString(GL_EXTENSIONS));

    // Solaris software renderer returns:
    // GL_RENDERER = Sun dpa software renderer, VIS
    string s(str);
    if (s.find("software renderer") != string::npos) {
        LOG_POST(Info
                 << "CGlUtils::GetAccelerated(): "
                 "auto-detected non-hardware-accelerated platform");
        m_Accel = eNotAccelerated;
    } else {
        LOG_POST(Info
                 << "CGlUtils::GetAccelerated(): "
                 "auto-detected hardware-accelerated platform");
        m_Accel = eAccelerated;
    }

    return m_Accel;
}


//
// check for and report OpenGL errors
//
CGlUtils::EGlDiagMode CGlUtils::GetDiagnosticMode()
{
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

    return mode;
}

string CGlUtils::GetErrMsg(GLint error)
{
    string msg;
    switch ( error ) {
    default:
        msg = "CGlUtils::CheckGlError(): unknown error";
        break;

    case GL_INVALID_OPERATION:
        msg = "CGlUtils::CheckGlError(): invalid operation";
        break;

    case GL_INVALID_ENUM:
        msg = "CGlUtils::CheckGlError(): invalid enum";
        break;

    case GL_INVALID_VALUE:
        msg = "CGlUtils::CheckGlError(): invalid value";
        break;

    case GL_STACK_OVERFLOW:
        msg = "CGlUtils::CheckGlError(): stack overflow";
        break;

    case GL_STACK_UNDERFLOW:
        msg = "CGlUtils::CheckGlError(): stack underflow";
        break;

    case GL_OUT_OF_MEMORY:
        msg = "CGlUtils::CheckGlError(): out of memory";
        break;
    }
    
    return msg;
}

bool CGlUtils::CheckGlError(void)
{
    GLint error = glGetError();
    if (error == GL_NO_ERROR) {
        return false;
    }

    static EGlDiagMode mode = GetDiagnosticMode();
    string msg = GetErrMsg(error);
 
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

    case eThrow:
        // throw
        NCBI_THROW(COpenGLException, eGlError, msg);
        break;
    }

    return true;
}


#ifdef _DEBUG

//
// dumpState()
// this is a debugging function designed to show a bunch of OpenGL enables
//
void CGlUtils::DumpState(void)
{
    LOG_POST(Info << "OpenGL Vendor: " << glGetString(GL_VENDOR));
    LOG_POST(Info << "OpenGL Renderer: " << glGetString(GL_RENDERER));
    LOG_POST(Info << "OpenGL Version: " << glGetString(GL_VERSION));
    LOG_POST(Info << "OpenGL Extensions: " << glGetString(GL_EXTENSIONS));
    LOG_POST(Info << "\n");

    GLint viewport[4];
    float modelview[16];
    float projection[16];
    float color[4];

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glGetFloatv(GL_CURRENT_COLOR, color);

    GLint red_bits = 0;
    GLint green_bits = 0;
    GLint blue_bits = 0;
    GLint alpha_bits = 0;
    GLint depth_bits = 0;
    GLint stencil_bits = 0;
    glGetIntegerv(GL_RED_BITS,     &red_bits);
    glGetIntegerv(GL_GREEN_BITS,   &green_bits);
    glGetIntegerv(GL_BLUE_BITS,    &blue_bits);
    glGetIntegerv(GL_ALPHA_BITS,   &alpha_bits);
    glGetIntegerv(GL_DEPTH_BITS,   &depth_bits);
    glGetIntegerv(GL_STENCIL_BITS, &stencil_bits);

    LOG_POST(Info << "Buffers:");
    LOG_POST(Info << "  Color:"
             << " Red=" << red_bits << " bits"
             << " Green=" << green_bits << " bits"
             << " Blue=" << blue_bits << " bits"
             << " Alpha=" << alpha_bits << " bits");
    LOG_POST(Info << "  Depth: " << depth_bits << " bits");
    LOG_POST(Info << "  Stencil: " << alpha_bits << " bits");

    LOG_POST(Info << "Viewport: "
        << viewport[0] << ", " << viewport[1] << ", "
        << viewport[2] << ", " << viewport[3]);

    int i;
    int j;
    LOG_POST(Info << "Projection matrix:");
    for (i = 0;  i < 4;  ++i) {
        string msg;
        for (j = 0;  j < 4;  ++j) {
            // remember, OpenGL matrices are transposed!
            msg += NStr::DoubleToString(projection[j * 4+i]) + " ";
        }
        LOG_POST(Info << msg);
    }

    LOG_POST(Info << "Modelview matrix:");
    for (i = 0;  i < 4;  ++i) {
        string msg;
        for (j = 0;  j < 4;  ++j) {
            // remember, OpenGL matrices are transposed!
            msg += NStr::DoubleToString(modelview[j * 4+i]) + " ";
        }
        LOG_POST(Info << msg);
    }

    LOG_POST(Info << "Current draw color: "
        << color[0] << ", " << color[1] << ", "
        << color[2] << ", " << color[3]);

    LOG_POST(Info << "Lighting:      "
             << (glIsEnabled(GL_LIGHTING)   ? "enabled" : "disabled"));
    LOG_POST(Info << "Depth Testing: "
             << (glIsEnabled(GL_DEPTH_TEST) ? "enabled" : "disabled"));
    LOG_POST(Info << "Face Culling:  "
             << (glIsEnabled(GL_CULL_FACE)  ? "enabled" : "disabled"));
    LOG_POST(Info << "Blending:      "
             << (glIsEnabled(GL_BLEND)      ? "enabled" : "disabled"));
    LOG_POST(Info << "Alpha Testing: "
             << (glIsEnabled(GL_ALPHA_TEST) ? "enabled" : "disabled"));
    LOG_POST(Info << "2D Texture:    "
             << (glIsEnabled(GL_TEXTURE_2D) ? "enabled" : "disabled"));
}

#endif

END_NCBI_SCOPE
