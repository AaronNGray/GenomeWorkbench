#ifndef PKG_ALIGNMENT___TEXT_ALIGN_LOAD_MANAGER__HPP
#define PKG_ALIGNMENT___TEXT_ALIGN_LOAD_MANAGER__HPP

/*  $Id: text_align_load_manager.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include <gui/widgets/loaders/file_load_panel_client.hpp>
#include <gui/widgets/loaders/tool_wizard.hpp>

#include <gui/utils/extension.hpp>

#include <gui/widgets/loaders/text_align_load_params.hpp>

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_ALIGNMENT
*
* @{
*/

class CTextAlignParamsPanel;

///////////////////////////////////////////////////////////////////////////////
/// CTextAlignLoaderManager
class  CTextAlignLoaderManager :
    public CObject,
    public IFileFormatLoaderManager,
	public IFileLoadPanelClient,
	public IToolWizard,
    public IExtension,
    public IRegSettings
{
public:
    CTextAlignLoaderManager();

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
    virtual IWizardPage*   GetOptionsPage() { return &m_OptionsPage; }
    virtual void           SetPrevPage(IWizardPage* prevPage) { m_OptionsPage.SetPrevPage(prevPage); }
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
    virtual bool     CanGuessFormat() const { return false; }

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

	static string Id() { return "file_loader_text_align"; }
	static string Label() { return "Text Align"; }

protected:
    enum EState {
        eInvalid = -1,
        eParams,
        eCompleted
    };

protected:
    class CPage : public CWizardPage
    {
    public:
        CPage(CTextAlignLoaderManager& manager) : m_Manager(manager) {}
        virtual wxPanel* GetPanel();
        virtual bool CanLeavePage(bool) { return true; }
        CTextAlignLoaderManager& m_Manager;
    };

    CTextAlignParamsPanel* x_GetParamsPanel();

    CUIObject m_Descr;

    IServiceLocator* m_SrvLocator;
    wxWindow*   m_ParentWindow;
    EState      m_State;

    string  m_RegPath;

    CTextAlignParamsPanel* m_ParamsPanel;
    CTextAlignParams m_Params;
    vector<wxString> m_FileNames;

    CPage            m_OptionsPage;
};

/* @} */

END_NCBI_SCOPE


#endif // PKG_ALIGNMENT___TEXT_ALIGN_LOAD_MANAGER__HPP
