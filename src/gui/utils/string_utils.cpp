/*  $Id: string_utils.cpp 34769 2016-02-08 20:17:51Z evgeniev $
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
 *    Generic FLTK utilities.
 */

#include <ncbi_pch.hpp>

#include <map>
#include <vector>

#include <corelib/ncbi_base64.h>
#include <gui/utils/string_utils.hpp>

USING_SCOPE(std);

BEGIN_NCBI_SCOPE

// do we need this code anymore?
string CStringUtil::BuildString(const char* cstrings[])
{
    string str;
    for (size_t i = 0;  cstrings[i];  i++) {
        str += cstrings[i];
    }
    return str;
}

string CStringUtil::ConcatenateStrings(const vector<string> &strings, const string &delimiter)
{
    string out_str;
    bool first = true;
    ITERATE(vector<string>, iter, strings) {
        if (first) {
            first = false;
            out_str = *iter;
        }
        else {
            out_str += delimiter + *iter;
        }
    }
    return out_str;
}

string CStringUtil::FormatStringsOccurrences(const vector<string> &strings, const string &delimiter)
{
    string out_str;
    bool first = true;
    typedef map<string, unsigned> TStrCountMap;
    TStrCountMap count;
    ITERATE(vector<string>, itStr, strings) {
        if (0 == count.count(*itStr))
            count[*itStr] = 1;
        else
            ++count[*itStr];
    }
    ITERATE(TStrCountMap, iter, count) {
        if (first) {
            first = false;
        }
        else {
            out_str += delimiter;
        }
        out_str += iter->first;
        out_str += '(';
        out_str += NStr::NumericToString(iter->second);
        out_str += ')';
    }
    return out_str;
}

string CStringUtil::base64Encode(const string& str)
{
    string encoded;
    size_t src_size = str.size();
    char dst_buf[128];
    size_t ptr = 0, src_read, dst_written, line_len = 0;
    while (src_size > 0) {
        BASE64_Encode(&str[ptr], src_size, &src_read,
                      dst_buf, sizeof(dst_buf), &dst_written, &line_len);
        ptr      += src_read;
        src_size -= src_read;
        encoded += string(dst_buf, dst_written);
    }
    return encoded;
}

string CStringUtil::base64Decode(const string& str)
{
    size_t src_size = str.size();
    string decoded;
    char dst_buf[128];
    size_t ptr = 0, src_read, dst_written;
    while (src_size > 0) {
        if (!BASE64_Decode(&str[ptr], src_size, &src_read, dst_buf, sizeof(dst_buf), &dst_written))
            break;
        ptr      += src_read;
        src_size -= src_read;
        decoded += string(dst_buf, dst_written);
    }
    return decoded;
}

END_NCBI_SCOPE

