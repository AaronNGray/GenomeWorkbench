#ifndef GUI_WIDGETS_WX___SAVE_FILE_HELPER__HPP
#define GUI_WIDGETS_WX___SAVE_FILE_HELPER__HPP

/*  $Id: save_file_helper.hpp 43818 2019-09-05 19:11:46Z katargir $
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
* Authors:  Vladislav Evgeniev
*
* File Description:
*
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/file_extensions.hpp>

class wxTextCtrl;
class wxWindow;
class wxString;

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CSaveFileHelper
{
public:
    CSaveFileHelper(wxWindow *parent, wxTextCtrl &text_ctrl) : m_Parent(parent), m_TextCtrl(text_ctrl) {}
    bool ShowSaveDialog(CFileExtensions::EFileType file_type);
    bool Validate(wxString &path);
private:
    wxWindow    *m_Parent;
    wxTextCtrl  &m_TextCtrl;
};

END_NCBI_SCOPE

#endif  /// GUI_WIDGETS_WX___SAVE_FILE_HELPER__HPP
