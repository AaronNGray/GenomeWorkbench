#ifndef GUI_FRAMEWORK_DEMO___GUI__HPP
#define GUI_FRAMEWORK_DEMO___GUI__HPP

/*  $Id: gui.hpp 34721 2016-02-04 14:05:23Z katargir $
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

#include <gui/framework/workbench.hpp>
#include <gui/framework/menu_service.hpp>
#include <gui/framework/tool_bar_service.hpp>
#include <gui/framework/ui_data_source.hpp>

#include <gui/widgets/wx/window_manager.hpp>

#include <objects/general/User_object.hpp>

#include <wx/event.h>


#define ID_CWXMAINFRAME 10000

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

BEGIN_NCBI_SCOPE

/// Application commands
enum EDemoFrameworkCommands {
    eCmdCreateClockView = 15000,
    eCmdCreateOpenGLView,
    eCmdCreateTaskView,
    eCmdCreateEventView,
    eCmdCreateDiagConsole,
    /*eCmdCreateTextCtrl,
    eCmdCreateHtmlView,*/
    eCmdCloseView,
    eCmdCloseAllViews,
    eCmdSaveLayout,
    eCmdRestoreLayout,
    eCmdTestDiagPanel,
    eCmdTestStatusProgress
};

///////////////////////////////////////////////////////////////////////////////
/// CFrameworkDemoFrame
class CFrameworkDemoGUI :
    public wxEvtHandler,
    public IWorkbenchAdvisor,
    public IWindowManagerAdvisor,
    public IMenuContributor
{
    DECLARE_EVENT_TABLE();
public:
    CFrameworkDemoGUI();

    virtual void SetRegistryPath(const string& path);

    /// @name IWorkbenchAdvisor implementation
    /// @{
    virtual void    SetWorkbench(IWorkbench* workbench);
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
    virtual const wxMenu* GetMenu();
    //virtual void    UpdateMenuBar(wxMenu& menu);
    /// @}

    virtual void    RestoreWindowLayout();

    /// @name Command handlers
    /// @{
     void OnCloseWorkspace(wxCommandEvent& event);

    void OnCreateClockViewClick(wxCommandEvent& event);
    void OnCreateOpenglViewClick(wxCommandEvent& event);
    void OnShowDiagnosticsConsoleClick(wxCommandEvent& event);
    void OnCreateEventViewClick(wxCommandEvent& event);
    void OnCreateTaskViewClick(wxCommandEvent& event);
    //void OnCreateTextCtrlClick(wxCommandEvent& event);
    //void OnCreateHTMLViewClick(wxCommandEvent& event);
    //void OnCreateTreeControlClick(wxCommandEvent& event);

    void OnTestDiagPanelClick(wxCommandEvent& event);
    void OnTestDiagPanelUpdate( wxUpdateUIEvent& event );

    void OnCloseViewClick(wxCommandEvent& event);
    void OnCloseAllViewsClick(wxCommandEvent& event);

    void OnSaveLayoutClick(wxCommandEvent& event);
    void OnSaveLayoutUpdate(wxUpdateUIEvent& event);
    void OnRestoreLayoutClick(wxCommandEvent& event);
    void OnRestoreLayoutUpdate(wxUpdateUIEvent& event);

    void OnTestStatusProgress(wxCommandEvent& event);
    void OnTestStatusProgressUpdate(wxUpdateUIEvent& event);

    void    OnDiagnosticsTimer(wxTimerEvent& event);
    void    OnStatusTimer(wxTimerEvent& event);
    /// @}

    /// @name IWindowManagerAdvisor implementation
    /// @{
    virtual void    OnCloseClientsRequest(const TClients& clients);
    virtual void    OnClientAboutToClose(IWMClient& client);
    virtual void    OnClientClosed(IWMClient& client);
    virtual void    OnActiveClientChanged(IWMClient* new_active);
    virtual string  GetFloatingFrameTitle(int index);
    /// @}

    void    ActivateClient(IWMClient* client);

    bool GetDiagnosticsTesting() const { return m_DiagnosticsTesting ; }
    void SetDiagnosticsTesting(bool value) { m_DiagnosticsTesting = value ; }

protected:
    void    x_RegisterImageAliases();
    void    x_RegisterCommands();
    void    x_CreateViewToolBar();
    void    x_CreateFloatingToolBars();
    void    x_RegisterAccelerators();
    void    x_CreateDefaultViews();

    void    x_AddClient(IWMClient* client); //TODO reties
    void    x_AddView(const string& view_ui_name);
    void    x_ShowSingletonView(const string& view_ui_name);
    void    x_CloseAllViews();

    wxWindow*   x_GetMainWindow();

protected:
    string    m_RegPath;

    IWorkbench*     m_Workbench;

    wxTimer m_Timer;
    bool m_DiagnosticsTesting;

    wxTimer m_StatusTimer;
    int     m_ProgressStage;

    CRef<objects::CUser_object>  m_Layout;
};


///////////////////////////////////////////////////////////////////////////////
/// CFrameworkDemoToolBarFactory
class CFrameworkDemoToolBarFactory :
    public IToolBarContributor
{
public:
    void    RegisterImageAliases(wxFileArtProvider& provider);

    /// @name IToolBarContributor implementation
    /// @{
    virtual void          GetToolBarNames(vector<string>& names);
    virtual wxAuiToolBar* CreateToolBar(const string& name, wxWindow* parent);
    /// @}
};


END_NCBI_SCOPE


#endif
// GUI_FRAMEWORK_DEMO___GUI__HPP
