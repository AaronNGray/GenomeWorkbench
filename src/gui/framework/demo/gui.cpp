/*  $Id: gui.cpp 39670 2017-10-25 18:01:21Z katargir $
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
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbitime.hpp>
#include <util/thread_pool.hpp>

#include <gui/framework/demo/gui.hpp>

#include <gui/framework/demo/test_views.hpp>

#include <gui/framework/view_manager_service.hpp>
#include <gui/framework/window_manager_service.hpp>
#include <gui/framework/event_log_service_impl.hpp>
#include <gui/framework/status_bar_service.hpp>
#include <gui/framework/diag_panel.hpp>
#include <gui/framework/event_view.hpp>
#include <gui/framework/task_view.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/commands.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <gui/widgets/wx/dock_frames.hpp> //TODO

#include <gui/utils/extension_impl.hpp>

#include <objects/general/User_object.hpp>

#include <wx/menu.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/toolbar.h>
#include <wx/dnd.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
/// CFrameworkDemoGUIDropTarget
///
/// An example of how File D&D can be handled in an application.
class CFrameworkDemoGUIDropTarget : public wxDropTarget
{
public:
    CFrameworkDemoGUIDropTarget(CFrameworkDemoGUI& gui)
    :   m_GUI(gui)
    {
        SetDataObject(new wxFileDataObject);
    }

    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def)
    {
        if ( !GetData() )
            return wxDragNone;

        wxFileDataObject* dobj = (wxFileDataObject *)m_dataObject;

        if(dobj  &&  (def == wxDragMove  ||  def == wxDragCopy)) {
            wxArrayString filenames = dobj->GetFilenames();
            vector<string> names;
            FromArrayString(filenames, names);

            // here we can do something usefull with the filenames
            string s = "The following files were dropped into the application:";
            for(   size_t i = 0;  i < names.size();  i++ )  {
                s += "\n" + names[i];
            }
            wxMessageBox(ToWxString(s));
            return def;
        }
        return wxDragError;
    }
protected:
    CFrameworkDemoGUI& m_GUI;
};

///////////////////////////////////////////////////////////////////////////////
///

static int const kDiagTimerID = 1;
static int const kStatusTimerID = 2;

BEGIN_EVENT_TABLE(CFrameworkDemoGUI, wxEvtHandler)
    EVT_MENU( eCmdCreateClockView, CFrameworkDemoGUI::OnCreateClockViewClick )
    EVT_MENU( eCmdCreateOpenGLView, CFrameworkDemoGUI::OnCreateOpenglViewClick )
    EVT_MENU( eCmdCreateDiagConsole, CFrameworkDemoGUI::OnShowDiagnosticsConsoleClick )
    EVT_MENU( eCmdCreateEventView, CFrameworkDemoGUI::OnCreateEventViewClick )
    EVT_MENU( eCmdCreateTaskView, CFrameworkDemoGUI::OnCreateTaskViewClick )
    //EVT_MENU( eCmdCreateTextCtrl, CFrameworkDemoGUI::OnCreateTextCtrlClick )
    //EVT_MENU( eCmdCreateHTMLView, CFrameworkDemoGUI::OnCreateHTMLViewClick )
    //EVT_MENU( eCmdCreateTreeCtrl, CFrameworkDemoGUI::OnCreateTreeControlClick )
    EVT_MENU( eCmdCloseView, CFrameworkDemoGUI::OnCloseViewClick )
    EVT_MENU( eCmdCloseAllViews, CFrameworkDemoGUI::OnCloseAllViewsClick)
    EVT_MENU( eCmdSaveLayout, CFrameworkDemoGUI::OnSaveLayoutClick )
    EVT_UPDATE_UI( eCmdSaveLayout, CFrameworkDemoGUI::OnSaveLayoutUpdate )
    EVT_MENU( eCmdRestoreLayout, CFrameworkDemoGUI::OnRestoreLayoutClick )
    EVT_UPDATE_UI( eCmdRestoreLayout, CFrameworkDemoGUI::OnRestoreLayoutUpdate )
    EVT_MENU( eCmdTestDiagPanel, CFrameworkDemoGUI::OnTestDiagPanelClick )
    EVT_UPDATE_UI( eCmdTestDiagPanel, CFrameworkDemoGUI::OnTestDiagPanelUpdate )
    EVT_MENU(eCmdTestStatusProgress, CFrameworkDemoGUI::OnTestStatusProgress)
    EVT_UPDATE_UI(eCmdTestStatusProgress, CFrameworkDemoGUI::OnTestStatusProgressUpdate)
    EVT_TIMER(kDiagTimerID, CFrameworkDemoGUI::OnDiagnosticsTimer)
    EVT_TIMER(kStatusTimerID, CFrameworkDemoGUI::OnStatusTimer)
END_EVENT_TABLE()


CFrameworkDemoGUI::CFrameworkDemoGUI()
:   m_Timer(this, kDiagTimerID),
    m_DiagnosticsTesting(true),
    m_StatusTimer(this, kStatusTimerID),
    m_ProgressStage(-1)
{
}


void CFrameworkDemoGUI::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}


void CFrameworkDemoGUI::SetWorkbench(IWorkbench* workbench)
{
    m_Workbench = workbench;
}


string CFrameworkDemoGUI::GetAppTitle()
{
    return "GUI Framework Demo";
}


void CFrameworkDemoGUI::PostCreateWindow()
{
    _ASSERT(m_Workbench);

    // register Commands
    x_RegisterCommands();

    // Connect to Menu Service
    IMenuService* menu_srv = m_Workbench->GetMenuService();
    menu_srv->AddContributor(this);
    menu_srv->ResetMenuBar();

    // Connect to ToolBar Service
    IToolBarService* tb_srv = m_Workbench->GetToolBarService();
    wxFileArtProvider& provider = m_Workbench->GetFileArtProvider();

    CFrameworkDemoToolBarFactory* factory = new CFrameworkDemoToolBarFactory();
    factory->RegisterImageAliases(provider);

    tb_srv->AddToolBarFactory(factory);

    /// TODO
    /// Connect Window Manager Service
    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    wm_srv->SetAdvisor(this);

    //TODO
    IMenuContributor* contributor = dynamic_cast<IMenuContributor*>(wm_srv);
    menu_srv->AddContributor(contributor);

    // connect itself to event handling
    m_Workbench->GetMainWindow()->PushEventHandler(this);

    m_Workbench->GetMainWindow()->SetDropTarget(new CFrameworkDemoGUIDropTarget(*this));
}


static const char* kDefLayoutTag = "DefaultLayout";

void CFrameworkDemoGUI::PostInit()
{
    // Register view factories
    if (m_Workbench) {
        IViewManagerService* manager = m_Workbench->GetViewManagerService();

        manager->RegisterFactory(*new CwxGLTestViewFactory());
        manager->RegisterFactory(*new CClockViewFactory());
    }
}


void CFrameworkDemoGUI::RestoreWindowLayout()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        /// load default window layout
        LOG_POST(Info << "Loading default window layout");
        IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
        CConstRef<objects::CUser_field> field = view.GetField("Layouts");

        if(field  &&  field->HasField(kDefLayoutTag))  {
            /// create windows
            LOG_POST(Info << "Creating default window layout");
            const CUser_object& layout = field->GetField(kDefLayoutTag).GetData().GetObject();
            IViewManagerService* view_mrg = m_Workbench->GetViewManagerService();
            IWMClientFactory* factory = dynamic_cast<IWMClientFactory*>(view_mrg);
            _ASSERT(factory);
            wm_srv->LoadLayout(layout, *factory);
        }
    }
}


void CFrameworkDemoGUI::PreShutDown()
{
    x_CloseAllViews();

    //here we can unregister application-specific services
}


void CFrameworkDemoGUI::PreDestroyWindow()
{
    // disconnect from event handling
    m_Workbench->GetMainWindow()->RemoveEventHandler(this);

    // unsubscribe from menus
    IMenuService* menu_srv = m_Workbench->GetMenuService();
    menu_srv->RemoveContributor(this);

    /// Disconnect Window Manager Service
    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    wm_srv->SetAdvisor(NULL);

    IMenuContributor* contributor = dynamic_cast<IMenuContributor*>(wm_srv);
    menu_srv->RemoveContributor(contributor);

    // toolbar factory will be deleted by ToolBar Service
}


bool CFrameworkDemoGUI::CanShutDown()
{
    return true;
}


/*void CFrameworkDemoGUI::SetMenuService(IMenuService* service)
{
    m_MenuService = service;
}*/


