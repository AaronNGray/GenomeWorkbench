/*  $Id: gb_ui_data_source.cpp 44934 2020-04-21 19:23:25Z asztalos $
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

#include <corelib/ncbiexec.hpp>
#include <corelib/ncbi_process.hpp>
#include <corelib/ncbi_system.hpp>

#include <gui/packages/pkg_sequence/gb_ui_data_source.hpp>

#include <gui/widgets/loaders/gb_load_option_panel.hpp>

#include <gui/core/app_dialogs.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/app_job_task.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/sys_path.hpp>

#include <objmgr/object_manager.hpp>
#include <objtools/data_loaders/genbank/gbloader.hpp>
#include <connect/services/neticache_client.hpp>
#include <objects/gbproj/ProjectItem.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/objutils/label.hpp>
#include <gui/utils/extension_impl.hpp>

#include <gui/widgets/loaders/gb_object_loader.hpp>
#include <gui/widgets/loaders/assembly_object_loader.hpp>
#include <gui/widgets/loaders/chain_loader.hpp>
#include <gui/core/object_loading_task.hpp>
#include <gui/core/pt_data_source.hpp>
#include <gui/core/project_service.hpp>

#include <sra/data_loaders/csra/csraloader.hpp>

#include <wx/menu.h>
#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* kGenBankLoadOption = "Data from GenBank";

///////////////////////////////////////////////////////////////////////////////
/// CGenBankDSEvtHandler - wxEvtHandler-derived adapter for GenBank data source.

class CGenBankDSEvtHandler : public wxEvtHandler
{
    DECLARE_EVENT_TABLE();
public:
    CGenBankDSEvtHandler(IWorkbench* workbench)
        :   m_Workbench(workbench)  {
    }
    void OnLoadFromGenBank(wxCommandEvent& event)
    {
        if(m_Workbench)    {
            CAppDialogs::COpenDialog(m_Workbench, kGenBankLoadOption);
        }
    }
protected:
    IWorkbench* m_Workbench;
};


BEGIN_EVENT_TABLE(CGenBankDSEvtHandler, wxEvtHandler)
    EVT_MENU(eCmdLoadFromGenBank, CGenBankDSEvtHandler::OnLoadFromGenBank)
END_EVENT_TABLE();


///////////////////////////////////////////////////////////////////////////////
/// CGenBankUIDataSource

static const char* kGB_DS_Icon = "icon::gb_data_source";

CGenBankUIDataSource::CGenBankUIDataSource(CGenBankUIDataSourceType& type)
:   m_Type(&type),
    m_Descr("GenBank", kGB_DS_Icon),
    m_SrvLocator(NULL),
    m_Open(false)
{
}

CGenBankUIDataSource::~CGenBankUIDataSource()
{
}


string CGenBankUIDataSource::GetExtensionIdentifier() const
{
    return "genbank_data_source";
}


string CGenBankUIDataSource::GetExtensionLabel() const
{
    return "GenBank Data Source";
}


void CGenBankUIDataSource::SetServiceLocator(IServiceLocator* locator)
{
    m_SrvLocator = locator;
}


IExplorerItemCmdContributor::TContribution
    CGenBankUIDataSource::GetMenu(wxTreeCtrl&, PT::TItems& items)
{
    IExplorerItemCmdContributor::TContribution contrib;

    /// this is not a good solution, but simple
    IWorkbench* workbench = dynamic_cast<IWorkbench*>(m_SrvLocator);

    if(items.size() != 1  ||  workbench == NULL)   {
        return contrib; // return empty object - nothing to contribute
    }

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    PT::CItem* item = items[0];
    int type = item->GetType();

    if(type == PT::eDataSource)    {
        PT::CDataSource* ds_item = dynamic_cast<PT::CDataSource*>(item);
        if(ds_item) {
            CIRef<IUIDataSource> ds = ds_item->GetData();
            CGenBankUIDataSource* gb_ds = dynamic_cast<CGenBankUIDataSource*>(ds.GetPointer());

            if(gb_ds)  {
                wxMenu* menu = new wxMenu;
                menu->Append(wxID_SEPARATOR, wxT("Top Actions"));
                cmd_reg.AppendMenuItem(*menu, eCmdLoadFromGenBank);

                contrib.first = menu;
                contrib.second = new CGenBankDSEvtHandler(workbench);
            }
        }
    }
    return contrib;
}


IUIDataSourceType& CGenBankUIDataSource::GetType() const
{
    return *m_Type;
}


const IUIObject& CGenBankUIDataSource::GetDescr()
{
    return m_Descr;
}


bool CGenBankUIDataSource::IsOpen()
{
    return m_Open;
}

//static const char* kCmdExtPoint = "scoped_objects::cmd_contributor";
static const char* kAppExpCmdExtPoint = "project_tree_view::context_menu::item_cmd_contributor";

bool CGenBankUIDataSource::Open()
{
    if (m_Open) {
        LOG_POST(Error << "CGenBankUIDataSource::Open(): "
            "attempt to open already open data source");
        return false;
    }

    CStopWatch sw;
    sw.Start();

    /// register itself as menu contributor
    CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
    reg->AddExtension(kAppExpCmdExtPoint, *this);

    /// initialize the object manager
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view =
        registry.GetReadView("GBENCH.Services.ObjectManager");

    m_ObjMgr = CObjectManager::GetInstance();

    // general genbank loader options
    int priority  = view.GetInt("Priority", 99);

    // caching options
    string cache_path = FnToStdString(CSysPath::ResolvePath(wxT("<home>/cache")));
    wxString system_cache_path = FnToWxString(cache_path);
    if (sizeof(void*) == 8)
        system_cache_path.append(wxT("64"));

    {{
        /// make sure we can write in the cache path directory
        if ( !wxFileName::DirExists(system_cache_path) ) {
            if ( !wxFileName::Mkdir(system_cache_path) ) {
                LOG_POST(Error << "Error creating cache path ("
                    << system_cache_path.ToUTF8() << "); caching will be disabled");
            } else {
                LOG_POST(Info << "Created cache path: " << system_cache_path.ToUTF8());
            }
        } else {
            // clear the cache after new build is run
            const string ts_key = "GBENCH.Application.LastRunBuildStamp";
            const string st_build_date = __DATE__ "."  __TIME__;
            string last_run_build_date = registry.GetString(ts_key);
            bool cache_reset = (last_run_build_date != st_build_date);

            // clean sqlite cash if previous launch of GBench ended with crash
            if (cache_reset || registry.GetBool("GBENCH.Application.CrashDetected", false)) {
                ::wxRemoveFile(wxFileName(system_cache_path, wxT("cache_ids.db")).GetFullPath());
                ::wxRemoveFile(wxFileName(system_cache_path, wxT("cache_blobs.db")).GetFullPath());
                wxFileName vdb_cache_path = wxFileName::DirName(system_cache_path + wxFileName::GetPathSeparator() + "vdb");
                vdb_cache_path.Rmdir(wxPATH_RMDIR_RECURSIVE);
                vdb_cache_path.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
            }
            registry.Set(ts_key, st_build_date);
        }
    }}

    int cache_age = view.GetInt("CacheAge", 5);
    if (cache_age == 0) {
        cache_age = 5;   // keep objects for 5 days (default)
    }

    /// ID resolution time in hours
    int id_resolution_time = view.GetInt("IdResolutionTime", 24);
    if (id_resolution_time > 24 * 3) {
        id_resolution_time = 24 * 3; //  correct the unreasonable value
    }

    wxString objmgr_config_path =
        CSysPath::ResolvePathExisting(wxT("<home>/gbench-objmgr.ini, ")
                                      wxT("<std>/etc/gbench-objmgr.ini"));

    if (objmgr_config_path.empty()) {
        CGBDataLoader::RegisterInObjectManager(*m_ObjMgr);
    } else {
        CMemoryRegistry reg;
        CNcbiIfstream istr(objmgr_config_path.fn_str(), ios::binary|ios::in);
        reg.Read(istr);


        // override variables as needed
        reg.Set("genbank/cache/id_cache/sqlite3", "database",
                cache_path + "/cache_ids.db");
        reg.Set("genbank/cache/id_cache/sqlite3", "cache_age",
                NStr::IntToString(id_resolution_time * (60*60)));
        reg.Set("genbank/cache/blob_cache/sqlite3", "database",
                cache_path + "/cache_blobs.db");
        reg.Set("genbank/cache/blob_cache/sqlite3", "cache_age",
                NStr::IntToString(cache_age * (24*60*60)));

        // now, create the GenBank data loader
        string msg;
        CGBDataLoader::TRegisterLoaderInfo rinfo;
        try {
            CConfig cfg(reg);
            rinfo =
                CGBDataLoader::RegisterInObjectManager
                    (*m_ObjMgr,
                     *cfg.GetTree(), CObjectManager::eDefault, priority);
        }
        catch (CException& e) {
            msg = "An error occurred while creating the GenBank connection\n(";
            msg += e.GetMsg();
            msg += ")";
        }
        catch (std::exception& e) {
            msg = "An error occurred while creating the GenBank connection\n(";
            msg += e.what();
            msg += ")";
        }

        if ( !msg.empty() ) {
            /// fall back to standard initialization
            LOG_POST(Error << msg);
            CGBDataLoader::RegisterInObjectManager(*CObjectManager::GetInstance());
        }
    }

    string t = NStr::DoubleToString(sw.Elapsed(), 3);
    LOG_POST(Info << "Registered GenBank Data Source  - " << t << " sec");

    sw.Restart();

    CCSRADataLoader::RegisterInObjectManager(*CObjectManager::GetInstance(), CObjectManager::eDefault, 98);
    string loader_name = CObjectManager::GetInstance()->RegisterDataLoader(0, "vdbgraph")->GetName();
    CObjectManager::GetInstance()->SetLoaderOptions(loader_name, CObjectManager::eDefault, 88);
    loader_name = CObjectManager::GetInstance()->RegisterDataLoader(0, "wgs")->GetName();
    CObjectManager::GetInstance()->SetLoaderOptions(loader_name, CObjectManager::eDefault, 88);
    loader_name = CObjectManager::GetInstance()->RegisterDataLoader(0, "snp")->GetName();
    CObjectManager::GetInstance()->SetLoaderOptions(loader_name, CObjectManager::eDefault, 88);

    t = NStr::DoubleToString(sw.Elapsed(), 3);
    LOG_POST(Info << "Registered VDBGraph, WGS and SNP data sources - " << t << " sec");

    m_Open = true;
    return true;
}


bool CGenBankUIDataSource::Close()
{
    if (m_Open) {

        /// remove itself from menu contribution points
        CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
        reg->RemoveExtension(kAppExpCmdExtPoint, *this);

        try {
            bool revoked = m_ObjMgr->RevokeDataLoader("GBLOADER");
            if (!revoked) {
                CDataLoader* dl = m_ObjMgr->FindDataLoader("GBLOADER");
                CGBDataLoader* gbdl = dynamic_cast<CGBDataLoader*>(dl);
                if (gbdl) {
                    gbdl->CloseCache();
                }
            }
        } 
        catch(std::exception&)
        {
            LOG_POST(Error << "Cannot revoke genbank dataloader");
        }

        /// finally, drop the object manager
        CObjectManager* ptr = m_ObjMgr.Release();

        if (ptr) {
            if ( !ptr->ReferencedOnlyOnce() ) {
                LOG_POST(Error << "CGenBankUIDataSource::ShutDownService(): "
                         "object manager still referenced");
            }
        }

        m_Open = false;
        return true;
    }
    return false;
}


void CGenBankUIDataSource::EditProperties()
{
    //TODO
}


IUIToolManager* CGenBankUIDataSource::GetLoadManager()
{
    // TODO may need to link the manager to this particular datasource
    return new CGenBankUILoadManager();
}


int CGenBankUIDataSource::GetDefaultCommand() {
    return eCmdLoadFromGenBank;
}

wxEvtHandler* CGenBankUIDataSource::CreateEvtHandler() {


    IWorkbench* workbench = dynamic_cast<IWorkbench*>(m_SrvLocator);

    return new CGenBankDSEvtHandler( workbench ); 
}



///////////////////////////////////////////////////////////////////////////////
/// CGenBankUIDataSourceType
CGenBankUIDataSourceType::CGenBankUIDataSourceType()
:   m_Descr("GenBank Connection", "")
{
   wxFileArtProvider* provider = GetDefaultFileArtProvider();
   provider->RegisterFileAlias(ToWxString(kGB_DS_Icon),
                               wxT("gb_data_source.png"));
}


const IUIObject& CGenBankUIDataSourceType::GetDescr()
{
    return m_Descr;
}


IUIDataSource* CGenBankUIDataSourceType::CreateDataSource()
{
    return new CGenBankUIDataSource(*this);
}


bool CGenBankUIDataSourceType::AutoCreateDefaultDataSource()
{
    return true; // we want to create default "GenBank" datasource
}


string CGenBankUIDataSourceType::GetExtensionIdentifier() const
{
    static string ext_id("genbank_data_source_type");
    return ext_id;
}


string CGenBankUIDataSourceType::GetExtensionLabel() const
{
    return m_Descr.GetLabel();
}



///////////////////////////////////////////////////////////////////////////////
/// CGenBankUILoadManager
CGenBankUILoadManager::CGenBankUILoadManager()
:   m_SrvLocator(NULL),
    m_ParentWindow(NULL),
    m_Descriptor(kGenBankLoadOption, ""),
    m_State(eInvalid),
    m_OptionPanel(NULL),
    m_ProjectSelPanel(NULL)
{
    m_Descriptor.SetLogEvent("loaders");
}


void CGenBankUILoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CGenBankUILoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CGenBankUILoadManager::GetDescriptor() const
{
    return m_Descriptor;
}


void CGenBankUILoadManager::InitUI()
{
    m_State = eSelectAcc;
}


void CGenBankUILoadManager::CleanUI()
{
    m_State = eInvalid;
    if(m_OptionPanel)   {
        m_OptionPanel = NULL; // window is destroyed by the system
    }
    m_ProjectSelPanel = NULL;
}


wxPanel* CGenBankUILoadManager::GetCurrentPanel()
{
    if(m_State == eSelectAcc)   {
        if(m_OptionPanel == NULL)   {
            m_OptionPanel = new CGenBankLoadOptionPanel(m_ParentWindow);
            LoadSettings();
        }
        return m_OptionPanel;
    }  else if(m_State == eSelectProject)   {
        if(m_ProjectSelPanel == NULL)   {
            CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();

            m_ProjectSelPanel = new CProjectSelectorPanel(m_ParentWindow);
            m_ProjectSelPanel->SetProjectService(srv);
            m_ProjectSelPanel->SetParams(m_ProjectParams);
            m_ProjectSelPanel->TransferDataToWindow();
        }
        return m_ProjectSelPanel;
    }
    return NULL;
}


bool CGenBankUILoadManager::CanDo(EAction action)
{
    switch(m_State) {
    case eSelectAcc:
        return action == eNext;
    case eSelectProject:
        return action == eBack  ||  action == eNext;
    case eCompleted:
        return false; // nothing left to do
    default:
        _ASSERT(false);
        return false;
    }
}


bool CGenBankUILoadManager::IsFinalState()
{
    return m_State == eSelectProject;
}


bool CGenBankUILoadManager::IsCompletedState()
{
    return m_State == eCompleted;
}


bool CGenBankUILoadManager::DoTransition(EAction action)
{
    if(m_State == eSelectAcc  &&  action == eNext)    {
        if (m_OptionPanel->IsInputValid())  {
            m_State = eSelectProject;
            return true;
        }
        return false;
    } else if( m_State == eSelectProject) {
        if(action == eBack)  {
            m_State = eSelectAcc;
            return true;
        } else if(action == eNext)  {
            if(m_ProjectSelPanel->TransferDataFromWindow()) {
                m_State = eCompleted;
                return true;
            }
            return false;
        }
    }
    _ASSERT(false);
    return false;
}


IAppTask* CGenBankUILoadManager::GetTask()
{
    // extract parameters from the dialog panels
    SaveMruAccessions();
    CIRef<IObjectLoader> loader;  
    if ((!m_OptionPanel->GetSeqIds().empty() || !m_OptionPanel->GetNAs().empty())
		&& !m_OptionPanel->GetGenomicAccessions().empty()) {
        CChainLoader *chain = new CChainLoader();
        loader.Reset( chain );
        chain->Add(new CGBObjectLoader(m_OptionPanel->GetSeqIds(), m_OptionPanel->GetNAs()));
        chain->Add(new CAssemblyObjectLoader(m_OptionPanel->GetGenomicAccessions()));
    }
    else if (!m_OptionPanel->GetGenomicAccessions().empty())
		loader.Reset(new CAssemblyObjectLoader(m_OptionPanel->GetGenomicAccessions()));
    else
		loader.Reset(new CGBObjectLoader(m_OptionPanel->GetSeqIds(), m_OptionPanel->GetNAs()));

    m_ProjectSelPanel->GetParams(m_ProjectParams);
    string folder_name = m_ProjectParams.m_CreateFolder ? m_ProjectParams.m_FolderName : "";

    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CSelectProjectOptions options;
    m_ProjectParams.ToLoadingOptions(options);
    return new CObjectLoadingTask(srv, *loader, options);
}


void CGenBankUILoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


static const char* kProjectParamsTag = "ProjectParams";

void CGenBankUILoadManager::SaveSettings() const
{
    if (!m_RegPath.empty()) {

        /// remember the selected Format (only if m_OptionPanel exists)
        if (m_OptionPanel)
            m_OptionPanel->SaveSettings(m_RegPath);

        /// save Project Panel settings
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        m_ProjectParams.SaveSettings(view, kProjectParamsTag);
    }
}

void CGenBankUILoadManager::SaveMruAccessions()
{
    if (!m_RegPath.empty() && m_OptionPanel != 0)
        m_OptionPanel->SaveMruAccessions(m_RegPath);
}

void CGenBankUILoadManager::LoadSettings()
{
    if (!m_RegPath.empty()) {

        if (m_OptionPanel)
            m_OptionPanel->LoadSettings(m_RegPath);

        /// load Project Panel settings
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
        m_ProjectParams.LoadSettings(view, kProjectParamsTag);
    }
}

END_NCBI_SCOPE
