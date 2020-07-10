/*  $Id: event_translator.cpp 33449 2015-07-27 17:47:12Z katargir $
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
 *
 */


#include <ncbi_pch.hpp>

#include <gui/utils/app_job_dispatcher.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAppJobNotification

CAppJobNotification::CAppJobNotification(TJobID job_id, TJobState state)
:   CEvent(eEvent_Message, eStateChanged),
    m_JobID(job_id),
    m_State(state)
{
}


CAppJobNotification::CAppJobNotification(TJobID job_id, CObject* result)
:   CEvent(eEvent_Message, eStateChanged),
    m_JobID(job_id),
    m_State(IAppJob::eCompleted),
    m_Result(result)
{
}


CAppJobNotification::CAppJobNotification(TJobID job_id, const IAppJobError& error)
:   CEvent(eEvent_Message, eStateChanged),
    m_JobID(job_id),
    m_State(IAppJob::eFailed),
    m_Error(&error)
{
}


CAppJobNotification::CAppJobNotification(TJobID job_id, const IAppJobProgress& progress)
:   CEvent(eEvent_Message, eProgress),
    m_JobID(job_id),
    m_State(IAppJob::eRunning),
    m_Progress(&progress)
{
}



///////////////////////////////////////////////////////////////////////////////
/// CAppJobEventTranslator

CAppJobEventTranslator::CAppJobEventTranslator(CEventHandler& target)
:   m_TargetEventHandler(target),
    m_Dispatcher(NULL)
{
    m_IsWeakTarget = true;
    CWeakObject* oweak = dynamic_cast<CWeakObject*>(&target);
    // We need to convert all event handling classes to be weak-ptr compatible (derive from CObjectEx)
    _ASSERT(oweak);
    if (!oweak) {
        m_IsWeakTarget = false;
    }
    else
    {
        try {
            m_TargetWeakPtr = &target;
        } catch (CObjectException& ) {
            m_IsWeakTarget = false;
            m_TargetWeakPtr.Reset();
        }
    }
}


void CAppJobEventTranslator::SetDispatcher(CAppJobDispatcher& disp)
{
    m_Dispatcher = &disp;
}

void CAppJobEventTranslator::x_NotifyObservers(CRef<CEvent> & evt)
{
    // Event handler supports weak pointer interface?
    if (m_IsWeakTarget) {
        CIRef<CEventHandler> weh(m_TargetWeakPtr.Lock());
        if (weh.NotNull()) {
            weh->Post(evt);
        }
    }
    else {
        m_TargetEventHandler.Post(evt);
    }
}

void CAppJobEventTranslator::OnJobStateChanged(CAppJobDispatcher::SJobRecord* job_rec, TJobState new_state)
{
    if(!m_Dispatcher) return;
    switch(new_state)   
    {
        case IAppJob::eCompleted:   
        {
            CRef<CObject> res(job_rec ? job_rec->m_Job->GetResult().GetPointer() : NULL);
            CRef<CEvent> evt(new CAppJobNotification(job_rec->m_ID, res.GetPointer()));
            x_NotifyObservers(evt);
            break;
        }
        case IAppJob::eFailed:  
        {
            CConstIRef<IAppJobError> err(job_rec ? job_rec->m_Job->GetError().GetPointer() : NULL);
            if (err) {            
                CRef<CEvent> evt(new CAppJobNotification(job_rec->m_ID, *err));
                x_NotifyObservers(evt);
            }
            else {
                ERR_POST("Job failed -- NULL error job_id= " << job_rec->m_ID);
                CRef<CEvent> evt(new CAppJobNotification(job_rec->m_ID, new_state));
                x_NotifyObservers(evt);
            }
            break;
        }
        case IAppJob::eSuspended:
        case IAppJob::eRunning:
            if( m_Dispatcher->x_IsCanceled(job_rec->m_ID) ) break;
            // intentional fall through

        case IAppJob::eCanceled:    
        {
            CRef<CEvent> evt(new CAppJobNotification(job_rec->m_ID, new_state));
            x_NotifyObservers(evt);
            break;
        }
        default:
            _ASSERT(false);
    }
}



void CAppJobEventTranslator::OnJobProgress(CAppJobDispatcher::SJobRecord* job_rec)
{
    if(m_Dispatcher)    {
        CConstIRef<IAppJobProgress> pr;
        if(job_rec) {
            if(job_rec->ActiveProgress()) {
                pr = job_rec->m_Progress;
            } else {
                pr = job_rec->m_Job->GetProgress();
            }
        }
        if(pr)  {
            CRef<CEvent> evt(new CAppJobNotification(job_rec->m_ID, *pr));
            x_NotifyObservers(evt);
        } else {
            LOG_POST(Error << "CAppJobEventTranslator::OnJobProgress() - "
                << " progress object is missing!");
        }
    }
}


END_NCBI_SCOPE