void CFrameworkDemoGUI::x_RegisterCommands()
{
    CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();
    wxFileArtProvider& provider = m_Workbench->GetFileArtProvider();

    // fisrt we call functions that register standard commands provided by the
    // libraries we depend on
    WidgetsWx_RegisterCommands(cmd_reg, provider);

    // now register our own application commands
    cmd_reg.RegisterCommand(eCmdCreateOpenGLView, "Create OpenGL View", "Create OpenGL View",
                              "", "");
    cmd_reg.RegisterCommand(eCmdCreateTaskView, "Create Task View", "Create Task View",
                              "", "");
    cmd_reg.RegisterCommand(eCmdCreateEventView, "Create Event View", "Create Event View",
                              "", "");
    cmd_reg.RegisterCommand(eCmdCreateDiagConsole, "Create Diagnostic Console", "Create Diagnostic Console",
                              "", "");
    /*cmd_reg.RegisterCommand(eCmdCreateTextCtrl, "Create Text View", "Create Text View",
                              "Creates and instance of Text View", "");
    cmd_reg.RegisterCommand(eCmdCreateHtmlView, "Create HTML View", "Create HTML View",
                              "Creates an instance of HTML View", "");
    cmd_reg.RegisterCommand(eCmdCreateClockView, "Create Clock View", "Create Clock View","", "");*/

    cmd_reg.RegisterCommand(eCmdCloseView, "Close View", "Close View",
                              "", "");
    cmd_reg.RegisterCommand(eCmdCloseAllViews, "Close All Views", "Close All Views",
                              "", "");
    cmd_reg.RegisterCommand(eCmdSaveLayout, "Save Layout", "Save Layout",
                              "", "");
    cmd_reg.RegisterCommand(eCmdRestoreLayout, "Restore Layout", "Restore Layout",
                              "", "");
    cmd_reg.RegisterCommand(eCmdTestDiagPanel, "Test Diagnostic Console", "",
                              "", "");
    cmd_reg.RegisterCommand(eCmdTestStatusProgress, "Test Status Progress", "",
                              "", "");
}


