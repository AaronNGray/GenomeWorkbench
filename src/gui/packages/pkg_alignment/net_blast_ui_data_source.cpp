/*  $Id: net_blast_ui_data_source.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Andrey Yazhuk, Anatoliy Kuznetsov
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbi_system.hpp>

#include <gui/packages/pkg_alignment/net_blast_ui_data_source.hpp>

#include <gui/packages/pkg_alignment/net_blast_ui_load_manager.hpp>
#include <gui/packages/pkg_alignment/blast_search_task.hpp>

#include <gui/core/data_mining_service.hpp>
#include <gui/core/data_mining_view.hpp>
#include <gui/core/app_dialogs.hpp>
#include <gui/core/pt_data_source.hpp>

#include <gui/utils/app_job_impl.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/view_manager_service.hpp>

#include <gui/widgets/wx/ui_command.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/utils/extension_impl.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/objutils/taxon_cache.hpp>
#include <gui/objutils/blast_databases.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <objects/general/Object_id.hpp>

#include <objects/blast/blastclient.hpp>
#include <objects/blast/Blast4_database.hpp>
#include <objects/blast/Blast4_database_info.hpp>
#include <objects/taxon1/taxon1.hpp>

#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/stopwatch.h>
#include <wx/filename.h>
#include <wx/datetime.h>

//#include <gui/packages/pkg_alignment/blast_winmask_dialog.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///

///////////////////////////////////////////////////////////////////////////////
/// CNetBlastDSEvtHandler - wxEvtHandler-derived adapter for Net BLAST data source.
/// This class translates menu commands into Net BLAST data source method calls.

class CNetBlastDSEvtHandler : public wxEvtHandler
{
    DECLARE_EVENT_TABLE();
public:
    CNetBlastDSEvtHandler(CNetBLASTUIDataSource& ds)
        :   m_NetBlastDS(&ds)
    {};

    CNetBlastDSEvtHandler(CNetBLASTUIDataSource& ds, CNetBLASTUIDataSource::TJobDescrVec& jobs)
        :   m_NetBlastDS(&ds), m_Jobs(jobs)
    {};

    ~CNetBlastDSEvtHandler()
    {
    }

    void OnRetrieve(wxCommandEvent& event)
    {
        m_NetBlastDS->StartRetrievingTask(m_Jobs);
    }
    void OnMonitor(wxCommandEvent& event)
    {
        m_NetBlastDS->StartMonitoringTask(m_Jobs);
    }
    void OnDelete(wxCommandEvent& event)
    {
        m_NetBlastDS->DeleteJobDescriptors(m_Jobs);
    }
    void OnExplore(wxCommandEvent& event)
    {
        m_NetBlastDS->OnExplore();
    }
    void OnLoadRIDs(wxCommandEvent& event)
    {
        m_NetBlastDS->OnLoadRIDs();
    }

public:
    CRef<CNetBLASTUIDataSource> m_NetBlastDS;
    CNetBLASTUIDataSource::TJobDescrVec m_Jobs;
};

BEGIN_EVENT_TABLE(CNetBlastDSEvtHandler, wxEvtHandler)
    EVT_MENU(eCmdRetriveBlastJobs, CNetBlastDSEvtHandler::OnRetrieve)
    EVT_MENU(eCmdMonitorBlastJobs, CNetBlastDSEvtHandler::OnMonitor)
    EVT_MENU(eCmdDeleteBlastJobs, CNetBlastDSEvtHandler::OnDelete)
    EVT_MENU(eCmdExploreNetBLAST, CNetBlastDSEvtHandler::OnExplore)
    EVT_MENU(eCmdLoadRIDs, CNetBlastDSEvtHandler::OnLoadRIDs)
END_EVENT_TABLE();

///////////////////////////////////////////////////////////////////////////////
/// CNetBLASTUIDataSource

static const char* kNetBLAST_DS_Icon = "icon::gb_data_source"; //TODO
//static const char* kNetBlastCmdContributor = "net_blast_cmd_contributor";
static const char* kCmdExtPoint = "scoped_objects::cmd_contributor";
static const char* kAppExpCmdExtPoint = "project_tree_view::context_menu::item_cmd_contributor";


CNetBLASTUIDataSource::CNetBLASTUIDataSource(CNetBLASTUIDataSourceType& type)
:   m_Descr("NCBI Net BLAST", kNetBLAST_DS_Icon),
    m_Type(&type),
    m_SrvLocator(NULL),
    m_Open(false)
{
}


CNetBLASTUIDataSource::~CNetBLASTUIDataSource()
{
    _ASSERT(! m_Open);
}


string CNetBLASTUIDataSource::GetExtensionIdentifier() const
{
    return "net_blast_data_source";
}


string CNetBLASTUIDataSource::GetExtensionLabel() const
{
    return "Net BLAST Data Source";
}


void CNetBLASTUIDataSource::SetServiceLocator(IServiceLocator* locator)
{
    m_SrvLocator = locator;
}

//static const char* kCNetBLASTUIRootKey = "NetBlastUI";
//static const char* kLocalMaskPath = "LocalMaskPath";
//static const char* kRemoteMaskURL = "RemoteMaskURL";

void CNetBLASTUIDataSource::SetRegistryPath( const string& path )
{
    m_RegPath = path;
}

void CNetBLASTUIDataSource::SaveSettings() const
{
    _ASSERT(! m_RegPath.empty() );

    if( !m_RegPath.empty() ){
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
    }
}


void CNetBLASTUIDataSource::LoadSettings()
{
    _ASSERT(! m_RegPath.empty());

    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
    }
}

IUIDataSourceType& CNetBLASTUIDataSource::GetType() const
{
    return *m_Type;
}


const IUIObject& CNetBLASTUIDataSource::GetDescr()
{
    return m_Descr;
}


bool CNetBLASTUIDataSource::IsOpen()
{
    return m_Open;
}


bool CNetBLASTUIDataSource::Open()
{
    if ( !m_Open )   {
        ERR_POST(Info <<"Opening Net BLAST Data Source...");

        CStopWatch sw;
        sw.Start();

        wxString homePath = CSysPath::ResolvePath(wxT("<home>"));
        wxString dbTreePath = CSysPath::ResolvePath(wxT("<std>/etc/blastdb-spec/blast-db-tree.asn"));
        CBLASTDatabases::GetInstance().Load(homePath.ToUTF8(), dbTreePath.ToUTF8());

        /// register itself as menu contributor
        CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
        reg->AddExtension(kCmdExtPoint, *this);
        reg->AddExtension(kAppExpCmdExtPoint, *this);

        x_LoadJobDescriptors();

        x_AutoStartMonitoring();

        m_Open = true;

        string t = NStr::DoubleToString(sw.Elapsed(), 3);
        ERR_POST(Info << "Registered Net BLAST Data Source - " << t << " sec");
        return true;
    } else {
        return false;
    }
}


bool CNetBLASTUIDataSource::Close()
{
    if(m_Open)  {
#       ifdef _DEBUG
        CStopWatch watch( CStopWatch::eStart );
#       endif

        x_SaveJobDescriptors();
        m_JobDescrs.clear();

#       ifdef _DEBUG
        watch.Stop();
        ERR_POST(Info << "x_SaveJobDescriptors(): saving takes " << watch.AsSmartString() );
        watch.Restart();
#       endif

        /// remove itself from menu contribution points
        CIRef<IExtensionRegistry> reg = CExtensionRegistry::GetInstance();
        reg->RemoveExtension(kCmdExtPoint, *this);
        reg->RemoveExtension(kAppExpCmdExtPoint, *this);

        m_Open = false;
        return true;
    } else return false;
}


void CNetBLASTUIDataSource::EditProperties()
{
    //TODO
}


IUIToolManager* CNetBLASTUIDataSource::GetLoadManager()
{
    // TODO may need to link the manager to this particular datasource
    return new CNetBLASTUILoadManager(*this); //TODO ?
}


int CNetBLASTUIDataSource::GetDefaultCommand() {
    return eCmdLoadRIDs;
}

wxEvtHandler* CNetBLASTUIDataSource::CreateEvtHandler() {

    return new CNetBlastDSEvtHandler( *this ); 
}


IObjectCmdContributor::TContribution
    CNetBLASTUIDataSource::GetMenu(TConstScopedObjects& objects)
{
    wxMenu* menu = NULL;
    wxEvtHandler* handler = NULL;

    if( ! objects.empty()) {
        // examine data
        int retrieve = 0;
        int monitor = 0;
        CNetBLASTUIDataSource::TJobDescrVec jobs;

        NON_CONST_ITERATE(TConstScopedObjects, it, objects) {
            const CObject* obj = it->object.GetPointer();
            const CNetBlastJobDescriptor* const_descr =
                dynamic_cast<const CNetBlastJobDescriptor*>(obj);
            CNetBlastJobDescriptor* descr = const_cast<CNetBlastJobDescriptor*>(const_descr);

            if(descr)   {
                jobs.push_back(CRef<CNetBlastJobDescriptor>(descr));

                CNetBlastJobDescriptor::EState state = descr->GetState();

                switch(state)   {
                case CNetBlastJobDescriptor::eCompleted:
                case CNetBlastJobDescriptor::eRetrieved:
                    retrieve++;
                    break;
                case CNetBlastJobDescriptor::eSubmitted:
                    monitor++;
                    break;
                default:
                    break;
                }
            }
        }

        CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

        menu = new wxMenu;
        menu->Append(wxID_SEPARATOR, wxT("Top Actions"));
        if(retrieve > 0)    {
            cmd_reg.AppendMenuItem(*menu, eCmdRetriveBlastJobs);
        }
        if(monitor > 0)    {
            cmd_reg.AppendMenuItem(*menu, eCmdMonitorBlastJobs);
        }
        if( ! jobs.empty()) {
            cmd_reg.AppendMenuItem(*menu, eCmdDeleteBlastJobs);
            handler = new CNetBlastDSEvtHandler(*this, jobs);
        }
    }

    return IObjectCmdContributor::TContribution(menu, handler);
}


IExplorerItemCmdContributor::TContribution
    CNetBLASTUIDataSource::GetMenu(wxTreeCtrl&, PT::TItems& items)
{
    IExplorerItemCmdContributor::TContribution contrib;
    if(items.size() != 1)   {
        return contrib;
    }

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    PT::CItem* item = items[0];
    int type = item->GetType();

    if(type == PT::eDataSource)    {
        PT::CDataSource* ds_item = dynamic_cast<PT::CDataSource*>(item);
        if(ds_item) {
            CIRef<IUIDataSource> ds = ds_item->GetData();
            CNetBLASTUIDataSource* blast_ds = dynamic_cast<CNetBLASTUIDataSource*>(ds.GetPointer());

            if(blast_ds)  {
                wxMenu* menu = new wxMenu;
                menu->Append(wxID_SEPARATOR, wxT("Top Actions"));
                cmd_reg.AppendMenuItem(*menu, eCmdExploreNetBLAST);
                cmd_reg.AppendMenuItem(*menu, eCmdLoadRIDs);

                contrib.first = menu;
                contrib.second = new CNetBlastDSEvtHandler(*this);
            }
        }
    }
    return contrib;
}

CRef<CNetBlastJobDescriptor>
    CNetBLASTUIDataSource::CreateJobDescriptor(const CBLASTParams& params)
{
    CFastMutexGuard guard(m_JobDescrMutex);

    CRef<CNetBlastJobDescriptor> descr(new CNetBlastJobDescriptor());
    descr->Init(params, *this);
    m_JobDescrs.push_back(descr);
    return descr;
}


CRef<CNetBlastJobDescriptor>
    CNetBLASTUIDataSource::CreateJobDescriptor(const string& rid)
{
    CFastMutexGuard guard(m_JobDescrMutex);

    CRef<CNetBlastJobDescriptor> descr(new CNetBlastJobDescriptor());
    descr->Init(rid, *this);
    m_JobDescrs.push_back(descr);
    return descr;
}


void CNetBLASTUIDataSource::GetJobDescriptors(vector< CRef<CNetBlastJobDescriptor> >& descriptors)
{
    CFastMutexGuard guard(m_JobDescrMutex);

    for(  size_t i = 0;  i < m_JobDescrs.size();  i++ )   {
        CRef<CNetBlastJobDescriptor>& descr = m_JobDescrs[i];
        if( ! descr->IsDeleted())  {
            descriptors.push_back(descr);
        }
    }
}


CRef<CNetBlastJobDescriptor> CNetBLASTUIDataSource::FindJobDescriptor(const string& rid)
{
    CFastMutexGuard guard(m_JobDescrMutex);

    CRef<CNetBlastJobDescriptor> result;
    for(  size_t i = 0;  i < m_JobDescrs.size();  i++ )   {
        CRef<CNetBlastJobDescriptor>& descr = m_JobDescrs[i];
        if( descr->GetRID() == rid ){
            result = descr;
            break;
        }
    }
    return result;
}


//static const wxChar* kBLAST_DBsPath = wxT("<home>/blast-dbs.asn");

static const wxChar* kJobDescrPath = wxT("<home>/net_blast_jobs.asn");
static const char* kJobsTag = "Jobs";

void CNetBLASTUIDataSource::x_SaveJobDescriptors()
{
    LOG_POST(Info << "Net BLAST Data Source - saving job descriptors...");

    try {

        vector< CRef<CUser_object> >    jobs;

        for(  size_t i = 0;  i < m_JobDescrs.size();  i++  )  {
            try {
                CNetBlastJobDescriptor& descr = *m_JobDescrs[i];

                // do not save Descriptors marked as deleted, we preserve them
                // at run-time, but purge between application sessions
                if( ! descr.IsDeleted())    {
                    CRef<CUser_object> job_obj(descr.ToUserObject());
                    jobs.push_back(job_obj);
                }
            } catch(CException& e)  {
                LOG_POST(Error << "Saving Net BLAST jobs - exception while saving a job - " << e.GetMsg());
            }
        }

        CRef<CUser_object> container(new CUser_object());
        container->SetType().SetStr(kJobsTag);
        container->AddField(kJobsTag, jobs);

        wxString path = CSysPath::ResolvePath(kJobDescrPath);
        CNcbiOfstream ostr(path.fn_str());
        ostr << MSerial_AsnText << *container;

        LOG_POST(Info << "Net BLAST Data Source - finished saving job descriptors");
    } catch(CException& e)  {
        LOG_POST(Info << "Net BLAST Data Source - failed to save jobs - " << e.GetMsg());
    }
}


void CNetBLASTUIDataSource::x_LoadJobDescriptors()
{
    LOG_POST(Info << "Net BLAST Data Source - loading job descriptors...");

    wxString path = CSysPath::ResolvePath(kJobDescrPath);

    if (wxFileName::FileExists(path)) {
        try {
            CRef<CUser_object> container(new CUser_object());
            CNcbiIfstream istr(path.fn_str());
            istr >> MSerial_AsnText >> *container;

            const CUser_field& field = container->GetField(kJobsTag);
            const vector< CRef<CUser_object> >& jobs = field.GetData().GetObjects();

            m_JobDescrs.clear();
            for( size_t i = 0;  i < jobs.size();  i++ ) {
                const CUser_object& job_obj = *jobs[i];

                CRef<CNetBlastJobDescriptor> descr(new CNetBlastJobDescriptor(*this));
                descr->FromUserObject(job_obj);

                if(descr->GetState() != CNetBlastJobDescriptor::eInvalidState)    {
                    m_JobDescrs.push_back(descr);
                }
            }

            LOG_POST(Info << "Net BLAST Data Source - finished loading job descriptors");
        }
        catch (CException& e) {
            LOG_POST(Error << "Net BLAST Data Source - failed to load job descriptors" << e.GetMsg());
        }
    }
}

void CNetBLASTUIDataSource::x_OnJobDescrChanged(CNetBlastJobDescriptor& descr)
{
    Post(CRef<CEvent>(new CNetBLASTUIDataSourceEvent()));
}


void CNetBLASTUIDataSource::StartRetrievingTask(TJobDescrVec& jobs)
{
    vector<string> RIDs;
    for(  size_t i = 0;  i < jobs.size();  i++) {
        CNetBlastJobDescriptor& descr = *jobs[i];
        RIDs.push_back(descr.GetRID());
    }

    CRef<CBlastSearchTask> task(new CBlastSearchTask(m_SrvLocator, *this, "Net BLAST Search"));
    task->Init_RetrieveRID(RIDs);

    x_StartTask(*task);
}


void CNetBLASTUIDataSource::StartMonitoringTask(TJobDescrVec& jobs)
{
    CRef<CBlastSearchTask> task(new CBlastSearchTask(m_SrvLocator, *this, "Net BLAST Search"));
    task->Init_Monitoring(jobs);

    x_StartTask(*task);
}


void CNetBLASTUIDataSource::DeleteJobDescriptors(TJobDescrVec& descriptors)
{
    for(  size_t i = 0;  i < descriptors.size();  i++)  {
        CNetBlastJobDescriptor& descr = *descriptors[i];
        descr.MarkDeleted(true);
    }
}


void CNetBLASTUIDataSource::OnExplore()
{
    if(m_SrvLocator)    {
        /// first show the view
        IViewManagerService* view_srv = m_SrvLocator->GetServiceByType<IViewManagerService>();
        CIRef<IView> view = view_srv->ShowSingletonView("Search View");

        // here - send command to the view
        CDataMiningView* dm_view = dynamic_cast<CDataMiningView*>(view.GetPointer());
        if(dm_view) {
            dm_view->SelectToolByName(NET_BLAST_DM_TOOL_NAME);
        }
    }
}


void CNetBLASTUIDataSource::OnLoadRIDs()
{
    /// TODO this is not a good solution, but simple
    IWorkbench* workbench = dynamic_cast<IWorkbench*>(m_SrvLocator);

    if(workbench)    {
        CAppDialogs::COpenDialog(workbench, NET_BLAST_LOADER_LABEL);
    }
}

void CNetBLASTUIDataSource::x_StartTask(CBlastSearchTask& task)
{
    CSelectProjectOptions options;
    options.Set_DecideLater();
    task.SetLoadingOptions(options);

    CAppTaskService* task_srv = m_SrvLocator->GetServiceByType<CAppTaskService>();
    task_srv->AddTask(task);
}


// start monitoring task for all jobs that are still in progress
void CNetBLASTUIDataSource::x_AutoStartMonitoring()
{
    TJobDescrVec monitor_jobs;
    {{
    CFastMutexGuard guard(m_JobDescrMutex);

    // get jobs that have "Submitted" status
    for(  size_t i = 0;  i < m_JobDescrs.size();  i++)  {
        CRef<CNetBlastJobDescriptor>& descr = m_JobDescrs[i];
        if(descr->GetState() == CNetBlastJobDescriptor::eSubmitted)  {
            monitor_jobs.push_back(descr);
        }
    }
    }}

    size_t n = monitor_jobs.size();
    if(n > 0)   {
        LOG_POST(Info << "Net BLAST start monitoring for " << n << " jobs");
        StartMonitoringTask(monitor_jobs);
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CNetBLASTUIDataSourceType
CNetBLASTUIDataSourceType::CNetBLASTUIDataSourceType()
:   m_Descr( "Net BLAST Client Service", "", "", "", "NetBlastUI" )
{
   //wxFileArtProvider* provider = GetDefaultFileArtProvider();
   //TODO provider->RegisterFileAlias(kNetBLAST_DS_Icon, "net_blast_data_source.png");
}


const IUIObject& CNetBLASTUIDataSourceType::GetDescr()
{
    return m_Descr;
}


IUIDataSource* CNetBLASTUIDataSourceType::CreateDataSource()
{
    return new CNetBLASTUIDataSource(*this);
}


bool CNetBLASTUIDataSourceType::AutoCreateDefaultDataSource()
{
    return true; // create NCBI Net BLAST by default
}


string CNetBLASTUIDataSourceType::GetExtensionIdentifier() const
{
    static string ext_id("net_blast_data_source_type");
    return ext_id;
}


string CNetBLASTUIDataSourceType::GetExtensionLabel() const
{
    return m_Descr.GetLabel();
}


END_NCBI_SCOPE
