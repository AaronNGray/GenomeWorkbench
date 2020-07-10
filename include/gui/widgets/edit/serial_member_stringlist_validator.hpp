#ifndef GUI_WIDGETS_EDIT___SERIAL_MEMBER_STRINGLIST_VALIDATOR__HPP
#define GUI_WIDGETS_EDIT___SERIAL_MEMBER_STRINGLIST_VALIDATOR__HPP

/*  $Id: serial_member_stringlist_validator.hpp 28136 2013-05-15 15:07:51Z bollin $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/edit/serial_member_validator.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CSerialStringListValidator : public CSerialMemberValidator
{
public:
    CSerialStringListValidator(CSerialObject& object, const string& memberName, bool require_one = false, const string& display_name = "")
        : CSerialMemberValidator(object, memberName), m_RequireOne(require_one), m_DisplayName(display_name) {}

    CSerialStringListValidator(const CSerialStringListValidator& val);

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new CSerialStringListValidator(*this); }

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow *parent);

    // Called to transfer data to the window
    virtual bool TransferToWindow();

    // Called to transfer data from the window
    virtual bool TransferFromWindow();

protected:
    bool CheckValidator() const;
private:
    CSerialStringListValidator& operator=(const CSerialStringListValidator&);
    bool m_RequireOne;
    string m_DisplayName;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___SERIAL_MEMBER_STRINGLIST_VALIDATOR__HPP
