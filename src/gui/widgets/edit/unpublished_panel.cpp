/*  $Id: unpublished_panel.cpp 37431 2017-01-10 16:55:15Z filippov $
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
#include <objects/biblio/Cit_gen.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
////@begin includes
////@end includes
////@begin includes
////@end includes

#include <gui/widgets/edit/unpublished_panel.hpp>

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/utils.h> 
#include <wx/uri.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CUnpublishedPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CUnpublishedPanel, wxPanel )


/*
 * CUnpublishedPanel event table definition
 */

BEGIN_EVENT_TABLE( CUnpublishedPanel, wxPanel )

////@begin CUnpublishedPanel event table entries
EVT_BUTTON( ID_UNPUB_TITLE_SEARCH, CUnpublishedPanel::OnSearchTitle )
////@end CUnpublishedPanel event table entries

END_EVENT_TABLE()


/*
 * CUnpublishedPanel constructors
 */

CUnpublishedPanel::CUnpublishedPanel()
{
    Init();
}

CUnpublishedPanel::CUnpublishedPanel( wxWindow* parent, const string &title,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
    m_title(title)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CUnpublishedPanel creator
 */

bool CUnpublishedPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CUnpublishedPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CUnpublishedPanel creation
    return true;
}


/*
 * CUnpublishedPanel destructor
 */

CUnpublishedPanel::~CUnpublishedPanel()
{
////@begin CUnpublishedPanel destruction
////@end CUnpublishedPanel destruction
}


/*
 * Member initialisation
 */

void CUnpublishedPanel::Init()
{
////@begin CUnpublishedPanel member initialisation
    m_TextCtrl = NULL;
////@end CUnpublishedPanel member initialisation
}


/*
 * Control creation for CUnpublishedPanel
 */

void CUnpublishedPanel::CreateControls()
{    
////@begin CUnpublishedPanel content construction
    CUnpublishedPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Tentative title for manuscript"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_TextCtrl = new CRichTextCtrl(itemPanel1, ID_UNPUB_TITLE, wxString(m_title), wxDefaultPosition, wxSize(400, 85), wxTE_MULTILINE);
    itemBoxSizer2->Add(m_TextCtrl, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    wxButton* itemButton1 = new wxButton( itemPanel1, ID_UNPUB_TITLE_SEARCH, _("Search Title on the Web"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CUnpublishedPanel content construction

}


/*
 * Should we show tooltips?
 */

bool CUnpublishedPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CUnpublishedPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CUnpublishedPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CUnpublishedPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CUnpublishedPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CUnpublishedPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CUnpublishedPanel icon retrieval
}

void CUnpublishedPanel::OnSearchTitle( wxCommandEvent& event )
{
    wxString title = m_TextCtrl->GetValue();
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


void CUnpublishedPanel::SetTitle(const string &title)
{
    m_TextCtrl->SetValue(wxString(title));
}

string CUnpublishedPanel::GetTitle(void)
{
    return ToAsciiStdString(m_TextCtrl->GetValue());
}

END_NCBI_SCOPE
