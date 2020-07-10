/*  $Id: save_file_helper.cpp 39326 2017-09-12 19:45:14Z evgeniev $
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
* Authors:  Vladislav Evgeniev
*/


#include <ncbi_pch.hpp>

#include <gui/widgets/wx/save_file_helper.hpp>

#include <wx/string.h>
#include <wx/filedlg.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>

#include <corelib/ncbifile.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

bool CSaveFileHelper::ShowSaveDialog(CFileExtensions::EFileType file_type)
{
    wxString path = m_TextCtrl.GetValue();

    wxFileDialog dlg(m_Parent, wxT("Select an output file"), wxT(""), wxT(""),
        CFileExtensions::GetDialogFilter(file_type) + wxT("|") +
        CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
        wxFD_SAVE);

    dlg.SetPath(path);

    if (dlg.ShowModal() != wxID_OK)
        return false;

    // Add an extension if the user didn't.  They can still erase/change it before saving if they wish.
    path = dlg.GetPath();
    CFileExtensions::AppendDefaultExtension(file_type, path);

    m_TextCtrl.SetValue(path);
    
    return true;
}

bool CSaveFileHelper::Validate(wxString &path)
{
    path.Trim();

    if (path.empty()) {
        wxMessageBox(wxT("Please select file name"), wxT("Error"), wxOK | wxICON_WARNING, m_Parent);

        m_TextCtrl.SetFocus();
        return false;
    }
    // Warn user if file exists.
    CFile f(ToAsciiStdString(path));
    if (f.Exists()) {
        int choice = wxMessageBox(wxT("Selected file already exists. Are you sure you want to overwrite it?"), wxT("Warning"), wxYES_NO | wxICON_WARNING, m_Parent);
        if (choice != wxYES) {
            return false;
        }
    }
    return true;
}

END_NCBI_SCOPE