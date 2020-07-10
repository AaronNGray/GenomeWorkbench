#ifndef GUI_OBJUTILS___APP_JOB__HPP
#define GUI_OBJUTILS___APP_JOB__HPP

/*  $Id: app_job.hpp 34478 2016-01-13 16:19:10Z katargir $
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

#include <gui/gui_export.h>

#include <corelib/ncbiobj.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// IAppJobProgress
class NCBI_GUIOBJUTILS_EXPORT IAppJobProgress
{
public:
    virtual ~IAppJobProgress() {};

    /// return a number indicating the progress(0.0 - just started, 1.0 - done)
    virtual float   GetNormDone() const = 0;

    /// returns a text string describing current state
    virtual string  GetText() const = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// IAppJobError
class NCBI_GUIOBJUTILS_EXPORT IAppJobError
{
public:
    virtual ~IAppJobError() {};

    /// returns a string describing the error
    virtual string GetText() const = 0;

    virtual bool IsUserLevel() const = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// IAppJob
///
/// Base interface class for background job execution
/// 
class NCBI_GUIOBJUTILS_EXPORT IAppJob
{
public:
    /// Job states (describe FSM)
    ///
    enum    EJobState    {
        eInvalid,
        eRunning,
        eCompleted,
        eFailed,
        eCanceled,
        eSuspended
    };

    virtual ~IAppJob()  {};

    /// Function will be called on the main thread
    /// before execution of Run
    /// Should not do any lengthy work
    /// returns true to continue job execution on worker thread
    ///         false to quit execution with status - failed
    virtual bool         BeforeRun() { return true; }

    /// Function that does all the useful work, called by the Engine.
    /// Run() shall be implemented so that it will allow for exiting once
    /// RequestCancel() is called.
    virtual EJobState    Run() = 0;

    /// return progress object, the function shall be synchronized internally.
    /// the returned object should not be modified by the job (job shall create
    /// a new instance of Progress object on every GetProgress() invocation).
    virtual CConstIRef<IAppJobProgress> GetProgress() = 0;

    /// Returns the Job Result. Derive your own object from CObject and return
    /// it from your Job class. Result shall be returned only if Job finished
    /// succesfully.
    virtual CRef<CObject>   GetResult() = 0;

    /// Returns IAppJobError object describing internal error that caused the Job
    /// to fail. Result shall be returned only if the Job did fail.
    virtual CConstIRef<IAppJobError>    GetError() = 0;

    /// Returns a human readable description of the Job (optional)
    virtual string    GetDescr() const = 0;

    /// RequestCancel() is called to notify the Job that it shall
    /// exit Run() function ASAP. The function shall be synchronized internally.
    virtual void RequestCancel() = 0;

    // Return true if job was canceled
    virtual bool IsCanceled() const = 0;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___APP_JOB__HPP
