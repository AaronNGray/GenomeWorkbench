/*  $Id: browser_config.cpp 39528 2017-10-05 15:27:37Z katargir $
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
 * File Description:  Configure things so that gbench MIME types
 *                    launch this copy of gbench
 *
 */

#include <ncbi_pch.hpp>
#include "browser_config.hpp"

#include <gui/utils/utils_platform.hpp>

#include <corelib/ncbiapp.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/sys_path.hpp>

#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/filefn.h>

BEGIN_NCBI_SCOPE


#ifdef NCBI_OS_MSWIN

// Set the appropriate Windows registry variables.
// class_name is just a name that gets used in the registry;
// something like "gbench.asntext" seems appropriate.

static void s_RegisterMimeType(EHkey root_key, const string& mime_type,
                               const string& extension,
                               const string& command_line,
                               const string& class_name,
                               const string& description)
{

    SetWinRegVariable(root_key,
        "Software\\CLASSES\\MIME\\Database\\Content Type\\" + mime_type,
        "Extension", extension);


    SetWinRegVariable(root_key,
        "Software\\CLASSES\\" + class_name,
        "", description);

    SetWinRegVariable(root_key,
        "Software\\CLASSES\\" + class_name + "\\shell",
        "", "open");

    SetWinRegVariable(root_key,
        "Software\\CLASSES\\" + class_name + "\\shell\\open",
        "", "open");

    SetWinRegVariable(root_key,
        "Software\\CLASSES\\" + class_name + "\\shell\\open\\command",
        "", command_line);


    SetWinRegVariable(root_key,
        "Software\\CLASSES\\" + extension,
        "", class_name);

    SetWinRegVariable(root_key,
        "Software\\CLASSES\\" + extension,
        "Content Type", mime_type);

    SetWinRegVariable(root_key,
        "Software\\CLASSES\\" + extension + "\\" + class_name,
        "", "");

    SetWinRegVariable(root_key,
        "Software\\CLASSES\\" + extension + "\\" + class_name + "\\ShellNew",
        "", "");

}

#endif  // NCBI_OS_MSWIN



// For windows, set a bunch of stuff in the registry.  Both IE and
// Netscape/Mozilla look at this.
// We associate each MIME type of interest with a file name extension,
// and associate each file name extension with a command line
// involving gbench.


void GBenchBrowserConfig(bool whole_machine, bool remote)
{
#ifdef NCBI_OS_MSWIN


    CNcbiApplication* app = CNcbiApplication::Instance();
    _ASSERT(app);
    // enclose the path name in quotes
    string gbench_exe = "\"" + app->GetArguments().GetProgramName() + "\"";

    EHkey root_key;
    if (whole_machine) {
        root_key = eHkey_local_machine;
    } else {
        root_key = eHkey_current_user;
    }

    s_RegisterMimeType(root_key, "application/x-gbench-project",
        ".gbp", gbench_exe + " -t project \"%1\"", "gbench.project",
        "Genome Workbench project file");

    // This should eventually go away in favor of x-gbench-message
    s_RegisterMimeType(root_key, "application/x-gbench-plugin-message",
        ".gbmsg", gbench_exe + " -t message \"%1\"", "gbench.plugin",
        "Genome Workbench plugin message");

    s_RegisterMimeType(root_key, "application/x-gbench-message",
        ".gbmsg", gbench_exe + " -t message \"%1\"", "gbench.message",
        "Genome Workbench plugin message");

    s_RegisterMimeType(root_key, "application/x-gbench",
        ".gbench", gbench_exe + " -t auto \"%1\"", "gbench.auto",
        "Genome Workbench input file");

    s_RegisterMimeType(root_key, "application/x-gbench-asntext",
        ".gbasntx", gbench_exe + " -t asntext \"%1\"", "gbench.asntext",
        "Genome Workbench ASN.1 text");

    s_RegisterMimeType(root_key, "application/x-gbench-asnbin",
        ".gbasnbn", gbench_exe + " -t asnbin \"%1\"", "gbench.asnbin",
        "Genome Workbench ASN.1 binary");

    s_RegisterMimeType(root_key, "application/x-gbench-xml",
        ".gbxml", gbench_exe + " -t xml \"%1\"", "gbench.xml",
        "Genome Workbench XML");

    s_RegisterMimeType(root_key, "application/x-gbench-fasta",
        ".gbfasta", gbench_exe + " -t fasta \"%1\"", "gbench.fasta",
        "Genome Workbench FASTA format file");

    s_RegisterMimeType(root_key, "application/x-gbench-newick",
        ".gbnewic", gbench_exe + " -t newick \"%1\"", "gbench.newick",
        "Genome Workbench Newick format tree file");

    s_RegisterMimeType(root_key, "application/x-gbench-textalign",
        ".gbtxaln", gbench_exe + " -t textalign \"%1\"", "gbench.textalign",
        "Genome Workbench text format sequence alignment");

#else  // some kind of unix

    // For unix we'll just manipulate the user's .mailcap file.
    // Netscape/Mozilla looks at this.  Unfortunately Opera and
    // Konqueror don't.

    wxFileName fname( wxStandardPaths::Get().GetUserConfigDir(), wxT(".mailcap") );
    wxString mailcap_path = fname.GetFullPath();
    wxString backup_file = mailcap_path + wxT("-gbench-backup");
    bool orig_mailcap_exists = fname.FileExists();
    if (orig_mailcap_exists) {
        ::wxRenameFile(mailcap_path, backup_file);
    }
    CNcbiOfstream new_mailcap(mailcap_path.fn_str());
    if (orig_mailcap_exists) {
        CNcbiIfstream orig_mailcap(backup_file.fn_str());
        string line;
        while (getline(orig_mailcap, line)) {
            if (!NStr::StartsWith(line, "application/x-gbench",
                NStr::eNocase)) {
                new_mailcap << line << '\n';
            }
        }
    }

    static const string sc_Types[] = {"asntext", "asnbin", "xml", "newick",
        "textalign", "fasta", "project", "message"};

    string bin_dir = string(CSysPath::GetStdPath().ToUTF8()) + "/bin/";
    string remoteness;
#if 0  // -remote and -noremote switches have been eliminated
    if (remote) {
        remoteness = "remote_";
    } else {
        remoteness = "noremote_";
    }
#else
    remoteness = "remote_";
#endif
    new_mailcap << "application/x-gbench; "
                << bin_dir << "gbench_" << remoteness << "auto %s" << '\n';
    for (unsigned int i = 0;
    i < sizeof(sc_Types) / sizeof(string);  ++i) {
        new_mailcap << "application/x-gbench-" << sc_Types[i] << "; "
            << bin_dir << "gbench_" << remoteness << sc_Types[i]
            << " %s" << '\n';
    }

#endif

}


END_NCBI_SCOPE
