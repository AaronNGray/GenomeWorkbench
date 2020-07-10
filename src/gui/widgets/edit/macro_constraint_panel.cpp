/*  $Id: macro_constraint_panel.cpp 40627 2018-03-21 15:34:12Z asztalos $
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

#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/pub_field_name_panel.hpp>
#include <gui/widgets/edit/struct_comm_field_panel.hpp>
#include <gui/widgets/edit/struct_fieldvalue_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>

#include <gui/widgets/edit/molinfo_fieldtype.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>
#include <gui/widgets/edit/dblink_fieldtype.hpp>
#include <gui/widgets/edit/misc_fieldtype.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>

#include <gui/widgets/edit/macro_constraint_panel.hpp>
#include <gui/widgets/edit/macro_add_constraint_panel.hpp>
#include <gui/widgets/edit/macro_edit_action_panel.hpp>

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CMacroCompoundConstraintPanel, wxPanel )


/*!
 * CMacroCompoundConstraintPanel event table definition
 */

BEGIN_EVENT_TABLE( CMacroCompoundConstraintPanel, wxPanel )

////@begin CMacroCompoundConstraintPanel event table entries
EVT_BUTTON( ID_CONSTRAINT_ADD, CMacroCompoundConstraintPanel::OnAddConstraint )
EVT_BUTTON( ID_CONSTRAINT_REMOVE, CMacroCompoundConstraintPanel::OnRemoveConstraint )
EVT_LISTBOX_DCLICK( ID_CONSTRAINT_LIST, CMacroCompoundConstraintPanel::OnEditConstraint )
////@end CMacroCompoundConstraintPanel event table entries

END_EVENT_TABLE()


/*!
 * CMacroCompoundConstraintPanel constructors
 */

CMacroCompoundConstraintPanel::CMacroCompoundConstraintPanel()
{
    Init();
}

CMacroCompoundConstraintPanel::CMacroCompoundConstraintPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_num(0)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CMacroCompoundConstraintPanel creator
 */

bool CMacroCompoundConstraintPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroCompoundConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMacroCompoundConstraintPanel creation
    return true;
}


/*!
 * CMacroCompoundConstraintPanel destructor
 */

CMacroCompoundConstraintPanel::~CMacroCompoundConstraintPanel()
{
////@begin CMacroCompoundConstraintPanel destruction
////@end CMacroCompoundConstraintPanel destruction
}


/*!
 * Member initialisation
 */

void CMacroCompoundConstraintPanel::Init()
{
////@begin CMacroCompoundConstraintPanel member initialisation
////@end CMacroCompoundConstraintPanel member initialisation
    m_CheckListBox = NULL;
    m_Add = NULL;
    m_Clear = NULL;
}


/*!
 * Control creation for CMacroCompoundConstraintPanel
 */

