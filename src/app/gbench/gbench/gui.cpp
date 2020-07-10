/*  $Id: gui.cpp 44935 2020-04-21 20:16:21Z asztalos $
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
#include <util/format_guess.hpp>
#include <util/icanceled.hpp>

#include "gui.hpp"
#include "app_services.hpp"
#include <gui/widgets/feedback/feedback.hpp>
#include "aboutdlg.hpp"
#include "app_updater_task.hpp"

#include <gui/core/selection_service_impl.hpp>
#include <gui/core/data_mining_service.hpp>
#include <gui/core/ui_data_source_service.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/project_view_base_impl.hpp>
#include <gui/core/visible_range_service_impl.hpp>
#include <gui/utils/ftp_utils.hpp>
#include <gui/utils/download_job.hpp>
#include <gui/utils/object_loader.hpp>
#include <gui/core/quick_launch_list.hpp>
#include <gui/core/recent_tool_list.hpp>
#include <gui/core/ui_tool_manager.hpp>
#include <gui/core/project_tree_panel.hpp>
#include <gui/core/pt_project.hpp>

#include <gui/widgets/wx/file_extensions.hpp>

#include <gui/utils/app_job_dispatcher.hpp>

#include <gui/core/project_task.hpp> //TODO
#include <gui/core/app_dialogs.hpp>

#include <gui/objutils/visible_range.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/obj_event.hpp>

#include <gui/core/broadcast_settings_dlg.hpp>
#include <gui/widgets/loaders/winmask_files.hpp>

#include <gui/core/commands.hpp>

#include <gui/framework/view_manager_service.hpp>
#include <gui/framework/window_manager_service.hpp>
#include <gui/framework/event_log_service_impl.hpp>
#include <gui/framework/status_bar_service.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/app_job_task.hpp>
#include <gui/framework/status_bar_service_impl.hpp>
#include <gui/framework/pkg_manager.hpp>
#include <gui/framework/gui_package.hpp>
#include <gui/framework/pkg_wb_connect.hpp>
#include <gui/framework/workbench_impl.hpp>

#include <gui/widgets/wx/dock_layout.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/commands.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/glresetdlg.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/log_gbench.hpp>
#include <gui/widgets/wx/wnd_layout_registry.hpp>

#include <gui/opengl/ftglfontmanager.hpp>
#include <gui/opengl/glresmgr.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/user_type.hpp>

#include <gui/widgets/wx/sys_path.hpp>
#include <gui/utils/extension_impl.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>

#include <gui/framework/app_options_dlg.hpp>
#include <gui/framework/app_packages_dlg.hpp>

#include <gui/widgets/macro_edit/edit_macro_dlg.hpp>

#include <gui/objects/WorkspaceFolder.hpp>
#include <gui/core/object_loading_task.hpp>
#include <gui/widgets/loaders/asn_object_loader.hpp>
#include <gui/widgets/loaders/text_align_object_loader.hpp>
#include <gui/widgets/loaders/fasta_object_loader.hpp>
#include <gui/widgets/loaders/assembly_cache.hpp>

#include <objmgr/object_manager.hpp>

#include <wx/menu.h>
#include <wx/app.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/toolbar.h>
#include <wx/aui/auibar.h>
#include <wx/dnd.h>
#include <wx/filename.h>
#include <wx/filedlg.h>
#include <wx/utils.h> 
#include <wx/clipbrd.h>
#include <wx/sstream.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CGBenchGUIDropTarget
///
class CGBenchGUIDropTarget : public wxDropTarget
{
public:
    CGBenchGUIDropTarget(CGBenchGUI& gui)
    :   m_GUI(gui)
    {
        SetDataObject(new wxFileDataObject);
    }

    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def)
    {
        if ( !GetData() )
            return wxDragNone;

        wxWindow* wnd = m_GUI.x_GetMainWindow();

        if (wnd && !wnd->IsEnabled())
            return wxDragNone;

        wxFileDataObject* dobj = (wxFileDataObject *)m_dataObject;

        if(dobj  &&  (def == wxDragMove  ||  def == wxDragCopy || def == wxDragLink)) {
            wxArrayString filenames = dobj->GetFilenames();

            vector<wxString> names;
            size_t n = filenames.GetCount();
            names.reserve(n);
            for( size_t i = 0; i < n; i++ ){
                names.push_back (filenames[i]);
            }

            m_GUI.ShowOpenDlg(names);
            return def;
        }
        return wxDragError;
    }
protected:
    CGBenchGUI& m_GUI;
};

///////////////////////////////////////////////////////////////////////////////
/// CGBenchGUI

BEGIN_EVENT_TABLE(CGBenchGUI, wxEvtHandler)
    EVT_MENU( wxID_OPEN, CGBenchGUI::OnFileOpen )
    EVT_MENU( wxID_ABOUT, CGBenchGUI::OnHelpAbout )
    EVT_MENU( eCmdAbout, CGBenchGUI::OnHelpAbout )
    EVT_UPDATE_UI(wxID_OPEN, CGBenchGUI::OnEnableCmdUpdate )
    EVT_UPDATE_UI( wxID_PRINT, CGBenchGUI::OnEnablePrintCmdUpdate )
    EVT_UPDATE_UI( eCmdSaveImages, CGBenchGUI::OnEnableSaveImagesCmdUpdate )
    EVT_UPDATE_UI( eCmdSavePdf, CGBenchGUI::OnEnableSavePdfCmdUpdate )
    EVT_UPDATE_UI(eCmdSaveSvg, CGBenchGUI::OnEnableSaveSvgCmdUpdate)

    EVT_MENU(eCmdImportClipboard, CGBenchGUI::OnImportClipboard)

    EVT_MENU(eCmdCloseWorkspace, CGBenchGUI::OnCloseWorkspace )
    EVT_UPDATE_UI(eCmdCloseWorkspace, CGBenchGUI::OnHasWorskpaceUpdate )

    EVT_MENU(eCmdShowProjectTreeView, CGBenchGUI::OnShowProjectTreeView )
    EVT_MENU(eCmdShowSelectionInspector, CGBenchGUI::OnShowSelectionInspector )
    EVT_MENU(eCmdShowDiagConsole, CGBenchGUI::OnShowDiagnosticsConsole )
    EVT_MENU(eCmdShowEventView, CGBenchGUI::OnShowEventView )
    EVT_MENU(eCmdShowTaskView, CGBenchGUI::OnShowTaskView )
    EVT_MENU(eCmdShowSearchView, CGBenchGUI::OnShowSearchView )

    // always enable these commands
    EVT_UPDATE_UI_RANGE( eCmdShowProjectTreeView, eCmdShowDiagConsole, CGBenchGUI::OnEnableCmdUpdate )

    EVT_MENU( eCmdBroadcastOptions, CGBenchGUI::OnBroadcastOptions )
    EVT_UPDATE_UI( eCmdBroadcastOptions, CGBenchGUI::OnEnableCmdUpdate )

    EVT_MENU( eCmdWindowMaskerFiles, CGBenchGUI::OnWindowMaskerFiles )
    EVT_UPDATE_UI( eCmdWindowMaskerFiles, CGBenchGUI::OnEnableCmdUpdate )

    EVT_MENU(eCmdToolsOptions, CGBenchGUI::OnToolsOptions)
    EVT_MENU(eCmdToolsPackages, CGBenchGUI::OnToolsPackages)

    EVT_MENU(eCmdCloseAllProjectViews, CGBenchGUI::OnCloseAllProjectViews)
    EVT_MENU(eCmdSaveLayout, CGBenchGUI::OnSaveLayout )
    EVT_UPDATE_UI( eCmdSaveLayout, CGBenchGUI::OnSaveLayoutUpdate )
    EVT_MENU(eCmdLoadLayout, CGBenchGUI::OnLoadLayout )
    EVT_MENU(eCmdResetLayout, CGBenchGUI::OnResetLayout )
    EVT_UPDATE_UI( eCmdResetLayout, CGBenchGUI::OnResetLayoutUpdate )

    EVT_MENU(eCmdOpenView, CGBenchGUI::OnOpenView )
    EVT_UPDATE_UI( eCmdOpenView, CGBenchGUI::OnEnableCmdOpen )
    EVT_MENU(eCmdOpenViewDefault, CGBenchGUI::OnOpenViewDefault )

    EVT_MENU(eCmdRunTool, CGBenchGUI::OnRunTool )
    EVT_UPDATE_UI( eCmdRunTool, CGBenchGUI::OnEnableCmdOpen )

    EVT_MENU_RANGE(eCmdQuickLaunchStart, eCmdQuickLaunchEnd,  CGBenchGUI::OnQuickLaunch)
    EVT_UPDATE_UI_RANGE(eCmdQuickLaunchStart, eCmdQuickLaunchEnd, CGBenchGUI::OnEnableCmdOpen)    
    
    EVT_MENU_RANGE(eCmdRecentToolStart, eCmdRecentToolEnd,  CGBenchGUI::OnRecentTool)
    EVT_UPDATE_UI_RANGE(eCmdRecentToolStart, eCmdRecentToolEnd, CGBenchGUI::OnEnableCmdOpen)

    EVT_MENU(eCmdSearch, CGBenchGUI::OnShowSearchView )
    EVT_UPDATE_UI( eCmdSearch, CGBenchGUI::OnEnableCmdUpdate )

    EVT_MENU_RANGE(eCmdHelpStart, eCmdHelpEnd, CGBenchGUI::OnLaunchWebHelp)
    EVT_MENU(eCmdSendFeedback, CGBenchGUI::OnSendFeedback)
    EVT_UPDATE_UI( eCmdSendFeedback, CGBenchGUI::OnEnableCmdUpdate ) // enable always

    EVT_MENU(eCmdCheckForUpdates, CGBenchGUI::OnCheckForUpdates)
    EVT_MENU(eCmdShowPagerMessage, CGBenchGUI::OnShowPagerMessage)

    EVT_MENU(eCmdTestLoadBlastMask, CGBenchGUI::OnTestLoadBlastMask )
    EVT_MENU(eCmdMacroEditDlg, CGBenchGUI::OnMacroEditor)

    EVT_MENU(eCmdExportObject, CGBenchGUI::OnExport)
    EVT_UPDATE_UI(eCmdExportObject, CGBenchGUI::OnUpdateExport)

    EVT_MENU(wxID_SAVE, CGBenchGUI::OnSave )
    EVT_MENU(wxID_SAVEAS, CGBenchGUI::OnSaveAs )
    EVT_UPDATE_UI(wxID_SAVE, CGBenchGUI::OnSaveUpdate )
    EVT_UPDATE_UI(wxID_SAVEAS, CGBenchGUI::OnSaveUpdate )

    EVT_MENU(eCmdDebugAsyncCall, CGBenchGUI::OnDebugAsyncCall)
    EVT_MENU(eCmdLogTestException, CGBenchGUI::OnLogTestException)

    EVT_UPDATE_UI(eCmdBroadcastSel, CGBenchGUI::OnDisableCmdUpdate)
    EVT_UPDATE_UI_RANGE(eCmdLoadProject, eCmdRemoveProject, CGBenchGUI::OnDisableCmdUpdate)
    EVT_UPDATE_UI_RANGE(eCmdBack, eCmdForward, CGBenchGUI::OnDisableCmdUpdate)
    EVT_UPDATE_UI_RANGE(eCmdZoomIn, eCmdSetEqualScale, CGBenchGUI::OnDisableCmdUpdate)
    EVT_UPDATE_UI(eCmdSettings, CGBenchGUI::OnDisableCmdUpdate)


    EVT_MENU(eCmdEnableEditingPackage, CGBenchGUI::OnEnableEditingPackage)
    EVT_UPDATE_UI(eCmdEnableEditingPackage, CGBenchGUI::OnEnableEditingPackageUpdate)

    EVT_TIMER(-1, CGBenchGUI::OnDebugTimer)
END_EVENT_TABLE()


CGBenchGUI::CGBenchGUI() : m_Workbench(), m_MenuService(), m_GBenchGUIApp()
#ifdef _DEBUG
    , m_DebugTimer(this)
#endif
{
#ifdef __WXMAC__
    wxApp::s_macAboutMenuItemId = eCmdAbout;
#endif
}


CGBenchGUI::~CGBenchGUI()
{
}

void CGBenchGUI::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}


void CGBenchGUI::SetWorkbench(CWorkbench* workbench)
{
    if(workbench)   {
        m_Workbench = workbench;
    } else {
        m_Workbench = workbench;
    }
}

static const string kAppTitle("NCBI Genome Workbench");

string CGBenchGUI::GetAppTitle()
{
    string title = kAppTitle;
    string inst = CSysPath::GetInst();
    if (!inst.empty())
        title += " (" + inst + ")";
    return title + " : Main";
}


void CGBenchGUI::x_PushEvtHandler(wxEvtHandler* new_handler)
{
    if( !new_handler ) return;

    wxEvtHandler* old_handler = m_Workbench->GetNextHandler();
    m_Workbench->SetNextHandler(new_handler);
    new_handler->SetPreviousHandler(m_Workbench);

    if (old_handler) {
        new_handler->SetNextHandler(old_handler);
        old_handler->SetPreviousHandler(new_handler);
    }
}


wxEvtHandler* CGBenchGUI::x_PopEvtHandler()
{
    wxEvtHandler* handler = m_Workbench->GetNextHandler();
    if (handler) {
        wxEvtHandler* next_handler = handler->GetNextHandler();

        handler->SetPreviousHandler(NULL);
        m_Workbench->SetNextHandler(next_handler);

        if (next_handler) {
            handler->SetNextHandler(NULL);
            next_handler->SetPreviousHandler(m_Workbench);
        }
    }
    return handler;
}


void CGBenchGUI::PostCreateWindow()
{
    _ASSERT(m_Workbench);

    x_RegisterUserTypes();
    x_RegisterCommands();

    // Connect to Menu Service
    IMenuService* menu_srv = m_Workbench->GetMenuService();
    menu_srv->AddContributor(this);

    // Connect to ToolBar Service
    IToolBarService* tb_srv = m_Workbench->GetToolBarService();
    wxFileArtProvider& provider = m_Workbench->GetFileArtProvider();

    /// Connect Window Manager Service
    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    wm_srv->SetAdvisor(this);

    // create and register a factory for standard toolbars
    CGBenchToolBarFactory* factory = new CGBenchToolBarFactory();
    factory->RegisterImageAliases(provider);
    tb_srv->AddToolBarFactory(factory);

    CProjectViewToolBatFactory* pr_factory = new CProjectViewToolBatFactory();
    tb_srv->AddToolBarFactory(pr_factory);

    x_RegisterPackageContributions();

    menu_srv->ResetMenuBar();
    tb_srv->AddToolBarContext(this);


    // setup D&D handling (does not currently work on mac - wxWidgets issue)
    wxFrame* frame = m_Workbench->GetMainWindow();
    frame->SetDropTarget(new CGBenchGUIDropTarget(*this));

    // Initialize OpenGL font library with directory and device resolution
    wxSize ppi = wxGetDisplayPPI();
    CFtglFontManager::Instance().SetDeviceResolution(ppi.y);
    CFtglFontManager::Instance().SetFontPath(string(CSysPath::GetResourcePath().ToUTF8()));


    x_PushEvtHandler(this);
}


//static const char* kDefLayoutTag = "DefaultLayout";

void CGBenchGUI::PostInit()
{
    // Register application-specific services
    string name = typeid(CUIDataSourceService).name();
    m_Workbench->RegisterService(name, new CUIDataSourceService());

    name = typeid(CProjectService).name();
    CProjectService* prj_srv = new CProjectService();
    m_Workbench->RegisterService(name, prj_srv);

    name = typeid(CSelectionService).name();
    CSelectionService* sel_srv = new CSelectionService();
    m_Workbench->RegisterService(name, sel_srv);

    name = typeid(CDataMiningService).name();
    CDataMiningService* dm_srv = new CDataMiningService();
    m_Workbench->RegisterService(name, dm_srv);

    name = typeid(IVisibleRangeService).name();
    CVisibleRangeService* vr_srv = new CVisibleRangeService();
    m_Workbench->RegisterService(name, vr_srv);

    CWinMaskerFileStorage& storage = CWinMaskerFileStorage::GetInstance();
    storage.SetRegistryPath( m_RegPath + ".WMFStorage" );
    storage.LoadSettings();

    x_SetupStatusBar();
}


void CGBenchGUI::x_SetupStatusBar()
{
    // configure popup window for the standard message slot
    CMessageSlotPopupWindow::SetExtraLink("Show Event View", eCmdShowEventView);

    // add a slot for App Task Service
    IStatusBarService* sb_srv = m_Workbench->GetStatusBarService();

    m_AppSrvSlot = new CAppTaskServiceSlot();
    m_AppSrvSlot->Create(x_GetMainWindow());
    wxSize size = m_AppSrvSlot->GetSize();

    sb_srv->InsertSlot(1, m_AppSrvSlot, size.x);

    m_AppSrvSlot->Show();

    CAppTaskService* task_srv = m_Workbench->GetAppTaskService();
    m_AppSrvSlot->SetAppTaskService(task_srv);

    CSelectionService* sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    sel_srv->InitStatusBar(x_GetMainWindow());
}


void CGBenchGUI::x_ClearStatusBar()
{
    m_AppSrvSlot->SetAppTaskService(0);
    m_AppSrvSlot = NULL; //will be deleted by Status Bar
}


void CGBenchGUI::PreShutDown()
{
    m_Workbench->GetAppTaskService()->PreShutDownService();

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    disp.CancelAllJobs();

    SaveDefaultWindowLayout();

    x_CloseAllViews();

    x_GetMainWindow()->Hide();

    x_ClearStatusBar();

    // Remove fonts from OpenGL texture font singleton
    CFtglFontManager::Instance().Clear();

    // Clear objects from OpenGL resource singleton
    if (CGlResMgr::Initialized())
        CGlResMgr::Instance().Clear();

    CWinMaskerFileStorage& storage = CWinMaskerFileStorage::GetInstance();
    storage.SaveSettings();

    m_Workbench->UnRegisterService(typeid(IVisibleRangeService).name());
    m_Workbench->UnRegisterService(typeid(CSelectionService).name());
    // This is intentionally commented to revoke DL services later
    //
    // un-registration at this point creates a problem, that some running tasks 
    // still hold objects and scopes.    
    // m_Workbench->UnRegisterService(typeid(CUIDataSourceService).name());    

    // Delayed revoke of dataloaders

    m_Workbench->UnRegisterService(typeid(CProjectService).name());
    m_Workbench->UnRegisterService(typeid(CUIDataSourceService).name());

    CAssemblyCache::GetInstance().Stop();
    CEventHandler::ClearPostQueue();
}


void CGBenchGUI::PreDestroyWindow()
{
    // disconnect from event handling, delete the handlers we created
    wxEvtHandler* handler = NULL;
    while( (handler = x_PopEvtHandler()) )  {
        if(handler != this) {
            delete handler;
        }
    }

    // remove self from the list of Tool Bar contexts
    IToolBarService* tb_srv = m_Workbench->GetToolBarService();
    tb_srv->RemoveToolBarContext(this);


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


bool CGBenchGUI::CanShutDown()
{
    return CProjectTask::RemoveAllProjects(m_Workbench);
}

/// reads window layout from the file and applies it to Window Manager
void CGBenchGUI::RestoreWindowLayout()
{
    CConstRef<CUser_object> layout = CWndLayoutReg::GetInstance().GetLayout();
    if (!layout) {
        LOG_POST(Info << "Failed to load the default window layout - installation is corrupted.");
        return;
    }

    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();

    // apply the new layout
    IViewManagerService* view_mrg = m_Workbench->GetViewManagerService();
    IWMClientFactory* factory = dynamic_cast<IWMClientFactory*>(view_mrg);

    _ASSERT(factory);

    wm_srv->LoadLayout(*layout, *factory);

    // On some platforms, (I'm looking at you, Mac..) the initial window may
    // not display until a sizing or docking event, so we send an extra size
    // notification to force display.
    m_Workbench->GetMainWindow()->SendSizeEvent();
    m_Workbench->GetMainWindow()->Refresh();
}

// saves current Window Manager layout to the file
void CGBenchGUI::SaveDefaultWindowLayout()
{
    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    if (!wm_srv) return;

    CRef<CUser_object> layout(wm_srv->SaveLayout());
    CWndLayoutReg::GetInstance().SetLayout(layout);
}


void CGBenchGUI::x_RegisterUserTypes()
{
    /////////////////////////////////////////////////////////////
    // register object symbols
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Sequence, "", "symbol::sequence");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Sequence, CGUIUserType::sm_SbTp_DNA, "symbol::sequence_dna");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Sequence, CGUIUserType::sm_SbTp_Protein, "symbol::sequence_protein");

    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Sequence_ID, "", "symbol::sequence_id");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Sequence_ID, CGUIUserType::sm_SbTp_DNA, "symbol::sequence_id_dna");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Sequence_ID, CGUIUserType::sm_SbTp_Protein, "symbol::sequence_id_protein");

    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Sequence_Set, "", "symbol::sequence_set");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Alignment_Set, "", "symbol::alignment_set");

    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Annotation, "", "symbol::annotation");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Annotation, CGUIUserType::sm_SbTp_Alignments, "symbol::annotation_alignments");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Annotation, CGUIUserType::sm_SbTp_Features, "symbol::annotation_features");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Annotation, CGUIUserType::sm_SbTp_Graphs, "symbol::annotation_graphs");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Annotation, CGUIUserType::sm_SbTp_Sequence_IDs, "symbol::annotation_sequence_ids");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Annotation, CGUIUserType::sm_SbTp_Locations, "symbol::locations");

    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Alignment, "", "symbol::alignment");
    CLabel::RegisterTypeIcon(CGUIUserType::sm_Tp_Feature, "", "symbol::feature");

    wxFileArtProvider& provider = m_Workbench->GetFileArtProvider();

    provider.RegisterFileAlias(wxT("symbol::sequence"), wxT("sequence_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::sequence"), wxT("sequence_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::sequence_dna"), wxT("sequence_dna_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::sequence_protein"), wxT("sequence_protein_symbol.png"));

    provider.RegisterFileAlias(wxT("symbol::sequence_id"), wxT("sequence_id_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::sequence_id_dna"), wxT("sequence_id_dna_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::sequence_id_protein"), wxT("sequence_id_protein_symbol.png"));

    provider.RegisterFileAlias(wxT("symbol::sequence_set"),   wxT("sequence_set_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::alignment_set"),   wxT("alignment_set_symbol.png"));

    provider.RegisterFileAlias(wxT("symbol::alignment"),     wxT("alignment_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::feature"),     wxT("feature_symbol.png"));

    provider.RegisterFileAlias(wxT("symbol::annotation"), wxT("annot_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::annotation_alignments"), wxT("annot_aligns_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::annotation_graphs"), wxT("annot_graphs_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::annotation_features"), wxT("annot_feats_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::annotation_locs"), wxT("annot_locs_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::annotation_ids"),  wxT("annot_ids_symbol.png"));
    provider.RegisterFileAlias(wxT("symbol::locations"), wxT("annot_locs_symbol.png"));

}


void CGBenchGUI::x_RegisterCommands()
{
    CUICommandRegistry& registry = m_Workbench->GetUICommandRegistry();
    wxFileArtProvider& provider = m_Workbench->GetFileArtProvider();

    WidgetsWx_RegisterCommands(registry, provider);
    Core_RegisterCommands(registry, provider);
    Gbench_RegisterCommands(registry, provider);

    wxFrame* frame = m_Workbench->GetMainWindow();
    registry.ApplyAccelerators(frame);
}

void CGBenchGUI::x_RegisterPackageContributions()
{
    if( !m_GBenchGUIApp ) return;

    CPkgManager* pkgManager = m_GBenchGUIApp->GetPackageManager();
    if( !pkgManager ) return;

    CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();
    IMenuService* menu_srv = m_Workbench->GetMenuService();
    IToolBarService* tb_srv = m_Workbench->GetToolBarService();


    ITERATE( CPkgManager, it, *pkgManager ){
        const CPackageInfo& pkg_info = it->second;
        if( pkg_info.m_State != CPackageInfo::eLoaded ){
            if( pkg_info.m_State == CPackageInfo::eValidated ){
                LOG_POST( "The package " << it->first << " is not loaded as not requested." );
            } else {
                ERR_POST( "The package " << it->first << " is not loaded due to some problems." );
            }

            continue;
        }

        IGuiPackage* pkg = pkg_info.m_Package.get();
        if( !pkg ) continue;

        // Should be called before GetEventHandler()
        IPackageWorkbenchConnector* pkg_wb = dynamic_cast<IPackageWorkbenchConnector*>( pkg );
        if (pkg_wb) {
            pkg_wb->SetWorkbench(m_Workbench);
        }


        IUICommandContributor* cmd_ctrb = dynamic_cast<IUICommandContributor*>( pkg );
        if( cmd_ctrb ){
            vector<CUICommand*> cmds = cmd_ctrb->GetCommands();
            NON_CONST_ITERATE( vector<CUICommand*>, pix, cmds ){
                cmd_reg.RegisterCommand( *pix );
            }
            x_PushEvtHandler( cmd_ctrb->GetEventHandler() );
        }


        // Register menu with CMenuService
        IMenuContributor* menu_ctrb = dynamic_cast<IMenuContributor*>( pkg );
        if( menu_ctrb ){
            menu_srv->AddContributor( menu_ctrb );
        }

        // Register menu with CToolbarService

        IToolBarContributor* tb_ctrb = dynamic_cast<IToolBarContributor*>( pkg );
        if( tb_ctrb ){
            tb_srv->AddToolBarFactory( tb_ctrb );
        }
    }
}

static
WX_DEFINE_MENU(kMainMenu)
    WX_SUBMENU("&File")
        WX_MENU_ITEM(wxID_OPEN)
        WX_MENU_ITEM(eCmdImportClipboard)
        WX_MENU_ITEM(eCmdCloseWorkspace)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(wxID_SAVE)
        WX_MENU_ITEM(wxID_SAVEAS)
        WX_MENU_ITEM(eCmdExportObject)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdRemoveProject)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdLoadProject)
        WX_MENU_ITEM(eCmdUnLoadProject)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdSaveImages) 
        WX_MENU_ITEM(eCmdSavePdf) 
        WX_MENU_ITEM(eCmdSaveSvg)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(wxID_EXIT)
    WX_END_SUBMENU()
    WX_SUBMENU("&Edit")
        WX_MENU_ITEM(wxID_UNDO)
        WX_MENU_ITEM(wxID_REDO)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(wxID_CUT)
        WX_MENU_ITEM(wxID_COPY)
        WX_MENU_ITEM(wxID_PASTE)
        WX_MENU_ITEM(wxID_DELETE)
        WX_MENU_ITEM(eCmdEnableEditingPackage)
        //WX_MENU_SEPARATOR()
        //WX_MENU_ITEM(wxID_FIND)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(wxID_SELECTALL)
        WX_MENU_ITEM(wxID_CLEAR)
        WX_MENU_SEPARATOR_L("Contribs")
        WX_MENU_SEPARATOR_L("Properties")
        WX_MENU_ITEM(wxID_PROPERTIES)
    WX_END_SUBMENU()
    WX_SUBMENU("&View")
        WX_MENU_ITEM(eCmdOpenView)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdShowProjectTreeView)
        WX_MENU_ITEM(eCmdShowSearchView)
        WX_MENU_ITEM(eCmdShowSelectionInspector)
        WX_MENU_ITEM(eCmdShowTaskView)
        WX_MENU_ITEM(eCmdShowEventView)
        WX_MENU_ITEM(eCmdShowDiagConsole)
        WX_MENU_SEPARATOR_L("Contribs")
        WX_MENU_SEPARATOR_L("Settings")
        WX_MENU_ITEM(eCmdSettings)
    WX_END_SUBMENU()
    WX_SUBMENU("&Navigate")
        WX_MENU_ITEM(eCmdBack)
        WX_MENU_ITEM(eCmdForward)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdZoomIn)
        WX_MENU_ITEM(eCmdZoomOut)
        WX_MENU_ITEM(eCmdZoomAll)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdZoomSeq)
        WX_MENU_ITEM(eCmdZoomSel)
        WX_MENU_ITEM(eCmdZoomSelObjects)
        WX_MENU_ITEM(eCmdZoomObjects)
        WX_MENU_SEPARATOR()
        WX_SUBMENU("Zoom &Special")
            WX_MENU_ITEM(eCmdZoomInX)
            WX_MENU_ITEM(eCmdZoomOutX)
            WX_MENU_ITEM(eCmdZoomAllX)
            WX_MENU_SEPARATOR()
            WX_MENU_ITEM(eCmdZoomInY)
            WX_MENU_ITEM(eCmdZoomOutY)
            WX_MENU_ITEM(eCmdZoomAllY)
            WX_MENU_SEPARATOR()
            WX_MENU_ITEM(eCmdSetEqualScale)
        WX_END_SUBMENU()
    WX_END_SUBMENU()
    WX_SUBMENU("&Tools")
        WX_MENU_ITEM(eCmdRunTool)
        WX_MENU_ITEM(eCmdSearch)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdBroadcastSel)
        WX_MENU_ITEM(eCmdBroadcastOptions)
        WX_MENU_ITEM(eCmdWindowMaskerFiles)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdToolsPackages)
        WX_MENU_ITEM(eCmdToolsOptions)
    WX_END_SUBMENU()
    WX_MENU_SEPARATOR_L("Contribs")
    WX_MENU_SEPARATOR_L("EndContribs")
    WX_SUBMENU("&Window")
        WX_MENU_SEPARATOR_L("Actions")
        WX_MENU_SEPARATOR_L("Close")
        WX_MENU_ITEM(eCmdCloseAllProjectViews)
        WX_MENU_SEPARATOR_L("Actions2")

        WX_MENU_ITEM(eCmdLoadLayout)
        WX_MENU_ITEM(eCmdSaveLayout)

        WX_MENU_ITEM(eCmdResetLayout)
        WX_MENU_SEPARATOR_L("Windows")
    WX_END_SUBMENU()
#ifdef _DEBUG
    WX_SUBMENU("&Debug")
        WX_MENU_ITEM(eCmdTestLoadBlastMask)
        WX_MENU_ITEM(eCmdMacroEditDlg)
        WX_MENU_ITEM(eCmdDebugAsyncCall)
        WX_MENU_ITEM(eCmdLogTestException)
    WX_END_SUBMENU()
#endif
    // Help has to be last, to work on Mac platform
    WX_SUBMENU("&Help")
        WX_MENU_ITEM(eCmdWebHelpApp)
        WX_MENU_ITEM(eCmdSubmissionsWebHelp)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdSendFeedback)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdCheckForUpdates)
#if defined( __WXMAC__ )
        WX_MENU_ITEM(eCmdAbout)
#else
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(wxID_ABOUT)
#endif
    WX_END_SUBMENU()
WX_END_MENU()


wxMenu* CGBenchGUI::GetMenu()
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    wxMenu* menu = cmd_reg.CreateMenu(kMainMenu);
    x_UpdateQuickLaunchMenu(*menu);

    CWorkbenchFrame* frameWnd = dynamic_cast<CWorkbenchFrame*>(CGBenchGUI::x_GetMainWindow());
    if (frameWnd && frameWnd->HasPagerMessage()) {
        const CUICommand* cmd = CUICommandRegistry::GetInstance().FindCommandByID(eCmdShowPagerMessage);
        if (cmd) {
            wxMenuItem* help_item = FindSubItem(*menu, wxT("&Help"));
            wxMenu* help_menu = help_item ? help_item->GetSubMenu() : nullptr;

            help_menu->FindItem(eCmdCheckForUpdates);
            if (help_menu) {
                size_t pos = 0;
                if (help_menu->FindChildItem(eCmdCheckForUpdates, &pos))
                    help_menu->Insert(pos, wxMenuItem::New(help_menu, cmd->GetCmdID(), cmd->GetMenuLabel(), cmd->GetHint()));
            }
        }
    }

    return menu;
}


void CGBenchGUI::GetCompatibleToolBars(vector<string>& names)
{
    names.push_back("Application Toolbar");
    names.push_back(CProjectViewToolBatFactory::sm_SelTBName);
}


void CGBenchGUI::x_ShowSingletonView(const string& view_ui_name)
{
    IViewManagerService* manager = m_Workbench->GetViewManagerService();
    manager->ShowSingletonView(view_ui_name);
}


void CGBenchGUI::x_AddSystemView(const string& view_ui_name)
{
    IViewManagerService* manager = m_Workbench->GetViewManagerService();
    CIRef<IView> view = manager->CreateViewInstance(view_ui_name);
    if(view)    {
        manager->AddToWorkbench(*view);
    }
}


wxWindow* CGBenchGUI::x_GetMainWindow()
{
    return m_Workbench->GetMainWindow();
}


void CGBenchGUI::OnFileOpen(wxCommandEvent& event)
{
    CAppDialogs::COpenDialog(m_Workbench);
}

static void s_AddObjects(CWorkbench* workbench, IObjectLoader::TObjects& objects)
{
    CSelectionService* selService = workbench->GetServiceByType<CSelectionService>();
    CProjectService* prjService = workbench->GetServiceByType<CProjectService>();
    if (!prjService->HasWorkspace())
        prjService->CreateNewWorkspace();

    CRef<CGBWorkspace> ws = prjService->GetGBWorkspace();
    if (!ws) return;

    int projectId = -1;
    vector<int> projIds;
    TConstScopedObjects selection;
    selService->GetActiveObjects(selection);
    if (!objects.empty())
        prjService->GetObjProjects(selection, projIds);
    if (!projIds.empty())
        projectId = projIds[0];

    if (projectId == -1) {
        for (CTypeIterator<CGBProjectHandle> it(ws->SetWorkspace()); it; ++it) {
            projectId = (*it).GetId();
            if (projectId >= 0)
                break;
        }
    }

    CRef<CScope> scope;
    if (projectId >= 0) {
        CGBProjectHandle* proj = ws->GetProjectFromId(projectId);
        if (proj)
            scope.Reset(proj->GetScope());
    }

    if (!scope) {
        scope.Reset(new CScope(*CObjectManager::GetInstance()));
        scope->AddDefaults();
    }

    IObjectLoader::TObjects filtered;
    if (scope) {
        CScope::TTSE_Handles tses;
        scope->GetAllTSEs(tses, CScope::eAllTSEs);
        vector<CSeq_id_Handle> idhs;

        for (auto& h : tses) {
            for (CBioseq_CI bioseq_it(h); bioseq_it; ++bioseq_it) {
                idhs.push_back((*bioseq_it).GetAccessSeq_id_Handle());
            }
        }

        for (auto& o : objects) {
            bool duplicate = false;
            CSeq_entry* se = dynamic_cast<CSeq_entry*>(o.GetObjectPtr());
            if (se) {
                for (CTypeIterator<CBioseq> it(*se); it && !duplicate; ++it) {
                    for (auto& id : it->GetId()) {
                        for (auto& idh : idhs) {
                            if (CSeqUtils::Match(CSeq_id_Handle::GetHandle(*id), idh, scope)) {
                                duplicate = true;
                                break;
                            }
                        }

                        if (duplicate)
                            break;
                    }
                }
            }
            if (!duplicate)
                filtered.push_back(o);
        }
    }

    if (filtered.empty())
        return;

    static const char* kFoldername = "Imported from Clipboard";

    CSelectProjectOptions options;
    if (projectId >= 0)
        options.Set_AddToExistingProject(projectId, kFoldername);
    else
        options.Set_CreateNewProject(kFoldername);

    CObjectLoadingTask::AddObjects(workbench, filtered, 0, options);
}

static const wxChar* szCcaption = wxT("Import from Clipboard");

void s_ReportClipboardError(const string& clipboard)
{
    int count = 0;
    bool allClipboard = true;
    string::size_type pos = 0;
    for (;;) {
        string::size_type p = clipboard.find('\n', pos);
        if (p == string::npos) {
            pos = string::npos;
            break;
        }
        pos = p + 1;
        if (++count == 10) {
            allClipboard = false;
            break;
        }
    }

    wxString part = wxString::FromUTF8(clipboard.substr(0, pos).c_str());
    wxString msg = wxT("Failed to parse the clipboard data");
    if (allClipboard)
        msg += wxT(":\n\n\n");
    else
        msg += wxT(".\nThe first 10 lines of the clipboard data:\n\n\n");

    wxMessageBox(msg + part, szCcaption, wxOK | wxICON_ERROR);
}

void CGBenchGUI::OnImportClipboard(wxCommandEvent&)
{
    string text;
    if (wxTheClipboard->Open()) {
        wxTextDataObject data;
        if (wxTheClipboard->IsSupported(wxDF_UNICODETEXT))
            wxTheClipboard->GetData(data);
        text = data.GetText().ToUTF8();
        wxTheClipboard->Close();
    }

    if (text.empty()) {
        wxMessageBox(wxT("The clipboard doesn't contain text data."), szCcaption, wxOK | wxICON_EXCLAMATION);
        return;
    }

    CTextAlignParams params;
    params.SetMatch(wxT(""));
    params.SetMiddle(wxT("-."));
    CRef<CTextAlignObjectLoader> textAlignLoader(new CTextAlignObjectLoader(params));
    textAlignLoader->Init();

    CFastaLoadParams fastaParams;
    CRef<CFastaObjectLoader> fastaLoader(new CFastaObjectLoader(fastaParams));
    fastaLoader->Init();

    CNcbiIstrstream istr(text.c_str());

    CFormatGuess guess(istr);
    CFormatGuess::EFormat fmt = guess.GuessFormat();

    if (guess.GuessFormat() == CFormatGuess::eTextASN) {
        istr.clear();
        istr.seekg(0);

        CRef<CAsnObjectLoader> asnLoader(new CAsnObjectLoader());
        asnLoader->Init();

        try {
            IObjectLoader::TObjects objects;
            asnLoader->LoadFromStream(istr, objects, eSerial_AsnText);
            if (!objects.empty()) {
                s_AddObjects(m_Workbench, objects);
                return;
            }
        }
        catch (const exception&) {}
        s_ReportClipboardError(text);
        return;
    }

    istr.clear();
    istr.seekg(0);
    try {
        IObjectLoader::TObjects objects; string errMsg;
        textAlignLoader->LoadFromStream(istr, objects, errMsg);
        if (!objects.empty()) {
            s_AddObjects(m_Workbench, objects);
            return;
        }
    }
    catch (const exception&) {}

    if (fmt == CFormatGuess::eFasta) {
        istr.clear();
        istr.seekg(0);
        try {
            IObjectLoader::TObjects objects;
            fastaLoader->LoadFromStream(istr, objects);
            if (!objects.empty()) {
                s_AddObjects(m_Workbench, objects);
                return;
            }
        }
        catch (const exception&) {}
        s_ReportClipboardError(text);
        return;
    }

    s_ReportClipboardError(text);
}

void CGBenchGUI::ShowOpenDlg(vector<wxString>& filenames)
{
    CAppDialogs::COpenDialog(m_Workbench, NcbiEmptyString, filenames);
}


void CGBenchGUI::OnRunTool(wxCommandEvent& event)
{
    CAppDialogs::RunToolDialog(m_Workbench);
}


void CGBenchGUI::OnQuickLaunch(wxCommandEvent& event)
{
    CQuickLaunchList& quickLaunch = CQuickLaunchList::GetInstance();
    if (quickLaunch.IsEmpty())
        return;

    string entryLabel;
    const list<string>& items = quickLaunch.GetItems();
    int cmd = eCmdQuickLaunchStart;
    ITERATE(list<string>, it, items) {
        if (cmd == event.GetId()) {
            entryLabel = *it;
            break;
        }
        ++cmd;
    }

    if (entryLabel.empty())
        return;

    string entriesPath = quickLaunch.GetItemPath(entryLabel);
    string toolLabel = CGuiRegistry::GetInstance().GetString(entriesPath + ".Tool");
    if (toolLabel.empty()) {
        quickLaunch.RemoveItem(entryLabel);
        wxMessageBox(wxT("Invalid entry: ") + ToWxString(entryLabel),
                     wxT("Quick Launch Error"), wxOK | wxICON_ERROR);
        return;
    }


    vector< CIRef<IUIAlgoToolManager> > managers;
    GetExtensionAsInterface("ui_algo_tool_manager", managers);

    CIRef<IUIAlgoToolManager> manager;
    for( size_t i =0; i < managers.size(); i++ ){
        if (managers[i]->GetDescriptor().GetLabel() == toolLabel) {
            manager = managers[i];
            break;
        }
    }

    if (!manager || !manager->CanQuickLaunch()) {
        quickLaunch.RemoveItem(entryLabel);
        wxMessageBox(wxT("Tool not found: ") + ToWxString(toolLabel),
                     wxT("Quick Launch Error"), wxOK | wxICON_ERROR);
        return;
    }

    IRegSettings* rgs = dynamic_cast<IRegSettings*>(manager.GetPointer());
    if(rgs) {
        rgs->SetRegistryPath(entriesPath + ".Params");
        rgs->LoadSettings();
    }

    manager->SetServiceLocator(m_Workbench->GetServiceLocator());

    vector<TConstScopedObjects> objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    sel_srv->GetActiveObjects(objects);

    manager->SetInputObjects(objects);

    try {
        IAppTask* task = manager->QuickLaunch();

        if( task ){
            quickLaunch.AddItem( entryLabel );
            m_Workbench->GetAppTaskService()->AddTask( *task );
        }
    } catch( CException& ex ){
        ERR_POST( ex.ReportAll() );
        NcbiErrorBox( ex.GetMsg() );

    } catch( std::exception& ex ){
        ERR_POST( ex.what() );
        NcbiErrorBox( ex.what() );
    }
}

void CGBenchGUI::OnRecentTool(wxCommandEvent& event)
{
    CRecentToolList& recentTool = CRecentToolList::GetInstance();
    if (recentTool.IsEmpty())
        return;

    string toolLabel;
    const list<string>& items = recentTool.GetItems();
    int cmd = eCmdRecentToolStart;
    ITERATE(list<string>, it, items) {
        if (cmd == event.GetId()) {
            toolLabel = *it;
            break;
        }
        ++cmd;
    }

    if (toolLabel.empty())
        return;

    vector< CIRef<IUIAlgoToolManager> > managers;
    GetExtensionAsInterface("ui_algo_tool_manager", managers);

    CIRef<IUIAlgoToolManager> manager;
    for( size_t i =0; i < managers.size(); i++ ){
        if (managers[i]->GetDescriptor().GetLabel() == toolLabel) {
            manager = managers[i];
            break;
        }
    }

    if (!manager) {
        recentTool.RemoveItem(toolLabel);
        wxMessageBox(wxT("Tool not found: ") + ToWxString(toolLabel),
                     wxT("Recent Tool Error"), wxOK | wxICON_ERROR);
        return;
    }

    CAppDialogs::RunToolDialog(m_Workbench, toolLabel);
}

void CGBenchGUI::x_UpdateQuickLaunchMenu(wxMenu& root_menu)
{
    CQuickLaunchList& quickLaunch = CQuickLaunchList::GetInstance();

    wxMenuItem* tools_item = ncbi::FindSubItem(root_menu, wxT("&Tools"));
    wxMenu* tools_menu = tools_item ? tools_item->GetSubMenu() : NULL;
    if (!tools_menu)
        return;

    wxMenuItemList& items = tools_menu->GetMenuItems();
    size_t pos = 0;
    ITERATE( wxMenuItemList, it, items ){
        if ((*it)->GetId() == eCmdRunTool) {
            quickLaunch.UpdateMenu(*tools_menu, eCmdQuickLaunchStart, eCmdQuickLaunchEnd, pos + 1);
            break;
        }
        ++pos;
    }

    CRecentToolList& recentTool = CRecentToolList::GetInstance();
    recentTool.UpdateMenu(*tools_menu, eCmdRecentToolStart, eCmdRecentToolEnd, pos + 2);
}

void CGBenchGUI::OnOpenView(wxCommandEvent& event)
{
    CAppDialogs::OpenViewlDialog(m_Workbench);
}

void CGBenchGUI::OnOpenViewDefault(wxCommandEvent& event)
{
    CAppDialogs::OpenViewlDialog(m_Workbench, true);
}

void CGBenchGUI::OnCloseWorkspace(wxCommandEvent& event)
{
    CProjectTask::RemoveAllProjects(m_Workbench);
}

void CGBenchGUI::OnHasWorskpaceUpdate(wxUpdateUIEvent& event)
{
    event.Enable(true);
}


void CGBenchGUI::OnEnableCmdUpdate(wxUpdateUIEvent& event)
{
    event.Enable(true);
}

void CGBenchGUI::OnEnableCmdOpen(wxUpdateUIEvent& event)
{
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv)
        sel_srv->GetActiveObjects(objects);
    event.Enable(objects.size() > 0);
}

void CGBenchGUI::OnEnablePrintCmdUpdate(wxUpdateUIEvent& event)
{
    // Disable print by default.  Any widget that wants to support print can
    // add this same event handler and call event.Enable(true);
    event.Enable(false);
}

void CGBenchGUI::OnEnableSaveImagesCmdUpdate(wxUpdateUIEvent& event)
{
    // Disable save images by default.  Any widget that wants to support 
    // save images can add this same event handler and call event.Enable(true);
    event.Enable(false);
}

void CGBenchGUI::OnEnableSavePdfCmdUpdate(wxUpdateUIEvent& event)
{
    // Disable save images by default.  Any widget that wants to support 
    // save images can add this same event handler and call event.Enable(true);
    event.Enable(false);
}

void CGBenchGUI::OnEnableSaveSvgCmdUpdate(wxUpdateUIEvent& event)
{
    // Disable save images by default.  Any widget that wants to support 
    // save images can add this same event handler and call event.Enable(true);
    event.Enable(false);
}

/// default "disabler" for unhandled commands
void CGBenchGUI::OnDisableCmdUpdate(wxUpdateUIEvent& event)
{
    event.Enable(false);
}


void CGBenchGUI::OnShowDiagnosticsConsole(wxCommandEvent& event)
{
    x_ShowSingletonView("Diagnostic Console");
}


void CGBenchGUI::OnShowEventView(wxCommandEvent& event)
{
    x_ShowSingletonView("Event View");
}


void CGBenchGUI::OnShowTaskView(wxCommandEvent& event)
{
    x_ShowSingletonView("Task View");
}


void CGBenchGUI::OnShowProjectTreeView(wxCommandEvent& event)
{
    x_ShowSingletonView("Project View");
}


void CGBenchGUI::OnShowSelectionInspector(wxCommandEvent& event)
{
    x_ShowSingletonView("Active Objects Inspector");
}


void CGBenchGUI::OnShowSearchView(wxCommandEvent& event)
{
    x_ShowSingletonView("Search View");
}


void CGBenchGUI::OnBroadcastOptions(wxCommandEvent& event)
{
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    _ASSERT( sel_srv );

    SBroadcastParams params;
    params.m_BioseqMatching = CSelectionEvent::sm_ObjMatchPolicy;
    params.m_SeqIDMatching = CSelectionEvent::sm_IdMatchPolicy;
    params.m_MatchAlnLocs = CSelectionEvent::sm_MatchAlnLocs;
    params.m_MatchByProduct = CSelectionEvent::sm_MatchByProduct;
    //params.m_AutoSelBroadcast = CSelectionEvent::sm_AutoBroadcast;
    params.m_AutoSelBroadcast = sel_srv->IsAutoBroadcast();
    params.m_BroadcastBetweenProjects = CSelectionEvent::sm_InterDocBroadcast;
    params.m_AutoVRBroadcast = CVisibleRange::IsAutoBroadcast();
    params.m_VRBPolicy = CVisibleRange::GetDefaultVisibleRangePolicy();
    params.m_TreeBroadcastOneToOne = CSelectionEvent::sm_TreeBroadcastOneToOne;
    params.m_TreeBroadcastProperties = CSelectionEvent::sm_TreeBroadcastProperties;

    CBroadcastDialog dlg(x_GetMainWindow());
    dlg.SetData( params );

    if( dlg.ShowModal() == wxID_OK ){ // apply changes
        params = dlg.GetData();

        CSelectionEvent::sm_ObjMatchPolicy
            = (CSelectionEvent::EObjMatchPolicy) params.m_BioseqMatching
        ;
        CSelectionEvent::sm_IdMatchPolicy
            = (CSelectionEvent::EIdMatchPolicy) params.m_SeqIDMatching
        ;
        CSelectionEvent::sm_MatchAlnLocs = params.m_MatchAlnLocs;
        CSelectionEvent::sm_MatchByProduct = params.m_MatchByProduct;
        CSelectionEvent::sm_InterDocBroadcast = params.m_BroadcastBetweenProjects;
        //CSelectionEvent::sm_AutoBroadcast = params.m_AutoSelBroadcast;
        sel_srv->SetAutoBroadcast( params.m_AutoSelBroadcast );

        CVisibleRange::SetAutoBroadcast( params.m_AutoVRBroadcast );
        CVisibleRange::SetDefaultVisibleRangePolicy((CVisibleRange::EBasicPolicy)params.m_VRBPolicy);
        CSelectionEvent::sm_TreeBroadcastOneToOne = params.m_TreeBroadcastOneToOne;
        CSelectionEvent::sm_TreeBroadcastProperties = params.m_TreeBroadcastProperties;
    }
}

void CGBenchGUI::OnWindowMaskerFiles( wxCommandEvent& event )
{
    CWinMaskerFileStorage::GetInstance().ShowOptionsDlg();
}


static const char* kOptionsDlgSection = ".OptionsDialog";

void CGBenchGUI::OnToolsOptions(wxCommandEvent& event)
{
    CAppOptionsDlg dlg(NULL, CAppOptionsDlg::ID_CAPPOPTIONSDLG, wxT("Genome Workbench Options"));
    dlg.SetRegistryPath(m_RegPath + kOptionsDlgSection);
    dlg.ShowModal();
}

void CGBenchGUI::OnToolsPackages(wxCommandEvent& event)
{
    if (!m_GBenchGUIApp) {
        wxMessageBox(wxT("GbenchGUI application not initialized."), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }
    CPkgManager* pkgManager = m_GBenchGUIApp->GetPackageManager();
    if (!pkgManager) {
        wxMessageBox(wxT("Package Manager not initialized."), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }

    CAppPackagesDlg dlg(NULL, pkgManager);
    dlg.SetRegistryPath(m_RegPath + ".PackagesDialog");
    dlg.ShowModal();
}

void CGBenchGUI::OnHelpAbout(wxCommandEvent& event)
{
    CAboutDlg dlg(NULL, -1);
    dlg.ShowModal();
}

static void s_LaunchWebHelp(const string& topic)
{
    ::wxLaunchDefaultBrowser(wxT("https://www.ncbi.nlm.nih.gov/tools/") + ToWxString(topic));
}

void CGBenchGUI::OnLaunchWebHelp(wxCommandEvent& evt)
{
    switch (evt.GetId()) {
    case eCmdWebHelpApp :
        s_LaunchWebHelp("gbench/");
        break;
    case eCmdSubmissionsWebHelp :
        s_LaunchWebHelp("gbench/editing_TOC/");
        break;
    case eCmdWebHelpGSVNavigation:
        s_LaunchWebHelp("gbench/tutorial23/");
        break;
    case eCmdWebHelpGSVInteractionGuide:
        s_LaunchWebHelp("gbench/tutorial1/#Navigating_Graphical_View");
        break;
    case eCmdWebHelpGSVLegends:
        s_LaunchWebHelp("sviewer/legends/");
        break;
    default:
        break;
    }
}

void CGBenchGUI::OnSendFeedback(wxCommandEvent& event)
{
    ReportMemoryUsage();
    CwxLogDiagHandler::GetInstance()->Flush();
    ShowFeedbackDialog(false);
}

void CGBenchGUI::OnCheckForUpdates(wxCommandEvent& event)
{
    CRef<CAppUpdaterTask> vcheck_task (new CAppUpdaterTask(false));
    m_Workbench->GetAppTaskService()->AddTask(*vcheck_task);
}

void CGBenchGUI::OnShowPagerMessage(wxCommandEvent&)
{
    CWorkbenchFrame* frameWnd = dynamic_cast<CWorkbenchFrame*>(CGBenchGUI::x_GetMainWindow());
    if (frameWnd)
        frameWnd->SetShowPagerMessage();
}

void CGBenchGUI::OnCloseAllProjectViews(wxCommandEvent& event)
{
    CProjectService* prj_srv = m_Workbench->GetServiceByType<CProjectService>();
    prj_srv->RemoveAllProjectViews();
}


void CGBenchGUI::x_CloseAllViews()
{
    // get all IWMClients currently opened and request closing
    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    vector<IWMClient*> clients;
    wm_srv->GetAllClients(clients);

    OnCloseClientsRequest(clients);
}

// handles a signal from Window Manager indicating that a client needs to be closed
void CGBenchGUI::OnCloseClientsRequest(const TClients& clients)
{
    /// close clients unconditionally
    IViewManagerService* manager = m_Workbench->GetViewManagerService();
    for( size_t i = 0;  i < clients.size();  i++ )    {
        IWMClient* client = clients[i];
        // use CIRef to postpone destruction
        CIRef<IView> view(dynamic_cast<IView*>(client));
        if(view)    {
            IProjectView* prj_view = dynamic_cast<IProjectView*>(&*view);

            if(prj_view)    {
                // Project Service handles Project Views
                CProjectService* prj_srv = m_Workbench->GetServiceByType<CProjectService>();
                prj_srv->RemoveProjectView(*prj_view);
            } else {
                // System View are handled by View Manager Service
                manager->RemoveFromWorkbench(*view);
            }
        } else {
            _ASSERT(false); // unexpected
        }
    }
}


void CGBenchGUI::OnClientAboutToClose(IWMClient& client)
{
    //LOG_POST("CGBenchGUI::OnClientAboutToClose()");
}


void CGBenchGUI::OnClientClosed(IWMClient& client)
{
    //LOG_POST("CGBenchGUI::OnClientClosed()");

#ifdef __WXOSX_COCOA__
    // Cocoa has a problem that if a view is closed and there is only 1
    // opengl view remaining, it sometimes goes blank and stays that way even
    // after redrawing/swapping/refreshing etc (GB-1898).  The only way I
    // found to fix this is to do a context switch to another view which
    // seems to somehow re-enable the drawing ability.  
    // This code creates a minimal floating OpenGL window and hides it behind
    // the mainwindow so you never actually see it.  But when a window closes,
    // we switch to its context and any remaining gl views that then grab the 
    // context will work.
    wxFrame* f = m_Workbench->GetMainWindow();
    wxPoint p;
    p = f->GetPosition();

    static GlResetDlg* dlg = NULL;

    if (dlg == NULL) {
        dlg = new GlResetDlg(f, p);
    }
     
    ncbi::AddChildWindowBelow(f, dlg); // keeps window invisibile
    dlg->SetPosition(f->GetPosition()); // match upper-left corner of main view

    dlg->Show(true); // user will not see -behind main window always.
    dlg->SwitchContext();
    f->Refresh();
    
    // hide window when we are not using it
    ncbi::RemoveChildWindow(f, dlg); 
    dlg->Hide(); 
#endif
}


string CGBenchGUI::GetFloatingFrameTitle(int index)
{
    string title = kAppTitle;
    string inst = CSysPath::GetInst();
    if (!inst.empty())
        title += " (" + inst + ")";

    return title + " : " + NStr::NumericToString(index);
}


void CGBenchGUI::OnActiveClientChanged(IWMClient* new_active)
{
    //_TRACE("CGBenchGUI::OnActiveClientChanged() " << new_active);

    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();

    if(sel_srv) {
        ISelectionClient* sel_client = dynamic_cast<ISelectionClient*>(new_active);
        sel_srv->OnActiveClientChanged(sel_client);
    }

    // update data mining service
    CIRef<CDataMiningService>  dm_srv =  m_Workbench->GetServiceByType<CDataMiningService>();
    if(dm_srv) {
        IDataMiningContext* sel_client = dynamic_cast<IDataMiningContext*>(new_active);
        dm_srv->OnActiveContextChanged(sel_client);
    }
}


void CGBenchGUI::ActivateClient(IWMClient* client)
{
    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    wm_srv->ActivateClient(*client);
}


void CGBenchGUI::OnSaveLayout(wxCommandEvent& event)
{
    wxFileDialog dlg(
        NULL, wxT("Select a file for window layout"), wxT(""), wxT(""),
        CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") 
        + CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
        wxFD_SAVE|wxFD_OVERWRITE_PROMPT
    );
    if( dlg.ShowModal() != wxID_OK ) return;

    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    CRef<CUser_object> layout( wm_srv->SaveLayout() );
    if (!layout) {
        wxMessageBox( wxT("Genome Workbench has empty layout"), "Error while saving window layout" );
        return;
    }

    try {
        CNcbiOfstream file_to_save( dlg.GetPath().fn_str() );
        if( !file_to_save ){
            wxMessageBox( "Error opening selected file: " + dlg.GetPath(), "File Error" );
            return;
        }

        file_to_save << MSerial_AsnText << *layout;
    } catch( CException& ex ){
        wxMessageBox( ToWxString( ex.GetMsg() ), "Error while saving window layout" );
        return;
    }
}


void CGBenchGUI::OnSaveLayoutUpdate( wxUpdateUIEvent& event )
{
}


void CGBenchGUI::OnLoadLayout(wxCommandEvent& event)
{
    wxFileDialog dlg(
        NULL, wxT("Select a file with window layout"), wxT(""), wxT(""),
        CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") 
        + CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
        wxFD_OPEN
    );
    if( dlg.ShowModal() != wxID_OK ) return;

    CRef<CUser_object> layout (new CUser_object());

    string errMsg;
    try {
        CNcbiIfstream file_to_load( dlg.GetPath().fn_str() );
        if (!file_to_load) {
            wxMessageBox( "Error opening selected file: " + dlg.GetPath(), "File Error" );
            return;
        }
        file_to_load >> MSerial_AsnText >> *layout;
    } catch(const CException& e) {
        errMsg = e.GetMsg();
    } catch(const exception& e) {
        errMsg = e.what();
    }

    if (!errMsg.empty()) {
        wxMessageBox (ToWxString(errMsg), "Error while loading window layout");
        return;
    }

    string videoID;

    if (layout && layout->GetType().IsStr() && layout->GetType().GetStr() == "Dock Manager Layout 2") {
        CConstRef<CUser_field> videoField = layout->GetFieldRef("VideoId");
        if (videoField && videoField->GetData().IsStr()) {
            videoID = videoField->GetData().GetStr();
        }
    }        

    if (videoID.empty()) {
        wxMessageBox (wxT("The file doesn't contain a valid Genome Workbench layout"), "Layout Error" );
        return;
    }

    if (videoID != GetVideoId() &&
        wxYES != wxMessageBox (wxT("The current video configuration differs from the layout.\n")
                               wxT("Do you still wish to apply the layout?"), wxT("Layout Problem"), wxYES_NO|wxICON_EXCLAMATION)) {
            return;
    }

    IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
    wm_srv->ApplyLayout(*layout);
}


void CGBenchGUI::OnResetLayout(wxCommandEvent& event)
{
    CWndLayoutReg& reg = CWndLayoutReg::GetInstance();
    reg.Reset();

    CConstRef<CUser_object> layout = reg.GetDefaultLayout();
    if(layout)  {
        /*{{
            CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
            CRegistryWriteView view = gui_reg.GetWriteView("WindowLayouts2", CGuiRegistry::ePriority_Local - 2);
            CRef<CUser_field> field = view.SetField("Default");
            field->SetData().SetObject(const_cast<CUser_object&>(*layout));
            wxString path(wxT("C:\\TEMP\\def_window_layout.asn"));
            CNcbiOfstream ostr( path.fn_str() );
            CGuiRegistry::GetInstance().Write( ostr , CGuiRegistry::ePriority_Local - 2 );
        }}*/
