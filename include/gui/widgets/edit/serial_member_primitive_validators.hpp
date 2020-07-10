#ifndef GUI_WIDGETS_EDIT___SERIAL_MEMBER_PRIMITIVE_VALIDATORS__HPP
#define GUI_WIDGETS_EDIT___SERIAL_MEMBER_PRIMITIVE_VALIDATORS__HPP

/*  $Id: serial_member_primitive_validators.hpp 31271 2014-09-16 16:24:01Z bollin $
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
#include <objects/biblio/Title.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CSerialTextValidator : public CSerialMemberValidator
{
public:
    CSerialTextValidator(CSerialObject& object, const string& memberName)
        : CSerialMemberValidator(object, memberName) {}

    CSerialTextValidator(const CSerialTextValidator& val);

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new CSerialTextValidator(*this); }

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow* /*parent*/) { return true; }

    // Called to transfer data to the window
    virtual bool TransferToWindow();

    // Called to transfer data from the window
    virtual bool TransferFromWindow();

protected:
    bool CheckValidator() const;
private:
    CSerialTextValidator& operator=(const CSerialTextValidator&);
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CSerialAsciiTextValidator : public CSerialTextValidator
{
public:
    CSerialAsciiTextValidator(CSerialObject& object, const string& memberName)
        : CSerialTextValidator(object, memberName) {}

    CSerialAsciiTextValidator(const CSerialTextValidator& val)
        : CSerialTextValidator(val) {}

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new CSerialAsciiTextValidator(*this); }

    // Called to transfer data from the window
    virtual bool TransferFromWindow();

private:
    CSerialAsciiTextValidator& operator=(const CSerialAsciiTextValidator&);
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CSerialBoolValidator : public CSerialMemberValidator
{
public:
    CSerialBoolValidator(CSerialObject& object, const string& memberName)
        : CSerialMemberValidator(object, memberName) {}

    CSerialBoolValidator(const CSerialBoolValidator& val);

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new CSerialBoolValidator(*this); }

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow* /*parent*/) { return true; }

    // Called to transfer data to the window
    virtual bool TransferToWindow();

    // Called to transfer data from the window
    virtual bool TransferFromWindow();

protected:
    bool CheckValidator() const;
private:
    CSerialBoolValidator& operator=(const CSerialBoolValidator&);
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CSerialEnumValidator : public CSerialMemberValidator
{
public:
    CSerialEnumValidator(CSerialObject& object, const string& memberName)
        : CSerialMemberValidator(object, memberName) {}

    CSerialEnumValidator(const CSerialEnumValidator& val);

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new CSerialEnumValidator(*this); }

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow* /*parent*/) { return true; }

    // Called to transfer data to the window
    virtual bool TransferToWindow();

    // Called to transfer data from the window
    virtual bool TransferFromWindow();

protected:
    bool CheckValidator() const;
private:
    CSerialEnumValidator& operator=(const CSerialEnumValidator&);
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CSerialTitleValidator : public wxValidator
{
public:
    CSerialTitleValidator(CSerialObject& object, objects::CTitle::C_E::E_Choice title_choice, const string& label, bool show_err = true) 
                       : m_Object (object),
                         m_TitleChoice (title_choice),
                         m_Label (label),
                         m_ShowErr (show_err)
                         { }

    CSerialTitleValidator(const CSerialTitleValidator& val);

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new CSerialTitleValidator(*this); }

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow* /*parent*/) { return true; }

    // Called to transfer data to the window
    virtual bool TransferToWindow();

    // Called to transfer data from the window
    virtual bool TransferFromWindow();

protected:
    bool CheckValidator() const;
private:
    CSerialTitleValidator& operator=(const CSerialTitleValidator&);
    bool Copy(const CSerialTitleValidator& val);
    CSerialObject& m_Object;
    objects::CTitle::C_E::E_Choice m_TitleChoice;
    string m_Label;
    bool m_ShowErr;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___SERIAL_MEMBER_PRIMITIVE_VALIDATORS__HPP
