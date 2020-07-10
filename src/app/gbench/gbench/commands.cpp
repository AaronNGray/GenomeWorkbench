/*  $Id: commands.cpp 44428 2019-12-17 17:49:17Z katargir $
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

#include "commands.hpp"

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/fileartprov.hpp>

#include <wx/wx.h>


BEGIN_NCBI_SCOPE


void Gbench_RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
    /// Standard wxWidgets commands that have non-standard attributes
    cmd_reg.RegisterCommand(wxID_OPEN, "&Open...\tCtrl+O", "Open...",
                            "menu::open", "Opens Project, or loads data");
    cmd_reg.RegisterCommand(wxID_NEW, "&New...", "New...",
                             "", "Creates a new Workspace or Project");
    cmd_reg.RegisterCommand(wxID_SAVE, "&Save Projects...\tCtrl+S", "Save Projects...",
                             "menu::save", "Saves Projects");
    cmd_reg.RegisterCommand(wxID_SAVEAS, "Save Projects &As...", "Save Projects As...",
                             "menu::save_as", "");

    cmd_reg.RegisterCommand(eCmdShowProjectTreeView, "&Project View", "Project View",
                            "", "Show Project View");
    cmd_reg.RegisterCommand(eCmdShowSearchView, "&Search View", "Search View",
                            "menu::search_view", "Shows Search View");
    cmd_reg.RegisterCommand(eCmdShowSelectionInspector, "Active Objects &Inspector", "Active Objects Inspector",
                            "", "Shows Active Objects Inspector");
    cmd_reg.RegisterCommand(eCmdShowTaskView, "&Task View", "Task View",
                            "menu::task_view", "Shows Task View");
    cmd_reg.RegisterCommand(eCmdShowEventView, "&Event View", "Event View",
                            "", "Shows Event View");
    cmd_reg.RegisterCommand(eCmdShowDiagConsole, "&Diagnostic Console", "Diagnostic Console",
                            "", "Shows Diagnostic Console");

    cmd_reg.RegisterCommand(eCmdSaveLayout, "S&ave Layout", "Save Layout",
                            "", "Saves window layout");
    cmd_reg.RegisterCommand(eCmdLoadLayout, "&Load Layout", "Load Layout",
                            "", "Loads window layout");
    cmd_reg.RegisterCommand(eCmdResetLayout, "Reset &Layout", "Reset Layout",
                            "", "Resets window layout");

    cmd_reg.RegisterCommand(eCmdRunTool, "Run &Tool...\tCtrl+T", "Run Tool...",
                            "menu::tool", "Shows a dialog with a list of available tools");
    cmd_reg.RegisterCommand(eCmdSearch, "&Search...", "Search...", "menu::search", "");

    cmd_reg.RegisterCommand(eCmdToolsOptions, "&Options", "Options",
                            "", "Options");

    cmd_reg.RegisterCommand(eCmdToolsPackages, "&Packages...", "Packages",
                            "", "Packages");

    cmd_reg.RegisterCommand(eCmdImportClipboard, "Import data from Clipboard", "Import data from Clipboard",
                            "menu::cb_open", "Imports data from Clipboard");

    cmd_reg.RegisterCommand(eCmdEnableEditingPackage, "Enable Advanced Editing", "Enable Editing Package",
        "", "Enables Editing Package");

        // menu Help TODO - move to Help Service
    #if defined( __WXMAC__ )
    cmd_reg.RegisterCommand(eCmdAbout, "&About Genome Workbench", "About Genome Workbench",
                            "", "About Genome Workbench");
    #else
    cmd_reg.RegisterCommand(wxID_ABOUT, "&About Genome Workbench", "About Genome Workbench",
                            "", "About Genome Workbench");
    #endif

    // Help commands
    cmd_reg.RegisterCommand(eCmdSendFeedback, "&Send feedback...", "Send feedback...",
        "", "Opens wizard to send feedback");

    cmd_reg.RegisterCommand(eCmdCheckForUpdates, "&Check for updates...", "Check for updates...",
        "", "Checks NCBI server for application updates");

    cmd_reg.RegisterCommand(eCmdShowPagerMessage, "&View Genome Workbench mesasage...", "View Genome Workbench mesasage...",
        "", "Dispaly message from Genome Workbench");

    cmd_reg.RegisterCommand(
        eCmdTestLoadBlastMask, "Test Load Blast Mask", "Test Load Blast Mask", "", ""
    );
    cmd_reg.RegisterCommand(eCmdMacroEditDlg, "Run Macro Dialog", "Run Macro Dialog",
                            "", "");

    cmd_reg.RegisterCommand(eCmdDebugAsyncCall, "Run/Stop Debug AsyncCall", "Run/Stop Debug AsyncCall",
                            "", "");

    cmd_reg.RegisterCommand(eCmdLogTestException, "Log test exception", "Log test exception",
        "", "");


    /*cmd_reg.RegisterCommand(, "", "",
                            "", "");
    */

    /// register menu images
    provider.RegisterFileAlias(wxT("menu::open"), wxT("file_open.png"));
    provider.RegisterFileAlias(wxT("menu::cb_open"), wxT("cb_open.png"));
    provider.RegisterFileAlias(wxT("menu::save"), wxT("file_save.png"));
    provider.RegisterFileAlias(wxT("menu::save_as"), wxT("file_save_as.png"));
    provider.RegisterFileAlias(wxT("menu::search"), wxT("search.png"));
    provider.RegisterFileAlias(wxT("menu::tool"), wxT("tool.png"));
    provider.RegisterFileAlias(wxT("menu::broadcast_options"), wxT("broadcast_options.png"));
    //provider.RegisterFileAlias(wxT("menu::winmask_files_dnld"), wxT("winmask_files_dnld.png"));

    provider.RegisterFileAlias(wxT("menu::search_view"), wxT("search.png"));
    provider.RegisterFileAlias(wxT("menu::task_view"), wxT("task_view.png"));

}


END_NCBI_SCOPE
