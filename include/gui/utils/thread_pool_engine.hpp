#ifndef GUI_OBJUTILS___THREAD_POOL_ENGINE__HPP
#define GUI_OBJUTILS___THREAD_POOL_ENGINE__HPP

/*  $Id: thread_pool_engine.hpp 24876 2011-12-16 16:03:41Z kuznets $
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
 */

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <gui/utils/app_job_engine.hpp>

#include <util/thread_pool.hpp>

BEGIN_NCBI_SCOPE


class CAppJobRequest;


///////////////////////////////////////////////////////////////////////////////
/// CThreadPoolEngine
class NCBI_GUIUTILS_EXPORT CThreadPoolEngine
    :   public CObject,
        public IAppJobEngine
{
public:
    friend class CAppJobRequest;
    typedef IAppJob::EJobState TJobState;

    CThreadPoolEngine(unsigned int max_threads, unsigned int queue_size);
    virtual ~CThreadPoolEngine();

    /// @name IAppJobEngine implementation
    /// @{
    virtual bool    IsActive();

    virtual void    SetListener(IAppJobEngineListener* listener);

    void    StartJob(IAppJob& job, IEngineParams* params = NULL);

    virtual void    CancelJob(IAppJob& job);
    virtual void    SuspendJob(IAppJob& job);
    virtual void    ResumeJob(IAppJob& job);

    virtual TJobState   GetJobState(IAppJob& job) const;

    virtual void    ShutDown();
    virtual void RequestCancel();
    /// @}

protected:

    /// IAppJob to CThreadPool_Task Adaptor
    class CThreadTask : public CThreadPool_Task
    {
    public:
        CThreadTask(IAppJob& job, CThreadPoolEngine& engine);
        virtual EStatus Execute(void);

        IAppJob& GetJob() { return m_Job.GetObject(); }

    private:
        CIRef<IAppJob>      m_Job;
        CThreadPoolEngine&  m_Engine;
        IAppJob::EJobState  m_JobState;
    };

    friend class CThreadTask;


    typedef CRef<CThreadTask>                 THandle;
    typedef map<IAppJob*, CRef<CThreadTask> > TJobToHandle;

    typedef CQueueItemBase TReqStates;
    typedef CAppJobDispatcher TJobStates;


    virtual void    x_OnTaskStatusChange(CThreadTask&       task,
                                         IAppJob::EJobState new_status);
    void RemoveJobRecord(IAppJob& job);

    TJobToHandle::iterator  x_Find(IAppJob& job);
    TJobToHandle::const_iterator    x_Find(IAppJob& job) const;

protected:
    // local Registry of all Jobs
    mutable CMutex          m_Mutex;
    TJobToHandle            m_JobToHandle;  ///
    CThreadPool             m_ThreadPool; ///< pool of threads with a request queue
    IAppJobEngineListener*  m_Listener;
};



END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___THREAD_POOL_ENGINE__HPP
