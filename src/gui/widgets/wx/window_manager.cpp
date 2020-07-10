/*  $Id: window_manager.cpp 40274 2018-01-19 17:05:09Z katargir $
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

#include <ncbi_pch.hpp>


#include <gui/widgets/wx/window_manager.hpp>

#include <gui/widgets/wx/dock_manager.hpp>
#include <gui/widgets/wx/dock_container.hpp>
#include <gui/widgets/wx/dock_notebook.hpp>
#include <gui/widgets/wx/dock_panel.hpp>
#include <gui/widgets/wx/min_panel_container.hpp>

#include <gui/widgets/wx/windows_dlg.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>

#include <gui/utils/menu_item.hpp>

#include <algorithm>

#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/toolbar.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// IWMClient::CFingerprint
IWMClient::CFingerprint::CFingerprint()
:   m_Persistent(false)
{
}


IWMClient::CFingerprint::CFingerprint(const string& id, bool persistent)
:   m_Id(id),
    m_Persistent(persistent)
{
}


IWMClient::CFingerprint::CFingerprint(const CFingerprint& orig)
:   m_Id(orig.m_Id),
    m_Persistent(orig.m_Persistent)
{
}


bool IWMClient::CFingerprint::operator==(const CFingerprint& orig) const
{
    return m_Id == orig.m_Id;
}


bool IWMClient::CFingerprint::IsEmpty() const
{
    return m_Id.empty();
}


string IWMClient::CFingerprint::AsPersistentString() const
{
    return m_Persistent ? m_Id : kEmptyStr;
}

const string IWMClient::CFingerprint::GetId() const 
{ 
    if (m_Persistent) {
        return m_Id; 
    } else {
        // strip out the pointer based component of ID
        vector<string> v;
        NStr::Split(m_Id, "@", v);
        if (!v.empty()) {
            string &s = v[0];
            size_t pos1 = s.find('(');
            if (pos1 == string::npos) {
                return s;
            }
            size_t pos2 = s.find(')');
            if (pos2 == string::npos) {
                return s;
            }

            string s2 = s.substr(pos1+1, (pos2 - pos1)-1);
            NStr::TruncateSpacesInPlace(s2);

            return s2;
        }
    }
    return kEmptyStr;
}


void IWMClient::CFingerprint::FromPersistentString(const string& s)
{
    m_Id = s;
    m_Persistent = ! s.empty();
}


///////////////////////////////////////////////////////////////////////////////
/// CWindowManagerEvent

CWindowManagerEvent::CWindowManagerEvent(EEventType type, IWMClient* client)
:   CEvent(type),
    m_Client(client),
    m_Cmd(eCmdInvalid)
{
}

CWindowManagerEvent::CWindowManagerEvent(IWMClient* client, TCmdID cmd)
:   CEvent(ePostCommand),
    m_Client(client),
    m_Cmd(cmd)
{
}


CWindowManagerEvent::CWindowManagerEvent(EEventType type, const vector<IWMClient*>& clients)
:   CEvent(type),
    m_Client(NULL),
    m_Clients(clients),
    m_Cmd(eCmdInvalid)
{
    _ASSERT(type == eCloseClientsRequest);
}


IWMClient* CWindowManagerEvent::GetClient()
{
    return m_Client;
}


void CWindowManagerEvent::GetClients(vector<IWMClient*>& clients)
{
    clients = m_Clients;
}


///////////////////////////////////////////////////////////////////////////////
/// CWindowManager

/// wxEvtHandler table
BEGIN_EVENT_TABLE(CWindowManager, wxPanel)
    EVT_MENU_RANGE(eCmdWindowXXXX, eCmdWindowLast, CWindowManager::OnActivateWindow)
    EVT_MENU(eCmdShowWindowsDlg, CWindowManager::OnShowWindowsDlg)
    EVT_MENU_RANGE(eCmdCloseDockPanel, eCmdWindowRestore,
                   CWindowManager::OnWindowCommand)
    EVT_UPDATE_UI_RANGE(eCmdCloseDockPanel, eCmdWindowRestore,
                   CWindowManager::OnUpdateWindowCommand)

    EVT_LEFT_UP(CWindowManager::OnLeftUp)
    EVT_MOTION(CWindowManager::OnMotion)
    EVT_MOUSE_CAPTURE_LOST(CWindowManager::OnMouseCaptureLost)
END_EVENT_TABLE()

/// CEventHandler map
BEGIN_EVENT_MAP(CWindowManager, CEventHandler)
    ON_EVENT(CWindowManagerEvent, CWindowManagerEvent::ePostCommand,
             &CWindowManager::OnPostCommand)
    ON_EVENT(CWindowManagerEvent, CWindowManagerEvent::eCloseClientsRequest,
             &CWindowManager::OnCloseClientsRequestEvent)
END_EVENT_MAP()


CWindowManager::CWindowManager(wxFrame* frame)
:   wxPanel(),
    m_Advisor(NULL),
    m_MenuListener(NULL),
    m_FrameWindow(frame),
    m_DockManager(NULL),
    m_ActiveClient(NULL),
    m_DragDockPanel(NULL),
    m_Canceled(false)
{
    SetSizer(new wxBoxSizer(wxVERTICAL));

    wxAuiDockArt* dock_art = m_AuiManager.GetArtProvider();
    dock_art->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE, 0);
}


void CWindowManager::RegisterImageAliases(wxFileArtProvider& provider)
{
    provider.RegisterFileAlias(wxT("wm_menu"), wxT("wm_menu.png"));
    provider.RegisterFileAlias(wxT("wm_menu_active"), wxT("wm_menu_active.png"));

    provider.RegisterFileAlias(wxT("wm_minimize"), wxT("wm_minimize.png"));
    provider.RegisterFileAlias(wxT("wm_minimize_active"), wxT("wm_minimize_active.png"));

    provider.RegisterFileAlias(wxT("wm_float"), wxT("wm_float.png"));
    provider.RegisterFileAlias(wxT("wm_float_active"), wxT("wm_float_active.png"));

    provider.RegisterFileAlias(wxT("wm_close"), wxT("wm_close.png"));
    provider.RegisterFileAlias(wxT("wm_close_active"), wxT("wm_close_active.png"));
}


void CWindowManager::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}


bool CWindowManager::Create(wxWindow *parent, wxWindowID id,
                            const wxPoint& pos, const wxSize& size)
{
    bool res = wxPanel::Create(parent, id, pos, size, wxBORDER_NONE, wxT("window_manager"));
    if(res) {
        m_AuiManager.SetManagedWindow(m_FrameWindow);

        wxAuiPaneInfo info;
        info.CenterPane();

        m_AuiManager.AddPane(this, info);

        m_DockManager = new CDockManager(*this, *this);
        m_DockManager->SetKeyboardHook(m_FrameWindow);

        // create the main Dock Container that will be hosted inside Window Manager
        CDockContainer* dock_cont = m_DockManager->GetMainContainer();
        GetSizer()->Add(dock_cont, 1, wxEXPAND);

        // create a container for minimized Clients
        CMinPanelContainer* cont = m_DockManager->GetMinPanelContainer();
        GetSizer()->Add(cont, 0, wxEXPAND);

        m_AuiManager.Update();
    }   

    return res;
}


CWindowManager::~CWindowManager()
{
    CloseAllClients();

    m_AuiManager.UnInit();

    delete m_DockManager;
    m_DockManager = NULL;
}


void CWindowManager::SetAdvisor(IWindowManagerAdvisor* advisor)
{
    m_Advisor = advisor;
}


void CWindowManager::SetMenuListener(IWindowManagerMenuListener* listener)
{
    m_MenuListener = listener;
}


wxFrame* CWindowManager::GetFrameWindow()
{
    return m_FrameWindow;
}

CDockManager* CWindowManager::GetDockManager()
{
    return m_DockManager;
}


bool CWindowManager::AddClient(IWMClient& client, bool bFloat)
{
    if(client.GetWindow())  {
        if(x_RegisterClient(client))    {
            if (!client.IsADialog()) {
                m_DockManager->AddToDefaultLocation(client, bFloat);
                RefreshClient(client);
                x_ResetMenuBar();
            } else {
                wxWindow* wnd = client.GetWindow();
                if (wnd) {
                    wnd->Show();
                    OnFocusChanged(wnd);
                }
            }
        }
        return true;
    }
    return false;
}


void CWindowManager::MoveToMainTab(IWMClient& client)
{
    m_DockManager->MoveToMainTab(client);
}


wxFrame* CWindowManager::MoveToFloatingFrame(IWMClient& client)
{
    return m_DockManager->MoveToFloatingFrame(client);
}


void CWindowManager::Minimize(IWMClient& client)
{
    return m_DockManager->Minimize(client);
}


void CWindowManager::Restore(IWMClient& client)
{
    return m_DockManager->Restore(client);
}


bool CWindowManager::IsInMainTab(IWMClient& client) const
{
    return m_DockManager->IsInMainTab(client);
}


bool CWindowManager::IsFloating(IWMClient& client) const
{
    return m_DockManager->IsFloating(client);
}


bool CWindowManager::IsMinimized(IWMClient& client) const
{
    return m_DockManager->IsMinimized(client);
}


/// max number of characters in the client labels shown in popups
static const int kMaxClientLabel = 30;

string CWindowManager::x_GetClientLabelForMenu(IWMClient& client)
{
    string label = client.GetClientLabel();
    if((int) label.size() > kMaxClientLabel)  {
        label.resize(kMaxClientLabel);
        label += "...";
    }
    return label;
}


// handles commands originating from IDockPanel
bool CWindowManager::OnCaptionPanelCommand(CDockPanel& panel, TCmdID cmd)
{
    switch(cmd) {
    case eCmdWindowMinimize:
        m_DockManager->Minimize(*panel.GetClient());
        return true;
    case eCmdCloseDockPanel:
        _ASSERT(m_Advisor);
        /// ask advisor to take necessary action
        if (m_Advisor) {
            TClients clients;
            clients.push_back(panel.GetClient());
            m_Advisor->OnCloseClientsRequest(clients);
        }
        return true;
    case eCmdMoveToMainTab:
    case eCmdWindowRestore:
    case eCmdWindowFloat:   {
        /// these commands can affect window hierarchy and result in some windows
        /// being destroyed because of this we need to execute them
        /// asynchronously so that we do not destroy windows on the current call
        /// stack
        CRef<CEvent> evt(new CWindowManagerEvent(panel.GetClient(), cmd));
        Post(evt, CEventHandler::eDispatch_SelfOnly);
        return true;
    }
    case eCmdSetClientColor:
        return OnSetClientColor(panel);
    default:
        //_ASSERT(false);
        break;
    }
    return false;
}


bool CWindowManager::OnSetClientColor(CDockPanel& panel)
{
    IWMClient* client = panel.GetClient();
    if (client)  {
        if(client->GetColor())   {
            // the client does support a notion of color
            CRgbaColor color = *client->GetColor();
            wxWindow* win = dynamic_cast<wxWindow*>(&panel);
            if (NcbiChooseColor(win, color)) {
                client->SetColor(color);
                win->Refresh();
            }
        }
    }
    return true;
}


bool CWindowManager::OnCloseClientsRequest(const TClients& clients)
{
    CRef<CEvent> evt(new CWindowManagerEvent(CWindowManagerEvent::eCloseClientsRequest, clients));
    Post(evt, CEventHandler::eDispatch_SelfOnly);
    return true;
}


void CWindowManager::x_RemoveClient(IWMClient& client)
{
    // disconnect from command routing
    CEventHandler* target = dynamic_cast<CEventHandler*>(&client);
    if(target)  {
        RemoveListener(target);
    }

    // remove from the container
    if (!client.IsADialog())
        m_DockManager->Remove(client);
}


bool CWindowManager::CloseClient(IWMClient& client)
{
    if(x_CloseClient(client))   {
        x_ResetMenuBar();
        return true;
    }
    return false;
}


void CWindowManager::CloseClients(const TClients& clients)
{
    for( size_t i = 0;  i < clients.size();  i++ )  {
        IWMClient& client = *clients[i];
        x_CloseClient(client);
    }
    x_ResetMenuBar();
}


void CWindowManager::CloseAllClients()
{
    while( ! m_Clients.empty())  {
        IWMClient& client = *m_Clients.back();
        x_CloseClient(client);
    }
    x_ResetMenuBar();
}


// removes from Root Container, Unregisters and sends notification
bool CWindowManager::x_CloseClient(IWMClient& client)
{
    TClients::iterator it =
        std::find(m_Clients.begin(), m_Clients.end(), &client);
    _ASSERT(it != m_Clients.end());

    if(it != m_Clients.end()) {
        m_Clients.erase(it);
        UnRegisterActiveClient(client);

        _ASSERT(m_Advisor);
        if(m_Advisor)   {
            m_Advisor->OnClientAboutToClose(client);
        }

        x_RemoveClient(client);

        if(m_Advisor)   {
            m_Advisor->OnClientClosed(client);
        }
        return true;
    } else {
        ERR_POST("CWindowManager::x_CloseClient() - client not registred, cannot close");
    }
    return false;
}


void CWindowManager::GetAllClients(vector<IWMClient*>& clients)
{
    clients = m_Clients;
}


bool CWindowManager::IsRegistered(IWMClient& client) const
{
    bool found = (std::find(m_Clients.begin(), m_Clients.end(), &client) != m_Clients.end());
    return found;
}


void CWindowManager::ActivateClient(IWMClient& client)
{
    TClients clients;
    clients.push_back(&client);
    ActivateClients(clients);
}


// traverses all containers and adjusts tab panels so that clients become
// visible
void CWindowManager::ActivateClients(TClients& clients)
{
    m_DockManager->ActivateClients(clients);
}

void CWindowManager::RegisterActiveClient(IWMClient& client)
{
    m_WindowToClient[client.GetWindow()] = &client;
}

void CWindowManager::UnRegisterActiveClient(IWMClient& client)
{
    m_WindowToClient.erase(client.GetWindow());

    if (&client == m_ActiveClient) {
        m_ActiveClient = NULL;

        if (m_Advisor)
            m_Advisor->OnActiveClientChanged(m_ActiveClient);

        x_ResetMenuBar();
    }
}

void CWindowManager::RaiseFloatingWindowsInZOrder()
{
    m_DockManager->RaiseFloatingInZOrder();
}

void CWindowManager::RaiseFloatingWindowsInZOrder(wxRect r)
{
    m_DockManager->RaiseFloatingInZOrder(r);
}

bool CWindowManager::m_IsDragging = false;

bool CWindowManager::IsDragging()
{
    return m_IsDragging;
}

bool CWindowManager::HasToolBar(const string& name)
{
    wxString wx_name = ToWxString(name);
    wxAuiPaneInfo& pane = m_AuiManager.GetPane(wx_name);
    return pane.IsOk();
}


void CWindowManager::AddToolBar(wxAuiToolBar* toolbar)
{
    _ASSERT(toolbar);
    if(toolbar) {
        wxAuiPaneInfo pane_info;

        wxString name = toolbar->GetName();
        pane_info.Name(name);
        pane_info.Caption(name);

        pane_info.ToolbarPane().Top();
        pane_info.TopDockable(true)
                 .LeftDockable(false)
                 .RightDockable(false)
                 .BottomDockable(false);
        pane_info.Resizable(false);
        pane_info.DestroyOnClose(true);

        m_AuiManager.AddPane(toolbar, pane_info);
        m_AuiManager.Update();
    }
}


void CWindowManager::DeleteToolBar(const string& name)
{
    wxAuiPaneInfo& pane = m_AuiManager.GetPane(ToWxString(name));
    if(pane.IsOk()) {
        wxWindow* toolbar = pane.window;
        m_AuiManager.DetachPane(toolbar);
        toolbar->Destroy();
    }
}


IWMClient* CWindowManager::GetActiveClient()
{
    return m_ActiveClient;
}


void CWindowManager::OnShowWindowsDlg(wxCommandEvent& WXUNUSED(event))
{
    CWindowsDlg dlg;

    string path = CGuiRegistryUtil::MakeKey(m_RegPath, "Windows Dialog");
    dlg.SetRegistryPath(path);

    dlg.SetWindowManager(this);

    dlg.Create(this);
    dlg.ShowModal();
}


// creates entry in m_ClientToRec table if it does not exist
bool CWindowManager::x_RegisterClient(IWMClient& client)
{
    bool registered = IsRegistered(client);
    _ASSERT( ! registered);

    if( ! registered)   {
        m_Clients.push_back(&client);
        RegisterActiveClient(client);
        return true;
    }
    return false;
}


void CWindowManager::x_UpdateListener(IWMClient& client)
{
    TEvent evt(TEvent::eClientChanged, &client);
    Send(&evt);
}


// backbone of the menu contributed to the main Menu Bar

WX_DEFINE_MENU(sBaseMenu)
    WX_SUBMENU("&Window")
        WX_MENU_SEPARATOR_L("Actions")
        WX_MENU_ITEM(eCmdWindowMinimize)
        WX_MENU_ITEM(eCmdWindowFloat)
        WX_MENU_ITEM(eCmdMoveToMainTab)
        WX_MENU_SEPARATOR_L("Close")
        WX_MENU_ITEM(eCmdCloseDockPanel)
        WX_MENU_SEPARATOR_L("Actions2")
        WX_MENU_ITEM(eCmdShowWindowsDlg)
        WX_MENU_SEPARATOR_L("Windows")
    WX_END_SUBMENU()
WX_END_MENU()


const wxMenu* CWindowManager::GetMenu()
{
    /// build on demand
    if( ! m_Menu.get())  {
        // create static backbone
        CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
        m_Menu.reset(cmd_reg.CreateMenu(sBaseMenu));

        _ASSERT(m_Menu.get());

        // create dynamic part
        x_UpdateWindowsMenu(*m_Menu);

        // get menus from only active client
        if (m_ActiveClient) {
            const wxMenu* client_root = m_ActiveClient->GetMenu();
            if(client_root)    {
                // merge Client's menu into Manager's menu
                Merge(*m_Menu, *client_root);
            }
        }
/*
        // get menus from clients
        ITERATE(TClients, it, m_Clients) {
            IWMClient* client = *it;
            const wxMenu* client_root = client->GetMenu();
            if(client_root)    {
                // merge Client's menu into Manager's menu
                Merge(*m_Menu, *client_root);
            }
        }
 */
    }
    return m_Menu.get();
}

