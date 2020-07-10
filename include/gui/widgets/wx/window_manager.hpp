#ifndef GUI_WX_APP_DEMO___WINDOW_MANAGER__HPP
#define GUI_WX_APP_DEMO___WINDOW_MANAGER__HPP

/*  $Id: window_manager.hpp 40274 2018-01-19 17:05:09Z katargir $
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
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/widgets/wx/iwindow_manager.hpp>
#include <gui/widgets/wx/splitter.hpp>

#include <gui/widgets/wx/dock_manager.hpp>

#include <gui/utils/event_handler.hpp>

#include <wx/panel.h>
#include <wx/aui/aui.h>

class wxFileArtProvider;


BEGIN_NCBI_SCOPE

class CDockPanel;
class CDockManager;
class CWindowManagerEvent;
class IWMClientFactory;

BEGIN_SCOPE(objects)
    class CUser_object;
END_SCOPE(objects)

///////////////////////////////////////////////////////////////////////////////
/// IWindowManagerAdvisor
///
/// This interface represents the context in which Window Manager operates,
/// Window Manager delegates some of the functions / decisions to the Advisor.
/// These functions should be implemented by the application to allow
/// the application components to control the life cycle of IWMClients
/// managed by Window Manager.

class NCBI_GUIWIDGETS_WX_EXPORT IWindowManagerAdvisor
{
public:
    typedef vector<IWMClient*>  TClients;

    /// indicates that user requested closing the following clients
    /// (by pressing a "close" button, selecting a command etc.)
    /// Advisor must take necessary actions to close the clients (if needed).
    virtual void    OnCloseClientsRequest(const TClients& clients) = 0;

    /// called before removing the client
    virtual void    OnClientAboutToClose(IWMClient& client) = 0;

    /// called after the client has been removed
    virtual void    OnClientClosed(IWMClient& client) = 0;

    /// called when Active Client is changed (i.e. focus goes into another client)
    virtual void    OnActiveClientChanged(IWMClient* new_active) = 0;

    /// returns the title for the floating frames created by the Window Manager
    virtual string  GetFloatingFrameTitle(int index) = 0;

protected:
    virtual ~IWindowManagerAdvisor()    {};
};


///////////////////////////////////////////////////////////////////////////////
///  CWindowManager - component controlling windowing of client windows.
///
/// CWindowManager provides centralized control over multiple clients (views)
/// associated with an application. Window Manager provides two ways to
/// place clients - in independent top-level frame windows (Floating Frame)
///  or in containers embedded into manager's main window (docked). CWindowManager
/// allows for dynamic creation of hierarchical layouts consisting of containers
/// such as CSplitter and CTabControl and clients.
/// IWMClient interface represents an abstract client window that can  be
/// managed by CWindowManager. IWMContainer interface represents an abstract
/// container that can be embedded into CWindowManager. IWMCPosition
/// represents abstract notion of position in IWMContainer.

class NCBI_GUIWIDGETS_WX_EXPORT  CWindowManager
    : public wxPanel,
      public CEventHandler
{
    typedef wxPanel TParent;
public:
    typedef vector<IWMClient*>  TClients;
    typedef CWindowManagerEvent TEvent;
    typedef wxFrame TFrame;

public:
    /// registers graphical resources used by Window Manager
    static void RegisterImageAliases(wxFileArtProvider& provider);

public:
    CWindowManager(wxFrame* frame);
    virtual ~CWindowManager();

    virtual bool Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize);

    virtual void    SetAdvisor(IWindowManagerAdvisor* advisor);
    virtual void    SetMenuListener(IWindowManagerMenuListener* listener);
    
    wxFrame*    GetFrameWindow();
    CDockManager* GetDockManager();

    /// @name Client manipulation routines
    /// @{

    /// adds the client to the Main Tabbed Pane
    virtual bool AddClient(IWMClient& client, bool bFloat);

    void        MoveToMainTab(IWMClient& client);
    wxFrame*    MoveToFloatingFrame(IWMClient& client);

    void        Minimize(IWMClient& client);
    void        Restore(IWMClient& client);

    bool        IsInMainTab(IWMClient& client) const;
    bool        IsFloating(IWMClient& client) const;
    bool        IsMinimized(IWMClient& client) const;

    bool    CloseClient(IWMClient& client);
    void    CloseClients(const TClients& clients);
    void    CloseAllClients();

    /// checks if the client in registered in the Window Manager
    bool    IsRegistered(IWMClient& client) const;

    void    GetAllClients(TClients& clients);
    IWMClient*  GetActiveClient();

    void    ActivateClient(IWMClient& client);
    void    ActivateClients(TClients& clients);

    void    RegisterActiveClient(IWMClient& client);
    void    UnRegisterActiveClient(IWMClient& client);

    void    RaiseFloatingWindowsInZOrder();
    void    RaiseFloatingWindowsInZOrder(wxRect r);

    static  bool IsDragging();

    void    RefreshClient(IWMClient& client);
    /// @}

    void    ApplyLayout(const objects::CUser_object& layout);
    void    LoadLayout(const objects::CUser_object& layout, IWMClientFactory& factory);
    objects::CUser_object*  SaveLayout();

    bool    HasToolBar(const string& name);
    void    AddToolBar(wxAuiToolBar* toolbar);
    void    DeleteToolBar(const string& name);

    /// @name Event handlers
    /// @{
    virtual bool    ProcessEvent(wxEvent& event);
    virtual bool    Dispatch(CEvent* evt, EDispatch disp_how, int pool);

    void    OnPostCommand(CEvent* evt);
    void    OnCloseClientsRequestEvent(CEvent* evt);
    void    OnMoveToTopLevel(CEvent* evt);

    void    OnLeftUp(wxMouseEvent& evt);
    void    OnMotion(wxMouseEvent& evt);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& event);

    void    OnActivateWindow(wxCommandEvent& event);
    void    OnShowWindowsDlg(wxCommandEvent& event);
    void    OnWindowCommand(wxCommandEvent& event);
    void    OnUpdateWindowCommand(wxUpdateUIEvent& event);
    void    OnUpdateWindowCommand_Client(wxUpdateUIEvent& event, IWMClient* client);
    /// @}

    bool    OnCaptionPanelCommand(CDockPanel& panel, TCmdID cmd);
    bool    OnCloseClientsRequest(const TClients& clients);
    void    OnFocusChanged(wxWindow* new_focus);
    string  GetNewFloatingFrameTitle();
    wxFrame* GetMainWindow() { return m_FrameWindow; }

    virtual void SetRegistryPath(const string& reg_path);

    bool    OnSetClientColor(CDockPanel& panel);

    /// Called by CDockPanel to delegated DnD handling to Window Manager
    void    OnDockPanelStartDrag(CDockPanel* panel, const wxPoint& pt);

    virtual const wxMenu* GetMenu();

protected:
    DECLARE_EVENT_MAP();
    DECLARE_EVENT_TABLE();

protected:
    bool    x_RegisterClient(IWMClient& client);

    bool    x_AddClient(IWMClient& client);

    bool    x_CloseClient(IWMClient& client);
    void    x_RemoveClient(IWMClient& client);

    void    x_UpdateListener(IWMClient& client);

    virtual void x_UpdateActiveClient();

    IWMClient* x_GetActiveClientByWindow(wxWindow* widget);

    void    x_HandlePanelCommand(IWMClient& client, int cmd);

    virtual void    x_UpdateWindowsMenu(wxMenu& root_menu);
    string  x_GetClientLabelForMenu(IWMClient& client);

    virtual void    x_ResetMenuBar();

    virtual void x_OnBeginDrag(const wxPoint& mouse_pos);
    virtual void x_OnDrag(const wxPoint& mouse_pos);
    virtual void x_OnEndDrag(const wxPoint& mouse_pos, CDockManager::EDragResult result);

protected:
    typedef map<wxWindow*, IWMClient*>     TWindowToClientMap;
    typedef map<int, IWMClient*>    TCmdToWindowMap;

protected:
    /// Advisor is an external object that wants to customize the behavior
    /// of Window Manager by providing a callback interface
    IWindowManagerAdvisor*  m_Advisor;

    IWindowManagerMenuListener* m_MenuListener;

    /// main frame window hosting window manager
    wxFrame*        m_FrameWindow;

    /// wxAuiManager is required for CNotebook
    wxAuiManager    m_AuiManager;

    // path to the setting in CGuiRegistry
    string m_RegPath;

    /// the Docking Manager associated with this Window Manager
    CDockManager* m_DockManager;

    /// the collection of registered Client
    TClients  m_Clients;

    /// index wxWindow -> IWMClient
    TWindowToClientMap  m_WindowToClient;

    /// belonging to focused IWMClient
    IWMClient*     m_ActiveClient;

    // a map for associating runtime generated command IDs with windows
    TCmdToWindowMap   m_CmdToWindow;

    // the latest menu
    auto_ptr<wxMenu>    m_Menu;

    // Dock Panel dragging
    CDockPanel*   m_DragDockPanel; // the panel being dragged
    wxPoint m_ClickPoint; // the point where DnD started
    bool    m_Canceled;   // Dnd has been canceled

    static bool m_IsDragging; // DnD is in progress
};


///////////////////////////////////////////////////////////////////////////////
/// CWindowManagerEvent
class NCBI_GUIWIDGETS_WX_EXPORT CWindowManagerEvent
    : public CEvent
{
public:
    enum EEventType {
        eClientChanged = CEvent::eEvent_MinClientID,
        eCloseClientsRequest,
        ePostCommand,
    };

    CWindowManagerEvent(EEventType type, IWMClient* client);
    CWindowManagerEvent(IWMClient* client, TCmdID cmd);
    CWindowManagerEvent(EEventType type, const vector<IWMClient*>& clients);

    IWMClient*      GetClient();
    void        GetClients(vector<IWMClient*>& clients);
    TCmdID  GetCmd() const  {   return m_Cmd;   }

protected:
    IWMClient*      m_Client; //TODO
    vector<IWMClient*>  m_Clients; //TODO
    TCmdID  m_Cmd; // the command posted
};


END_NCBI_SCOPE;

#endif  // GUI_WX_APP_DEMO___WINDOW_MANAGER__HPP
