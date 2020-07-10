#ifndef GUI_GBENCH_NEW___GUI__HPP
#define GUI_GBENCH_NEW___GUI__HPP

/*  $Id: gui.hpp 43956 2019-09-25 21:02:14Z katargir $
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
 * Authors: Andrey Yazhuk
 *
 * File Description:
 *
 */

#include "commands.hpp"

#include <gui/framework/workbench.hpp>
#include <gui/framework/menu_service.hpp>
#include <gui/framework/tool_bar_service.hpp>
#include <gui/framework/ui_data_source.hpp>

#include <gui/widgets/wx/window_manager.hpp>

#include <gui/objutils/objects.hpp>

#include <objects/general/User_object.hpp>

#include <wx/event.h>
#include <wx/aui/auibar.h>


#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

BEGIN_NCBI_SCOPE

class CDockLayoutTree;
class CPkgManager;
class CAppTaskServiceSlot;
class CWorkbench;

class IGBenchGUIApp
{
public:
    virtual ~IGBenchGUIApp() {}

    virtual CPkgManager* GetPackageManager() = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// CGBenchGUI

class CGBenchGUI :
    public wxEvtHandler,
    public IWorkbenchAdvisor,
    public IWindowManagerAdvisor,
    public IMenuContributor,
    public IToolBarContext
{
    DECLARE_EVENT_TABLE();
public:
    friend class CGBenchGUIDropTarget;

    CGBenchGUI();
    virtual ~CGBenchGUI();

    void    SetGBenchGUIApp (IGBenchGUIApp* app) { m_GBenchGUIApp = app; }

    void SetRegistryPath(const string& path);
    void SetWorkbench(CWorkbench* workbench);

    /// @name IWorkbenchAdvisor implementation
    /// @{
    virtual string  GetAppTitle();
    virtual void    PostCreateWindow();
    virtual void    PostInit();
    virtual void    PreShutDown();
    virtual void    PreDestroyWindow();
    virtual bool    CanShutDown();
    /// @}

    /// @name IMenuContributor implementation
    /// @{
    /// we implement this interface in order to be able to add commands to the menu
    //virtual void    SetMenuService(IMenuService* service);
    virtual wxMenu* GetMenu();
    //virtual void    UpdateMenuBar(wxMenu& menu);
    /// @}

    /// @name IToolBarContext implementation
    /// @{
    virtual void    GetCompatibleToolBars(vector<string>& names);
    /// @}

    void RestoreWindowLayout();

    /// @name Command handlers
    /// @{
    void OnOpenView(wxCommandEvent& event);
    void OnOpenViewDefault(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnRunTool(wxCommandEvent& event);
    void OnQuickLaunch(wxCommandEvent& event);
    void OnRecentTool(wxCommandEvent& event);

    void OnImportClipboard(wxCommandEvent& event);

    void OnCloseWorkspace(wxCommandEvent& event);
    void OnHasWorskpaceUpdate(wxUpdateUIEvent& event);

    void OnShowProjectTreeView(wxCommandEvent& event);
    void OnShowSelectionInspector(wxCommandEvent& event);
    void OnShowDiagnosticsConsole(wxCommandEvent& event);
    void OnShowEventView(wxCommandEvent& event);
    void OnShowTaskView(wxCommandEvent& event);
    void OnShowSearchView(wxCommandEvent& event);

    void OnEnableCmdUpdate(wxUpdateUIEvent& event);
    void OnDisableCmdUpdate(wxUpdateUIEvent& event);

    void OnEnableCmdOpen(wxUpdateUIEvent& event);
    void OnEnablePrintCmdUpdate(wxUpdateUIEvent& event);
    void OnEnableSaveImagesCmdUpdate(wxUpdateUIEvent& event);
    void OnEnableSavePdfCmdUpdate(wxUpdateUIEvent& event);
    void OnEnableSaveSvgCmdUpdate(wxUpdateUIEvent& event);

    void OnBroadcastOptions(wxCommandEvent& event);
    void OnWindowMaskerFiles(wxCommandEvent& event);
    void OnToolsOptions(wxCommandEvent& event);
    void OnToolsPackages(wxCommandEvent& event);

    void OnCloseAllProjectViews(wxCommandEvent& event);

    void OnSaveLayout(wxCommandEvent& event);
    void OnSaveLayoutUpdate(wxUpdateUIEvent& event);
    void OnLoadLayout(wxCommandEvent& event);
    void OnResetLayout(wxCommandEvent& event);
    void OnResetLayoutUpdate(wxUpdateUIEvent& event);

    void OnLaunchWebHelp(wxCommandEvent& event);
    void OnSendFeedback(wxCommandEvent& event);
    void OnCheckForUpdates(wxCommandEvent& event);
    void OnShowPagerMessage(wxCommandEvent& event);
    void OnHelpAbout(wxCommandEvent& event);

    void OnTestLoadBlastMask(wxCommandEvent& event);
    void OnMacroEditor(wxCommandEvent& event);

    void OnExport(wxCommandEvent& event);
    void OnUpdateExport(wxUpdateUIEvent& event);

    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnSaveUpdate(wxUpdateUIEvent& event);

    void OnDebugAsyncCall(wxCommandEvent& event);
    void OnLogTestException(wxCommandEvent& event);

    void OnEnableEditingPackage(wxCommandEvent& event);
    void OnEnableEditingPackageUpdate(wxUpdateUIEvent& event);
    /// @}

    void OnDebugTimer(wxTimerEvent&);

    /// @name IWindowManagerAdvisor implementation
    /// @{
    virtual void    OnCloseClientsRequest(const TClients& clients);
    virtual void    OnClientAboutToClose(IWMClient& client);
    virtual void    OnClientClosed(IWMClient& client);
    virtual void    OnActiveClientChanged(IWMClient* new_active);
    virtual string  GetFloatingFrameTitle(int index);
    /// @}

    void    ShowOpenDlg(vector<wxString>& filenames);

    void    ActivateClient(IWMClient* client);

    void    SaveDefaultWindowLayout();

protected:
    void    x_RegisterUserTypes();
    void    x_RegisterCommands();
    void    x_RegisterAccelerators();

    void x_RegisterPackageContributions();

    void    x_SetupStatusBar();
    void    x_ClearStatusBar();

    void    x_CreateViewToolBar();
    void    x_CreateFloatingToolBars();

    void    x_PushEvtHandler(wxEvtHandler* new_handler);
    wxEvtHandler*   x_PopEvtHandler();

    void    x_AddSystemView(const string& view_ui_name);
    void    x_ShowSingletonView(const string& view_ui_name);
    void    x_CloseAllViews();

    wxWindow*   x_GetMainWindow();

    void    x_UpdateQuickLaunchMenu(wxMenu& root_menu);

protected:
    string    m_RegPath;

    CWorkbench*     m_Workbench;
    IMenuService*   m_MenuService;
    IGBenchGUIApp*  m_GBenchGUIApp;

    CAppTaskServiceSlot* m_AppSrvSlot;

#ifdef _DEBUG
    wxTimer m_DebugTimer;
#endif
};


///////////////////////////////////////////////////////////////////////////////
/// CGBenchToolBarFactory
class CGBenchToolBarFactory :
    public IToolBarContributor
{
public:
    void    RegisterImageAliases(wxFileArtProvider& provider);

    /// @name IToolBarContributor implementation
    /// @{
    virtual void        GetToolBarNames(vector<string>& names);
    virtual wxAuiToolBar*  CreateToolBar(const string& name, wxWindow* parent);
    /// @}
};


END_NCBI_SCOPE


#endif
// GUI_GBENCH_NEW___GUI__HPP
