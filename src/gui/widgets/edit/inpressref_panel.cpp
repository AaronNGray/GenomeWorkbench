/*  $Id: inpressref_panel.cpp 42780 2019-04-11 16:35:17Z asztalos $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/edit/inpressref_panel.hpp>

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*
 * CInPressRefPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CInPressRefPanel, wxPanel )


/*
 * CInPressRefPanel event table definition
 */

BEGIN_EVENT_TABLE( CInPressRefPanel, wxPanel )

////@begin CInPressRefPanel event table entries
    EVT_BUTTON( ID_PMIDLOOKBTN, CInPressRefPanel::OnLookupClick )
////@end CInPressRefPanel event table entries

END_EVENT_TABLE()


/*
 * CInPressRefPanel constructors
 */

CInPressRefPanel::CInPressRefPanel()
{
    Init();
}

CInPressRefPanel::CInPressRefPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CInPressRefPanel creator
 */

bool CInPressRefPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CInPressRefPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CInPressRefPanel creation
    return true;
}


/*
 * CInPressRefPanel destructor
 */

CInPressRefPanel::~CInPressRefPanel()
{
////@begin CInPressRefPanel destruction
////@end CInPressRefPanel destruction
}


/*
 * Member initialisation
 */

void CInPressRefPanel::Init()
{
////@begin CInPressRefPanel member initialisation
    m_PubMedID = NULL;
    m_ReferenceTitle = NULL;
    m_JournalTitle = NULL;
    m_YearCtrl = NULL;
    m_VolCtrl = NULL;
    m_IssueCtrl = NULL;
    m_PagesFromCtrl = NULL;
    m_PagesToCtrl = NULL;
////@end CInPressRefPanel member initialisation
}


/*
 * Control creation for CInPressRefPanel
 */

void CInPressRefPanel::CreateControls()
{    
////@begin CInPressRefPanel content construction
    CInPressRefPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer1, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText2 = new wxStaticText( itemPanel1, wxID_STATIC, _("PubMed ID"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemStaticText2, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_PubMedID = new wxTextCtrl( itemPanel1, ID_PMIDTEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer1->Add(m_PubMedID, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxButton* itemButton4 = new wxButton( itemPanel1, ID_PMIDLOOKBTN, _("Lookup"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemButton4, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxFlexGridSizer* itemFlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer1, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Reference title*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Journal title*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_ReferenceTitle = new wxTextCtrl( itemPanel1, ID_INPREFTITLE, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer1->Add(m_ReferenceTitle, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_JournalTitle = new wxTextCtrl( itemPanel1, ID_INPJOURNAL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer1->Add(m_JournalTitle, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(0, 5, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer2, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Year*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Volume"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Issue"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Pages from"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Pages to"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_YearCtrl = new wxTextCtrl( itemPanel1, ID_INPYEAR, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_YearCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_VolCtrl = new wxTextCtrl( itemPanel1, ID_INPVOL, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_VolCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IssueCtrl = new wxTextCtrl( itemPanel1, ID_INPISSUE, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_IssueCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PagesFromCtrl = new wxTextCtrl( itemPanel1, ID_INPPAGESFROM, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_PagesFromCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PagesToCtrl = new wxTextCtrl( itemPanel1, ID_INPPAGESTO, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_PagesToCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CInPressRefPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CInPressRefPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CInPressRefPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CInPressRefPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CInPressRefPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CInPressRefPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CInPressRefPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CInPressRefPanel icon retrieval
}

/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PMIDLOOKBTN
 */

void CInPressRefPanel::OnLookupClick( wxCommandEvent& event )
{

}

END_NCBI_SCOPE

