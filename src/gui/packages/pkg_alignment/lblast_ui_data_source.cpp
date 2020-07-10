/*  $Id: lblast_ui_data_source.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Liangshou Wu
 *
 */

#include <ncbi_pch.hpp>

#include <objtools/data_loaders/blastdb/bdbloader.hpp>

#include <gui/widgets/loaders/lblast_params_panel.hpp>
#include <gui/widgets/loaders/lblast_object_loader.hpp>

#include <gui/packages/pkg_alignment/lblast_ui_data_source.hpp>
#include <gui/core/app_dialogs.hpp>
#include <gui/core/object_loading_task.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/pt_data_source.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/app_task_service.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/objects/assembly_info.hpp>

#include <gui/utils/extension_impl.hpp>

#include <wx/menu.h>

#include <objmgr/object_manager.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/gbproj/LoaderDescriptor.hpp>

#include <objtools/readers/idmapper.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* kLBLASTLoadOption = "Local BLAST DB";
static const int   kLoaderPriority = 90;


///////////////////////////////////////////////////////////////////////////////
/// CLBLASTDSEvtHandler - wxEvtHandler-derived adapter for local BLAST  data source.

class CLBLASTDSEvtHandler : public wxEvtHandler
{
    DECLARE_EVENT_TABLE();
public:
    CLBLASTDSEvtHandler(IWorkbench* workbench)
        :   m_Workbench(workbench)  {
    }
    void OnLoadBLASTDB(wxCommandEvent& event)
    {
        if(m_Workbench)    {
            CAppDialogs::COpenDialog(m_Workbench, kLBLASTLoadOption);
        }
    }
protected:
    IWorkbench* m_Workbench;
};

BEGIN_EVENT_TABLE(CLBLASTDSEvtHandler, wxEvtHandler)
EVT_MENU(eCmdLoadDB, CLBLASTDSEvtHandler::OnLoadBLASTDB)
END_EVENT_TABLE();


///////////////////////////////////////////////////////////////////////////////
/// CLBLASTUIDataSource

static const char* kLBLAST_DS_Icon = "icon::lblast_data_source";

CLBLASTUIDataSource::CLBLASTUIDataSource(CLBLASTUIDataSourceType& type)
:   m_Type(&type),
    m_Descr("Local BLAST", kLBLAST_DS_Icon),
    m_SrvLocator(NULL),
    m_Open(false)
{
}

CLBLASTUIDataSource::~CLBLASTUIDataSource()
{
}


string CLBLASTUIDataSource::GetExtensionIdentifier() const
{
    return "local_blast_data_source";
}


string CLBLASTUIDataSource::GetExtensionLabel() const
{
    return "Local BLAST Data Source";
}


void CLBLASTUIDataSource::SetServiceLocator(IServiceLocator* locator)
{
    m_SrvLocator = locator;
}


IExplorerItemCmdContributor::TContribution CLBLASTUIDataSource::GetMenu(wxTreeCtrl&, PT::TItems& items)
{
    IExplorerItemCmdContributor::TContribution contrib;

    /// this is not a good solution, but simple
    IWorkbench* workbench = dynamic_cast<IWorkbench*>(m_SrvLocator);

    if(items.size() != 1  ||  workbench == NULL)   {
        return contrib; // return empty object - nothin to contribute
    }

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    PT::CItem* item = items[0];
    int type = item->GetType();

    if(type == PT::eDataSource)    {
        PT::CDataSource* ds_item = dynamic_cast<PT::CDataSource*>(item);

        if(ds_item) {
            CIRef<IUIDataSource> ds = ds_item->GetData();
            CLBLASTUIDataSource* bam_ds = dynamic_cast<CLBLASTUIDataSource*>(ds.GetPointer());

            if(bam_ds)  {
                wxMenu* menu = new wxMenu;
                menu->Append(wxID_SEPARATOR, wxT("Top Actions"));
                cmd_reg.AppendMenuItem(*menu, eCmdLoadDB);

                contrib.first = menu;
                contrib.second = new CLBLASTDSEvtHandler(workbench);
            }
        }
    }
    return contrib;
}


