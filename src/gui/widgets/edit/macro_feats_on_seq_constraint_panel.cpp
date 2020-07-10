/*  $Id: macro_feats_on_seq_constraint_panel.cpp 44072 2019-10-21 17:57:10Z asztalos $
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


#include <gui/objutils/macro_fn_where.hpp>
#include <gui/objutils/macro_fn_seq_constr.hpp>
#include <gui/objutils/macro_fn_loc_constr.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/macro_editor_context.hpp>
#include <gui/widgets/edit/molinfo_fieldtype.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>
#include <gui/widgets/edit/macro_constraint_panel.hpp>
#include <gui/widgets/edit/macro_feats_on_seq_constraint_panel.hpp>


BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*
 * CMacroFeatsOnSeqPanel type definition
 */

IMPLEMENT_CLASS( CMacroFeatsOnSeqPanel, wxPanel )


/*
 * CMacroFeatsOnSeqPanel event table definition
 */

BEGIN_EVENT_TABLE( CMacroFeatsOnSeqPanel, wxPanel )
    EVT_UPDATE_UI(ID_MACRO_FEATS_LENGTHTEXT, CMacroFeatsOnSeqPanel::OnLengthTextCtrlUpdate)
    EVT_CHOICE(ID_MACRO_FEATS_DISTCHOICE1, CMacroFeatsOnSeqPanel::OnDistanceSelected)
    EVT_CHOICE(ID_MACRO_FEATS_DISTCHOICE2, CMacroFeatsOnSeqPanel::OnDistanceSelected)
END_EVENT_TABLE()


/*
 * CMacroFeatsOnSeqPanel constructors
 */

CMacroFeatsOnSeqPanel::CMacroFeatsOnSeqPanel()
{
    Init();
}

CMacroFeatsOnSeqPanel::CMacroFeatsOnSeqPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, pos, size, style );
}


/*
 * CMacroFeatsOnSeqPanel creator
 */

bool CMacroFeatsOnSeqPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroFeatsOnSeqPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    Centre();
////@end CMacroFeatsOnSeqPanel creation
    return true;
}


/*
 * CMacroFeatsOnSeqPanel destructor
 */

CMacroFeatsOnSeqPanel::~CMacroFeatsOnSeqPanel()
{
////@begin CMacroFeatsOnSeqPanel destruction
////@end CMacroFeatsOnSeqPanel destruction
}


/*
 * Member initialisation
 */

void CMacroFeatsOnSeqPanel::Init()
{
////@begin CMacroFeatsOnSeqPanel member initialisation
    m_FeatureType = NULL;
    m_FeatChoice = NULL;
    m_FeatCount = NULL;
    m_LengthChoice = NULL;
    m_Length = NULL;
    m_SeqReprChoice = NULL;
    m_Dist5EndChoice = NULL;
    m_Dist3EndChoice = NULL;
    m_5EndDistance = NULL;
    m_3EndDistance = NULL;
    m_FirstFeatureChoice = NULL;
    m_LastFeatureChoice = NULL;
////@end CMacroFeatsOnSeqPanel member initialisation
}


/*
 * Control creation for CMacroFeatsOnSeqPanel
 */

