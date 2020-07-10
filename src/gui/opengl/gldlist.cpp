/*  $Id: gldlist.cpp 20736 2010-01-26 20:06:23Z tereshko $
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

#include <ncbi_pch.hpp>
#include <gui/opengl/gldlist.hpp>


BEGIN_NCBI_SCOPE


CGlDisplayList::CGlDisplayList()
    : m_DList(0), m_IsValid(false)
{
}


CGlDisplayList::~CGlDisplayList()
{
    Delete();
}

void CGlDisplayList::x_Init()
{
    if (m_DList && glIsList(m_DList)) {
        glDeleteLists(m_DList, 1);
    }
    m_DList = glGenLists(1);
}


void CGlDisplayList::Begin(GLenum mode)
{
    if ((!m_DList) || (!glIsList(m_DList))) {
        x_Init();
    }

    glNewList(m_DList, mode);
}


void CGlDisplayList::End()
{
    glEndList();
    m_IsValid = true;
}


void CGlDisplayList::Call() const
{
    if (m_DList) {
        glCallList(m_DList);
    }
}

void CGlDisplayList::Delete()
{
    // this may fail if we don't have a valid OpenGL context made current, but
    // we have to risk it.
    if (m_DList && glIsList(m_DList)) {
        glDeleteLists(m_DList, 1);
        m_DList = 0;
    }
    m_IsValid = false;
}

CGlDisplayListCompile::CGlDisplayListCompile(CGlDisplayList& dlist,
                                             GLenum mode)
    : m_DList(dlist)
{
    /// HACK:
    /// ATI Radeon drivers version 8.4XX and above exhibit non-standard behavior:
    /// the driver will leak pixel buffer information associated with glBitmap()
    /// calls if we choose to reuse a display list.
    /// We work around this by deleting the display list first.
    m_DList.Delete();
    m_DList.Begin(mode);
}


CGlDisplayListCompile::~CGlDisplayListCompile()
{
    m_DList.End();
}


END_NCBI_SCOPE
