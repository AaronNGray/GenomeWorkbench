#ifndef GUI_UTILS___VIEW_EVENT__HPP
#define GUI_UTILS___VIEW_EVENT__HPP

/*  $Id: view_event.hpp 40289 2018-01-19 19:02:37Z katargir $
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
 *  Government have not placed CAnyType restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for CAnyType particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *    Event object for view communications
 */

#include <gui/utils/event.hpp>
#include <gui/objutils/objects.hpp>
#include <objects/general/User_object.hpp>

BEGIN_NCBI_SCOPE

class CViewEvent : public CEvent
{
public:
    enum EEventType {
        eUnknownEvent,

        /// general notification that selections have changed
        /// This is to be packaged in a CSelectionEvent.
        eSelectionChanged,
        eProjectChanged, /// general data update notification
        eViewAttached,   /// a view has been initialized and ready
        eViewReleased,    /// a view has been destroyed

        /// notification from child to parent that a selection has changed
        eWidgetSelectionChanged,

        /// notification from child to parent that the visible range has changed
        eWidgetRangeChanged,

        /// notification from child to parent that the underlying data has changed
        eWidgetDataChanged,

        eEditSelectedObjects,
        eDeleteSelectedObjects,
        eWidgetItemActivated,
        eOpenGraphicalViewEvent
    };

    /// default ctor
    CViewEvent()
        : CEvent()
    {
    }

    /// create an event for a given event ID, with event class Unknown
    CViewEvent(TEventID eid)
        : CEvent(eid)
    {
    }

    /// create an event for a specific event class and ID
    CViewEvent(EEventClass ecl, TEventID eid)
        : CEvent(ecl, eid)
    {
    }

    EEventType GetType() const { return (EEventType)GetID(); }
};

class COpenGraphicViewEvent : public CEvent
{
public:
    COpenGraphicViewEvent(const SConstScopedObject& object, const objects::CUser_object* params)
        : CEvent(CViewEvent::eOpenGraphicalViewEvent), m_Object(object), m_Params(params) {}

    SConstScopedObject& GetObject() { return m_Object; }
    const objects::CUser_object* GetParams() const { return m_Params; }

private:
    SConstScopedObject m_Object;
    CConstRef<objects::CUser_object> m_Params;
};

END_NCBI_SCOPE

#endif  // GUI_UTILS___VIEW_EVENT__HPP
