/*  $Id: app_job_dispatcher.cpp 34478 2016-01-13 16:19:10Z katargir $
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

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/app_job_engine.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/event_translator.hpp>

#include <corelib/ncbimtx.hpp>
#include <corelib/ncbi_system.hpp>


BEGIN_NCBI_SCOPE

#define DISP_CATCH_AND_THROW(message, job) \
        catch (CException& e) {   \
            string s(message);      \
            s += job.GetDescr();    \
            NCBI_RETHROW(e, CAppJobException, eEngineFailed, s);  \
        } catch (std::exception& ee) {   \
            NCBI_THROW(CAppJobException, eFatalError, ee.what()); \
        }

#define DISP_CATCH_AND_REPORT(message, job) \
        catch (CException& e) {   \
            ERR_POST(message << job.GetDescr());   \
            ERR_POST(e.ReportAll());      \
        } catch (std::exception& ee) {   \
            ERR_POST(message  <<  ee.what());    \
        } 



///////////////////////////////////////////////////////////////////////////////
/// CAppJobDispatcher::SJobRecord
CAppJobDispatcher::SJobRecord::SJobRecord(IAppJob& job, TJobID id,
                                          TJobState state,
                                          IAppJobEngine& engine,
                                          CAppJobEventTranslator* listener,
                                          int report_period, bool auto_delete)
:   m_Job(&job), m_ID(id), m_State(state),
    m_Engine(engine), m_Listener(listener),
    m_ReportPeriod(report_period),
    m_AutoDelete(auto_delete)
{
    if (m_ReportPeriod > 0) {
        if (m_ReportPeriod < 3) // limit the minimum report period 
            m_ReportPeriod = 3;
    }
}


///////////////////////////////////////////////////////////////////////////////
///  CAppJobDispatcher
DEFINE_CLASS_STATIC_MUTEX(CAppJobDispatcher::sm_Mutex);

CRef<CAppJobDispatcher> CAppJobDispatcher::sm_Dispatcher;

CAppJobDispatcher& CAppJobDispatcher::GetInstance()
{
    if( ! sm_Dispatcher) {
        CMutexGuard Guard(sm_Mutex);
        if( ! sm_Dispatcher) {
            sm_Dispatcher.Reset(new CAppJobDispatcher());
        }
    }
    return *sm_Dispatcher;
}


void CAppJobDispatcher::ReleaseInstance()
{
    CMutexGuard Guard(sm_Mutex);
    sm_Dispatcher.Reset();
}


CAppJobDispatcher::CAppJobDispatcher()
:   m_LastJobID(eInvalidJobID),
    m_Mute(false),
    m_CancelVect(bm::BM_GAP),
    m_ShutDownInProgress(false)
{
}


CAppJobDispatcher::~CAppJobDispatcher()
{
}


void CAppJobDispatcher::Lock() const
{
    m_MainMutex.Lock();
}


void CAppJobDispatcher::Unlock() const
{
    m_MainMutex.Unlock();
}


void CAppJobDispatcher::ShutDown()
{
    //LOG_POST(Info << "CAppJobDispatcher::ShutDown()");

    static const char *kMessage = "CAppJobDispatcher::ShutDown() ";

    m_ShutDownInProgress = true;

    {{
    TDispatcherGuard guard(*this);

    /// shutdown polling
    m_PollQueue.clear();

    /// cancel all running Jobs and delete all records
    NON_CONST_ITERATE(TPtrToRec, it, m_PtrToRec)  {
        SJobRecord* rec = it->second;
        if(rec->m_State == IAppJob::eRunning) {
            IAppJob& job = rec->m_Job.GetObject();

            try {
                rec->m_Engine.CancelJob(job);
            } catch(CAppJobException& e)    {
                // analyze what happened
                switch(e.GetErrCode())  {
                case CAppJobException::eEngine_UnknownJob:
                case CAppJobException::eEngine_InvalidOperation:
                    break; // this is normal
                default:
                    ERR_POST(kMessage << job.GetDescr());
                    ERR_POST(e.ReportAll());
                }
            } catch (std::exception& ee) {
                ERR_POST(kMessage << ee.what());
            } 
        }
        delete rec;
    }
    m_PtrToRec.clear();
    // clear m_JobRecs to block processing of notifications from Engines
    m_JobRecs.clear();

    }}


    // now shut down all engines
    NON_CONST_ITERATE(TNameToEngine, it, m_NameToEngine) {
        IAppJobEngine& engine = *it->second;
        try {
            engine.ShutDown();
        } NCBI_CATCH(kMessage);
    }

    /// clear Engines
    m_NameToEngine.clear();
}


bool CAppJobDispatcher::IdleCallback()
{
    return x_PollEngines();
}


bool CAppJobDispatcher::RegisterEngine(const string& name, IAppJobEngine& engine)
{
    _ASSERT(&engine != NULL);

    CFastMutexGuard guard(m_EngineMutex);

    TNameToEngine::const_iterator it = m_NameToEngine.find(name);
    if(it == m_NameToEngine.end())  {
        CIRef<IAppJobEngine> ref(&engine);
        m_NameToEngine[name] = ref;
        if(engine.IsActive())  {
            engine.SetListener(this);
        }
        return true;
    } else {
        ERR_POST("CAppJobDispatcher::RegisterEngine() engine \"" << name <<
                 "\" is already registered");
        return false;
    }
}

void  CAppJobDispatcher::CancelAllJobs()
{
    CFastMutexGuard guard(m_EngineMutex);

    NON_CONST_ITERATE(TNameToEngine, it, m_NameToEngine) {
        it->second->RequestCancel();
    }
}


CAppJobDispatcher::TJobID
    CAppJobDispatcher::StartJob(IAppJob& job, const string& engine_name,
                                IEngineParams* params)
{
    // start in passive mode - no default listener, no auto delete
    return x_StartJob(job, engine_name, NULL, 0, false, params);
}


CAppJobDispatcher::TJobID
    CAppJobDispatcher::StartJob(IAppJob& job, const string& engine_name,
                                CEventHandler& listener, int report_period,
                                bool auto_delete, IEngineParams* params)
{
    CAppJobEventTranslator* translator = new CAppJobEventTranslator(listener);
    translator->SetDispatcher(*this);
    return x_StartJob(job, engine_name, translator, report_period,
                      auto_delete, params);
}


CAppJobDispatcher::TJobID
    CAppJobDispatcher::StartJob(IAppJob& job, const string& engine_name,
                                CAppJobEventTranslator& listener, int report_period,
                                bool auto_delete, IEngineParams* params)
{
    return x_StartJob(job, engine_name, &listener, report_period,
                      auto_delete, params);
}


static const char* kCannotStart = "Cannot start the job ";
static const char* kJobRegistered = "Cannot start the job - it is already registered";
static const char* kUnknownEngine = "Cannot start the job - engine is not registred";


CAppJobDispatcher::TJobID
    CAppJobDispatcher::x_StartJob(IAppJob& job, const string& engine_name,
                                  CAppJobEventTranslator* listener, int report_period,
                                  bool auto_delete, IEngineParams* params)
{
    CIRef<IAppJob> job_rf(&job);

    IAppJobEngine* engine = x_GetRegisteredEngine(engine_name);    
    if(!engine) {
        NCBI_THROW(CAppJobException, eInvalidOperation, kUnknownEngine);
    }
    TJobID id = eInvalidJobID;

    // create job record and register it
    {{
        TDispatcherGuard guard(*this);
        SJobRecord* rec = x_GetJobRecord(job);
        if(rec) {
            NCBI_THROW(CAppJobException, eInvalidOperation, kJobRegistered);
        }

        id = ++m_LastJobID;
        rec = new SJobRecord(job, id, IAppJob::eRunning, *engine,
                             listener, report_period, auto_delete);
        x_AddJobRecord(*rec);
    }}


    // Submit job to the engine 
    // In this section we do NOT hold the main lock  
    {{
        unsigned re_try = 0;
        do {
            try {
                engine->StartJob(job, params);
            }
            catch (CAppJobException& ex) {
                if (ex.GetErrCode() == CAppJobException::eEngineBusy) {
                    ++re_try;
                    SleepMilliSec(2); 
                }
                re_try = 0;
            }
            catch (CException& e) {
                string s(kCannotStart);
                s += job.GetDescr();
                NCBI_RETHROW(e, CAppJobException, eEngineFailed, s);
            } catch (std::exception& e) {
                NCBI_THROW(CAppJobException, eFatalError, e.what());
            }

        } while (re_try);
    }}

    // Post notification about job start
    {{
    TDispatcherGuard guard(*this);
    x_OnJobStarted(job, *engine, listener, report_period, auto_delete);
    }}

    return id;

}


///  handles state transition
void CAppJobDispatcher::x_OnJobStarted(IAppJob& job, IAppJobEngine& /*engine*/,
                                       CAppJobEventTranslator* listener,
                                       int report_period, bool /*auto_delete*/)
{
    SJobRecord* rec = x_GetJobRecord(job);
    if (rec == 0) {
        // Job record already deleted (job finished?)
        // in the current implementation it is hard to check (no post-execution trace)...
        return;
    }
    if (rec->m_State == IAppJob::eRunning) {
        if(report_period > 0)   {
            _ASSERT(listener);
            if(listener)    {
                time_t t = time(NULL) + report_period;
                m_PollQueue.insert(TTimeToItem::value_type(t, SQueueItem(rec->m_ID, true)));
            }
        }
    }
}


