/*  $Id: genomeinfo_panel.cpp 43209 2019-05-29 14:01:26Z bollin $
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
#include <gui/widgets/edit/gassembly_panel.hpp>
#include <gui/widgets/edit/gassembly_other_panel.hpp>
#include <gui/widgets/edit/genomeinfo_panel.hpp>
#include <objects/general/User_object.hpp>
#include <objects/valid/Comment_rule.hpp>

#include <wx/sizer.h>
#include <wx/listbook.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*
 * CGenomeInfoPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGenomeInfoPanel, wxPanel )


/*
 * CGenomeInfoPanel event table definition
 */

BEGIN_EVENT_TABLE( CGenomeInfoPanel, wxPanel )
    EVT_LISTBOOK_PAGE_CHANGED(ID_GENOMENOTEBOOK, CGenomeInfoPanel::OnPageChanged)
    EVT_LISTBOOK_PAGE_CHANGING(ID_GENOMENOTEBOOK, CGenomeInfoPanel::OnPageChanging)
END_EVENT_TABLE()


/*
 * CGenomeInfoPanel constructors
 */

CGenomeInfoPanel::CGenomeInfoPanel()
{
    Init();
}

CGenomeInfoPanel::CGenomeInfoPanel( wxWindow* parent,  ICommandProccessor*  cmdproc, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Seh(seh), m_CmdProcessor(cmdproc)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CGenomeInfoPanel creator
 */

bool CGenomeInfoPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGenomeInfoPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGenomeInfoPanel creation
    return true;
}


/*
 * CGenomeInfoPanel destructor
 */

CGenomeInfoPanel::~CGenomeInfoPanel()
{
////@begin CGenomeInfoPanel destruction
////@end CGenomeInfoPanel destruction
}


/*
 * Member initialisation
 */

void CGenomeInfoPanel::Init()
{
////@begin CGenomeInfoPanel member initialisation
    m_Listbook = NULL;
    m_AssemblyPanel = NULL;
    m_OtherPanel = NULL;
////@end CGenomeInfoPanel member initialisation
}


/*
 * Control creation for CGenomeInfoPanel
 */

void CGenomeInfoPanel::CreateControls()
{    
////@begin CGenomeInfoPanel content construction
    CGenomeInfoPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Listbook = new wxListbook( itemPanel1, ID_GENOMENOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_TOP|wxNO_BORDER );
    itemBoxSizer2->Add(m_Listbook, 1, wxGROW | wxALL, 5);

    m_AssemblyPanel = new CGAssemblyPanel(m_Listbook, m_CmdProcessor, m_Seh);
    m_Listbook->AddPage(m_AssemblyPanel, _("Assembly"));

    m_OtherPanel = new CGAssemblyOtherPanel(m_Listbook, m_CmdProcessor, m_Seh);
    m_Listbook->AddPage(m_OtherPanel, _("Sequencing information"));

    

////@end CGenomeInfoPanel content construction
}

bool CGenomeInfoPanel::StepForward()
{
    return x_DoStepForward(m_Listbook);
}

bool CGenomeInfoPanel::StepBackward()
{
    return x_DoStepBackward(m_Listbook);
}

void CGenomeInfoPanel::OnPageChanged(wxBookCtrlEvent& event)
{
    GetParentWizard(this)->UpdateOnPageChange(this);
}

/*
 * Should we show tooltips?
 */

bool CGenomeInfoPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CGenomeInfoPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGenomeInfoPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGenomeInfoPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CGenomeInfoPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGenomeInfoPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGenomeInfoPanel icon retrieval
}


void CGenomeInfoPanel::ApplyDescriptor(objects::CSeqdesc& desc)
{
    if (!desc.IsUser() || desc.GetUser().GetObjectType() != objects::CUser_object::eObjectType_StructuredComment) {
        return;
    }
    objects::CUser_object& user = desc.SetUser();
    string prefix = objects::CComment_rule::GetStructuredCommentPrefix(user);
    if (NStr::Equal(prefix, "Genome-Assembly-Data")) {
        m_AssemblyPanel->ApplyUser(user);
        m_OtherPanel->ApplyUser(user);
    }
}

bool CGenomeInfoPanel::TransferDataToWindow()
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) {
        m_Listbook->GetPage(i)->TransferDataToWindow();
    }
    return wxPanel::TransferDataToWindow();
}

void CGenomeInfoPanel::OnPageChanging(wxBookCtrlEvent& event)
{  
    ApplyCommand();
}

void CGenomeInfoPanel::ApplyCommand()
{
    wxWindow *win = m_Listbook->GetCurrentPage();
    if (!win)
        return;
    ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
    if (!page)
        return;
    page->ApplyCommand();
}

void CGenomeInfoPanel::SetSeqEntryHandle(objects::CSeq_entry_Handle seh) 
{ 
    m_Seh = seh; 
    m_AssemblyPanel->SetSeqEntryHandle(m_Seh); 
    m_OtherPanel->SetSeqEntryHandle(m_Seh); 
}

void CGenomeInfoPanel::ReportMissingFields(string &text)
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) 
    {
        wxWindow* win = m_Listbook->GetPage(i);
        ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
        page->ReportMissingFields(text);
    }
}

wxString CGenomeInfoPanel::GetAnchor()
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
