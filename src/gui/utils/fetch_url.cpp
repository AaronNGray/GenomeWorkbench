/*  $Id: fetch_url.cpp 34957 2016-03-04 17:40:23Z evgeniev $
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
 * File Description:  fetch data from the net using http
 *
 */


#include <ncbi_pch.hpp>
#include <gui/utils/fetch_url.hpp>

#include <corelib/ncbimtx.hpp>
#include <corelib/ncbi_system.hpp>

#include <connect/ncbi_conn_stream.hpp>
#include <connect/ncbi_core_cxx.hpp>
#include <connect/ncbi_util.h>
#include <util/checksum.hpp>

BEGIN_NCBI_SCOPE

/// URL fetch cache (MT safe)
/// @internal
class CURLFetchCache
{
public:
    CURLFetchCache(){}
    bool Get(const string& key, string& result) const
    {
        CFastMutexGuard guard(m_Lock);
        map<string, string> ::const_iterator it = m_Map.find(key);
        if (it == m_Map.end()) {
            return false;
        }
        result = it->second;
        return true;
    }

    void Set(const string& key, const string& result)
    {
        CFastMutexGuard guard(m_Lock);

        if (m_Map.size() >= 1000) {  // limiter not to overflow memory
            m_Map.clear();
        }

        m_Map[key] = result;
    }
private:
    mutable CFastMutex           m_Lock;
    map<string, string>  m_Map;
};

static CURLFetchCache s_url_cache;

void CFetchURL::Fetch(const string& url,
                      string& result,
                      const STimeout* timeout,
                      bool is_cache)
{
    // check if it is already cached
    //
    unsigned char digest[16];
    string md5_str;

    if (is_cache) {
        CChecksum cs(CChecksum::eMD5);
        cs.AddLine(url);
        cs.GetMD5Digest(digest);

        md5_str.append((char*)&(digest[0]), sizeof(digest));
        if (s_url_cache.Get(md5_str, result)) {
            if (!result.empty()) {
                return;
            }
        }
    }


    // pretend to be a 'real' web browser; some servers
    // care about this
    string header =
        "User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; "
        "Windows NT 5.0; (R1 1.1))";

    CConn_HttpStream http(url, NULL, header, 0 /* parse_header */,
                          NULL /* user_data */, 0 /* adjust */, 
                          0 /* cleanup */, fHTTP_AutoReconnect, timeout);

    result.erase();
    char buf[1024];
    while (!http.fail()) {
        http.read(buf, 1024);
        result.append(buf, http.gcount());
    }

    if (is_cache && !result.empty()) {
        s_url_cache.Set(md5_str, result);
    }
}

END_NCBI_SCOPE
