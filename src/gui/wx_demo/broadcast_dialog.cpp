/*  $Id: broadcast_dialog.cpp 25633 2012-04-13 20:51:47Z katargir $
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
 * Authors:  Roman Katargin
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

////@begin includes
////@end includes

////@begin XPM images
////@end XPM images

#include <gui/objutils/obj_event.hpp>
#include <gui/wx_demo/broadcastdialog.hpp>


USING_NCBI_SCOPE;

/*!
 * CBroadcastDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CBroadcastDialog, wxDialog )


/*!
 * CBroadcastDialog event table definition
 */

BEGIN_EVENT_TABLE( CBroadcastDialog, wxDialog )

////@begin CBroadcastDialog event table entries
    EVT_BUTTON( wxID_APPLY, CBroadcastDialog::OnApplyClick )

////@end CBroadcastDialog event table entries

END_EVENT_TABLE()


/*!
 * CBroadcastDialog constructors
 */

CBroadcastDialog::CBroadcastDialog()
{
    SBroadcastDlgParams params;
    Init(params);
}

CBroadcastDialog::CBroadcastDialog( const SBroadcastDlgParams& params,
                                   wxWindow* parent, wxWindowID id,
                                   const wxString& caption,
                                   const wxPoint& pos, const wxSize& size, long style )
{
    Init(params);
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CBroadcastDialog creator
 */

bool CBroadcastDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBroadcastDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
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


/*!
 * CBroadcastDialog destructor
 */

CBroadcastDialog::~CBroadcastDialog()
{
////@begin CBroadcastDialog destruction
////@end CBroadcastDialog destruction
}


/*!
 * Member initialisation
 */

void CBroadcastDialog::Init( const SBroadcastDlgParams& params )
{
////@begin CBroadcastDialog member initialisation
////@end CBroadcastDialog member initialisation

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
    m_VRPolicy = params.m_VRPolicy;
}

SBroadcastDlgParams CBroadcastDialog::GetData() const
{
    SBroadcastDlgParams params;

    params.m_BioseqMatching = (m_MatchRadios == 0) ?
            CSelectionEvent::eAllIds : CSelectionEvent::eAtLeastOne;
    params.m_SeqIDMatching = (m_MatchRadios == 2) ?
            CSelectionEvent::eAccOnly : CSelectionEvent::eExact;

    params.m_MatchByProduct = m_MatchByProduct;
    params.m_MatchAlnLocs = m_MatchAlnLocs;
    params.m_BroadcastBetweenProjects = m_BroadcastBetweenProjects;
    params.m_AutoSelBroadcast = m_AutoSelBroadcast;
    params.m_AutoVRBroadcast = m_AutoVRBroadcast;
    params.m_VRPolicy = m_VRPolicy;

    return params;
}

/*!
 * Control creation for CBroadcastDialog
 */

void CBroadcastDialog::CreateControls()
{
////@begin CBroadcastDialog content construction
    CBroadcastDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 7);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Policy"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxHORIZONTAL);
    itemBoxSizer4->Add(itemStaticBoxSizer5, 0, wxGROW|wxALL, 0);

    wxArrayString itemChoice6Strings;
    itemChoice6Strings.Add(_("Default"));
    itemChoice6Strings.Add(_("Strict"));
    itemChoice6Strings.Add(_("Relaxed"));
    wxChoice* itemChoice6 = new wxChoice( itemDialog1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, itemChoice6Strings, 0 );
    itemChoice6->SetStringSelection(_("Default"));
    itemStaticBoxSizer5->Add(itemChoice6, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_APPLY, _("&Apply"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemRadioBox8Strings;
    itemRadioBox8Strings.Add(_("Exact Match by all IDs"));
    itemRadioBox8Strings.Add(_("Match at least one ID"));
    itemRadioBox8Strings.Add(_("Match at least one ID, ignore version"));
    wxRadioBox* itemRadioBox8 = new wxRadioBox( itemDialog1, ID_RADIOBOX1, _("Sequence Matching Selection Options"), wxDefaultPosition, wxDefaultSize, itemRadioBox8Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox8->SetSelection(0);
    itemBoxSizer4->Add(itemRadioBox8, 0, wxGROW|wxALL, 0);

    wxStaticBox* itemStaticBoxSizer9Static = new wxStaticBox(itemDialog1, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer9 = new wxStaticBoxSizer(itemStaticBoxSizer9Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer9, 0, wxGROW|wxALL, 0);

    wxCheckBox* itemCheckBox10 = new wxCheckBox( itemDialog1, ID_CHECKBOX1, _("Match Features by Products"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox10->SetValue(false);
    itemStaticBoxSizer9->Add(itemCheckBox10, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox11 = new wxCheckBox( itemDialog1, ID_CHECKBOX2, _("Use locations to identify alignment rows"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox11->SetValue(false);
    itemStaticBoxSizer9->Add(itemCheckBox11, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox12 = new wxCheckBox( itemDialog1, ID_CHECKBOX3, _("Auto broadcast selection changes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox12->SetValue(false);
    itemBoxSizer4->Add(itemCheckBox12, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer13, 0, wxGROW|wxALL, 5);

    wxArrayString itemRadioBox14Strings;
    itemRadioBox14Strings.Add(_("Track range changes"));
    itemRadioBox14Strings.Add(_("Track and Center range changes"));
    itemRadioBox14Strings.Add(_("Track and Expand range changes"));
    itemRadioBox14Strings.Add(_("Track and Contract range changes"));
    itemRadioBox14Strings.Add(_("Slave ranges"));
    wxRadioBox* itemRadioBox14 = new wxRadioBox( itemDialog1, ID_RADIOBOX2, _("Visible Range Synchronization Options"), wxDefaultPosition, wxDefaultSize, itemRadioBox14Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox14->SetSelection(0);
    itemBoxSizer13->Add(itemRadioBox14, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    itemBoxSizer13->Add(5, 5, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxCheckBox* itemCheckBox16 = new wxCheckBox( itemDialog1, ID_CHECKBOX5, _("Auto broadcast range changes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox16->SetValue(false);
    itemBoxSizer13->Add(itemCheckBox16, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxCheckBox* itemCheckBox18 = new wxCheckBox( itemDialog1, ID_CHECKBOX4, _("Broadcast between Projects"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox18->SetValue(false);
    itemBoxSizer17->Add(itemCheckBox18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine19 = new wxStaticLine( itemDialog1, ID_STATICLINE2, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine19, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer20, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton21 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton21->SetDefault();
    itemBoxSizer20->Add(itemButton21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton22 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer20->Add(itemButton22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemRadioBox8->SetValidator( wxGenericValidator(& m_MatchRadios) );
    itemCheckBox10->SetValidator( wxGenericValidator(& m_MatchByProduct) );
    itemCheckBox11->SetValidator( wxGenericValidator(& m_MatchAlnLocs) );
    itemCheckBox12->SetValidator( wxGenericValidator(& m_AutoSelBroadcast) );
    itemRadioBox14->SetValidator( wxGenericValidator(& m_VRPolicy) );
    itemCheckBox16->SetValidator( wxGenericValidator(& m_AutoVRBroadcast) );
    itemCheckBox18->SetValidator( wxGenericValidator(& m_BroadcastBetweenProjects) );
////@end CBroadcastDialog content construction
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY
 */

void CBroadcastDialog::OnApplyClick( wxCommandEvent& event )
{
    wxChoice* policyCtrl = (wxChoice*)FindWindow(ID_CHOICE1);
    wxRadioBox* selMatchCtrl = (wxRadioBox*)FindWindow(ID_RADIOBOX1);
    wxCheckBox* matchByProduct = (wxCheckBox*)FindWindow(ID_CHECKBOX1);
    wxCheckBox* matchAlnLocs = (wxCheckBox*)FindWindow(ID_CHECKBOX2);

    switch(policyCtrl->GetSelection())
    {
    case 1:
        selMatchCtrl->SetSelection(0);
        matchAlnLocs->SetValue(false);
        matchByProduct->SetValue(true);
        break;
    case 2:
        selMatchCtrl->SetSelection(2);
        matchAlnLocs->SetValue(true);
        matchByProduct->SetValue(false);
        break;
    default:
        selMatchCtrl->SetSelection(1);
        matchAlnLocs->SetValue(true);
        matchByProduct->SetValue(true);
        break;
    }
}


/*!
 * Should we show tooltips?
 */

bool CBroadcastDialog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBroadcastDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBroadcastDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBroadcastDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBroadcastDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBroadcastDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBroadcastDialog icon retrieval
}
