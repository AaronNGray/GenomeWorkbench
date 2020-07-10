/*  $Id: search_tool_base.cpp 40276 2018-01-19 17:40:36Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>


#include <gui/core/search_tool_base.hpp>

#include <gui/core/data_mining_service.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/event_translator.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// IDMSearchTool

static const char* kDefaultErrMessage = "CSearchToolBase: Tool Execution Failed";

CSearchToolBase::CSearchToolBase()
:   m_ReportPeriod(1)
{   
}


CSearchToolBase::~CSearchToolBase()
{

}

string CSearchToolBase::GetPatternTypeStr(EPatternType ptype)
{
    switch (ptype)
    {
    case  eExactMatch: return "eExactMatch";
    case  eWildcard:   return "eWildcard";
    case  eRegexp:     return "eRegexp";
    case  eNamed:      return "eNamed";
    default: return "Unknown Search Pattern";
    }
}


BEGIN_EVENT_MAP(CSearchToolBase, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CSearchToolBase::OnAJNotification)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eProgress,
             &CSearchToolBase::OnProgress)
END_EVENT_MAP()


bool CSearchToolBase::PullSearch(IDMSearchQuery& query, IDMSearchFormController& listener)
{
    if ((GetFlags() & IDMSearchTool::eCache) && 
        !(GetFlags() & IDMSearchTool::eAutorun) && m_Cache) {    
        CRef<CSearchJobBase> job = x_CreateJob(query);   
        if(job) {
            CRef<CDMSearchResult> res(
                            new CDMSearchResult("", 
                                                job->x_GetNewOLTModel(),
                                                new CObjectList(*m_Cache))
                                               );
            //res->m_QueryDescr = GetDescr();
            listener.OnSearchFinished(*res);
        }    
        return true;
    }
    return false;
}

bool CSearchToolBase::StartSearch(IDMSearchQuery& query, IDMSearchFormController& listener)
{
    CRef<CSearchJobBase> job = x_CreateJob(query);
    if(job) {       
        try {
            // setup additional stuff
            job->SetFilters(m_Filters);

            string descr = job->GetDescr();

            LOG_POST(Info << descr);

            /// launch on ThreadPool, receive notifications, no progress reports
            /// delete the Job when completed
            CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
            int job_id = disp.StartJob(*job, "ThreadPool", *this, m_ReportPeriod, true);

            /// create session record
            SSession* session = new SSession;
            session->m_Query = &query;
            session->m_Listener = &listener;
            session->m_JobID = job_id;
            session->m_Descr = descr;
            m_Sessions.push_back(session);

            //LOG_POST("Started " << descr);
            return true;
        } catch(CAppJobException& e)  {
            ERR_POST("CSearchToolBase::StartSearch() - Failed to execute query");
            LOG_POST(Error << e.ReportAll());
            // TODO pass error msg to UI
            return false;
        }
    } else {
        // throw?
        return false;
    }
}


bool CSearchToolBase::CancelSearch(IDMSearchFormController& listener)
{
    int index = x_GetSessionIndexByListener(listener);

    if(index == -1) {
     //   _ASSERT(false);
       // ERR_POST("CSearchToolBase::CancelSearch() - invalid listener");
    } else {
        SSession* session = m_Sessions[index];
        try {
            CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
            disp.DeleteJob(session->m_JobID);

            m_Sessions.erase(m_Sessions.begin() + index);
            delete session;
            //LOG_POST("Canceled Job" );
            return true;
        } catch (CAppJobException&)  {
            ERR_POST("CSearchToolBase::CancelSearch() - Failed to cancel search");
            // TODO pass error msg to UI
        }
    }
    return false;
}


void CSearchToolBase::OnAJNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if(notn)    {
        int job_id = notn->GetJobID();
        int s_i = x_GetSessionIndexByID(job_id);
        if(s_i == -1) {
            ERR_POST("CSearchToolBase::OnAJNotification() - unknown Job ID " << job_id);
        } else {
            SSession* session = m_Sessions[s_i];

            switch(notn->GetState())    {
            case IAppJob::eCompleted:
                x_OnJobCompleted(*notn, *session);
                break;
            case IAppJob::eFailed:
                x_OnJobFailed(*notn, *session);
                break;
            case IAppJob::eCanceled:
                x_OnJobCanceled(*notn, *session);
                break;
            default:
                session = NULL;
            }

            if(session) {
                m_Sessions.erase(m_Sessions.begin() + s_i);
                delete session;
            }
        }
    }
}


void CSearchToolBase::OnProgress(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);

    _ASSERT(notn);

    if(notn)    {
        int job_id = notn->GetJobID();
        int s_i = x_GetSessionIndexByID(job_id);
        if(s_i == -1) {
            ERR_POST("CSearchToolBase::OnProgress() - unknown Job ID " << job_id);
        } else {
            SSession* session = m_Sessions[s_i];

            CConstIRef<IAppJobProgress> prg = notn->GetProgress();
            const CDMSearchProgress* progress =
                dynamic_cast<const CDMSearchProgress*>(prg.GetPointer());

            CDMSearchResult* result = NULL;
            if (progress) {
                CRef<CDMSearchResult> ref = progress->m_Result;
                result = ref.GetPointer();
            }

            _ASSERT(progress);
            if(progress  &&  result)  {
                string s = progress->GetText();
                session->m_Listener->OnSearchProgress(*result, s);
            }
        }
    }
}


string CSearchToolBase::GetJobDescr(IDMSearchFormController& form)
{
    string s;
    int index = x_GetSessionIndexByListener(form);
    if(index != -1) {
        s = m_Sessions[index]->m_Descr;
    }
    return s;
}


int CSearchToolBase::x_GetSessionIndexByID(int job_id)
{
    for( size_t i = 0;  i < m_Sessions.size(); i++)    {
        if(m_Sessions[i]->m_JobID == job_id)    {
            return (int)i;
        }
    }
    return -1;
}


int CSearchToolBase::x_GetSessionIndexByQuery(IDMSearchQuery& query)
{
    for( size_t i = 0;  i < m_Sessions.size(); i++)    {
        if(m_Sessions[i]->m_Query.GetPointer() == &query)    {
            return (int)i;
        }
    }
    return -1;
}


int CSearchToolBase::x_GetSessionIndexByListener(IDMSearchFormController& listener)
{
    for( size_t i = 0;  i < m_Sessions.size(); i++)    {
        if(m_Sessions[i]->m_Listener == &listener)    {
            return (int)i;
        }
    }
    return -1;
}


void CSearchToolBase::x_OnJobCompleted(CAppJobNotification& notify, SSession& session)
{
    CRef<CObject> res_obj = notify.GetResult();
    CDMSearchResult* result = dynamic_cast<CDMSearchResult*>(res_obj.GetPointer());
    if(result)  {
        session.m_Listener->OnSearchFinished(*result);

        if (GetFlags() & IDMSearchTool::eCache) {
            m_Cache.Reset(new CObjectList(*result->GetObjectList()));
        }
        //LOG_POST("Job Completed ");
    } else {
        m_Cache.Reset();
        session.m_Listener->OnSearchFailed("Internal error - no results available");
        ERR_POST("CSearchToolBase::x_OnJobCompleted() notification for job "
                 << session.m_JobID << " does not contain results.");
    }
}


void CSearchToolBase::x_OnJobFailed(CAppJobNotification& notify, SSession& session)
{
    m_Cache.Reset();        
    CConstIRef<IAppJobError> err = notify.GetError();
    if(err) {
        session.m_Listener->OnSearchFailed(err->GetText());
    } else {
        session.m_Listener->OnSearchFailed("Internal error - Search Failed");
        ERR_POST("CSearchToolBase::x_OnJobFailed() notification for job "
                 << session.m_JobID << " does not have an error object");
    } 
}


void CSearchToolBase::x_OnJobCanceled(CAppJobNotification& notify, SSession& session)
{
    //LOG_POST("CSearchToolBase::x_OnJobCanceled()");
    m_Cache.Reset();        
    session.m_Listener->OnSearchCanceled();
}


IDMSearchTool::TUIToolFlags CSearchToolBase::GetFlags(void)
{
    return IDMSearchTool::eCache;
}

IDMSearchTool::TConversions & CSearchToolBase::GetConversions(void)
{   
    return m_Conversions;
}

const IDMSearchTool::TFilters& CSearchToolBase::GetFilters(void) const
{   
    return m_Filters;
}

IDMSearchTool::TFilters& CSearchToolBase::SetFilters()
{   
    return m_Filters;
}

///////////////////////////////////////////////////////////////////////////////
/// CSearchJobBase

/// override the constructor, initialize m_Descr
CSearchJobBase::CSearchJobBase()
{
    m_MaxResultsCount = 0;
}


CSearchJobBase::~CSearchJobBase()
{
}


IAppJob::EJobState CSearchJobBase::Run()
{
    if(x_ValidateParams())  {
        x_PrepareSearch();

        EJobState state = x_DoSearch();
        if(state == eCompleted)    {
            x_TransferResults();
        }
        return state;
    }
    else {
        LOG_POST(Warning << m_Descr << " -- " << "Parameter validation failed!");
    }
    return eFailed;
}


void CSearchJobBase::x_PrepareSearch()
{
    CMutexGuard Guard(m_Mutex);

    m_ResultsCount = 0;
    m_AccList.Clear();
    x_SetupColumns(m_AccList);

    m_ProgressStr = "";

    // create new Temp Result object
    m_TempResult.Reset( 
                new CDMSearchResult(GetDescr(), x_GetNewOLTModel(), new CObjectList()) 
                       );

    //m_TempResult->m_Handler = x_GetColumnHandler();

    x_SetupColumns( *m_TempResult->GetObjectList() );

    /// reset Final Result and Error objects
    m_Result.Reset();
    m_Error.Reset(new CAppJobError(kDefaultErrMessage));
}


void CSearchJobBase::x_TransferResults()
{
    CMutexGuard Guard(m_Mutex);

    /// flush the last portion
    m_TempResult->GetObjectList()->Append(m_AccList);
    m_AccList.ClearRows();

    m_Result = m_TempResult;
    m_TempResult.Reset();

    if (m_MaxResultsCount > 0) {  // max result limitation exists?
        if (m_ResultsCount >= m_MaxResultsCount) {
            m_Result->SetIncomplete(true);
            m_Result->SetMaxSearchResult(m_ResultsCount);
        }
    }

    m_ProgressStr = "Finished.";
}


void CSearchJobBase::x_SetupColumns( CObjectList& obj_list )
{
    //override in derived classes if you need to add columns
}




CConstIRef<IAppJobProgress> CSearchJobBase::GetProgress()
{
    CMutexGuard Guard(m_Mutex); // synchronize

    CRef<CDMSearchResult> res;
    if(m_TempResult)    {
        CObjectList *obj_list = m_TempResult->GetObjectList();
        res.Reset( 
            new CDMSearchResult(m_TempResult->GetQueryDescr(),
                                x_GetNewOLTModel(),
                                new CObjectList( *obj_list)
                                ) 
            );

        //res->m_Handler = x_GetColumnHandler();
    }

    CConstIRef<IAppJobProgress> pr(new CDMSearchProgress(m_ProgressStr, res));
    return pr;
}


CRef<CObject> CSearchJobBase::GetResult()
{
    CMutexGuard Guard(m_Mutex); // synchronize

    return CRef<CObject>(m_Result.GetPointer());
}


CConstIRef<IAppJobError> CSearchJobBase::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CSearchJobBase::GetDescr() const
{
    return m_Descr;
}

/*
IObjectColumnHandler* CSearchJobBase::x_GetColumnHandler() const
{
    /// no handler by default, override this function if needed
    return NULL;
}
*/

CObjectListTableModel* CSearchJobBase::x_GetNewOLTModel() const
{
    return NULL;
}

END_NCBI_SCOPE