static const char* kCancelErrEngine =
    "CAppJobDispatcher::CancelJob() cannot cancel job";
static const char* kCancelErrNotRunning =
    "CAppJobDispatcher::CancelJob() cannot cancel job - the job is not running.";
static const char* kCancelErrNotReg =
    "CAppJobDispatcher::CancelJob() cannot cancel job  - the job is not registered.";
static const char* kDefaultErrorMessage = 
    "CAppJobDispatcher: Tool failed with unspecified error";



bool CAppJobDispatcher::x_IsCanceled(int job_id) const
{
    return m_CancelVect[job_id];
}


void CAppJobDispatcher::CancelJob(TJobID job_id)
{
    TDispatcherGuard guard(*this);

    if (m_CancelVect[job_id]) {
        LOG_POST("Repeated job cancel: ignored. job=" << job_id);
        return;
    }
    m_CancelVect[job_id] = true;

    SJobRecord* rec = x_GetJobRecord(job_id);
    if(rec) {

        try {
            switch (rec->m_State) {
            case IAppJob::eRunning:
                rec->m_Engine.CancelJob(*rec->m_Job);
                break;
            case IAppJob::eCompleted:
            case IAppJob::eFailed:
            case IAppJob::eCanceled:
                // nothing to do
                break;
            default:
                NCBI_THROW(CAppJobException, eInvalidOperation, kCancelErrNotRunning);
            }
        }
        DISP_CATCH_AND_THROW(kCancelErrEngine, rec->m_Job.GetObject())

    } else {
        NCBI_THROW(CAppJobException, eUnknownJob, kCancelErrNotReg);
    }
}


