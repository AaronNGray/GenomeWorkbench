/*  $Id: report_dialog.cpp 43155 2019-05-22 17:29:04Z asztalos $
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
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/usage_report_job.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <wx/stdpaths.h>

BEGIN_NCBI_SCOPE

bool CReportEditingDialog::Show(bool show)
{
    REPORT_USAGE("editing", .Add("dialog_name", ToStdString(GetLabel())));
    return wxDialog::Show(show);
}

namespace NEditingStats
{
    void ReportUsage(const wxString& dialog_name)
    {
        REPORT_USAGE("editing", .Add("dialog_name", ToStdString(dialog_name)));
    }

    void ReportSubWizardUsage(const string& action_name)
    {
        REPORT_USAGE("submission_wizard", .Add("action_name", action_name));
    }
}

namespace NMacroStats
{
    void ReportUsage(const wxString& dialog_name, const string& action_name)
    {
        REPORT_USAGE("macro", .Add("dialog_name", ToStdString(dialog_name))
                              .Add("action_name", action_name));
    }

    void ReportMacroExecution()
    {
        ReportUsage("MacroScript", "execute");
    }

    void SaveScriptToFile(const string& macro, const string& filename)
    {
        if (macro.empty()) return;


        wxStandardPaths stdp = wxStandardPaths::Get();
        const string doc_dir = ToStdString(stdp.GetDocumentsDir());
        const string macro_dir = CDirEntry::ConcatPathEx(doc_dir, "GbenchMacro");

        CDir dir(macro_dir);
        if (!dir.Exists()) {
            if (!dir.Create()) {
                LOG_POST(Error << "Cannot create directory '" << macro_dir << "'");
                return;
            }
        }

        const string macro_file = CDirEntry::ConcatPathEx(macro_dir, filename);
        CNcbiOfstream ostr(macro_file.c_str(), ios::out | ios::app);
        ostr << macro << endl;
    }

    void SaveScriptCreated(const string& macro)
    {
        SaveScriptToFile(macro, "created_macro_log.txt");
    }

    void SaveScriptFromLibrary(const string& macro)
    {
        SaveScriptToFile(macro, "library_macro_log.txt");
    }

    void ReportMacroToBeUpdated(const string& username, const string& filename, const string& action)
    {
        REPORT_USAGE("macro_update", .Add("user", username)
                                     .Add("file", filename)
                                     .Add("action", action));
    }
}


END_NCBI_SCOPE

