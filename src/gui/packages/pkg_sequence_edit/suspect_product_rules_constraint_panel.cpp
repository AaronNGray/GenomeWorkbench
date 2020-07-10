/*  $Id: suspect_product_rules_constraint_panel.cpp 39746 2017-11-01 12:26:57Z filippov $
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

#include <gui/packages/pkg_sequence_edit/suspect_product_rules_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/suspect_product_rules_add_constraint.hpp>

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CSuspectCompoundConstraintPanel, wxPanel )


/*!
 * CSuspectCompoundConstraintPanel event table definition
 */

BEGIN_EVENT_TABLE( CSuspectCompoundConstraintPanel, wxPanel )

////@begin CSuspectCompoundConstraintPanel event table entries
EVT_BUTTON( ID_CONSTRAINT_ADD, CSuspectCompoundConstraintPanel::OnAddConstraint )
EVT_BUTTON( ID_CONSTRAINT_REMOVE, CSuspectCompoundConstraintPanel::OnRemoveConstraint )
EVT_LISTBOX_DCLICK( ID_CONSTRAINT_LIST, CSuspectCompoundConstraintPanel::OnEditConstraint )
////@end CSuspectCompoundConstraintPanel event table entries

END_EVENT_TABLE()


/*!
 * CSuspectCompoundConstraintPanel constructors
 */

CSuspectCompoundConstraintPanel::CSuspectCompoundConstraintPanel()
{
    Init();
}

CSuspectCompoundConstraintPanel::CSuspectCompoundConstraintPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSuspectCompoundConstraintPanel creator
 */

bool CSuspectCompoundConstraintPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSuspectCompoundConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSuspectCompoundConstraintPanel creation
    return true;
}


/*!
 * CSuspectCompoundConstraintPanel destructor
 */

CSuspectCompoundConstraintPanel::~CSuspectCompoundConstraintPanel()
{
////@begin CSuspectCompoundConstraintPanel destruction
////@end CSuspectCompoundConstraintPanel destruction
}


/*!
 * Member initialisation
 */

void CSuspectCompoundConstraintPanel::Init()
{
////@begin CSuspectCompoundConstraintPanel member initialisation
////@end CSuspectCompoundConstraintPanel member initialisation
    m_CheckListBox = NULL;
}


/*!
 * Control creation for CSuspectCompoundConstraintPanel
 */

void CSuspectCompoundConstraintPanel::CreateControls()
{    
////@begin CSuspectCompoundConstraintPanel content construction
    CSuspectCompoundConstraintPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_CheckListBox = new wxCheckListBox(itemPanel1, ID_CONSTRAINT_LIST, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_CheckListBox, 1, wxGROW|wxALL, 5);
    
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemPanel1, ID_CONSTRAINT_ADD, _("Add Constraint"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemPanel1, ID_CONSTRAINT_REMOVE, _("Clear Constraints"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


void CSuspectCompoundConstraintPanel::ClearValues(void)
{
    m_CheckListBox->Clear();
    m_label_to_constraint.clear();
}

/*!
 * Should we show tooltips?
 */

bool CSuspectCompoundConstraintPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSuspectCompoundConstraintPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSuspectCompoundConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSuspectCompoundConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSuspectCompoundConstraintPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSuspectCompoundConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSuspectCompoundConstraintPanel icon retrieval
}


void CSuspectCompoundConstraintPanel::OnAddConstraint( wxCommandEvent& event )
{
    CSuspectAddConstraint * dlg = new CSuspectAddConstraint(this);
    dlg->Show(true);
}

void CSuspectCompoundConstraintPanel::OnRemoveConstraint( wxCommandEvent& event )
{
    Freeze();
    set<int> to_delete;
    wxArrayInt checked_items;
    m_CheckListBox->GetCheckedItems(checked_items);
    if (checked_items.IsEmpty())
    {
        int answer = wxMessageBox (_("No items checked, remove all?"), _("Warning"), wxYES_NO | wxICON_QUESTION);
        if (answer == wxYES)
        {
            for (size_t i = 0; i < m_CheckListBox->GetCount(); i++)
            {
                string label = m_CheckListBox->GetString(i).ToStdString();
                m_label_to_constraint.erase(label);
                to_delete.insert(i);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < checked_items.GetCount(); i++)
        {
            int j = checked_items.Item(i);
            string label = m_CheckListBox->GetString(j).ToStdString();
            m_label_to_constraint.erase(label);
            to_delete.insert(j);
        }
    }
    for (auto rit=to_delete.rbegin(); rit != to_delete.rend(); ++rit)
    {
        m_CheckListBox->Delete(*rit);
    }
    Thaw();
    Refresh();
}

void CSuspectCompoundConstraintPanel::AddConstraint(pair<string, CRef<objects::CConstraint_choice> > item, int id)
{
    if (id < 0)
    {
        m_CheckListBox->Append(wxString(item.first));
    }
    else
    {
        string label = m_CheckListBox->GetString(id).ToStdString();
        m_label_to_constraint.erase(label);
        m_CheckListBox->SetString(id, wxString(item.first));
    }
    m_label_to_constraint.insert(item);
    Refresh();
}

void CSuspectCompoundConstraintPanel::OnEditConstraint( wxCommandEvent& event )
{
    const string &label = event.GetString().ToStdString();
    int id = event.GetInt();
    CSuspectAddConstraint * dlg = new CSuspectAddConstraint(this);
    dlg->SetEditId(id);
    dlg->SetConstraint(*m_label_to_constraint[label]);
    dlg->Show(true);
}

void CSuspectCompoundConstraintPanel::SetConstraints(const objects::CConstraint_choice_set& constraints)
{
    m_label_to_constraint.clear();
    if (constraints.IsSet())
    {
        for (auto c : constraints.Get())
        {
            string label = CSuspectAddConstraint::GetDescription(*c);
            m_label_to_constraint[label] = c;
            m_CheckListBox->Append(wxString(label));
        }
    }
}

bool CSuspectCompoundConstraintPanel::IsSetConstraints()
{
    return ! m_label_to_constraint.empty();
}

CRef<objects::CConstraint_choice_set> CSuspectCompoundConstraintPanel::GetConstraints()
{
    CRef<objects::CConstraint_choice_set> constraints(new objects::CConstraint_choice_set());
    for (auto &item : m_label_to_constraint)
    {
        CRef<objects::CConstraint_choice> constraint = item.second;
        constraints->Set().push_back(constraint);
    }
    
    return constraints;
}

END_NCBI_SCOPE
