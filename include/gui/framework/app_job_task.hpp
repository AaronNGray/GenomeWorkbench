#ifndef GUI_FRAMEWORK___APP_JOB_TASK__HPP
#define GUI_FRAMEWORK___APP_JOB_TASK__HPP

/*  $Id: app_job_task.hpp 26758 2012-11-02 16:27:30Z katargir $
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
 *      CAppJobTask makes it possible to execute App Jobs as Tasks in Task manager.
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/app_task_impl.hpp>

#include <gui/utils/app_job_dispatcher.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAppJobTask
/// CAppJobTask is an adapter that allows for running IAppJobs as Tasks in
/// App Task Service.
///
/// CAppJobTask is a wrapper around IAppJob that implements IAppTask interface
/// and so can be used with CAppTaskService. When launched by Task Service
/// CAppJobTask starts the given Job and then returns with eBackgrounded
/// status. When the job finishes CAppJobTask notifies Task Service so that it
/// could resume executing the Task.

/// This class is NOT thread-safe and is designed to be used
/// on the main UI thread only (what is consistent with Task Manager design).

class NCBI_GUIFRAMEWORK_EXPORT CAppJobTask :
    public CAppTask,
    public CEventHandler
{
    DECLARE_EVENT_MAP();
public:
    CAppJobTask(bool visible,
                const string& descr = kEmptyStr,
                int prog_rep_period = 5,
                const string& engine_name = "ThreadPool",
                int   display_delay = 0);

    CAppJobTask(IAppJob& job,
                bool visible,
                const string& descr = kEmptyStr,
                int prog_rep_period = 5,
                const string& engine_name = "ThreadPool",
                int   display_delay = 0);
    virtual ~CAppJobTask();

    /// enabled / disbale error reporting for failed Jobs
    void    SetReportErrors(bool report);

    /// @name IAppTask interface implementation
    /// @{
    virtual ETaskState    Run();
    virtual void    CancelBackgrounded();
    virtual string  GetStatusText() const;
    /// @}

    CIRef<IAppJob>   GetJob();

    /// functions below handle notifications from the IAppJob
    void    OnAppJobNotification(CEvent* event);
    void    OnAppJobProgress(CEvent* event);

protected:
    /// "wakes up" a backgrounded job to resume processing
    virtual ETaskState  x_OnWakeUp();

    virtual void    x_Finish(IAppJob::EJobState state);

    // reports the Job error; override this function if you want to implement
    // it differently
    virtual void    x_ReportError();

protected:
    typedef CAppJobDispatcher::TJobID   TJobID;

    string      m_EngineName;

    // Job execution state
    CIRef<IAppJob>  m_Job;
    int     m_ProgressReportPeriod;
    bool    m_ReportErrors;

    TJobID  m_JobID; // ID of the Job in App Job Dispatcher
    IAppJob::EJobState  m_JobState; // the state of the Job is different from the state of the Task
    string  m_StatusText;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___APP_JOB_TASK__HPP