static const char* kSuspendErrEngine =
    "CAppJobDispatcher::SuspendJob() cannot suspend job";
static const char* kSuspendErrNotRunning =
    "CAppJobDispatcher::SuspendJob() cannot suspend job - the job is not running.";
static const char* kSuspendErrNotReg =
    "CAppJobDispatcher::SuspendJob() cannot suspend job  - the job is not registered.";


void CAppJobDispatcher::SuspendJob(TJobID job_id)
{
    TDispatcherGuard guard(*this);

    SJobRecord* rec = x_GetJobRecord(job_id);
    if(rec) {
        if(rec->m_State == IAppJob::eRunning)   {
            IAppJob& job = rec->m_Job.GetObject();
            try {
                rec->m_Engine.SuspendJob(job);
            }
            DISP_CATCH_AND_THROW(kSuspendErrEngine, job)

            // if we are here the request was accepted
            rec->m_State = IAppJob::eSuspended;
            x_OnJobStateChanged(*rec, rec->m_State); //TODO ?
        } else {
            NCBI_THROW(CAppJobException, eInvalidOperation, kSuspendErrNotRunning);
        }
    } else {
        NCBI_THROW(CAppJobException, eUnknownJob, kSuspendErrNotReg);
    }
}


static const char* kResumeErrEngine =
    "CAppJobDispatcher::ResumeJob() cannot resume job";
static const char* kResumeErrNotRunning =
    "CAppJobDispatcher::ResumeJob() cannot resume job - the job is not running.";
static const char* kResumeErrNotReg =
    "CAppJobDispatcher::ResumeJob() cannot resume job - the job is not registered.";

