/*  $Id: macro_fn_aecr.cpp 44877 2020-04-02 20:22:08Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 */

#include <ncbi_pch.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/seqblock/GB_block.hpp>
#include <objects/seq/Seqdesc.hpp>

#include <objects/valid/Comment_rule.hpp>
#include <objects/valid/Comment_set.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/general_macros.hpp>
#include <util/xregexp/regexp.hpp>
#include <objmgr/util/sequence.hpp>

#include <objtools/cleanup/cleanup.hpp>
#include <objtools/edit/field_handler.hpp>

#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/util_cmds.hpp>

#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_field_resolve.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

// All DO functions should make changes on the "Edited" object of the BioDataIterator

/// class CMacroFunction_SwapRelatedFeaturesQual_Depr
/// SwapRelateFeaturesQual(src_field, dest_feat_subtype, dest_field);
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SwapRelatedFeaturesQual_Depr, "SwapRelateFeaturesQual")
void CMacroFunction_SwapRelatedFeaturesQual_Depr::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* src_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!src_feat || !scope)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();

    CMQueryNodeValue::TObs src_objs;
    CMQueryNodeValue::EType src_type = m_Args[0]->GetDataType();

    // the source field has to be set
    if (src_type == CMQueryNodeValue::eString) {
        if (!GetFieldsByName(&src_objs, oi, m_Args[0]->GetString()))
            return;
    }
    else if (src_type == CMQueryNodeValue::eObjects) {
        src_objs = m_Args[0]->GetObjects();
    }
    else if (src_type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(src_objs, 0);
    }

    if (src_objs.empty()) {
        return;
    }

    // the destination field also has to be set and 
    // should be a field from the corresponding feature
    CSeqFeatData::ESubtype related_feature = NMacroUtil::GetFeatSubtype(m_Args[1]->GetString());
    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(*src_feat, related_feature, scope);
    if (feat_list.empty()) {
        return;
    }

    CConstRef<CSeq_feat> orig_feat = *feat_list.begin();
    CRef<CSeq_feat> new_feat(new CSeq_feat);
    new_feat->Assign(*orig_feat);
    CObjectInfo objInfo(new_feat.GetPointer(), new_feat.GetPointer()->GetTypeInfo());

    CMQueryNodeValue::TObs dest_objs;
    const string& dest_field = m_Args[2]->GetString();
    if (!GetFieldsByName(&dest_objs, objInfo, dest_field) || dest_objs.empty()) {
        return;
    }

    CMQueryNodeValue::TObs::iterator src_it = src_objs.begin();
    CMQueryNodeValue::TObs::iterator dest_it = dest_objs.begin();

    while (src_it != src_objs.end() && dest_it != dest_objs.end()) {
        CObjectInfo src_prim = NMacroUtil::GetPrimitiveObjInfo(src_it->field);
        CObjectInfo dest_prim = NMacroUtil::GetPrimitiveObjInfo(dest_it->field);
        if (CMacroFunction_SwapQual::s_SwapFields(src_prim, dest_prim)) {
            m_QualsChangedCount += 2;
        }
        ++src_it;
        ++dest_it;
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();

        CRef<CCmdComposite> cmd(new CCmdComposite("Change Related feature"));
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(scope->GetSeq_featHandle(*orig_feat), *new_feat)));
        m_DataIter->RunCommand(cmd, m_CmdComposite);

        CNcbiOstrstream log;
        log << m_QualsChangedCount << ";swapping qualifiers";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SwapRelatedFeaturesQual_Depr::x_ValidArguments() const
{
    if (m_Args.size() != 3) {
        return false;
    }

    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    bool first_ok = (type == CMQueryNodeValue::eString)
        || (type == CMQueryNodeValue::eObjects)
        || (type == CMQueryNodeValue::eRef);
    if (!first_ok) {
        return false;
    }

    for (size_t index = 1; index < m_Args.size(); ++index) {
        if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString) {
            return false;
        }
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveQual
/// RemoveQual("genome");
/// or
/// obj = PUB_AFFIL("sub"); 
/// RemoveQual(obj);
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveQual, "RemoveQual")

void CMacroFunction_RemoveQual::TheFunction()
{
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    if (type == CMQueryNodeValue::eString) {
        const string& field_name = m_Args[0]->GetString();
        if (!GetFieldsByName(&res_oi, oi, field_name))
            return;
    }
    else if (type == CMQueryNodeValue::eObjects) {
        res_oi = m_Args[0]->GetObjects();
    }
    else if (type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(res_oi, 0);
    }

    if (res_oi.empty()) {
        return;
    }

    m_QualsChangedCount = s_RemoveFields(m_DataIter, res_oi);

    if (m_QualsChangedCount) {
        CNcbiOstrstream log;
        log << m_QualsChangedCount << ";removal of qualifiers";
        x_LogFunction(log);
    }
}

bool CMacroFunction_RemoveQual::x_ValidArguments() const
{
    if (m_Args.empty()) return false;
    bool first_ok = (m_Args[0]->IsString() || m_Args[0]->AreObjects() || m_Args[0]->IsRef());      
    return (m_Args.size() == 1 && first_ok);
}

Int4 CMacroFunction_RemoveQual::s_RemoveFields(CIRef<IMacroBioDataIter>& dataiter, CMQueryNodeValue::TObs& objs)
{
    if (objs.empty())
        return 0;

    Int4 quals_changed = 0;
    CObjectInfo oi = dataiter->GetEditedObject();

    // special attention to gene qualifiers that are removed from gene Xrefs
    bool is_gene_suppressed_before = false;
    bool is_gene_suppressed_after = false;
    CConstRef<CObject> obj = dataiter->GetScopedObject().object;
    if (dynamic_cast<const CSeq_feat*>(obj.GetPointer())) {
        CSeq_feat* feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
        auto gene_xref = feat->GetGeneXref();
        is_gene_suppressed_before = gene_xref && gene_xref->IsSuppressed();
    }

    bool is_taxname = NMacroUtil::IsTaxname(objs.front());
    NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, objs) {
        if (NStr::EqualNocase(it->parent.GetName(), "Name-std")) {
            CObjectInfoMI mem = it->parent.FindClassMember("first");
            if (mem.IsSet() && mem.GetMember() == it->field) {
                CName_std* std_name = CTypeConverter<CName_std>::SafeCast(it->parent.GetObjectPtr());
                NMacroUtil::RemoveFirstName(*std_name);
                quals_changed++;
            }
            else {
                if (RemoveFieldByName(*it)) {
                    quals_changed++;
                }
            }
        }
        else {
            if (RemoveFieldByName(*it)) {
                quals_changed++;
            }
        }
    }

    if (quals_changed) {
        if (is_taxname) {
            NMacroUtil::CleanupForTaxnameChange(oi);
        }
        CConstRef<CObject> obj = dataiter->GetScopedObject().object;
        CRef<CScope> scope = dataiter->GetScopedObject().scope;

        if (dynamic_cast<const CUser_object*>(obj.GetPointer())) {
            CUser_object* userobj = CTypeConverter<CUser_object>::SafeCast(oi.GetObjectPtr());
            if (userobj->IsSetData() && userobj->GetData().empty()) {
                dataiter->SetToDelete(true);
            }
            else if (userobj->GetType().IsStr() && NStr::EqualCase(userobj->GetType().GetStr(), "DBLink")) {
                EDIT_EACH_USERFIELD_ON_USEROBJECT(field_it, *userobj) {
                    CUser_field& field = **field_it;
                    if (field.IsSetData()) {
                        if ((field.GetData().IsStrs() && field.GetData().GetStrs().empty()) ||
                            (field.GetData().IsStr() && field.GetData().GetStr().empty())) {
                            ERASE_USERFIELD_ON_USEROBJECT(field_it, *userobj);
                        }
                    }
                }
            }
        }
        else if (dynamic_cast<const CSeqdesc*>(obj.GetPointer())) {
            CSeqdesc* desc = CTypeConverter<CSeqdesc>::SafeCast(oi.GetObjectPtr());
            if (desc->IsGenbank() && desc->GetGenbank().IsEmpty()) {
                dataiter->SetToDelete(true);
            }
        }
        else if (dynamic_cast<const CBioseq*>(obj.GetPointer())) {
            CBioseq* bseq = CTypeConverter<CBioseq>::SafeCast(oi.GetObjectPtr());
            NMacroUtil::RemoveEmptyDescriptors(*bseq);
        } else {
            CCleanup cleanup;
            if (dynamic_cast<const CSeq_feat*>(obj.GetPointer())) {
                CSeq_feat* seq_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
                cleanup.BasicCleanup(*seq_feat);
                if (seq_feat->IsSetXref()) {
                    auto gene_xref = seq_feat->GetGeneXref();
                    if (gene_xref) {
                        is_gene_suppressed_after = gene_xref->IsSuppressed();
                        if (!is_gene_suppressed_before && is_gene_suppressed_after)
                            NMacroUtil::RemoveGeneXref(*seq_feat);
                    }
                }
            }
            else if (dynamic_cast<const CBioSource*>(obj.GetPointer())) {
                CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
                cleanup.BasicCleanup(*bsrc);
                if (bsrc->IsSetOrg() && bsrc->GetOrg().IsSetDb() && bsrc->GetOrg().GetDb().empty()) {
                    bsrc->SetOrg().ResetDb();
                }
            }
        }

        dataiter->SetModified();
    }
    return quals_changed;
}


// cleanup for biosource
static void s_ResetModSubsrcQuals(CBioSource& bsrc)
{
    if (bsrc.IsSetOrgMod()) {
        if (bsrc.GetOrg().GetOrgname().GetMod().empty()) {
            bsrc.SetOrg().SetOrgname().ResetMod();
        }
    }

    if (bsrc.IsSetSubtype()) {
        if (bsrc.GetSubtype().empty()) {
            bsrc.ResetSubtype();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveModifier
/// o = Resolve(path_to_container) Where o.subtype = subtype;
/// RemoveModifier(o) - deleting subtype modifier from the list of modifiers
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveModifier, "RemoveModifier")

void CMacroFunction_RemoveModifier::TheFunction()
{
    // objects to be removed:
    CMQueryNodeValue::TObs res_oi;
    x_GetObjectsFromRef(res_oi, 0);
    if (res_oi.empty()) {
        // nothing to delete
        return;
    }

    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioSource* c_bsrc = dynamic_cast<const CBioSource*>(obj.GetPointer());
    // the asn selector should be a biosource
    if (!c_bsrc) {
        return;
    }

    NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, res_oi) {
        if (RemoveFieldByName(*it)) {
            m_QualsChangedCount++;
        }
    }

    if (m_QualsChangedCount) {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
        s_ResetModSubsrcQuals(*bsrc);
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_QualsChangedCount << ";removing source modifiers";
        x_LogFunction(log);
    }
}

bool CMacroFunction_RemoveModifier::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eRef);
}


///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_RemoveOutside
/// RemoveOutsideStringQual(field_name(str/obj), before_match(b), left_del(str), rmv_left(b), 
///                         after_match(b), right_del(str), rmv_right(b), case_insensitive(b), whole_word(b), update_mrna(b))
/// Last parameter is optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveOutside, "RemoveOutsideStringQual")

void CMacroFunction_RemoveOutside::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();

    if (type == CMQueryNodeValue::eString) {
        if (!GetFieldsByName(&res_oi, oi, m_Args[0]->GetString()))
            return;
    }
    else if (type == CMQueryNodeValue::eObjects) {
        res_oi = m_Args[0]->GetObjects();
    }
    else if (type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(res_oi, 0);
        NMacroUtil::SwapGbQualWithValues(res_oi);
    }

    if (res_oi.empty()) {
        return;
    }


    CRef<CRemoveTextOptions> options = x_GetRemoveTextOptions(1);
    x_RmvOutsideFields(res_oi, *options);

    if (m_QualsChangedCount) {
        NMacroUtil::CleanupForTaxnameChange(res_oi.front(), oi);
        m_DataIter->SetModified();

        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": removed text outside string in " << m_QualsChangedCount << " qualifiers";

        bool update_mrna = (m_Args.size() == 10) ? m_Args[9]->GetBool() : false;
        CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(object.GetPointer());
        
        if (update_mrna && feat && feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
            string message;
            CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
            CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
            string prot_product = edit_feat->GetData().GetProt().GetName().front();
            CRef<CCmdComposite> cmd = UpdatemRNAProduct(prot_product, object, *scope);
            if (cmd) {
                m_DataIter->RunCommand(cmd, m_CmdComposite);
                log << ", applied " + prot_product + " to mRNA product name ";
            }
        }
        x_LogFunction(log);
    }
}

CRef<CRemoveTextOptions> CMacroFunction_RemoveOutside::x_GetRemoveTextOptions(size_t start_index) const
{
    CRemoveTextOptions::EMatchType before_match = CRemoveTextOptions::eNone;
    string left_del = m_Args[start_index + 1]->GetString();
    bool remove_before_match = false;
    if (m_Args[start_index]->GetBool()) {
        if (NStr::EqualCase(left_del, "eDigits")) {
            before_match = CRemoveTextOptions::eDigits;
            left_del = kEmptyStr;
        }
        else if (NStr::EqualCase(left_del, "eLetters")) {
            before_match = CRemoveTextOptions::eLetters;
            left_del = kEmptyStr;
        }
        else {
            before_match = CRemoveTextOptions::eText;
        }
        remove_before_match = m_Args[start_index + 2]->GetBool();
    }
    

    CRemoveTextOptions::EMatchType after_match = CRemoveTextOptions::eNone;
    string right_del = m_Args[start_index + 4]->GetString();
    bool remove_after_match = false;
    if (m_Args[start_index + 3]->GetBool()) {
        if (NStr::EqualCase(right_del, "eDigits")) {
            after_match = CRemoveTextOptions::eDigits;
            right_del = kEmptyStr;
        }
        else if (NStr::EqualCase(right_del, "eLetters")) {
            after_match = CRemoveTextOptions::eLetters;
            right_del = kEmptyStr;
        }
        else {
            after_match = CRemoveTextOptions::eText;
        }
        remove_after_match = m_Args[start_index + 5]->GetBool();
    }
    
    bool case_insensitive = m_Args[start_index + 6]->GetBool();
    bool whole_word = m_Args[start_index + 7]->GetBool();

    CRef<CRemoveTextOptions> options(new CRemoveTextOptions(before_match, left_del, remove_before_match,
                                                            after_match, right_del, remove_after_match,
                                                            case_insensitive, whole_word));
    return options;
}

void CMacroFunction_RemoveOutside::x_RmvOutsideFields(CMQueryNodeValue::TObs& resolved_objs, const CRemoveTextOptions& options)
{
    for (auto& it : resolved_objs) {
        CMQueryNodeValue::TObs objs;
        NMacroUtil::GetPrimitiveObjectInfos(objs, it);
        for (auto& iter : objs) {
            CObjectInfo obj_oi = iter.field;
            if (obj_oi.GetPrimitiveValueType() == ePrimitiveValueString) {
                string value = obj_oi.GetPrimitiveValueString();
                if (options.EditText(value)) {
                    SetQualStringValue(obj_oi, value);
                }
            }
            else if (obj_oi.GetPrimitiveValueType() == ePrimitiveValueEnum) {
                string value;
                try {
                    value = obj_oi.GetPrimitiveValueString();
                }
                catch (const CException&) {
                    value = NStr::NumericToString(obj_oi.GetPrimitiveValueInt4());
                }
                if (options.EditText(value)) {
                    try {
                        obj_oi.SetPrimitiveValueString(value);
                        m_QualsChangedCount++;
                    }
                    catch (const CException&) {
                        // if this fails, try to convert the string to int and assign again
                        try {
                            obj_oi.SetPrimitiveValueInt4(NStr::StringToLong(value));
                            m_QualsChangedCount++;
                        }
                        catch (const CException&) {}
                    }
                }
            }
        }
    }
}

bool CMacroFunction_RemoveOutside::x_ValidArguments() const
{
    if (m_Args.size() != 9 && m_Args.size() != 10)
        return false;

    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    bool first_ok = (type == CMQueryNodeValue::eString) || (type == CMQueryNodeValue::eObjects) || (type == CMQueryNodeValue::eRef);
    if (!first_ok)
        return false;

    return x_CheckArguments(1);
}

bool CMacroFunction_RemoveOutside::x_CheckArguments(size_t index) const
{
    if (m_Args[index]->GetDataType() != CMQueryNodeValue::eBool
        || m_Args[++index]->GetDataType() != CMQueryNodeValue::eString
        || m_Args[++index]->GetDataType() != CMQueryNodeValue::eBool) {
        return false;
    }
    if (m_Args[++index]->GetDataType() != CMQueryNodeValue::eBool
        || m_Args[++index]->GetDataType() != CMQueryNodeValue::eString
        || m_Args[++index]->GetDataType() != CMQueryNodeValue::eBool) {
        return false;
    }
    ++index;
    for (size_t i = index; i < m_Args.size(); ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool)
            return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetStringQual
/// Expected Syntax:
/// SetStringQual(field_name, newValue, existingtext_option, delimiter, remove_blank)
/// The last two parameters are optional. 
/// Empty new values don't have any effect by default. If remove_blank is True,
/// the existing field is removed.
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetStringQual, "SetStringQual")

void CMacroFunction_SetStringQual::TheFunction()
{
    size_t index = 1;
    string newValue = NMacroUtil::GetStringValue(m_Args[index]);
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();

    if (type == CMQueryNodeValue::eString) {
        if (remove_field && newValue.empty()) {
            if (!GetFieldsByName(&res_oi, oi, m_Args[0]->GetString()))
                return;
        }
        else if (!newValue.empty() && !SetFieldsByName(&res_oi, oi, m_Args[0]->GetString())) {
            return;
        }
    }
    else if (type == CMQueryNodeValue::eObjects) {
        res_oi = m_Args[0]->GetObjects();
    }
    else if (type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(res_oi, 0);
    }

    if (res_oi.empty()) {
        return;
    }

    if (!newValue.empty()) {
        vector<string> new_values;
        edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);
        x_SetFields(res_oi, newValue, existing_text, new_values);

        m_Result->SetBool(false);
        if (m_QualsChangedCount) {
            NMacroUtil::CleanupForTaxnameChange(res_oi.front(), oi);
            m_DataIter->SetModified();
            m_Result->SetBool(true);

            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr();
            for (size_t i = 0; i < new_values.size(); ++i) {
                log << ": set '" << new_values[i] << "' as a new value\n";
            }
            x_LogFunction(log);
        }
    }
    else if (remove_field) {
        m_QualsChangedCount = CMacroFunction_RemoveQual::s_RemoveFields(m_DataIter, res_oi);
        if (m_QualsChangedCount) {
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " qualifiers";
            x_LogFunction(log);
        }
    }
}

bool CMacroFunction_SetStringQual::x_ValidArguments() const
{
    // can accept as its first parameter: objects, string or reference
    size_t arg_nr = m_Args.size();
    if (arg_nr < 3 && arg_nr > 5) {
        return false;
    }

    size_t index = 0;
    bool first_ok = m_Args[index]->IsString() || m_Args[index]->AreObjects() || m_Args[index]->IsRef();
    if (!first_ok) return false;

    ++index;
    NMacroUtil::GetPrimitiveFromRef(m_Args[index].GetNCObject());
    bool second_ok = m_Args[index]->IsString() || m_Args[index]->IsInt() || m_Args[index]->IsDouble();
    if (!second_ok) return false;

    if (!m_Args[++index]->IsString()) return false;

    if (arg_nr > 3 && (!m_Args[++index]->IsString() && !m_Args[index]->IsBool())) return false;

    if (arg_nr > 4 && !m_Args[++index]->IsBool()) return false;

    return true;
}

