#ifndef _NUMBER_VALIDATOR_H_
#define _NUMBER_VALIDATOR_H_

/*  $Id: number_validator.hpp 24602 2011-10-25 17:59:16Z katargir $
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

#include <wx/valtext.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CNumberValidator : public wxTextValidator
{
public:
    CNumberValidator(long* val_ptr,
                     long minVal = numeric_limits<long>::min(),
                     long maxVal = numeric_limits<long>::max(),
                     const char* szErrMsg = 0);
    CNumberValidator(const CNumberValidator& val);
    virtual ~CNumberValidator(){}

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new CNumberValidator(*this); }
    bool Copy(const CNumberValidator& val);

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow *parent);

    // Called to transfer data to the window
    virtual bool TransferToWindow();

    // Called to transfer data from the window
    virtual bool TransferFromWindow();

protected:
    bool CheckValidator() const;

    long*  m_ValPtr;
    long   m_MinVal;
    long   m_MaxVal;
    wxString m_ErrMsg;
private:
    CNumberValidator& operator=(const CNumberValidator&);
};


class NCBI_GUIWIDGETS_WX_EXPORT CNumberOrBlankValidator : public CNumberValidator
{
public:
    CNumberOrBlankValidator(long* val_ptr,
                     long minVal = numeric_limits<long>::min(),
                     long maxVal = numeric_limits<long>::max(),
                     const char* szErrMsg = 0);
    CNumberOrBlankValidator(const CNumberOrBlankValidator& val);
    virtual ~CNumberOrBlankValidator(){}
    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new CNumberOrBlankValidator(*this); }

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow *parent);

    // Called to transfer data to the window
    virtual bool TransferToWindow();

private:
    CNumberValidator& operator=(const CNumberValidator&);
};

END_NCBI_SCOPE

#endif // _NUMBER_VALIDATOR_H_

