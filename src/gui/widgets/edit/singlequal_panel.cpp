/*  $Id: singlequal_panel.cpp 44787 2020-03-13 19:03:59Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>
#include <gui/widgets/edit/singlequal_panel.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/macro_selectqual_dlg.hpp>
#include <gui/widgets/edit/macro_applysrc_table.hpp>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/combobox.h>

BEGIN_NCBI_SCOPE

/*!
 * CSingleQualPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleQualPanel, wxPanel )


/*!
 * CSingleQualPanel event table definition
 */

BEGIN_EVENT_TABLE( CSingleQualPanel, wxPanel )

////@begin CSingleQualPanel event table entries
////@end CSingleQualPanel event table entries

END_EVENT_TABLE()


/*!
 * CSingleQualPanel constructors
 */

CSingleQualPanel::CSingleQualPanel()
{
    Init();
}

CSingleQualPanel::CSingleQualPanel( wxWindow* parent, EMacroFieldType type, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    :m_FieldType(type), m_MatchType(type)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSingleQualPanel creator
 */

bool CSingleQualPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleQualPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleQualPanel creation
    return true;
}


/*!
 * CSingleQualPanel destructor
 */

CSingleQualPanel::~CSingleQualPanel()
{
////@begin CSingleQualPanel destruction
////@end CSingleQualPanel destruction
}


/*!
 * Member initialisation
 */

void CSingleQualPanel::Init()
{
////@begin CSingleQualPanel member initialisation
    m_FieldFirst = NULL;
    m_Column = NULL;
    m_MatchCheck = NULL;
////@end CSingleQualPanel member initialisation
}


/*!
 * Control creation for CSingleQualPanel
 */

void CSingleQualPanel::CreateControls()
{    
////@begin CSingleQualPanel content construction
    CSingleQualPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer1);

    wxArrayString m_FieldFirstStrings;
    m_FieldFirst = new wxComboBox( itemPanel1, ID_SNGLQUALCH1, wxEmptyString, wxDefaultPosition, wxSize(169, -1), m_FieldFirstStrings, wxCB_DROPDOWN);
    itemBoxSizer1->Add(m_FieldFirst, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
    m_FieldFirst->Bind(wxEVT_COMBOBOX, &CSingleQualPanel::OnFieldSelected, this);
    m_FieldFirst->SetToolTip("Use 'Select' to choose qualifier");

    m_Column = new wxTextCtrl(itemPanel1, ID_SNGLQUALCOL, wxEmptyString, wxDefaultPosition, wxSize(30, -1), 0);
    itemBoxSizer1->Add(m_Column, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    m_MatchCheck = new wxCheckBox(itemPanel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer1->Add(m_MatchCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_MatchCheck->SetValue(false);

    wxHyperlinkCtrl* link = new wxHyperlinkCtrl(itemPanel1, wxID_ANY, wxT("Select"), wxT("select"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer1->Add(link, 0, wxALIGN_CENTER_VERTICAL, 5);
    link->Bind(wxEVT_HYPERLINK, &CSingleQualPanel::OnSelectClicked, this);

    x_LoadChoices();
////@end CSingleQualPanel content construction
}


void CSingleQualPanel::x_LoadChoices()
{
    wxArrayString arraystring;
    m_FieldFirst->AppendString(wxEmptyString);
    m_FieldFirst->AppendString(kFieldTypeSeqId);
    if (m_FieldType != EMacroFieldType::eStructComment && 
        m_FieldType != EMacroFieldType::eRNA) {  // don't load fields for structured comment or for RNA fields
        
        auto choices = CMacroEditorContext::GetInstance().GetFieldNames(m_FieldType);
        ToArrayString(choices, arraystring);
        m_FieldFirst->Append(arraystring);
    }
}

void CSingleQualPanel::SetField(const string& first_field, int col)
{
    string field = first_field;
    if (NStr::EqualNocase(field, "accession") ||
        NStr::EqualNocase(field, kFieldTypeSeqId) ||
        NStr::StartsWith(field, "sequence", NStr::eNocase) ||
        NStr::StartsWith(field, "filename", NStr::eNocase) ||
        NStr::StartsWith(field, "bankit", NStr::eNocase)) {
        field = kFieldTypeSeqId;
    }
    else if (NStr::EqualNocase(field, "authors")) {
        field = CPubFieldType::GetLabelForType(CPubFieldType::ePubFieldType_Authors);
        field = CPubFieldType::NormalizePubFieldName(field);
    }


    int sel = m_FieldFirst->FindString(ToWxString(field));
    if (sel > -1) {
        m_FieldFirst->SetSelection(sel);
    }
    m_Column->SetValue(ToWxString(NStr::IntToString(col)));
    m_MatchCheck->SetValue(field == kFieldTypeSeqId);
}

SFieldFromTable CSingleQualPanel::GetField()
{
    auto col = m_Column->GetValue().ToStdString();
    const string& field = m_FieldFirst->GetStringSelection().ToStdString();
    for (auto&& it : m_AdditionalFields) {
        if (it.m_GUIName == field) {
            it.m_Column = col;
            return it;
        }
    }
    return SFieldFromTable(col, field, field, false);
}

void CSingleQualPanel::OnSelectClicked(wxHyperlinkEvent& event)
{
    if (event.GetURL() != wxT("select")) {
        event.Skip();
        return;
    }

    CSelectQualdDlg dlg(this, m_FieldType, m_MatchType);
    if (dlg.ShowModal() == wxID_OK) {
        m_MatchType = dlg.GetMatchType();
        auto sel_field = dlg.GetSelection();
        m_FieldFirst->SetString(0, sel_field.m_GUIName);
        m_FieldFirst->SetSelection(0);
        m_AdditionalFields.push_back(sel_field);
    }

    event.Skip();
}

void CSingleQualPanel::OnFieldSelected(wxCommandEvent& event)
{
    if (!m_MatchCheck->IsChecked())
        m_MatchCheck->SetValue(ToStdString(m_FieldFirst->GetStringSelection()) == kFieldTypeSeqId);
}


/*!
 * Should we show tooltips?
 */

bool CSingleQualPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSingleQualPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleQualPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleQualPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSingleQualPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleQualPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleQualPanel icon retrieval
}

END_NCBI_SCOPE