void CMacroFunction_SetStringQual::x_SetFields(CMQueryNodeValue::TObs& objs, const string& newValue,
    edit::EExistingText existing_text, vector<string>& new_values)
{
    for (auto& it : objs) {
        CObjectInfo obj = it.field;
        switch (obj.GetTypeFamily()) {
        case eTypeFamilyPrimitive: // works when the field exists or does not
            if (existing_text != edit::eExistingText_add_qual) {
                x_SetNewPrimitiveValue(obj, newValue, existing_text, new_values);
            }
            else if (it.parent.GetTypeFamily() == eTypeFamilyContainer) {
                // add new element to the container
                CObjectInfo new_oi(it.parent.AddNewElement());
                SetQualStringValue(new_oi, newValue);
                new_values.push_back(newValue);
            }
            break;
        case eTypeFamilyPointer:  // works only when the field exists
            if (obj.GetPointedObject().GetTypeFamily() == eTypeFamilyPrimitive) {
                CObjectInfo oi_ptr(obj.GetPointedObject());
                x_SetNewPrimitiveValue(oi_ptr, newValue, existing_text, new_values);
            }
            break;
        case eTypeFamilyContainer: { // works when the field exists or does not
            CMQueryNodeValue::TObs objs;
            NMacroUtil::GetPrimitiveObjectInfos(objs, it);

            if (objs.empty()) { // add new element when the container is empty
                CObjectInfo new_oi(obj.AddNewElement());
                SetQualStringValue(new_oi, newValue);
                new_values.push_back(newValue);
            }
            else {
                if (existing_text == edit::eExistingText_replace_old && objs.size() > 1) {
                    // keep the first element, update it and delete the rest from the container
                    CObjectInfoEI first = obj.BeginElements();
                    CObjectInfoEI e = first;
                    ++e;
                    while (e) {
                        e.Erase();
                        e = first;
                        ++e;
                    }
                    SetQualStringValue(objs.front().field, newValue);
                    new_values.push_back(newValue);
                }
                else {
                    // there are multiple ones, and all of them needs to be updated
                    NON_CONST_ITERATE(CMQueryNodeValue::TObs, iter, objs) {
                        x_SetNewPrimitiveValue(iter->field, newValue, existing_text, new_values);
                    }
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

void CMacroFunction_SetStringQual::x_SetNewPrimitiveValue(CObjectInfo& oi,
    const string& newValue, edit::EExistingText existing_text, vector<string>& new_values)
{
    _ASSERT(oi.GetTypeFamily() == eTypeFamilyPrimitive);

    if (oi.GetPrimitiveValueType() == ePrimitiveValueString ||
        oi.GetPrimitiveValueType() == ePrimitiveValueEnum) {

        string orig_value = oi.GetPrimitiveValueString();
        if (edit::AddValueToString(orig_value, newValue, existing_text)) {
            if (SetQualStringValue(oi, orig_value)) {
                new_values.push_back(orig_value);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///class CMacroFunction_SetRnaProduct
/// SetRnaProduct(new_value, action_type, delimiter, remove_blank)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetRnaProduct, "SetRnaProduct")
void CMacroFunction_SetRnaProduct::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(object.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!seq_feat || !seq_feat->GetData().IsRna() || !scope)
        return;

    size_t index = 0;
    string newValue = NMacroUtil::GetStringValue(m_Args[index]);
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    CRNA_ref& rna_ref = edit_feat->SetData().SetRna();
    string orig_value = rna_ref.GetRnaProductName();

    if (!newValue.empty()) {
        string remainder;
        if (edit::AddValueToString(orig_value, newValue, existing_text)) {
            rna_ref.SetRnaProductName(orig_value, remainder);
            m_QualsChangedCount++;
            if (!remainder.empty()) {
                if (edit_feat->IsSetComment()) {
                    existing_text = edit::eExistingText_append_semi;
                    edit::AddValueToString(edit_feat->SetComment(), remainder, existing_text);
                }
            }
        }
    }
    else if (remove_field) {
        string remainder;
        rna_ref.SetRnaProductName(kEmptyStr, remainder);
        m_QualsChangedCount++;
    }
    
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();

        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr();
        if (newValue.empty() && remove_field) {
            log << ": removed rna product";
        }
        else {
            log << ": set '" << orig_value << "' as new rna product value";
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetRnaProduct::x_ValidArguments() const
{
    // can accept as its first parameter: objects, string or reference
    size_t arg_nr = m_Args.size();
    if (arg_nr < 2 && arg_nr > 4) {
        return false;
    }

    size_t index = 0;
    NMacroUtil::GetPrimitiveFromRef(m_Args[index].GetNCObject());
    bool first_ok = m_Args[index]->IsString() || m_Args[index]->IsInt() || m_Args[index]->IsDouble();
    if (!first_ok) return false;

    if (!m_Args[++index]->IsString()) return false;
    if (arg_nr > 2 && (!m_Args[++index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (arg_nr > 3 && !m_Args[++index]->IsBool()) return false;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_GetRnaProduct
/// GetRnaProduct()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_GetRnaProduct, "GETRNAPRODUCT")

void CMacroFunction_GetRnaProduct::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    m_Result->SetNotSet();
    if (!feat || !scope || !(feat->IsSetData() && feat->GetData().IsRna()))
        return;

    const CRNA_ref& rna_ref = feat->GetData().GetRna();
    string value = rna_ref.GetRnaProductName();

    if (m_Nested == eNotNested) { // return a string
        m_Result->SetString(value);
    }
    else {
        // return a reference to a CMQueryNodeValue of type string
        CRef<CMQueryNodeValue> new_node(new CMQueryNodeValue);
        new_node->SetString(value);
        m_Result->SetRef(new_node);
    }
}

bool CMacroFunction_GetRnaProduct::x_ValidArguments() const
{
    return (m_Args.empty());
}
///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveRnaProduct
/// RemoveRnaProduct()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveRnaProduct, "RemoveRnaProduct")
void CMacroFunction_RemoveRnaProduct::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!edit_feat || !edit_feat->GetData().IsRna() || !scope)
        return;

    CRNA_ref& rna_ref = edit_feat->SetData().SetRna();
    string remainder;
    rna_ref.SetRnaProductName(kEmptyStr, remainder);

    m_DataIter->SetModified();
    m_QualsChangedCount++;

    CNcbiOstrstream log;
    log << m_QualsChangedCount << ";removing RNA product";
    x_LogFunction(log);
}

bool CMacroFunction_RemoveRnaProduct::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_EditStringQual
/// Usage: EditStringQual(field_name|rt_obj, find_text, repl_text, location, case_sensitive, is_regex)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_EditStringQual, "EditStringQual")

void CMacroFunction_EditStringQual::TheFunction()
{
    size_t index = 1;
    const string& find_txt = NMacroUtil::GetStringValue(m_Args[index]);
    string repl_txt = NMacroUtil::GetStringValue(m_Args[++index]);
    const string& location = m_Args[++index]->GetString();
    bool case_sensitive = m_Args[++index]->GetBool();
    bool is_regex = (++index < m_Args.size()) ? m_Args[index]->GetBool() : false;

    m_Result->SetBool(false);  // assume, that no change has been made

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();

    if (type == CMQueryNodeValue::eString) {
        if (!GetFieldsByName(&res_oi, oi, m_Args[0]->GetString()))
            return;
    }
    else if (type == CMQueryNodeValue::eObjects) {
        res_oi = m_Args[0]->GetObjects();
    }
    else if (type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(res_oi, 0);
        NMacroUtil::SwapGbQualWithValues(res_oi);
    }

    if (res_oi.empty()) {
        return;
    }

    x_EditFields(res_oi, find_txt, repl_txt, s_GetLocFromName(location), case_sensitive, is_regex);
    if (m_QualsChangedCount) {
        NMacroUtil::CleanupForTaxnameChange(res_oi.front(), oi);

        m_DataIter->SetModified();
        m_Result->SetBool(true);  // the field has been modified

        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": edited " << m_QualsChangedCount << " qualifiers, replaced " << find_txt;
        if (NStr::IsBlank(repl_txt)) {
            repl_txt.assign("''");
        }
        log << " with " << repl_txt;
        x_LogFunction(log);
    }
}

bool CMacroFunction_EditStringQual::s_EditText(string& str, const string& find,
    const string& replace, ESearchLoc loc, bool case_sensitive, bool is_regex)
{
    bool rval = false;
    if (is_regex)
    {
        CRegexp::TCompile options = CRegexp::fCompile_ignore_case;
        if (case_sensitive)
        {
            options = CRegexp::fCompile_default;
        }

        CRegexpUtil replacer(str);
        size_t num = replacer.Replace(find, replace, options, CRegexp::fMatch_default, 0);
        if (num > 0)
        {
            str = replacer.GetResult();
            rval = true;
        }
    }
    else
    {
        SIZE_TYPE pos = NPOS;
        if (loc == eAnywhere) {
            pos = NStr::Find(str, find, (case_sensitive) ? NStr::eCase : NStr::eNocase);
            while (pos != NPOS) {
                str = str.substr(0, pos) + replace + str.substr(pos + find.length());
                if (case_sensitive) {
                    pos = NStr::FindCase(str, find, pos + replace.length());
                }
                else {
                    pos = NStr::FindNoCase(str, find, pos + replace.length());
                }
                rval = true;
            }
        }
        else {
            pos = NPOS;
            if (loc == eBeginning) {
                if (NStr::StartsWith(str, find, (case_sensitive) ? NStr::eCase : NStr::eNocase)) {
                    pos = 0;
                }
            }
            else if (loc == eEnd) {
                if (NStr::EndsWith(str, find, (case_sensitive) ? NStr::eCase : NStr::eNocase)) {
                    pos = str.length() - find.length();
                }
            }
            if (pos != NPOS) {
                str = str.substr(0, pos) + replace + str.substr(pos + find.length());
                rval = true;
            }
        }
    }
    NStr::TruncateSpacesInPlace(str);

    return rval;
}

void CMacroFunction_EditStringQual::x_EditFields(
    CMQueryNodeValue::TObs& resolved_objs, 
    const string& find_txt, 
    const string& repl_txt, 
    ESearchLoc loc,
    bool case_sensitive,
    bool is_regex)
{
    for (auto& it : resolved_objs) {
        CMQueryNodeValue::TObs objs;
        NMacroUtil::GetPrimitiveObjectInfos(objs, it);
        for(auto& iter : objs) {
            CObjectInfo oi = iter.field;
            if (oi.GetPrimitiveValueType() == ePrimitiveValueString) {
                string value = oi.GetPrimitiveValueString();
                if (s_EditText(value, find_txt, repl_txt, loc, case_sensitive, is_regex)) {
                    if (NStr::EqualNocase(iter.parent.GetName(), "Name-std")) {
                        CObjectInfoMI mem = iter.parent.FindClassMember("first");
                        if (mem.IsSet() && mem.GetMember() == oi) {
                            CName_std* std_name = CTypeConverter<CName_std>::SafeCast(iter.parent.GetObjectPtr());
                            NMacroUtil::ApplyFirstName(*std_name, value, edit::eExistingText_replace_old);
                            m_QualsChangedCount++;
                        }
                        SetQualStringValue(oi, value);
                    }
                    else {
                        SetQualStringValue(oi, value);
                    }
                }
            }
            else if (oi.GetPrimitiveValueType() == ePrimitiveValueInteger) {
                Int8 value = oi.GetPrimitiveValueInt8();
                string orig_val = NStr::Int8ToString(value);
                if (s_EditText(orig_val, find_txt, repl_txt, loc, case_sensitive, is_regex)) {
                    oi.SetPrimitiveValueInt8(NStr::StringToInt8(orig_val));
                    m_QualsChangedCount++;
                }
            }

            else if (oi.GetPrimitiveValueType() == ePrimitiveValueEnum) {
                string value;
                try {
                    value = oi.GetPrimitiveValueString();
                }
                catch (const CException&) {
                    value = NStr::NumericToString(oi.GetPrimitiveValueInt4());
                }
                if (s_EditText(value, find_txt, repl_txt, loc, case_sensitive, is_regex)) {
                    try {
                        oi.SetPrimitiveValueString(value);
                        m_QualsChangedCount++;
                    }
                    catch (const CException&) {
                        // if this fails, try to convert the string to int and assign again
                        try {
                            oi.SetPrimitiveValueInt4(NStr::StringToLong(value));
                            m_QualsChangedCount++;
                        }
                        catch (const CException&) {}
                    }
                }
            }
        }
    }
}

CMacroFunction_EditStringQual::ESearchLoc CMacroFunction_EditStringQual::s_GetLocFromName(const string& name)
{
    ESearchLoc loc = eAnywhere;  // default search location
    if (NStr::EqualNocase(name, "at the beginning")) {
        loc = eBeginning;
    }
    else if (NStr::EqualNocase(name, "at the end")) {
        loc = eEnd;
    }
    return loc;
}

bool CMacroFunction_EditStringQual::x_ValidArguments() const
{
    if (m_Args.size() != 5 && m_Args.size() != 6)
        return false;

    bool first_ok = m_Args[0]->IsString() || m_Args[0]->AreObjects() || m_Args[0]->IsRef();
    if (!first_ok) {
        return false;
    }

    for (size_t index = 1; index < 3; index++) {
        if (!m_Args[index]->IsString() && !m_Args[index]->IsInt()) {
            return false;
        }
    }
    if (m_Args.size() == 6) {
        if (!m_Args.back()->IsBool())
            return false;
    }
    return (m_Args[3]->IsString() && m_Args[4]->IsBool());
}


///////////////////////////////////////////////////////////////////////////////
namespace {

    vector<string> kSatelliteTypes = { "satellite", "microsatellite", "minisatellite" };
    vector<string> kMobileETypeTypes = {
        "insertion sequence",
        "integron",
        "LINE",
        "MITE",
        "non-LTR retrotransposon",
        "other",
        "P-element",
        "retrotransposon",
        "SINE",
        "superintegron",
        "transposable element",
        "transposon"
    };
    bool GetBioSourceDestObjects(CObjectInfo& oi, const string& field_name, CMQueryNodeValue::TObs& dest_objs);

    bool SetFeatDestinationField(CRef<CSeq_feat> feat, const string& field_name, CMQueryNodeValue::TObs& result);

    bool IstRNAProductField(const CSeq_feat& feat, const string& field_name);

    bool GetDestinationObjects(CConstRef<CObject> object, CObjectInfo& oi, const string& field_name, CMQueryNodeValue::TObs& result)
    {
        const CBioSource* bsrc = dynamic_cast<const CBioSource*>(object.GetPointer());
        const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(object.GetPointer());
        if (bsrc) {
            return GetBioSourceDestObjects(oi, field_name, result);
        }
        if (seq_feat) {
            CSeq_feat* feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
            return SetFeatDestinationField(Ref(feat), field_name, result);
        }

        return SetFieldsByName(&result, oi, field_name);
    }

    bool GetBioSourceDestObjects(CObjectInfo& oi, const string& field_name, CMQueryNodeValue::TObs& dest_objs)
    {
        if (field_name.empty())
            return false;

        size_t orig_size = dest_objs.size();
        // check if the field denotes an orgmod or a subsource modifier
        if (NMacroUtil::IsBiosourceModifier(field_name)) {

            // extract it into a function that adds these modifier fields
            CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
            if (!bsrc)
                return false;
            // check if it's a subsource modifier
            if (CSubSource::IsValidSubtypeName(field_name, CSubSource::eVocabulary_insdc)) {
                CSubSource::TSubtype st = CSubSource::GetSubtypeValue(field_name, CSubSource::eVocabulary_insdc);

                EDIT_EACH_SUBSOURCE_ON_BIOSOURCE(subsrc, *bsrc) {
                    if ((*subsrc)->IsSetSubtype() && (*subsrc)->GetSubtype() == st) {
                        CObjectInfo subsrc_oi((*subsrc).GetPointer(), (*subsrc)->GetTypeInfo());
                        CObjectInfo name_oi = subsrc_oi.FindClassMember("name").GetMember();
                        dest_objs.push_back(CMQueryNodeValue::SResolvedField(subsrc_oi, name_oi));
                    }
                }

                if (dest_objs.empty()) {
                    CRef<CSubSource> sub_src(new CSubSource());
                    sub_src->SetSubtype(st);
                    bsrc->SetSubtype().push_back(sub_src);
                    CObjectInfo subsrc_oi(sub_src.GetPointer(), sub_src->GetTypeInfo());
                    CObjectInfo name_oi = subsrc_oi.FindClassMember("name").GetMember();
                    dest_objs.push_back(CMQueryNodeValue::SResolvedField(subsrc_oi, name_oi));
                }
                // check if it's an orgmod modifier
            }
            else if (COrgMod::IsValidSubtypeName(field_name, COrgMod::eVocabulary_insdc)) {
                COrgMod::TSubtype st = COrgMod::GetSubtypeValue(field_name, COrgMod::eVocabulary_insdc);

                EDIT_EACH_ORGMOD_ON_BIOSOURCE(orgmod, *bsrc) {
                    if ((*orgmod)->IsSetSubtype() && (*orgmod)->GetSubtype() == st) {
                        CObjectInfo orgmod_oi((*orgmod).GetPointer(), (*orgmod)->GetTypeInfo());
                        CObjectInfo subname_oi = orgmod_oi.FindClassMember("subname").GetMember();
                        dest_objs.push_back(CMQueryNodeValue::SResolvedField(orgmod_oi, subname_oi));
                    }
                }

                if (dest_objs.empty()) {
                    CRef<COrgMod> orgmod(new COrgMod());
                    orgmod->SetSubtype(st);
                    orgmod->SetSubname(kEmptyStr);
                    if (!bsrc->IsSetOrgname()) {
                        CRef<COrgName> orgname(new COrgName());
                        orgname->SetMod().push_back(orgmod);
                        bsrc->SetOrg().SetOrgname(*orgname);
                    }
                    else {
                        bsrc->SetOrg().SetOrgname().SetMod().push_back(orgmod);
                    }
                    CObjectInfo orgmod_oi(orgmod.GetPointer(), orgmod->GetTypeInfo());
                    CObjectInfo subname_oi = orgmod_oi.FindClassMember("subname").GetMember();
                    dest_objs.push_back(CMQueryNodeValue::SResolvedField(orgmod_oi, subname_oi));
                }
            }
        }
        else {
            SetFieldsByName(&dest_objs, oi, field_name);
        }

        return dest_objs.size() - orig_size > 0;
    }

        // feature related functions
    bool SetFeatDestinationField(CRef<CSeq_feat> feat, const string& field_name, CMQueryNodeValue::TObs& result)
    {
        if (feat.IsNull() || field_name.empty())
            return false;

        CObjectInfo dest_oi(feat.GetPointer(), feat->GetThisTypeInfo());
        if (SetFieldsByName(&result, dest_oi, field_name)) {
            return true;
        }

        size_t orig_size = result.size();
        string field = field_name;
        if (NMacroUtil::IsSatelliteSubfield(field_name)) {
            field = "satellite";
        }
        else if (NMacroUtil::StringsAreEquivalent(field_name, "mobile-element-type-type")) {
            field = "mobile-element-type";
        }

        // field_name is a possible GB qualifier
        EDIT_EACH_GBQUAL_ON_SEQFEAT(gbq_it, *feat) {
            if ((*gbq_it)->IsSetQual() && NStr::EqualNocase((*gbq_it)->GetQual(), field)) {
                CObjectInfo gbqual_oi((*gbq_it).GetPointer(), (*gbq_it)->GetTypeInfo());
                CObjectInfo val_oi = gbqual_oi.FindClassMember("val").GetMember();
                result.push_back(CMQueryNodeValue::SResolvedField(gbqual_oi, val_oi));
            }
        }

        if (result.empty()) {
            if (IstRNAProductField(*feat, field_name))
                return true; // returning true but the output argument results is empty!

            CRef<CGb_qual> new_gbqual(new CGb_qual(field, kEmptyStr));
            feat->SetQual().push_back(new_gbqual);
            CObjectInfo gbqual_oi(new_gbqual.GetPointer(), new_gbqual->GetTypeInfo());
            CObjectInfo val_oi = gbqual_oi.FindClassMember("val").GetMember();
            result.push_back(CMQueryNodeValue::SResolvedField(gbqual_oi, val_oi));
        }

        return result.size() - orig_size > 0;
    }

    bool GetFeatDestinationField(CRef<CSeq_feat> feat, const string& field_name, CMQueryNodeValue::TObs& result)
    {
        if (field_name.empty())
            return false;

        CObjectInfo dest_oi(feat.GetPointer(), feat->GetThisTypeInfo());
        if (GetFieldsByName(&result, dest_oi, field_name)) {
            return true;
        }

        string field = field_name;
        if (NMacroUtil::IsSatelliteSubfield(field_name)) {
            field = "satellite";
        }
        else if (NMacroUtil::StringsAreEquivalent(field_name, "mobile-element-type-type")) {
            field = "mobile-element-type";
        }

        // field_name is a possible GB qualifier
        EDIT_EACH_GBQUAL_ON_SEQFEAT(gbq_it, *feat) {
            if ((*gbq_it)->IsSetQual() && NStr::EqualNocase((*gbq_it)->GetQual(), field)) {
                CObjectInfo gbqual_oi((*gbq_it).GetPointer(), (*gbq_it)->GetTypeInfo());
                CObjectInfo val_oi = gbqual_oi.FindClassMember("val").GetMember();
                result.push_back(CMQueryNodeValue::SResolvedField(gbqual_oi, val_oi));
            }
        }
        return (!result.empty());
    }

    bool IstRNAProductField(const CSeq_feat& feat, const string& field_name)
    {
        return (feat.IsSetData() && 
            feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_tRNA &&
            NStr::EqualNocase(field_name, "tRNA::product"));
    }

    CRef<CSeq_feat> CreateNewGene(const CSeq_feat& src_feat, CScope& scope);

    CRef<CSeq_feat> CreateNewProtein(const CSeq_feat& src_feat, CScope& scope);

    CRef<CSeq_feat> CreateNewRelatedFeature(const CSeq_feat& src_feat, const string& field_name, CScope& scope)
    {
        if (NStr::EndsWith(field_name, "locus")) {
            return CreateNewGene(src_feat, scope);
        }
        else if (NMacroUtil::StringsAreEquivalent(field_name, "protein name") ||
            NMacroUtil::StringsAreEquivalent(field_name, "data.prot.name") ||
            NMacroUtil::StringsAreEquivalent(field_name, "data.prot.ec")) {
            return CreateNewProtein(src_feat, scope);
        }
        return CRef<CSeq_feat>(nullptr);
    }

    CRef<CSeq_feat> CreateNewGene(const CSeq_feat& src_feat, CScope& scope)
    {
        if (!src_feat.IsSetData() || !src_feat.IsSetLocation())
            return CRef<CSeq_feat>(nullptr);

        CRef<CSeq_feat> new_gene(new CSeq_feat);
        CConstRef<CSeq_loc> loc(&src_feat.GetLocation());

        CConstRef<CSeq_feat> cds_feat;
        if (src_feat.GetData().IsProt()) {
            CBioseq_Handle product = scope.GetBioseqHandle(src_feat.GetLocation());
            if (product) {
                cds_feat = sequence::GetCDSForProduct(product);
            }
        }
        else if (src_feat.GetData().IsCdregion()) {
            cds_feat = ConstRef(&src_feat);
        }

        if (cds_feat) {
            CSeq_feat_Handle mrna = feature::GetBestMrnaForCds(scope.GetSeq_featHandle(*cds_feat));
            if (mrna) {
                loc = Ref(&mrna.GetLocation());
            }
            else {
                loc = Ref(&cds_feat->GetLocation());
            }
        }

        CRef<CSeq_loc> gene_loc = sequence::Seq_loc_Merge(*loc, CSeq_loc::fMerge_SingleRange, &scope);
        new_gene->SetLocation(*gene_loc);
        new_gene->SetPartial(new_gene->GetLocation().IsPartialStart(eExtreme_Biological) || new_gene->GetLocation().IsPartialStop(eExtreme_Biological));
        return new_gene;
    }

    CRef<CSeq_feat> CreateNewProtein(const CSeq_feat& src_feat, CScope& scope)
    {
        if (!src_feat.IsSetData() || !src_feat.IsSetLocation())
            return CRef<CSeq_feat>(nullptr);

        CRef<CSeq_feat> prot(new CSeq_feat);
        if (src_feat.GetData().IsCdregion()) {
            if (src_feat.IsSetProduct()) {
                CBioseq_Handle product = scope.GetBioseqHandle(src_feat.GetProduct());
                if (product && !CFeat_CI(product, CSeqFeatData::eSubtype_prot)) {
                    SetProteinFeature(*prot, product, src_feat);
                }
            }
        }
        else if (src_feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA) {
            CSeq_feat_Handle cds = feature::GetBestCdsForMrna(scope.GetSeq_featHandle(src_feat));
            if (cds && cds.IsSetProduct()) {
                CBioseq_Handle product = scope.GetBioseqHandle(cds.GetProduct());
                if (product && !CFeat_CI(product, CSeqFeatData::eSubtype_prot)) {
                    SetProteinFeature(*prot, product, *cds.GetOriginalSeq_feat());
                }
            }
        }
        else if (src_feat.GetData().IsGene()) {
            list<CMappedFeat> cds_feats;
            feature::GetCdssForGene(scope.GetSeq_featHandle(src_feat), cds_feats);
            for (auto& it : cds_feats) {
                if (it.IsSetProduct()) {
                    CBioseq_Handle product = scope.GetBioseqHandle(it.GetProduct());
                    if (product && !CFeat_CI(product, CSeqFeatData::eSubtype_prot)) {
                        SetProteinFeature(*prot, product, src_feat);
                        continue;
                    }
                }
            }
        }
        else if (src_feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_mat_peptide_aa) {
            CBioseq_Handle product = scope.GetBioseqHandle(src_feat.GetLocation());
            if (product && !CFeat_CI(product, CSeqFeatData::eSubtype_prot)) {
                CConstRef<CSeq_feat> cds_feat(sequence::GetCDSForProduct(product));
                if (cds_feat) {
                    SetProteinFeature(*prot, product, *cds_feat);
                }
            }
        }
        return prot;
    }

}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_CopyStringQual
/// Expected syntax:
/// CopyStringQual( src_field, dest_field, action_type, delimiter)
/// the last parameter is optional
/// src_field_name and dest_field_name are subfields of the same object
/// Can not use this function to copy from taxname to protein description

DEFINE_MACRO_FUNCNAME(CMacroFunction_CopyStringQual, "CopyStringQual")

void CMacroFunction_CopyStringQual::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs src_objs;
    size_t index = 0;
    if (!x_GetSourceFields(oi, index, src_objs))
        return;

    CMQueryNodeValue::TObs dest_objs;
    if (!x_GetDestFields(oi, ++index, dest_objs))
        return;

    const string& action_type = m_Args[++index]->GetString();
    string delimiter = (++index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;
    m_ExistingText = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    ChangeFields(src_objs, dest_objs);

    if (m_QualsChangedCount) {
        NMacroUtil::CleanupForTaxnameChange(dest_objs.front(), oi);
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": copied " << m_QualsChangedCount << " qualifiers";
        x_LogFunction(log);
    }
}

bool CMacroFunction_CopyStringQual::x_ValidArguments() const
{
    size_t arg_size = m_Args.size();
    if (arg_size != 3 && arg_size != 4) {
        return false;
    }

    for (size_t i = 0; i < 2; ++i) {
        CMQueryNodeValue::EType type = m_Args[i]->GetDataType();
        bool type_ok = (type == CMQueryNodeValue::eString)
            || (type == CMQueryNodeValue::eObjects)
            || (type == CMQueryNodeValue::eRef);
        if (!type_ok)
            return false;
    }

    for (size_t i = 2; i < arg_size; ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eString)
            return false;
    }
    return true;
}

bool CMacroFunction_CopyStringQual::x_ChangeFields(CObjectInfo& src, CObjectInfo& dest)
{
    if (dest.GetTypeFamily() != eTypeFamilyPrimitive) {
        return false;
    }

    string src_val = x_GetSourceString(src);
    string dest_val = dest.GetPrimitiveValueString();
    if (edit::AddValueToString(dest_val, src_val, m_ExistingText)) {
        return SetQualStringValue(dest, dest_val);
    }
    return false;
}


// class CMacroFunction_CopyFeatQual
/// CopyFeatureQual((src_field, dest_feature_type, dest_feature_field, update_mrna, existing_text_opt, delimiter)
/// The last parameter is optional
/// The related mRNA is only updated if the action is successful and the destination feature is protein
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_CopyFeatQual, "CopyFeatureQual");
void CMacroFunction_CopyFeatQual::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* src_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!src_feat || !scope)
        return;

    const string& dest_field = m_Args[2]->GetString();
    if (dest_field.empty())
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs src_objs;
    if (!x_GetSourceFields(oi, 0, src_objs))
        return;

    // the destination field can be set at this point
    CSeqFeatData::ESubtype target_feature = NMacroUtil::GetFeatSubtype(m_Args[1]->GetString());
    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(*src_feat, target_feature, scope);
    CMQueryNodeValue::TObs dest_objs;
    bool changed = false, created = false;
    CRef<CSeq_feat> dest_feat;
    CSeq_feat_Handle fh;
    if (!feat_list.empty() && feat_list.size() == 1) {
        dest_feat.Reset(new CSeq_feat);
        dest_feat->Assign(*feat_list.front());
        if (!SetFeatDestinationField(dest_feat, dest_field, dest_objs)) {
            return;
        }
        fh = scope->GetSeq_featHandle(*feat_list.front());
        changed = true;
    }
    else {
        // new feature
        dest_feat = CreateNewRelatedFeature(*src_feat, dest_field, *scope);
        if (!dest_feat) return;
        if (!SetFeatDestinationField(dest_feat, dest_field, dest_objs)) {
            return;
        }
        created = true;
        if (src_feat->GetData().IsCdregion() && src_feat->IsSetXref() && dest_feat->GetData().IsProt()) {
            CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
            NON_CONST_ITERATE(CSeq_feat::TXref, it, edit_feat->SetXref()) {
                if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                    dest_feat->SetData().SetProt().Assign((*it)->GetData().GetProt());
                    edit_feat->SetXref().erase(it);
                    break;
                }
            }
            if (edit_feat->GetXref().empty()) {
                edit_feat->ResetXref();
            }
        }
    }

    size_t index = 3;
    bool update_mrna = m_Args[index]->GetBool();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter = (++index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;

    m_ExistingText = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    if (IstRNAProductField(*dest_feat, dest_field)) {
        for (auto&& it : src_objs) {
            CMQueryNodeValue::TObs src_prim_objs;
            NMacroUtil::GetPrimitiveObjectInfos(src_prim_objs, it);
            
            CObjectInfo src = src_prim_objs.front().field;
            string src_val = x_GetSourceString(src);

            string remainder;
            dest_feat->SetData().SetRna().SetRnaProductName(src_val, remainder);
            m_QualsChangedCount++;
        }
    }
    else {
        ChangeFields(src_objs, dest_objs);
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": copied " << m_QualsChangedCount << " qualifiers";

        CRef<CCmdComposite> cmd;
        if (changed && fh) {
            cmd.Reset(new CCmdComposite("Change Related feature"));
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *dest_feat)));
        }
        else if (created) {
            cmd.Reset(new CCmdComposite("Create feature"));
            CBioseq_Handle bsh = scope->GetBioseqHandle(dest_feat->GetLocation());
            cmd->AddCommand(*CRef<CCmdCreateFeatBioseq>(new CCmdCreateFeatBioseq(bsh, *dest_feat)));
        }

        if (update_mrna) {
            if (dest_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                string message;
                CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(*dest_feat, *scope, message);
                if (upd_cmd) {
                    cmd->AddCommand(*upd_cmd);
                    log << ", " << message;
                }
            }
            else if (src_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
                string prot_product = edit_feat->GetData().GetProt().GetName().front();
                CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(prot_product, obj, *scope);
                if (upd_cmd) {
                    cmd->AddCommand(*upd_cmd);
                    log << ", applied " + prot_product + " to mRNA product name ";
                }
            }
        }
        m_DataIter->RunCommand(cmd, m_CmdComposite);
        x_LogFunction(log);
    }
}

bool CMacroFunction_CopyFeatQual::x_ValidArguments() const
{
    if (m_Args.size() < 5 || m_Args.size() > 6)
        return false;

    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    bool first_ok = (type == CMQueryNodeValue::eString)
        || (type == CMQueryNodeValue::eObjects)
        || (type == CMQueryNodeValue::eRef);
    if (!first_ok)
        return false;

    for (size_t index = 1; index < m_Args.size(); ++index) {
        type = (index != 3) ? CMQueryNodeValue::eString : CMQueryNodeValue::eBool;
        if (m_Args[index]->GetDataType() != type)
            return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// CMacroFunction_SetRelFeatQual
/// SetRelatedFeatureQual(feat_type, field_name, new_value, existing_text_option, delimiter, update_mrna, remove_blank)
/// The last three parameters are optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetRelFeatQual,"SetRelatedFeatureQual");
void CMacroFunction_SetRelFeatQual::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* src_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!src_feat || !scope)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    const string& field_name = m_Args[1]->GetString();
    CSeqFeatData::ESubtype feat_type = NMacroUtil::GetFeatSubtype(m_Args[0]->GetString());
    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(*src_feat, feat_type, scope);
    
    size_t index = 2;
    string newValue = NMacroUtil::GetStringValue(m_Args[index]);
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    bool update_mrna = false;
    size_t arg_nr = m_Args.size();

    if (++index < arg_nr) {
        if (m_Args[index]->IsString()) {
            delimiter = m_Args[index]->GetString();
            if (++index < arg_nr) {
                remove_field = m_Args[index]->GetBool();
                if (++index < arg_nr)
                    update_mrna = m_Args[index]->GetBool();
            }
        }
        else if (m_Args[index]->IsBool()) {
            remove_field = m_Args[index]->GetBool();
            if (++index < arg_nr)
                update_mrna = m_Args[index]->GetBool();
        }
    }
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);

    if (!newValue.empty()) {
        CMQueryNodeValue::TObs objs;
        bool changed = false, created = false;
        CRef<CSeq_feat> dest_feat;
        CSeq_feat_Handle fh;
        if (!feat_list.empty() && feat_list.size() == 1) {
            dest_feat.Reset(new CSeq_feat);
            dest_feat->Assign(*feat_list.front());
            if (!SetFeatDestinationField(dest_feat, field_name, objs)) {
                return;
            }
            fh = scope->GetSeq_featHandle(*feat_list.front());
            changed = true;
        }
        else {
            // new feature
            dest_feat = CreateNewRelatedFeature(*src_feat, field_name, *scope);
            if (!dest_feat) return;
            if (!SetFeatDestinationField(dest_feat, field_name, objs)) {
                return;
            }
            created = true;
            if (src_feat->GetData().IsCdregion() && src_feat->IsSetXref() && dest_feat->GetData().IsProt()) {
                CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
                NON_CONST_ITERATE(CSeq_feat::TXref, it, edit_feat->SetXref()) {
                    if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                        dest_feat->SetData().SetProt().Assign((*it)->GetData().GetProt());
                        edit_feat->SetXref().erase(it);
                        m_QualsChangedCount++;
                        break;
                    }
                }
                if (edit_feat->GetXref().empty()) {
                    edit_feat->ResetXref();
                }
            }
        }

        vector<string> new_values;
        x_SetFields(objs, newValue, existing_text, new_values);

        if (m_QualsChangedCount) {
            m_DataIter->SetModified();
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr();
            for (size_t i = 0; i < new_values.size(); ++i) {
                log << ": set '" << new_values[i] << "' as a new value";
            }

            if (changed && fh) {
                CRef<CCmdComposite> cmd(new CCmdComposite("Change Related feature"));
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *dest_feat)));
                if (update_mrna && dest_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                    string prot_product = dest_feat->GetData().GetProt().GetName().front();
                    CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(prot_product, feat_list.front(), *scope);
                    if (upd_cmd) {
                        cmd->AddCommand(*upd_cmd);
                        log << ", applied " + prot_product + " to mRNA product name ";
                    }
                }
                m_DataIter->RunCommand(cmd, m_CmdComposite);
            }
            else if (created) {
                CRef<CCmdComposite> cmd(new CCmdComposite("Create feature"));
                CBioseq_Handle bsh = scope->GetBioseqHandle(dest_feat->GetLocation());
                cmd->AddCommand(*CRef<CCmdCreateFeatBioseq>(new CCmdCreateFeatBioseq(bsh, *dest_feat)));
                if (update_mrna && dest_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                    string message;
                    CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(*dest_feat, *scope, message);
                    if (upd_cmd) {
                        cmd->AddCommand(*upd_cmd);
                        log << ", " << message;
                    }
                }
                m_DataIter->RunCommand(cmd, m_CmdComposite);
            }

            x_LogFunction(log);
        }
    }
    else if (remove_field) {
        CMQueryNodeValue::TObs objs;
        CRef<CSeq_feat> dest_feat;
        CSeq_feat_Handle fh;
        if (!feat_list.empty() && feat_list.size() == 1) {
            dest_feat.Reset(new CSeq_feat);
            dest_feat->Assign(*feat_list.front());
            if (!GetFeatDestinationField(dest_feat, field_name, objs)) {
                return;
            }
            fh = scope->GetSeq_featHandle(*feat_list.front());
        }

        for (auto& it : objs) {
            if (RemoveFieldByName(it)) {
                m_QualsChangedCount++;
            }
        }

        if (m_QualsChangedCount) {
            CNcbiOstrstream log;
            if (fh) {
                CCleanup cleanup(scope);
                cleanup.BasicCleanup(*dest_feat);

                CRef<CCmdComposite> cmd(new CCmdComposite("Change Related feature"));
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *dest_feat)));
                log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " qualifiers";

                if (update_mrna && dest_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                    string message;
                    CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(*dest_feat, *scope, message);
                    if (upd_cmd) {
                        cmd->AddCommand(*upd_cmd);
                        log << ", " << message;
                    }
                }
                m_DataIter->RunCommand(cmd, m_CmdComposite);
            }
            x_LogFunction(log);
        }
    }
}

