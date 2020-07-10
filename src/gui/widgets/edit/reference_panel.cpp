/*  $Id: reference_panel.cpp 43221 2019-05-29 19:53:05Z filippov $
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
#include <gui/widgets/edit/pubauthor_panel.hpp>
#include <gui/widgets/edit/pubstatus_panel.hpp>
#include <gui/widgets/edit/reference_panel.hpp>
#include <gui/widgets/edit/authornames_panel.hpp>

#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/submit/Contact_info.hpp>

#include <wx/sizer.h>
#include <wx/listbook.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE

/*
 * CReferencePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CReferencePanel, wxPanel )


/*
 * CReferencePanel event table definition
 */

BEGIN_EVENT_TABLE( CReferencePanel, wxPanel )
    EVT_LISTBOOK_PAGE_CHANGED(ID_REFERENCELISTBOOK, CReferencePanel::OnPageChanged)
    EVT_LISTBOOK_PAGE_CHANGING(ID_REFERENCELISTBOOK, CReferencePanel::OnPageChanging)
END_EVENT_TABLE()


/*
 * CReferencePanel constructors
 */

CReferencePanel::CReferencePanel()
{
    Init();
}

CReferencePanel::CReferencePanel( wxWindow* parent, ICommandProccessor*  cmdproc, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CReferencePanel creator
 */

bool CReferencePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CReferencePanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CReferencePanel creation
    return true;
}


/*
 * CReferencePanel destructor
 */

CReferencePanel::~CReferencePanel()
{
////@begin CReferencePanel destruction
////@end CReferencePanel destruction
}


/*
 * Member initialisation
 */

void CReferencePanel::Init()
{
////@begin CReferencePanel member initialisation
    m_Listbook = NULL;
    //m_AuthorPanel = NULL;
    m_StatusPanel = NULL;
////@end CReferencePanel member initialisation
}


/*
 * Control creation for CReferencePanel
 */

void CReferencePanel::CreateControls()
{    
////@begin CReferencePanel content construction
    CReferencePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Listbook = new wxListbook( itemPanel1, ID_REFERENCELISTBOOK, wxDefaultPosition, wxDefaultSize, wxBK_TOP|wxNO_BORDER );
    itemBoxSizer2->Add(m_Listbook, 1, wxGROW|wxALL, 5);

    CRef<objects::CAuth_list> authlist(new objects::CAuth_list());
    m_AuthorsPanel = new CAuthorNamesPanel(m_Listbook, *authlist, false);
    m_Listbook->AddPage(m_AuthorsPanel, wxT("Sequence authors"));

    m_StatusPanel = new CPubStatusPanel(m_Listbook, m_CmdProcessor, m_Seh);
    m_Listbook->AddPage(m_StatusPanel, _("Publication"));

}

bool CReferencePanel::StepForward()
{
    return x_DoStepForward(m_Listbook);
}

bool CReferencePanel::StepBackward()
{
    return x_DoStepBackward(m_Listbook);
}

void CReferencePanel::OnPageChanging(wxBookCtrlEvent& event)
{  
    ApplyCommand();
}

void CReferencePanel::OnPageChanged(wxBookCtrlEvent& event)
{
    GetParentWizard(this)->UpdateOnPageChange(this);
}

/*
 * Should we show tooltips?
 */

bool CReferencePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CReferencePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CReferencePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CReferencePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CReferencePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CReferencePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CReferencePanel icon retrieval
}


void CReferencePanel::ApplySubmitBlock(objects::CSubmit_block& block)
{
    m_SubmitBlock.Reset(&block);
    m_AuthorsPanel->ApplySubmitBlock(block);        
    m_StatusPanel->ApplyCitSub(block.SetCit());
}


void CReferencePanel::ApplyDescriptor(objects::CSeqdesc& desc)
{
    m_StatusPanel->ApplyPub(desc.SetPub());
}


bool CReferencePanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }
    m_AuthorsPanel->PopulateAuthors(m_SubmitBlock->SetCit().SetAuthors());   
    return true;
}


bool CReferencePanel::MatchesCitSubAuthors(const objects::CAuth_list& auth_list)
{   
    m_AuthorsPanel->PopulateAuthors(m_SubmitBlock->SetCit().SetAuthors());  
    if (m_SubmitBlock && m_SubmitBlock->IsSetCit() && m_SubmitBlock->GetCit().IsSetAuthors() && m_SubmitBlock->GetCit().GetAuthors().Equals(auth_list)) {
        return true;
    } else {
        return false;
    }
}


bool CReferencePanel::CopyAuthors(objects::CAuth_list& authlist)
{
    m_AuthorsPanel->PopulateAuthors(m_SubmitBlock->SetCit().SetAuthors());  
    if (m_SubmitBlock && m_SubmitBlock->IsSetCit() && m_SubmitBlock->GetCit().IsSetAuthors()) {
        authlist.Assign(m_SubmitBlock->GetCit().GetAuthors());
        return true;
    } else {
        return false;
    }
}

void CReferencePanel::CopyAffil(objects::CAuth_list &auth_list)
{
    if (m_SubmitBlock && m_SubmitBlock->IsSetContact() && m_SubmitBlock->GetContact().IsSetContact() && m_SubmitBlock->GetContact().GetContact().IsSetAffil()) {
        auth_list.SetAffil().Assign(m_SubmitBlock->GetContact().GetContact().GetAffil());
    }
}

bool CReferencePanel::TransferDataToWindow()
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) {
        m_Listbook->GetPage(i)->TransferDataToWindow();
    }
    return wxPanel::TransferDataToWindow();
}


void CReferencePanel::ApplyCommand()
{
    wxWindow *win = m_Listbook->GetCurrentPage();
    if (!win)
        return;
    ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
    if (!page)
        return;
    TransferDataFromWindow();
    page->ApplyCommand();
}

void CReferencePanel::SetSeqEntryHandle(objects::CSeq_entry_Handle seh) 
{ 
    m_Seh = seh; 
    m_StatusPanel->SetSeqEntryHandle(m_Seh); 
}

void CReferencePanel::ReportMissingFields(string &text)
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) 
    {
        wxWindow* win = m_Listbook->GetPage(i);
        ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
        page->ReportMissingFields(text);
    }
}

wxString CReferencePanel::GetAnchor()
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