void CAppJobDispatcher::ResumeJob(TJobID job_id)
{
    TDispatcherGuard guard(*this);

    SJobRecord* rec = x_GetJobRecord(job_id);
    if(rec) {
        if(rec->m_State == IAppJob::eSuspended) {
            IAppJob& job = rec->m_Job.GetObject();
            try {
                rec->m_Engine.ResumeJob(job);
            }
            DISP_CATCH_AND_REPORT(kResumeErrEngine, job)

            // if we are here the request was accepted
            rec->m_State = IAppJob::eRunning;
            //x_OnJobStateChanged(*rec, rec->m_State); TODO
        } else {
            NCBI_THROW(CAppJobException, eInvalidOperation, kResumeErrNotRunning);
        }
    } else {
        NCBI_THROW(CAppJobException, eUnknownJob, kResumeErrNotReg);
    }
}


static const char* kDeleteErrEngine =
    "CAppJobDispatcher::DeleteJob() cannot delete job";

bool CAppJobDispatcher::DeleteJob(TJobID job_id)
{
    TDispatcherGuard guard(*this);

    SJobRecord* rec = x_GetJobRecord(job_id);
    if( rec ){
        CIRef<IAppJob> job(rec->m_Job);
        IAppJobEngine& engine = rec->m_Engine;

        m_CancelVect.set(rec->m_ID, true); // mark as canceled

        // delete the job from Registry
        bool running = rec->m_State == IAppJob::eRunning;
        x_RemoveJobRecord(*rec);
        delete rec;

        guard.Release();  // Important! Release guard without waiting for engine

        if (running) {
            try {
                engine.CancelJob(*job);
            } 
            catch(CAppJobException& e)    {
                // analyze what happened
                switch(e.GetErrCode())  {
                case CAppJobException::eEngine_UnknownJob:
                case CAppJobException::eEngine_InvalidOperation:
                    break; // this is normal
                default:
                    NCBI_RETHROW(e, CAppJobException, eEngineFailed, kDeleteErrEngine);
                }
            } catch (std::exception& ee) {
                NCBI_THROW(CAppJobException, eFatalError, ee.what());
            } 
            return true;
        }
    }
    
    return false;
}


//////////////////////////////////////////////////////////////////////////////
/// All Get() functions return values stored in the Registy not the actual

CAppJobDispatcher::TJobState CAppJobDispatcher::GetJobState(TJobID job_id)
{
    TDispatcherGuard guard(*this);

    const SJobRecord* rec = x_GetJobRecord(job_id);
    return rec ? rec->m_State : IAppJob::eInvalid;
}


CConstIRef<IAppJobProgress> CAppJobDispatcher::GetJobProgress(TJobID job_id)
{
    TDispatcherGuard guard(*this);

    SJobRecord* rec = x_GetJobRecord(job_id);
    if(rec) {
        if(rec->ActiveProgress()) {
            /// active reporting - x_PollEngines() function updates the record
            /// periodically, just return the last value
            return rec->m_Progress;
        } else {
            ///
            return rec->m_Job->GetProgress();
        }
    }
    return CConstIRef<IAppJobProgress>(NULL);
}


CRef<CObject> CAppJobDispatcher::GetJobResult(TJobID job_id)
{
    TDispatcherGuard guard(*this);

    SJobRecord* rec = x_GetJobRecord(job_id);
    CRef<CObject> res(rec ? rec->m_Job->GetResult().GetPointer() : NULL);
    return res;
}


CConstIRef<IAppJobError> CAppJobDispatcher::GetJobError(TJobID job_id)
{
    TDispatcherGuard guard(*this);

    SJobRecord* rec = x_GetJobRecord(job_id);
    
    CConstIRef<IAppJobError> err(rec ? rec->m_Job->GetError().GetPointer() : NULL);

    // if job failed and there is no error object, create one here marked at CAppJobDispatcher
    if (!err && (rec->m_State == IAppJob::eFailed)) {
        err.Reset(new CAppJobError(kDefaultErrorMessage));
    }

    return err;
}


///////////////////////////////////////////////////////////////////////////////
/// Helper functions
/// these functions rely on external synchronization and do not throw / catch exception

IAppJobEngine* CAppJobDispatcher::x_GetRegisteredEngine(const string& engine_name)
{
    CFastMutexGuard guard(m_EngineMutex);

    TNameToEngine::iterator it = m_NameToEngine.find(engine_name);
    if(it == m_NameToEngine.end())  {
        ERR_POST("x_CAppJobDispatcher::x_GetRegisteredEngine() engine \""
                 << engine_name << "\" is not registered.");
        return NULL;
    }
    return it->second.GetPointer();
}


