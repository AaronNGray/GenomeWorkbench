/*  $Id: aboutdlg.cpp 43515 2019-07-22 16:21:38Z katargir $
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

#include <gui/objects/gbench_version.hpp>
#include <gui/objects/GBenchVersionInfo.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/utils/app_popup.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objects/general/Date.hpp>

////@begin includes
////@end includes

#include "aboutdlg.hpp"

#include <gui/osx_api.h>

////@begin XPM images
////@end XPM images

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/settings.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CAboutDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAboutDlg, wxDialog )


/*!
 * CAboutDlg event table definition
 */

BEGIN_EVENT_TABLE( CAboutDlg, wxDialog )

////@begin CAboutDlg event table entries
    EVT_BUTTON( ID_BUTTON, CAboutDlg::OnOK )

////@end CAboutDlg event table entries

END_EVENT_TABLE()


/*!
 * CAboutDlg constructors
 */

CAboutDlg::CAboutDlg()
{
    Init();
}

CAboutDlg::CAboutDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CAboutDlg creator
 */

bool CAboutDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAboutDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAboutDlg creation
    return true;
}


/*!
 * CAboutDlg destructor
 */

CAboutDlg::~CAboutDlg()
{
////@begin CAboutDlg destruction
////@end CAboutDlg destruction
}


/*!
 * Member initialisation
 */

void CAboutDlg::Init()
{
////@begin CAboutDlg member initialisation
    m_Image = NULL;
    m_VersionString = NULL;
    m_BuildDate = NULL;
////@end CAboutDlg member initialisation
}


/*!
 * Control creation for CAboutDlg
 */

void CAboutDlg::CreateControls()
{
////@begin CAboutDlg content construction
    CAboutDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Image = new wxStaticBitmap( itemDialog1, wxID_STATIC, wxNullBitmap, wxDefaultPosition, wxSize(128, 128), wxSUNKEN_BORDER );
    itemBoxSizer3->Add(m_Image, 0, wxALIGN_CENTER_VERTICAL|wxALL, 10);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("NCBI Genome Workbench"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText6->SetFont(wxFont(int(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetPointSize()*1.2), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Sans")));
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_VersionString = new wxStaticText( itemDialog1, wxID_STATIC, _("Version String"), wxDefaultPosition, wxDefaultSize, 0 );
    m_VersionString->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL, false, wxT("Sans")));
    itemBoxSizer5->Add(m_VersionString, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_BuildDate = new wxStaticText( itemDialog1, wxID_STATIC, _("Build Date"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BuildDate->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL, false, wxT("Sans")));
    itemBoxSizer5->Add(m_BuildDate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl9 = new wxHyperlinkCtrl( itemDialog1, m_HYPERLINKCTRL, _("https://www.ncbi.nlm.nih.gov/projects/gbench/"), _T("https://www.ncbi.nlm.nih.gov/projects/gbench/"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer5->Add(itemHyperlinkCtrl9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticLine* itemStaticLine10 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine10, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemDialog1, ID_BUTTON, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton12->SetDefault();
    itemBoxSizer11->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CAboutDlg content construction

    CGBenchVersionInfo version;
    GetGBenchVersionInfo(version);
    string version_str = "version ";
    version_str += NStr::NumericToString(version.GetVer_major());
    version_str += ".";
    version_str += NStr::NumericToString(version.GetVer_minor());
    version_str += ".";
    version_str += NStr::NumericToString(version.GetVer_patch());

#ifdef __WXOSX_COCOA__
    #ifdef USE_METAL
    version_str += " (Metal)";
    #else
    version_str += " (OpenGL)";
    #endif
#endif

    m_VersionString->SetLabel(ToWxString(version_str));

    string build_date = "built on ";
    build_date += version.GetBuild_date().AsCTime().AsString("M/D/Y h:m");
    m_BuildDate->SetLabel(ToWxString(build_date));

    wxString splash_str = CSysPath::ResolvePathExisting(wxT("<res>/gbench_about.png"));
    if ( !splash_str.empty() ) {
        wxBitmap bmp(splash_str, wxBITMAP_TYPE_PNG);
        m_Image->SetBitmap(bmp);
    }
}


/*!
 * Should we show tooltips?
 */

bool CAboutDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAboutDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAboutDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAboutDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAboutDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAboutDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAboutDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void CAboutDlg::OnOK( wxCommandEvent& event )
{
    event.Skip();
    EndModal(wxID_OK);
}

END_NCBI_SCOPE
