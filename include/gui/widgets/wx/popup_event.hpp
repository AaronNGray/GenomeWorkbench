#ifndef GUI_WIDGETS_WX___POPUP_EVENT__HPP
#define GUI_WIDGETS_WX___POPUP_EVENT__HPP

/*  $Id: popup_event.hpp 30858 2014-07-31 14:05:43Z ucko $
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
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/event.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CPopupMenuEvent - an event that is sent when a popup menu needs to be shown.
/// The event allows to delegate one of the two functions - decorating popup menu
/// that is about to be shown or constructing and showing the menu.
/// For eDecorate event the reciever can add wxEvtHandlers to the event objects,
/// then menu sender should use these handler to handle the contributed commands.

class NCBI_GUIWIDGETS_WX_EXPORT CPopupMenuEvent : public CEvent
{
public:
    typedef vector<wxEvtHandler*>   THandlers;

    // TODO combine both types in one
    enum EType {
        eDecorate = eEvent_MinClientID, // the reciever should decorate the menu
        eShow // the reciever shall construct and display popup menu
    };

    CPopupMenuEvent(TEventSender* sender, wxMenu* menu)
    :   CEvent(eEvent_Message, eDecorate, sender),
        m_Menu(menu)
    {
    }
    CPopupMenuEvent(TEventSender* sender)
    :   CEvent(eEvent_Message, eShow, sender),
        m_Menu(NULL)
    {
    }

    wxMenu*  GetMenu()
    {
        return m_Menu;
    }
    void    SetMenu(wxMenu* menu)
    {
        m_Menu = menu;
    }
    void    AddHandler(wxEvtHandler* handler)
    {
        _ASSERT(handler);
        m_Handlers.push_back(handler);
    }
    THandlers&  GetHandlers()
    {
        return m_Handlers;
    }

protected:
    // the menu that sender is about to show; the reciever can modify the
    // existing object or replace it with a new one
    wxMenu* m_Menu;

    // a collection of wxEvtHandler-s that will handle commands contributed
    // to the menu. The caller must add these handlers to its event processing
    // chain. The caller assumes ownership of the handlers and must delete
    // them after use
    THandlers   m_Handlers;
};

END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___POPUP_EVENT__HPP
