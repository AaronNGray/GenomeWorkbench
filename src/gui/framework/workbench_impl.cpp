/*  $Id: workbench_impl.cpp 34721 2016-02-04 14:05:23Z katargir $
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

#include <wx/platform.h>

#include <gui/framework/workbench_impl.hpp>

#include <gui/framework/event_log_service_impl.hpp>
#include <gui/framework/status_bar_service_impl.hpp>
#include <gui/framework/window_manager_service_impl.hpp>
#include <gui/framework/menu_service_impl.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/view_manager_service_impl.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/app_status_bar.hpp>
#include <gui/widgets/wx/fileartprov.hpp>

#include <gui/widgets/wx/sys_path.hpp>
#include <gui/utils/extension_impl.hpp>

#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/// TODO
/*
current implementation does not have robust protection against NULL pointers.
This is done intentionaly, as it is not clear what policy we should implement –
throw exceptions, ignore incorrect inputs, return boolean values indicating success etc.
*/

///////////////////////////////////////////////////////////////////////////////
/// CWorkbench


CWorkbench::CWorkbench(IWorkbenchAdvisor* advisor)
:   m_Advisor(advisor),
    m_FileArtProvider(NULL),
    m_MainFrame(NULL),
    m_EventLogService(NULL),
    m_MenuService(NULL),
    m_StatusBarService(NULL),
    m_WindowManagerService(NULL),
    m_AppTaskService(NULL),
    m_ViewManagerService(NULL)
{
    //LOG_POST(Info << "CWorkbench constructor");
    _ASSERT(advisor);
}


CWorkbench::~CWorkbench()
{
    //LOG_POST(Info << "CWorkbench destructor");
}


/// register extra i.e. application-defined service
/// the recommended way is to used interface class name (typeid(interface).name())
/// as the name of a service
void CWorkbench::RegisterService(const string& class_name, IService* service)
{
    TNameToService::iterator it = m_NameToExtraService.find(class_name);

    if(it != m_NameToExtraService.end())    {
        _ASSERT(false);
        string msg = "Service with name \"" + class_name +
                     "\" is already registered in Workbench";
        NCBI_THROW(CException, eUnknown,  msg);
    } else {
        /// if this service implements IRegSettings - set its registry path
        IRegSettings* rs = dynamic_cast<IRegSettings*>(service);
        if(rs)  {
            string reg_path = GetRegistrySectionPath(eServices);
            rs->SetRegistryPath(reg_path);
        }

        // provide a reference to Service Locator if needed
        Consumer_SetLocator(service, this);

        // initialize the service and save the reference
        service->InitService();
        m_NameToExtraService[class_name] = CIRef<IService>(service);
    }
}


void CWorkbench::UnRegisterService(const string& class_name)
{
    TNameToService::iterator it = m_NameToExtraService.find(class_name);

    if(it == m_NameToExtraService.end())    {
        _ASSERT(false);
        string msg = "Service with name \"" + class_name +
                     "\" is not registered in Workbench";
        NCBI_THROW(CException, eUnknown,  msg);
    } else {
        IService* service = it->second.GetPointer();
        service->ShutDownService();

        // disconnect from the Service Locator
        Consumer_SetLocator(service, NULL);

        // reset registry path to prevent service from using it
        IRegSettings* rs = dynamic_cast<IRegSettings*>(service);
        if(rs)  {
            rs->SetRegistryPath("");
        }

        m_NameToExtraService.erase(it); // remove the entry from the map
    }
}


