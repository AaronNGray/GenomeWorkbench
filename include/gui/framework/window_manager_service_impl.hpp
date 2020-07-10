#ifndef GUI_FRAMEWORK___WINDOW_MANAGER_SERVICE_IMPL__HPP
#define GUI_FRAMEWORK___WINDOW_MANAGER_SERVICE_IMPL__HPP

/*  $Id: window_manager_service_impl.hpp 33163 2015-06-05 16:45:34Z katargir $
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
 *  CWindowManagerService - standard implementation of IWindowManagerService.
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/framework/window_manager_service.hpp>
#include <gui/framework/tool_bar_service.hpp>
#include <gui/framework/service.hpp>
#include <gui/framework/menu_service.hpp>

#include <gui/widgets/wx/window_manager.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <gui/objutils/reg_settings.hpp>

class wxFrame;
class wxFileArtProvider;
class wxCommandEvent;

#include <set>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CWindowManagerService - standard implementation of IWindowManagerService
/// interface built on CWindowManager class.
/// CWindowManagerService also implements IToolBarService.
///
/// CWindowManagerService implements IMenuContributor in order to be compatible
/// with IMenuService. It also implements IWindowManagerMenuListener to allow
/// CWindowManager to change menu dynamically when needed.
class  NCBI_GUIFRAMEWORK_EXPORT CWindowManagerService :
    public CObject,
    public IWindowManagerService,
    public IToolBarService,
    public IService,
    public IWindowManagerMenuListener,
    public IMenuContributor,
    public IRegSettings
{
public:
    CWindowManagerService();
    virtual ~CWindowManagerService();

    virtual void    SetFrame(wxFrame* frame);

    /// @name IService implementation
    /// @{
    virtual void    InitService();
    virtual void    ShutDownService();
    /// @}

    /// @name IWindowManagerService implementation
    /// @
    virtual CWindowManager* GetWindowManager() { return m_WindowManager; }

    virtual void    SetAdvisor(IWindowManagerAdvisor* advisor);

    virtual void    AddClient(IWMClient& client, bool bFloat);
    virtual wxFrame* MoveToFloatingFrame(IWMClient& client);

    virtual void    CloseClient(IWMClient& client);
    virtual void    CloseClients(const TClients& clients);
    virtual void    CloseAllClients();

    virtual void    GetAllClients(TClients& clients);
    virtual IWMClient*  GetActiveClient();

    virtual void    ActivateClient(IWMClient& client);
    virtual void    ActivateClients(TClients& clients);

    virtual void    RegisterActiveClient(IWMClient& client);
    virtual void    UnRegisterActiveClient(IWMClient& client);

    virtual void    RaiseFloatingWindowsInZOrder();
    virtual void    RaiseFloatingWindowsInZOrder(wxRect r);

    virtual bool    IsDragging() const;

    virtual void    RefreshClient(IWMClient& client);

    virtual void    ApplyLayout(const objects::CUser_object& layout);
    virtual objects::CUser_object*  SaveLayout();
    virtual void    LoadLayout(const objects::CUser_object& layout,
                               IWMClientFactory& factory);
    /// @}

    /// @name IToolBarService implementation
    /// @{
    virtual void    AddToolBarFactory(IToolBarContributor* factory);
    virtual void    DeleteToolBarFactory(IToolBarContributor* factory);
    virtual void    AddToolBarContext(IToolBarContext* context);
    virtual void    RemoveToolBarContext(IToolBarContext* context);
    /// @}

    /// @name IWindowManagerMenuListener implementation
    /// @{
    virtual void    UpdateMenuBar();
    /// @}

    /// @name IMenuContributor
    /// @{
    virtual const wxMenu* GetMenu();
    //virtual void    UpdateMenuBar(wxMenu& menu);
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    virtual void    SetMenuService(IMenuService* service);

    bool    OnCommandEvent(wxCommandEvent& event);

protected:
    virtual void    x_ShowToolBar(const string& name, bool show);
    virtual void    x_ShutDownToolBars();

protected:
    typedef map<string, int> TTBNameToCount;
    typedef vector<IToolBarContributor*>    TToolBarFactories;
    typedef set<IToolBarContext*>   TToolBarContexts;

    string m_RegPath;

    CWindowManager* m_WindowManager;

    wxFrame*    m_Frame;

    // registered toolbar factories
    TToolBarFactories   m_ToolBarFactories;

    /// holds registered toolbar contexts
    TToolBarContexts    m_ToolBarContexts;

    /// holds a map of toolbar names declared by registered contexts
    /// toolbar name -> context count
    TTBNameToCount  m_TBNameToCount;

    IMenuService* m_MenuService;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___WINDOW_MANAGER_SERVICE_IMPL__HPP

