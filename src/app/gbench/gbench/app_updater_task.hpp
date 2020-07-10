#ifndef GUI_GBENCH_NEW___APP_UPDATER_TASK__HPP
#define GUI_GBENCH_NEW___APP_UPDATER_TASK__HPP

/*  $Id: app_updater_task.hpp 34801 2016-02-12 13:40:56Z evgeniev $
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
* Authors:  Vladislav Evgeniev
*
* File Description:
*
*/


#include <corelib/ncbiobj.hpp>

#include <gui/framework/app_task_impl.hpp>
#include <gui/utils/app_job_dispatcher.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAppUpdaterTask - a task to check if new version is available and to download and install it

class CAppUpdaterTask : public CAppTask, public CEventHandler
{
    DECLARE_EVENT_MAP();
public:
    CAppUpdaterTask(bool silentMode = true);
    ~CAppUpdaterTask();

    /// @name IAppTask interface implementation
    /// @{
    virtual ETaskState    Run();
    virtual string  GetStatusText() const;
    /// @}

    /// functions below handle notifications from the IAppJob
    void    OnAppJobNotification(CEvent* event);
    void    OnAppJobProgress(CEvent* event);

protected:
    void       x_DeleteJob();
    ETaskState x_OnWakeUp();
    void       x_Finish(IAppJob::EJobState state);
    void       x_UpdateNotification();
    void       x_ShellExecuteUpdate();

    typedef CAppJobDispatcher::TJobID   TJobID;

    CIRef<IAppJob>     m_Job;
    TJobID             m_JobID;
    IAppJob::EJobState m_JobState;

    enum EJobType {
        eCheckForUpdates,
        eDownloadUpdate
    };

    float           m_Progress;
    string          m_StatusText;
    bool            m_NewVersionAvailable;
    bool            m_SilentMode;
    string          m_Package;
    CRef<CObject>   m_Result;
    EJobType        m_JobType;
};

END_NCBI_SCOPE;

#endif  // GUI_GBENCH_NEW___APP_UPDATER_TASK__HPP
