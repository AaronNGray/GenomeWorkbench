/*  $Id: browser_utils.cpp 38348 2017-04-27 14:54:37Z ivanov $
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
 * File Description:
 *    Utilities for interacting with a web browser
 */


#include <ncbi_pch.hpp>
#include <gui/utils/browser_utils.hpp>
#include <gui/utils/app_popup.hpp>

#include <corelib/ncbireg.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbifile.hpp>

#include <util/random_gen.hpp>


BEGIN_NCBI_SCOPE

/// Convenience function for html
void CBrowserUtils::SendToBrowser(const string& html)
{
    SendToBrowser(html, "text/html");
}

/// Send data to a web browser with appropriate mime type.
/// For now do this via a temp file.
void CBrowserUtils::SendToBrowser(const string& body, const string& mime_type)
{
    // figure out a usable file name

    string dir;
    CNcbiApplication* app = CNcbiApplication::Instance();
    _ASSERT(app);
    const CNcbiRegistry& app_registry = app->GetConfig();
    dir = app_registry.Get("app", "popup_tmp_dir");

    // this is a lame temporary solution
    string fname;
    string extension;
    if (mime_type == "application/pdf") {
        extension = ".pdf";
    } else if (mime_type == "text/comma-separated-values") {
        extension = ".csv";
    } else if (mime_type == "text/plain") {
        extension = ".txt";
    } else {
        extension = ".html";
    }

    // make the file name end in appropriate extension
    CRandom r((CRandom::TValue)time(0));
    const int max_tries = 100;
    int i;
    for (i = 0;  i < max_tries;  i++) {
        if (dir.empty()) {
            fname = CFile::GetTmpName();
        } else {
            fname = CFile::GetTmpNameEx(dir);
        }
        fname += NStr::IntToString(r.GetRand()) + extension;
        CFile file(fname);
        if (!file.Exists()) {
            break;
        }
    }
    if (i == max_tries) {
        LOG_POST(Error << "Couldn't create temporary file");
        return;
    }

    CNcbiOfstream tmpfile(fname.c_str());
    tmpfile << body;
    tmpfile.close();
    CFileDeleteAtExit::Add(fname);

#ifdef NCBI_OS_MSWIN
    // change '\' to '/' for url
    string tmp_str;
    NStr::Replace(fname, "\\", "/", tmp_str);
    fname = tmp_str;
#endif

    CAppPopup::PopupURL(string("file:") + fname);
}


void CBrowserUtils::AddBaseTag(string& html, const string& url)
{
    string::size_type idx = NStr::FindNoCase(html, "</head>");
    if (idx != string::npos) {
        html.insert(idx, string("<base href=\"") + url + "\">\n");
    }
}


END_NCBI_SCOPE