const static int    kMaxRecentItems = 10;

// generate a menu item label with numeric access key "&N"
inline string   GenerateWinItemPrefix(int index)
{
    if(index < 10)  {
        return string("&" +  NStr::IntToString(index) + "  ");
    } else if(index == 10) {
        return "1&0  ";
    }
    return "    ";
}


void CWindowManager::x_UpdateWindowsMenu(wxMenu& root_menu)
{
    m_CmdToWindow.clear();

    static wxString sWindow(wxT("&Window"));

    wxMenuItem* win_item = ncbi::FindSubItem(root_menu, sWindow);
    wxMenu* win_menu = win_item ? win_item->GetSubMenu() : NULL;

    if(win_menu == NULL)    {
        // not found - create new one
        win_menu = new wxMenu(sWindow);
        root_menu.Append(wxID_ANY, win_menu->GetTitle(), win_menu);
    } else {
        // remove child items corresponding to windows
        wxMenuItemList& items = win_menu->GetMenuItems();
        for(wxMenuItemList::compatibility_iterator it = items.GetFirst();
                                                    it; ) {
            wxMenuItem* sub_item = it->GetData();

            wxMenuItemList::compatibility_iterator it_next = it->GetNext();

            if(sub_item->GetKind() == wxITEM_NORMAL)  {
                TCmdID cmd = sub_item->GetId();
                if(cmd >= eCmdWindowXXXX  &&  cmd <= eCmdWindowLast)    {
                    win_menu->Delete(sub_item);
                }
            }
            it = it_next;
        }
    }

    // show first kMaxRecentItems clients in "Windows" menu
    int index = 0;
    string s;
    ITERATE(TClients, it, m_Clients) {
        IWMClient* client = *it;

        TCmdID cmd = eCmdWindowXXXX + index++;
        m_CmdToWindow[cmd] = client;
        s = GenerateWinItemPrefix(index) + x_GetClientLabelForMenu(*client);
        win_menu->Append(cmd, ToWxString(s));

        if(index >= kMaxRecentItems)    { // show no more then kMaxRecentItems items
            break;
        }
    }
}

