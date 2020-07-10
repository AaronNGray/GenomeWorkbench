#ifndef GUI_OBJUTILS___APP_JOB_DISPATCHER__HPP
#define GUI_OBJUTILS___APP_JOB_DISPATCHER__HPP

/*  $Id: app_job_dispatcher.hpp 34478 2016-01-13 16:19:10Z katargir $
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

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <util/bitset/ncbi_bitset.hpp>
#include <gui/utils/app_job.hpp>

#include <gui/utils/event_handler.hpp>


BEGIN_NCBI_SCOPE

class IAppJobListener;
class IAppJobEngine;
class IEngineParams;

///////////////////////////////////////////////////////////////////////////////
/// IAppJobEngineListener - receives notifications about changes in job state.
/// Job Engines use this interface to notify external objects.
class NCBI_GUIUTILS_EXPORT IAppJobEngineListener
{
public:
    typedef IAppJob::EJobState TJobState;

    virtual ~IAppJobEngineListener()   {}

    virtual void    OnEngineJobStateChanged(IAppJob& job, TJobState new_state) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// IAppJobListener
/// Interface for components that need to be notified about changes in Jobs.
/*
class NCBI_GUIOBJUTILS_EXPORT IAppJobListener
{
public:
    typedef CAppJobDispatcher::TJobID   TJobID;
    typedef IAppJob::EJobState   TJobState;

    virtual ~IAppJobListener()    {};

    virtual void    SetDispatcher(CAppJobDispatcher& disp) = 0;

    virtual void    OnJobStateChanged(TJobID job_id, TJobState new_state) = 0;
    virtual void    OnJobProgress(TJobID job_id) = 0;
};
*/

///////////////////////////////////////////////////////////////////////////////
/// CAppJobException - Exception thrown by Job Dispatcher and Job Engines.
class NCBI_GUIOBJUTILS_EXPORT CAppJobException
            : EXCEPTION_VIRTUAL_BASE public CException
{
public:
    enum    EErrCode {
        eInvalidOperation,       ///< Dispatcher - operation is invalid
        eUnknownJob,             ///< Job record lost
        eEngine_IncompatibleJob, ///< the Job is incompatible with the Engine
        eEngine_InvalidParams,   ///< Invalid parameters provided to the Engine
        eEngine_InvalidOperation,///< Engine - operation is invalid
        eEngine_UnknownJob,      ///< the job is not registered in the Engine
        eEngineFailed,           ///< Engine failed to perforn an operation
        eEngineBusy,             ///< Engine is busy, caller needs to re-try the operation
        eFatalError              ///< unknown tragic error
    };
    NCBI_EXCEPTION_DEFAULT(CAppJobException, CException);
};

class CAppJobEventTranslator;

