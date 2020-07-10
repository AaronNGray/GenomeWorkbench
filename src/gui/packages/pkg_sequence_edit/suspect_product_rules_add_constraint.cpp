/*  $Id: suspect_product_rules_add_constraint.cpp 39866 2017-11-14 21:17:01Z filippov $
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
#include <objects/macro/Field_constraint.hpp>
#include <objects/macro/Feature_field.hpp>
#include <objects/macro/Feat_qual_choice.hpp>

#include <gui/packages/pkg_sequence_edit/suspect_product_rules_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/suspect_product_rules_add_constraint.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/statline.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);



IMPLEMENT_DYNAMIC_CLASS( CSuspectAddConstraint, wxFrame )

BEGIN_EVENT_TABLE( CSuspectAddConstraint, wxFrame )

////@begin CSuspectAddConstraint event table entries
    EVT_BUTTON( wxID_OK, CSuspectAddConstraint::OnAccept )
    EVT_BUTTON( wxID_CANCEL, CSuspectAddConstraint::OnCancel )
////@end CSuspectAddConstraint event table entries

END_EVENT_TABLE()

CSuspectAddConstraint::CSuspectAddConstraint()
{
    Init();
}

CSuspectAddConstraint::CSuspectAddConstraint( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_id(-1)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CSuspectAddConstraint::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSuspectAddConstraint creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CSuspectAddConstraint creation
    return true;
}

CSuspectAddConstraint::~CSuspectAddConstraint()
{
////@begin CSuspectAddConstraint destruction
////@end CSuspectAddConstraint destruction
}

void CSuspectAddConstraint::Init()
{
////@begin CSuspectAddConstraint member initialisation
    m_MatchType = NULL;
    m_MatchText = NULL;
    m_IgnoreCase = NULL;
    m_IgnoreSpace = NULL;
    m_WholeWord = NULL;
    m_IgnorePunct = NULL;
    m_IgnoreSyn = NULL;
    m_AnyLetters = NULL;
    m_AllUpper = NULL;
    m_AllLower = NULL;
    m_AllPunct = NULL;
    m_MatchChoice = NULL;
    m_TextSizer = NULL;
    m_ClearButton = NULL;
    m_WordSubst = NULL;
    m_is_choice = false;
////@end CSuspectAddConstraint member initialisation
}

void CSuspectAddConstraint::CreateControls()
{    
    wxBoxSizer* itemBoxSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer0);

    wxPanel* parentPanel = new wxPanel(this, wxID_ANY);
    itemBoxSizer0->Add(parentPanel, 1, wxGROW, 0);
 
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    parentPanel->SetSizer(itemBoxSizer1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

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
    
    m_MatchType = new wxChoice( parentPanel, ID_STRING_CONSTRAINT_MATCH_CHOICE, wxDefaultPosition, wxDefaultSize, m_MatchTypeStrings, 0 );
    m_MatchType->SetStringSelection(_("Contains"));
    itemBoxSizer3->Add(m_MatchType, 0, wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);
    m_MatchType->Bind(wxEVT_CHOICE, &CSuspectAddConstraint::OnMatchTypeSelected, this);
    
    m_TextSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(m_TextSizer, 0, wxALIGN_TOP, 0);

    m_MatchText = new wxTextCtrl( parentPanel, ID_STRING_CONSTRAINT_MATCH_TEXT, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_MULTILINE );
    m_TextSizer->Add(m_MatchText, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);   
    wxArrayString choice_strings;
    m_MatchChoice = new wxComboBox(parentPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), choice_strings, 0 );
    m_MatchChoice->Hide();


    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_IgnoreCase = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX5, _("Ignore Case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreCase->SetValue(true);
    itemBoxSizer6->Add(m_IgnoreCase, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IgnoreSpace = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX6, _("Ignore Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreSpace->SetValue(false);
    itemBoxSizer6->Add(m_IgnoreSpace, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IgnorePunct = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX8, _("Ignore Punctuation"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnorePunct->SetValue(false);
    itemBoxSizer6->Add(m_IgnorePunct, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);    

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_WholeWord = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX7, _("Whole Word"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WholeWord->SetValue(false);
    itemBoxSizer7->Add(m_WholeWord, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   

    m_IgnoreSyn = new wxCheckBox( parentPanel, ID_STRING_CONSTRAINT_CHECKBOX9, _("Ignore 'putative' synonyms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IgnoreSyn->SetValue(false);
    itemBoxSizer7->Add(m_IgnoreSyn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AnyLetters = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO0, _("Any letters"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AnyLetters->SetValue(true);
    itemBoxSizer8->Add(m_AnyLetters, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AllUpper = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO1, _("All letters are upper case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllUpper->SetValue(false);
    itemBoxSizer8->Add(m_AllUpper, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AllLower = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO2, _("All letters are lower case"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllLower->SetValue(false);
    itemBoxSizer8->Add(m_AllLower, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AllPunct = new wxRadioButton( parentPanel, ID_STRING_CONSTRAINT_RADIO3, _("All characters are punctuation"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AllPunct->SetValue(false);
    itemBoxSizer8->Add(m_AllPunct, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
   
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ClearButton = new wxButton( parentPanel, ID_STRING_CONSTRAINT_CLEAR_BUTTON, _("Clear Constraint"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_ClearButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_ClearButton->Connect(ID_STRING_CONSTRAINT_CLEAR_BUTTON, wxEVT_BUTTON, wxCommandEventHandler(CSuspectAddConstraint::OnClear), NULL, this);

    m_WordSubst = new wxButton( parentPanel, ID_STRING_CONSTRAINT_WORDSUBST_BUTTON, _("Word Substitutions"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_WordSubst, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_WordSubst->Connect(ID_STRING_CONSTRAINT_WORDSUBST_BUTTON, wxEVT_BUTTON, wxCommandEventHandler(CSuspectAddConstraint::OnWordSubstitution), NULL, this);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxButton* itemButton13 = new wxButton( parentPanel, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( parentPanel, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

}

bool CSuspectAddConstraint::ShowToolTips()
{
    return true;
}
wxBitmap CSuspectAddConstraint::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSuspectAddConstraint bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSuspectAddConstraint bitmap retrieval
}
wxIcon CSuspectAddConstraint::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSuspectAddConstraint icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSuspectAddConstraint icon retrieval
}

void CSuspectAddConstraint::OnCancel( wxCommandEvent& event )
{
    Close();
}

void CSuspectAddConstraint::OnAccept( wxCommandEvent& event )
{
    string label = GetDescription();
    CRef<CString_constraint> sc = GetStringConstraint();
    if (!sc || label.empty()) 
    {
        event.Skip();
        return;
    }
    CRef<CConstraint_choice> constraint(new CConstraint_choice);
    constraint->SetString(*sc);
    pair<string, CRef<CConstraint_choice> > item(label, constraint);
    wxWindow* parent = GetParent();
    CSuspectCompoundConstraintPanel* compound_panel = dynamic_cast<CSuspectCompoundConstraintPanel*>(parent);
    compound_panel->AddConstraint(item, m_id);
    Close();
}


void CSuspectAddConstraint::OnClear( wxCommandEvent& event )
{
    ClearValues();  
}

void CSuspectAddConstraint::ClearValues()
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
    m_IgnoreCase->SetValue(true);
    m_IgnoreSpace->SetValue(false);
    m_WholeWord->SetValue(false);
    m_IgnorePunct->SetValue(false);
    m_IgnoreSyn->SetValue(false);
    m_AnyLetters->SetValue(true);
    m_AllUpper->SetValue(false);
    m_AllLower->SetValue(false);
    m_AllPunct->SetValue(false);
    m_word_subst.Reset();
}

void CSuspectAddConstraint::SetChoices(const vector<string> &choices)
{
    wxArrayString choice_strings;
    ITERATE(vector<string>, it, choices) 
    {
        choice_strings.Add(ToWxString(*it));
    }
    m_MatchChoice->Set(choice_strings);
    ShowChoiceOrText();
}

void CSuspectAddConstraint::ShowChoiceOrText()
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

void CSuspectAddConstraint::OnMatchTypeSelected( wxCommandEvent& event )
{
    ShowChoiceOrText();
    event.Skip();
}

CRef<CString_constraint> CSuspectAddConstraint::GetStringConstraint()
{
    string match_text; 
    if (!m_is_choice)
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchText->GetValue()),"\r"," "),"\n"," ");
    else 
        match_text = NStr::Replace(NStr::Replace(ToStdString(m_MatchChoice->GetValue()),"\r"," "),"\n"," ");

    CRef<CString_constraint> c(new CString_constraint);
    c->SetMatch_text(match_text);
    c->SetCase_sensitive(!m_IgnoreCase->GetValue());
    c->SetIgnore_space(m_IgnoreSpace->GetValue());
    c->SetIgnore_punct(m_IgnorePunct->GetValue());
    c->SetWhole_word(m_WholeWord->GetValue());
    c->SetIs_all_caps(m_AllUpper->GetValue());
    c->SetIs_all_lower(m_AllLower->GetValue());
    c->SetIs_all_punct(m_AllPunct->GetValue());
    c->SetIgnore_weasel(m_IgnoreSyn->GetValue());

    if (m_word_subst && m_word_subst->IsSet() && !m_word_subst->Get().empty())
    {
        c->SetIgnore_words(*m_word_subst);
    }

    switch(m_MatchType->GetSelection()) {
        case eMatchType_Contains:
            c->SetMatch_location(eString_location_contains);
            break;
        case eMatchType_DoesNotContain:
            c->SetMatch_location(eString_location_contains);
            c->SetNot_present(true);
            break;
        case eMatchType_Equals:
            c->SetMatch_location(eString_location_equals);
            break;
        case eMatchType_DoesNotEqual:
            c->SetMatch_location(eString_location_equals);
            c->SetNot_present(true);
            break;
        case eMatchType_StartsWith:
            c->SetMatch_location(eString_location_starts);
            break;
        case eMatchType_EndsWith:
            c->SetMatch_location(eString_location_ends);
            break;
        case eMatchType_IsOneOf:
            c->SetMatch_location(eString_location_inlist);
            break;
        case eMatchType_IsNotOneOf:
            c->SetMatch_location(eString_location_inlist);
            c->SetNot_present(true);
            break;
        case eMatchType_DoesNotStartWith:
            c->SetMatch_location(eString_location_starts);
            c->SetNot_present(true);
            break;
        case eMatchType_DoesNotEndWith:
            c->SetMatch_location(eString_location_ends);
            c->SetNot_present(true);
            break;
    default: c.Reset(); break;
    }
  
    return c;
}

string CSuspectAddConstraint::GetDescription(CRef<CString_constraint> c)
{
    string str;
    if (!c)
        return str;

    string match_text; 
    if (c->IsSetMatch_text())
        match_text = c->GetMatch_text();   

  if (!match_text.empty())
  {
      if (c->IsSetNot_present() && c->GetNot_present())
      {
          switch (c->GetMatch_location())
          {
          case eString_location_contains : str = "does not contain "; break;
          case eString_location_equals :   str = "does not equal "; break;
          case eString_location_starts :   str = "does not start with "; break;
          case eString_location_ends :     str = "does not ends with "; break;
          case eString_location_inlist :   str = "is not one of "; break;
          default : break;
          }
      }
      else
      {
          switch (c->GetMatch_location())
          {
          case eString_location_contains :  str = "contains "; break;
          case eString_location_equals : str = "equals "; break;
          case eString_location_starts : str = "starts with "; break;
          case eString_location_ends   : str = "ends with "; break;
          case eString_location_inlist : str = "is one of "; break;
          default : break;
          }
      }
  
        str += "'" + match_text + "' ";
  
     
        vector<string> subs;
        if (c->IsSetCase_sensitive() && c->GetCase_sensitive())
            subs.push_back("case-sensitive");
        
        if (c->IsSetIgnore_space() && c->GetIgnore_space())
            subs.push_back("ignore spaces");
        
        if (c->IsSetIgnore_punct() && c->GetIgnore_punct())
            subs.push_back("ignore punctuation");
        
        if (c->IsSetWhole_word() && c->GetWhole_word())
            subs.push_back("whole word");
        
        if (c->IsSetIgnore_weasel() && c->GetIgnore_weasel())
            subs.push_back("ignore 'putative' synonyms");
        
        if (!subs.empty())
        {
            string sub = NStr::Join(subs, ", ");
            str += "(" + sub + ")";
        }
        
        if (c->IsSetIgnore_words() && c->GetIgnore_words().IsSet())
        {
            for (auto ws :  c->GetIgnore_words().Get())
            {
                string label = CWordSubstitutionDlg::GetDescription(ws); 
                str += ", " + label;
            }
        }
        
  }
  
      
  string sub;
  if (c->IsSetIs_all_caps() && c->GetIs_all_caps())
      sub = "all letters are upper case";
  
  if (c->IsSetIs_all_lower() && c->GetIs_all_lower())
      sub = "all letters are lower case";
  
  if (c->IsSetIs_all_punct() && c->GetIs_all_punct())
      sub = "all characters are punctiation";
  
  if (!sub.empty())
  {
      if (!str.empty())
          str += ", ";
      str += sub;        
    }
  
  return str;
}

string CSuspectAddConstraint::GetDescription()
{
    CRef<CString_constraint> c = GetStringConstraint();
    return GetDescription(c);
}

string CSuspectAddConstraint::GetDescription(const CConstraint_choice &constraint)
{
    
    CRef<CString_constraint> c(new CString_constraint);
    if (constraint.IsString())
        c->Assign(constraint.GetString());
    else if (constraint.IsField() && constraint.GetField().IsSetField() && constraint.GetField().GetField().IsFeature_field()
             && constraint.GetField().GetField().GetFeature_field().IsSetType() && constraint.GetField().GetField().GetFeature_field().GetType() == eMacro_feature_type_cds
             && constraint.GetField().GetField().GetFeature_field().IsSetField() && constraint.GetField().GetField().GetFeature_field().GetField().IsLegal_qual()
             && constraint.GetField().GetField().GetFeature_field().GetField().GetLegal_qual() == eFeat_qual_legal_product 
             && constraint.GetField().IsSetString_constraint())
        c->Assign(constraint.GetField().GetString_constraint());
    else
        kEmptyStr;
    return GetDescription(c);
}

void CSuspectAddConstraint::OnWordSubstitution( wxCommandEvent& event )
{
    CWordSubstitutionDlg * dlg = new CWordSubstitutionDlg(this, m_word_subst); 
    dlg->Show(true);
}

void CSuspectAddConstraint::AddWordSubstSet(CRef<objects::CWord_substitution_set> word_subst)
{
    m_word_subst = word_subst;
}

void CSuspectAddConstraint::SetConstraint(const CConstraint_choice &constraint)
{
    
    CRef<CString_constraint> c(new CString_constraint);
    if (constraint.IsString())
        c->Assign(constraint.GetString());
    else if (constraint.IsField() && constraint.GetField().IsSetField() && constraint.GetField().GetField().IsFeature_field()
             && constraint.GetField().GetField().GetFeature_field().IsSetType() && constraint.GetField().GetField().GetFeature_field().GetType() == eMacro_feature_type_cds
             && constraint.GetField().GetField().GetFeature_field().IsSetField() && constraint.GetField().GetField().GetFeature_field().GetField().IsLegal_qual()
             && constraint.GetField().GetField().GetFeature_field().GetField().GetLegal_qual() == eFeat_qual_legal_product 
             && constraint.GetField().IsSetString_constraint())
        c->Assign(constraint.GetField().GetString_constraint());
    else
        return;

    string match_text = c->GetMatch_text();
   
    
    
    if (c->IsSetIgnore_words())
    {
        m_word_subst.Reset(new objects::CWord_substitution_set);
        m_word_subst->Assign(c->GetIgnore_words());
    }

    if (c->IsSetNot_present() && c->GetNot_present())
    {
        switch (c->GetMatch_location())
        {
        case eString_location_contains : m_MatchType->SetSelection(eMatchType_DoesNotContain); break;
        case eString_location_equals : m_MatchType->SetSelection(eMatchType_DoesNotEqual); break;
        case eString_location_starts : m_MatchType->SetSelection(eMatchType_DoesNotStartWith); break;
        case eString_location_ends : m_MatchType->SetSelection(eMatchType_DoesNotEndWith); break;
        case eString_location_inlist : m_MatchType->SetSelection(eMatchType_IsNotOneOf); break;
        default : break;
        }
    }
    else
    {
        switch (c->GetMatch_location())
        {
        case eString_location_contains : m_MatchType->SetSelection(eMatchType_Contains); break;
        case eString_location_equals : m_MatchType->SetSelection(eMatchType_Equals); break;
        case eString_location_starts : m_MatchType->SetSelection(eMatchType_StartsWith); break;
        case eString_location_ends : m_MatchType->SetSelection(eMatchType_EndsWith); break;
        case eString_location_inlist : m_MatchType->SetSelection(eMatchType_IsOneOf); break;
        default : break;
        }
    }
    
    if (c->IsSetCase_sensitive() && c->GetCase_sensitive())
      m_IgnoreCase->SetValue(false);
    else
      m_IgnoreCase->SetValue(true);
    
    if (c->IsSetIgnore_space() && c->GetIgnore_space())
      m_IgnoreSpace->SetValue(true);
    else
       m_IgnoreSpace->SetValue(false);
        
    if (c->IsSetIgnore_punct() && c->GetIgnore_punct())
      m_IgnorePunct->SetValue(true);
    else
      m_IgnorePunct->SetValue(false);
        
    if (c->IsSetWhole_word() && c->GetWhole_word())
      m_WholeWord->SetValue(true);
    else
      m_WholeWord->SetValue(false);
        
    if (c->IsSetIgnore_weasel() && c->GetIgnore_weasel())
      m_IgnoreSyn->SetValue(true);
    else
      m_IgnoreSyn->SetValue(false);

    if (c->IsSetIs_all_caps() && c->GetIs_all_caps())
      m_AllUpper->SetValue(true);
    else
      m_AllUpper->SetValue(false);
  
    if (c->IsSetIs_all_lower() && c->GetIs_all_lower())
      m_AllLower->SetValue(true);
    else
      m_AllLower->SetValue(false);
  
    if (c->IsSetIs_all_punct() && c->GetIs_all_punct())
      m_AllPunct->SetValue(true);
    else
      m_AllPunct->SetValue(false);

    ShowChoiceOrText();

    if (!m_is_choice)
        m_MatchText->SetValue(wxString(match_text));
    else 
        m_MatchChoice->SetValue(wxString(match_text));
}


END_NCBI_SCOPE
