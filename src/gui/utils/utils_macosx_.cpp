/*  $Id: utils_macosx_.cpp 19811 2009-08-07 18:01:45Z tereshko $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>

#ifndef NCBI_OS_DARWIN
#  error "This fine should be compiled on MacOS alone."
#endif


#ifdef NCBI_COMPILER_METROWERKS
#define __NOEXTENSIONS__
#endif
#include <Carbon/Carbon.h>

BEGIN_NCBI_SCOPE


//
// Call AppleScript engine
/*
 * TODO: If we still need this, we should move it to a higher-level lib which will be linked to Carbon framework
 *
bool CMacOSX::ExecuteAppleScript(string script, string& result)
{
    char buf[1024];
    bool success = false;

    AEDesc script_desc, result_desc, error_desc;

    AECreateDesc (typeChar, script.c_str(), script.length(), &script_desc);
    ComponentInstance osaAppleScript = OpenDefaultComponent (kOSAComponentType,
                                        kAppleScriptSubtype);
    OSAError osaerror = OSADoScript (osaAppleScript, &script_desc,
                                        kOSANullScript, typeChar,
                                        kOSAModeNull, &result_desc);
    AEDisposeDesc (&script_desc);

    int msg_length = 0;
    if (osaerror == errOSAScriptError) {
        OSAScriptError (osaAppleScript, kOSAErrorMessage, typeChar, &error_desc);
        msg_length = AEGetDescDataSize(&error_desc);
        AEGetDescData(&error_desc, buf, msg_length);
        AEDisposeDesc (&error_desc);
    } else  if (osaerror == noErr) {
        success = true;
        msg_length = AEGetDescDataSize(&result_desc);
        AEGetDescData(&result_desc, buf, msg_length);
        AEDisposeDesc (&result_desc);
    }

    *(buf + msg_length) = '\0';
    result = buf;

    CloseComponent (osaAppleScript);
    return success;
}
*/

END_NCBI_SCOPE
