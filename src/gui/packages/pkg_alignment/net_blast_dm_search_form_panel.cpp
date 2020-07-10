/*  $Id: net_blast_dm_search_form_panel.cpp 27848 2013-04-12 18:51:49Z kuznets $
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
 * Authors: Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/packages/pkg_alignment/net_blast_dm_search_form_panel.hpp>

#include <gui/core/dm_search_tool.hpp>

////@begin XPM images
////@end XPM images


#include <wx/sizer.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>


BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CNetBlastDMSearchFormPanel, wxPanel )

BEGIN_EVENT_TABLE( CNetBlastDMSearchFormPanel, wxPanel )
////@begin CNetBlastDMSearchFormPanel event table entries
    EVT_TEXT( ID_FILTER_INPUT, CNetBlastDMSearchFormPanel::OnFilterInputTextUpdated )

    EVT_BUTTON( wxID_RESET, CNetBlastDMSearchFormPanel::OnResetClick )

    EVT_CHECKBOX( ID_SUBMITTED_CHECK, CNetBlastDMSearchFormPanel::OnSubmittedCheckClick )

////@end CNetBlastDMSearchFormPanel event table entries
    EVT_CHECKBOX( ID_COMPLETED_CHECK, CNetBlastDMSearchFormPanel::OnSubmittedCheckClick )
    EVT_CHECKBOX( ID_RETRIEVED_CHECK, CNetBlastDMSearchFormPanel::OnSubmittedCheckClick )
    EVT_CHECKBOX( ID_FAILED_CHECK, CNetBlastDMSearchFormPanel::OnSubmittedCheckClick )
    EVT_CHECKBOX( ID_EXPIRED_CHECK, CNetBlastDMSearchFormPanel::OnSubmittedCheckClick )
END_EVENT_TABLE()


CNetBlastDMSearchFormPanel::CNetBlastDMSearchFormPanel()
{
    Init();
}


CNetBlastDMSearchFormPanel::CNetBlastDMSearchFormPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CNetBlastDMSearchFormPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CNetBlastDMSearchFormPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    Centre();
////@end CNetBlastDMSearchFormPanel creation
    return true;
}


CNetBlastDMSearchFormPanel::~CNetBlastDMSearchFormPanel()
{
////@begin CNetBlastDMSearchFormPanel destruction
////@end CNetBlastDMSearchFormPanel destruction
}


void CNetBlastDMSearchFormPanel::Init()
{
////@begin CNetBlastDMSearchFormPanel member initialisation
    m_Completed = true;
    m_Submitted = true;
    m_Failed = true;
    m_Retrieved = true;
    m_Expired = true;
    m_SearchCtrl = NULL;
    m_ResetBtn = NULL;
////@end CNetBlastDMSearchFormPanel member initialisation
}


void CNetBlastDMSearchFormPanel::CreateControls()
{
////@begin CNetBlastDMSearchFormPanel content construction
    CNetBlastDMSearchFormPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, wxT("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SearchCtrl = new wxTextCtrl( itemPanel1, ID_FILTER_INPUT, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_SearchCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ResetBtn = new wxButton( itemPanel1, wxID_RESET, wxT("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_ResetBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxGROW, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, wxT("Show Jobs:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox9 = new wxCheckBox( itemPanel1, ID_SUBMITTED_CHECK, wxT("Submitted"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox9->SetValue(false);
    itemBoxSizer7->Add(itemCheckBox9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox10 = new wxCheckBox( itemPanel1, ID_COMPLETED_CHECK, wxT("Completed"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox10->SetValue(false);
    itemBoxSizer7->Add(itemCheckBox10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox11 = new wxCheckBox( itemPanel1, ID_FAILED_CHECK, wxT("Failed"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox11->SetValue(false);
    itemBoxSizer7->Add(itemCheckBox11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox12 = new wxCheckBox( itemPanel1, ID_RETRIEVED_CHECK, wxT("Retrieved"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox12->SetValue(false);
    itemBoxSizer7->Add(itemCheckBox12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox13 = new wxCheckBox( itemPanel1, ID_EXPIRED_CHECK, wxT("Expired / Invalid"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox13->SetValue(false);
    itemBoxSizer7->Add(itemCheckBox13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_SearchCtrl->SetValidator( wxGenericValidator(& m_SearchValue) );
    itemCheckBox9->SetValidator( wxGenericValidator(& m_Submitted) );
    itemCheckBox10->SetValidator( wxGenericValidator(& m_Completed) );
    itemCheckBox11->SetValidator( wxGenericValidator(& m_Failed) );
    itemCheckBox12->SetValidator( wxGenericValidator(& m_Retrieved) );
    itemCheckBox13->SetValidator( wxGenericValidator(& m_Expired) );
////@end CNetBlastDMSearchFormPanel content construction
}


wxBitmap CNetBlastDMSearchFormPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CNetBlastDMSearchFormPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CNetBlastDMSearchFormPanel bitmap retrieval
}


wxIcon CNetBlastDMSearchFormPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CNetBlastDMSearchFormPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CNetBlastDMSearchFormPanel icon retrieval
}


void CNetBlastDMSearchFormPanel::OnFilterInputTextUpdated(wxCommandEvent& event)
{
    if( ! m_BlockEvents)    {
        m_SearchValue = m_SearchCtrl->GetValue();
        RestartSearch();
    }
}


void CNetBlastDMSearchFormPanel::OnResetClick( wxCommandEvent& event )
{
    m_SearchCtrl->SetValue(wxT(""));
}


bool CNetBlastDMSearchFormPanel::ShowToolTips()
{
    return true;
}


void CNetBlastDMSearchFormPanel::OnSubmittedCheckClick( wxCommandEvent& event )
{
    RestartSearch();
}


void CNetBlastDMSearchFormPanel::RestartSearch()
{
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, IDMSearchForm::eCmdDMSearch);
    evt.SetEventObject(this);
    GetEventHandler()->ProcessEvent(evt);
}


bool CNetBlastDMSearchFormPanel::TransferDataToWindow()
{
    m_BlockEvents = true;
    bool res = wxPanel::TransferDataToWindow();
    m_BlockEvents = false;
    return res;
}

END_NCBI_SCOPE