void CWindowManager::OnFocusChanged( wxWindow* new_focus )
{
    IWMClient* new_client = 0;
    // check if focused widget is our client or a child of our client
    for (wxWindow* w = new_focus; w != NULL && w != this;  w = w->GetParent()) {
        IWMClient* client = x_GetActiveClientByWindow (w);

        if (!client)
            continue;

        // this OUR client
        new_client = client;
        break;
    }

    if (new_client != m_ActiveClient) {
        if (m_ActiveClient != 0) {
            // redraw captions for views affected by changes in focus
            for (wxWindow* w = m_ActiveClient->GetWindow(); w != 0; w = w->GetParent()) {
                CDockPanel* panel = dynamic_cast<CDockPanel*>(w);
                if (panel) {
                    w->Refresh();
                    break;
                }
            }
        }
        m_ActiveClient = new_client;

        x_ResetMenuBar();
    }

    if (m_Advisor)
        m_Advisor->OnActiveClientChanged(m_ActiveClient);
}

// update m_ActiveClient to keep it in-sync with focus
void   CWindowManager::x_UpdateActiveClient()
{
    OnFocusChanged( FindFocus() );
}

void CWindowManager::OnActivateWindow( wxCommandEvent& event )
{
    int cmd = event.GetId();
    IWMClient* client = m_CmdToWindow[cmd];
    ActivateClient(*client);
}


