/*  $Id: macro_fn_where.cpp 45077 2020-05-22 23:43:56Z rudnev $
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
 * Authors:  Anatoly Osipov, Andrea Asztalos
 *
 */

// this should not be defined until variation_lib moves into the public part of the Tkit
// #define USE_VARIATION_LIB

//!! this is a hack to fool Tkit version skew protection defence so I can use a prebuilt version w/o
//!! having to change SV build environment
//!! remove once we don't have to jump through hoops to use variation_lib
#ifdef USE_VARIATION_LIB
int Ncbi_library_version_mismatch__Full_rebuild_to_fix_20200522
(void)
{
    static int v;
    return ++v;
}
#endif
//!!

#include <ncbi_pch.hpp>
#include <objects/general/Dbtag.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objects/valid/Comment_set.hpp>
#include <objtools/edit/field_handler.hpp>
#include <objtools/validator/validator.hpp>
#include <objtools/snputil/snp_bitfield.hpp>
#include <objtools/snputil/snp_utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/macro_fn_where.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_field_resolve.hpp>


#ifdef USE_VARIATION_LIB
#include <internal/variation/lib/Variant/Spdi.hpp>
#include <internal/variation/lib/Constructors/VcfForSpdi.hpp>
#include <internal/variation/lib/Primitives/Sequence.hpp>
#include <internal/variation/lib/Primitives/SeqId.hpp>
#endif

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ChoiceType
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ChoiceType, "CHOICETYPE")
void CMacroFunction_ChoiceType::TheFunction()
{
    string field_name;
    if (!m_Args.empty()) {
        field_name = m_Args[0]->GetString();
    }

    string choice_type;
    if (m_DataIter->GetChoiceType(field_name, choice_type)) {
        m_Result->SetString(choice_type);
    }
}

bool CMacroFunction_ChoiceType::x_ValidArguments() const
{
    return (m_Args.empty() || (m_Args.size() == 1 && m_Args[0]->IsString()));
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_IsPresent
/// ISPRESENT(field_name)
/// ISPRESENT(container, element_name)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_IsPresent, "ISPRESENT");

void CMacroFunction_IsPresent::TheFunction()
{   
    size_t index = 0;
    if (m_Args[index]->IsNotSet()) {
        m_Result->SetBool(false);
        return;
    }

    if (m_Args[index]->IsRef()) {
        CMQueryNodeValue& objs = m_Args[index].GetNCObject();
        objs.Dereference();

        if (m_Args[index]->IsString()) {
            bool rval = !m_Args[index]->GetString().empty();
            m_Result->SetBool(rval);
            return;
        }
    }

    if (m_Args[index]->AreObjects()) {
        bool rval = !m_Args[index]->GetObjects().empty();
        m_Result->SetBool(rval);
        return;
    }

    if (m_Args[index]->AreStrings()) {
        bool rval = !m_Args[index]->GetStrings().empty();
        m_Result->SetBool(rval);
        return;
    }

    bool cont_element = (m_Args.size() == 2); // describing an element of a container
    string field_name = m_Args[index]->GetString();
    string container;
    if (cont_element) {
        container = field_name;
        field_name = m_Args[++index]->GetString();
    }

    if (field_name.empty() || (cont_element && container.empty()))
        return;

    if (!cont_element) {
        x_IsSimpleTypePresent(field_name);
    } else {
        x_IsContainerElementPresent(container, field_name);
    }
}


bool CMacroFunction_IsPresent::x_ValidArguments() const
{
    size_t as = m_Args.size();
    if (as != 1 && as != 2)
        return false;

    size_t index = 0;
    bool first_ok = m_Args[index]->IsString() || m_Args[index]->AreObjects() || m_Args[index]->IsNotSet() || m_Args[index]->IsRef();
    if (!first_ok) {
        return false;
    }

    if (as == 2) {
        for (auto&& it : m_Args) {
            if (!it->IsString())
                return false;
        }
    }
    return true;
}

void CMacroFunction_IsPresent::x_IsSimpleTypePresent(const string& field_name)
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    GetFieldsByName(&res_oi, oi, field_name);
    m_Result->SetBool(!res_oi.empty());
}