void CWorkbench::Init()
{
    LOG_POST(Info << "Initializing Workbench...");

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    x_CreateFileArtProvider();

    // Create Event Logging Service
    m_EventLogService = new CEventLogService();
    RegisterService(typeid(CEventLogService).name(), m_EventLogService);

    x_InitMainFrame();

    // Status Bar Service
    m_StatusBarService = new CStatusBarService();
    m_StatusBarService->SetEventDisplayTime(5);
    RegisterService(typeid(IStatusBarService).name(), m_StatusBarService);
    m_StatusBarService->CreateStatusBar(m_MainFrame);

    // Menu Service
    m_MenuService = new CMenuService();
    m_MenuService->SetFrame(m_MainFrame, cmd_reg);
    RegisterService(typeid(IMenuService).name(), m_MenuService);

    // Window Manager
    m_WindowManagerService = new CWindowManagerService();
    m_WindowManagerService->SetFrame(m_MainFrame);
    RegisterService(typeid(IWindowManagerService).name(), m_WindowManagerService);

    /// give the application a chance to initialize its part of GUI
    m_Advisor->PostCreateWindow();

    /// connect Window Manager Service and Menu Service
    m_MenuService->AddContributor(m_WindowManagerService);
    m_WindowManagerService->SetMenuService(m_MenuService);

    m_MainFrame->Show();

    // View Manager Service
    m_ViewManagerService = new CViewManagerService();
    m_ViewManagerService->SetWorkbench(this);
    RegisterService(typeid(IViewManagerService).name(), m_ViewManagerService);

    // Application Task Service
    m_AppTaskService = new CAppTaskService();
    RegisterService(typeid(CAppTaskService).name(), m_AppTaskService);

    m_Advisor->PostInit();

    LOG_POST(Info << "Finished initializing Workbench");
}


void CWorkbench::ShutDown()
{
    {{
    CIRef<IStatusBarService> sb_srv = GetServiceByType<IStatusBarService>();
    sb_srv->SetStatusMessage("Shutdown initiated.");
    }}

    LOG_POST(Info << "Shutting down Workbench...");
    _ASSERT(m_MainFrame);

    m_Advisor->PreShutDown();

    if(m_MainFrame)   {
        UnRegisterService(typeid(IViewManagerService).name());

        m_Advisor->PreDestroyWindow();

        m_WindowManagerService = NULL;
        UnRegisterService(typeid(IWindowManagerService).name());

        m_MenuService = NULL;
        UnRegisterService(typeid(IMenuService).name());

        x_DestroyMainFrame();

        m_FileArtProvider = NULL; // the instance will be destroyed automatically
    }

    x_ShutDownServices();

    m_AppTaskService = NULL;
    m_ViewManagerService = NULL;
    m_StatusBarService = NULL;
    m_EventLogService = NULL;

    LOG_POST(Info << "Finished shutting down Workbench");
}


bool CWorkbench::CanShutDown()
{
    //TODO ask View Manager to check whether we can Shut Down Data Views
    return m_Advisor->CanShutDown();
}

IEventLogService* CWorkbench::GetEventLogService()
{
    return m_EventLogService;
}


IStatusBarService* CWorkbench::GetStatusBarService()
{
    return m_StatusBarService;
}


IWindowManagerService* CWorkbench::GetWindowManagerService()
{
    return m_WindowManagerService;
}


CUICommandRegistry& CWorkbench::GetUICommandRegistry()
{
    return CUICommandRegistry::GetInstance();
}


// Registry keys
static const char* kMainFrameSection = ".MainFrame";


string CWorkbench::GetRegistrySectionPath(ERegistrySection section)
{
    switch(section) {
    case eServices:
        return m_RegPath + ".Services";
    case eDialogs:
        return m_RegPath + ".Dialogs";
    case eTools:
        return m_RegPath + ".Tools";
    default:
        _ASSERT(false);
        return "";
    }
}

IMenuService* CWorkbench::GetMenuService()
{
    return m_MenuService;
}


IToolBarService* CWorkbench::GetToolBarService()
{
    return m_WindowManagerService;
}


CAppTaskService* CWorkbench::GetAppTaskService()
{
    return m_AppTaskService;
}


IViewManagerService* CWorkbench::GetViewManagerService()
{
    return m_ViewManagerService;
}


wxFileArtProvider& CWorkbench::GetFileArtProvider()
{
    _ASSERT(m_FileArtProvider);
    return *m_FileArtProvider;
}


wxFrame* CWorkbench::GetMainWindow()
{
    return m_MainFrame;
}


IServiceLocator* CWorkbench::GetServiceLocator()
{
    return this;
}


