/*  $Id: object_manager_engine.cpp 43654 2019-08-13 18:41:30Z katargir $
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
 * Authors:  Andrey Yazhuk, Eugene Vasilchenko, Anatoliy Kuznetsov
 *
 * File Description:
 *   Implementation of Application Job Engine using object manager prefetcher.
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/object_manager_engine.hpp>
#include <corelib/ncbi_system.hpp>


BEGIN_NCBI_SCOPE

using namespace objects;

class CObjectManagerEngineAction : public CObject,
                                         public IPrefetchAction
{
public:
    CObjectManagerEngineAction(IAppJob* job)
        : m_Job(job)
        {
        }

    IAppJob& GetIAppJob(void)
        {
            return *m_Job;
        }

    bool Execute(CRef<CPrefetchRequest> token)
        {
            IAppJob::EJobState state = m_Job->Run();
            if ( state == IAppJob::eCanceled ) {
                //NCBI_THROW(CPrefetchCanceled, eCanceled, "canceled");
                return false;
            }
            return state == IAppJob::eCompleted;
        }

private:
    CIRef<IAppJob> m_Job;
};


///////////////////////////////////////////////////////////////////////////////
/// CObjectManagerEngine
CObjectManagerEngine::CObjectManagerEngine(unsigned max_threads)
: m_PrefetchManager(max_threads), m_Listener(NULL)
{
}


CObjectManagerEngine::~CObjectManagerEngine()
{
}

void CObjectManagerEngine::ShutDown()
{
    SetListener( NULL );
    m_PrefetchManager.Shutdown();
}


void CObjectManagerEngine::RequestCancel()
{
    m_PrefetchManager.CancelAllTasks();
}

bool CObjectManagerEngine::IsActive()
{
    return true;
}


void CObjectManagerEngine::SetListener(IAppJobEngineListener* listener)
{
    CMutexGuard guard(m_Mutex);
    m_Listener = listener;
}


void CObjectManagerEngine::StartJob(IAppJob& job, IEngineParams* params)
{
    CMutexGuard guard(m_Mutex);
    m_Jobs[&job] = m_PrefetchManager.AddAction(new CObjectManagerEngineAction(&job), this);
}


void CObjectManagerEngine::CancelJob(IAppJob& job)
{
    CMutexGuard guard(m_Mutex);
    job.RequestCancel();
    TJobHandles::iterator iter = m_Jobs.find(&job);
    if ( iter == m_Jobs.end() ) {
        NCBI_THROW(CAppJobException, eEngine_UnknownJob, "cannot cancel job");
    }
    iter->second->RequestToCancel();
}


void CObjectManagerEngine::SuspendJob(IAppJob& job)
{
    NCBI_THROW(CException, eUnknown, "cannot suspend job");
}


void CObjectManagerEngine::ResumeJob(IAppJob& job)
{
    NCBI_THROW(CException, eUnknown, "cannot resume job");
}


IAppJobEngine::TJobState
CObjectManagerEngine::GetJobState(IAppJob& job) const
{
    CMutexGuard guard(m_Mutex);
    TJobHandles::const_iterator iter = m_Jobs.find(&job);
    if ( iter == m_Jobs.end() ) {
        return IAppJob::eInvalid;
    }
    CThreadPool_Task::EStatus status = iter->second->GetStatus();
    switch ( status ) {
    case CThreadPool_Task::eIdle:
    case CThreadPool_Task::eQueued:
        return IAppJob::eSuspended;
    case CThreadPool_Task::eExecuting:
        return IAppJob::eRunning;
    case CThreadPool_Task::eCompleted:
        return IAppJob::eCompleted;
    case CThreadPool_Task::eFailed:
        return IAppJob::eFailed;
    case CThreadPool_Task::eCanceled:
        return IAppJob::eCanceled;
    default:
        return IAppJob::eInvalid;
    }
}


void CObjectManagerEngine::PrefetchNotify(
    CRef<CPrefetchRequest> token, EEvent event
){
    IAppJob::EJobState new_state;
    bool remove = true;

    switch ( event ) {
    case eStarted:
        remove = false;
        new_state = IAppJob::eRunning;
        break;
    case eCompleted:
        new_state = IAppJob::eCompleted;
        break;
    case eFailed:
        new_state = IAppJob::eFailed;
        break;
    case eCanceled:
        new_state = IAppJob::eCanceled;
        break;
    default:
        return;
    }

    // Here we try to avoid multiple mutex lock (Engine -> AppJobDispatcher)
    // Engine can throw a eEngineBusy error code, caller takes a short break with all mutex free
    //
    
    for( unsigned re_try = 0;; ){

        IAppJob* app_job = 0;
        {{
            CMutexGuard guard(m_Mutex);
            CObjectManagerEngineAction& action =
                dynamic_cast<CObjectManagerEngineAction&>( *token->GetAction() )
            ;
            app_job = &action.GetIAppJob();
        }}

        try {
            if( m_Listener ){
                m_Listener->OnEngineJobStateChanged( *app_job, new_state );
            } else {
                LOG_POST( Warning << 
                    "OME::PrefetchNotify: No listener already! Probably, late job." 
                );
            }

            if( remove ){
                CMutexGuard guard(m_Mutex);
                m_Jobs.erase( app_job );
            }

            break;

        } catch( CAppJobException& ex ){
            if( ex.GetErrCode() == CAppJobException::eEngineBusy ){
                SleepMilliSec( ++re_try );

            } else {
                throw;
            }
        }
    }
}


END_NCBI_SCOPE