// the preferred way to defines statc menus is using static declarations (see below)
WX_DEFINE_MENU(kMainMenu)
    WX_SUBMENU("&File")
        WX_MENU_ITEM(wxID_EXIT)
    WX_END_SUBMENU()
    WX_SUBMENU("&Edit")
        WX_MENU_ITEM(wxID_CUT)
        WX_MENU_ITEM(wxID_COPY)
        WX_MENU_ITEM(wxID_PASTE)
    WX_END_SUBMENU()
    WX_SUBMENU("&View")
        WX_MENU_ITEM(eCmdCreateOpenGLView)
        WX_MENU_CHECK_ITEM(eCmdCreateDiagConsole)
        WX_MENU_CHECK_ITEM(eCmdCreateTaskView)
        WX_MENU_CHECK_ITEM(eCmdCreateEventView)
        //WX_MENU_ITEM(eCmdCreateTextCtrl)
        //WX_MENU_ITEM(eCmdCreateHtmlView)
    WX_END_SUBMENU()
    WX_SUBMENU("&Window")
        WX_MENU_SEPARATOR_L("Actions")
        WX_MENU_ITEM(eCmdCloseView)
        WX_MENU_ITEM(eCmdCloseAllViews)
        WX_MENU_SEPARATOR_L("Actions2")
        WX_MENU_ITEM(eCmdSaveLayout)
        WX_MENU_ITEM(eCmdRestoreLayout)
        WX_MENU_SEPARATOR_L("Windows")
    WX_END_SUBMENU()
WX_END_MENU()


const wxMenu* CFrameworkDemoGUI::GetMenu()
{
    //  Use CUICommandRegistry to create the menu from the static definition
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    wxMenu* menu = cmd_reg.CreateMenu(kMainMenu);
    return menu;
}


/*void CFrameworkDemoGUI::UpdateMenuBar(wxMenu& menu)
{
}*/


void CFrameworkDemoGUI::x_AddClient(IWMClient* client)
{
    IViewManagerService* manager = m_Workbench->GetViewManagerService();
    IView* view = dynamic_cast<IView*>(client);
    if(view)    {
        manager->AddToWorkbench(*view);
    }
}


void CFrameworkDemoGUI::x_AddView(const string& view_ui_name)
{
    IViewManagerService* manager = m_Workbench->GetViewManagerService();

    CIRef<IView> view = manager->CreateViewInstance(view_ui_name);

    if(view)    {
        /// create wxWindow for the view
        view->CreateViewWindow(x_GetMainWindow());
        manager->AddToWorkbench(*view);
    }
}


void CFrameworkDemoGUI::x_ShowSingletonView(const string& view_ui_name)
{
    IViewManagerService* manager = m_Workbench->GetViewManagerService();

    IView* view = manager->GetSingletonView(view_ui_name);
    if(view)    {
        // activate existing view
        IWindowManagerService* srv = m_Workbench->GetWindowManagerService();
        IWMClient* client = dynamic_cast<IWMClient*>(view);

        _ASSERT(client);

        srv->ActivateClient(*client);
    } else {
        // create a new view
        x_AddView(view_ui_name);
    }
}


