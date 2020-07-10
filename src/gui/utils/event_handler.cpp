/*  $Id: event_handler.cpp 40834 2018-04-16 19:16:21Z katargir $
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
 * Authors:  Vladimir Tereshkov, Andrey Yazhuk
 *
 * File Description:
 *      Event receiver implementation
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <gui/utils/event_handler.hpp>

#include <algorithm>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CEventHandler

CEventHandler::CEventHandler()
{
}


CEventHandler::~CEventHandler()
{
    x_DeclareDead();

#ifdef _DEBUG
    ///
    /// disconnect our listeners
    ///
    TPools::iterator iter = m_Pools.find(ePool_Default);
    if (iter != m_Pools.end()) {
        NON_CONST_ITERATE (TListeners, it, iter->second) {
            if (*it) {
                (*it)->RemoveListener(this);
            }
        }
    }
    m_Pools.clear();
#endif
}


void CEventHandler::x_DeclareDead()
{
    // make sure it not receive any async events after from the queue
    if(m_Queue)   {
        m_Queue->DeclareDead(this);
    }
}


/// Traverses a list of Command Maps in order to locate a handler corresponding
/// to the given command ID.
const SEvtMapEntry* FindEventMapEntry(const SEvtMap* pMap,
                                      CEvent::EEventClass cls,
                                      CEvent::TEventTypeInfo tif,
                                      CEvent::TEventID CmdID)
{
    const SEvtMapEntry* pEntry = 0;

    while(pMap) {
        for (pEntry = pMap->entries;  pEntry->id != CEvent::eEvent_InvalidID;  ++pEntry) {

            bool cmd_between = (CmdID >= pEntry->id  &&  CmdID <= pEntry->last_id);
            bool typeinfo_safe = (strcmp(tif, pEntry->type_info) == 0);
            bool class_safe = (cls == pEntry->evt_class);

            if (cmd_between  &&  typeinfo_safe  &&  class_safe) {
                return pEntry; // matching handler has been found
            }
        }
        pMap = pMap->base_map; // go to the command map of the base class
    }

    return pEntry; // handler not found, or "fake" update handler found
}

////////////////////////////////////////////////////////////////////////////////
// class CEventHandler

const SEvtMap CEventHandler::sm_EvtMap =
{
    NULL, // no base class
    &CEventHandler::sm_EvtMapEntries[0]
};


const SEvtMap* CEventHandler::GetEventMap() const
{
    return &CEventHandler::sm_EvtMap;
}

const SEvtMapEntry CEventHandler::sm_EvtMapEntries[] =
{
    {   CEvent::eEvent_Message, typeid(void).name(),
        CEvent::eEvent_InvalidID, CEvent::eEvent_InvalidID, 0 } // no entries
};


void CEventHandler::AddListener(CEventHandler* listener, int pool)
{
    if(listener) {
        if(pool != ePool_Default)   {
            x_AddListenerToPool(listener, ePool_Default);
        }
        x_AddListenerToPool(listener, pool);
    } else {
        _TRACE("CEventHandler::AddListener(): NULL listener");
        return;
    }
}


void CEventHandler::x_AddListenerToPool(CEventHandler* listener, int pool_name)
{
    _ASSERT(listener);

    TListeners& pool = m_Pools[pool_name];
    TListeners::iterator it = std::find(pool.begin(), pool.end(), listener);
    if(it == pool.end()) {
        pool.push_back(listener);
    }
}


void CEventHandler::RemoveListener(CEventHandler* listener)
{
    if(listener) {
        for(TPools::iterator it = m_Pools.begin(); it != m_Pools.end();  ) {
            TListeners& pool = it->second;
            TListeners::iterator it_lst = std::find(pool.begin(), pool.end(), listener);
            if(it_lst != pool.end())  {   // the listener was found in the pool
                pool.erase(it_lst);
            }
            if(pool.empty()) {  // delete the pool
                TPools::iterator it_del = it;
                ++it;
                m_Pools.erase(it_del);
            } else {
                ++it;
            }
        }
    } else {
        _TRACE("CEventHandler::RemoveListener(): NULL listener");
        return;
    }
}


void CEventHandler::RemoveAllListeners(void)
{
    m_Pools.clear();
}


bool CEventHandler::HasListener(CEventHandler* listener, int pool_name) const
{
    if(listener != NULL)    {
        TPools::const_iterator it_pool = m_Pools.find(pool_name);
        if(it_pool != m_Pools.end())    {
            const TListeners& pool = it_pool->second;
            TListeners::const_iterator it = std::find(pool.begin(), pool.end(), listener);
            return (it != pool.end());
        }
    }
    return false;
}


const CEventHandler::TListeners* CEventHandler::GetListeners(int pool_name) const
{
    TPools::const_iterator it_pool = m_Pools.find(pool_name);
    return (it_pool != m_Pools.end()) ? &it_pool->second : NULL;
}


bool CEventHandler::OnEvent(CEvent * evt)
{
    _ASSERT(evt);
    evt->Visit(this);

    try {
        const CEvent::EEventClass cls = evt->GetEventClass();
        CEvent::TEventTypeInfo info = evt->GetTypeInfo();
        CEvent::TEventID id = evt->GetID();

        const SEvtMapEntry* pEntry =
            FindEventMapEntry(GetEventMap(), cls, info, id);

        if (pEntry) {
            if (pEntry->handler) {
                FEventHandler handler = pEntry->handler;
                (this->*handler)(evt);
                return true;
            }
        }
    } NCBI_CATCH("CEventHandler::OnEvent()");

    return false; // not handled
}


bool CEventHandler::Dispatch(CEvent* evt, EDispatch disp_how, int pool)
{
    _ASSERT(evt);
    bool handled = false;

    try {
        TPools::iterator iter = m_Pools.find(pool);
        if(iter == m_Pools.end()) {
            return false;
        }

        switch (disp_how) {
        case eDispatch_SelfOnly: // does not try to handle - Send() does that
            break;

        case eDispatch_AllHandlers:
        {
            TListeners listeners_cpy(iter->second);
            ITERATE(TListeners, listener, listeners_cpy) {
                CEventHandler*   handler = *listener;
                if (evt->Visit(handler)) {
                    handled |= handler->Send(evt, disp_how, pool);
                }
            }
        }
        break;

        case eDispatch_FirstHandler:
        {
            TListeners listeners_cpy(iter->second);
            ITERATE(TListeners, listener, listeners_cpy){
                CEventHandler*   handler = *listener;
                if (evt->Visit(handler)) {
                    handled = handler->Send(evt, disp_how, pool);
                    if (handled) {
                        return true;
                    }
                }
            }
        }
        break;
        }
    } NCBI_CATCH("CEventHandler::Dispatch()");

    return handled;
}


bool CEventHandler::Send(CEvent* evt, EDispatch disp_how, int pool_name)
{
    _ASSERT(evt);

    bool handled = OnEvent(evt);

    switch (disp_how) {
    case eDispatch_SelfOnly: // does not dispatch to the listeners
        break;

    case eDispatch_AllHandlers: // dispatch always
        handled |= Dispatch(evt, disp_how, pool_name);
        break;

    case eDispatch_FirstHandler: // dispatch only if not handled
        if( ! handled)  {
            handled = Dispatch(evt, disp_how, pool_name);
        }
        break;
    }
    return handled;
}


bool CEventHandler::Send(CEvent* evt, int pool_name)
{
    return Send(evt, eDispatch_Default, pool_name);
}


void CEventHandler::Post(CRef<CEvent> evt, EDispatch disp_how, int pool_name)
{
    _ASSERT(evt);
    if( ! m_Queue) {
        m_Queue = CPostQueue::GetInstance();
    }

    SPostRequest* req = new SPostRequest();
    req->m_Target = this;
    req->m_Event = evt;
    req->m_DispHow = disp_how;
    req->m_PoolName = pool_name;

    m_Queue->Post(req);
}


bool CEventHandler::HandlePostRequest()
{
    CRef<CPostQueue> queue = CPostQueue::GetInstance();
    return queue->ExecuteFirstRequest();
}


/// removes all requests from the Queue
void CEventHandler::ClearPostQueue()
{
    CRef<CPostQueue> queue = CPostQueue::GetInstance();
    queue->Clear();
}


void CEventHandler::DestroyPostQueue()
{
    CPostQueue::DestroyInstance();
}


///////////////////////////////////////////////////////////////////////////////
/// CPostQueue

CRef<CEventHandler::CPostQueue>   CEventHandler::CPostQueue::sm_PostQueue;

CRef<CEventHandler::CPostQueue> CEventHandler::CPostQueue::GetInstance()
{
    if( ! sm_PostQueue)    {        // fast check
        static CMutex s_CreateQueueMutex;
        TMutexGuard lock(s_CreateQueueMutex);

        /// now safe check
        if( ! sm_PostQueue)    {
            sm_PostQueue.Reset(new CPostQueue()); // create the singleton
        }
    }
    return sm_PostQueue;
}


void CEventHandler::CPostQueue::DestroyInstance()
{
    sm_PostQueue.Reset();
}


CEventHandler::CPostQueue::~CPostQueue()
{
    CMutexGuard guard(m_Mutex);

    NON_CONST_ITERATE(THandlerToCount, it, m_AliveTargets)  {
        it->first->m_Queue.Reset(); // disconnect from the queue
    }

    m_AliveTargets.clear();
    m_Queue.clear();
}

void CEventHandler::CPostQueue::Post(SPostRequest* req)
{
    if (!req) return;

    size_t size = 0;
    {
        CMutexGuard guard(m_Mutex);

        CEventHandler* target = req->m_Target;
        THandlerToCount::iterator it = m_AliveTargets.find(target);
        if (it == m_AliveTargets.end()) {  // this is the first event
            m_AliveTargets[target] = 1; // register itself as "alive" target
        }
        else {
            it->second++; // increment event counter
        }

        m_Queue.push_back(req); // post the request
        size = m_Queue.size();
    }

    if (size >= 1000 && (size % 100) == 0) {
#ifdef _DEBUG
        LOG_POST(Error << "CEventHandler::CPostQueue::Post() - "
            "queue is too long, size = " << size);
#endif
    }
}


/// extracts a request from the front of the queue and executes it
bool CEventHandler::CPostQueue::ExecuteFirstRequest()
{
    CRef<CObject> target_guard; // protect the target if possible
    // requests will be deleted at the end of the function
    // this prevents deadlocks if CEvent::~CEvent uses CEventHandler functions
    TPostRequests toDelete;
    SPostRequest* request = 0;

    {{
         TMutexGuard guard(m_Mutex);
         for (auto i = m_Queue.begin(); i != m_Queue.end() && !request;) {
             CEventHandler* target = (*i)->m_Target;
             auto it = m_AliveTargets.find(target);
             if (it != m_AliveTargets.end()) {
                 request = i->get();
                 target_guard.Reset(dynamic_cast<CObject*>(target));
                 if (it->second == 1) // target does not have any other events in the queue
                     m_AliveTargets.erase(it);
                 else
                     it->second--;
             }
             toDelete.push_back(*i);
             i = m_Queue.erase(i);
         }
    }}

    if (request) {
        CEventHandler* target = request->m_Target;
        CEvent* evt = request->m_Event.GetPointerOrNull();
        _ASSERT(target  &&  evt);
        target->Send(evt, request->m_DispHow, request->m_PoolName);
        return true;
    }

    return false; // the Queue is empty
}


void CEventHandler::CPostQueue::DeclareDead(CEventHandler* handler)
{
    if(handler) {
        CMutexGuard guard(m_Mutex);
        m_AliveTargets.erase(handler); // delete this from the list of alive Post() targets
    }
}


void CEventHandler::CPostQueue::Clear()
{
    CMutexGuard guard(m_Mutex);
    m_Queue.clear();
    m_AliveTargets.clear();
}


END_NCBI_SCOPE
