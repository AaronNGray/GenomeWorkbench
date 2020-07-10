#ifndef GUI_WIDGETS_WX___GUI_EVENT__HPP
#define GUI_WIDGETS_WX___GUI_EVENT__HPP

/*  $Id: gui_event.hpp 17877 2008-09-24 13:03:09Z dicuccio $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>

#include <map>
#include <vector>
#include <set>

class wxEvent;
class wxKeyEvent;
class wxMouseEvent;


BEGIN_NCBI_SCOPE


/// CGUIEvent provides mapping of FLTK events to Semantic Events.
///
/// Goals:
/// 1. To isolate programs from the complexities of platform-dependent bindings
/// of mouse and keyboard events to Actions.
/// 2. To simplify event handling in programs by mapping multiple FLTK events
/// to a single Semantic Event (for instance 'Ctrl'+C and 'Ctrl'+'Ins' - >
/// eCopy)
/// 3. To provide uniform and consistent mapping of Events to Actions
/// controlled in one place.
/// 4. To provide workarounds for some deficiencies of FLTK event handling
/// system.
///
/// CGUIEvent support two type of Semantic Events - mouse events and shortcuts.
/// Both types of events may emit Signals depending on the current state of
/// modifiers , however, there is a significant difference in a way mapping is
/// declared.
///
/// States for mouse events are explicintly declared and registered in
/// CGUIEvent using RegisterState() function. Then it is possible to bind a
/// Signal to a particular mouse Event depending on the current State. States
/// for mouse events represent different GUI modes, user enter a mode by
/// pressing a combination of modifier keys and remains in this mode by holding
/// the keys pressed.
///
/// Shortcuts also depend on pressed modifiers, but defining Shortcuts does not
/// require registering special states. RegisterShortcut() function takes a
/// combination of modifiers and an action key. If action key is pressed while
/// specified combination of modifiers is being hold - CGUIEvent generates a
/// Signal corresponding to shortcut.  Handling shortcuts does not change
/// current state and so do not interfere with handling of mouse events.

class  NCBI_GUIWIDGETS_WX_EXPORT CGUIEvent
{
public:
    /*enum EModifierFlags {
        fShift              = 0x0001,
        fCtrl               = 0x0002,
        fAlt                = 0x0004,
        fMeta               = 0x0008,
        fModifiersMask  = fShift | fCtrl | fAlt | fMeta
    };

    enum EMouseEvent {
        eNone,
        eMouseMove,
        eLeftMousePush,
        eLeftMouseDrag,
        eLeftMouseRelease,
        eMiddleMousePush,
        eMiddleMouseDrag,
        eMiddleMouseRelease,
        eRightMousePush,
        eRightMouseDrag,
        eRightMouseRelease
    };*/

    enum EGUIState {
        eDefaultState,
        eSelectState,
        eSelectIncState,
        eSelectExtState,
        eLast
    };

    /*enum EGUISignal {
        eDefaultSignal = 0,
        eSelectSignal,
        ePopupSignal,
        ePopupSelectSignal,
        ePush,
        eDrag,
        eRelease,
    };*/

public:
    /// @name New wxWidgets API
    /// @{
    //void    wxWidgetsEvent(const wxEvent& event);

    // alternative state-less API
    static EGUIState wxGetSelectState(const wxMouseEvent& event);
    static EGUIState wxGetSelectState(const wxKeyEvent& event);
    /// @}
};

END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___GUI_EVENT__HPP
