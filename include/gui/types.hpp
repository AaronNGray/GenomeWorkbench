#ifndef GUI___TYPES__HPP
#define GUI___TYPES__HPP

/*  $Id: types.hpp 15475 2007-12-06 00:01:59Z yazhuk $
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
 * Authors:  Denis Vakatov
 *
 * File Description:
 *   NCBI GUI types
 *
 */

#include <corelib/ncbistd.hpp>
#include <util/range.hpp>


BEGIN_NCBI_SCOPE


/// enumerated type for dialog boxes
enum EDialogType
{
    eDialog_StyleMask   = 0x00ff,
    eDialog_Ok          = 0x0001,
    eDialog_OkCancel    = 0x0002,
    eDialog_YesNo       = 0x0003,
    eDialog_YesNoCancel = 0x0004,

    // flags that can modify the styles
    eDialog_Modal           = 0x0100,
    eDialog_Centered        = 0x0200,
    eDialog_CenteredAtMouse = 0x0400
};
typedef unsigned int TDialogType;


/// enumerated type for dialog icon
enum EDialogIcon
{
    eIcon_Info,
    eIcon_Question,
    eIcon_Exclamation,
    eIcon_Stop
};

/// enumerated return values for dialog boxes, starting from 1 to undermine
/// attempts to cast it "bool"
enum EDialogReturnValue {
    eCancel = 1,
    eNo,
    eOK,
    eYes
};


/// enumerated return values for dialog boxes
enum EDialogTextMode {
    eRaw,
    eWrap
};

// TODO drop this enumeration, use a class derived from CObserverUpdate instead
// we disregard most of these flags anyway

/// Enumerated values for update states.
/// These are used by the framework to communicate between projects and views.
//
enum EUpdateFlags {
    // Used to indicate internally in a view or among sibling views that the
    // components selected by a view have changed.
    fSelectionChanged    = 0x0001,

    // Used to indicate to a set of views bound to a project that the data
    // represented by the project has changed.  This can be used to segregate
    // potentially expensive operations our from other events.
    fProjectChanged      = 0x0002,

    // Used to indicate to all projects and views that a project has been
    // destroyed.  For this event, projects and views could verify that all
    // selections refer to active projects.
    fProjectReleased     = 0x0004,

    // Used to indicate to all projects and views that a new project has been
    // created.
    fProjectCreated      = 0x0008,

    // Used to indicated among a set of sibling views that a view has changed
    // its view focus.
    fViewChanged         = 0x0010,

    // Used to communicate that a view has been deleted
    fViewReleased        = 0x0020,

    // Used to indicate that a new view exists.
    fViewCreated         = 0x0040,

    // Used to communicate that a viewer has been deleted
    fViewerReleased      = 0x0080,

    // Used to indicate that a new viewer exists.
    fViewerCreated       = 0x0040,

    // Used to indicate that a view's visible range has changed
    fVisibleRangeChanged = 0x0100,

    // Used to notify CDocMgrView that it is attached to CDocManager
    fAttached            = 0x0200,

    // Used to notify CDocMgrView that it is detached to CDocManager
    fDetached            = 0x0400,

    // Binary mask for all events
    fAllEvents           = 0xffffffff
};
typedef unsigned int TUpdateFlags;    // bitwise OR of "EUpdateFlags"



END_NCBI_SCOPE

#endif  /* GUI___TYPES__HPP */
