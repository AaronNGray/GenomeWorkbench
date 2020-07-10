/*  $Id: string_constraint_panel.cpp 36143 2016-08-16 18:21:37Z filippov $
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

#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/button.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CStringConstraintPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CStringConstraintPanel, wxPanel )


/*!
 * CStringConstraintPanel event table definition
 */

BEGIN_EVENT_TABLE( CStringConstraintPanel, wxPanel )

////@begin CStringConstraintPanel event table entries
////@end CStringConstraintPanel event table entries
    EVT_BUTTON( ID_STRING_CONSTRAINT_CLEAR_BUTTON, CStringConstraintPanel::OnClear )
    EVT_CHOICE( ID_STRING_CONSTRAINT_MATCH_CHOICE, CStringConstraintPanel::OnMatchTypeSelected )
END_EVENT_TABLE()


/*!
 * CStringConstraintPanel constructors
 */

CStringConstraintPanel::CStringConstraintPanel()
{
    Init();
}

CStringConstraintPanel::CStringConstraintPanel( wxWindow* parent, bool parentAECRDlg, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool cap_constraints )
    : m_ParentAECRDlg(parentAECRDlg), m_CapConstraints(cap_constraints)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CStringConstraintPanel creator
 */

bool CStringConstraintPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CStringConstraintPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CStringConstraintPanel creation
    ShowChoiceOrText();
    
    return true;
}


/*!
 * CStringConstraintPanel destructor
 */

CStringConstraintPanel::~CStringConstraintPanel()
{
////@begin CStringConstraintPanel destruction
////@end CStringConstraintPanel destruction
}


/*!
 * Member initialisation
 */

void CStringConstraintPanel::Init()
{
////@begin CStringConstraintPanel member initialisation
    m_MatchType = NULL;
    m_MatchText = NULL;
    m_IgnoreCase = NULL;
    m_IgnoreSpace = NULL;
    m_MatchChoice = NULL;
    m_TextSizer = NULL;
    m_is_choice = false;
////@end CStringConstraintPanel member initialisation
}


/*!
 * Control creation for CStringConstraintPanel
 */

