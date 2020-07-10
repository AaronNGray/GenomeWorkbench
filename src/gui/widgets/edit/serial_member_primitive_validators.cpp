/*  $Id: serial_member_primitive_validators.cpp 38981 2017-07-14 18:37:03Z filippov $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <serial/objectinfo.hpp>
#include <objects/biblio/Title.hpp>

#include <wx/msgdlg.h>
#include <wx/textctrl.h>
#include <wx/radiobox.h>
#include <wx/choice.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE

static bool CheckValueType(const CMemberInfo* memberInfo, EPrimitiveValueType type)
{
    TTypeInfo memberType = memberInfo->GetTypeInfo();
    if (memberType->GetTypeFamily() != eTypeFamilyPrimitive) {
        wxCHECK_MSG( 0, false, wxT("Invalid member name") );
    }

    const CPrimitiveTypeInfo* primitiveType =
        CTypeConverter<CPrimitiveTypeInfo>::SafeCast(memberType);
    wxCHECK_MSG( primitiveType->GetPrimitiveValueType() == type,
                 false, wxT("Invalid member name") );
    return true;
}

/*
 * CSerialTextValidator
 */

CSerialTextValidator::CSerialTextValidator(const CSerialTextValidator& val)
    : CSerialMemberValidator(val)
{
}

bool CSerialTextValidator::TransferToWindow()
{
    if( !CheckValidator() )
        return false;

    wxTextCtrl *control = (wxTextCtrl*)m_validatorWindow;

    TConstObjectPtr memberPtr = m_MemberInfo->GetItemPtr(&m_Object);
    const CPrimitiveTypeInfo* primitiveType =
        CTypeConverter<CPrimitiveTypeInfo>::SafeCast(m_MemberInfo->GetTypeInfo());

    bool optional = m_MemberInfo->Optional();

    string value;

    if (!optional || m_MemberInfo->GetSetFlagYes(&m_Object))
        primitiveType->GetValueString(memberPtr, value);

    control->SetValue(ToWxString(value));

    return true;
}

bool CSerialTextValidator::TransferFromWindow()
{
    if( !CheckValidator() )
        return false;

    wxTextCtrl *control = (wxTextCtrl*)m_validatorWindow;
    string value = ToStdString(control->GetValue());
    NStr::TruncateSpacesInPlace(value);

    bool optional = m_MemberInfo->Optional();

    if (optional && value.empty()) {
        m_MemberInfo->UpdateSetFlagNo(&m_Object);
    }
    else {
        TObjectPtr memberPtr = m_MemberInfo->GetItemPtr(&m_Object);
        const CPrimitiveTypeInfo* primitiveType =
            CTypeConverter<CPrimitiveTypeInfo>::SafeCast(m_MemberInfo->GetTypeInfo());
        primitiveType->SetValueString(memberPtr, value);
        m_MemberInfo->UpdateSetFlagYes(&m_Object);
    }

    return true;
}

bool CSerialTextValidator::CheckValidator() const
{
    wxCHECK_MSG( m_MemberInfo, false, wxT("Invalid member name") );

    if (!CheckValueType(m_MemberInfo, ePrimitiveValueString))
        return false;

    wxCHECK_MSG( m_validatorWindow, false,
                 wxT("No window associated with validator") );
    wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), false,
                 wxT("CSerialTextValidator is only for wxTextCtrl's") );

    return true;
}

/*
 * CSerialAsciiTextValidator
 */

bool CSerialAsciiTextValidator::TransferFromWindow()
{
    if( !CheckValidator() )
        return false;

    wxTextCtrl *control = (wxTextCtrl*)m_validatorWindow;
    string value = ToAsciiStdString(control->GetValue());
    // get rid of leading and trailing spaces
    NStr::TruncateSpacesInPlace(value);
    // get rid of carriage returns
    NStr::ReplaceInPlace(value, " \n", " ");
    NStr::ReplaceInPlace(value, "\n ", " ");
    NStr::ReplaceInPlace(value, "\n", " ");

    bool optional = m_MemberInfo->Optional();

    if (optional && value.empty()) {
        m_MemberInfo->UpdateSetFlagNo(&m_Object);
    }
    else {
        TObjectPtr memberPtr = m_MemberInfo->GetItemPtr(&m_Object);
        const CPrimitiveTypeInfo* primitiveType =
            CTypeConverter<CPrimitiveTypeInfo>::SafeCast(m_MemberInfo->GetTypeInfo());

        try
        {
            primitiveType->SetValueString(memberPtr, value);
        } 
        catch(CException &e) 
        {
            return false;
        } 
        catch (exception &e) 
        {
            return false;
        }
        m_MemberInfo->UpdateSetFlagYes(&m_Object);
    }

    return true;
}

