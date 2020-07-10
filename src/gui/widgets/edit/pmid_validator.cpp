/*  $Id: pmid_validator.cpp 39654 2017-10-24 19:03:22Z filippov $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistr.hpp>

#include <gui/widgets/edit/pmid_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/msgdlg.h>
#include <wx/textctrl.h>

BEGIN_NCBI_SCOPE

CPmidValidator::CPmidValidator(long* val_ptr, long minVal,long maxVal,
                                   const char* szErrMsg)
    : wxTextValidator(wxFILTER_NONE)
{
    m_ValPtr = val_ptr;
    m_MinVal = minVal;
    m_MaxVal = maxVal;
    if (szErrMsg) {
        m_ErrMsg = ToWxString(szErrMsg);
    }
    else {
        if (m_MinVal > numeric_limits<long>::min() &&
            m_MaxVal < numeric_limits<long>::max())
            m_ErrMsg.Printf(wxT("Please enter number between %ld and %ld."),
                            m_MinVal, m_MaxVal);
        else if (m_MinVal > numeric_limits<long>::min())
            m_ErrMsg.Printf(wxT("Please enter number greater or equal then %ld."),
                            m_MinVal);
        else if (m_MaxVal < numeric_limits<long>::max())
            m_ErrMsg.Printf(wxT("Please enter number less or equal then %ld."),
                            m_MaxVal);
    }
   
}

CPmidValidator::CPmidValidator(const CPmidValidator& val)
    : wxTextValidator()
{
    Copy(val);
}

bool CPmidValidator::Copy(const CPmidValidator& val)
{
    wxTextValidator::Copy(val);
    m_ValPtr = val.m_ValPtr;
    m_MinVal = val.m_MinVal;
    m_MaxVal = val.m_MaxVal;
    m_ErrMsg = val.m_ErrMsg;
    return true;
}

bool CPmidValidator::CheckValidator() const
{
    wxCHECK_MSG( m_validatorWindow, false,
                 wxT("No window associated with validator") );
    wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), false,
                 wxT("CPmidValidator is only for wxTextCtrl's") );
    wxCHECK_MSG( m_MinVal <= m_MaxVal, false,
                 wxT("Invalid number range in CPmidValidator") );

    return true;
}


// Called to transfer data to the window
bool CPmidValidator::TransferToWindow(void)
{
    if( !CheckValidator() )
        return false;

    if ( m_ValPtr )
    {
        wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
        wxString s;
        if (*m_ValPtr != 0) {
            s = wxString::Format(wxT("%ld"), *m_ValPtr);
        }
        control->ChangeValue(s);
    }

    return true;
}

// Called to transfer data to the window
bool CPmidValidator::TransferFromWindow(void)
{
    if( !CheckValidator() )
        return false;

    if ( m_ValPtr )
    {
        *m_ValPtr = -1;
        wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
        wxString pmid_wx = control->GetValue();
        string pmid_str(pmid_wx.c_str());
        NStr::TruncateSpacesInPlace(pmid_str);
        if (NStr::StartsWith(pmid_str, "PMC") || NStr::Find(pmid_str, "/") != string::npos)
            return true;
        pmid_wx.ToLong(m_ValPtr);
    }
    
    return true;
}

bool CPmidValidator::Validate(wxWindow *parent)
{
    if( !CheckValidator() )
        return false;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;

    // If window is disabled, simply return
    if ( !control->IsEnabled() )
        return true;

    if (!wxTextValidator::Validate(parent))
        return false;
    
    wxString pmid_wx = control->GetValue();
    if (pmid_wx.IsEmpty()) {
        return true;
    }
    
    string pmid_str(pmid_wx.c_str());
    NStr::TruncateSpacesInPlace(pmid_str);
    if (NStr::StartsWith(pmid_str, "PMC") || NStr::Find(pmid_str, "/") != string::npos)
        return true;        
    
    long val;
    if (pmid_wx.ToLong(&val) && val >= m_MinVal && val <= m_MaxVal)
        return true;
    
    wxMessageBox(m_ErrMsg, wxT("Input is not valid"), wxOK | wxICON_ERROR, parent);
    control->SetSelection(-1,-1);
    control->SetFocus();
    return false;
}

END_NCBI_SCOPE
