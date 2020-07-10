#ifndef GUI_OBJUTILS___APP_JOB_IMPL__HPP
#define GUI_OBJUTILS___APP_JOB_IMPL__HPP

/*  $Id: app_job_impl.hpp 34471 2016-01-12 21:51:15Z katargir $
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
#include <gui/utils/locks.hpp>

#include <corelib/ncbiobj.hpp>

#include <gui/utils/canceled_impl.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/utils/locker.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAppJobProgress
/// Default implementation for IAppJobProgress - the class encapsulates a text
/// message and a numeric value indicating the progress of the task normalized
/// in [0.0, 1.0] range (0.0 - just started, 1.0 - 100% done).
///
/// MT safety: class is sync-ed for concurrent access from multiple thereads.
///
class NCBI_GUIUTILS_EXPORT CAppJobProgress :
    public CObject,
    public IAppJobProgress
{
public:
    CAppJobProgress();
    CAppJobProgress(float done, const string& text);

    CAppJobProgress(const CAppJobProgress& progress);
    CAppJobProgress& operator=(const CAppJobProgress& progress);

    void    SetNormDone(float done);
    void    SetText(const string& text);

    /// @name IAppJobProgress implementation
    /// @{
    virtual float   GetNormDone() const;
    virtual string  GetText() const;
    /// @}

private:
    /// copy constructor, assignment implementation
    void x_CopyFrom(const CAppJobProgress& progress);

protected:
    float               m_Done;
    string              m_Text;
    mutable CGuiRWLock  m_Lock;  ///< MT sync.lock
};


///////////////////////////////////////////////////////////////////////////////
/// CAppJobError
/// Default implementation for IAppJobError - encapsulates a text error message.

class NCBI_GUIUTILS_EXPORT CAppJobError :
    public CObject,
    public IAppJobError
{
public:
    CAppJobError( const string& text, bool user_level = false );

    /// @name IAppJobError implementation
    /// @{
    virtual string GetText() const;

    virtual bool IsUserLevel() const;
    /// @}
protected:
    string m_Text;
    bool m_UserLevel;
};


///////////////////////////////////////////////////////////////////////////////
/// CAppJobTextResult
/// Default implementation for result object - encapsulates a text error message.

class NCBI_GUIUTILS_EXPORT CAppJobTextResult :
    public CObject
{
public:
    CAppJobTextResult(const string& text);

    virtual string    GetText() const;

protected:
    string m_Text;
};

///////////////////////////////////////////////////////////////////////////////
/// Base class to build jobs with cancel functionality
///

class CJobCancelable : public CObject, public IAppJob
{
    class CCanceled : public CObject, public CCanceledImpl
    {
    };

public:
    CJobCancelable() : m_Canceled(new CCanceled) {}

    void SetDataLocker(ILocker* locker) { m_DataLocker.reset(locker); }

    /// @name IAppJob implementation
    /// @{
    virtual void RequestCancel() { return m_Canceled->RequestCancel(); }
    virtual bool IsCanceled() const { return m_Canceled->IsCanceled(); }
    /// @}

protected:
    CLockerGuard x_GetGuard() { return CLockerGuard(m_DataLocker.get(), m_Canceled); }

    ICanceled*  x_GetICanceled() { return m_Canceled; }

private:
    CRef<CCanceled> m_Canceled;

    auto_ptr<ILocker> m_DataLocker;
};

///////////////////////////////////////////////////////////////////////////////
/// CAppJob - default implementation of IAppJob that could be used as a base
/// class.
class NCBI_GUIOBJUTILS_EXPORT CAppJob : public CJobCancelable
{
public:
    CAppJob(const string& descr = "AppJob");

    /// @name IAppJob implementation
    /// @{

    /// implement Run() in derived classes, make sure that exceptions are
    /// handled and m_StopRequested flag is respected.
    virtual EJobState   Run() = 0;

    virtual CConstIRef<IAppJobProgress> GetProgress() ;
    virtual CRef<CObject>   GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string    GetDescr() const;
    /// @}


protected:
    virtual void    x_SetStatusText(const string& text);
    virtual void    x_ResetState();

protected:
    mutable CFastMutex m_Mutex; /// mutex to sync our internals

    /// UI-friendly Job descriptor
    string  m_Descr;

    /// Current status of the Job
    string  m_Status;

    CRef<CAppJobError>  m_Error;
};

class NCBI_GUIOBJUTILS_EXPORT CAppJobExecuteUnit : public CAppJob
{
public:
    CAppJobExecuteUnit(IExecuteUnit& executeUnit, const string& descr)
        : CAppJob(descr), m_ExecuteUnit(&executeUnit) {}

    virtual EJobState Run()
    {
        return m_ExecuteUnit->Execute(*x_GetICanceled()) ? eCompleted : eFailed;
    }

private:
    CIRef<IExecuteUnit> m_ExecuteUnit;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___APP_JOB_IMPL__HPP