/*
        wxFrame* mainWnd = m_Workbench->GetMainWindow();
        if (mainWnd) {
            int y = 0;
#ifdef __WXMAC__
            int mbar_height, maxy, maxx;
            GetMacDragWindowBounds(mainWnd, mbar_height, maxy, maxx);
            y = mbar_height;
#endif
            wxRect rc(0, y, 600, 500);
            mainWnd->SetSize(rc);
            mainWnd->Maximize();
        }
 */
        IWindowManagerService* wm_srv = m_Workbench->GetWindowManagerService();
        wm_srv->ApplyLayout(*layout);
    }
}


void CGBenchGUI::OnResetLayoutUpdate(wxUpdateUIEvent& event)
{
}


void CGBenchGUI::OnExport(wxCommandEvent& event)
{
    CAppDialogs::ExportDialog(m_Workbench);
}

void CGBenchGUI::OnUpdateExport(wxUpdateUIEvent& event)
{
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    TConstScopedObjects objects;
    sel_srv->GetActiveObjects(objects);
    event.Enable(!objects.empty());
}

static void s_GetSelectedProjects(IWorkbench* workbench, vector<int>& selProjects)
{
    selProjects.clear();

    if (!workbench) return;
    CProjectService* projectService = workbench->GetServiceByType<CProjectService>();
    if (!projectService) return;
    CProjectTreePanel* projectTreePanel = projectService->GetProjectTreePanel();
    if (!projectTreePanel) return;

    PT::TItems sel_items;
    projectTreePanel->GetSelectedItems(sel_items);

    NON_CONST_ITERATE(PT::TItems, it, sel_items)  {
        if ((*it)->GetType() == PT::eProject) {
            PT::CProject& item = static_cast<PT::CProject&>(**it);
            selProjects.push_back(item.GetData()->GetId());
        }
    }
}

