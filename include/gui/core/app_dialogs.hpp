#ifndef GUI_CORE___APP_DIALOGS__HPP
#define GUI_CORE___APP_DIALOGS__HPP

/*  $Id: app_dialogs.hpp 29963 2014-03-20 15:34:24Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <corelib/ncbiobj.hpp>

#include <wx/string.h>

BEGIN_NCBI_SCOPE

class IWorkbench;

class NCBI_GUICORE_EXPORT CAppDialogs
{
public:
    static void RunToolDialog(IWorkbench* workbench, const string& toolLabel = NcbiEmptyString);
    static void ExportDialog(IWorkbench* workbench);
    static void OpenViewlDialog(IWorkbench* workbench, bool openDefault = false);
    static void COpenDialog(IWorkbench* workbench, const string& loader_label = NcbiEmptyString, const vector<wxString>& filenames = vector<wxString>());
};

END_NCBI_SCOPE

#endif  // GUI_CORE___APP_DIALOGS__HPP