IUIDataSourceType& CLBLASTUIDataSource::GetType() const
{
    return *m_Type;
}


const IUIObject& CLBLASTUIDataSource::GetDescr()
{
    return m_Descr;
}


bool CLBLASTUIDataSource::IsOpen()
{
    return m_Open;
}

//static const char* kCmdExtPoint = "scoped_objects::cmd_contributor";
static const char* kAppExpCmdExtPoint = "project_tree_view::context_menu::item_cmd_contributor";

bool CLBLASTUIDataSource::Open()
{
    if (m_Open) {
        LOG_POST(Error << "CLBLASTUIDataSource::Open(): "
            "attempt to open already open data source");
        return false;
    }

    CStopWatch sw;
    sw.Start();

    /// register itself as menu contributor
    CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
    reg->AddExtension(kAppExpCmdExtPoint, *this);

    m_Open = true;
    return true;
}


bool CLBLASTUIDataSource::Close()
{
    if (m_Open) {
        /// remove itself from menu contribution points
        CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
        reg->RemoveExtension(kAppExpCmdExtPoint, *this);

        m_Open = false;
        return true;
    }

    return false;
}


void CLBLASTUIDataSource::EditProperties()
{
    //TODO
}


IUIToolManager* CLBLASTUIDataSource::GetLoadManager()
{
    // TODO may need to link the manager to this particular datasource
    return new CLBLASTUILoadManager();
}

int CLBLASTUIDataSource::GetDefaultCommand() {
    return eCmdLoadDB;
}

wxEvtHandler* CLBLASTUIDataSource::CreateEvtHandler() {
    
    
    IWorkbench* workbench = dynamic_cast<IWorkbench*>(m_SrvLocator);

    return new CLBLASTDSEvtHandler( workbench ); 
}

string CLBLASTUIDataSource::AddDataLoader(const objects::CUser_object& obj)
{
    return CLBLASTObjectLoader::AddDataLoader(obj);
}

string CLBLASTUIDataSource::GetLoaderName(const objects::CUser_object& obj) const
{
    return CLBLASTObjectLoader::GetLoaderName(obj);
}


///////////////////////////////////////////////////////////////////////////////
/// CLBLASTUIDataSourceType
CLBLASTUIDataSourceType::CLBLASTUIDataSourceType()
    :   m_Descr("Local BLAST data Loader", "")
{
   wxFileArtProvider* provider = GetDefaultFileArtProvider();
   provider->RegisterFileAlias(ToWxString(kLBLAST_DS_Icon),
                               wxT("lblast_data_source.png"));
}


const IUIObject& CLBLASTUIDataSourceType::GetDescr()
{
    return m_Descr;
}


IUIDataSource* CLBLASTUIDataSourceType::CreateDataSource()
{
    return new CLBLASTUIDataSource(*this);
}


bool CLBLASTUIDataSourceType::AutoCreateDefaultDataSource()
{
    return true; // we want to create default "Bam" datasource
}


string CLBLASTUIDataSourceType::GetExtensionIdentifier() const
{
    static string ext_id("local_blast_data_source_type");
    return ext_id;
}


string CLBLASTUIDataSourceType::GetExtensionLabel() const
{
    return m_Descr.GetLabel();
}



///////////////////////////////////////////////////////////////////////////////
/// CLBLASTUILoadManager
CLBLASTUILoadManager::CLBLASTUILoadManager()
:   m_SrvLocator(NULL),
    m_ParentWindow(NULL),
    m_Descriptor(kLBLASTLoadOption, ""),
    m_State(eInvalid),
    m_ParamsPanel(),
    m_ProjectSelPanel()
{
    m_ProjectParams.m_EnableDecideLater = false;
    m_Descriptor.SetLogEvent("loaders");
}


void CLBLASTUILoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CLBLASTUILoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CLBLASTUILoadManager::GetDescriptor() const
{
    return m_Descriptor;
}


void CLBLASTUILoadManager::InitUI()
{
    m_State = eSelectDir;
}


void CLBLASTUILoadManager::CleanUI()
{
    m_State = eInvalid;
    m_ParamsPanel = 0;
    m_ProjectSelPanel = 0;   // window is destroyed by its parent
}


wxPanel* CLBLASTUILoadManager::GetCurrentPanel()
{
    switch (m_State) {
    case eSelectDir:
        if (m_ParamsPanel == NULL)   {
            m_ParamsPanel = new CLBLASTParamsPanel(m_ParentWindow);
            m_ParamsPanel->SetData(m_LBLASTParams);
            m_ParamsPanel->TransferDataToWindow();
        }
        return m_ParamsPanel;
    case eSelectProject:
        if(m_ProjectSelPanel == NULL)   {
            CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
            m_ProjectSelPanel = new CProjectSelectorPanel(m_ParentWindow);
            m_ProjectSelPanel->SetProjectService(srv);
            m_ProjectSelPanel->SetParams(m_ProjectParams);
            m_ProjectSelPanel->TransferDataToWindow();
        }
        return m_ProjectSelPanel;
    default:
        return NULL;
    }
    return NULL;
}


bool CLBLASTUILoadManager::CanDo(EAction action)
{
    switch(m_State) {
    case eSelectDir:
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


bool CLBLASTUILoadManager::IsFinalState()
{
    return m_State == eSelectProject;
}


bool CLBLASTUILoadManager::IsCompletedState()
{
    return m_State == eCompleted;
}


bool CLBLASTUILoadManager::DoTransition(EAction action)
{
    if (m_State == eSelectDir) {
        if (action == eNext) {
            if (m_ParamsPanel->TransferDataFromWindow()) {
                m_LBLASTParams = m_ParamsPanel->GetData();
                m_State = eSelectProject;
                return true;
            }
            return false;
        }
        else {
            return false;
        }
    }
    else if (m_State == eSelectProject) {
        if (action == eBack) {
            m_State = eSelectDir;
            return true;
        }
        else if (action == eNext) {
            if (m_ProjectSelPanel->TransferDataFromWindow()) {
                m_State = eCompleted;
                return true;
            }
            return false;
        }
    }

    _ASSERT(false);
    return false;
}


IAppTask* CLBLASTUILoadManager::GetTask()
{
    CIRef<IObjectLoader> loader(new CLBLASTObjectLoader(m_LBLASTParams));

    m_ProjectSelPanel->GetParams(m_ProjectParams);
    string folder_name = m_ProjectParams.m_CreateFolder ? m_ProjectParams.m_FolderName : "";

    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CSelectProjectOptions options;
    m_ProjectParams.ToLoadingOptions(options);
    return new CObjectLoadingTask(srv, *loader, options);
}


void CLBLASTUILoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    if (!m_RegPath.empty())
        m_LBLASTParams.SetRegistryPath(m_RegPath + ".LBLASTParams");
    else
        m_LBLASTParams.SetRegistryPath(m_RegPath);
}

static const char* kProjectParamsTag = "ProjectParams";
static const char* kMappingAssembly = ".MapAssembly";

void CLBLASTUILoadManager::SaveSettings() const
{
    if ( !m_RegPath.empty() ) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        /// save Project Panel settings
        m_ProjectParams.SaveSettings(view, kProjectParamsTag);
        m_LBLASTParams.SaveSettings();
    }
}

void CLBLASTUILoadManager::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        /// load Project Panel settings
        m_ProjectParams.LoadSettings(view, kProjectParamsTag);
        m_LBLASTParams.LoadSettings();
    }
}

END_NCBI_SCOPE
