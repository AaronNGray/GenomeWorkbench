/*  $Id: commands.cpp 43842 2019-09-09 21:57:24Z evgeniev $
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

#include <gui/widgets/wx/commands.hpp>
#include <gui/widgets/wx/iwindow_manager.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/dock_panel.hpp>

#include <wx/defs.h>
#include <wx/accel.h>

BEGIN_NCBI_SCOPE


void WidgetsWx_RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
    // Register standard wxWidgets commands ///////////////////////////////////

    //cmd_reg.RegisterCommand(wxID_, "", "", "", "");

    CUICommand* cmd = 0;

    // File
    cmd_reg.RegisterCommand(wxID_PRINT, "&Print...", "Print", "menu::print", "Print current view"); 
    cmd_reg.RegisterCommand(wxID_EXIT, "E&xit\tAlt+X", "Exit", "", "Exit application");

    // Edit
    cmd = new CUICommand(wxID_UNDO, "&Undo\tCtrl+Z", "Undo", "menu::undo", "Undo the last editing command");
    cmd->AddAccelerator(wxACCEL_CTRL, 'Z');
    cmd_reg.RegisterCommand(cmd);

    cmd = new CUICommand(wxID_REDO, "&Redo\tCtrl+Y", "Redo", "menu::redo", "Redo the last editing command");
    cmd->AddAccelerator(wxACCEL_CTRL, 'Y');
    cmd_reg.RegisterCommand(cmd);

    cmd_reg.RegisterCommand(wxID_REMOVE, "&Remove", "Remove", "", "Remove selected items");

    // On mac the 'delete' key works as the backspace key, and this is the usual key
    // for deleting items such as deleting a file from the list.  
#ifdef __WXMAC__
    cmd_reg.RegisterCommand(wxID_DELETE, "&Delete\tCtrl+Back", "Delete", "menu::delete", "Delete selected items");
#else
    cmd_reg.RegisterCommand(wxID_DELETE, "&Delete\tCtrl+Del", "Delete", "menu::delete", "Delete selected items");
#endif

    cmd_reg.RegisterCommand(wxID_FIND, "&Find...\tCtrl+F", "Find...", "", "Find");

    cmd = new CUICommand(wxID_SELECTALL, "Select A&ll\tCtrl+A", "Select All", "", "Select All");
    cmd->AddAccelerator(wxACCEL_CTRL, 'A');
    cmd_reg.RegisterCommand(cmd);

    cmd_reg.RegisterCommand(wxID_CLEAR, "Cle&ar", "Clear", "", "Clear selection");

    // Clipboard command
    cmd = new CUICommand(wxID_CUT, "Cu&t\tCtrl+X", "Cut", "menu::cut", "Cut selection to Clipboard");
    cmd->AddAccelerator(wxACCEL_SHIFT, WXK_DELETE);
    cmd_reg.RegisterCommand(cmd);

    cmd = new CUICommand(wxID_COPY, "&Copy\tCtrl+C", "Copy", "menu::copy", "Copy selection to Clipboard");
    cmd->AddAccelerator(wxACCEL_CTRL, WXK_INSERT);
    cmd->AddAccelerator(wxACCEL_CTRL, 'C');
    cmd_reg.RegisterCommand(cmd);

    cmd = new CUICommand(wxID_PASTE, "&Paste\tCtrl+V", "Paste", "menu::paste", "Paste from Clipboard");
    cmd->AddAccelerator(wxACCEL_SHIFT, WXK_INSERT);
    cmd_reg.RegisterCommand(cmd);

    cmd_reg.RegisterCommand(wxID_PROPERTIES, "&Properties...", "Properties...",
                            "menu::properties", "Show Properties dialog");

    // Help
    cmd_reg.RegisterCommand(wxID_HELP_SEARCH, "&Search", "Search", "", "Search in Help");
    cmd_reg.RegisterCommand(wxID_HELP_CONTENTS, "&Contents", "Contents", "", "Show Help contents");
    cmd_reg.RegisterCommand(wxID_HELP_INDEX, "&Index", "Index", "", "Show Help Index");

    // Register standard GUI NCBI commands /////////////////////////////////////////////////
    cmd_reg.RegisterCommand(eCmdZoomIn, "Zoom In\tCtrl++", "Zoom In", "menu::zoom_in", "Zoom In");
    cmd_reg.RegisterCommand(eCmdZoomOut, "Zoom Out\tCtrl+-", "Zoom Out", "menu::zoom_out", "Zoom Out");

    cmd = new CUICommand(eCmdZoomAll, "Zoom All", "Zoom All", "menu::zoom_all", "Zoom All");
    cmd_reg.RegisterCommand(cmd);
    
    cmd_reg.RegisterCommand(eCmdZoomSel, "Zoom to Selection", "Zoom to Selection",
                            "menu::zoom_sel", "Zoom to Selection");
    cmd_reg.RegisterCommand(eCmdZoomSelObjects, "Zoom to Selected Objects", "Zoom to Selected Objects",
                            "", "Zoom to Selected Objects");
    cmd_reg.RegisterCommand(eCmdZoomSeq, "Zoom to Sequence", "Zoom to Sequence",
                            "menu::zoom_seq", "Zoom to Sequence");
    cmd_reg.RegisterCommand(eCmdZoomObjects, "Zoom to Data", "Zoom to Data",
                            "", "Adjust views so that all data objects are visible");

    cmd_reg.RegisterCommand(eCmdZoomInX, "Zoom In X", "Zoom In X", "", "Zoom In horizontally");
    cmd_reg.RegisterCommand(eCmdZoomOutX, "Zoom Out X", "Zoom Out X", "", "Zoom Out horizontally");
    cmd_reg.RegisterCommand(eCmdZoomAllX, "Zoom All X", "Zoom All X", "", "Zoom All horizontally");

    cmd_reg.RegisterCommand(eCmdZoomInY, "Zoom In Y", "Zoom In Y", "", "Zoom In vertically");
    cmd_reg.RegisterCommand(eCmdZoomOutY, "Zoom Out Y", "Zoom Out Y", "", "Zoom Out vertically");
    cmd_reg.RegisterCommand(eCmdZoomAllY, "Zoom All Y", "Zoom All Y", "", "Zoom All vertically");

    // command ids for using in context menus only
    cmd_reg.RegisterCommand(eCmdZoomInMouse, "Zoom In", "Zoom In", "menu::zoom_in", "Zoom In");
    cmd_reg.RegisterCommand(eCmdZoomOutMouse, "Zoom Out", "Zoom Out", "menu::zoom_out", "Zoom Out");

    cmd_reg.RegisterCommand(eCmdSetEqualScale, "Make Proportional", "Make Proportional",
                    "", "Adjust view so that horizontal and vertical scales are sequal");

    cmd_reg.RegisterCommand(eCmdSettings, "Settings...", "Settings", "", "Shows \"Settings\" dialog");
    cmd_reg.RegisterCommand(eCmdConfigureTracks, "Tracks...", "Tracks...", "menu::configure", "Shows \"Configure Tracks\" dialog");

    cmd_reg.RegisterCommand(eCmdWebHelpApp, "All Help Topics", "All Help Topics", "", "Launch Application Help");
    cmd_reg.RegisterCommand(eCmdSubmissionsWebHelp, "NCBI GenBank Submissions", "NCBI GenBank Submissions", "", "Launch NCBI GenBank Submissions Help");
    cmd_reg.RegisterCommand(eCmdWebHelpGSVNavigation, "Graphical View Navigation", "Graphical View Navigation", "", "Launch Graphical View Navigation Help");
    cmd_reg.RegisterCommand(eCmdWebHelpGSVInteractionGuide, "Interaction Guide", "Interaction Guide", "", "Launch Interaction Guide Help");
    cmd_reg.RegisterCommand(eCmdWebHelpGSVLegends, "Graphical Legends", "Graphical Legends", "", "Launch Graphical Legends Help");

    cmd_reg.RegisterCommand(eCmdResetSelection, "Reset Selection", "Reset Selection",
                            "", "Resets current selection");

    /// Register Mavigation commands
    cmd_reg.RegisterCommand(eCmdBack, "Back", "Back", "menu::back", "Go Back");
    cmd_reg.RegisterCommand(eCmdForward, "Forward", "Forward", "menu::forward", "Go Forward");

    cmd_reg.RegisterCommand(eCmdSeqGoto,
        "Go to Sequence Position/Range...", "Go to Sequence Position or Range", "", "");

    /// Window Manager commands
    cmd_reg.RegisterCommand(eCmdSetClientColor, "Change Color...", "Change Color", "", "");
    cmd_reg.RegisterCommand(eCmdShowWindowsDlg, "&Windows...", "View List Dialog", "menu::viewer_item", "Shows View List Dialog");

    cmd_reg.RegisterCommand(eCmdCloseDockPanel, "Close", "Close", "menu::win_close", "");
    cmd_reg.RegisterCommand(eCmdMoveToMainTab, "&Reset", "Reset", "", "");
    cmd_reg.RegisterCommand(eCmdWindowFloat, "&Float", "Float", "", "");
    cmd_reg.RegisterCommand(eCmdWindowMinimize, "&Minimize", "Minimize", "menu::win_minimize", "");
    cmd_reg.RegisterCommand(eCmdWindowRestore, "&Restore", "Restore", "menu::win_restore", "");

    cmd_reg.RegisterCommand(eCmdSaveImages, "Save Images...", "Save Images...",
                              "", "Saves the current view as one or more images");
    cmd_reg.RegisterCommand(eCmdSavePdf, "Save As PDF...", "Save As PDF...", "", "Saves the current view as a PDF file");
    cmd_reg.RegisterCommand(eCmdSaveSvg, "Save As SVG...", "Save As SVG...", "", "Saves the current view as a Scalable Vector Graphics (.SVG) file");

    provider.RegisterFileAlias(wxT("menu::print"), wxT("print.png"));

    provider.RegisterFileAlias(wxT("menu::zoom_in"), wxT("zoom_in.png"));
    provider.RegisterFileAlias(wxT("menu::zoom_out"), wxT("zoom_out.png"));
    provider.RegisterFileAlias(wxT("menu::zoom_all"), wxT("zoom_all.png"));
    provider.RegisterFileAlias(wxT("menu::zoom_seq"), wxT("zoom_seq.png"));
    provider.RegisterFileAlias(wxT("menu::zoom_sel"), wxT("zoom_sel.png"));

    provider.RegisterFileAlias(wxT("menu::configure"), wxT("configure.png"));

    provider.RegisterFileAlias(wxT("menu::undo"), wxT("undo.png"));
    provider.RegisterFileAlias(wxT("menu::redo"), wxT("redo.png"));

    provider.RegisterFileAlias(wxT("menu::cut"), wxT("cut.png"));
    provider.RegisterFileAlias(wxT("menu::copy"), wxT("copy.png"));
    provider.RegisterFileAlias(wxT("menu::paste"), wxT("paste.png"));
    provider.RegisterFileAlias(wxT("menu::properties"), wxT("properties.png"));

    provider.RegisterFileAlias(wxT("menu::back"), wxT("back.png"));
    provider.RegisterFileAlias(wxT("menu::forward"), wxT("forward.png"));

    provider.RegisterFileAlias(wxT("menu::win_minimize"), wxT("wm_menu_minimize.png"));
    provider.RegisterFileAlias(wxT("menu::win_restore"), wxT("wm_menu_restore.png"));
    provider.RegisterFileAlias(wxT("menu::win_close"), wxT("wm_menu_close.png"));

    provider.RegisterFileAlias(wxT("menu::viewer_item"), wxT("viewer_item.png"));
}


END_NCBI_SCOPE
