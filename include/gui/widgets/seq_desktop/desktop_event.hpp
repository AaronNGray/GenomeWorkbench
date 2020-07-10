#ifndef GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_EVENT__HPP
#define GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_EVENT__HPP

/*  $Id: desktop_event.hpp 40275 2018-01-19 17:27:58Z katargir $
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
 * Authors:  Andrea Asztalos, based on a file by Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <gui/utils/event.hpp>


BEGIN_NCBI_SCOPE

class CDesktopWidgetEvent : public CEvent
{
public:
    enum EEventType {
        eNotSet,
        eRefreshData,
        eSingleClick
    };

    /// create an event for a given event ID, with event class Unknown
    CDesktopWidgetEvent(TEventID eid)
        : CEvent(eid)
    {
    }

    CDesktopWidgetEvent(EEventClass ecl, TEventID eid)
        : CEvent(ecl, eid)
    {
    }
};

END_NCBI_SCOPE

#endif  
    // GUI_WIDGETS_SEQ_DESKTOP___DESKTOP_EVENT__HPP

