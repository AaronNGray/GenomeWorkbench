#ifndef GUI_OPENGL___GLDLIST__HPP
#define GUI_OPENGL___GLDLIST__HPP

/*  $Id: gldlist.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
 *    CGlDisplayList        - manager for a single display list
 *    CGlDisplayListCompile - utility class to permit scoped creation of
 *                            display lists.
 */

#include <corelib/ncbiobj.hpp>
#include <gui/opengl.h>
#include <gui/gui.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */


BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////
// CGlDisplayList::
//   Wrapper for an OpenGL display list.  This class maintains (in addition)
//   flags indicating whether the current display list should be re-created at
//   the next attempt to render.

class NCBI_GUIOPENGL_EXPORT CGlDisplayList : public CObject
{
public:
    // Modes for creation
    // These are straight from OpenGL - one simply remembers the commands, the
    // other remebers the commands and performs their actions.
    enum EMode {
        eCompile            = GL_COMPILE,
        eCompileAndExecute  = GL_COMPILE_AND_EXECUTE
    };

    // ctor/dtor
    CGlDisplayList();
    ~CGlDisplayList();

    // Draw the display list
    void Call() const;

    // Begin/End compilation of the display list
    void Begin(GLenum mode);
    void End();

    // Get/Set the current valid flag.  This can be used to mark a display list
    // for re-creation in a drawing loop.
    bool IsValid(void) const        { return m_IsValid; }
    void Invalidate(void);

    // this
    void Delete();
private:
    GLuint m_DList;
    bool   m_IsValid;

    // initialize our display list, if it isn't already
    void x_Init();

    // copying prohibited
    CGlDisplayList(const CGlDisplayList&);
    CGlDisplayList& operator= (const CGlDisplayList&);
};


inline
void CGlDisplayList::Invalidate(void)
{
    // we simply mark invalid - we can't delete our display list until we have
    // a valid GL context
    m_IsValid = false;
}


/////////////////////////////////////////////////////////////////////////
// CGlDisplayListCompile::
//  This class mediates display list creation and compilation
//  Creation and compilation can be accomplished via scoping a display list
//  compiler:
//
//  void Draw()
//  {
//      if (m_DList.IsValid()) {
//          m_DList.Call();
//      } else {
//          CGlDisplayListCompile COMPILE(m_DList,
//                                        CGlDisplayList::eCompileAndExecute);
//
//          [...rendering commands...]
//      }
//  }

class NCBI_GUIOPENGL_EXPORT CGlDisplayListCompile
{
public:
    typedef CGlDisplayList::EMode EMode;

    CGlDisplayListCompile(CGlDisplayList& dlist, GLenum mode = GL_COMPILE);
    ~CGlDisplayListCompile();

private:
    CGlDisplayList& m_DList;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GLDLIST__HPP
