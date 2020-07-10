/*  $Id: userfield_stringlist_validator.cpp 38247 2017-04-18 19:27:40Z filippov $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>

#include <serial/objectinfo.hpp>

#include <gui/widgets/edit/userfield_stringlist_validator.hpp>
#include "string_list_ctrl.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CSerialStringListValidator
 */


CUserFieldStringListValidator::CUserFieldStringListValidator(CUser_object& user, const string& fieldName)
    : m_User(user), m_FieldName(fieldName)
{
}


CUserFieldStringListValidator::CUserFieldStringListValidator(const CUserFieldStringListValidator& val)
    : wxValidator(), m_User(val.m_User), m_FieldName(val.m_FieldName)
{
}


bool CUserFieldStringListValidator::TransferToWindow()
{
    CStringListCtrl *control = (CStringListCtrl*)m_validatorWindow;
    control->Clear();

    size_t maxStringsCount =  control->GetMaxStringsCount();

	  if (m_User.IsSetData()) {
        ITERATE (CUser_object::TData, field, m_User.GetData()) {
            if ((*field)->IsSetData()
                && (*field)->IsSetLabel() && (*field)->GetLabel().IsStr()
                && NStr::EqualCase((*field)->GetLabel().GetStr(), m_FieldName)) {
                if ((*field)->GetData().IsStr()) {
                    control->AddString((*field)->GetData().GetStr());
                    if (--maxStringsCount == 0)
                        break;
                } else if ((*field)->GetData().IsStrs()) {
                    ITERATE(CUser_field::TData::TStrs, str, (*field)->GetData().GetStrs()) {
                        control->AddString(*str);
                        --maxStringsCount;
                    }
                }
            }
        }
    }

    if (maxStringsCount > 0)
        control->AddString("");

    control->FitInside();

    return true;
}

bool CUserFieldStringListValidator::TransferFromWindow()
{
    CUser_field& field = m_User.SetField(m_FieldName);
    field.ResetData();

    CStringListCtrl  *control = (CStringListCtrl *)m_validatorWindow;
    CStringListCtrl::CConstIterator it(*control);

    int num_values = 0;
    if (it) {
        for (; it; ++it) {
            string value = it.GetValue();
            NStr::TruncateSpacesInPlace(value);
			      if (!value.empty()) {
                field.SetData().SetStrs().push_back(value);
                num_values++;
            }
        }
    }
    field.SetNum(num_values);
    if (num_values == 0) {
        NON_CONST_ITERATE (CUser_object::TData, field, m_User.SetData()) {
            if ((*field)->IsSetLabel() && (*field)->GetLabel().IsStr() 
                && NStr::EqualCase((*field)->GetLabel().GetStr(), m_FieldName)) {
                m_User.SetData().erase(field);
                break;
            }
        }
    }
    return true;
}

END_NCBI_SCOPE
