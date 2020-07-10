#ifndef GUI_UTILS___EVENT_HANDLER__HPP
#define GUI_UTILS___EVENT_HANDLER__HPP

/*  $Id: event_handler.hpp 40286 2018-01-19 18:22:51Z katargir $
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
 *    Endpoint or target for public event.
 */

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <map>
#include <functional>
#include <corelib/ncbiexpt.hpp>
#include <corelib/ncbimtx.hpp>

#include <gui/utils/event.hpp>

BEGIN_NCBI_SCOPE
class CEventHandler;
END_NCBI_SCOPE

BEGIN_NCBI_SCOPE

class CEvent;

typedef int TCmdID;

/// event handler
typedef     void (CEventHandler::*FEventHandler)   (CEvent *);

///////////////////////////////////////////////////////////////////////////////
/// Command map entry.
struct SEvtMapEntry
{
    CEvent::EEventClass     evt_class; /// message or command or update
    CEvent::TEventTypeInfo  type_info; /// Type Information
    CEvent::TEventID        id;        /// Event ID (or ID range start)
    CEvent::TEventID        last_id;   /// Envent ID range end
    FEventHandler           handler;   /// pointer to command handler mapped to the Command ID
};


/// Command Map.
struct SEvtMap
{
    const SEvtMap*        base_map; /// pointer to command map of the base class
    const SEvtMapEntry*   entries;  /// pointer to array of map entries
};


// MACRO for declaring and defining command maps
#define DECLARE_EVENT_MAP() \
private: \
    static const SEvtMapEntry sm_EvtMapEntries[]; \
protected: \
    static  const SEvtMap   sm_EvtMap; \
    virtual const SEvtMap*  GetEventMap() const

/// Begins definition of Command Map for CEventHandler-derived class.
#define BEGIN_EVENT_MAP(thisClass, baseClass) \
    const SEvtMap* thisClass::GetEventMap() const \
        { return &thisClass::sm_EvtMap; } \
    const SEvtMap thisClass::sm_EvtMap = \
        { &baseClass::sm_EvtMap, &thisClass::sm_EvtMapEntries[0] }; \
    const SEvtMapEntry thisClass::sm_EvtMapEntries[] = \
        { \

/// Ends definition of Command Map.
#define END_EVENT_MAP() \
    { CEvent::eEvent_Message, typeid(void).name(), \
      CEvent::eEvent_InvalidID, CEvent::eEvent_InvalidID, 0} \
    };

#define ON_EVENT(type, id, handler) \
{ ncbi::CEvent::eEvent_Message, typeid(type).name(), id, id, (FEventHandler) handler },

#define ON_EVENT_RANGE(type, id, id_last, handler) \
{ CEvent::eEvent_Message, typeid(type).name(), id, id_last, (FEventHandler) handler },


///////////////////////////////////////////////////////////////////////////////
/// CEventHandler
///
/// CEventHandler provides basic support for event handling. Event handling
/// includes processing events and dispatching events.
/// Processing involves recognizing an event of the particular type and invoking
/// a corresponding callback. CEventHandler provide support for mapping events
/// to callback by means of Event Maps (see macros in this file). A class can
/// define its own Event Map that can be inherited from the Event map of the
/// base class.
/// Dispatching is a process of forwarding an event to other CEventHandler-s
/// subscribed as listeners to the current handler. Dispatching algorithm can be
/// controlled by specifying a strategy (EDispatch enumeration).
/// All events are dispatched within pools. Pools are isolated networks of
/// listeners subscribed to each other. An event sent to a pool will reach only
/// listeners added to this particular pool, all other listeners will be ignored.
/// CEventHandler has several entry points:
///   OnEvent() – processes an event, locates a callback corresponding to it
///               and executes the callback.
///   Dispatch() – dispatches an event to listeners.
///   Send() – synchronously handles events by calling OnEvent() and Dispatch().
///   Post() – posts an event to the event queue. Events are retrieved from the
///            queue by HandlePostRequest() and then sent to the handler.


class NCBI_GUIUTILS_EXPORT CEventHandler
{
public:
    /// enum controlling dispatching strategies
    enum EDispatch {
        eDispatch_SelfOnly,      /// handle and do not dispatch to listeners
        eDispatch_AllHandlers,   /// dispatch to all handlers
        eDispatch_FirstHandler,  /// dispatch until handled at least by one handler

