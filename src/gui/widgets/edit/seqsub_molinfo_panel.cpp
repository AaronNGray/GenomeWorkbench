/*  $Id: seqsub_molinfo_panel.cpp 43209 2019-05-29 14:01:26Z bollin $
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
#include <gui/widgets/edit/seqsub_plasmid_panel.hpp>
#include <gui/widgets/edit/seqsub_organelle_panel.hpp>
#include <gui/widgets/edit/seqsub_chrm_panel.hpp>
#include <gui/widgets/edit/seqsub_molinfo_panel.hpp>

#include <wx/sizer.h>
#include <wx/listbook.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE

/*
 * CSubMolinfoPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubMolinfoPanel, wxPanel )


/*
 * CSubMolinfoPanel event table definition
 */

BEGIN_EVENT_TABLE( CSubMolinfoPanel, wxPanel )
    EVT_LISTBOOK_PAGE_CHANGED(ID_MOLINFONOTEBOOK, CSubMolinfoPanel::OnPageChanged)
    EVT_LISTBOOK_PAGE_CHANGING(ID_MOLINFONOTEBOOK, CSubMolinfoPanel::OnPageChanging)
END_EVENT_TABLE()


/*
 * CSubMolinfoPanel constructors
 */

CSubMolinfoPanel::CSubMolinfoPanel()
{
    Init();
}

CSubMolinfoPanel::CSubMolinfoPanel( wxWindow* parent, ICommandProccessor* cmdproc, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSubMolinfoPanel creator
 */

bool CSubMolinfoPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubMolinfoPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubMolinfoPanel creation
    return true;
}


/*
 * CSubMolinfoPanel destructor
 */

CSubMolinfoPanel::~CSubMolinfoPanel()
{
////@begin CSubMolinfoPanel destruction
////@end CSubMolinfoPanel destruction
}


/*
 * Member initialisation
 */

void CSubMolinfoPanel::Init()
{
////@begin CSubMolinfoPanel member initialisation
    m_Listbook = NULL;
    m_PlasmidPanel = NULL;
    m_ChromosomePanel = NULL;
    m_OrganellePanel = NULL;
////@end CSubMolinfoPanel member initialisation
}


/*
 * Control creation for CSubMolinfoPanel
 */

void CSubMolinfoPanel::CreateControls()
{    
////@begin CSubMolinfoPanel content construction
    CSubMolinfoPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Listbook = new wxListbook( itemPanel1, ID_MOLINFONOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_TOP|wxNO_BORDER );
    itemBoxSizer2->Add(m_Listbook, 1, wxGROW | wxALL, 5);

    m_ChromosomePanel = new CSeqSubChromoPanel(m_Listbook, m_CmdProcessor, m_Seh);
    m_Listbook->AddPage(m_ChromosomePanel, _("Chromosome"));

    m_PlasmidPanel = new CSeqSubPlasmidPanel(m_Listbook, m_CmdProcessor, m_Seh);
    m_Listbook->AddPage(m_PlasmidPanel, _("Plasmid"));

    m_OrganellePanel = new CSeqSubOrganellePanel(m_Listbook, m_CmdProcessor, m_Seh);
    m_Listbook->AddPage(m_OrganellePanel, _("Organelle"));

////@end CSubMolinfoPanel content construction
}

bool CSubMolinfoPanel::StepForward()
{
    return x_DoStepForward(m_Listbook);
}

bool CSubMolinfoPanel::StepBackward()
{
    return x_DoStepBackward(m_Listbook);
}

void CSubMolinfoPanel::OnPageChanged(wxBookCtrlEvent& event)
{
    GetParentWizard(this)->UpdateOnPageChange(this);
}

/*
 * Should we show tooltips?
 */

bool CSubMolinfoPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSubMolinfoPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubMolinfoPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubMolinfoPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSubMolinfoPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubMolinfoPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubMolinfoPanel icon retrieval
}

bool CSubMolinfoPanel::TransferDataToWindow()
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) {
        m_Listbook->GetPage(i)->TransferDataToWindow();
    }
    return wxPanel::TransferDataToWindow();
}

void CSubMolinfoPanel::OnPageChanging(wxBookCtrlEvent& event)
{  
    ApplyCommand();
}

void CSubMolinfoPanel::ApplyCommand()
{
    wxWindow *win = m_Listbook->GetCurrentPage();
    if (!win)
        return;
    ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
    if (!page)
        return;
    page->ApplyCommand();
}

void CSubMolinfoPanel::SetSeqEntryHandle(objects::CSeq_entry_Handle seh) 
{ 
    m_Seh = seh; 
    m_PlasmidPanel->SetSeqEntryHandle(m_Seh); 
    m_ChromosomePanel->SetSeqEntryHandle(m_Seh); 
    m_OrganellePanel->SetSeqEntryHandle(m_Seh); 
}

void CSubMolinfoPanel::ReportMissingFields(string &text)
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) 
    {
        wxWindow* win = m_Listbook->GetPage(i);
        ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
        page->ReportMissingFields(text);
    }
}

wxString CSubMolinfoPanel::GetAnchor()
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
