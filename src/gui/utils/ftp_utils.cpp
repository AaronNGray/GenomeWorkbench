/*  $Id: ftp_utils.cpp 37350 2016-12-27 19:18:15Z katargir $
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
 * Authors:  Anatoliy Kuznetsov
 *
 * File Description:
 *    Utilities for interacting with a web browser
 */


#include <ncbi_pch.hpp>
#include <gui/utils/ftp_utils.hpp>

#include <connect/ncbi_conn_stream.hpp>

BEGIN_NCBI_SCOPE

void CFtpUtils::GetDirContent(vector<string>& content, const string& ftp_url)
{
    content.clear();

    SConnNetInfo* net_info = ConnNetInfo_Create(0);

    const char* url = ftp_url.c_str();

    if (!ConnNetInfo_ParseURL(net_info, url)) {
        string err = "Cannot parse URL \"";
        err.append(ftp_url);
        err.append("\"");
        NCBI_THROW(CException, eUnknown, err);
    }

    if (net_info->scheme != eURL_Ftp) {
        string err = "URL scheme must be FTP";
        err.append(ftp_url);
        err.append("\"");
        NCBI_THROW(CException, eUnknown, err);
    }

    if (!*net_info->user) {
        strcpy(net_info->user, "ftp");
    }

    CConn_FtpStream ftp(net_info->host,
        net_info->user,
        net_info->pass,
        net_info->path,
        0);

    ftp << "NLST";
    ConnNetInfo_Destroy(net_info);

    string fname;
    while (ftp.good()) {
        fname.clear();
        ftp >> fname;
        if (!fname.empty())
            content.push_back(fname);
    } // while
}

END_NCBI_SCOPE
