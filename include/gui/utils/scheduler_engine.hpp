#ifndef GUI_OBJUTILS___SCHEDULER_ENGINE__HPP
#define GUI_OBJUTILS___SCHEDULER_ENGINE__HPP

/*  $Id: scheduler_engine.hpp 35731 2016-06-15 21:00:09Z katargir $
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

#include <gui/utils/app_job_engine.hpp>

#include <corelib/ncbithr.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIUTILS_EXPORT IWaitPeriod
{
public:
    virtual ~IWaitPeriod() {}

    /// returns a period of time after which the task shall be executed again
    virtual CTimeSpan   GetWaitPeriod() = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// CSchedulerEngine
/// CSchedulerEngine is an engine that can be used with CAppJobDispatcher.
/// The idea of CSchedulerEngine is to use a single background thread for
/// periodic execution of multiple jobs. This kind of engine is useful for
/// doing quick repetitive tasks such as polling external services every once
/// in while. All jobs share the same thread so they should execute really quick.
/// To be compatible with CSchedulerEngine a job needs to implement
/// IWaitPeriod interface.

class NCBI_GUIUTILS_EXPORT CSchedulerEngine
    :   public CObject,
        public IAppJobEngine
{
    friend class CSchedulerEngineThread;
public:
    typedef IAppJob::EJobState TJobState;

    CSchedulerEngine();
    virtual ~CSchedulerEngine();

    /// @name IAppJobEngine implementation
    /// @{
    virtual bool    IsActive();

    virtual void    SetListener(IAppJobEngineListener* listener);

    void    StartJob(IAppJob& job, IEngineParams* /*params = NULL*/);

    virtual void    CancelJob(IAppJob& job);
    virtual void    SuspendJob(IAppJob& /*job*/);
    virtual void    ResumeJob(IAppJob& /*job*/);

    virtual TJobState   GetJobState(IAppJob& job) const;

    virtual void    ShutDown();
    virtual void    RequestCancel();
    /// @}

protected:
    /// record that holds infromation about a job
    struct CJobRecord
    {
        CJobRecord(IAppJob& job) : 
            m_Job(&job),
            m_JobState(TJobState::eRunning),
            m_NextExec(CTime::eCurrent) {}

        CIRef<IAppJob> m_Job;
        TJobState      m_JobState;
        CTime          m_NextExec;
    };

protected:
    /// performs job execution on a separate scheduling thread
    void    x_WorkOnThread();
    void    x_RunJob();

    unsigned long x_GetWaitPeriod();

    mutable CMutex      m_Mutex;
    list<CJobRecord>    m_Jobs;
    CSemaphore          m_WakeSignal;
    CAtomicCounter_WithAutoInit m_StopRequested;

    IAppJobEngineListener*  m_Listener;

    CRef<CThread>    m_WorkerThread;
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___SCHEDULER_ENGINE__HPP