CAppJobDispatcher::SJobRecord* CAppJobDispatcher::x_GetJobRecord(IAppJob& job)
{
    TPtrToRec::const_iterator it = m_PtrToRec.find(&job);
    return (it == m_PtrToRec.end()) ? NULL : it->second;
}


CAppJobDispatcher::SJobRecord* CAppJobDispatcher::x_GetJobRecord(TJobID job_id)
{
    if(job_id > eInvalidJobID  &&  job_id <= m_LastJobID)   {
        TIDToRec::const_iterator it = m_JobRecs.find(job_id);
        return (it != m_JobRecs.end()) ? it->second : NULL;
    }
    return NULL;
}


void CAppJobDispatcher::x_AddJobRecord(SJobRecord& rec)
{
    _ASSERT(m_JobRecs.find(rec.m_ID) == m_JobRecs.end());
    _ASSERT(m_PtrToRec.find(rec.m_Job.GetPointer()) == m_PtrToRec.end());

    m_JobRecs[rec.m_ID] = &rec;
    m_PtrToRec[rec.m_Job.GetPointer()] = &rec;
}


void CAppJobDispatcher::x_RemoveJobRecord(SJobRecord& rec)
{
    _ASSERT(m_JobRecs.find(rec.m_ID) != m_JobRecs.end());
    _ASSERT(m_PtrToRec.find(rec.m_Job.GetPointer()) != m_PtrToRec.end());

    m_JobRecs.erase(rec.m_ID);
    m_PtrToRec.erase(rec.m_Job.GetPointer());
}


static const char* kListenerException =
    "Exception in CAppJobDispatcher::x_OnJobStateChangedNotify() ";

// notify job Listener about state change
void CAppJobDispatcher::x_OnJobStateChangedNotify(SJobRecord& rec)
{
    //if(rec.m_State == IAppJob::eCompleted)    {
    //    LOG_POST("CAppJobDispatcher::x_OnJobStateChangedNotify - Completed, id = " << rec.m_ID);
    //}

    if(rec.m_Listener && !m_Mute)  {
        //LOG_POST("m_Listener->OnJobStateChanged() " << rec.m_ID << "  " << StateToStr(rec.m_State));
        try {
            rec.m_Listener->OnJobStateChanged(&rec, rec.m_State);
        }
        DISP_CATCH_AND_REPORT(kListenerException, (*rec.m_Job));
    }
    else {
        LOG_POST("CAppJobDispatcher: Job Notification not delivered (muted/no listener) ");
    }
}


void CAppJobDispatcher::x_OnJobProgressNotify(SJobRecord& rec)
{
    if(rec.m_Listener && !m_Mute)  {
        rec.m_Listener->OnJobProgress(&rec);
    }
}

void CAppJobDispatcher::Mute(bool bMute)
{
    m_Mute = bMute;        
}

///////////////////////////////////////////////////////////////////////////////
/// IAppJobEngineListener


/// handles state change notifications from active Engines
void CAppJobDispatcher::OnEngineJobStateChanged(IAppJob& job, TJobState new_state)
{
    //string s = StateToStr(new_state);
    //LOG_POST("CAppJobDispatcher::OnEngineJobStateChanged()  new state = " <<
    //         s << ",  " << job.GetDescr());


    if (m_ShutDownInProgress) {
        return;
    }

    // Job execution start is not important, so it gets silenced to minimize mutex clash
    if (new_state == IAppJob::eRunning) {
        return;
    }



    // Anatoliy July-10-2009 :
    // Code being re-written to fix bug GB-500 reported by Andrew Shkeda
    // 
    // JobDispatcher uses 2 connected event queues here
    // m_StateEventQueue is used to pre-queue the job state change event, one of the functions
    // it guarantees Job::Complete is not coming before Job::Running (logical state-races in event delivery)
    // so all notofication is serialized here (probably a good deal of parallel processing too...)
    //
    
    {{
        CFastMutexGuard guard(m_StateEventMutex);
        m_StateEventQueue.push_back(SJobStateEvent(job, new_state));
    }}

    // Second part of the story: 
    // 
    // spin-flush to send all events to the event re-translator
    // Here we MUST guaratee the event pre-queue is flushed to GUI retranslator (another queue)
    // The spin guarantees that the queue is flushed, TryLock-sleep is "to avoid a deadlock"
    // (previous author), but no idea of what resources are involved..
    // 
    // TODO: need to find a clear way to fix the "deadlock" and remove the m_StateEventQueue
    //
    for (unsigned i = 0; true; ++i) {
        if(m_MainMutex.TryLock()) {
            try {
                x_FlushStateEventQueue();
            } 
            catch (exception&)
            {
                m_MainMutex.Unlock();
                throw;
            }
            m_MainMutex.Unlock();
            break; // queue is empty at this point, our job notification being retranslated to main GUI queue
        }
        if (i > 5) {
            SleepMilliSec( 5+i );
        }
        if (i > 20) {
            //LOG_POST("CAppJobDispatcher: event spin-translation takes too long...");
            NCBI_THROW(CAppJobException, eEngineBusy, "");
        }
    } // for

    
}



