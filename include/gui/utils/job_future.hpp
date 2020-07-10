#ifndef GUI_OBJUTILS___JOB_FUTURE__HPP
#define GUI_OBJUTILS___JOB_FUTURE__HPP

/*  $Id: job_future.hpp 44085 2019-10-23 16:29:21Z katargir $
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

///
/// Here is set of classes to implement launching jobs
/// (to execute code on a worker thread using C++ Toolkit thread pool implementation)
/// This code mimics (not exactly) STL async/future concept (delayed function call).
/// There ar two entities - template class job_future<T>, T is returned type.
/// and job_async template function which creates job_future<> instance.
/// job_async accepts as a parameter a function object created by std::bind() template function or 
/// lambda function with parameters [](ICanceled& canceled).
/// The returned object must have a call signature T f() or T f(ICanceled&).
/// Then the returned job_future<> may be polled periodically for completion.
/// Recommended way is to use either wxTimer "void OnTimer(wxTimerEvent& event)"
/// or OnIdle "void OnIdle(wxIdleEvent& event)".
/// After completetion job_future<> object must be "called" via call operator.
/// The call will either return a value (may be void) or rethrow an uncaught exception.
/// Only std::exception derived exception will be rethrown.
/// An Example:
///
///  class CSomeGUIClass
///  {
///      void LaunchJob();
///      void OnIdle(wxIdleEvent& event)
///  private:
///      job_future<vector<string> > m_Future;
///  };
///
///  void CSomeGUIClass::LaunchJob()
///  {
///      TConstScopedObjects objects;
///      m_Future = job_async([objects](ICanceled& canceled)
///      {
///         vector<string> result;
///         if (canceled.IsCanceled())
///             return result;
///         ... // implementation
///         return result;
///      }
///  }
///
///  void CSomeGUIClass::OnIdle(wxIdleEvent& event)
///  {
///      if (m_Future.IsComplete()) {
///          try {
///              vector<string>& res = m_Future();
///              ... // do someting with res;
///          } catch (const std::exception& e) {
///              ... // report error
///          }
///          m_Future.reset(); // not neccessary but recommended to release resources
///       }
///       else
///           event.Skip();
///  }
///
/// There is a member job_future<>::cancel() which causes ICanceled::IsCanceled() to return true to interrupt a job.
/// On job completion a programmer can check if job_future<>::cancel() was called by "bool job_future<>::IsCanceled()"
///


#include <gui/gui_export.h>
#include <corelib/ncbiobj.hpp>

#include <gui/utils/event_handler.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/app_job_impl.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIUTILS_EXPORT CJobFutureAdapter : public CObjectEx, public CEventHandler
{
    DECLARE_EVENT_MAP();
public:
    class IJobCallback
    {
    public:
        virtual ~IJobCallback() {}
        virtual void OnJobResult(CObject* result) = 0;
        virtual void OnJobFailed(const IAppJobError* error) = 0;
        virtual void OnJobCanceled() = 0;
    };

    CJobFutureAdapter(IJobCallback& callback);
    ~CJobFutureAdapter() { Detach(); }

    void Start(IAppJob& job);

    void Cancel();
    void Detach();

    void SetCallback(IJobCallback* callback) { m_Callback = callback; }

    bool IsRunning() const;

private:
    void x_OnJobNotification(CEvent* evt);
    void x_CancelJob();

    IJobCallback*             m_Callback;
    CAppJobDispatcher::TJobID m_JobId;
};

class NCBI_GUIUTILS_EXPORT CJobFutureError :
    public CObject,
    public IAppJobError
{
public:
    CJobFutureError(const std::exception_ptr& ex) : m_Exception(ex) {}

    void Rethrow() const
    {
        std::rethrow_exception(m_Exception);
    }

    /// @name IAppJobError implementation
    /// @{
    virtual string GetText() const { return ""; };
    virtual bool IsUserLevel() const { return false; }
    /// @}
private:
    std::exception_ptr m_Exception;
};

class NCBI_GUIUTILS_EXPORT job_future_base : public CJobFutureAdapter::IJobCallback
{
public:
    job_future_base() : m_State(eUndefined) {}
    job_future_base(IAppJob& job);

    job_future_base(job_future_base const&) = delete;
    job_future_base& operator=(job_future_base const&) = delete;

    job_future_base(job_future_base&& rhs)
    {
        m_State = rhs.m_State;
        m_Canceled = rhs.m_Canceled;
        m_Result = std::move(rhs.m_Result);
        m_Error = std::move(rhs.m_Error);
        m_JobAdapter = std::move(rhs.m_JobAdapter);

        if (m_JobAdapter)
            m_JobAdapter->SetCallback(this);

        rhs.m_State = eUndefined;
        rhs.m_Canceled = false;
    }

    job_future_base& operator=(job_future_base&& rhs)
    {
        if (this == &rhs)
            return *this;

        reset();

        m_State = rhs.m_State;
        m_Canceled = rhs.m_Canceled;
        m_Result = std::move(rhs.m_Result);
        m_Error = std::move(rhs.m_Error);
        m_JobAdapter = std::move(rhs.m_JobAdapter);

        if (m_JobAdapter)
            m_JobAdapter->SetCallback(this);

        rhs.m_State = eUndefined;
        rhs.m_Canceled = false;

        return *this;
    }

    virtual ~job_future_base() { reset(); }

    bool IsRunning() const { return m_JobAdapter ? m_JobAdapter->IsRunning() : false; }
    bool IsComplete() const { return m_State == eComplete; }
    bool IsCanceled() const { return m_Canceled; }

    void reset();

    void cancel()
    {
        if (m_JobAdapter)
            m_JobAdapter->Cancel();
    }

    /// @name CJobFutureAdapter::IJobCallback interface implementation
    /// @{
    virtual void OnJobResult(CObject* result);
    virtual void OnJobFailed(const IAppJobError* error);
    virtual void OnJobCanceled();
    /// @}

protected:
    void handle_error();

    enum EState
    {
        eRunning,
        eComplete,
        eUndefined
    };

    EState m_State;
    bool m_Canceled = false;
    CRef<CJobFutureAdapter> m_JobAdapter;
    CRef<CObject> m_Result;
    CConstRef<CJobFutureError> m_Error;

    static const char* m_BadJobResponse;
};

template<class _Rty>
class job_future : public job_future_base
{
public:
    job_future() {}
    job_future(IAppJob& job) : job_future_base(job) {}

    job_future(job_future&& rhs) : job_future_base(std::move(rhs)) {}

    job_future& operator=(job_future&& rhs)
    {
        job_future_base::operator=(std::move(rhs));
        return *this;
    }

    _Rty& operator()();
};

class CJobFutureResult : public CObject
{
public:
    CJobFutureResult() : m_Canceled(false) {}

    void SetCanceled() { m_Canceled = true; }
    bool GetCanceled() const { return m_Canceled; }

private:
    bool m_Canceled;
};

template<typename T> class CJobFutureResultRet : public CJobFutureResult
{
public:
    T& SetData() { return m_Data; }

private:
    T m_Data;
};

template<class _Rty>
_Rty& job_future<_Rty>::operator()()
{
    handle_error();
    CJobFutureResultRet<_Rty>* result
        = dynamic_cast<CJobFutureResultRet<_Rty>* >(m_Result.GetPointerOrNull());
    if (result) {
        m_Canceled = result->GetCanceled();
        return result->SetData();
    }
    throw std::runtime_error(m_BadJobResponse);
}

template<>
class job_future<void> : public job_future_base
{
public:
    job_future() {}
    job_future(IAppJob& job) : job_future_base(job) {}

    job_future(job_future&& rhs) : job_future_base(std::move(rhs)) {}

    job_future& operator=(job_future&& rhs)
    {
        job_future_base::operator=(std::move(rhs));
        return *this;
    }

    NCBI_GUIUTILS_EXPORT void operator()();
};

class CJobFutureJobBase : public CJobCancelable
{
public:
    CJobFutureJobBase(const string& descr) : m_Descr() {}

    /// @name IAppJob implementation
    /// @{
    virtual string    GetDescr() const { return m_Descr; }
    virtual CConstIRef<IAppJobProgress> GetProgress()
    {
        return CConstIRef<IAppJobProgress>();
    }
    virtual CRef<CObject> GetResult()
    {
        return m_Result;
    }
    virtual CConstIRef<IAppJobError> GetError()
    {
        return CConstIRef<IAppJobError>(m_Error.GetPointer());
    }
    /// @}

protected:
    virtual string x_GetJobName() const { return m_Descr; }

    string m_Descr;
    CRef<CObject> m_Result;
    CConstRef<CJobFutureError> m_Error;
};

template<class _Fty, typename _Rty> class CJobFutureJob : public CJobFutureJobBase
{
public:
    typedef CJobFutureResultRet<_Rty> TResult;

    CJobFutureJob(const _Fty& _Fnarg, const string& descr)
        : CJobFutureJobBase(descr), m_Farg(_Fnarg) {}

    /// @name IAppJob implementation
    /// @{
    virtual EJobState Run();
    /// @}

private:
    _Fty m_Farg;
};

template<class _Fty, typename _Rty>
CJobCancelable::EJobState CJobFutureJob<_Fty, _Rty>::Run()
{
    m_Result.Reset(new TResult());
    TResult& result = static_cast<TResult&>(*m_Result);

    try {
        result.SetData() = m_Farg(*x_GetICanceled());
    }
    catch (const std::exception&) {
        m_Error.Reset(new CJobFutureError(std::current_exception()));
    }

    if (m_Error)
        return eFailed;

    if (IsCanceled())
        result.SetCanceled();

    return eCompleted;
}

template<class _Fty>
class CJobFutureJob<_Fty, void> : public CJobFutureJobBase
{
public:
    CJobFutureJob(const _Fty& _Fnarg, const string& descr)
        : CJobFutureJobBase(descr), m_Farg(_Fnarg) {}

    /// @name IAppJob implementation
    /// @{
    virtual EJobState Run();
    /// @}

private:
    _Fty m_Farg;
};

template<class _Fty>
CJobCancelable::EJobState CJobFutureJob<_Fty, void>::Run()
{
    m_Result.Reset(new CJobFutureResult());
    CJobFutureResult& result = static_cast<CJobFutureResult&>(*m_Result);

    try {
        m_Farg(*x_GetICanceled());
    }
    catch (const std::exception&) {
        m_Error.Reset(new CJobFutureError(std::current_exception()));
    }

    if (m_Error)
        return eFailed;

    if (IsCanceled())
        result.SetCanceled();

    return eCompleted;
}

template<class _Fty>
struct job_function_traits
{
    typedef typename std::result_of<_Fty(ICanceled&)>::type result;
    typedef CJobFutureJob<_Fty, result> job;
    typedef job_future<result> future;
};

template<class _Fty>
typename job_function_traits<_Fty>::future job_async(const _Fty& _Fnarg, const string& descr)
{
    typedef job_function_traits<_Fty> tr;
    CIRef<IAppJob> job(new typename tr::job(_Fnarg, descr));
    return typename tr::future(*job);
}

class NCBI_GUIUTILS_EXPORT async_job
{
    friend class CJobHandler;

public:
    virtual ~async_job();

    virtual bool running() = 0;
    virtual void cancel() = 0;

private:
    virtual bool complete() = 0;
    virtual bool canceled() = 0;
};

class NCBI_GUIUTILS_EXPORT CJobHandler
{
    friend class async_job;
public:
    static CJobHandler& Instance();

    void AddJob(async_job* job) { m_Jobs.push_back(job); }
    bool Process();

private:
    void x_RemoveJob(async_job* job) { m_Jobs.remove(job); }
    CJobHandler() {}

    list<async_job*> m_Jobs;
};

template<class _Fty, typename _Rty>
class async_job_impl : public async_job
{
public:
    async_job_impl(const _Fty& _Fnres, job_future<_Rty>&& _future) :
        fnres(_Fnres), future(std::move(_future)) {}

    virtual bool running() { return future.IsRunning();  }
    virtual void cancel() { future.cancel(); }

private:
    virtual bool complete() {
        if (future.IsComplete()) {
            fnres(future);
            future.reset();
            return true;
        }
        return false;
    }

    virtual bool canceled() { return future.IsCanceled(); }

    _Fty fnres;
    job_future<_Rty> future;
};

template<class _Fty, class _Fty2>
async_job* job_async(const _Fty& _Fnarg, const _Fty2& _Fnarg2, const string& descr)
{
    typedef job_function_traits<_Fty> tr;
    typedef async_job_impl<_Fty2, typename tr::result> async_job_type;
    async_job_type* job = new async_job_type(_Fnarg2, job_async(_Fnarg, descr));
    CJobHandler::Instance().AddJob(job);
    return job;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___JOB_FUTURE__HPP
