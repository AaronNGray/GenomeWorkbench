#ifndef GUI_UTILS__REPORT_DIALOG__HPP
#define GUI_UTILS__REPORT_DIALOG__HPP
/*  $Id: report_dialog.hpp 43155 2019-05-22 17:29:04Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrea Asztalos
 */


#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <wx/dialog.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_DATA_EXPORT CReportEditingDialog : public wxDialog
{
public:
    /// Report opening events in dialogs used in the editing package
    virtual bool Show(bool show = 1);
};


namespace NEditingStats
{
    /// Report opening & accepting events in the editing package
    void NCBI_GUIWIDGETS_DATA_EXPORT ReportUsage(const wxString& dialog_name);
    /// Report usage of the submission wizard dialog
    void NCBI_GUIWIDGETS_DATA_EXPORT ReportSubWizardUsage(const string& action_name);
}

namespace NMacroStats
{
    /// Report events in the macro_flow_editor and in the macro_editor
    void NCBI_GUIWIDGETS_DATA_EXPORT ReportUsage(const wxString& dialog_name, const string& action_name);
    /// Report when a macro is executed
    void NCBI_GUIWIDGETS_DATA_EXPORT ReportMacroExecution();
    /// Save macro created by the editor
    void NCBI_GUIWIDGETS_DATA_EXPORT SaveScriptCreated(const string& macro);
    /// Save macro obtained from the library
    void NCBI_GUIWIDGETS_DATA_EXPORT SaveScriptFromLibrary(const string& macro);
    /// Save macro to a file
    void SaveScriptToFile(const string& macro, const string& filename);
    /// Report attempt to update a local macro script
    void NCBI_GUIWIDGETS_DATA_EXPORT ReportMacroToBeUpdated(const string& username, const string& filename, const string& action);
}

END_NCBI_SCOPE

#endif
    // GUI_UTILS__REPORT_DIALOG__HPP