void CGBenchGUI::OnSave(wxCommandEvent& event)
{
    vector<int> selProjects;
    s_GetSelectedProjects(m_Workbench, selProjects);
    CProjectTask::Save(m_Workbench, selProjects, false);
}

void CGBenchGUI::OnSaveAs(wxCommandEvent& event)
{
    vector<int> selProjects;
    s_GetSelectedProjects(m_Workbench, selProjects);
    CProjectTask::Save(m_Workbench, selProjects, true);
}

void CGBenchGUI::OnSaveUpdate(wxUpdateUIEvent& event)
{
    event.Enable(false);
    CProjectService* projectService = m_Workbench->GetServiceByType<CProjectService>();
    if (!projectService) return;
    CRef<CGBWorkspace> ws = projectService->GetGBWorkspace();
    if (!ws) return;
    event.Enable(true);
}

void CGBenchGUI::OnMacroEditor(wxCommandEvent& event)
{
    if (!m_Workbench) return;
    
    CSeq_entry_Handle top_seq_entry;
    CConstRef<CSeq_submit> seq_submit;
    ICommandProccessor* cmd_proccessor = nullptr;

    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) 
        sel_srv->GetActiveObjects(objects);

    if (objects.empty()) {
        wxMessageBox( ToWxString("Make sure a project is loaded and some project item is selected.\n"));
        return;
    }

    for (auto& it : objects) {
        const CSeq_submit* const_sub = dynamic_cast<const CSeq_submit*>(it.object.GetPointer());
        if (const_sub) {
            seq_submit.Reset(const_sub);
        }

        if (!top_seq_entry) {
            CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(it);
            if (seh) {
                top_seq_entry = seh;
                if (seq_submit) {
                    break;
                }
            }
        }
    }

    if (!top_seq_entry) {
        wxMessageBox( ToWxString("Top seq entry was not found.\n"));
        return;
    }
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (srv)
    {
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(top_seq_entry.GetScope()));
        if (doc)
            cmd_proccessor = &doc->GetUndoManager(); 
    }

    CEditMacroDlg dlg(NULL);
    dlg.SetParams(top_seq_entry, seq_submit, cmd_proccessor);
    dlg.ShowModal();
}

