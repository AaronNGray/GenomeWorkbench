/*  $Id: dblinkpanel.cpp 37106 2016-12-05 19:27:24Z asztalos $
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
#include "string_list_ctrl.hpp"
////@end includes

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/icon.h>

#include <gui/widgets/edit/dblinkpanel.hpp>
#include <gui/widgets/edit/userfield_stringlist_validator.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CDBLinkPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDBLinkPanel, wxPanel )


/*!
 * CDBLinkPanel event table definition
 */

BEGIN_EVENT_TABLE( CDBLinkPanel, wxPanel )

////@begin CDBLinkPanel event table entries
////@end CDBLinkPanel event table entries

END_EVENT_TABLE()


/*!
 * CDBLinkPanel constructors
 */

CDBLinkPanel::CDBLinkPanel()
{
    Init();
}

CDBLinkPanel::CDBLinkPanel( wxWindow* parent, CRef<CUser_object> user, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_User = new objects::CUser_object();
    if (user) {
        m_User->Assign(*user);
    }
    Create(parent, id, pos, size, style);
}


/*!
 * CDBLinkPanel creator
 */

bool CDBLinkPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDBLinkPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CDBLinkPanel creation
    return true;
}


/*!
 * CDBLinkPanel destructor
 */

CDBLinkPanel::~CDBLinkPanel()
{
////@begin CDBLinkPanel destruction
////@end CDBLinkPanel destruction
}


/*!
 * Member initialisation
 */

void CDBLinkPanel::Init()
{
////@begin CDBLinkPanel member initialisation
    m_BioProject = NULL;
    m_BioSample = NULL;
    m_ProbeDB = NULL;
    m_TraceAssembly = NULL;
    m_SRA = NULL;
////@end CDBLinkPanel member initialisation
}


/*!
 * Control creation for CDBLinkPanel
 */

void CDBLinkPanel::CreateControls()
{    
////@begin CDBLinkPanel content construction
    CDBLinkPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("BioProject"), wxDefaultPosition, wxSize(250, -1), wxALIGN_CENTRE );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(0, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_BioProject = new CStringListCtrl( itemPanel1, ID_BIOPROJECT, wxDefaultPosition, wxSize(100, 100), wxSUNKEN_BORDER|wxScrolledWindowStyle|wxTAB_TRAVERSAL );
    itemFlexGridSizer3->Add(m_BioProject, 1, wxGROW | wxALL, 0);

    itemFlexGridSizer3->Add(0, 75, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("BioSample"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(0, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_BioSample = new CStringListCtrl( itemPanel1, ID_BIOSAMPLE, wxDefaultPosition, wxSize(100, 100), wxSUNKEN_BORDER|wxScrolledWindowStyle|wxTAB_TRAVERSAL );
    itemFlexGridSizer3->Add(m_BioSample, 1, wxGROW|wxALL, 0);

    itemFlexGridSizer3->Add(0, 75, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("ProbeDB"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(0, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProbeDB = new CStringListCtrl( itemPanel1, ID_PROBEDB, wxDefaultPosition, wxSize(100, 100), wxSUNKEN_BORDER|wxScrolledWindowStyle|wxTAB_TRAVERSAL );
    itemFlexGridSizer3->Add(m_ProbeDB, 1, wxGROW|wxALL, 0);

    itemFlexGridSizer3->Add(0, 75, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("Trace Assembly"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(0, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TraceAssembly = new CStringListCtrl( itemPanel1, ID_TRACEASSEMBLY, wxDefaultPosition, wxSize(100, 100), wxSUNKEN_BORDER|wxScrolledWindowStyle|wxTAB_TRAVERSAL );
    itemFlexGridSizer3->Add(m_TraceAssembly, 1, wxGROW|wxALL, 0);

    itemFlexGridSizer3->Add(0, 75, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText20 = new wxStaticText( itemPanel1, wxID_STATIC, _("Sequence Read Archive"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText20, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(0, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_SRA = new CStringListCtrl( itemPanel1, ID_SRA, wxDefaultPosition, wxSize(100, 100), wxSUNKEN_BORDER|wxScrolledWindowStyle|wxTAB_TRAVERSAL );
    itemFlexGridSizer3->Add(m_SRA, 1, wxGROW|wxALL, 0);

    itemFlexGridSizer3->Add(0, 75, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    // Set validators
    m_BioProject->SetValidator( CUserFieldStringListValidator(*m_User, "BioProject") );
    m_BioSample->SetValidator( CUserFieldStringListValidator(*m_User, "BioSample") );
    m_ProbeDB->SetValidator( CUserFieldStringListValidator(*m_User, "ProbeDB") );
    m_TraceAssembly->SetValidator( CUserFieldStringListValidator(*m_User, "Trace Assembly") );
    m_SRA->SetValidator( CUserFieldStringListValidator(*m_User, "Sequence Read Archive") );
////@end CDBLinkPanel content construction
}


bool CDBLinkPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;
        
    return true;
}


bool CDBLinkPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) 
        return false;
    return true;
}


CRef<objects::CUser_object> CDBLinkPanel::GetUser_object() const
{
    if (m_User)
        return m_User;

    return CRef<objects::CUser_object>();
}


/*!
 * Should we show tooltips?
 */

bool CDBLinkPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDBLinkPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDBLinkPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CDBLinkPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDBLinkPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDBLinkPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CDBLinkPanel icon retrieval
}


void CDBLinkPanel::ChangeSeqdesc(const objects::CSeqdesc& desc)
{
    m_User.Reset(new CUser_object);
    m_User->Assign(desc.GetUser());
    m_BioProject->SetValidator( CUserFieldStringListValidator(*m_User, "BioProject") );
    m_BioSample->SetValidator( CUserFieldStringListValidator(*m_User, "BioSample") );
    m_ProbeDB->SetValidator( CUserFieldStringListValidator(*m_User, "ProbeDB") );
    m_TraceAssembly->SetValidator( CUserFieldStringListValidator(*m_User, "Trace Assembly") );
    m_SRA->SetValidator( CUserFieldStringListValidator(*m_User, "Sequence Read Archive") );
    TransferDataToWindow();
}


void CDBLinkPanel::UpdateSeqdesc(objects::CSeqdesc& desc)
{
    TransferDataFromWindow();
    CRef<CUser_object> user = GetUser_object();
    desc.SetUser(*user);
}


END_NCBI_SCOPE
