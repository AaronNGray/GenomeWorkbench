/*  $Id: macro_fn_do.cpp 45006 2020-05-06 15:50:53Z asztalos $
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

#include <ncbi_pch.hpp>
#include <objects/valid/Comment_rule.hpp>
#include <objects/valid/Comment_set.hpp>

#include <objmgr/util/create_defline.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/writers/write_util.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/report_trim.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/util_cmds.hpp>

#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_field_resolve.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <objtools/edit/apply_object.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

// All DO functions should make changes on the "Edited" object of the BioDataIterator

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveDescriptor
/// RemoveDescriptor()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveDescriptor, "RemoveDescriptor")
const vector<SArgMetaData> CMacroFunction_RemoveDescriptor::sm_Arguments{ SArgMetaData("descriptor_field", CMQueryNodeValue::eString, false) };
void CMacroFunction_RemoveDescriptor::TheFunction()
{
    if (!m_DataIter->IsDescriptor()) {
        return;
    }

    m_DataIter->SetToDelete(true);
    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr() << ": descriptor removed";
    x_LogFunction(log);
}

bool CMacroFunction_RemoveDescriptor::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveSequence
/// RemoveSequence()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveSequence, "RemoveSequence");

void CMacroFunction_RemoveSequence::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    if (!bseq)
        return;

    m_DataIter->SetToDelete(true);
    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr() << ": sequence removed";
    x_LogFunction(log);
}

bool CMacroFunction_RemoveSequence::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveAlignment
/// RemoveAlignment()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveAlignment, "RemoveAlignment");

void CMacroFunction_RemoveAlignment::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_align* align = dynamic_cast<const CSeq_align*>(obj.GetPointer());
    if (!align)
        return;

    m_DataIter->SetToDelete(true);
    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr() << " removed";
    x_LogFunction(log);
}

bool CMacroFunction_RemoveAlignment::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_TrimStringQual
/// TrimStringQual(fieldname, nr_start_chars)
/// Trims nr_start_chars number of characters from the beginning of the field
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_TrimStringQual, "TrimStringQual");
void CMacroFunction_TrimStringQual::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    if (!GetFieldsByName(&res_oi, oi, m_Args[0]->GetString())) {
        return;
    }

    int nr_chars = (int)(m_Args[1]->GetInt());
    if (res_oi.empty() || nr_chars == 0) {
        return;
    }

    NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, res_oi) {
        CMQueryNodeValue::TObs objs;
        NMacroUtil::GetPrimitiveObjectInfos(objs, *it);
        NON_CONST_ITERATE(CMQueryNodeValue::TObs, iter, objs) {
            CObjectInfo obj = iter->field;
            if (obj.GetPrimitiveValueType() == ePrimitiveValueString) {
                string value = obj.GetPrimitiveValueString();
                value.erase(0, nr_chars);
                SetQualStringValue(obj, value);
            }
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();

        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": trimmed " << nr_chars << " characters from " << m_QualsChangedCount << " qualifiers";
        x_LogFunction(log);
    }
}

bool CMacroFunction_TrimStringQual::x_ValidArguments() const
{
    if (m_Args.size() != 2) {
        return false;
    }
    return (m_Args[0]->GetDataType() == CMQueryNodeValue::eString && m_Args[1]->GetDataType() == CMQueryNodeValue::eInt);
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_CorrectGeneticCodes
/// CorrectGeneticCode()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_CorrectGeneticCodes, "CorrectGeneticCode");

void CMacroFunction_CorrectGeneticCodes::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!feat 
        || !(feat->IsSetData() && feat->GetData().IsCdregion()) 
        || feat->HasExceptionText("genetic code exception")
        || !scope) {
        return;
    }

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CBioseq_Handle bsh = GetBioseqForSeqFeat(*feat, *scope);
    if (!bsh)
        return;

    CSeq_feat* edit_feat = CTypeConverter<CSeq_feat>::SafeCast(oi.GetObjectPtr());
    if (!edit_feat) {
        return;
    }
    const CCdregion& cdregion = edit_feat->GetData().GetCdregion(); 
    int cds_gc = cdregion.IsSetCode() ? cdregion.GetCode().GetId() : 0;
    CRef<CGenetic_code> code = edit::GetGeneticCodeForBioseq(bsh);
    if (!code || code->GetId() == cds_gc) {
        return;
    }
    
    edit_feat->SetData().SetCdregion().SetCode(*code);
    m_DataIter->SetModified();

    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr() << ": corrected genetic code";
    x_LogFunction(log);
}

bool CMacroFunction_CorrectGeneticCodes::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveOrgName
/// RemoveOrgName(fieldname) - removes organism name words from the field
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveOrgName, "RemoveOrgName");
const vector<SArgMetaData> CMacroFunction_RemoveOrgName::sm_Arguments
{ SArgMetaData("source_field", CMQueryNodeValue::eString, false)};

void CMacroFunction_RemoveOrgName::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
    if (!bsrc)
        return;

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

    NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, res_oi) {
        CMQueryNodeValue::TObs objs;
        NMacroUtil::GetPrimitiveObjectInfos(objs, *it);
        NON_CONST_ITERATE(CMQueryNodeValue::TObs, iter, objs) {
            CObjectInfo obj = iter->field;
            if (obj.GetPrimitiveValueType() == ePrimitiveValueString) {
                string value = obj.GetPrimitiveValueString();
                if (s_RemoveOrgName(*bsrc, value)) {
                    SetQualStringValue(obj, value);
                }
            }
        }
    }
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();

        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": removed organism name from " << m_QualsChangedCount << " qualifier";
        x_LogFunction(log);
    }
}

bool CMacroFunction_RemoveOrgName::s_RemoveOrgName(const CBioSource& bsrc, string& field)
{
    if (!bsrc.IsSetTaxname() || (bsrc.IsSetTaxname() && bsrc.GetTaxname().empty()) || field.empty())
        return false;

    string taxname = bsrc.GetTaxname();

    vector<CTempString> taxname_words;
    NStr::Split(taxname, " ", taxname_words, NStr::fSplit_Tokenize);

    vector<CTempString> words;
    NStr::Split(field, " ", words, NStr::fSplit_Tokenize);

    for (auto& it : taxname_words) {
        words.erase(remove(words.begin(), words.end(), it), words.end());
    }

    string new_field = NStr::Join(words, " ");
    if (new_field != field) {
        field = new_field;
        return true;
    }
    return false;
}

bool CMacroFunction_RemoveOrgName::x_ValidArguments() const
{
    if (m_Args.size() != 1) return false;
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    return (type == CMQueryNodeValue::eString) || (type == CMQueryNodeValue::eObjects) || (type == CMQueryNodeValue::eRef);
}


///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_TrimJunkFromPrimerSeq
/// TrimJunkFromPrimerSeq(field_name)
/// The field_name specifies one of the primer sequences
///

DEFINE_MACRO_FUNCNAME(CMacroFunction_TrimJunkFromPrimers, "TrimJunkFromPrimerSeq");
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixIInPrimers,"FixIInPrimerSeq");

void CMacroFunction_ModifyPrimerSeq::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;
    if (!GetFieldsByName(&res_oi, oi, m_Args[0]->GetString()))
        return;

    vector<string> orig_seq, new_seq; // for logging

    NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, res_oi){
        CObjectInfo oi = NMacroUtil::GetPrimitiveObjInfo(it->field);
        if (oi.GetPrimitiveValueType() == ePrimitiveValueString) {
            string seq = oi.GetPrimitiveValueString();
            orig_seq.push_back(seq);
            switch (m_Type) {
                case eTrimJunk:
                    if (CPCRPrimerSeq::TrimJunk(seq)) {
                        SetQualStringValue(oi, seq);
                        new_seq.push_back(seq);
                    }
                    break;
                case eFixI:
                    if (CPCRPrimerSeq::Fixi(seq)) {
                        SetQualStringValue(oi, seq);
                        new_seq.push_back(seq);
                    }
                    break;
            }
            
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        for (size_t n = 0; n < new_seq.size(); ++n) {
            log << "On " << m_DataIter->GetBestDescr();
            log << ": changed primer seq from " << orig_seq[n];
            log << " to " << new_seq[n];
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_ModifyPrimerSeq::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eString);
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_FixUSAandStatesAbbrev - fixes for USA country name and state abbreviations
/// FixUSAAndStateAbbreviations()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixUSAandStatesAbbrev, "FixUSAAndStateAbbreviations");

void CMacroFunction_FixUSAandStatesAbbrev::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CPubdesc* const_pubdesc = dynamic_cast<const CPubdesc*>(object.GetPointer());
    const CSubmit_block* const_block = dynamic_cast<const CSubmit_block*>(object.GetPointer());
    
    bool modified = false;
    if (const_pubdesc) {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CPubdesc* pubdesc = CTypeConverter<CPubdesc>::SafeCast(oi.GetObjectPtr());

        CPubdesc::TPub& pub_equiv = pubdesc->SetPub();
        EDIT_EACH_PUB_ON_PUBEQUIV(it, pub_equiv) {
            CPub &pub = **it;
            if (pub.IsSub()) {
                modified |= FixStateAbbreviationsInCitSub(pub.SetSub());
            }
        }

    }
    else if (const_block) {
        CObjectInfo oi = m_DataIter->GetEditedObject();
        CSubmit_block* submit_block = CTypeConverter<CSubmit_block>::SafeCast(oi.GetObjectPtr());

        if (submit_block->IsSetCit()) {
            modified = FixStateAbbreviationsInCitSub(submit_block->SetCit());
        }
        if (submit_block->IsSetContact()) {
            CSubmit_block::TContact& contact = submit_block->SetContact();
            if (contact.IsSetContact() && contact.GetContact().IsSetAffil()) {
                modified |= FixUSAAbbreviationInAffil(contact.SetContact().SetAffil());
                modified |= FixStateAbbreviationsInAffil(contact.SetContact().SetAffil());
            }
        }

    }
    
    if (modified) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": fixed USA and state abbreviations";
        x_LogFunction(log);
    }
}

bool CMacroFunction_FixUSAandStatesAbbrev::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// Usage: 
/// o = Resolve("subtype") Where o.subtype = "country";
/// FixSourceQualCaps(o);
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixSourceQualCaps, "FixSourceQualCaps")
const vector<SArgMetaData> CMacroFunction_FixSourceQualCaps::sm_Arguments
{ SArgMetaData("srcqual_field", CMQueryNodeValue::eString, false) };
void CMacroFunction_FixSourceQualCaps::TheFunction()
{
    CMQueryNodeValue::TObs res_oi;
    x_GetObjectsFromRef(res_oi, 0);
    if (res_oi.empty()) {
        return;
    }
    
    vector<string> orig, fixed; 
    NON_CONST_ITERATE (CMQueryNodeValue::TObs, obj_it, res_oi) {
        // decide whether the object is a SubSource or an OrgMod
        CObjectInfo obj = obj_it->field;
        bool is_subsrc(false);
        if (NStr::EqualNocase(obj.GetName(), "SubSource")) {
            is_subsrc = true;
        } else if (!NStr::EqualNocase(obj.GetName(), "OrgMod")) {
            continue;
        }
        
        const string field_name = (is_subsrc) ? "name" : "subname";
        CObjectInfo oi_field = obj.FindClassMember(field_name).GetMember();
        _ASSERT(oi_field.GetTypeFamily() == eTypeFamilyPrimitive);
        string value = oi_field.GetPrimitiveValueString();
        string new_value = kEmptyStr;
        
        Int4 subtype = obj.FindClassMember("subtype").GetMember().GetPrimitiveValueInt4();
        if (is_subsrc) {
            switch (subtype) {
            case CSubSource::eSubtype_country: {
                bool capitalize_after_colon(false);
                new_value = CCountries::CountryFixupItem(value, capitalize_after_colon);
                break;
            }
            default:
                new_value = CSubSource::FixCapitalization(CSubSource::ESubtype(subtype), value);
                break;
            }
        } else {
            if (subtype != COrgMod::eSubtype_strain) {
                new_value = COrgMod::FixCapitalization(COrgMod::ESubtype(subtype), value);
            }
        }
                
        // update the field
        if (!NStr::IsBlank(new_value) && !NStr::EqualCase( value, new_value )) { 
            orig.push_back(value);
            SetQualStringValue(oi_field, new_value);
            fixed.push_back(new_value);
        }
    }
    
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": ";
        for (size_t i = 0; i < fixed.size(); ++i) {
            log << " corrected " << orig[i] << " to " << fixed[i];
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_FixSourceQualCaps::x_ValidArguments() const
{
    // it will accept a reference to an eObjects node
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eRef);
}

///////////////////////////////////////////////////////////////////////////////
/// o = Resolve("org.orgname.mod") Where o.subtype = "strain";
/// FixMouseStrains(o);
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixMouseStrain, "FixMouseStrains");

void CMacroFunction_FixMouseStrain::TheFunction()
{
    CMQueryNodeValue::TObs res_oi;
    x_GetObjectsFromRef(res_oi, 0);
    if (res_oi.empty()) {
        return;
    }

    vector<string> orig, fixed;
    NON_CONST_ITERATE(CMQueryNodeValue::TObs, obj_it, res_oi) {
        // decide whether the object is a SubSource or an OrgMod
        CObjectInfo obj = obj_it->field;
        if (!NStr::EqualNocase(obj.GetName(), "OrgMod")) {
            continue;
        }

        CObjectInfo oi_field = obj.FindClassMember("subname").GetMember();
        _ASSERT(oi_field.GetTypeFamily() == eTypeFamilyPrimitive);
        string value = oi_field.GetPrimitiveValueString();
        string new_value = kEmptyStr;

        Int4 subtype = obj.FindClassMember("subtype").GetMember().GetPrimitiveValueInt4();
        if (subtype == COrgMod::eSubtype_strain) {
            CObjectInfo oi = m_DataIter->GetEditedObject();
            CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
            if (bsrc && bsrc->IsSetTaxname() && NStr::StartsWith(bsrc->GetTaxname(), "Mus musculus", NStr::eNocase)) {
                string orig_value = value;
                if (FixupMouseStrain(orig_value)) {
                    new_value = orig_value;
                }
            }
        }
            
        // update the field
        if (!NStr::IsBlank(new_value) && !NStr::EqualCase(value, new_value)) {
            orig.push_back(value);
            SetQualStringValue(oi_field, new_value);
            fixed.push_back(new_value);
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": ";
        for (size_t i = 0; i < fixed.size(); ++i) {
            log << " corrected " << orig[i] << " to " << fixed[i];
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_FixMouseStrain::x_ValidArguments() const
{
    // it will accept a reference to an eObjects node
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eRef);
}


///////////////////////////////////////////////////////////////////////////////
/// obj = PUB_AUTHORS("last") WHERE ISUPPER(obj);
/// FixCapsAuthorLastName(obj);
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixAuthorCaps, "FixCapsAuthorLastName");
void CMacroFunction_FixAuthorCaps::TheFunction()
{
    CMQueryNodeValue& obj = m_Args[0].GetNCObject();
    obj.Dereference();
    if (obj.GetDataType() != CMQueryNodeValue::eObjects)
        return;

    CMQueryNodeValue::TObs objects = obj.GetObjects();
    if (objects.empty())
        return;

    // correcting only last names!
    vector<string> orig, fixed;
    for (auto&& it: objects) {
        CObjectInfo oi = it.field;
        if (oi.GetTypeFamily() != eTypeFamilyPrimitive)
            continue;
        
        const string& orig_value = oi.GetPrimitiveValueString();
        string newValue(orig_value); 
        bool fix_abbrev(false), fix_short_words(false);
        bool cap_after_apostrophe(true);
        if (CMacroFunction_FixPubCaps::
            s_FixCapitalizationInElement(newValue, fix_abbrev, fix_short_words, cap_after_apostrophe)) {
            
            orig.push_back(orig_value);
            SetQualStringValue(oi, newValue);
            fixed.push_back(newValue);
        }
    }
    
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": ";
        for (size_t i = 0; i < fixed.size(); ++i) {
            log << " fixed author last name capitalization: from  " << orig[i] << " to " << fixed[i] << "\n";
        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_FixAuthorCaps::x_ValidArguments() const
{
    // it will accept a reference to an eObjects node
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eRef);
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_FixPubCaps - fixes the capitalization in publication title, authors, affiliation, affil-country
/// obj = PUB_TITLE();
/// FixPubCapsTitle(obj, punct_only); - the second parameter is optional, it's default value is false
/// affil_obj = PUB_AFFIL();
/// FixPubCapsAffilCountry(affil_obj, punct_only);
/// affil_obj = PUB_AFFIL();
/// FixPubCapsAffiliation(affil_obj, punct_only);
/// author_obj = PUB_AUTHORS();
/// FixPubCapsAuthors(author_obj, punct_only);
/// obj = PUB_AFFIL();
/// FixPubCapsAffiliation_NOInstDept(obj, punct_only); - fixes caps in all affiliation fields except affil and div
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixPubCapsTitle, "FixPubCapsTitle");
const vector<SArgMetaData> CMacroFunction_FixPubCapsTitle::sm_Arguments { SArgMetaData("punct_only", CMQueryNodeValue::eBool, true) };
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixPubCapsAuthor, "FixPubCapsAuthors");
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixPubCapsAffil, "FixPubCapsAffiliation");
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixPubCapsAffilWithExcept, "FixPubCapsAffiliation_NOInstDept");
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixPubCapsAffilCountry, "FixPubCapsAffilCountry");

void CMacroFunction_FixPubCaps::TheFunction()
{
    CMQueryNodeValue::TObs objects;
    size_t index = 0;
    if (m_Args[index]->GetDataType() == CMQueryNodeValue::eObjects) {
        objects = m_Args[index]->GetObjects();
    }
    else if (m_Args[index]->GetDataType() == CMQueryNodeValue::eRef) {
        x_GetObjectsFromRef(objects, index);
    }
    
    if (objects.empty()) {
        return;
    }
    
    bool punct_only = (++index < m_Args.size()) ? m_Args[index]->GetBool() : false;
    
    switch (m_Type) {
    case eTitle:
    {
        for (auto&& it: objects) {
            CObjectInfo title_oi = it.field;
            if (title_oi.GetTypeFamily() == eTypeFamilyPrimitive) {
                string orig_value = title_oi.GetPrimitiveValueString();
                if (!punct_only) {
                    CSeq_entry_Handle seh = m_DataIter->GetSEH();
                    bool first_is_upper(true);
                    string newValue = s_FixCapitalizationInTitle(orig_value, first_is_upper, seh);
                    if (!NStr::EqualCase(orig_value, newValue) && !NStr::IsBlank(newValue)) {
                        SetQualStringValue(title_oi, newValue);
                    }
                }
            }
        }
        break;
    }
    case eAuthor: 
    {
        for (auto&& it : objects) {
            // it can be either CName_std object or a string
            if (it.field.GetTypeFamily() == eTypeFamilyPrimitive) {
                return;
            }
            else {
                CName_std* std_name = CTypeConverter<CName_std>::SafeCast(it.field.GetObjectPtr());
                if (std_name) {
                    m_QualsChangedCount += s_FixCapitalizationInAuthor(*std_name);
                }
            }
        }
        break;
    }
    case eAffiliation: {
        for (auto&& it : objects) {
            CAffil* affil = CTypeConverter<CAffil>::SafeCast(it.field.GetObjectPtr());
            if (affil) {
                m_QualsChangedCount += s_FixCapsInPubAffil(*affil, punct_only);
            }
        }
        break;
    }
    case eAffilExceptAffilDiv: {
        for (auto&& it : objects) {
            CAffil* affil = CTypeConverter<CAffil>::SafeCast(it.field.GetObjectPtr());
            if (affil) {
                m_QualsChangedCount += s_FixCapsInPubAffil_NoAffilDiv(*affil, punct_only);
            }
        }
        break;
    }
    case eAffilCountry: {
        for (auto&& it : objects) {
            CAffil* affil = CTypeConverter<CAffil>::SafeCast(it.field.GetObjectPtr());
            if (affil) {
                x_FixCapsInPubAffilCountry(*affil, punct_only);
            }
        }
        break;
    }
    }
    
    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": ";
        log << " fixed capitalization in " << m_QualsChangedCount << " publication fields. ";
        x_LogFunction(log);
    }
}

bool CMacroFunction_FixPubCaps::x_ValidArguments() const
{
    size_t as = m_Args.size();
    if (as < 1 || as > 2) {
        return false;
    }

    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    bool first_ok = (type == CMQueryNodeValue::eObjects) || 
                    (type == CMQueryNodeValue::eRef);
    if (!first_ok)
        return false;

    if (as == 2 && m_Args[1]->GetDataType() != CMQueryNodeValue::eBool)
        return false;

    return true;
}

void CMacroFunction_FixPubCaps::x_FixCapsInPubAffilCountry(CAffil& affil, bool punct_only)
{
    if (affil.IsStd() && affil.GetStd().IsSetCountry()) {
        string country = affil.GetStd().GetCountry();
        string new_country = country;
        NStr::ReplaceInPlace(new_country, "  ", " ");
        NStr::TruncateSpacesInPlace(new_country);
        s_FixCapitalizationInCountryString(new_country, punct_only);
        if (!NStr::EqualCase(new_country, country)) {
            affil.SetStd().SetCountry(new_country);
            m_QualsChangedCount++;
        }

        if (!punct_only && NStr::EqualNocase(new_country, "USA")) {
            if (FixStateAbbreviationsInAffil(affil)) {
                m_QualsChangedCount++;
            }
        }
    }
}

bool CMacroFunction_FixPubCaps::s_FixCapitalizationInElement(string& value, bool fix_abbrev, bool fix_short_words, bool cap_after_apostrophe)
{
    string orig(value);
    FixCapitalizationInElement(value);
    if (cap_after_apostrophe) {
        CapitalizeAfterApostrophe(value);
    }
    
    if (fix_short_words) {
        FixShortWordsInElement(value);
    }
    if (fix_abbrev) {
        FixAbbreviationsInElement(value);
    }
    
    return !NStr::EqualCase(orig, value);
}

string CMacroFunction_FixPubCaps::s_FixCapitalizationInTitle(const string& title, bool first_is_upper, const CSeq_entry_Handle& seh)
{
    if (NStr::IsBlank(title))
        return kEmptyStr;
    
    string fixed_title(title);
    ResetCapitalization(fixed_title, first_is_upper);
    FixAbbreviationsInElement(fixed_title);
    FixOrgNames(seh,fixed_title); 
    FixCountryCapitalization(fixed_title);
    
    return fixed_title;
}

// FixCapitalizationInAuthor - sqnutil2.c
Int4 CMacroFunction_FixPubCaps::s_FixCapitalizationInAuthor(CName_std& auth_names)
{
    Int4 modified = 0;
    bool cap_after_apostrophe(true);
    bool fix_abbrev(false), fix_short_words(false);
    if (auth_names.IsSetLast()) {
        if (s_FixCapitalizationInElement(auth_names.SetLast(), fix_abbrev, fix_short_words, cap_after_apostrophe)) {
            modified++;
        }
    }
    if (auth_names.IsSetFirst()) {
        if (s_FixCapitalizationInElement(auth_names.SetFirst(), fix_abbrev, fix_short_words, !cap_after_apostrophe)) {
            modified++;
        }
    }
    // set initials to all caps
    if (auth_names.IsSetInitials()) {
        string orig_initials = auth_names.GetInitials();
        NStr::ToUpper(auth_names.SetInitials());
        if (!NStr::EqualCase(orig_initials, auth_names.GetInitials())) {
            modified++;
        }
    }
    return modified;
}

// FixCapitalizationInCountryStringEx - sqnutil2.c
void CMacroFunction_FixPubCaps::s_FixCapitalizationInCountryString(string& country, bool punct_only) 
{
    if (NStr::IsBlank(country) || NStr::EqualNocase(country, "US"))
        return;
    
    InsertMissingSpacesAfterCommas (country);
    InsertMissingSpacesAfterNo(country);
    if (!punct_only) {
        s_FixCapitalizationInElement(country, true, true, false);
    }
    
    FindReplaceString_CountryFixes(country);
}

Int4 CMacroFunction_FixPubCaps::s_FixCapsInPubAffil_Affil(CAffil& affil, bool punct_only)
{
    if (!affil.IsStd() || !(affil.IsStd() && affil.GetStd().IsSetAffil())) {
        return 0;
    }

    CAffil::C_Std& affil_std = affil.SetStd();
    Int4 modified = 0;

    string affiliation = affil_std.GetAffil();
    NStr::ReplaceInPlace(affiliation, "  ", " ");
    NStr::TruncateSpacesInPlace(affiliation);

    if (!punct_only) {
        s_FixCapitalizationInElement(affiliation, true, true, false);
        FixAffiliationShortWordsInElement(affiliation);
        FixOrdinalNumbers(affiliation);
    }

    FixKnownAbbreviationsInElement(affiliation);
    InsertMissingSpacesAfterCommas(affiliation);
    InsertMissingSpacesAfterNo(affiliation);

    if (!NStr::EqualCase(affiliation, affil_std.GetAffil())) {
        modified++;
        affil_std.SetAffil(affiliation);
    }

    return modified;
}


Int4 CMacroFunction_FixPubCaps::s_FixCapsInPubAffil_Div(CAffil& affil, bool punct_only)
{
    if (!affil.IsStd() || !(affil.IsStd() && affil.GetStd().IsSetDiv())) {
        return 0;
    }

    CAffil::C_Std& affil_std = affil.SetStd();
    Int4 modified = 0;

    string div = affil_std.GetDiv();
    NStr::ReplaceInPlace(div, "  ", " ");
    NStr::TruncateSpacesInPlace(div);

    if (!punct_only) {
        s_FixCapitalizationInElement(div, true, true, false);
        FixAffiliationShortWordsInElement(div);
        FixOrdinalNumbers(div);
    }
    FixKnownAbbreviationsInElement(div);
    InsertMissingSpacesAfterCommas(div);
    InsertMissingSpacesAfterNo(div);

    if (!NStr::EqualCase(div, affil_std.GetDiv())) {
        modified++;
        affil_std.SetDiv(div);
    }

    return modified;
}

Int4 CMacroFunction_FixPubCaps::s_FixCapsInPubAffil_City(CAffil& affil, bool punct_only)
{
    if (!affil.IsStd() || !(affil.IsStd() && affil.GetStd().IsSetCity())) {
        return 0;
    }

    CAffil::C_Std& affil_std = affil.SetStd();
    Int4 modified = 0;

    string city = affil_std.GetCity();
    NStr::ReplaceInPlace(city, "  ", " ");
    NStr::TruncateSpacesInPlace(city);

    if (!punct_only) {
        s_FixCapitalizationInElement(city, false, true, false);
        FixAffiliationShortWordsInElement(city);
        FixOrdinalNumbers(city);
    }
    FixKnownAbbreviationsInElement(city);
    InsertMissingSpacesAfterCommas(city);
    InsertMissingSpacesAfterNo(city);

    if (!NStr::EqualCase(city, affil_std.GetCity())) {
        modified++;
        affil_std.SetCity(city);
    }

    return modified;
}


Int4 CMacroFunction_FixPubCaps::s_FixCapsInPubAffil_Street(CAffil& affil, bool punct_only)
{
    if (!affil.IsStd() || !(affil.IsStd() && affil.GetStd().IsSetStreet())) {
        return 0;
    }

    CAffil::C_Std& affil_std = affil.SetStd();
    Int4 modified = 0;

    string street = affil_std.GetStreet();
    NStr::ReplaceInPlace(street, "  ", " ");
    NStr::TruncateSpacesInPlace(street);
    FixKnownAbbreviationsInElement(street);

    if (!punct_only) {
        s_FixCapitalizationInElement(street, false, true, false);
        FixAffiliationShortWordsInElement(street);
    }
    InsertMissingSpacesAfterCommas(street);
    InsertMissingSpacesAfterNo(street);
    if (!punct_only) {
        FixOrdinalNumbers(street);
    }

    if (!NStr::EqualCase(street, affil_std.GetStreet())) {
        modified++;
        affil_std.SetStreet(street);
    }

    return modified;
}


Int4 CMacroFunction_FixPubCaps::s_FixCapsInPubAffil_Sub(CAffil& affil, bool punct_only)
{
    if (!affil.IsStd() || !(affil.IsStd() && affil.GetStd().IsSetSub())) {
        return 0;
    }

    CAffil::C_Std& affil_std = affil.SetStd();
    Int4 modified = 0;

    string state = affil_std.GetSub();
    NStr::ReplaceInPlace(state, "  ", " ");
    NStr::TruncateSpacesInPlace(state);

    if (punct_only) {
        InsertMissingSpacesAfterCommas(state);
    }
    else {
        if (state.length() == 2) {
            NStr::ToUpper(state);
        }
        else {
            s_FixCapitalizationInElement(state, false, true, false);
            FixAffiliationShortWordsInElement(state);
            InsertMissingSpacesAfterCommas(state);
        }
    }

    if (!NStr::EqualCase(state, affil_std.GetSub())) {
        modified++;
        affil_std.SetSub(state);
    }

    if (FixStateAbbreviationsInAffil(affil)) {
        modified++;
    }

    return modified;
}


Int4 CMacroFunction_FixPubCaps::s_FixCapsInPubAffil_Country(CAffil& affil, bool punct_only)
{
    if (!affil.IsStd() || !(affil.IsStd() && affil.GetStd().IsSetCountry())) {
        return 0;
    }

    CAffil::C_Std& affil_std = affil.SetStd();
    Int4 modified = 0;

    string country = affil_std.GetCountry();
    NStr::ReplaceInPlace(country, "  ", " ");
    NStr::TruncateSpacesInPlace(country);
    if (!punct_only) {
        s_FixCapitalizationInCountryString(country, punct_only);
    }

    if (!NStr::EqualCase(country, affil_std.GetCountry())) {
        modified++;
        affil_std.SetCountry(country);
    }
    
    return modified;
}

// FixCapsInPubAffilEx - sqnutil2.c
Int4 CMacroFunction_FixPubCaps::s_FixCapsInPubAffil(CAffil& affil, bool punct_only) 
{
    if (!affil.IsStd()) {
        return 0;
    }
    Int4 modified = 0;
        
    modified += s_FixCapsInPubAffil_Affil(affil, punct_only);
    modified += s_FixCapsInPubAffil_Div(affil, punct_only);
    modified += s_FixCapsInPubAffil_City(affil, punct_only);
    modified += s_FixCapsInPubAffil_Street(affil, punct_only);
    modified += s_FixCapsInPubAffil_Sub(affil, punct_only);
    modified += s_FixCapsInPubAffil_Country(affil, punct_only);
    
    return modified;
}    

Int4 CMacroFunction_FixPubCaps::s_FixCapsInPubAffil_NoAffilDiv(CAffil& affil, bool punct_only)
{
    if (!affil.IsStd()) {
        return 0;
    }
    Int4 modified = 0;

    modified += s_FixCapsInPubAffil_City(affil, punct_only);
    modified += s_FixCapsInPubAffil_Street(affil, punct_only);
    modified += s_FixCapsInPubAffil_Sub(affil, punct_only);
    modified += s_FixCapsInPubAffil_Country(affil, punct_only);

    return modified;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_FixFormat - fixes the format of lat-lon, collection-date, altitude
/// FixFormat(obj) - obj - run-time variable 
/// 

// Changes in the function and parameter names require changes in the respective
// XRC file used in the macro editor
DEFINE_MACRO_FUNCNAME(CMacroFunction_FixFormat, "FixFormat");

const vector<SArgMetaData>
CMacroFunction_FixFormat::sm_Arguments{ SArgMetaData("field", CMQueryNodeValue::eString, false) };

void CMacroFunction_FixFormat::TheFunction()
{
    CMQueryNodeValue::TObs objects;
    x_GetObjectsFromRef(objects, 0);
    if (objects.empty())
        return;

    vector<string> orig, fixed;
    NON_CONST_ITERATE(CMQueryNodeValue::TObs, it, objects) {
        CObjectInfo oi = it->field;
        // return if it isn't a subsource
        if ( ! NStr::EqualNocase(oi.GetName(), "SubSource")) 
            return;

        Int4 subtype = oi.FindClassMember("subtype").GetMember().GetPrimitiveValueInt4();
        CObjectInfo name = oi.FindClassMember("name").GetMember();
        string orig_value = name.GetPrimitiveValueString();
        string newValue = kEmptyStr;

        switch (subtype) {
            case CSubSource::eSubtype_lat_lon: {
                CObjectInfo objInfo = m_DataIter->GetEditedObject();
                CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(objInfo.GetObjectPtr());
                // change it only if biosource is a descriptor
                if (!bsrc || m_DataIter->IsFeature()) {
                    return;
                }
                newValue = s_FixLatLonFormat(*bsrc, name.GetPrimitiveValueString());
                if (!NStr::IsBlank(newValue)) {
                    orig.push_back(orig_value);
                    SetQualStringValue(name, newValue);
                    fixed.push_back(newValue);
                }
                break;
            } case CSubSource::eSubtype_collection_date: {
                bool ambiguous = false, day_first = false;
                CSubSource::DetectDateFormat(orig_value, ambiguous, day_first);
                if (!ambiguous) {
                    bool month_ambiguous = false;
                    newValue = CSubSource::FixDateFormat(orig_value, !day_first, month_ambiguous);
                }
                else {
                    CMacroBioData_BioSourceIter* bsrc_iter = dynamic_cast<CMacroBioData_BioSourceIter*>(m_DataIter.GetPointer());
                    if (!bsrc_iter) {
                        // this should be enforced
                        return;
                    }

                    if (bsrc_iter->GetCollDateType() == CMacroBioData_BioSourceIter::eCollDateType_NotSet) {
                        // find a date in the record that is non-ambiguous
                        CSeq_entry_Handle seh = m_DataIter->GetSEH();
                        for (CBioseq_CI b_iter(seh, CSeq_inst::eMol_na); b_iter && ambiguous; ++b_iter) {
                            CSeqdesc_CI desc_it(*b_iter, CSeqdesc::e_Source);
                            while (desc_it && ambiguous) {
                                const CBioSource& bsrc = desc_it->GetSource();
                                FOR_EACH_SUBSOURCE_ON_BIOSOURCE(subsrc, bsrc) {
                                    if ((*subsrc)->IsSetSubtype() && (*subsrc)->GetSubtype() == CSubSource::eSubtype_collection_date) {
                                        if ((*subsrc)->IsSetName() && !NStr::EqualCase(orig_value, (*subsrc)->GetName())) {
                                            CSubSource::DetectDateFormat((*subsrc)->GetName(), ambiguous, day_first);
                                        }
                                    }
                                }
                                ++desc_it;
                            }
                        }
                        bsrc_iter->SetCollDateType(ambiguous);
                    }
                        
                    switch (bsrc_iter->GetCollDateType()) {
                    case CMacroBioData_BioSourceIter::eAmbiguous_True: {
                        // if it is still ambiguous, use the existing rules
                        newValue = CSubSource::FixDateFormat(orig_value);
                        break;
                    }
                    case CMacroBioData_BioSourceIter::eAmbiguous_False: {
                        bool month_ambiguous = false;
                        newValue = CSubSource::FixDateFormat(orig_value, !day_first, month_ambiguous);
                        break;
                    }
                    default:
                        break;
                    }
                }
                if (!newValue.empty() && !NStr::EqualCase(orig_value, newValue)) {
                    orig.push_back(name.GetPrimitiveValueString());
                    SetQualStringValue(name, newValue);
                    fixed.push_back(newValue);
                }
                break;
            } case CSubSource::eSubtype_altitude: {
                newValue = s_ConvertAltitudeToMeters(orig_value);
                if (!NStr::EqualNocase(orig_value, newValue)) {
                    orig.push_back(orig_value);
                    SetQualStringValue(name, newValue);
                    fixed.push_back(newValue);
                }
                break;
            }
        }
    }

    if (m_QualsChangedCount) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": ";
        for (size_t i = 0; i < fixed.size(); ++i) {
            if (!NStr::IsBlank(fixed[i]))
                log << " corrected " << orig[i] << " to " << fixed[i];
        }
        x_LogFunction(log);
    }
}

string CMacroFunction_FixFormat::s_ConvertAltitudeToMeters(const string& value)
{
    // converts altitude given in feet or km to meters
    if (NMacroUtil::StringsAreEquivalent(value, "sea level")) {
        return string("0 m");
    }

    return CSubSource::FixAltitude(value);
}

bool CMacroFunction_FixFormat::s_AddAltitudeToSubSourceNote(CBioSource& bsrc, const string& value)
{
    if (NStr::IsBlank(value))
        return false;

    string additional("altitude: " + value);
    bool modified(false);
    EDIT_EACH_SUBSOURCE_ON_BIOSOURCE(subsrc, bsrc) {
        if ((*subsrc)->IsSetSubtype() && (*subsrc)->GetSubtype() == CSubSource::eSubtype_other) {
            if ((*subsrc)->IsSetName()) {
                string new_note = (*subsrc)->GetName() + ";" + additional;
                (*subsrc)->SetName() = new_note;
                modified  = true;
            }
        }
    }

    if (!modified) {
        CRef<CSubSource> new_subsrc(new CSubSource(CSubSource::eSubtype_other, additional));
        bsrc.SetSubtype().push_back(new_subsrc);
        modified = true;
    } 
    return modified;
}

    
string CMacroFunction_FixFormat::s_FixLatLonFormat(CBioSource& bsrc, const string& lat_lon)
{
    if (lat_lon.empty())
        return kEmptyStr;

    bool format_correct = false, precision_correct = false, lat_in_range = false, lon_in_range = false;
    double lat_value = 0.0, lon_value = 0.0;
    string orig_value(lat_lon);
    CSubSource::IsCorrectLatLonFormat(lat_lon, format_correct, precision_correct, lat_in_range, lon_in_range, lat_value, lon_value);

    string fix = kEmptyStr;
    if (!format_correct) {
        fix = CSubSource::FixLatLonFormat(lat_lon, true); // returns empty, if unable to fix
        if (!NStr::IsBlank(fix)) {
            // the fix may have comma and the altitude, so we need to separate them
            SIZE_TYPE pos = NStr::Find(fix, ", ");
            if (pos != NPOS) {
                fix = fix.substr(0, pos);
                string extra_text = fix.substr(pos + 2, NPOS);
                s_AddAltitudeToSubSourceNote(bsrc, extra_text);
            }
        }
    }
    return fix;
}

bool CMacroFunction_FixFormat::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eRef);
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_Resolve
/// Resolves an existing field
DEFINE_MACRO_FUNCNAME(CMacroFunction_Resolve, "Resolve");
void CMacroFunction_Resolve::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    m_Result->SetNotSet();
    if (type == CMQueryNodeValue::eString) {
        ResolveIdentToObjects(oi, m_Args[0]->GetString(), *m_Result);
    }
    else if (type == CMQueryNodeValue::eObjects) {
        m_Result->SetObjects(m_Args[0]->GetObjects());
    }
    else if (type == CMQueryNodeValue::eRef) {
        CMQueryNodeValue::TObs objs;
        x_GetObjectsFromRef(objs, 0);
        if (objs.empty()) {
            m_Result->SetNotSet();
        }
        else {
            m_Result->SetObjects(objs);
        }
    }
}

bool CMacroFunction_Resolve::x_ValidArguments() const
{
    if (m_Args.size() != 1)
        return false;
    CMQueryNodeValue::EType type = m_Args[0]->GetDataType();
    return (type == CMQueryNodeValue::eString
        || type == CMQueryNodeValue::eObjects
        || type == CMQueryNodeValue::eRef);
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ResolveBioSourceQuals
/// Resolves all string qualifiers except the subsource and orgmod modifiers
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ResolveBioSourceQuals, "ResolveBsrcQuals");
void CMacroFunction_ResolveBioSourceQuals::TheFunction()
{
    CConstRef<CObject> object = m_DataIter->GetScopedObject().object;
    const CBioSource* bsrc = dynamic_cast<const CBioSource*>(object.GetPointer());
    if (!bsrc) {
        return;
    }

    vector<string> field_names;
    field_names.push_back("pcr-primers..forward..seq");
    field_names.push_back("pcr-primers..forward..name");
    field_names.push_back("pcr-primers..reverse..seq");
    field_names.push_back("pcr-primers..reverse..name");
    field_names.push_back("org.taxname");
    field_names.push_back("org.common");
    field_names.push_back("org.orgname.lineage");
    field_names.push_back("org.orgname.div");

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs objs;
    CMQueryNodeValue::TObs res_oi;
    ITERATE(vector<string>, it, field_names) {
        if (GetFieldsByName(&res_oi, oi, *it) && !res_oi.empty()) {
            objs.push_back(res_oi.front());
            res_oi.clear();
        }
    }

    if (!objs.empty()) {
        m_Result->SetObjects(objs);
    }
}

bool CMacroFunction_ResolveBioSourceQuals::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ResolveNA
/// Resolves an existing field within the Bioseq
DEFINE_MACRO_FUNCNAME(CMacroFunction_ResolveNASeq, "ResolveNASeq");

void CMacroFunction_ResolveNASeq::TheFunction()
{
    // TO DO: extend it for other iterators than biosource
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioSource* bsrc = dynamic_cast<const CBioSource*>(obj.GetPointer());
    if (!bsrc)
        return;

    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    if (bsh) {
        CBioseq& seq = const_cast<CBioseq&>(bsh.GetCompleteBioseq().GetObject());
        CObjectInfo objInfo(&seq, seq.GetTypeInfo());
        if (!ResolveIdentToObjects(objInfo, m_Args[0]->GetString(), *m_Result)) {
            return;
        }
    }
}

bool CMacroFunction_ResolveNASeq::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eString);
}



///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ChangeSeqInst
/// ChangeSeqInst(fieldname, value);
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ChangeSeqInst, "ChangeSeqInst");
void CMacroFunction_ChangeSeqInst::TheFunction()
{
    // the iterator should be molinfo
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CMolInfo* orig_molinfo = dynamic_cast<const CMolInfo*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!orig_molinfo || !scope) {
        return;
    }

    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    if (!bsh)
        return;

    CConstRef<CBioseq> bseq = bsh.GetCompleteBioseq();
    _ASSERT(bseq);
    
    CRef<CSeq_inst> new_inst(new CSeq_inst());
    new_inst->Assign(bseq->GetInst());
    CObjectInfo objInfo(new_inst.GetPointer(), new_inst->GetTypeInfo());
    
    CMQueryNodeValue::TObs objs;
    const string& field = m_Args[0]->GetString();
    if (!SetFieldsByName(&objs, objInfo, field) || objs.empty()) {
        return;
    }

    CMQueryNodeValue& new_value = *m_Args[1];
    if (SetSimpleTypeValue(objs.front().field, new_value)) {
        m_QualsChangedCount++;
    }
    
    if (m_QualsChangedCount) {
        CRef<CCmdComposite> cmd(new CCmdComposite("Set new seq-inst"));
        CBioseq_Handle bsh = scope->GetBioseqHandle(*bseq);
        CRef<CCmdChangeBioseqInst> chgInst(new CCmdChangeBioseqInst(bsh, *new_inst));
        cmd->AddCommand(*chgInst);
        m_DataIter->RunCommand(cmd, m_CmdComposite);

        CNcbiOstrstream log;
        string best_seqid;
        CWriteUtil::GetBestId(bsh.GetAccessSeq_id_Handle(), *scope, best_seqid);
        log << "Changed " << field << " to " << m_Args[1]->GetString() << " for sequence " << best_seqid;
        x_LogFunction(log);
    }
}

bool CMacroFunction_ChangeSeqInst::x_ValidArguments() const
{
    size_t arg_size = m_Args.size();
    if (arg_size != 2) {
        return false;
    }

    for (size_t i = 0; i < arg_size; ++i) {
        if (m_Args[i]->GetDataType() != CMQueryNodeValue::eString)
            return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveLineageNotes
/// Removes orgmod and subsource notes that are exclusively composed from special words, 
/// appearing in a list, in the lineage or in the taxname 
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveLineageSourceNotes, "RemoveLineageSourceNotes");

void CMacroFunction_RemoveLineageSourceNotes::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CBioSource* bsrc = CTypeConverter<CBioSource>::SafeCast(oi.GetObjectPtr());
    if (!bsrc )
        return;

    vector<string> orig_value; // for logging - consider rewriting it
    FOR_EACH_ORGMOD_ON_BIOSOURCE( orgmod, *bsrc){
        if ((*orgmod)->IsSetSubtype() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_other) {
            if ((*orgmod)->IsSetSubname()) {
                orig_value.push_back((*orgmod)->GetSubname());
            }
        }
    }

    FOR_EACH_SUBSOURCE_ON_BIOSOURCE( subsrc, *bsrc){
        if ((*subsrc)->IsSetSubtype() && (*subsrc)->GetSubtype() == CSubSource::eSubtype_other) {
            if ((*subsrc)->IsSetName()) {
                orig_value.push_back((*subsrc)->GetName());
            }
        }
    }
    
    bool any_removed = bsrc->RemoveLineageSourceNotes();
    if (any_removed) {
        vector<string> new_value;
        FOR_EACH_ORGMOD_ON_BIOSOURCE( orgmod, *bsrc){
            if ((*orgmod)->IsSetSubtype() && (*orgmod)->GetSubtype() == COrgMod::eSubtype_other) {
                if ((*orgmod)->IsSetSubname()) {
                    new_value.push_back((*orgmod)->GetSubname());
                }
            }
        }

        FOR_EACH_SUBSOURCE_ON_BIOSOURCE( subsrc, *bsrc){
            if ((*subsrc)->IsSetSubtype() && (*subsrc)->GetSubtype() == CSubSource::eSubtype_other) {
                if ((*subsrc)->IsSetName()) {
                    new_value.push_back((*subsrc)->GetName());
                }
            }
        }
        size_t i = 0, j =0;
        string msg;
        while (i < orig_value.size()) {
            if (j < new_value.size()) {
                if (NStr::EqualNocase(orig_value[i], new_value[j])) {
                    j++; i++;
                } else {
                    msg.append( "removed note " + orig_value[i] + " where lineage is " + bsrc->GetLineage());
                    i++;
                }
            } else {
                msg.append( "removed note " + orig_value[i] + " where lineage is " + bsrc->GetLineage());
                i++;
            }
        }

        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": " << msg;
        x_LogFunction(log);
    }
}

bool CMacroFunction_RemoveLineageSourceNotes::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RemoveSegGaps
/// RemoveSegGaps(); 
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RemoveSegGaps, "RemoveSegGaps");

void CMacroFunction_RemoveSegGaps::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CSeq_align* align = CTypeConverter<CSeq_align>::SafeCast(oi.GetObjectPtr());
    if (!align)
        return;

    bool modified = s_RemoveSegGaps(*align);
    if (modified) {
        m_DataIter->SetModified();
        CNcbiOstrstream log;
        log << "Removed gaps from the alignment";
        x_LogFunction(log);
    }
}

bool CMacroFunction_RemoveSegGaps::s_RemoveSegGaps(CSeq_align& align)
{
    // handles only DENSEG type
    if (align.IsSetSegs() && align.GetSegs().IsDenseg()) {
        CDense_seg& denseg = align.SetSegs().SetDenseg();
        CDense_seg orig_denseg;
        orig_denseg.Assign(denseg);
        if (!(denseg.IsSetDim() && denseg.IsSetNumseg() && 
            denseg.IsSetIds() && denseg.IsSetStarts() &&
            denseg.IsSetLens())) {
            return false;
        }
        denseg.RemovePureGapSegs();
        denseg.Compact();
        if (!denseg.Equals(orig_denseg)) {
            return true;
        }
    }
    return false;
}

bool CMacroFunction_RemoveSegGaps::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_RmvDuplStructComments
/// Usage: RemoveDuplicateStructComments()
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_RmvDuplStructComments, "RemoveDuplicateStructComments");
void CMacroFunction_RmvDuplStructComments::TheFunction()
{
    // the asn selector should be a sequence
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!bseq || !scope)
        return;
    
    CBioseq_Handle bsh = scope->GetBioseqHandle(*bseq);
    map<CSeq_entry_Handle, set<CConstRef<CSeqdesc> > > duplicates; // duplicate seqdescs
    vector<CConstRef<CSeqdesc> > struct_comments; // all structured comments
    for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_User); desc_it; ++desc_it) {
        const CUser_object& user_obj = desc_it->GetUser();
        if (CComment_rule::IsStructuredComment(user_obj)) {
            struct_comments.push_back(CConstRef<CSeqdesc>(&(*desc_it)));
        }
    }
    
    if (struct_comments.size() < 2)
        return;

    vector<CConstRef<CSeqdesc> >::const_iterator it1 = struct_comments.begin();
    vector<CConstRef<CSeqdesc> >::const_iterator it2(it1);
    while (it1 != struct_comments.end()) {
        it2 = it1;
        ++it2;
        while (it2 != struct_comments.end()) {
            if ((*it2)->Equals((**it1))) {
                CSeq_entry_Handle seh1 = edit::GetSeqEntryForSeqdesc(scope, it1->GetObject());
                CSeq_entry_Handle seh2 = edit::GetSeqEntryForSeqdesc(scope, it2->GetObject());
                // remove duplicate structured comment always from the sequence level
                if (seh1.IsSeq() && seh2.IsSet()) {
                    duplicates[seh1].insert(*it1);
                } else {
                    duplicates[seh2].insert(*it2);
                }
            }
            ++it2;
        }
        ++it1;
    }

    if (duplicates.empty())
        return;

    CRef<CCmdComposite> cmd(new CCmdComposite("Delete duplicate structured comments"));
    size_t count = 0;
    map<CSeq_entry_Handle, set<CConstRef<CSeqdesc> > >::iterator it = duplicates.begin();
    for ( ; it != duplicates.end(); ++it) {
        ITERATE(set<CConstRef<CSeqdesc> >, desc_it, it->second) {
            CIRef<IEditCommand> del_cmd(new CCmdDelDesc(it->first, desc_it->GetObject()));
            cmd->AddCommand(*del_cmd);
            ++count;
        }
    }

    if (cmd) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);
        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": Removed " << count << " duplicate structured comments" ;
        x_LogFunction(log);
    }
}

bool CMacroFunction_RmvDuplStructComments::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ReorderStructComment
/// ReorderStructuredComment();
/// 
DEFINE_MACRO_FUNCNAME(CMacroFunction_ReorderStructComment, "ReorderStructuredComment");
void CMacroFunction_ReorderStructComment::TheFunction()
{
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CUser_object* user = CTypeConverter<CUser_object>::SafeCast(oi.GetObjectPtr());
    if (!user || !CComment_rule::IsStructuredComment(*user)) {
        return;
    }

    string prefix = CComment_rule::GetStructuredCommentPrefix(*user);
    CConstRef<CComment_set> rules = CComment_set::GetCommentRules();
    if (!rules) {
        return;
    }

    try {
        const CComment_rule& rule = rules->FindCommentRule(prefix);
        if (rule.ReorderFields(*user)) {
            m_QualsChangedCount++;
            m_DataIter->SetModified();
            CNcbiOstrstream log;
            log << m_QualsChangedCount << ";reorder structured comment fields";
            x_LogFunction(log);
        }
    }
    catch (const CException& e) {
        LOG_POST(Error << "No rule was found for this prefix: " << prefix);
        LOG_POST(Error << "\n" << e.GetMsg());
    }
}

bool CMacroFunction_ReorderStructComment::x_ValidArguments() const
{
    return (m_Args.empty());
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_MakeBoldXrefs
/// MakeBOLDXrefs();
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_MakeBoldXrefs, "MakeBOLDXrefs");
void CMacroFunction_MakeBoldXrefs::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!bseq || !scope) {
        return;
    }

    string barcode_id = s_GetBarcodeId(*bseq);
    if (barcode_id.empty() || s_HasBarcodeDbXref(*bseq, barcode_id)) {
        return;
    }

    CSeqdesc_CI desc_it(m_DataIter->GetBioseqHandle(), CSeqdesc::e_Source);
    if (!desc_it) {
        return; // don't create new biosource if there is not one already
    }

    CRef<CSeqdesc> edited_desc(new CSeqdesc);
    edited_desc->Assign(*desc_it);
    CRef<CDbtag> bold_xref = s_MakeBarcodeDbXref(barcode_id);
    edited_desc->SetSource().SetOrg().SetDb().push_back(bold_xref);

    CRef<CCmdChangeSeqdesc> edit_cmd(new CCmdChangeSeqdesc(desc_it.GetSeq_entry_Handle(), *desc_it, *edited_desc));
    CRef<CCmdComposite> cmd(new CCmdComposite("Create new BARCODE dbxref"));
    cmd->AddCommand(*edit_cmd);

    m_DataIter->RunCommand(cmd, m_CmdComposite);
    CNcbiOstrstream log;
    log << m_DataIter->GetBestDescr() << ": Created a BARCODE dbxref";
    x_LogFunction(log);
}

bool CMacroFunction_MakeBoldXrefs::x_ValidArguments() const
{
    return (m_Args.empty());
}

string CMacroFunction_MakeBoldXrefs::s_GetBarcodeId(const CBioseq& bseq)
{
    FOR_EACH_SEQID_ON_BIOSEQ(seqid_it, bseq) {
        const CSeq_id& seqid = **seqid_it;
        if (seqid.IsGeneral()) {
            const CDbtag& dbtag = seqid.GetGeneral();
            if (dbtag.IsSetDb()
                && NStr::EqualCase(dbtag.GetDb(), "uoguelph")
                && dbtag.IsSetTag()) {
                CNcbiOstrstream ss_tag;
                dbtag.GetTag().AsString(ss_tag);
                return CNcbiOstrstreamToString(ss_tag);
            }
        }
    }

    return kEmptyStr;
}
static const char* kBOLDDb = "BOLD";

bool CMacroFunction_MakeBoldXrefs::s_HasBarcodeDbXref(const CBioseq& bseq, const string& barcode_id)
{
    CConstRef<CSeqdesc> desc = bseq.GetClosestDescriptor(CSeqdesc::e_Source);
    if (!desc || !desc->GetSource().IsSetOrg()) {
        return false;
    }

    FOR_EACH_DBXREF_ON_ORGREF(dbtag, desc->GetSource().GetOrg()) {
        if ((*dbtag)->IsSetDb() && NStr::EqualCase((*dbtag)->GetDb(), kBOLDDb)) {
            if ((*dbtag)->IsSetTag()) {
                CNcbiOstrstream ss_tag;
                (*dbtag)->GetTag().AsString(ss_tag);
                string current_tag = CNcbiOstrstreamToString(ss_tag);
                if (NStr::EqualCase(current_tag, barcode_id)) {
                    return true;
                }
            }
        }
    }

    return false;
}

CRef<CDbtag> CMacroFunction_MakeBoldXrefs::s_MakeBarcodeDbXref(const string& barcode_id)
{
    CRef<CDbtag> bold_xref(new CDbtag);
    bold_xref->SetDb(kBOLDDb);
    bold_xref->SetTag().SetStr(barcode_id);
    return bold_xref;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_AddProteinTitles
/// AddProteinTitles();
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_AddProteinTitles, "AddProteinTitles");
void CMacroFunction_AddProteinTitles::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!bseq || bseq->IsNa() || !scope) {
        return;
    }

    CBioseq_Handle bsh = scope->GetBioseqHandle(*bseq);
    CSeqdesc_CI desc(bsh, CSeqdesc::e_Title);
    if (desc) {
        return;
    }

    string defline = sequence::CDeflineGenerator().GenerateDefline(bsh, sequence::CDeflineGenerator::fAllProteinNames);
    CRef<CCmdComposite> cmd;
    if (!defline.empty()) {
        cmd = Ref(new CCmdComposite("Instantiate protein title"));
        CRef<CSeqdesc> new_desc(new CSeqdesc());
        new_desc->SetTitle(defline);
        cmd->AddCommand(*CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetSeq_entry_Handle(), *new_desc)));
    }

    if (cmd) {
        m_DataIter->RunCommand(cmd, m_CmdComposite);

        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": instantiated protein title" ;
        x_LogFunction(log);
    }
}

bool CMacroFunction_AddProteinTitles::x_ValidArguments() const
{
    return (m_Args.empty());
}



///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_TrimTerminalNs
/// TrimTerminalNs()
/// Trim ambiguous characters from the ends of the sequences
DEFINE_MACRO_FUNCNAME(CMacroFunction_TrimTerminalNs, "TrimTerminalNs");

void CMacroFunction_TrimTerminalNs::TheFunction()
{
    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;

    if (!bsh || !scope)
        return;

    if (m_DataIter->IsBegin()) {
        m_ProductToCds.clear();
        GetProductToCDSMap(*scope, m_ProductToCds);
    }

    CSequenceAmbigTrimmer::TTrimRuleVec trim_rules;
    trim_rules.push_back(CSequenceAmbigTrimmer::STrimRule{ 1, 0 });

    CReportTrim trimmer(CSequenceAmbigTrimmer::eMeaningOfAmbig_OnlyCompletelyUnknown, 0, trim_rules, 50);   

    CRef<CBioseq> bseq( new CBioseq );
    bseq->Assign(*bsh.GetCompleteBioseq());

    CBioseq_Handle edited_bsh = scope->AddBioseq(*bseq);

    TSignedSeqPos from = 0;
    TSignedSeqPos to = bsh.GetInst_Length() - 1;
    CSequenceAmbigTrimmer::EResult eTrimResult = CSequenceAmbigTrimmer::eResult_NoTrimNeeded;
    
    try {
        eTrimResult = trimmer.Trim( edited_bsh, from, to );
    }
    catch (const CException&) {}

    if (eTrimResult == CSequenceAmbigTrimmer::eResult_SuccessfullyTrimmed) {
        CNcbiOstrstream log;
        if ((from == 0) && (to == bsh.GetInst_Length() - 1)) {
            m_DataIter->SetToDelete(true);
            log << m_DataIter->GetBestDescr() << ": removed";
        }
        else {
            CRef<CCmdComposite> composite(new CCmdComposite("Trim Terminal Ns"));
            CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, bseq->SetInst()));
            composite->AddCommand(*cmd);


            // Here is a trick - watch the hands carefully. We want to retranslate based on the new Inst, but it's not in scope yet.
            // We want to keep the original Inst so that Undo command works properly.
            CRef<CSeq_inst> orig_inst(new CSeq_inst());
            orig_inst->Assign(bsh.GetInst());
            CBioseq_EditHandle bseh = bsh.GetEditHandle();
            bseh.SetInst(bseq->SetInst());
            CScope& new_scope = bseh.GetScope();
            int length = edited_bsh.GetInst_Length();

            trimmer.AdjustAndTranslate(bsh, length, from, to, composite, new_scope, m_ProductToCds);
            bseh.SetInst(*orig_inst);
            if (bseq->IsSetId())
            {
                trimmer.TrimAlignments(composite, bsh, bseq->GetId(), from, to);
                trimmer.TrimSeqGraphs(composite, bsh, bseq->GetId(), from, to, length);
            }

            if (from != 0)
                log << m_DataIter->GetBestDescr() << ":1-" << from;
            if (to != bsh.GetInst_Length() - 1)
                log << m_DataIter->GetBestDescr() << ":" << to + 2 << "-" << bsh.GetInst_Length();
            m_DataIter->RunCommand(composite, m_CmdComposite);

        }
        x_LogFunction(log);
    }
}

bool CMacroFunction_TrimTerminalNs::x_ValidArguments() const
{
    return (m_Args.empty());
}


///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_PrintCSV
/// Print(field_name|object, delimiter)
/// Prints the argument's value to the output stream and appends a delimiter
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_PrintCSV, "Print")
void CMacroFunction_PrintCSV::TheFunction()
{
    if (!m_DataIter->IsSetOutputStream())
        return;

    CNcbiOstream& ostr = m_DataIter->GetOutputStream();
    const string& del = (m_Args.size() == 2) ? m_Args[1]->GetString() : kEmptyStr;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;

    if (m_Args[0]->IsString()) {
        GetFieldsByName(&res_oi, oi, m_Args[0]->GetString());
    }
    else if (m_Args[0]->AreObjects()) {
        res_oi = m_Args[0]->GetObjects();
    }
    else if (m_Args[0]->IsRef()) {
        CMQueryNodeValue& val = m_Args[0].GetNCObject();
        val.Dereference();
        if (val.AreObjects()) {
            res_oi = val.GetObjects();
        }
        else if (val.IsInt()) {
            ostr << NMacroUtil::TransformForCSV(NStr::Int8ToString(val.GetInt())) << del;
            return;
        }
        else if (val.IsString()) {
            ostr << NMacroUtil::TransformForCSV(val.GetString()) << del;
            return;
        }
    }

    if (res_oi.empty()) {
        ostr << del;
        return;
    }

    for (auto& it : res_oi) {
        CMQueryNodeValue::TObs objs;
        NMacroUtil::GetPrimitiveObjectInfos(objs, it);
        for (auto& iter : objs) {
            CObjectInfo oi = iter.field;
            if (oi.GetPrimitiveValueType() == ePrimitiveValueString) {
                ostr << NMacroUtil::TransformForCSV(oi.GetPrimitiveValueString()) << del;
            }
            else if (oi.GetPrimitiveValueType() == ePrimitiveValueInteger) {
                ostr << NMacroUtil::TransformForCSV(NStr::Int8ToString(oi.GetPrimitiveValueInt8())) << del;
            }

            else if (oi.GetPrimitiveValueType() == ePrimitiveValueEnum) {
                string value;
                try {
                    ostr << NMacroUtil::TransformForCSV(oi.GetPrimitiveValueString()) << del;
                }
                catch (const CException&) {
                    ostr << NMacroUtil::TransformForCSV(NStr::IntToString(oi.GetPrimitiveValueInt4())) << del;
                }
            }
        }
        //NOTE: a del character is inserted if there are multiple values
        //TODO
    }
}

bool CMacroFunction_PrintCSV::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 2)
        return false;

    bool first_ok = (m_Args[0]->IsString() || m_Args[0]->AreObjects() || m_Args[0]->IsRef());
    if (!first_ok)
        return false;
    
    if (m_Args.size() == 2 && !m_Args[1]->IsString()) {
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_PrintTSV
/// PrintTSV(field_name|object, delimiter)
/// Print function for tab separated values
DEFINE_MACRO_FUNCNAME(CMacroFunction_PrintTSV, "PrintTSV")
void CMacroFunction_PrintTSV::TheFunction()
{
    if (!m_DataIter->IsSetOutputStream())
        return;

    CNcbiOstream& ostr = m_DataIter->GetOutputStream();
    const string& del = (m_Args.size() == 2) ? NStr::ParseEscapes(m_Args[1]->GetString()) : kEmptyStr;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;

    NStr::TPrintableMode mode = NStr::fNewLine_Quote | NStr::fNonAscii_Passthru;
    size_t index = 0;
    if (m_Args[index]->IsString()) {
        GetFieldsByName(&res_oi, oi, m_Args[index]->GetString());
    }
    else if (m_Args[index]->AreObjects()) {
        res_oi = m_Args[index]->GetObjects();
    }
    else if (m_Args[index]->IsRef()) {
        CMQueryNodeValue& val = m_Args[index].GetNCObject();
        val.Dereference();
        if (val.AreObjects()) {
            res_oi = val.GetObjects();
        }
        else if (val.IsInt()) {
            ostr << NStr::PrintableString(NStr::Int8ToString(val.GetInt()), mode) << del;
            return;
        }
        else if (val.IsString()) {
            ostr << NStr::PrintableString(val.GetString(), mode) << del;
            return;
        }
    }

    if (res_oi.empty()) {
        ostr << del;
        return;
    }

    for (auto& it : res_oi) {
        CMQueryNodeValue::TObs objs;
        NMacroUtil::GetPrimitiveObjectInfos(objs, it);
        for (auto& iter : objs) {
            CObjectInfo oi = iter.field;
            if (oi.GetPrimitiveValueType() == ePrimitiveValueString) {
                ostr << NStr::PrintableString(oi.GetPrimitiveValueString(), mode) << del;
            }
            else if (oi.GetPrimitiveValueType() == ePrimitiveValueInteger) {
                ostr << NStr::PrintableString(NStr::Int8ToString(oi.GetPrimitiveValueInt8()), mode) << del;
            }

            else if (oi.GetPrimitiveValueType() == ePrimitiveValueEnum) {
                string value;
                try {
                    ostr << NStr::PrintableString(oi.GetPrimitiveValueString(), mode) << del;
                }
                catch (const CException&) {
                    ostr << NStr::PrintableString(NStr::IntToString(oi.GetPrimitiveValueInt4()), mode) << del;
                }
            }
        }
        //NOTE: a del character is inserted if there are multiple values
        //TODO
    }
}

bool CMacroFunction_PrintTSV::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 2)
        return false;

    bool first_ok = (m_Args[0]->IsString() || m_Args[0]->AreObjects() || m_Args[0]->IsRef());
    if (!first_ok)
        return false;

    if (m_Args.size() == 2 && !m_Args[1]->IsString()) {
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_PrintBankit
/// PrintBankit(field_name|object, multiple_del, append_del)
/// The last two parameters are optional
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_PrintBankit, "PrintBankit")
void CMacroFunction_PrintBankit::TheFunction()
{
    if (!m_DataIter->IsSetOutputStream())
        return;

    CNcbiOstream& ostr = m_DataIter->GetOutputStream();
    size_t index = 1;
    string multiple_del = (index < m_Args.size()) ? m_Args[index]->GetString() : kEmptyStr;
    string append_del = (++index < m_Args.size()) ? NStr::ParseEscapes(m_Args[index]->GetString()) : kEmptyStr;

    CObjectInfo oi = m_DataIter->GetEditedObject();
    CMQueryNodeValue::TObs res_oi;

    index = 0;
    if (m_Args[index]->IsString()) {
        GetFieldsByName(&res_oi, oi, m_Args[index]->GetString());
    }
    else if (m_Args[index]->AreObjects()) {
        res_oi = m_Args[index]->GetObjects();
    }
    else if (m_Args[index]->IsRef()) {
        CMQueryNodeValue& val = m_Args[index].GetNCObject();
        val.Dereference();
        if (val.AreObjects()) {
            res_oi = val.GetObjects();
        }
        else if (val.IsInt()) {
            ostr << NStr::Int8ToString(val.GetInt()) << append_del;
            return;
        }
        else if (val.IsString()) {
            ostr << val.GetString() << append_del;
            return;
        }
    }

    if (res_oi.empty()) {
        ostr << append_del;
        return;
    }

    bool appended = false;
    for (auto& it : res_oi) {
        CMQueryNodeValue::TObs objs;
        NMacroUtil::GetPrimitiveObjectInfos(objs, it);
        for (auto& iter : objs) {
            CObjectInfo oi = iter.field;
            if (oi.GetPrimitiveValueType() == ePrimitiveValueString) {
                if (appended) ostr << multiple_del;
                ostr << oi.GetPrimitiveValueString();
                appended = true;
            }
            else if (oi.GetPrimitiveValueType() == ePrimitiveValueInteger) {
                if (appended) ostr << multiple_del;
                ostr << NStr::Int8ToString(oi.GetPrimitiveValueInt8());
                appended = true;
            }

            else if (oi.GetPrimitiveValueType() == ePrimitiveValueEnum) {
                string value;
                try {
                    if (appended) ostr << multiple_del;
                    ostr << oi.GetPrimitiveValueString();
                    appended = true;
                }
                catch (const CException&) {
                    if (appended) ostr << multiple_del;
                    ostr << NStr::IntToString(oi.GetPrimitiveValueInt4());
                    appended = true;
                }
            }
        }
    }

    if (appended) {
        ostr << append_del;
    }
}

bool CMacroFunction_PrintBankit::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 3)
        return false;

    bool first_ok = (m_Args[0]->IsString() || m_Args[0]->AreObjects() || m_Args[0]->IsRef());
    if (!first_ok)
        return false;

    for (size_t index = 1; index < m_Args.size(); ++index) {
        if (!m_Args[index]->IsString())
            return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_PrintLiteral
/// PrintLiteral("literal", print_once)
/// Prints a string literal. The last parameter is optional, by default is false. 
/// If it's true, it prints only once, at the beginning of the iteration
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_PrintLiteral, "PrintLiteral")
void CMacroFunction_PrintLiteral::TheFunction()
{
    if (!m_DataIter->IsSetOutputStream())
        return;

    CNcbiOstream& ostr = m_DataIter->GetOutputStream();
    string str = m_Args[0]->GetString();
    bool print_once = (m_Args.size() == 2) ? m_Args[1]->GetBool() : false;
    try {
        if (print_once) {
            if (m_DataIter->IsBegin()) {
                ostr << NStr::ParseEscapes(str);
            }
        }
        else {
            ostr << NStr::ParseEscapes(str);
        }
    }
    catch (const CException&) {
        LOG_POST(Error << "CMacroFunction_PrintLiteral: could not print literal " << str);
    }
}

bool CMacroFunction_PrintLiteral::x_ValidArguments() const
{
    if (m_Args.empty() || m_Args.size() > 2)
        return false;

    if (!m_Args[0]->IsString())
        return false;
    if (m_Args.size() == 2 && !m_Args[1]->IsBool())
        return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_ToUnknownLengthGap
/// ConvertGapsByGapSize(gap_length)
///
DEFINE_MACRO_FUNCNAME(CMacroFunction_ToUnknownLengthGap, "ConvertGapsByGapSize")
void CMacroFunction_ToUnknownLengthGap::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(obj.GetPointer());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!bseq || !bseq->IsNa() || !scope) {
        return;
    }
    if (m_Args[0]->GetInt() <= 0)
        NCBI_THROW(CException, eUnknown, "The gap size should be a positive number instead of " + NStr::Int8ToString(m_Args[0]->GetInt()));

    const TSeqPos gap_len = (TSeqPos)(m_Args[0]->GetInt());
    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    if (!bsh)
        return;
    CRef<CSeq_inst> new_inst(new CSeq_inst());
    new_inst->Assign(bsh.GetInst());
    if (new_inst->IsSetRepr() && 
        new_inst->GetRepr() == CSeq_inst::eRepr_delta &&
        new_inst->IsSetExt() && 
        new_inst->GetExt().IsDelta()) {

        auto literals = new_inst->SetExt().SetDelta().Set();
        for (auto&& it : literals) {
            if (it->IsLiteral()) {
                auto& lit = it->SetLiteral();
                if (lit.IsSetSeq_data() && lit.GetSeq_data().IsGap() &&
                    lit.IsSetLength() && lit.GetLength() == gap_len && !lit.IsSetFuzz()) {
                    lit.SetFuzz().SetLim(CInt_fuzz::eLim_unk);
                    m_QualsChangedCount++;
                }
            }
        }
    }

    if (m_QualsChangedCount) {
        CRef<CCmdComposite> cmd(new CCmdComposite("Change bioseq-inst"));
        CRef<CCmdChangeBioseqInst> chgInst(new CCmdChangeBioseqInst(bsh, *new_inst));
        cmd->AddCommand(*chgInst);
        m_DataIter->RunCommand(cmd, m_CmdComposite);

        CNcbiOstrstream log;
        log << m_DataIter->GetBestDescr() << ": converted " << m_QualsChangedCount << " estimated length gap of size " << gap_len << " to unknown length gap";
        x_LogFunction(log);
    }
}

bool CMacroFunction_ToUnknownLengthGap::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->IsInt());
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