/// handles an internal command posted to itself
void CWindowManager::OnPostCommand(CEvent* evt)
{
    CWindowManagerEvent* wm_evt = dynamic_cast<CWindowManagerEvent*>(evt);
    _ASSERT(wm_evt  &&  wm_evt->GetClient());

    TCmdID cmd = wm_evt->GetCmd();
    IWMClient* client = wm_evt->GetClient();

    switch(cmd) {
    case eCmdCloseDockPanel:
    case eCmdMoveToMainTab:
    case eCmdWindowFloat:
    case eCmdWindowMinimize:
    case eCmdWindowRestore:
        x_HandlePanelCommand(*client, cmd);
        break;

    default:
        _ASSERT(false);
    }
}


// handles commands arriving from the application menu bar
void CWindowManager::OnWindowCommand(wxCommandEvent& event)
{
    IWMClient* client = GetActiveClient();
    if(client)  {
        x_HandlePanelCommand(*client, event.GetId());
    }
}


void CWindowManager::OnUpdateWindowCommand(wxUpdateUIEvent& event)
{
    IWMClient* client = GetActiveClient();
    OnUpdateWindowCommand_Client(event, client);
}


void CWindowManager::OnUpdateWindowCommand_Client(wxUpdateUIEvent& event, IWMClient* client)
{
    bool en = false;
    if(client && !client->IsADialog())  {
        switch(event.GetId()) {
        case eCmdCloseDockPanel:
            en = true;
            break;

        case eCmdMoveToMainTab:
            en = m_DockManager->IsInMainTab(*client);
            break;

        case eCmdWindowFloat:
            en = ! m_DockManager->IsFloating(*client);
            break;

        case eCmdWindowMinimize:
            en = ! m_DockManager->IsMinimized(*client);
            break;

        case eCmdWindowRestore:
            en = m_DockManager->IsMinimized(*client);
            break;

        default:
            _ASSERT(false);
            break;
        }
    }
    event.Enable(en);
}


