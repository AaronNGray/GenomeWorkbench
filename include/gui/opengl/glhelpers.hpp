#ifndef GUI_OPENGL___GL_HELPERS__HPP
#define GUI_OPENGL___GL_HELPERS__HPP

/*  $Id: glhelpers.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <gui/opengl/glutils.hpp>

#include <gui/opengl.h>
#include <gui/opengl/glpane.hpp>

#if 0
/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

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


END_NCBI_SCOPE

/* @} */
#endif

#endif  // GUI_OPENGL___GL_HELPERS__HPP