void CMacroFunction_IsPresent::x_IsContainerElementPresent(const string& container, const string& field_name)
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;

    GetFieldsByName(&res_oi, oi, container);
    if (res_oi.empty()) {
        m_Result->SetBool(false);
        return;
    }

    CObjectInfo mod_obj = res_oi.front().field;
    if (mod_obj.GetTypeFamily() != eTypeFamilyContainer) {
        m_Result->SetBool(false);
        return;
    }
    
    CObjectInfoEI elem = mod_obj.BeginElements();
    while (elem.Valid()) {
        CObjectInfo objInfo = elem.GetElement();
        if (objInfo.GetTypeFamily() == eTypeFamilyPointer) {
            objInfo = objInfo.GetPointedObject();
        }

        _ASSERT(objInfo.GetTypeFamily() == eTypeFamilyClass);
        TMemberIndex index = 1;
        CObjectInfo type_oi = objInfo.GetClassMemberIterator(index).GetMember();
        if (type_oi.GetTypeFamily() == eTypeFamilyPointer) {
            type_oi = type_oi.GetPointedObject();
        }
        
        if (type_oi.GetPrimitiveValueType() == ePrimitiveValueString) {
            string sValue = type_oi.GetPrimitiveValueString();
            if (NStr::EqualNocase(sValue, field_name)) {
                m_Result->SetBool(true);
                return;
            }
        } else if (type_oi.GetPrimitiveValueType() == ePrimitiveValueEnum) {
            string sValue;
            try {
                sValue = type_oi.GetPrimitiveValueString();
            }
            catch (const CException&) {
                sValue = NStr::NumericToString(type_oi.GetPrimitiveValueInt4());
            }
            if (NStr::EqualNocase(sValue, field_name)) {
                m_Result->SetBool(true);
                return;
            }
        }
        ++elem;
    }
    m_Result->SetBool(false);
}
///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_GetSeqdesc
/// BIOSOURCE_FOR_MOLINFO(field_name) or BIOSOURCE_FOR_MOLINFO(container, element_name)
/// BIOSOURCE_FOR_SEQUENCE(field_name) or BIOSOURCE_FOR_SEQUENCE(container, element_name)
/// MOLINFO_FOR_BIOSOURCE(field_name)
/// BIOSOURCE_FOR_SEQFEAT(field_name) or BIOSOURCE_FOR_SEQFEAT(container, element_name)
/// MOLINFO_FOR_SEQFEAT(field_name)
/// Can be used as an argument of another function
///
const char* CMacroFunction_GetSeqdesc::sm_BsrcForMolinfo = "BIOSOURCE_FOR_MOLINFO";
const char* CMacroFunction_GetSeqdesc::sm_MolinfoForBsrc = "MOLINFO_FOR_BIOSOURCE";
const char* CMacroFunction_GetSeqdesc::sm_BsrcForSeq = "BIOSOURCE_FOR_SEQUENCE";
const char* CMacroFunction_GetSeqdesc::sm_BsrcForFeat = "BIOSOURCE_FOR_SEQFEAT";
const char* CMacroFunction_GetSeqdesc::sm_MolinfoForFeat = "MOLINFO_FOR_SEQFEAT";
const char* CMacroFunction_GetSeqdesc::sm_BsrcForSeqdesc = "BIOSOURCE_FOR_SEQDESC";
void CMacroFunction_GetSeqdesc::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!obj || !scope)
        return;

    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    if (!bsh) return;

    CConstRef<CSeqdesc> seqdesc = bsh.GetCompleteBioseq()->GetClosestDescriptor(m_SeqdescType);
    if (!seqdesc)
        return;

    if (m_SeqdescType == CSeqdesc::e_Molinfo) {
        CMolInfo& molinfo = const_cast<CMolInfo&>(seqdesc->GetMolinfo());
        CObjectInfo objInfo(&molinfo, molinfo.GetTypeInfo());
        x_AssignReturnValue(objInfo, m_Args[0]->GetString());
    } else if (m_SeqdescType == CSeqdesc::e_Source) {
        CBioSource& bsrc = const_cast<CBioSource&>(seqdesc->GetSource());
        CObjectInfo objInfo(&bsrc, bsrc.GetTypeInfo());
        
        if (m_Args.size() == 1) {
            x_AssignReturnValue(objInfo, m_Args[0]->GetString());
        }
        else {
            x_AssignReturnValueFromContainer(objInfo, m_Args[0]->GetString(), m_Args[1]->GetString());
        }
    }
}

bool CMacroFunction_GetSeqdesc::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 2) {
        return false;
    }

    for (auto& it : m_Args) {
        if (!it->IsString()) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_GetDBLink
/// DBLinkType(dblink_type)
/// Can be used as an argument of another function
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_GetDBLink, "DBLINKTYPE");
void CMacroFunction_GetDBLink::TheFunction()
{
    const string field_type = m_Args[0]->GetString();
    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    if (!bsh) return;

    
    for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_User); desc_it; ++desc_it) {
        const auto& user = desc_it->GetUser();
        if (user.GetType().IsStr() && user.GetType().GetStr() == "DBLink") {
            CConstRef<CUser_field> const_user_field = user.GetFieldRef(field_type, ".", NStr::eNocase);
            if (const_user_field) {
                CUser_field& user_field = const_cast<CUser_field&>(const_user_field.GetObject());
                CObjectInfo objInfo(&user_field, user_field.GetTypeInfo());
                if (user_field.IsSetData() && user_field.GetData().IsStr())
                    x_AssignReturnValue(objInfo, "data.str");
                else if (user_field.IsSetData() && user_field.GetData().IsStrs()) 
                    x_AssignReturnValue(objInfo, "data.strs");
            }
        }
    }
}

bool CMacroFunction_GetDBLink::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->IsString());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Sequence_For_Seqdesc
/// SEQUENCE_FOR_SEQDESC(field_name)
/// SEQUENCE_FOR_SEQFEAT(field_name)
/// Can be used as an argument of another function
///
const char* CMacroFunction_GetSequence::sm_SeqForDescr = "SEQUENCE_FOR_SEQDESC";
const char* CMacroFunction_GetSequence::sm_SeqForFeat = "SEQUENCE_FOR_SEQFEAT";
void CMacroFunction_GetSequence::TheFunction()
{
    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    _ASSERT(bsh);

    if (bsh) {
        CConstRef<CBioseq> bseq = bsh.GetCompleteBioseq();
        CBioseq& seq = const_cast<CBioseq&>(bseq.GetObject());
        CObjectInfo objInfo(&seq, seq.GetTypeInfo());
        x_AssignReturnValue(objInfo, m_Args[0]->GetString());
    }
}

