#ifndef __APP_SVIEWER___RMTFILESTATUS___HPP
#define __APP_SVIEWER___RMTFILESTATUS___HPP

/*  $Id: rmtfilestatus.hpp 41935 2018-11-15 18:19:14Z rudnev $
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
 * Authors:  Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <time.h>
#include <connect/services/neticache_client.hpp>

#include <gui/gui.hpp>

BEGIN_NCBI_SCOPE

// this class is a black box that instructs the caller whether to try to access a remote file
// if access is done, it wants to know whether the access was successful or not
class NCBI_GUIUTILS_EXPORT CRmtFileStatus {
public: 
    enum ESuggestedAction {
        ESuggestedAction_Access,    ///< try to access the remote file
        ESuggestedAction_Skip       ///< do not access the file
    };

    // gives suggested course of action for a given file
    static ESuggestedAction Check(const string& sKey);

    // this must be called when accessibility of the file is known
    static void Set(const string& sKey, ESuggestedAction KnownAccessibility);

private:
    typedef size_t TCounter;

    typedef time_t TTimeStamp;  ///< in seconds
    static unique_ptr<CNetICacheClient> m_pCacheClient;
    static CNetICacheClient* x_Init();
    static bool x_Read(CNetICacheClient* cache, const string& sNCKey, int version, const string& sSubKey, void* data, size_t data_size);
    static void x_ResetSkipLimits(const string& sNCKey);  

    // convert any string to a key that can be an NC blob key
    static string x_KeyToNCKey(const string& sKey);
    // going over this count will force an "Access"
    static TCounter m_MaxCount;

    // max time intervals for checks
    static TTimeStamp m_MaxCheckInterval;

    // it was identified that cache is not available or not configured so it should not be used
    // in this case, ESuggestedAction_Access will always be returned
    static bool m_isStatusNotAvailable;
};

// get the timeout in seconds to run the remote pipleine scripts (rmt_*.sh)
class NCBI_GUIUTILS_EXPORT CRmtScriptTimeout {
public:
    static time_t Get();

private:
    static time_t m_Timeout;
};


END_NCBI_SCOPE

#endif