wxWindow* CFrameworkDemoGUI::x_GetMainWindow()
{
    return m_Workbench->GetMainWindow();
}


void CFrameworkDemoGUI::OnCreateClockViewClick(wxCommandEvent& event)
{
    x_AddView("Clock View");
}


/*void CFrameworkDemoGUI::OnCreateHTMLViewClick(wxCommandEvent& event)
{

    _ASSERT(false); // not implemented
}


void CFrameworkDemoGUI::OnCreateTreeControlClick(wxCommandEvent& event)
{
    _ASSERT(false); // not implemented
}


*void CFrameworkDemoGUI::OnCreateTextCtrlClick(wxCommandEvent& event)
{
    _ASSERT(false); // not implemented/
}*/


void CFrameworkDemoGUI::OnCreateOpenglViewClick(wxCommandEvent& event)
{
    x_AddView("OpenGL Test View");
}


void CFrameworkDemoGUI::OnShowDiagnosticsConsoleClick(wxCommandEvent& event)
{
    x_ShowSingletonView("Diagnostic Console");
}


void CFrameworkDemoGUI::OnCreateEventViewClick(wxCommandEvent& event)
{
    x_ShowSingletonView("Event View");
}


void CFrameworkDemoGUI::OnCreateTaskViewClick(wxCommandEvent& event)
{
    x_ShowSingletonView("Task View");
}


void CFrameworkDemoGUI::OnCloseViewClick(wxCommandEvent& event)
{
    IWindowManagerService* srv = m_Workbench->GetWindowManagerService();
    IWMClient* client = srv->GetActiveClient();
    if(client)  {
        srv->CloseClient(*client);
    }
}


void CFrameworkDemoGUI::OnCloseAllViewsClick(wxCommandEvent& event)
{
    x_CloseAllViews();
}


void CFrameworkDemoGUI::x_CloseAllViews()
{
    _ASSERT(m_Workbench);

    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    vector<IWMClient*> clients;
    wm_srv->GetAllClients(clients);

    OnCloseClientsRequest(clients);
}


void CFrameworkDemoGUI::OnTestDiagPanelClick(wxCommandEvent& event)
{
    m_DiagnosticsTesting = !m_DiagnosticsTesting;
    if (m_DiagnosticsTesting)
        m_Timer.Start(100);
    else
        m_Timer.Stop();
}



void CFrameworkDemoGUI::OnTestDiagPanelUpdate( wxUpdateUIEvent& event )
{
    wxString text = (m_DiagnosticsTesting) ?
        _("Stop Diagnostics Testing") : _("Start Diagnostics Testing");
    event.SetText(text);
}


void CFrameworkDemoGUI::OnDiagnosticsTimer(wxTimerEvent& event)
{
    static int counter = 1;
    int val = (rand()%7);
    if (val&4) {
        switch(val&3) {
        case 3:
            wxLogDebug(wxT("wxWidgets Debug %d"), counter);
            break;
        case 2:
            wxLogMessage(wxT("wxWidgets Info %d"), counter);
            break;
        case 1:
            wxLogWarning(wxT("wxWidgets Warning %d"), counter);
            break;
        default:
            wxLogError(wxT("wxWidgets Error %d"), counter);
            break;
        }
    }
    else {
        CNcbiDiag diag;
        switch(val&3) {
        case 3:
            diag << Trace;
            diag << "NCBI Trace " << counter << Endm;
            break;
        case 2:
            diag << Info;
            diag << "NCBI Info " << counter << Endm;
            break;
        case 1:
            diag << Warning;
            diag << "NCBI Warning " << counter << Endm;
            break;
        default:
            diag << Error;
            diag << "NCBI Error " << counter << Endm;
            break;
        }
    }
    counter++;
}


void CFrameworkDemoGUI::OnStatusTimer(wxTimerEvent& event)
{
    IStatusBarService* sb_srv = m_Workbench->GetStatusBarService();

    if(m_ProgressStage <= 10)  {
        if(m_ProgressStage == 0)    { // just starting
            sb_srv->SetStatusMessage("Testing progress bar - regular mode");
        }

        sb_srv->ShowStatusProgress(m_ProgressStage, 10);
        m_ProgressStage++;
    } else if(m_ProgressStage < 20)   {
        if(m_ProgressStage == 11)    { // just starting pulse mode
            sb_srv->SetStatusMessage("Testing progress bar - pulse mode");

        }
        sb_srv->ShowStatusProgress(); // call this on every tick
        m_ProgressStage++;
    } else {
        m_StatusTimer.Stop();
        sb_srv->HideStatusProgress();
        sb_srv->SetStatusMessage("");

        m_ProgressStage = -1;
    }
}


