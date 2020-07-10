/*  $Id: edit_feat_loc_panel.cpp 34894 2016-02-26 19:03:19Z asztalos $
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
#include <wx/stattext.h>
#include <wx/sizer.h>

#include <gui/packages/pkg_sequence_edit/edit_feat_loc_panel.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

/*!
 * CEditFeatLocPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CEditFeatLocPanel, wxPanel )


/*!
 * CEditFeatLocPanel event table definition
 */

BEGIN_EVENT_TABLE( CEditFeatLocPanel, wxPanel )

////@begin CEditFeatLocPanel event table entries
    EVT_CHOICE( ID_EFL_PARTIAL5, CEditFeatLocPanel::OnEflPartial5Selected )
    EVT_CHOICE( ID_EFL_PARTIAL3, CEditFeatLocPanel::OnEflPartial3Selected )
////@end CEditFeatLocPanel event table entries

END_EVENT_TABLE()


/*!
 * CEditFeatLocPanel constructors
 */

CEditFeatLocPanel::CEditFeatLocPanel()
{
    Init();
}

CEditFeatLocPanel::CEditFeatLocPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CEditFeatLocPanel creator
 */

bool CEditFeatLocPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditFeatLocPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEditFeatLocPanel creation
    return true;
}


/*!
 * CEditFeatLocPanel destructor
 */

CEditFeatLocPanel::~CEditFeatLocPanel()
{
////@begin CEditFeatLocPanel destruction
////@end CEditFeatLocPanel destruction
}


/*!
 * Member initialisation
 */

void CEditFeatLocPanel::Init()
{
////@begin CEditFeatLocPanel member initialisation
    m_Adjust5Partial = NULL;
    m_Extend5Btn = NULL;
    m_Adjust3Partial = NULL;
    m_Extend3Btn = NULL;
    m_JoinOrderMerge = NULL;
    m_RetranslateBtn = NULL;
    m_AdjustGeneBtn = NULL;
////@end CEditFeatLocPanel member initialisation
    m_Retranslate = false;
    m_AdjustGene = false;
}


/*!
 * Control creation for CEditFeatLocPanel
 */

