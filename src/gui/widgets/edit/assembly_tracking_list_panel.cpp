/*  $Id: assembly_tracking_list_panel.cpp 40132 2017-12-22 15:45:12Z bollin $
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

#include <wx/stattext.h>
#include <wx/icon.h>

#include <gui/widgets/edit/assembly_tracking_list_panel.hpp>
#include <gui/widgets/edit/single_assembly_tracking_panel.hpp>

////@begin XPM images
////@end XPM images

#include <wx/stattext.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * CAssemblyTrackingListPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAssemblyTrackingListPanel, CUserFieldListPanel )


/*!
 * CAssemblyTrackingListPanel event table definition
 */

BEGIN_EVENT_TABLE( CAssemblyTrackingListPanel, CUserFieldListPanel )

////@begin CAssemblyTrackingListPanel event table entries
////@end CAssemblyTrackingListPanel event table entries

END_EVENT_TABLE()


/*!
 * CAssemblyTrackingListPanel constructors
 */

CAssemblyTrackingListPanel::CAssemblyTrackingListPanel()
{
    Init();
}

CAssemblyTrackingListPanel::CAssemblyTrackingListPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_UserObj = new CUser_object();
    Create(parent, id, pos, size, style);
}


/*!
 * CAssemblyTrackingListPanel creator
 */

bool CAssemblyTrackingListPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAssemblyTrackingListPanel creation
    CUserFieldListPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAssemblyTrackingListPanel creation
    return true;
}


/*!
 * CAssemblyTrackingListPanel destructor
 */

CAssemblyTrackingListPanel::~CAssemblyTrackingListPanel()
{
////@begin CAssemblyTrackingListPanel destruction
////@end CAssemblyTrackingListPanel destruction
    delete m_FieldManager;
}


/*!
 * Member initialisation
 */

void CAssemblyTrackingListPanel::Init()
{
////@begin CAssemblyTrackingListPanel member initialisation
////@end CAssemblyTrackingListPanel member initialisation
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
    m_FieldManager = new CAssemblyTrackFieldManager();
    SetNeedsEmptyLastRow(true);
}


/*!
 * Control creation for CAssemblyTrackingListPanel
 */

void CAssemblyTrackingListPanel::CreateControls()
{    
////@begin CAssemblyTrackingListPanel content construction
    CAssemblyTrackingListPanel* itemCUserFieldListPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCUserFieldListPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCUserFieldListPanel1, wxID_STATIC, _("Accession"), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCUserFieldListPanel1, wxID_STATIC, _("From"), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCUserFieldListPanel1, wxID_STATIC, _("To"), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer3->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CAssemblyTrackingListPanel content construction
    m_ScrolledWindow = new wxScrolledWindow( itemCUserFieldListPanel1, wxID_ANY, wxDefaultPosition, wxSize(310, 100), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_ScrolledWindow, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxBoxSizer(wxVERTICAL);
    m_ScrolledWindow->SetSizer(m_Sizer);

    m_ScrolledWindow->FitInside();
}


/*!
 * Should we show tooltips?
 */

bool CAssemblyTrackingListPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAssemblyTrackingListPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAssemblyTrackingListPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAssemblyTrackingListPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAssemblyTrackingListPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAssemblyTrackingListPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAssemblyTrackingListPanel icon retrieval
}


END_NCBI_SCOPE
