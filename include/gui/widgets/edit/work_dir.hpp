#ifndef GUI_WIDGETS_EDIT___WORK_DIR__HPP
#define GUI_WIDGETS_EDIT___WORK_DIR__HPP

/*  $Id: work_dir.hpp 41446 2018-07-31 15:24:14Z katargir $
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
 * Authors: Roman Katargin
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <wx/string.h>
#include <wx/window.h>

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
/// IWorkDirClient
/////////////////////////////////////////////////////////////////////////////

class IWorkDirClient
{
public:
    virtual ~IWorkDirClient() {}
    virtual void SetWorkDir(const wxString& workDir) = 0;
};

NCBI_GUIWIDGETS_EDIT_EXPORT void SetWorkDir(wxWindow* wnd, const wxString& workDir);

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___WORK_DIR__HPP