void CEditFeatLocPanel::CreateControls()
{    
////@begin CEditFeatLocPanel content construction
    CEditFeatLocPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Partials:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer5->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("5' partial:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_Adjust5PartialStrings;
    m_Adjust5PartialStrings.Add(_("Set"));
    m_Adjust5PartialStrings.Add(_("Set only if at 5' end"));
    m_Adjust5PartialStrings.Add(_("Set if bad start codon"));
    m_Adjust5PartialStrings.Add(_("Set if CDS frame > 1"));
    m_Adjust5PartialStrings.Add(_("Clear"));
    m_Adjust5PartialStrings.Add(_("Clear if not at 5' end"));
    m_Adjust5PartialStrings.Add(_("Clear if good start codon"));
    m_Adjust5PartialStrings.Add(_("Do not change"));
    m_Adjust5Partial = new wxChoice( itemPanel1, ID_EFL_PARTIAL5, wxDefaultPosition, wxDefaultSize, m_Adjust5PartialStrings, 0 );
    m_Adjust5Partial->SetStringSelection(_("Do not change"));
    itemBoxSizer6->Add(m_Adjust5Partial, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Extend5Btn = new wxCheckBox( itemPanel1, ID_EFL_EXTEND5, _("Extend to 5' end of sequence if setting 5' partial"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Extend5Btn->SetValue(false);
    m_Extend5Btn->Enable(false);
    itemBoxSizer5->Add(m_Extend5Btn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer5->Add(itemBoxSizer10, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("3' partial:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_Adjust3PartialStrings;
    m_Adjust3PartialStrings.Add(_("Set"));
    m_Adjust3PartialStrings.Add(_("Set only if at 3' end"));
    m_Adjust3PartialStrings.Add(_("Set if bad stop codon"));
    m_Adjust3PartialStrings.Add(_("Clear"));
    m_Adjust3PartialStrings.Add(_("Clear if not at 3' end"));
    m_Adjust3PartialStrings.Add(_("Clear if good stop codon"));
    m_Adjust3PartialStrings.Add(_("Do not change"));
    m_Adjust3Partial = new wxChoice( itemPanel1, ID_EFL_PARTIAL3, wxDefaultPosition, wxDefaultSize, m_Adjust3PartialStrings, 0 );
    m_Adjust3Partial->SetStringSelection(_("Do not change"));
    itemBoxSizer10->Add(m_Adjust3Partial, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Extend3Btn = new wxCheckBox( itemPanel1, ID_EFL_EXTEND3, _("Extend to 3' end of sequence if setting 3' partial"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Extend3Btn->SetValue(false);
    m_Extend3Btn->Enable(false);
    itemBoxSizer5->Add(m_Extend3Btn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel1, wxID_STATIC, _("Join/Order/Merge:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText14, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_JoinOrderMergeStrings;
    m_JoinOrderMergeStrings.Add(_("Do not convert location"));
    m_JoinOrderMergeStrings.Add(_("Convert location to join"));
    m_JoinOrderMergeStrings.Add(_("Convert location to order"));
    m_JoinOrderMergeStrings.Add(_("Convert location to single interval"));
    m_JoinOrderMerge = new wxChoice( itemPanel1, ID_EFL_JOM, wxDefaultPosition, wxDefaultSize, m_JoinOrderMergeStrings, 0 );
    m_JoinOrderMerge->SetStringSelection(_("Do not convert location"));
    itemFlexGridSizer3->Add(m_JoinOrderMerge, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RetranslateBtn = new wxCheckBox( itemPanel1, ID_EFL_RETRANSLATE, _("Retranslate coding regions"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RetranslateBtn->SetValue(false);
    itemBoxSizer2->Add(m_RetranslateBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AdjustGeneBtn = new wxCheckBox( itemPanel1, ID_EFL_ADJUST_GENE, _("Adjust gene location"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AdjustGeneBtn->SetValue(false);
    itemBoxSizer2->Add(m_AdjustGeneBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CEditFeatLocPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CEditFeatLocPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditFeatLocPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEditFeatLocPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEditFeatLocPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEditFeatLocPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEditFeatLocPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEditFeatLocPanel icon retrieval
}


edit::CLocationEditPolicy::EPartialPolicy CEditFeatLocPanel::x_GetPartial5Policy()
{
    edit::CLocationEditPolicy::EPartialPolicy policy5 = edit::CLocationEditPolicy::ePartialPolicy_eNoChange;
    int val = m_Adjust5Partial->GetSelection();
    switch (val) {
        case 0:
            policy5 = edit::CLocationEditPolicy::ePartialPolicy_eSet;
            break;
        case 1:
            policy5 = edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd;
            break;
        case 2:
            policy5 = edit::CLocationEditPolicy::ePartialPolicy_eSetForBadEnd;
            break;
        case 3:
            policy5 = edit::CLocationEditPolicy::ePartialPolicy_eSetForFrame;
            break;
        case 4:
            policy5 = edit::CLocationEditPolicy::ePartialPolicy_eClear;
            break;
        case 5:
            policy5 = edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd;
            break;
        case 6:
            policy5 = edit::CLocationEditPolicy::ePartialPolicy_eClearForGoodEnd;
            break;
    }
    return policy5;
}


void CEditFeatLocPanel::x_SetPartial5Policy(edit::CLocationEditPolicy::EPartialPolicy partial5)
{
    switch (partial5) {
        case edit::CLocationEditPolicy::ePartialPolicy_eNoChange:
            m_Adjust5Partial->SetSelection(7);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eSet:
            m_Adjust5Partial->SetSelection(0);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd:
            m_Adjust5Partial->SetSelection(1);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eSetForBadEnd:
            m_Adjust5Partial->SetSelection(2);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eSetForFrame:
            m_Adjust5Partial->SetSelection(3);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eClear:
            m_Adjust5Partial->SetSelection(4);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd:
            m_Adjust5Partial->SetSelection(5);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eClearForGoodEnd:
            m_Adjust5Partial->SetSelection(6);
            break;
    }
}


edit::CLocationEditPolicy::EPartialPolicy CEditFeatLocPanel::x_GetPartial3Policy()
{
    edit::CLocationEditPolicy::EPartialPolicy policy3 = edit::CLocationEditPolicy::ePartialPolicy_eNoChange;
    int val = m_Adjust3Partial->GetSelection();
    switch (val) {
        case 0:
            policy3 = edit::CLocationEditPolicy::ePartialPolicy_eSet;
            break;
        case 1:
            policy3 = edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd;
            break;
        case 2:
            policy3 = edit::CLocationEditPolicy::ePartialPolicy_eSetForBadEnd;
            break;
        case 3:
            policy3 = edit::CLocationEditPolicy::ePartialPolicy_eClear;
            break;
        case 4:
            policy3 = edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd;
            break;
        case 5:
            policy3 = edit::CLocationEditPolicy::ePartialPolicy_eClearForGoodEnd;
            break;
    }
    return policy3;
}


void CEditFeatLocPanel::x_SetPartial3Policy(edit::CLocationEditPolicy::EPartialPolicy partial3)
{
    switch (partial3) {
        case edit::CLocationEditPolicy::ePartialPolicy_eNoChange:
            m_Adjust3Partial->SetSelection(6);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eSet:
            m_Adjust3Partial->SetSelection(0);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd:
            m_Adjust3Partial->SetSelection(1);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eSetForBadEnd:
        case edit::CLocationEditPolicy::ePartialPolicy_eSetForFrame:
            m_Adjust3Partial->SetSelection(2);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eClear:
            m_Adjust3Partial->SetSelection(3);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd:
            m_Adjust3Partial->SetSelection(4);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eClearForGoodEnd:
            m_Adjust3Partial->SetSelection(5);
            break;      
    }
}


edit::CLocationEditPolicy::EMergePolicy CEditFeatLocPanel::x_GetMergePolicy()
{
    edit::CLocationEditPolicy::EMergePolicy merge = edit::CLocationEditPolicy::eMergePolicy_NoChange;
    int val = m_JoinOrderMerge->GetSelection();
    switch (val) {
        case 0:
            merge = edit::CLocationEditPolicy::eMergePolicy_NoChange;
            break;
        case 1:
            merge = edit::CLocationEditPolicy::eMergePolicy_Join;
            break;
        case 2:
            merge = edit::CLocationEditPolicy::eMergePolicy_Order;
            break;
        case 3:
            merge = edit::CLocationEditPolicy::eMergePolicy_SingleInterval;
            break;
    }
    return merge;
}


void CEditFeatLocPanel::x_SetMergePolicy(edit::CLocationEditPolicy::EMergePolicy merge)
{
    switch (merge) {
        case edit::CLocationEditPolicy::eMergePolicy_NoChange:
            m_JoinOrderMerge->SetSelection(0);
            break;
        case edit::CLocationEditPolicy::eMergePolicy_Join:
            m_JoinOrderMerge->SetSelection(1);
            break;
        case edit::CLocationEditPolicy::eMergePolicy_Order:
            m_JoinOrderMerge->SetSelection(2);
            break;
        case edit::CLocationEditPolicy::eMergePolicy_SingleInterval:
            m_JoinOrderMerge->SetSelection(3);
            break;
    }
}


void CEditFeatLocPanel::x_EnableExtend5Btn()
{
     edit::CLocationEditPolicy::EPartialPolicy policy5 = x_GetPartial5Policy();
     switch (policy5) {
        case edit::CLocationEditPolicy::ePartialPolicy_eNoChange:
        case edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd:
        case edit::CLocationEditPolicy::ePartialPolicy_eClear:
        case edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd:
        case edit::CLocationEditPolicy::ePartialPolicy_eClearForGoodEnd:
            m_Extend5Btn->Enable(false);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eSet:
        case edit::CLocationEditPolicy::ePartialPolicy_eSetForBadEnd:
        case edit::CLocationEditPolicy::ePartialPolicy_eSetForFrame:
            m_Extend5Btn->Enable(true);
            break;
     }
}


void CEditFeatLocPanel::x_EnableExtend3Btn()
{
     edit::CLocationEditPolicy::EPartialPolicy policy3 = x_GetPartial3Policy();
     switch (policy3) {
        case edit::CLocationEditPolicy::ePartialPolicy_eNoChange:
        case edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd:
        case edit::CLocationEditPolicy::ePartialPolicy_eClear:
        case edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd:
        case edit::CLocationEditPolicy::ePartialPolicy_eClearForGoodEnd:
            m_Extend3Btn->Enable(false);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eSet:
        case edit::CLocationEditPolicy::ePartialPolicy_eSetForBadEnd:
        case edit::CLocationEditPolicy::ePartialPolicy_eSetForFrame:
            m_Extend3Btn->Enable(true);
            break;
     }
}


void CEditFeatLocPanel::SetPolicy(CRef<edit::CLocationEditPolicy> policy)
{
    if (policy) {
        x_SetPartial5Policy(policy->GetPartial5Policy());
        x_SetPartial3Policy(policy->GetPartial3Policy());
        x_SetMergePolicy(policy->GetMergePolicy());
        m_Extend5Btn->SetValue(policy->GetExtend5());
        m_Extend3Btn->SetValue(policy->GetExtend3());
    } else {
        x_SetPartial5Policy(edit::CLocationEditPolicy::ePartialPolicy_eNoChange);
        x_SetPartial3Policy(edit::CLocationEditPolicy::ePartialPolicy_eNoChange);
        x_SetMergePolicy(edit::CLocationEditPolicy::eMergePolicy_NoChange);
        m_Extend5Btn->SetValue(false);
        m_Extend3Btn->SetValue(false);
    }
    x_EnableExtend5Btn();
    x_EnableExtend3Btn();
}


CRef<edit::CLocationEditPolicy> CEditFeatLocPanel::GetPolicy()
{
    edit::CLocationEditPolicy::EPartialPolicy policy5 = x_GetPartial5Policy();
    edit::CLocationEditPolicy::EPartialPolicy policy3 = x_GetPartial3Policy();
    edit::CLocationEditPolicy::EMergePolicy merge = x_GetMergePolicy();
    bool extend5 = m_Extend5Btn->GetValue();
    bool extend3 = m_Extend3Btn->GetValue();

    CRef<edit::CLocationEditPolicy> policy(new edit::CLocationEditPolicy(policy5, policy3, extend5, extend3, merge));
    return policy;
}


void CEditFeatLocPanel::SetRetranslate(bool val)
{
    m_Retranslate = val;
    m_RetranslateBtn->SetValue(val);
}


bool CEditFeatLocPanel::GetRetranslate()
{
    return m_RetranslateBtn->GetValue();
}


void CEditFeatLocPanel::SetAdjustGene(bool val)
{
    m_AdjustGene = val;
    m_AdjustGeneBtn->SetValue(val);
}


bool CEditFeatLocPanel::GetAdjustGene()
{
    return m_AdjustGeneBtn->GetValue();
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_PARTIAL5
 */

void CEditFeatLocPanel::OnEflPartial5Selected( wxCommandEvent& event )
{
     x_EnableExtend5Btn();
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_PARTIAL3
 */

void CEditFeatLocPanel::OnEflPartial3Selected( wxCommandEvent& event )
{
     edit::CLocationEditPolicy::EPartialPolicy policy3 = x_GetPartial3Policy();
     switch (policy3) {
        case edit::CLocationEditPolicy::ePartialPolicy_eNoChange:
        case edit::CLocationEditPolicy::ePartialPolicy_eSetAtEnd:
        case edit::CLocationEditPolicy::ePartialPolicy_eClear:
        case edit::CLocationEditPolicy::ePartialPolicy_eClearNotAtEnd:
        case edit::CLocationEditPolicy::ePartialPolicy_eClearForGoodEnd:
            m_Extend3Btn->Enable(false);
            break;
        case edit::CLocationEditPolicy::ePartialPolicy_eSet:
        case edit::CLocationEditPolicy::ePartialPolicy_eSetForBadEnd:
        case edit::CLocationEditPolicy::ePartialPolicy_eSetForFrame:
            m_Extend3Btn->Enable(true);
            break;
     }
}


END_NCBI_SCOPE