bool CMacroFunction_GetSequence::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->IsString());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RelatedFeatures
/// RELATED_FEATURE(feat_type, field_name)
/// RELATED_FEATURE(feat_type, container, element_name)
/// Can be used as an argument of another function
/// If the field has not been set, it will no be set by calling this function.
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RelatedFeatures, "RELATED_FEATURE")
void CMacroFunction_RelatedFeatures::TheFunction()
{
    CSeqFeatData::ESubtype constr_feat_type = NMacroUtil::GetFeatSubtype(m_Args[0]->GetString());
    string field_name = m_Args[1]->GetString();
    string container;
    if (m_Args.size() == 3) {
        container = field_name;
        field_name = m_Args[2]->GetString();
    }

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!feat || !scope) {
        return;
    }

    NMacroUtil::TVecFeatList feat_list = edit::GetRelatedFeatures(*feat, constr_feat_type, scope);
    NMacroUtil::TVecFeatList::iterator it = feat_list.begin();
    if (!feat_list.empty()) {
        CRef<CSeq_feat> related_feat(const_cast<CSeq_feat*>((*it).GetPointerOrNull()));
        CObjectInfo objInfo(related_feat.GetPointer(), related_feat.GetPointer()->GetTypeInfo());
        if (container.empty()) {
            x_AssignReturnValue(objInfo, field_name);
        }
        else {
            x_AssignReturnValueFromContainer(objInfo, container, field_name);
        }
    }
}

bool CMacroFunction_RelatedFeatures::x_ValidArguments() const
{
    if (m_Args.size() != 2 && m_Args.size() != 3) {
        return false;
    }
    for (auto& it : m_Args) {
        if (!it->IsString()) {
            return false;
        }
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Features_For_Object
/// Usage: FEATURES(feat_type, field_name) 
/// Returns objects corresponding to the field_name within features of type feat_type
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_Features_For_Object, "FEATURES")

void CMacroFunction_Features_For_Object::TheFunction()
{
    size_t index = 0;
    CSeqFeatData::ESubtype subtype = NMacroUtil::GetFeatSubtype(m_Args[index]->GetString());
    const string field_name = (++index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;

    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    if (!obj || !scope) 
        return;

    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    _ASSERT(bsh);
    if (!bsh){
        return;
    }

    CFeat_CI feat_it(bsh, SAnnotSelector(subtype));
    while (feat_it) {
        CConstRef<CSeq_feat> const_feat = feat_it->GetOriginalSeq_feat();
        CSeq_feat& feat = const_cast<CSeq_feat&>(const_feat.GetObject());
        CObjectInfo objInfo(&feat, feat.GetTypeInfo());
        x_AssignReturnValue(objInfo, field_name);
        ++feat_it;
    }
}

bool CMacroFunction_Features_For_Object::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 2) {
        return false;
    }
    for (auto& it : m_Args) {
        if (!it->IsString())
            return false;
    }
    return true;
}


///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_FirstItem
/// FIRSTOF(objects) - returns the first item from the list of objects
/// LASTOF(objects) - returns the last item from the list of objects
const char* CMacroFunction_FirstOrLastItem::sm_First = "FIRSTOF";
const char* CMacroFunction_FirstOrLastItem::sm_Last = "LASTOF";

void CMacroFunction_FirstOrLastItem::TheFunction()
{
    m_Result->SetNotSet();
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    if (type == CMQueryNodeValue::eObjects) {
        res_oi = m_Args[0]->GetObjects();
    }
    else if (type == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(res_oi, 0);
    }

    if (res_oi.empty()) {
        return;
    }

    m_Result->SetDataType(CMQueryNodeValue::eObjects);
    if (m_First) {
        m_Result->SetObjects().emplace_back(res_oi.front());
    }
    else {
        m_Result->SetObjects().emplace_back(res_oi.back());
    }
}

bool CMacroFunction_FirstOrLastItem::x_ValidArguments() const
{
    return (m_Args.size() == 1 && (m_Args[0]->AreObjects() || m_Args[0]->IsRef() || m_Args[0]->IsNotSet()));
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_CDSTranslation
/// TRANSLATION()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_CDSTranslation, "TRANSLATION")

void CMacroFunction_CDSTranslation::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* seq_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!scope || !seq_feat || !seq_feat->GetData().IsCdregion()) {
        return;
    }

    string prot_seq;
    CSeqTranslator::Translate(*seq_feat, *scope, prot_seq);
    if (prot_seq.empty()) {
        return;
    }

    if (m_Nested == eNotNested) { // return a string
        m_Result->SetString(prot_seq);
    } else {
        // return a reference to a CMQueryNodeValue of type string
        CRef<CMQueryNodeValue> new_node(new CMQueryNodeValue);
        new_node->SetString(prot_seq);
        m_Result->SetRef(new_node);
    }
}

bool CMacroFunction_CDSTranslation::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SeqID
/// SeqID()
/// Returns the string representation of sequence ids 
DEFINE_MACRO_FUNCNAME(CMacroFunction_SeqID, "SEQID")
void CMacroFunction_SeqID::TheFunction()
{
    m_Result->SetNotSet();
    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    if (!bsh)
        return;
    
    vector<string> id_labels;
    const CBioseq& bseq = *bsh.GetCompleteBioseq();
    for (const auto& it : bseq.GetId()) {
        string label;
        it->GetLabel(&label, CSeq_id::eContent, CSeq_id::fLabel_GeneralDbIsContent);
        id_labels.push_back(label);

        if (it->IsGeneral() && it->GetGeneral().IsSetDb() && it->GetGeneral().GetDb() == "BankIt") {
            id_labels.push_back("BankIt" + label);
        }

        if (it->IsGenbank()) {
            label.resize(0);
            it->GetLabel(&label, CSeq_id::eContent);
            id_labels.push_back(label);
        }
    }
    m_Result->SetStrings(id_labels);
}

