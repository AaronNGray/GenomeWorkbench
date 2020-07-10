#ifndef PKG_SEQUENCE___5COL_LOAD_MANAGER__HPP
#define PKG_SEQUENCE___5COL_LOAD_MANAGER__HPP

/*  $Id: 5col_load_manager.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/core/ui_file_load_manager.hpp>
#include <gui/core/project_selector_panel.hpp>

#include <gui/utils/extension.hpp>

#include <gui/widgets/loaders/file_load_panel_client.hpp>
#include <gui/widgets/loaders/tool_wizard.hpp>

#include <gui/widgets/wx/wizard_page_impl.hpp>

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// C5ColLoadManager
class  C5ColLoadManager :
    public CObject,
    public IFileFormatLoaderManager,
	public IFileLoadPanelClient,
	public IToolWizard,
    public IExtension,
    public IRegSettings
{
public:
    // support several different flavors of gff files.
    C5ColLoadManager();

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
    virtual IAppTask*      GetTask();
    virtual IExecuteUnit*  GetExecuteUnit();
    virtual IWizardPage*   GetFirstPage() { return 0; }
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

	static string Id() { return "file_loader_5col"; }
	static string Label() { return "5 Column"; }

protected:
    IServiceLocator* m_SrvLocator;
    string           m_RegPath;
    vector<wxString> m_FileNames;

    CUIObject m_Descr;
};

/* @} */

END_NCBI_SCOPE


#endif // PKG_SEQUENCE___5COL_LOAD_MANAGER__HPP
