#ifndef GUI_OBJUTILS___JOB_ADAPTER__HPP
#define GUI_OBJUTILS___JOB_ADAPTER__HPP

/*  $Id: job_adapter.hpp 36528 2016-10-04 17:41:48Z katargir $
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

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <gui/gui_export.h>
#include <corelib/ncbiobj.hpp>

#include <gui/utils/event_handler.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/utils/app_job_impl.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIUTILS_EXPORT CJobAdapter : public CObjectEx,  public CEventHandler
{
    DECLARE_EVENT_MAP();
public:
    class IJobCallback
    {
    public:
        virtual ~IJobCallback() {}
        virtual void OnJobResult(CObject* result, CJobAdapter& adapter) = 0;
        virtual void OnJobFailed(const string& errMsg, CJobAdapter& adapter) = 0;
    };

    CJobAdapter(IJobCallback& callback)
        : m_Callback(&callback), m_JobId(CAppJobDispatcher::eInvalidJobID) {}
    ~CJobAdapter() { x_CancelJob(); }

    void Start(IAppJob& job);
    void Cancel();

    CAppJobDispatcher::TJobID GetJobId() const { return m_JobId; }

private:
    void x_OnJobNotification(CEvent* evt);
    void x_CancelJob();

    IJobCallback*             m_Callback;
    CAppJobDispatcher::TJobID m_JobId;
};

template<typename T> class CJobAdapterResult : public CObject
{
public:
    CJobAdapterResult() {}
    virtual ~CJobAdapterResult() {}
    T& SetData() { return m_Data; }
    const T& GetData() { return m_Data; }

private:
    T m_Data;
};

template<typename TInput, typename TOutput> class CJobAdapterJob : public CJobCancelable
{
public:
    typedef CJobAdapterResult<TOutput> TResult;
    typedef bool(*TFunc)(TInput&, TOutput&, string&, ICanceled&);

    CJobAdapterJob(TInput &input, TFunc func, const string& name, const string& descr)
        : m_Input(input), m_Func(func), m_Name(name), m_Descr(descr) {}
    virtual ~CJobAdapterJob() {}

    /// @name IAppJob implementation
    /// @{
    virtual string                      GetDescr() const { return m_Descr; }
    virtual EJobState                   Run()
    {
        string errMsg;
        m_Result.Reset(new TResult());
        bool success = m_Func(m_Input, m_Result->SetData(), errMsg, *x_GetICanceled());
        if (IsCanceled()) return eCanceled;
        if (success) return eCompleted;
        m_Error.Reset(new CAppJobError(errMsg));
        return eFailed;
    }
    virtual CConstIRef<IAppJobProgress> GetProgress()
        { return CConstIRef<IAppJobProgress>(); }
    virtual CRef<CObject>               GetResult()
        { return CRef<CObject>(m_Result.GetPointer()); }
    virtual CConstIRef<IAppJobError>    GetError()
        { return CConstIRef<IAppJobError>(m_Error.GetPointer()); }
    /// @}

protected:
    virtual string x_GetJobName() const { return m_Name; }

private:
    TInput m_Input;
    TFunc m_Func;
    CRef<TResult> m_Result;
    CRef<CAppJobError> m_Error;
    string m_Name;
    string m_Descr;
};

template<typename TInput, typename TOutput>
CJobAdapter* LaunchAdapterJob(CJobAdapter::IJobCallback* callback,
                              TInput &input,
                              bool (*func)(TInput&, TOutput&, string&, ICanceled&),
                              const string& name,
                              const string& descr)
{
    CRef<CJobAdapter> adapter(new CJobAdapter(*callback));
    CIRef<IAppJob> job(new CJobAdapterJob<TInput, TOutput>(input, func, name, descr));
    adapter->Start(*job);
    return adapter.Release();
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___JOB_ADAPTER__HPP