/*
 * CSerialBoolValidator
 */

CSerialBoolValidator::CSerialBoolValidator(const CSerialBoolValidator& val)
    : CSerialMemberValidator(val)
{
}

bool CSerialBoolValidator::TransferToWindow()
{
    if( !CheckValidator() )
        return false;

    wxControlWithItems *control = (wxControlWithItems*)m_validatorWindow;

    TConstObjectPtr memberPtr = m_MemberInfo->GetItemPtr(&m_Object);
    const CPrimitiveTypeInfo* primitiveType =
        CTypeConverter<CPrimitiveTypeInfo>::SafeCast(m_MemberInfo->GetTypeInfo());

    if (m_MemberInfo->GetSetFlagYes(&m_Object)) {
        bool value = primitiveType->GetValueBool(memberPtr);
        int index = value ? 0 : 1;
        if (control->GetCount() == 3)
            ++index;
        control->SetSelection(index);
    }
    else {
        control->SetSelection(control->GetCount() == 3 ? 0 : 1);
    }

    return true;
}

bool CSerialBoolValidator::TransferFromWindow()
{
    if( !CheckValidator() )
        return false;

    wxControlWithItems *control = (wxControlWithItems*)m_validatorWindow;
    int index = control->GetSelection();

    TObjectPtr memberPtr = m_MemberInfo->GetItemPtr(&m_Object);
    const CPrimitiveTypeInfo* primitiveType =
        CTypeConverter<CPrimitiveTypeInfo>::SafeCast(m_MemberInfo->GetTypeInfo());

    if (control->GetCount() == 3) {
        if (index == 0)
            m_MemberInfo->UpdateSetFlagNo(&m_Object);
        else {
            m_MemberInfo->UpdateSetFlagYes(&m_Object);
            primitiveType->SetValueBool(memberPtr, index == 1);
        }
    }
    else {
        m_MemberInfo->UpdateSetFlagYes(&m_Object);
        primitiveType->SetValueBool(memberPtr, index == 0);
    }

    return true;
}

bool CSerialBoolValidator::CheckValidator() const
{
    wxCHECK_MSG( m_MemberInfo, false, wxT("Invalid member name") );

    if (!CheckValueType(m_MemberInfo, ePrimitiveValueBool))
        return false;

    bool optional = m_MemberInfo->Optional();

    wxCHECK_MSG( m_validatorWindow, false,
                 wxT("No window associated with validator") );
    wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(wxRadioBox)), false,
                 wxT("CSerialBoolValidator is only for wxRadioBox's") );

    wxControlWithItems *control = (wxControlWithItems*)m_validatorWindow;
    int count = control->GetCount();
    wxCHECK_MSG( (optional ? count == 3 : count == 2), false,
                 wxT("CSerialBoolValidator assosiated with invalid control") );

    return true;
}

/*
 * CSerialEnumValidator
 */

CSerialEnumValidator::CSerialEnumValidator(const CSerialEnumValidator& val)
    : CSerialMemberValidator(val)
{
}

bool CSerialEnumValidator::TransferToWindow()
{
    if( !CheckValidator() )
        return false;

    bool optional = m_MemberInfo->Optional();

    TConstObjectPtr memberPtr = m_MemberInfo->GetItemPtr(&m_Object);
    const CEnumeratedTypeInfo* enumType =
        CTypeConverter<CEnumeratedTypeInfo>::SafeCast(m_MemberInfo->GetTypeInfo());

    TConstObjectPtr defaultPtr = m_MemberInfo->GetDefault();

    bool valueSet = false;
    int value;

    if (m_MemberInfo->GetSetFlagYes(&m_Object)) {
        valueSet = true;
        value = enumType->GetValueInt4(memberPtr);
    }
    else if (optional) {
        if (defaultPtr) {
            valueSet = true;
            value = enumType->GetValueInt4(defaultPtr);
        }
    }

    int newValue = 0;

    if (valueSet) {
        const CEnumeratedTypeValues::TValues& values = enumType->Values().GetValues();

        CEnumeratedTypeValues::TValues::const_iterator it;
        int index = 0;
        for (it = values.begin();it != values.end();++it, ++index) {
            if (value == it->second)
                break;
        }

        if (it == values.end())
            index = 0;

        if (optional && !defaultPtr)
            ++index;

        newValue = index;
    }

    wxRadioBox* radioBox = wxDynamicCast(m_validatorWindow, wxRadioBox);
    if (radioBox) {
        radioBox->SetSelection(newValue);
        return true;
    }

    wxChoice* choice = wxDynamicCast(m_validatorWindow, wxChoice);
    if (choice) {
        choice->SetSelection(newValue);
        return true;
    }

    return false;
}

