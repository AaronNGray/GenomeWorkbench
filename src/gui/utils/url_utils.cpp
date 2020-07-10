/*  
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
 * Authors:  Andrei Shkeda
 *
 * File Description:
 *    URL Utilities
 */


#include <ncbi_pch.hpp>
#include <gui/utils/url_utils.hpp>

#include <connect/ncbi_conn_stream.hpp>
#include <util/md5.hpp>

BEGIN_NCBI_SCOPE

static 
void s_GetFtpHostAndFile(const string& url, string& host, string& file)
{
    size_t start_pos = string::npos;
    if (NStr::StartsWith(url, "ftp://")) {
        start_pos = 6;
    } else if (NStr::StartsWith(url, "ftp.")) {
        start_pos = 0;
    }

    if (start_pos != string::npos) {
        size_t pos = url.find('/', start_pos);
        if (pos != string::npos) {
            host = url.substr(start_pos, pos - start_pos);
            file = url.substr(pos + 1);
        }
    }
}

void CUrlUtils::GetLastModified(const string& url, string& last_modified)
{
    last_modified.clear();
    string err_msg = "Specified URL is not valid: '";
    err_msg += url;
    err_msg += "'";
    if (NStr::StartsWith(url, "ftp:", NStr::eNocase)) {
        string ftp_host, ftp_file;
        s_GetFtpHostAndFile(url, ftp_host, ftp_file);
        CConn_FTPDownloadStream ftp(ftp_host, ftp_file);
        if (!ftp.good())
            NCBI_THROW(CException, eUnknown, err_msg);
        
        char buffer[256*2];
        ftp.read(&buffer[0], 256*2);
        size_t count  = ftp.gcount();
        if (count == 0 || ftp.bad())  
            NCBI_THROW(CException, eUnknown, "Failed to read from URL: " + url);
        CMD5 md5;
        md5.Update(buffer, count);
        unsigned char digest[16];
        md5.Finalize (digest);
        last_modified = md5.GetHexSum();
        
    } else {
        STimeout timeout;
        timeout.sec=15;
        timeout.usec=0 ;
        CConn_HttpStream http(url, eReqMethod_Head, NcbiEmptyString, fHTTP_NoAutoRetry | fHTTP_AutoReconnect, &timeout);
        if (!http.good())
            NCBI_THROW(CException, eUnknown, err_msg);
        string s;
        http >> s;
        int status_code = http.GetStatusCode();
        if (status_code != 200)
            NCBI_THROW(CException, eUnknown, err_msg);
        auto header = http.GetHTTPHeader();
        list<string> header_tags;
        NStr::Split(header, "\n", header_tags);
        for (const auto& h : header_tags) {
            string key, value;
            NStr::SplitInTwo(h, ":", key, value);
            if (!NStr::EqualNocase(key, "Last-Modified"))
                continue;
            NStr::TruncateSpacesInPlace(value);
            last_modified = value;
        }
    }
}

bool CUrlUtils::IfModifiedSince(const string& url, string& last_modified)
{
    string user_header = "If-Modified-Since: " + last_modified;
    CConn_HttpStream modified(url, eReqMethod_Head, user_header);
    string data;
    modified >> data;
    if (data.empty() && modified.GetStatusCode() == 304)
        return false;
    auto header = modified.GetHTTPHeader();
    list<string> header_tags;
    NStr::Split(header, "\n", header_tags);
    for (const auto& h : header_tags) {
        string key, value;
        NStr::SplitInTwo(h, ":", key, value);
        if (!NStr::EqualNocase(key, "Last-Modified"))
            continue;
        NStr::TruncateSpacesInPlace(value);
        last_modified = value;
    }
    return true;
}




END_NCBI_SCOPE