bool CMacroFunction_SetRelFeatQual::x_ValidArguments() const
{
    // can accept as its first parameter: objects, string or reference
    size_t arg_nr = m_Args.size();
    if (arg_nr < 4 && arg_nr > 7) {
        return false;
    }

    NMacroUtil::GetPrimitiveFromRef(m_Args[2].GetNCObject());
    size_t index = 0;
    for (; index < 4; ++index) {
        if (!m_Args[index]->IsString())
            return false;
    }

    if (index < arg_nr && (!m_Args[index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (++index < arg_nr && !m_Args[index]->IsBool()) return false;
    if (++index < arg_nr && !m_Args[index]->IsBool()) return false;
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// CMacroFunction_EditRelFeatQual
/// EditRelatedFeatureQual(feat_type, field_name, find_text, repl_text, location, case_sensitive, is_regex)
///
const char* CMacroFunction_EditRelFeatQual::sm_FunctionName = "EditRelatedFeatureQual";
void CMacroFunction_EditRelFeatQual::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* src_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!src_feat || !scope)
        return;

    size_t index = 2;
    const string& find_txt = NMacroUtil::GetStringValue(m_Args[index]);
    string repl_txt = NMacroUtil::GetStringValue(m_Args[++index]);
    const string& location = m_Args[++index]->GetString();
    bool case_sensitive = m_Args[++index]->GetBool();
    bool is_regex = (++index < m_Args.size()) ? m_Args[index]->GetBool() : false;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    const string& field_name = m_Args[1]->GetString();
    CSeqFeatData::ESubtype feat_type = NMacroUtil::GetFeatSubtype(m_Args[0]->GetString());
    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(*src_feat, feat_type, scope);

    CMQueryNodeValue::TObs objs;
    CRef<CSeq_feat> dest_feat;
    CSeq_feat_Handle fh;
    if (!feat_list.empty() && feat_list.size() == 1) {
        dest_feat.Reset(new CSeq_feat);
        dest_feat->Assign(*feat_list.front());
        if (!GetFeatDestinationField(dest_feat, field_name, objs)) {
            return;
        }
        fh = scope->GetSeq_featHandle(*feat_list.front());
    }

    x_EditFields(objs, find_txt, repl_txt, s_GetLocFromName(location), case_sensitive, is_regex);
    if (m_QualsChangedCount) {
        if (fh) {
            CRef<CCmdComposite> cmd(new CCmdComposite("Change Related feature"));
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *dest_feat)));
            m_DataIter->RunCommand(cmd, m_CmdComposite);
        }

        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": edited " << m_QualsChangedCount << " qualifiers, replaced " << find_txt;
        if (NStr::IsBlank(repl_txt)) {
            repl_txt.assign("''");
        }
        log << " with " << repl_txt;
        x_LogFunction(log);
    }
}

bool CMacroFunction_EditRelFeatQual::x_ValidArguments() const
{
    if (m_Args.size() != 6 && m_Args.size() != 7)
        return false;

    for (size_t index = 2; index < 4; index++) {
        if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString && m_Args[index]->GetDataType() != CMQueryNodeValue::eInt) {
            return false;
        }
    }

    if (m_Args.size() == 7) {
        if (m_Args.back()->GetDataType() != CMQueryNodeValue::eBool)
            return false;
    }
    return (m_Args[0]->GetDataType() == CMQueryNodeValue::eString
            && m_Args[1]->GetDataType() == CMQueryNodeValue::eString
            && m_Args[5]->GetDataType() == CMQueryNodeValue::eBool);
}


///////////////////////////////////////////////////////////////////////////////
// CMacroFunction_RmvRelFeatQual
/// RemoveRelatedFeatureQual(feat_type, field_name)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RmvRelFeatQual, "RemoveRelatedFeatureQual")
void CMacroFunction_RmvRelFeatQual::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* src_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!src_feat || !scope)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    const string& field_name = m_Args[1]->GetString();
    CSeqFeatData::ESubtype feat_type = NMacroUtil::GetFeatSubtype(m_Args[0]->GetString());
    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(*src_feat, feat_type, scope);

    CMQueryNodeValue::TObs objs;
    CRef<CSeq_feat> dest_feat;
    CSeq_feat_Handle fh;
    if (!feat_list.empty() && feat_list.size() == 1) {
        dest_feat.Reset(new CSeq_feat);
        dest_feat->Assign(*feat_list.front());
        if (!GetFeatDestinationField(dest_feat, field_name, objs)) {
            return;
        }
        fh = scope->GetSeq_featHandle(*feat_list.front());
    }

    for (auto& it : objs) {
        if (RemoveFieldByName(it)) {
            m_QualsChangedCount++;
        }
    }

    if (m_QualsChangedCount) {
        if (fh) {
            CCleanup cleanup(scope);
            cleanup.BasicCleanup(*dest_feat);

            CRef<CCmdComposite> cmd(new CCmdComposite("Change Related feature"));
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *dest_feat)));
            m_DataIter->RunCommand(cmd, m_CmdComposite);
        }
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " qualifiers";
        x_LogFunction(log);
    }
}

bool CMacroFunction_RmvRelFeatQual::x_ValidArguments() const
{
    return (m_Args.size() == 2 &&
            m_Args[0]->GetDataType() == CMQueryNodeValue::eString &&
            m_Args[1]->GetDataType() == CMQueryNodeValue::eString);
}


///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_RmvOutsideRelFeatQual
/// RmvOutsideRelatedFeatureQual(feat_type, field_name, before_match(b), left_del(str), rmv_left(b), 
///                         after_match(b), right_del(str), rmv_right(b), case_insensitive(b), whole_word(b), update_mrna(b))

DEFINE_MACRO_FUNCNAME(CMacroFunction_RmvOutsideRelFeatQual, "RmvOutsideRelatedFeatureQual")

void CMacroFunction_RmvOutsideRelFeatQual::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* src_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!src_feat || !scope)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    const string& field_name = m_Args[1]->GetString();
    CSeqFeatData::ESubtype feat_type = NMacroUtil::GetFeatSubtype(m_Args[0]->GetString());
    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(*src_feat, feat_type, scope);


    CMQueryNodeValue::TObs objs;
    CRef<CSeq_feat> dest_feat;
    CSeq_feat_Handle fh;
    if (!feat_list.empty() && feat_list.size() == 1) {
        dest_feat.Reset(new CSeq_feat);
        dest_feat->Assign(*feat_list.front());
        if (!GetFeatDestinationField(dest_feat, field_name, objs)) {
            return;
        }
        fh = scope->GetSeq_featHandle(*feat_list.front());
    }

    CRef<CRemoveTextOptions> options = x_GetRemoveTextOptions(2);
    x_RmvOutsideFields(objs, *options);
    if (m_QualsChangedCount) {
        if (fh) {
            CRef<CCmdComposite> cmd(new CCmdComposite("Change Related feature"));
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *dest_feat)));
            m_DataIter->RunCommand(cmd, m_CmdComposite);
        }

        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": removed text outside string in " << m_QualsChangedCount << " qualifiers";

        bool update_mrna = (m_Args.size() == 11) ? m_Args[10]->GetBool() : false;
        if (update_mrna && dest_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
            string message;
            CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
            string prot_product = dest_feat->GetData().GetProt().GetName().front();
            CRef<CCmdComposite> cmd = UpdatemRNAProduct(prot_product, feat_list.front(), *scope);
            if (cmd) {
                m_DataIter->RunCommand(cmd, m_CmdComposite);
                log << ", applied " + prot_product + " to mRNA product name ";
            }
        }

        x_LogFunction(log);
    }
}

bool CMacroFunction_RmvOutsideRelFeatQual::x_ValidArguments() const
{
    if (m_Args.size() != 10 && m_Args.size() != 11)
        return false;

    if (m_Args[0]->GetDataType() != CMQueryNodeValue::eString || m_Args[1]->GetDataType() != CMQueryNodeValue::eString)
        return false;

    return x_CheckArguments(2);
}


// IOperateOnTwoQuals interface
void IOperateOnTwoQuals::ChangeFields(CMQueryNodeValue::TObs& src_objs, CMQueryNodeValue::TObs& dest_objs)
{
    _ASSERT(!src_objs.empty() && !dest_objs.empty());
    CMQueryNodeValue::TObs::iterator src_it = src_objs.begin();
    CMQueryNodeValue::TObs::iterator dest_it = dest_objs.begin();

    while (src_it != src_objs.end()) {
        CMQueryNodeValue::TObs src_prim_objs;
        NMacroUtil::GetPrimitiveObjectInfos(src_prim_objs, *src_it);
        CMQueryNodeValue::TObs dest_prim_objs;
        NMacroUtil::GetPrimitiveObjectInfos(dest_prim_objs, *dest_it);
        if (dest_prim_objs.empty() && dest_it->field.GetTypeFamily() == eTypeFamilyContainer) {
            // add new element when the container is empty
            CObjectInfo new_oi(dest_it->field.AddNewElement());
            dest_prim_objs.push_back(CMQueryNodeValue::SResolvedField(dest_it->field, new_oi));
        }
        for (auto& it : src_prim_objs) {
            x_ChangeFields(it.field, dest_prim_objs.front().field);
        }

        ++src_it;
        ++dest_it;
        if (dest_it == dest_objs.end()) {
            --dest_it;
        }
    }
}

bool IOperateOnTwoQuals::x_GetSourceFields(CObjectInfo oi, size_t index, CMQueryNodeValue::TObs& result)
{
    if (index >= m_Args.size())
        return false;

    // the source field has to be set
    CMQueryNodeValue::EType src_type = m_Args[index]->GetDataType();

    if (src_type == CMQueryNodeValue::eString) {
        if (!GetFieldsByName(&result, oi, m_Args[index]->GetString()))
            return false;
    }
    else if (src_type == CMQueryNodeValue::eObjects) {
        result = m_Args[index]->GetObjects();
    }
    else if (src_type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(result, index);
        NMacroUtil::SwapGbQualWithValues(result);
    }

    return (!result.empty());
}

bool IOperateOnTwoQuals::x_GetDestFields(CObjectInfo oi, size_t index, CMQueryNodeValue::TObs& result)
{
    // the destination field might be set during this action
    CMQueryNodeValue::EType dest_type = m_Args[index]->GetDataType();

    if (dest_type == CMQueryNodeValue::eString) {
        const string& dest_field = m_Args[index]->GetString();
        CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
        GetDestinationObjects(object, oi, dest_field, result);
    }
    else if (dest_type == CMQueryNodeValue::eObjects) {
        result = m_Args[index]->GetObjects();
    }
    else if (dest_type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(result, index);
    }
    return !result.empty();
}

string IOperateOnTwoQuals::x_GetSourceString(CObjectInfo& src)
{
    string src_val;
    if (src.GetTypeFamily() == eTypeFamilyPrimitive) {
        if (src.GetPrimitiveValueType() == ePrimitiveValueString) {
            src_val = src.GetPrimitiveValueString();
            // strip_name is not implemented yet
        }
        else if (src.GetPrimitiveValueType() == ePrimitiveValueEnum) { // codon-start
            src_val = NStr::IntToString(src.GetPrimitiveValueInt());
        }
    }
    else if (src.GetTypeFamily() == eTypeFamilyClass) {
        if (src.GetName() == "Dbtag") {
            CDbtag* tag = CTypeConverter<CDbtag>::SafeCast(src.GetObjectPtr());
            if (tag) {
                tag->GetLabel(&src_val);
            }
        }
    }
    return src_val;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ConvertStringQual
/// ConvertStringQual(src_field, dst_field, capitalization, strip_name, existing_text_opt, delimiter)
/// last parameter is optional
/// The function will NOT delete the src_field.
/// If the user wants to delete the src_field, (s)he should use RemoveQual() or RemoveModifier() functions
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ConvertStringQual, "ConvertStringQual")

void CMacroFunction_ConvertStringQual::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs src_objs;
    size_t index = 0;
    if (!x_GetSourceFields(oi, index, src_objs))
        return;

    CMQueryNodeValue::TObs dest_objs;
    if (!x_GetDestFields(oi, ++index, dest_objs))
        return;

    const string& capitalization = m_Args[++index]->GetString();
    bool strip_name = m_Args[++index]->GetBool();
    strip_name = false; // for now we only use it with default value
    const string& action_type = m_Args[++index]->GetString();
    string delimiter = (++index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;

    m_ExistingText = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);
    m_CapChange = NMacroUtil::ConvertStringtoCapitalOption(capitalization);

    ChangeFields(src_objs, dest_objs);

    if (m_QualsChangedCount) {
        NMacroUtil::CleanupForTaxnameChange(dest_objs.front(), oi);
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": converted " << m_QualsChangedCount << " qualifiers";
        x_LogFunction(log);
    }
}

bool CMacroFunction_ConvertStringQual::x_ChangeFields(CObjectInfo& src, CObjectInfo& dest)
{
    if (dest.GetTypeFamily() != eTypeFamilyPrimitive) {
        return false;
    }

    string src_val = x_GetSourceString(src);
    if (dest.GetPrimitiveValueType() == ePrimitiveValueString) {
        string dest_val = dest.GetPrimitiveValueString();
        if (edit::AddValueToString(dest_val, src_val, m_ExistingText)) {
            return SetQualStringValue(dest, dest_val);
        }
    }
    return false;
}

string CMacroFunction_ConvertStringQual::x_GetSourceString(CObjectInfo& src)
{
    string src_val = IOperateOnTwoQuals::x_GetSourceString(src);
    CSeq_entry_Handle seh = m_DataIter->GetSEH();
    FixCapitalizationInString(seh, src_val, m_CapChange);
    return src_val;
}


