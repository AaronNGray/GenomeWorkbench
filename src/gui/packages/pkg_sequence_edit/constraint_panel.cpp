/*  $Id: constraint_panel.cpp 45101 2020-05-29 20:53:24Z asztalos $
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

#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/misc_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/gene_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/protein_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/molinfoedit_util.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/pub_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/pub_field.hpp>
#include <gui/packages/pkg_sequence_edit/dblink_field.hpp>
#include <gui/packages/pkg_sequence_edit/generalid_panel.hpp>
#include <gui/widgets/edit/struct_comm_field_panel.hpp>
#include <gui/widgets/edit/struct_fieldvalue_panel.hpp>
#include <gui/packages/pkg_sequence_edit/dbxref_name_panel.hpp>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/aecr_frame.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/add_constraint_panel.hpp>


BEGIN_NCBI_SCOPE



// Constraint Panel
IMPLEMENT_DYNAMIC_CLASS( CConstraintPanel, wxPanel )


/*!
 * CConstraintPanel event table definition
 */

BEGIN_EVENT_TABLE( CConstraintPanel, wxPanel )

////@begin CConstraintPanel event table entries
EVT_CHOICEBOOK_PAGE_CHANGED(ID_CONSTRAINT_CHOICEBOOK, CConstraintPanel::OnPageChanged)
////@end CConstraintPanel event table entries

END_EVENT_TABLE()


/*!
 * CConstraintPanel constructors
 */

CConstraintPanel::CConstraintPanel()
{
    Init();
}

CConstraintPanel::CConstraintPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, vector<const objects::CFeatListItem *> *featlist, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_featlist(featlist)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CConstraintPanel creator
 */

bool CConstraintPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CConstraintPanel creation
    return true;
}


/*!
 * CConstraintPanel destructor
 */

CConstraintPanel::~CConstraintPanel()
{
////@begin CConstraintPanel destruction
////@end CConstraintPanel destruction
}


/*!
 * Member initialisation
 */

void CConstraintPanel::Init()
{
////@begin CConstraintPanel member initialisation
////@end CConstraintPanel member initialisation
    m_Notebook = NULL;
    m_StringConstraintPanel = NULL;
    m_feat_field_panel = NULL;
}


/*!
 * Control creation for CConstraintPanel
 */

