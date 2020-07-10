#ifndef PKG_SEQUENCE___GB_UI_DATA_SOURCE__HPP
#define PKG_SEQUENCE___GB_UI_DATA_SOURCE__HPP

/*  $Id: gb_ui_data_source.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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
 * Authors:  Andrey Yazhuk
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

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */
BEGIN_SCOPE(objects)
    class CObjectManager;
END_SCOPE(objects)

/// commands introduced by Net BLAST data source
enum EGenBankCommands  {
    eCmdLoadFromGenBank = 12250,
};

class CGenBankUIDataSourceType;

///////////////////////////////////////////////////////////////////////////////
/// CGenBankUIDataSource

class  CGenBankUIDataSource :
    public CObject,
    public IExtension,
    public IServiceLocatorConsumer,
    public IUIDataSource,
    public IExplorerItemCmdContributor
{
public:
    CGenBankUIDataSource(CGenBankUIDataSourceType& type);
    ~CGenBankUIDataSource();

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

    /// @name IExplorerItemCmdContributor implementation
    /// @{
    virtual IExplorerItemCmdContributor::TContribution GetMenu(wxTreeCtrl& treeCtrl, PT::TItems& items);
    /// @}

protected:
    mutable CRef<CGenBankUIDataSourceType>  m_Type;
    CRef<objects::CObjectManager> m_ObjMgr;

    CUIObject   m_Descr;

    IServiceLocator*    m_SrvLocator;

    bool    m_Open;
};


///////////////////////////////////////////////////////////////////////////////
/// CGenBankUIDataSourceType

class  CGenBankUIDataSourceType :
    public CObject,
    public IUIDataSourceType,
    public IExtension
{
public:
    CGenBankUIDataSourceType();

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

class CGenBankLoadOptionPanel;

///////////////////////////////////////////////////////////////////////////////
/// CGenBankUILoadManager

class  CGenBankUILoadManager :
    public CObject,
    public IUIToolManager,
    public IRegSettings
{
public:
    CGenBankUILoadManager();

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

    void SaveMruAccessions();

protected:
    enum EState {
        eInvalid = -1,
        eSelectAcc,
        eSelectProject,
        eCompleted
    };

protected:
    IServiceLocator* m_SrvLocator;
    wxWindow*   m_ParentWindow;

    CUIObject m_Descriptor;
    EState  m_State;

    string  m_RegPath;

    CGenBankLoadOptionPanel* m_OptionPanel;
    CProjectSelectorPanel*   m_ProjectSelPanel;
    SProjectSelectorParams   m_ProjectParams; //TODO
};

END_NCBI_SCOPE;

#endif  // PKG_SEQUENCE___GB_UI_DATA_SOURCE__HPP