bool CMacroFunction_ConvertStringQual::x_ValidArguments() const
{
    size_t as = m_Args.size();
    if (!(as == 5 || as == 6)) {
        return false;
    }

    for (size_t i = 0; i < 2; ++i) {
        CMQueryNodeValue::EType type = m_Args[i]->GetDataType();
        bool type_ok = (type == CMQueryNodeValue::eString)
            || (type == CMQueryNodeValue::eObjects)
            || (type == CMQueryNodeValue::eRef);
        if (!type_ok)
            return false;
    }

    bool m_Args_ok = (m_Args[2]->GetDataType() == CMQueryNodeValue::eString);
    m_Args_ok = m_Args_ok && (m_Args[3]->GetDataType() == CMQueryNodeValue::eBool)
        && (m_Args[4]->GetDataType() == CMQueryNodeValue::eString);

    if (as == 6) {
        m_Args_ok = m_Args_ok && (m_Args[5]->GetDataType() == CMQueryNodeValue::eString);
    }

    if (!m_Args_ok) {
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SwapQual
/// SwapQual(src_fieldname, dest_fieldname)
/// Swaps the content of two fields. If one field is blank/does not exist, the other field will be removed
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SwapQual, "SwapQual")

void CMacroFunction_SwapQual::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs src_objs;
    CMQueryNodeValue::TObs dest_objs;
    size_t src_index = 0, dest_index = 1;

    if ((m_Args.size() == 3 && m_Args[0]->GetDataType() != CMQueryNodeValue::eString)
        || m_Args.size() == 4) {
        dest_index = 2;
    }

    // check that at least one of the fields exist
    bool src_set = x_DoFieldsExist(oi, src_objs, src_index);
    bool dest_set = x_DoFieldsExist(oi, dest_objs, dest_index);

    if (!src_set && !dest_set)
        return;

    // special attention to gene qualifiers that are removed from gene Xrefs
    bool is_gene_suppressed_before = false;
    bool is_gene_suppressed_after = false;
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    if (dynamic_cast<const CSeq_feat*>(obj.GetPointer())) {
        CSeq_feat* feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
        auto gene_xref = feat->GetGeneXref();
        is_gene_suppressed_before = gene_xref && gene_xref->IsSuppressed();
    }

    if (!src_set) {
        x_SetFields(oi, src_objs, src_index);
    } 
    if (!dest_set) {
        x_SetFields(oi, dest_objs, dest_index);
    }

    if (src_set && dest_set) {
        CMQueryNodeValue::TObs::iterator src_it = src_objs.begin();
        CMQueryNodeValue::TObs::iterator dest_it = dest_objs.begin();

        while (src_it != src_objs.end() && dest_it != dest_objs.end()) {
            CMQueryNodeValue::TObs src_prim_objs;
            NMacroUtil::GetPrimitiveObjInfosWithContainers(src_prim_objs, *src_it);
            CMQueryNodeValue::TObs dest_prim_objs;
            NMacroUtil::GetPrimitiveObjInfosWithContainers(dest_prim_objs, *dest_it);
            if (dest_prim_objs.empty() && dest_it->field.GetTypeFamily() == eTypeFamilyContainer) {
                // add new element when the container is empty
                CObjectInfo new_oi(dest_it->field.AddNewElement());
                dest_prim_objs.push_back(CMQueryNodeValue::SResolvedField(dest_it->field, new_oi));
            }
            x_SwapFields(src_prim_objs.front().field, dest_prim_objs.front().field);

            ++src_it;
            ++dest_it;
        }
        NMacroUtil::CleanupForTaxnameChange(src_objs.front(), oi);
        NMacroUtil::CleanupForTaxnameChange(dest_objs.front(), oi);
    }
    else if (!src_set) {
        // copy destination field into the source field
        x_CopyFields(dest_objs, src_objs);

        bool is_taxname = NMacroUtil::IsTaxname(dest_objs.front());
        // delete destination field
        NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, dest_objs) {
            RemoveFieldByName(*it);
        }
        if (is_taxname) {
            NMacroUtil::CleanupForTaxnameChange(oi);
        }
    }
    else if (!dest_set) {
        // copy source field into the destination field
        x_CopyFields(src_objs, dest_objs);

        bool is_taxname = NMacroUtil::IsTaxname(src_objs.front());
        // delete source field
        NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, src_objs) {
            RemoveFieldByName(*it);
        }
        if (is_taxname) {
            NMacroUtil::CleanupForTaxnameChange(oi);
        }
    }

    CCleanup cleanup;
    if (dynamic_cast<const CSeq_feat*>(obj.GetPointer())) {
        CSeq_feat* seq_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
        cleanup.BasicCleanup(*seq_feat);
        if (seq_feat->IsSetXref()) {
            auto gene_xref = seq_feat->GetGeneXref();
            is_gene_suppressed_after = gene_xref->IsSuppressed();
            if (!is_gene_suppressed_before && is_gene_suppressed_after)
                NMacroUtil::RemoveGeneXref(*seq_feat);
        }
    }
    else if (dynamic_cast<const CBioSource*>(obj.GetPointer())) {
        CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
        cleanup.BasicCleanup(*bsrc);
        if (bsrc->IsSetOrg() && bsrc->GetOrg().IsSetDb() && bsrc->GetOrg().GetDb().empty()) {
            bsrc->SetOrg().ResetDb();
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": swapped " << m_QualsChangedCount << " qualifiers";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SwapQual::x_ValidArguments() const
{
    if (m_Args.size() < 2) {
        return false;
    }

    for (auto& it : m_Args) {
        CMQueryNodeValue::EType type = it->GetDataType();
        if (type != CMQueryNodeValue::eString && type != CMQueryNodeValue::eObjects && type != CMQueryNodeValue::eRef) {
            return false;
        }
    }
    return true;
}


bool CMacroFunction_SwapQual::x_DoFieldsExist(CObjectInfo oi, CMQueryNodeValue::TObs& result, size_t index)
{
    if (index >= m_Args.size())
        return false;

    CMQueryNodeValue::EType type = m_Args[index]->GetDataType();
    if (type == CMQueryNodeValue::eString) {
        GetFieldsByName(&result, oi, m_Args[index]->GetString());
    }
    else if (type == CMQueryNodeValue::eObjects) {
        result = m_Args[index]->GetObjects();
    }
    else if (type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(result, index);
    }

    return (!result.empty());
}

bool CMacroFunction_SwapQual::x_SetFields(CObjectInfo oi, CMQueryNodeValue::TObs& result, size_t index)
{
    if (index >= m_Args.size())
        return false;

    // the field does not have to be set
    CMQueryNodeValue::EType type = m_Args[index]->GetDataType();
    if (type == CMQueryNodeValue::eString) {
        SetFieldsByName(&result, oi, m_Args[index]->GetString());
    }
    else {
        CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
        x_SetField(obj, m_Args[++index]->GetString(), result);
    }
    return (!result.empty());
}

static void s_SetBioSourceField(CRef<CBioSource> bsrc, CObjectInfo& oi, const string& field_name, CMQueryNodeValue::TObs& dest_objs)
{
    if (field_name.empty())
        return;

    // check if the field denotes an orgmod or a subsource modifier
    if (NMacroUtil::IsBiosourceModifier(field_name)) {
        // check if it's a subsource modifier
        if (CSubSource::IsValidSubtypeName(field_name, CSubSource::eVocabulary_insdc)) {
            CSubSource::TSubtype st = CSubSource::GetSubtypeValue(field_name, CSubSource::eVocabulary_insdc);

            CRef<CSubSource> sub_src(new CSubSource());
            sub_src->SetSubtype(st);
            sub_src->SetName(kEmptyStr);
            bsrc->SetSubtype().push_back(sub_src);
            CObjectInfo subsrc_oi(sub_src.GetPointer(), sub_src->GetTypeInfo());
            dest_objs.push_back(CMQueryNodeValue::SResolvedField(oi, subsrc_oi));
            // check if it's an orgmod modifier
        }
        else if (COrgMod::IsValidSubtypeName(field_name, COrgMod::eVocabulary_insdc)) {
            COrgMod::TSubtype st = COrgMod::GetSubtypeValue(field_name, COrgMod::eVocabulary_insdc);

            CRef<COrgMod> orgmod(new COrgMod());
            orgmod->SetSubtype(st);
            orgmod->SetSubname(kEmptyStr);
            CRef<COrgName> orgname;
            if (!bsrc->IsSetOrgname()) {
                orgname.Reset(new COrgName());
                orgname->SetMod().push_back(orgmod);
                bsrc->SetOrg().SetOrgname(*orgname);
            }
            else {
                orgname = Ref(&(bsrc->SetOrg().SetOrgname()));
                orgname->SetMod().push_back(orgmod);
            }
            CObjectInfo parent_mod_oi(orgname.GetPointer(), orgname->GetThisTypeInfo());
            CObjectInfo orgmod_oi(orgmod.GetPointer(), orgmod->GetThisTypeInfo());
            dest_objs.push_back(CMQueryNodeValue::SResolvedField(parent_mod_oi, orgmod_oi));
        }
    }
    else {
        SetFieldsByName(&dest_objs, oi, field_name);
    }
}

static void s_SetFeatField(CRef<CSeq_feat> feat, CObjectInfo& oi,const string& field_name, CMQueryNodeValue::TObs& result)
{
    if (field_name.empty())
        return;

    if (SetFieldsByName(&result, oi, field_name)) {
        return;
    }

    CRef<CGb_qual> new_gbqual(new CGb_qual(field_name, kEmptyStr));
    feat->SetQual().push_back(new_gbqual);
    CObjectInfo gbqual_oi(new_gbqual.GetPointer(), new_gbqual->GetTypeInfo());
    result.push_back(CMQueryNodeValue::SResolvedField(oi, gbqual_oi));
}

void CMacroFunction_SwapQual::x_SetField(CConstRef<CObject>& obj, const string& field_name, CMQueryNodeValue::TObs& result)
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    const CBioSource* cbsrc = dynamic_cast<const CBioSource*>(obj.GetPointer());
    const CSeq_feat* cfeat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    if (cbsrc) {
        CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
        return s_SetBioSourceField(Ref(bsrc), oi, field_name, result);
    }
    if (cfeat) {
        CSeq_feat* feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
        return s_SetFeatField(Ref(feat), oi, field_name, result);
    }
}

static string s_GetEnumValue(CObjectInfo& obj)
{
    string value;
    if (obj.GetTypeFamily() == eTypeFamilyPrimitive && obj.GetPrimitiveValueType() == ePrimitiveValueEnum) {
        try {
            value = obj.GetPrimitiveValueString();
        }
        catch (const CException&) {
            value = NStr::NumericToString(obj.GetPrimitiveValueInt4(), NStr::fConvErr_NoThrow);
        }
    }
    return value;
}

static bool s_SetEnumValue(CObjectInfo& obj, const string& str)
{
    bool set = false;
    if (obj.GetTypeFamily() == eTypeFamilyPrimitive && obj.GetPrimitiveValueType() == ePrimitiveValueEnum) {
        try {
            obj.SetPrimitiveValueString(str);
            set = true;
        }
        catch (const CException&) {
            // if this fails, try to convert the string to int and assign again
            try {
                obj.SetPrimitiveValueInt4(NStr::StringToLong(str));
                set = true;
            }
            catch (const CException&) {}
        }
    }
    return set;
}


bool CMacroFunction_SwapQual::x_SwapFields(CObjectInfo& src, CObjectInfo& dest)
{
    if (src.GetTypeFamily() != dest.GetTypeFamily() || src.GetTypeFamily() != eTypeFamilyPrimitive) {
        return false;
    }

    bool swapped = false;
    if (src.GetPrimitiveValueType() == ePrimitiveValueString) {
        string tmp = src.GetPrimitiveValueString();
        if (dest.GetPrimitiveValueType() == ePrimitiveValueString) {
            src.SetPrimitiveValueString(dest.GetPrimitiveValueString());
            dest.SetPrimitiveValueString(tmp);
            swapped = true;
        }
        else if (dest.GetPrimitiveValueType() == ePrimitiveValueEnum) {
            string dest_value = s_GetEnumValue(dest);
            src.SetPrimitiveValueString(dest_value);
            swapped = s_SetEnumValue(dest, tmp);
        }
    }
    else if (src.GetPrimitiveValueType() == ePrimitiveValueEnum) {
        string tmp = s_GetEnumValue(src);
        if (dest.GetPrimitiveValueType() == ePrimitiveValueString) {
            swapped = s_SetEnumValue(src, dest.GetPrimitiveValueString());
            dest.SetPrimitiveValueString(tmp);
        }
        else if (dest.GetPrimitiveValueType() == ePrimitiveValueEnum) {
            string dest_value = s_GetEnumValue(dest);
            swapped = s_SetEnumValue(src, dest_value);
            swapped = swapped && s_SetEnumValue(dest, tmp);
        }
    }

    if (swapped)
        m_QualsChangedCount++;
    return swapped;
}

bool CMacroFunction_SwapQual::s_SwapFields(CObjectInfo& src, CObjectInfo& dest)
{
    // swaps primitive values
    // both src and dest should be of primitive type
    if (src.GetTypeFamily() != dest.GetTypeFamily() || src.GetTypeFamily() != eTypeFamilyPrimitive) {
        return false;
    }

    switch (src.GetPrimitiveValueType()) {
    case ePrimitiveValueBool: {
        bool tmp = dest.GetPrimitiveValueBool();
        dest.SetPrimitiveValueBool(src.GetPrimitiveValueBool());
        src.SetPrimitiveValueBool(tmp);
        return true;
    }
    case ePrimitiveValueInteger:
    case ePrimitiveValueChar: {
        int tmp = dest.GetPrimitiveValueInt();
        dest.SetPrimitiveValueInt(src.GetPrimitiveValueInt());
        src.SetPrimitiveValueInt(tmp);
        return true;
    }
    case ePrimitiveValueReal: {
        double tmp = dest.GetPrimitiveValueDouble();
        dest.SetPrimitiveValueDouble(src.GetPrimitiveValueDouble());
        src.SetPrimitiveValueDouble(tmp);
        return true;
    }
    case ePrimitiveValueString: {
        string tmp = dest.GetPrimitiveValueString();
        dest.SetPrimitiveValueString(src.GetPrimitiveValueString());
        src.SetPrimitiveValueString(tmp);
        return true;
    }
    case ePrimitiveValueEnum: // to be done
        return false;
    default:
        ;
    }
    return false;
}

void CMacroFunction_SwapQual::x_CopyFields(CMQueryNodeValue::TObs& src_objs, CMQueryNodeValue::TObs& dest_objs)
{
    // copy contents of the destination field into the source field
    CMQueryNodeValue::TObs::iterator src_it = src_objs.begin();
    CMQueryNodeValue::TObs::iterator dest_it = dest_objs.begin();

    while (src_it != src_objs.end() && dest_it != dest_objs.end()) {
        CMQueryNodeValue::TObs src_prim_objs;
        NMacroUtil::GetPrimitiveObjInfosWithContainers(src_prim_objs, *src_it);
        CMQueryNodeValue::TObs dest_prim_objs;
        NMacroUtil::GetPrimitiveObjInfosWithContainers(dest_prim_objs, *dest_it);

        if (dest_prim_objs.empty() && dest_it->field.GetTypeFamily() == eTypeFamilyContainer) {
            // add new element when the container is empty
            CObjectInfo new_oi(dest_it->field.AddNewElement());
            dest_prim_objs.push_back(CMQueryNodeValue::SResolvedField(dest_it->field, new_oi));
        }
        x_CopyFields(src_prim_objs.front().field, dest_prim_objs.front().field);

        ++src_it;
        ++dest_it;
    }
}

bool CMacroFunction_SwapQual::x_CopyFields(CObjectInfo& src, CObjectInfo& dest)
{
    if (src.GetTypeFamily() != dest.GetTypeFamily() || src.GetTypeFamily() != eTypeFamilyPrimitive) {
        return false;
    }

    string src_val;
    if (src.GetPrimitiveValueType() == ePrimitiveValueString) {
        src_val = src.GetPrimitiveValueString();
    }
    else if (src.GetPrimitiveValueType() == ePrimitiveValueEnum) { // codon-start
        src_val = NStr::IntToString(src.GetPrimitiveValueInt());
    }

    string dest_val = dest.GetPrimitiveValueString();
    if (edit::AddValueToString(dest_val, src_val, edit::eExistingText_replace_old)) {
        return SetQualStringValue(dest, dest_val);
    }
    return false;
}


/// class CMacroFunction_SwapRelFeatQual
/// SwapFeatureQual([src_obj,] src_field, dest_feat_subtype, dest_field, update_mrna);
/// 
DEFINE_MACRO_FUNCNAME(CMacroFunction_SwapRelFeatQual, "SwapFeatureQual")

void CMacroFunction_SwapRelFeatQual::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* src_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!src_feat || !scope)
        return;

    size_t src_index = 0, dest_index = 1;
    if (m_Args.size() == 5) {
        dest_index = 2;
    }
    if (m_Args[dest_index]->GetString().empty() || m_Args[dest_index + 1]->GetString().empty())
        return;

    const string& dest_feattype = m_Args[dest_index]->GetString();
    const string& dest_field = m_Args[dest_index + 1]->GetString();
    const string src_field = (m_Args[src_index]->GetDataType() == CMQueryNodeValue::eString) ?
        m_Args[src_index]->GetString() : kEmptyStr;

    // reset members
    m_ConstDestFeat.Reset(nullptr);
    m_EditDestFeat.Reset(nullptr);
    m_CreatedFeat.Reset(nullptr);

    // check that at least one of the fields exist
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs src_objs;
    CMQueryNodeValue::TObs dest_objs;
    CSeq_feat* seq_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());

    bool src_set = x_DoFieldsExist(oi, src_objs, src_index);
    if (IstRNAProductField(*src_feat, src_field)) {
        src_set = !src_feat->GetData().GetRna().GetRnaProductName().empty();
    }
    bool dest_set = x_DoDestFeatFieldsExist(*src_feat, dest_objs, dest_feattype, dest_field);

    if (!src_set && !dest_set)
        return;

    if (!src_set) {
        x_SetFields(oi, src_objs, src_index);
    }
    if (!dest_set) {
        x_SetOrCreateDestFeatFields(*src_feat, dest_objs, dest_index + 1);
    }

    if (src_set && dest_set) {
        CMQueryNodeValue::TObs::iterator src_it = src_objs.begin();
        CMQueryNodeValue::TObs::iterator dest_it = dest_objs.begin();

        while (src_it != src_objs.end() && dest_it != dest_objs.end()) {
            CMQueryNodeValue::TObs src_prim_objs;
            NMacroUtil::GetPrimitiveObjInfosWithContainers(src_prim_objs, *src_it);
            CMQueryNodeValue::TObs dest_prim_objs;
            NMacroUtil::GetPrimitiveObjInfosWithContainers(dest_prim_objs, *dest_it);
            if (dest_prim_objs.empty() && dest_it->field.GetTypeFamily() == eTypeFamilyContainer) {
                // add new element when the container is empty
                CObjectInfo new_oi(dest_it->field.AddNewElement());
                dest_prim_objs.push_back(CMQueryNodeValue::SResolvedField(dest_it->field, new_oi));
            }
            x_SwapFields(src_prim_objs.front().field, dest_prim_objs.front().field);

            ++src_it;
            ++dest_it;
        }

        if (m_EditDestFeat && IstRNAProductField(*m_EditDestFeat, dest_field)) {
            while (src_it != src_objs.end()) {
                CMQueryNodeValue::TObs src_prim_objs;
                NMacroUtil::GetPrimitiveObjInfosWithContainers(src_prim_objs, *src_it);

                string dest_str = m_EditDestFeat->GetData().GetRna().GetRnaProductName();
                CObjectInfo src = src_prim_objs.front().field;

                if (src.GetPrimitiveValueType() == ePrimitiveValueString) {
                    string tmp = src.GetPrimitiveValueString();
                    src.SetPrimitiveValueString(dest_str);
                    string remainder;
                    m_EditDestFeat->SetData().SetRna().SetRnaProductName(tmp, remainder);
                    m_QualsChangedCount++;
                }
                ++src_it;
            }
        }
        else if (IstRNAProductField(*src_feat, src_field)) {
            while (dest_it != dest_objs.end()) {
                CMQueryNodeValue::TObs dest_prim_objs;
                NMacroUtil::GetPrimitiveObjInfosWithContainers(dest_prim_objs, *dest_it);
                if (dest_prim_objs.empty() && dest_it->field.GetTypeFamily() == eTypeFamilyContainer) {
                    // add new element when the container is empty
                    CObjectInfo new_oi(dest_it->field.AddNewElement());
                    dest_prim_objs.push_back(CMQueryNodeValue::SResolvedField(dest_it->field, new_oi));
                }

                string src_str = src_feat->GetData().GetRna().GetRnaProductName();
                CObjectInfo d = dest_prim_objs.front().field;
                if (d.GetPrimitiveValueType() == ePrimitiveValueString) {
                    string tmp = d.GetPrimitiveValueString();
                    d.SetPrimitiveValueString(src_str);
                    
                    string remainder;
                    seq_feat->SetData().SetRna().SetRnaProductName(tmp, remainder);
                    m_QualsChangedCount++;
                }
                ++dest_it;
            }
        }
    }
    else if (!src_set) {
        // copy destination field into the source field
        x_CopyFields(dest_objs, src_objs);

        if (IstRNAProductField(*seq_feat, src_field)) {
            CMQueryNodeValue::TObs::iterator dest_it = dest_objs.begin();

            while (dest_it != dest_objs.end()) {
                CMQueryNodeValue::TObs dest_prim_objs;
                NMacroUtil::GetPrimitiveObjInfosWithContainers(dest_prim_objs, *dest_it);

                CObjectInfo d = dest_prim_objs.front().field;
                string dest_val;
                if (d.GetPrimitiveValueType() == ePrimitiveValueString) {
                    dest_val = d.GetPrimitiveValueString();
                }
                else if (d.GetPrimitiveValueType() == ePrimitiveValueEnum) { // codon-start
                    dest_val = NStr::IntToString(d.GetPrimitiveValueInt());
                }

                string remainder;
                seq_feat->SetData().SetRna().SetRnaProductName(dest_val, remainder);
                m_QualsChangedCount++;
                ++dest_it;
            }
        }

        if (m_EditDestFeat && IstRNAProductField(*m_EditDestFeat, dest_field)) {
            CMQueryNodeValue::TObs::iterator src_it = src_objs.begin();

            while (src_it != src_objs.end()) {
                CMQueryNodeValue::TObs src_prim_objs;
                NMacroUtil::GetPrimitiveObjInfosWithContainers(src_prim_objs, *src_it);

                string rna_str = m_EditDestFeat->GetData().GetRna().GetRnaProductName();
                CObjectInfo src = src_prim_objs.front().field;
                src.SetPrimitiveValueString(rna_str);
                m_QualsChangedCount++;
                ++src_it;
            }
        }

        // delete destination field
        if (m_EditDestFeat && IstRNAProductField(*m_EditDestFeat, dest_field)) {
            string remainder;
            m_EditDestFeat->SetData().SetRna().SetRnaProductName(kEmptyStr, remainder);
        }
        
        NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, dest_objs) {
            if (it->parent.GetName() == "Gb-qual" && m_EditDestFeat) {
                const string& qual_name = m_Args[m_Args.size() - 2]->GetString();
                m_EditDestFeat->RemoveQualifier(qual_name);
            }
            else {
                RemoveFieldByName(*it);
            }
        }
    }
    else if (!dest_set) {
        // copy source field into the destination field
        x_CopyFields(src_objs, dest_objs);

        if (m_EditDestFeat && IstRNAProductField(*m_EditDestFeat, dest_field)) {
            CMQueryNodeValue::TObs::iterator src_it = src_objs.begin();

            while (src_it != src_objs.end()) {
                CMQueryNodeValue::TObs src_prim_objs;
                NMacroUtil::GetPrimitiveObjInfosWithContainers(src_prim_objs, *src_it);

                CObjectInfo src = src_prim_objs.front().field;
                string src_val;
                if (src.GetPrimitiveValueType() == ePrimitiveValueString) {
                    src_val = src.GetPrimitiveValueString();
                }
                else if (src.GetPrimitiveValueType() == ePrimitiveValueEnum) { // codon-start
                    src_val = NStr::IntToString(src.GetPrimitiveValueInt());
                }

                string remainder;
                m_EditDestFeat->SetData().SetRna().SetRnaProductName(src_val, remainder);
                m_QualsChangedCount++;
                ++src_it;
            }
        }

        if (IstRNAProductField(*seq_feat, src_field)) {
            CMQueryNodeValue::TObs::iterator dest_it = dest_objs.begin();

            while (dest_it != dest_objs.end()) {
                CMQueryNodeValue::TObs dest_prim_objs;
                NMacroUtil::GetPrimitiveObjInfosWithContainers(dest_prim_objs, *dest_it);

                string rna_str = seq_feat->GetData().GetRna().GetRnaProductName();
                CObjectInfo d = dest_prim_objs.front().field;
                d.SetPrimitiveValueString(rna_str);
                m_QualsChangedCount++;
                ++dest_it;
            }
        }

        // delete source field
        if (IstRNAProductField(*seq_feat, src_field)) {
            string remainder;
            seq_feat->SetData().SetRna().SetRnaProductName(kEmptyStr, remainder);
        }
        NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, src_objs) {
            RemoveFieldByName(*it);
        }
    }

    bool update_mrna = (m_Args.back()->GetDataType() == CMQueryNodeValue::eBool) ? m_Args.back()->GetBool() : false;

    CCleanup cleanup;
    cleanup.BasicCleanup(*seq_feat);
    if (m_EditDestFeat) {
        cleanup.BasicCleanup(*m_EditDestFeat);
    }
    else if (m_CreatedFeat) {
        cleanup.BasicCleanup(*m_CreatedFeat);
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": swapped " << m_QualsChangedCount << " qualifiers";

        CRef<CCmdComposite> cmd;
        if (m_ConstDestFeat && m_EditDestFeat) {
            cmd.Reset(new CCmdComposite("Change Related feature"));
            CSeq_feat_Handle fh = scope->GetSeq_featHandle(*m_ConstDestFeat);
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *m_EditDestFeat)));
            if (update_mrna && m_EditDestFeat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                string message;
                CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(*m_EditDestFeat, *scope, message);
                if (upd_cmd) {
                    cmd->AddCommand(*upd_cmd);
                    log << ", " << message;
                }
            }
        }
        else if (m_CreatedFeat) {
            cmd.Reset(new CCmdComposite("Create feature"));
            CBioseq_Handle bsh = scope->GetBioseqHandle(m_CreatedFeat->GetLocation());
            cmd->AddCommand(*CRef<CCmdCreateFeatBioseq>(new CCmdCreateFeatBioseq(bsh, *m_CreatedFeat)));
            if (update_mrna && m_CreatedFeat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                string message;
                CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(*m_CreatedFeat, *scope, message);
                if (upd_cmd) {
                    cmd->AddCommand(*upd_cmd);
                    log << ", " << message;
                }
            }
        }
        if (cmd) {
            m_DataIter->RunCommand(cmd, m_CmdComposite);
        }

        if (update_mrna && src_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
            CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
            string prot_product = edit_feat->GetData().GetProt().GetName().front();
            CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(prot_product, obj, *scope);
            if (upd_cmd) {
                m_DataIter->RunCommand(upd_cmd, m_CmdComposite);
                log << ", applied " + prot_product + " to mRNA product name ";
            }
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_SwapRelFeatQual::x_DoDestFeatFieldsExist(
    const CSeq_feat& src_feat, CMQueryNodeValue::TObs& result, const string& dest_feattype, const string& dest_field)
{
    CSeqFeatData::ESubtype target_feature = NMacroUtil::GetFeatSubtype(dest_feattype);
    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(src_feat, target_feature, Ref(&(m_DataIter->GetSEH().GetScope())));
    
    if (!feat_list.empty() && feat_list.size() == 1) {
        m_ConstDestFeat.Reset(feat_list.front());
        m_EditDestFeat.Reset(new CSeq_feat);
        m_EditDestFeat->Assign(*feat_list.front());
        GetFeatDestinationField(m_EditDestFeat, dest_field, result);

        if (IstRNAProductField(*m_EditDestFeat, dest_field)) {
            return (!m_EditDestFeat->GetData().GetRna().GetRnaProductName().empty());
            // it might return true but the output argument results is empty!
        }
    }
    return (!result.empty());
}

void CMacroFunction_SwapRelFeatQual::x_SetOrCreateDestFeatFields(const CSeq_feat& src_feat, CMQueryNodeValue::TObs& result, size_t index)
{
    if (index >= m_Args.size())
        return;
    const string& dest_field = m_Args[index]->GetString();

    if (m_ConstDestFeat && m_EditDestFeat) {
        if (!SetFeatDestinationField(m_EditDestFeat, dest_field, result)) {
            return;
        }
    }
    else {
        // create a new feature
        m_CreatedFeat = CreateNewRelatedFeature(src_feat, dest_field, m_DataIter->GetSEH().GetScope());
        if (!SetFeatDestinationField(m_CreatedFeat, dest_field, result)) {
            return;
        }

        if (src_feat.GetData().IsCdregion() && src_feat.IsSetXref() && m_CreatedFeat->GetData().IsProt()) {
            CObjectInfo oi = m_DataIter->GetEditedObject();
            CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
            NON_CONST_ITERATE(CSeq_feat::TXref, it, edit_feat->SetXref()) {
                if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                    m_CreatedFeat->SetData().SetProt().Assign((*it)->GetData().GetProt());
                    edit_feat->SetXref().erase(it);
                    break;
                }
            }
            if (edit_feat->GetXref().empty()) {
                edit_feat->ResetXref();
            }
        }
    }
}

bool CMacroFunction_SwapRelFeatQual::x_ValidArguments() const
{
    if (m_Args.size() < 4 || m_Args.size() > 5) {
        return false;
    }

    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    bool first_ok = (type == CMQueryNodeValue::eString)
        || (type == CMQueryNodeValue::eObjects)
        || (type == CMQueryNodeValue::eRef);
    if (!first_ok) {
        return false;
    }

    for (size_t index = 1; index < m_Args.size() - 1; ++index) {
        if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString) {
            return false;
        }
    }

    return (m_Args.back()->GetDataType() == CMQueryNodeValue::eBool);
}


/// class CMacroFunction_ConvertFeatQual
/// ConvertFeatureQual(src_field, dest_feature_type, dest_feature_field, capitalization, strip_name, update_mrna, existing_text_opt, delimiter)
/// The last parameter is optional
/// The function will NOT delete the src_field.
/// To delete the src_field, use RemoveQual() function
/// The related mRNA is only updated if the action is successful and the destination feature is protein
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ConvertFeatQual, "ConvertFeatureQual");
void CMacroFunction_ConvertFeatQual::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* src_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!src_feat || !scope)
        return;

    const string& dest_field = m_Args[2]->GetString();
    if (dest_field.empty())
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs src_objs;
    if (!x_GetSourceFields(oi, 0, src_objs))
        return;

    // the destination field can be set at this point
    CSeqFeatData::ESubtype target_feature = NMacroUtil::GetFeatSubtype(m_Args[1]->GetString());
    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(*src_feat, target_feature, scope);
    CMQueryNodeValue::TObs dest_objs;
    bool changed = false, created = false;
    CRef<CSeq_feat> dest_feat;
    CSeq_feat_Handle fh;
    if (!feat_list.empty() && feat_list.size() == 1) {
        dest_feat.Reset(new CSeq_feat);
        dest_feat->Assign(*feat_list.front());
        if (!SetFeatDestinationField(dest_feat, dest_field, dest_objs)) {
            return;
        }
        fh = scope->GetSeq_featHandle(*feat_list.front());
        changed = true;
    }
    else {
        // new feature
        dest_feat = CreateNewRelatedFeature(*src_feat, dest_field, *scope);
        if (!dest_feat) return;
        if (!SetFeatDestinationField(dest_feat, dest_field, dest_objs)) {
            return;
        }
        created = true;
        if (src_feat->GetData().IsCdregion() && src_feat->IsSetXref() && dest_feat->GetData().IsProt()) {
            CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
            NON_CONST_ITERATE(CSeq_feat::TXref, it, edit_feat->SetXref()) {
                if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                    dest_feat->SetData().SetProt().Assign((*it)->GetData().GetProt());
                    edit_feat->SetXref().erase(it);
                    break;
                }
            }
            if (edit_feat->GetXref().empty()) {
                edit_feat->ResetXref();
            }
        }
    }

    size_t index = 3;
    const string& capitalization = m_Args[index]->GetString();
    bool strip_name = m_Args[++index]->GetBool();
    strip_name = false; // for now we only use it with default value
    bool update_mrna = m_Args[++index]->GetBool();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter = (++index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;

    m_ExistingText = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);
    m_CapChange = NMacroUtil::ConvertStringtoCapitalOption(capitalization);

    if (IstRNAProductField(*dest_feat, dest_field)) {
        for (auto&& it : src_objs) {
            CMQueryNodeValue::TObs src_prim_objs;
            NMacroUtil::GetPrimitiveObjectInfos(src_prim_objs, it);

            CObjectInfo src = src_prim_objs.front().field;
            string src_val = x_GetSourceString(src);

            string remainder;
            dest_feat->SetData().SetRna().SetRnaProductName(src_val, remainder);
            m_QualsChangedCount++;
        }
    }
    else {
        ChangeFields(src_objs, dest_objs);
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": converted " << m_QualsChangedCount << " qualifiers";

        CRef<CCmdComposite> cmd;
        if (changed && fh) {
            cmd.Reset(new CCmdComposite("Change Related feature"));
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *dest_feat)));
        }
        else if (created) {
            cmd.Reset(new CCmdComposite("Create feature"));
            CBioseq_Handle bsh = scope->GetBioseqHandle(dest_feat->GetLocation());
            cmd->AddCommand(*CRef<CCmdCreateFeatBioseq>(new CCmdCreateFeatBioseq(bsh, *dest_feat)));
        }

        if (update_mrna) {
            if (dest_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                string message;
                CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(*dest_feat, *scope, message);
                if (upd_cmd) {
                    cmd->AddCommand(*upd_cmd);
                    log << ", " << message;
                }
            }
            else if(src_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
                string prot_product = edit_feat->GetData().GetProt().GetName().front();
                CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(prot_product, obj, *scope);
                if (upd_cmd) {
                    cmd->AddCommand(*upd_cmd);
                    log << ", applied " + prot_product + " to mRNA product name ";
                }
            }
        }
        m_DataIter->RunCommand(cmd, m_CmdComposite);
        x_LogFunction(log);
    }
}

