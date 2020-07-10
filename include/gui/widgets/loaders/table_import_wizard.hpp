#ifndef GUI_WIDGETS___LOADERS___TABLE_IMPORT_WIZARD__HPP
#define GUI_WIDGETS___LOADERS___TABLE_IMPORT_WIZARD__HPP

/*  $Id: table_import_wizard.hpp 28604 2013-08-06 16:10:27Z katargir $
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
* File Description:
*
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/wizard_page_impl.hpp>

#include <gui/widgets/loaders/table_import_data_source.hpp>
#include <gui/widgets/loaders/table_annot_data_source.hpp>

class wxPanel;
class wxWindow;

BEGIN_NCBI_SCOPE

class CTableFormatPanel;
class CTableDelimitersPanel;
class CTableFixedWidthPanel;
class CTableColumnIdPanel;
class CTableXformPanel;

class IExecuteUnit;

///////////////////////////////////////////////////////////////////////////////
/// CTableImportWizard
class NCBI_GUIWIDGETS_LOADERS_EXPORT CTableImportWizard
{
public:
    CTableImportWizard();

    IWizardPage*  GetFirstPage() { return &m_SelectFormatPage; }
    void          SetPrevPage(IWizardPage* prevPage) { m_SelectFormatPage.SetPrevPage(prevPage); }

    void          SetParentWindow(wxWindow* parent) { m_ParentWindow = parent; }
    void          InitUI();
    void          CleanUI();
    wxPanel*      GetCurrentPanel();

    IExecuteUnit* GetExecuteUnit(const vector<wxString>& filenames);

    bool          ValidateFilenames(const vector<wxString>& filenames);

    bool          IsCompletedState();
    bool          OnFirstPage();
    bool          OnLastPage();

    bool          GoNextPage();
    bool          GoPrevPage();

    void          SetRegistryPath(const string& path) { m_RegPath = path; }

protected:

private:
    void                    x_ConvertToSeqAnnot();
    void                    x_GuessColumns();

    CTableFormatPanel*      x_GetSelectFormatPanel();
    CTableDelimitersPanel*  x_GetDelimitersPanel();
    CTableFixedWidthPanel*  x_GetFixedWidthPanel();
    CTableColumnIdPanel*    x_GetColumnIdPanel();
    CTableXformPanel*       x_GetColumnXForm();


    CTableFormatPanel*      m_TableFormatPanel;
    CTableDelimitersPanel*  m_TableDelimitersPanel;
    CTableFixedWidthPanel*  m_TableFixedWidthPanel;
    CTableColumnIdPanel*    m_TableColumnIdPanel;
    CTableXformPanel*       m_TableColumnXFormPanel;

// Wizard navigation implementation

    class CSelectFormatPage : public CWizardPage
    {
    public:
        CSelectFormatPage(CTableImportWizard& manager) : m_Manager(manager) {}
        virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool forward);
        virtual IWizardPage* GetNextPage();
        CTableImportWizard& m_Manager;
    } m_SelectFormatPage;

    class CDelimitersPage : public CWizardPage
    {
    public:
        CDelimitersPage(CTableImportWizard& manager) : m_Manager(manager) {}
        virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool forward);
        virtual IWizardPage* GetPrevPage();
        virtual IWizardPage* GetNextPage();
        CTableImportWizard& m_Manager;
    } m_DelimitersPage;

    class CFixedWidthPage : public CWizardPage
    {
    public:
        CFixedWidthPage(CTableImportWizard& manager) : m_Manager(manager) {}
        virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool forward);
        virtual IWizardPage* GetPrevPage();
        virtual IWizardPage* GetNextPage();
        CTableImportWizard& m_Manager;
    } m_FixedWidthPage;

    class CColumnIdPage : public CWizardPage
    {
    public:
        CColumnIdPage(CTableImportWizard& manager) : m_Manager(manager) {}
        virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool forward);
        virtual IWizardPage* GetPrevPage();
        virtual IWizardPage* GetNextPage();
        CTableImportWizard& m_Manager;
    } m_ColumnIdPage;

    class CColumnXFormPage : public CWizardPage
    {
    public:
        CColumnXFormPage(CTableImportWizard& manager) : m_Manager(manager) {}
        virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool forward);
        virtual IWizardPage* GetPrevPage();
        virtual IWizardPage* GetNextPage();
        CTableImportWizard& m_Manager;
    } m_ColumnXForm;

    /// Data is loaded initially into this table for delimiter processing
    CRef<CTableImportDataSource> m_ImportedTableData;
    
    /// After delimiters and column types are identified, table is copied
    /// to this table for saving as asn and data (feature) transforms
    CRef<CTableAnnotDataSource> m_AnnotTableData;

    string       m_RegPath;
    wxWindow*    m_ParentWindow;
    IWizardPage* m_CurrentPage;
};

/* @} */

END_NCBI_SCOPE


#endif // GUI_WIDGETS___LOADERS___TABLE_IMPORT_WIZARD__HPP
