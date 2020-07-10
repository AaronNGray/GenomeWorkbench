#ifndef GUI_WIDGETS___DATA___DATA_COMMANDS__HPP
#define GUI_WIDGETS___DATA___DATA_COMMANDS__HPP

/*  $Id: data_commands.hpp 17877 2008-09-24 13:03:09Z dicuccio $
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

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>

class wxFileArtProvider;

BEGIN_NCBI_SCOPE

class CUICommandRegistry;

/// Register standard commands defined in this file and wxWidgets headers.
/// Call this function during application start-up to make the command
/// definitions available.
NCBI_GUIWIDGETS_DATA_EXPORT
    void WidgetsData_RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

enum EAlnColoringCommands   {
    eCmdAlnShowMethodsDlg = 8100,
    eCmdMethodProperties,
    eCmdDisableScoring,
    eCmdSetAsDefaultMethod,

    eCmdScoringMethodXXXX = 8150,
    eCmdScoringMethodLast = 8999
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS___DATA___DATA_COMMANDS__HPP