void CFrameworkDemoGUI::OnCloseClientsRequest(const TClients& clients)
{
    /// close clients unconditionally
    //IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    //wm_srv->CloseClients(clients);
    IViewManagerService* manager = m_Workbench->GetViewManagerService();
    for( size_t i = 0;  i < clients.size();  i++ )    {
        IWMClient* client = clients[i];
        IView* view = dynamic_cast<IView*>(client);
        if(view)    {
            manager->RemoveFromWorkbench(*view);
        }
    }
}


void CFrameworkDemoGUI::OnClientAboutToClose(IWMClient& client)
{
    LOG_POST(Info << "CFrameworkDemoGUI::OnClientAboutToClose()");
}


void CFrameworkDemoGUI::OnClientClosed(IWMClient& client)
{
    LOG_POST(Info << "CFrameworkDemoGUI::OnClientClosed()");
}


void CFrameworkDemoGUI::OnActiveClientChanged(IWMClient* new_active)
{
    LOG_POST(Info << "CFrameworkDemoGUI::OnActiveClientChanged()");
}


string CFrameworkDemoGUI::GetFloatingFrameTitle(int index)
{
    return GetAppTitle();
}


void CFrameworkDemoGUI::ActivateClient(IWMClient* client)
{
    if(client)  {
        IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
        wm_srv->ActivateClient(*client);
    }
}


void CFrameworkDemoGUI::OnSaveLayoutClick(wxCommandEvent& event)
{
    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    m_Layout.Reset(wm_srv->SaveLayout());
}


void CFrameworkDemoGUI::OnSaveLayoutUpdate( wxUpdateUIEvent& event )
{
}


void CFrameworkDemoGUI::OnRestoreLayoutClick(wxCommandEvent& event)
{
    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    IViewManagerService* view_mrg = m_Workbench->GetViewManagerService();
    IWMClientFactory* factory = dynamic_cast<IWMClientFactory*>(view_mrg);

    _ASSERT(wm_srv  &&  factory);

    wm_srv->CloseAllClients();
    wm_srv->LoadLayout(*m_Layout, *factory);
}


void CFrameworkDemoGUI::OnRestoreLayoutUpdate( wxUpdateUIEvent& event )
{
    event.Enable(m_Layout.NotEmpty());
}


void CFrameworkDemoGUI::OnTestStatusProgress(wxCommandEvent& event)
{
    m_ProgressStage = 0;
    m_StatusTimer.Start(250);
}


void CFrameworkDemoGUI::OnTestStatusProgressUpdate(wxUpdateUIEvent& event)
{
    event.Enable(m_ProgressStage < 0);
}


///////////////////////////////////////////////////////////////////////////////
/// CFrameworkDemoToolBarFactory

void CFrameworkDemoToolBarFactory::RegisterImageAliases(wxFileArtProvider& provider)
{
    provider.RegisterFileAlias(wxT("about"), wxT("about.png"));
    // other icons already registered
}


static const char* kTBViews = "Views";

void CFrameworkDemoToolBarFactory::GetToolBarNames(vector<string>& names)
{
    names.push_back(kTBViews);
}


wxAuiToolBar* CFrameworkDemoToolBarFactory::CreateToolBar(const string& name, wxWindow* parent)
{
    if(name == kTBViews) {
        long style = wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT | wxTB_HORZ_LAYOUT | wxTB_NODIVIDER;
        wxAuiToolBar* toolbar = new wxAuiToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
        toolbar->SetMargins(4, 4);

        /*toolbar->AddTool(ID_CREATE_TEXT_CTRL, wxT("Text Ctrl"), wxArtProvider::GetBitmap("menu::about"));
        toolbar->AddTool(ID_CREATE_HTML_VIEW, wxT("HTML View"), wxArtProvider::GetBitmap("menu::search"));
        toolbar->AddTool(ID_CREATE_TREE_CONTROL, wxT("Tree Ctrl"), wxArtProvider::GetBitmap("menu::zoom_in"));
        toolbar->AddTool(ID_CREATE_CLOCK_VIEW, wxT("Clock View"), wxArtProvider::GetBitmap("menu::zoom_out"));*/
        toolbar->AddTool(eCmdCreateOpenGLView, wxT("OpenGL View"),
                         wxArtProvider::GetBitmap(wxT("menu::zoom_all")));

        toolbar->Realize();
        return toolbar;
    }
    return NULL;
}



END_NCBI_SCOPE
