#ifndef GUI_WIDGETS_WX___GRID_TABLE_EXPORT__HPP
#define GUI_WIDGETS_WX___GRID_TABLE_EXPORT__HPP

/*  $Id: grid_table_export.hpp 33774 2015-09-15 20:44:45Z evgeniev $
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
#include <gui/gui.hpp>
#include <wx/string.h>

class wxGridTableBase;

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CwxGridTableExport
{
public:
    CwxGridTableExport(wxGridTableBase& data, ostream& os, bool header = true, char delimiter = '\t', char quote = '"');
    CwxGridTableExport(wxGridTableBase& data, ostream& os,
                       const vector<int>& rows, const vector<int>& cols,
                       bool header = true,
                       char delimiter = '\t', char quote = '"');
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___GRID_TABLE_EXPORT__HPP
