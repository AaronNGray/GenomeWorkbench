#ifndef GUI_FRAMEWORK___COMPOSITE_APP_TASK__HPP
#define GUI_FRAMEWORK___COMPOSITE_APP_TASK__HPP

/*  $Id: app_task_impl.hpp 24498 2011-10-04 20:42:44Z kuznets $
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

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/app_task.hpp>

#include <gui/utils/app_job.hpp>
#include <gui/utils/event_handler.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAppTask - default implementation of IAppTask, use it as a base class for
/// custom tasks.

class NCBI_GUIFRAMEWORK_EXPORT CAppTask :
    public CObjectEx,
    public IAppTask
{
public:
    CAppTask(const string& descr = kEmptyStr, bool visible = true, int display_delay = 0);

    /// @name IAppTask interface implementation
    /// @{
    virtual ETaskState    Run();
    virtual ETaskState    GetState();
    virtual void    CancelBackgrounded();
    virtual void    OnCancel();
    virtual bool    IsVisible();
    int GetStatusDisplayDelay();
    virtual string  GetDescr() const;
    virtual string  GetStatusText() const;
    virtual void    SetListener(CEventHandler* handler);
    virtual IEventLogAction*    CreateEventLogAction();
    /// @}

    static string   StateToString(ETaskState state);

protected:
    /// override this function in derived classes
    virtual ETaskState    x_Run();

    virtual bool    x_IsFiniteState();

protected:
    CEventHandler* m_Listener;

    ETaskState  m_State;
    const bool  m_Visible;  ///< indicates whether the task is visible
    string  m_Descr;  ///< task description
    int     m_DisplayDelay; ///< tasks display delay
};


///////////////////////////////////////////////////////////////////////////////
/// CCompositeAppTask - an application task that consists of subtasks.
///
/// CCompositeAppTask executes the subtask one by one. If a subtask goes into
/// background (pauses), the composite task also goes into background.
/// If a subtask fails the composite task will also terminate with eFailed status.

class NCBI_GUIFRAMEWORK_EXPORT CCompositeAppTask :
    public CAppTask,
    public CEventHandler
{
    DECLARE_EVENT_MAP();
public:
    typedef CIRef<IAppTask> TTaskRef;

    CCompositeAppTask(const string& descr = kEmptyStr, bool visible = true);

    /// @name IAppTask interface implementation
    /// @{
    virtual ETaskState    Run();
    virtual void    CancelBackgrounded();
    /// @}

    void    OnTaskEvent(CEvent* event);

protected:
    /// returns a references to the current subtask
    virtual TTaskRef    x_GetCurrentSubTask() = 0;

    /// prepares the next subtask (makes it current), this function is used for
    /// iterating subtasks.
    virtual TTaskRef    x_NextSubTask() = 0;

    /// runs the current subtask; one may override this function in order to
    /// perform special pre-execution or post-execution actions
    virtual ETaskState    x_RunCurrentSubTask();

protected:

};


END_NCBI_SCOPE

#endif  // GUI_FRAMEWORK___COMPOSITE_APP_TASK__HPP
