/*  $Id: glcontainer.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *    CGlContainer - container for OpenGL widgets
 */

#include <ncbi_pch.hpp>
#include <gui/opengl/glcontainer.hpp>


BEGIN_NCBI_SCOPE


CGlContainer::CGlContainer()
{
}


CGlContainer::~CGlContainer()
{
}


void CGlContainer::Draw(void)
{
    if (!IsVisible()) {
        return;
    }

    ITERATE (TWidgetList, it, m_Children) {
        TWidgetRef child = *it;
        if (!child->IsVisible()) {
            continue;
        }
        child->Draw();
    }
}


void CGlContainer::ProcessEvent(CGlEvent& event)
{
    ITERATE (TWidgetList, it, m_Children) {
        TWidgetRef child = *it;
        child->ProcessEvent(event);
    }
}



void CGlContainer::Add(CGlWidget* widget)
{
    m_Children.push_back(TWidgetRef(widget));
}


void CGlContainer::Remove(CGlWidget* widget)
{
    NON_CONST_ITERATE (TWidgetList, it, m_Children) {
        if (widget == it->GetPointer()) {
            m_Children.erase(it);
        }
    }
}


void CGlContainer::Clear(void)
{
    m_Children.clear();
}


size_t CGlContainer::Size(void) const
{
    return m_Children.size();
}


CGlContainer::iterator CGlContainer::begin(void)
{
    return m_Children.begin();
}


CGlContainer::const_iterator CGlContainer::begin(void) const
{
    return m_Children.begin();
}


CGlContainer::iterator CGlContainer::end(void)
{
    return m_Children.end();
}


CGlContainer::const_iterator CGlContainer::end(void) const
{
    return m_Children.end();
}


END_NCBI_SCOPE
