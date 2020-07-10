#ifndef GUI_UTILS___EVENT__HPP
#define GUI_UTILS___EVENT__HPP

/*  $Id: event.hpp 40751 2018-04-09 18:24:49Z katargir $
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
 *    Event object for communicating between transmitter and receiver
 */

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <memory>
#include <set>

BEGIN_NCBI_SCOPE

class CEventHandler;

/// object, that will be attached to event
class IEventAttachment {
public:
    virtual ~IEventAttachment() { }
};


/// templatized IEventAttachment wrapper
template<typename T>
class CEventAttachmentFor : public IEventAttachment
{
public:
    typedef T TObjectType;

    /// provide non-const access to the packaged data
    TObjectType& SetData(void)                {  return m_Data; }
    void         SetData(const TObjectType& data)   {  m_Data = data; }

    /// provide const access to the packaged data
    const TObjectType& GetData(void) const    {  return m_Data; }

    /// cast operators to get the data
    operator TObjectType& (void)              {  return SetData();  }
    operator const TObjectType& (void) const  {  return GetData();  }

private:
    TObjectType m_Data;
};


///////////////////////////////////////////////////////////////////////////////
/// CEvent - generic event implementation
/// TODO
/// TODO - Attachments

class NCBI_GUIUTILS_EXPORT CEvent : public CObject
{
public:
    typedef const char*         TEventTypeInfo;
    typedef CRef<CEvent>        TEventObject;
    typedef Int4                TEventID;
    typedef CEventHandler       TEventSender;
    typedef Int4                TEventRefs;

    typedef std::type_info               TAttachmentTypeInfo;


    /// default event classes
    enum EEventClass {
        eEvent_Message         /// message from one class to another
    };

    /// Predefining event IDs
    enum EEventID {
        eEvent_LastID = -1,
        eEvent_InvalidID = -2,
        eEvent_MinClientID = 0  // set to 0, because by default enums start from 0
    };

    enum EOwnershipPolicy    {
        eDelete,    /// delete object when ownership end
        eRelease    /// release object (do not delete)
    };

    /// default ctor
    CEvent();

    /// create an event for a given event ID, with event class Unknown
    CEvent(TEventID eid);

    /// create an event for a specific event class and ID
    CEvent(EEventClass ecl, TEventID eid, TEventSender* sender = NULL);

    /// create an event with all the bells and whistles
    CEvent(EEventClass ecl, TEventID eid, IEventAttachment* att,
           EOwnershipPolicy policy, TEventSender* sender);

    /// virtual dtor
    virtual ~CEvent(void);

    const TEventRefs    GetRefs(void) const;
    void                AddRef(void);
    const TEventID      GetID(void) const;
    const EEventClass   GetEventClass(void) const;
    const TEventSender* GetSender(void) const;
    TEventTypeInfo      GetTypeInfo(void);

    /// attach an object to the Event
    virtual void SetAttachment(IEventAttachment* att, EOwnershipPolicy policy);
    virtual  IEventAttachment* GetAttachment(void);
    virtual bool HasAttachment(void) const;


    /// Checks whether this event has been visited by the given handler.
    /// Returns "true" if this is the first visit.
    bool    Visit(CEventHandler* handler);

protected:
    EEventClass m_Class; /// Event class - to speed up avoiding extra dynamic casts
    TEventID    m_ID; /// Defines unique event identity within its C++ class and Event Class
    TEventSender* m_Sender; /// pointer to the class that sent this event

    unique_ptr<IEventAttachment>  m_Attachment;
    EOwnershipPolicy    m_AttPolicy;

    /// list of handler who have seen this event already
    set<CEventHandler*>     m_Visited;
};


/////////////////////////////////////////////////////////////////////////////
/// Inline Implementation

inline
const CEvent::TEventID CEvent::GetID(void) const
{
    return m_ID;
}


inline
const CEvent::EEventClass CEvent::GetEventClass(void) const
{
    return m_Class;
}


inline
const CEvent::TEventSender* CEvent::GetSender(void) const
{
    return m_Sender;
}


inline
CEvent::TEventTypeInfo CEvent::GetTypeInfo(void)
{
    return typeid(*this).name();
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___EVENT__HPP