bool CMacroFunction_SeqID::x_ValidArguments() const
{
    return m_Args.empty();
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Accession
/// Accession()
/// Returns the accession with its version, if it exists. Otherwise, it returns the string
/// representation of the best id
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_Accession, "ACCESSION")
void CMacroFunction_Accession::TheFunction()
{
    m_Result->SetNotSet();
    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    if (!bsh)
        return;
    
    bool with_version = true;
    try {
        CSeq_id_Handle idh = sequence::GetId(*bsh.GetSeqId(), bsh.GetScope(), sequence::eGetId_Best);
        if (idh) {
            string acc = idh.GetSeqId()->GetSeqIdString(true);
            m_Result->SetString(acc);
        }
    }
    catch (const CException& e) {
        LOG_POST(Error << "CMacroFunction_Accession: failed when retrieving accession, " << e.GetMsg());
        return;
    }
}

bool CMacroFunction_Accession::x_ValidArguments() const
{
    return m_Args.empty();
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_InconsistentTaxa
/// INCONSISTENT_TAXA() - returns true if taxnames in the set are not the same
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_InconsistentTaxa, "INCONSISTENT_TAXA")
void CMacroFunction_InconsistentTaxa::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    CConstRef<CBioseq_set> seqset(dynamic_cast<const CBioseq_set*>(obj.GetPointer()));
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!seqset || !scope) {
        return;
    }

    string taxname, first_taxname;
    bool is_first(true), inconsistent(false);
    CBioseq_set_Handle bssh = scope->GetBioseq_setHandle(*seqset);
    for (CBioseq_CI b_iter(bssh, CSeq_inst::eMol_na); b_iter; ++b_iter) {
        CSeqdesc_CI desc(*b_iter, CSeqdesc::e_Source);
        if (desc) {
            if (desc->GetSource().IsSetTaxname()) {
                taxname = desc->GetSource().GetTaxname();
            }
        } else {
            CFeat_CI feat_it(*b_iter, CSeqFeatData::e_Biosrc);
            if (feat_it && feat_it->GetData().GetBiosrc().IsSetTaxname()) {
                taxname = feat_it->GetData().GetBiosrc().GetTaxname();
            }
        }

        if (is_first) {
            first_taxname = taxname;
            is_first = false;
            continue;
        }

        if (!NStr::EqualNocase(first_taxname, taxname)) {
            inconsistent = true;
            break;
        }
    }
    m_Result->SetBool(inconsistent);
}

bool CMacroFunction_InconsistentTaxa::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_StringLength
/// STRLENGTH(field_name|object) - returns the character length of the field
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_StringLength, "STRLENGTH")

void CMacroFunction_StringLength::TheFunction()
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
    }

    if (res_oi.empty()) {
        return;
    }

    CObjectInfo obj = res_oi.front().field;
    if (obj.GetTypeFamily() == eTypeFamilyPrimitive &&
        obj.GetPrimitiveValueType() == ePrimitiveValueString) {
        m_Result->SetInt((int)obj.GetPrimitiveValueString().length());
    }
    else if (obj.GetTypeFamily() == eTypeFamilyContainer) {
        CObjectInfoEI elem = obj.BeginElements();
        if (elem.Valid()) {
            CObjectInfo elem_oi = elem.GetElement();
            if (elem_oi.GetTypeFamily() == eTypeFamilyPrimitive &&
                elem_oi.GetPrimitiveValueType() == ePrimitiveValueString) {
                m_Result->SetInt((int)elem_oi.GetPrimitiveValueString().length());
            }
        }
    }
}

bool CMacroFunction_StringLength::x_ValidArguments() const
{
    return (m_Args.size() == 1 && (m_Args[0]->IsString() || m_Args[0]->AreObjects() || m_Args[0]->IsRef()));
}


///////////////////////////////////////////////////////////////////////////////
/// StructCommField(fieldname)
/// Returns the value for the corresponding fieldname
DEFINE_MACRO_FUNCNAME(CMacroFunction_StructCommField, "STRUCTCOMMFIELD")
void CMacroFunction_StructCommField::TheFunction()
{
    // the iterator should be a structured comment
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CUser_object* cuser_obj = dynamic_cast<const CUser_object*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!scope || (cuser_obj && !CComment_rule::IsStructuredComment(*cuser_obj))) 
        return;

    const string& field_name = m_Args[0]->GetString();
    m_Result->SetNotSet();

    if (cuser_obj) {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CUser_object* user_obj = CTypeConverter<CUser_object>::SafeCast(oi.GetObjectPtr());
        CConstRef<CUser_field> cuser_field = user_obj->GetFieldRef(field_name);
        if (cuser_field) {
            CUser_field* user_field = const_cast<CUser_field*>(cuser_field.GetPointer());
            CObjectInfo objInfo(user_field, user_field->GetThisTypeInfo());
            x_AssignReturnValue(objInfo, "data.str");
        }
    }
    else {
        CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
        if (!bsh) return;
        for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_User); desc_it; ++desc_it) {
            if (CComment_rule::IsStructuredComment(desc_it->GetUser())) {
                CConstRef<CUser_field> cuser_field = desc_it->GetUser().GetFieldRef(field_name);
                if (cuser_field) {
                    CUser_field* user_field = const_cast<CUser_field*>(cuser_field.GetPointer());
                    CObjectInfo objInfo(user_field, user_field->GetThisTypeInfo());
                    x_AssignReturnValue(objInfo, "data.str");
                    // find the first one and return
                    break;
                }
            }
        }
    }
}