// this function must be exception-safe
void CAppJobDispatcher::x_FlushStateEventQueue()
{
    // loop to process one event per mutex lock at a time 
    while (true) {
        {{
            CFastMutexGuard guard(m_StateEventMutex);
            if(m_StateEventQueue.empty())  {
                break;
            }

            SJobStateEvent& event = *m_StateEventQueue.begin();
            
            IAppJob& job = *event.m_Job;
            TJobState new_state = event.m_NewState;

            try {
                m_StateEventQueue.pop_front();
                guard.Release(); // Important! m_StateEventMutex is no protection from this point

                SJobRecord* rec = x_GetJobRecord(job);
                if (rec) {
                    x_OnJobStateChanged(*rec, new_state);

                    if (!m_Mute)
                    {
                        // call to event re-translator
                        if (rec->m_Listener) {
                            rec->m_Listener->OnJobStateChanged(rec, rec->m_State);
                        }
                    }

                    // if job finished and AutoDelete flag is specified - delete the Job record
                    if (IsTerminal(rec->m_State)  &&  rec->m_AutoDelete) {
                        x_RemoveJobRecord(*rec);
                        delete rec;
                    }
                } 
            }
            DISP_CATCH_AND_REPORT("CAppJobDispatcher::x_FlushStateEventQueue() notification failed", job)

        }}
    } // while
}

string CAppJobDispatcher::GetStatusString(TJobState job_state)
{
    switch (job_state)
    {
    case IAppJob::eInvalid:
        return "eInvalid";
    case IAppJob::eRunning:
        return "eRunning";
    case IAppJob::eCompleted:
        return "eCompleted";
    case IAppJob::eFailed:
        return "eFailed";
    case IAppJob::eCanceled:
        return "eCanceled";
    case IAppJob::eSuspended:
        return "eSuspended";
    default:
        return "Unknown";
    }
}



// validates state transition and issues notifications
void CAppJobDispatcher::x_OnJobStateChanged(SJobRecord& rec, TJobState new_state)
{
    // check that the new state is valid (there is a valid transition from
    // the current state. If there is not, then Engine is wrong - throw an
    // exception
    bool error = false;
    switch(rec.m_State)   {
    case IAppJob::eRunning:
    case IAppJob::eCanceled:
        error = (new_state == IAppJob::eInvalid);
        break;
    case IAppJob::eCompleted:
    case IAppJob::eFailed:
        error = true; // Job finished, now new states possible
        break;
    case IAppJob::eSuspended:
        error = (new_state != IAppJob::eRunning);
        break;
    default:
        _ASSERT(false); // not a valid state
    }

    if(error)   {
        string s = "Job " + rec.m_Job->GetDescr();
        s += "Transition from state ";
        s += StateToStr(rec.m_State);
        s += " to state ";
        s += StateToStr(new_state);
        s += " is invalid.";
        NCBI_THROW(CAppJobException, eEngineFailed, s);
        //return; false; // unreachable
    } else {
        // change state and notify clients
        rec.m_State = new_state;
        //return true;
    }
}


string CAppJobDispatcher::StateToStr(TJobState state)
{
    switch(state) {
    case IAppJob::eInvalid:     return "Invalid";
    case IAppJob::eRunning:     return "Running";
    case IAppJob::eCompleted:   return "Completed";
    case IAppJob::eFailed:      return "Failed";
    case IAppJob::eCanceled:    return "Cancelled";
    case IAppJob::eSuspended:   return "Suspended";
    }
    _ASSERT(false);
    return "";
}


