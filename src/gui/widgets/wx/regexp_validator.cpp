/*  $Id: regexp_validator.cpp 25483 2012-03-27 15:02:30Z kuznets $
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

#include <gui/widgets/wx/regexp_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <util/xregexp/regexp.hpp>

#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS(CRegexpValidator, wxValidator)

CRegexpValidator::CRegexpValidator( const char* szRegExp, const char* szExamples, wxString *val )
    : m_Name( "field" )
{
    m_StringValue = val;
    if( szRegExp ) m_RegExp = szRegExp;
    if( szExamples ) m_Examples = szExamples;
}

CRegexpValidator::CRegexpValidator( const CRegexpValidator& val )
    : wxValidator()
{
    Copy( val );
}

bool CRegexpValidator::Copy( const CRegexpValidator& val )
{
    wxValidator::Copy( val );

    m_StringValue = val.m_StringValue;
    m_RegExp = val.m_RegExp;
    m_Examples = val.m_Examples;
    m_Name = val.m_Name;

    return true;
}

// Called to transfer data to the window
bool CRegexpValidator::TransferToWindow()
{
    if( !CheckValidator() ){
        return false;
    }

    if ( m_StringValue ){
        wxTextCtrl *control = (wxTextCtrl*) m_validatorWindow;
        control->SetValue( *m_StringValue );
    }

    return true;
}

// Called to transfer data to the window
bool CRegexpValidator::TransferFromWindow()
{
    if( !CheckValidator() ){
        return false;
    }

    if( m_StringValue ){
        wxTextCtrl *control = (wxTextCtrl*) m_validatorWindow;
        *m_StringValue = control->GetValue();
    }

    return true;
}

bool CRegexpValidator::Validate( wxWindow *parent )
{
    if( !CheckValidator() ){
        return false;
    }

    wxTextCtrl *control = (wxTextCtrl*) m_validatorWindow;

    // If window is disabled, simply return
    if( !control->IsEnabled() ){
        return true;
    }

    string val = ToStdString( control->GetValue() );

    CRegexp re( m_RegExp );
    if( re.IsMatch( val ) ){
        return true;
    }

    string errmsg = "Invalid value for " + m_Name + ".";
    if( m_Examples.length() > 0 ){
        errmsg += "\nValid " + m_Name + " is like \n" + m_Examples + ".";
    }

    wxMessageBox( 
        ToWxString(errmsg), wxT("Validation conflict"),
        wxOK | wxICON_EXCLAMATION, parent
    );

    control->SetSelection( -1, -1 );
    control->SetFocus();

    return false;
}

END_NCBI_SCOPE
