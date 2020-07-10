/*  $Id: macro_util.cpp 44753 2020-03-05 17:24:00Z asztalos $
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
 * File Description: Utility functions used in macros
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbimisc.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/taxon3/taxon3.hpp>
#include <objects/misc/sequence_macros.hpp>

#include <objtools/edit/source_edit.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>
#include <gui/objutils/macro_util.hpp>


/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

namespace NMacroUtil
{
    edit::EExistingText ActionTypeToExistingTextOption(const string& action_type, const string& delimiter)
    {
        edit::EExistingText exist_text = edit::eExistingText_cancel;

        if (NStr::EqualNocase(action_type, "eLeaveOld")) {
            exist_text = edit::eExistingText_leave_old;
        }
        else if (NStr::EqualNocase(action_type, "ePrepend")) {
            if (NStr::EqualNocase(delimiter, " "))
                exist_text = edit::eExistingText_prefix_space;
            else if (NStr::EqualNocase(delimiter, ";"))
                exist_text = edit::eExistingText_prefix_semi;
            else if (NStr::EqualNocase(delimiter, ","))
                exist_text = edit::eExistingText_prefix_comma;
            else if (NStr::EqualNocase(delimiter, ":"))
                exist_text = edit::eExistingText_prefix_colon;
            else
                exist_text = edit::eExistingText_prefix_none;
        }
        else if (NStr::EqualNocase(action_type, "eAppend")) {
            if (NStr::EqualNocase(delimiter, " "))
                exist_text = edit::eExistingText_append_space;
            else if (NStr::EqualNocase(delimiter, ";"))
                exist_text = edit::eExistingText_append_semi;
            else if (NStr::EqualNocase(delimiter, ","))
                exist_text = edit::eExistingText_append_comma;
            else if (NStr::EqualNocase(delimiter, ":"))
                exist_text = edit::eExistingText_append_colon;
            else
                exist_text = edit::eExistingText_append_none;
        }
        else if (NStr::EqualNocase(action_type, "eReplace")) {
            exist_text = edit::eExistingText_replace_old;
        }
        else if (NStr::EqualNocase(action_type, "eAddQual")) {
            exist_text = edit::eExistingText_add_qual;
        }
        
        return exist_text;
    }

    ECapChange ConvertStringtoCapitalOption(const string& cap_name)
    {
        ECapChange cap_change = eCapChange_none;

        if (NStr::EqualNocase(cap_name, "none")) {
            return cap_change;
        }

        if (NStr::EqualNocase(cap_name, "tolower")) {
            cap_change = eCapChange_tolower;
        }
        else if (NStr::EqualNocase(cap_name, "toupper")) {
            cap_change = eCapChange_toupper;
        }
        else if (NStr::EqualNocase(cap_name, "firstcap")) {
            cap_change = eCapChange_firstcap_restlower;
        }
        else if (NStr::EqualNocase(cap_name, "firstcap-restnochange")) {
            cap_change = eCapChange_firstcap_restnochange;
        }
        else if (NStr::EqualNocase(cap_name, "firstlower-restnochange")) {
            cap_change = eCapChange_firstlower_restnochange;
        }
        else if (NStr::EqualNocase(cap_name, "cap-word-space")) {
            cap_change = eCapChange_capword_afterspace;
        }
        else if (NStr::EqualNocase(cap_name, "cap-word-space-punct")) {
            cap_change = eCapChange_capword_afterspacepunc;
        }

        return cap_change;
    }

    CSeq_entry_Handle GetParentEntryForBioSource(const CBioSource& bsrc, const CSeq_entry_Handle& tse)
    {
        for (CSeq_entry_CI entry_ci(tse, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry); entry_ci; ++entry_ci) {
            if (entry_ci->IsSetDescr()) {
                ITERATE(CBioseq::TDescr::Tdata, dit, entry_ci->GetDescr().Get()) {
                    if ((*dit)->IsSource()) {
                        const CBioSource& desc_src = (*dit)->GetSource();
                        if (&desc_src == &bsrc) {
                            return *entry_ci;
                        }
                    }
                }
            }
            if (entry_ci->IsSeq()) {
                for (CFeat_CI fi(entry_ci->GetSeq(), SAnnotSelector(CSeqFeatData::e_Biosrc)); fi; ++fi) {
                    if (&(fi->GetData().GetBiosrc()) == &bsrc) {
                        return *entry_ci;
                    }
                }
            }
        }
        return CSeq_entry_Handle();
    }

    CConstRef<CBioseq> GetBioseqForSeqdescObject(const CSerialObject* obj, const CSeq_entry_Handle& tse)
    {

        const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(obj);
        const CBioSource* bsrc = dynamic_cast<const CBioSource*>(obj);
        const CMolInfo* molinfo = dynamic_cast<const CMolInfo*>(obj);

        for (CBioseq_CI b_iter(tse); b_iter; ++b_iter) {
            CConstRef<CBioseq> bseq = b_iter->GetCompleteBioseq();
            if (seqdesc) {
                for (CSeqdesc_CI desc_it(*b_iter); desc_it; ++desc_it) {
                    if (seqdesc == &(*desc_it)) {
                        return bseq;
                    }
                }

            }
            else if (bsrc) {
                for (CSeqdesc_CI desc_it(*b_iter, CSeqdesc::e_Source); desc_it; ++desc_it) {
                    if (bsrc == &(desc_it->GetSource())) {
                        return bseq;
                    }
                }

            }
            else if (molinfo) {
                for (CSeqdesc_CI desc_it(*b_iter, CSeqdesc::e_Molinfo); desc_it; ++desc_it) {
                    if (molinfo == &(desc_it->GetMolinfo())) {
                        return bseq;
                    }
                }
            }

        }
        return CConstRef<CBioseq>(NULL);
    }

    CObjectInfo GetPrimitiveObjInfo(const CObjectInfo& info)
    {
        if (info.GetTypeFamily() == eTypeFamilyPrimitive) {
            return info;
        }
        else if (info.GetTypeFamily() == eTypeFamilyPointer) {
            if (info.GetPointedObject().GetTypeFamily() == eTypeFamilyPrimitive) {
                return info.GetPointedObject();
            }
        }
        return CObjectInfo();
    }

    void GetPrimitiveObjectInfos(CMQueryNodeValue::TObs& objs, const CMQueryNodeValue::SResolvedField& info)
    {
        const string dbtag_name = "Dbtag";
        CObjectInfo obj = info.field;
        switch (obj.GetTypeFamily()) {
        case eTypeFamilyPrimitive:
            objs.push_back(info);
            break;
        case eTypeFamilyPointer: {
            if (obj.GetPointedObject().GetTypeFamily() == eTypeFamilyPrimitive)
                objs.push_back(CMQueryNodeValue::SResolvedField(info.parent, obj.GetPointedObject()));
            break;
        }
        case eTypeFamilyContainer: {
            CObjectInfoEI elem = obj.BeginElements();
            while (elem.Valid()) {
                CObjectInfo elem_oi = elem.GetElement();
                if (elem_oi.GetTypeFamily() == eTypeFamilyPrimitive) {
                    objs.push_back(CMQueryNodeValue::SResolvedField(obj, elem_oi));
                }
                else if (elem_oi.GetTypeFamily() == eTypeFamilyPointer) {
                    CObjectInfo pointed = elem_oi.GetPointedObject();
                    if (pointed.GetName() == dbtag_name) {
                        objs.push_back(CMQueryNodeValue::SResolvedField(obj, pointed));
                    }
                }
                ++elem;
            }
            break;
        }
        case eTypeFamilyClass: {
            if (obj.GetName() == dbtag_name) {
                objs.push_back(info);
            }
        }
        default:
            break;
        }
    }

    void GetPrimitiveObjInfosWithContainers(CMQueryNodeValue::TObs& objs, const CMQueryNodeValue::SResolvedField& info)
    {
        CObjectInfo obj = info.field;
        if (obj.GetTypeFamily() == eTypeFamilyClass) {
            if (obj.GetName() == "OrgMod") {
                CObjectInfo subname_oi = obj.FindClassMember("subname").GetMember();
                objs.push_back(CMQueryNodeValue::SResolvedField(obj, subname_oi));
            }
            else if (obj.GetName() == "SubSource") {
                CObjectInfo subname_oi = obj.FindClassMember("name").GetMember();
                objs.push_back(CMQueryNodeValue::SResolvedField(obj, subname_oi));
            }
            else if (obj.GetName() == "Gb-qual") {
                CObjectInfo subname_oi = obj.FindClassMember("val").GetMember();
                objs.push_back(CMQueryNodeValue::SResolvedField(obj, subname_oi));
            }
        }
        else {
            NMacroUtil::GetPrimitiveObjectInfos(objs, info);
        }
    }

    CMQueryNodeValue::EType GetPrimitiveFromRef(CMQueryNodeValue& node)
    {
        if (node.IsRef()) {
            node.Dereference();
        }
        return node.GetDataType();
    }

    void SwapGbQualWithValues(CMQueryNodeValue::TObs& objs)
    {
        bool all_gbquals = true;
        for (auto&& it : objs) {
            if (!NStr::EqualNocase(it.field.GetName(), "Gb-qual")) {
                all_gbquals = false;
                break;
            }
        }

        if (all_gbquals && !objs.empty()) {
            CMQueryNodeValue::TObs tmp;
            for (auto&& it : objs) {
                auto val_member = it.field.FindClassMember("val").GetMember();
                tmp.push_back(CMQueryNodeValue::SResolvedField(it.field, val_member));
            }
            swap(tmp, objs);
        }
    }

    void RemoveGeneXref(CSeq_feat& feat)
    {
        EDIT_EACH_SEQFEATXREF_ON_SEQFEAT(it, feat) {
            if ((*it)->IsSetData() && (*it)->GetData().IsGene()) {
                ERASE_SEQFEATXREF_ON_SEQFEAT(it, feat);
            }
        }
        if (feat.GetXref().empty())
            feat.ResetXref();
    }

    void RemoveEmptyDescriptors(CBioseq& bseq)
    {
        if (bseq.IsNa()) {
            EDIT_EACH_SEQDESC_ON_BIOSEQ(it, bseq) {
                if ((*it)->IsTitle() && (*it)->GetTitle().empty()) {
                    ERASE_SEQDESC_ON_BIOSEQ(it, bseq);
                }
                else if ((*it)->IsComment() && (*it)->GetComment().empty()) {
                    ERASE_SEQDESC_ON_BIOSEQ(it, bseq);
                }
                else if ((*it)->IsGenbank() && (*it)->GetGenbank().IsEmpty()) {
                    ERASE_SEQDESC_ON_BIOSEQ(it, bseq);
                }
            }
        }
    }

    string GetStringValue(CRef<CMQueryNodeValue>& value)
    {
        string newValue;
        if (value->IsString()) {
            newValue = value->GetString();
        }
        else if (value->IsInt()) {
            value->SetString(NStr::Int8ToString(value->GetInt()));
            newValue = value->GetString();
        }
        else if (value->IsDouble()) {
            value->SetString(NStr::DoubleToString(value->GetDouble()));
            newValue = value->GetString();
        }
        else {
            NCBI_THROW(CMacroExecException, eWrongArguments, "Wrong type of new_value was passed to function");
        }
        return newValue;
    }

    CSeqFeatData::ESubtype GetFeatSubtype(const string& feat_type)
    {
        if (NStr::IsBlank(feat_type)) {
            return CSeqFeatData::eSubtype_bad;
        }

        if (NStr::EqualNocase(feat_type, "any") || NStr::EqualNocase(feat_type, "all")) {
            return CSeqFeatData::eSubtype_any;
        }
        // handle exceptional cases first
        if (StringsAreEquivalent(feat_type, "source") ||
            StringsAreEquivalent(feat_type, "biosrc") ||
            StringsAreEquivalent(feat_type, "biosource")) {
            return CSeqFeatData::eSubtype_biosrc;

        }
        else if (StringsAreEquivalent(feat_type, "preRNA") ||
            StringsAreEquivalent(feat_type, "precursor_RNA")) {
            return CSeqFeatData::eSubtype_preRNA;

        }
        else if (StringsAreEquivalent(feat_type, "miscRNA") ||
            StringsAreEquivalent(feat_type, "otherRNA")) {
            return CSeqFeatData::eSubtype_otherRNA;
        }
        else {
            int subtype = -1;
            const CFeatList* feat_list = CSeqFeatData::GetFeatList();
            ITERATE(CFeatList, it, *feat_list) {
                const CFeatListItem* f = &(*it);
                subtype = f->GetSubtype();
                string descr = f->GetDescription();
                string name = CSeqFeatData::SubtypeValueToName(CSeqFeatData::ESubtype(subtype));
                if (StringsAreEquivalent(feat_type, descr) ||
                    StringsAreEquivalent(feat_type, name)) {
                    // found the subtype
                    break;
                }
            }
            if (subtype > 0) {
                return CSeqFeatData::ESubtype(subtype);
            }
        }
        return CSeqFeatData::eSubtype_bad;
    }


    bool StringsAreEquivalent(const string& name1, const string& name2)
    {
        string str1(name1), str2(name2);
        // spaces, dashes and underscores do not count
        NStr::ReplaceInPlace(str1, " ", "");
        NStr::ReplaceInPlace(str1, "_", "");
        NStr::ReplaceInPlace(str1, "-", "");
        NStr::ReplaceInPlace(str2, " ", "");
        NStr::ReplaceInPlace(str2, "_", "");
        NStr::ReplaceInPlace(str2, "-", "");

        return NStr::EqualNocase(str1, str2);
    }

    bool GetLocusTagFromGene(const CGene_ref& gene, string& locus_tag)
    {
        if (gene.IsSuppressed()) {
            return false;
        }

        if (gene.IsSetLocus_tag() && !NStr::IsBlank(gene.GetLocus_tag())) {
            locus_tag = gene.GetLocus_tag();
            return true;
        }
        else if (gene.IsSetLocus() && !NStr::IsBlank(gene.GetLocus())) {
            locus_tag = gene.GetLocus();
            return true;
        }

        return false;
    }

    bool GetLocusTagFromProtRef(const CSeq_feat& prot_feat, CScope& scope, string& locus_tag)
    {
        if (!prot_feat.GetData().IsProt())
            return false;

        const CGene_ref* gene = prot_feat.GetGeneXref();
        if (gene) {
            return GetLocusTagFromGene(*gene, locus_tag);
        }

        CBioseq_Handle bsh = scope.GetBioseqHandle(prot_feat.GetLocation());
        if (bsh) {
            const CSeq_feat* cds = sequence::GetCDSForProduct(*bsh.GetCompleteBioseq(), &scope);
            if (cds) {
                gene = cds->GetGeneXref();
                if (gene) {
                    return GetLocusTagFromGene(*gene, locus_tag);
                }

                CConstRef<CSeq_feat> gene_feat = sequence::GetOverlappingGene(cds->GetLocation(), scope);
                if (gene_feat && gene_feat->GetData().IsGene()) {
                    return GetLocusTagFromGene(gene_feat->GetData().GetGene(), locus_tag);
                }
            }
        }
        return false;
    }

    CRef<CSeqdesc> MakeNcbiAutofixUserObject()
    {
        CRef<CSeqdesc> desc(new CSeqdesc);
        CRef<CUser_object> user_obj(new CUser_object);
        user_obj->SetType().SetStr("NcbiAutofix");
        user_obj->SetData();
        desc->SetUser(user_obj.GetNCObject());
        return desc;
    }

    bool FindNcbiAutofixUserObject(const CSeq_entry_Handle& tse)
    {
        for (CSeq_entry_CI entry_ci(tse, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry); entry_ci; ++entry_ci) {
            if (entry_ci->IsSetDescr()) {
                for (CSeqdesc_CI desc_it(*entry_ci, CSeqdesc::e_User); desc_it; ++desc_it) {
                    const CUser_object& user_obj = desc_it->GetUser();
                    if (user_obj.IsSetType()
                        && user_obj.GetType().IsStr()
                        && NStr::EqualCase(user_obj.GetType().GetStr(), "NcbiAutofix")) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    CRNA_ref::EType GetRNAType(const string& name)
    {
        CRNA_ref::EType rna_type = CRNA_ref::eType_unknown;
        if (NStr::EqualNocase(name, "preRNA"))
            rna_type = CRNA_ref::eType_premsg;
        else if (NStr::EqualNocase(name, "mRNA"))
            rna_type = CRNA_ref::eType_mRNA;
        else if (NStr::EqualNocase(name, "tRNA"))
            rna_type = CRNA_ref::eType_tRNA;
        else if (NStr::EqualNocase(name, "rRNA"))
            rna_type = CRNA_ref::eType_rRNA;
        else if (NStr::EqualNocase(name, "ncRNA"))
            rna_type = CRNA_ref::eType_ncRNA;
        else if (NStr::EqualNocase(name, "tmRNA"))
            rna_type = CRNA_ref::eType_tmRNA;
        else if (NStr::EqualNocase(name, "miscRNA"))
            rna_type = CRNA_ref::eType_miscRNA;

        return rna_type;
    }

    bool IsBiosourceModifier(const string& field)
    {
        bool issubsrc = CSubSource::IsValidSubtypeName(field, CSubSource::eVocabulary_insdc);
        bool isorgmod = COrgMod::IsValidSubtypeName(field, COrgMod::eVocabulary_insdc);

        return (issubsrc || isorgmod);
    }

    bool IsTaxname(CMQueryNodeValue::SResolvedField& res)
    {
        bool is_taxname = false;
        if (res.parent.GetName() == "Org-ref") {
            CObjectInfoMI mi = res.parent.FindClassMember("taxname");
            // name is encoded in mi.GetItemInfo()->GetId().GetName()
            if (mi.IsSet() && mi.GetMember() == res.field) {
                is_taxname = true;
            }
        }
        return is_taxname;
    }

    void CleanupForTaxnameChange(CObjectInfo oi)
    {
        CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
        if (bsrc) {
            edit::RemoveTaxId(*bsrc);
            edit::RemoveOldName(*bsrc);
        }
    }

    void CleanupForTaxnameChange(CMQueryNodeValue::SResolvedField& res, CObjectInfo oi)
    {
        if (IsTaxname(res)) {
            CleanupForTaxnameChange(oi);
        }
    }

    void s_SetInitials(CName_std& std_name, const string& middle_init)
    {
        // don't add another period if one is already there
        if (!NStr::EndsWith(middle_init, ".")) {
            std_name.SetInitials(middle_init + ".");
        }
        else {
            std_name.SetInitials(middle_init);
        }
    }

    bool ApplyFirstName(CName_std& std_name, const string& newValue, edit::EExistingText existing_text)
    {
        bool modified = false;
        if (existing_text == edit::eExistingText_leave_old || existing_text == edit::eExistingText_cancel) {
            return modified;
        }

        string orig_initials = (std_name.IsSetInitials()) ? std_name.GetInitials() : kEmptyStr;
        string first_initials = (std_name.IsSetFirst()) ?
            CMacroFunction_AuthorFix::s_GetFirstNameInitials(std_name.GetFirst()) : kEmptyStr;

        string new_first_init = CMacroFunction_AuthorFix::s_GetFirstNameInitials(newValue);

        if (first_initials.empty()) { // there is no first name
            if (orig_initials.empty()) {  // there is no middle initial
                std_name.SetFirst(newValue);
                std_name.SetInitials(new_first_init);
                modified = true;
            }
            else {  // there is a middle initial in the initials field
                std_name.SetFirst(newValue);
                string middle_init = new_first_init + orig_initials;
                s_SetInitials(std_name, middle_init);
                modified = true;
            }
        }
        else {  // there is a first name
            if (orig_initials.empty()) {  // the 'initials' field is not set
                string first_name = std_name.GetFirst();
                if (edit::AddValueToString(first_name, newValue, existing_text)) {
                    if (!NStr::EqualCase(std_name.GetFirst(), first_name)) {
                        std_name.SetFirst(first_name);
                        std_name.SetInitials(CMacroFunction_AuthorFix::s_GetFirstNameInitials(std_name.GetFirst()));
                        modified = true;
                    }
                }
            }
            else { // the initials field is set
                string middle_init;
                if (NStr::StartsWith(orig_initials, first_initials, NStr::eCase)) {
                    middle_init = orig_initials.substr(first_initials.length());
                }

                if (middle_init.empty()) { // there are no middle initials
                    string first_name = std_name.GetFirst();
                    if (edit::AddValueToString(first_name, newValue, existing_text)) {
                        if (!NStr::EqualCase(std_name.GetFirst(), first_name)) {
                            std_name.SetFirst(first_name);
                            std_name.SetInitials(CMacroFunction_AuthorFix::s_GetFirstNameInitials(std_name.GetFirst()));
                            modified = true;
                        }
                    }
                }
                else {
                    string first_name = std_name.GetFirst();
                    if (edit::AddValueToString(first_name, newValue, existing_text)) {
                        if (!NStr::EqualCase(std_name.GetFirst(), first_name)) {
                            std_name.SetFirst(first_name);
                            string new_init = CMacroFunction_AuthorFix::s_GetFirstNameInitials(std_name.GetFirst());
                            new_init += middle_init;
                            s_SetInitials(std_name, new_init);
                            modified = true;
                        }
                    }
                }
            }
        }

        if (modified) {
            CMacroFunction_AuthorFix::s_FixInitials(std_name);
        }

        return modified;
    }

    bool RemoveFirstName(CName_std& std_name)
    {
        if (!std_name.IsSetFirst())
            return false;

        bool modified = false;
        string first_initials = CMacroFunction_AuthorFix::s_GetFirstNameInitials(std_name.GetFirst());
        string orig_initials = std_name.IsSetInitials() ? std_name.GetInitials() : kEmptyStr;


        if (orig_initials.empty()) {
            std_name.ResetFirst();
            modified = true;
        }
        else {
            std_name.ResetFirst();
            modified = true;

            string middle_init;
            if (NStr::StartsWith(orig_initials, first_initials, NStr::eCase)) {
                middle_init = orig_initials.substr(first_initials.length());
            }
            if (middle_init.empty()) { // there is no middle initial
                std_name.ResetInitials();
            }
            else {
                std_name.SetInitials(middle_init);
            }
        }

        return modified;
    }
    
    bool ApplyMiddleInitial(CName_std& std_name, const string& newValue, edit::EExistingText existing_text)
    {
        bool modified = false;
        if (existing_text == edit::eExistingText_leave_old || existing_text == edit::eExistingText_cancel) {
            return modified;
        }

        string orig_initials = (std_name.IsSetInitials()) ? std_name.GetInitials() : kEmptyStr;
        string first_initials = (std_name.IsSetFirst()) ?
            CMacroFunction_AuthorFix::s_GetFirstNameInitials(std_name.GetFirst()) : kEmptyStr;

        if (first_initials.empty()) { // there is no first name
            if (orig_initials.empty()) {  // there is no middle initial
                std_name.SetInitials(newValue);
                modified = true;
            }
            else {  // there is a middle initial
                string middle_init = orig_initials;
                if (edit::AddValueToString(middle_init, newValue, existing_text)) {
                    if (!NStr::EqualCase(middle_init, orig_initials)) {
                        s_SetInitials(std_name, middle_init);
                        modified = true;
                    }
                }
            }
        }
        else { // there is a first name
            if (orig_initials.empty()) {  // the 'initials' field is not set
                string inits = first_initials;
                inits.append(newValue);
                s_SetInitials(std_name, inits);
                modified = true;
            }
            else { // the initials field is set
                string middle_init;
                if (NStr::StartsWith(orig_initials, first_initials, NStr::eCase)) {
                    middle_init = orig_initials.substr(first_initials.length());
                }

                if (middle_init.empty()) { // there are no middle initials
                    middle_init = orig_initials + newValue;
                    s_SetInitials(std_name, middle_init);
                    modified = true;
                }
                else {
                    string orig_val = middle_init;
                    if (edit::AddValueToString(orig_val, newValue, existing_text)) {
                        if (!NStr::EqualCase(orig_val, middle_init)) {
                            s_SetInitials(std_name, orig_val);
                            modified = true;
                        }
                    }
                }
            }
        }

        if (modified) {
            CMacroFunction_AuthorFix::s_FixInitials(std_name);
        }

        return modified;
    }

    bool RemoveMiddleInitial(CName_std& std_name)
    {
        if (!std_name.IsSetInitials())
            return false;

        bool modified = false;
        string orig_initials = std_name.GetInitials();
        string first_initials = (std_name.IsSetFirst()) ?
            CMacroFunction_AuthorFix::s_GetFirstNameInitials(std_name.GetFirst()) : kEmptyStr;

        if (first_initials.empty()) {
            std_name.ResetInitials();
            modified = true;
        }
        else {
            if (NStr::StartsWith(orig_initials, first_initials, NStr::eCase)) {
                string middle_init = orig_initials.substr(first_initials.length());
                if (!middle_init.empty()) {
                    std_name.SetInitials(orig_initials.substr(0, first_initials.length()));
                    modified = true;
                }
            }
        }

        return modified;
    }

    unsigned ApplyAuthorNames(CAuth_list& auth_list, const string& newValue)
    {
        unsigned count = 0;
        if (newValue.empty())
            return count;

        vector<string> names;
        NStr::Split(newValue, ",", names, NStr::fSplit_Tokenize);

        auth_list.ResetNames();
        for (auto&& it : names) {
            vector<string> full_name;
            NStr::Split(it, " ", full_name, NStr::fSplit_Tokenize);
            string first_name, last_name, middle_init, suffix;

            if (full_name.size() == 4) {
                first_name = full_name[0];
                middle_init = full_name[1];
                last_name = full_name[2];
                suffix = full_name[3];
            }
            else if (full_name.size() == 3) {
                first_name = full_name[0];
                middle_init = full_name[1];
                last_name = full_name[2];
            }
            else if (full_name.size() == 2) {
                first_name = full_name[0];
                last_name = full_name[1];
            }
            else if (full_name.size() == 1)
            {
                last_name = full_name[0];
            }

            if (!last_name.empty()) {
                CRef<CAuthor> new_auth(new CAuthor());
                CName_std& std_name = new_auth->SetName().SetName();
                std_name.SetLast(last_name);
                if (!first_name.empty())
                    std_name.SetFirst(first_name);

                string initials = (std_name.IsSetFirst()) ? CMacroFunction_AuthorFix::s_GetFirstNameInitials(std_name.GetFirst()) : kEmptyStr;

                if (!middle_init.empty()) {
                    initials.append(CMacroFunction_AuthorFix::s_GetFirstNameInitials(middle_init));
                }
                // don't add another period if one is already there
                if (!initials.empty() && !NStr::EndsWith(initials, ".")) {
                    initials.append(".");
                }
                if (!initials.empty()) {
                    std_name.SetInitials(initials);
                }

                if (!suffix.empty())
                    std_name.SetSuffix(suffix);
                auth_list.SetNames().SetStd().push_back(new_auth);
                count++;
            }
        }
        return count;
    }

    EStructVoucherPart GetSVPartFromString(const string voucher_part)
    {
        auto part = eSV_Error;
        const map<string, EStructVoucherPart> part_map =
        { {"inst", eSV_Inst}, {"coll", eSV_Coll}, {"specid", eSV_Specid} };

        auto it = part_map.find(voucher_part);
        if (it != part_map.end()) part = it->second;
        return part;
    }

    bool IsStructVoucherPart(const string& field)
    {
        return (NStr::EndsWith(field, kColl_suffix) ||
            NStr::EndsWith(field, kInst_suffix) ||
            NStr::EndsWith(field, kSpecid_suffix));
    }

    void ConvertMultiSpaces(string& line, const string& delimiter)
    {
        string result;
        int spaces = 0;
        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] != ' ' && (spaces == 0)) {
                result += line[i];
            }
            else if (line[i] == ' ') {
                spaces += 1;
            }
            else if (line[i] != ' ' && (spaces > 0)) {
                if (spaces == 1) {
                    result += ' ';
                }
                else {
                    result += delimiter;
                }
                result += line[i];
                spaces = 0;
            }
        }

        // handle trailing space or spaces
        if (spaces > 1)
            result += delimiter;
        else if (spaces == 1)
            result += ' ';
        line.assign(result);
    }

    string TransformForCSV(const string& str)
    {
        string escaped = str;
        if (str.find('\n') != NPOS || str.find(',') != NPOS || str.find('"') != NPOS) {
            escaped = NStr::Quote(str, '"', '"');
        }
        return escaped;
    }

    using TFeatLabelMap = SStaticPair<const char*, CLabel::ELabelType>;
    static const TFeatLabelMap s_LabelTypeToName[] =
    {
        { "", CLabel::eInvalid },
        { "eContent", CLabel::eContent },
        { "eContentAndProduct", CLabel::eContentAndProduct },
        { "eDefault", CLabel::eDefault },
        { "eDescription", CLabel::eDescription },
        { "eDescriptionBrief", CLabel::eDescriptionBrief },
        { "eGi", CLabel::eGi },
        { "eSymbolAlias", CLabel::eSymbolAlias},
        { "eType", CLabel::eType },
        { "eUserSubtype", CLabel::eUserSubtype },
        { "eUserType", CLabel::eUserType },
        { "eUserTypeAndContent", CLabel::eUserTypeAndContent },
    };

    using TLabelTypeToNameMap = CStaticPairArrayMap<string, CLabel::ELabelType>;
    DEFINE_STATIC_ARRAY_MAP(TLabelTypeToNameMap, sm_CLabelMap, s_LabelTypeToName);

    CLabel::ELabelType NameToLabelType(const string& name)
    {
        CLabel::ELabelType type = CLabel::eInvalid;
        TLabelTypeToNameMap::const_iterator iter = sm_CLabelMap.find(name);
        if (iter != sm_CLabelMap.end()) {
            type = iter->second;
        }
        return type;
    }

    bool IsSatelliteSubfield(const string& field)
    {
        return (StringsAreEquivalent(field, kSatelliteType) || StringsAreEquivalent(field, kSatelliteName));
    }

    bool IsMobileElementTSubfield(const string& field)
    {
        return (StringsAreEquivalent(field, kMobileElementTType) || StringsAreEquivalent(field, kMobileElementTName));
    }
}

const char* kInst_suffix = "-inst";
const char* kColl_suffix = "-coll";
const char* kSpecid_suffix = "-specid";
const char* kSatelliteType = "satellite-type";
const char* kSatelliteName = "satellite-name";
const char* kMobileElementTQual = "mobile_element_type";
const char* kMobileElementTType = "mobile-element-type-type";
const char* kMobileElementTName = "mobile-element-type-name";


// CRemoveTextOptions
bool CRemoveTextOptions::EditText(string& str) const
{
    // examine beginning
    size_t start_pos = string::npos;
    size_t start_len = 0;

    switch (m_BeforeMatch) {
    case eNone:
        // nothing to remove
        start_pos = 0;
        break;
    case eText:
        start_pos = edit::FindWithOptions(str, m_BeforeText, 0, m_CaseInsensitive, m_WholeWord);
        start_len = m_BeforeText.length();
        break;
    case eDigits:
        start_pos = 0;
        while (start_pos < str.length() && !isdigit(str.c_str()[start_pos])) {
            start_pos++;
        }
        if (start_pos < str.length()) {
            start_len = 1;
            while (start_pos + start_len < str.length() && isdigit(str.c_str()[start_pos + start_len])) {
                start_len++;
            }
        }
        break;
    case eLetters:
        start_pos = 0;
        while (start_pos < str.length() && !isalpha(str.c_str()[start_pos])) {
            start_pos++;
        }
        if (start_pos < str.length()) {
            start_len = 1;
            while (start_pos + start_len < str.length() && isalpha(str.c_str()[start_pos + start_len])) {
                start_len++;
            }
        }
        break;
    }
    if (start_pos == string::npos || start_pos == str.length()) {
        // beginning not found
        return false;
    }


    // examine end
    size_t end_pos = string::npos;
    size_t end_len = 0;

    switch (m_AfterMatch) {
    case eNone:
        // nothing to remove
        end_pos = str.length();
        break;
    case eText:
        end_pos = edit::FindWithOptions(str, m_AfterText, start_pos + start_len, m_CaseInsensitive, m_WholeWord);
        end_len = m_AfterText.length();
        break;
    case eDigits:
        end_pos = start_pos + start_len;
        while (end_pos < str.length() && !isdigit(str.c_str()[end_pos])) {
            end_pos++;
        }
        if (end_pos < str.length()) {
            end_len = 1;
            while (end_pos + end_len < str.length() && isdigit(str.c_str()[end_pos + end_len])) {
                end_len++;
            }
        }
        break;
    case eLetters:
        end_pos = start_pos + start_len;
        while (end_pos < str.length() && !isalpha(str.c_str()[end_pos])) {
            end_pos++;
        }
        if (end_pos < str.length()) {
            end_len = 1;
            while (end_pos + end_len < str.length() && isalpha(str.c_str()[end_pos + end_len])) {
                end_len++;
            }
        }
        break;
    }
    if (end_pos == string::npos || (end_pos == str.length() && m_AfterMatch != eNone) || end_pos < start_pos) {
        // end not found
        return false;
    }

    bool any_change = false;
    // remove end first
    if (m_AfterMatch != eNone) {
        size_t stop = end_pos;
        if (!m_RemoveAfterMatch) {
            stop += end_len;
        }
        if (stop < str.length()) {
            str = str.substr(0, stop);
            any_change = true;
        }
    }

    if (m_BeforeMatch != eNone) {
        size_t start = start_pos;
        if (m_RemoveBeforeMatch) {
            start += start_len;
        }
        if (start > 0) {
            str = str.substr(start);
            any_change = true;
        }
    }
    return any_change;
}


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