bool CAppJobDispatcher::IsTerminal(TJobState state)
{
    return (state == IAppJob::eCompleted)  ||  (state == IAppJob::eFailed)
                                           ||  (state == IAppJob::eCanceled);
}


void CAppJobDispatcher::x_VerifyProgressNotNull(CAppJobDispatcher::SJobRecord& rec)
{
#ifdef _DEBUG
    if( ! rec.m_Progress)  {
        string descr = rec.m_Job->GetDescr();
        LOG_POST(Error << "Active progress reporting is requested for job \""
            << descr << "\".\n The job must implement GetProgress() funtion!");
        _ASSERT(false);
    }
#endif
}


bool CAppJobDispatcher::x_PollEngines()
{
    //LOG_POST("CAppJobDispatcher::x_PollEngines()");
    /// check Job state every kPeriod seconds. All local Engines are supposed to
    /// be Active, so this delay is for remote Engines only
    static const int kPeriod = 15;
    bool done_something = false;

    // fast check to see if the queue is not empty
    if( ! m_PollQueue.empty())  {
        time_t now = time(NULL);

        TDispatcherGuard guard(*this);

        // now synchronized check
        if( ! m_PollQueue.empty())  {
            time_t check_time = m_PollQueue.begin()->first;
            if(now >= check_time)   {
                done_something = true;

                // it's time to check this Job
                SQueueItem& item = m_PollQueue.begin()->second;
                //SJobRecord* rec = item.m_Rec;
                SJobRecord* rec = x_GetJobRecord(item.m_JobId);
                bool progress = item.m_Progress;

                /// delete this item from the Queue (item variable becomes invalid)
                m_PollQueue.erase(m_PollQueue.begin());
                if (rec == NULL) { // record is already removed, no need to poll
                    return done_something;
                }

                if(progress)    {
                    // obtain Progress information from the job
                    if(rec->m_State == IAppJob::eRunning) {
                        rec->m_Progress = rec->m_Job->GetProgress();
                        x_VerifyProgressNotNull(*rec);
                        x_OnJobProgressNotify(*rec);

                        time_t new_time = now + rec->m_ReportPeriod;
                        m_PollQueue.insert(TTimeToItem::value_type(new_time, SQueueItem(rec->m_ID, true)));
                    }
                } else {
                    // request Status information from the passive engine
                    IAppJob& job = rec->m_Job.GetObject();
                    try {
                        TJobState new_state = rec->m_Engine.GetJobState(job);

                        if(rec->m_State != new_state) {
                            x_OnJobStateChanged(*rec, new_state);
                            x_OnJobStateChangedNotify(*rec);
                        }
                    }
                    DISP_CATCH_AND_REPORT("CAppJobDispatcher::x_PollEngines()", job)

                    if(IsTerminal(rec->m_State))  {
                        // passive engine requires deleting jobs manually
                        x_RemoveJobRecord(*rec);
                        delete rec;
                    } else {
                        // reschedule Job monitoring
                        time_t new_time = now + kPeriod;
                        m_PollQueue.insert(TTimeToItem::value_type(new_time, SQueueItem(rec->m_ID, false)));
                    }
                }
            }
        }
    }
    return done_something;
}


void CAppJobDispatcher::RunSync(IAppJob& job, TJobID& jobId, CEventHandler& listener)
{
    jobId = 7777;

    IAppJob::EJobState state = job.Run();

    switch (state)
    {
    case IAppJob::eCompleted:
    {
        CRef<CObject> res(job.GetResult().GetPointer());
        CRef<CEvent> evt(new CAppJobNotification(jobId, res.GetPointer()));
        listener.Send(evt);
        break;
    }
    case IAppJob::eSuspended:
    case IAppJob::eRunning:
    case IAppJob::eFailed:
    {
        CConstIRef<IAppJobError> err(job.GetError().GetPointer());
        if (err) {
            CRef<CEvent> evt(new CAppJobNotification(jobId, *err));
            listener.Send(evt);
        }
        else {
            ERR_POST("Job failed -- NULL error job_id= " << jobId);
            CRef<CEvent> evt(new CAppJobNotification(jobId, state));
            listener.Send(evt);
        }
        break;
    }

    case IAppJob::eCanceled:
    {
        CRef<CEvent> evt(new CAppJobNotification(jobId, state));
        listener.Send(evt);
        break;
    }
    default:
        _ASSERT(false);
    }
}

END_NCBI_SCOPE
