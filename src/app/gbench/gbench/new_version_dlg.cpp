/*  $Id: new_version_dlg.cpp 36065 2016-08-03 16:12:46Z katargir $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include "new_version_dlg.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/statbmp.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/artprov.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE

const char* kRNotesURL = "https://www.ncbi.nlm.nih.gov/tools/gbench/releasenotes/";

/*!
 * CNewVersionDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CNewVersionDlg, wxDialog )


/*!
 * CNewVersionDlg event table definition
 */

BEGIN_EVENT_TABLE( CNewVersionDlg, wxDialog )

////@begin CNewVersionDlg event table entries
    EVT_BUTTON( wxID_APPLY, CNewVersionDlg::OnApplyClick )

    EVT_BUTTON( ID_POSTPONE_BTN, CNewVersionDlg::OnPostponeBtnClick )

    EVT_BUTTON( wxID_IGNORE, CNewVersionDlg::OnIgnoreClick )

////@end CNewVersionDlg event table entries

END_EVENT_TABLE()


/*!
 * CNewVersionDlg constructors
 */

CNewVersionDlg::CNewVersionDlg() :
    m_InstallationEnabled(false)
{
    Init();
}

CNewVersionDlg::CNewVersionDlg(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) :
    m_InstallationEnabled(false)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CNewVersionDlg creator
 */

bool CNewVersionDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CNewVersionDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CNewVersionDlg creation
    return true;
}


/*!
 * CNewVersionDlg destructor
 */

CNewVersionDlg::~CNewVersionDlg()
{
////@begin CNewVersionDlg destruction
////@end CNewVersionDlg destruction
}


/*!
 * Member initialisation
 */

void CNewVersionDlg::Init()
{
////@begin CNewVersionDlg member initialisation
    m_DownloadURL = wxT("https://www.ncbi.nlm.nih.gov/projects/gbench");
    m_InstallationEnabled = false;
    m_Image = NULL;
    m_Hyperlink = NULL;
    m_RNotesHyperlink = NULL;
    m_InstallBtn = NULL;
////@end CNewVersionDlg member initialisation
}


/*!
 * Control creation for CNewVersionDlg
 */

void CNewVersionDlg::CreateControls()
{    
////@begin CNewVersionDlg content construction
    CNewVersionDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Image = new wxStaticBitmap( itemDialog1, wxID_STATIC, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_Image, 0, wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("A new version of Genome Workbench is available."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer5->Add(itemBoxSizer7, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("You are running version"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("2.3.4"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer5->Add(itemBoxSizer10, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("The latest version is"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemDialog1, wxID_STATIC, _("2.4.0"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _("The new version can be downloaded from:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText13, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    m_Hyperlink = new wxHyperlinkCtrl( itemDialog1, ID_HYPERLINKCTRL, _("https://www.ncbi.nlm.nih.gov/projects/gbench"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer5->Add(m_Hyperlink, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _("Release notes:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText15, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    m_RNotesHyperlink = new wxHyperlinkCtrl( itemDialog1, ID_RNOTES_HYPERLINKCTRL, _("https://www.ncbi.nlm.nih.gov/tools/gbench/releasenotes/"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer5->Add(m_RNotesHyperlink, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxStaticLine* itemStaticLine17 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine17, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer18, 0, wxALIGN_RIGHT|wxALL, 0);

    m_InstallBtn = new wxButton( itemDialog1, wxID_APPLY, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(m_InstallBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer18->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton21 = new wxButton( itemDialog1, ID_POSTPONE_BTN, _("Remind me later"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton22 = new wxButton( itemDialog1, wxID_IGNORE, _("Ignore"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemStaticText9->SetValidator( wxGenericValidator(& m_CurrentVersion) );
    itemStaticText12->SetValidator( wxGenericValidator(& m_LatestVersion) );
////@end CNewVersionDlg content construction

    m_Hyperlink->SetURL(m_DownloadURL);
    m_Hyperlink->SetLabel(m_DownloadURL);

    wxBitmap bmp = wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_CMN_DIALOG);
    m_Image->SetBitmap(bmp);

    m_InstallBtn->Enable(m_InstallationEnabled);

    wxString rnotesURL(kRNotesURL);
    rnotesURL += "#V";
    rnotesURL += m_LatestVersion;
    m_RNotesHyperlink->SetURL(rnotesURL);
}


/*!
 * Should we show tooltips?
 */

bool CNewVersionDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CNewVersionDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CNewVersionDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CNewVersionDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CNewVersionDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CNewVersionDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CNewVersionDlg icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_POSTPONE_BTN
 */

void CNewVersionDlg::OnPostponeBtnClick( wxCommandEvent& event )
{
    EndModal(ID_POSTPONE_BTN);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY
 */

void CNewVersionDlg::OnApplyClick( wxCommandEvent& event )
{
    EndModal(wxID_APPLY);
}

void CNewVersionDlg::OnIgnoreClick( wxCommandEvent& event )
{
    if (wxYES == wxMessageBox(wxT("If you ignore this update, you won't be notified about it again.\nAre you sure?"), wxT("Confirm"), wxICON_INFORMATION | wxYES_NO))
        EndModal(wxID_IGNORE);
    else
        EndModal(ID_POSTPONE_BTN);
}

END_NCBI_SCOPE
