/*  $Id: app_task_impl.cpp 40283 2018-01-19 18:08:34Z katargir $
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

#include <gui/framework/app_task_impl.hpp>
#include <gui/framework/app_task_service.hpp>

#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/event_translator.hpp>


BEGIN_NCBI_SCOPE

#define TASK_CATCH_AND_REPORT(message, task) \
    catch (CException& e) {   \
        ERR_POST(message << task.GetDescr());   \
        ERR_POST(e.ReportAll());      \
    } catch (std::exception& ee) {   \
        ERR_POST(message  << task.GetDescr() <<  ee.what());    \
    } 

///////////////////////////////////////////////////////////////////////////////
/// CAppTask
CAppTask::CAppTask(const string& descr, bool visible, int display_delay)
:   m_Listener(NULL),
    m_State(eInitial),
    m_Visible(visible),
    m_Descr(descr),
    m_DisplayDelay(display_delay)
{
}


static const char* kChildFailed =
    "CAppTask - unexpected exception while executing a child task.";


CAppTask::ETaskState CAppTask::Run()
{
    //LOG_POST("CAppTask::Run() - BEGIN");

    // init Time
    _ASSERT(m_State == eInitial);
    m_State = eRunning;
    // TODO init start time

    ETaskState new_state = eFailed; // we are pessimistic
    try {
        new_state = x_Run();
    }
    TASK_CATCH_AND_REPORT(kChildFailed, (*this));

    m_State = new_state;
    //LOG_POST("CAppTask::Run() - END");
    return m_State;
}


CAppTask::ETaskState CAppTask::x_Run()
{
    _ASSERT(false); // implement in derived classes or override Run()
    return eCompleted;
}


bool CAppTask::x_IsFiniteState()
{
    return m_State == eCompleted  ||  m_State == eFailed  ||  m_State == eCanceled;
}


CAppTask::ETaskState CAppTask::GetState()
{
    return m_State;
}


void CAppTask::CancelBackgrounded()
{
    // not needed for task that do not support background execution
}

void CAppTask::OnCancel()
{
    // override to implement cleanup procedures
}

bool CAppTask::IsVisible()
{
    return m_Visible;
}

int CAppTask::GetStatusDisplayDelay()
{
    return m_DisplayDelay;
}

string CAppTask::GetDescr() const
{
    return m_Descr;
}


string CAppTask::GetStatusText() const
{
    // trivial implementation - simply return the state
    // override this function to return more detailed description
    return StateToString(m_State);
}


void CAppTask::SetListener(CEventHandler* listener)
{
    m_Listener = listener;
}


IEventLogAction* CAppTask::CreateEventLogAction()
{
    return NULL;
}


string CAppTask::StateToString(ETaskState state)
{
    static string sInvalid = "Invalid";
    static string sInitial = "Initial";
    static string sBackgrounded = "Running";
    static string sCompleted = "Completed";
    static string sFailed = "Failed";
    static string sCanceled = "Canceled";

    switch(state) {
    case eInvalid:
        return sInvalid;
    case eInitial:
        return sInitial;
    case eBackgrounded:
        return sBackgrounded;
    case eCompleted:
        return sCompleted;
    case eFailed:
        return sFailed;
    case eCanceled:
        return sCanceled;
    default:
        return "Unknown State";
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CCompositeAppTask

BEGIN_EVENT_MAP(CCompositeAppTask, CEventHandler)
    ON_EVENT_RANGE(CAppTaskServiceEvent,
                   CAppTaskServiceEvent::eWakeUpSignal,
                   CAppTaskServiceEvent::eTaskCanceled,
                   &CCompositeAppTask::OnTaskEvent)
END_EVENT_MAP()


CCompositeAppTask::CCompositeAppTask(const string& descr, bool visible)
:   CAppTask(descr, visible)
{
}


CCompositeAppTask::ETaskState CCompositeAppTask::Run()
{
    if(m_State == eInitial)   {
        /// this is the first call to Run() - we need to start the task
        m_State = eRunning;
    }
    if(m_State == eRunning  ||  m_State == eBackgrounded)   {
        /// execute child tasks until they all completed or
        /// until one of them backgrounds or fails
        bool stop = false;
        while( ! stop  &&  x_GetCurrentSubTask())    {
            // set pessimistic defaults
            ETaskState state = IAppTask::eFailed;
            stop = true;

            try {
                TTaskRef r_task = x_GetCurrentSubTask();
                r_task->SetListener(this);  // we will be listening on child tasks

                state = x_RunCurrentSubTask();

                switch(state)   {
                case eCompleted:
                    r_task->SetListener(NULL);
                    x_NextSubTask();
                    stop = false; // continue execution
                    break;
                case eBackgrounded:
                    // stop execution until background process finishes
                    m_State = eBackgrounded;
                    break;
                case eFailed:
                    r_task->SetListener(NULL);
                    m_State = eFailed;
                    break;
                default:
                    _ASSERT(false);
                    r_task->SetListener(NULL);
                    m_State = eFailed;
                    break;
                }
            }
            TASK_CATCH_AND_REPORT(kChildFailed, x_GetCurrentSubTask().GetObject());
        }
        if( ! stop) {
            m_State = eCompleted;
        }
    } else {
        _ASSERT(false); // unexpected state
        m_State = eFailed;
    }
    //LOG_POST("CCompositeAppTask::Run() - finished");
    return m_State;
}


CCompositeAppTask::ETaskState CCompositeAppTask::x_RunCurrentSubTask()
{
    return x_GetCurrentSubTask()->Run();
}


void CCompositeAppTask::CancelBackgrounded()
{
    if(m_State == eBackgrounded)    {
        CIRef<IAppTask> task = x_GetCurrentSubTask();
        
        // listener is off - task is not needed
        if (!m_Listener) {
            task->SetListener(NULL);
        }
        
        task->CancelBackgrounded();
    }
}


void CCompositeAppTask::OnTaskEvent(CEvent* event)
{
    CAppTaskServiceEvent* tm_evt = dynamic_cast<CAppTaskServiceEvent*>(event);
    _ASSERT(tm_evt);

    if(tm_evt)  {
        CIRef<IAppTask> task = tm_evt->GetTask();

        _ASSERT(m_State == eBackgrounded);
        _ASSERT(task == x_GetCurrentSubTask());
        _ASSERT(m_Listener);

        CAppTaskServiceEvent::EType type = (CAppTaskServiceEvent::EType) event->GetID();
        CRef<CEvent> evt(new CAppTaskServiceEvent(*this, type));
        m_Listener->Post(evt);
    }
}


END_NCBI_SCOPE
