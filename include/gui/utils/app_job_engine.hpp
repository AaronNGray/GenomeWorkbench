#ifndef GUI_OBJUTILS___APP_JOB_ENGINE__HPP
#define GUI_OBJUTILS___APP_JOB_ENGINE__HPP

/*  $Id: app_job_engine.hpp 24876 2011-12-16 16:03:41Z kuznets $
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

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <gui/utils/app_job.hpp>

#include <gui/utils/app_job_dispatcher.hpp>

BEGIN_NCBI_SCOPE


class IEngineParams;

///////////////////////////////////////////////////////////////////////////////
/// IAppJobEngine
///
/// Base interface for building engines for background job execution
///
/// @sa IAppJob
class NCBI_GUIUTILS_EXPORT IAppJobEngine
{
public:
    typedef CAppJobDispatcher   TDisp;
    typedef IAppJob::EJobState  TJobState;

    virtual ~IAppJobEngine() {};

    /// Returns true if Engine supports "active" model i.e. will send
    /// notifications to Dispatcher when a Job running on this Engine changes
    /// its state.
    virtual bool    IsActive() = 0;

    /// For "active" mode set a Listener that will be notified when
    /// the state of a Job changes.
    virtual void    SetListener(IAppJobEngineListener* listener) = 0;

    /// If Engine cannot start the Job and exception shall be thrown.
    virtual void    StartJob(IAppJob& job, IEngineParams* params = NULL) = 0;

    /// Cancel job in the engine 
    /// If job is not running yet - just remove from the pending queue
    /// otherwise uses IInterruptible to notify the job about cancel request 
    virtual void    CancelJob(IAppJob& job) = 0;
    virtual void    SuspendJob(IAppJob& job) = 0;
    virtual void    ResumeJob(IAppJob& job) = 0;

    /// @return current job state 
    virtual TJobState GetJobState(IAppJob& job) const = 0;

    /// stop any background threads and free resources associated with the Engine
    virtual void    ShutDown() = 0;

    /// asyncronous request all jobs to Cancel
    virtual void RequestCancel() = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// IAppJobEngineParams
/// A base class for Engine Parmeters classes. If Engine takes additional
/// parameters that affect Job execution, then a special class need to be
/// derived from IAppJobEngineParams. The class shall provide functions for
/// setting and getting Engine-specific parameters, an instance of this class
/// could be passed to StartJob() function together with a Job object.
class NCBI_GUIOBJUTILS_EXPORT IAppJobEngineParams
{
public:
    virtual ~IAppJobEngineParams()   {}
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___APP_JOB_ENGINE__HPP
