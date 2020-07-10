/*  $Id: job_adapter.cpp 40279 2018-01-19 17:48:49Z katargir $
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

#include <gui/utils/job_adapter.hpp>

BEGIN_NCBI_SCOPE

BEGIN_EVENT_MAP(CJobAdapter, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged, &CJobAdapter::x_OnJobNotification)
END_EVENT_MAP()

void CJobAdapter::Start(IAppJob& job)
{
    x_CancelJob();
    m_JobId = CAppJobDispatcher::GetInstance().StartJob(job, "ObjManagerEngine", *this, -1, true);
}

void CJobAdapter::Cancel()
{
    m_Callback = 0;
    x_CancelJob();
}

void CJobAdapter::x_CancelJob()
{
    if (m_JobId != CAppJobDispatcher::eInvalidJobID) {
        CAppJobDispatcher::GetInstance().DeleteJob(m_JobId);
        m_JobId = CAppJobDispatcher::eInvalidJobID;
    }
}

void CJobAdapter::x_OnJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);
    if (!notn) return;

    switch (notn->GetState()) {
    case IAppJob::eCompleted :
        m_JobId = CAppJobDispatcher::eInvalidJobID;

        if (m_Callback) {
            CRef<CObject> result = notn->GetResult();
            m_Callback->OnJobResult(result.GetPointerOrNull(), *this);
            return;
        }
        break;
    case IAppJob::eFailed:
        m_JobId = CAppJobDispatcher::eInvalidJobID;

        if (m_Callback) {
            CConstIRef<IAppJobError> error = notn->GetError();
            string errMsg = error ? error->GetText() : string();
            m_Callback->OnJobFailed(errMsg, *this);
            return;
        }
        break;
    case IAppJob::eCanceled:
        m_JobId = CAppJobDispatcher::eInvalidJobID;
        break;
    default:
        break;
    }
}

END_NCBI_SCOPE
