/*  $Id: blast_jobs.cpp 40209 2018-01-08 20:35:14Z joukovv $
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

#include <corelib/ncbifile.hpp>
#include <objmgr/object_manager.hpp>

#include <gui/packages/pkg_alignment/blast_jobs.hpp>

#include <gui/packages/pkg_alignment/blast_search_params.hpp>
#include <gui/packages/pkg_alignment/net_blast_ui_data_source.hpp>
#include <gui/packages/pkg_alignment/blast_search_task.hpp>

#include <gui/framework/app_task_service.hpp>

#include <gui/objutils/label.hpp>
#include <gui/core/prj_helper.hpp>

#include <objects/gbproj/ProjectItem.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Score.hpp>
#include <objects/general/Object_id.hpp>

#include <algo/sequence/util.hpp>
#include <algo/align/util/score_builder.hpp>

#include <corelib/ncbi_system.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(blast);


///////////////////////////////////////////////////////////////////////////////
/// CNetBlastSubmittingJob

CNetBlastSubmittingJob::CNetBlastSubmittingJob(string tool_name,
                                               const CBLASTParams& params,
                                               CNetBLASTUIDataSource& ds)
{
    m_Descr = tool_name + " - submitting sequences to NCBI Net BLAST";

    m_SeqLocs = params.GetSeqLocs();

    _ASSERT(m_SeqLocs.size());

    for(  size_t i = 0;  i < m_SeqLocs.size(); i++) {
        CRef<CNetBlastJobDescriptor> job_descr = ds.CreateJobDescriptor(params);
        m_Descriptors.push_back(job_descr);
    }
}


// this is an entry point for execution
IAppJob::EJobState CNetBlastSubmittingJob::Run()
{
    static const char* kLogMsg = "CNetBlastSubmittingJob::Run() - exception in x_Run() ";
    static string kErrorMsg = "Unhandled exception in CNetBlastSubmittingJob::Run()";  //TODO
    static const char* kBlastError = "BLAST returned error(s) in CNetBlastSubmittingJob::Run() :";

    //LOG_POST(Info << "CNetBlastSubmittingJob::Run()  Started  " << m_Descr);

    x_ResetState(); // reset state before execution

    string err_msg;
    try {
        x_Run(); // does real work
    } catch (CException& e) {
        err_msg = kLogMsg + GetDescr() + ". " + e.GetMsg();
        LOG_POST(Error << err_msg);
        LOG_POST(e.ReportAll());
    } catch (std::exception& ee) {
        err_msg = kLogMsg + GetDescr() + ". " + ee.what();
        LOG_POST(Error << err_msg);
    }
    CFastMutexGuard lock(m_Mutex);

    if( !err_msg.empty() ){
        string s = kErrorMsg + err_msg;
        m_Error.Reset(new CAppJobError(s));
        return eFailed;
         
    } else if( !m_Errors.empty() ){
        string s = kBlastError;
        LOG_POST( Error << kBlastError );
        ITERATE( vector<string>, it, m_Errors ){
            LOG_POST( Error << *it );
            s += "\n" + *it;
        }
        m_Error.Reset( new CAppJobError(s) );
        return eFailed;
    }

    return eCompleted;
}


/// this fucntions performs real work
void CNetBlastSubmittingJob::x_Run()
{
    _ASSERT(m_SeqLocs.size());
    static const char* kFailedToSubmit = "Failed to submit BLAST search for ";

    string str;

    for( size_t i = 0;  i < m_SeqLocs.size();  i++  )   {
        const CObject& obj = *m_SeqLocs[i].object;
        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(&obj);

        _ASSERT(loc);

        if(loc) {
            CScope& scope = *m_SeqLocs[i].scope;
            CLabel::GetLabel(*loc, &str, CLabel::eDefault, &scope);

            try {
                // convert the location to a sequence
                // this is a constructed bioseq containing only the covered sequence...

                //TODO - move to CNetBlastJobDescriptor ?
                CRef<CBioseq> bioseq(SeqLocToBioseq(*loc, scope));

                if(bioseq)  {
                    CRef<CBioseq_set> bset(new CBioseq_set());
                    CRef<CSeq_entry> se(new CSeq_entry());
                    se->SetSeq(*bioseq);
                    bset->SetSeq_set().push_back(se);

                    CRef<CNetBlastJobDescriptor> descr = m_Descriptors[i];
                    descr->MarkDeleted(false);  // resurrect if needed

                    descr->Submit(*bset);
                    string rid = descr->GetRID();
                    LOG_POST(Info << "BLAST RID: " << rid);

                    CFastMutexGuard lock(m_Mutex);
                    //m_RIDs.push_back(rid);
                } else {
                    string err = "Failed to create a bioseq for item " + str;
                    err += " The item has not been submitted.";
                    x_AddError(err);
                    continue;
                }
            } catch (CException& e) {
                string err = string(kFailedToSubmit) + str + ". " + e.GetMsg();
                x_AddError(err);
            }
            catch (std::exception& e) {
                string err = string(kFailedToSubmit) + str + ". " + e.what();
                x_AddError(err);
            }
        }
    }
}


void CNetBlastSubmittingJob::x_AddError(const string& error)
{
    LOG_POST(Error << m_Descr << " " << error);
    CFastMutexGuard lock(m_Mutex);
    m_Errors.push_back(error);
}


void CNetBlastSubmittingJob::x_SetStatusText(const string& text)
{
    CFastMutexGuard lock(m_Mutex);
    m_Status = text;
}


void CNetBlastSubmittingJob::x_ResetState()
{
    CFastMutexGuard lock(m_Mutex);

    m_Error.Reset();

    m_Status = "Starting...";
}




CConstIRef<IAppJobProgress> CNetBlastSubmittingJob::GetProgress()
{
    CFastMutexGuard lock(m_Mutex);
    CConstIRef<IAppJobProgress> pr(new CAppJobProgress(-1.0, m_Status));
    return pr;
}


CRef<CObject> CNetBlastSubmittingJob::GetResult()
{
    return CRef<CObject>(); // we do not support this
}


CConstIRef<IAppJobError> CNetBlastSubmittingJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CNetBlastSubmittingJob::GetDescr() const
{
    return m_Descr;
}


void CNetBlastSubmittingJob::GetDescriptors(TDescriptors& descriptors)
{
    descriptors = m_Descriptors;
}


void CNetBlastSubmittingJob::GetErrors(vector<string>& errors) const
{
    //errors = m_Errors;
    for(  size_t i = 0;  i < m_Descriptors.size();  i++ )   {
        const CNetBlastJobDescriptor& descr = *m_Descriptors[i];
        string s = descr.GetErrors();
        if( ! s.empty())    {
            errors.push_back(s);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CNetBlastSubmittingJob

CNetBlastMonitoringJob::CNetBlastMonitoringJob(CNetBLASTUIDataSource& ds,
                                               IServiceLocator* srv_locator,
                                               const string& tool_name,
                                               TDescriptors& descriptors)
:   m_NetBlastDS(&ds),
    m_WaitPeriodIndex(0),
    m_Descriptors(descriptors)
{
    CFastMutexGuard lock(m_Mutex);

    m_ToolName = tool_name;
    m_SrvLocator = srv_locator;
    m_Descr = m_ToolName + " - monitoring NCBI NetBlast"; // TODO RIDs
}


void CNetBlastMonitoringJob::SetLoadingOptions(CSelectProjectOptions& options)
{
    m_LoadingOptions = options;
}

// this is an entry point for execution
IAppJob::EJobState CNetBlastMonitoringJob::Run()
{
    static const char* kLogMsg = "CNetBlastMonitoringJob::Run() - exception in x_Run() ";
    static string kErrorMsg = "Unhandled exception in CNetBlastMonitoringJob::Run()";  //TODO
    static const char* kBlastError = "BLAST returned error(s) in CNetBlastMonitoringJob::Run() :";

    //LOG_POST(Info << "CNetBlastSubmittingJob::Run()  Started  " << m_Descr);

    x_ResetState(); // reset state before execution

    string err_msg;
    EJobState state = eFailed;
    try {
        state = x_Run(); // does real work
    }
    catch (CException& e) {
        err_msg = kLogMsg + GetDescr() + ". " + e.GetMsg();
        LOG_POST(Error << err_msg);
        LOG_POST(e.ReportAll());
    } catch (std::exception& ee) {
        err_msg = kLogMsg + GetDescr() + ". " + ee.what();
        LOG_POST(Error << err_msg);
    } 

    CFastMutexGuard lock(m_Mutex);

    if( !err_msg.empty() ){
        string s = kErrorMsg + err_msg;
        m_Error.Reset(new CAppJobError(s));
        return eFailed;
    
    } else if( !m_Errors.empty() ){
        string s = kBlastError;
        LOG_POST( Error << kBlastError );
        ITERATE( vector<string>, it, m_Errors ){
            LOG_POST( Error << *it );
            s += "\n" + *it;
        }
        m_Error.Reset( new CAppJobError(s) );
        return eFailed;
    }

    return state;
}


/// this functions performs real work
/// every call to this function does pools each job once, then functions exits
/// if some of jobs are still pending it returns eRunning state, so that
/// it can be executed one more time
IAppJob::EJobState CNetBlastMonitoringJob::x_Run()
{
    //LOG_POST(Info << "CNetBlastMonitoringJob::x_Run() STARTED");

    vector<string> retrieve_ids;

    size_t i = 0;
    while( !IsCanceled() && i < m_Descriptors.size() ){

        //LOG_POST(Info << "CNetBlastMonitoringJob::x_Run() ITERATION");
        CRef<CNetBlastJobDescriptor> descr = m_Descriptors[i];

        CTime t; t.SetCurrent();
        //LOG_POST(Info << "CNetBlastMonitoringJob::x_Run() " << t.AsString());

        descr->Check();
        bool done = false;

        switch(descr->GetState())    {
        case CNetBlastJobDescriptor::eCompleted:
            retrieve_ids.push_back(descr->GetRID());
            done = true;
            break;

        case CNetBlastJobDescriptor::eFailed:   {{
            CFastMutexGuard lock(m_Mutex);
            m_Errors.push_back(descr->GetErrors()); // job failed
            done = true;
            break;
        }}
        case CNetBlastJobDescriptor::eRetrieved:
            done = true;
            break;

        case CNetBlastJobDescriptor::eExpired:  {{
            string s = "RID " + descr->GetRID() + " is invalid or expired";
            CFastMutexGuard lock(m_Mutex);
            m_Errors.push_back(s);
            done = true;
            break;
        }}
        default:
            break;
        }

        if( done ){
            m_Descriptors.erase( m_Descriptors.begin() + i );
        } else {
            i++;
        }
    }
    if( ! retrieve_ids.empty() && !IsCanceled() ){
        x_StartRetrivingTask( retrieve_ids );
    }

    if( IsCanceled() ){
        m_NetBlastDS->DeleteJobDescriptors( m_Descriptors );
        return eCanceled;

    } else if( m_Descriptors.empty() ){
        return eCompleted;
    
    }

    return eRunning;
}


void CNetBlastMonitoringJob::x_StartRetrivingTask(const vector<string>& RIDs)
{
    if (IsCanceled()) {
        return;
    }

    _ASSERT(m_SrvLocator);

    CRef<CBlastSearchTask> task(new CBlastSearchTask(m_SrvLocator, *m_NetBlastDS, m_ToolName));
    task->SetLoadingOptions(m_LoadingOptions);
    // Init_RetrieveRID needs correct option to find the project
    task->Init_RetrieveRID(RIDs);

    CAppTaskService* task_srv = m_SrvLocator->GetServiceByType<CAppTaskService>();

    if (IsCanceled() || (task_srv == 0)) {
        return;
    }

    task_srv->AddTask(*task);
}


void CNetBlastMonitoringJob::x_SetStatusText(const string& text)
{
    CFastMutexGuard lock(m_Mutex);
    m_Status = text;
}


void CNetBlastMonitoringJob::x_ResetState()
{
    CFastMutexGuard lock(m_Mutex);

    m_Error.Reset();

    m_Status = "Starting...";
}


CConstIRef<IAppJobProgress> CNetBlastMonitoringJob::GetProgress()
{
    CFastMutexGuard lock(m_Mutex);
    CConstIRef<IAppJobProgress> pr(new CAppJobProgress(-1.0, m_Status));
    return pr;
}


CRef<CObject> CNetBlastMonitoringJob::GetResult()
{
    return CRef<CObject>(); // we do not support this
}


CConstIRef<IAppJobError> CNetBlastMonitoringJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CNetBlastMonitoringJob::GetDescr() const
{
    return m_Descr;
}


CTimeSpan CNetBlastMonitoringJob::GetWaitPeriod()
{
    static long periods[] = { 1, 1, 1, 2, 3, 4, 6, 10, 20, 30, 60    };
    static const int max_index = sizeof(periods) / sizeof(long) - 1;

    long t = periods[m_WaitPeriodIndex];
    m_WaitPeriodIndex = std::min(m_WaitPeriodIndex + 1, max_index);
    return CTimeSpan(t);
}


///////////////////////////////////////////////////////////////////////////////
/// CNetBlastLoadingJob
CNetBlastLoadingJob::CNetBlastLoadingJob(CNetBLASTUIDataSource& ds,
                                         const vector<string>* RIDs,
                                         CScope *scope)
                                         : m_Scope(scope)
{
    CFastMutexGuard lock(m_Mutex);

    if(RIDs)    {
        SetRIDs(ds, *RIDs);
    }
    m_Descr = "NCBI Net BLAST - retrieving results"; //TODO
}


void CNetBlastLoadingJob::SetRIDs(CNetBLASTUIDataSource& ds, const vector<string>& RIDs)
{
    for( size_t i = 0; i < RIDs.size();  i++)   {
        const string& rid = RIDs[i];
        CRef<CNetBlastJobDescriptor> descr = ds.FindJobDescriptor(rid);
        if(descr)   {
            descr->MarkDeleted(false); // resurrect if needed
        } else {
            descr = ds.CreateJobDescriptor(rid);  // create a new descriptor
        }
        m_Descriptors.push_back(descr);
    }
}


bool CNetBlastLoadingJob::HasErrors() const
{
    return ! m_Errors.empty();
}


void CNetBlastLoadingJob::GetErrors(vector<string>& errors) const
{
    errors = m_Errors;
}


/// iterates on the given RIDs and creates Project Items or accumulates error
void CNetBlastLoadingJob::x_CreateProjectItems()
{
    // iterate by ids and create items
    for( size_t i = 0;  i < m_Descriptors.size();  i++  )   {
        CNetBlastJobDescriptor& descr = *m_Descriptors[i];
        string rid = descr.GetRID(); //TODO

        CNetBlastJobDescriptor::EState state = descr.GetState();
        if(state == CNetBlastJobDescriptor::eInitial  ||
           state == CNetBlastJobDescriptor::eSubmitted  ) {
            descr.Check();
        }

        switch(descr.GetState())    {
        case CNetBlastJobDescriptor::eRetrieved:
        case CNetBlastJobDescriptor::eCompleted:    {{
            /// job finished, results are ready
            CRef<CSeq_align_set> results = descr.Retrieve();
            if(results) {
                x_CreateProjectItemsFromBlastResult(*results, descr.GetRemoteBlast());
            } else {
                string s = "BLAST Search for RID " + rid  + " produced no results";
                m_Errors.push_back(s);
            }
            break;
        }}
        case CNetBlastJobDescriptor::eSubmitted:   {{
            string s = "RID " + rid  + " is still pending";
            m_Errors.push_back(s);
            break;
        }}
        case CNetBlastJobDescriptor::eFailed:   {{
            CFastMutexGuard lock(m_Mutex);
            m_Errors.push_back(descr.GetErrors()); // job failed
            break;
        }}

        case CNetBlastJobDescriptor::eExpired:  {{
            string s = "RID " + descr.GetRID() + " is invalid or expired";
            CFastMutexGuard lock(m_Mutex);
            m_Errors.push_back(s);
            break;
        }}
        default:
            break;
        }
    }
}

void CNetBlastLoadingJob::x_CreateProjectItemsFromBlastResult(CSeq_align_set& results,
                                                              CRef<blast::CRemoteBlast> RemoteBlast)
{
    // If we're passed a scope, use it. If not, create a default one.
    CRef<objects::CScope> scope;
    if (!m_Scope) {
        CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
        scope.Reset(new objects::CScope(*obj_mgr));
        scope->AddDefaults();
    }
    else {
        scope.Reset(m_Scope);
    }

    CGBProjectHelper::SRIDStatInfo RIDStatInfo;
    set<string> EmptyFilter;
    
    CGBProjectHelper::AddProjectItemsFromRID(scope,
                                      results,
                                      false,
                                      RemoteBlast,
                                      EmptyFilter,
                                      *this,
                                      RIDStatInfo);
}

END_NCBI_SCOPE
