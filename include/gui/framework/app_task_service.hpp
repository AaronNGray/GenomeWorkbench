#ifndef GUI_FRAMEWORK___APP_TASK_SERVICE__HPP
#define GUI_FRAMEWORK___APP_TASK_SERVICE__HPP

/*  $Id: app_task_service.hpp 31807 2014-11-17 19:32:54Z katargir $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>
#include <util/sync_queue.hpp>

#include <gui/framework/app_task.hpp>
#include <gui/framework/service.hpp>
#include <gui/framework/event_log_service.hpp>

#include <gui/utils/event_handler.hpp>

BEGIN_NCBI_SCOPE


class IEventLogService;
class IStatusBarService;
class CTaskDescr;

class CAppTaskServiceGuiUpdateEvent : public CEvent
{
public:
    enum EEventId {
        eUpdate
    };

    CAppTaskServiceGuiUpdateEvent() : CEvent(eUpdate) {}
};

///////////////////////////////////////////////////////////////////////////////
/// CAppTaskService - Application Task Service.

class NCBI_GUIFRAMEWORK_EXPORT CAppTaskService :
    public CObjectEx,
    public CEventHandler,
    public IService,
    public IServiceLocatorConsumer
{
    DECLARE_EVENT_MAP();
public:
    typedef IAppTask::ETaskState TTaskState;

    enum EConsts    {
        eInvalidTaskID = -1
    };

    typedef CIRef<IAppTask> TTaskRef;
    typedef vector<TTaskRef>    TTaskRefVec;

    class CTaskRecord : public CObject
    {
    public:
        CTaskRecord()
            : m_StartTime(CTime(CTime::eCurrent).GetTimeT())
        {
        }

        TTaskRef    m_Task;
        time_t      m_StartTime; // executing start
    };
    typedef CRef<CTaskRecord>   TRecordRef;
    typedef vector<TRecordRef>  TRecRefVec;

public:
    CAppTaskService();
    virtual ~CAppTaskService();

    /// @name IService implementation
    /// @{
    virtual void    InitService();

    /// Cancels all backgrounded tasks, removes pending tasks and blocks
    /// processing of incoming requests and events.
    virtual void    ShutDownService();

    void    PreShutDownService();

    /// @}

    /// @name IServiceLocatorConsumer interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* srv_locator);
    /// @}

    /// Add a task to the queue. The task will be executed on the main UI
    /// thread at some point in future. Tasks are executed according to FIFO
    /// principle.
    void    AddTask(IAppTask& task);

    /// Places a request to cancel a backgrounded task. It is guaranteed that
    /// Task Manager will not further execute the task on the main thread,
    /// however subcomponents of the task may still continue to run on a
    /// background thread.
    ///
    /// @param task - task to cancel
    /// @param throw_on_error - flag to throw an exception on error or
    ///                         return false
    /// @return true if taks was canceled 
    ///
    bool CancelTask(IAppTask& task, bool throw_on_error=true);

    /// This function shall be called in the the application idle function, it
    /// polls the Task Queue and executes pending tasks.
    bool    IdleCallback();

    /// Inspection interface - supposed to be used from the main UI thread only
    void    GetPendingTasks(TRecRefVec& tasks);
    void    GetBackgroundedTasks(TRecRefVec& tasks);

    /// returns the number of running and backgrounded tasks
    int     GetRunningTasksCount(bool vis_only = true);

protected:
    void    x_OnWakeUpSignal(CEvent* event);
    void    x_OnTaskCanceled(CEvent* event);
    void    x_OnStatusUpdate(CEvent* event);

    void    x_ExecuteNextTaskInQueue();

    void    x_LogTask(IEventRecord::EType type,
                      const string& prefix, IAppTask& task,
                      const string& postfix, const string& details = kEmptyStr);

    void    x_ReportTaskFailure(IAppTask& task, const string& err_msg, const string& details);

protected:

    typedef CSyncQueue<TRecordRef> TQueue;
    typedef map<TTaskRef, TRecordRef> TRecordMap;

protected:
    CMutex  m_Mutex;
    IServiceLocator* m_ServiceLocator;

    IEventLogService*   m_EventLogService;

    bool    m_ShutDown;

    TQueue  m_PendingTasks;
    TRecordMap  m_BackgoundedTasks;

    IAppTask*   m_RunningTask; // currently executed task
};


///////////////////////////////////////////////////////////////////////////////
/// CAppTaskServiceEvent
class NCBI_GUIFRAMEWORK_EXPORT CAppTaskServiceEvent:
    public CEvent
{
public:
    enum EType  {
        eInvalid = -1,
        eWakeUpSignal,
        eTaskCanceled,
        eStatusUpdate
    };

    CAppTaskServiceEvent(IAppTask& task, EType type)
        :   CEvent(eEvent_Message, type),   m_Task(&task)   {}

    CIRef<IAppTask> GetTask()     {   return m_Task;  }

protected:
    CIRef<IAppTask> m_Task;
};

///////////////////////////////////////////////////////////////////////////////
/// CAppTaskServiceException
class NCBI_GUIFRAMEWORK_EXPORT CAppTaskServiceException
            : EXCEPTION_VIRTUAL_BASE public CException
{
public:
    enum EErrCode {
        eInvalidOperation,
        eUnexpectedTask,
        eFatalError /// unknown tragic error
    };
    NCBI_EXCEPTION_DEFAULT(CAppTaskServiceException, CException);
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___APP_TASK_SERVICE__HPP
