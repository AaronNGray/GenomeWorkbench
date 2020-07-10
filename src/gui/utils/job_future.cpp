/*  $Id: job_future.cpp 43932 2019-09-20 19:58:40Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/utils/job_future.hpp>
#include <corelib/ncbi_system.hpp>

BEGIN_NCBI_SCOPE

///
/// CJobFutureAdapter
///

BEGIN_EVENT_MAP(CJobFutureAdapter, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged, &CJobFutureAdapter::x_OnJobNotification)
END_EVENT_MAP()

CJobFutureAdapter::CJobFutureAdapter(IJobCallback& callback)
    : m_Callback(&callback), m_JobId(CAppJobDispatcher::eInvalidJobID)
{

}

void CJobFutureAdapter::Start(IAppJob& job)
{
    m_JobId = CAppJobDispatcher::GetInstance().StartJob(job, "ObjManagerEngine", *this, -1, true);
}

void CJobFutureAdapter::Detach()
{
    m_Callback = 0;

    if (m_JobId == CAppJobDispatcher::eInvalidJobID)
        return;

    CAppJobDispatcher::GetInstance().DeleteJob(m_JobId);
    m_JobId = CAppJobDispatcher::eInvalidJobID;
}

void CJobFutureAdapter::Cancel()
{
    if (m_JobId == CAppJobDispatcher::eInvalidJobID)
        return;

    try {
        CAppJobDispatcher::GetInstance().CancelJob(m_JobId);
    }
    catch (const std::exception&) {}
}

bool CJobFutureAdapter::IsRunning() const
{
    if (m_JobId == CAppJobDispatcher::eInvalidJobID)
        return false;

    return (IAppJob::eRunning == CAppJobDispatcher::GetInstance().GetJobState(m_JobId));
}

void CJobFutureAdapter::x_OnJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);
    if (!notn) return;

    switch (notn->GetState()) {
    case IAppJob::eCompleted:
        m_JobId = CAppJobDispatcher::eInvalidJobID;

        if (m_Callback) {
            CRef<CObject> result = notn->GetResult();
            m_Callback->OnJobResult(result);
            return;
        }
        break;
    case IAppJob::eFailed:
        m_JobId = CAppJobDispatcher::eInvalidJobID;

        if (m_Callback) {
            CConstIRef<IAppJobError> error = notn->GetError();
            m_Callback->OnJobFailed(error);
            return;
        }
        break;
    case IAppJob::eCanceled:
        m_JobId = CAppJobDispatcher::eInvalidJobID;

        if (m_Callback) {
            m_Callback->OnJobCanceled();
        }
        break;
    default:
        break;
    }
}

///
/// job_future_base
///

const char* job_future_base::m_BadJobResponse = "Improper response from job";

job_future_base::job_future_base(IAppJob& job)
    : m_State(eRunning), m_JobAdapter(new CJobFutureAdapter(*this))
{
    m_JobAdapter->Start(job);
}

void job_future_base::reset()
{
    if (m_JobAdapter) {
        m_JobAdapter->Detach();
        m_JobAdapter.Reset();
    }
    m_Result.Reset();
    m_Error.Reset();
    m_State = eUndefined;
    m_Canceled = false;
}

void job_future_base::handle_error()
{
    if (m_State != eComplete)
        throw std::bad_function_call();

    m_State = eUndefined;

    if (m_Error)
        m_Error->Rethrow();
}

void job_future_base::OnJobResult(CObject* result)
{
    m_Result.Reset(result);
    m_State = eComplete;
}

void job_future_base::OnJobFailed(const IAppJobError* error)
{
    const CJobFutureError *jobError = dynamic_cast<const CJobFutureError*>(error);
    if (jobError) {
        m_Error.Reset(jobError);
    }
    else {
        std::exception_ptr exc = 
            std::make_exception_ptr(std::runtime_error(m_BadJobResponse));
        m_Error.Reset(new CJobFutureError(exc));
    }
    m_State = eComplete;
}

void job_future_base::OnJobCanceled()
{
    m_State = eUndefined;
}

void job_future<void>::operator()()
{
    handle_error();

    CJobFutureResult* result
        = dynamic_cast<CJobFutureResult*>(m_Result.GetPointerOrNull());
    if (result) {
        m_Canceled = result->GetCanceled();
        return;
    }
    throw std::runtime_error(m_BadJobResponse);
}

CJobHandler& CJobHandler::Instance()
{
    static CJobHandler _instance;
    return _instance;
}

bool CJobHandler::Process()
{
    for (auto it = m_Jobs.begin(); it != m_Jobs.end();) {
        if ((*it)->canceled())
            it = m_Jobs.erase(it);
        else
            ++it;
    }

    for (auto it = m_Jobs.begin(); it != m_Jobs.end(); ++it) {
        if ((*it)->complete()) {
            m_Jobs.erase(it);
            return true;
        }
    }
    return false;
}

async_job::~async_job()
{
    CJobHandler::Instance().x_RemoveJob(this);
}

END_NCBI_SCOPE