void CConstraintPanel::CreateControls()
{    
////@begin CConstraintPanel content construction
    CConstraintPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Notebook = new wxChoicebook(itemPanel1, ID_CONSTRAINT_CHOICEBOOK, wxDefaultPosition, wxDefaultSize, wxCHB_TOP);
    itemBoxSizer2->Add(m_Notebook, 0, wxALIGN_TOP|wxALL, 5);
    
    CSourceFieldNamePanel *panel1 = new CSourceFieldNamePanel(m_Notebook, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    m_Notebook->AddPage(panel1,_("Taxname"));
    panel1->SetFieldName("taxname");

    CSourceFieldNamePanel *panel2 = new CSourceFieldNamePanel(m_Notebook, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    m_Notebook->AddPage(panel2,_("Source"));

    m_feat_field_panel =  new CFeatureFieldNamePanel(m_Notebook, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    m_Notebook->AddPage(m_feat_field_panel,_("Feature"));
    m_feat_field_panel->ListPresentFeaturesFirst(m_TopSeqEntry, m_featlist);

    CCDSGeneProtFieldNamePanel *panel4 = new CCDSGeneProtFieldNamePanel(m_Notebook);
    m_Notebook->AddPage(panel4,_("CDS-Gene-Prot-mRNA"));
    panel4->SetFieldName("protein name");

    CRNAFieldNamePanel *panel5 = new CRNAFieldNamePanel(m_Notebook, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    m_Notebook->AddPage(panel5,_("RNA"));
    panel5->SetFieldName("product");

    CFieldHandlerNamePanel *panel6 =  new CFieldHandlerNamePanel(m_Notebook);
    panel6->SetFieldNames(CMolInfoField::GetFieldNames());
    m_Notebook->AddPage(panel6,_("MolInfo"));
    panel6->SetFieldName("molecule");

    CFieldHandlerNamePanel *panel7 =  new CFieldHandlerNamePanel(m_Notebook);
    panel7->SetFieldNames(CPubField::GetFieldNames());
    m_Notebook->AddPage(panel7,_("Pub"));
    panel7->SetFieldName("title");

    CStructCommentFieldPanel *panel8 =  new CStructCommentFieldPanel(m_Notebook);
    m_Notebook->AddPage(panel8,_("Structured Comment"));

    CFieldHandlerNamePanel *panel9 =  new CFieldHandlerNamePanel(m_Notebook);
    panel9->SetFieldNames(CDBLinkField::GetFieldNames());
    m_Notebook->AddPage(panel9,_("DBLink"));
    panel9->SetFieldName("title");

    CFieldHandlerNamePanel *panel10 =  new CFieldHandlerNamePanel(m_Notebook);
    panel10->SetFieldNames(CMiscFieldPanel::GetStrings());
    m_Notebook->AddPage(panel10,_("Misc"));
    panel10->SetFieldName(kDefinitionLineLabel);

    wxPanel *panel11 =  new wxPanel(m_Notebook);
    m_Notebook->AddPage(panel11,_("SeqId"));

    wxPanel *panel12 =  new wxPanel(m_Notebook);
    m_Notebook->AddPage(panel12,_("File ID"));

////@end CConstraintPanel content construction

    m_StringConstraintPanel = new CStringConstraintPanel(itemPanel1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_TOP|wxFIXED_MINSIZE|wxALL, 0);

    wxCommandEvent fieldTypeEvent(wxEVT_COMMAND_UPDATE_STRUCTCOMMENT_FIELD);
    fieldTypeEvent.SetEventObject(this);
    GetEventHandler()->ProcessEvent(fieldTypeEvent);   

    UpdateEditor();
}

CFieldNamePanel* CConstraintPanel::GetFieldNamePanel(void)
{
    wxWindow *win =  m_Notebook->GetCurrentPage();
    CFieldNamePanel* panel = dynamic_cast<CFieldNamePanel*>(win);
    return panel;
}

CRef<edit::CStringConstraint> CConstraintPanel::GetStringConstraint()  
{
    return m_StringConstraintPanel->GetStringConstraint();
}

string CConstraintPanel::GetFieldType(void) const
{
    string field_type;
    int sel = m_Notebook->GetSelection();
    if (sel != wxNOT_FOUND)
    {
        field_type = m_Notebook->GetPageText(sel).ToStdString();
    }
    return field_type;
}

void CConstraintPanel::ClearValues(void)
{
    m_StringConstraintPanel->ClearValues();
    CFieldNamePanel* panel = GetFieldNamePanel();
    panel->ClearValues();
}

/*!
 * Should we show tooltips?
 */

bool CConstraintPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CConstraintPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CConstraintPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CConstraintPanel icon retrieval
}

void CConstraintPanel::SetSelection(int page)
{
    if (page != wxNOT_FOUND)
    {
        m_Notebook->SetSelection(page);     
        UpdateEditor();
    }
}

bool CConstraintPanel::SetFieldName(const string &field)
{
    bool res(false);
    if (m_Notebook)
    {
        wxWindow* win = m_Notebook->GetCurrentPage();
        CFieldNamePanel* panel = dynamic_cast<CFieldNamePanel*>(win);
        if (panel && !field.empty())
        {           
            res = panel->SetFieldName(field);
            UpdateEditor();
        }
    }
    return res;
}

void CConstraintPanel::UpdateEditor()
{
    if (m_Notebook)
    {
        wxWindow* win = m_Notebook->GetCurrentPage();
        CFieldNamePanel* panel = dynamic_cast<CFieldNamePanel*>(win);
        if (panel)
        {
            bool allow_other = true;
            vector<string> choices = panel->GetChoices(allow_other); 
            if (m_StringConstraintPanel)
            {
                m_StringConstraintPanel->SetChoices(choices);      
            }
        }
    }
}

void CConstraintPanel::OnPageChanged(wxBookCtrlEvent& event)
{
    UpdateEditor();
    event.Skip();
}

CRef<CEditingActionConstraint> CConstraintPanel::GetConstraint(const string &field, CFieldNamePanel::EFieldType field_type, int subtype)
{
    CFieldNamePanel* field_name_panel = GetFieldNamePanel();
    CRef<CEditingActionConstraint> constraint = CreateEditingActionConstraint(field,
                                                                              field_type,
                                                                              subtype,
                                                                              field_name_panel ? field_name_panel->GetFieldName(true) : GetFieldType(), 
                                                                              CFieldNamePanel::GetFieldTypeFromName(GetFieldType()), 
                                                                              GetSubtype(field_name_panel),
                                                                              CRef<CConstraintMatcher>(new CSimpleConstraintMatcher(GetStringConstraint())));
    return constraint;
}











IMPLEMENT_DYNAMIC_CLASS( CAdvancedConstraintPanel, wxPanel )


/*!
 * CAdvancedConstraintPanel event table definition
 */

BEGIN_EVENT_TABLE( CAdvancedConstraintPanel, wxPanel )

////@begin CAdvancedConstraintPanel event table entries
////@end CAdvancedConstraintPanel event table entries

END_EVENT_TABLE()


/*!
 * CAdvancedConstraintPanel constructors
 */

CAdvancedConstraintPanel::CAdvancedConstraintPanel()
{
    Init();
}

CAdvancedConstraintPanel::CAdvancedConstraintPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, vector<const objects::CFeatListItem *> *featlist, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_featlist(featlist)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAdvancedConstraintPanel creator
 */

bool CAdvancedConstraintPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAdvancedConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAdvancedConstraintPanel creation
    return true;
}


/*!
 * CAdvancedConstraintPanel destructor
 */

CAdvancedConstraintPanel::~CAdvancedConstraintPanel()
{
////@begin CAdvancedConstraintPanel destruction
////@end CAdvancedConstraintPanel destruction
}


/*!
 * Member initialisation
 */

void CAdvancedConstraintPanel::Init()
{
////@begin CAdvancedConstraintPanel member initialisation
////@end CAdvancedConstraintPanel member initialisation
    m_Notebook = NULL;
    m_BasicConstraintPanel = NULL;
    m_CompoundConstraintPanel = NULL;
}


/*!
 * Control creation for CAdvancedConstraintPanel
 */

void CAdvancedConstraintPanel::CreateControls()
{    
////@begin CAdvancedConstraintPanel content construction
    CAdvancedConstraintPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Notebook = new wxNotebook(itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_Notebook, 1, wxGROW|wxALL, 0);
    
    m_BasicConstraintPanel = new CConstraintPanel( m_Notebook, m_TopSeqEntry, m_featlist, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    m_Notebook->AddPage(m_BasicConstraintPanel,_("Simple"));

    m_CompoundConstraintPanel = new CCompoundConstraintPanel( m_Notebook, m_TopSeqEntry, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    m_Notebook->AddPage(m_CompoundConstraintPanel,_("Advanced"));
}


void CAdvancedConstraintPanel::ClearValues(void)
{
    m_BasicConstraintPanel->ClearValues();
    m_CompoundConstraintPanel->ClearValues();
}

/*!
 * Should we show tooltips?
 */

bool CAdvancedConstraintPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAdvancedConstraintPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAdvancedConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAdvancedConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAdvancedConstraintPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAdvancedConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAdvancedConstraintPanel icon retrieval
}

void CAdvancedConstraintPanel::SetSelection(int page)
{
    m_BasicConstraintPanel->SetSelection(page);     
    m_CompoundConstraintPanel->SetSelection(page);
}


CRef<CEditingActionConstraint> CAdvancedConstraintPanel::GetConstraint(const string &field, CFieldNamePanel::EFieldType field_type, int subtype)
{
    wxWindow* win = m_Notebook->GetCurrentPage();
    CConstraintPanelBase* panel = dynamic_cast<CConstraintPanelBase*>(win);
    
    if (panel)
        return panel->GetConstraint(field, field_type, subtype); 
    
    return CRef<CEditingActionConstraint>(new CEditingActionConstraint());
}




IMPLEMENT_DYNAMIC_CLASS( CCompoundConstraintPanel, wxPanel )


/*!
 * CCompoundConstraintPanel event table definition
 */

BEGIN_EVENT_TABLE( CCompoundConstraintPanel, wxPanel )

////@begin CCompoundConstraintPanel event table entries
EVT_BUTTON( ID_CONSTRAINT_ADD, CCompoundConstraintPanel::OnAddConstraint )
EVT_BUTTON( ID_CONSTRAINT_REMOVE, CCompoundConstraintPanel::OnRemoveConstraint )
EVT_LISTBOX_DCLICK( ID_CONSTRAINT_LIST, CCompoundConstraintPanel::OnEditConstraint )
////@end CCompoundConstraintPanel event table entries

END_EVENT_TABLE()


/*!
 * CCompoundConstraintPanel constructors
 */

CCompoundConstraintPanel::CCompoundConstraintPanel()
{
    Init();
}

CCompoundConstraintPanel::CCompoundConstraintPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CCompoundConstraintPanel creator
 */

bool CCompoundConstraintPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCompoundConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCompoundConstraintPanel creation
    return true;
}


/*!
 * CCompoundConstraintPanel destructor
 */

CCompoundConstraintPanel::~CCompoundConstraintPanel()
{
////@begin CCompoundConstraintPanel destruction
////@end CCompoundConstraintPanel destruction
}


/*!
 * Member initialisation
 */

void CCompoundConstraintPanel::Init()
{
////@begin CCompoundConstraintPanel member initialisation
////@end CCompoundConstraintPanel member initialisation
    m_CheckListBox = NULL;
    m_page = wxNOT_FOUND;
}


/*!
 * Control creation for CCompoundConstraintPanel
 */

void CCompoundConstraintPanel::CreateControls()
{    
////@begin CCompoundConstraintPanel content construction
    CCompoundConstraintPanel* itemPanel1 = this;

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


void CCompoundConstraintPanel::ClearValues(void)
{
    m_CheckListBox->Clear();
    m_label_to_constraint.clear();
}

/*!
 * Should we show tooltips?
 */

bool CCompoundConstraintPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCompoundConstraintPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCompoundConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCompoundConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCompoundConstraintPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCompoundConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCompoundConstraintPanel icon retrieval
}

void CCompoundConstraintPanel::SetSelection(int page)
{
    m_page = page;
}


CRef<CEditingActionConstraint> CCompoundConstraintPanel::GetConstraint(const string &field, CFieldNamePanel::EFieldType field_type, int subtype)
{
    CRef<CEditingActionConstraint> constraint(new CEditingActionConstraint());
    for (auto &item : m_label_to_constraint)
    {
        CRef<CEditingActionConstraint> add = CreateEditingActionConstraint(field,
                                                                           field_type,
                                                                           subtype,
                                                                           item.second.field,
                                                                           item.second.field_type,
                                                                           item.second.subtype,
                                                                           item.second.matcher);
        *constraint += add;

        if (item.second.enabled2)
        {
            CRef<CEditingActionConstraint> add2 = CreateEditingActionConstraint(field,
                                                                                field_type,
                                                                                subtype,
                                                                                item.second.field2,
                                                                                item.second.field_type2,
                                                                                item.second.subtype2,
                                                                                item.second.matcher2);
            *constraint += add2;
        }
    }
    return constraint;
}

void CCompoundConstraintPanel::OnAddConstraint( wxCommandEvent& event )
{
    CAddConstraint * dlg = new CAddConstraint(this, m_TopSeqEntry); 
    dlg->SetSelection(m_page);
    dlg->Show(true);
}

void CCompoundConstraintPanel::OnRemoveConstraint( wxCommandEvent& event )
{
    Freeze();
    set<int> to_delete;
    wxArrayInt checked_items;
    m_CheckListBox->GetCheckedItems(checked_items);
    if (checked_items.IsEmpty())
    {
        int answer = wxMessageBox (_("No items checked, remove all?"), ToWxString("Warning"), wxYES_NO | wxICON_QUESTION);
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

void CCompoundConstraintPanel::AddConstraint(pair<string, SFieldTypeAndMatcher> item, int id)
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

void CCompoundConstraintPanel::OnEditConstraint( wxCommandEvent& event )
{
    const string &label = event.GetString().ToStdString();
    int id = event.GetInt();
    CAddConstraint * dlg = new CAddConstraint(this, m_TopSeqEntry); 
    dlg->SetEditId(id);
    dlg->SetConstraint(m_label_to_constraint[label]);
    dlg->Show(true);
}

END_NCBI_SCOPE
