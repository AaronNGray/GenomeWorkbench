#ifndef GUI_OPENGL___GL_UTILS__HPP
#define GUI_OPENGL___GL_UTILS__HPP

/*  $Id: glutils.hpp 39087 2017-07-25 20:39:56Z falkrb $
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


#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/gltypes.hpp>
#include <gui/opengl/glexception.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
//  CGlUtils::
//

class NCBI_GUIOPENGL_EXPORT CGlUtils
{
public:
    /// status of hardware acceleration.
    enum EAccelState {
        eNotDetermined,
        eAccelerated,
        eNotAccelerated
    };

    // diagnostic (error handling) mode
    enum EGlDiagMode {
        eUndefined,
        eIgnore,
        eLogPost,
        eThrow,
        eAbort
    };

    /// Check if there are any OpenGL errors
    static bool CheckGlError();

    /// Get current diagnostic (error handling) mode
    static EGlDiagMode GetDiagnosticMode();

    /// Get message for specified OpenGL error
    static string GetErrMsg(GLint error);

    /// Dump many of the most common OpenGL states
    static void DumpState();

    /// Get the status of hardware acceleration.  This cannot be determined
    /// without an active OpenGL context.
    static EAccelState  GetAccelerated(void);

    /// Set the status of hardware acceleration.  This is provided to permit a
    /// user override of the detected state.
    static void         SetAccelerated(EAccelState acc);

private:

    // hardware acceleration
    static EAccelState m_Accel;

};

/// Same as CGlUtils::CheckGlError except the log message will include
/// the name of the function logging the message (instead of CheckGlError)
#define CHECK_GLERROR() {\
      GLint error = glGetError(); \
      if (error != GL_NO_ERROR) {  \
          static CGlUtils::EGlDiagMode mode = CGlUtils::GetDiagnosticMode(); \
          string msg = CGlUtils::GetErrMsg(error); \
                                         \
          switch (mode) {                \
          case CGlUtils::eLogPost:       \
              LOG_POST(Error << msg);    \
              break;                     \
          case CGlUtils::eAbort:         \
              LOG_POST(Error << msg);    \
              Abort();                   \
              break;                     \
          case CGlUtils::eThrow:         \
              NCBI_THROW(COpenGLException, eGlError, msg); \
              break;                     \
          case CGlUtils::eUndefined:     \
          case CGlUtils::eIgnore:        \
          default:                       \
              break;                     \
          }                              \
      }                                  \
    }

///////////////////////////////////////////////////////////////////////////////
/// CGlAttrGuard - guard class for restoring OpenGL attributes
class CGlAttrGuard
{
public:
    CGlAttrGuard(GLbitfield mask)
    {
        glPushAttrib(mask);
    }
    ~CGlAttrGuard()
    {
        glPopAttrib();
    }
};

#ifdef GLEW_MX
///////////////////////////////////////////////////////////////////////////////
/// CGLGlewContext - Store instance of glew context in a singleton that is
/// available to all opengl libs, not just ones dependent on wx.  When
/// the onctext is created (from a wx window) it can be stored here where it
/// can be accessed by any opengl lib (normally via "GLEWContext*  glewGetContext();"
class NCBI_GUIOPENGL_EXPORT CGLGlewContext
{
public:
    static CGLGlewContext& GetInstance();

    GLEWContext* GetGlewContext() { return m_GLEWContext; }
    void SetGlewContext(GLEWContext* gc) { m_GLEWContext = gc; }

private:
    CGLGlewContext() : m_GLEWContext(NULL) {}
    GLEWContext*  m_GLEWContext;
};
#endif

///////////////////////////////////////////////////////////////////////////////
///

inline void glRectC(const TModelRect& rc)
{
    glRectd(rc.Left(), rc.Bottom(), rc.Right(), rc.Top());
}


inline void glRectC(const TVPRect& rc)
{
    glRecti(rc.Left(), rc.Bottom(), rc.Right(), rc.Top());
}


inline void glColorC(const CRgbaColor& color)
{
    glColor4fv(color.GetColorArray());
}


///////////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG

//
// in release mode, this function does nothing
//
inline
void CGlUtils::DumpState(void)
{
}

#endif


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_UTILS__HPP