void CWindowManager::x_HandlePanelCommand(IWMClient& client, int cmd)
{
    switch(cmd) {
    case eCmdCloseDockPanel:
        _ASSERT(m_Advisor);
        /// ask advisor to take necessary action
        if(m_Advisor) {
            TClients clients;
            clients.push_back(&client);
            m_Advisor->OnCloseClientsRequest(clients);
        }
        break;

    case eCmdMoveToMainTab:
        m_DockManager->MoveToMainTab(client);
        break;

    case eCmdWindowFloat:
        m_DockManager->MoveToFloatingFrame(client);
        break;

    case eCmdWindowMinimize:
        m_DockManager->Minimize(client);
        break;

    case eCmdWindowRestore:
        m_DockManager->Restore(client);
        break;
    default:
        _ASSERT(false);
        break;
    }
}

void CWindowManager::OnCloseClientsRequestEvent(CEvent* evt)
{
    CWindowManagerEvent* wm_evt = dynamic_cast<CWindowManagerEvent*>(evt);
    _ASSERT(wm_evt);
    _ASSERT(m_Advisor);

    if(m_Advisor)   {
        TClients clients;
        wm_evt->GetClients(clients);
        m_Advisor->OnCloseClientsRequest(clients);
    }
}


IWMClient* CWindowManager::x_GetActiveClientByWindow (wxWindow* widget)
{
    CDockPanel* panel = dynamic_cast<CDockPanel*>(widget);
    if(panel)   {
        return panel->GetClient();
    } else {
        TWindowToClientMap::const_iterator it = m_WindowToClient.find(widget);
        return (it == m_WindowToClient.end()) ? NULL : it->second;
    }
}