bool CMacroFunction_ConvertFeatQual::x_ValidArguments() const
{
    if (m_Args.size() < 7 || m_Args.size() > 8)
        return false;

    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    bool first_ok = (type == CMQueryNodeValue::eString)
        || (type == CMQueryNodeValue::eObjects)
        || (type == CMQueryNodeValue::eRef);
    if (!first_ok)
        return false;

    for (size_t index = 1; index < m_Args.size(); ++index) {
        type = (index != 4 && index != 5) ? CMQueryNodeValue::eString : CMQueryNodeValue::eBool;
        if (m_Args[index]->GetDataType() != type)
            return false;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ParseStringQual - used when parsing SeqId
/// ParseStringQual(from_field, to_field, capitalization, existing_text_opt, delimiter) // when all the text should be copied
/// Last parameter is optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ParseStringQual, "ParseStringQual");
void CMacroFunction_ParseStringQual::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs src_objs;
    size_t index = 0;
    if (!x_GetSourceFields(oi, index, src_objs))
        return;

    CMQueryNodeValue::TObs dest_objs;
    if (!x_GetDestFields(oi, ++index, dest_objs))
        return;

    const string& capitalization = m_Args[++index]->GetString();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter = (++index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;

    m_ExistingText = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);
    m_CapChange = NMacroUtil::ConvertStringtoCapitalOption(capitalization);

    ChangeFields(src_objs, dest_objs);

    if (m_QualsChangedCount) {
        NMacroUtil::CleanupForTaxnameChange(dest_objs.front(), oi);
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": parsed " << m_QualsChangedCount << " fields";
        x_LogFunction(log);
    }
}

bool CMacroFunction_ParseStringQual::x_ChangeFields(CObjectInfo& src, CObjectInfo& dest)
{
    if (dest.GetTypeFamily() != eTypeFamilyPrimitive) {
        return false;
    }

    _ASSERT(dest.GetPrimitiveValueType() == ePrimitiveValueString);

    string src_val = x_GetSourceString(src);
    string dest_val = dest.GetPrimitiveValueString();
    if (edit::AddValueToString(dest_val, src_val, m_ExistingText)) {
        return SetQualStringValue(dest, dest_val);
    }
    return false;
}

string CMacroFunction_ParseStringQual::x_GetSourceString(CObjectInfo& src)
{
    string src_val = IOperateOnTwoQuals::x_GetSourceString(src);
    // it copies the whole string
    CSeq_entry_Handle seh = m_DataIter->GetSEH();
    FixCapitalizationInString(seh, src_val, m_CapChange);
    return src_val;
}

bool CMacroFunction_ParseStringQual::x_ValidArguments(void) const
{
    size_t as = m_Args.size();
    if (as < 1 || as > 5) {
        return false;
    }

    for (size_t i = 0; i < 2; ++i) {
        CMQueryNodeValue::EType type = m_Args[i]->GetDataType();
        bool type_ok = (type == CMQueryNodeValue::eString)
            || (type == CMQueryNodeValue::eObjects)
            || (type == CMQueryNodeValue::eRef);
        if (!type_ok)
            return false;
    }

    for (size_t index = 2; index < as; ++index) {
        if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString) {
            return false;
        }
    }

    return true;
}


/// class CMacroFunction_ParseFeatQual
/// ParseFeatureQual(src_field, dest_feature_type, dest_feature_field, capitalization, update_mrna, existing_text_opt, delimiter)
/// The last parameter is optional
/// The related mRNA is only updated if the action is successful and the destination feature is protein
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ParseFeatQual, "ParseFeatureQual");
void CMacroFunction_ParseFeatQual::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* src_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!src_feat || !scope)
        return;

    const string& dest_field = m_Args[2]->GetString();
    if (dest_field.empty())
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs src_objs;
    if (!x_GetSourceFields(oi, 0, src_objs))
        return;

    // the destination field can be set at this point
    CSeqFeatData::ESubtype target_feature = NMacroUtil::GetFeatSubtype(m_Args[1]->GetString());
    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(*src_feat, target_feature, scope);
    CMQueryNodeValue::TObs dest_objs;
    bool changed = false, created = false;
    CRef<CSeq_feat> dest_feat;
    CSeq_feat_Handle fh;
    if (!feat_list.empty() && feat_list.size() == 1) {
        dest_feat.Reset(new CSeq_feat);
        dest_feat->Assign(*feat_list.front());
        if (!SetFeatDestinationField(dest_feat, dest_field, dest_objs)) {
            return;
        }
        fh = scope->GetSeq_featHandle(*feat_list.front());
        changed = true;
    }
    else {
        // new feature
        dest_feat = CreateNewRelatedFeature(*src_feat, dest_field, *scope);
        if (!dest_feat) return;
        if (!SetFeatDestinationField(dest_feat, dest_field, dest_objs)) {
            return;
        }
        created = true;
        if (src_feat->GetData().IsCdregion() && src_feat->IsSetXref() && dest_feat->GetData().IsProt()) {
            CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
            NON_CONST_ITERATE(CSeq_feat::TXref, it, edit_feat->SetXref()) {
                if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                    dest_feat->SetData().SetProt().Assign((*it)->GetData().GetProt());
                    edit_feat->SetXref().erase(it);
                    break;
                }
            }
            if (edit_feat->GetXref().empty()) {
                edit_feat->ResetXref();
            }
        }
    }

    size_t index = 3;
    const string& capitalization = m_Args[index]->GetString();
    bool update_mrna = m_Args[++index]->GetBool();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter = (++index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;

    m_ExistingText = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);
    m_CapChange = NMacroUtil::ConvertStringtoCapitalOption(capitalization);

    if (IstRNAProductField(*dest_feat, dest_field)) {
        for (auto&& it : src_objs) {
            CMQueryNodeValue::TObs src_prim_objs;
            NMacroUtil::GetPrimitiveObjectInfos(src_prim_objs, it);

            CObjectInfo src = src_prim_objs.front().field;
            string src_val = x_GetSourceString(src);

            string remainder;
            dest_feat->SetData().SetRna().SetRnaProductName(src_val, remainder);
            m_QualsChangedCount++;
        }
    }
    else {
        ChangeFields(src_objs, dest_objs);
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": parsed " << m_QualsChangedCount << " qualifiers";

        CRef<CCmdComposite> cmd;
        if (changed && fh) {
            cmd.Reset(new CCmdComposite("Change Related feature"));
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *dest_feat)));
        }
        else if (created) {
            cmd.Reset(new CCmdComposite("Create feature"));
            CBioseq_Handle bsh = scope->GetBioseqHandle(dest_feat->GetLocation());
            cmd->AddCommand(*CRef<CCmdCreateFeatBioseq>(new CCmdCreateFeatBioseq(bsh, *dest_feat)));
        }

        if (update_mrna) {
            if (dest_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                string message;
                CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(*dest_feat, *scope, message);
                if (upd_cmd) {
                    cmd->AddCommand(*upd_cmd);
                    log << ", " << message;
                }
            }
            else if (src_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
                CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
                string prot_product = edit_feat->GetData().GetProt().GetName().front();
                CRef<CCmdComposite> upd_cmd = UpdatemRNAProduct(prot_product, obj, *scope);
                if (upd_cmd) {
                    cmd->AddCommand(*upd_cmd);
                    log << ", applied " + prot_product + " to mRNA product name ";
                }
            }
        }
        m_DataIter->RunCommand(cmd, m_CmdComposite);
        x_LogFunction(log);
    }
}

bool CMacroFunction_ParseFeatQual::x_ValidArguments() const
{
    if (m_Args.size() < 6 || m_Args.size() > 7)
        return false;

    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    bool first_ok = (type == CMQueryNodeValue::eString)
        || (type == CMQueryNodeValue::eObjects)
        || (type == CMQueryNodeValue::eRef);
    if (!first_ok)
        return false;

    for (size_t index = 1; index < m_Args.size(); ++index) {
        type = (index != 4) ? CMQueryNodeValue::eString : CMQueryNodeValue::eBool;
        if (m_Args[index]->GetDataType() != type)
            return false;
    }
    return true;
}


/// class CMacroFunction_AddParsedText
/// AddParsedText(parsed_text, to_field, capitalization, existing_text_opt, delimiter)
/// Deprecated name: AECRParseStringQual
/// when only the "text" portion is copied 
/// The capitalization change applies to the parsed text portion
/// 
DEFINE_MACRO_FUNCNAME(CMacroFunction_AddParsedText, "AddParsedText");

void CMacroFunction_AddParsedText::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();

    // obtain the wanted text portion
    CMQueryNodeValue& parsed_obj = m_Args[0].GetNCObject();
    parsed_obj.Dereference();
    if (parsed_obj.GetDataType() != CMQueryNodeValue::eString) {
        return;
    }
    string text_portion = parsed_obj.GetString();
    if (text_portion.empty()) {
        return;
    }
    size_t index = 1;
    // the destination field might be set during this action
    CMQueryNodeValue::TObs dest_objs;
    CMQueryNodeValue::EType dest_type = m_Args[index]->GetDataType();

    if (dest_type == CMQueryNodeValue::eString) {
        const string& dest_field = m_Args[index]->GetString();
        CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
        GetDestinationObjects(obj, oi, dest_field, dest_objs);
    }
    else if (dest_type == CMQueryNodeValue::eObjects) {
        dest_objs = m_Args[index]->GetObjects();
    }
    else if (dest_type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(dest_objs, index);
    }

    if (dest_objs.empty()) {
        return;
    }

    const string& capitalization = m_Args[++index]->GetString();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter = (++index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;

    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);
    ECapChange cap_change = NMacroUtil::ConvertStringtoCapitalOption(capitalization);

    CSeq_entry_Handle seh = m_DataIter->GetSEH();
    FixCapitalizationInString(seh, text_portion, cap_change);
    x_ParseFields(dest_objs, text_portion, existing_text);

    if (m_QualsChangedCount) {
        NMacroUtil::CleanupForTaxnameChange(dest_objs.front(), oi);
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": parsed " << m_QualsChangedCount << " fields";
        x_LogFunction(log);
    }
}

void CMacroFunction_AddParsedText::x_ParseFields(CMQueryNodeValue::TObs& dest_objs, 
    const string& text_portion, edit::EExistingText existing_text)
{
    CMQueryNodeValue::TObs objs;
    NMacroUtil::GetPrimitiveObjectInfos(objs, dest_objs.front());
    if (objs.empty() && dest_objs.front().field.GetTypeFamily() == eTypeFamilyContainer) {
        // add new element when the container is empty
        CObjectInfo new_oi(dest_objs.front().field.AddNewElement());
        objs.push_back(CMQueryNodeValue::SResolvedField(dest_objs.front().field, new_oi));
    }
    CObjectInfo dest = objs.front().field;
    _ASSERT(dest.GetPrimitiveValueType() == ePrimitiveValueString);

    if (dest.GetPrimitiveValueType() == ePrimitiveValueString) {

        string dest_val = dest.GetPrimitiveValueString();
        if (edit::AddValueToString(dest_val, text_portion, existing_text)) {
            SetQualStringValue(dest, dest_val);
        }
    }
}


bool CMacroFunction_AddParsedText::x_ValidArguments(void) const
{
    size_t as = m_Args.size();
    if (as != 4 && as != 5) {
        return false;
    }

    if (m_Args[0]->GetDataType() != CMQueryNodeValue::eRef) {
        return false;
    }

    CMQueryNodeValue::EType type = m_Args[1]->GetDataType();
    bool type_ok = (type == CMQueryNodeValue::eString)
        || (type == CMQueryNodeValue::eObjects)
        || (type == CMQueryNodeValue::eRef);
    if (!type_ok)
        return false;

    for (size_t index = 2; index < as; ++index) {
        if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString) {
            return false;
        }
    }

    return true;
}

// class CMacroFunction_AddParsedFeatQual
/// AddParsedTextToFeatureQual(parsed_text, dest_feature_type, dest_feature_field, capitalization, update_mrna, existing_text_opt, delimiter)
/// The last parameter is optional
/// The related mRNA is only updated if the action is successful and the destination feature is protein
DEFINE_MACRO_FUNCNAME(CMacroFunction_AddParsedToFeatQual, "AddParsedTextToFeatureQual");
void CMacroFunction_AddParsedToFeatQual::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* src_feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!src_feat || !scope)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();

    // obtain the wanted text portion
    CMQueryNodeValue& parsed_obj = m_Args[0].GetNCObject();
    parsed_obj.Dereference();
    if (parsed_obj.GetDataType() != CMQueryNodeValue::eString) {
        return;
    }
    string text_portion = parsed_obj.GetString();
    if (text_portion.empty()) {
        return;
    }

    const string& dest_field = m_Args[2]->GetString();
    if (dest_field.empty())
        return;

    // the destination field can be set at this point
    CSeqFeatData::ESubtype target_feature = NMacroUtil::GetFeatSubtype(m_Args[1]->GetString());
    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(*src_feat, target_feature, scope);
    CMQueryNodeValue::TObs dest_objs;
    bool changed = false, created = false;
    CRef<CSeq_feat> dest_feat;
    CSeq_feat_Handle fh;

    if (!feat_list.empty() && feat_list.size() == 1) {
        dest_feat.Reset(new CSeq_feat);
        dest_feat->Assign(*feat_list.front());
        if (!SetFeatDestinationField(dest_feat, dest_field, dest_objs)) {
            return;
        }
        fh = scope->GetSeq_featHandle(*feat_list.front());
        changed = true;
    }
    else {
        // new feature
        dest_feat = CreateNewRelatedFeature(*src_feat, dest_field, *scope);
        if (!dest_feat) return;
        if (!SetFeatDestinationField(dest_feat, dest_field, dest_objs)) {
            return;
        }
        created = true;
        if (src_feat->GetData().IsCdregion() && src_feat->IsSetXref() && dest_feat->GetData().IsProt()) {
            CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
            NON_CONST_ITERATE(CSeq_feat::TXref, it, edit_feat->SetXref()) {
                if ((*it)->IsSetData() && (*it)->GetData().IsProt()) {
                    dest_feat->SetData().SetProt().Assign((*it)->GetData().GetProt());
                    edit_feat->SetXref().erase(it);
                    break;
                }
            }
            if (edit_feat->GetXref().empty()) {
                edit_feat->ResetXref();
            }
        }
    }

    size_t index = 3;
    const string& capitalization = m_Args[index]->GetString();
    bool update_mrna = m_Args[++index]->GetBool();
    const string& action_type = m_Args[++index]->GetString();
    string delimiter = (++index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;

    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);
    ECapChange cap_change = NMacroUtil::ConvertStringtoCapitalOption(capitalization);

    CSeq_entry_Handle seh = m_DataIter->GetSEH();
    FixCapitalizationInString(seh, text_portion, cap_change);
    if (IstRNAProductField(*dest_feat, dest_field)) {
        string remainder;
        dest_feat->SetData().SetRna().SetRnaProductName(text_portion, remainder);
        m_QualsChangedCount++;
    }
    else {
        x_ParseFields(dest_objs, text_portion, existing_text);
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": parsed " << m_QualsChangedCount << " fields";

        if (changed && fh) {
            CRef<CCmdComposite> cmd(new CCmdComposite("Change Related feature"));
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *dest_feat)));
            m_DataIter->RunCommand(cmd, m_CmdComposite);
        }
        else if (created) {
            CRef<CCmdComposite> cmd(new CCmdComposite("Create feature"));
            CBioseq_Handle bsh = scope->GetBioseqHandle(dest_feat->GetLocation());
            cmd->AddCommand(*CRef<CCmdCreateFeatBioseq>(new CCmdCreateFeatBioseq(bsh, *dest_feat)));
            m_DataIter->RunCommand(cmd, m_CmdComposite);
        }

        if (update_mrna && dest_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_prot) {
            string message;
            CRef<CCmdComposite> cmd = UpdatemRNAProduct(*dest_feat, *scope, message);
            if (cmd) {
                m_DataIter->RunCommand(cmd, m_CmdComposite);
                log << ", " << message;
            }
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_AddParsedToFeatQual::x_ValidArguments() const
{
    if (m_Args.size() < 6 || m_Args.size() > 7)
        return false;
    
    if (m_Args[0]->GetDataType() != CMQueryNodeValue::eRef) {
        return false;
    }
    CMQueryNodeValue::EType type;
    for (size_t index = 1; index < m_Args.size(); ++index) {
        type = (index != 4) ? CMQueryNodeValue::eString : CMQueryNodeValue::eBool;
        if (m_Args[index]->GetDataType() != type)
            return false;
    }
    return true;

}

///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_ParseToStructComm
/// ParseToStructComment(text/src_field, structcomm_field ("fieldvalue"|"dbname"|"fieldname"), [fieldname,] 
/// capitalization, existing_text_opt, delimiter))
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ParseToStructComm, "ParseToStructComment")

