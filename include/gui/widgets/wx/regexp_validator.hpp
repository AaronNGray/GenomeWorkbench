#ifndef _REGEXPVALIDATOR_H_
#define _REGEXPVALIDATOR_H_

/*  $Id: regexp_validator.hpp 25479 2012-03-27 14:55:33Z kuznets $
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

#include <wx/validate.h>
#include <wx/window.h>
#include <wx/textctrl.h>


BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CRegexpValidator
    : public wxValidator
{
    DECLARE_DYNAMIC_CLASS(CRegexpValidator)
public:
    CRegexpValidator(const char* szRegExp = 0, const char* szExamples = 0, wxString *val = 0);
    CRegexpValidator(const CRegexpValidator& val);
    virtual ~CRegexpValidator(){}

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new CRegexpValidator(*this); }
    bool Copy(const CRegexpValidator& val);

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow *parent);

    // Called to transfer data to the window
    virtual bool TransferToWindow();

    // Called to transfer data from the window
    virtual bool TransferFromWindow();

    void SetName( const string& name ){ m_Name = name; }
    const string& GetName() const { return m_Name; }

protected:
    bool CheckValidator() const
    {
        wxCHECK_MSG( m_validatorWindow, false,
                     wxT("No window associated with validator") );
        wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), false,
                     wxT("wxTextValidator is only for wxTextCtrl's") );

        return true;
    }

private:
    CRegexpValidator& operator=(const CRegexpValidator&);

protected:
    wxString* m_StringValue;

private:
    string m_RegExp;
    string m_Examples;
    string m_Name;
};


END_NCBI_SCOPE

#endif // _REGEXPVALIDATOR_H_