void CMacroCompoundConstraintPanel::CreateControls()
{    
////@begin CMacroCompoundConstraintPanel content construction
    CMacroCompoundConstraintPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_CheckListBox = new wxCheckListBox(itemPanel1, ID_CONSTRAINT_LIST, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_CheckListBox, 1, wxGROW|wxALL, 5);
    
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Add = new wxButton( itemPanel1, ID_CONSTRAINT_ADD, _("Add Constraint"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_Add, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_Add->Disable();

    m_Clear = new wxButton( itemPanel1, ID_CONSTRAINT_REMOVE, _("Clear Constraints"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_Clear, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_Clear->Disable();
}


void CMacroCompoundConstraintPanel::ClearValues(bool enable_add)
{
    m_CheckListBox->Clear();
    m_label_to_constraint.clear();
    m_label_to_vars.clear();
    m_label_to_qual_name.clear();
    m_num = 0;
    m_Clear->Disable();
    if (enable_add)
	m_Add->Enable();
    else
	m_Add->Disable();
}

/*!
 * Should we show tooltips?
 */

bool CMacroCompoundConstraintPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMacroCompoundConstraintPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroCompoundConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroCompoundConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMacroCompoundConstraintPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroCompoundConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroCompoundConstraintPanel icon retrieval
}

void CMacroCompoundConstraintPanel::OnAddConstraint( wxCommandEvent& event )
{
    CMacroEditingActionPanel *parent = dynamic_cast<CMacroEditingActionPanel*>(GetParent());
    if (!parent)
        return;
    pair<string, string> target = parent->GetForTarget();
    if (target.first.empty())
        return;
    CMacroAddConstraint * dlg = new CMacroAddConstraint(this, m_TopSeqEntry, target);
    dlg->SetNumConstraints(m_num++);
    dlg->Show(true);
}

void CMacroCompoundConstraintPanel::OnRemoveConstraint( wxCommandEvent& event )
{
    Freeze();
    wxArrayInt checked_items;
    m_CheckListBox->GetCheckedItems(checked_items);
    if (checked_items.IsEmpty())
    {
        int answer = wxMessageBox (_("No items checked, remove all?"), ToWxString("Warning"), wxYES_NO | wxICON_QUESTION);
        if (answer == wxYES)
        {
	    ClearValues(true);
        }
    }
    else
    {
	set<int> to_delete;
        for (size_t i = 0; i < checked_items.GetCount(); i++)
        {
            int j = checked_items.Item(i);
            to_delete.insert(j);
            string label = m_CheckListBox->GetString(j).ToStdString();
            m_label_to_constraint.erase(label);
	    m_label_to_vars.erase(label);
	    m_label_to_qual_name.erase(label);
        }
    
	for (auto rit=to_delete.rbegin(); rit != to_delete.rend(); ++rit)
	{
	    m_CheckListBox->Delete(*rit);
	}
    }
    m_Clear->Enable(!m_label_to_constraint.empty());
    Thaw();
    Refresh();

    CMacroEditingActionPanel *parent = dynamic_cast<CMacroEditingActionPanel*>(GetParent());
    if (parent)
        parent->UpdateParentFrame();
}

void CMacroCompoundConstraintPanel::AddConstraint(pair<string, string> item, int id, const vector<string> &vars, const string &qual_name)
{
    if (id < 0)
    {
        m_CheckListBox->Append(wxString(item.first));
    }
    else
    {
        string label = m_CheckListBox->GetString(id).ToStdString();
        m_label_to_constraint.erase(label);
	m_label_to_vars.erase(label);
	m_label_to_qual_name.erase(label);
        m_CheckListBox->SetString(id, wxString(item.first));
    }
    m_label_to_constraint.insert(item);
    if (!vars.empty())
	m_label_to_vars[item.first] = vars;
    if (!qual_name.empty())
	m_label_to_qual_name[item.first] = qual_name;
    m_Clear->Enable();
    Refresh();

    CMacroEditingActionPanel *parent = dynamic_cast<CMacroEditingActionPanel*>(GetParent());
    if (parent)
        parent->UpdateParentFrame();
}

void CMacroCompoundConstraintPanel::OnEditConstraint( wxCommandEvent& event )
{
// TODO
/*
  CMacroEditingActionPanel *parent = dynamic_cast<CMacroEditingActionPanel*>(GetParent());
  if (!parent)
  return;
  string target = parent->GetForTarget();
  if (target.empty())
  return;
  const string &label = event.GetString().ToStdString();
  int id = event.GetInt();
  CMacroAddConstraint * dlg = new CMacroAddConstraint(this, m_TopSeqEntry, target);
  dlg->SetEditId(id);
  dlg->SetConstraint(m_label_to_constraint[label]);
  dlg->Show(true);
*/
}

void CMacroCompoundConstraintPanel::SetConstraints(const vector<string>& constraints)
{
    ClearValues(true);
    for (auto s : constraints)
    {
        // TODO
        string label = s; // GetDescription
        m_label_to_constraint[label] = s;
        m_CheckListBox->Append(wxString(label));
    }
    m_Clear->Enable();
}

bool CMacroCompoundConstraintPanel::IsSetConstraints()
{
    return ! m_CheckListBox->IsEmpty();
}

vector<pair<string, string> > CMacroCompoundConstraintPanel::GetConstraints()
{
    vector<pair<string, string> > constraints;
    constraints.reserve(m_CheckListBox->GetCount());
    for (size_t i = 0; i < m_CheckListBox->GetCount(); i++)
    {
        string label = m_CheckListBox->GetString(i).ToStdString();
        string constraint = m_label_to_constraint[label];
        string qual_name = m_label_to_qual_name[label];
        if (!constraint.empty())
            constraints.push_back(make_pair(qual_name, constraint));
    }    
    return constraints;
}

vector<string> CMacroCompoundConstraintPanel::GetDescription()
{
    vector<string> constraints;
    constraints.reserve(m_CheckListBox->GetCount());
    for (size_t i = 0; i < m_CheckListBox->GetCount(); i++)
    {
        string label = m_CheckListBox->GetString(i).ToStdString();
        if (!label.empty())
            constraints.push_back(label);
    }    
    return constraints;
}

bool CMacroCompoundConstraintPanel::HasVariables()
 {
     for (size_t i = 0; i < m_CheckListBox->GetCount(); i++)
     {
	 string label = m_CheckListBox->GetString(i).ToStdString();
	 auto it = m_label_to_vars.find(label);
	 if (it != m_label_to_vars.end() && !it->second.empty())
	     return true;
     }    
     return false;
 }
  
string CMacroCompoundConstraintPanel::GetVariables()
{
    string vars;
    for (size_t i = 0; i < m_CheckListBox->GetCount(); i++)
    {
	string label = m_CheckListBox->GetString(i).ToStdString();
	auto it = m_label_to_vars.find(label);
	if (it != m_label_to_vars.end() && !it->second.empty())
	{
	    vars += NStr::Join(it->second, "\n") + "\n";
	}
    }    
    return vars;
}

END_NCBI_SCOPE
