#ifndef GUI_CORE___PROJECT_LOADING_TASK__HPP
#define GUI_CORE___PROJECT_LOADING_TASK__HPP

/*  $Id: project_loading_task.hpp 39528 2017-10-05 15:27:37Z katargir $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/app_task_impl.hpp>
#include <gui/framework/app_job_task.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <gui/utils/app_job_impl.hpp>


BEGIN_NCBI_SCOPE

class CProjectService;

class  NCBI_GUICORE_EXPORT  CProjectLoadingTask :
    public CAppTask,
    public CEventHandler
{
    DECLARE_EVENT_MAP();
public:
    /// static data structures for File dialogs
    static SWFileDlgData sm_ProjectDlgData;

    CProjectLoadingTask(int projId, CProjectService* srv, bool add_to_mru);
    virtual ~CProjectLoadingTask();

    /// @name IAppTask interface implementation
    /// @{
    virtual ETaskState    Run();
    virtual void    CancelBackgrounded();
    /// @}

    /// functions below handle notifications from the IAppJob
    void    OnAppJobNotification(CEvent* event);
    void    OnAppJobProgress(CEvent* event);
protected:
    typedef CAppJobDispatcher::TJobID TJobID;

    ETaskState          x_StartJob();
    ETaskState          x_OnWakeUp();

    string              m_TaskName;
    int                 m_ProjId;
    CProjectService*    m_ProjectService;
    bool                m_AddToMRU;
    TJobID              m_JobID;
    IAppJob::EJobState  m_JobState;
};

END_NCBI_SCOPE


#endif  // GUI_CORE___PROJECT_LOADING_TASK__HPP

