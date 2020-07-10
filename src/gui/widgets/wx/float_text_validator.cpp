/*  $Id: float_text_validator.cpp 25482 2012-03-27 15:00:20Z kuznets $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistr.hpp>

#include <gui/widgets/wx/float_text_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/msgdlg.h>


BEGIN_NCBI_SCOPE

//IMPLEMENT_DYNAMIC_CLASS(CFloatTextValidator, wxValidator)

#define INIT_BASE_MEMBERS \
    : wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST) \
    , m_OutputFormat( outputFormat ? outputFormat : "%.3g" ) \
    , m_ValPtr( valPtr ) 

CFloatTextValidator::CFloatTextValidator(
    double* valPtr,
    double minVal,
    double maxVal,
    const char* outputFormat
)
    INIT_BASE_MEMBERS
    , m_MinVal(minVal)
    , m_MaxVal(maxVal)
{
    Init();
}

CFloatTextValidator::CFloatTextValidator( double* valPtr, const char* outputFormat )
    INIT_BASE_MEMBERS
    , m_MinVal( -numeric_limits<double>::infinity() )
    , m_MaxVal( numeric_limits<double>::infinity() )
{
    Init();
}

void CFloatTextValidator::Init()
{
    static const wxChar* chars[]
    = { wxT("0"), wxT("1"), wxT("2"), wxT("3"), wxT("4"), wxT("5"), wxT("6"), wxT("7"), wxT("8"), wxT("9"), wxT("e"), wxT("E"), wxT("-"), wxT("+"), wxT(".") };
    const int n = sizeof(chars) / sizeof(wxChar*);

    wxArrayString valid_chars(n, chars);
    SetIncludes(valid_chars);
}

CFloatTextValidator::CFloatTextValidator(const CFloatTextValidator& val)
    : wxTextValidator()
{
    Copy(val);
}


bool CFloatTextValidator::Copy(const CFloatTextValidator& val)
{
    wxTextValidator::Copy(val);

    m_OutputFormat = val.m_OutputFormat;
    m_ValPtr = val.m_ValPtr;
    m_MinVal = val.m_MinVal;
    m_MaxVal = val.m_MaxVal;

    return true;
}

bool CFloatTextValidator::TransferToWindow(void)
{
    if ( ! CheckValidator() )    {
        return false;
    }

    if (m_ValPtr)    {
        char s[128];
        sprintf(s, m_OutputFormat.c_str(), *m_ValPtr);

        wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
        control->SetValue(ToWxString(s));
    }
    return true;
}


// Called to transfer data to the window
bool CFloatTextValidator::TransferFromWindow(void)
{
    if ( ! CheckValidator() ){
        return false;
    }

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;

    if( m_ValPtr  &&  control->IsEnabled() && !control->HasFlag( wxTE_READONLY ) ){
        string str = ToStdString(control->GetValue());
        *m_ValPtr = NStr::StringToDouble(str);
    }
    return true;
}


bool CFloatTextValidator::Validate(wxWindow *parent)
{
    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
    if ( ! control->IsEnabled())
        return true;

    if(wxTextValidator::Validate(parent))    {
        double val = 0;
        try {
            string str = ToStdString(control->GetValue());
            val = NStr::StringToDouble(str);
        } catch(CStringException& e)    {
            string err_msg = e.GetMsg();
            wxMessageBox(ToWxString(err_msg), wxT("Input is not valid"),
                         wxOK | wxICON_EXCLAMATION, parent);

            control->SetSelection(-1,-1);
            control->SetFocus();
            return false;
        }

        if (val >= m_MinVal && val <= m_MaxVal)
            return true;

        wxString errMsg, fmt;
        if (m_MinVal > -numeric_limits<double>::infinity() &&
            m_MaxVal < numeric_limits<double>::infinity()) {
            fmt.Printf(wxT("Please enter number between %s and %s."),
                       m_OutputFormat.c_str(), m_OutputFormat.c_str());
            errMsg.Printf (fmt, m_MinVal, m_MaxVal);
        }
        else if (m_MinVal > -numeric_limits<double>::infinity()) {
            fmt.Printf(wxT("Please enter number greater or equal then %s."),
                       m_OutputFormat.c_str());
            errMsg.Printf(fmt, m_MinVal);
        }
        else if (m_MaxVal < numeric_limits<double>::infinity()) {
            fmt.Printf(wxT("Please enter number less or equal then %s."),
                m_OutputFormat.c_str());
            errMsg.Printf(fmt, m_MaxVal);
        }

        wxMessageBox(errMsg, wxT("Input is not valid"), wxOK | wxICON_ERROR, parent);
        control->SetSelection(-1,-1);
        control->SetFocus();
        return false;
    }
    return false;
}


END_NCBI_SCOPE