static const char* kSequenceEditing = "Sequence Editing";

void CGBenchGUI::OnEnableEditingPackage(wxCommandEvent& event)
{
    CPkgManager* pkgManager = m_GBenchGUIApp->GetPackageManager();
    if (!pkgManager) return;

    vector<string> loadPkgs;

    for (const auto& it : *pkgManager) {
        if (it.second.m_State == CPackageInfo::eLoaded)
            loadPkgs.push_back(it.first);

        if (NStr::EqualNocase(it.first, kSequenceEditing))
            loadPkgs.push_back(it.first);
    }

    pkgManager->SetPackagesToLoad(loadPkgs);

    wxMessageBox(wxT("Changes will take effect when you restart Genome Workbench."),
        SYMBOL_CAPPPACKAGESDLG_TITLE, wxOK | wxICON_INFORMATION);
}

void CGBenchGUI::OnEnableEditingPackageUpdate(wxUpdateUIEvent& event)
{
    CPkgManager* pkgManager = m_GBenchGUIApp->GetPackageManager();
    event.Enable(false);

    if (!pkgManager) return;

    for (const auto& it : *pkgManager) {

        if (NStr::EqualNocase(it.first, kSequenceEditing)) {
            if (it.second.m_State != CPackageInfo::eLoaded)
                event.Enable(true);
            break;
        }
    }
}

