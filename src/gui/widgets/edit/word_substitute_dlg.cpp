/*  $Id: word_substitute_dlg.cpp 39739 2017-10-31 18:00:13Z filippov $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <objects/macro/Word_substitution_set.hpp>

#include <gui/widgets/edit/word_substitute_dlg.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/statline.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CWordSubstitutionDlg, wxFrame )

BEGIN_EVENT_TABLE( CWordSubstitutionDlg, wxFrame )

////@begin CWordSubstitutionDlg event table entries
    EVT_BUTTON( wxID_OK, CWordSubstitutionDlg::OnAccept )
    EVT_BUTTON( wxID_CANCEL, CWordSubstitutionDlg::OnCancel )
    EVT_BUTTON( ID_WORDSUBST_ADD, CWordSubstitutionDlg::OnAdd )
    EVT_BUTTON( ID_WORDSUBST_REMOVE, CWordSubstitutionDlg::OnRemove )
////@end CWordSubstitutionDlg event table entries

END_EVENT_TABLE()

CWordSubstitutionDlg::CWordSubstitutionDlg()
{
    Init();
}

CWordSubstitutionDlg::CWordSubstitutionDlg( wxWindow* parent, CRef<objects::CWord_substitution_set> word_subst, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
    InitSubstitutions(word_subst);
}

bool CWordSubstitutionDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CWordSubstitutionDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CWordSubstitutionDlg creation
    return true;
}

CWordSubstitutionDlg::~CWordSubstitutionDlg()
{
////@begin CWordSubstitutionDlg destruction
////@end CWordSubstitutionDlg destruction
}

void CWordSubstitutionDlg::Init()
{
////@begin CWordSubstitutionDlg member initialisation
    m_Pattern = NULL;
    m_Subst = NULL;
    m_IgnoreCase = NULL;
    m_WholeWord = NULL;
    m_CheckListBox = NULL;
////@end CWordSubstitutionDlg member initialisation
}

void CWordSubstitutionDlg::CreateControls()
{    
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer1);

    wxPanel* itemCBulkCmdDlg1 = new wxPanel(this, wxID_ANY);
    itemBoxSizer1->Add(itemCBulkCmdDlg1, 1, wxGROW, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("Pattern Word"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALL, 5);

    m_Pattern = new wxTextCtrl(itemCBulkCmdDlg1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, -1), 0);
    itemBoxSizer4->Add(m_Pattern, 0, wxALIGN_LEFT|wxALL, 5);

    m_Subst = new wxTextCtrl( itemCBulkCmdDlg1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, 90), wxTE_MULTILINE );
    itemBoxSizer2->Add(m_Subst, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_IgnoreCase = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Ignore Case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreCase->SetValue(true);
    itemBoxSizer12->Add(m_IgnoreCase, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WholeWord = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Whole Word"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WholeWord->SetValue(false);
    itemBoxSizer12->Add(m_WholeWord, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemCBulkCmdDlg1, ID_WORDSUBST_ADD, _("Add Word Substitution"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticLine *line1 = new wxStaticLine (itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    itemBoxSizer2->Add(line1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxSHAPED, 5);

    m_CheckListBox = new wxCheckListBox(itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_CheckListBox, 1, wxGROW|wxALL, 5);
    
    wxButton* itemButton14 = new wxButton( itemCBulkCmdDlg1, ID_WORDSUBST_REMOVE, _("Remove Word Substitution"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
   
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxButton* itemButton15 = new wxButton( itemCBulkCmdDlg1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemCBulkCmdDlg1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

}

bool CWordSubstitutionDlg::ShowToolTips()
{
    return true;
}
wxBitmap CWordSubstitutionDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWordSubstitutionDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CWordSubstitutionDlg bitmap retrieval
}
wxIcon CWordSubstitutionDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWordSubstitutionDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CWordSubstitutionDlg icon retrieval
}

void CWordSubstitutionDlg::OnCancel( wxCommandEvent& event )
{
    Close();
}

void CWordSubstitutionDlg::OnAccept( wxCommandEvent& event )
{
    CRef<objects::CWord_substitution_set> word_subst(new objects::CWord_substitution_set);
    for (auto item : m_label_to_subst)
    {
        CRef<CWord_substitution> ws = item.second;
        word_subst->Set().push_back(ws);
    }

    wxWindow* parent = GetParent();
    CWordSubstitutionSetConsumer* panel = dynamic_cast<CWordSubstitutionSetConsumer*>(parent);
    panel->AddWordSubstSet(word_subst);
    Close();
}

void CWordSubstitutionDlg::OnAdd( wxCommandEvent& event )
{
    string pattern = NStr::TruncateSpaces(m_Pattern->GetValue().ToStdString());
    string syn = NStr::TruncateSpaces(m_Subst->GetValue().ToStdString());
    if (pattern.empty() || syn.empty())
        return;

    CRef<CWord_substitution> ws(new CWord_substitution);
    ws->SetWord(pattern);

    vector<string> lines;
    NStr::Split(syn, "\n", lines);
    
    for (auto& l : lines)
    {
        NStr::ReplaceInPlace(l, "\r", "");
        string str = NStr::TruncateSpaces(l);
        if (!str.empty())
            ws->SetSynonyms().push_back(str);
    }

    ws->SetCase_sensitive(!m_IgnoreCase->GetValue());
    ws->SetWhole_word(m_WholeWord->GetValue());

    string label = GetDescription(ws);
    if (!label.empty() && m_label_to_subst.find(label) == m_label_to_subst.end())
    {
        m_label_to_subst[label] = ws;
        m_CheckListBox->Append(label);
        Refresh();
    }
}

void CWordSubstitutionDlg::OnRemove( wxCommandEvent& event )
{
    Freeze();
    wxArrayInt checked_items;
    m_CheckListBox->GetCheckedItems(checked_items);
    set<int> to_delete;
    for (size_t i = 0; i < checked_items.GetCount(); i++)
    {
        int j = checked_items.Item(i);
        string label = m_CheckListBox->GetString(j).ToStdString();
        m_label_to_subst.erase(label);
        to_delete.insert(j);
    }
    for (auto rit=to_delete.rbegin(); rit != to_delete.rend(); ++rit)
    {
        m_CheckListBox->Delete(*rit);
    }
    Thaw();
    Refresh();
}

string CWordSubstitutionDlg::GetDescription(CRef<CWord_substitution> ws)
{
    string str;
    if (ws && ws->IsSetWord() && ws->IsSetSynonyms() && !ws->GetSynonyms().empty())
    {
        str = "allow " + ws->GetWord() + " to be replaced by " + NStr::Join(ws->GetSynonyms(), ", ");

        if (ws->IsSetCase_sensitive() && ws->GetCase_sensitive())
            str += ", case-sensitive";
        if (ws->IsSetWhole_word() && ws->GetWhole_word())
            str += ", whole word";
    }
    return str;
}

void CWordSubstitutionDlg::InitSubstitutions(CRef<objects::CWord_substitution_set> word_subst)
{
    if (word_subst && word_subst->IsSet())
    {
        for (auto ws : word_subst->Set())
        {
            string label = GetDescription(ws);
            m_label_to_subst[label] = ws;
            m_CheckListBox->Append(label);
        }
    }
    Refresh();
}


END_NCBI_SCOPE
