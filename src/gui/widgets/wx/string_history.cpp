/*  $Id: string_history.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Roman  Katargin
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/string_history.hpp>
#include <gui/widgets/wx/cont_text_completer.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/registry.hpp>

BEGIN_NCBI_SCOPE

static const char* kStringHistory = "StringHistory";

CStringHistory::CStringHistory(size_t maxSize, const char* regPath)
    : m_Initialized(false), m_MaxSize(maxSize), m_RegPath(regPath)
{
}

void CStringHistory::Load()
{
    if (m_Initialized)
        return;

    _ASSERT(!m_RegPath.empty());
    if (m_RegPath.empty())
        return;

    CRegistryReadView view = CGuiRegistry::GetInstance().GetReadView(m_RegPath);

    list<string> history;
    view.GetStringList(kStringHistory, history);
    ITERATE(list<string>, it, history)
        push_back(ToWxString(*it));
    m_Initialized = true;
}

void CStringHistory::Save()
{
    if (!m_Initialized)
        return;

    _ASSERT(!m_RegPath.empty());
    if (m_RegPath.empty())
        return;

    CRegistryWriteView view = CGuiRegistry::GetInstance().GetWriteView(m_RegPath);

    list<string> history;
    ITERATE(list<wxString>, it, *this)
        history.push_back(ToStdString(*it));
    view.Set(kStringHistory, history);
}

wxTextCompleter* CStringHistory::CreateCompleter()
{
    if (!m_Initialized)
        Load();

    return new CContTextCompleter<list<wxString> >(*this);
}

void CStringHistory::AddString(const wxString& str)
{
    if (!m_Initialized)
        Load();
    if (str.empty())
        return;

    for (iterator it = this->begin(); it != this->end();) {
        if (*it == str)
            it = erase(it);
        else
            ++it;
    }

    push_front(str);
    if (size() > m_MaxSize)
        resize(m_MaxSize);

    Save();
}

END_NCBI_SCOPE
