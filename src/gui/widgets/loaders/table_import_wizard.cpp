/*  $Id: table_import_wizard.cpp 43757 2019-08-28 16:52:27Z katargir $
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
 * Authors:  Roman Katargin
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/async_call.hpp>

#include <gui/widgets/loaders/table_import_wizard.hpp>

#include "table_format_panel.hpp"
#include "table_delimeters_panel.hpp"
#include "table_fixed_width_panel.hpp"
#include "table_column_id_panel.hpp"
#include "table_xform_panel.hpp"

#include <gui/widgets/loaders/table_column_type_guesser.hpp>
#include <gui/widgets/loaders/table_object_loader.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CTableImportWizard::CTableImportWizard() :
      m_TableFormatPanel()
    , m_TableDelimitersPanel()
    , m_TableFixedWidthPanel()
    , m_TableColumnIdPanel()
    , m_TableColumnXFormPanel()
    , m_SelectFormatPage(*this)
    , m_DelimitersPage(*this)
    , m_FixedWidthPage(*this)
    , m_ColumnIdPage(*this)
    , m_ColumnXForm(*this)
    , m_ImportedTableData(new CTableImportDataSource())
    , m_AnnotTableData(new CTableAnnotDataSource())
    , m_ParentWindow(NULL)
    , m_CurrentPage(&m_SelectFormatPage)
{
}

void CTableImportWizard::InitUI()
{
    m_CurrentPage = &m_SelectFormatPage;
}

void CTableImportWizard::CleanUI()
{
    m_TableFormatPanel = NULL;
    m_TableDelimitersPanel = NULL;
    m_TableFixedWidthPanel = NULL;
    m_TableColumnIdPanel = NULL;
    m_TableColumnXFormPanel = NULL;
    m_CurrentPage = &m_SelectFormatPage;
}

bool CTableImportWizard::OnFirstPage()
{
    return (m_CurrentPage == &m_SelectFormatPage);
}

bool CTableImportWizard::OnLastPage()
{
    return (m_CurrentPage->GetNextPage() == 0);
}

bool CTableImportWizard::IsCompletedState()
{
    return (m_CurrentPage == 0);
}

wxPanel* CTableImportWizard::GetCurrentPanel()
{
    return m_CurrentPage ? m_CurrentPage->GetPanel() : (wxPanel*)0;
}

bool CTableImportWizard::ValidateFilenames(const vector<wxString>& filenames)
{
    if (filenames.empty())
        return false;

    CTableFormatPanel* formatPanel = x_GetSelectFormatPanel();
    formatPanel->m_PreviewFname = filenames[0];

    return formatPanel->PreviewData();
}

IExecuteUnit* CTableImportWizard::GetExecuteUnit(const vector<wxString>& filenames)
{
    return new CTableObjectLoader(*m_AnnotTableData, filenames);
}

bool CTableImportWizard::GoNextPage()
{
    if (m_CurrentPage == 0)
        return false;

    if (!m_CurrentPage->CanLeavePage(true))
        return false;

    m_CurrentPage = m_CurrentPage->GetNextPage();
    return true;
}

bool CTableImportWizard::GoPrevPage()
{
    if (m_CurrentPage == 0) {
        m_CurrentPage = &m_ColumnXForm;
    } else {
        m_CurrentPage = m_CurrentPage->GetPrevPage();
    }
    return true;
}

static const string kSelectFormatTag = ".SelectFormatPanel";
static const string kDelimitersTag = ".DelimitersPanel";
static const string kFixedWidthTag = ".FixedWidthPanel";
static const string kColumnIdTag = ".ColumnIdPanel";
static const string kColumnXformTag = ".ColumnXformPanel";

CTableFormatPanel* CTableImportWizard::x_GetSelectFormatPanel()
{
    if (m_TableFormatPanel == NULL) {
        m_TableFormatPanel = new CTableFormatPanel(m_ParentWindow);        
     
        if ( !m_RegPath.empty() ) {
            m_TableFormatPanel->SetRegistryPath(m_RegPath + kSelectFormatTag);
            m_TableFormatPanel->LoadSettings();
        }
    }
    m_TableFormatPanel->SetImportDataSource(m_ImportedTableData);

    return m_TableFormatPanel;
}

CTableDelimitersPanel* CTableImportWizard::x_GetDelimitersPanel()
{
    if (m_TableDelimitersPanel == NULL) {
        m_TableDelimitersPanel = new CTableDelimitersPanel(m_ParentWindow);        
     
        if ( !m_RegPath.empty() ) {
            m_TableDelimitersPanel->SetRegistryPath(m_RegPath + kDelimitersTag);
            m_TableDelimitersPanel->LoadSettings();
        }
    }
    m_TableDelimitersPanel->SetImportDataSource(m_ImportedTableData);

    return m_TableDelimitersPanel;
}

CTableFixedWidthPanel* CTableImportWizard::x_GetFixedWidthPanel()
{
    if (m_TableFixedWidthPanel == NULL) {
        m_TableFixedWidthPanel = new CTableFixedWidthPanel(m_ParentWindow);        
     
        if ( !m_RegPath.empty() ) {
            m_TableFixedWidthPanel->SetRegistryPath(m_RegPath + kFixedWidthTag);
            m_TableFixedWidthPanel->LoadSettings();
        }
    }
    m_TableFixedWidthPanel->SetImportDataSource(m_ImportedTableData);

    return m_TableFixedWidthPanel;
}

CTableColumnIdPanel* CTableImportWizard::x_GetColumnIdPanel()
{
    if (m_TableColumnIdPanel == NULL) {
        m_TableColumnIdPanel = new CTableColumnIdPanel(m_ParentWindow);        
     
        if ( !m_RegPath.empty() ) {
            m_TableColumnIdPanel->SetRegistryPath(m_RegPath + kColumnIdTag);
            m_TableColumnIdPanel->LoadSettings();
        }
    }

    m_TableColumnIdPanel->SetImportDataSource(m_ImportedTableData);

    return m_TableColumnIdPanel;
}

CTableXformPanel* CTableImportWizard::x_GetColumnXForm()
{
    if (m_TableColumnXFormPanel == NULL) {
        m_TableColumnXFormPanel = new CTableXformPanel(m_ParentWindow);
     
        if ( !m_RegPath.empty() ) {
            m_TableColumnXFormPanel->SetRegistryPath(m_RegPath + kColumnXformTag);
            m_TableColumnXFormPanel->LoadSettings();
        }
    }
    m_TableColumnXFormPanel->SetImportDataSource(m_ImportedTableData);
    m_TableColumnXFormPanel->SetAnnotDataSource(m_AnnotTableData);

    return m_TableColumnXFormPanel;
}

void CTableImportWizard::x_ConvertToSeqAnnot()
{
    // Now convert the table to a seq-annot - could be slow if there is a lot
    // of data so use async converter to give user a message as to what's
    // going on.   
    m_AnnotTableData.Reset(new CTableAnnotDataSource()),   

    ERR_POST(Info << "Import Table - Converting SeqTable to SeqAnnot");                

    GUI_AsyncExec([this](ICanceled&)
    { 
        m_ImportedTableData->ConvertToSeqAnnot(m_AnnotTableData->GetContainer());
    }, wxT("Preparing Table Data..."));
}

void CTableImportWizard::x_GuessColumns()
{
    GUI_AsyncExec([this](ICanceled&)
    {
        CTableColumnTypeGuesser  guesser(m_ImportedTableData);
        guesser.GuessColumns();
    }, wxT("Analyzing column data..."));

    ERR_POST(Info << "Table Import - Guessed Columns: ");
    m_ImportedTableData->LogColumnInfo();
}

//
// CSelectFormatPage navigation
//

wxPanel* CTableImportWizard::CSelectFormatPage::GetPanel()
{
    return m_Manager.x_GetSelectFormatPanel();
}

bool CTableImportWizard::CSelectFormatPage::CanLeavePage(bool forward)
{
    CTableFormatPanel* panel = m_Manager.x_GetSelectFormatPanel();
    if (forward && !panel->IsInputValid())
        return false;

    panel->SaveSettings();
    return true;
}

IWizardPage* CTableImportWizard::CSelectFormatPage::GetNextPage()
{
    CTableFormatPanel* panel = m_Manager.x_GetSelectFormatPanel();
    if (panel->IsDelimitedFile())
        return &m_Manager.m_DelimitersPage;
    else
        return &m_Manager.m_FixedWidthPage;
}

//
// CDelimitersPage navigation
//

wxPanel* CTableImportWizard::CDelimitersPage::GetPanel()
{
    return m_Manager.x_GetDelimitersPanel();
}

bool CTableImportWizard::CDelimitersPage::CanLeavePage(bool forward)
{
    CTableDelimitersPanel* panel = m_Manager.x_GetDelimitersPanel();
    if (forward) {
        if (!panel->IsInputValid())
            return false;

        panel->SaveSettings();
        m_Manager.x_GuessColumns();
        return true;
    }

    panel->SaveSettings();
    return true;
}

IWizardPage* CTableImportWizard::CDelimitersPage::GetPrevPage()
{
    return &m_Manager.m_SelectFormatPage;
}

IWizardPage* CTableImportWizard::CDelimitersPage::GetNextPage()
{
    m_Manager.m_ColumnIdPage.SetPrevPage(this);
    return &m_Manager.m_ColumnIdPage;
}

//
// CFixedWidthPage navigation
//

wxPanel* CTableImportWizard::CFixedWidthPage::GetPanel()
{
    return m_Manager.x_GetFixedWidthPanel();
}

bool CTableImportWizard::CFixedWidthPage::CanLeavePage(bool forward)
{
    CTableFixedWidthPanel* panel = m_Manager.x_GetFixedWidthPanel();
    if (forward) {
        if (!panel->IsInputValid())
            return false;

        panel->SaveSettings();
        m_Manager.x_GuessColumns();
        return true;
    }

    panel->SaveSettings();
    return true;
}

IWizardPage* CTableImportWizard::CFixedWidthPage::GetPrevPage()
{
    return &m_Manager.m_SelectFormatPage;
}

IWizardPage* CTableImportWizard::CFixedWidthPage::GetNextPage()
{
    m_Manager.m_ColumnIdPage.SetPrevPage(this);
    return &m_Manager.m_ColumnIdPage;
}

//
// CColumnIdPage navigation
//

wxPanel* CTableImportWizard::CColumnIdPage::GetPanel()
{
    return m_Manager.x_GetColumnIdPanel();
}

bool CTableImportWizard::CColumnIdPage::CanLeavePage(bool forward)
{
    CTableColumnIdPanel* panel = m_Manager.x_GetColumnIdPanel();
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

IWizardPage* CTableImportWizard::CColumnIdPage::GetPrevPage()
{
    return CWizardPage::GetPrevPage();
}

IWizardPage* CTableImportWizard::CColumnIdPage::GetNextPage()
{
    return &m_Manager.m_ColumnXForm;
}

//
// CColumnXFormPage navigation
//

wxPanel* CTableImportWizard::CColumnXFormPage::GetPanel()
{
    return m_Manager.x_GetColumnXForm();
}

bool CTableImportWizard::CColumnXFormPage::CanLeavePage(bool forward)
{
    CTableXformPanel* panel = m_Manager.x_GetColumnXForm();
    if (forward && !panel->IsInputValid())
        return false;

    panel->SaveSettings();
    return true;
}

IWizardPage* CTableImportWizard::CColumnXFormPage::GetPrevPage()
{
    return &m_Manager.m_ColumnIdPage;
}

IWizardPage* CTableImportWizard::CColumnXFormPage::GetNextPage()
{
    return 0;
}

END_NCBI_SCOPE