void CGBenchGUI::OnTestLoadBlastMask(wxCommandEvent& event)
{
// experimental code
/*
    vector<string> files;
    CFtpUtils::GetDirContent(files, "ftp://ftp.ncbi.nlm.nih.gov/toolbox/gbench/ver-2.1.2");

    string url = "ftp://ftp.ncbi.nlm.nih.gov/toolbox/gbench/ver-2.1.2/gbench-src-2.1.2.tgz";
    CRef<CDownloadJob> djob(new CDownloadJob("Download", "c:\\test", url));
    CRef<CAppJobTask> task(new CAppJobTask(*djob, true, "Download " + url));

    IServiceLocator* srv_loc = m_Workbench->GetServiceLocator();
    CAppTaskService* task_srv = srv_loc->GetServiceByType<CAppTaskService>();

    task_srv->AddTask(*task);
*/  
}

#ifdef _DEBUG
namespace
{
    void testAsyncCall() {
        GUI_AsyncExec([](ICanceled& canceled)
        {
            ::wxSleep(10);
            for (size_t i = 0; !canceled.IsCanceled() && i < 20; ++i)
                ::wxSleep(1);
        }, wxT("Testing AsyncCall..."));
    }
}
#endif

void CGBenchGUI::OnDebugAsyncCall(wxCommandEvent& event)
{
#ifdef _DEBUG
    if (m_DebugTimer.IsRunning())
        m_DebugTimer.Stop();
    else {
        testAsyncCall();
        m_DebugTimer.StartOnce(10000);
    }
#endif
}

