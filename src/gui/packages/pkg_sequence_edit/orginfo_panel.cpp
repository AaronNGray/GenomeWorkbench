/*  $Id: orginfo_panel.cpp 43209 2019-05-29 14:01:26Z bollin $
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
#include <gui/widgets/edit/orggeneral_panel.hpp>
#include <gui/widgets/edit/orgadvanced_panel.hpp>
#include <gui/utils/command.hpp>
#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include "open_data_source_object_dlg.hpp"
#include "attrib_table_column_id_panel.hpp"
#include <gui/utils/object_loader.hpp>
#include <gui/packages/pkg_sequence_edit/qual_table_load_manager.hpp>
#include <gui/packages/pkg_sequence_edit/orginfo_panel.hpp>



#include <wx/sizer.h>
#include <wx/listbook.h>
#include <wx/button.h>
#include <wx/cshelp.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*
 * COrganismInfoPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( COrganismInfoPanel, wxPanel )


/*
 * COrganismInfoPanel event table definition
 */

BEGIN_EVENT_TABLE( COrganismInfoPanel, wxPanel )
    EVT_LISTBOOK_PAGE_CHANGED(ID_ORGINFONTBK, COrganismInfoPanel::OnPageChanged)
    EVT_LISTBOOK_PAGE_CHANGING(ID_ORGINFONTBK, COrganismInfoPanel::OnPageChanging)
    EVT_BUTTON(ID_ORGINFOBTN, COrganismInfoPanel::OnImportSrcTable)
    END_EVENT_TABLE()


/*
 * COrganismInfoPanel constructors
 */

COrganismInfoPanel::COrganismInfoPanel()
{
    Init();
}

COrganismInfoPanel::COrganismInfoPanel( wxWindow* parent, IWorkbench* wb, ICommandProccessor*  cmdproc, objects::CSeq_entry_Handle seh, const wxString& dir, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Workbench(wb), m_Seh(seh), m_CmdProcessor(cmdproc), m_WorkDir(dir)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * COrganismInfoPanel creator
 */

bool COrganismInfoPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin COrganismInfoPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end COrganismInfoPanel creation
    return true;
}


/*
 * COrganismInfoPanel destructor
 */

COrganismInfoPanel::~COrganismInfoPanel()
{
////@begin COrganismInfoPanel destruction
////@end COrganismInfoPanel destruction
}


/*
 * Member initialisation
 */

void COrganismInfoPanel::Init()
{
////@begin COrganismInfoPanel member initialisation
    m_Listbook = NULL;
    m_GeneralPanel = NULL;
    m_AdvancedPanel = NULL;
////@end COrganismInfoPanel member initialisation
}


/*
 * Control creation for COrganismInfoPanel
 */