void CMacroFunction_ParseToStructComm::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!scope || !object)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    size_t index = 0;

    CMQueryNodeValue text_holder;
    string text_portion;

    CMQueryNodeValue::TObs res_oi;
    CMQueryNodeValue::EType type = m_Args[index]->GetDataType();
    if (type == CMQueryNodeValue::eString) {
        const string& field = m_Args[0]->GetString();
        ResolveIdentToSimple(oi, field, text_holder);
        GetFieldsByName(&res_oi, oi, field);
    }
    else if (type == CMQueryNodeValue::eObjects) {
        res_oi = m_Args[index]->GetObjects();
        if (!res_oi.empty()) {
            ResolveIdentToSimple(res_oi.front().field, kEmptyStr, text_holder);
        }
    }
    else if (type == CMQueryNodeValue::eRef) {
        CMQueryNodeValue& objs = m_Args[index].GetNCObject();
        objs.Dereference();

        if (objs.GetDataType() == CMQueryNodeValue::eObjects) {
            res_oi = objs.GetObjects();
            if (!res_oi.empty()) {
                ResolveIdentToSimple(res_oi.front().field, kEmptyStr, text_holder);
            }
        }
        else if (objs.GetDataType() == CMQueryNodeValue::eString) {
            text_portion = objs.GetString();
        }
    }

    if (text_holder.GetDataType() == CMQueryNodeValue::eString && text_portion.empty()) {
        text_portion = text_holder.GetString();
    }

    if (text_portion.empty())
        return;
    
    x_SetFieldType(m_Args[++index]->GetString());
    string fieldname = (m_Type == eFieldValue) ? m_Args[++index]->GetString() : kEmptyStr;
    if (m_Type == eFieldError) {
        NCBI_THROW(CMacroExecException, eWrongArguments, "Wrong structured comment fieldname is given (consider replacing it with either 'fieldvalue', 'dbname' or 'fieldname'");
    }

    string capitalization, action_type;
    if (++index < m_Args.size()) {
        capitalization = m_Args[index]->GetString();
    }
    if (++index < m_Args.size()) {
        action_type = m_Args[index]->GetString();
    }
    if (capitalization.empty() || action_type.empty()) {
        NCBI_THROW(CMacroExecException, eWrongArguments, "Wrong arguments specified");
    }

    string delimiter = (++index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;
    
    ECapChange cap_change = NMacroUtil::ConvertStringtoCapitalOption(capitalization);
    CSeq_entry_Handle seh = m_DataIter->GetSEH();
    FixCapitalizationInString(seh, text_portion, cap_change);

    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);
    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();

    CRef<CCmdComposite> cmd;
    CSeqdesc_CI strcomm_it(bsh, CSeqdesc::e_User);
    // obtain the existing structured comment
    for ( ; strcomm_it; ++strcomm_it) {
        if (CComment_rule::IsStructuredComment(strcomm_it->GetUser())) {
            break;
        }
    }
    const string kPrefix = "StructuredCommentPrefix";
    const string kSuffix = "StructuredCommentSuffix";

    if (strcomm_it) {
        CRef<CSeqdesc> new_desc(new CSeqdesc);
        new_desc->Assign(*strcomm_it);

        CUser_object& user_object = new_desc->SetUser();
        if (existing_text == edit::eExistingText_add_qual) {
            // add a new field to the structured comment
            if (m_Type == eFieldValue) {
                user_object.AddField(fieldname, text_portion);
                m_QualsChangedCount++;
            }
        }
        else {
            if (m_Type == eFieldValue) {
                if (user_object.HasField(fieldname)) {
                    CUser_field& user_field = user_object.SetField(fieldname);
                    if (user_field.IsSetData() && user_field.GetData().IsStr()) {
                        string orig_val = user_field.GetData().GetStr();
                        if (AddValueToString(orig_val, text_portion, existing_text)) {
                            user_field.SetData().SetStr(orig_val);
                            m_QualsChangedCount++;
                        }
                    }
                }
                else {
                    // add a new field to the structured comment
                    user_object.AddField(fieldname, text_portion);
                    m_QualsChangedCount++;
                }
            }
            else if (m_Type == eDbName) {
                m_QualsChangedCount += CMacroFunction_SetStructCommDb::s_UpdateStructCommentDb(*new_desc, kPrefix, text_portion, existing_text);
                m_QualsChangedCount += CMacroFunction_SetStructCommDb::s_UpdateStructCommentDb(*new_desc, kSuffix, text_portion, existing_text);
            }
            else if (m_Type == eFieldName) {
                EDIT_EACH_USERFIELD_ON_USEROBJECT(field_it, user_object) {
                    CUser_field& user_field = **field_it;
                    if (user_field.GetLabel().IsStr() &&
                        !NStr::Equal(user_field.GetLabel().GetStr(), kPrefix) &&
                        !NStr::Equal(user_field.GetLabel().GetStr(), kSuffix)) {
                        string orig_val = user_field.GetLabel().GetStr();
                        if (AddValueToString(orig_val, text_portion, existing_text)) {
                            user_field.SetLabel().SetStr(orig_val);
                            m_QualsChangedCount++;
                        }
                    }
                }
            }

            if (m_QualsChangedCount) {
                CMacroFunction_SetStructCommDb::s_MoveSuffixToTheEnd(new_desc->SetUser());
                CCleanup::CleanupUserObject(new_desc->SetUser());

                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(strcomm_it.GetSeq_entry_Handle(), *strcomm_it, *new_desc));
                cmd.Reset(new CCmdComposite("Update structured comment"));
                cmd->AddCommand(*ecmd);
            }
        }

    }
    else { // make a new one
        if ( existing_text == edit::eExistingText_leave_old) {
            return;
        }

        CRef<CCmdComposite> cmd;
        CRef<CSeqdesc> new_desc(new CSeqdesc());
        CUser_object& user_object = new_desc->SetUser();
        user_object.SetObjectType(CUser_object::eObjectType_StructuredComment);
        switch (m_Type) {
        case eFieldValue:
            user_object.AddField(fieldname, text_portion);
            m_QualsChangedCount++;
            break;
        case eDbName:
            user_object.AddField(kPrefix, CComment_rule::MakePrefixFromRoot(text_portion));
            user_object.AddField(kSuffix, CComment_rule::MakeSuffixFromRoot(text_portion));
            m_QualsChangedCount++;
            break;
        case eFieldName:
            user_object.AddField(text_portion, kEmptyStr);
            m_QualsChangedCount++;
            break;
        default:
            break;
        }

        if (m_QualsChangedCount) {
            cmd.Reset(new CCmdComposite("Create structured comment"));
            cmd->AddCommand(*(new CCmdCreateDesc(bsh.GetSeq_entry_Handle(), *new_desc)));
        }
    }

    if (cmd) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);
        CNcbiOstrstream log;
        log << m_QualsChangedCount << ";parsing text to structured comment ";
        switch (m_Type) {
        case eFieldValue:
            log << "field '" << fieldname << "'";
            break;
        case eDbName:
            log << "database name";
            break;
        case eFieldName:
            log << "fieldname";
            break;
        default:
            break;
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_ParseToStructComm::x_ValidArguments() const
{
    if (m_Args.size() > 6 || m_Args.size() < 4) {
        return false;
    }
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    bool first_ok = (type == CMQueryNodeValue::eString) || 
                    (type == CMQueryNodeValue::eObjects) || 
                    (type == CMQueryNodeValue::eRef);
    if (!first_ok)
        return false;

    for (size_t index = 1; index < m_Args.size(); ++index) {
        if (m_Args[index]->GetDataType() != CMQueryNodeValue::eString)
            return false;
    }
    return true;
}

void CMacroFunction_ParseToStructComm::x_SetFieldType(const string& strcomm_field)
{
    m_Type = eFieldError;
    if (NStr::EqualNocase(strcomm_field, "fieldvalue")) {
        m_Type = eFieldValue;
    }
    else if (NStr::EqualNocase(strcomm_field, "dbname")) {
        m_Type = eDbName;
    }
    else if (NStr::EqualNocase(strcomm_field, "fieldname")) {
        m_Type = eFieldName;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ParsedText
/// Obtains a portion of the text
/// ParsedText(field_name(str/objects), left_del(str), include_left(b), right_del(str), include_right(b), 
///            case_sensitive(b), whole_word(b), rmv_from_parsed(b), rmv_left(b), rmv_right(b));
/// left_del and right_del can be free_text, "eDigits", "eLetters"
/// the last three parameters are optional
/// It may also modify the content of the field
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ParsedText, "ParsedText");
void CMacroFunction_ParsedText::TheFunction()
{
    m_ChangedDescriptors.clear();
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    CObjectInfo oi = m_DataIter->GetEditedObject();

    CMQueryNodeValue::TObs res_oi;
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    if (type == CMQueryNodeValue::eString) {
        const string& field = m_Args[0]->GetString();
        if (NStr::EqualNocase(field, "localid") || NStr::EqualNocase(field, "defline")) {
            x_GetSpecialField(field, res_oi, *scope);
        }
        else {
            ResolveIdentToSimple(oi, field, *m_Result);
            GetFieldsByName(&res_oi, oi, field);
        }
    }
    else {
        if (type == CMQueryNodeValue::eObjects) {
            res_oi = m_Args[0]->GetObjects();
        }
        else if (type == CMQueryNodeValue::eRef) {
            x_GetObjectsFromRef(res_oi, 0);
            NMacroUtil::SwapGbQualWithValues(res_oi);
        }

        if (res_oi.empty()) return;
        ResolveIdentToSimple(res_oi.front().field, kEmptyStr, *m_Result);
    }
    
    if (m_Result->GetDataType() != CMQueryNodeValue::eString) {
        m_Result->SetDataType(CMQueryNodeValue::eNotSet);
        return;
    }

    const string& full_text = m_Result->GetString();
    CRef<edit::CParseTextOptions> options = x_GetParseOptions();
    string parsed_text = options->GetSelectedText(full_text);
    m_Result->SetString(parsed_text);

    if (options->ShouldRemoveFromParsed()) {
        if (!m_ChangedDescriptors.empty()) {
            auto it = m_ChangedDescriptors.begin();
            if (CSeqdesc* desc = dynamic_cast<CSeqdesc*>(it->second.GetNCPointer())) {
                if (desc->IsTitle()) {
                    string orig_val = desc->GetTitle();
                    options->RemoveSelectedText(orig_val);
                    if (!orig_val.empty()) {
                        desc->SetTitle() = orig_val;

                        CRef<CCmdChangeSeqdesc> chg_cmd(new CCmdChangeSeqdesc(it->first.GetSeq_entry_Handle(), *it->first, *it->second));
                        CRef<CCmdComposite> cmd(new CCmdComposite("Update definition line"));
                        cmd->AddCommand(*chg_cmd);
                        m_DataIter->RunCommand(cmd, m_CmdComposite);
                    }
                    // the title will not be deleted
                }
            }
        }
        else if (!res_oi.empty()) {
            CMQueryNodeValue::TObs objs;
            NMacroUtil::GetPrimitiveObjectInfos(objs, res_oi.front());
            CObjectInfo objInfo = objs.front().field;
            _ASSERT(objInfo.GetPrimitiveValueType() == ePrimitiveValueString);

            bool is_taxname = NMacroUtil::IsTaxname(objs.front());
            string orig_val = objInfo.GetPrimitiveValueString();
            options->RemoveSelectedText(orig_val);
            if (orig_val.empty()) {
                RemoveFieldByName(res_oi.front());
                m_QualsChangedCount++;
            }
            else {
                SetQualStringValue(objInfo, orig_val);
            }
            if (is_taxname) {
                NMacroUtil::CleanupForTaxnameChange(oi);
            }
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
    }
}

bool CMacroFunction_ParsedText::x_ValidArguments() const
{
    size_t as = m_Args.size();
    if (as < 7 || as > 10) {
        return false;
    }
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    bool first_ok = (type == CMQueryNodeValue::eString) || (type == CMQueryNodeValue::eObjects) || (type == CMQueryNodeValue::eRef);
    if (!first_ok)
        return false;

    if (m_Args[1]->GetDataType() != CMQueryNodeValue::eString
        || m_Args[2]->GetDataType() != CMQueryNodeValue::eBool
        || m_Args[3]->GetDataType() != CMQueryNodeValue::eString) {
        return false;
    }

    for (size_t i = 4; i < as; ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eBool)
            return false;
    }
    return true;
}

bool CMacroFunction_ParsedText::x_GetSpecialField(const string& field, CMQueryNodeValue::TObs& objs, CScope& scope)
{
    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    if (bsh && bsh.IsAa()) {
        const CSeq_feat* cds = sequence::GetCDSForProduct(*bsh.GetCompleteBioseq(), &scope);
        if (cds) {
            bsh = scope.GetBioseqHandle(cds->GetLocation());
        }
    }
    if (!bsh) return false;

    if (NStr::EqualNocase(field, "defline")) {
        CSeqdesc_CI title_ci(bsh, CSeqdesc::e_Title, 1);
        if (title_ci && !title_ci->GetTitle().empty()) {
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(*title_ci);
            CObjectInfo oi(new_desc, new_desc->GetThisTypeInfo());
            CObjectInfo parent;
            objs.push_back(CMQueryNodeValue::SResolvedField(parent, oi));
            if (ResolveIdentToSimple(oi, "title", *m_Result)) {
                m_ChangedDescriptors.emplace_back(title_ci, new_desc);
                return true;
            }
        }
    }
    else if (NStr::EqualNocase(field, "localid")) {
        for (CSeqdesc_CI desc(bsh, CSeqdesc::e_User); desc; ++desc) {
            const CUser_object& user = desc->GetUser();
            if (user.GetObjectType() == CUser_object::eObjectType_OriginalId) {
                if (!user.GetData().empty()) {
                    const CRef<CUser_field>& user_field = user.GetData().front();
                    if (user_field->IsSetLabel() && user_field->GetLabel().IsStr()
                        && user_field->IsSetData()
                        && NStr::EqualCase(user_field->GetLabel().GetStr(), "LocalId")) {

                        if (user_field->GetData().IsStr()) {
                            CObjectInfo objInfo(user_field.GetNCPointer(), user_field->GetTypeInfo());
                            if (ResolveIdentToSimple(objInfo, "data.str", *m_Result))
                                return true;
                        }
                    }
                }
            }
        }


        if (m_Result->GetDataType() == CMQueryNodeValue::eNotSet) {
            CBioseq& seq = const_cast<CBioseq&>(bsh.GetCompleteBioseq().GetObject());
            CObjectInfo objInfo(&seq, seq.GetTypeInfo());
            if (ResolveIdentToSimple(objInfo, "id.local.str", *m_Result)) {
                return true;
            }
        }
    }
    return false;
}

CRef<edit::CParseTextOptions> CMacroFunction_ParsedText::x_GetParseOptions()
{
    CRef<edit::CParseTextOptions> options(new edit::CParseTextOptions());
    const string& left_del = m_Args[1]->GetString();
    if (NStr::EqualCase(left_del, "eDigits")) {
        options->SetStartDigits();
    }
    else if (NStr::EqualCase(left_del, "eLetters")) {
        options->SetStartLetters();
    }
    else {
        options->SetStartText(left_del);
    }
    options->SetIncludeStart(m_Args[2]->GetBool());  // include_start

    const string& right_del = m_Args[3]->GetString();
    if (NStr::EqualCase(right_del, "eDigits")) {
        options->SetStopDigits();
    }
    else if (NStr::EqualCase(right_del, "eLetters")) {
        options->SetStopLetters();
    }
    else {
        options->SetStopText(right_del);
    }
    options->SetIncludeStop(m_Args[4]->GetBool()); // include_stop

    options->SetCaseInsensitive(!m_Args[5]->GetBool()); // case_sensitivity
    options->SetWholeWord(m_Args[6]->GetBool());
    if (m_Args.size() > 7) {
        options->SetShouldRemove(m_Args[7]->GetBool());
    }
    if (m_Args.size() > 8) {
        options->SetShouldRmvBeforePattern(m_Args[8]->GetBool());
    }
    if (m_Args.size() > 9) {
        options->SetShouldRmvAfterPattern(m_Args[9]->GetBool());
    }

    return options;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_LocalID
/// Resolves the local id: the original id if there is one, otherwise the actual local id on the sequence
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_LocalID, "LocalID")
void CMacroFunction_LocalID::TheFunction()
{
    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    if (!bsh) return;

    for (CSeqdesc_CI desc(bsh, CSeqdesc::e_User); desc; ++desc) {
        const CUser_object& user = desc->GetUser();
        if (user.GetObjectType() == CUser_object::eObjectType_OriginalId) {
            if (!user.GetData().empty()) {
                const CRef<CUser_field>& user_field = user.GetData().front();
                if (user_field->IsSetLabel() && user_field->GetLabel().IsStr()
                    && user_field->IsSetData()
                    && NStr::EqualCase(user_field->GetLabel().GetStr(), "LocalId")) {

                    if (user_field->GetData().IsStr()) {
                        CObjectInfo objInfo(user_field.GetNCPointer(), user_field->GetTypeInfo());
                        if (ResolveIdentToObjects(objInfo, "data.str", *m_Result))
                            return;
                    }
                }
            }
        }
    }


    if (m_Result->GetDataType() == CMQueryNodeValue::eNotSet) {
        CBioseq& seq = const_cast<CBioseq&>(bsh.GetCompleteBioseq().GetObject());
        CObjectInfo objInfo(&seq, seq.GetTypeInfo());
        ResolveIdentToObjects(objInfo, "id.local.str", *m_Result);
    }
}

bool CMacroFunction_LocalID::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetQual
/// SetQual(field_name, newValue)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetQual, "SetQual")

void CMacroFunction_SetQual::TheFunction()
{
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    CMQueryNodeValue& new_value = *m_Args[1];

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    if (type == CMQueryNodeValue::eString) {
        if (!SetFieldsByName(&res_oi, oi, m_Args[0]->GetString())) {
            return;
        }
    }
    else if (type == CMQueryNodeValue::eObjects) {
        res_oi = m_Args[0]->GetObjects();
    }
    else if (type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(res_oi, 0);
    }

    if (res_oi.empty()) {
        return;
    }

    NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, res_oi) {
        if (SetSimpleTypeValue(it->field, new_value)) {
            m_QualsChangedCount++;
        }
    }

    if (m_QualsChangedCount) {
        NMacroUtil::CleanupForTaxnameChange(res_oi.front(), oi);
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": set new value to " << m_QualsChangedCount << " qualifiers";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetQual::x_ValidArguments() const
{
    // can accept as its first parameter: objects, string or reference
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    bool first_ok = (type == CMQueryNodeValue::eString) || (type == CMQueryNodeValue::eObjects) || (type == CMQueryNodeValue::eRef);
    if (m_Args.size() != 2 || !first_ok)
        return false;

    if (!m_Args[1]->IsSimpleType()) {
        return false;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// class CMacroFunction_AddDBLink
/// AddDBLink(dblink_type, newValue, existing_text, delimiter, remove_blank)
/// The last parameter is optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_AddDBLink, "AddDBLink")

static void s_AddNewDBlinkValue(CUser_field& user_field, const string& newValue);

static void s_SetVectorValue(CUser_field& user_field, const string& newValue, const string& del);

void CMacroFunction_AddDBLink::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!scope || !bseq || bseq->IsAa())
        return;

    size_t index = 0;
    const string& dblink = m_Args[index]->GetString();
    string newValue = NMacroUtil::GetStringValue(m_Args[++index]);
    const string& existing_text_option = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(existing_text_option, delimiter);

    CRef<CCmdComposite> cmd;
    CNcbiOstrstream log;

    if (!newValue.empty()) {
        vector<pair<CSeqdesc_CI, CRef<CSeqdesc>>> changed_descs;
        CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();

        for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_User); desc_it; ++desc_it) {
            if (desc_it->GetUser().GetType().IsStr() && desc_it->GetUser().GetType().GetStr() == "DBLink") {
                CRef<CSeqdesc> new_desc(new CSeqdesc);
                new_desc->Assign(*desc_it);

                bool found_user_field = false;
                EDIT_EACH_USERFIELD_ON_USEROBJECT(user_it, new_desc->SetUser()) {
                    CUser_field& field = **user_it;
                    if (field.GetLabel().IsStr() && NStr::Equal(field.GetLabel().GetStr(), dblink) && field.IsSetData()) {
                        found_user_field = true;
                        if (field.GetData().IsStrs()) {
                            if (existing_text == edit::eExistingText_add_qual) {
                                s_AddNewDBlinkValue(field, newValue);
                                m_QualsChangedCount++;
                            }
                            else if (existing_text == edit::eExistingText_replace_old) {
                                // apply the same logic as in SetStrinQual:
                                // keep the first element, update it and delete the rest 
                                field.SetData().Reset();
                                field.ResetNum();
                                s_AddNewDBlinkValue(field, newValue);
                                m_QualsChangedCount++;
                                
                            } else {
                                // update all of them
                                for (auto& it : field.SetData().SetStrs()) {
                                    string orig_value = it;
                                    if (edit::AddValueToString(orig_value, newValue, existing_text)) {
                                        it = orig_value;
                                        m_QualsChangedCount++;
                                    }
                                }
                            }
                        }
                    }
                }

                if (!found_user_field) {
                    x_AddNewUserField(new_desc, dblink, newValue);
                }
                changed_descs.push_back(make_pair(desc_it, new_desc));
            }
        }

        if (changed_descs.empty()) {
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->SetUser().SetType().SetStr("DBLink");
            x_AddNewUserField(new_desc, dblink, newValue);

            cmd.Reset(new CCmdComposite("Create DBLink user object"));
            cmd->AddCommand(*(new CCmdCreateDesc(bsh.GetSeq_entry_Handle(), *new_desc)));
        }
        else if (!changed_descs.empty()) {
            cmd.Reset(new CCmdComposite("Update DBLink user object"));
            for (auto& it : changed_descs) {
                CRef<CCmdChangeSeqdesc> chg_cmd(new CCmdChangeSeqdesc(it.first.GetSeq_entry_Handle(), *it.first, *it.second));
                cmd->AddCommand(*chg_cmd);
            }
        }

        if (m_QualsChangedCount) {
            log << m_QualsChangedCount << ";setting new value to " << dblink;
        }
    }
    else if (remove_field) {
        CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
        for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_User); desc_it; ++desc_it) {
            if (desc_it->GetUser().GetType().IsStr() && desc_it->GetUser().GetType().GetStr() == "DBLink") {
                CRef<CSeqdesc> new_desc(new CSeqdesc);
                new_desc->Assign(*desc_it);

                EDIT_EACH_USERFIELD_ON_USEROBJECT(user_it, new_desc->SetUser()) {
                    CUser_field& field = **user_it;
                    if (field.GetLabel().IsStr() && NStr::Equal(field.GetLabel().GetStr(), dblink) && field.IsSetData()) {
                        ERASE_USERFIELD_ON_USEROBJECT(user_it, new_desc->SetUser());
                        m_QualsChangedCount++;
                    }
                }
                if (new_desc->GetUser().GetData().empty()) {
                    // to be deleted
                    if (!cmd) {
                        cmd.Reset(new CCmdComposite("Delete DBLink user object"));
                    }
                    cmd->AddCommand(*(new CCmdDelDesc(desc_it.GetSeq_entry_Handle(), *desc_it)));
                }
                else {
                    if (!cmd) {
                        cmd.Reset(new CCmdComposite("Update DBLink user object"));
                    }
                    CRef<CCmdChangeSeqdesc> chg_cmd(new CCmdChangeSeqdesc(desc_it.GetSeq_entry_Handle(), *desc_it, *new_desc));
                    cmd->AddCommand(*chg_cmd);
                }
            }
        }
        if (m_QualsChangedCount) {
            log << m_QualsChangedCount << ";removed " << dblink << " value";
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);
        x_LogFunction(log);
    }
}


void CMacroFunction_AddDBLink::x_AddNewUserField(CRef<CSeqdesc>& user_object_desc, const string& dblink, const string& newValue)
{
    CRef<CUser_field> new_field(new CUser_field);
    new_field->SetLabel().SetStr(dblink);

    string del = (newValue.find(',') != NPOS) ? "," : kEmptyStr;
    if (del.empty() && newValue.find(';') != NPOS) {
        del = ";";
    }

    if (del.empty()) {
        new_field->SetNum(1);
        new_field->SetData().SetStrs().push_back(newValue);
    }
    else {
        s_SetVectorValue(*new_field, newValue, del);
    }

    if (new_field->IsSetData() && new_field->GetData().IsStrs()) {
        new_field->SetNum((int)new_field->GetData().GetStrs().size());
    }
    user_object_desc->SetUser().SetData().push_back(new_field);
    m_QualsChangedCount++;
}

void s_SetVectorValue(CUser_field& user_field, const string& newValue, const string& del)
{
    vector<string> values;
    NStr::Split(newValue, del, values);
    for (auto&& it : values)
        NStr::TruncateSpacesInPlace(it);
    user_field.SetValue(values);
}

static string s_GatherExistingValues(CUser_field& user_field, const string& newValue, const string& del)
{
    string tmp_values;
    if (user_field.IsSetData() && user_field.GetData().IsStrs()) {
        for (auto&& it : user_field.GetData().GetStrs()) {
            if (!tmp_values.empty()) {
                tmp_values += del + " ";
            }
            tmp_values += it;
        }
    }
    if (!tmp_values.empty()) {
        tmp_values += del + " ";
    }
    tmp_values += newValue;
    return tmp_values;
}

void s_AddNewDBlinkValue(CUser_field& user_field, const string& newValue)
{
    string del = (newValue.find(',') != NPOS) ? "," : kEmptyStr;
    if (del.empty() && newValue.find(';') != NPOS) {
        del = ";";
    }

    if (del.empty()) {
        user_field.SetData().SetStrs().push_back(newValue);
    }
    else {
        string tmp_values = s_GatherExistingValues(user_field, newValue, del);
        s_SetVectorValue(user_field, tmp_values, del);
    }
    if (user_field.IsSetData() && user_field.GetData().IsStrs()) {
        user_field.SetNum((int)user_field.GetData().GetStrs().size());
    }
}

bool CMacroFunction_AddDBLink::x_ValidArguments() const
{
    auto arg_nr = m_Args.size();
    if (arg_nr < 3 || arg_nr > 5) {
        return false;
    }

    size_t index = 0;
    if (!m_Args[index]->IsString()) return false;
    NMacroUtil::GetPrimitiveFromRef(m_Args[++index].GetNCObject());
    if (!m_Args[index]->IsString() && !m_Args[index]->IsInt())
        return false;

    if (!m_Args[++index]->IsString()) return false;
    if (arg_nr > 3 && (!m_Args[++index]->IsString() && !m_Args[index]->IsBool())) return false;
    if (arg_nr > 4 && !m_Args[++index]->IsBool()) return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_AddorSetContElement
/// Usage: 
/// o = Resolve("org.orgname.mod") where o.subtype = "strain";
/// SetModifier(o, "org.orgname.mod", "strain", newvalue, existing_text, delimiter, remove_blank);
/// SetGBQualifier(o, container_path, gb_qualifier, newvalue, existing_text, delimiter, remove_blank);
/// The last two parameters are optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_AddorSetBsrcModifier, "SetModifier")
DEFINE_MACRO_FUNCNAME(CMacroFunction_AddorSetGbQual, "SetGBQualifier");

void CMacroFunction_AddorSetContElement::TheFunction()
{
    size_t index = 2;
    const string& subtype_name = m_Args[index]->GetString();
    const string& newValue = NMacroUtil::GetStringValue(m_Args[++index]);
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi = m_Args[0]->GetObjects();

    if (!newValue.empty()) {
        vector<string> new_values;
        if (res_oi.empty()) {  // there are no container elements(modifiers|gb qualifiers) of this kind

            CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
            const CBioSource* const_bsrc = dynamic_cast<const CBioSource*>(object.GetPointer());
            const CSeq_feat* const_feat = dynamic_cast<const CSeq_feat*>(object.GetPointer());
            if (const_bsrc && m_IsBiosrc) {
                x_AddBioSrcModifier(subtype_name, newValue);
            }
            else if (const_feat && !m_IsBiosrc) {
                CSeq_feat* seqfeat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
                m_QualsChangedCount += s_AddGBQualifier(*seqfeat, subtype_name, newValue);
            }
            new_values.push_back(newValue);

        }
        else {
            // edit the existing qualifiers
            edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);
            if (existing_text == edit::eExistingText_leave_old) {
                return;
            }

            for (auto&& it : res_oi) {
                CObjectInfo obj = it.field;
                TMemberIndex index = 2;

                if (NMacroUtil::IsSatelliteSubfield(subtype_name)) {
                    CObjectInfo value_oi = obj.GetClassMemberIterator(index).GetMember();
                    _ASSERT(value_oi.GetPrimitiveValueType() == ePrimitiveValueString);

                    string orig_value = value_oi.GetPrimitiveValueString();
                    string sat_type = CMacroFunction_SatelliteFields::s_GetSatelliteType(orig_value);
                    string sat_name = CMacroFunction_SatelliteFields::s_GetSatelliteName(orig_value);

                    if (subtype_name == kSatelliteName) {
                        if (edit::AddValueToString(sat_name, newValue, existing_text)) {
                            string sat_value = CMacroFunction_SatelliteFields::s_MakeSatellite(sat_type, sat_name);
                            SetQualStringValue(value_oi, sat_value);
                            new_values.push_back(sat_value);
                        }
                    } else if (subtype_name == kSatelliteType) {
                        string orig_sat_type = sat_type;
                        if (edit::AddValueToString(sat_type, newValue, existing_text)) {
                            auto found_it = find(kSatelliteTypes.begin(), kSatelliteTypes.end(), sat_type);
                            bool good_type = (found_it != kSatelliteTypes.end()) ? true : false;

                            string sat_value;
                            if (good_type) {
                                sat_value = CMacroFunction_SatelliteFields::s_MakeSatellite(sat_type, sat_name);
                            }
                            else {
                                if (!sat_name.empty()) {
                                    sat_name = sat_type + ":" + sat_name;
                                }
                                else {
                                    sat_name = sat_type;
                                }
                                sat_value = CMacroFunction_SatelliteFields::s_MakeSatellite(orig_sat_type, sat_name);
                            }
                            SetQualStringValue(value_oi, sat_value);
                            new_values.push_back(sat_value);
                        }
                    }
                }
                else if (NMacroUtil::IsMobileElementTSubfield(subtype_name)) {
                    CObjectInfo value_oi = obj.GetClassMemberIterator(index).GetMember();
                    _ASSERT(value_oi.GetPrimitiveValueType() == ePrimitiveValueString);

                    string orig_value = value_oi.GetPrimitiveValueString();
                    string met_type = CMacroFunction_MobileElementTypeFields::s_GetMobileElemTType(orig_value);
                    string met_name = CMacroFunction_MobileElementTypeFields::s_GetMobileElemTName(orig_value);

                    if (subtype_name == kMobileElementTName) {
                        if (edit::AddValueToString(met_name, newValue, existing_text)) {
                            string met_value = CMacroFunction_MobileElementTypeFields::s_MakeMobileElementT(met_type, met_name);
                            SetQualStringValue(value_oi, met_value);
                            new_values.push_back(met_value);
                        }
                    }
                    else if (subtype_name == kMobileElementTType) {
                        string orig_met_type = met_type;
                        if (edit::AddValueToString(met_type, newValue, existing_text)) {
                            auto found_it = find(kMobileETypeTypes.begin(), kMobileETypeTypes.end(), met_type);
                            bool good_type = (found_it != kMobileETypeTypes.end()) ? true : false;

                            string met_value;
                            if (good_type) {
                                met_value = CMacroFunction_MobileElementTypeFields::s_MakeMobileElementT(met_type, met_name);
                            }
                            else {
                                // if it's a bad type, don't insert it
                                met_value = CMacroFunction_MobileElementTypeFields::s_MakeMobileElementT(orig_met_type, met_name);
                            }
                            SetQualStringValue(value_oi, met_value);
                            new_values.push_back(met_value);
                        }
                    }
                } 
                else {
                    CObjectInfoMI mem = obj.GetClassMemberIterator(index);
                    string field_name = mem.GetMemberInfo()->GetId().GetName();

                    if (!ResolveAndSetSimpleTypeValue(obj, field_name, *m_Args[3], existing_text)) {
                        return;
                    }
                    m_QualsChangedCount++;
                    CMQueryNodeValue new_val;
                    GetSimpleTypeValue(obj, field_name, new_val);
                    if (new_val.GetDataType() == CMQueryNodeValue::eString) {
                        new_values.push_back(new_val.GetString());
                    }
                }
            }
        }

        if (m_QualsChangedCount) {
            m_DataIter->SetModified();
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr();
            for (size_t i = 0; i < new_values.size(); ++i) {
                log << " '" << new_values[i] << "' has been set as a new value";
            }
            x_LogFunction(log);
        }
    }
    else if (remove_field) {
        if (!res_oi.empty()) {
            if (NMacroUtil::IsSatelliteSubfield(subtype_name)) {
                for (auto&& it : res_oi) {
                    TMemberIndex index = 2;
                    CObjectInfo value_oi = it.field.GetClassMemberIterator(index).GetMember();
                    _ASSERT(value_oi.GetPrimitiveValueType() == ePrimitiveValueString);

                    string newValue;
                    string orig_value = value_oi.GetPrimitiveValueString();
                    if (subtype_name == kSatelliteName) {
                        newValue = CMacroFunction_SatelliteFields::s_GetSatelliteType(orig_value);
                    }
                    else if (subtype_name == kSatelliteType) {
                        newValue = CMacroFunction_SatelliteFields::s_GetSatelliteName(orig_value);
                    }

                    if (!newValue.empty()) {
                        SetQualStringValue(value_oi, newValue);
                    }
                    else {
                        // remove the object
                        if (RemoveFieldByName(it)) {
                            m_QualsChangedCount++;
                        }
                    }
                }
            }
            else if (NMacroUtil::IsMobileElementTSubfield(subtype_name)) {
                for (auto&& it : res_oi) {
                    TMemberIndex index = 2;
                    CObjectInfo value_oi = it.field.GetClassMemberIterator(index).GetMember();
                    _ASSERT(value_oi.GetPrimitiveValueType() == ePrimitiveValueString);

                    string newValue;
                    string orig_value = value_oi.GetPrimitiveValueString();
                    if (subtype_name == kMobileElementTName) {
                        newValue = CMacroFunction_MobileElementTypeFields::s_GetMobileElemTType(orig_value);
                    }
                    else if (subtype_name == kMobileElementTType) {
                        newValue = CMacroFunction_MobileElementTypeFields::s_GetMobileElemTName(orig_value);
                    }

                    if (!newValue.empty()) {
                        SetQualStringValue(value_oi, newValue);
                    }
                    else {
                        // remove the object
                        if (RemoveFieldByName(it)) {
                            m_QualsChangedCount++;
                        }
                    }
                }
            }
            else {
                for (auto&& it : res_oi) {
                    if (RemoveFieldByName(it)) {
                        m_QualsChangedCount++;
                    }
                }
            }
        }

        if (m_QualsChangedCount) {
            CConstRef<CObject> const_obj = m_DataIter->GetScopedObject().object;
            CCleanup cleanup;

            if (dynamic_cast<const CSeq_feat*>(const_obj.GetPointer())) {
                CSeq_feat* seq_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
                cleanup.BasicCleanup(*seq_feat);
            }
            else if (dynamic_cast<const CBioSource*>(const_obj.GetPointer())) {
                CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
                cleanup.BasicCleanup(*bsrc);
            }

            m_DataIter->SetModified();
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " modifiers";
            x_LogFunction(log);
        }
    }
}

