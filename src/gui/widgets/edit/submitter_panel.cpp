/*  $Id: submitter_panel.cpp 43206 2019-05-28 21:41:23Z asztalos $
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
#include <objects/submit/Contact_info.hpp>
#include <objects/submit/Submit_block.hpp>
#include <gui/widgets/edit/submitter_name_panel.hpp>
#include <gui/widgets/edit/submitter_affil_panel.hpp>
#include <gui/widgets/edit/submitter_panel.hpp>

#include <wx/sizer.h>
#include <wx/listbook.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CSubmitterPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS(CSubmitterPanel, wxPanel )


/*
 * CSubmitterPanel event table definition
 */

BEGIN_EVENT_TABLE(CSubmitterPanel, wxPanel )
    EVT_LISTBOOK_PAGE_CHANGED(ID_CSUBMITTERNOTEBOOK, CSubmitterPanel::OnPageChanged)
    EVT_LISTBOOK_PAGE_CHANGING(ID_CSUBMITTERNOTEBOOK, CSubmitterPanel::OnPageChanging)
END_EVENT_TABLE()


/*
 * CSubmitterPanel constructors
 */

CSubmitterPanel::CSubmitterPanel()
{
    Init();
}

CSubmitterPanel::CSubmitterPanel( wxWindow* parent, CContact_info& contact_info, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Contact(&contact_info)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSubmitterPanel creator
 */

bool CSubmitterPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubmitterPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubmitterPanel creation
    return true;
}


/*
 * CSubmitterPanel destructor
 */

CSubmitterPanel::~CSubmitterPanel()
{
////@begin CSubmitterPanel destruction
////@end CSubmitterPanel destruction
}


/*
 * Member initialisation
 */

void CSubmitterPanel::Init()
{
////@begin CSubmitterPanel member initialisation
    m_Listbook = NULL;
    m_NamePanel = NULL;
    m_AffilPanel = NULL;
////@end CSubmitterPanel member initialisation
}


/*
 * Control creation for CSubmitterPanel
 */

void CSubmitterPanel::CreateControls()
{    
////@begin CSubmitterPanel content construction
    CSubmitterPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Listbook = new wxListbook( itemPanel1, ID_CSUBMITTERNOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_TOP|wxNO_BORDER );
    itemBoxSizer2->Add(m_Listbook, 1, wxGROW | wxALL, 0);

    m_NamePanel = new CSubmitterNamePanel(m_Listbook, *m_Contact);
    m_Listbook->AddPage(m_NamePanel, _("Name"));

    m_AffilPanel = new CSubmitterAffilPanel(m_Listbook);
    m_Listbook->AddPage(m_AffilPanel, _("Affiliation"));

////@end CSubmitterPanel content construction
}

bool CSubmitterPanel::StepForward()
{
    return x_DoStepForward(m_Listbook);
}

bool CSubmitterPanel::StepBackward()
{
    return x_DoStepBackward(m_Listbook);
}

bool CSubmitterPanel::IsFirstPage() const
{
    return (m_Listbook->GetSelection() == 0);
}

void CSubmitterPanel::OnPageChanged(wxBookCtrlEvent& event)
{
    GetParentWizard(this)->UpdateOnPageChange(this);
}

void CSubmitterPanel::OnPageChanging(wxBookCtrlEvent& event)
{  
    ApplyCommand();
}

/*
 * Should we show tooltips?
 */

bool CSubmitterPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSubmitterPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubmitterPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubmitterPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSubmitterPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubmitterPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubmitterPanel icon retrieval
}


void CSubmitterPanel::ApplySubmitBlock(objects::CSubmit_block& block)
{
    m_NamePanel->ApplyContactInfo(block.SetContact());
    m_AffilPanel->ApplySubmitBlock(block);
}

bool CSubmitterPanel::TransferDataToWindow()
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) {
        m_Listbook->GetPage(i)->TransferDataToWindow();
    }
    return wxPanel::TransferDataToWindow();
}

void CSubmitterPanel::ApplyCommand()
{
    wxWindow *win = m_Listbook->GetCurrentPage();
    if (!win)
        return;
    ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
    if (!page)
        return;
    page->ApplyCommand();
}

void CSubmitterPanel::ReportMissingFields(string &text)
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) 
    {
        wxWindow* win = m_Listbook->GetPage(i);
        ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
        page->ReportMissingFields(text);
    }
}

wxString CSubmitterPanel::GetAnchor()
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
