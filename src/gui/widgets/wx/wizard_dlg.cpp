/*  $Id: wizard_dlg.cpp 43611 2019-08-08 18:36:29Z filippov $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/widgets/wx/wizard_dlg.hpp>

#include <wx/panel.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/hyperlink.h>
BEGIN_NCBI_SCOPE

DEFINE_EVENT_TYPE(CEVT_WIZARD_DLG_EVENT)

IMPLEMENT_DYNAMIC_CLASS( CWizardDlg, CDialog )

BEGIN_EVENT_TABLE( CWizardDlg, CDialog )

////@begin CWizardDlg event table entries
    EVT_BUTTON( ID_BUTTON_OPTIONS, CWizardDlg::OnButtonOptionsClick )

    EVT_BUTTON( wxID_BACKWARD, CWizardDlg::OnBackwardClick )

    EVT_BUTTON( wxID_FORWARD, CWizardDlg::OnForwardClick )

////@end CWizardDlg event table entries

    EVT_MENU(eCmdUpdateButtons, CWizardDlg::OnUpdateButtons)

END_EVENT_TABLE()

CWizardDlg::CWizardDlg()
 : m_CurrentPage()
{
    Init();
}

CWizardDlg::CWizardDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
 : m_CurrentPage()
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CWizardDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CWizardDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CWizardDlg creation
    return true;
}

CWizardDlg::~CWizardDlg()
{
////@begin CWizardDlg destruction
////@end CWizardDlg destruction
}

void CWizardDlg::Init()
{
////@begin CWizardDlg member initialisation
    m_ButtonsSizer = NULL;
    m_OptionsBtn = NULL;
    m_BackBtn = NULL;
    m_NextBtn = NULL;
    m_Help = NULL;
////@end CWizardDlg member initialisation
}

void CWizardDlg::CreateControls()
{    
////@begin CWizardDlg content construction
    CWizardDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxStaticLine* itemStaticLine3 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine3, 0, wxGROW|wxALL, 5);

    m_ButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_ButtonsSizer, 0, wxGROW|wxALL, 5);

    m_Help = new wxHyperlinkCtrl( itemCDialog1, ID_HELP_LINK, _("Help"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_Help->SetForegroundColour(wxColour(192, 192, 192));
    m_ButtonsSizer->Add(m_Help, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_Help->Hide();

    m_ButtonsSizer->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OptionsBtn = new wxButton( itemCDialog1, ID_BUTTON_OPTIONS, _("Options"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OptionsBtn->Show(false);
    m_ButtonsSizer->Add(m_OptionsBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_BackBtn = new wxButton( itemCDialog1, wxID_BACKWARD, _("&Back"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ButtonsSizer->Add(m_BackBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_NextBtn = new wxButton( itemCDialog1, wxID_FORWARD, _("&Forward"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NextBtn->SetDefault();
    m_ButtonsSizer->Add(m_NextBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_ButtonsSizer->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxButton* itemButton11 = new wxButton( itemCDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ButtonsSizer->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

////@end CWizardDlg content construction


    CreatePages();
    UpdateButtons();
}

void CWizardDlg::SetHelpUrl(const wxString &url)
{
    m_Help->SetURL(url);
    m_Help->Show(!url.IsEmpty());
}


void CWizardDlg::SetCurrentPage(IWizardPage* page)
{
    if (m_CurrentPage == page)
        return;

    wxPanel* panel = m_CurrentPage ? m_CurrentPage->GetPanel() : 0;
    if (panel) {
        panel->Hide();
    }

    m_CurrentPage = page;
    if (!m_CurrentPage)
        return;

    panel = m_CurrentPage->GetPanel();

    if (panel->GetParent() != this) {
        panel->Reparent(this);
    }

    if (panel->GetContainingSizer() != GetSizer()) {
        GetSizer()->Insert(0, panel, 1, wxEXPAND | wxALL, 5);
    }

    panel->Show();
    panel->SetFocus();

    UpdateButtons();

    this->Layout();
}

void CWizardDlg::UpdateButtons()
{
    IWizardPage *next = 0, *prev = 0, *options = 0;
    if (m_CurrentPage) {
        next = m_CurrentPage->GetNextPage();
        prev = m_CurrentPage->GetPrevPage();
        options = m_CurrentPage->GetOptionsPage();
    }

    m_OptionsBtn->Show(options != 0);
    m_BackBtn->Show(prev != 0);
    m_NextBtn->SetLabel(next ? wxT("Next >") : wxT("Finish"));
	m_ButtonsSizer->Layout();
}

bool CWizardDlg::ShowToolTips()
{
    return true;
}
wxBitmap CWizardDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWizardDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CWizardDlg bitmap retrieval
}
wxIcon CWizardDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWizardDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CWizardDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BACKWARD
 */

void CWizardDlg::OnBackwardClick( wxCommandEvent& WXUNUSED(event) )
{
    if (m_CurrentPage) {
        IWizardPage *prev = m_CurrentPage->GetPrevPage();
        if (prev && m_CurrentPage->CanLeavePage(false))
            SetCurrentPage(prev);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FORWARD
 */

void CWizardDlg::OnForwardClick( wxCommandEvent& WXUNUSED(event) )
{
    if (m_CurrentPage) {
        IWizardPage *next = m_CurrentPage->GetNextPage();
        if (m_CurrentPage->CanLeavePage(true)) {
            if (next)
                SetCurrentPage(next);
            else
                EndModal(wxID_OK);
        }
    }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OPTIONS
 */

void CWizardDlg::OnButtonOptionsClick( wxCommandEvent& WXUNUSED(event) )
{
    if (m_CurrentPage) {
        IWizardPage *next = m_CurrentPage->GetOptionsPage();
        if (next != 0 && m_CurrentPage->CanLeavePage(true)) {
            SetCurrentPage(next);
        }
    }
}

END_NCBI_SCOPE
