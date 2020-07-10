/*  $Id: thread_pool_engine.cpp 34060 2015-10-27 19:54:34Z katargir $
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
 * File Description:
 *
 *
 */


#include <ncbi_pch.hpp>


#include <gui/utils/thread_pool_engine.hpp>

#include <corelib/ncbi_system.hpp>


BEGIN_NCBI_SCOPE


CThreadPoolEngine::CThreadTask::CThreadTask(IAppJob& job, CThreadPoolEngine& engine)
: m_Job(&job), 
  m_Engine(engine),
  m_JobState(IAppJob::eInvalid)
{
}

CThreadPool_Task::EStatus CThreadPoolEngine::CThreadTask::Execute(void)
{
    CThreadPool_Task::EStatus task_status = CThreadPool_Task::eFailed;

    try {
        m_JobState = m_Job->Run();
        m_Engine.x_OnTaskStatusChange(*this, m_JobState);
    } 
    catch(std::exception&)
    {
        m_Engine.x_OnTaskStatusChange(*this, IAppJob::eFailed);
        throw;
    }

    //m_Engine.RemoveJobRecord(m_Job);

    switch (m_JobState)
    {
    case IAppJob::eInvalid:
        task_status = CThreadPool_Task::eFailed;
        break;
    case IAppJob::eRunning:
        task_status = CThreadPool_Task::eFailed; // unexpected
        break;
    case IAppJob::eCompleted:
        task_status = CThreadPool_Task::eCompleted;
        break;
    case IAppJob::eFailed:
        task_status = CThreadPool_Task::eFailed; 
        break;
    case IAppJob::eCanceled: 
        task_status = CThreadPool_Task::eCanceled; 
        break;
    case IAppJob::eSuspended:
        task_status = CThreadPool_Task::eFailed; // unexpected
        break;
    }
    return task_status;
}



CThreadPoolEngine::CThreadPoolEngine(unsigned int max_threads,
                                     unsigned int queue_size)
:   m_ThreadPool(max_threads, queue_size),
    m_Listener(NULL)
{
}


CThreadPoolEngine::~CThreadPoolEngine()
{
}


void CThreadPoolEngine::ShutDown()
{
    // Wait for all threads to finish
    m_ThreadPool.Abort();
}

void CThreadPoolEngine::RequestCancel()
{
    m_ThreadPool.CancelTasks(
        CThreadPool::fCancelExecutingTasks |
        CThreadPool::fCancelQueuedTasks);
}

bool CThreadPoolEngine::IsActive()
{
    /// this engine is active - it send notifications when a job changes state
    return true;
}


void CThreadPoolEngine::SetListener(IAppJobEngineListener* listener)
{
    m_Listener = listener;
}


void CThreadPoolEngine::StartJob(IAppJob& job, IEngineParams* params)
{
    _ASSERT(params == NULL); // do not support any

    CMutexGuard guard(m_Mutex);

    /// check that job is new
    TJobToHandle::const_iterator it = m_JobToHandle.find(&job);
    if(it != m_JobToHandle.end())  {
        string s("Job ");
        s += job.GetDescr();
        s += " has been already started on this Engine";
        NCBI_THROW(CAppJobException, eEngine_InvalidOperation, s);
    } else {
        // create a new Request for the given Job
        // post the Request to Queue and add to Registry
        CRef<CThreadTask> task(new CThreadTask(job, *this));
        m_JobToHandle[&job] = task;
        m_ThreadPool.AddTask(task);
    }
}


void CThreadPoolEngine::CancelJob(IAppJob& job)
{
    CMutexGuard guard(m_Mutex);

    TJobToHandle::iterator it = x_Find(job);
    if (it == m_JobToHandle.end()) {
        return;
    }
    THandle& handle = it->second;
    handle->GetJob().RequestCancel();
    m_ThreadPool.CancelTask(handle);
    m_JobToHandle.erase(&job);
}


void CThreadPoolEngine::SuspendJob(IAppJob& job)
{
    static const char* kMsg = "CThreadPoolEngine::SuspendJob()\
                              - operation is not supported";
    NCBI_THROW(CAppJobException, eEngine_InvalidOperation, kMsg);
}


