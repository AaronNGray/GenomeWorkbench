#ifndef PKG_ALIGNMENT___LBLAST_UI_DATA_SOURCE__HPP
#define PKG_ALIGNMENT___LBLAST_UI_DATA_SOURCE__HPP

/*  $Id: lblast_ui_data_source.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>

#include <gui/framework/ui_data_source.hpp>
#include <gui/framework/service.hpp>

#include <gui/core/ui_tool_manager.hpp>
#include <gui/core/pt_item.hpp>
#include <gui/core/project_selector_panel.hpp>

#include <gui/utils/extension.hpp>
#include <gui/utils/ui_object.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <gui/widgets/loaders/lblast_load_params.hpp>

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_NCBI_INTERNAL
*
* @{
*/

/// commands introduced by BAM data source
enum ELBLASTCommands  {
    eCmdLoadDB = 12351
};

class CLBLASTUIDataSourceType;
class CLBLASTParamsPanel;

///////////////////////////////////////////////////////////////////////////////
/// CLBLASTUIDataSource

class  CLBLASTUIDataSource :
    public CObject,
    public IExtension,
    public IServiceLocatorConsumer,
    public IUIDataSource,
    public IDataLoaderProvider,
    public IExplorerItemCmdContributor
{
public:
    CLBLASTUIDataSource(CLBLASTUIDataSourceType& type);
    ~CLBLASTUIDataSource();

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IServiceLocatorConsumer implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* locator);
    /// @}

    /// @name IUIDataSource interface implementation
    /// @{
    virtual IUIDataSourceType&  GetType() const;

    virtual const IUIObject&    GetDescr();

    virtual bool    IsOpen();
    virtual bool    Open();
    virtual bool    Close();

    virtual void    EditProperties();

    virtual IUIToolManager*    GetLoadManager();

    virtual int GetDefaultCommand();
    virtual wxEvtHandler* CreateEvtHandler();
    /// @}

    /// @name IDataLoaderProvider implementation
    /// @{
    virtual string AddDataLoader(const objects::CUser_object& obj);

    virtual string GetLoaderName(const objects::CUser_object& obj) const;
    /// @}

    /// @name IExplorerItemCmdContributor implementation
    /// @{
    virtual IExplorerItemCmdContributor::TContribution GetMenu(wxTreeCtrl& treeCtrl, PT::TItems& items);
    /// @}

protected:
    mutable CRef<CLBLASTUIDataSourceType>  m_Type;

    CUIObject   m_Descr;

    IServiceLocator*    m_SrvLocator;

    bool    m_Open;
};


///////////////////////////////////////////////////////////////////////////////
/// CLBLASTUIDataSourceType

class  CLBLASTUIDataSourceType :
    public CObject,
    public IUIDataSourceType,
    public IExtension
{
public:
    CLBLASTUIDataSourceType();

    /// @name IUIDataSourceType interface implementation
    /// @{
    virtual const IUIObject&    GetDescr();
    virtual IUIDataSource*  CreateDataSource();
    virtual bool    AutoCreateDefaultDataSource();
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

protected:
    CUIObject   m_Descr;
};


///////////////////////////////////////////////////////////////////////////////
/// CLBLASTUILoadManager

class  CLBLASTUILoadManager :
    public CObject,
    public IUIToolManager,
    public IRegSettings
{
public:
    CLBLASTUILoadManager();

    /// @name IUIToolManager interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* srv_locator);
    virtual void    SetParentWindow(wxWindow* parent);
    virtual const IUIObject&  GetDescriptor() const;
    virtual void    InitUI();
    virtual void    CleanUI();
    virtual wxPanel*    GetCurrentPanel();
    virtual bool        CanDo(EAction action);
    virtual bool        IsFinalState();
    virtual bool        IsCompletedState();
    virtual bool    DoTransition(EAction action);
    virtual IAppTask*      GetTask();
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    enum EState {
        eInvalid = -1,
        eSelectDir,
        eSelectProject,
        eCompleted
    };

protected:
    IServiceLocator* m_SrvLocator;
    wxWindow*        m_ParentWindow;

    CUIObject        m_Descriptor;
    EState           m_State;

    string           m_RegPath;
    mutable string   m_SavedInput;   // input saved in the registry
    string           m_AssmAcc;
    vector<CRef<objects::CSeq_id> >     m_RefSeqIds;

    CLBLASTParamsPanel*      m_ParamsPanel;
    CProjectSelectorPanel*   m_ProjectSelPanel;
    SProjectSelectorParams   m_ProjectParams; //TODO

    CLBLASTLoadParams        m_LBLASTParams;
};


/* @} */

END_NCBI_SCOPE;

#endif  // PKG_ALIGNMENT___LBLAST_UI_DATA_SOURCE__HPP