bool CMacroFunction_AddorSetContElement::x_ValidArguments() const
{
    // 1 - objects
    // 2 - asn path to the modifier's container
    // 3 - type of modifier
    // 4 - value to be set, simple type
    // 5 - handling existint text
    // 6 - delimiter (optional)
    // 7 - remove blank (optional)
    size_t arg_size = m_Args.size();
    if (arg_size < 5 || arg_size > 7)
        return false;

    CMQueryNodeValue& objs = m_Args[0].GetNCObject();
    objs.Dereference();
    if (!objs.AreObjects() && !objs.IsNotSet())
        return false;

    if (!m_Args[1]->IsString() || !m_Args[2]->IsString()) return false;

    NMacroUtil::GetPrimitiveFromRef(m_Args[3].GetNCObject());
    bool type_ok = m_Args[3]->IsString() || m_Args[3]->IsInt() || m_Args[3]->IsDouble();
    if (!type_ok) return false;

    if (!m_Args[4]->IsString()) return false;

    if (arg_size > 5 && (!m_Args[5]->IsString() && !m_Args[5]->IsBool())) return false;

    if (arg_size > 6 && !m_Args[6]->IsBool()) return false;
    return true;
}

void CMacroFunction_AddorSetContElement::x_AddBioSrcModifier(const string& subtype, const string& newValue)
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
    if (!bsrc)
        return;

    if (CSubSource::IsValidSubtypeName(subtype)) {
        int subtype_val = CSubSource::GetSubtypeValue(subtype);
        if (subtype_val == CSubSource::eSubtype_other) {
            if (m_Args[1]->GetString() == "subtype" && s_AddSrcSubSource(*bsrc, subtype_val, newValue))
                m_QualsChangedCount++;
            else if (s_AddSrcOrgMod(*bsrc, subtype_val, newValue))
                m_QualsChangedCount++;
        }
        else if (s_AddSrcSubSource(*bsrc, subtype_val, newValue)) {
            m_QualsChangedCount++;
        }
    }
    else if (COrgMod::IsValidSubtypeName(subtype)) {
        int subtype_val = COrgMod::GetSubtypeValue(subtype);
        if (s_AddSrcOrgMod(*bsrc, subtype_val, newValue)) {
            m_QualsChangedCount++;
        }
    }
    else {
        NCBI_THROW(CException, eUnknown, "Wrong subtype name: " + subtype);
    }
}

int CMacroFunction_AddorSetContElement::s_AddGBQualifier(CSeq_feat& feat, const string& qual_name, const string& newValue)
{
    int count = 0;
    string qualifier = qual_name;
    string apply_value = newValue;

    if (qual_name == kSatelliteName) {
        apply_value = CMacroFunction_SatelliteFields::s_MakeSatellite(kEmptyStr, newValue);
        qualifier = "satellite";
    }
    else if (qual_name == kSatelliteType) {
        apply_value = CMacroFunction_SatelliteFields::s_MakeSatellite(newValue, kEmptyStr);
        qualifier = "satellite";
    }
    else if (qual_name == kMobileElementTName) {
        apply_value = CMacroFunction_MobileElementTypeFields::s_MakeMobileElementT(kEmptyStr, newValue);
        qualifier = kMobileElementTQual;
    }
    else if (qual_name == kMobileElementTType) {
        apply_value = CMacroFunction_MobileElementTypeFields::s_MakeMobileElementT(newValue, kEmptyStr);
        qualifier = kMobileElementTQual;
    }
    
    if (!apply_value.empty()) {
        feat.AddQualifier(qualifier, apply_value);
        count++;
    }

    return count;
}

bool CMacroFunction_AddorSetContElement::s_AddSrcOrgMod(CBioSource& bsrc, int subtype, const string& value)
{
    bool added = false;
    CRef<COrgMod> orgmod(new COrgMod());
    orgmod->SetSubtype(subtype);
    orgmod->SetSubname(value);
    if (!bsrc.IsSetOrgname()) {
        CRef<COrgName> orgname(new COrgName());
        orgname->SetMod().push_back(orgmod);
        bsrc.SetOrg().SetOrgname(*orgname);
        added = true;
    }
    else {
        bsrc.SetOrg().SetOrgname().SetMod().push_back(orgmod);
        added = true;
    }
    return added;
}

bool CMacroFunction_AddorSetContElement::s_AddSrcSubSource(CBioSource& bsrc, int subtype, const string& value)
{
    bool added = false;
    CRef<CSubSource> sub_src(new CSubSource());
    sub_src->SetSubtype(subtype);
    sub_src->SetName(value);
    bsrc.SetSubtype().push_back(sub_src);
    added = true;

    return added;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SatelliteFields
/// SATELLITE_NAME()
/// SATELLITE_TYPE()
/// 
DEFINE_MACRO_FUNCNAME(CMacroFunction_SatelliteType, "SATELLITE_TYPE")
DEFINE_MACRO_FUNCNAME(CMacroFunction_SatelliteName, "SATELLITE_NAME")

void CMacroFunction_SatelliteFields::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    m_Result->SetNotSet();
    if (!feat || !scope || !feat->IsSetQual())
        return;

    const string& qual_value = feat->GetNamedQual("satellite");
    if (!qual_value.empty()) {
        string value;
        switch (m_FieldType) {
        case eSatelliteType:
            value = s_GetSatelliteType(qual_value);
            break;
        case eSatelliteName:
            value = s_GetSatelliteName(qual_value);
            break;
        }

        if (m_Nested == eNotNested) { // return a string
            m_Result->SetString(value);
        }
        else {
            // return a reference to a CMQueryNodeValue of type string
            CRef<CMQueryNodeValue> new_node(new CMQueryNodeValue);
            new_node->SetString(value);
            m_Result->SetRef(new_node);
        }
    }
}

string CMacroFunction_SatelliteFields::s_GetSatelliteType(const string& value)
{
    SIZE_TYPE pos = NStr::FindNoCase(value, ":");
    if (pos != NPOS) {
        return value.substr(0, pos);
    }

    auto it = find(kSatelliteTypes.begin(), kSatelliteTypes.end(), value);
    return (it != kSatelliteTypes.end()) ? value : kEmptyStr;
}

string CMacroFunction_SatelliteFields::s_GetSatelliteName(const string& value)
{
    SIZE_TYPE pos = NStr::FindNoCase(value, ":");
    if (pos != NPOS) {
        return value.substr(pos + 1, NPOS);
    }

    auto it = find(kSatelliteTypes.begin(), kSatelliteTypes.end(), value);
    return (it != kSatelliteTypes.end()) ? kEmptyStr : value;
}

string CMacroFunction_SatelliteFields::s_MakeSatellite(const string& type, const string& name)
{
    string new_type = type;
    string new_name;
    if (!name.empty()) {
        new_name = ":" + name;
    }
    if (type.empty()) {
        new_type = kSatelliteTypes.front();
    }
    else {
        auto found_it = find(kSatelliteTypes.begin(), kSatelliteTypes.end(), type);
        bool good_type = (found_it != kSatelliteTypes.end()) ? true : false;

        if (!good_type) {
            new_type = kSatelliteTypes.front();
            new_name = ":" + type + new_name;
        }
    }
    return new_type + new_name;
}

bool CMacroFunction_SatelliteFields::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_MobileElementTypeFields
/// MOBILE_ELEMENTTYPE_NAME()
/// MOBILE_ELEMENTTYPE_TYPE()
/// 
DEFINE_MACRO_FUNCNAME(CMacroFunction_MobileElementType, "MOBILE_ELEMENTTYPE_TYPE")
DEFINE_MACRO_FUNCNAME(CMacroFunction_MobileElementName, "MOBILE_ELEMENTTYPE_NAME")

void CMacroFunction_MobileElementTypeFields::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    m_Result->SetNotSet();
    if (!feat || !scope || !feat->IsSetQual())
        return;

    const string& qual_value = feat->GetNamedQual(kMobileElementTQual);
    if (!qual_value.empty()) {
        string value;
        switch (m_FieldType) {
        case eMobileElemTType:
            value = s_GetMobileElemTType(qual_value);
            break;
        case eMobileElemTName:
            value = s_GetMobileElemTName(qual_value);
            break;
        }

        if (m_Nested == eNotNested) { // return a string
            m_Result->SetString(value);
        }
        else {
            // return a reference to a CMQueryNodeValue of type string
            CRef<CMQueryNodeValue> new_node(new CMQueryNodeValue);
            new_node->SetString(value);
            m_Result->SetRef(new_node);
        }
    }
}

string CMacroFunction_MobileElementTypeFields::s_GetMobileElemTType(const string& value)
{
    SIZE_TYPE pos = NStr::FindNoCase(value, ":");
    if (pos != NPOS) {
        string type = value.substr(0, pos);
        auto it = find(kMobileETypeTypes.begin(), kMobileETypeTypes.end(), type);
        return (it != kMobileETypeTypes.end()) ? type : kEmptyStr;
    }

    auto it = find(kMobileETypeTypes.begin(), kMobileETypeTypes.end(), value);
    return (it != kMobileETypeTypes.end()) ? value : kEmptyStr;
}

string CMacroFunction_MobileElementTypeFields::s_GetMobileElemTName(const string& value)
{
    SIZE_TYPE pos = NStr::FindNoCase(value, ":");
    if (pos != NPOS) {
        string type = value.substr(0, pos);
        string name = value.substr(pos + 1, NPOS);
        auto it = find(kMobileETypeTypes.begin(), kMobileETypeTypes.end(), type);
        return (it != kMobileETypeTypes.end()) ? name : value;
    }

    auto it = find(kMobileETypeTypes.begin(), kMobileETypeTypes.end(), value);
    return (it != kMobileETypeTypes.end()) ? kEmptyStr : value;
}

string CMacroFunction_MobileElementTypeFields::s_MakeMobileElementT(const string& type, const string& name)
{
    // don't add a default type
    string value;
    if (!type.empty()) {
        auto found_it = find(kMobileETypeTypes.begin(), kMobileETypeTypes.end(), type);
        bool good_type = (found_it != kMobileETypeTypes.end()) ? true : false;

        if (good_type) {
            if (!name.empty()) {
                value = type + ":" + name;
            }
            else {
                value = type;
            }
        }
    }
    else {
        value = name;
    }
    return value;
}

bool CMacroFunction_MobileElementTypeFields::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_EditSubfield
/// EditSubfield(obj, subfield, find_text, repl_text, location, case_sensitive, is_regex)
DEFINE_MACRO_FUNCNAME(CMacroFunction_EditSubfield, "EditSubfield")
    void CMacroFunction_EditSubfield::TheFunction()
{
    size_t index = 1;
    const string& subfield = m_Args[1]->GetString();
    if (subfield.empty())
        NCBI_THROW(CException, eUnknown, "  Empty subfield is specified");

    const string& find_txt = NMacroUtil::GetStringValue(m_Args[++index]);
    string repl_txt = NMacroUtil::GetStringValue(m_Args[++index]);
    const string& location = m_Args[++index]->GetString();
    bool case_sensitive = m_Args[++index]->GetBool();
    bool is_regex = (++index < m_Args.size()) ? m_Args[index]->GetBool() : false;

    CMQueryNodeValue::TObs objs;
    if (m_Args[0]->AreObjects()) {
        objs = m_Args[0]->GetObjects();
    }
    else if (m_Args[0]->IsRef()) {
        x_GetObjectsFromRef(objs, 0);
    }
    if (objs.empty()) {
        return;
    }
    
    auto loc = CMacroFunction_EditStringQual::s_GetLocFromName(location);
    for (auto&& it : objs) {
        CObjectInfo value_oi = it.field;
        _ASSERT(value_oi.GetPrimitiveValueType() == ePrimitiveValueString);

        string orig_value = value_oi.GetPrimitiveValueString();
        string newValue;
        if (NMacroUtil::IsSatelliteSubfield(subfield)) {
            string sat_type = CMacroFunction_SatelliteFields::s_GetSatelliteType(orig_value);
            string sat_name = CMacroFunction_SatelliteFields::s_GetSatelliteName(orig_value);
            if (subfield == kSatelliteName) {
                if (CMacroFunction_EditStringQual::s_EditText(sat_name, find_txt, repl_txt, loc, case_sensitive, is_regex)) {
                    newValue = CMacroFunction_SatelliteFields::s_MakeSatellite(sat_type, sat_name);
                    SetQualStringValue(value_oi, newValue);
                }
            }
            else if (subfield == kSatelliteType) {
                string orig_sat_type = sat_type;
                if (CMacroFunction_EditStringQual::s_EditText(sat_type, find_txt, repl_txt, loc, case_sensitive, is_regex)) {
                    auto found_it = find(kSatelliteTypes.begin(), kSatelliteTypes.end(), sat_type);
                    bool good_type = (found_it != kSatelliteTypes.end()) ? true : false;

                    if (good_type) {
                        newValue = CMacroFunction_SatelliteFields::s_MakeSatellite(sat_type, sat_name);
                    }
                    else {
                        if (!sat_name.empty())
                            sat_name = sat_type + ":" + sat_name;
                        newValue = CMacroFunction_SatelliteFields::s_MakeSatellite(orig_sat_type, sat_name);
                    }
                    SetQualStringValue(value_oi, newValue);
                }
            }
        }
        else if (NMacroUtil::IsMobileElementTSubfield(subfield)) {
            string met_type = CMacroFunction_MobileElementTypeFields::s_GetMobileElemTType(orig_value);
            string met_name = CMacroFunction_MobileElementTypeFields::s_GetMobileElemTName(orig_value);
            if (subfield == kMobileElementTName) {
                if (CMacroFunction_EditStringQual::s_EditText(met_name, find_txt, repl_txt, loc, case_sensitive, is_regex)) {
                    newValue = CMacroFunction_MobileElementTypeFields::s_MakeMobileElementT(met_type, met_name);
                    SetQualStringValue(value_oi, newValue);
                }
            }
            else if (subfield == kMobileElementTType) {
                string orig_met_type = met_type;
                if (CMacroFunction_EditStringQual::s_EditText(met_type, find_txt, repl_txt, loc, case_sensitive, is_regex)) {
                    auto found_it = find(kMobileETypeTypes.begin(), kMobileETypeTypes.end(), met_type);
                    bool good_type = (found_it != kMobileETypeTypes.end()) ? true : false;

                    if (good_type) {
                        newValue = CMacroFunction_MobileElementTypeFields::s_MakeMobileElementT(met_type, met_name);
                    }
                    else {
                        newValue = CMacroFunction_MobileElementTypeFields::s_MakeMobileElementT(orig_met_type, met_name);
                    }
                    SetQualStringValue(value_oi, newValue);
                }
            }
        } else {
            auto part = NMacroUtil::GetSVPartFromString(subfield);
            if (part != NMacroUtil::eSV_Error) {
                string inst, coll, id;
                COrgMod::ParseStructuredVoucher(orig_value, inst, coll, id);

                switch (part) {
                case NMacroUtil::eSV_Inst:
                    CMacroFunction_EditStringQual::s_EditText(inst, find_txt, repl_txt, loc, case_sensitive, is_regex);
                    break;
                case NMacroUtil::eSV_Coll:
                    CMacroFunction_EditStringQual::s_EditText(coll, find_txt, repl_txt, loc, case_sensitive, is_regex);
                    break;
                case NMacroUtil::eSV_Specid:
                    CMacroFunction_EditStringQual::s_EditText(id, find_txt, repl_txt, loc, case_sensitive, is_regex);
                    break;
                default: break;
                }
                newValue = COrgMod::MakeStructuredVoucher(inst, coll, id);
                SetQualStringValue(value_oi, newValue);
            }
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": edited " << m_QualsChangedCount << " qualifiers, replaced " << find_txt;
        if (NStr::IsBlank(repl_txt)) {
            repl_txt.assign("''");
        }
        log << " with " << repl_txt;
        x_LogFunction(log);
    }
}

bool CMacroFunction_EditSubfield::x_ValidArguments() const
{
    if (m_Args.size() != 7) 
        return false;
    
    bool first_ok = m_Args[0]->AreObjects() || m_Args[0]->IsRef();
    if (!first_ok || !m_Args[1]->IsString()) {
        return false;
    }

    for (size_t index = 2; index < 4; index++) {
        if (!m_Args[index]->IsString() && !m_Args[index]->IsInt()) {
            return false;
        }
    }
    return (m_Args[4]->IsString() && m_Args[5]->IsBool() && m_Args[6]->IsBool());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveSubfield
/// RemoveSubField(object, field_name)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveSubfield, "RemoveSubField")

void CMacroFunction_RemoveSubfield::TheFunction()
{
    const string& subfield = m_Args[1]->GetString();
    if (subfield.empty())
        NCBI_THROW(CException, eUnknown, "  Empty subfield is specified");

    CMQueryNodeValue::TObs objs;
    if (m_Args[0]->AreObjects()) {
        objs = m_Args[0]->GetObjects();
    }
    else if (m_Args[0]->IsRef()) {
        x_GetObjectsFromRef(objs, 0);
    }
    if (objs.empty()) {
        return;
    }

    for (auto&& it : objs) {
        CObjectInfo res_oi = it.field;
        _ASSERT(res_oi.GetTypeFamily() == eTypeFamilyClass);

        TMemberIndex index = 2;
        CObjectInfo value_oi = res_oi.GetClassMemberIterator(index).GetMember();
        _ASSERT(value_oi.GetPrimitiveValueType() == ePrimitiveValueString);

        string newValue;
        if (subfield == kSatelliteName) {
            newValue = CMacroFunction_SatelliteFields::s_GetSatelliteType(value_oi.GetPrimitiveValueString());
        }
        else if (subfield == kSatelliteType) {
            newValue = CMacroFunction_SatelliteFields::s_GetSatelliteName(value_oi.GetPrimitiveValueString());
        }
        else if (subfield == kMobileElementTName) {
            newValue = CMacroFunction_MobileElementTypeFields::s_GetMobileElemTType(value_oi.GetPrimitiveValueString());
        }
        else if (subfield == kMobileElementTType) {
            newValue = CMacroFunction_MobileElementTypeFields::s_GetMobileElemTName(value_oi.GetPrimitiveValueString());
        }
        else {
            auto part = NMacroUtil::GetSVPartFromString(subfield);
            if (part != NMacroUtil::eSV_Error) {
                string inst, coll, id;
                COrgMod::ParseStructuredVoucher(value_oi.GetPrimitiveValueString(), inst, coll, id);

                switch (part) {
                case NMacroUtil::eSV_Inst: inst.clear();  break;
                case NMacroUtil::eSV_Coll: coll.clear();  break;
                case NMacroUtil::eSV_Specid: id.clear(); break;
                default: break;
                }
                newValue = COrgMod::MakeStructuredVoucher(inst, coll, id);
            }
        }

        if (!newValue.empty()) {
            SetQualStringValue(value_oi, newValue);
        }
        else {
            // remove the object
            if (RemoveFieldByName(it)) {
                m_QualsChangedCount++;
            }
        }
    }

    if (m_QualsChangedCount) {
        CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
        CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CCleanup cleanup;
        if (dynamic_cast<const CSeq_feat*>(obj.GetPointer())) {
            CSeq_feat* seq_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
            cleanup.BasicCleanup(*seq_feat);
        }
        else if (dynamic_cast<const CBioSource*>(obj.GetPointer())) {
            CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
            cleanup.BasicCleanup(*bsrc);
        }

        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " qualifiers";
        x_LogFunction(log);
    }
}


bool CMacroFunction_RemoveSubfield::x_ValidArguments() const
{
    return m_Args.size() == 2 && (m_Args[0]->AreObjects() || m_Args[0]->IsRef()) && m_Args[1]->IsString();
}

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_SetCodonsRecognized
/// SetCodonsRecognized(field_name)
/// The field_name specifies one of the primer sequences
///

static bool s_IsATGC(char ch)
{
    if (ch == 'A' || ch == 'T' || ch == 'G' || ch == 'C' || ch == 'U') {
        return true;
    }
    else {
        return false;
    }
}


static const  string kAmbiguities = "MRSVWYHKDBN";
static const string kReplacements[] = {
    "AC", "AG", "CG", "ACG", "AT", "CT", "ACT", "GT", "AGT", "CGT", "ACGT" };

static const string s_GetExpansion(const string& ch)
{
    size_t pos = NStr::Find(kAmbiguities, ch);
    if (pos != string::npos) {
        return kReplacements[pos];
    }
    else {
        return ch;
    }
}


static vector<string> ParseDegenerateCodons(string codon)
{
    vector<string> replacements;

    if (codon.length() == 3 && s_IsATGC(codon.c_str()[0])) {
        string this_codon = codon.substr(0, 1);
        replacements.push_back(this_codon);

        for (int i = 1; i < 3; i++) {
            string ch = s_GetExpansion(codon.substr(i, 1));
            size_t num_now = replacements.size();
            // add copies for each expansion letter beyond the first
            for (unsigned int j = 1; j < ch.length(); j++) {
                for (unsigned int k = 0; k < num_now; k++) {
                    string cpy = replacements[k];
                    replacements.push_back(cpy);
                }
            }
            for (unsigned int k = 0; k < num_now; k++) {
                for (unsigned int j = 0; j < ch.length(); j++) {
                    replacements[j * num_now + k].append(ch.substr(j, 1));
                }
            }
        }
    }
    else {
        replacements.push_back(codon);
    }
    return replacements;
}

DEFINE_MACRO_FUNCNAME(CMacroFunction_SetCodonsRecognized, "SetCodonsRecognized")
void CMacroFunction_SetCodonsRecognized::TheFunction()
{
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!scope) {
        return;
    }

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    if (!edit_feat) {
        return;
    }

    CMQueryNodeValue& parsed_obj = m_Args[0].GetNCObject();
    parsed_obj.Dereference();
    if (parsed_obj.GetDataType() != CMQueryNodeValue::eString) {
        return;
    }
    string value = parsed_obj.GetString();
    if (value.empty()) {
        return;
    }

    if (edit_feat->IsSetData() && edit_feat->GetData().IsRna()
        && edit_feat->GetData().GetRna().IsSetType()
        && edit_feat->GetData().GetRna().GetType() == CRNA_ref::eType_tRNA)
    {
        edit_feat->SetData().SetRna().SetExt().SetTRNA().ResetCodon();

        NStr::TruncateSpacesInPlace(value);
        NStr::ToUpper(value);
        if (value.empty())
            return;

        vector<string> codons = ParseDegenerateCodons(value);
        for (unsigned int j = 0; j < codons.size(); j++)
        {
            int val = CGen_code_table::CodonToIndex(codons[j]);
            if (val > -1)
            {
                edit_feat->SetData().SetRna().SetExt().SetTRNA().SetCodon().push_back(val);
            }
        }

        m_DataIter->SetModified();

        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": set codons-recognized";
        x_LogFunction(log);
    }
}

bool CMacroFunction_SetCodonsRecognized::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eRef);
}



