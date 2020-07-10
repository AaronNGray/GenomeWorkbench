/*  $Id: app_popup.cpp 27993 2013-05-06 20:30:36Z rafanovi $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *    CAppPopup -- launch a url in a web browser
 */

#include <ncbi_pch.hpp>
#include <gui/utils/app_popup.hpp>
#include <gui/utils/exec.hpp>
#include <corelib/ncbiexec.hpp>

#ifdef NCBI_OS_MSWIN
#include <windows.h>
#include <ShellAPI.h>
#include <atlbase.h>
#include <atlconv.h>
#endif

#if defined(NCBI_OS_MAC) || defined(NCBI_OS_DARWIN)
#   ifdef NCBI_COMPILER_METROWERKS
#   define __NOEXTENSIONS__
#   endif
#   include <ApplicationServices/ApplicationServices.h>
#endif



BEGIN_NCBI_SCOPE


//
// function prototypes
//
CAppRegistry s_CreateDefaultAppRegistry(void);


//
// CAppPopup:: static member definition
//
CAppRegistry CAppPopup::m_Registry(s_CreateDefaultAppRegistry());


CAppInfo::CAppInfo(const string& path)
    : m_Exepath(path)
{
}


CAppInfo::~CAppInfo()
{
}


const string& CAppInfo::GetExePath(void) const
{
    return m_Exepath;
}


CNcbiOstream& filetype::operator<< (CNcbiOstream& strm,
                                    filetype::TFileType& ftype)
{
    switch (ftype) {
    case filetype::ePdf:
        strm << "PDF";
        break;
    case filetype::eUnknown:
        strm << "Unknown";
        break;
    default:
        strm << "Unrecognized file type";
        break;
    }
    return strm;
}


CAppRegistry s_CreateDefaultAppRegistry(void)
{
    CAppRegistry ar;

    ar[filetype::ePdf] = CRef<CAppInfo>(new CAppInfo("acroread"));

    return ar;
}


CAppRegistry::CAppRegistry()
{
}


CAppRegistry::~CAppRegistry()
{
}


CAppRegistry::TAppInfoRef& CAppRegistry::operator[](const TFileType& type)
{
    return m_AppReg[type];
}


const CAppRegistry::TAppInfoRef& CAppRegistry::Find(TFileType filetype) const
{
    TRegistry::const_iterator it = m_AppReg.find(filetype);
    if (it == m_AppReg.end()) {
        ERR_POST(Warning
                 << "CAppRegistry::Find(): no application associated with type "
                 << filetype
                );
    }

    return it->second;
}




CAppRegistry& CAppPopup::GetRegistry(void)
{
    return m_Registry;
}


bool CAppPopup::PopupFile(const string& file, TFileType filetype)
{
    if (file.empty()) {
        return false;
    }

#ifdef NCBI_OS_MSWIN

    return PopupURL(file);

#elif  defined(NCBI_OS_DARWIN)

    FSRef fileRef;
    OSErr err = FSPathMakeRef((const UInt8 *) file.c_str(), &fileRef, NULL);
    if (err == noErr) {
        err = LSOpenFSRef(&fileRef, NULL);
    }
    return (err == noErr);

#else

    const CRef<CAppInfo>& ai = m_Registry.Find(filetype);
    const string& prog = ai->GetExePath();
    const string app(prog + " " + file);
    return CExec::System(app.c_str()) != 0;

#endif
}


bool CAppPopup::PopupURL(const string& url)
{
    _TRACE(Info << "CAppPopup::PopupURL: opening URL: " << url);
    if (url.empty()) {
        return false;
    }

#ifdef NCBI_OS_MSWIN

    USES_CONVERSION;

    int res =
        reinterpret_cast<int>(ShellExecute(NULL,
                                           _T("open"),
                                           CA2T(url.c_str()),
                                           NULL,
                                           NULL,
                                           SW_SHOWNORMAL));
    return res > 32;

#elif defined(NCBI_OS_DARWIN)

   /* open the url with the default browser */

    CFURLRef urlRef = CFURLCreateWithBytes(kCFAllocatorDefault,
                                           (const UInt8 *) url.c_str(),
                                           url.size(),
                                           kCFStringEncodingMacRoman,
                                           NULL
                                          );
    const OSErr err = LSOpenCFURLRef(urlRef, NULL);
    return (err == noErr);

#elif defined(NCBI_OS_UNIX)

    // Use netscape/mozilla -remote mechanism is netscape is running
    // Otherwise, launch browser anew

    string std_in, std_out, std_err;
    vector<string> args;
    int exit_status;

    vector<string> programs;
    programs.push_back("firefox");
    programs.push_back("mozilla");
    programs.push_back("netscape");

    // try pop up a new window using netscape/mozilla -remote
    args.push_back("-remote");
    string arg = "openURL(" + url + ",new-window)";
    args.push_back(arg);
    ITERATE (vector<string>, program, programs) {
        exit_status = CExecute::Exec(*program, args,
                                     std_in, std_out, std_err);
        if (!exit_status && std_err.empty()) {
            return true;
        }
    }
    // start a new browser
    ITERATE (vector<string>, program, programs) {
        try {
           CExec::SpawnLP(CExec::eDetach, program->c_str(), url.c_str(), NULL);
        }
        catch(const std::exception&)
        {
            continue;
        }
        return true;
    }
    return false;

#else

    return false;

#endif
}


END_NCBI_SCOPE
