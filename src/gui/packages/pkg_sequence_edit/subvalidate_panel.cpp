/*  $Id: subvalidate_panel.cpp 43399 2019-06-24 18:58:25Z filippov $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/edit/table_data_validate_params.hpp>
#include <gui/packages/pkg_sequence_edit/validate_panel.hpp>
#include <gui/packages/pkg_sequence_edit/subvalidate_panel.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE

/*
 * CSubValidatePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubValidatePanel, wxPanel )


/*
 * CSubValidatePanel event table definition
 */

BEGIN_EVENT_TABLE( CSubValidatePanel, wxPanel )

////@begin CSubValidatePanel event table entries
    EVT_BUTTON( ID_CSUBVALIDATE_REFRESH, CSubValidatePanel::OnValidateClick )
////@end CSubValidatePanel event table entries

END_EVENT_TABLE()


/*
 * CSubValidatePanel constructors
 */

CSubValidatePanel::CSubValidatePanel()
    : m_Workbench(nullptr)
{
    Init();
}

CSubValidatePanel::CSubValidatePanel( wxWindow* parent, IWorkbench* wb, objects::CGBProjectHandle* ph,
                                      objects::CProjectItem* pi, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Workbench(wb), m_ProjectHandle(ph), m_ProjectItem(pi)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSubValidatePanel creator
 */

bool CSubValidatePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubValidatePanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();   
    Centre();
////@end CSubValidatePanel creation
    return true;
}


/*
 * CSubValidatePanel destructor
 */

CSubValidatePanel::~CSubValidatePanel()
{
////@begin CSubValidatePanel destruction
////@end CSubValidatePanel destruction
}


/*
 * Member initialisation
 */

void CSubValidatePanel::Init()
{
////@begin CSubValidatePanel member initialisation
    m_Panel = NULL;
////@end CSubValidatePanel member initialisation
}


/*
 * Control creation for CSubValidatePanel
 */

void CSubValidatePanel::CreateControls()
{    
////@begin CSubValidatePanel content construction
    CSubValidatePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    CRef<objects::CUser_object> params(new objects::CUser_object());
    CTableDataValidateParams::SetDoAlignment(*params, true);
    CTableDataValidateParams::SetDoInference(*params, false);

    m_Panel = new CValidatePanel( itemPanel1, m_Workbench, params);
    itemBoxSizer2->Add(m_Panel, 1, wxGROW|wxALL, 0);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton4 = new wxButton( itemPanel1, ID_CSUBVALIDATE_REFRESH, _("Validate record"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);   

////@end CSubValidatePanel content construction
}

bool CSubValidatePanel::IsLastPage() const
{
    return false;
}


/*
 * Should we show tooltips?
 */

bool CSubValidatePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSubValidatePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubValidatePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubValidatePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSubValidatePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubValidatePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubValidatePanel icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CSUBVALIDATE_REFRESH
 */

void CSubValidatePanel::OnValidateClick(wxCommandEvent& event)
{
    m_Panel->SetProjectHandleAndItem(m_ProjectHandle, m_ProjectItem);
    m_Panel->LoadData();
    Refresh();
}


bool CSubValidatePanel::TransferDataToWindow()
{
    return wxPanel::TransferDataToWindow();
}

END_NCBI_SCOPE