bool CMacroFunction_StructCommField::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->IsString());
}

///////////////////////////////////////////////////////////////////////////////
/// StructCommDB()
/// Returns the normalized database name of the structured comment
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_StructCommDatabase, "STRUCTCOMMDB")
void CMacroFunction_StructCommDatabase::TheFunction()
{
    // the iterator should be a structured comment
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CUser_object* user_obj = dynamic_cast<const CUser_object*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!obj || !scope || !CComment_rule::IsStructuredComment(*user_obj))
        return;

    string prefix = CComment_rule::GetStructuredCommentPrefix(*user_obj, true);
    if (m_Nested == eNotNested) { // return the string
        m_Result->SetString(prefix);
    }
    else {
        // return a reference to a CMQueryNodeValue of type string
        CRef<CMQueryNodeValue> new_node(new CMQueryNodeValue);
        new_node->SetString(prefix);
        m_Result->SetRef(new_node);
    }
}

bool CMacroFunction_StructCommDatabase::x_ValidArguments() const
{
    return m_Args.empty();
}

///////////////////////////////////////////////////////////////////////////////
/// StructCommFieldName()
/// Returns the set of fieldnames within a structured comment
DEFINE_MACRO_FUNCNAME(CMacroFunction_StructCommFieldname, "STRUCTCOMMFIELDNAME")
void CMacroFunction_StructCommFieldname::TheFunction()
{
    // the iterator should be a structured comment
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CUser_object* cuser_obj = dynamic_cast<const CUser_object*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!obj || !scope || !CComment_rule::IsStructuredComment(*cuser_obj))
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CUser_object* user_obj = CTypeConverter<CUser_object>::SafeCast(oi.GetObjectPtr());

    CMQueryNodeValue::TObs objs;
    for (auto& it : user_obj->SetData()) {
        CRef<CUser_field> field = it;
        if (field->GetLabel().IsStr() &&
            !NStr::Equal(field->GetLabel().GetStr(), "StructuredCommentPrefix") &&
            !NStr::Equal(field->GetLabel().GetStr(), "StructuredCommentSuffix")) {
            CObjectInfo objInfo(field, field->GetThisTypeInfo());
            CObjectInfo label_oi = objInfo.FindClassMember("label").GetMember();
            label_oi = label_oi.GetPointedObject();
            if (label_oi.GetCurrentChoiceVariantIndex() == 2) { // str
                CObjectInfo name = label_oi.GetCurrentChoiceVariant().GetVariant();
                objs.push_back(CMQueryNodeValue::SResolvedField(label_oi, name));
            }
        }
    }
    
    if (objs.empty())
        return;
    if (m_Nested == eNotNested) { // return a standard type value
        m_Result->AssignFromObjectInfo(objs.front().field); // for now, return only one value
    }
    else {
        m_Result->SetObjects(objs);
    }
}

bool CMacroFunction_StructCommFieldname::x_ValidArguments() const
{
    return m_Args.empty();
}

///////////////////////////////////////////////////////////////////////////////
/// INTABLE(field_name, [qual_name,] table_name, col_number(integer), delimiter, merge_delimiter, split_firstcol, convert_multi, merge_firstcols)
/// col_number starts from 1, it represents the column within the table
/// that has the values for the 'field_name' field
/// The last four flags are optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_InTable, "INTABLE");
void CMacroFunction_InTable::TheFunction()
{
    auto arg_nr = m_Args.size();
    bool cont_element = (m_Args[3]->IsInt()); // describing an element of a container
    
    size_t index = cont_element ? 2 : 1;
    const string& table_name = m_Args[index]->GetString();
    int col = (int)m_Args[++index]->GetInt();
    if (col <= 0) {
        NCBI_THROW(CException, eUnknown, "The last argument (column number) should be at least 1");
    }
    string delimiter = m_Args[++index]->GetString();
    bool merge_del = (++index < arg_nr) ? m_Args[index]->GetBool() : false;
    bool split_firstcol = (++index < arg_nr) ? m_Args[index]->GetBool() : false;
    bool convert_multi = (++index < arg_nr) ? m_Args[index]->GetBool() : false;
    bool merge_firstcols = (++index < arg_nr) ? m_Args[index]->GetBool() : false;

    m_DataIter->SetTableToApply(table_name, col - 1, delimiter, merge_del, split_firstcol, convert_multi, merge_firstcols);

    bool found = false;
    if (m_Args[0]->AreStrings()) {
        found = m_DataIter->FindInTable(m_Args[0]->GetStrings());
    }
    else {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CMQueryNodeValue::TObs res_oi;
        if (m_Args[0]->IsString()) {
            if (cont_element) {
                // objects will be returned in m_Result, only when the function is 'nested'
                m_Nested = eNested;  // temporarily
                x_AssignReturnValueFromContainer(oi, m_Args[0]->GetString(), m_Args[1]->GetString());
                res_oi = m_Result->GetObjects();
                // reset the states:
                m_Result->SetNotSet(); 
                m_Nested = eNotNested;
            }
            else if (!GetFieldsByName(&res_oi, oi, m_Args[0]->GetString())) {
                return;
            }
        }
        else if (m_Args[0]->AreObjects()) {
            res_oi = m_Args[0]->GetObjects();
        }

        found = m_DataIter->FindInTable(res_oi);
    }

    m_Result->SetBool(found);
}