string CWindowManager::GetNewFloatingFrameTitle()
{
    static int frameId = 1;

    if(m_Advisor)   {
        return m_Advisor->GetFloatingFrameTitle(frameId++);
    } else {
        return kEmptyStr;
    }
}

void CWindowManager::RefreshClient( IWMClient& client )
{
    bool found = IsRegistered( client );
    //_ASSERT(found); This is valid for composite view such as Multi-pane alignment view
    if( !found ) return;

    wxWindow* window = client.GetWindow();
    if( !window ) return;


    CDockPanel* panel = dynamic_cast<CDockPanel*>( window->GetParent() );
    if( !panel ) return;

    panel->UpdateCaption();

    wxWindow* parent = panel->GetParent();
    CDockNotebook* notebook = dynamic_cast<CDockNotebook*>( parent );
    if( notebook ){
        notebook->RefreshPageByWindow( panel );
    } 
    parent->Layout();
    parent->Refresh();
}

bool CWindowManager::Dispatch( CEvent* evt, EDispatch disp_how, int pool )
{
    if( disp_how == eDispatch_FirstHandler ){
        // try to deliver event to the active client
        IWMClient* client = GetActiveClient();
        CEventHandler* client_handler = dynamic_cast<CEventHandler*>(client);

        if(HasListener(client_handler, pool)) {
            return client_handler->Send(evt, disp_how, pool);
        }
    }

    // default pathway
    return CEventHandler::Dispatch(evt, disp_how, pool);
}


