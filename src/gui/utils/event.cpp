/*  $Id: event.cpp 40751 2018-04-09 18:24:49Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/utils/event.hpp>

BEGIN_NCBI_SCOPE


CEvent::CEvent()
    : m_Class(eEvent_Message),
      m_ID(0),
      m_Sender(NULL),
      m_AttPolicy(eRelease)
{
}

CEvent::CEvent(TEventID eid)
    : m_Class(eEvent_Message),
      m_ID(eid),
      m_Sender(NULL),
      m_AttPolicy(eRelease)
{
}


CEvent::CEvent(EEventClass ecl, TEventID eid, TEventSender* sender)
    : m_Class(ecl),
      m_ID(eid),
      m_Sender(sender),
      m_AttPolicy(eRelease)
{
}


CEvent::CEvent(EEventClass ecl, TEventID eid, IEventAttachment* att,
               EOwnershipPolicy policy, TEventSender* tr)
    : m_Class(ecl),
      m_ID(eid),
      m_Sender(tr)
{
    SetAttachment(att, policy);
}

CEvent::~CEvent()
{
    if(m_AttPolicy == eRelease)   {
        m_Attachment.release(); // save it from deleting
    }
}


IEventAttachment* CEvent::GetAttachment(void)
{
    return m_Attachment.get();
}


void CEvent::SetAttachment(IEventAttachment* att, EOwnershipPolicy policy)
{
    if(m_AttPolicy == eRelease)   {
        m_Attachment.release(); // save it from deleting
    }

    m_Attachment.reset(att);
    m_AttPolicy = policy;
}


bool CEvent::HasAttachment(void) const
{
    return (m_Attachment.get() != NULL);
}


bool CEvent::Visit(CEventHandler* handler)
{
    set<CEventHandler*>::const_iterator it = m_Visited.find(handler);
    if(it == m_Visited.end()) {
        m_Visited.insert(handler);
        return true;
    } else {
        return false; // visited before
    }
}



END_NCBI_SCOPE
