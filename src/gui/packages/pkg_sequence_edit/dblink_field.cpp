/*  $Id: dblink_field.cpp 39649 2017-10-24 15:22:12Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objtools/edit/seqid_guesser.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/util_cmds.hpp>

#include <gui/packages/pkg_sequence_edit/dblink_field.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);
USING_SCOPE(edit);


bool CDBLinkField::SetVal(CObject& object, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    CSeqdesc* seqdesc = dynamic_cast<CSeqdesc*>(&object);
    CUser_object* user = dynamic_cast<CUser_object*>(&object);

    if (seqdesc && seqdesc->IsUser()) {
        user = &(seqdesc->SetUser());
    }
    if (user && IsDBLink(*user)) {
        bool found = false;
        if (user->IsSetData()) {
            CUser_object::TData::iterator it = user->SetData().begin();
            while (it != user->SetData().end()) {
                if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()) {
                    CDBLinkFieldType::EDBLinkFieldType check = GetTypeForLabel((*it)->GetLabel().GetStr());
                    if (check == m_FieldType) {
                        rval |= SetVal(**it, newValue, existing_text);
                        found = true;
                    }
                }
                if (!(*it)->IsSetData()) {
                    it = user->SetData().erase(it);
                } else {
                    it++;
                }
            }
        }
        if (!found && (m_ConstraintFieldType != m_FieldType || !m_StringConstraint)) {
            CRef<CUser_field> new_field(new CUser_field());
            new_field->SetLabel().SetStr(GetLabelForType(m_FieldType));
            if (SetVal(*new_field, newValue, eExistingText_replace_old)) {
                user->SetData().push_back(new_field);
                rval = true;
            }
        }
        
        // if User object now has no fields, reset so it will be detected as empty
        if (user->GetData().empty()) {
            user->ResetData();
        }
    }
    return rval;
}


bool CDBLinkField::SetVal(CUser_field& field, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    
    if (field.IsSetData()) {
        if (field.GetData().IsStr()) {
            string curr_val = field.GetData().GetStr();
            if (m_ConstraintFieldType != m_FieldType || !m_StringConstraint
                || m_StringConstraint->DoesTextMatch(curr_val)) {
                if (existing_text == eExistingText_add_qual) {
                    field.SetData().SetStrs().push_back(curr_val);
                    field.SetData().SetStrs().push_back(newValue);
                    rval = true;
                } else if (AddValueToString(curr_val, newValue, existing_text)) {
                    field.SetData().SetStrs().push_back(curr_val);
                    rval = true;
                }
            }
        } else if (field.GetData().IsStrs()) {
            if ((existing_text == eExistingText_add_qual || field.GetData().GetStrs().empty())
                && (m_ConstraintFieldType != m_FieldType || !m_StringConstraint)) {
                field.SetData().SetStrs().push_back(newValue);
                rval = true;
            } else {
                NON_CONST_ITERATE(CUser_field::TData::TStrs, s, field.SetData().SetStrs()) {
                    if (m_ConstraintFieldType != m_FieldType || !m_StringConstraint
                        || m_StringConstraint->DoesTextMatch(*s)) {
                        rval |= AddValueToString(*s, newValue, existing_text);
                    }
                }
            }
        } else if (field.GetData().Which() == CUser_field::TData::e_not_set) {
            if (m_ConstraintFieldType != m_FieldType || !m_StringConstraint) {
                field.SetData().SetStrs().push_back(newValue);
                rval = true;
            }
        }
    } else if (m_ConstraintFieldType != m_FieldType || !m_StringConstraint) {
        field.SetData().SetStrs().push_back(newValue);
        rval = true;
    }

    return rval;
}


string CDBLinkField::GetVal( const CObject& object)
{
    vector<string> vals = GetVals(object);
    if (vals.size() > 0) {
        return vals[0];
    } else {
        return "";
    }

}


vector<string> CDBLinkField::GetVals(const CObject& object)
{
    vector<string> vals;
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(&object);
    const CUser_object* user = dynamic_cast<const CUser_object*>(&object);

    if (seqdesc && seqdesc->IsUser()) {
        user = &(seqdesc->GetUser());
    }
    if (IsDBLink(*user) && user->IsSetData()) {
        CUser_object::TData::const_iterator it = user->GetData().begin();
        while (it != user->GetData().end()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr() && (*it)->IsSetData()) {
                CDBLinkFieldType::EDBLinkFieldType check = GetTypeForLabel((*it)->GetLabel().GetStr());
                if (check == m_FieldType) {
                    switch((*it)->GetData().Which()) {
                        case CUser_field::TData::e_Str:
                            vals.push_back((*it)->GetData().GetStr());
                            break;
                        case CUser_field::TData::e_Strs:
                            ITERATE(CUser_field::TData::TStrs, s, (*it)->GetData().GetStrs()) {
                                vals.push_back(*s);
                            }
                            break;
                        default:
                            //not handling other types of fields
                            break;
                    }
                }
            }
            ++it;
        }
    }
    return vals;
}


string CDBLinkField::GetLabel() const
{
    return "DBLink " + GetLabelForType(m_FieldType);
}


void CDBLinkField::SetConstraint(const string& field_name, CConstRef<edit::CStringConstraint> string_constraint)
{
    // constraints only apply for authors, since there could be more than one
    m_ConstraintFieldType = GetTypeForLabel(field_name);
    if (m_ConstraintFieldType == CDBLinkFieldType::eDBLinkFieldType_Unknown || !string_constraint) {
        string_constraint.Reset(NULL);
    } else {
        m_StringConstraint = new CStringConstraint(" ");
        m_StringConstraint->Assign(*string_constraint);
    }
}


CDBLinkFieldType::EDBLinkFieldType CDBLinkField::GetTypeForLabel(string label)
{
    return CDBLinkFieldType::GetTypeForLabel(label);
}


string CDBLinkField::GetLabelForType(CDBLinkFieldType::EDBLinkFieldType field_type)
{
    return CDBLinkFieldType::GetLabelForType(field_type);
}


vector<CConstRef<CObject> > CDBLinkField::GetObjects(CBioseq_Handle bsh)
{
    vector<CConstRef<CObject> > objects;

    CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_User);
    while (desc_ci) {
        if (IsDBLink(desc_ci->GetUser())) {
            CConstRef<CObject> object;
            object.Reset(&(*desc_ci));
            objects.push_back(object);
        }
        ++desc_ci;
    }

    return objects;
}


vector<CRef<CApplyObject> > CDBLinkField::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objects;

    // add existing descriptors
    CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_User);
    while (desc_ci) {
        if (IsDBLink(desc_ci->GetUser())) {
            CRef<CApplyObject> obj(new CApplyObject(bsh, *desc_ci));
            objects.push_back(obj);
        }
        ++desc_ci;
    }

    if (objects.empty()) {
        CRef<CApplyObject> new_obj(new CApplyObject(bsh, "DBLink"));
        objects.push_back(new_obj);
    }        

    return objects;
}


vector<CConstRef<CObject> > CDBLinkField::GetObjects(CSeq_entry_Handle seh, const string& constraint_field, CRef<edit::CStringConstraint> string_constraint)
{
    vector<CConstRef<CObject> > objs;
    CRef<CScope> scope(&seh.GetScope());

    CBioseq_CI bi (seh, objects::CSeq_inst::eMol_na);
    while (bi) {
        if (NStr::EqualNocase(constraint_field, kFieldTypeSeqId)) {
            if (edit::CSeqIdGuesser::DoesSeqMatchConstraint(*bi, string_constraint)) {
                vector<CConstRef<CObject> > these_objs = GetObjects(*bi);
                objs.insert(objs.end(), these_objs.begin(), these_objs.end());
            }
        } else {
            vector<CConstRef<CObject> > these_objs = GetObjects(*bi);
            ITERATE (vector<CConstRef<CObject> >, it, these_objs) {
                if (DoesObjectMatchFieldConstraint (**it, constraint_field, string_constraint, scope)) {
                    objs.push_back (*it);
                }
            }
        }
        ++bi;
    }

    return objs;
}


bool CDBLinkField::IsEmpty(const CObject& object) const
{
    bool rval = false;
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(&object);
    const CUser_object* user = dynamic_cast<const CUser_object*>(&object);
    if (seqdesc && seqdesc->IsUser()) {
        user = &(seqdesc->GetUser());
    }
    if (user && IsDBLink(*user)) {
        if (!user->IsSetData() || user->GetData().empty()) {
            rval = true;
        }
    }

    return rval;
}


void CDBLinkField::ClearVal(CObject& object)
{
    CSeqdesc* seqdesc = dynamic_cast<CSeqdesc*>(&object);
    CUser_object* user = dynamic_cast<CUser_object*>(&object);

    if (seqdesc && seqdesc->IsUser()) {
        user = &(seqdesc->SetUser());
    }
    if (user && user->IsSetData()) {
        CUser_object::TData::iterator it = user->SetData().begin();
        while (it != user->SetData().end()) {
            bool do_erase = false;
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()) {
                CDBLinkFieldType::EDBLinkFieldType check = GetTypeForLabel((*it)->GetLabel().GetStr());
                if (check == m_FieldType) {
                    do_erase = true;
                }
            }
            if (do_erase) {
                it = user->SetData().erase(it);
            } else {
                it++;
            }
        }
        if (user->GetData().empty()) {
            user->ResetData();
        }
    }
}


vector<CConstRef<CObject> > CDBLinkField::GetRelatedObjects(const CObject& object, CRef<CScope> scope)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&object);

   if (obj_feat) {
        // find closest related DBLink User Objects
        CBioseq_Handle bsh = scope->GetBioseqHandle(obj_feat->GetLocation());
        related = GetObjects(bsh);
    } else if (obj_desc) {
        if (obj_desc->IsUser() && IsDBLink(obj_desc->GetUser())) {
            CConstRef<CObject> obj(obj_desc);
            related.push_back(obj);
        } else {
            CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(scope, *obj_desc);
            related = GetObjects(seh, GetLabelForType(m_ConstraintFieldType), m_StringConstraint);
        }
    }

    return related;
}


vector<CConstRef<CObject> > CDBLinkField::GetRelatedObjects(const CApplyObject& object)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&object);

   if (obj_feat) {
        related = GetObjects(object.GetSEH(), "", CRef<edit::CStringConstraint>(NULL));
    } else if (obj_desc) {
        if (obj_desc->IsUser() && IsDBLink(obj_desc->GetUser())) {
            CConstRef<CObject> obj(obj_desc);
            related.push_back(obj);
        } else {
            related = GetObjects(object.GetSEH(), GetLabelForType(m_ConstraintFieldType), m_StringConstraint);
        }
    }

    return related;
}


bool CDBLinkField::IsDBLink (const CUser_object& user)
{
    if (user.IsSetType()
        && user.GetType().IsStr()
        && NStr::EqualNocase(user.GetType().GetStr(),"DBLink")) {
        return true;
    } else {
        return false;
    }
}


void CDBLinkField::NormalizeDBLinkFieldName(string& orig_label)
{
    CDBLinkFieldType::NormalizeDBLinkFieldName(orig_label);
}


vector<string> CDBLinkField::GetFieldNames()
{
    return CDBLinkFieldType::GetFieldNames();
}


END_NCBI_SCOPE
