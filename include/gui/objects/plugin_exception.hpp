#ifndef GUI_CORE___PLUGIN_EXCEPTION__HPP
#define GUI_CORE___PLUGIN_EXCEPTION__HPP

/*  $Id: plugin_exception.hpp 14666 2007-07-09 13:40:22Z dicuccio $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *    CPluginException -- defines exceptions produced by plugins
 */

#include <corelib/ncbiexpt.hpp>

BEGIN_NCBI_SCOPE


class CPluginException : EXCEPTION_VIRTUAL_BASE public CException
{
public:
    // Enumerated list of document management errors
    enum EErrCode {
        eNotSupported,
        eDefaultArgNotSupported,
        eInvalidArg,
        eInvalidCommand,
        eInvalidHandle,
        eInvalidMessage,
        eMissingRequiredArg,
        ePluginLoadFailed,
        eVersionMismatch,
        eUnknownError
    };

    // Translate the specific error code into a string representations of
    // that error code.
    virtual const char* GetErrCodeString(void) const
    {
        switch (GetErrCode()) {
        case eNotSupported:             return "eNotSupported";
        case eDefaultArgNotSupported:   return "eDefaultArgNotSupported";
        case eInvalidArg:               return "eInvalidArg";
        case eInvalidCommand:           return "eInvalidCommand";
        case eInvalidHandle:            return "eInvalidHandle";
        case eInvalidMessage:           return "eInvalidMessage";
        case eMissingRequiredArg:       return "eMissingRequiredArg";
        case ePluginLoadFailed:         return "ePluginLoadFailed";
        case eVersionMismatch:          return "eVersionMismatch";
        case eUnknownError:             return "eUnknownError";
        default:                        return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT(CPluginException, CException);
};


END_NCBI_SCOPE

#endif  // GUI_CORE___PLUGIN_EXCEPTION__HPP
