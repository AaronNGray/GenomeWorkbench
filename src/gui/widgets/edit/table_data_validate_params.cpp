/*  $Id: table_data_validate_params.cpp 32200 2015-01-20 18:21:29Z katargir $
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
 * Authors: Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/table_data_validate_params.hpp>

#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CTableDataValidateParams::SetNamedBoolField(CUser_object& params, const string& field_name, bool val)
{
    bool found = false;
    if (params.IsSetData()) {
        NON_CONST_ITERATE(CUser_object::TData, it, params.SetData()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr() 
                && NStr::EqualNocase((*it)->GetLabel().GetStr(), field_name)) {
                found = true;
                (*it)->SetData().SetBool(val);
            }
        }
    }
    if (!found) {
        CRef<CUser_field> f(new CUser_field());
        f->SetLabel().SetStr(field_name);
        f->SetData().SetBool(val);
        params.SetData().push_back(f);
    }
}


bool CTableDataValidateParams::GetNamedBoolField(const CUser_object& params, const string& field_name)
{
    bool rval = false;
    if (!params.IsSetData()) {
        return false;
    }
    ITERATE(CUser_object::TData, it, params.GetData()) {
        if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr() 
            && NStr::EqualNocase((*it)->GetLabel().GetStr(), field_name)
            && (*it)->IsSetData() && (*it)->GetData().IsBool()) {
            rval = (*it)->GetData().GetBool();
            break;
        }
    }
    return rval;
}


static const string kAlignment = "Alignments";
static const string kInferences = "Inferences";


void CTableDataValidateParams::SetDoAlignment(CUser_object& params, bool val)
{
    SetNamedBoolField(params, kAlignment, val);
}


bool CTableDataValidateParams::GetDoAlignment(const CUser_object& params)
{
    return GetNamedBoolField(params, kAlignment);
}


void CTableDataValidateParams::SetDoInference(CUser_object& params, bool val)
{
    SetNamedBoolField(params, kInferences, val);
}


bool CTableDataValidateParams::GetDoInference(const CUser_object& params)
{
    return GetNamedBoolField(params, kInferences);
}

END_NCBI_SCOPE

