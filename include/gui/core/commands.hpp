#ifndef GUI_CORE___COMMANDS__HPP
#define GUI_CORE___COMMANDS__HPP

/*  $Id: commands.hpp 39528 2017-10-05 15:27:37Z katargir $
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
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui.hpp>

class wxFileArtProvider;

BEGIN_NCBI_SCOPE

class CUICommandRegistry;

/// Registers commands defined in gui_core library (see below),
/// also registers icons for the commands.
NCBI_GUICORE_EXPORT void Core_RegisterCommands(CUICommandRegistry& cmd_reg,
                                               wxFileArtProvider& provider);

enum ECoreCommands   {
    // View commands
    eCmdBroadcastSel = 9000,
    eCmdBroadcastSelRange,
    eCmdVisibleRangeChangeAllow,
    eCmdVisibleRangeBroadcast,
    eCmdDefaultVRBroadcast,
    eCmdTrackVRBroadcast,
    eCmdTrackCenterVRBroadcast,
    eCmdTrackExpandVRBroadcast,
    eCmdTrackContractVRBroadcast,
    eCmdSlaveVRBroadcast,
    eCmdIgnoreVRBroadcast,

    eCmdObjToRangeSelection,
    eCmdRangeToObjSelection,

    eCmdOpenView,

    // Workspace commands
    eCmdNewWorkspace,
    eCmdCloseWorkspace,
    eCmdShowHideDisabledItems,

    // Project commands
    eCmdAddNewProject,
    eCmdLoadProject,
    eCmdUnLoadProject,
    eCmdRemoveProject,
    eCmdAddExistingProject,
    eCmdMergeProjects,
    eCmdProjectTableView,
    //eCmdCloseProjects,

    // Window commands
    eCmdCloseProjectView,
    eCmdCloseAllProjectViews,
    eCmdActivateClients,

    eCmdExport,
    eCmdShowDMView, // show Data Mining View
    eCmdShowHomePage,

    // Project Tree commands
    eCmdMergeItems,
    eCmdNewFolder,
    eCmdEnableDisable,

    eCmdAddToHomePage,

    // Export
    eCmdExportObject,

    eCmdBroadcastOptions, // broadcast selection
	eCmdWindowMaskerFiles, // window masker files download

    eCmdOpenViewDefault,

    eCmdCreateFeature  // Data Mining panel command

    /*  eCmdConfigureWebBrowser,
    eCmdManagePlugins,

    /// bookmark commands
    eCmdAddBookmark,
    eCmdManageBookmark,

    /// view menu commands
    eCmdVisRangeTrack,
    eCmdVisRangeSlave,
    eCmdVisRangeIgnore,
    eCmdVisRangeTrackExpand,
    eCmdVisRangeTrackContract,
    eCmdVisRangeTrackCenter,

  */    // Workspace Window commands
};

END_NCBI_SCOPE

#endif  // GUI_CORE___COMMANDS__HPP
