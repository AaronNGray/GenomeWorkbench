/*  $Id: commands.cpp 39528 2017-10-05 15:27:37Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/core/commands.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/fileartprov.hpp>

BEGIN_NCBI_SCOPE


void Core_RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
    /*cmd_reg.RegisterCommand(, "", "", "", "");
    */

    // View commands
    cmd_reg.RegisterCommand(
        eCmdBroadcastSel, "&Broadcast Selection", "Broadcast Selection",
        "menu::broadcast", "Broadcast selection from the active view"
    );

    cmd_reg.RegisterCommand(
        eCmdBroadcastSelRange, "&Broadcast Ranges", "Broadcast Ranges",
        "menu::broadcast_range", "Broadcast selection as ranges from the active view"
    );


    cmd_reg.RegisterCommand(
		eCmdOpenView, "Open &New View...\tCtrl+N", "Open New View...",
        "", "Show a dialog with a list of available Views"
	);

    cmd_reg.RegisterCommand(
		eCmdOpenViewDefault, "Open &Default View",
        "", "Open default view if exists or show a dialog with a list of available Views"
	);

    // Workspace commands
    cmd_reg.RegisterCommand(eCmdCloseWorkspace, "&Close All Projects", "Close All Projects", "", "");
    cmd_reg.RegisterCommand(eCmdShowHideDisabledItems, "Show / Hide", "Show / Hide",
                            "", "Show / Hide Disabled Project Items");

    // Project commands
    cmd_reg.RegisterCommand(eCmdAddNewProject, "New Project...", "New Project...",
                            "", "Creates a new empty Project in the current Workspace");
    cmd_reg.RegisterCommand(eCmdLoadProject, "&Load Project", "Load Project",
                            "", "Loads Projects that currently are not loaded");
    cmd_reg.RegisterCommand(eCmdUnLoadProject, "&Unload Project", "",
                            "", "Unload Projects that are currently loaded");
    //cmd_reg.RegisterCommand(eCmdAddExistingProject, "&", "", "", "");
    cmd_reg.RegisterCommand(eCmdRemoveProject, "&Remove Project", "Remove Project",
                            "", "Remove Projects from the Workspace");
    cmd_reg.RegisterCommand(eCmdMergeProjects, "&Merge Projects", "Merge Projects",
                            "", "");
    cmd_reg.RegisterCommand(eCmdProjectTableView, "Open &Table View", "",
                            "", "Open Table View On Project");

    // Window commands
    cmd_reg.RegisterCommand(eCmdActivateClients, "Activate", "Activate",
                            "", "Activates selected views");

    cmd_reg.RegisterCommand(eCmdCloseProjectView, "&Close View", "Close View",
                            "", "Closes selected project views");
    cmd_reg.RegisterCommand(eCmdCloseAllProjectViews, "Close &All Views", "Close All Views",
                            "", "Closes all project views");

    cmd_reg.RegisterCommand(eCmdEnableDisable, "Enable / Disable", "Enable / Disable",
                            "", "Enable / Disable Project Item");
    cmd_reg.RegisterCommand(eCmdNewFolder, "New Folder...", "New Folder...",
                            "", "Creates a new folder");

    // Export
    cmd_reg.RegisterCommand(eCmdExportObject, "Export Data...", "Exports selected object(s)", "", "");

    cmd_reg.RegisterCommand(eCmdBroadcastOptions, "&Broadcast Options...", "Broadcast Options",
        "menu::broadcast_options", "Shows a dialog with Selection Broadcasting options");

    cmd_reg.RegisterCommand(eCmdWindowMaskerFiles, "&WindowMasker Data...", "WindowMasker Data",
        "menu::winmask_data_dnld", "Shows a dialog for WindowMasker data files download");

    // Data Mining panel - create feature from search results
    cmd_reg.RegisterCommand(eCmdCreateFeature,
        "&Create New Feature", "Create New Feature", "", "");

	// Register icon aliases
    provider.RegisterFileAlias(wxT("menu::broadcast"), wxT("broadcast.png"));
    provider.RegisterFileAlias(wxT("menu::broadcast_range"), wxT("broadcast_range.png"));
}


END_NCBI_SCOPE