void CMacroFeatsOnSeqPanel::CreateControls()
{    
////@begin CMacroFeatsOnSeqPanel content construction
    CMacroFeatsOnSeqPanel* itemPanel2 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBoxSizer1);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Number of features on sequence"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxHORIZONTAL);
    itemBoxSizer1->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    m_FeatureType = new CFeatureTypePanel( itemPanel2, wxID_ANY, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
    itemStaticBoxSizer4->Add(m_FeatureType, 0, wxALIGN_TOP|wxALL, 5);
    objects::CSeq_entry_Handle seh;
    m_FeatureType->ListPresentFeaturesFirst(seh);
    m_FeatureType->ListAnyOption();
    m_FeatureType->ListAllOption();

    wxArrayString countTypeStrings;
    countTypeStrings.Add(_("Exactly"));
    countTypeStrings.Add(_("Fewer than"));
    countTypeStrings.Add(_("Fewer than or equal to"));
    countTypeStrings.Add(_("More than"));
    countTypeStrings.Add(_("More than or equal to"));
    m_FeatChoice = new wxChoice( itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, countTypeStrings, 0 );
    m_FeatChoice->SetSelection(0);
    itemStaticBoxSizer4->Add(m_FeatChoice, 0, wxALIGN_TOP|wxALL, 5);

    m_FeatCount = new wxTextCtrl( itemPanel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add(m_FeatCount, 0, wxALIGN_TOP|wxALL, 5);
    m_FeatCount->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer1->Add(itemFlexGridSizer5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0);

    wxStaticText* itemStaticText6 = new wxStaticText(itemPanel2, wxID_STATIC, _("Length of sequence"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_RIGHT | wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer5->Add(itemBoxSizer7, 0, wxALIGN_LEFT, 0);

    countTypeStrings.Insert("Any", 0);
    m_LengthChoice = new wxChoice(itemPanel2, ID_MACRO_FEATS_CHOICE, wxDefaultPosition, wxDefaultSize, countTypeStrings, 0);
    m_LengthChoice->SetSelection(0);
    itemBoxSizer7->Add(m_LengthChoice, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_Length = new wxTextCtrl(itemPanel2, ID_MACRO_FEATS_LENGTHTEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer7->Add(m_Length, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_Length->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

    wxStaticText* itemStaticText8 = new wxStaticText(itemPanel2, wxID_STATIC, _("Sequence representation"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer5->Add(itemStaticText8, 0, wxALIGN_RIGHT | wxALL, 5);

    wxArrayString seqReprStrings;
    bool allow_other = false;
    vector<string> choices = CMolInfoFieldType::GetChoicesForField(CMolInfoFieldType::e_Repr, allow_other);
    ToArrayString(choices, seqReprStrings);
    seqReprStrings.Insert("Default", 0);
    m_SeqReprChoice = new wxChoice(itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, seqReprStrings, 0);
    m_SeqReprChoice->SetSelection(0);
    itemFlexGridSizer5->Add(m_SeqReprChoice, 0, wxALIGN_LEFT | wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer1->Add(itemFlexGridSizer8, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText(itemPanel2, wxID_STATIC, _("There are"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer8->Add(itemStaticText9, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer8->Add(itemBoxSizer10, 0, wxALIGN_LEFT | wxALL, 0);

    wxArrayString distancetypes;
    distancetypes.Add(_("Any distance"));
    distancetypes.Add(_("Exactly"));
    distancetypes.Add(_("At least"));
    distancetypes.Add(_("At most"));
    m_Dist5EndChoice = new wxChoice(itemPanel2, ID_MACRO_FEATS_DISTCHOICE1, wxDefaultPosition, wxDefaultSize, distancetypes, 0);
    m_Dist5EndChoice->SetSelection(0);
    itemBoxSizer10->Add(m_Dist5EndChoice, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_5EndDistance = new wxTextCtrl(itemPanel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer10->Add(m_5EndDistance, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_5EndDistance->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_5EndDistance->Disable();

    wxStaticText* itemStaticText11 = new wxStaticText(itemPanel2, wxID_STATIC, _("nucleotides before the first"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxArrayString features = x_GetFeatureChoices();
    m_FirstFeatureChoice = new wxChoice(itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, features, 0);
    m_FirstFeatureChoice->SetSelection(0);
    itemBoxSizer10->Add(m_FirstFeatureChoice, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText(itemPanel2, wxID_STATIC, _("There are"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer8->Add(itemStaticText12, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL |wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer8->Add(itemBoxSizer13, 0, wxALIGN_LEFT | wxALL, 0);

    m_Dist3EndChoice = new wxChoice(itemPanel2, ID_MACRO_FEATS_DISTCHOICE2, wxDefaultPosition, wxDefaultSize, distancetypes, 0);
    m_Dist3EndChoice->SetSelection(0);
    itemBoxSizer13->Add(m_Dist3EndChoice, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_3EndDistance = new wxTextCtrl(itemPanel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer13->Add(m_3EndDistance, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_3EndDistance->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_3EndDistance->Disable();

    wxStaticText* itemStaticText14 = new wxStaticText(itemPanel2, wxID_STATIC, _("nucleotides after the last"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_LastFeatureChoice = new wxChoice(itemPanel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, features, 0);
    m_LastFeatureChoice->SetSelection(0);
    itemBoxSizer13->Add(m_LastFeatureChoice, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

////@end CMacroFeatsOnSeqPanel content construction
}

// includes the feature names and the option 'Any'
wxArrayString CMacroFeatsOnSeqPanel::x_GetFeatureChoices()
{
    set<string> existing;
    map<pair<int, int>, string> types;
    vector<string> features;

    CSeq_entry_Handle seh;
    vector<const CFeatListItem *> featlist = GetSortedFeatList(seh);

    ITERATE(vector<const CFeatListItem *>, feat_it, featlist) {
        const CFeatListItem& item = **feat_it;
        string desc = item.GetDescription();
        int feat_type = item.GetType();
        int feat_subtype = item.GetSubtype();
        types[pair<int, int>(feat_type, feat_subtype)] = desc;
        if (existing.find(desc) == existing.end()) {
            existing.insert(desc);
            features.push_back(desc);
        }
    }

    wxArrayString selected_features;
    selected_features.push_back(_("Any"));
    bool show_legal_only = false;
    for (size_t i = 1; i < features.size(); ++i) {
        CSeqFeatData::ESubtype subtype = CSeqFeatData::SubtypeNameToValue(features[i]);
        bool legal = !show_legal_only || !CSeqFeatData::GetLegalQualifiers(subtype).empty();
        if ((!CFeatureTypePanel::s_IsRarelyUsedOrDiscouragedFeatureType(subtype) && !CSeqFeatData::IsRegulatory(subtype) && legal)) {
            selected_features.push_back(ToWxString(features[i]));
        }
    }

    return selected_features;
}


void CMacroFeatsOnSeqPanel::OnLengthTextCtrlUpdate(wxUpdateUIEvent &event)
{
    bool enable = m_LengthChoice->GetSelection() > 0;
    event.Enable(enable);
}

/*
 * Should we show tooltips?
 */

bool CMacroFeatsOnSeqPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CMacroFeatsOnSeqPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroFeatsOnSeqPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroFeatsOnSeqPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CMacroFeatsOnSeqPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroFeatsOnSeqPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroFeatsOnSeqPanel icon retrieval
}

pair<string,string> CMacroFeatsOnSeqPanel::GetMatcher(const pair<string, string> &target, size_t num)
{
  string feat = m_FeatureType->GetFieldName();
  if (NStr::EqualNocase(feat, "any")) 
    {
        feat = "any";
    }
  if (NStr::EqualNocase(feat, "all")) 
    {
      feat = "all";
    }
  return make_pair(GetDescription(feat), GetLocationConstraint(feat, target.first)); 
}

void CMacroFeatsOnSeqPanel::SetMatcher(const string &item)
{
// TODO
}

string CMacroFeatsOnSeqPanel::GetLocationConstraint(const string &feat, const string& target)
{
    vector<string> constraints;

    int repr_index = m_SeqReprChoice->GetSelection();
    if (repr_index > 0) {
        string repr_asn = "inst.repr";
        string str = repr_asn;
        if (CFieldNamePanel::IsFeature(target)) {
            str = macro::CMacroFunction_GetSequence::sm_SeqForFeat + string("(\"" + repr_asn +"\")");
        }
        else if (CFieldNamePanel::IsDescriptor(target)) {
            str = macro::CMacroFunction_GetSequence::sm_SeqForDescr + string("(\"" + repr_asn + "\")");
        }

        string repr = CMolInfoFieldType::GetFieldName(CMolInfoFieldType::e_Repr);
        string value = m_SeqReprChoice->GetString(repr_index).ToStdString();

        str += " = \"" + CMacroEditorContext::GetInstance().GetAsnMolinfoValue(repr, value) + "\"";
        constraints.push_back(str);
    }

    if (!m_Length->GetValue().IsEmpty()) {
        string repr_asn = "inst.length";
        string str = repr_asn;
        if (CFieldNamePanel::IsFeature(target)) {
            str = macro::CMacroFunction_GetSequence::sm_SeqForFeat + string("(\"" + repr_asn + "\")");
        }
        else if (CFieldNamePanel::IsDescriptor(target)) {
            str = macro::CMacroFunction_GetSequence::sm_SeqForDescr + string("(\"" + repr_asn + "\")");
        }

        string d = m_Length->GetValue().ToStdString();
        switch (m_LengthChoice->GetSelection()) {
        case 1: str += " = " + d; break;
        case 2: str += " < " + d; break;
        case 3: str += " <= " + d; break;
        case 4: str += " > " + d; break;
        case 5: str += " >= " + d; break;
        default: str.clear();  break;
        }
        if (!str.empty())
            constraints.push_back(str);
    }

    if (m_FeatChoice->GetSelection() >= 0 && !m_FeatCount->GetValue().IsEmpty() && !feat.empty())
    {
        string d = m_FeatCount->GetValue().ToStdString();
        string str = macro::CMacroFunction_NumberOfFeatures::sm_FunctionName;
        str += "(\"" + feat + "\")";
        switch (m_FeatChoice->GetSelection())
        {
        case 0: str += " = " + d; break;
        case 1: str += " < " + d; break;
        case 2: str += " <= " + d; break;
        case 3: str += " > " + d; break;
        case 4: str += " >= " + d; break;
        default: str.clear();  break;
        }
        if (!str.empty())
            constraints.push_back(str);
    }

    if (m_Dist5EndChoice->GetSelection() > 0 && !m_5EndDistance->IsEmpty()) {
        string str = macro::CMacroFunction_LocationDistConstraint::sm_FromStart;
        str += "(" + string(macro::CMacroFunction_FirstOrLastItem::sm_First) + "(";
        str += macro::CMacroFunction_Features_For_Object::GetFuncName();

        string feature = m_FirstFeatureChoice->GetStringSelection().ToStdString();
        if (NStr::EqualNocase(feature, "any"))
            feature = "any";
        str += "(\"" + feature + "\")))";

        string d = m_5EndDistance->GetValue().ToStdString();
        switch (m_Dist5EndChoice->GetSelection()) {
        case 1: str += " = " + d; break;
        case 2: str += " >= " + d; break;
        case 3: str += " <= " + d; break;
        default: str.clear(); break;
        }

        if (!str.empty())
            constraints.push_back(str);
    }

    if (m_Dist3EndChoice->GetSelection() > 0 && !m_3EndDistance->IsEmpty()) {
        string str = macro::CMacroFunction_LocationDistConstraint::sm_FromStop;
        str += "(" + string(macro::CMacroFunction_FirstOrLastItem::sm_Last) + "(";
        str += macro::CMacroFunction_Features_For_Object::GetFuncName();

        string feature = m_LastFeatureChoice->GetStringSelection().ToStdString();
        if (NStr::EqualNocase(feature, "any"))
            feature = "any";
        str += "(\"" + feature + "\")))";

        string d = m_3EndDistance->GetValue().ToStdString();
        switch (m_Dist3EndChoice->GetSelection()) {
        case 1: str += " = " + d; break;
        case 2: str += " >= " + d; break;
        case 3: str += " <= " + d; break;
        default: str.clear(); break;
        }

        if (!str.empty())
            constraints.push_back(str);
    }

    if (!constraints.empty()) {
        return NStr::Join(constraints, " AND ");
    }
    return kEmptyStr;
}

string CMacroFeatsOnSeqPanel::GetDescription(const string& feat) const
{
    vector<string> descriptions;

    int repr_index = m_SeqReprChoice->GetSelection();
    if (repr_index > 0) {
        string value = m_SeqReprChoice->GetString(repr_index).ToStdString();
        string label = "sequence is " + value;
        descriptions.push_back(label);
    }

    if (!m_Length->GetValue().IsEmpty()) {
        string label = "sequence length is ";

        switch (m_LengthChoice->GetSelection()) {
        case 1: label += "exactly "; break;
        case 2: label += "fewer than "; break;
        case 3: label += "fewer than or equal to "; break;
        case 4: label += "more than "; break;
        case 5: label += "more than or equal to "; break;
        default: break;
        }
        label += m_Length->GetValue().ToStdString();
        descriptions.push_back(label);
    }

    if (m_FeatChoice->GetSelection() >= 0 && !m_FeatCount->GetValue().IsEmpty() && !feat.empty() )
    {
      string label = "the number of "+ feat + " features on sequence is ";
      switch (m_FeatChoice->GetSelection())
        {
        case 0: label += "exactly "; break;
        case 1: label += "fewer than "; break;
        case 2: label += "fewer than or equal to "; break;
        case 3: label += "more than "; break;
        case 4: label += "more than or equal to "; break;
        default: break;
        }
        label += m_FeatCount->GetValue().ToStdString();
        descriptions.push_back(label);
    }

    if (m_Dist5EndChoice->GetSelection() > 0 && !m_5EndDistance->IsEmpty()) {
        string label = "there are ";
        
        switch (m_Dist5EndChoice->GetSelection()) {
        case 1: label += "exactly "; break;
        case 2: label += "at least "; break;
        case 3: label += "at most "; break;
        default: label.clear(); break;
        }

        label += m_5EndDistance->GetValue().ToStdString();
        label += " nucleotides before the first ";
        
        string feature = m_FirstFeatureChoice->GetStringSelection().ToStdString();
        if (!NStr::EqualNocase(feature, "any"))
            label += feature + " ";

        label += "feature";
        descriptions.push_back(label);
    }

    if (m_Dist3EndChoice->GetSelection() > 0 && !m_3EndDistance->IsEmpty()) {
        string label = "there are ";

        switch (m_Dist3EndChoice->GetSelection()) {
        case 1: label += "exactly "; break;
        case 2: label += "at least "; break;
        case 3: label += "at most "; break;
        default: label.clear(); break;
        }

        label += m_3EndDistance->GetValue().ToStdString();
        label += " nucleotides after the last ";

        string feature = m_LastFeatureChoice->GetStringSelection().ToStdString();
        if (!NStr::EqualNocase(feature, "any"))
            label += feature + " ";

        label += "feature";
        descriptions.push_back(label);
    }

    if (!descriptions.empty()) {
        return NStr::Join(descriptions, " and ");
    }
    return kEmptyStr;
}

void CMacroFeatsOnSeqPanel::OnDistanceSelected(wxCommandEvent& event)
{
    if (event.GetId() == ID_MACRO_FEATS_DISTCHOICE1) {
        if (m_Dist5EndChoice->GetSelection() > 0) {
            m_5EndDistance->Enable();
        }
        else {
            m_5EndDistance->SetValue(wxEmptyString);
            m_5EndDistance->Disable();
        }
    }
    else if (event.GetId() == ID_MACRO_FEATS_DISTCHOICE2) {
        if (m_Dist3EndChoice->GetSelection() > 0) {
            m_3EndDistance->Enable();
        }
        else {
            m_3EndDistance->SetValue(wxEmptyString);
            m_3EndDistance->Disable();
        }
    }
}


END_NCBI_SCOPE