bool CMacroFunction_InTable::x_ValidArguments() const
{
    auto arg_nr = m_Args.size();
    if (arg_nr < 4 && arg_nr > 9)
        return false;

    bool first_ok = m_Args[0]->IsString() || 
        m_Args[0]->AreStrings() ||
        m_Args[0]->AreObjects() ||
        m_Args[0]->IsNotSet();
    if (!first_ok) return false;
    if (!m_Args[2]->IsInt() && !m_Args[3]->IsInt())
        return false;
    
    size_t index = (m_Args[2]->IsInt()) ? 2 : 3;

    for (size_t i = 1; i < index; ++i) {
        if (!m_Args[i]->IsString())
            return false;
    }

    if (!m_Args[index]->IsInt() || !m_Args[++index]->IsString())
        return false;
    
    for (size_t i = ++index; i < arg_nr; ++i) {
        if (!m_Args[i]->IsBool())
            return false;
    }
    return true;
}



///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_Contained
/// ISCONTAINED(feat_type)
/// Returns true when the current feature is contained within a feature of feat_type type
DEFINE_MACRO_FUNCNAME(CMacroFunction_Contained, "ISCONTAINEDIN");
static bool s_ContainedIn(const CSeq_loc& loc1, const CSeq_loc& loc2, CScope* scope);

void CMacroFunction_Contained::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!feat || !scope)
        return;

    // works only with tRNA features
    bool contained = false;
    if (feat->IsSetData() && feat->GetData().IsRna() && feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_tRNA) {
        CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
        CSeqFeatData::ESubtype type = NMacroUtil::GetFeatSubtype(m_Args[0]->GetString());
        if (type == CSeqFeatData::eSubtype_tmRNA) {
            for (CFeat_CI feat_it(bsh, SAnnotSelector(CSeqFeatData::eSubtype_tmRNA)); feat_it; ++feat_it) {
                if (s_ContainedIn(feat_it->GetLocation(), feat->GetLocation(), scope)) {
                    contained = true;
                    break;
                }
            }
        }
    }

    m_Result->SetBool(contained);
}

bool CMacroFunction_Contained::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->IsString());
}

bool s_ContainedIn(const CSeq_loc& loc1, const CSeq_loc& loc2, CScope* scope)
{
    TSeqPos start1 = loc1.GetStart(eExtreme_Positional);
    TSeqPos stop1 = loc1.GetStop(eExtreme_Positional);
    TSeqPos start2 = loc2.GetStart(eExtreme_Positional);
    TSeqPos stop2 = loc2.GetStop(eExtreme_Positional);

    if (start1 == stop2 + 1 || start2 == stop1 + 1) {
        // abut
        return false;
    }

    return (sequence::TestForOverlapEx(loc1, loc2, sequence::eOverlap_Contained, scope) >= 0);
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_IllegalDbXref
/// IllegalDBXref(db_xref_object)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_IllegalDbXref, "ILLEGALDBXREF")
void CMacroFunction_IllegalDbXref::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!feat || !scope)
        return;

    const auto& objs = m_Args[0]->GetObjects();
    if (objs.empty() || objs.front().field.GetName() != "Dbtag") {
        m_Result->SetBool(false);
        return;
    }

    _ASSERT(objs.size() == 1);
    CDbtag* tag = CTypeConverter<CDbtag>::SafeCast(objs.front().field.GetObjectPtr());
    validator::CValidator::TDbxrefValidFlags flags =
        validator::CValidator::IsValidDbxref(*tag, false, false);

    bool non_valid = flags | validator::CValidator::eValid;
    m_Result->SetBool(non_valid);
}

bool CMacroFunction_IllegalDbXref::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->AreObjects());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_StructVoucherPart
/// StructVoucherPart(type|object, part)
DEFINE_MACRO_FUNCNAME(CMacroFunction_StructVoucherPart, "STRUCTVOUCHERPART")