///////////////////////////////////////////////////////////////////////////////
/// CAppJobDispatcher
class NCBI_GUIOBJUTILS_EXPORT CAppJobDispatcher :
    public CObject,
    public IAppJobEngineListener
{
public:
    typedef IAppJob::EJobState TJobState;
    enum EConsts    {
        eInvalidJobID = -1
    };

    typedef int TJobID;
    typedef CGuard<CAppJobDispatcher>  TDispatcherGuard;

public:

    static CAppJobDispatcher& GetInstance();    /// get the Singleton Dispatcher
    static void ReleaseInstance();  /// Release the singleton

    CAppJobDispatcher();
    virtual ~CAppJobDispatcher();

    /// Terminates all jobs and releases Engines
    void    ShutDown();

    /// this function shall be called in the the application idle function.
    /// Calling this function gives Dispatcher a change to effectively poll
    /// passive Engines.
    bool    IdleCallback();

    /// Registers a new Engine, returns true if successful. Dispatcher does not
    /// assume ownership of the engine.
    bool    RegisterEngine(const string& name, IAppJobEngine& engine);

    /// Starts a Job on the specified engine in "passive mode" - no notifications
    /// or progress reports will be sent.
    TJobID  StartJob(IAppJob& job, const string& engine_name,
                     IEngineParams* params = NULL);

    /// Starts a Job on the specified engine; the provided listener will
    /// receive notification events when the Job's state changes.
    TJobID  StartJob(IAppJob& job, const string& engine_name,
                     CEventHandler& listener, int report_period = -1,
                     bool auto_delete = false,
                     IEngineParams* params = NULL);

    /// Starts a Job on the specified engine, Dispatcher will call methods of
    /// the provide listener when the Job's state changes.
    TJobID  StartJob(IAppJob& job, const string& engine_name,
                     CAppJobEventTranslator& listener, int report_period = -1,
                     bool auto_delete = false,
                     IEngineParams* params = NULL);

    /// Runs jon synchronously sending job notifications synchronously
    /// Returns when job is finished
    void RunSync(IAppJob& job, TJobID& jobId, CEventHandler& listener);

    /// Mute all notifications
    void    Mute(bool bMute=true);

    void    CancelJob(TJobID job_id);
    void    SuspendJob(TJobID job_id);
    void    ResumeJob(TJobID job_id);
    /// Request to cancel all jobs (func returns without waiting)
    void    CancelAllJobs();

    /// when a Job is deleted the listener is not notified
    ///
    /// @return true - if job was deleted, false if job not found
    bool    DeleteJob(TJobID job_id);

    TJobState   GetJobState(TJobID job_id);

    CConstIRef<IAppJobProgress> GetJobProgress(TJobID job_id);
    CRef<CObject>               GetJobResult(TJobID job_id);
    CConstIRef<IAppJobError>    GetJobError(TJobID job_id);

    /// @name IAppJobEngineListener
    /// @{
    virtual void    OnEngineJobStateChanged(IAppJob& job, TJobState new_state);
    /// @}

    static  string  StateToStr(TJobState state);
    static  bool    IsTerminal(TJobState state);

    // CGuard related
    void Lock()     const;
    void Unlock()   const;

    /// Debugging method for status strings
    static
    string GetStatusString(TJobState job_state);

protected:
    /// SJobRecord is a Job Descriptor
    struct SJobRecord;
    struct SQueueItem;

    friend struct SJobRecord;
    friend struct SQueueItem;

    /// SJobRecord describes a Job registered in Dispatcher
    struct  SJobRecord
    {
        CIRef<IAppJob>  m_Job;
        TJobID          m_ID;
        TJobState       m_State;
        IAppJobEngine&  m_Engine;
        CIRef<CAppJobEventTranslator>   m_Listener; ///< if not null - "active" mode
        int     m_ReportPeriod; ///< if > 0, active progress reporting is required

        CConstIRef<IAppJobProgress> m_Progress;
        bool    m_AutoDelete; ///< delete the record when job finishes

        SJobRecord(IAppJob& job, TJobID id, TJobState state,
                   IAppJobEngine& engine, CAppJobEventTranslator* listener,
                   int report_period, bool auto_delete);

        inline bool ActiveProgress() const {   return m_ReportPeriod > 0;  }
    };

    /// SQueueItem - element of the Polling Queue
    struct  SQueueItem
    {
        int     m_JobId;
        bool    m_Progress;

        SQueueItem() : m_JobId(0), m_Progress(false)   {}
        SQueueItem(int job_id, bool progress)
            : m_JobId(job_id), m_Progress(progress)  {}
    };

    struct SJobStateEvent
    {
        CIRef<IAppJob>  m_Job;
        TJobState       m_NewState;

        SJobStateEvent()    : m_NewState(IAppJob::eInvalid) {}
        SJobStateEvent(IAppJob& job, TJobState new_state)
            : m_Job(&job), m_NewState(new_state)    {}
    };

protected:
    void    x_AddJobRecord(SJobRecord& rec);
    void    x_RemoveJobRecord(SJobRecord& rec);

    TJobID  x_StartJob(IAppJob& job, const string& engine_name,
                       CAppJobEventTranslator* listener, int report_period,
                       bool auto_delete, IEngineParams* params);
    void  x_OnJobStarted(IAppJob& job, IAppJobEngine& /*engine*/,
                           CAppJobEventTranslator* listener, int report_period,
                         bool /*auto_delete*/);

    IAppJobEngine*  x_GetRegisteredEngine(const string& engine_name);
    SJobRecord*     x_GetJobRecord(TJobID job_id);
    SJobRecord*     x_GetJobRecord(IAppJob& job);

    /// Update job record, throws an exception if new state change is incorrect
    ///
    static
    void    x_OnJobStateChanged(SJobRecord& rec, TJobState new_state);

    void    x_OnJobStateChangedNotify(SJobRecord& rec);
    void    x_OnJobProgressNotify(SJobRecord& rec);

    bool    x_PollEngines();

    //void    x_OnJobStateChangedEvent(IAppJob& job, TJobState new_state);

    void    x_FlushStateEventQueue();

    void    x_VerifyProgressNotNull(CAppJobDispatcher::SJobRecord& rec);

    bool    x_IsCanceled(int job_id) const; 

protected:
    typedef map<string, CIRef<IAppJobEngine> > TNameToEngine;
    typedef map<TJobID, SJobRecord*>    TIDToRec;
    typedef map<IAppJob*, SJobRecord*>  TPtrToRec;
    typedef multimap<time_t, SQueueItem>    TTimeToItem;
    typedef list<SJobStateEvent>    TStateEventQueue;

    friend class CAppJobEventTranslator;
protected:
    DECLARE_CLASS_STATIC_MUTEX(sm_Mutex); // guards sm_Dispatcher

    /// global dispatcher, this instance is used by default in most cases,
    /// however it is possible to create another instance if needed
    static  CRef<CAppJobDispatcher>  sm_Dispatcher;

    /// guards this instance of the Dispatcher
    mutable CMutex  m_MainMutex;


    TNameToEngine   m_NameToEngine; ///< Engines Registry
    CFastMutex      m_EngineMutex;  ///< Engines registry mutex

    int     m_LastJobID;
    TIDToRec    m_JobRecs; /// Job Registry (index by JobID)
    TPtrToRec   m_PtrToRec; /// Job Index (by pointer)

    TTimeToItem  m_PollQueue; /// priority queue for Dispatcher to poll on

    /// a Queue that holds state change events posted by Engines
    TStateEventQueue    m_StateEventQueue;
    CFastMutex          m_StateEventMutex;

    bool                m_Mute;
    bm::bvector<>       m_CancelVect; ///< Canceled jobs vector
    bool                m_ShutDownInProgress; ///< Shutdown flag
};

