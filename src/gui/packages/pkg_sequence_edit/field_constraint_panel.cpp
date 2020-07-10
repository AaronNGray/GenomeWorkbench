/*  $Id: field_constraint_panel.cpp 39649 2017-10-24 15:22:12Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/packages/pkg_sequence_edit/field_constraint_panel.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/misc_field_panel.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CFieldConstraintPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFieldConstraintPanel, wxPanel )


/*!
 * CFieldConstraintPanel event table definition
 */

BEGIN_EVENT_TABLE( CFieldConstraintPanel, wxPanel )

////@begin CFieldConstraintPanel event table entries
////@end CFieldConstraintPanel event table entries

END_EVENT_TABLE()


/*!
 * CFieldConstraintPanel constructors
 */

CFieldConstraintPanel::CFieldConstraintPanel()
{
    Init();
}

CFieldConstraintPanel::CFieldConstraintPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CFieldConstraintPanel creator
 */

bool CFieldConstraintPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFieldConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    Centre();
////@end CFieldConstraintPanel creation
    return true;
}


/*!
 * CFieldConstraintPanel destructor
 */

CFieldConstraintPanel::~CFieldConstraintPanel()
{
////@begin CFieldConstraintPanel destruction
////@end CFieldConstraintPanel destruction
}


/*!
 * Member initialisation
 */

void CFieldConstraintPanel::Init()
{
////@begin CFieldConstraintPanel member initialisation
    m_StringConstraintPanel = NULL;
////@end CFieldConstraintPanel member initialisation
}


/*!
 * Control creation for CFieldConstraintPanel
 */

void CFieldConstraintPanel::CreateControls()
{    
    //CFieldConstraintPanel* itemPanel1 = this;
    wxPanel* parentPanel = new wxPanel(this);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    parentPanel->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    vector<CFieldNamePanel::EFieldType> field_types;
    field_types.push_back(CFieldNamePanel::eFieldType_Taxname);
    field_types.push_back(CFieldNamePanel::eFieldType_Source);
    field_types.push_back(CFieldNamePanel::eFieldType_Misc);
    field_types.push_back(CFieldNamePanel::eFieldType_Feature);
    field_types.push_back(CFieldNamePanel::eFieldType_CDSGeneProt);
    field_types.push_back(CFieldNamePanel::eFieldType_RNA);
    field_types.push_back(CFieldNamePanel::eFieldType_MolInfo);
    field_types.push_back(CFieldNamePanel::eFieldType_Pub);
    field_types.push_back(CFieldNamePanel::eFieldType_DBLink);
    field_types.push_back(CFieldNamePanel::eFieldType_SeqId);
    m_FieldType = new CFieldChoicePanel(parentPanel, field_types, false, false, ID_CFIELDCONSTRAINTPANEL_FIELD, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_FieldType, 0, wxEXPAND|wxTOP, 5);

    bool shorten_constr_text = false; 
    if (x_ShouldDecreaseStrConstrPanel()) {
        shorten_constr_text = true; // shorten the length of the string constraint text control 
    }
    m_StringConstraintPanel = new CStringConstraintPanel(parentPanel, shorten_constr_text, ID_CFIELDCONSTRAINTPANEL_CONSTRAINT, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE|wxALL, 0);

    wxBoxSizer* frame_sizer = new wxBoxSizer(wxHORIZONTAL);
    frame_sizer->Add(parentPanel, 1, wxALL|wxGROW, 0);
    SetSizerAndFit(frame_sizer);  

    UpdateEditor();
}


/*!
 * Should we show tooltips?
 */

bool CFieldConstraintPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFieldConstraintPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFieldConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFieldConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFieldConstraintPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFieldConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFieldConstraintPanel icon retrieval
}

bool CFieldConstraintPanel::SetFieldType(CFieldNamePanel::EFieldType field_type)
{
    bool rval = m_FieldType->SetFieldType(field_type, false);
    this->SetMinClientSize(wxDefaultSize);

    Layout();
    Fit();
    Refresh();
    return rval;
}


string CFieldConstraintPanel::GetFieldName(const bool subfield)
{
    string field_name = m_FieldType->GetFieldName(subfield);
    return field_name;
}


bool CFieldConstraintPanel::SetFieldName(const string& field)
{
    return m_FieldType->SetFieldName(field);
}


vector<string> CFieldConstraintPanel::GetChoices(bool& allow_other)
{
    return m_FieldType->GetChoices(allow_other);
}


CRef<edit::CStringConstraint> CFieldConstraintPanel::GetStringConstraint()
{
    return m_StringConstraintPanel->GetStringConstraint();
}


void CFieldConstraintPanel::UpdateEditor()
{
    this->SetMinClientSize(wxDefaultSize);
    Layout();
    Fit();
    Refresh();

    if (x_IsParentAECRDlg()) {
        x_UpdateAECRDlgLayout();
    } else {
        x_UpdateParent();
    }
}


END_NCBI_SCOPE