class CLogTestException : public CException
{
public:
    NCBI_EXCEPTION_DEFAULT(CLogTestException, CException);
};

void CGBenchGUI::OnLogTestException(wxCommandEvent& event)
{
    try {
        NCBI_THROW(CLogTestException, eUnknown, "Testing exception logging.");
    } NCBI_CATCH("CGBenchGUI::OnLogTestException()");
}

void CGBenchGUI::OnDebugTimer(wxTimerEvent&)
{
#ifdef _DEBUG
    testAsyncCall();
    m_DebugTimer.StartOnce(10000);
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// CGBenchToolBarFactory

void CGBenchToolBarFactory::RegisterImageAliases(wxFileArtProvider& provider)
{
    // we show only standard commands, icons for them shall be registered already
}


static const string kTBApplication = "Application Toolbar";

void CGBenchToolBarFactory::GetToolBarNames(vector<string>& names)
{
    names.push_back(kTBApplication);
}


wxAuiToolBar* CGBenchToolBarFactory::CreateToolBar(const string& name, wxWindow* parent)
{
    if(name == kTBApplication) {

        long style = wxAUI_TB_HORIZONTAL | wxBORDER_NONE;

        wxAuiToolBar* toolbar = new wxAuiToolBar(parent, wxID_ANY, wxDefaultPosition,
                                           wxDefaultSize, style);
                                           
        toolbar->SetName(ToWxString(kTBApplication));

        toolbar->SetToolBitmapSize(wxSize(16, 16));

        CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
        cmd_reg.AppendTool(*toolbar, wxID_OPEN);
        cmd_reg.AppendTool(*toolbar, eCmdImportClipboard);
        cmd_reg.AppendTool(*toolbar, wxID_SAVE);
        toolbar->AddSeparator();
        cmd_reg.AppendTool(*toolbar, eCmdRunTool);
        cmd_reg.AppendTool(*toolbar, eCmdShowSearchView);
        cmd_reg.AppendTool(*toolbar, eCmdShowWindowsDlg);

        toolbar->Realize();
        return toolbar;
    }
    return NULL;
}

END_NCBI_SCOPE
