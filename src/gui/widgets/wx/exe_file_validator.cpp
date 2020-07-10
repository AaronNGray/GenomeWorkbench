/*  $Id: exe_file_validator.cpp 43792 2019-08-30 18:31:36Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/exe_file_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE

CExeFileValidator::CExeFileValidator(wxString *val, const wxString name)
    : m_Name(name)
{
    m_ValPtr = val;
}

CExeFileValidator::CExeFileValidator( const CExeFileValidator& val )
    : wxValidator()
{
    Copy( val );
}

bool CExeFileValidator::Copy( const CExeFileValidator& val )
{
    wxValidator::Copy( val );

    m_ValPtr = val.m_ValPtr;
    m_Name = val.m_Name;

    return true;
}

// Called to transfer data to the window
bool CExeFileValidator::TransferToWindow()
{
    if( !CheckValidator() ){
        return false;
    }

    if ( m_ValPtr ){
        wxTextCtrl *control = (wxTextCtrl*) m_validatorWindow;
        control->SetValue( *m_ValPtr );
    }

    return true;
}

// Called to transfer data to the window
bool CExeFileValidator::TransferFromWindow()
{
    if( !CheckValidator() ){
        return false;
    }

    if( m_ValPtr ){
        wxTextCtrl *control = (wxTextCtrl*) m_validatorWindow;
        *m_ValPtr = control->GetValue();
    }

    return true;
}

bool CExeFileValidator::Validate( wxWindow *parent )
{
    if( !CheckValidator() ){
        return false;
    }

    wxTextCtrl *control = (wxTextCtrl*) m_validatorWindow;

    // If window is disabled, simply return
    if( !control->IsEnabled() ){
        return true;
    }

    wxString errMsg, val = control->GetValue();

    if (val.IsEmpty()) {
        errMsg = wxT("Please locate ") + m_Name + wxT(" executable.");
    }
    else {
        wxString exePath = FindExeFile(val);
        if (exePath.IsEmpty()) {
            errMsg = m_Name + wxT(" executable doesn't exist: ") + val + wxT(".");
        }
        else if (!wxFileName::IsFileExecutable(exePath)) {
            errMsg = m_Name + wxT(" file is not executable: ") + exePath + wxT(".");
        }
    }

    if (errMsg.IsEmpty())
        return true;

    wxMessageBox(errMsg, wxT("Validation conflict"), wxOK | wxICON_EXCLAMATION, parent);

    control->SetSelection( -1, -1 );
    control->SetFocus();

    return false;
}

END_NCBI_SCOPE
