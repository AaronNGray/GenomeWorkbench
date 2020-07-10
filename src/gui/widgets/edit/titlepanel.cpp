/*  $Id: titlepanel.cpp 37383 2017-01-03 21:02:29Z filippov $
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
#include <objects/biblio/Title.hpp>
#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/button.h>
#include <wx/utils.h> 
#include <wx/uri.h>


////@begin includes
////@end includes

#include "titlepanel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CTitlePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CTitlePanel, wxPanel )


/*
 * CTitlePanel event table definition
 */

BEGIN_EVENT_TABLE( CTitlePanel, wxPanel )

////@begin CTitlePanel event table entries
EVT_BUTTON( ID_PUB_TITLE_SEARCH, CTitlePanel::OnSearchTitle )
////@end CTitlePanel event table entries

END_EVENT_TABLE()


/*
 * CTitlePanel constructors
 */

CTitlePanel::CTitlePanel()
{
    Init();
}

CTitlePanel::CTitlePanel( wxWindow* parent, CRef<objects::CTitle::C_E> title, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Title(title)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * TitlePanel creator
 */

bool CTitlePanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTitlePanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTitlePanel creation
    return true;
}


/*
 * CTitlePanel destructor
 */

CTitlePanel::~CTitlePanel()
{
////@begin CTitlePanel destruction
////@end CTitlePanel destruction
}


/*
 * Member initialisation
 */

void CTitlePanel::Init()
{
////@begin CTitlePanel member initialisation
    m_TitleCtrl = NULL;
////@end CTitlePanel member initialisation
}


/*
 * Control creation for TitlePanel
 */

void CTitlePanel::CreateControls()
{    
////@begin CTitlePanel content construction
    CTitlePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_TitleCtrl = new CRichTextCtrl( itemPanel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(400, 85), wxTE_MULTILINE );
    itemBoxSizer2->Add(m_TitleCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton1 = new wxButton( itemPanel1, ID_PUB_TITLE_SEARCH, _("Search Title on the Web"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);


////@end CTitlePanel content construction


}


bool CTitlePanel::TransferDataToWindow()
{
    switch (m_Title->Which()) {
        case objects::CTitle::C_E::e_Abr:
            m_TitleCtrl->SetValue(ToWxString(m_Title->GetAbr()));
            break;
        case objects::CTitle::C_E::e_Coden:
            m_TitleCtrl->SetValue(ToWxString(m_Title->GetCoden()));
            break;
        case objects::CTitle::C_E::e_Isbn:
            m_TitleCtrl->SetValue(ToWxString(m_Title->GetIsbn()));
            break;
        case objects::CTitle::C_E::e_Iso_jta:
            m_TitleCtrl->SetValue(ToWxString(m_Title->GetIso_jta()));
            break;
        case objects::CTitle::C_E::e_Issn:
            m_TitleCtrl->SetValue(ToWxString(m_Title->GetIssn()));
            break;
        case objects::CTitle::C_E::e_Jta:
            m_TitleCtrl->SetValue(ToWxString(m_Title->GetJta()));
            break;
        case objects::CTitle::C_E::e_Ml_jta:
            m_TitleCtrl->SetValue(ToWxString(m_Title->GetMl_jta()));
            break;
        case objects::CTitle::C_E::e_Name:
            m_TitleCtrl->SetValue(ToWxString(m_Title->GetName()));
            break;
        case objects::CTitle::C_E::e_Trans:
            m_TitleCtrl->SetValue(ToWxString(m_Title->GetTrans()));
            break;
        case objects::CTitle::C_E::e_Tsub:
            m_TitleCtrl->SetValue(ToWxString(m_Title->GetTsub()));
            break;
        default:
            m_TitleCtrl->SetValue(wxT(""));
            break;
    }
    return wxPanel::TransferDataToWindow();
}


bool CTitlePanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    string title = ToStdString (m_TitleCtrl->GetValue ());
    NStr::ReplaceInPlace(title, " \n", " ");
    NStr::ReplaceInPlace(title, "\n ", " ");
    NStr::ReplaceInPlace(title, "\n", " ");

    switch (m_Title->Which()) {
        case objects::CTitle::C_E::e_Abr:
            m_Title->SetAbr(title);
            break;
        case objects::CTitle::C_E::e_Coden:
            m_Title->SetCoden(title);
            break;
        case objects::CTitle::C_E::e_Isbn:
            m_Title->SetIsbn(title);
            break;
        case objects::CTitle::C_E::e_Iso_jta:
            m_Title->SetIso_jta(title);
            break;
        case objects::CTitle::C_E::e_Issn:
            m_Title->SetIssn(title);
            break;
        case objects::CTitle::C_E::e_Jta:
            m_Title->SetJta(title);
            break;
        case objects::CTitle::C_E::e_Ml_jta:
            m_Title->SetMl_jta(title);
            break;
        case objects::CTitle::C_E::e_Name:
            m_Title->SetName(title);
            break;
        case objects::CTitle::C_E::e_Trans:
            m_Title->SetTrans(title);
            break;
        case objects::CTitle::C_E::e_Tsub:
            m_Title->SetTsub(title);
            break;
        default:
            if (!NStr::IsBlank(title)) {
                m_Title->SetName(title);
            }
            break;
    }
    return true;
}


/*
 * Should we show tooltips?
 */

bool CTitlePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CTitlePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTitlePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTitlePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CTitlePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTitlePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTitlePanel icon retrieval
}

void CTitlePanel::OnSearchTitle( wxCommandEvent& event )
{
    wxString title = m_TitleCtrl->GetValue();
    if (title.IsEmpty())
        return;

    wxURI google_url (wxT("http://www.google.com/search?q=") + title);
    wxString google_encoded_url = google_url.BuildURI();
    wxLaunchDefaultBrowser(google_encoded_url, 0); // wxBROWSER_NEW_WINDOW
    wxSleep(3);
    wxURI pubmed_url (wxT("https://www.ncbi.nlm.nih.gov/pubmed/?term=") + title);
    wxString pubmed_encoded_url = pubmed_url.BuildURI();
    wxLaunchDefaultBrowser(pubmed_encoded_url, 0); // wxBROWSER_NEW_WINDOW
}

END_NCBI_SCOPE
