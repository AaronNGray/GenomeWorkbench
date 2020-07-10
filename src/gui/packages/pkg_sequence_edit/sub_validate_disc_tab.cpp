/*  $Id: sub_validate_disc_tab.cpp 43399 2019-06-24 18:58:25Z filippov $
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
#include <gui/packages/pkg_sequence_edit/subvalidate_panel.hpp>
#include <gui/packages/pkg_sequence_edit/subdiscrepancy_panel.hpp>
#include <gui/packages/pkg_sequence_edit/sub_validate_disc_tab.hpp>
#include <gui/utils/command.hpp>
#include <gui/widgets/edit/sequence_editing_cmds.hpp>

#include <wx/sizer.h>
#include <wx/listbook.h>
#include <wx/button.h>
#include <wx/cshelp.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*
 * CSubValidateDiscPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubValidateDiscPanel, wxPanel )


/*
 * CSubValidateDiscPanel event table definition
 */

BEGIN_EVENT_TABLE( CSubValidateDiscPanel, wxPanel )
    EVT_LISTBOOK_PAGE_CHANGED(ID_VALID_DISC_LISTBOOK, CSubValidateDiscPanel::OnPageChanged)
    EVT_LISTBOOK_PAGE_CHANGING(ID_VALID_DISC_LISTBOOK, CSubValidateDiscPanel::OnPageChanging)
    END_EVENT_TABLE()


/*
 * CSubValidateDiscPanel constructors
 */

CSubValidateDiscPanel::CSubValidateDiscPanel()
{
    Init();
}

CSubValidateDiscPanel::CSubValidateDiscPanel( wxWindow* parent, IWorkbench* wb, const wxString &dir,
                           CGBProjectHandle* ph, CProjectItem* pi, ICommandProccessor* proc, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Workbench(wb), m_WorkDir(dir), m_ProjectHandle(ph), m_ProjectItem(pi), m_CmdProcessor(proc)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSubValidateDiscPanel creator
 */

bool CSubValidateDiscPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubValidateDiscPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubValidateDiscPanel creation
    return true;
}


/*
 * CSubValidateDiscPanel destructor
 */

CSubValidateDiscPanel::~CSubValidateDiscPanel()
{
////@begin CSubValidateDiscPanel destruction
////@end CSubValidateDiscPanel destruction
}


/*
 * Member initialisation
 */

void CSubValidateDiscPanel::Init()
{
////@begin CSubValidateDiscPanel member initialisation
    m_Listbook = NULL;
    m_ValidatePanel = NULL;
    m_DiscrepancyPanel = NULL;
////@end CSubValidateDiscPanel member initialisation
}


/*
 * Control creation for CSubValidateDiscPanel
 */

void CSubValidateDiscPanel::CreateControls()
{    
////@begin CSubValidateDiscPanel content construction
    CSubValidateDiscPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Listbook = new wxListbook( itemPanel1, ID_VALID_DISC_LISTBOOK, wxDefaultPosition, wxDefaultSize, wxBK_TOP|wxNO_BORDER );
    itemBoxSizer2->Add(m_Listbook, 1, wxGROW | wxALL, 5);

    m_ValidatePanel = new CSubValidatePanel(m_Listbook, m_Workbench, m_ProjectHandle, m_ProjectItem);
    m_Listbook->AddPage(m_ValidatePanel, _("Validate"));

    m_DiscrepancyPanel = new CSubDiscrepancyPanel(m_Listbook, m_Workbench, m_WorkDir, m_ProjectHandle, m_CmdProcessor);
    m_Listbook->AddPage(m_DiscrepancyPanel, _("Submitter Report"));

////@end CSubValidateDiscPanel content construction
}

bool CSubValidateDiscPanel::StepForward()
{
    return x_DoStepForward(m_Listbook);
}

bool CSubValidateDiscPanel::StepBackward()
{  
    return x_DoStepBackward(m_Listbook);
}
    
void CSubValidateDiscPanel::OnPageChanging(wxBookCtrlEvent& event)
{  
    ApplyCommand();
}

void CSubValidateDiscPanel::OnPageChanged(wxBookCtrlEvent& event)
{  
    GetParentWizard(this)->UpdateOnPageChange(this);
}

/*
 * Should we show tooltips?
 */

bool CSubValidateDiscPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSubValidateDiscPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubValidateDiscPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubValidateDiscPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSubValidateDiscPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubValidateDiscPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubValidateDiscPanel icon retrieval
}

bool CSubValidateDiscPanel::TransferDataToWindow()
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) {
        m_Listbook->GetPage(i)->TransferDataToWindow();
    }
    return wxPanel::TransferDataToWindow();
}

void CSubValidateDiscPanel::ApplyCommand()
{
    wxWindow *win = m_Listbook->GetCurrentPage();
    if (!win)
        return;
    ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
    if (!page)
        return;
    page->ApplyCommand();
}

bool CSubValidateDiscPanel::IsLastPage() const
{
    wxWindow *win = m_Listbook->GetCurrentPage();
    if (!win)
        return false;
    ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
    if (!page)
        return false;
    return page->IsLastPage();
}

void CSubValidateDiscPanel::SetProjectItem(objects::CProjectItem* pi) 
{
    m_ProjectItem = pi; 
    m_ValidatePanel->SetProjectItem(pi);
}

wxString CSubValidateDiscPanel::GetAnchor()
{
    wxWindow *win = m_Listbook->GetCurrentPage();
    if (!win)
        return wxEmptyString;
    ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
    if (!page)
        return wxEmptyString;
    return page->GetAnchor();
}

END_NCBI_SCOPE
