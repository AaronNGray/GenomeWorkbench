#ifndef GUI_CORE___ACCEL_TABLE__HPP
#define GUI_CORE___ACCEL_TABLE__HPP

/*  $Id: accel_table.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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

#include <corelib/ncbistd.hpp>

#include <gui/utils/command.hpp>

/** @addtogroup GUI_UTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CAccelTable - Accelerator Table
///
/// CAccelTable maps Accelerators to commands and vice versa. Accelerator is a
/// combination of generic key with modifier keys such as Ctrl, Shift and Alt.
/// Accelerators are encoded as integer valuss using FLTK conventions for shortcuts.
/// In this convention accelerator id is a bitwise combination of the key with bit
/// flags corresponding to modifiers. For instance "Ctrl + C" is encoded as
/// (FL_CTRL + 'C'). Accelerators are case insensetive, all lower case characters
/// are converted to upper case.
/// Mapping of accelerators to commands is "many to one".

class NCBI_GUIUTILS_EXPORT  CAccelTable
{
public:
    static bool     RegisterAccelerator(int accel, TCmdID cmd);

    /// returns in "cmd" command corresponding to the accelerator
    static bool     GetCommandByAccel(int accel, TCmdID& cmd);

    /// returns in "accel" accelerator corresponding to the command, if multiple
    /// accelerators are mapped to this command the first one is returned
    static bool     GetAccelByCommand(TCmdID cmd, int& accel);

    /// generates text label describing accelerator such as "Ctrl+C"
    static string   GetAccelLabel(int accel);

    /// registers platform-specific accelerators for standard commands such as
    /// "Copy", "Paste"
    static void     RegisterStdAccelerators();

protected:
    typedef map<int, TCmdID>    TAccelToCmdMap;
    typedef map<TCmdID, int>    TCmdToAccelMap;

    static TAccelToCmdMap  sm_AccelToCmd;
    static TCmdToAccelMap  sm_CmdToAccel;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_CORE___ACCEL_TABLE__HPP
