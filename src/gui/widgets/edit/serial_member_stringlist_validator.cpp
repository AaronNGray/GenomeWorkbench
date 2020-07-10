/*  $Id: serial_member_stringlist_validator.cpp 37586 2017-01-25 15:53:12Z filippov $
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

#include <gui/widgets/edit/serial_member_stringlist_validator.hpp>

#include <serial/objectinfo.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include "string_list_ctrl.hpp"

#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE

/*
 * CSerialStringListValidator
 */

CSerialStringListValidator::CSerialStringListValidator(const CSerialStringListValidator& val)
    : CSerialMemberValidator(val)
{
    m_RequireOne = val.m_RequireOne;
    m_DisplayName = val.m_DisplayName;
}

bool CSerialStringListValidator::TransferToWindow()
{
    if( !CheckValidator() )
        return false;

    CStringListCtrl *control = (CStringListCtrl*)m_validatorWindow;
    control->Clear();
    TConstObjectPtr memberPtr = m_MemberInfo->GetItemPtr(&m_Object);
    const CContainerTypeInfo* containerType =
        CTypeConverter<CContainerTypeInfo>::SafeCast(m_MemberInfo->GetTypeInfo());
    const CPrimitiveTypeInfo* elementType =
        CTypeConverter<CPrimitiveTypeInfo>::SafeCast(containerType->GetElementType());

    size_t maxStringsCount =  control->GetMaxStringsCount();

    if (m_MemberInfo->GetSetFlagYes(&m_Object)) {
        CContainerTypeInfo::CConstIterator i;
        if ( containerType->InitIterator(i, memberPtr) ) {
            do {
                TConstObjectPtr elementPtr = containerType->GetElementPtr(i);
                string value;
                elementType->GetValueString(elementPtr, value);
                if (!value.empty()) {
                    control->AddString(value);
                    if (--maxStringsCount == 0)
                        break;
                }
            } while ( containerType->NextElement(i) );
        }
    }

    if (maxStringsCount > 0)
        control->AddString("");

    control->FitInside();

    return true;
}

bool CSerialStringListValidator::TransferFromWindow()
{
    if( !CheckValidator() )
        return false;

    TObjectPtr memberPtr = m_MemberInfo->GetItemPtr(&m_Object);
    const CContainerTypeInfo* containerType =
        CTypeConverter<CContainerTypeInfo>::SafeCast(m_MemberInfo->GetTypeInfo());

    bool optional = m_MemberInfo->Optional();

    CContainerTypeInfo::CIterator i;
    if (containerType->InitIterator(i, memberPtr))
        containerType->EraseAllElements(i);


    CStringListCtrl  *control = (CStringListCtrl *)m_validatorWindow;
    CStringListCtrl::CConstIterator it(*control);

    if (it) {
        for (; it; ++it) {
            string value = it.GetValue();
            NStr::TruncateSpacesInPlace(value);
            if (!value.empty())
                containerType->AddElement(memberPtr, &value);
        }
        m_MemberInfo->UpdateSetFlagYes(&m_Object);
    }
    else if (optional)
        m_MemberInfo->UpdateSetFlagNo(&m_Object);

    return true;
}


bool CSerialStringListValidator::Validate(wxWindow *parent)
{
    if (m_RequireOne) {
        bool has_one = false;
        CStringListCtrl  *control = (CStringListCtrl *)m_validatorWindow;
        CStringListCtrl::CConstIterator it(*control);

        if (it) {
            for (; it; ++it) {
                string value = it.GetValue();
                NStr::TruncateSpacesInPlace(value);
                if (!value.empty()) {
                    has_one = true;
                }
            }
        }
        if (!has_one) {
            string message = "You must specify ";
            if (NStr::IsBlank(m_DisplayName)) {
                message += "a value";
            } else {
                message += m_DisplayName;
            }
            wxMessageBox(ToWxString(message), _("Error"), wxOK | wxICON_ERROR);
        }
        return has_one;
    } else {
        return true;
    }        
}


bool CSerialStringListValidator::CheckValidator() const
{
// Validate member type
    wxCHECK_MSG( m_MemberInfo, false, wxT("Invalid member name") );

    TTypeInfo memberType = m_MemberInfo->GetTypeInfo();
    if (memberType->GetTypeFamily() != eTypeFamilyContainer) {
        wxCHECK_MSG( 0, false, wxT("Invalid member name") );
    }

    const CContainerTypeInfo* containerType =
        CTypeConverter<CContainerTypeInfo>::SafeCast(memberType);

    TTypeInfo elementType = containerType->GetElementType();

    if (elementType->GetTypeFamily() != eTypeFamilyPrimitive) {
        wxCHECK_MSG( 0, false, wxT("Invalid member name") );
    }

    const CPrimitiveTypeInfo* primitiveType =
        CTypeConverter<CPrimitiveTypeInfo>::SafeCast(elementType);
    wxCHECK_MSG( primitiveType->GetPrimitiveValueType() == ePrimitiveValueString,
                 false, wxT("Invalid member name") );

// Validate window type
    wxCHECK_MSG( m_validatorWindow, false,
                 wxT("No window associated with validator") );
    wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(CStringListCtrl)), false,
                 wxT("CSerialStringListValidator is only for CStringListCtrl's") );

    return true;
}

END_NCBI_SCOPE