void CStringConstraintPanel::CreateControls()
{    
////@begin CStringConstraintPanel content construction
    //CStringConstraintPanel* itemPanel1 = this;
    wxPanel* parentPanel = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    parentPanel->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_MatchTypeStrings;
    m_MatchTypeStrings.Add(_("Contains"));
    m_MatchTypeStrings.Add(_("Does not contain"));
    m_MatchTypeStrings.Add(_("Equals"));
    m_MatchTypeStrings.Add(_("Does not equal"));
    m_MatchTypeStrings.Add(_("Starts with"));
    m_MatchTypeStrings.Add(_("Ends with"));
    m_MatchTypeStrings.Add(_("Is one of"));
    m_MatchTypeStrings.Add(_("Is not one of"));
    m_MatchTypeStrings.Add(_("Does not start with"));
    m_MatchTypeStrings.Add(_("Does not end with"));
    if (m_CapConstraints)
    {
        m_MatchTypeStrings.Add(_("All caps"));
        m_MatchTypeStrings.Add(_("First letter of each word"));
        m_MatchTypeStrings.Add(_("First letter of first word"));
    }

    m_MatchType = new wxChoice( parentPanel, ID_STRING_CONSTRAINT_MATCH_CHOICE, wxDefaultPosition, wxDefaultSize, m_MatchTypeStrings, 0 );
    m_MatchType->SetStringSelection(_("Contains"));
    itemBoxSizer3->Add(m_MatchType, 0, wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);
    
    m_TextSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(m_TextSizer, 0, wxALIGN_TOP, 0);

    m_MatchText = new wxTextCtrl( parentPanel, ID_STRING_CONSTRAINT_MATCH_TEXT, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_MULTILINE );
    m_TextSizer->Add(m_MatchText, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);
    if (m_ParentAECRDlg) {
        m_MatchText->SetMinSize(wxSize(170, -1)); 
        //itemBoxSizer3->Layout();
    }   
    wxArrayString choice_strings;
    m_MatchChoice = new wxComboBox(parentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), choice_strings, 0 );
    m_MatchChoice->Hide();


    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_IgnoreCase = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX5, _("Ignore Case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreCase->SetValue(false);
    itemBoxSizer6->Add(m_IgnoreCase, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IgnoreSpace = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX6, _("Ignore Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreSpace->SetValue(false);
    itemBoxSizer6->Add(m_IgnoreSpace, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton4 = new wxButton( parentPanel, ID_STRING_CONSTRAINT_CLEAR_BUTTON, _("Clear Constraint"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

   
    ////@end CStringConstraintPanel content construction
}

void CStringConstraintPanel::OnClear( wxCommandEvent& event )
{
    ClearValues();  
}

void CStringConstraintPanel::ClearValues()
{
    if (m_is_choice)
    {
        m_MatchChoice->Hide();
        m_TextSizer->Replace(m_MatchChoice, m_MatchText);
        m_MatchText->Show();
        m_is_choice = false;
        Layout();
        Fit();
    }
    m_MatchType->SetStringSelection(_("Contains"));
    m_MatchText->SetValue(wxEmptyString);
    m_IgnoreCase->SetValue(false);
    m_IgnoreSpace->SetValue(false);
}

void CStringConstraintPanel::SetChoices(const vector<string> &choices)
{
    wxArrayString choice_strings;
    ITERATE(vector<string>, it, choices) 
    {
        choice_strings.Add(ToWxString(*it));
    }
    m_MatchChoice->Set(choice_strings);
    ShowChoiceOrText();
}

void CStringConstraintPanel::ShowChoiceOrText()
{
   if (!m_MatchChoice->IsListEmpty() && (m_MatchType->GetSelection() == eMatchType_Equals || m_MatchType->GetSelection() == eMatchType_DoesNotEqual) )
    {
        if (!m_is_choice)
        {
            m_MatchText->Hide();
            m_TextSizer->Replace(m_MatchText, m_MatchChoice);
            m_MatchChoice->Show();
        }
        m_is_choice = true;
    }
    else
    {
        if (m_is_choice)
        {
            m_MatchChoice->Hide();
            m_TextSizer->Replace(m_MatchChoice, m_MatchText);
            m_MatchText->Show();
        }
        m_is_choice = false;
    }
    Layout();
    Fit();

}

void CStringConstraintPanel::OnMatchTypeSelected( wxCommandEvent& event )
{
    ShowChoiceOrText();
    event.Skip();
}

/*!
 * Should we show tooltips?
 */

bool CStringConstraintPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CStringConstraintPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CStringConstraintPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CStringConstraintPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CStringConstraintPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CStringConstraintPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CStringConstraintPanel icon retrieval
}


CRef<edit::CStringConstraint> CStringConstraintPanel::GetStringConstraint()
{
    string match_text; 
    if (!m_is_choice)
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchText->GetValue()),"\r"," "),"\n"," ");
    else 
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchChoice->GetValue()),"\r"," "),"\n"," ");

    edit::CStringConstraint::EMatchType match_type = edit::CStringConstraint::eMatchType_Contains;
    int val = m_MatchType->GetSelection();
    bool not_present = false;
    bool not_found = false;
    switch(val) {
        case eMatchType_Contains:
            match_type = edit::CStringConstraint::eMatchType_Contains;
            not_present = false;
            break;
        case eMatchType_DoesNotContain:
            match_type = edit::CStringConstraint::eMatchType_Contains;
            not_present = true;
            break;
        case eMatchType_Equals:
            match_type = edit::CStringConstraint::eMatchType_Equals;
            not_present = false;
            break;
        case eMatchType_DoesNotEqual:
            match_type = edit::CStringConstraint::eMatchType_Equals;
            not_present = true;
            break;
        case eMatchType_StartsWith:
            match_type = edit::CStringConstraint::eMatchType_StartsWith;
            not_present = false;
            break;
        case eMatchType_EndsWith:
            match_type = edit::CStringConstraint::eMatchType_EndsWith;
            not_present = false;
            break;
        case eMatchType_IsOneOf:
            match_type = edit::CStringConstraint::eMatchType_IsOneOf;
            not_present = false;
            break;
        case eMatchType_IsNotOneOf:
            match_type = edit::CStringConstraint::eMatchType_IsOneOf;
            not_present = true;
            break;
        case eMatchType_DoesNotStartWith:
            match_type = edit::CStringConstraint::eMatchType_StartsWith;
            not_present = true;
            break;
        case eMatchType_DoesNotEndWith:
            match_type = edit::CStringConstraint::eMatchType_EndsWith;
            not_present = true;
            break;
        default: not_found = true; break;
    }
    CRef<edit::CStringConstraint> sc(NULL);
    if (!not_found)
    {
        if (match_text.size() > 0 || (match_type == edit::CStringConstraint::eMatchType_Equals && not_present)) {
            sc.Reset(new edit::CStringConstraint(match_text, match_type,
                                           m_IgnoreCase->GetValue(), 
                                           m_IgnoreSpace->GetValue(),
                                           not_present));
        }
    }
    return sc;
}

bool CStringConstraintPanel::Match(const string &str)
{
    CRef<edit::CStringConstraint> constraint = GetStringConstraint();
    if (constraint)
        return constraint->DoesTextMatch(str);

    CString_constraint s;
    bool not_found = false;
    int val = m_MatchType->GetSelection();
    switch(val) 
    {
    case eMatchType_Is_all_caps: s.SetIs_all_caps(true); break;
    case eMatchType_Is_first_each_cap: s.SetIs_first_each_cap(true); break;
    case eMatchType_IsFirstCap: s.SetIs_first_cap(true); break;
    default: not_found = true;
    }

    if (not_found)
        return false;
    return s.Match(str);     
}

END_NCBI_SCOPE