void CThreadPoolEngine::ResumeJob(IAppJob& job)
{
    static const char* kMsg = "CThreadPoolEngine::ResumeJob()\
                              - operation is not supported";
    NCBI_THROW(CAppJobException, eEngine_InvalidOperation, kMsg);
}


IAppJob::EJobState CThreadPoolEngine::GetJobState(IAppJob& job) const
{
    CMutexGuard guard(m_Mutex);

    TJobToHandle::const_iterator it = x_Find(job);

    if (it == m_JobToHandle.end()) {
        return IAppJob::eInvalid;
    }
    
    const THandle& handle = it->second;
    CThreadPool_Task::EStatus task_status = handle->GetStatus();

    switch (task_status)
    {
    case CThreadPool_Task::eIdle:     return IAppJob::eRunning;
    case CThreadPool_Task::eQueued:   return IAppJob::eRunning;
    case CThreadPool_Task::eExecuting:return IAppJob::eRunning;
    case CThreadPool_Task::eCompleted:return IAppJob::eCompleted;
    case CThreadPool_Task::eFailed:   return IAppJob::eFailed;
    case CThreadPool_Task::eCanceled: return IAppJob::eCanceled;        
    }
    return IAppJob::eInvalid;

}


void CThreadPoolEngine::x_OnTaskStatusChange(CThreadTask& task,
                                             IAppJob::EJobState new_status)
{
    IAppJob& job = task.GetJob();

    if(m_Listener)  {
        IAppJob::EJobState state = new_status;

        // the Job marked as Completed by the Pool in reality could have been
        // canceled or failed
        if(state == IAppJob::eCompleted) {
            CThreadPool_Task::EStatus status = task.GetStatus();
            switch(status)
            {
            case CThreadPool_Task::eFailed:
                state = IAppJob::eFailed;
                break;
            case CThreadPool_Task::eCanceled:
                state = IAppJob::eCanceled;
                break;
            default:
                break;
            }
            //state = request.GetJobState();
        }

        unsigned re_try = 0;
        do {
            try {
                m_Listener->OnEngineJobStateChanged(job, state);
                re_try = 0;
            } catch (CAppJobException& ex) {
                if (ex.GetErrCode() == CAppJobException::eEngineBusy) {
                    ++re_try;
                }
                else {
                    throw;
                }
            }
            if (re_try) {
                SleepMilliSec(re_try);
                continue;
            }
        } while (re_try);

    }

    if(new_status == IAppJob::eCompleted  ||
       new_status == IAppJob::eCanceled  || 
       new_status == IAppJob::eFailed) {
       // job is in terminal state -- delete it from the map
       CMutexGuard guard(m_Mutex);
       m_JobToHandle.erase(&job);
    }
}


/// try to find a job, if not found - throws an exception
CThreadPoolEngine::TJobToHandle::iterator
    CThreadPoolEngine::x_Find(IAppJob& job)
{
    TJobToHandle::iterator it = m_JobToHandle.find(&job);
    if(it == m_JobToHandle.end())  {
        string s("Job ");
        s += job.GetDescr();
        s += " is not registered with this Engine";
        NCBI_THROW(CAppJobException, eEngine_UnknownJob, s);
    }
    return it;
}

void CThreadPoolEngine::RemoveJobRecord(IAppJob& job)
{
    CMutexGuard guard(m_Mutex);
    m_JobToHandle.erase(&job);
}

/// try to find a job, if not found - throws an exception
CThreadPoolEngine::TJobToHandle::const_iterator
    CThreadPoolEngine::x_Find(IAppJob& job) const
{
    TJobToHandle::const_iterator it = m_JobToHandle.find(&job);
    if(it == m_JobToHandle.end())  {
        string s("Job ");
        s += job.GetDescr();
        s += " is not registred with this Engine";
        NCBI_THROW(CAppJobException, eEngine_UnknownJob, s);
    }
    return it;
}



END_NCBI_SCOPE