bool CWindowManager::ProcessEvent( wxEvent& event )
{
    // Stops the same event being processed repeatedly
    static wxEventType inEvent = wxEVT_NULL;
    if( inEvent == event.GetEventType() ){
        return false;
    }
    inEvent = event.GetEventType();
    
    // Redirect events to active child first.
    bool res = false;
    if( event.IsCommandEvent() ){
        wxEventType type = event.GetEventType();

        if( type == wxEVT_UPDATE_UI || type == wxEVT_COMMAND_MENU_SELECTED ){
            IWMClient* client = GetActiveClient();
            if( client ){

                wxEvtHandler* handler = client->GetCommandHandler();
                _ASSERT(handler);

                res = handler->ProcessEvent( event );
            }
        }
    }
    if( !res ){
        res = TParent::ProcessEvent( event );
    }

    inEvent = wxEVT_NULL;
    return res;
}


objects::CUser_object* CWindowManager::SaveLayout()
{
    objects::CUser_object* layout = m_DockManager->SaveLayout();
    return layout;
}


void CWindowManager::ApplyLayout( const objects::CUser_object& layout )
{
    m_DockManager->ApplyLayout( layout );

    x_ResetMenuBar();
}


void CWindowManager::LoadLayout( const objects::CUser_object& layout, IWMClientFactory& factory )
{
    m_DockManager->LoadLayout( layout, factory );

   // We have lot of fixes with window layout since GB-575
   // This call causes gtk warnings on Linux, hopefully we don't need it anymore
   // This probably fixes window layout issues. JIRA: GB-575
   //Fit();

    x_ResetMenuBar();
}