bool CSerialEnumValidator::TransferFromWindow()
{
    if( !CheckValidator() )
        return false;

    int index = 0;
    wxRadioBox* radioBox = wxDynamicCast(m_validatorWindow, wxRadioBox);
    if (radioBox) {
        index = radioBox->GetSelection();
    } else {
        wxChoice* choice = wxDynamicCast(m_validatorWindow, wxChoice);
        if (choice)
            index = choice->GetSelection();
        else
            return false;
    }

    bool optional = m_MemberInfo->Optional();
    TConstObjectPtr defaultPtr = m_MemberInfo->GetDefault();

    TObjectPtr memberPtr = m_MemberInfo->GetItemPtr(&m_Object);
    const CEnumeratedTypeInfo* enumType =
        CTypeConverter<CEnumeratedTypeInfo>::SafeCast(m_MemberInfo->GetTypeInfo());

    if (optional) {
        if (index == 0 && !defaultPtr)
            m_MemberInfo->UpdateSetFlagNo(&m_Object);
        else {
            if (!defaultPtr)
                index--;

            const CEnumeratedTypeValues::TValues& values = enumType->Values().GetValues();
            CEnumeratedTypeValues::TValues::const_iterator it;
            for (it = values.begin(); it != values.end() && index > 0;++it, --index);

            if (it == values.end())
                it = values.begin();

            if (enumType->IsDefault((TConstObjectPtr)(&it->second)))
                m_MemberInfo->UpdateSetFlagNo(&m_Object);
            else {
                enumType->SetValueInt4(memberPtr, it->second);
                m_MemberInfo->UpdateSetFlagYes(&m_Object);
            }
        }
    }
    else {
        m_MemberInfo->UpdateSetFlagYes(&m_Object);
        enumType->SetValueInt4(memberPtr, index);
    }

    return true;
}

bool CSerialEnumValidator::CheckValidator() const
{
    wxCHECK_MSG( m_MemberInfo, false, wxT("Invalid member name") );

    if (!CheckValueType(m_MemberInfo, ePrimitiveValueEnum))
        return false;

    wxCHECK_MSG( m_validatorWindow, false,
                 wxT("No window associated with validator") );

    wxRadioBox* radioBox = wxDynamicCast(m_validatorWindow, wxRadioBox);
    wxChoice* choice = wxDynamicCast(m_validatorWindow, wxChoice);

    wxCHECK_MSG( radioBox || choice,
                 false,
                 wxT("CSerialEnumValidator is only for wxRadioBox or wxChoice") );

    TTypeInfo type = m_MemberInfo->GetTypeInfo();
    const CEnumeratedTypeInfo* enumType =
        CTypeConverter<CEnumeratedTypeInfo>::SafeCast(type);
    const CEnumeratedTypeValues::TValues& values = enumType->Values().GetValues();

    bool optional = m_MemberInfo->Optional();

    size_t count = optional && m_MemberInfo->GetDefault() == 0 ?
                        values.size() + 1 : values.size();

    int ctrlCount = radioBox ? radioBox->GetCount() : choice->GetCount();

    wxCHECK_MSG( count == ctrlCount, false,
                 wxT("CSerialEnumValidator assosiated with invalid control") );

    return true;
}


/*
 * CSerialTitleValidator
 */

CSerialTitleValidator::CSerialTitleValidator(const CSerialTitleValidator& val)
    : wxValidator(), m_Object(val.m_Object), m_TitleChoice(val.m_TitleChoice), m_Label(val.m_Label), m_ShowErr(val.m_ShowErr)
{
    Copy(val);
}

