#ifndef _PMID_VALIDATOR_H_
#define _PMID_VALIDATOR_H_

/*  $Id: pmid_validator.hpp 39654 2017-10-24 19:03:22Z filippov $
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

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <wx/valtext.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CPmidValidator : public wxTextValidator
{
public:
    CPmidValidator(long* val_ptr,
                     long minVal = numeric_limits<long>::min(),
                     long maxVal = numeric_limits<long>::max(),
                     const char* szErrMsg = 0);
    CPmidValidator(const CPmidValidator& val);
    virtual ~CPmidValidator(){}

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new CPmidValidator(*this); }
    bool Copy(const CPmidValidator& val);

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
    CPmidValidator& operator=(const CPmidValidator&);
};


END_NCBI_SCOPE

#endif // _PMID_VALIDATOR_H_