bool CWorkbench::HasService(const string& name)
{
    TNameToService::iterator it = m_NameToExtraService.find(name);
    return it != m_NameToExtraService.end();
}


CIRef<IService> CWorkbench::GetService(const string& name)
{
    TNameToService::iterator it = m_NameToExtraService.find(name);
    if(it != m_NameToExtraService.end())    {
        return it->second;
    } else {
        return CIRef<IService>();
    }
}


void CWorkbench::ShowCommandHint(const string& text)
{
    m_StatusBarService->ShowHintMessage(text);
}


void CWorkbench::HideCommandHint()
{
    if (m_StatusBarService) {
        m_StatusBarService->HideHintMessage();
    }
}


void CWorkbench::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}


void CWorkbench::LoadSettings()
{
}


void CWorkbench::SaveSettings() const
{
}


///////////////////////////////////////////////////////////////////////////////
///
void CWorkbench::x_CreateFileArtProvider()
{
    wxArtProvider::Push( new CwxSplittingArtProvider() );

    wxString dir = CSysPath::ResolvePath( wxT("<res>") );
    InitDefaultFileArtProvider( dir );

    m_FileArtProvider = GetDefaultFileArtProvider();
}


/// create application Main Frame
void CWorkbench::x_InitMainFrame()
{
    string title = m_Advisor->GetAppTitle();

    /// Create main Frame window
    m_MainFrame = x_CreateMainFrame(title);
    _ASSERT(m_MainFrame);

    m_MainFrame->SetRegistryPath(m_RegPath + kMainFrameSection);
    m_MainFrame->LoadSettings();
}


/// destroy application Main Frame
void CWorkbench::x_DestroyMainFrame()
{
    m_MainFrame->SaveSettings();

    m_MainFrame->Destroy();
    m_MainFrame = NULL; // will be deleted by the system
}


CWorkbenchFrame* CWorkbench::x_CreateMainFrame(const string& title)
{
    wxPoint def_pos(0, 0);
    wxSize  def_size(600, 500);
    m_MainFrame = new CWorkbenchFrame(NULL, wxID_ANY, ToWxString(title),
                                      def_pos, def_size);
    m_MainFrame->SetWorkbench(this);
    m_MainFrame->SetHintListener(this);

    wxFileArtProvider& provider = GetFileArtProvider();
    provider.RegisterFileAlias(wxT("frame::icon"), wxT("ncbilogo.ico"));

    wxIcon icon = provider.GetIcon(wxT("frame::icon"));
    if (icon.IsOk())
        m_MainFrame->SetIcon(icon);

    return m_MainFrame;
}


/// free resources associated with Extra Service Locator
void CWorkbench::x_ShutDownServices()
{
    LOG_POST(Info << "Shutting down services...");

    NON_CONST_ITERATE(TNameToService, it, m_NameToExtraService) {
        IService& service = *it->second;
        service.ShutDownService();

        /// disconnect the service from Service Locator
        Consumer_SetLocator(&service, NULL);
    }
    m_NameToExtraService.clear();

    LOG_POST(Info << "Finished shutting down services");
}

/// filter events; forward command events to Window Manager in order to
/// deliver commands to active windows
bool CWorkbench::ProcessEvent(wxEvent& event)
{
    static wxEventType inEvent = wxEVT_NULL;
    if (inEvent == event.GetEventType())
        return false;
    inEvent = event.GetEventType();

    bool res = false;

    if(m_WindowManagerService  &&  event.IsCommandEvent())  {
        wxEventType type = event.GetEventType();
        if(type == wxEVT_UPDATE_UI  ||  type == wxEVT_COMMAND_MENU_SELECTED)    {
           // let Window Manager to process or deliver this command
           wxCommandEvent* cmd_evt = dynamic_cast<wxCommandEvent*>(&event);
           res = m_WindowManagerService->OnCommandEvent(*cmd_evt);
        }
    }
    if( ! res)  {
        res = wxEvtHandler::ProcessEvent(event);
    }

    inEvent = wxEVT_NULL;
    return res;
}


END_NCBI_SCOPE
