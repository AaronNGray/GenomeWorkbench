/*  $Id: app_task_service.cpp 43784 2019-08-29 19:41:12Z katargir $
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

#include <gui/framework/app_task_service.hpp>

#include <gui/framework/event_log_service_impl.hpp>
#include <gui/framework/status_bar_service_impl.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <corelib/ncbimtx.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAppTaskService

string sGetMessageAndReport(const string& prefix, IAppTask& task, CException& e)
{
    ERR_POST(prefix << task.GetDescr() << ". " << e.GetMsg());
    ERR_POST(e.ReportAll());
    return e.GetMsg();
}

string sGetMessageAndReport(const string& prefix, IAppTask& task, std::exception& e)
{
    ERR_POST(prefix  << task.GetDescr() << ". "  <<  e.what());
    return e.what();
}


string sGetMessageAndReport(const string& prefix, IAppTask& task)
{
    ERR_POST(prefix  << task.GetDescr() <<  ". Unknown fatal error.");
    return "Unknown fatal error.";
}


BEGIN_EVENT_MAP(CAppTaskService, CEventHandler)
    ON_EVENT(CAppTaskServiceEvent, CAppTaskServiceEvent::eWakeUpSignal,
             &CAppTaskService::x_OnWakeUpSignal)
    ON_EVENT(CAppTaskServiceEvent, CAppTaskServiceEvent::eTaskCanceled,
             &CAppTaskService::x_OnTaskCanceled)
    ON_EVENT(CAppTaskServiceEvent, CAppTaskServiceEvent::eStatusUpdate,
             &CAppTaskService::x_OnStatusUpdate)
END_EVENT_MAP()


CAppTaskService::CAppTaskService()
:   m_ServiceLocator(NULL),
    m_EventLogService(NULL),
    m_ShutDown(false),
    m_RunningTask(NULL)
{
}


CAppTaskService::~CAppTaskService()
{
}


void CAppTaskService::InitService()
{
}


void CAppTaskService::ShutDownService()
{
}

void CAppTaskService::PreShutDownService() 
{
    CMutexGuard guard(m_Mutex);

    m_ShutDown = true;

    // mute app job dispatcher
    CAppJobDispatcher& disp = 
        CAppJobDispatcher::GetInstance();
    disp.Mute();

    // cancel all running tasks
    NON_CONST_ITERATE(TRecordMap, it, m_BackgoundedTasks)   {
        IAppTask& task = const_cast<IAppTask&>(*it->first);
        task.SetListener(NULL);
        task.CancelBackgrounded();
    }
    m_BackgoundedTasks.clear();

    // clear all remaining tasks
    TQueue::TAccessGuard q_guard(m_PendingTasks);

    for (TQueue::TAccessGuard::TIterator it = q_guard.Begin();  it != q_guard.End();  ++it) {
        (*it)->m_Task->SetListener(NULL);        
    }
    m_PendingTasks.Clear();
}


void CAppTaskService::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_ServiceLocator = srv_locator;

    if(m_ServiceLocator)    {
        m_EventLogService =
            m_ServiceLocator->GetServiceByType<CEventLogService>();

        _ASSERT(m_EventLogService);
    } else {
        m_EventLogService = NULL;
    }

}

bool CAppTaskService::IdleCallback()
{
    /// recursion counter; we need to guard against recursive calls because
    /// tasks can show dialogs that will run their own event loops and we
    /// must not execute other tasks within those loops - this will violate
    /// the rules of serial execution
    CMutexGuard guard(m_Mutex);

    if( ! m_ShutDown)   {
        x_ExecuteNextTaskInQueue();
    }
    return !m_PendingTasks.IsEmpty();
}


static const char* kShutDown =
    "Cannot perform operation, Task Manager has been shut down.";

void CAppTaskService::AddTask(IAppTask& task)
{
    LOG_POST(Info);
    LOG_POST(Info << "CAppTaskService::AddTask() " << task.GetDescr());

    if(m_ShutDown)   {
        NCBI_THROW(CAppTaskServiceException, eInvalidOperation, kShutDown);
    } else {
        CMutexGuard guard(m_Mutex);

        CRef<CTaskRecord> rec(new CTaskRecord());
        rec->m_Task.Reset(&task);

        m_PendingTasks.Push(rec);

        Post(CRef<CEvent>(new CAppTaskServiceGuiUpdateEvent()));
    }
}


static const char* kExecErr =
    "CAppTaskService::x_ExecuteNextTaskInQueue() - cannot execute task.";

//TODO this function became too long
void CAppTaskService::x_ExecuteNextTaskInQueue()
{
    // get a task from the Pending Queue
    if( ! m_PendingTasks.IsEmpty()) {

        CRef<CTaskRecord> rec = m_PendingTasks.Pop();

        _ASSERT(rec->m_Task);

        IAppTask* task = rec->m_Task.GetPointer();
        TTaskState state = task->GetState();
        //bool progress = task->IsVisible();

        m_RunningTask = task;

        _ASSERT(state == IAppTask::eInitial || state == IAppTask::eBackgrounded);

        if(state == IAppTask::eInitial) {
            rec->m_StartTime = CTime(CTime::eCurrent).GetTimeT();
            x_LogTask(IEventRecord::eInfo, "Started ", *task, " task");
        }

        //if(m_ServiceLocator  &&  progress)    {
            //CIRef<IStatusBarService> sb_srv = m_ServiceLocator->GetServiceByType<IStatusBarService>();
            //sb_srv->ShowStatusProgress();
        //}

        // now run the Task and catch exceptions
        string err_msg;
        try {
            task->SetListener(this);
            state = task->Run();
        }
        catch (CException& e) {
            err_msg = sGetMessageAndReport(kExecErr, *task, e);
        } catch (std::exception& ee) {
            err_msg = sGetMessageAndReport(kExecErr, *task, ee);
        } 

        m_RunningTask = NULL;

        if( ! err_msg.empty())  {
            state = IAppTask::eFailed;
        }

        switch(state)    {
        case IAppTask::eInitial:
            // if we are here  - Run did not finish correctly
            x_ReportTaskFailure(*task, "Unexpected error while executing a task.", err_msg);
            task->SetListener(NULL);
            //TODO post event to notify observers
            break;

        case IAppTask::eCanceled:
            // user canceled the action
            x_ReportTaskFailure(*task, "Task canceled by user", err_msg);
            task->SetListener(NULL);
            break;

        case IAppTask::eFailed:
            x_ReportTaskFailure(*task, task->GetStatusText(), err_msg);
            task->SetListener(NULL);
            break;

        case IAppTask::eBackgrounded:
            {{
                // add record to m_BackgoundedTasks and wait for wake-up signals
                m_BackgoundedTasks[rec->m_Task] = rec;
                if(task->IsVisible())    {
                    //x_NotifyObservers();
                    CMutexGuard guard(m_Mutex);
                    Send(CRef<CEvent>(new CAppTaskServiceGuiUpdateEvent()));
                }
            }}
            break;

        case IAppTask::eCompleted:
            {{
                task->SetListener(NULL);

                x_LogTask(IEventRecord::eInfo, "Task ", *task, " is completed");
            }}
            break;

        default:
            _ASSERT(false);
            ERR_POST("CAppTaskService::x_ExecuteNextTaskInQueue() - unexpected state.");
        }

        //if(m_ServiceLocator  &&  progress) {
          //  CIRef<IStatusBarService> sb_srv = m_ServiceLocator->GetServiceByType<IStatusBarService>();
          //  sb_srv->HideStatusProgress();
        //}
    }
}


void CAppTaskService::x_LogTask(IEventRecord::EType type,
                                const string& prefix, IAppTask& task,
                                const string& postfix, const string& details)
{
    if(task.IsVisible())    {
        if(m_EventLogService)   {
            string title = prefix + "\"" + task.GetDescr() + "\"" + postfix;
            m_EventLogService->AddRecord(new CEventRecord(type, title, details, time(0)));
        }

        //x_NotifyObservers();
        CMutexGuard guard(m_Mutex);
        Send(CRef<CEvent>(new CAppTaskServiceGuiUpdateEvent()));
    }
}


void CAppTaskService::x_ReportTaskFailure(IAppTask& task, const string& err_msg, const string& details)
{
    ERR_POST("CAppTaskService - task \"" << task.GetDescr() <<
             "\" failed. Details : " << err_msg);

    string s = err_msg + " " + details;
    x_LogTask(IEventRecord::eError, "Task ", task, " failed. ", s);
}



static const char* kCancelUnknown =
    "Cannot cancel the task, it is not running in background";
static const char* kCancelFatal =
    "Unexpected fatal error while trying to cancel the task ";

// cancel a backgrounded task
bool CAppTaskService::CancelTask(IAppTask& task, bool throw_on_error)
{
    LOG_POST(Info << "CAppTaskService::CancelTask() " << task.GetDescr());

    CMutexGuard guard(m_Mutex);

    TTaskRef ref(&task);
    TRecordMap::iterator it = m_BackgoundedTasks.find(ref);

    if(it == m_BackgoundedTasks.end()) {  // Task is pending
        TQueue::TAccessGuard guard(m_PendingTasks);
        TQueue::TAccessGuard::TIterator it2 = guard.Begin();

        for (; it2 != guard.End(); ++it2) {
            if (ref.GetPointer() == (**it2).m_Task.GetPointer()) {
                guard.Erase(it2);
                ref->OnCancel();
                break;
            }
        }

        if (it2 == guard.End()) {
            if (throw_on_error) {
                NCBI_THROW(CAppTaskServiceException, eInvalidOperation, kCancelUnknown);
            }
            return false;
        }
    } else { // Task is backgrounded
        // make a controlled attempt to cancel background task
        try {
            task.CancelBackgrounded(); 
        }
        catch (CException& e) {
            sGetMessageAndReport(kCancelFatal, task, e);
            return false;
        } catch (std::exception& ee) {
            sGetMessageAndReport(kCancelFatal, task, ee);
            return false;
        } 
    }
    return true;
}


static const char* kUnexpectedTaskNot =
    "Task Manager  - notification for unexpected task (";

// handles an event that indicated that a task has been canceled
void CAppTaskService::x_OnTaskCanceled(CEvent* event)
{
    CMutexGuard guard(m_Mutex);

    if(m_ShutDown)
        return;

    CAppTaskServiceEvent* tm_evt = dynamic_cast<CAppTaskServiceEvent*>(event);
    _ASSERT(tm_evt);

    if(tm_evt)  {
        CIRef<IAppTask> task = tm_evt->GetTask();
        _ASSERT(task.GetPointer());

        TRecordMap::iterator it = m_BackgoundedTasks.find(task);
        if(it == m_BackgoundedTasks.end())  {
            NCBI_THROW(CAppTaskServiceException, eUnexpectedTask, kUnexpectedTaskNot + task->GetDescr() + ").");
        } else {
            m_BackgoundedTasks.erase(task);
            x_LogTask(IEventRecord::eInfo, "Canceled ", *task, " task");
        }
    }

    Post(CRef<CEvent>(new CAppTaskServiceGuiUpdateEvent()));
}


void CAppTaskService::x_OnWakeUpSignal(CEvent* event)
{
    CMutexGuard guard(m_Mutex);

    if(m_ShutDown)
        return;

    CAppTaskServiceEvent* tm_evt = dynamic_cast<CAppTaskServiceEvent*>(event);
    _ASSERT(tm_evt);
    if(tm_evt)  {
        CIRef<IAppTask> task = tm_evt->GetTask();
        _ASSERT(task.GetPointer());

        TRecordMap::iterator it = m_BackgoundedTasks.find(task);
        if(it == m_BackgoundedTasks.end())  {
            NCBI_THROW(CAppTaskServiceException, eUnexpectedTask, kUnexpectedTaskNot + task->GetDescr() + ").");
        } else {
            // move the task to foreground queue
            CMutexGuard guard(m_Mutex);

            CRef<CTaskRecord> rec(it->second);
            m_BackgoundedTasks.erase(it);
            m_PendingTasks.Push(rec);

            //x_NotifyObservers();
            Send(CRef<CEvent>(new CAppTaskServiceGuiUpdateEvent()));
        }
    }
}


void CAppTaskService::x_OnStatusUpdate(CEvent* event)
{
    Post(CRef<CEvent>(new CAppTaskServiceGuiUpdateEvent()));
}


void CAppTaskService::GetPendingTasks(TRecRefVec& tasks)
{
    CMutexGuard guard(m_Mutex);

    // obtain Access Guard to perform a bulk operation
    TQueue::TAccessGuard q_guard(m_PendingTasks);

    for (TQueue::TAccessGuard::TIterator it = q_guard.Begin();  it != q_guard.End();  ++it) {
        TRecordRef rec = *it;
        tasks.push_back(rec);
    }
}


void CAppTaskService::GetBackgroundedTasks(TRecRefVec& tasks)
{
    CMutexGuard guard(m_Mutex);

    tasks.reserve(tasks.size() + m_BackgoundedTasks.size());
    NON_CONST_ITERATE(TRecordMap, it, m_BackgoundedTasks)   {
        tasks.push_back(it->second);
    }
}


int CAppTaskService::GetRunningTasksCount(bool vis_only)
{
    int n = 0;
    {
        CMutexGuard guard(m_Mutex);
        NON_CONST_ITERATE(TRecordMap, it, m_BackgoundedTasks)   {
            IAppTask& task = *it->second->m_Task;
            if( ! vis_only  ||  task.IsVisible())   {
                n++;
            }
        }
    }
    //we are not afraid of dirty read on this, because it is atomic
    if(m_RunningTask)   {
        n++;
    }
    return n;
}


END_NCBI_SCOPE
