/*  $Id: auto_complete_text_ctrl.cpp 25995 2012-06-22 16:44:18Z katargir $
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

#include <gui/widgets/wx/auto_complete_text_ctrl.hpp>

BEGIN_NCBI_SCOPE

#ifndef __WXMSW__

BEGIN_EVENT_TABLE(CAutoCompleteTextCtrl, wxSearchCtrl)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_ANY, CAutoCompleteTextCtrl::OnSearch)
    EVT_TEXT(wxID_ANY, CAutoCompleteTextCtrl::OnTextChange)
    EVT_TIMER(-1, CAutoCompleteTextCtrl::OnTimer)
END_EVENT_TABLE()

CAutoCompleteTextCtrl::~CAutoCompleteTextCtrl()
{
    delete m_Completer;
}

void CAutoCompleteTextCtrl::Init()
{
}

bool CAutoCompleteTextCtrl::Create(
    wxWindow *parent,
    wxWindowID id,
    const wxString& value,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxValidator& validator,
    const wxString& name)
{
    return wxSearchCtrl::Create(parent, id, value, pos, size, style, validator, name);
}

bool CAutoCompleteTextCtrl::AutoComplete(wxTextCompleter *completer)
{
    delete m_Completer;
    m_Completer = completer;
    m_Prefix.clear();
    return true;
}

void CAutoCompleteTextCtrl::OnTextChange(wxCommandEvent& event)
{
    m_Prefix.clear();
}

void CAutoCompleteTextCtrl::OnSearch(wxCommandEvent&)
{
    // After calling this function SetFocus called
    // on Text control which causes selection of the whole text
    // and prevents selecting what we want
    m_Timer.Start(10, true);
}

void CAutoCompleteTextCtrl::OnTimer(wxTimerEvent&)
{
    if (!m_Completer)
        return;

    if (m_Prefix.empty()) {
        m_Prefix = GetValue();
        if (m_Prefix.empty())
            return;
        
        if (!m_Completer->Start(m_Prefix)) {
            m_Prefix.clear();
            return;
        }
    }

    wxString value = m_Completer->GetNext();
    if (value.empty()) {
        value = m_Prefix;
        m_Prefix.clear();
    }

    wxString savePrefix = m_Prefix;
    SetValue(value);
    m_Prefix = savePrefix;
    SetSelection((long)m_Prefix.length(), (long)value.length());
}

#endif

END_NCBI_SCOPE