void CWindowManager::x_ResetMenuBar()
{
    m_Menu.reset(); // delete the old one

    if( m_MenuListener ){
        m_MenuListener->UpdateMenuBar(); // request Menu Bar update
    }
}


void CWindowManager::OnDockPanelStartDrag( CDockPanel* panel, const wxPoint& pt )
{
    m_DragDockPanel = panel;

    // translate mouse position into local (client) coordinate system
    wxPoint sc_mouse_pos = m_DragDockPanel->ClientToScreen(pt);
    m_ClickPoint = ScreenToClient(sc_mouse_pos);
    m_IsDragging = true;

    /// capture mouse to receive all mouse events - now we handle the mouse
    if (!HasCapture())
        CaptureMouse();

    x_OnBeginDrag(m_ClickPoint);
}


void CWindowManager::OnMotion(wxMouseEvent& evt)
{
    wxPoint pt = evt.GetPosition();
    //WM_POST("CWindowManager::OnMotion()  " << pt.x << " " << pt.y);

    if(m_IsDragging) {
        if ( ! evt.LeftIsDown())    {
            x_OnEndDrag(pt, CDockManager::eDrop);
        } else {
            x_OnDrag(pt);
        }
    }
}


void CWindowManager::OnLeftUp(wxMouseEvent& evt)
{
    wxPoint pt = evt.GetPosition();
    //WM_POST("CWindowManager::OnLeftUp()  " << pt.x << " " << pt.y);

    if (HasCapture())   {
        //WM_POST("ReleaseMouse()");
        ReleaseMouse();
    }

    if (m_IsDragging)   {
        x_OnEndDrag(pt, CDockManager::eDrop);
    }
}


void CWindowManager::OnMouseCaptureLost(wxMouseCaptureLostEvent& WXUNUSED(evt))
{
    //WM_POST("CWindowManager::OnMouseCaptureLost()");
    if (m_IsDragging)   {
        wxPoint pt = ::wxGetMousePosition();
        x_OnEndDrag(pt, CDockManager::eCancel);
    }
}


void CWindowManager::x_OnBeginDrag(const wxPoint& mouse_pos)
{
    //WM_POST("CWindowManager::x_OnBeginDrag()");

    m_IsDragging = true;
    wxPoint sc_mouse_pos = ClientToScreen(mouse_pos);
    m_DockManager->OnBeginDrag(*m_DragDockPanel, sc_mouse_pos);
}


void CWindowManager::x_OnDrag(const wxPoint& mouse_pos)
{
    //WM_POST("CWindowManager::x_OnDrag()");

    wxPoint sc_mouse_pos = ClientToScreen(mouse_pos);
    bool ok = m_DockManager->OnDrag(*m_DragDockPanel, sc_mouse_pos);

    if( ! ok) {
        // dragging has been canceled
        //WM_POST("CWindowManager::x_OnDrag() ok = false - canceling");

        m_Canceled = true;
        ReleaseMouse();
        //WM_POST("ReleaseMouse()");

        m_IsDragging = false;

        m_DragDockPanel->FinishDrag();
    }
}


void CWindowManager::x_OnEndDrag(const wxPoint& mouse_pos, CDockManager::EDragResult result)
{
    //LOG_POST("CWindowManager::x_OnEndDrag()");
    wxPoint sc_mouse_pos = ClientToScreen(mouse_pos);
    m_DockManager->OnEndDrag(*m_DragDockPanel, result, sc_mouse_pos);
    m_IsDragging = false;

    m_DragDockPanel->FinishDrag();
}


END_NCBI_SCOPE
