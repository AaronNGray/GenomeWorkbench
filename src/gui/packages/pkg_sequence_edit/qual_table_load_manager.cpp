/*  $Id: qual_table_load_manager.cpp 43762 2019-08-28 17:29:25Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin and Bob Falk
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence_edit/qual_table_load_manager.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/loaders/table_object_loader.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/framework/service.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>

#include "attrib_table_delimiters_panel.hpp"
#include "attrib_table_column_id_panel.hpp"

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

//
// CDelimitersPage navigation
//

wxPanel* CQualTableLoadManager::CDelimitersPage::GetPanel()
{
    return m_Manager.x_GetDelimitersPanel();
}

bool CQualTableLoadManager::CDelimitersPage::CanLeavePage(bool forward)
{
    CAttribTableDelimitersPanel* panel = m_Manager.x_GetDelimitersPanel();
    if (forward) {
        if (!panel->IsInputValid())
            return false;

        panel->SaveSettings();
        return true;
    }

    panel->SaveSettings();
    return true;
}

IWizardPage* CQualTableLoadManager::CDelimitersPage::GetNextPage()
{
    m_Manager.m_ColumnIdPage.SetPrevPage(this);
    return &m_Manager.m_ColumnIdPage;
}

//
// CColumnIdPage navigation
//

wxPanel* CQualTableLoadManager::CColumnIdPage::GetPanel()
{
    return m_Manager.x_GetColumnIdPanel();
}

bool CQualTableLoadManager::CColumnIdPage::CanLeavePage(bool forward)
{
    CAttribTableColumnIdPanel* panel = m_Manager.x_GetColumnIdPanel();
    if (forward) {
        if (!panel->IsInputValid())
            return false;

        panel->SaveSettings();
        m_Manager.x_ConvertToSeqAnnot();
        return true;
    }

    panel->SaveSettings();
    return true;
}

IWizardPage* CQualTableLoadManager::CColumnIdPage::GetPrevPage()
{
    return &m_Manager.m_DelimitersPage;
}

IWizardPage* CQualTableLoadManager::CColumnIdPage::GetNextPage()
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// CQualTableLoadManager
CQualTableLoadManager::CQualTableLoadManager()
:   m_Descr("Qualifier Table files", ""),
    m_SrvLocator(NULL),
    m_ParentWindow(NULL),
    m_ImportedTableData(new CTableImportDataSource()),
    m_AnnotTableData(new CTableAnnotDataSource()),
    m_TableDelimitersPanel(),
    m_TableColumnIdPanel(),   
    m_DelimitersPage(*this),    
    m_ColumnIdPage(*this),
    m_CurrentPage(&m_DelimitersPage)
{
    m_Descr.SetLogEvent("loaders");
}

//
// IFileLoadPanelClient implementation
//

string CQualTableLoadManager::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CQualTableLoadManager::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//


void CQualTableLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}

void CQualTableLoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CQualTableLoadManager::GetDescriptor() const
{
    return m_Descr;
}


void CQualTableLoadManager::InitUI()
{
    m_CurrentPage = &m_DelimitersPage;
}


void CQualTableLoadManager::CleanUI()
{
    m_TableDelimitersPanel = NULL;
    m_TableColumnIdPanel = NULL;
    m_CurrentPage = &m_DelimitersPage;
}


wxPanel* CQualTableLoadManager::GetCurrentPanel()
{
    return m_CurrentPage ? m_CurrentPage->GetPanel() : (wxPanel*)0;
}

void CQualTableLoadManager::x_ConvertToSeqAnnot()
{
    // Now convert the table to a seq-annot - could be slow if there is a lot
    // of data so use async converter to give user a message as to what's
    // going on.   
    m_AnnotTableData.Reset(new CTableAnnotDataSource());   

    LOG_POST(Info << "Import Table - Converting SeqTable to SeqAnnot");                

    GUI_AsyncExec([this](ICanceled&) { m_ImportedTableData->ConvertToSeqAnnot(m_AnnotTableData->GetContainer()); },
        wxT("Preparing Table Data..."));
}

static const string kDelimitersTag = ".AttribDelimitersPanel";
static const string kColumnIdTag = ".AttribColumnIdPanel";

void CQualTableLoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CQualTableLoadManager::SaveSettings() const
{
}


void CQualTableLoadManager::LoadSettings()
{
}

CAttribTableDelimitersPanel* CQualTableLoadManager::x_GetDelimitersPanel()
{
    if (m_TableDelimitersPanel == NULL) {
        m_TableDelimitersPanel = new CAttribTableDelimitersPanel(m_ParentWindow);
        m_TableDelimitersPanel->SetImportDataSource(m_ImportedTableData);
        m_TableDelimitersPanel->SetLoadManager(this);

        // If filename already set, load it:
        if (!m_FileNames.empty()) {
            m_TableDelimitersPanel->SetPreviewFileName(m_FileNames[0]);
            m_TableDelimitersPanel->PreviewData();
        }
     
        if ( !m_RegPath.empty() ) {
            m_TableDelimitersPanel->SetRegistryPath(m_RegPath + kDelimitersTag);
            m_TableDelimitersPanel->LoadSettings();
        }
    }
    return m_TableDelimitersPanel;
}

CAttribTableColumnIdPanel* CQualTableLoadManager::x_GetColumnIdPanel()
{
    if (m_TableColumnIdPanel == NULL) {
        m_TableColumnIdPanel = new CAttribTableColumnIdPanel(m_ParentWindow);
        m_TableColumnIdPanel->SetImportDataSource(m_ImportedTableData);
        m_TableColumnIdPanel->SetTopLevelEntry(m_TopSeqEntry);
        m_TableColumnIdPanel->SetFileNames(m_FileNames);

        CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
        if (srv)
        {
            CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
            if (ws) {
                CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
                if (doc) {
                    ICommandProccessor* cp = &doc->GetUndoManager();
                    m_TableColumnIdPanel->SetCommandProcessor(cp);
                }
            }
        }

     
        if ( !m_RegPath.empty() ) {
            m_TableColumnIdPanel->SetRegistryPath(m_RegPath + kColumnIdTag);
            m_TableColumnIdPanel->LoadSettings();
        }
    }
    return m_TableColumnIdPanel;
}

bool CQualTableLoadManager::CanDo(EAction action)
{
    if (m_CurrentPage == 0)
        return false;

    if (IsInitialState()) {
        return action == eNext;
    }

    if (action == eBack || action == eNext)
        return true;

    _ASSERT(false);
    return false;
}


bool CQualTableLoadManager::IsFinalState()
{
    return (m_CurrentPage->GetNextPage() == 0);
}


bool CQualTableLoadManager::IsCompletedState()
{
    return (m_CurrentPage == 0);
}


bool CQualTableLoadManager::DoTransition(EAction action)
{
    if (eNext == action) {
        if (m_CurrentPage == 0)
            return false;

        if (!m_CurrentPage->CanLeavePage(true))
            return false;

        m_CurrentPage = m_CurrentPage->GetNextPage();
        return true;
    }
    else if (eBack == action) {
        if (m_CurrentPage == 0) {
            m_CurrentPage = &m_ColumnIdPage;
        } else {
            m_CurrentPage = m_CurrentPage->GetPrevPage();
        }
        return true;
    }
    else {
        return false;
    }
    _ASSERT(false);
    return false;
}


IAppTask* CQualTableLoadManager::GetTask()
{
    return nullptr;
}

IExecuteUnit* CQualTableLoadManager::GetExecuteUnit()
{
    return new CTableObjectLoader(*m_AnnotTableData, m_FileNames);
}

wxString CQualTableLoadManager::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CQualTableLoadManager::ValidateFilenames(const vector<wxString>& filenames)
{
    if (filenames.empty())
        return false;

    CAttribTableDelimitersPanel* delimiters_panel = x_GetDelimitersPanel();
    delimiters_panel->SetPreviewFileName(filenames[0]);

    return delimiters_panel->PreviewData();
}


void CQualTableLoadManager::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CQualTableLoadManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool CQualTableLoadManager::IsInitialState()
{
    return (m_CurrentPage == &m_DelimitersPage);
}


bool CQualTableLoadManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kTable, ext);
}

bool CQualTableLoadManager::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    return fmt == CFormatGuess::eTable;
}

string CQualTableLoadManager::GetExtensionIdentifier() const
{
    static string sid("qual_table_load_manager");
    return sid;
}


string CQualTableLoadManager::GetExtensionLabel() const
{
    static string slabel("Qualifier Table Load Manager");
    return slabel;
}


bool CQualTableLoadManager::ShouldMakeMailReport()
{
    CAttribTableColumnIdPanel* pnl = x_GetColumnIdPanel();
    return pnl->ShouldMakeMailReport();
}


END_NCBI_SCOPE