void COrganismInfoPanel::CreateControls()
{    
////@begin COrganismInfoPanel content construction
    COrganismInfoPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer1, 0, wxALIGN_LEFT|wxALL, 5);

    wxButton* itemButton2 = new wxButton( itemPanel1, ID_ORGINFOBTN, _("Import tab-delimited table"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    //wxContextHelpButton* itemContextHelpButton3 = new wxContextHelpButton( itemPanel1, wxID_CONTEXT_HELP, wxDefaultPosition, wxSize(20, -1), wxBU_AUTODRAW );
    //itemBoxSizer1->Add(itemContextHelpButton3, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    m_Listbook = new wxListbook( itemPanel1, ID_ORGINFONTBK, wxDefaultPosition, wxDefaultSize, wxBK_TOP|wxNO_BORDER );
    itemBoxSizer2->Add(m_Listbook, 1, wxGROW | wxALL, 5);

    m_GeneralPanel = new COrgGeneralPanel(m_Listbook, m_CmdProcessor, m_Seh);
    m_Listbook->AddPage(m_GeneralPanel, _("General"));

    m_AdvancedPanel = new COrgAdvancedPanel(m_Listbook, m_CmdProcessor, m_Seh);
    m_Listbook->AddPage(m_AdvancedPanel, _("Additional qualifiers"));

////@end COrganismInfoPanel content construction
}

bool COrganismInfoPanel::StepForward()
{
    return x_DoStepForward(m_Listbook);
}

bool COrganismInfoPanel::StepBackward()
{  
    return x_DoStepBackward(m_Listbook);
}

    
bool COrganismInfoPanel::TransferDataFromWindow()
{
    wxWindow *win = m_Listbook->GetCurrentPage();
    if (!win)
        return false;
    return win->TransferDataFromWindow();
}


void COrganismInfoPanel::OnPageChanging(wxBookCtrlEvent& event)
{  
    ApplyCommand();
}

void COrganismInfoPanel::OnPageChanged(wxBookCtrlEvent& event)
{  
    GetParentWizard(this)->UpdateOnPageChange(this);
}


void COrganismInfoPanel::OnImportSrcTable(wxCommandEvent& event)
{
     if (!m_Seh) return;
 
     CQualTableLoadManager* qual_mgr = NULL;
     CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
     vector<string> format_ids;
     format_ids.push_back("file_loader_qual_table");
     fileManager->LoadFormats(format_ids);
     fileManager->SetWorkDir(m_WorkDir);
 
     for( size_t i = 0;  i < fileManager->GetFormatManagers().size();  ++i  )   {
         const IFileLoadPanelClient* mgr = fileManager->GetFormatManagers()[0].GetPointerOrNull();
 
         if ("file_loader_qual_table" == mgr->GetFileLoaderId()) {
             const CQualTableLoadManager* const_qual_mgr = dynamic_cast<const CQualTableLoadManager*>(mgr);
             if (const_qual_mgr) {
                 qual_mgr = const_cast<CQualTableLoadManager*>(const_qual_mgr);
                 qual_mgr->SetTopLevelEntry(m_Seh);
                 qual_mgr->SetServiceLocator(m_Workbench);
             }            
         }
     }  
 
     vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
     loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));
 
     COpenObjectsDlg dlg(this);
     dlg.SetSize(760, 940);
     dlg.Centre(wxBOTH|wxCENTRE_ON_SCREEN);
     dlg.SetMinSize(wxSize(760, 940));
     dlg.SetRegistryPath("Dialogs.Edit.OpenTables");
     dlg.SetManagers(loadManagers);
 
     if (dlg.ShowModal() == wxID_OK) {
         CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
         qual_mgr->x_GetColumnIdPanel()->x_TableReaderCommon(object_loader, wxT("Reading file(s)..."));
     }
}


/*
 * Should we show tooltips?
 */

bool COrganismInfoPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap COrganismInfoPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin COrganismInfoPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end COrganismInfoPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon COrganismInfoPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin COrganismInfoPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end COrganismInfoPanel icon retrieval
}


void COrganismInfoPanel::ApplyDescriptor(objects::CSeqdesc& desc)
{
    if (desc.IsSource()) {
        m_GeneralPanel->ApplyBioSource(desc.SetSource());
        m_AdvancedPanel->ApplyBioSource(desc.SetSource());
    }
}

bool COrganismInfoPanel::TransferDataToWindow()
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) {
        m_Listbook->GetPage(i)->TransferDataToWindow();
    }
    return wxPanel::TransferDataToWindow();
}

void COrganismInfoPanel::ApplyCommand()
{
    wxWindow *win = m_Listbook->GetCurrentPage();
    if (!win)
        return;
    ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
    if (!page)
        return;
    page->ApplyCommand();
}

void COrganismInfoPanel::SetSeqEntryHandle(objects::CSeq_entry_Handle seh) 
{ 
    m_Seh = seh; 
    m_GeneralPanel->SetSeqEntryHandle(m_Seh); 
    m_AdvancedPanel->SetSeqEntryHandle(m_Seh); 
}

void COrganismInfoPanel::ReportMissingFields(string &text)
{
    for (size_t i = 0; i < m_Listbook->GetPageCount(); ++i) 
    {
        wxWindow* win = m_Listbook->GetPage(i);
        ISubmissionPage* page = dynamic_cast<ISubmissionPage*>(win);
        page->ReportMissingFields(text);
    }
}

wxString COrganismInfoPanel::GetAnchor()
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
