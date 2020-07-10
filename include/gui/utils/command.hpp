#ifndef GUI_UTILS___COMMAND__HPP
#define GUI_UTILS___COMMAND__HPP

/*  $Id: command.hpp 43842 2019-09-09 21:57:24Z evgeniev $
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
 */

/** @addtogroup GUI_UTILS
*
* @{
*/

/// @file command.hpp
/// GUI command routing and handling framework.
///
/// The following are parts of the framework:
/// CCommandTarget class - represents a class capable of handling commands
/// using command maps.
/// Command map - structure mapping command IDs to handler funtion pointers.
/// IMenu - interface for CCommandTraget compatible menus.

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>

#include <gui/gui.hpp>

#include <gui/utils/event_handler.hpp>


BEGIN_NCBI_SCOPE

/// Definitions for generic commands.
///
/// Subsytems of GUI should define they own command enumerations, preferable
/// with non-overlapping ranges.

enum EBaseCommands   {
    eCmdMenuEnd = -2, /// marks menu end in array initializers
    eCmdInvalid = -1, /// not a valid command
    eCmdNone = 0, /// empty command

    /// zoom events
    eCmdZoomIn,
    eCmdZoomOut,
    eCmdZoomAll,
    eCmdZoomInMouse,
    eCmdZoomOutMouse,
    eCmdZoomSel, /// Zoom to Selection
    eCmdZoomSeq, /// Zoom to Sequence
    eCmdZoomObjects,
    eCmdZoomSelObjects,
    eCmdZoomInX,
    eCmdZoomOutX,
    eCmdZoomAllX,
    eCmdZoomInY,
    eCmdZoomOutY,
    eCmdZoomAllY,
    eCmdSetEqualScale,

    eCmdSettings, // widget/view settings
    eCmdConfigureTracks,

    // Web help commands
    eCmdHelpStart,
    eCmdWebHelpApp = eCmdHelpStart,
    eCmdSubmissionsWebHelp,
    eCmdWebHelpGSVNavigation,
    eCmdWebHelpGSVInteractionGuide,
    eCmdWebHelpGSVLegends,
    eCmdHelpEnd = eCmdWebHelpGSVLegends,

    eCmdResetSelection,

    /// Navigation commands
    eCmdBack,
    eCmdForward,

    /// goto sequence position
    eCmdSeqGoto,

    eCmdSaveImages,
    eCmdSavePdf,
    eCmdSaveSvg,

    // insert new command here
    eBaseCmdLast
};

/// Commands needed to mange sticky tool tip windows
enum EStickyToolTipCommands   {
    eCmdFirstStickyToolTipCmd = 8000,
    eCmdParentMove,          /// tool tip window parent moves
    eCmdSuspend,             /// tool tip parent window about to dock
    eCmdReCreate,            /// tool tip parent window finished docking
    eCmdDockMainWindow,      /// tool tip cmd indicating a main-window docking update
    eCmdParentActivate,      /// tool tip parent window activated
    eCmdParentShow,          /// tool tip parent window shown
    eCmdParentHide,          /// tool tip parent window hidden
    eCmdPinTip,              /// tool tip pinned or unpinned
    eCmdSearchTip,           /// search (scroll) window to tip element
    eCmdZoomTip,             /// zoom into tip element
    eCmdInfoTip,             /// tip info button
    eCmdClickTip,            /// User clicked somewhere on a tip
    eCmdReleaseTip,          /// Left mouse (or equiv) up inside tip area
    eCmdMoveTip,             /// User is interactively moving a tool tip
    eCmdTipActive,           /// Mouse entered a tip (highlight matching glyph)
    eCmdTipInactive,         /// Mouse left a tip (remove glyph highlight)
    eCmdCopyTipText,         /// User selected option to copy (all) tip text
    eCmdSelectTipText,       /// User selected option to make text selectable
    eCmdCopySelectedTipText, /// Copy currently selected tip text to clipboard
    eCmdTipAdded,            /// Event fired when a new tip added
    eCmdTipRemoved,          /// Event fired when an existing tip is removed
    eCmdLastStickyToolTipCmd
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___COMMAND__HPP
