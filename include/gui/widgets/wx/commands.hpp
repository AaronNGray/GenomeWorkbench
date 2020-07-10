#ifndef GUI_WIDGETS_WX___COMMANDS__HPP
#define GUI_WIDGETS_WX___COMMANDS__HPP

/*  $Id: commands.hpp 18039 2008-10-08 18:11:48Z yazhuk $
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


/// Register standard commands defined in this file and wxWidgets headers.
/// Call this function during application start-up to make the command
/// definitions available.
NCBI_GUIWIDGETS_WX_EXPORT
    void WidgetsWx_RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

///////////////////////////////////////////////////////////////////////////////
/// EBasicCommands defines standard commands that can be used in most applications.
///
/// Before defining a new command, please see wxWidgets standard commands such as
/// wxID_CUT,wxID_COPY, wxID_OPEN etc.


enum EBasicCommands   {
    //TODO - these commands should be defined in Help Service
    /// help menu commands
    eCmdHelpFaq,
    eCmdHelpAbout,
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___COMMANDS__HPP
