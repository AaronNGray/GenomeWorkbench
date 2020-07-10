/*  $Id: project_item_extra.cpp 35093 2016-03-22 18:51:23Z katargir $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/project_item_extra.hpp>

#include <objects/gbproj/ProjectItem.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Annotdesc.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

bool CProjectItemExtra::GetStr(const objects::CProjectItem& pi, const string& tag, string& value)
{
    CConstRef<CUser_object> userObj;

    for (auto d : pi.GetDescr()) {
        if (d->IsUser()) {
            const CUser_object& user = d->GetUser();
            const CObject_id& type = user.GetType();
            if (type.IsStr() && type.GetStr() == "ProjectItemExtra") {
                userObj.Reset(&user);
                break;
            }
        }
    }

    if (!userObj)
        return false;

    for (auto f : userObj->GetData()) {
        const CObject_id& label = f->GetLabel();
        if (label.IsStr() && label.GetStr() == tag) {
            const CUser_field::TData& data = f->GetData();
            if (data.IsStr()) {
                value = data.GetStr();
                return true;
            }
        }
    }

    return false;
}

void CProjectItemExtra::SetStr(objects::CProjectItem& pi, const string& tag, const string& value)
{
    CRef<CAnnotdesc> desc;

    for (auto d : pi.GetDescr()) {
        if (d->IsUser()) {
            const CUser_object& user = d->GetUser();
            const CObject_id& type = user.GetType();
            if (type.IsStr() && type.GetStr() == "ProjectItemExtra") {
                desc.Reset(d);
                break;
            }
        }
    }

    if (!desc) {
        CRef<CUser_object> user(new CUser_object());
        CRef<CObject_id> type(new CObject_id());
        type->SetStr("ProjectItemExtra");
        user->SetType(*type);

        desc.Reset(new CAnnotdesc());
        desc->SetUser(*user);
        pi.SetDescr().push_back(desc);
    }

    CRef<CUser_field> field;

    for (auto f : desc->GetUser().GetData()) {
        const CObject_id& label = f->GetLabel();
        if (label.IsStr() && label.GetStr() == tag) {
            field.Reset(f);
            break;
        }
    }

    if (!field) {
        field.Reset(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr(tag);
        field->SetLabel(*label);
        desc->SetUser().SetData().push_back(field);
    }

    field->SetString(value);
}



END_NCBI_SCOPE
