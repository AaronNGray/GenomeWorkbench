#ifndef GUI_FRAMEWORK___JOB_UI_TASK__HPP
#define GUI_FRAMEWORK___JOB_UI_TASK__HPP

/*  $Id: job_ui_task.hpp 36528 2016-10-04 17:41:48Z katargir $
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
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/app_task_impl.hpp>

#include <gui/utils/app_job_dispatcher.hpp>

#include <wx/event.h>
#include <wx/timer.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIFRAMEWORK_EXPORT CJobUITask :
    public CAppTask,
    public wxEvtHandler
{
    DECLARE_EVENT_TABLE()
public:
    CJobUITask(const string& descr,
               CAppJobDispatcher::TJobID jobID);

    virtual ~CJobUITask();

    /// enabled / disbale error reporting for failed Jobs
    void    SetReportErrors(bool report);

    /// @name IAppTask interface implementation
    /// @{
    virtual ETaskState    Run();
    virtual void    CancelBackgrounded();
    virtual string  GetStatusText() const;
    /// @}

    void OnTimer(wxTimerEvent& event);

protected:
    CAppJobDispatcher::TJobID m_JobID;
    wxTimer m_Timer;
    bool m_Canceled;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___JOB_UI_TASK__HPP
