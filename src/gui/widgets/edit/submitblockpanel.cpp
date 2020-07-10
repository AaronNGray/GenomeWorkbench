/*  $Id: submitblockpanel.cpp 42280 2019-01-24 20:12:34Z asztalos $
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
#include <objects/submit/Submit_block.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_equiv.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/flexibledate_panel.hpp>
#include <gui/widgets/edit/submitblockpanel.hpp>
#include <wx/sizer.h>
#include <wx/choice.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CSubmitBlockPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubmitBlockPanel, wxPanel )


/*
 * CSubmitBlockPanel event table definition
 */

BEGIN_EVENT_TABLE( CSubmitBlockPanel, wxPanel )

////@begin CSubmitBlockPanel event table entries
    EVT_RADIOBUTTON( ID_RBTN_SUBPNL1, CSubmitBlockPanel::OnImmediateReleaseSelected )
    EVT_RADIOBUTTON( ID_RBTN_SUBPNL2, CSubmitBlockPanel::OnHUPSelected )
////@end CSubmitBlockPanel event table entries

END_EVENT_TABLE()


/*
 * CSubmitBlockPanel constructors
 */

CSubmitBlockPanel::CSubmitBlockPanel()
{
    Init();
}

CSubmitBlockPanel::CSubmitBlockPanel( wxWindow* parent, CRef<CSubmit_block> submit_block, bool create_submit_block, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_SubBlock(submit_block), m_CreateMode(create_submit_block)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSubmitBlockPanel creator
 */

bool CSubmitBlockPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubmitBlockPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubmitBlockPanel creation
    return true;
}


/*
 * CSubmitBlockPanel destructor
 */

CSubmitBlockPanel::~CSubmitBlockPanel()
{
////@begin CSubmitBlockPanel destruction
////@end CSubmitBlockPanel destruction
}


/*
 * Member initialisation
 */

void CSubmitBlockPanel::Init()
{
////@begin CSubmitBlockPanel member initialisation
    m_ImmediateRelease = NULL;
    m_HUP = NULL;
    m_Title = NULL;
    m_Reldate = NULL;
    m_Submitdate = NULL;
////@end CSubmitBlockPanel member initialisation
}


/*
 * Control creation for CSubmitBlockPanel
 */

void CSubmitBlockPanel::CreateControls()
{    
////@begin CSubmitBlockPanel content construction
    CSubmitBlockPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    if (m_CreateMode) {
        itemBoxSizer2->AddSpacer(10);
    }

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("When should this submission be released to the public?"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ImmediateRelease = new wxRadioButton( itemPanel1, ID_RBTN_SUBPNL1, _("Immediately after processing"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ImmediateRelease->SetValue(true);
    itemBoxSizer4->Add(m_ImmediateRelease, 0, wxALIGN_LEFT|wxALL, 5);

    m_HUP = new wxRadioButton( itemPanel1, ID_RBTN_SUBPNL2, _("Release on specified date:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_HUP->SetValue(false);
    itemBoxSizer4->Add(m_HUP, 0, wxALIGN_LEFT|wxALL, 5);

    CRef<CDate> r_date(&(m_SubBlock->SetReldate()));
    m_Reldate = new CFlexibleDatePanel(itemPanel1, r_date);
    itemBoxSizer2->Add(m_Reldate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("NOTE: Please inform GenBank when the accession number or any portion of the sequence is published, "
        "as published data must be released."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText8->Wrap(350);
    itemBoxSizer2->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Submission title (Recommended)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Title = new wxTextCtrl( itemPanel1, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(360, -1), wxTE_MULTILINE );
    itemBoxSizer2->Add(m_Title, 0, wxALIGN_CENTER_HORIZONTAL |wxALL, 5);

    if (!m_CreateMode) {
        wxStaticText* itemStaticText10 = new wxStaticText(itemPanel1, wxID_STATIC, _("Submission Date"), wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer2->Add(itemStaticText10, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 5);

        CCit_sub& citsub = m_SubBlock->SetCit();
        CRef<CDate> subdate(&(citsub.SetDate()));
        m_Submitdate = new CFlexibleDatePanel(itemPanel1, subdate);
        itemBoxSizer2->Add(m_Submitdate, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0);
    }
  
////@end CSubmitBlockPanel content construction
}


bool CSubmitBlockPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }
    
    if (m_SubBlock->IsSetHup() && m_SubBlock->GetHup()) {
        m_ImmediateRelease->SetValue (false);
        m_HUP->SetValue (true);  
        m_Reldate->Enable(true);
    } else {
        m_ImmediateRelease->SetValue (true);
        m_HUP->SetValue (false);
        m_Reldate->Enable(false);
    }

    if (m_SubBlock->IsSetCit() && m_SubBlock->GetCit().IsSetDescr()) {
        m_Title->SetValue(m_SubBlock->GetCit().GetDescr());
    } else {
        m_Title->SetValue(wxEmptyString);
    }
    if (m_Submitdate) {
        m_Submitdate->TransferDataToWindow();
    }
    return true;
}


bool CSubmitBlockPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }
    if (m_HUP->GetValue()) {
        m_SubBlock->SetHup(true);
    } else {
        m_SubBlock->SetHup(false);
        m_SubBlock->ResetReldate();
    }

    if (!m_Title->IsEmpty()) {
        m_SubBlock->SetCit().SetDescr(ToAsciiStdString(m_Title->GetValue()));
    }
    if (m_Submitdate) {
        m_Submitdate->TransferDataFromWindow();
    }
    return true;
}

void CSubmitBlockPanel::SetData(const CSubmit_block& submit_block)
{
    m_SubBlock->Assign(submit_block);
    TransferDataToWindow();
}

/*
 * Should we show tooltips?
 */

bool CSubmitBlockPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSubmitBlockPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubmitBlockPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubmitBlockPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSubmitBlockPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubmitBlockPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubmitBlockPanel icon retrieval
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON
 */

void CSubmitBlockPanel::OnImmediateReleaseSelected( wxCommandEvent& event )
{
    m_Reldate->Enable(false);
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON1
 */

void CSubmitBlockPanel::OnHUPSelected( wxCommandEvent& event )
{
    m_Reldate->Enable(true);
}

END_NCBI_SCOPE
