/*  $Id: job_ui_task.cpp 41621 2018-08-30 19:14:16Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 *
 */

#include <ncbi_pch.hpp>

#include <gui/framework/job_ui_task.hpp>
#include <gui/framework/app_task_service.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CJobUITask

BEGIN_EVENT_TABLE(CJobUITask, wxEvtHandler)
EVT_TIMER(-1, CJobUITask::OnTimer)
END_EVENT_TABLE()

CJobUITask::CJobUITask(const string& descr, CAppJobDispatcher::TJobID jobID)
: CAppTask(descr, true, 0), m_JobID(jobID), m_Timer(this), m_Canceled(false)
{
}


CJobUITask::~CJobUITask()
{
}

void CJobUITask::OnTimer(wxTimerEvent& event)
{
    if (CAppJobDispatcher::GetInstance().GetJobState(m_JobID) != IAppJob::eRunning) {
        _ASSERT(m_Listener);
        if (m_Listener) {
            CRef<CEvent> evt(new CAppTaskServiceEvent(*this, CAppTaskServiceEvent::eWakeUpSignal));
            m_Listener->Post(evt);
        }
        m_Timer.Stop();
    }
}

CJobUITask::ETaskState CJobUITask::Run()
{
    switch (m_State) {
    case eInitial:
        if (m_JobID != CAppJobDispatcher::eInvalidJobID) {
            m_State = eBackgrounded;
            m_Timer.Start(250);
        } else {
            m_State = eFailed;
        }
        break;
    case eBackgrounded:
        m_State = eCompleted;
        if (m_Canceled) {
            if (m_Listener) {
                CRef<CEvent> evt(new CAppTaskServiceEvent(*this, CAppTaskServiceEvent::eTaskCanceled));
                m_Listener->Post(evt);
            }
            else {
                m_State = eCanceled;
            }
        }
        break;
    default:
        _ASSERT(false); // unexpected
        m_State = eInvalid;
        break;
    }
    return m_State;
}

void CJobUITask::CancelBackgrounded()
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    disp.CancelJob(m_JobID);
    m_Canceled = true;
}

string CJobUITask::GetStatusText() const
{
    if (m_Canceled)
        return "Canceled";
    else if (m_State == eBackgrounded)
        return "Running";
    return "Completed";
}

END_NCBI_SCOPE
