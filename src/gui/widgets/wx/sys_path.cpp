/*  $Id: sys_path.cpp 33237 2015-06-19 20:28:12Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 * Authors:  Robert G. Smith
 *
 * File Description:
 *    CSysPath -- System dependent functions needed by CGBenchApp, the main
 *                         application class for GBENCH
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbimtx.hpp>

#include <gui/widgets/wx/sys_path.hpp>

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>
#include <wx/tokenzr.h>

#include <wx/app.h>
#include <wx/apptrait.h>

BEGIN_NCBI_SCOPE

string CSysPath::sm_Inst;

void CSysPath::SetInst(const string& inst)
{
    sm_Inst = inst;
}

string CSysPath::GetInst()
{
    return sm_Inst;
}

wxStandardPathsBase& CSysPath::GetStandardPaths()
{
    if( wxTheApp ){
        wxStandardPathsBase& paths = wxStandardPaths::Get();
        paths.UseAppInfo(wxStandardPaths::AppInfo_None);
        return paths;
    } else {
        // workaround for stupid wxWidgets politics
        //!HACK we know that wxStandardPath object is static
        wxStandardPathsBase& paths =  wxGUIAppTraits().GetStandardPaths();
        paths.UseAppInfo(wxStandardPaths::AppInfo_None);
        return paths;
    }
}

CSysPath::TAliasMap CSysPath::sm_AliasMap;

void CSysPath::BindAlias(const wxString& alias, const wxString& path)
{
    sm_AliasMap[alias] = path;
}


//
// ResolvePath()
// this function performs any standard app-specific directory mapping
// so that we can use a series of standard aliases for directory names
//
wxString CSysPath::ResolvePath(const wxString& path)
{
    wxString path1 = path, path2;

    size_t pos = path.find_first_of(wxT("\\/"));
    if (pos != wxString::npos) {
        path1 = path.substr(0, pos);
        path2 = path.substr(pos + 1);
    }

    if ( !path1.empty() )
        return ResolvePath(path1, path2);

    return path;
}


wxString CSysPath::ResolvePath(const wxString& path,
                               const wxString& subdir)
{
    wxString ret_val(path);
    TAliasMap::const_iterator it = sm_AliasMap.find(path);
    if (it != sm_AliasMap.end()) {
        ret_val = it->second;
    } else {
        if (ret_val == wxT("<bin>")) {
#if defined( NCBI_OS_DARWIN ) && !defined( _DEBUG )
            ret_val = GetStdPath();
#else
            wxFileName fname( GetStdPath(), wxEmptyString );
            fname.AppendDir(wxT("bin"));
            ret_val = fname.GetPath();
#endif
        } else if (ret_val == wxT("<std>")) {
            ret_val = GetStdPath();
        } else if (ret_val == wxT("<home>")) {
            ret_val = GetHomePath();
        } else if (ret_val == wxT("<pkgdir>")) {
            wxFileName fname(GetStdPath(), wxEmptyString);
            fname.AppendDir(wxT("packages"));
            ret_val = fname.GetPath();
        } else if (ret_val == wxT("<res>")) {
            ret_val = GetResourcePath();
        }
    }

    if ( !subdir.empty() ) {
        ret_val += wxFileName::GetPathSeparator();
        ret_val += subdir;
    }
    return ret_val;
}

wxString CSysPath::ResolvePathExisting(const wxString& path,
                                       const wxString& delim)
{
    wxArrayString toks = ::wxStringTokenize(path, delim);

    size_t count = toks.GetCount();
    for ( size_t i = 0; i < count; i++ ) {
        wxString p = ResolvePath(toks[i].Trim().Trim(false));
        if (wxFileName::FileExists(p))
            return p;
    }

    return wxEmptyString;
}

// the GBench installation directory. Usually the parent of the App Path.
// corresponds to the <std> alias in ResolvePath().
wxString CSysPath::GetStdPath(void)
{
    wxString ret_val;

    // First, was the value set in the environment variable $NCBI_GBENCH_HOME
    ::wxGetEnv(wxT("NCBI_GBENCH_HOME"), &ret_val);

    if (ret_val.empty()) {

        ret_val = GetStandardPaths().GetExecutablePath();
        if (!ret_val.empty()) {
            wxFileName fname(ret_val);
            const wxArrayString& dirs = fname.GetDirs();
            if (!dirs.IsEmpty() && dirs.Last() == wxT("bin"))
                fname.RemoveLastDir();

            ret_val = fname.GetPath();
        }

        // Hmmm... We aren't where we should be.
        if (ret_val.empty()) {
            LOG_POST(Warning
                     << "Can't identify the application's execution path.  "
                     "Some components may be unavailable.");
        }
    }
    return ret_val;
}


wxString CSysPath::GetResourcePath()
{
    wxString path = GetStdPath();
    wxFileName fname(path, wxEmptyString);
    fname.AppendDir(wxT("share"));
    fname.AppendDir(wxT("gbench"));
    return fname.GetPath();
}

// The place gbench keeps stuff.
// corresponds to the <home> alias in ResolvePath().
//   Is NOT the same as the user's home directory.
//   That is returned by CDir::GetHome()
wxString CSysPath::GetHomePath(void)
{
    // The Linux implementation of GetUserDataDir() uses static variable down the call stack
    // This makes this function not thread safe
    // Addiing mutex to fix Linux crashes
    DEFINE_STATIC_FAST_MUTEX(s_Mutex);
    CFastMutexGuard LOCK(s_Mutex);

    wxString ret_val = GetStandardPaths().GetUserDataDir();
    if (!ret_val.empty()) {
        wxFileName fname(ret_val, wxEmptyString);
#if defined(NCBI_OS_MSWIN) || defined(NCBI_OS_DARWIN)
        fname.AppendDir(wxT("GenomeWorkbench2"));
#else
        fname.AppendDir(wxT(".gbench2"));
#endif
        if (!sm_Inst.empty()) {
            fname.AppendDir(wxT("instances"));
            fname.AppendDir(wxString::FromUTF8(sm_Inst.c_str()));
        }

        ret_val = fname.GetPath();
    }

    return ret_val;
}

///
/// PATH variable manipulators
///
void CSysPath::AddToPath(const wxString& path, EPathMode mode)
{
    wxString PATH = wxT("PATH");

    wxString base_path;
    ::wxGetEnv(PATH, &base_path);

    const wxChar path_sep =
#ifdef NCBI_OS_MSWIN
        wxT(';')
#else
        wxT(':')
#endif
        ;

    wxString::size_type pos = base_path.find(path);
    if ( (pos == 0  && mode == ePath_Prepend)  ||
         (pos + path.length() == base_path.length()  &&  mode == ePath_Append) ) {
        return;
    }

    for (;pos != wxString::npos; pos = base_path.find(path, pos)) {
        if (pos > 0  &&  base_path[pos - 1] != path_sep) {
            pos += path.length();
            continue;
        }
        if (pos + path.length() < base_path.length()  &&  base_path[pos + 1] != path_sep) {
            pos += path.length();
            continue;
        }

        base_path.erase(pos, path.length());
    }

    switch (mode) {
    case ePath_Prepend:
        {{
            wxString p(path);
            p += path_sep;
            p += base_path;
            base_path = p;
        }}
        break;

    default:
    case ePath_Append:
        base_path += path_sep;
        base_path += path;
        break;
    }

    ::wxSetEnv(PATH, base_path);
}

END_NCBI_SCOPE
