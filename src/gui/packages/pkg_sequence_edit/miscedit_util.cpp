/*  $Id: miscedit_util.cpp 42449 2019-02-27 20:41:38Z asztalos $
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

#include <gui/framework/workbench_impl.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/core/project_tree_view.hpp>

#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/seqloc/Seq_loc_mix.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqblock/GB_block.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objects/valid/Comment_set.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/seq_entry_ci.hpp>

#include <util/sequtil/sequtil_convert.hpp>

#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>
#include <objtools/format/flat_expt.hpp>

#include <objtools/edit/seqid_guesser.hpp>
#include <objtools/edit/cds_fix.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/util_cmds.hpp>

#include <gui/widgets/edit/singleauthor_panel.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/molinfoedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/dblink_field.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/dblink_fieldtype.hpp>
#include <gui/widgets/edit/molinfo_fieldtype.hpp>
#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);
USING_SCOPE(edit);


CRef<CFieldHandler>
CFieldHandlerFactory::Create(const string& field_name)
{
    if (CMiscSeqTableColumn::s_IsSequenceIDField(field_name)) {
        return CRef<CFieldHandler>(new CSeqIdField());
    }
    CPubFieldType::EPubFieldType pub_field = CPubFieldType::GetTypeForLabel(field_name);
    if (pub_field != CPubFieldType::ePubFieldType_Unknown) {
        return CRef<CFieldHandler>(new CPubField(pub_field));
    }
    CDBLinkFieldType::EDBLinkFieldType dblink_field = CDBLinkFieldType::GetTypeForLabel(field_name);
    if (dblink_field != CDBLinkFieldType::eDBLinkFieldType_Unknown) {
        return CRef<CFieldHandler>(new CDBLinkField(dblink_field));
    }
    CMolInfoFieldType::EMolInfoFieldType molinfo_field = CMolInfoFieldType::GetFieldType(field_name);
    if (molinfo_field != CMolInfoFieldType::e_Unknown) {
        return CRef<CFieldHandler>(new CMolInfoField(molinfo_field));
    }

    if (NStr::EqualNocase(field_name, kGenomeProjectID)) {
        return CRef<CFieldHandler>(new CGenomeProjectField());
    }
    if (QualifierNamesAreEquivalent(field_name, kCommentDescriptorLabel)) {
        return CRef<CFieldHandler>(new CCommentDescField());
    }
    if (QualifierNamesAreEquivalent(field_name, kDefinitionLineLabel)) {
        return CRef<CFieldHandler>(new CDefinitionLineField());
    }
    if (NStr::EqualNocase(field_name, kGenbankBlockKeyword)) {
        return CRef<CFieldHandler>(new CGenbankKeywordField());
    }
    if (NStr::EqualNocase(field_name, kBankITComment)) {
        return CRef<CFieldHandler>(new CBankITCommentField());
    }
    if (NStr::EqualNocase(field_name, kComment)) {
        return CRef<CFieldHandler>(new CGeneralCommentField());
    }
    if (NStr::EqualNocase(field_name, kFlatFile)){
        return CRef<CFieldHandler>(new CFlatFileField());
    }
    if (NStr::EqualNocase(field_name, kLocalId)) {
        return CRef<CFieldHandler>(new CLocalIDField());
    }
    if (NStr::EqualNocase(field_name, kFileSeqId)) {
        return CRef<CFieldHandler>(new CFileIDField());
    }
    if (NStr::StartsWith(field_name, CGeneralIDField::GetName_GeneralIDPart(CGeneralIDField::eGenIdTag))) {
        string tag_name = CGeneralIDField::GetName_GeneralIDPart(CGeneralIDField::eGenIdTag);
        string db_name = field_name.substr(tag_name.length());
        NStr::TruncateSpacesInPlace(db_name);
        return CRef<CFieldHandler>(new CGeneralIDField(CGeneralIDField::eGenIdTag, db_name));
    }
    CGeneralIDField::EGeneralIDPart genId_field = CGeneralIDField::GetGeneralIDPart_FromName(field_name);
    if (genId_field != CGeneralIDField::eUnknown) {
        return CRef<CFieldHandler>(new CGeneralIDField(genId_field));
    }
    if (NStr::EqualNocase(field_name, kStructCommDBName)) {
        return CRef<CFieldHandler>(new CStructCommDBName());
    }
    if (NStr::EqualNocase(field_name, kStructCommFieldName)) {
        return CRef<CFieldHandler>(new CStructCommFieldName());
    }
    if (NStr::StartsWith(field_name, kStructCommFieldValuePair, NStr::eNocase)) {
        string field = field_name.substr(string("Structured comment").length() + 1, NPOS);
        string label = field.substr(string("Field").length() + 1, NPOS);
        return CRef<CFieldHandler>(new CStructCommFieldValuePair(label));
    }

    // empty
    CRef<CFieldHandler> empty;
    return empty;
}


// class CTextDescriptorField
vector<CConstRef<CObject> > CTextDescriptorField::GetObjects(CBioseq_Handle bsh)
{
    vector< CConstRef<CObject> > rval;
    CSeqdesc_CI desc_ci(bsh, m_Subtype);
    while (desc_ci) {
        CConstRef<CObject> o(&(*desc_ci));
        rval.push_back(o);
        ++desc_ci;
    }
    return rval;
}

vector<CConstRef<CObject> > CTextDescriptorField::GetObjects(CSeq_entry_Handle seh, const string& constraint_field, CRef<edit::CStringConstraint> string_constraint)
{
    vector<CConstRef<CObject> > objs;
    CRef<CScope> scope(&seh.GetScope());

    CBioseq_CI bi(seh, objects::CSeq_inst::eMol_na);
    while (bi) {
        if (NStr::EqualNocase(constraint_field, kFieldTypeSeqId)) {
            if (edit::CSeqIdGuesser::DoesSeqMatchConstraint(*bi, string_constraint)) {
                vector<CConstRef<CObject> > these_objs = GetObjects(*bi);
                objs.insert(objs.end(), these_objs.begin(), these_objs.end());
            }
        }
        else {
            vector<CConstRef<CObject> > these_objs = GetObjects(*bi);
            ITERATE(vector<CConstRef<CObject> >, it, these_objs) {
                if (DoesObjectMatchFieldConstraint(**it, constraint_field, string_constraint, scope)) {
                    objs.push_back(*it);
                }
            }
        }
        ++bi;
    }

    return objs;
}

vector<CRef<edit::CApplyObject> > CTextDescriptorField::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objects;

    // add existing descriptors
    CSeqdesc_CI desc_ci(bsh, m_Subtype);
    while (desc_ci) {
        CRef<CApplyObject> new_obj(new CApplyObject(bsh, *desc_ci));
        objects.push_back(new_obj);
        ++desc_ci;
    }

    if (objects.empty()) {
        CRef<CApplyObject> new_obj(new CApplyObject(bsh, m_Subtype));
        objects.push_back(new_obj);
    }

    return objects;
}


vector<CConstRef<CObject> > CTextDescriptorField::GetRelatedObjects(const CObject& object, CRef<CScope> scope)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&object);

    if (obj_feat) {
        // find closest related DBLink User Objects
        CBioseq_Handle bsh = scope->GetBioseqHandle(obj_feat->GetLocation());
        related = GetObjects(bsh);
    }
    else if (obj_desc) {
        if (obj_desc->Which() == m_Subtype) {
            CConstRef<CObject> obj(obj_desc);
            related.push_back(obj);
        }
        else {
            CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(scope, *obj_desc);
            related = GetObjects(seh, "", CRef<edit::CStringConstraint>(NULL));
        }
    }

    return related;
}

vector<CConstRef<CObject> > CTextDescriptorField::GetRelatedObjects(const CApplyObject& object)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&(object.GetObject()));
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&(object.GetObject()));
    const CSeq_inst * inst = dynamic_cast<const CSeq_inst *>(&(object.GetObject()));
    const CBioseq * bioseq = dynamic_cast<const CBioseq *>(&(object.GetObject()));

    if (obj_feat) {
        related = GetObjects(object.GetSEH(), "", CRef<edit::CStringConstraint>(NULL));
    }
    else if (obj_desc) {
        if (obj_desc->Which() == m_Subtype) {
            CConstRef<CObject> obj(obj_desc);
            related.push_back(obj);
        }
        else {
            related = GetObjects(object.GetSEH(), "", CRef<edit::CStringConstraint>(NULL));
        }
    }
    else if (bioseq || inst) {
        related = GetObjects(object.GetSEH(), "", CRef<edit::CStringConstraint>(NULL));
    }

    return related;
}


// class CCommentDescField

string CCommentDescField::GetVal(const CObject& object)
{
    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    if (obj_desc && obj_desc->IsComment()) {
        return obj_desc->GetComment();
    }
    else {
        return "";
    }
}

vector<string> CCommentDescField::GetVals(const CObject& object)
{
    vector<string> vals;
    vals.push_back(GetVal(object));
    return vals;
}


bool CCommentDescField::IsEmpty(const CObject& object) const
{
    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    if (obj_desc && obj_desc->IsComment() && !NStr::IsBlank(obj_desc->GetComment())) {
        return true;
    }
    else {
        return false;
    }
}


void CCommentDescField::ClearVal(CObject& object)
{
    CSeqdesc * obj_desc = dynamic_cast<CSeqdesc *>(&object);
    if (obj_desc) {
        obj_desc->SetComment(" ");
    }
}


bool CCommentDescField::SetVal(CObject& object, const string& val, EExistingText existing_text)
{
    bool rval = false;
    CSeqdesc * obj_desc = dynamic_cast<CSeqdesc *>(&object);
    if (!obj_desc) {
        return false;
    }
    string curr_val = "";
    if (obj_desc->IsComment()) {
        curr_val = obj_desc->GetComment();
    }
    if (AddValueToString(curr_val, val, existing_text)) {
        obj_desc->SetComment(curr_val);
        rval = true;
    }
    return rval;
}


// class CDefinitionLineField

string CDefinitionLineField::GetVal(const CObject& object)
{
    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    if (obj_desc && obj_desc->IsTitle()) {
        return obj_desc->GetTitle();
    }
    else {
        return "";
    }
}


vector<string> CDefinitionLineField::GetVals(const CObject& object)
{
    vector<string> vals;
    vals.push_back(GetVal(object));
    return vals;
}


bool CDefinitionLineField::IsEmpty(const CObject& object) const
{
    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    if (obj_desc && obj_desc->IsTitle() && !NStr::IsBlank(obj_desc->GetTitle())) {
        return true;
    }
    else {
        return false;
    }
}


void CDefinitionLineField::ClearVal(CObject& object)
{
    CSeqdesc * obj_desc = dynamic_cast<CSeqdesc *>(&object);
    if (obj_desc) {
        obj_desc->SetTitle(" ");
    }
}


bool CDefinitionLineField::SetVal(CObject& object, const string& val, EExistingText existing_text)
{
    bool rval = false;
    CSeqdesc * obj_desc = dynamic_cast<CSeqdesc *>(&object);
    if (!obj_desc) {
        return false;
    }
    string curr_val = "";
    if (obj_desc->IsTitle()) {
        curr_val = obj_desc->GetTitle();
    }
    if (AddValueToString(curr_val, val, existing_text)) {
        obj_desc->SetTitle(curr_val);
        rval = true;
    }
    return rval;
}


// class CGenbankKeywordField

string CGenbankKeywordField::GetVal(const CObject& object)
{
    vector<string> vals = GetVals(object);
    return (vals.empty()) ? kEmptyStr : vals[0];
}


vector<string> CGenbankKeywordField::GetVals(const CObject& object)
{
    vector<string> rval;

    const CSeqdesc* desc = dynamic_cast<const CSeqdesc *>(&object);
    if (desc && desc->IsGenbank() && desc->GetGenbank().IsSetKeywords()) {
        ITERATE(CGB_block::TKeywords, it, desc->GetGenbank().GetKeywords()) {
            rval.push_back(*it);
        }
    }
    return rval;
}


bool CGenbankKeywordField::IsEmpty(const CObject& object) const
{
    const CSeqdesc* desc = dynamic_cast<const CSeqdesc *>(&object);
    if (!desc || !desc->IsGenbank()) {
        return false;
    }
    const CGB_block& block = desc->GetGenbank();
    if (!block.IsSetDate()
        && !block.IsSetDiv()
        && !block.IsSetEntry_date()
        && !block.IsSetExtra_accessions()
        && !block.IsSetKeywords()
        && !block.IsSetOrigin()
        && !block.IsSetSource()
        && !block.IsSetTaxonomy()) {
        return true;
    }
    else {
        return false;
    }
}


void CGenbankKeywordField::ClearVal(CObject& object)
{
    CSeqdesc* desc = dynamic_cast<CSeqdesc *>(&object);
    if (!desc || !desc->IsGenbank()) {
        return;
    }
    desc->SetGenbank().ResetKeywords();
}


bool CGenbankKeywordField::SetVal(CObject& object, const string& val, EExistingText existing_text)
{
    bool rval = false;

    CSeqdesc* desc = dynamic_cast<CSeqdesc *>(&object);
    if (!desc) {
        return false;
    }
    if (!desc->IsGenbank()
        || !desc->GetGenbank().IsSetKeywords()
        || desc->GetGenbank().GetKeywords().empty()
        || existing_text == eExistingText_add_qual) {
        desc->SetGenbank().SetKeywords().push_back(val);
        rval = true;
    }
    else {
        CGB_block::TKeywords::iterator it = desc->SetGenbank().SetKeywords().begin();
        while (it != desc->SetGenbank().SetKeywords().end()) {
            string curr_val = *it;
            if (!m_StringConstraint || m_StringConstraint->DoesTextMatch(curr_val)) {
                if (AddValueToString(curr_val, val, existing_text)) {
                    *it = curr_val;
                    rval = true;
                }
            }
            if (NStr::IsBlank(*it)) {
                it = desc->SetGenbank().SetKeywords().erase(it);
            }
            else {
                ++it;
            }
        }
    }
    return rval;
}


void CGenbankKeywordField::SetConstraint(const string& field, CConstRef<edit::CStringConstraint> string_constraint)
{
    if (NStr::EqualNocase(field, kGenbankBlockKeyword) && string_constraint) {
        m_StringConstraint = new edit::CStringConstraint(" ");
        m_StringConstraint->Assign(*string_constraint);
    }
    else {
        m_StringConstraint.Reset(NULL);
    }
}



// class CGenomeProjectField

vector<CConstRef<CObject> > CGenomeProjectField::GetObjects(CBioseq_Handle bsh)
{
    vector<CConstRef<CObject> > objects;

    CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_User);
    while (desc_ci) {
        if (IsGenomeProject(desc_ci->GetUser())) {
            CConstRef<CObject> object;
            object.Reset(&(*desc_ci));
            objects.push_back(object);
        }
        ++desc_ci;
    }

    return objects;
}


vector<CConstRef<CObject> > CGenomeProjectField::GetObjects(CSeq_entry_Handle seh, const string& constraint_field, CRef<CStringConstraint> string_constraint)
{
    vector<CConstRef<CObject> > objs;
    CRef<CScope> scope(&seh.GetScope());

    CBioseq_CI bi(seh, objects::CSeq_inst::eMol_na);
    while (bi) {
        if (NStr::EqualNocase(constraint_field, kFieldTypeSeqId)) {
            if (edit::CSeqIdGuesser::DoesSeqMatchConstraint(*bi, string_constraint)) {
                vector<CConstRef<CObject> > these_objs = GetObjects(*bi);
                objs.insert(objs.end(), these_objs.begin(), these_objs.end());
            }
        }
        else {
            vector<CConstRef<CObject> > these_objs = GetObjects(*bi);
            ITERATE(vector<CConstRef<CObject> >, it, these_objs) {
                if (DoesObjectMatchFieldConstraint(**it, constraint_field, string_constraint, scope)) {
                    objs.push_back(*it);
                }
            }
        }
        ++bi;
    }

    return objs;
}


vector<CRef<CApplyObject> > CGenomeProjectField::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > apply_objs;
    vector<CConstRef<CObject> > objs = GetObjects(bsh);
    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeqdesc* orig_desc = dynamic_cast<const CSeqdesc*>((*it).GetPointer());
        CRef<CApplyObject> new_obj(new CApplyObject(bsh, *orig_desc));
        apply_objs.push_back(new_obj);
    }
    if (apply_objs.empty()) {
        CRef<CApplyObject> new_obj(new CApplyObject(bsh, "GenomeProjectsDB"));
        apply_objs.push_back(new_obj);
    }

    return apply_objs;
}


string CGenomeProjectField::GetVal(const CObject& object)
{
    string val = "";

    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(&object);
    const CUser_object* user = dynamic_cast<const CUser_object*>(&object);

    if (seqdesc && seqdesc->IsUser()) {
        user = &(seqdesc->GetUser());
    }
    if (IsGenomeProject(*user) && user->IsSetData()) {
        CUser_object::TData::const_iterator it = user->GetData().begin();
        while (it != user->GetData().end()) {
            if (IsGenomeProjectID(**it)
                && (*it)->IsSetData() && (*it)->GetData().IsInt()) {
                val = NStr::NumericToString((*it)->GetData().GetInt());
                break;
            }
            ++it;
        }
    }
    return val;
}


vector<string> CGenomeProjectField::GetVals(const CObject& object)
{
    vector<string> vals;
    vals.push_back(GetVal(object));
    return vals;
}


bool CGenomeProjectField::IsEmpty(const CObject& object) const
{
    bool rval = false;
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(&object);
    const CUser_object* user = dynamic_cast<const CUser_object*>(&object);
    if (seqdesc && seqdesc->IsUser()) {
        user = &(seqdesc->GetUser());
    }
    if (user && IsGenomeProject(*user)) {
        if (!user->IsSetData() || user->GetData().empty()) {
            rval = true;
        }
    }

    return rval;
}


void CGenomeProjectField::ClearVal(CObject& object)
{
    CSeqdesc* seqdesc = dynamic_cast<CSeqdesc*>(&object);
    CUser_object* user = dynamic_cast<CUser_object*>(&object);

    if (seqdesc && seqdesc->IsUser()) {
        user = &(seqdesc->SetUser());
    }
    if (user && user->IsSetData()) {
        CUser_object::TData::iterator it = user->SetData().begin();
        while (it != user->SetData().end()) {
            if (IsGenomeProjectID(**it)) {
                it = user->SetData().erase(it);
            }
            else {
                it++;
            }
        }
        if (user->GetData().empty()) {
            user->ResetData();
        }
    }
}


static const string kGenomeProjectId = "ProjectID";

bool CGenomeProjectField::IsGenomeProjectID(const CUser_field& field)
{
    if (field.IsSetLabel() && field.GetLabel().IsStr()
        && NStr::EqualNocase(field.GetLabel().GetStr(), kGenomeProjectId)) {
        return true;
    }
    else {
        return false;
    }
}


bool CGenomeProjectField::SetVal(CObject& object, const string& val, EExistingText existing_text)
{
    bool rval = false;
    CSeqdesc* seqdesc = dynamic_cast<CSeqdesc*>(&object);
    CUser_object* user = dynamic_cast<CUser_object*>(&object);

    if (seqdesc && seqdesc->IsUser()) {
        user = &(seqdesc->SetUser());
    }
    if (user && IsGenomeProject(*user)) {
        bool found = false;
        if (user->IsSetData()) {
            CUser_object::TData::iterator it = user->SetData().begin();
            while (it != user->SetData().end()) {
                if (IsGenomeProjectID(**it)) {
                    string curr_val = "";
                    bool can_change = false;
                    if ((*it)->IsSetData()) {
                        if ((*it)->GetData().IsStr()) {
                            curr_val = (*it)->GetData().GetStr();
                            can_change = true;
                        }
                        else if ((*it)->GetData().IsInt()) {
                            curr_val = NStr::NumericToString((*it)->GetData().GetInt());
                            can_change = true;
                        }
                    }
                    if (can_change && AddValueToString(curr_val, val, existing_text)) {
                        try {
                            int numval = NStr::StringToInt(curr_val);
                            (*it)->SetData().SetInt(numval);
                            rval = true;
                        }
                        catch (const exception&) {
                        }
                    }
                    found = true;
                }
                ++it;
            }
        }
        if (!found) {
            try {
                int numval = NStr::StringToInt(val);
                CRef<CUser_field> new_field(new CUser_field());
                new_field->SetLabel().SetStr(kGenomeProjectId);
                new_field->SetData().SetInt(numval);
                user->SetData().push_back(new_field);
                rval = true;
            }
            catch (const exception&) {
            }
        }

    }
    return rval;
}


string CGenomeProjectField::IsValid(const string& val)
{
    string rval = "Genome Project ID should be a number";
    try {
        int numval = NStr::StringToInt(val);
        rval = "";
    }
    catch (const exception&) {
    }
    return rval;
}


vector<string> CGenomeProjectField::IsValid(const vector<string>& values)
{
    vector<string> rval;
    ITERATE(vector<string>, it, values) {
        rval.push_back(IsValid(*it));
    }
    return rval;
}


vector<CConstRef<CObject> > CGenomeProjectField::GetRelatedObjects(const CObject& object, CRef<CScope> scope)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&object);
    const CBioseq * bioseq = dynamic_cast<const CBioseq *>(&object);

    if (obj_feat) {
        // find closest related DBLink User Objects
        CBioseq_Handle bsh = scope->GetBioseqHandle(obj_feat->GetLocation());
        related = GetObjects(bsh);
    }
    else if (obj_desc) {
        if (obj_desc->IsUser() && IsGenomeProject(obj_desc->GetUser())) {
            CConstRef<CObject> obj(obj_desc);
            related.push_back(obj);
        }
        else {
            CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(scope, *obj_desc);
            related = GetObjects(seh, "", CRef<CStringConstraint>(NULL));
        }
    }
    else if (bioseq) {
        CBioseq_Handle bsh = scope->GetBioseqHandle(*bioseq);
        related = GetObjects(bsh);
    }

    return related;
}


vector<CConstRef<CObject> > CGenomeProjectField::GetRelatedObjects(const CApplyObject& object)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&(object.GetObject()));
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&(object.GetObject()));

    if (obj_feat) {
        // find closest related DBLink User Objects
        CBioseq_Handle bsh = object.GetSEH().GetScope().GetBioseqHandle(obj_feat->GetLocation());
        related = GetObjects(bsh);
    }
    else if (obj_desc) {
        if (obj_desc->IsUser() && IsGenomeProject(obj_desc->GetUser())) {
            CConstRef<CObject> obj(obj_desc);
            related.push_back(obj);
        }
        else {
            related = GetObjects(object.GetSEH(), "", CRef<CStringConstraint>(NULL));
        }
    }

    return related;
}


bool CGenomeProjectField::IsGenomeProject(const CUser_object& user)
{
    if (user.IsSetType()
        && user.GetType().IsStr()
        && NStr::EqualNocase(user.GetType().GetStr(), "GenomeProjectsDB")) {
        return true;
    }
    else {
        return false;
    }
}


 // class CBankITCommentField
vector<CConstRef<CObject> > CBankITCommentField::GetObjects(CBioseq_Handle bsh)
{
    vector<CConstRef<CObject> > objects;
    objects.push_back(CConstRef<CObject>(bsh.GetCompleteBioseq()));
    return objects;
}

vector<CConstRef<CObject> > CBankITCommentField::GetObjects(CSeq_entry_Handle seh,
    const string& constraint_field, CRef<edit::CStringConstraint> string_constraint)
{
    vector<CConstRef<CObject> > objects;
    CBioseq_CI bi(seh, objects::CSeq_inst::eMol_na);
    CRef<CScope> scope(&seh.GetScope());
    while (bi) {
        CConstRef<CObject> obj(bi->GetCompleteBioseq());
        if (CMiscSeqTableColumn::s_IsSequenceIDField(constraint_field)) {
            if (edit::CSeqIdGuesser::DoesSeqMatchConstraint(*bi, string_constraint)) {
                objects.push_back(obj);
            }
        }
        else if (DoesObjectMatchFieldConstraint(*obj, constraint_field, string_constraint, scope)) {
            objects.push_back(obj);
        }
        ++bi;
    }

    return objects;
}

vector<CRef<CApplyObject> > CBankITCommentField::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objects;
    objects.push_back(CRef<CApplyObject>(new CApplyObject(bsh)));
    return objects;
}

vector<CConstRef<CObject> > CBankITCommentField::GetRelatedObjects(const CObject& object, CRef<CScope> scope)
{
    vector<CConstRef<CObject> > objects;
    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&object);
    const CBioseq * bioseq = dynamic_cast<const CBioseq *>(&object);
    if (obj_desc) {
        CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(scope, *obj_desc);
        CBioseq_CI bi(seh, objects::CSeq_inst::eMol_na);
        while (bi) {
            objects.push_back(CConstRef<CObject>(bi->GetCompleteBioseq()));
            ++bi;
        }
    } else if (obj_feat) {
        CBioseq_Handle bsh = scope->GetBioseqHandle(*(obj_feat->GetLocation().GetId()));
        if (bsh) {
            objects.push_back(CConstRef<CObject>(bsh.GetCompleteBioseq()));
        }
    } else if (bioseq) {
        objects.push_back(CConstRef<CObject>(bioseq));
    }

    return objects;
}
vector<CConstRef<CObject> > CBankITCommentField::GetRelatedObjects(const CApplyObject& object)
{
    vector<CConstRef<CObject> > objects;
    CBioseq_CI bi(object.GetSEH(), objects::CSeq_inst::eMol_na);
    while (bi) {
        objects.push_back(CConstRef<CObject>(bi->GetCompleteBioseq()));
        ++bi;
    }
    return objects;
}


string CBankITCommentField::GetVal(const CObject& object)
{
    vector<string> vals = GetVals(object);
    return (vals.empty()) ? kEmptyStr : vals[0];
}

vector<string> CBankITCommentField::GetVals(const CObject& object)
{
    vector<string> vals;
    const CBioseq* bioseq = dynamic_cast<const CBioseq *>(&object);
    CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*bioseq);
    if (!bsh) {
        return vals;
    }

    for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_User); desc_it; ++desc_it) {
        const CUser_object& user_obj = desc_it->GetUser();
        if (user_obj.IsSetClass() && NStr::EqualCase(user_obj.GetClass(), "SMART_V1.0"))
            continue;

        if (user_obj.GetType().IsStr() && NStr::EqualCase(user_obj.GetType().GetStr(), "Submission")) {
            ITERATE(CUser_object::TData, data_it, user_obj.GetData()) {
                const CUser_field& field = data_it->GetObject();
                if (field.IsSetLabel()
                    && field.GetLabel().IsStr()
                    && NStr::EqualCase(field.GetLabel().GetStr(), "AdditionalComment")) {

                    if (field.IsSetData() && field.GetData().IsStr()) {
                        vals.push_back(field.GetData().GetStr());
                    }
                }
            }
        }
    }
    return vals;
}

// class CGeneralCommentField
vector<string> CGeneralCommentField::GetVals(const CObject& object)
{
    vector<string> vals;
    const CBioseq* bioseq = dynamic_cast<const CBioseq *>(&object);
    CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*bioseq);
    if (!bsh) {
        return vals;
    }

    // gather the comment descriptors
    for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_Comment); desc_it; ++desc_it) {
        vals.push_back(desc_it->GetComment());
    }

    // gather the BankIt comments
    vector<string> bankit = CBankITCommentField::GetVals(object);
    copy(bankit.begin(), bankit.end(), back_inserter(vals));
    return vals;
}

// class CSeqIdField

vector<CConstRef<CObject> > CSeqIdField::GetObjects(CBioseq_Handle bsh)
{
    vector<CConstRef<CObject> > objects;
    objects.push_back(CConstRef<CObject>(bsh.GetCompleteBioseq()));
    return objects;
}


vector<CConstRef<CObject> > CSeqIdField::GetObjects
(CSeq_entry_Handle seh,
const string& constraint_field,
CRef<edit::CStringConstraint> string_constraint)
{
    vector<CConstRef<CObject> > objects;
    CBioseq_CI bi(seh, objects::CSeq_inst::eMol_na);
    CRef<CScope> scope(&seh.GetScope());
    while (bi) {
        CConstRef<CObject> obj(bi->GetCompleteBioseq());
        if (CMiscSeqTableColumn::s_IsSequenceIDField(constraint_field)) {
            if (edit::CSeqIdGuesser::DoesSeqMatchConstraint(*bi, string_constraint)) {
                objects.push_back(obj);
            }
        }
        else if (DoesObjectMatchFieldConstraint(*obj,
            constraint_field, string_constraint,
            scope)) {
            objects.push_back(obj);
        }
        ++bi;
    }

    return objects;
}


vector<CRef<CApplyObject> > CSeqIdField::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objects;
    objects.push_back(CRef<CApplyObject>(new CApplyObject(bsh)));
    return objects;
}


vector<CConstRef<CObject> > CSeqIdField::GetRelatedObjects(const CObject& object, CRef<CScope> scope)
{
    vector<CConstRef<CObject> > objects;
    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&object);
    const CBioseq * bioseq = dynamic_cast<const CBioseq *>(&object);
    if (obj_desc) {
        CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(scope, *obj_desc);
        CBioseq_CI bi(seh, objects::CSeq_inst::eMol_na);
        while (bi) {
            objects.push_back(CConstRef<CObject>(bi->GetCompleteBioseq()));
            ++bi;
        }
    }
    else if (obj_feat) {
        CBioseq_Handle bsh = scope->GetBioseqHandle(*(obj_feat->GetLocation().GetId()));
        if (bsh) {
            objects.push_back(CConstRef<CObject>(bsh.GetCompleteBioseq()));
        }
    }
    else if (bioseq) {
        objects.push_back(CConstRef<CObject>(bioseq));
    }
    return objects;
}


vector<CConstRef<CObject> > CSeqIdField::GetRelatedObjects(const CApplyObject& object)
{
    vector<CConstRef<CObject> > objects;
    CBioseq_CI bi(object.GetSEH(), objects::CSeq_inst::eMol_na);
    while (bi) {
        objects.push_back(CConstRef<CObject>(bi->GetCompleteBioseq()));
        ++bi;
    }
    return objects;
}


string CSeqIdField::GetVal(const CObject& object)
{
    string val;
    const CBioseq* bioseq = dynamic_cast<const CBioseq *>(&object);
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat * >(&object);
    if (bioseq && bioseq->IsSetId()) {
        bioseq->GetId().front()->GetLabel(&val, CSeq_id::eContent);
    }
    else if (feat && feat->IsSetLocation()) {
        feat->GetLocation().GetId()->GetLabel(&val, CSeq_id::eContent);
    }
    return val;
}


vector<string> CSeqIdField::GetVals(const CObject& object)
{
    vector<string> vals;
    vals.push_back(GetVal(object));
    return vals;
}


// class CFileIDField

string CFileIDField::GetVal(const CObject& object)
{
    string val = kEmptyStr;
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(&object);
    if (bioseq && bioseq->IsSetId()) {
        FOR_EACH_SEQID_ON_BIOSEQ(seqid_ci, *bioseq) {
            if (x_IsNCBIFILEId(**seqid_ci)) {
                (*seqid_ci)->GetLabel(&val, CSeq_id::eContent); // returned value: NCBIFILE:fileid_content
                val = val.substr(NStr::FindNoCase(val, ":") + 1); // truncate the 'NCBIFILE:' prefix
                // assume that there is a single File ID per sequence
                break;
            }
        }
    }
    return val;
}

bool CFileIDField::IsEmpty(const CObject& object) const
{
    string value = (const_cast<CFileIDField*> (this))->GetVal(object);
    if (NStr::IsBlank(value)) {
        return true;
    }
    return false;
}

bool CFileIDField::x_IsNCBIFILEId(const CSeq_id& seq_id)
{
    if (seq_id.IsGeneral() && seq_id.GetGeneral().IsSetDb()
        && NStr::EqualCase(seq_id.GetGeneral().GetDb(), "NCBIFILE")) {
        return true;
    }
    return false;
}


// class CGeneralIDField

typedef SStaticPair<const char*, CGeneralIDField::EGeneralIDPart> TGeneralIDPart;
static const TGeneralIDPart s_GenIDPartName[] = {
        { "", CGeneralIDField::eUnknown },
        { "Entire General ID", CGeneralIDField::eGenId },
        { "General ID DB", CGeneralIDField::eGenIdDb },
        { "General ID Tag", CGeneralIDField::eGenIdTag },
};

typedef CStaticArrayMap<string, CGeneralIDField::EGeneralIDPart> TGenIdPartMap;
DEFINE_STATIC_ARRAY_MAP(TGenIdPartMap, sm_GenIdMap, s_GenIDPartName);


const string& CGeneralIDField::GetName_GeneralIDPart(CGeneralIDField::EGeneralIDPart stype_part)
{
    TGenIdPartMap::const_iterator iter = sm_GenIdMap.begin();
    for (; iter != sm_GenIdMap.end(); ++iter){
        if (iter->second == stype_part){
            return iter->first;
        }
    }
    return kEmptyStr;
}

CGeneralIDField::EGeneralIDPart CGeneralIDField::GetGeneralIDPart_FromName(const string& name)
{
    TGenIdPartMap::const_iterator iter = sm_GenIdMap.find(name);
    if (iter != sm_GenIdMap.end()){
        return iter->second;
    }
    return 	CGeneralIDField::eUnknown;
}

string CGeneralIDField::GetVal(const CObject& object)
{
    string val = kEmptyStr;
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(&object);
    if (bioseq && bioseq->IsSetId()) {
        FOR_EACH_SEQID_ON_BIOSEQ(seqid_ci, *bioseq) {
            const CSeq_id& seqid = **seqid_ci;
            if (seqid.IsGeneral()) {
                if (!NStr::IsBlank(val)) {
                    val += ";";
                }
                switch (m_GeneralIDPart) {
                case eGenId: {
                    string name = kEmptyStr;
                    seqid.GetGeneral().GetLabel(&name);
                    val += name;
                    break;
                }
                case eGenIdDb:
                    if (seqid.GetGeneral().IsSetDb()) {
                        val += seqid.GetGeneral().GetDb();
                    }
                    break;
                case eGenIdTag:
                    if (seqid.GetGeneral().IsSetTag()) {
                        if (NStr::IsBlank(m_PreferredDB) ||
                            (seqid.GetGeneral().IsSetDb() &&
                            NStr::EqualCase(seqid.GetGeneral().GetDb(), m_PreferredDB))) {
                            CNcbiOstrstream ostr;
                            seqid.GetGeneral().GetTag().AsString(ostr);
                            val += CNcbiOstrstreamToString(ostr);
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
    if (NStr::EndsWith(val, ";")) {
        val.erase(val.end() - 1);
    }
    return val;
}


// class CLocalIDField

vector<CConstRef<CObject> > CLocalIDField::GetObjects(CBioseq_Handle bsh)
{
    vector< CConstRef<CObject> > objects;
    CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_User);
    while (desc_ci){
        if (desc_ci->GetUser().GetObjectType() == CUser_object::eObjectType_OriginalId) {
            CConstRef<CObject> obj(&(*desc_ci));
            objects.push_back(obj);
        }
        ++desc_ci;
    }

    if (objects.empty()) {
        objects.push_back(CConstRef<CObject>(bsh.GetCompleteBioseq()));
    }

    return objects;
}

vector<CConstRef<CObject> > CLocalIDField::GetObjects(CSeq_entry_Handle seh,
    const string& constraint_field, CRef<edit::CStringConstraint> string_constraint)
{
    vector< CConstRef<CObject> > objects;
    return objects;
}

vector<CRef<CApplyObject> > CLocalIDField::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objects;
    CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_User);
    while (desc_ci) {
        if (desc_ci->GetUser().GetObjectType() == CUser_object::eObjectType_OriginalId) {
            CRef<CApplyObject> obj(new CApplyObject(bsh, *desc_ci));
            objects.push_back(obj);
        }
        ++desc_ci;
    }

    if (objects.empty()) {
        objects.push_back(CRef<CApplyObject>(new CApplyObject(bsh)));
    }
    return objects;
}

vector<CConstRef<CObject> > CLocalIDField::GetRelatedObjects(const CObject& object, CRef<CScope> scope)
{
    vector< CConstRef<CObject> > objects;
    return objects;
}

vector<CConstRef<CObject> > CLocalIDField::GetRelatedObjects(const CApplyObject& object)
{
    vector< CConstRef<CObject> > objects;
    return objects;
}

string CLocalIDField::GetVal(const CObject& object)
{
    vector<string> vals = GetVals(object);
    return (vals.empty()) ? kEmptyStr : vals[0];
}

vector<string> CLocalIDField::GetVals(const CObject& object)
{
    vector<string> vals;
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(&object);
    if (seqdesc && seqdesc->IsUser()) {
        const CUser_object& user = seqdesc->GetUser();
        if (user.GetObjectType() == CUser_object::eObjectType_OriginalId) {
            if (!user.GetData().empty()) {
                
                const CRef<CUser_field>& user_field = user.GetData().front();
                if (user_field->IsSetLabel() && user_field->GetLabel().IsStr()
                    && user_field->IsSetData()
                    && NStr::EqualCase(user_field->GetLabel().GetStr(), "LocalId")){
                    
                    if (user_field->GetData().IsStr()) {
                        vals.push_back(user_field->GetData().GetStr());
                    }  //not handling other types of fields
                }
            }
        }
    }

    if (!vals.empty()) {
        return vals;
    }

    const CBioseq* bioseq = dynamic_cast<const CBioseq *>(&object);
    if (bioseq && bioseq->IsSetId()) {
        const CSeq_id* lcl_id = bioseq->GetLocalId();
        if (lcl_id) {
            string val;
            lcl_id->GetLabel(&val, CSeq_id::eContent);
            vals.push_back(val);
        }
    }
    
    return vals;
}

bool CLocalIDField::IsEmpty(const CObject& object) const
{
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(&object);
    if (seqdesc && seqdesc->IsUser()) {
        const CUser_object& user = seqdesc->GetUser();
        if (user.GetObjectType() == CUser_object::eObjectType_OriginalId) {
            if (user.GetData().empty()) {
                return true;
            }

            const CRef<CUser_field>& user_field = user.GetData().front();
            if (user_field->IsSetLabel() && user_field->GetLabel().IsStr()
                && NStr::EqualCase(user_field->GetLabel().GetStr(), "LocalId")){
                if (!user_field->IsSetData()) {
                    return true;
                }
                if (!user_field->GetData().IsStr()) {
                    return true;
                }
                if (NStr::IsBlank(user_field->GetData().GetStr())) {
                    return true;
                }
            }
        }
    } else {
        const CBioseq* bioseq = dynamic_cast<const CBioseq *>(&object);
        if (bioseq && bioseq->IsSetId()) {
            const CSeq_id* lcl_id = bioseq->GetLocalId();
            if (!lcl_id) {
                return true;
            }
            string label;
            lcl_id->GetLabel(&label, CSeq_id::eContent);
            return (NStr::IsBlank(label)) ? true : false;
        }
    }
    return false;
}

// class CStructCommFieldValuePair

vector<CConstRef<CObject> > CStructCommFieldValuePair::GetObjects(CBioseq_Handle bsh)
{
    vector< CConstRef<CObject> > rval;
    CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_User);
    while (desc_ci) {
        if (CComment_rule::IsStructuredComment(desc_ci->GetUser())) {
            CConstRef<CObject> obj(&(*desc_ci));
            rval.push_back(obj);
        }
        ++desc_ci;
    }

    return rval;
}

vector<CConstRef<CObject> > CStructCommFieldValuePair::GetObjects(CSeq_entry_Handle seh, const string& constraint_field, CRef<edit::CStringConstraint> string_constraint)
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
                    objs.push_back(*it);
                }
            }
        }
        ++bi;
    }

    return objs;
}

static const string kStructComment("StructuredComment");
static const string kStructCommentPrefix("StructuredCommentPrefix");
static const string kStructCommentSuffix("StructuredCommentSuffix");

vector<CRef<CApplyObject> > CStructCommFieldValuePair::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objects;

    CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_User);
    while (desc_ci) {
        if (CComment_rule::IsStructuredComment(desc_ci->GetUser())) {
            CRef<CApplyObject> obj(new CApplyObject(bsh, *desc_ci));
            objects.push_back(obj);
        }
        ++desc_ci;
    }
    if (objects.empty()) {
        CRef<CUser_object> usr_obj(new CUser_object());
        usr_obj->SetType().SetStr(kStructComment);
        CRef<CSeqdesc> desc(new CSeqdesc());
        desc->SetUser(*usr_obj);
        CRef<CApplyObject> new_obj(new CApplyObject(bsh, *desc));
        objects.push_back(new_obj);
    }        

    return objects;
}

vector<CConstRef<CObject> > CStructCommFieldValuePair::GetRelatedObjects(const CObject& object, CRef<CScope> scope)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&object);

    if (obj_feat) {
        // find closest related Structured Comment Objects
        CBioseq_Handle bsh = scope->GetBioseqHandle(obj_feat->GetLocation());
        related = GetObjects(bsh);
    } else if (obj_desc && obj_desc->IsUser() && CComment_rule::IsStructuredComment(obj_desc->GetUser())) {
        CConstRef<CObject> obj(obj_desc);
        related.push_back(obj);
    }

    return related;
}


vector<CConstRef<CObject> > CStructCommFieldValuePair::GetRelatedObjects(const CApplyObject& object)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&object);

   if (obj_feat) {
        related = GetObjects(object.GetSEH(), "", CRef<CStringConstraint>(NULL));
    } else if (obj_desc && obj_desc->IsUser() && CComment_rule::IsStructuredComment(obj_desc->GetUser())) {
        CConstRef<CObject> obj(obj_desc);
        related.push_back(obj);
    }

    return related;
}

bool CStructCommFieldValuePair::IsEmpty(const CObject& object) const
{
    bool rval = false;
    const CUser_object* user = x_GetUser_Object(object);

    // empty, is there are no field-value pairs
    if (user && CComment_rule::IsStructuredComment(*user)) {
        if (!user->IsSetData() || user->GetData().empty()) {
            // empty if no fields
            rval = true;
        }
    }
    return rval;
}

string CStructCommFieldValuePair::GetVal(const CObject& object)
{
    vector<string> vals = GetVals(object);
    return (vals.empty()) ? kEmptyStr : vals[0];
}

vector<string> CStructCommFieldValuePair::GetVals(const CObject& object)
{
    vector<string> vals;
    const CUser_object* user = x_GetUser_Object(object);
    if (user && CComment_rule::IsStructuredComment(*user) && user->IsSetData()) {
        ITERATE(CUser_object::TData, it, user->GetData()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr() && (*it)->IsSetData() 
                && NStr::Equal((*it)->GetLabel().GetStr(), m_FieldName)) {
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
    }
    return vals;
}

void CStructCommFieldValuePair::ClearVal(CObject& object)
{
    CUser_object* user = x_GetUser_Object(object);
    if (user && user->IsSetData() && CComment_rule::IsStructuredComment(*user)) {
        CUser_object::TData::iterator it = user->SetData().begin();
        while (it != user->SetData().end()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr() && 
                NStr::Equal((*it)->GetLabel().GetStr(), m_FieldName)) {
                it = user->SetData().erase(it);
            } else {
                ++it;
            }
        }
        if (user->GetData().empty()) {
            user->ResetData();
        }
    }
}

bool CStructCommFieldValuePair::SetVal(CObject& object, const string& newValue, EExistingText existing_text)
{
    bool rval = false;
    CUser_object* user = x_GetUser_Object(object);
    if (user && CComment_rule::IsStructuredComment(*user)) {
        bool found = false;
        if (user->IsSetData()) {
            CUser_object::TData::iterator it = user->SetData().begin();
            while (it != user->SetData().end()) {
                if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()
                    && NStr::Equal((*it)->GetLabel().GetStr(), m_FieldName)) {
                    rval |= SetVal(**it, newValue, existing_text);
                    found = true;
                }
                if (!(*it)->IsSetData()) {
                    it = user->SetData().erase(it);
                } else {
                    it++;
                }
            }
        }
        if (!found) {
            CRef<CUser_field> new_field(new CUser_field());
            new_field->SetLabel().SetStr(m_FieldName);
            if (SetVal(*new_field, newValue, eExistingText_replace_old)) {
                x_InsertFieldAtCorrectPosition(*user, new_field);
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

bool CStructCommFieldValuePair::SetVal(CUser_field& field, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    
    if (field.IsSetData()) {
        if (field.GetData().IsStr()) {
            string curr_val = field.GetData().GetStr();
            if (AddValueToString(curr_val, newValue, existing_text)) {
                field.SetData().SetStr(curr_val);
                rval = true;
            }
        } else if (field.GetData().Which() == CUser_field::TData::e_not_set) {
            field.SetData().SetStr(newValue);
            rval = true;
        }
    } else {
        field.SetData().SetStr(newValue);
        rval = true;
    }

    return rval;
}

void CStructCommFieldValuePair::x_InsertFieldAtCorrectPosition(CUser_object& user, CRef<CUser_field> field)
{
    if (!field) {
        return;
    }
    if (!user.IsSetData()) {
        // no fields yet, just add the field
        user.SetData().push_back(field);
        return;
    }
    string this_field_label = field->GetLabel().GetStr();


    string prefix = CComment_rule::GetStructuredCommentPrefix(user);
    vector<string> field_names = CComment_set::GetFieldNames(prefix);
    if (field_names.empty()) {
        // no information about field order, just add to end
        user.SetData().push_back(field);
        return;
    }

    vector<string>::iterator sit = field_names.begin();
    CUser_object::TData::iterator fit = user.SetData().begin();
    while (sit != field_names.end() && fit != user.SetData().end()) {
        string field_label = (*fit)->GetLabel().GetStr();
        if (NStr::EqualNocase(field_label, kStructCommentPrefix)) {
            // skip
            ++fit;
        } else if (NStr::EqualNocase(*sit, (*fit)->GetLabel().GetStr())) {
            ++sit;
            ++fit;
        } else if (NStr::EqualNocase(*sit, this_field_label)) {
            // insert field here
            user.SetData().insert(fit, field);
            return;
        } else if (NStr::EqualNocase(field_label, kStructCommentSuffix)){
            user.SetData().insert(fit, field);
            return;
        } else {
            // field is missing
            ++sit;
        }
    }
    user.SetData().push_back(field);
}

const CUser_object* CStructCommFieldValuePair::x_GetUser_Object(const CObject& object) const
{
    const CSeqdesc * seqdesc = dynamic_cast<const CSeqdesc *>(&object);
    const CUser_object* user = dynamic_cast<const CUser_object*>(&object);
    
    if (seqdesc && seqdesc->IsUser()) {
        user = &(seqdesc->GetUser());
    }
    return user;
}

CUser_object* CStructCommFieldValuePair::x_GetUser_Object(CObject& object) const
{
    CSeqdesc* seqdesc = dynamic_cast<CSeqdesc*>(&object);
    CUser_object* user = dynamic_cast<CUser_object*>(&object);

    if (seqdesc && seqdesc->IsUser()) {
        user = &(seqdesc->SetUser());
    }
    return user;
}


// class CStructCommDBName

vector<string> CStructCommDBName::GetVals(const CObject& object)
{
    vector<string> vals;
    const CUser_object* user = x_GetUser_Object(object);
    vals.push_back(CComment_rule::GetStructuredCommentPrefix(*user));
    return vals;
}

void CStructCommDBName::ClearVal(CObject& object)
{
    CUser_object* user = x_GetUser_Object(object);
    if (user && user->IsSetData() && CComment_rule::IsStructuredComment(*user)) {
        CUser_object::TData::iterator it = user->SetData().begin();
        while (it != user->SetData().end()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()
                && (NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentPrefix)
                || NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentSuffix))) {
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

bool CStructCommDBName::SetVal(CObject& object, const string& newValue, EExistingText existing_text)
{
    bool rval = false;
    CUser_object* user = x_GetUser_Object(object);
    if (user && CComment_rule::IsStructuredComment(*user)) {
        bool found = false;
        if (user->IsSetData()) {
            CUser_object::TData::iterator it = user->SetData().begin();
            while (it != user->SetData().end()) {
                if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()
                    && (NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentPrefix)
                    || NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentSuffix))) {
                    rval |= SetVal(**it, newValue, existing_text);
                    found = true;
                }
                if (!(*it)->IsSetData()) {
                    it = user->SetData().erase(it);
                } else {
                    it++;
                }
            }
        }
        if (!found) {
            CRef<CUser_field> pre_field(new CUser_field());
            pre_field->SetLabel().SetStr(kStructCommentPrefix);
            if (SetVal(*pre_field, newValue, eExistingText_replace_old)) {
                x_InsertFieldAtCorrectPosition(*user, pre_field);
                rval |= true;
            }
            CRef<CUser_field> suf_field(new CUser_field());
            suf_field->SetLabel().SetStr(kStructCommentSuffix);
            if (SetVal(*suf_field, newValue, eExistingText_replace_old)) {
                x_InsertFieldAtCorrectPosition(*user, suf_field);
                rval |= true;
            }
        }
        
        // if User object now has no fields, reset so it will be detected as empty
        if (user->GetData().empty()) {
            user->ResetData();
        }
    }
    return rval;
}

bool CStructCommDBName::SetVal(CUser_field& field, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    
    if (field.IsSetData()) {
        if (field.GetData().IsStr()) {
            string curr_val = field.GetData().GetStr();
            string root = curr_val;
            CComment_rule::NormalizePrefix(root);
            if (AddValueToString(root, newValue, existing_text)) {
                if (NStr::Equal(field.GetLabel().GetStr(), kStructCommentPrefix)) {
                    string pre = CComment_rule::MakePrefixFromRoot(root);
                    field.SetData().SetStr(pre);
                    rval = true;
                } else if (NStr::Equal(field.GetLabel().GetStr(), kStructCommentSuffix)) {
                    string suf = CComment_rule::MakeSuffixFromRoot(root);
                    field.SetData().SetStr(suf);
                    rval = true;
                }
            }
        } else if (field.GetData().Which() == CUser_field::TData::e_not_set) {
            if (NStr::Equal(field.GetLabel().GetStr(), kStructCommentPrefix)) {
                string pre = CComment_rule::MakePrefixFromRoot(newValue);
                field.SetData().SetStr(pre);
                rval = true;
            } else if (NStr::Equal(field.GetLabel().GetStr(), kStructCommentSuffix)) {
                string suf = CComment_rule::MakeSuffixFromRoot(newValue);
                field.SetData().SetStr(suf);
                rval = true;
            }
        }
    } else {
        if (NStr::Equal(field.GetLabel().GetStr(), kStructCommentPrefix)) {
            string pre = CComment_rule::MakePrefixFromRoot(newValue);
            field.SetData().SetStr(pre);
            rval = true;
        } else if (NStr::Equal(field.GetLabel().GetStr(), kStructCommentSuffix)) {
            string suf = CComment_rule::MakeSuffixFromRoot(newValue);
            field.SetData().SetStr(suf);
            rval = true;
        }
    }

    return rval;
}

 // class CStructCommFieldName

vector<string> CStructCommFieldName::GetVals(const CObject& object)
{
    vector<string> vals;
    x_Init();
    const CUser_object* user = x_GetUser_Object(object);
    if (user && user->IsSetData() && CComment_rule::IsStructuredComment(*user)) {
        ITERATE (CUser_object::TData, it, user->GetData()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()
                && !(NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentPrefix)
                || NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentSuffix))) {
                vals.push_back((*it)->GetLabel().GetStr());
                CRef<CUser_field> new_field(new CUser_field());
                new_field->Assign(**it);
                m_FieldValuePairs.push_back(new_field);
            }
        }
    }
    return vals;
}

void CStructCommFieldName::ClearVal(CObject& object)
{
    CUser_object* user = x_GetUser_Object(object);
    if (user && user->IsSetData() && CComment_rule::IsStructuredComment(*user)) {
        CUser_object::TData::iterator it = user->SetData().begin();
        while (it != user->SetData().end()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()
                && !(NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentPrefix)
                || NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentSuffix))) {
                it = user->SetData().erase(it);
                m_Cleared = true;
            } else {
                ++it;
            }
        }
        if (user->GetData().empty()) {
            user->ResetData();
        }
    }
}

bool CStructCommFieldName::SetVal(CObject& object, const string& newValue, EExistingText existing_text)
{
    bool rval = false;
    CUser_object* user = x_GetUser_Object(object);
    if (user && CComment_rule::IsStructuredComment(*user)) {
        if (!m_Cleared && m_NrOfSetting == 0) {
            bool found = false;
            if (user->IsSetData()) {
                CUser_object::TData::iterator it = user->SetData().begin();
                while (it != user->SetData().end()) {
                    if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()
                        && !(NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentPrefix)
                        || NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentSuffix))) {
                        rval |= SetVal(**it, newValue, existing_text);
                        found = true;
                    }
                    it++;
                }
            }
            if (!found) {
                CRef<CUser_field> new_field(new CUser_field());
                new_field->SetLabel().SetStr(newValue);
                new_field->SetData().SetStr(kEmptyStr);
                x_InsertFieldAtCorrectPosition(*user, new_field);
                rval = true;
            }
        } else {
            // if it was cleared:
            size_t members = m_FieldValuePairs.size();
            size_t index = m_NrOfSetting % members;
            rval |= SetVal(*(m_FieldValuePairs[index]), newValue, eExistingText_replace_old);
            m_NrOfSetting++;
            if (m_NrOfSetting == members) {
                if (!user->IsSetData()) {
                    // no fields yet, just add the field
                    user->SetData().assign(m_FieldValuePairs.begin(), m_FieldValuePairs.end());
                } else {
                    CUser_object::TData::iterator it = user->SetData().begin();
                    if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()
                        && NStr::Equal((*it)->GetLabel().GetStr(), kStructCommentPrefix)) {
                        ++it;
                    }
                    user->SetData().insert(it, m_FieldValuePairs.begin(), m_FieldValuePairs.end());
                    m_Cleared = false;
                }
            }
        }
        
        // if User object now has no fields, reset so it will be detected as empty
        if (user->GetData().empty()) {
            user->ResetData();
        }
    }

    return rval;
}

bool CStructCommFieldName::SetVal(CUser_field& field, const string & newValue, EExistingText existing_text)
{
    bool rval = false;
    string curr_val = field.GetLabel().GetStr();
    if (AddValueToString(curr_val, newValue, existing_text)) {
        field.SetLabel().SetStr(curr_val);
        rval = true;
    }
    return rval;
}

void CStructCommFieldName::x_Init(void)
{
    m_FieldValuePairs.clear();
    m_NrOfSetting = 0;
}


// class CFlatFileField
vector<CConstRef<CObject> > CFlatFileField::GetObjects(CBioseq_Handle bsh)
{
    vector< CConstRef<CObject> > objects;
    objects.push_back(CConstRef<CObject>(bsh.GetCompleteBioseq()));
    return objects;
}

vector<CConstRef<CObject> > CFlatFileField::GetObjects(CSeq_entry_Handle seh,
    const string& constraint_field, CRef<edit::CStringConstraint> string_constraint)
{
    vector< CConstRef<CObject> > objects;
    return objects;
}


vector<CRef<CApplyObject> > CFlatFileField::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objects;
    objects.push_back(CRef<CApplyObject>(new CApplyObject(bsh)));
    return objects;
}

vector<CConstRef<CObject> > CFlatFileField::GetRelatedObjects(const CObject& object, CRef<CScope> scope)
{
    vector< CConstRef<CObject> > objects;
    return objects;
}

vector<CConstRef<CObject> > CFlatFileField::GetRelatedObjects(const CApplyObject& object)
{
    vector< CConstRef<CObject> > objects;
    return objects;
}

string CFlatFileField::GetVal(const CObject& object)
{
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(&object);
    if (!bioseq) 
        return kEmptyStr;

    // generate flat file without the features and sequence for the bioseq
    CNcbiOstrstream ostr;
    try {
        CFlatFileConfig cfg;
        cfg.SetFormatGenbank();
        cfg.SetModeGBench();
        cfg.SetStyleNormal();
        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_Sequence);
        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_Basecount);
        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_FeatAndGap);
        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_Segment);
        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_Featheader);
        cfg.SkipGenbankBlock(CFlatFileConfig::fGenbankBlocks_Origin);
        CRef<CFlatFileGenerator> ffgen(new CFlatFileGenerator(cfg));

        ffgen->Generate(*bioseq, *m_Scope, ostr);
    }
    catch (const CFlatException& e) {
        LOG_POST("Failed to generate flat file: " << e.GetMsg());
        return kEmptyStr;
    }

    return (IsOssEmpty(ostr)) ? kEmptyStr : CNcbiOstrstreamToString(ostr);
}


// class CMiscSeqTableColumn

CMiscSeqTableColumn::CMiscSeqTableColumn(const string & name)
{
    m_Src.Reset(NULL);
    m_Feature.Reset(NULL);
    m_FieldHandler.Reset(NULL);
    CRef<CSrcTableColumnBase> src_col = CSrcTableColumnBaseFactory::Create(name);
    if (src_col) {
        m_Src = src_col;
    } else {
        m_FieldHandler = CFieldHandlerFactory::Create(name);
        if (!m_FieldHandler) {
            CSeqFeatData::E_Choice major_type = CSeqFeatData::e_not_set;
            size_t pos = NStr::Find(name, " ");
            if (pos == string::npos) {
                m_Feature = CFeatureSeqTableColumnBaseFactory::Create(name);
            } else {
                string key = name.substr(0, pos);
                CSeqFeatData::ESubtype subtype = CSeqFeatData::eSubtype_bad;
                if (NStr::EqualNocase(key, "RNA")) {
                    major_type = CSeqFeatData::e_Rna;                    
                } else {
                    subtype = CSeqFeatData::SubtypeNameToValue(key);
                }
                if (subtype == CSeqFeatData::eSubtype_bad && NStr::StartsWith(name, "protein ")) {
                    subtype = CSeqFeatData::eSubtype_prot;
                }
                if (major_type != CSeqFeatData::e_not_set || subtype != CSeqFeatData::eSubtype_bad) {
                    m_Feature = CFeatureSeqTableColumnBaseFactory::Create(name.substr(pos + 1), subtype);
                } else {
                    m_Feature = CFeatureSeqTableColumnBaseFactory::Create(name);
                }
                if (major_type != CSeqFeatData::e_not_set) {
                    m_Feature->SetMajorType(major_type);
                }
            }
        }
    }
}


bool CMiscSeqTableColumn::s_IsSequenceIDField(const string& field)
{
    if (QualifierNamesAreEquivalent(field, kSequenceIdColLabel)
        || QualifierNamesAreEquivalent(field, kFieldTypeSeqId)) {
        return true;
    } else {
        return false;
    }
}

bool CMiscSeqTableColumn::IsProductField (void )
{
    if (m_Feature && NStr::EqualNocase(m_Feature->GetLabel(), "product")) {
        return true;
    }
    return false;
}

void CMiscSeqTableColumn::SetScope(CRef<CScope> scope)
{
    if (m_Feature) {
        m_Feature->SetScope(scope);
    } else if (m_FieldHandler) {
        m_FieldHandler->SetScope(scope);
    }
}

vector<CConstRef<CObject> > CMiscSeqTableColumn::GetObjects(CBioseq_Handle bsh)
{
    vector<CConstRef<CObject> > objects;
    if (m_Src) {
       objects = m_Src->GetObjects(bsh);
    } else if (m_FieldHandler) {
        objects = m_FieldHandler->GetObjects(bsh);
    } else if (m_Feature) {
        objects = m_Feature->GetObjects(bsh);
    }
    return objects;
}


vector<CConstRef<CObject> > CMiscSeqTableColumn::GetObjects(CSeq_entry_Handle seh, const string& constraint_field, CRef<edit::CStringConstraint> string_constraint)
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


vector<CRef<CApplyObject> > CMiscSeqTableColumn::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<CApplyObject> > objs;
    if (m_FieldHandler) {
        objs = m_FieldHandler->GetApplyObjects(bsh);
    } else if (m_Src) {
        objs = m_Src->GetApplyObjects(bsh);
    } else if (m_Feature) {
        objs = m_Feature->GetApplyObjects(bsh);
        vector<CConstRef<CObject> > feats = GetObjects(bsh);
    } 
    return objs;
}


vector<CRef<CApplyObject> > CMiscSeqTableColumn::GetApplyObjects(CSeq_entry_Handle seh, const string& constraint_field, CRef<edit::CStringConstraint> string_constraint)
{
    vector<CRef<CApplyObject> > objs;

    if (m_Src) {
        CBioseq_CI bi (seh, objects::CSeq_inst::eMol_na);
        while (bi) {
            if (NStr::EqualNocase(constraint_field, kFieldTypeSeqId)) {
                if (edit::CSeqIdGuesser::DoesSeqMatchConstraint(*bi, string_constraint)) {
                    vector<CRef<CApplyObject> > these_objs = m_Src->GetApplyObjects(*bi);
                    objs.insert(objs.end(), these_objs.begin(), these_objs.end());
                }
            } else {
                vector<CRef<CApplyObject> > these_objs = m_Src->GetApplyObjects(*bi);
                NON_CONST_ITERATE (vector<CRef<CApplyObject> >, it, these_objs) {
                    if (DoesApplyObjectMatchFieldConstraint (**it, constraint_field, string_constraint)) {
                        objs.push_back (*it);
                    }
                }
            }
            ++bi;
        }
    } else if (m_FieldHandler) {
        CBioseq_CI bi (seh, objects::CSeq_inst::eMol_na);
        while (bi) {
            if (NStr::EqualNocase(constraint_field, kFieldTypeSeqId)) {
                if (edit::CSeqIdGuesser::DoesSeqMatchConstraint(*bi, string_constraint)) {
                    vector<CRef<CApplyObject> > these_objs = m_FieldHandler->GetApplyObjects(*bi);
                    objs.insert(objs.end(), these_objs.begin(), these_objs.end());
                }
            } else {
                vector<CRef<CApplyObject> > these_objs = m_FieldHandler->GetApplyObjects(*bi);
                NON_CONST_ITERATE (vector<CRef<CApplyObject> >, it, these_objs) {
                    if (DoesApplyObjectMatchFieldConstraint (**it, constraint_field, string_constraint)) {
                        objs.push_back (*it);
                    }
                }
            }
            ++bi;
        }
    } else if (m_Feature) {
        CBioseq_CI bi (seh, objects::CSeq_inst::eMol_na);
        while (bi) {
            if (NStr::EqualNocase(constraint_field, kFieldTypeSeqId)) {
                if (edit::CSeqIdGuesser::DoesSeqMatchConstraint(*bi, string_constraint)) {
                    vector<CRef<CApplyObject> > these_objs = m_Feature->GetApplyObjects(*bi);
                    objs.insert(objs.end(), these_objs.begin(), these_objs.end());
                }
            } else {
                vector<CRef<CApplyObject> > these_objs = m_Feature->GetApplyObjects(*bi);
                NON_CONST_ITERATE (vector<CRef<CApplyObject> >, it, these_objs) {
                    if (DoesApplyObjectMatchFieldConstraint (**it, constraint_field, string_constraint)) {
                        objs.push_back (*it);
                    }
                }
            }
            ++bi;
        }
    }

    return objs;
}

void CMiscSeqTableColumn::GetApplyObjectsValues(CSeq_entry_Handle seh, const string& constraint_field, set<string> &values)
{

    if (m_Src) {
        CBioseq_CI bi (seh, objects::CSeq_inst::eMol_na);
        while (bi) {          
            vector<CRef<CApplyObject> > these_objs = m_Src->GetApplyObjects(*bi);
            NON_CONST_ITERATE (vector<CRef<CApplyObject> >, it, these_objs) {
                AddApplyObjectValue(**it,constraint_field,values);
            }
            ++bi;
        }
    } else if (m_FieldHandler) {
        CBioseq_CI bi (seh, objects::CSeq_inst::eMol_na);
        while (bi) {          
            vector<CRef<CApplyObject> > these_objs = m_FieldHandler->GetApplyObjects(*bi);
            NON_CONST_ITERATE (vector<CRef<CApplyObject> >, it, these_objs) {
                AddApplyObjectValue(**it,constraint_field,values);
            }
            ++bi;
        }
    } else if (m_Feature) {
        CRef<CScope> scope(&(seh.GetScope()));
        CBioseq_CI bi (seh, objects::CSeq_inst::eMol_na);
        while (bi) {
            vector<CConstRef<CObject> > const_objs = m_Feature->GetObjects(*bi);
            ITERATE(vector<CConstRef<CObject> >, it, const_objs) {
                const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(it->GetPointer());
                if (feat) {
                    CRef<CApplyObject> obj(new CApplyObject(*bi, *feat));
                    AddApplyObjectValue(*obj,constraint_field,values);
                }
            }
            ++bi;
        }
    }
}

void CMiscSeqTableColumn::AddApplyObjectValue(const CApplyObject& object, const string& field, set<string> &values)
{
    if (NStr::IsBlank(field)) {
        return;
    }

    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(field)); 
    if (!col) {
        return;
    }

    vector<CConstRef<CObject> > objs = col->GetRelatedObjects (object);
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        vector<string> add = col->GetVals(**it);
        values.insert(add.begin(), add.end());
    }
}


string CMiscSeqTableColumn::GetVal(const CObject& object)
{
    string val = "";
    if (m_Src) {
        val = m_Src->GetVal(object);
    } else if (m_FieldHandler) {
        val = m_FieldHandler->GetVal(object);
    } else if (m_Feature) {
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat * >(&object);
        if (feat) {
            val = m_Feature->GetFromFeat(*feat);
        }
    }
    return val;
}


vector<string> CMiscSeqTableColumn::GetVals(const CObject& object)
{
    vector<string> vals;    
    if (m_Src) {
        vals = m_Src->GetVals(object);
    } else if (m_FieldHandler) {
        vals = m_FieldHandler->GetVals(object);
    } else if (m_Feature) {
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat * >(&object);
        if (feat) {
            vals.push_back(m_Feature->GetFromFeat(*feat));
        }
    }
    return vals;
}


bool CMiscSeqTableColumn::IsEmpty(const CObject& object) const
{
    bool rval = false;
    if (m_FieldHandler) {
        rval = m_FieldHandler->IsEmpty(object);
    } else {
        rval = false;
    }
    return rval;
}


void CMiscSeqTableColumn::ClearVal(CObject& object)
{
    if (m_Src) {
        m_Src->ClearVal(object);
    } else if (m_FieldHandler) {
        m_FieldHandler->ClearVal(object);
    } else if (m_Feature) {
        CSeq_feat* feat = dynamic_cast<CSeq_feat * >(&object);
        if (feat) {
            m_Feature->ClearInFeature(*feat);
        }
    }
}


CRef<CCmdComposite> CMiscSeqTableColumn::ClearValCmd(CConstRef<CObject> object, bool update_mrna_product, CScope& scope)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Set Value"));
    bool any_change = false;

    string orig_val = GetVal(*object);
    if (!NStr::IsBlank(orig_val)) {
        const CSeqdesc* old_desc = dynamic_cast<const CSeqdesc *>(object.GetPointer());
        const CSeq_feat* old_feat = dynamic_cast<const CSeq_feat * >(object.GetPointer());
        if (old_desc) {
            CRef<CSeqdesc> new_desc( new CSeqdesc() );            
            new_desc->Assign(*old_desc);            
            ClearVal(*((CObject *)new_desc));
            CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(Ref(&scope), *old_desc);
            if (seh) {
                if (IsEmpty(*((CObject *)new_desc))) {
                    CRef<CCmdDelDesc> ecmd(new CCmdDelDesc(seh, *old_desc));
                    cmd->AddCommand(*ecmd);
                    any_change = true;
                }
                else {
                    CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(seh, *old_desc, *new_desc));
                    cmd->AddCommand(*ecmd);
                    any_change = true;
                }
            }
        } else if (old_feat) {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(*old_feat);
            ClearVal(*((CObject *)new_feat));

            if (IsEmpty(*((CObject*)new_feat))) {
                cmd->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(scope.GetSeq_featHandle(*old_feat))));
                any_change = true;
            } else {
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*old_feat), *new_feat)));
                any_change = true;
            }
            
            // the mRNA product name is required to match the CDS product name
            if (update_mrna_product) {
                string prot_product = GetVal(*new_feat);
                CRef<CCmdComposite> also_change_mrna = UpdatemRNAProduct(prot_product, CConstRef<CObject>(&(*new_feat)), scope);
                if (also_change_mrna) {
                    cmd->AddCommand(*also_change_mrna);
                    any_change = true;
                }
            }

        }
    }
    if (!any_change) {
        cmd.Reset(NULL);
    }
    return cmd;
}


bool CMiscSeqTableColumn::SetVal(CObject& object, const string& val, EExistingText existing_text)
{
    bool rval = false;
    if (m_Src) {
        CSeqdesc* desc = dynamic_cast<CSeqdesc * >(&object);
        CSeq_feat* feat = dynamic_cast<CSeq_feat * >(&object);
        if (desc && desc->IsSource()) {
            rval = m_Src->AddToBioSource(desc->SetSource(), val, existing_text);
        } else if (feat && feat->IsSetData() && feat->GetData().IsBiosrc()) {
            rval = m_Src->AddToBioSource(feat->SetData().SetBiosrc(), val, existing_text);
        }
    } else if (m_FieldHandler) {
        rval = m_FieldHandler->SetVal(object, val, existing_text);
    } else if (m_Feature) {
        CSeq_feat* feat = dynamic_cast<CSeq_feat * >(&object);
        if (feat) {
            rval = m_Feature->AddToFeature(*feat, val, existing_text);
        }
    }
    return rval;
}


CRef<CCmdComposite> CMiscSeqTableColumn::SetValCmd(CConstRef<CObject> object, const string& val, 
                                                EExistingText existing_text, CScope& scope,
                                                bool update_mrna_product)
{
    CRef<CCmdComposite> cmd(NULL);

    CRef<CObject> new_obj = GetNewObject(object);
    CRef<CCmdComposite> also_change_mrna(NULL);
    if (new_obj) {
        if (SetVal(*new_obj, val, existing_text)) {
            cmd = GetReplacementCommand(object, new_obj, scope, "Apply Value");
            if (update_mrna_product) {
                string prot_product = GetVal(*new_obj);
                also_change_mrna.Reset(UpdatemRNAProduct(prot_product, new_obj, scope));
                if (also_change_mrna) {
                    cmd->AddCommand(*also_change_mrna);
                }
            }
        }
    }

    return cmd;
}


CRef<CObject> CMiscSeqTableColumn::GetNewObject(CConstRef<CObject> object)
{
    const CSeqdesc * old_desc = dynamic_cast<const CSeqdesc *>(object.GetPointer());
    const CSeq_feat* old_feat = dynamic_cast<const CSeq_feat * >(object.GetPointer());
    CRef<CObject> new_obj(NULL);
        
    if (old_desc) {
        // prepare replacement descriptor
        CRef<CSeqdesc> new_desc( new CSeqdesc() );
        new_desc->Assign(*old_desc);      
        new_obj = (CObject *)new_desc;
    } else if (old_feat) {
        // prepare replacement feature
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*old_feat);
        new_obj = (CObject *)new_feat;
    }
    return new_obj;
}


CRef<CCmdComposite> CMiscSeqTableColumn::GetReplacementCommand(CConstRef<CObject> oldobj, CRef<CObject> newobj, CScope& scope, const string& cmd_name)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Convert Value"));
    bool success = false;

    const CSeqdesc* old_desc = dynamic_cast<const CSeqdesc *>(oldobj.GetPointer());
    const CSeq_feat* old_feat = dynamic_cast<const CSeq_feat * >(oldobj.GetPointer());

    if (old_desc) {
        CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(Ref(&scope), *old_desc);
        const CSeqdesc* new_desc = dynamic_cast<const CSeqdesc*>(newobj.GetPointer());
        if (seh && new_desc) {
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(seh, *old_desc, *new_desc));
            cmd->AddCommand(*ecmd);
            success = true;
        }
    } else if (old_feat) {
        CSeq_feat* new_feat = dynamic_cast<CSeq_feat * >(newobj.GetPointer());
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*old_feat), *new_feat)));
        success = true;
    }

    if (!success) {
        cmd.Reset(NULL);
    }
    return cmd;
}

CRef<CCmdComposite> CMiscSeqTableColumn::x_GetMrnaUpdate(CApplyObject& object, CMiscSeqTableColumn& other)
{
    bool src_prot_name = (GetFeatureSubtype() == CSeqFeatData::eSubtype_prot && IsProductField());
    bool dest_prot_name = (other.GetFeatureSubtype() == CSeqFeatData::eSubtype_prot && other.IsProductField());
    string prot_product = kEmptyStr;
    if (src_prot_name) { 
        prot_product = GetVal(object.SetObject());
    } else if (dest_prot_name) {
        prot_product = other.GetVal(object.SetObject());
    }

    CRef<CCmdComposite> also_change_mrna = UpdatemRNAProduct(prot_product, 
                                                                   CConstRef<CObject>(&(object.SetObject())), 
                                                                   object.GetSEH().GetScope()); 
    return also_change_mrna;
}


CRef<CCmdComposite> CMiscSeqTableColumn::x_GetMrnaUpdate(CApplyObject& src_object,
                                                         CApplyObject& dst_object, 
                                                         CMiscSeqTableColumn& other,
                                                         bool& changed_src)
{
    bool src_prot_name = (GetFeatureSubtype() == CSeqFeatData::eSubtype_prot && IsProductField());
    bool dest_prot_name = (other.GetFeatureSubtype() == CSeqFeatData::eSubtype_prot && other.IsProductField());
    bool dest_mRNA_notproduct = (other.GetFeatureSubtype() == CSeqFeatData::eSubtype_mRNA && (!other.IsProductField()));
    bool src_notmRNA_product =(GetFeatureSubtype() != CSeqFeatData::eSubtype_mRNA) || (GetFeatureSubtype() == CSeqFeatData::eSubtype_mRNA && IsProductField());
    string prot_product = kEmptyStr;
    CRef<CCmdComposite> also_change_mrna(NULL);
    if (src_prot_name) { 
        prot_product = GetVal(src_object.SetObject());
        if (dest_mRNA_notproduct) {
            CSeq_feat* mrna = dynamic_cast<CSeq_feat*>(&(dst_object.SetObject()));
            if (mrna && mrna->GetData().IsRna()) {
                string remainder;
                SetRnaProductName(mrna->SetData().SetRna(), prot_product, remainder, eExistingText_replace_old);
            }
        } else {
            also_change_mrna.Reset(UpdatemRNAProduct(prot_product,
                                                     CConstRef<CObject>(&(src_object.SetObject())), 
                                                     src_object.GetSEH().GetScope()));
        }
    } else if (dest_prot_name) {
        prot_product = other.GetVal(dst_object.SetObject());  // the updated protein name
        if (src_notmRNA_product) {
            also_change_mrna.Reset(UpdatemRNAProduct(prot_product,
                                                     CConstRef<CObject>(&(dst_object.SetObject())), 
                                                     src_object.GetSEH().GetScope()));
        } else {
            CSeq_feat* mrna = dynamic_cast<CSeq_feat*>(&(src_object.SetObject()));
            if (mrna && mrna->GetData().IsRna()) {
                string remainder;
                SetRnaProductName(mrna->SetData().SetRna(), prot_product, remainder, eExistingText_replace_old);
                changed_src = true;
            }
        }
    }
    return also_change_mrna;
}


CRef<CCmdComposite> CMiscSeqTableColumn::ConvertValCmd(CApplyObject& object, CMiscSeqTableColumn& other, 
                        EExistingText existing_text, ECapChange cap_change, const bool leave_original, 
                        const string& field_name, const bool update_mrna_product)
{
    CRef<CCmdComposite> cmd(NULL);
    string val = GetVal(object.SetObject());
    RemoveFieldNameFromString(field_name, val);

    CSeq_entry_Handle seh = object.GetSEH().GetTopLevelEntry();    
    FixCapitalizationInString(seh, val, cap_change);

    string orig_val = other.GetVal(object.SetObject());
    CRef<CCmdComposite> also_change_mrna(NULL);
    if (other.SetVal(object.SetObject(), val, existing_text)) {
        if (update_mrna_product) {
            also_change_mrna.Reset(x_GetMrnaUpdate(object, other)); 
        }
        if ( ! leave_original ) {
            ClearVal(object.SetObject());
        }
        cmd = GetCommandFromApplyObject(object);
        if (also_change_mrna) {
            cmd->AddCommand(*also_change_mrna);
        }
    }
    return cmd;
}


CRef<CCmdComposite> CMiscSeqTableColumn::ConvertValCmd
(CRef<CApplyObject> src_object,
 CRef<CApplyObject> dst_object,
 CMiscSeqTableColumn& other, 
 EExistingText existing_text,
 ECapChange cap_change, 
 const bool leave_original,
 const string& field_name,
 const bool update_mrna_product)
{
    CRef<CCmdComposite> cmd(NULL);
    if (!src_object || !dst_object) {
        return cmd;
    }

    if (src_object == dst_object || src_object->GetOriginalObject() == dst_object->GetOriginalObject()) {
        return ConvertValCmd(*src_object, other, existing_text, cap_change, leave_original, field_name, update_mrna_product);
    }

    string val = GetVal(src_object->SetObject());
    RemoveFieldNameFromString(field_name, val);

    CSeq_entry_Handle seh = src_object->GetSEH().GetTopLevelEntry();    
    FixCapitalizationInString(seh, val, cap_change);

    string orig_val = other.GetVal(dst_object->SetObject());
    CRef<CCmdComposite> also_change_mrna(NULL);

    if (other.SetVal(dst_object->SetObject(), val, existing_text)) {
        cmd.Reset(new CCmdComposite("Convert value"));
        bool changed_src = false;

        if (update_mrna_product) {
            also_change_mrna = x_GetMrnaUpdate(*src_object, *dst_object, other, changed_src);
        } 
        if ( ! leave_original) {
            ClearVal(src_object->SetObject());
            changed_src = true;
        }

        if (changed_src) {
            CRef<CCmdComposite> change_src = GetCommandFromApplyObject(*src_object);
            cmd->AddCommand(*change_src);
        }

        CRef<CCmdComposite> change_dst = GetCommandFromApplyObject(*dst_object);
        cmd->AddCommand(*change_dst);
        if (also_change_mrna) {
            cmd->AddCommand(*also_change_mrna);
        }
    }

    return cmd;
}


CRef<CCmdComposite> CMiscSeqTableColumn::CopyValCmd(CApplyObject& object, CMiscSeqTableColumn& other, 
                                    EExistingText existing_text, bool update_mrna_product)
{
    CRef<CCmdComposite> cmd(NULL);

    string val = GetVal(object.SetObject());
    CRef<CCmdComposite> also_change_mrna(NULL);
    if (other.SetVal(object.SetObject(), val, existing_text)) {
        if (update_mrna_product) {
            also_change_mrna.Reset(x_GetMrnaUpdate(object, other)); 
        }
        
        cmd = GetCommandFromApplyObject(object);
        if (also_change_mrna) {
            cmd->AddCommand(*also_change_mrna);
        }
    }

    return cmd;
}


CRef<CCmdComposite> CMiscSeqTableColumn::CopyValCmd
(CRef<CApplyObject> src_object,
 CRef<CApplyObject> dst_object,
 CMiscSeqTableColumn& other, 
 EExistingText existing_text, 
 bool update_mrna_product)
{
    CRef<CCmdComposite> cmd(NULL);
    if (!src_object || !dst_object) {
        return cmd;
    }

    if (src_object == dst_object || src_object->GetOriginalObject() == dst_object->GetOriginalObject()) {
        return CopyValCmd(*src_object, other, existing_text, update_mrna_product);
    }
    string val = GetVal(src_object->SetObject());
    string orig_val = other.GetVal(dst_object->SetObject());
    CRef<CCmdComposite> also_change_mrna(NULL);

    if (other.SetVal(dst_object->SetObject(), val, existing_text)) {
        cmd.Reset(new CCmdComposite("Copy value"));
        bool changed_src = false;

        if (update_mrna_product) {
            also_change_mrna = x_GetMrnaUpdate(*src_object, *dst_object, other, changed_src);
        } 

        if (changed_src) {
            CRef<CCmdComposite> change_src = GetCommandFromApplyObject(*src_object);
            cmd->AddCommand(*change_src);
        }

        CRef<CCmdComposite> change_dst = GetCommandFromApplyObject(*dst_object);
        cmd->AddCommand(*change_dst);
        if (also_change_mrna) {
            cmd->AddCommand(*also_change_mrna);
        }
    }

    return cmd;
}


CRef<CCmdComposite> CMiscSeqTableColumn::SwapValCmd(CApplyObject& object, CMiscSeqTableColumn& other, 
                                EExistingText existing_text, bool update_mrna_product)
{
    CRef<CCmdComposite> cmd(NULL);

    string val1 = GetVal(object.SetObject());
    string val2 = other.GetVal(object.SetObject());
    bool change = true;
    CRef<CCmdComposite> also_change_mrna(NULL);

    if (NStr::IsBlank(val1) && NStr::IsBlank(val2)) {
        // do nothing
        change = false;
    } else if (NStr::IsBlank(val1)) {
        ClearVal(object.SetObject());
        SetVal(object.SetObject(), val2, eExistingText_replace_old);
        other.ClearVal(object.SetObject());
    } else if (NStr::IsBlank(val2)) {
        other.ClearVal(object.SetObject());
        other.SetVal(object.SetObject(), val1, eExistingText_replace_old);
        ClearVal(object.SetObject());
    } else {
        ClearVal(object.SetObject());
        SetVal(object.SetObject(), val2, eExistingText_replace_old);
        other.ClearVal(object.SetObject());
        other.SetVal(object.SetObject(), val1, eExistingText_replace_old);
    }
    
    if (update_mrna_product) {
        also_change_mrna.Reset(x_GetMrnaUpdate(object, other)); 
    }
    
    if (change) {
        cmd = GetCommandFromApplyObject(object);
        if (also_change_mrna) {
            cmd->AddCommand(*also_change_mrna);
        }
    }
    return cmd;
}


CRef<CCmdComposite> CMiscSeqTableColumn::SwapValCmd
(CRef<CApplyObject> src_object,
 CRef<CApplyObject> dst_object,
 CMiscSeqTableColumn& other, 
 EExistingText existing_text,
 bool update_mrna_product)
{
    CRef<CCmdComposite> cmd(NULL);
    if (!src_object || !dst_object) {
        return cmd;
    }

    if (src_object == dst_object || src_object->GetOriginalObject() == dst_object->GetOriginalObject()) {
        return SwapValCmd(*src_object, other, existing_text, update_mrna_product);
    }
    string val1 = GetVal(src_object->SetObject());
    string val2 = other.GetVal(dst_object->SetObject());

    bool change = true;
    if (NStr::IsBlank(val1) && NStr::IsBlank(val2)) {
        // do nothing
        change = false;
    } else if (NStr::IsBlank(val1)) {
        ClearVal(src_object->SetObject());
        SetVal(src_object->SetObject(), val2, eExistingText_replace_old);
        other.ClearVal(dst_object->SetObject());
    } else if (NStr::IsBlank(val2)) {
        other.ClearVal(dst_object->SetObject());
        other.SetVal(dst_object->SetObject(), val1, eExistingText_replace_old);
        ClearVal(src_object->SetObject());
    } else {
        ClearVal(src_object->SetObject());
        SetVal(src_object->SetObject(), val2, eExistingText_replace_old);
        other.ClearVal(dst_object->SetObject());
        other.SetVal(dst_object->SetObject(), val1, eExistingText_replace_old);
    }

    CRef<CCmdComposite> also_change_mrna(NULL);
    if (change && update_mrna_product) {
        bool changed_src = false;
        also_change_mrna = x_GetMrnaUpdate(*src_object, *dst_object, other, changed_src);
    }

    if (change) {
        cmd.Reset(new CCmdComposite("Swap Values"));
        CRef<CCmdComposite> change_src = GetCommandFromApplyObject(*src_object);
        cmd->AddCommand(*change_src);

        CRef<CCmdComposite> change_dst = GetCommandFromApplyObject(*dst_object);
        cmd->AddCommand(*change_dst);
        if (also_change_mrna) {
            cmd->AddCommand(*also_change_mrna);
        }
    }

    return cmd;
}


CRef<CCmdComposite> CMiscSeqTableColumn::ParseValCmd(CApplyObject& object, CMiscSeqTableColumn& other, 
                            const CParseTextOptions& parse_options, EExistingText existing_text, 
                            bool update_mrna_product)
{
    CRef<CCmdComposite> cmd(NULL);

    string val1 = GetVal(object.SetObject());
    string val2 = parse_options.GetSelectedText(val1);
    if (NStr::IsBlank(val2)) {
        return cmd;
    }

    other.SetVal(object.SetObject(), val2, existing_text);
    if (parse_options.ShouldRemoveFromParsed()) {
        parse_options.RemoveSelectedText(val1);
        if (NStr::IsBlank(val1)) {
            ClearVal(object.SetObject());
        } else {
            SetVal(object.SetObject(), val1, eExistingText_replace_old);
        }
    }

    CRef<CCmdComposite> also_change_mrna(NULL);
    if (update_mrna_product) {
        also_change_mrna.Reset(x_GetMrnaUpdate(object, other));  
    }

    cmd = GetCommandFromApplyObject(object);
    if (also_change_mrna) {
        cmd->AddCommand(*also_change_mrna);
    }

    return cmd;
}


CRef<CCmdComposite> CMiscSeqTableColumn::ParseValCmd
(CRef<CApplyObject> src_object,
 CRef<CApplyObject> dst_object,
 CMiscSeqTableColumn& other, 
 const CParseTextOptions& parse_options, 
 EExistingText existing_text, 
 bool update_mrna_product)
{
    CRef<CCmdComposite> cmd(NULL);
    if (!src_object || !dst_object) {
        return cmd;
    }

    if (src_object == dst_object || src_object->GetOriginalObject() == dst_object->GetOriginalObject()) {
        return ParseValCmd(*src_object, other, parse_options, existing_text, update_mrna_product);
    }
    string val1 = GetVal(src_object->SetObject());
    string val2 = parse_options.GetSelectedText(val1);

    if (NStr::IsBlank(val2)) {
        return cmd;
    }

    other.SetVal(dst_object->SetObject(), val2, existing_text);

    bool changed_src = false;
    if (parse_options.ShouldRemoveFromParsed()) {
        parse_options.RemoveSelectedText(val1);
        if (NStr::IsBlank(val1)) {
            ClearVal(src_object->SetObject());
        } else {
            SetVal(src_object->SetObject(), val1, eExistingText_replace_old);
        }
        changed_src = true;
    }

    cmd.Reset(new CCmdComposite("Parse Value"));
    CRef<CCmdComposite> also_change_mrna(NULL);
    if (update_mrna_product) {
        also_change_mrna = x_GetMrnaUpdate(*src_object, *dst_object, other, changed_src);
    } 

    if (changed_src) {
        CRef<CCmdComposite> change_src = GetCommandFromApplyObject(*src_object);
        cmd->AddCommand(*change_src);
    }

    CRef<CCmdComposite> change_dst = GetCommandFromApplyObject(*dst_object);
    cmd->AddCommand(*change_dst);
    if (also_change_mrna) {
        cmd->AddCommand(*also_change_mrna);
    }

    return cmd;
}


string CMiscSeqTableColumn::IsValid(const string& val)
{
    string error = "";
    if (m_Src) {
        vector<string> values;
        values.push_back(val);
        vector<string> problems = m_Src->IsValid(values);
        if (problems.size() > 0) {
            error = problems[0];
        }
    } else if (m_FieldHandler) {
        error = m_FieldHandler->IsValid(val);
    } else if (m_Feature) {
        vector<string> values;
        values.push_back(val);
        vector<string> problems = m_Feature->IsValid(values);
        if (problems.size() > 0) {
            error = problems[0];
        }
    }
    return error;
}


vector<string> CMiscSeqTableColumn::IsValid(const vector<string>& values)
{
    vector<string> problems;
    if (m_Src) {
        problems = m_Src->IsValid(values);
    } else if (m_FieldHandler) {
        problems = m_FieldHandler->IsValid(values);
    } else if (m_Feature) {
        problems = m_Feature->IsValid(values);
    }
    return problems;
}


int CMiscSeqTableColumn::CountConflicts(const CObject& obj, const string& val)
{
    int conflicts = 0;
    vector<string> curr_vals = GetVals(obj);
    ITERATE(vector<string>, it, curr_vals) {
        if (!NStr::IsBlank(*it) && !NStr::Equal(val, *it)) {
            conflicts++;
        }
    }
    return conflicts;
}


int CMiscSeqTableColumn::CountConflicts(set<CConstRef<CObject> > objs, const string& val)
{
    int conflicts = 0;

    if (objs.empty() || NStr::IsBlank(val)) {
        return 0;
    }
    ITERATE(set<CConstRef<CObject> >, it, objs) {
        if (*it) {
            conflicts += CountConflicts(**it, val);
        }
    }
    return conflicts;
}


CSeqFeatData::ESubtype CMiscSeqTableColumn::GetFeatureSubtype()
{
    if (m_Feature) {
        return m_Feature->GetSubtype();
    } else {
        return CSeqFeatData::eSubtype_any;
    }
}


CSeqdesc::E_Choice CMiscSeqTableColumn::GetDescriptorSubtype()
{
    if (m_FieldHandler) {
        return m_FieldHandler->GetDescriptorSubtype();
    } else {
        return CSeqdesc::e_not_set;
    }
}


void CMiscSeqTableColumn::SetConstraint(string field, CRef<edit::CStringConstraint> string_constraint)
{
    if (m_Src) {
        m_Src->SetConstraint(field, string_constraint);
    } else if (m_FieldHandler) {
        m_FieldHandler->SetConstraint(field, string_constraint);
    }
}


bool CMiscSeqTableColumn::AllowMultipleValues()
{
    bool rval = false;
    if (m_Src) {
        rval = m_Src->AllowMultipleValues();
    } else if (m_FieldHandler) {
        rval = m_FieldHandler->AllowMultipleValues();
    } else if (m_Feature) {
        rval = m_Feature->AllowMultipleValues();
    }

    return rval;
}


void AddMiscColumnsToTable(CRef<CSeq_table> table, const vector<string> &fields, CSeq_entry_Handle entry)
{
    TMiscSeqTableColumnList field_list;
    vector<CRef<CSeqTable_column> > column_list;

    ITERATE (vector<string>, sit, fields) {    
        CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(*sit));
        field_list.push_back(col);
        column_list.push_back(AddStringColumnToTable(table, *sit));        
    }

    for (int row = 0; row < table->GetNum_rows(); row++) {
        CBioseq_Handle bsh = entry.GetScope().GetBioseqHandle(*(table->GetColumns()[0]->GetData().GetId()[row]));
        size_t col_num = 0;
        NON_CONST_ITERATE (TMiscSeqTableColumnList, it, field_list) {
            if ((*it)->IsFeature()) {
                //TODO if this is a feature, need to add feature ID to table
            } else {
                vector<CConstRef< CObject> > objects = (*it)->GetObjects(bsh);
                if (objects.size() > 0) {
                    string val = (*it)->GetVal(*objects[0]);
                    AddValueToColumn(column_list[col_num], val, row);
                }
            }
            col_num++;
        }
    }
    FillShortColumns(table);
}

static size_t s_GetMaxRows(const vector<size_t> &rows)
{
    size_t max_rows = 0;
    for (size_t i = 0; i < rows.size(); i++)
        if (rows[i] > max_rows)
            max_rows = rows[i];
    return max_rows;
}

CRef<CSeq_table> GetMiscTableFromSeqEntry(const vector<string> &fields, CSeq_entry_Handle entry)
{
    CRef<CSeq_table> table(new CSeq_table());

    int top_id = FindHighestFeatureId(entry);

    // first column is sequence ID
    CRef<CSeqTable_column> id_col(new CSeqTable_column());
    id_col->SetHeader().SetField_id(CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    table->SetColumns().push_back(id_col);

    TMiscSeqTableColumnList field_list;
    vector<CRef<CSeqTable_column> > column_list;

    ITERATE (vector<string>, sit, fields) {    
        CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(*sit));
        field_list.push_back(col);
        column_list.push_back(AddStringColumnToTable(table, *sit));        
    }

    CBioseq_CI bi(entry, objects::CSeq_inst::eMol_na);
    vector<size_t> rows(field_list.size(),0);
    while (bi) {
        size_t row1 = s_GetMaxRows(rows);
        size_t col_num = 0;
        NON_CONST_ITERATE (TMiscSeqTableColumnList, it, field_list) {
            vector<CConstRef< CObject> > objects = (*it)->GetObjects(*bi);
            for (size_t i = 0; i < objects.size(); i++) {
                string val = (*it)->GetVal(*objects[i]);
                AddValueToColumn(column_list[col_num], val, rows[col_num]);
                rows[col_num]++;
            }
            col_num++;
        }
        size_t row2 = s_GetMaxRows(rows);
        for (size_t i = row1; i < row2; i++)
        {
            CSeq_id_Handle best = sequence::GetId(*bi, sequence::eGetId_Best);
            CRef<CSeq_id> id(new objects::CSeq_id());
            id->Assign(*(best.GetSeqId()));
            table->SetColumns()[0]->SetData().SetId().push_back(id);
        }
        for (size_t i = 0; i < field_list.size(); i++)
            rows[i] = row2;
        ++bi;
    }

    size_t row = s_GetMaxRows(rows);
    table->SetNum_rows(row);
    FillShortColumns(table);
    return table;
}

void x_AdjustListPair (vector<CRef<CApplyObject> > prev_list, vector<CRef<CApplyObject> > this_list)
{
    NON_CONST_ITERATE(vector<CRef<CApplyObject> >, t, this_list) {
        NON_CONST_ITERATE(vector<CRef<CApplyObject> >, p, prev_list) {
            if ((*t)->GetOriginalObject() == (*p)->GetOriginalObject()) {
                (*t)->ReplaceEditable((*p)->SetObject());
                break;
            }
        }
    }
}
 
string GetExistingTextInstruction(EExistingText existing)
{
    string rval = "";

    string ur = "";
    string sep = "";

    switch (existing) {
        case eExistingText_replace_old:
            ur = "replace";
            break;
        case eExistingText_append_semi:
            ur = "append";
            sep = ";";
            break;
        case eExistingText_append_space:
            ur = "append";
            sep = " ";
            break;
        case eExistingText_append_colon:
            ur = "append";
            sep = ":";
            break;
        case eExistingText_append_comma:
            ur = "append";
            sep = ",";
            break;
        case eExistingText_append_none:
            ur = "append";
            sep = "";
            break;
        case eExistingText_prefix_semi:
            ur = "prefix";
            sep = ";";
            break;
        case eExistingText_prefix_space:
            ur = "prefix";
            sep = " ";
            break;
        case eExistingText_prefix_colon:
            ur = "prefix";
            sep = ":";
            break;
        case eExistingText_prefix_comma:
            ur = "prefix";
            sep = ",";
            break;
        case eExistingText_prefix_none:
            ur = "prefix";
            sep = "";
            break;
        case eExistingText_leave_old:
            ur = "ignore";
            sep = "";
            break;
        case eExistingText_add_qual:
            ur = "add_new_qual";
            sep = "";
            break;
        case eExistingText_cancel:
            break;
    }

    if (!NStr::IsBlank(ur)) {
        rval = "&update_rule=" + ur + "&text_separator=" + sep;
    }

    return rval;
}

string MakeTableMetaInfoString(EExistingText rule, bool erase_blanks)
{
    string value = GetExistingTextInstruction(rule);
    value += "&blanks=";
    if (erase_blanks) {
        value += "erase";
    } else {
        value += "ignore";
    }

    return value;
}


CRef<CUser_field> MakeTableMetaInfoField(EExistingText rule, bool erase_blanks)
{
    CRef<CUser_field> field(new CUser_field());
    field->SetData().SetStr(MakeTableMetaInfoString(rule, erase_blanks));
    return field;
}


vector<CConstRef<CSeq_feat> > s_GetProtFeatures(CBioseq_Handle p_bsh, CSeqFeatData::ESubtype constraint_type)
{
    vector<CConstRef<CSeq_feat> > feat_list;
    if (p_bsh) {
        CFeat_CI f(p_bsh, constraint_type);
        while (f) {
            CConstRef<CSeq_feat> object;
            object.Reset(f->GetOriginalSeq_feat());
            feat_list.push_back(object);
            ++f;
        }
    }
    return feat_list;
}


vector<CConstRef<CSeq_feat> > GetRelatedFeatures (const CSeq_feat& obj_feat, CSeqFeatData::ESubtype constraint_type, CRef<CScope> scope)
{
    vector<CConstRef<CSeq_feat> > feat_list;

    CSeqFeatData::ESubtype obj_type = obj_feat.GetData().GetSubtype();

    // is one feature type a protein and the other not?
    bool obj_is_prot = (CSeqFeatData::GetTypeFromSubtype(obj_type) == CSeqFeatData::e_Prot);
    bool constraint_is_prot = (CSeqFeatData::GetTypeFromSubtype(constraint_type) == CSeqFeatData::e_Prot);               
    if (obj_is_prot && constraint_is_prot) {
        // find feature anywhere on protein sequence
        CBioseq_Handle p_bsh = scope->GetBioseqHandle(obj_feat.GetLocation());
        feat_list = s_GetProtFeatures(p_bsh, constraint_type);
    } else if (obj_is_prot && !constraint_is_prot) {
        // use coding region for starting point of overlap comparison
        CBioseq_Handle p_bsh = scope->GetBioseqHandle(obj_feat.GetLocation());
        const CSeq_feat* cds = sequence::GetCDSForProduct(p_bsh);
        if (cds) {
            if (CSeqFeatData::GetTypeFromSubtype(constraint_type) == CSeqFeatData::e_Cdregion) {
                feat_list.push_back(CConstRef<CSeq_feat>(cds));
            } else {
                feat_list = GetRelatedFeatures(*cds, constraint_type, scope);
            }
        }                      
    } else if (!obj_is_prot && constraint_is_prot) {
        // examine objects on protein sequence
        // need to find coding region for obj_feat
        if (obj_type == CSeqFeatData::eSubtype_cdregion) {
            if (obj_feat.IsSetProduct()) {
                CBioseq_Handle p_bsh = scope->GetBioseqHandle(obj_feat.GetProduct());
                feat_list = s_GetProtFeatures(p_bsh, constraint_type);
            }
        } else if (obj_type == CSeqFeatData::eSubtype_mRNA) {
            const CSeq_feat* cds = sequence::GetBestCdsForMrna(obj_feat, *scope);
            if (cds) {
                feat_list = GetRelatedFeatures(*cds, constraint_type, scope);
            }                    
        } else if (obj_type == CSeqFeatData::eSubtype_gene) {
            sequence::TFeatScores scores;
            sequence::GetOverlappingFeatures (obj_feat.GetLocation(), 
                                              CSeqFeatData::e_Cdregion, 
                                              CSeqFeatData::eSubtype_cdregion, 
                                              sequence::eOverlap_Contained, 
                                              scores, *scope);
            ITERATE (sequence::TFeatScores, it, scores) {
                vector<CConstRef<CSeq_feat> > this_list = GetRelatedFeatures(*(it->second), constraint_type, scope);
                feat_list.insert(feat_list.end(), this_list.begin(), this_list.end());
            }
        }
    } else {
        // neither is a protein
        if (constraint_type == CSeqFeatData::eSubtype_gene) {
            CConstRef<CSeq_feat> f = sequence::GetOverlappingGene(obj_feat.GetLocation(), *scope);
            if (f) {
                feat_list.push_back(f);
            }
        } else if (obj_type == CSeqFeatData::eSubtype_gene) {
            sequence::TFeatScores scores;
            sequence::GetOverlappingFeatures (obj_feat.GetLocation(), 
                                              CSeqFeatData::GetTypeFromSubtype(constraint_type), 
                                              constraint_type, 
                                              sequence::eOverlap_Contained, 
                                              scores, *scope);            
            ITERATE (sequence::TFeatScores, it, scores) {
                feat_list.push_back(it->second);
            }
        } else if (obj_type == CSeqFeatData::eSubtype_cdregion
            && constraint_type == CSeqFeatData::eSubtype_mRNA) {
            CConstRef<CSeq_feat> f = sequence::GetBestMrnaForCds(obj_feat, *scope);
            if (f) {
                feat_list.push_back(f);
            }
        } else if (constraint_type == CSeqFeatData::eSubtype_any || constraint_type == obj_type) {
            CConstRef<CSeq_feat> f(&obj_feat);
            feat_list.push_back(f);
        }
    }
    return feat_list;
}


vector<CRef<CApplyObject> > s_GetProtApplyObjectsFeatures(CBioseq_Handle p_bsh, CSeqFeatData::ESubtype constraint_type)
{
    vector<CRef<CApplyObject> > feat_list;
    if (p_bsh) {
        CFeat_CI f(p_bsh, constraint_type);
        while (f) {
            CRef<CApplyObject> p(new CApplyObject(p_bsh, *(f->GetOriginalSeq_feat())));
            feat_list.push_back(p);
            ++f;
        }
    }
    if (feat_list.empty() && 
        (constraint_type == CSeqFeatData::eSubtype_any || constraint_type == CSeqFeatData::eSubtype_prot)) {
        CSeq_entry_Handle seh = p_bsh.GetSeq_entry_Handle();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign(*(p_bsh.GetId().front().GetSeqId()));
        new_feat->SetLocation().SetInt().SetId(*id);
        new_feat->SetLocation().SetInt().SetFrom(0);
        new_feat->SetLocation().SetInt().SetTo(p_bsh.GetInst_Length() - 1);
        new_feat->SetData().SetProt();
        const CSeq_feat* cds = sequence::GetCDSForProduct(p_bsh);
        if (cds) {
            edit::AdjustProteinFeaturePartialsToMatchCDS(*new_feat, *cds);
        }
        CRef<CObject> new_obj(new_feat.GetPointer());
        feat_list.push_back(CRef<CApplyObject>(new CApplyObject(seh, CConstRef<CObject>(NULL), new_obj)));
    }
    return feat_list;
}


vector<CRef<CApplyObject> > GetRelatedFeatureApplyObjects (const CSeq_feat& obj_feat, CSeqFeatData::ESubtype constraint_type, CRef<CScope> scope)
{
    vector<CRef<CApplyObject> > feat_list;

    CSeqFeatData::ESubtype obj_type = obj_feat.GetData().GetSubtype();

    // is one feature type a protein and the other not?
    bool obj_is_prot = (CSeqFeatData::GetTypeFromSubtype(obj_type) == CSeqFeatData::e_Prot);
    bool constraint_is_prot = (CSeqFeatData::GetTypeFromSubtype(constraint_type) == CSeqFeatData::e_Prot);               
    if (obj_is_prot && constraint_is_prot) {
        // find feature anywhere on protein sequence
        CBioseq_Handle p_bsh = scope->GetBioseqHandle(obj_feat.GetLocation());
        feat_list = s_GetProtApplyObjectsFeatures(p_bsh, constraint_type);
    } else if (obj_is_prot && !constraint_is_prot) {
        // use coding region for starting point of overlap comparison
        CBioseq_Handle p_bsh = scope->GetBioseqHandle(obj_feat.GetLocation());
        const CSeq_feat* cds = sequence::GetCDSForProduct(p_bsh);
        if (cds) {
            if (CSeqFeatData::GetTypeFromSubtype(constraint_type) == CSeqFeatData::e_Cdregion) {
                CBioseq_Handle c_bsh = scope->GetBioseqHandle(cds->GetLocation());
                CRef<CApplyObject> p(new CApplyObject(c_bsh, *cds));
                feat_list.push_back(p);
            } else {
                feat_list = GetRelatedFeatureApplyObjects(*cds, constraint_type, scope);
            }
        }                      
    } else if (!obj_is_prot && constraint_is_prot) {
        // examine objects on protein sequence
        // need to find coding region for obj_feat
        if (obj_type == CSeqFeatData::eSubtype_cdregion) {
            CBioseq_Handle p_bsh = scope->GetBioseqHandle(obj_feat.GetProduct());
            feat_list = feat_list = s_GetProtApplyObjectsFeatures(p_bsh, constraint_type);
        } else if (obj_type == CSeqFeatData::eSubtype_mRNA) {
            const CSeq_feat* cds = sequence::GetBestCdsForMrna(obj_feat, *scope);
            if (cds) {
                feat_list = GetRelatedFeatureApplyObjects(*cds, constraint_type, scope);
            }                    
        } else if (obj_type == CSeqFeatData::eSubtype_gene) {
            sequence::TFeatScores scores;
            sequence::GetOverlappingFeatures (obj_feat.GetLocation(), 
                                              CSeqFeatData::e_Cdregion, 
                                              CSeqFeatData::eSubtype_cdregion, 
                                              sequence::eOverlap_Contained, 
                                              scores, *scope);
            ITERATE (sequence::TFeatScores, it, scores) {
                vector<CRef<CApplyObject> > this_list = GetRelatedFeatureApplyObjects(*(it->second), constraint_type, scope);
                feat_list.insert(feat_list.end(), this_list.begin(), this_list.end());
            }
        }
    } else {
        // neither is a protein
        if (constraint_type == CSeqFeatData::eSubtype_gene) {
            CConstRef<CSeq_feat> f = sequence::GetOverlappingGene(obj_feat.GetLocation(), *scope);
            if (f) {
                CBioseq_Handle f_bsh = scope->GetBioseqHandle(f->GetLocation());
                CRef<CApplyObject> p(new CApplyObject(f_bsh, *f));
                feat_list.push_back(p);
            } else {
                // create new gene for feature
                CSeq_entry_Handle seh = scope->GetBioseqHandle(obj_feat.GetLocation()).GetSeq_entry_Handle();
                CRef<CSeq_feat> new_feat(new CSeq_feat());
                new_feat->SetLocation().Assign(*(obj_feat.GetLocation().Merge(CSeq_loc::fMerge_SingleRange, NULL)));
                new_feat->SetData().SetGene();
                CRef<CObject> new_obj(new_feat.GetPointer());
                feat_list.push_back(CRef<CApplyObject>(new CApplyObject(seh, CConstRef<CObject>(NULL), new_obj)));
            }
        } else if (obj_type == CSeqFeatData::eSubtype_gene) {           
            sequence::TFeatScores scores;
            sequence::GetOverlappingFeatures (obj_feat.GetLocation(), 
                                              CSeqFeatData::GetTypeFromSubtype(constraint_type), 
                                              constraint_type, 
                                              sequence::eOverlap_Contains, 
                                              scores, *scope);            
            ITERATE (sequence::TFeatScores, it, scores) {
                CBioseq_Handle f_bsh = scope->GetBioseqHandle(it->second->GetLocation());
                CRef<CApplyObject> p(new CApplyObject(f_bsh, *(it->second)));
                feat_list.push_back(p);
            }
            
        } else if (obj_type == CSeqFeatData::eSubtype_cdregion
            && constraint_type == CSeqFeatData::eSubtype_mRNA) {
            CConstRef<CSeq_feat> f = sequence::GetBestMrnaForCds(obj_feat, *scope);
            if (f) {
                CBioseq_Handle f_bsh = scope->GetBioseqHandle(f->GetLocation());
                CRef<CApplyObject> p(new CApplyObject(f_bsh, *f));
                feat_list.push_back(p);
            }
        } else if (constraint_type == CSeqFeatData::eSubtype_any || constraint_type == obj_type) {
            CBioseq_Handle f_bsh = scope->GetBioseqHandle(obj_feat.GetLocation());
            CRef<CApplyObject> p(new CApplyObject(f_bsh, obj_feat));
            feat_list.push_back(p);
        }
    }
    return feat_list;
}




vector<CConstRef<CObject> > CMiscSeqTableColumn::GetRelatedObjects(const CObject& object, CRef<CScope> scope)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&object);
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&object);
    const CBioseq * bioseq = dynamic_cast<const CBioseq *>(&object);

    if (bioseq) {
        CBioseq_Handle bsh = scope->GetBioseqHandle(*bioseq);
        if (bsh) {
            related = GetObjects(bsh);
        }
    } else if (IsSource()) {
        if (obj_desc) {
            if (obj_desc->IsSource()) {
                CConstRef<CObject> obj(obj_desc);
                related.push_back(obj);
            } else {
                // find closest related BioSource
                CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(scope, *obj_desc);
                if (seh && obj_desc->IsTitle()) {
                    CBioseq_set_Handle parent = seh.GetParentBioseq_set();
                    if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_nuc_prot) {
                        seh = parent.GetParentEntry();
                    }
                }
                if (seh) {
                    related = GetObjects(seh, "", CRef<edit::CStringConstraint>(NULL));
                }
            }
        } else if (obj_feat) {
            if (obj_feat->IsSetData() && obj_feat->GetData().IsBiosrc()) {
                CConstRef<CObject> obj(obj_feat);
                related.push_back(obj);
            } else {
                // find closest related BioSource
                CBioseq_Handle bsh = scope->GetBioseqHandle(obj_feat->GetLocation());
                if (bsh) {
                    related = GetObjects(bsh);
                }
            }
        }
    } else if (m_FieldHandler) {
        related = m_FieldHandler->GetRelatedObjects(object, scope);
    } else if (IsFeature()) {
        if (obj_feat) {
            CSeqFeatData::ESubtype constraint_type = GetFeatureSubtype();
            CSeqFeatData::ESubtype obj_type = obj_feat->GetData().GetSubtype();
            if (obj_type == constraint_type) {
                CConstRef<CObject> obj(obj_feat);
                related.push_back(obj);
            } else {
                vector<CConstRef<CSeq_feat> > feat_list = GetRelatedFeatures(*obj_feat, constraint_type, scope);
                related.insert(related.end(), feat_list.begin(), feat_list.end());
            }
        } else if (obj_desc) {
            // find related feature
            CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(scope, *obj_desc);
            if (seh && obj_desc->IsTitle()) {
                CBioseq_set_Handle parent = seh.GetParentBioseq_set();
                if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_nuc_prot) {
                    seh = parent.GetParentEntry();
                }
            }
            if (seh) {
                CFeat_CI f(seh, SAnnotSelector(GetFeatureSubtype()));
                while (f) {
                    CConstRef<CObject> obj(f->GetOriginalSeq_feat());
                    related.push_back(obj);
                    ++f;
                }
            }
        }
    }


    return related;
}


vector<CConstRef<CObject> > CMiscSeqTableColumn::GetRelatedObjects(const CApplyObject& object)
{
    vector<CConstRef<CObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&(object.GetObject()));
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&(object.GetObject()));
    const CBioseq * bioseq = dynamic_cast<const CBioseq *>(&(object.GetObject()));
    const CSeq_inst * inst = dynamic_cast<const CSeq_inst *>(&(object.GetObject()));

    if (bioseq || inst) {
        related = GetObjects(object.GetSEH().GetSeq());
    } else if (IsSource()) {
        if (obj_desc) {
            if (obj_desc->IsSource()) {
                CConstRef<CObject> obj(obj_desc);
                related.push_back(obj);
            } else {
                // find closest related BioSource
                CSeq_entry_Handle seh = object.GetSEH();
                if (seh && obj_desc->IsTitle()) {
                    CBioseq_set_Handle parent = seh.GetParentBioseq_set();
                    if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_nuc_prot) {
                        seh = parent.GetParentEntry();
                    }
                }
                if (seh) {
                    related = GetObjects(seh, "", CRef<edit::CStringConstraint>(NULL));
                }
            }
        } else if (obj_feat) {
            if (obj_feat->IsSetData() && obj_feat->GetData().IsBiosrc()) {
                CConstRef<CObject> obj(obj_feat);
                related.push_back(obj);
            } else {
                // find closest related BioSource
                related = GetObjects(object.GetSEH(), "", CRef<edit::CStringConstraint>(NULL));
            }
        }
    } else if (m_FieldHandler) {
        related = m_FieldHandler->GetRelatedObjects(object);
    } else if (IsFeature()) {
        if (obj_feat) {
            CSeqFeatData::ESubtype constraint_type = GetFeatureSubtype();
            CSeqFeatData::E_Choice major_type = m_Feature->GetMajorType();
            CSeqFeatData::ESubtype obj_type = obj_feat->GetData().GetSubtype();
            CSeqFeatData::E_Choice obj_major_type = obj_feat->GetData().Which();
            if (obj_type == constraint_type ||
                (constraint_type == CSeqFeatData::eSubtype_bad && major_type == obj_major_type)) {
                CConstRef<CObject> obj(obj_feat);
                related.push_back(obj);
            } else {
                CRef<CScope> scope(&(object.GetSEH().GetScope()));
                vector<CConstRef<CSeq_feat> > feat_list = GetRelatedFeatures(*obj_feat, constraint_type, scope);
                related.insert(related.end(), feat_list.begin(), feat_list.end());
            }
        } else if (obj_desc) {
            // find related feature
            CSeq_entry_Handle seh = object.GetSEH();
            if (seh && obj_desc->IsTitle()) {
                CBioseq_set_Handle parent = seh.GetParentBioseq_set();
                if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_nuc_prot) {
                    seh = parent.GetParentEntry();
                }
            }
            if (seh) {
                CFeat_CI f(seh, SAnnotSelector(GetFeatureSubtype()));
                while (f) {
                    CConstRef<CObject> obj(f->GetOriginalSeq_feat());
                    related.push_back(obj);
                    ++f;
                }
            }
        }
    }


    return related;
}


CSeq_entry_Handle s_GetNPParent(CSeq_entry_Handle seh)
{
    if (!seh) {
        return seh;
    }
    CBioseq_set_Handle parent = seh.GetParentBioseq_set();
    if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_nuc_prot) {
        seh = parent.GetParentEntry();
    }
    return seh;
}


vector<CRef<CApplyObject> > CMiscSeqTableColumn::GetRelatedApplyObjects(CApplyObject& object)
{
    vector<CRef<CApplyObject> > related;

    const CSeqdesc * obj_desc = dynamic_cast<const CSeqdesc *>(&(object.GetObject()));
    const CSeq_feat * obj_feat = dynamic_cast<const CSeq_feat *>(&(object.GetObject()));
    const CBioseq * bioseq = dynamic_cast<const CBioseq *>(&(object.GetObject()));
    const CSeq_inst * inst = dynamic_cast<const CSeq_inst *>(&(object.GetObject()));

    if (bioseq || inst) {
        related = GetApplyObjects(object.GetSEH().GetSeq());
    } else if (IsSource()) {
        if (obj_desc) {
            CSeq_entry_Handle seh = object.GetSEH();
            if (obj_desc->IsSource()) {                
                CRef<CApplyObject> obj(&object);
                related.push_back(obj);
            } else {
                // find closest related BioSource
                seh = s_GetNPParent(seh);

                if (seh) {
                    related = GetApplyObjects(seh, "", CRef<CStringConstraint>(NULL));
                }
            }
        } else if (obj_feat) {
            if (obj_feat->IsSetData() && obj_feat->GetData().IsBiosrc()) {
                CRef<CApplyObject> obj(&object);
                related.push_back(obj);
            } else {
                // find closest related BioSource
                CSeq_entry_Handle seh = object.GetSEH();
                seh = s_GetNPParent(seh);
                if (seh) {
                    related = GetApplyObjects(seh, "", CRef<CStringConstraint>(NULL));
                }
            }
        }
    } else if (m_FieldHandler) {
        related = GetApplyObjects(object.GetSEH(), "", CRef<CStringConstraint>(NULL));
    } else if (IsFeature()) {
        if (obj_feat) {
            CSeqFeatData::ESubtype constraint_type = GetFeatureSubtype();
            CSeqFeatData::ESubtype obj_type = obj_feat->GetData().GetSubtype();
            if (obj_type == constraint_type) {
                CRef<CApplyObject> obj(&object);
                related.push_back(obj);
            } else {
                CRef<CScope> scope(&(object.GetSEH().GetScope()));
                vector<CRef<CApplyObject> > feat_list = GetRelatedFeatureApplyObjects(*obj_feat, constraint_type, scope);
                related.insert(related.end(), feat_list.begin(), feat_list.end());
            }
        } else if (obj_desc) {
            // find related feature
            CSeq_entry_Handle seh = object.GetSEH();
            CBioseq_set_Handle parent = seh.GetParentBioseq_set();
            if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_nuc_prot) {
                seh = parent.GetParentEntry();
            }
            if (seh) {
                CFeat_CI f(seh, SAnnotSelector(GetFeatureSubtype()));
                while (f) {
                    CBioseq_Handle bsh = seh.GetBioseqHandle(*(f->GetLocation().GetId()));
                    CRef<CApplyObject> obj(new CApplyObject(bsh, *(f->GetOriginalSeq_feat())));
                    related.push_back(obj);
                    ++f;
                }
            }
        }
    }


    return related;
}


bool DoesObjectMatchFieldConstraint (const CObject& object, const string& field_name, CRef<CStringConstraint> string_constraint, CRef<CScope> scope)
{
    if (NStr::IsBlank(field_name) || !string_constraint) {
        return true;
    }
    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(field_name)); 
    if (!col) {
        return false;
    }

    vector<string> val_list; 
    vector<CConstRef<CObject> > objs = col->GetRelatedObjects (object, scope);
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        vector<string> add = col->GetVals(**it);
        val_list.insert(val_list.end(), add.begin(), add.end());
    }

    return string_constraint->DoesListMatch(val_list);
}


bool DoesApplyObjectMatchFieldConstraint (const CApplyObject& object, const string& field_name, CRef<CStringConstraint> string_constraint)
{
    if (NStr::IsBlank(field_name) || !string_constraint) {
        return true;
    }

    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(field_name)); 
    if (!col) {
        return false;
    }
    vector<string> val_list; 
    vector<CConstRef<CObject> > objs = col->GetRelatedObjects (object);

    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        vector<string> add = col->GetVals(**it);
        val_list.insert(val_list.end(), add.begin(), add.end());
    }
    return string_constraint->DoesListMatch(val_list);
}


bool IsSeqOrNP(CSeq_entry_Handle entry) 
{
    if (entry.IsSeq()) {
        return true;
    } else if (entry.IsSet() && entry.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot) {
        return true;
    } else {
        return false;
    }
}


bool PropagateDescriptor(CSeq_entry_Handle entry, const CSeqdesc& desc, CCmdComposite& cmd)
{
    bool rval = false;

    if (IsSeqOrNP(entry)) {
        CRef<CSeqdesc> new_desc(new CSeqdesc());
        new_desc->Assign(desc);
        cmd.AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(entry, *new_desc)) );
        rval = true;
    } else if (entry.IsSet()) {
        ITERATE(CBioseq_set::TSeq_set, it, entry.GetSet().GetCompleteBioseq_set()->GetSeq_set()) {
            CSeq_entry_Handle e = entry.GetScope().GetSeq_entryHandle(**it);
            rval |= PropagateDescriptor(e, desc, cmd);
        }
    }
    return rval;
}


bool PropagateDBLink(CSeq_entry_Handle entry, CCmdComposite& cmd)
{
    bool rval = false;
    if (IsSeqOrNP(entry)) {
        return false;
    }
    if (entry.IsSetDescr()) {
        ITERATE(CSeq_entry_Handle::TDescr::Tdata, it, entry.GetDescr().Get()) {
            if ((*it)->IsUser() && (*it)->GetUser().GetObjectType() == CUser_object::eObjectType_DBLink) {
                // propagate to sequences inside the set
                bool this_success = PropagateDescriptor(entry, **it, cmd);
                if (this_success) {
                    rval = true;
                    // remove from this set
                    cmd.AddCommand(*CRef<CCmdDelDesc>(new CCmdDelDesc(entry, **it)) );
                }
            }
        }
    }
    return rval;
}


CRef<CCmdComposite> GetPropagateDBLinkCmd(CSeq_entry_Handle entry)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("PropagateDBLink") );

    bool any = PropagateDBLink(entry, *cmd);

    if (!any) {
        cmd.Reset(NULL);
    }
    return cmd;
}


CRef<CCmdComposite> GetCommandFromApplyObject(CApplyObject& obj)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Apply"));
    bool any = false;

    CSeqdesc* desc = dynamic_cast<CSeqdesc * >(&(obj.SetObject()));
    CSeq_feat* feat = dynamic_cast<CSeq_feat * >(&(obj.SetObject()));
    CSeq_inst* inst = dynamic_cast<CSeq_inst * >(&(obj.SetObject()));
    
    CScope& scope = obj.GetSEH().GetScope();
    if (desc) {
        if (obj.PreExists()) {
            const CSeqdesc* orig_desc = dynamic_cast<const CSeqdesc*>(obj.GetOriginalObject());
            if (orig_desc) {
                CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(Ref(&scope), *orig_desc);
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(seh, *orig_desc, *desc));
                cmd->AddCommand(*ecmd);
            }
        } else {
            cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(obj.GetSEH(), *desc)) );
        }
        any = true;
    } else if (feat) {
        if (obj.PreExists()) {
            const CSeq_feat* old_feat = dynamic_cast<const CSeq_feat * >(obj.GetOriginalObject());
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(scope.GetSeq_featHandle(*old_feat), *feat)));
        } else {
            CSeq_entry_Handle seh = obj.GetSEH();
            cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *feat)));
        }
        any = true;
    } else if (inst) {
        CCmdChangeBioseqInst* ecmd = new CCmdChangeBioseqInst(obj.GetSEH().GetSeq(), *inst);
        cmd->AddCommand (*ecmd);
        any = true;
    }

    if (!any) {
        cmd.Reset(NULL);
    }
    return cmd;
}

void GetViewObjects(IWorkbench* workbench, TConstScopedObjects &objects)
{
    if (!workbench)
        return;
    IViewManagerService* view_srv = workbench->GetServiceByType<IViewManagerService>();
    if (!view_srv) return;
    IViewManagerService::TViews views;
    view_srv->GetViews(views);
    NON_CONST_ITERATE(IViewManagerService::TViews, it, views) {
        IProjectView* project_view = dynamic_cast<IProjectView*>((*it).GetPointer());
        if (project_view && project_view->GetLabel(IProjectView::eType) == "Text View") {
            objects.clear();
            project_view->GetActiveObjects(objects);
            if (objects.empty())
                project_view->GetMainObject(objects);
        }
    }

    if (objects.empty())
    {
        NON_CONST_ITERATE(IViewManagerService::TViews, it, views) {
            CProjectTreeView* projectTree = dynamic_cast<CProjectTreeView*>((*it).GetPointer());
            if (projectTree && objects.empty()) {
                projectTree->GetSelection(objects);
                if (objects.empty())
                    projectTree->GetSingleObject(objects);
            }
        }
    }
}

int GetNumberOfViewedBioseqs(TConstScopedObjects& objects)
{
    int bioseqs = 0;
    ITERATE(TConstScopedObjects, it, objects) {
        const CObject* ptr = it->object.GetPointer();
        const CSeq_loc* seqloc = dynamic_cast<const CSeq_loc*>(ptr);
        const CBioseq* bioseq = dynamic_cast<const CBioseq* >(ptr);
        if (seqloc || bioseq) {
            bioseqs++;
        }
    }

    return bioseqs;
}

bool FocusedOnNucleotides(TConstScopedObjects& objects, const CSeq_entry_Handle& tse)
{
    int NAseqs = 0;
    CBioseq_CI b_iter(tse, CSeq_inst::eMol_na);
    for (; b_iter; ++b_iter, ++NAseqs);

    if (NAseqs == 1 && objects.size() == 2) {
        // it might be scoped on the sequence or on 'Nucleotides'
        return false;
    }
    
    int viewed_bseqs = GetNumberOfViewedBioseqs(objects);
    return (viewed_bseqs == NAseqs);
}

bool FocusedOnAll(TConstScopedObjects& objects, const objects::CSeq_entry_Handle& tse)
{
    int NAseqs = 0;
    CBioseq_CI b_iter(tse, CSeq_inst::eMol_na);
    for (; b_iter; ++b_iter, ++NAseqs);

    if (NAseqs == 1 && objects.size() == 2) {
        // it might be scoped on the sequence or on 'Nucleotides'
        return false;
    }

    int viewed_bseqs = GetNumberOfViewedBioseqs(objects);
    return (viewed_bseqs > NAseqs);
}


END_NCBI_SCOPE