bool CSerialTitleValidator::Copy(const CSerialTitleValidator& val)
{
    wxValidator::Copy(val);
    m_Object = val.m_Object;
    m_TitleChoice = val.m_TitleChoice;
    m_Label = val.m_Label;
    m_ShowErr = val.m_ShowErr;
    return true;
}


bool CSerialTitleValidator::TransferToWindow()
{
    if( !CheckValidator() )
        return false;

    wxTextCtrl *control = (wxTextCtrl*)m_validatorWindow;

    objects::CTitle& title = dynamic_cast<objects::CTitle&>(m_Object);

    string value;

    if (title.IsSet() && !title.Set().empty()) {
      switch (title.Set().front()->Which()) {
        case objects::CTitle::C_E::e_Name:
          value = title.Set().front()->GetName();
          break;
        case objects::CTitle::C_E::e_Tsub:
          value = title.Set().front()->GetTsub();
          break;
        case objects::CTitle::C_E::e_Trans:
          value = title.Set().front()->GetTrans();
          break;
        case objects::CTitle::C_E::e_Jta:
          value = title.Set().front()->GetJta();
          break;
        case objects::CTitle::C_E::e_Iso_jta:
          value = title.Set().front()->GetIso_jta();
          break;
        case objects::CTitle::C_E::e_Ml_jta:
          value = title.Set().front()->GetMl_jta();
          break;
        case objects::CTitle::C_E::e_Coden:
          value = title.Set().front()->GetCoden();
          break;
        case objects::CTitle::C_E::e_Issn:
          value = title.Set().front()->GetIssn();
          break;
        case objects::CTitle::C_E::e_Abr:
          value = title.Set().front()->GetAbr();
          break;
        case objects::CTitle::C_E::e_Isbn:
          value = title.Set().front()->GetIsbn();
          break;
        default:
          break;
      }
    }

    control->SetValue(ToWxString(value));

    return true;
}

bool CSerialTitleValidator::TransferFromWindow()
{
    if( !CheckValidator() )
        return false;

    wxTextCtrl *control = (wxTextCtrl*)m_validatorWindow;
    string value = ToStdString(control->GetValue());
    NStr::TruncateSpacesInPlace(value);

    objects::CTitle& title = dynamic_cast<objects::CTitle&>(m_Object);

    CRef<objects::CTitle::C_E> item;

    if (title.Set().empty()) {
        item = new objects::CTitle::C_E();
        title.Set().push_back(item);
    } else {
        item = title.Set().front();
    }

    if (NStr::IsBlank(value)) {
        item->Reset();
        if (m_ShowErr) {
            wxMessageBox(NStr::IsBlank(m_Label) ? wxT("Missing Title") : ToWxString("Missing required field " + m_Label),
                         wxT("Error"), wxOK | wxICON_ERROR, (wxWindow*)(control));
            return false;
        }
    } else {
      switch (m_TitleChoice) {
        case objects::CTitle::C_E::e_Name:
          item->SetName(value);
          break;
        case objects::CTitle::C_E::e_Tsub:
          item->SetTsub(value);
          break;
        case objects::CTitle::C_E::e_Trans:
          item->SetTrans(value);
          break;
        case objects::CTitle::C_E::e_Jta:
          item->SetJta(value);
          break;
        case objects::CTitle::C_E::e_Iso_jta:
          item->SetIso_jta(value);
          break;
        case objects::CTitle::C_E::e_Ml_jta:
          item->SetMl_jta(value);
          break;
        case objects::CTitle::C_E::e_Coden:
          item->SetCoden(value);
          break;
        case objects::CTitle::C_E::e_Issn:
          item->SetIssn(value);
          break;
        case objects::CTitle::C_E::e_Abr:
          item->SetAbr(value);
          break;
        case objects::CTitle::C_E::e_Isbn:
          item->SetIsbn(value);
          break;
        default:
          break;
      }
    } 

    return true;
}

bool CSerialTitleValidator::CheckValidator() const
{
    wxCHECK_MSG( m_validatorWindow, false,
                 wxT("No window associated with validator") );
    wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), false,
                 wxT("CSerialTitleValidator is only for wxTextCtrl's") );

    return true;
}

END_NCBI_SCOPE
