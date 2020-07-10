/*  $Id: app_job_task.cpp 40276 2018-01-19 17:40:36Z katargir $
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

#include <gui/framework/app_job_task.hpp>
#include <gui/framework/app_task_service.hpp>

#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/event_translator.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/msgdlg.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAppJobTask

BEGIN_EVENT_MAP(CAppJobTask, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CAppJobTask::OnAppJobNotification)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eProgress,
             &CAppJobTask::OnAppJobProgress)
END_EVENT_MAP()

CAppJobTask::CAppJobTask(bool visible,
                         const string& descr,
                         int prog_rep_period,
                         const string& engine_name,
                         int display_delay)
:   CAppTask(descr, visible, display_delay),
    m_EngineName(engine_name),
    m_ProgressReportPeriod(prog_rep_period),
    m_ReportErrors(true),
    m_JobID(CAppJobDispatcher::eInvalidJobID),
    m_JobState(IAppJob::eInvalid)
{
}

CAppJobTask::CAppJobTask(IAppJob& job,
                         bool visible,
                         const string& descr,
                         int prog_rep_period,
                         const string& engine_name,
                         int   display_delay)
:   CAppTask(descr, visible, display_delay),
    m_EngineName(engine_name),
    m_Job(&job),
    m_ProgressReportPeriod(prog_rep_period),
    m_ReportErrors(true),
    m_JobID(CAppJobDispatcher::eInvalidJobID),
    m_JobState(IAppJob::eInvalid)
{
    if(m_Descr.empty()) {
        m_Descr = m_Job->GetDescr();
    }
}


CAppJobTask::~CAppJobTask()
{
}


void CAppJobTask::SetReportErrors(bool report)
{
    m_ReportErrors = report;
}


CAppJobTask::ETaskState CAppJobTask::Run()
{
    switch (m_State) {
    case eInitial:
        {{
            // running for the first time - launching Job
            _ASSERT(m_JobID == CAppJobDispatcher::eInvalidJobID);
            _ASSERT(m_JobState == IAppJob::eInvalid);

            if( ! m_Job)    {
                ERR_POST("CAppJobTask::Run() - m_Job is empty!");
                m_State = eFailed;
                break;
            }

            m_State = eRunning;
            m_StatusText = "Starting...";

            try {
                if (!m_Job->BeforeRun()) {
                    m_JobState = IAppJob::eFailed;
                    m_State = eFailed;
                    m_StatusText = "Failed to execute";
                    return m_State;
                }

                CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();

                m_JobID = disp.StartJob(*m_Job, m_EngineName, *this,
                                        m_ProgressReportPeriod, true);

                _ASSERT(m_JobID != CAppJobDispatcher::eInvalidJobID);

                m_JobState = IAppJob::eRunning;
                m_State = eBackgrounded;
                m_StatusText = "";
            } catch(CAppJobException& e)  {
                ERR_POST("CAppJobTask::Run() - Dispatcher failed to launch the job");
                ERR_POST(e.ReportAll());
                m_StatusText = "Failed to execute";
                m_JobState = IAppJob::eFailed;
                m_State = eFailed;
            }
        }}
        break;

    case eBackgrounded:
        // Job has been launched already - check whether its status has changed
        m_State = x_OnWakeUp();
        break;

    default:
        _ASSERT(false); // unexpected
        m_State = eInvalid;
        break;
    }
    return m_State;
}


// this function is called when the jobs sends and event and the task is
// executed again
CAppJobTask::ETaskState CAppJobTask::x_OnWakeUp()
{
    switch(m_JobState)  {
    case IAppJob::eRunning:
        return eBackgrounded;

    case IAppJob::eCompleted:
        return eCompleted;

    case IAppJob::eFailed:
        if(m_ReportErrors)  {
            x_ReportError();
        }
        return eFailed;

    case IAppJob::eCanceled:
        return eCanceled;

    default:
        return eInvalid;
    }
}


void CAppJobTask::x_ReportError()
{
    CConstIRef<IAppJobError> error = m_Job->GetError();
    if(error)   {
        m_StatusText = error->GetText();
        _ASSERT( ! m_StatusText.empty());
    } else {
        m_StatusText = "Unknown error";
    }
    NcbiErrorBox(m_StatusText, GetDescr());
}


void CAppJobTask::CancelBackgrounded()
{
    if(m_State == IAppTask::eBackgrounded)   {
        _ASSERT(m_JobID != CAppJobDispatcher::eInvalidJobID);

        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        //disp.DeleteJob(m_JobID);
        // if a job is orphaned -> shutdown is in progress
        // it will be cancelled by AppJobDispatcher shutdown
        if (m_Listener) {
            disp.CancelJob(m_JobID);
        }
    }
}


string CAppJobTask::GetStatusText() const
{
    return m_StatusText;
}


CIRef<IAppJob> CAppJobTask::GetJob()
{
    return m_Job;
}


/// handles "state changed" notification from CAppJobDispatcher
void CAppJobTask::OnAppJobNotification( CEvent* evt )
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if( notn ){
        if( m_JobID !=  notn->GetJobID() ){
            _ASSERT(false);
            ERR_POST(
                "CAppJobTask::OnAppJobNotification() - unknown or invalid Job ID " 
                << notn->GetJobID()
            );
            return;
        }

        switch( notn->GetState() ){
        case IAppJob::eCompleted: {
            m_StatusText = "Finished";

            CRef<CObject> result = m_Job->GetResult();
            CAppJobTextResult* txt_res = dynamic_cast<CAppJobTextResult*>(result.GetPointer());
            if( txt_res ){
                string text = txt_res->GetText();
                if( !text.empty() ){
                    wxMessageBox(
                        ToWxString(text), ToWxString( m_Job->GetDescr() + " is finished" ),
                        wxOK | wxICON_INFORMATION
                    );
                }
            }

            x_Finish( IAppJob::eCompleted );
            break;

        } case IAppJob::eFailed:  {
            CConstIRef<IAppJobError> error = notn->GetError();
            if( error ){
                m_StatusText = error->GetText();
                _ASSERT(!m_StatusText.empty());
            } else {
                m_StatusText = "Unknown error";
            }

            wxMessageBox(
                ToWxString(m_StatusText), ToWxString( m_Job->GetDescr() + " has failed" ),
                wxOK | wxICON_ERROR
            );
            SetReportErrors( false );

            x_Finish(IAppJob::eFailed);
            break;
        }
        case IAppJob::eCanceled:    {
            m_StatusText = "";
            x_Finish(IAppJob::eCanceled);
            break;
        }
        default:
            break;
        }
    }
}


void CAppJobTask::x_Finish(IAppJob::EJobState state)
{
    m_JobState = state;
    m_JobID = CAppJobDispatcher::eInvalidJobID;

    _ASSERT(m_Listener);

    if(m_Listener)  {
        CAppTaskServiceEvent::EType type = CAppTaskServiceEvent::eWakeUpSignal;
        if(m_JobState == IAppJob::eCanceled) {
            type = CAppTaskServiceEvent::eTaskCanceled;
        }

        CRef<CEvent> evt(new CAppTaskServiceEvent(*this, type));
        m_Listener->Post(evt);
    }
}


void CAppJobTask::OnAppJobProgress(CEvent* evt)
{
    CAppJobNotification* notn =
        dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if(notn)    {
        int job_id = notn->GetJobID();

        if(m_JobID != job_id) {
            _ASSERT(false);
            ERR_POST("CAppJobTask::OnAppJobNotification() - unknown or invalid Job ID " << job_id);
        } else {
            CConstIRef<IAppJobProgress> prg = notn->GetProgress();
            const CAppJobProgress* progress =
                dynamic_cast<const CAppJobProgress*>(prg.GetPointer());
            if(progress)  {
                m_StatusText = progress->GetText();

                //TODO
                if (m_Listener) {
                    CRef<CEvent> taskEvt(new CAppTaskServiceEvent(*this, CAppTaskServiceEvent::eStatusUpdate));
                    m_Listener->Post(taskEvt);
                }
            }
        }
    }
}

END_NCBI_SCOPE
