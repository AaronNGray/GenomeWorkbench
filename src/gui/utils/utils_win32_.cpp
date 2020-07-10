/*  $Id: utils_win32_.cpp 39094 2017-07-27 19:08:09Z katargir $
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
 * Authors:  Josh Cherry
 *
 * File Description:  Wrappers for interacting with Windows registry
 *
 */

#include <ncbi_pch.hpp>
#include <gui/utils/utils_win32_.hpp>

#ifndef NCBI_OS_MSWIN
#  error "This file should be compiled on Windows alone."
#endif

#include <windows.h>
#include <winreg.h>

#include <corelib/ncbistr.hpp>

BEGIN_NCBI_SCOPE

// convert from enumeration for HKEY to Windows HKEY
static HKEY s_GetRootKey(EHkey root_key)
{
    switch (root_key) {
    case eHkey_classes_root:
        return HKEY_CLASSES_ROOT;
    case eHkey_current_user:
        return HKEY_CURRENT_USER;
    case eHkey_local_machine:
        return HKEY_LOCAL_MACHINE;
    case eHkey_users:
        return HKEY_USERS;
    case eHkey_performance_data:
        return HKEY_PERFORMANCE_DATA;
    default:
        throw runtime_error("Invalid root key");
    }
}

/// Set a Windows registry variable to a string value
void SetWinRegVariable(EHkey root_key, const string& subkey,
    const string& varname, const string& value)
{
    HKEY open_key;

    if (RegCreateKey(s_GetRootKey(root_key), _T_XCSTRING(subkey), &open_key)
        != ERROR_SUCCESS) {
        throw runtime_error("Error setting registry variable " + varname
            + " in section " + subkey);
    }

    TXString val = _T_XCSTRING(value);
    DWORD cbData = (val.length() + 1)*sizeof(TXString::value_type);

    if (RegSetValueEx(open_key, _T_XCSTRING(varname), 0, REG_SZ, (const BYTE*)val.c_str(), cbData)
        != ERROR_SUCCESS) {
        throw runtime_error("Error setting registry variable " + varname
            + " in section " + subkey);
    }

    RegCloseKey(open_key);
}

END_NCBI_SCOPE
