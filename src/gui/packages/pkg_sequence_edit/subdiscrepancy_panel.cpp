/*  $Id: subdiscrepancy_panel.cpp 44321 2019-11-29 17:24:25Z filippov $
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
#include <gui/packages/pkg_sequence_edit/subdiscrepancy_panel.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE

/*
 * CSubDiscrepancyPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubDiscrepancyPanel, wxPanel )


/*
 * CSubDiscrepancyPanel event table definition
 */

BEGIN_EVENT_TABLE( CSubDiscrepancyPanel, wxPanel )

////@begin CSubDiscrepancyPanel event table entries
    EVT_BUTTON( ID_CSUBDISCREPANCY_REFRESH, CSubDiscrepancyPanel::OnRefreshClick )
////@end CSubDiscrepancyPanel event table entries

END_EVENT_TABLE()


/*
 * CSubDiscrepancyPanel constructors
 */

CSubDiscrepancyPanel::CSubDiscrepancyPanel()
: m_Workbench(nullptr), m_CmdProcessor(nullptr)
{
    Init();
}

CSubDiscrepancyPanel::CSubDiscrepancyPanel( wxWindow* parent, IWorkbench* wb, const wxString &dir,
                                            CGBProjectHandle* ph, ICommandProccessor* proc, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_Workbench(wb), m_WorkDir(dir), m_ProjectHandle(ph), m_CmdProcessor(proc)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSubDiscrepancyPanel creator
 */

bool CSubDiscrepancyPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubDiscrepancyPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();   
    Centre();
////@end CSubDiscrepancyPanel creation
    return true;
}


/*
 * CSubDiscrepancyPanel destructor
 */

CSubDiscrepancyPanel::~CSubDiscrepancyPanel()
{
////@begin CSubDiscrepancyPanel destruction
////@end CSubDiscrepancyPanel destruction
}


/*
 * Member initialisation
 */

void CSubDiscrepancyPanel::Init()
{
////@begin CSubDiscrepancyPanel member initialisation
    m_Panel = NULL;
    m_Modified = NULL;
////@end CSubDiscrepancyPanel member initialisation
}


/*
 * Control creation for CSubDiscrepancyPanel
 */

void CSubDiscrepancyPanel::CreateControls()
{    
////@begin CSubDiscrepancyPanel content construction
    CSubDiscrepancyPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);
	
    m_Panel = new CDiscrepancyPanel(CDiscrepancyDlg::eSubmitter, itemPanel1, this, m_Workbench);
    itemBoxSizer2->Add(m_Panel, 1, wxGROW|wxALL, 0);
    m_Panel->SetWorkDir(m_WorkDir);
    m_Panel->SetProject(m_ProjectHandle);
    m_Panel->SetCmdProccessor(m_CmdProcessor);
	
    m_Modified = new wxStaticText( itemPanel1, wxID_STATIC, _("Data has been modified, please refresh"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_Modified, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton4 = new wxButton( itemPanel1, ID_CSUBDISCREPANCY_REFRESH, _("Refresh submitter report"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);   

////@end CSubDiscrepancyPanel content construction
}

bool CSubDiscrepancyPanel::IsLastPage() const
{
    return true;
}


/*
 * Should we show tooltips?
 */

bool CSubDiscrepancyPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSubDiscrepancyPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubDiscrepancyPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubDiscrepancyPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSubDiscrepancyPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubDiscrepancyPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubDiscrepancyPanel icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CSUBDISCREPANCY_REFRESH
 */

void CSubDiscrepancyPanel::OnRefreshClick(wxCommandEvent& event)
{   
    m_Modified->SetLabel(wxEmptyString);
    m_Panel->RefreshData();
    Refresh();
}


bool CSubDiscrepancyPanel::TransferDataToWindow()
{
    m_Modified->SetLabel(_("Data has been modified, please refresh"));
    return wxPanel::TransferDataToWindow();
}

END_NCBI_SCOPE

