/*  $Id: isojta_lookup_dlg.cpp 34090 2015-11-02 18:35:47Z filippov $
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

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/isojta_lookup_dlg.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h> 

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*
 * CISOJTALookupDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CISOJTALookupDlg, wxDialog )


/*
 * CISOJTALookupDlg event table definition
 */

BEGIN_EVENT_TABLE( CISOJTALookupDlg, wxDialog )

////@begin CISOJTALookupDlg event table entries
    EVT_BUTTON( wxID_OK, CISOJTALookupDlg::OnOkClick )

    EVT_BUTTON( wxID_CANCEL, CISOJTALookupDlg::OnCancelClick )

    EVT_BUTTON( ID_ISO_JTA_REPORT_BTN, CISOJTALookupDlg::OnIsoJtaReportBtnClick )

////@end CISOJTALookupDlg event table entries

END_EVENT_TABLE()


/*
 * CISOJTALookupDlg constructors
 */

CISOJTALookupDlg::CISOJTALookupDlg()
{
    Init();
}

CISOJTALookupDlg::CISOJTALookupDlg( wxWindow* parent, const vector<string> &journals, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_titles(journals)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CISOJTALookupDlg creator
 */

bool CISOJTALookupDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CISOJTALookupDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CISOJTALookupDlg creation
    return true;
}


/*
 * CISOJTALookupDlg destructor
 */

CISOJTALookupDlg::~CISOJTALookupDlg()
{
////@begin CISOJTALookupDlg destruction
////@end CISOJTALookupDlg destruction
}


/*
 * Member initialisation
 */

void CISOJTALookupDlg::Init()
{
////@begin CISOJTALookupDlg member initialisation
    m_JournalChoice = NULL;
////@end CISOJTALookupDlg member initialisation
}


/*
 * Control creation for CISOJTALookupDlg
 */

void CISOJTALookupDlg::CreateControls()
{    
////@begin CISOJTALookupDlg content construction
    CISOJTALookupDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Select the desired journal from the list:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Journal"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString journalChoiceStrings;
    for (size_t i = 0; i < m_titles.size(); i++)
        journalChoiceStrings.Add(wxString::FromUTF8(m_titles[i].c_str()));

    m_JournalChoice = new wxChoice( itemDialog1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, journalChoiceStrings, 0 );
    itemBoxSizer4->Add(m_JournalChoice, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemDialog1, ID_ISO_JTA_REPORT_BTN, _("Make Report"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CISOJTALookupDlg content construction
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CISOJTALookupDlg::OnOkClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CISOJTALookupDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in CISOJTALookupDlg. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void CISOJTALookupDlg::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CISOJTALookupDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in CISOJTALookupDlg. 
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ISO_JTA_REPORT_BTN
 */

void CISOJTALookupDlg::OnIsoJtaReportBtnClick( wxCommandEvent& event )
{
    wxString msg;
    for (size_t i = 0; i < m_titles.size(); i++)
        msg << i+1 << ". " << wxString::FromUTF8(m_titles[i].c_str())<< "\r\n";
    wxMessageBox(msg, _("Possible Journal Titles"), wxOK, this);
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ISO_JTA_REPORT_BTN in CISOJTALookupDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ISO_JTA_REPORT_BTN in CISOJTALookupDlg. 
}


/*
 * Should we show tooltips?
 */

bool CISOJTALookupDlg::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CISOJTALookupDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CISOJTALookupDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CISOJTALookupDlg bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CISOJTALookupDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CISOJTALookupDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CISOJTALookupDlg icon retrieval
}


void CISOJTALookupDlg::SetJournalList(const vector<string> &journals)
{
    m_titles = journals;
    m_JournalChoice->Clear();
    ITERATE(vector<string>, it, journals) {
        m_JournalChoice->Append(wxString::FromUTF8(it->c_str()));
    }
    Refresh();
}


string CISOJTALookupDlg::GetISOJTA()
{
    return ToStdString(m_JournalChoice->GetStringSelection());
}


END_NCBI_SCOPE