        eDispatch_Default = eDispatch_AllHandlers
    };

    /// Identifiers for standard pools. Set of pools is extandable, programmers can
    /// define their own pools but need to make sure that pool identifiers are unique.
    enum EPoolName {
        ePool_Default = 0,
        ePool_Parent,
        ePool_Child,
        ePool_Sibling,

        ePool_NextAvailable /// this needs to be last!
    };

    typedef vector<CEventHandler*> TListeners;

    struct SPostRequest
    {
        CEventHandler*  m_Target;
        CRef<CEvent>    m_Event;
        EDispatch       m_DispHow;
        int             m_PoolName;
    };

    typedef map<int, TListeners>    TPools;
    typedef list< AutoPtr<SPostRequest> >    TPostRequests;
    typedef std::map<CEventHandler*, int>    THandlerToCount;
    typedef void (*FOnPostCallback)();

    CEventHandler();
    virtual ~CEventHandler();

    /// @name Event Dispatching Interface
    /// @{

    /// Add a listener.  The listener will always be added to the default pool;
    /// the 'name' parameter may be used to indicate an additional pool to which
    // the listener is subscribed.
    virtual void AddListener(CEventHandler* listener,
                             int pool_name = ePool_Default);

    /// Remove a listener. This will remove the listener from all pools in which
    /// it exists.
    virtual void RemoveListener(CEventHandler* listener);

    virtual void RemoveAllListeners(void);

    /// returns "true" if the given listener belongs to the specified pool
    virtual bool HasListener(CEventHandler* listener,
                             int pool_name = ePool_Default) const;

    /// returns a set of listeners fro the specified pool
    virtual const TListeners*   GetListeners(int pool_name = ePool_Default) const;

    /// Processes en event. Returns "true" if event has been processed.
    virtual bool OnEvent(CEvent * evt);

    /// Dispatches an event to the listeners (but does not handle it). Returns
    /// "true" if event has been dispatched and handled by a listener.
    virtual bool Dispatch(CEvent* evt,
                          EDispatch disp_how = eDispatch_Default,
                          int pool_name = ePool_Default);

    /// Sends an event synchronously. Returns "true" if event has been handled.
    /// An event that is provided as the argument can be created on heap or stack.
    virtual bool Send(CEvent* evt,
                      EDispatch disp_how = eDispatch_Default,
                      int pool_name = ePool_Default);
    virtual bool Send(CEvent* evt, int pool_name);

    /// Handles an event asynchronously (process and/or dispatch). The event
    /// will be posted to the event queue and then Post() will return, the event
    /// will be processed at indefenite point in time after Post() returns.
    /// The event must be created on heap and managed by CRef.
    void Post(CRef<CEvent> evt,
              EDispatch disp_how = eDispatch_Default,
              int pool_name = ePool_Default);
    /// @}

    /// @name Static API
    /// @{
    /// extracts the next request from the Post Queue and sends it to the target
    /// returns true if the Queue is empty

    static bool HandlePostRequest();

    /// erases all events from the queue
    static void ClearPostQueue();
    static void DestroyPostQueue();

    /// @}

protected:
    DECLARE_EVENT_MAP();

    /// Removes itself unavailable for async event delivery.
    void    x_DeclareDead();

    void    x_AddListenerToPool(CEventHandler* listener, int pool_name);

protected:
    TPools  m_Pools;

private:
    /// CPostQueue - singleton queue for asynchronous events
    /// all methods are synchronized
    class CPostQueue : public CObject
    {
    public:
        static CRef<CPostQueue>    GetInstance(); /// returns Singleton
        static void     DestroyInstance(); /// destroys singleton

        ~CPostQueue();

        void    Post(SPostRequest* req);
        bool    ExecuteFirstRequest();

        void    DeclareDead(CEventHandler* handler);
        void    Clear();

    private:
        static CRef<CPostQueue> sm_PostQueue; /// the only instance

        /// contains pointer to "alive" event targets and number of events for every target.
        /// When target is destroyed it must remove itself from this map.
        THandlerToCount m_AliveTargets;

        TPostRequests   m_Queue; /// queue of Events for async sending
        CMutex          m_Mutex; /// mutex guarding the sm_PostQueue and sm
    };

private:
    CRef<CPostQueue>    m_Queue; /// a reference to the singleton (to make sure that
    /// the queue will not be destroyed until the Last handler is destroyed
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___EVENT_HANDLER__HPP
