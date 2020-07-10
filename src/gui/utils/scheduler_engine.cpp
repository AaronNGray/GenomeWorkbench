/*  $Id: scheduler_engine.cpp 35730 2016-06-15 20:46:53Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>


#include <gui/utils/scheduler_engine.hpp>

#include <corelib/ncbi_system.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CSchedulerEngineThread
class CSchedulerEngineThread : public CThread
{
public:
    CSchedulerEngineThread(CSchedulerEngine* engine)
        :   m_Engine(engine)    {}

    virtual void* Main(void)
    {
        m_Engine->x_WorkOnThread();
        return NULL;
    }

protected:
    CSchedulerEngine* m_Engine;
};


///////////////////////////////////////////////////////////////////////////////
/// CSchedulerEngine
CSchedulerEngine::CSchedulerEngine()
:   m_WakeSignal(0, 10000000),
    m_Listener(NULL)
    {
    m_WorkerThread.Reset(new CSchedulerEngineThread(this));
    m_WorkerThread->Run(CThread::fRunNice);
}

CSchedulerEngine::~CSchedulerEngine()
{
}


void CSchedulerEngine::ShutDown()
{
    LOG_POST(Info << "CSchedulerEngine::ShutDown()");
    m_StopRequested.Set(1);
    m_WakeSignal.Post();
    m_WorkerThread->Join();
    LOG_POST(Info << "CSchedulerEngine::ShutDown(): completed");
}

void CSchedulerEngine::RequestCancel()
{
    // TODO: implement
}

bool CSchedulerEngine::IsActive()
{
    /// this engine is active - it send notifications when a job changes state
    return true;
}


void CSchedulerEngine::SetListener(IAppJobEngineListener* listener)
{
    m_Listener = listener;
}

void CSchedulerEngine::StartJob(IAppJob& job, IEngineParams* /*params*/)
{
    //_ASSERT(params == NULL); // do not support any

    // we only accept jobs that implement IWaitPeriod - verify
    IWaitPeriod* waitPeriod = dynamic_cast<IWaitPeriod*>(&job);
    if (!waitPeriod) {
        string s = string("Job ") + job.GetDescr();
        s += " does not implement IWaitPeriod interface";
        NCBI_THROW(CAppJobException, eEngine_InvalidOperation, s);
    }

    CMutexGuard guard(m_Mutex);

    /// check that job is new
    for (const auto& i : m_Jobs) {
        if (i.m_Job == &job) {
            string s = string("Job ") + job.GetDescr();
            s += " has been already started on this Engine";
            NCBI_THROW(CAppJobException, eEngine_InvalidOperation, s);
        }
    }

    if (m_Listener)
        m_Listener->OnEngineJobStateChanged(job, IAppJob::eRunning);

    CJobRecord record(job);
    m_Jobs.push_back(record);

    m_WakeSignal.Post();
}

void CSchedulerEngine::CancelJob(IAppJob& job)
{
    job.RequestCancel();

    CMutexGuard guard(m_Mutex);
    CTime now(CTime::eCurrent);
    for (auto& i : m_Jobs) {
        if (i.m_Job == &job) {
            i.m_NextExec = CTime(CTime::eCurrent);
            m_WakeSignal.Post();
            break;
        }
    }
}

void CSchedulerEngine::SuspendJob(IAppJob& /*job*/)
{
    static const char* kMsg = "CSchedulerEngine::SuspendJob()\
                              - operation is not supported";
    NCBI_THROW(CAppJobException, eEngine_InvalidOperation, kMsg);
}


void CSchedulerEngine::ResumeJob(IAppJob& /*job*/)
{
    static const char* kMsg = "CSchedulerEngine::ResumeJob()\
                              - operation is not supported";
    NCBI_THROW(CAppJobException, eEngine_InvalidOperation, kMsg);
}


IAppJob::EJobState CSchedulerEngine::GetJobState(IAppJob& job) const
{
    CMutexGuard guard(m_Mutex);

    for (const auto& i : m_Jobs) {
        if (i.m_Job == &job) return i.m_JobState;
    }

    return IAppJob::eInvalid;
}


void CSchedulerEngine::x_WorkOnThread()
{
    LOG_POST(Info << "Scheduler Engine -  working thread started");

    unsigned long wait = 10000;

    for (;;) {
        m_WakeSignal.TryWait(wait);

        if (m_StopRequested.Get())
            break;

        x_RunJob();

        wait = x_GetWaitPeriod();
    }

    LOG_POST(Info << "Scheduler Engine -  working thread finished");
}

unsigned long CSchedulerEngine::x_GetWaitPeriod()
{
    Int8 wait = 100000;
    CMutexGuard guard(m_Mutex);
    CTime now(CTime::eCurrent);
    for (auto& i : m_Jobs) {
        wait = min(wait, i.m_NextExec.DiffSecond(now));
    }
    return (unsigned long)max((Int8)0, wait);
}

void CSchedulerEngine::x_RunJob() {
    IAppJob* toRun = 0;
    {
        CMutexGuard guard(m_Mutex);
        CTime now(CTime::eCurrent);
        for (auto& i : m_Jobs) {
            if (i.m_NextExec <= now) {
                toRun = i.m_Job;
                break;
            }
        }
    }

    if (!toRun)
        return;


    IAppJob::EJobState state = IAppJob::eFailed;
    try {
        state = toRun->Run();
    }
    NCBI_CATCH("CSchedulerEngine::x_RunJobs()");

    CMutexGuard guard(m_Mutex);
    list<CJobRecord>::iterator it = m_Jobs.begin();
    for (;it != m_Jobs.end(); ++it) {
        if (it->m_Job == toRun)
            break;
    }

    if (it == m_Jobs.end()) return;

    if (state != IAppJob::eRunning) {
        it->m_JobState = state;
        if (m_Listener)
            m_Listener->OnEngineJobStateChanged(*toRun, state);
        m_Jobs.erase(it);
    }
    else {
        it->m_NextExec = CTime(CTime::eCurrent) + dynamic_cast<IWaitPeriod*>(toRun)->GetWaitPeriod();
    }
}

END_NCBI_SCOPE