///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetStructCommField
/// SetStructCommField([obj_to_modify,] field_name, field_value, existing_text, delimiter)
/// The last parameter is optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetStructCommField, "SetStructCommField")
void CMacroFunction_SetStructCommField::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(object.GetPointer());
    const CUser_object* user = dynamic_cast<const CUser_object*>(object.GetPointer());
    if (!(bioseq && bioseq->IsNa()) && 
        !(user && CComment_rule::IsStructuredComment(*user))) {
        return;
    }

    size_t index = 1;
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    if (type == CMQueryNodeValue::eString) {
        --index;
    }
    const string& field_name = m_Args[index]->GetString();
    const string& field_value = m_Args[++index]->GetString();
    const string& existing_text_option = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);

    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(existing_text_option, delimiter);
    if (!field_value.empty()) {
        if (type == CMQueryNodeValue::eString && user) {
            CObjectInfo oi = m_DataIter->GetEditedObject();
            CUser_object* user_object = CTypeConverter<CUser_object>::SafeCast(oi.GetObjectPtr());
            if (existing_text == edit::eExistingText_add_qual) {
                // add a new field to the structured comment
                user_object->AddField(field_name, field_value);
                m_QualsChangedCount++;
            }
            else {
                if (user_object->HasField(field_name)) {
                    CUser_field& user_field = user_object->SetField(field_name);
                    if (user_field.IsSetData() && user_field.GetData().IsStr()) {
                        string orig_val = user_field.GetData().GetStr();
                        if (AddValueToString(orig_val, field_value, existing_text)) {
                            user_field.SetData().SetStr(orig_val);
                            m_QualsChangedCount++;
                        }
                    }
                }
                else {
                    // add a new field to the structured comment
                    user_object->AddField(field_name, field_value);
                    m_QualsChangedCount++;
                }
            }

            if (m_QualsChangedCount) {
                CMacroFunction_SetStructCommDb::s_MoveSuffixToTheEnd(*user_object);
                CCleanup::CleanupUserObject(*user_object);
            }
        }
        else if (bioseq) { // for creating new structured comment for the nucleotide sequence
            // if there is an object
            CMQueryNodeValue::TObs res_oi = m_Args[0]->GetObjects();
            if (res_oi.empty() || existing_text == edit::eExistingText_leave_old) {
                return;
            }

            CObjectInfo oi = m_DataIter->GetEditedObject();
            CBioseq* edit_bseq = CTypeConverter<CBioseq>::SafeCast(oi.GetObjectPtr());
            if (!edit_bseq) return;

            CSeqdesc* seqdesc = CTypeConverter<CSeqdesc>::SafeCast(res_oi.front().field.GetObjectPtr());
            if (!seqdesc) return;

            if (seqdesc->GetUser().HasField(field_name)) {
                CUser_field& user_field = seqdesc->SetUser().SetField(field_name);
                if (user_field.IsSetData()) {
                    if (user_field.GetData().IsStr()) {
                        string orig_val = user_field.GetData().GetStr();
                        if (AddValueToString(orig_val, field_value, existing_text)) {
                            user_field.SetData().SetStr(orig_val);
                            m_QualsChangedCount++;
                        }
                    }
                    else if (user_field.GetData().Which() == CUser_field::TData::e_not_set) {
                        user_field.SetData().SetStr(field_value);
                        m_QualsChangedCount++;
                    }
                }
            }
            else {
                seqdesc->SetUser().AddField(field_name, field_value);
                m_QualsChangedCount++;
            }

            if (m_QualsChangedCount) {
                CMacroFunction_SetStructCommDb::s_MoveSuffixToTheEnd(seqdesc->SetUser());
                CCleanup::CleanupUserObject(seqdesc->SetUser());
            }
        }

        if (m_QualsChangedCount) {
            m_DataIter->SetModified();
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr() << ": applied ('" << field_name << "', '" << field_value << "') field-value pair";
            x_LogFunction(log);
        }
    }
    else if (remove_field) {
        if (type == CMQueryNodeValue::eString && user) {
            CObjectInfo oi = m_DataIter->GetEditedObject();
            CUser_object* user_object = CTypeConverter<CUser_object>::SafeCast(oi.GetObjectPtr());
            if (user_object && user_object->HasField(field_name) && user_object->RemoveNamedField(field_name)) {
                m_QualsChangedCount++;
            }
        }
        else if (bioseq) {
            CMQueryNodeValue::TObs res_oi = m_Args[0]->GetObjects();
            if (res_oi.empty() || existing_text == edit::eExistingText_leave_old) {
                return;
            }

            CObjectInfo oi = m_DataIter->GetEditedObject();
            CBioseq* edit_bseq = CTypeConverter<CBioseq>::SafeCast(oi.GetObjectPtr());
            if (!edit_bseq) return;

            CObjectInfo desc_oi = res_oi.front().field;
            CSeqdesc* seqdesc = CTypeConverter<CSeqdesc>::SafeCast(desc_oi.GetObjectPtr());
            if (!seqdesc) return;

            if (seqdesc->GetUser().HasField(field_name) && seqdesc->SetUser().RemoveNamedField(field_name)) {
                m_QualsChangedCount++;
            }
        }

        if (m_QualsChangedCount) {
            m_DataIter->SetModified();
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " structured comment fields";
            x_LogFunction(log);
        }
    }
}

bool CMacroFunction_SetStructCommField::x_ValidArguments() const
{
    if (m_Args.size() < 3 || m_Args.size() > 6)
        return false;

    if (m_Args[0]->IsRef()) {
        CMQueryNodeValue& objs = m_Args[0].GetNCObject();
        objs.Dereference();
        if (!objs.AreObjects() && !objs.IsNotSet())
            return false;
    }
    else if (!m_Args[0]->IsString()) {
        return false;
    }

    NMacroUtil::GetPrimitiveFromRef(m_Args[1].GetNCObject());
    for (size_t i = 1; i < m_Args.size(); ++i) {
        if (!m_Args[i]->IsString() && !m_Args[i]->IsBool()) {
            return false;
        }
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SetStructCommDb
/// SetStructCommDb(db_name, existing_text, delimiter, remove_blank)
/// The last two parameters are optional
DEFINE_MACRO_FUNCNAME(CMacroFunction_SetStructCommDb, "SetStructCommDb")

void CMacroFunction_SetStructCommDb::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(object.GetPointer());
    const CUser_object* user = dynamic_cast<const CUser_object*>(object.GetPointer());
    if (!(bioseq && bioseq->IsNa()) &&
        !(user && CComment_rule::IsStructuredComment(*user))) {
        return;
    }

    size_t index = 0;
    const string& db_name = m_Args[index]->GetString();
    const string kPrefix = "StructuredCommentPrefix";
    const string kSuffix = "StructuredCommentSuffix";
    const string& existing_text_option = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(existing_text_option, delimiter);

    if (!db_name.empty()) {
        CNcbiOstrstream log;
        if (bioseq) {
            CRef<CCmdComposite> cmd;
            if (existing_text == edit::eExistingText_add_qual) {
                if (x_MakeNewStructuredComment(db_name, kPrefix, kSuffix)) {
                    m_DataIter->SetModified();
                }
            }
            else {
                bool found_user = false;
                for (CSeqdesc_CI desc_it(m_DataIter->GetBioseqHandle(), CSeqdesc::e_User); desc_it; ++desc_it) {
                    if (CComment_rule::IsStructuredComment(desc_it->GetUser())) {
                        CRef<CSeqdesc> new_seqdesc(new CSeqdesc());
                        new_seqdesc->Assign(*desc_it);
                        m_QualsChangedCount += s_UpdateStructCommentDb(*new_seqdesc, kPrefix, db_name, existing_text);
                        m_QualsChangedCount += s_UpdateStructCommentDb(*new_seqdesc, kSuffix, db_name, existing_text);

                        if (m_QualsChangedCount) {
                            found_user = true;
                            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_it.GetSeq_entry_Handle(), *desc_it, *new_seqdesc));
                            cmd.Reset(new CCmdComposite("Update structured comment"));
                            cmd->AddCommand(*ecmd);
                            m_DataIter->RunCommand(cmd, m_CmdComposite);
                        }
                    }
                }
                if (!found_user) {
                    if (x_MakeNewStructuredComment(db_name, kPrefix, kSuffix)) {
                        m_DataIter->SetModified();
                    }
                }
            }
        }
        else if (user) {
            CObjectInfo oi = m_DataIter->GetEditedObject();
            CUser_object* user_object = CTypeConverter<CUser_object>::SafeCast(oi.GetObjectPtr());
            m_QualsChangedCount += s_UpdateStructCommentDb(*user_object, kPrefix, db_name, existing_text);
            m_QualsChangedCount += s_UpdateStructCommentDb(*user_object, kSuffix, db_name, existing_text);
            m_DataIter->SetModified();
        }
        if (m_QualsChangedCount) {
            log << m_DataIter->GetBestDescr() << ": applied structured comment database name '" << db_name << "'";
        }
        x_LogFunction(log);
    }
    else if (remove_field) {
        if (user) {
            CObjectInfo oi = m_DataIter->GetEditedObject();
            CUser_object* user_object = CTypeConverter<CUser_object>::SafeCast(oi.GetObjectPtr());
            if (user_object) {
                if (user_object->HasField(kPrefix) && user_object->RemoveNamedField(kPrefix)) {
                    m_QualsChangedCount++;
                    m_DataIter->SetModified();
                }
                if (user_object->HasField(kSuffix) && user_object->RemoveNamedField(kSuffix)) {
                    m_QualsChangedCount++;
                    m_DataIter->SetModified();
                }
            }
        }
        else if (bioseq) {
            CMQueryNodeValue::TObs res_oi = m_Args[0]->GetObjects();
            if (res_oi.empty() || existing_text == edit::eExistingText_leave_old) {
                return;
            }

            CRef<CCmdComposite> cmd;
            for (CSeqdesc_CI desc_it(m_DataIter->GetBioseqHandle(), CSeqdesc::e_User); desc_it; ++desc_it) {
                if (CComment_rule::IsStructuredComment(desc_it->GetUser())) {
                    CRef<CSeqdesc> new_seqdesc(new CSeqdesc());
                    new_seqdesc->Assign(*desc_it);
                    if (new_seqdesc->GetUser().HasField(kPrefix) && new_seqdesc->SetUser().RemoveNamedField(kPrefix))
                        m_QualsChangedCount++;
                    if (new_seqdesc->GetUser().HasField(kSuffix) && new_seqdesc->SetUser().RemoveNamedField(kSuffix))
                        m_QualsChangedCount++;
                    
                    if (m_QualsChangedCount) {
                        CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_it.GetSeq_entry_Handle(), *desc_it, *new_seqdesc));
                        cmd.Reset(new CCmdComposite("Update structured comment"));
                        cmd->AddCommand(*ecmd);
                        m_DataIter->RunCommand(cmd, m_CmdComposite);
                    }
                }
            }
        }

        if (m_QualsChangedCount) {
            CNcbiOstrstream log;
            log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " structured comment fields";
            x_LogFunction(log);
        }
    }
}

bool CMacroFunction_SetStructCommDb::x_MakeNewStructuredComment(const string& db_name, const string& kPrefix, const string& kSuffix)
{
    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    CRef<CUser_object> user_obj(new CUser_object);
    user_obj->SetObjectType(CUser_object::eObjectType_StructuredComment);
    user_obj->AddField(kPrefix, CComment_rule::MakePrefixFromRoot(db_name));
    user_obj->AddField(kSuffix, CComment_rule::MakeSuffixFromRoot(db_name));
    seqdesc->SetUser(*user_obj);

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CBioseq* edit_bseq = CTypeConverter<CBioseq>::SafeCast(oi.GetObjectPtr());
    if (!edit_bseq) return false;

    auto& descriptors = edit_bseq->SetDescr().Set();
    descriptors.push_back(seqdesc);
    m_QualsChangedCount++;

    CObjectInfo desc_oi(seqdesc, seqdesc->GetThisTypeInfo());
    CMQueryNodeValue::TObs objs;
    objs.push_back(CMQueryNodeValue::SResolvedField(oi, desc_oi));
    m_Result->SetObjects(objs);

    return true;
}

bool CMacroFunction_SetStructCommDb::x_ValidArguments() const
{
    if (m_Args.size() < 2 || m_Args.size() > 4)
        return false;

    NMacroUtil::GetPrimitiveFromRef(m_Args[0].GetNCObject());
    if (!m_Args[0]->IsString() || !m_Args[1]->IsString()) {
        return false;
    }
    size_t index = 2;
    if (m_Args.size() > 2 && (!m_Args[index]->IsString() && !m_Args[index]->IsBool()))
        return false;
    if (m_Args.size() > 3 && !m_Args[++index]->IsBool())
        return false;

    return true;
}

int CMacroFunction_SetStructCommDb::s_UpdateStructCommentDb(CSeqdesc& seqdesc, const string& field, const string& db_name, edit::EExistingText existing_text)
{
    if (!seqdesc.IsUser()) {
        return 0;
    }

    return s_UpdateStructCommentDb(seqdesc.SetUser(), field, db_name, existing_text);
}

int CMacroFunction_SetStructCommDb::s_UpdateStructCommentDb(CUser_object& user, const string& field, const string& db_name, edit::EExistingText existing_text)
{
    int count = 0;
    bool field_is_prefix = NStr::EndsWith(field, "Prefix") ? true : false;
    string root = (field_is_prefix) ? CComment_rule::MakePrefixFromRoot(db_name) : CComment_rule::MakeSuffixFromRoot(db_name);

    if (user.HasField(field)) {
        CUser_field& user_field = user.SetField(field);
        if (user_field.IsSetData()) {
            if (user_field.GetData().IsStr()) {
                string orig_val = user_field.GetData().GetStr();
                CComment_rule::NormalizePrefix(orig_val);
                if (AddValueToString(orig_val, db_name, existing_text)) {
                    orig_val = (field_is_prefix) ? CComment_rule::MakePrefixFromRoot(orig_val) : CComment_rule::MakeSuffixFromRoot(orig_val);
                    user_field.SetData().SetStr(orig_val);
                    count++;
                }
            }
            else if (user_field.GetData().Which() == CUser_field::TData::e_not_set) {
                user_field.SetData().SetStr(root);
                count++;
            }
        }
    }
    else {
        user.AddField(field, root);
        count++;
    }
    return count;
}

void CMacroFunction_SetStructCommDb::s_MoveSuffixToTheEnd(CUser_object& user_object)
{
    if (!user_object.IsSetData()) return;
    auto& user_fields = user_object.SetData();

    for (CUser_object::TData::iterator it = user_fields.begin(); it != user_fields.end(); ++it) {
        CUser_field& field = it->GetNCObject();
        if (field.IsSetLabel() && field.GetLabel().IsStr()) {
            if (NStr::Equal(field.GetLabel().GetStr(), "StructuredCommentSuffix") && it != user_fields.end() - 1) {
                swap(*it, *(user_fields.end() - 1));
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ValueFromTable
/// ValueFromTable(table_name, col_number(integer))
/// col_number - starts from 1
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ValueFromTable, "ValueFromTable");
void CMacroFunction_ValueFromTable::TheFunction()
{
    const string& table_name = m_Args[0]->GetString();
    int col = (int)m_Args[1]->GetInt();
    if (col <= 0) {
        NCBI_THROW(CException, eUnknown, "The third argument (column number) should be at least 1");
    }

    m_Result->SetNotSet();
    if (NStr::EqualCase(m_DataIter->GetTableName(), table_name)) {
        string value = m_DataIter->GetValueFromTable(col - 1);
        m_Result->SetString(value);
    }
}

bool CMacroFunction_ValueFromTable::x_ValidArguments() const
{
    if (m_Args.size() != 2)
        return false;

    return (m_Args[0]->GetDataType() == CMQueryNodeValue::eString
        && m_Args[1]->GetDataType() == CMQueryNodeValue::eInt);
}


///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_GeneQual
/// GeneQual(gene_field_name);
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_GeneQual, "GeneQual");
void CMacroFunction_GeneQual::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CSeq_feat* cfeat = dynamic_cast<const CSeq_feat*>(object.GetPointer());
    if (!cfeat)
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    string field_name = m_Args[0]->GetString();
    const bool is_locustag = NMacroUtil::StringsAreEquivalent(field_name, "locus_tag");


    if (cfeat->IsSetData() && cfeat->GetData().IsGene()) {
        ResolveIdentToObjects(oi, field_name, *m_Result);
    }
    else {
        CSeq_feat* feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
        _ASSERT(feat);
        bool found_qual = false;
        if (feat->IsSetQual()) {  // locus_tag
            CMQueryNodeValue tmp_result;
            ResolveIdentToObjects(oi, "qual", tmp_result);
            if (tmp_result.GetDataType() == CMQueryNodeValue::eNotSet) {
                return;
            }
            else {
                m_Result->SetDataType(CMQueryNodeValue::eObjects);
                if (is_locustag) {
                    NStr::ReplaceInPlace(field_name, "-", "_");
                }
                for (auto&& it : tmp_result.SetObjects()) {
                    CObjectInfo qual_oi = it.field;
                    auto type_oi = qual_oi.FindClassMember("qual").GetMember();
                    if (NStr::EqualNocase(type_oi.GetPrimitiveValueString(), field_name)) {
                        m_Result->SetObjects().push_back(it);
                        found_qual = true;
                    }
                }
            }
        }
        if (!found_qual && feat->IsSetXref()) {  // locus-tag
            CGene_ref* gene_xref = nullptr;
            for (auto&& it : feat->SetXref()) {
                if (it->IsSetData() && it->GetData().IsGene()) {
                    gene_xref = &(it->SetData().SetGene());
                    break;
                }
            }
            if (gene_xref) {
                if (is_locustag)
                    NStr::ReplaceInPlace(field_name, "_", "-");
                CObjectInfo gene_oi(gene_xref, gene_xref->GetThisTypeInfo());
                ResolveIdentToObjects(gene_oi, field_name, *m_Result);
            }
        }
    }
    if (m_Result->GetDataType() == CMQueryNodeValue::eObjects && 
        m_Result->GetObjects().empty()) {
        m_Result->SetNotSet();
    }
}

bool CMacroFunction_GeneQual::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eString);
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ApplyStrucVoucherPart
/// ApplyStructVoucherPart(obj/voucher_type, part_name, newValue, existing_text, delimiter, remove_blank)
/// Empty new values don't have any effect by default. If remove_blank is True,
/// the existing field is removed.
/// The last two parameters are optional

DEFINE_MACRO_FUNCNAME(CMacroFunction_ApplyStrucVoucherPart, "ApplyStructVoucherPart")
void CMacroFunction_ApplyStrucVoucherPart::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioSource* cbsrc = dynamic_cast<const CBioSource*>(obj.GetPointer());
    if (!cbsrc)
        return;

    size_t index = 1;
    string voucher_type;
    string voucher_part = m_Args[index]->GetString();
    string newValue = NMacroUtil::GetStringValue(m_Args[++index]);
    const string& action_type = m_Args[++index]->GetString();
    string delimiter;
    bool remove_field = false;
    x_GetOptionalArgs(delimiter, remove_field, index);
    
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs objs;

    if (m_Args[0]->IsString()) {
        voucher_type = m_Args[0]->GetString();
    }
    else if (m_Args[0]->AreObjects()) {
        objs = m_Args[0]->GetObjects();
    }
    else if (m_Args[0]->IsRef()) {
        x_GetObjectsFromRef(objs, 0);
    }

    if (voucher_type.empty() && objs.empty()) {
        return;
    }
    auto part = NMacroUtil::GetSVPartFromString(voucher_part);
    if (part == NMacroUtil::eSV_Error)
        NCBI_THROW(CException, eUnknown, "  Unrecognized name of structured voucher part: " + voucher_part);


    CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
    edit::EExistingText existing_text = NMacroUtil::ActionTypeToExistingTextOption(action_type, delimiter);
    if (!voucher_type.empty()) {
        auto subtype = COrgMod::GetSubtypeValue(voucher_type);
        if (subtype != COrgMod::eSubtype_bio_material &&
            subtype != COrgMod::eSubtype_culture_collection &&
            subtype != COrgMod::eSubtype_specimen_voucher)
            NCBI_THROW(CException, eUnknown, "Invalid voucher type specified: " + voucher_type);

        if (!newValue.empty()) {
            if (existing_text != edit::eExistingText_add_qual) {
                bool has_voucher = false;
                EDIT_EACH_ORGMOD_ON_BIOSOURCE(mod_it, *bsrc) {
                    auto orgmod = *mod_it;
                    if (orgmod->IsSetSubtype() && orgmod->GetSubtype() == subtype) {
                        x_ApplyPart(*orgmod, part, newValue, existing_text);
                        has_voucher = true;
                    }
                }

                if (!has_voucher) {
                    auto new_mod = x_AddNewQual(subtype, part, newValue);
                    bsrc->SetOrg().SetOrgname().SetMod().push_back(new_mod);
                    m_QualsChangedCount++;
                }
            }
            else {
                auto new_mod = x_AddNewQual(subtype, part, newValue);
                bsrc->SetOrg().SetOrgname().SetMod().push_back(new_mod);
                m_QualsChangedCount++;
            }
        }
        else if (remove_field) {
            EDIT_EACH_ORGMOD_ON_BIOSOURCE(mod_it, *bsrc) {
                auto orgmod = *mod_it;
                if (orgmod->IsSetSubtype() && orgmod->GetSubtype() == subtype) {
                    x_RemovePart(*orgmod, part);
                    m_QualsChangedCount++;
                    if (orgmod->GetSubname().empty())
                        ERASE_ORGMOD_ON_BIOSOURCE(mod_it, *bsrc);
                }
            }
            if (bsrc->GetOrg().GetOrgname().GetMod().empty()) {
                bsrc->SetOrg().SetOrgname().ResetMod();
            }
        }
    }
    else {
        for (auto&& it : objs) {
            CObjectInfo res_oi = it.field;
            _ASSERT(res_oi.GetTypeFamily() == eTypeFamilyClass);

            COrgMod* mod = CTypeConverter<COrgMod>::SafeCast(res_oi.GetObjectPtr());
            if (!mod) continue;

            if (!newValue.empty()) {
                if (existing_text != edit::eExistingText_add_qual) {
                    x_ApplyPart(*mod, part, newValue, existing_text);
                }
                else {
                    auto new_mod = x_AddNewQual(mod->GetSubtype(), part, newValue);
                    bsrc->SetOrg().SetOrgname().SetMod().push_back(new_mod);
                    m_QualsChangedCount++;
                }
            }
            else if (remove_field) {
                x_RemovePart(*mod, part);
                if (mod->GetSubname().empty() && RemoveFieldByName(it)) {
                    m_QualsChangedCount++;
                }
            }
        }
    }


    if (m_QualsChangedCount) {
        m_DataIter->SetModified();

        CNcbiOstrstream log;
        if (!newValue.empty()) {
            log << m_QualsChangedCount << ";setting new value to structured voucher " << voucher_part;
        }
        else if (remove_field) {
            log << m_DataIter->GetBestDescr() << ": removed " << m_QualsChangedCount << " qualifiers";
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_ApplyStrucVoucherPart::x_ValidArguments() const
{
    // 4 - delimiter (optional)
    // 5 - remove blank (optional)
    auto arg_nr = m_Args.size();
    if (arg_nr < 4 || arg_nr > 6)
        return false;

    bool first_ok = m_Args[0]->IsString() || m_Args[0]->AreObjects() || m_Args[0]->IsRef();
    if (!first_ok) return false;

    if (!m_Args[1]->IsString()) return false;

    NMacroUtil::GetPrimitiveFromRef(m_Args[2].GetNCObject());
    bool third_ok = m_Args[2]->IsString() || m_Args[2]->IsInt() || m_Args[2]->IsDouble();
    if (!third_ok) return false;

    if (!m_Args[3]->IsString()) return false;

    if (arg_nr > 4 && (!m_Args[4]->IsString() && !m_Args[4]->IsBool())) return false;

    if (arg_nr > 5 && !m_Args[5]->IsBool()) return false;

    return true;
}

void CMacroFunction_ApplyStrucVoucherPart::x_ApplyPart(
    COrgMod& orgmod, NMacroUtil::EStructVoucherPart part, 
    const string& newValue, edit::EExistingText existing_text)
{
    string orig_inst, orig_coll, orig_id;
    if (orgmod.IsSetSubname()) {
        COrgMod::ParseStructuredVoucher(orgmod.GetSubname(), orig_inst, orig_coll, orig_id);
    }

    auto update = [&newValue, existing_text, this](string& value, const string& orig_value) 
    {
        edit::AddValueToString(value, newValue, existing_text);
        if (!NStr::EqualCase(orig_value, value))
            m_QualsChangedCount++;
    };

    string inst{ orig_inst }, coll{ orig_coll }, id{ orig_id };
    switch (part) {
    case NMacroUtil::eSV_Inst: update(inst, orig_inst);  break;
    case NMacroUtil::eSV_Coll: update(coll, orig_coll);  break;
    case NMacroUtil::eSV_Specid: update(id, orig_id);  break;
    default: break;
    }

    orgmod.SetSubname(COrgMod::MakeStructuredVoucher(inst, coll, id));
}

void CMacroFunction_ApplyStrucVoucherPart::x_RemovePart(COrgMod& orgmod, NMacroUtil::EStructVoucherPart part)
{
    string inst, coll, id;
    if (orgmod.IsSetSubname()) {
        COrgMod::ParseStructuredVoucher(orgmod.GetSubname(), inst, coll, id);
    }

    switch (part) {
    case NMacroUtil::eSV_Inst: inst.clear();  break;
    case NMacroUtil::eSV_Coll: coll.clear();  break;
    case NMacroUtil::eSV_Specid: id.clear(); break;
    default: break;
    }

    orgmod.SetSubname(COrgMod::MakeStructuredVoucher(inst, coll, id));
}

CRef<COrgMod> CMacroFunction_ApplyStrucVoucherPart::x_AddNewQual(COrgMod::TSubtype subtype, NMacroUtil::EStructVoucherPart part, const string& newValue)
{
    CRef<COrgMod> new_mod(new COrgMod);
    new_mod->SetSubtype(subtype);
    string inst, coll, id;
    switch (part) {
    case NMacroUtil::eSV_Inst:  inst = newValue; break;
    case NMacroUtil::eSV_Coll: coll = newValue; break;
    case NMacroUtil::eSV_Specid: id = newValue; break;
    default: break;
    }
    new_mod->SetSubname(COrgMod::MakeStructuredVoucher(inst, coll, id));
    return new_mod;
}


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
