/*  $Id: broadcast_settings_dlg.cpp 35786 2016-06-23 15:40:09Z katargir $
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
 * Authors:
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/broadcast_settings_dlg.hpp>
#include <gui/objutils/obj_event.hpp>

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/icon.h>
#include <wx/editlbox.h>
#include <wx/hyperlink.h>

////@begin includes
#include "wx/imaglist.h"
////@end includes

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CBroadcastDialog, wxDialog )

BEGIN_EVENT_TABLE( CBroadcastDialog, wxDialog )

////@begin CBroadcastDialog event table entries
    EVT_BUTTON( wxID_APPLY, CBroadcastDialog::OnApplyClick )

    EVT_BUTTON( ID_SET_DEFAULTS_BTN, CBroadcastDialog::OnSetDefaultsBtnClick )

////@end CBroadcastDialog event table entries

END_EVENT_TABLE()

CBroadcastDialog::CBroadcastDialog()
{
    Init();
}

CBroadcastDialog::CBroadcastDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CBroadcastDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBroadcastDialog creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBroadcastDialog creation
    return true;
}

CBroadcastDialog::~CBroadcastDialog()
{
////@begin CBroadcastDialog destruction
////@end CBroadcastDialog destruction
}

void CBroadcastDialog::Init()
{
////@begin CBroadcastDialog member initialisation
    m_BroadcastList = NULL;
    m_SelectOneToOne = NULL;
////@end CBroadcastDialog member initialisation
}

void CBroadcastDialog::CreateControls()
{
////@begin CBroadcastDialog content construction
    CBroadcastDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxNotebook* itemNotebook3 = new wxNotebook( itemDialog1, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );

    wxPanel* itemPanel4 = new wxPanel( itemNotebook3, ID_GENERAL_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemPanel4->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel4->SetSizer(itemBoxSizer5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer5->Add(itemBoxSizer6, 0, wxGROW|wxALL, 5);
    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemPanel4, wxID_ANY, _("Policy"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxHORIZONTAL);
    itemBoxSizer6->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 0);
    wxArrayString itemChoice8Strings;
    itemChoice8Strings.Add(_("Default"));
    itemChoice8Strings.Add(_("Strict"));
    itemChoice8Strings.Add(_("Relaxed"));
    wxChoice* itemChoice8 = new wxChoice( itemStaticBoxSizer7->GetStaticBox(), ID_POLICY_CHOICE, wxDefaultPosition, wxDefaultSize, itemChoice8Strings, 0 );
    itemChoice8->SetStringSelection(_("Default"));
    itemStaticBoxSizer7->Add(itemChoice8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemStaticBoxSizer7->GetStaticBox(), wxID_APPLY, _("&Apply"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer7->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemRadioBox10Strings;
    itemRadioBox10Strings.Add(_("Exact Match by all IDs"));
    itemRadioBox10Strings.Add(_("Match at least one ID"));
    itemRadioBox10Strings.Add(_("Match at least one ID, ignore version"));
    wxRadioBox* itemRadioBox10 = new wxRadioBox( itemPanel4, ID_SEQUENCE_MATCHING_RADIOBOX, _("Sequence Matching Selection Options"), wxDefaultPosition, wxDefaultSize, itemRadioBox10Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox10->SetSelection(0);
    itemBoxSizer6->Add(itemRadioBox10, 0, wxGROW|wxALL, 0);

    wxStaticBox* itemStaticBoxSizer11Static = new wxStaticBox(itemPanel4, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer11 = new wxStaticBoxSizer(itemStaticBoxSizer11Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer11, 0, wxGROW|wxALL, 0);
    wxCheckBox* itemCheckBox12 = new wxCheckBox( itemStaticBoxSizer11->GetStaticBox(), ID_USE_LOCATIONS_CHECKBOX, _("Use locations to identify alignment rows"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox12->SetValue(false);
    itemStaticBoxSizer11->Add(itemCheckBox12, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox13 = new wxCheckBox( itemStaticBoxSizer11->GetStaticBox(), ID_MATCH_BY_PRODUCTS_CHECKBOX, _("Match Features by Products"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox13->SetValue(false);
    itemStaticBoxSizer11->Add(itemCheckBox13, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox14 = new wxCheckBox( itemStaticBoxSizer11->GetStaticBox(), ID_PROJECTS_BROADCAST_CHECKBOX, _("Broadcast between Projects"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox14->SetValue(false);
    itemStaticBoxSizer11->Add(itemCheckBox14, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox15 = new wxCheckBox( itemPanel4, ID_AUTO_BROADCAST_SEL_CHECKBOX, _("Auto broadcast selection changes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox15->SetValue(false);
    itemBoxSizer6->Add(itemCheckBox15, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer6->Add(0, 10, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer5->Add(itemBoxSizer17, 0, wxGROW|wxALL, 5);
    wxArrayString itemRadioBox18Strings;
    itemRadioBox18Strings.Add(_("Track range changes"));
    itemRadioBox18Strings.Add(_("Track and Center range changes"));
    itemRadioBox18Strings.Add(_("Track and Expand range changes"));
    itemRadioBox18Strings.Add(_("Track and Contract range changes"));
    itemRadioBox18Strings.Add(_("Track and Follow ranges"));
    wxRadioBox* itemRadioBox18 = new wxRadioBox( itemPanel4, ID_VISIBLE_RANGE_RADIOBOX, _("Visible Range Synchronization Options"), wxDefaultPosition, wxDefaultSize, itemRadioBox18Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox18->SetSelection(0);
    itemBoxSizer17->Add(itemRadioBox18, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    wxCheckBox* itemCheckBox19 = new wxCheckBox( itemPanel4, ID_AUTO_BROADCAST_RANGE_CHECKBOX, _("Auto broadcast range changes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox19->SetValue(false);
    itemBoxSizer17->Add(itemCheckBox19, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer17->Add(0, 0, 1, wxALIGN_CENTER_HORIZONTAL, 0);

    itemNotebook3->AddPage(itemPanel4, _("General"));

    wxPanel* itemPanel21 = new wxPanel( itemNotebook3, ID_BIOTREE_CONTAINER_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemPanel21->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
    itemPanel21->SetSizer(itemBoxSizer22);

    wxStaticBox* itemStaticBoxSizer23Static = new wxStaticBox(itemPanel21, wxID_ANY, _("Properties for Comparing Trees in Prioritized Order"));
    wxStaticBoxSizer* itemStaticBoxSizer23 = new wxStaticBoxSizer(itemStaticBoxSizer23Static, wxVERTICAL);
    itemBoxSizer22->Add(itemStaticBoxSizer23, 0, wxGROW|wxALL, 5);
    m_BroadcastList = new wxEditableListBox( itemStaticBoxSizer23->GetStaticBox(), ID_BROADCAST_LISTBOX, _("Broadcast Properties"), wxDefaultPosition, wxDefaultSize );
    itemStaticBoxSizer23->Add(m_BroadcastList, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer25 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer22->Add(itemBoxSizer25, 1, wxGROW|wxALL, 5);
    m_SelectOneToOne = new wxCheckBox( itemPanel21, ID_ONE_TO_ONE_SELECT_CHECKBOX, _("Select Nodes from Views of Same Tree One-to-One"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SelectOneToOne->SetValue(false);
    itemBoxSizer25->Add(m_SelectOneToOne, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton27 = new wxButton( itemPanel21, ID_SET_DEFAULTS_BTN, _("Set Default Properties"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer25->Add(itemButton27, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl28 = new wxHyperlinkCtrl( itemPanel21, ID_HYPERLINKCTRL, _("Help"), _T("https://www.ncbi.nlm.nih.gov/tools/gbench/tutorial3A/#Ch1Step5"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer22->Add(itemHyperlinkCtrl28, 0, wxALIGN_LEFT|wxALL, 5);

    itemNotebook3->AddPage(itemPanel21, _("BioTree Settings"));

    itemBoxSizer2->Add(itemNotebook3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticLine* itemStaticLine29 = new wxStaticLine( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine29, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer30 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer30, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton31 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton31->SetDefault();
    itemStdDialogButtonSizer30->AddButton(itemButton31);

    wxButton* itemButton32 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer30->AddButton(itemButton32);

    itemStdDialogButtonSizer30->Realize();

    // Set validators
    itemChoice8->SetValidator( wxGenericValidator(& m_Policy) );
    itemRadioBox10->SetValidator( wxGenericValidator(& m_MatchRadios) );
    itemCheckBox12->SetValidator( wxGenericValidator(& m_MatchAlnLocs) );
    itemCheckBox13->SetValidator( wxGenericValidator(& m_MatchByProduct) );
    itemCheckBox14->SetValidator( wxGenericValidator(& m_BroadcastBetweenProjects) );
    itemCheckBox15->SetValidator( wxGenericValidator(& m_AutoSelBroadcast) );
    itemRadioBox18->SetValidator( wxGenericValidator(& m_VRBPolicy) );
    itemCheckBox19->SetValidator( wxGenericValidator(& m_AutoVRBroadcast) );
////@end CBroadcastDialog content construction
}

void CBroadcastDialog::SetData( const SBroadcastParams& params )
{
    m_MatchRadios = 1;
    if (params.m_BioseqMatching == CSelectionEvent::eAllIds)
        m_MatchRadios = 0;
    else if(params.m_SeqIDMatching == CSelectionEvent::eAccOnly)
        m_MatchRadios = 2;

    m_MatchByProduct = params.m_MatchByProduct;
    m_MatchAlnLocs = params.m_MatchAlnLocs;
    m_BroadcastBetweenProjects = params.m_BroadcastBetweenProjects;
    m_AutoSelBroadcast = params.m_AutoSelBroadcast;
    m_AutoVRBroadcast = params.m_AutoVRBroadcast;
    m_VRBPolicy = params.m_VRBPolicy;

    m_SelectOneToOne->SetValue(params.m_TreeBroadcastOneToOne);
    wxArrayString strings;
    for (size_t i=0; i<params.m_TreeBroadcastProperties.size();++i)
        strings.Add(params.m_TreeBroadcastProperties[i]);
    m_BroadcastList->SetStrings(strings);
}

SBroadcastParams CBroadcastDialog::GetData() const
{
    SBroadcastParams params;

    params.m_BioseqMatching = (m_MatchRadios == 0) ?
            CSelectionEvent::eAllIds : CSelectionEvent::eAtLeastOne;
    params.m_SeqIDMatching = (m_MatchRadios == 2) ?
            CSelectionEvent::eAccOnly : CSelectionEvent::eExact;

    params.m_MatchByProduct = m_MatchByProduct;
    params.m_MatchAlnLocs = m_MatchAlnLocs;
    params.m_BroadcastBetweenProjects = m_BroadcastBetweenProjects;
    params.m_AutoSelBroadcast = m_AutoSelBroadcast;
    params.m_AutoVRBroadcast = m_AutoVRBroadcast;
    params.m_VRBPolicy = m_VRBPolicy;

    params.m_TreeBroadcastOneToOne = m_SelectOneToOne->IsChecked();
    wxArrayString strings;
    m_BroadcastList->GetStrings(strings);
    params.m_TreeBroadcastProperties.clear();
    for (size_t i=0; i<strings.size();++i)
        params.m_TreeBroadcastProperties.push_back(strings[i].ToStdString());

    return params;
}

void CBroadcastDialog::OnApplyClick( wxCommandEvent& WXUNUSED(event) )
{
	TransferDataFromWindow();
    switch(m_Policy)
    {
    case 1:
		m_MatchRadios = 0;
		m_MatchAlnLocs = false;
		m_MatchByProduct = true;
        break;
    case 2:
		m_MatchRadios = 2;
		m_MatchAlnLocs = true;
		m_MatchByProduct = false;
        break;
    default:
		m_MatchRadios = 1;
		m_MatchAlnLocs = true;
		m_MatchByProduct = true;
        break;
    }
	TransferDataToWindow();
}

void CBroadcastDialog::OnSetDefaultsBtnClick( wxCommandEvent& event )
{
    wxArrayString wxprops;
    wxprops.Add(wxT("seq-id"));
    wxprops.Add(wxT("fullasm_id"));
    wxprops.Add(wxT("asm_accession"));
    wxprops.Add(wxT("tax-id"));
    wxprops.Add(wxT("taxid"));
    wxprops.Add(wxT("label"));

    m_BroadcastList->SetStrings(wxprops);
    m_SelectOneToOne->SetValue(true);
}


bool CBroadcastDialog::ShowToolTips()
{
    return true;
}

wxBitmap CBroadcastDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBroadcastDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBroadcastDialog bitmap retrieval
}
wxIcon CBroadcastDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBroadcastDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBroadcastDialog icon retrieval
}

END_NCBI_SCOPE