static void s_ParseValues(const string& subname, NMacroUtil::EStructVoucherPart part, vector<string>& values)
{
    if (subname.empty()) return;

    string inst, coll, id;
    COrgMod::ParseStructuredVoucher(subname, inst, coll, id);

    switch (part) {
    case NMacroUtil::eSV_Inst: if (!inst.empty()) values.push_back(inst);  break;
    case NMacroUtil::eSV_Coll: if (!coll.empty()) values.push_back(coll);  break;
    case NMacroUtil::eSV_Specid: if (!id.empty()) values.push_back(id); break;
    default: break;
    }
}
void CMacroFunction_StructVoucherPart::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioSource* cbsrc = dynamic_cast<const CBioSource*>(obj.GetPointer());
    if (!cbsrc)
        return;

    size_t index = 1;
    string voucher_type;
    string voucher_part = m_Args[index]->GetString();

    CMQueryNodeValue::TObs res_oi;
    if (m_Args[0]->IsString()) {
        voucher_type = m_Args[0]->GetString();
    }
    else if (m_Args[0]->AreObjects()) {
        res_oi = m_Args[0]->GetObjects();
    }
    else if (m_Args[0]->IsRef()) {
        x_GetObjectsFromRef(res_oi, 0);
    }

    if (voucher_type.empty() && res_oi.empty()) {
        return;
    }

    auto part = NMacroUtil::GetSVPartFromString(voucher_part);
    if (part == NMacroUtil::eSV_Error)
        NCBI_THROW(CException, eUnknown, "  Unrecognized name of structured voucher part: " + voucher_part);

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());

    vector<string> values;
    m_Result->SetNotSet();

    if (!voucher_type.empty()) {
        auto subtype = COrgMod::GetSubtypeValue(voucher_type);
        if (subtype != COrgMod::eSubtype_bio_material &&
            subtype != COrgMod::eSubtype_culture_collection &&
            subtype != COrgMod::eSubtype_specimen_voucher)
            NCBI_THROW(CException, eUnknown, "Invalid voucher type specified: " + voucher_type);

        
        FOR_EACH_ORGMOD_ON_BIOSOURCE(mod_it, *bsrc) {
            auto orgmod = *mod_it;
            if (orgmod->IsSetSubtype() && orgmod->GetSubtype() == subtype && orgmod->IsSetSubname()) {
                s_ParseValues(orgmod->GetSubname(), part, values);
            }
        }
    }
    else {
        for (auto& it : res_oi) {
            if (it.field.GetPrimitiveValueType() == ePrimitiveValueString) {
                s_ParseValues(it.field.GetPrimitiveValueString(), part, values);
            }
        }
    }

    if (!values.empty()) {
        if (m_Nested == eNotNested) {
            m_Result->SetStrings(values);
        }
        else {
            // return a reference to a CMQueryNodeValue of type strings
            CRef<CMQueryNodeValue> new_node(new CMQueryNodeValue);
            new_node->SetStrings(values);
            m_Result->SetRef(new_node);
        }
    }
}

bool CMacroFunction_StructVoucherPart::x_ValidArguments() const
{
    if (m_Args.size() != 2)
        return false;

    bool first_ok = m_Args[0]->IsString() || m_Args[0]->AreObjects() || m_Args[0]->IsRef();
    return first_ok && m_Args[1]->IsString();
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_GeneType
/// GeneType() - used with genes. Returns 'gene' unless the gene is pseudo. In that case, it returns 'pseudogene'.
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_GeneType, "GENETYPE")
void CMacroFunction_GeneType::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    if (!feat || !feat->IsSetData() || !feat->GetData().IsGene())
        return;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_feat* gene_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());

    m_Result->SetNotSet();
    if (gene_feat) {
        string label = (CSeqUtils::IsPseudoFeature(*gene_feat)) ? "pseudogene" : "gene";
        m_Result->SetString(label);
    }
}

bool CMacroFunction_GeneType::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Label
/// Label() - mainly used with features, but it could accept any kind of CObject instance
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_Label, "LABEL")
void CMacroFunction_Label::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    if (!obj)
        return;

    CLabel::ELabelType type = NMacroUtil::NameToLabelType(m_Args[0]->GetString());

    string label;
    CLabel::GetLabel(*obj, &label, type);
    m_Result->SetString(label);
}

bool CMacroFunction_Label::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->IsString());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_VariationType
/// VariationType() - returns type of variation
DEFINE_MACRO_FUNCNAME(CMacroFunction_VariationType, "VARIATIONTYPE")
void CMacroFunction_VariationType::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    if (!feat || feat->GetData().GetSubtype() != CSeqFeatData::eSubtype_variation)
        return;

    string var_class = NSnp::GetBitfield(*feat).GetVariationClassString();
    m_Result->SetString(var_class);
}

bool CMacroFunction_VariationType::x_ValidArguments() const
{
    return m_Args.empty();
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RefAllele
/// RefAllele()
DEFINE_MACRO_FUNCNAME(CMacroFunction_RefAllele, "REFALLELE")
void CMacroFunction_RefAllele::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    if (!feat || feat->GetData().GetSubtype() != CSeqFeatData::eSubtype_variation)
        return;
    
    NSnp::TAlleles alleles;
    CBioseq_Handle bsh(m_DataIter->GetBioseqHandle());
    NSnp::GetAlleles(*feat, alleles, true, &bsh);
    if (alleles.empty()) {
        m_Result->SetNotSet();
    }
    else {
        m_Result->SetString(alleles.front());
    }
}

bool CMacroFunction_RefAllele::x_ValidArguments() const
{
    return m_Args.empty();
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_AltAlleles
/// AltAlleles()
DEFINE_MACRO_FUNCNAME(CMacroFunction_AltAlleles, "ALTALLELES")
void CMacroFunction_AltAlleles::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    if (!feat || feat->GetData().GetSubtype() != CSeqFeatData::eSubtype_variation)
        return;

    string str = m_Args[0]->GetString();
    string sDelimiter = (m_Args.size() == 1) ? m_Args[0]->GetString() : "/";

    NSnp::TAlleles alleles;
    CBioseq_Handle bsh(m_DataIter->GetBioseqHandle());
    NSnp::GetAlleles(*feat, alleles, true, &bsh);
    if (alleles.empty()) {
        m_Result->SetNotSet();
    }
    else {
        string alt_allele;
        for (size_t i = 1; i < alleles.size(); ++i) {
            if (!alt_allele.empty()) {
                alt_allele += sDelimiter;
            }
            alt_allele += alleles[i];
        }
        m_Result->SetString(alt_allele);
    }
}