///////////////////////////////////////////////////////////////////////////////
/// CAppJobNotification
/// Notification send by CAppJobEventTranslator
class NCBI_GUIUTILS_EXPORT CAppJobNotification:
    public CEvent
{
public:
    typedef IAppJob::EJobState TJobState;
    typedef CAppJobDispatcher::TJobID TJobID;

    enum EType  {
        eStateChanged,
        eProgress
    };

    CAppJobNotification(TJobID job_id, TJobState state);
    CAppJobNotification(TJobID job_id, CObject* result = NULL);
    CAppJobNotification(TJobID job_id, const IAppJobError& error);
    CAppJobNotification(TJobID job_id, const IAppJobProgress& progress);

    TJobID      GetJobID()  const   {   return m_JobID; }
    TJobState   GetState()  const   {   return m_State; }

    /// returns non-null pointer only if Completed or Running
    /// and has temporary results available
    CRef<CObject>   GetResult() const   {   return  m_Result;  }

    /// returns non-null pointer only if job Failed
    CConstIRef<IAppJobError>   GetError() const   {   return  m_Error;  }

    /// returns non-null pointer only if notification type is eProgress
    CConstIRef<IAppJobProgress>  GetProgress() const    { return m_Progress; }

protected:
    TJobID      m_JobID;
    TJobState   m_State;
    CRef<CObject>    m_Result;
    CConstIRef<IAppJobError>     m_Error;
    CConstIRef<IAppJobProgress>  m_Progress;
};


///////////////////////////////////////////////////////////////////////////////
/// CAppJobEventTranslator
/// Standard Listener that generates notification events
class NCBI_GUIOBJUTILS_EXPORT CAppJobEventTranslator :
    public CObject//,
    //public IAppJobListener
{
public:
    typedef CAppJobDispatcher::TJobID   TJobID;
    typedef IAppJob::EJobState          TJobState;

public:
    CAppJobEventTranslator(CEventHandler& target);

    /// @name IAppJobListener implementation
    /// @{
    void SetDispatcher(CAppJobDispatcher& disp);

    //void OnJobStateChanged(TJobID job_id, TJobState new_state);
    void OnJobStateChanged(CAppJobDispatcher::SJobRecord* job_rec, TJobState new_state);
    //void OnJobProgress(TJobID job_id);
    void OnJobProgress(CAppJobDispatcher::SJobRecord* job_rec);
    /// @}

protected:
    virtual void    x_NotifyObservers(CRef<CEvent> & evt);
private:
    CEventHandler&            m_TargetEventHandler;
    CWeakIRef<CEventHandler>  m_TargetWeakPtr;
    bool                      m_IsWeakTarget; 
    CAppJobDispatcher*        m_Dispatcher;
};




END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___APP_JOB_DISPATCHER__HPP
