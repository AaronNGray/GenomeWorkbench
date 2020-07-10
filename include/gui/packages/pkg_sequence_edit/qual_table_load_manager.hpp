#ifndef PKG_SEQUENCE_EDIT___QUAL_TABLE_LOAD_MANAGER__HPP
#define PKG_SEQUENCE_EDIT___QUAL_TABLE_LOAD_MANAGER__HPP

/*  $Id: qual_table_load_manager.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/core/ui_file_load_manager.hpp>

#include <gui/widgets/loaders/file_load_panel_client.hpp>
#include <gui/widgets/loaders/tool_wizard.hpp>

#include <gui/widgets/loaders/table_import_data_source.hpp>
#include <gui/widgets/loaders/table_annot_data_source.hpp>

#include <gui/utils/extension.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <objmgr/seq_entry_handle.hpp>

BEGIN_NCBI_SCOPE

class CAttribTableDelimitersPanel;
class CAttribTableColumnIdPanel;

///////////////////////////////////////////////////////////////////////////////
/// CTableFileLoadManager
///
/// CQualTableLoadManager - a Load Manager for Sequin qualifier table files. This
/// class implements IFileFormatLoaderManager interface in order to work with 
/// generic Format LoadManager.
class  CQualTableLoadManager :
    public CObject,
    public IFileFormatLoaderManager,
	public IFileLoadPanelClient,
	public IToolWizard,
    public IExtension,
    public IRegSettings
{
public:
    CQualTableLoadManager();
    void SetTopLevelEntry(CSeq_entry_Handle seq_entry) { m_TopSeqEntry = seq_entry; }
    

    /// @name IFileLoadPanelClient interface implementation
    /// @{
	virtual string   GetLabel() const;
    virtual wxString GetFormatWildcard() const;
	virtual string   GetFileLoaderId() const { return Id(); }
    /// @}

    /// @name IUILoadManager interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* srv_locator);
    virtual void    SetParentWindow(wxWindow* parent);
    virtual const IUIObject&  GetDescriptor() const;
    virtual void    InitUI();
    virtual void    CleanUI();
    virtual wxPanel*    GetCurrentPanel();
    virtual bool    CanDo(EAction action);
    virtual bool    IsFinalState();
    virtual bool    IsCompletedState();
    virtual bool    DoTransition(EAction action);
    virtual IAppTask*     GetTask();
    virtual IExecuteUnit* GetExecuteUnit();
    virtual bool          SingleFileLoader() const { return true; }
    virtual IWizardPage*  GetFirstPage() { return &m_DelimitersPage; }
    virtual void          SetPrevPage(IWizardPage* prevPage) { m_DelimitersPage.SetPrevPage(prevPage); }
    /// @}

    /// @{ IFileFormatLoaderManager - additional members
    /// @{
    virtual wxString GetFormatWildcard();
    virtual bool     ValidateFilenames(const vector<wxString>& filenames);
    virtual void     SetFilenames(const vector<wxString>& filenames);
    virtual void     GetFilenames(vector<wxString>& filenames) const;
    virtual bool     IsInitialState();
    virtual bool     RecognizeFormat(const wxString& filename);
    virtual bool     RecognizeFormat(CFormatGuess::EFormat fmt);
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

	static string Id() { return "file_loader_qual_table"; }
	static string Label() { return "Qualifier Table"; }

    bool ShouldMakeMailReport();
    CAttribTableColumnIdPanel*    x_GetColumnIdPanel();

protected:
    CUIObject m_Descr;
    IServiceLocator* m_SrvLocator;
    wxWindow* m_ParentWindow;
    string m_RegPath;
    vector<wxString> m_FileNames;

    CAttribTableDelimitersPanel*  x_GetDelimitersPanel();

    void x_ConvertToSeqAnnot();

    /// Data is loaded initially into this table for delimiter processing
    CRef<CTableImportDataSource> m_ImportedTableData;
    
    /// After delimiters and column types are identified, table is copied
    /// to this table for saving as asn and data (feature) transforms
    CRef<CTableAnnotDataSource> m_AnnotTableData;

    CAttribTableDelimitersPanel*  m_TableDelimitersPanel;
    CAttribTableColumnIdPanel*    m_TableColumnIdPanel;

    CSeq_entry_Handle m_TopSeqEntry;

    // Wizard navigation implementation

    class CDelimitersPage : public CWizardPage
    {
    public:
        CDelimitersPage(CQualTableLoadManager& manager) : m_Manager(manager) {}
        virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool forward);
        virtual IWizardPage* GetNextPage();
        CQualTableLoadManager& m_Manager;
    } m_DelimitersPage;


    class CColumnIdPage : public CWizardPage
    {
    public:
        CColumnIdPage(CQualTableLoadManager& manager) : m_Manager(manager) {}
        virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool forward);
        virtual IWizardPage* GetPrevPage();
        virtual IWizardPage* GetNextPage();
        CQualTableLoadManager& m_Manager;
    } m_ColumnIdPage;


    IWizardPage* m_CurrentPage;
};

END_NCBI_SCOPE


#endif  // PKG_SEQUENCE_EDIT___QUAL_TABLE_LOAD_MANAGER__HPP