bool CMacroFunction_AltAlleles::x_ValidArguments() const
{
    if(m_Args.empty()) {
        return true;
    }
    if(m_Args.size() == 1 && m_Args[0]->IsString()) {
        return true;
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_VcfSevenCol
/// VcfSevenCol()
DEFINE_MACRO_FUNCNAME(CMacroFunction_VcfSevenCol, "VcfSevenCol")
void CMacroFunction_VcfSevenCol::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    if (!feat || feat->GetData().GetSubtype() != CSeqFeatData::eSubtype_variation)
        return;
    
    m_Result->SetNotSet();
    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    if (!bsh)
        //!! this can create empty colums, ungood
        return;
    
    string sResult;
    string sAcc;
    CConstRef<CSeq_id> seq_id;
    try {
        CSeq_id_Handle idh = sequence::GetId(*bsh.GetSeqId(), bsh.GetScope(), sequence::eGetId_Best);
        if (idh) {
            seq_id = idh.GetSeqId();
            sAcc = seq_id->GetSeqIdString(true);
        }
    }
    catch (const CException& e) {
        LOG_POST(Error << "CMacroFunction_VcfSevenCol: failed when retrieving accession, " << e.GetMsg());
        return;
    }
    sResult += sAcc + "\t";

    const CSeq_loc& loc = feat->GetLocation();
    TSeqPos pos(loc.GetStart(eExtreme_Positional) + 1);

    string snp_id;
    if (feat->IsSetDbxref()) {
        auto dbtag = feat->GetNamedDbxref("dbSNP");
        if (dbtag && dbtag->IsSetTag()) {
            const auto& tag = dbtag->GetTag();
            if (tag.IsStr()) {
                snp_id = tag.GetStr();
            }
            else if (tag.IsId()) {
                snp_id = "rs" + NStr::IntToString(tag.GetId());
            }
            else if (tag.IsId8()) {
                snp_id = "rs" + NStr::Int8ToString(tag.GetId8());
            }
        }
    }
    NSnp::TAlleles alleles;
    NSnp::GetAlleles(*feat, alleles, true, &bsh);
    if (!alleles.empty()) {
        bool isUsedVarLib(false);
#ifdef USE_VARIATION_LIB
        try {
            if(!seq_id.IsNull() && alleles.size() == 2) {
                NVariation::NPrimitives::CSeqId var_seq_id(NVariation::NPrimitives::CSeqId::FromDatatoolObject(*seq_id));
                NVariation::NVariant::CSpdi spdi(var_seq_id,
                                                    pos-1,
                                                    alleles[0],
                                                    alleles[1]);
                auto seq_cache(NVariation::NPrimitives::ISequenceFactory::CreateDefault(m_DataIter->GetScopedObject().scope)->GetSequence(var_seq_id, 
                                                                                                                     NVariation::NPrimitives::EStrand::Pos));
                auto vcf_var = NVariation::NConstructors::g_VcfFromSpdi(spdi, *seq_cache);
                sResult += NStr::NumericToString(vcf_var.Position()) + "\t";
                sResult += snp_id + "\t";
                sResult += vcf_var.Reference() + "\t" + vcf_var.Alternate() + "\t";
                isUsedVarLib = true;
            }
        } catch(...) {
        }
#endif
        // fallback to regular processing if for some reason we could not get info through SPDI route
        if(!isUsedVarLib) {
            sResult += NStr::NumericToString(pos) + "\t";
            sResult += snp_id + "\t";
            if(alleles.empty()) {
                sResult += ".";
            } else {
                sResult += alleles.front();
            }
            sResult += "\t";
            if(alleles.size() < 2) {
                sResult += ".";
            } else {
                string alt_allele;
                for (size_t i = 1; i < alleles.size(); ++i) {
                    if (!alt_allele.empty()) {
                        alt_allele += ",";
                    }
                    alt_allele += alleles[i];
                }
                sResult += alt_allele;
            }
            sResult += "\t";
        }
    }
    // do not set any specific QUAL and FILTER
    sResult += ".\t.";
    if(!sResult.empty()) {
        m_Result->SetString(sResult);
    }
}

bool CMacroFunction_VcfSevenCol::x_ValidArguments() const
{
    return m_Args.empty();
}
///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Consequence
/// Consequence()
DEFINE_MACRO_FUNCNAME(CMacroFunction_Consequence, "CONSEQUENCE")
void CMacroFunction_Consequence::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    if (!feat || feat->GetData().GetSubtype() != CSeqFeatData::eSubtype_variation)
        return;

    string conseq = NSnp::GetBitfield(*feat).GetGenePropertyString();
    m_Result->SetString(conseq);
}

bool CMacroFunction_Consequence::x_ValidArguments() const
{
    return m_Args.empty();
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_SnpID
DEFINE_MACRO_FUNCNAME(CMacroFunction_SnpID, "SNPID")
void CMacroFunction_SnpID::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    if (!feat || feat->GetData().GetSubtype() != CSeqFeatData::eSubtype_variation)
        return;

    m_Result->SetNotSet();
    if (feat->IsSetDbxref()) {
        auto dbtag = feat->GetNamedDbxref("dbSNP");
        if (dbtag && dbtag->IsSetTag()) {
            string snp_id;
            const auto& tag = dbtag->GetTag();
            if (tag.IsStr()) {
                snp_id = tag.GetStr();
            }
            else if (tag.IsId()) {
                snp_id = "rs" + NStr::IntToString(tag.GetId());
            }
            else if (tag.IsId8()) {
                snp_id = "rs" + NStr::Int8ToString(tag.GetId8());
            }
            m_Result->SetString(snp_id);
        }
    }
}

bool CMacroFunction_SnpID::x_ValidArguments() const
{
    return m_Args.empty();
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
