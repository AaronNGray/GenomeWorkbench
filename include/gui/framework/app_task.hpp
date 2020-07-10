#ifndef GUI_FRAMEWORK___APP_TASK__HPP
#define GUI_FRAMEWORK___APP_TASK__HPP

/*  $Id: app_task.hpp 40034 2017-12-12 20:12:38Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

class CEventHandler;
class IEventLogAction;

///////////////////////////////////////////////////////////////////////////////
/// IAppTask
///
/// IAppTask represents an Application Task that can be executed in Application
/// Task Manager. Application Tasks correspond to application-level commands or
/// operations, it is recommended that various application functions are
/// packaged as tasks. The functionality of an application can be easily
/// extended by declaring new types of Tasks.
///
/// Tasks can consist of other tasks, the parent task can control
/// execution of the child tasks. Such composite tasks can be used to implement
/// complex “macro” operations or scripts – scenarios that combine atomic
/// tasks.
///
/// Task Manager executes tasks on the main UI thread. Execution on the main UI
/// thread guarantees that Tasks do not overlap in time what reduces risks of
/// concurrency problems. It also ensures ability of a Task to interact with UI
/// (show dialogs, communicate with views) which is only safe from the main UI
/// thread.
///
/// Tasks can involve computations on background threads, in that case a task
/// shall launch a background job and wait until it finishes or exits with
/// “Backgrounded” status. Task Manager places backgrounded tasks aside where
/// they can wait completion of the associated jobs running asynchronously.
/// When background part of execution is completed a Task shall notify Task
/// Manager, then Task Manager will schedule the Task for execution on the main
/// UI thread.  A Task can go to backgrounded state more than once.
///
/// A Task must be thread-safe relative to its IAppTask interface, i.e.  any
/// calls to IAppTask from the main UI thread should be safe even if not
/// synchronized externaly.
///
class NCBI_GUIFRAMEWORK_EXPORT IAppTask
{
public:
    /// List of task states defining the task management FSM
    ///
    enum    ETaskState {
        eInvalid = -1,
        eInitial,      ///< has not been executed yet
        eRunning,      ///< task is executing on the main UI thread
        eBackgrounded, ///< task is executing in background
        eCompleted,    ///< successfully finished
        eFailed,       ///< failed during execution
        eCanceled,     ///< canceled by Task Manager
        eLastState
    };

    virtual ~IAppTask()  {};

    /// set a Listener; this allows a taks running in background to notify
    /// the Listener (usually Task Manager) about changes in Status
    virtual void    SetListener(CEventHandler* handler) = 0;

    /// execute the task, this function is called on the main UI thread if a
    /// task needs to execute in background it should launch a job that will
    /// execute asynchronously; then the function shall return eBackgrounded
    virtual ETaskState  Run() = 0;

    /// returns the current task state
    virtual ETaskState  GetState() = 0;

    /// make a request to cancel backgrounded task (not called for foreground
    /// tasks)
    virtual void    CancelBackgrounded() = 0;

    /// this will be called on pending tasks that will be removed
    /// from the queue and won't get a chance to run
    /// the purpose is to implement any cleanup procedures
    /// for the canceled task
    virtual void    OnCancel() = 0;

    /// returns true if the task should be visible in UI
    /// visible task shall represent operations understood by users and
    /// usually correspond to menu commands.
    virtual bool    IsVisible() = 0;

    /// returns delay in seconds used for certain fast (and not very important tasks) to 
    /// only show its status when it is running/pending for long enough
    virtual int GetStatusDisplayDelay() = 0;

    /// returns a human-readable description of the Task (optional)
    virtual string  GetDescr() const = 0;

    /// returns human-readable text describing the current task state
    virtual string  GetStatusText() const = 0;

    /// returns an action for activating task results (optional, can return
    /// NULL)
    virtual IEventLogAction*    CreateEventLogAction() = 0;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___APP_TASK__HPP
