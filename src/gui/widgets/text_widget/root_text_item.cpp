/*  $Id: root_text_item.cpp 28692 2013-08-19 16:38:23Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/text_widget/root_text_item.hpp>

BEGIN_NCBI_SCOPE

CRootTextItem::~CRootTextItem()
{
    ITERATE(TContainer, it, m_UserObjects) {
        delete it->second;
    }
}

void CRootTextItem::AddUserObject(const string& name, wxObject* obj)
{
    TContainer::iterator it = m_UserObjects.find(name);
    if (it != m_UserObjects.end()) {
        delete it->second;
    }
    m_UserObjects[name] = obj;
}

wxObject* CRootTextItem::GetUserObject(const string& name)
{
    TContainer::iterator it = m_UserObjects.find(name);
    if (it != m_UserObjects.end())
        return it->second;
    return 0;
}

END_NCBI_SCOPE
