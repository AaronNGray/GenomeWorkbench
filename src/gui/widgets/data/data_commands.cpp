/*  $Id: data_commands.cpp 37492 2017-01-13 21:51:26Z shkeda $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/data/data_commands.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/fileartprov.hpp>


BEGIN_NCBI_SCOPE


void WidgetsData_RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
    bool static initialized = false;
    if( initialized)
        return;
    initialized = true;

    cmd_reg.RegisterCommand(eCmdAlnShowMethodsDlg, "Select Method...", "Select Coloring Method", "", "");
    cmd_reg.RegisterCommand(eCmdMethodProperties, "Method Properties...", "Method Properties", "", "");
    cmd_reg.RegisterCommand(eCmdDisableScoring, "Disable Coloring",
        "Disable Coloring", "", "", "", "", wxITEM_CHECK);
    cmd_reg.RegisterCommand(eCmdSetAsDefaultMethod, "Set as Default Method",
        "Set as Default Method", "", "", "", "", wxITEM_CHECK);
}

END_NCBI_SCOPE
