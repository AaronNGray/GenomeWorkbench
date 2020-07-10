/*  $Id: macro_fn_string_constr.cpp 44155 2019-11-07 16:42:35Z asztalos $
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
#include <objects/macro/Word_substitution_set.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_fn_string_constr.hpp>
#include <gui/objutils/macro_field_resolve.hpp>
#include <gui/objutils/macro_engine.hpp>
#include <gui/objutils/macro_ex.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

//////////////////////////////////////////////////////////////////////////////
/// Classes related to STRING CONSTRAINTS
///
/// Testing whether the string is all upper case, lower case or fully composed of punctuation
/// ISUPPER(fieldname), ISLOWER(fieldname), ISPUNCTUATION(fieldname)
///
/// In case of a modifier, the functions accept two parameters:
/// ISUPPER(asn_path_to_modifier_container, modifier_subtype)
/// 
///
/// String comparison functions with various options:
/// STARTS(field_name, match_text, case_sensitive, ignore_space, ignore_punctuation, 
/// whole_word, ignore_weasel, syn_fname, word, case_sensitive, whole_word)
/// Minimum number of arguments: 2 
///
/// In case of a modifier, number of minimum/possible parameters increases by one:
/// STARTS(asn_path_to_modifier_container, modifier_subtype, match_text, case_sensitive, 
/// ignore_space, ignore_punctuation, whole_word, ignore_weasel, syn_fname, word, case_sensitive, whole_word)
///
/// Any String constraint function can form a composite function 
///
const string CMacroFunction_StringConstraints::sm_syn("synonym");
const char* CMacroFunction_StringConstraints::sm_Upper = "ISUPPER";
const char* CMacroFunction_StringConstraints::sm_UpperAll = "IS_ALL_UPPER";
const char* CMacroFunction_StringConstraints::sm_Lower = "ISLOWER";
const char* CMacroFunction_StringConstraints::sm_LowerAll = "IS_ALL_LOWER";
const char* CMacroFunction_StringConstraints::sm_Punct = "ISPUNCTUATION";
const char* CMacroFunction_StringConstraints::sm_PunctAll = "IS_ALL_PUNCTUATION";
const char* CMacroFunction_StringConstraints::sm_Start = "STARTS";
const char* CMacroFunction_StringConstraints::sm_End = "ENDS";
const char* CMacroFunction_StringConstraints::sm_Equal = "EQUALS";
const char* CMacroFunction_StringConstraints::sm_Contain = "CONTAINS";
const char* CMacroFunction_StringConstraints::sm_InList = "INLIST";

CMacroFunction_StringConstraints::CMacroFunction_StringConstraints(EScopeEnum func_scope,
    const string& type, EStrConstrCase testcase, ELogicType op_type)
    : IEditMacroFunction(func_scope),
    m_TestCase(testcase),
    m_Type(type),
    m_Modifier(false),
    m_Operation(op_type),
    m_Constraint(0)
{
}

void CMacroFunction_StringConstraints::TheFunction()
{
    x_SetModifierFlag();

    // build the string constraint from the argument list
    x_BuildStringConstraint();
    if (!m_Constraint) {
        return;
    }

    // obtain the field name (path to container and subtype in case of a modifier) or obtain the object
    string field_name, container;
    bool match = false;
    if (m_Args[0]->IsString()) {
        field_name = m_Args[0]->GetString();
        if (m_Modifier) {
            container = field_name;                  // asn path to orgmod/subsource modifier container
            field_name = m_Args[1]->GetString(); // modifier subtype
        }

        if (field_name.empty() || (m_Modifier && container.empty()))
            return;

        if (m_Modifier) {
            match = x_CheckConstraintContainer(container, field_name);
        } else {
            match = x_CheckConstraintPrimitive(field_name);
        }
    }
    else if (m_Args[0]->AreStrings()) {
        const vector<string>& strs = m_Args[0]->GetStrings();
        for (auto& it : strs) {
            if (m_Constraint->Match(it)) {
                match = true;
                break;
            }
        }
    }
    else if (m_Args[0]->AreObjects()) {
        // if the first parameter is an object returned by another function
        CMQueryNodeValue& obj = m_Args[0].GetNCObject();
        obj.Dereference();
        CMQueryNodeValue::TObs res_oi = obj.GetObjects();
        NMacroUtil::SwapGbQualWithValues(res_oi);

        match = x_CheckConstraintObjects(res_oi);
    }
    else if (m_Args[0]->IsRef()) {
        m_Args[0]->Dereference();
        if (m_Args[0]->IsString()) {
            match = m_Constraint->Match(m_Args[0]->GetString());
        }
        else if (m_Args[0]->AreObjects()) {
            CMQueryNodeValue::TObs res_oi = m_Args[0]->GetObjects();
            NMacroUtil::SwapGbQualWithValues(res_oi);
            match = x_CheckConstraintObjects(res_oi);
        }
        else if (m_Args[0]->AreStrings()) {
            const vector<string>& strs = m_Args[0]->GetStrings();
            for (auto& it : strs) {
                if (m_Constraint->Match(it)) {
                    match = true;
                    break;
                }
            }
        }
    }
        
    // return a bool type variable
    m_Result->SetDataType(CMQueryNodeValue::eBool);
    m_Result->SetBool(match);
}


bool CMacroFunction_StringConstraints::x_ValidArguments() const
{
    /* For testing only case constraints (isupper, islower and ispunctuation),
       number of parameters should be one (or two, for modifiers)
       In Both cases the first parameter can be a CObjectInfo - occurring in composite functions
    */
    if (m_Args.empty()) {
        return false;
    }
    size_t as = m_Args.size();
    bool first_ok = m_Args[0]->IsString() || 
        m_Args[0]->AreStrings() || 
        m_Args[0]->AreObjects() || 
        m_Args[0]->IsRef();
    
    if (NStr::IsBlank(m_Type)) { // for testing case constraints
        if (as > 2 || !first_ok)
            return false;
        if (as == 2 && !m_Args[1]->IsString())
            return false;
    } else { // for testing more complex string constraints
        if (as < 2 || !first_ok || !(m_Args[1]->IsString() || m_Args[1]->IsInt() || m_Args[1]->IsDouble()))
            return false;
    }
    return true;
}

bool CMacroFunction_StringConstraints::x_IsNestedFunctionReturnValid() const
{
    return !(!m_Args.empty() && m_Args[0]->IsNotSet());
}

void CMacroFunction_StringConstraints::x_SetModifierFlag()
{
    m_Modifier = false; // by default
    if (NStr::IsBlank(m_Type)) { // for testing case constraints
        // set the modifier flag
        if (m_Args.size() == 2) {
            m_Modifier = true;
        }
    }
    else {
        if (m_Args.size() > 2
            && (m_Args[2]->IsString() || m_Args[2]->IsInt() || m_Args[2]->IsDouble())
            && !NStr::StartsWith(m_Args[2]->GetString(), sm_syn)) {
            m_Modifier = true;
        }
    }
}


void  CMacroFunction_StringConstraints::x_BuildStringConstraint()
{
    if (NStr::IsBlank(m_Type)){  // for testing case constraints
        x_BuildCaseStringConstraint();
    } else {
        x_BuildNoCaseStringConstraint();
    }

    // if nothing else is specified, default values will apply
}

void CMacroFunction_StringConstraints::x_BuildCaseStringConstraint()
{
    m_Constraint = Ref(new CString_constraint());
    switch (m_TestCase) {
    case eUpper:
        m_Constraint->SetIs_all_caps(true);
        break;
    case eLower:
        m_Constraint->SetIs_all_lower(true);
        break;
    case ePunct:
        m_Constraint->SetIs_all_punct(true);
        break;
    case eNotSet:
        break;
    }
}

void CMacroFunction_StringConstraints::x_BuildNoCaseStringConstraint()
{
    size_t arg_size = m_Args.size();
    m_Constraint = Ref(new CString_constraint());
    
    size_t check = (m_Modifier) ? 2 : 1; // the index of the parameter corresponding to the match_text
    if (check < arg_size) {
        m_Constraint->SetMatch_text(NMacroUtil::GetStringValue(m_Args[check]));
    } else {
        m_Constraint.Reset();
        return;
    }

    // set the type of string comparison:
    if (NStr::EqualNocase(m_Type, "starts")) {
        m_Constraint->SetMatch_location(eString_location_starts);
    } else if (NStr::EqualNocase(m_Type, "ends")) {
        m_Constraint->SetMatch_location(eString_location_ends);
    } else if (NStr::EqualNocase(m_Type, "contains")) {
        m_Constraint->SetMatch_location(eString_location_contains);
    } else if (NStr::EqualNocase(m_Type, "equals")) {
        m_Constraint->SetMatch_location(eString_location_equals);
    } else if (NStr::EqualNocase(m_Type, "inlist")) {
        m_Constraint->SetMatch_location(eString_location_inlist);
    }
    
    // setting the optional boolean flags
    size_t opt_min = (m_Modifier) ? 3 : 2; // the optional boolean flags 
    size_t opt_max = (m_Modifier) ? 8 : 7;
    size_t  n = opt_min;
    for ( ; n < min(arg_size, opt_max); ++n) {
        if (m_Args[n]->GetDataType() == CMQueryNodeValue::eBool) {
            switch (n) {
                case 2:
                    m_Constraint->SetCase_sensitive(m_Args[n]->GetBool());
                    break;
                case 3:
                    if (m_Modifier) 
                        m_Constraint->SetCase_sensitive(m_Args[n]->GetBool());
                    else 
                        m_Constraint->SetIgnore_space(m_Args[n]->GetBool());
                    break;
                case 4:
                    if (m_Modifier)
                        m_Constraint->SetIgnore_space(m_Args[n]->GetBool());
                    else 
                        m_Constraint->SetIgnore_punct(m_Args[n]->GetBool());
                    break;
                case 5:
                    if (m_Modifier)
                        m_Constraint->SetIgnore_punct(m_Args[n]->GetBool());
                    else
                        m_Constraint->SetWhole_word(m_Args[n]->GetBool());
                    break;
                case 6:
                    if (m_Modifier)
                        m_Constraint->SetWhole_word(m_Args[n]->GetBool());
                    else 
                        m_Constraint->SetIgnore_weasel(m_Args[n]->GetBool());
                    break;
                case 7:
                    if (m_Modifier)
                        m_Constraint->SetIgnore_weasel(m_Args[n]->GetBool());
                    break;
            }
        } else if (m_Args[n]->GetDataType() == CMQueryNodeValue::eString) {
            break;
        } else {
            m_Constraint.Reset();
            return;
        }
    }

    size_t index = 0; // the possible index for finding the synonym list filename
    if (n < arg_size) {
        index = min(opt_max, n);
    }

    if (index) {
        string synonym = m_Args[index]->GetString();  // "synonyms.txt" or "synonyms:"
        if (NStr::StartsWith(synonym, sm_syn)) {
            n = index + 1;
            if ( arg_size < n || m_Args[n]->GetDataType() != CMQueryNodeValue::eString) {
                m_Constraint.Reset();
                return;
            }
            list<CRef<CWord_substitution> > word_subst_list;
            while ( n < arg_size )  {
                if (m_Args[n]->GetDataType() == CMQueryNodeValue::eString) {
                    CRef<CWord_substitution> word_subst = x_ReadSynonymsFor(m_Args[n]->GetString());
                    if (!word_subst) {
                        LOG_POST(Info << "String constraint was reset as no synonyms were found for " << m_Args[n]->GetString());
                        m_Constraint.Reset();
                        return;
                    }
                    if ( n + 1 < arg_size && m_Args[n + 1]->GetDataType() == CMQueryNodeValue::eBool) {
                        word_subst->SetCase_sensitive(m_Args[n + 1]->GetBool());
                        n++;
                    }
                    if (n + 1 < arg_size && m_Args[n + 1]->GetDataType() == CMQueryNodeValue::eBool) {
                        word_subst->SetWhole_word(m_Args[n + 1]->GetBool());
                        n++;
                    }
                    word_subst_list.push_back(word_subst);
                    n++;
                } else {
                    m_Constraint.Reset();
                    return;
                }
            }
            if (!word_subst_list.empty()) {
                m_Constraint->SetIgnore_words().Set() = word_subst_list;
            }
        } else {
            m_Constraint.Reset();
            return;
        }
    }
}


CRef<CWord_substitution> CMacroFunction_StringConstraints::x_ReadSynonymsFor(const string& phrase)
{
    if (NStr::IsBlank(phrase)) {
        return CRef<CWord_substitution>(NULL);
    }

    /* When the match word is prefixed by the "optional-" word it means that the match word is optional
       and has no synonyms. The word-substitution object is:
       Word-substitution::= 
        {
            word "match_word",
            case-sensitive FALSE,
            whole - word FALSE
        }
    */

    /* When the match word starts with "emptystring", the match word is considered to be empty and its
       synonyms are read from the "synonyms.txt" text file.
    */

    const char* kOptional = "optional-";
    const char* kEmpty = "emptystring";

    CRef<CWord_substitution> word_subst(new CWord_substitution());
    if (NStr::StartsWith(phrase, kOptional)) {
        string word = phrase.substr(CTempString(kOptional).size(), NPOS);
        if (!word.empty()) {
            word_subst->SetWord(word);
        }
    }
    else if (NStr::StartsWith(phrase, kEmpty)) {
        word_subst->SetWord(kEmptyStr);
        word_subst->SetSynonyms() = CMacroLib::GetInstance().GetSynonymListForPhrase(phrase);
        if (word_subst->GetSynonyms().empty()) {
            word_subst.Reset();
        }
    }
    else if (NStr::StartsWith(phrase, sm_syn)) {  // phrases like "synonym03-CBS"
        string word = phrase.substr(sm_syn.size() + 3, NPOS);
        if (!word.empty()) {
            word_subst->SetWord(word);
            word_subst->SetSynonyms() = CMacroLib::GetInstance().GetSynonymListForPhrase(phrase);
            if (word_subst->GetSynonyms().empty()) {
                word_subst.Reset();
            }
        }
    }
    else {
        word_subst->SetWord(phrase);
        word_subst->SetSynonyms() = CMacroLib::GetInstance().GetSynonymListForPhrase(phrase);
        if (word_subst->GetSynonyms().empty()) {
            word_subst.Reset();
        }
    }
    return word_subst;
}


bool CMacroFunction_StringConstraints::x_CheckStringConstraint(const CObjectInfo& oi)
{
    if (oi.GetTypeFamily() == eTypeFamilyPrimitive) {
        if (oi.GetPrimitiveValueType() == ePrimitiveValueString && 
            m_Constraint->Match(oi.GetPrimitiveValueString())) {
            return true;
        } else if (oi.GetPrimitiveValueType() == ePrimitiveValueEnum) {
            string value;
            try {
                value = oi.GetPrimitiveValueString();
            } catch(const CException&) {
                value = NStr::NumericToString(oi.GetPrimitiveValueInt4());
            }
            if (m_Constraint->Match(value))
                return true;
        }
    } else if (oi.GetTypeFamily() == eTypeFamilyContainer) { // in case of protein names
        bool constr_sat = false;
        CObjectInfoEI elem = oi.BeginElements();
        while (elem.Valid() && !constr_sat) {
            constr_sat = x_CheckStringConstraint(elem.GetElement());
            ++elem;
        }
        return constr_sat;
    }
    else if (oi.GetTypeFamily() == eTypeFamilyClass) {
        if (NStr::EqualNocase(oi.GetName(), "Name-std")) {
            return x_CheckAuthorNames(oi);
        }
    }
    else if (oi.GetTypeFamily() == eTypeFamilyChoice) {
        CObjectInfoCV var = oi.GetCurrentChoiceVariant();
        return x_CheckStringConstraint(var.GetVariant());
    } 
    else if (oi.GetTypeFamily() == eTypeFamilyPointer) {
        return x_CheckStringConstraint(oi.GetPointedObject());
    }
    return false;
}

bool CMacroFunction_StringConstraints::x_CheckConstraintPrimitive(const string& field)
{
    CMQueryNodeValue::TObs res_oi;
    CObjectInfo objInfo = m_DataIter->GetEditedObject();
    if (!GetFieldsByName(&res_oi, objInfo, field) || res_oi.size() != 1)
        return false;

    return x_CheckStringConstraint(res_oi.front().field);
}

bool CMacroFunction_StringConstraints::x_CheckConstraintObjects(const CMQueryNodeValue::TObs& objs)
{
    if (objs.empty())
        return false;

    // to make ISUPPER() and its relative also work for publication authors
    // IS_ALL_UPPER() will be deprecated
    bool all_authors = true;
    for (auto& it : objs) {
        if (it.field.GetName() != "Name-std") {
            all_authors = false;
            break;
        }
    }

    if (all_authors) {
        m_Operation = eAND;
    }

    if (m_Operation == eOR) {
        // return true if there is at least one field that matches the string constraint
        ITERATE(CMQueryNodeValue::TObs, it, objs) {
            if (x_CheckStringConstraint(it->field))
                return true;
        }
    } else if (m_Operation == eAND) {
        ITERATE(CMQueryNodeValue::TObs, it, objs) {
            if (!x_CheckStringConstraint(it->field))
                return false;
        }
        return true;
    }
    return false;
}

bool CMacroFunction_StringConstraints::x_CheckAuthorNames(const CObjectInfo& oi)
{
    _ASSERT(NStr::EqualNocase(oi.GetName(), "Name-std"));

    bool match(true);
    CObjectInfoMI mem = oi.BeginMembers();
    while (mem.Valid() && match) {
        if (mem.IsSet()) {
            match = m_Constraint->Match(mem.GetMember().GetPrimitiveValueString());
        }
        ++mem;
    }
    return match;
}

// returns true if there is at least one subtype of type "subtype" that satisfies the constraint
bool CMacroFunction_StringConstraints::x_CheckConstraintContainer(const string& container, const string& subtype)
{
    CMQueryNodeValue::TObs res_oi;
    CObjectInfo objinfo = m_DataIter->GetEditedObject();
    if (!GetFieldsByName(&res_oi, objinfo, container) || res_oi.empty()) {
        return false;
    }

    bool constr_sat = false;
    CObjectInfo obj = res_oi.front().field;
    if (obj.GetTypeFamily() == eTypeFamilyContainer) {
        CObjectInfoEI elem = obj.BeginElements();
        // iterating over present modifiers
        while (elem.Valid() && !constr_sat) {
            CObjectInfo oi = elem.GetElement();
            if (oi.GetTypeFamily() == eTypeFamilyPointer) {
                CObjectInfo obj(oi.GetPointedObject());
                if (obj.GetTypeFamily() == eTypeFamilyClass) {
                    constr_sat = x_CheckConstraintWithinClass(obj, subtype);
                } else {
                    NCBI_THROW(CMacroExecException, eVarNotResolved, 
                        "Family type mismatch (class is expected), object type is: " + NStr::IntToString(obj.GetTypeFamily()));
                }
            } else {
                NCBI_THROW(CMacroExecException, eVarNotResolved, 
                    "Family type mismatch (pointer is expected), object type is: " + NStr::IntToString(oi.GetTypeFamily()));
            }
            ++elem;
        }
    }
    return constr_sat;
}

bool CMacroFunction_StringConstraints::x_CheckConstraintWithinClass(const CObjectInfo& obj, const string& subtype)
{
    if (obj.GetTypeFamily() != eTypeFamilyClass)
        return false;
    
    // check whether this object (CObjectInfo) has the right "subtype" (class COrgMod or class CSubSource)
    // the first member is the name of the subtype
    TMemberIndex index = 1;
    CObjectInfo::CMemberIterator mem = obj.GetClassMemberIterator(index);
    if (!mem.Valid()) {
        return false;
    }
    string mem_name = mem.GetMemberInfo()->GetId().GetName();
    CMQueryNodeValue::TObs type_objs;
    if (!GetFieldsByName(&type_objs, obj, mem_name) || type_objs.size() != 1) {
       return false;
    }
    
    string type;
    bool subtype_found = false;
    CObjectInfo oi = type_objs.front().field;
    if (oi.GetTypeFamily() == eTypeFamilyPrimitive) {
        switch (oi.GetPrimitiveValueType()) {
        case ePrimitiveValueString:
        {
            type = oi.GetPrimitiveValueString();
            break;
        }
        case ePrimitiveValueEnum:
        {
            try {
               type = oi.GetPrimitiveValueString();
            } catch(const CException&) {
            type = NStr::NumericToString(oi.GetPrimitiveValueInt4());
            }
            break;
        }
        default:
            break;
        }
        
        if (NStr::EqualNocase(type, subtype)) { // case_insensitive string matching
            subtype_found = true;
        }
            
    } else {
        string msg = oi.GetName() + " class member's type is " + NStr::IntToString(oi.GetTypeFamily());
        NCBI_THROW(CMacroExecException, eVarNotResolved, msg);
    }
    
    if (!subtype_found) 
        return false;
      
    // once the subtype is found, check the value associated to it
    // the second member gives the value of the subtype
    ++index;        
    mem = obj.GetClassMemberIterator(index);
    if (!mem.Valid()) {
        return false;
    }
    
    mem_name = mem.GetMemberInfo()->GetId().GetName();
    type_objs.clear();
    if (!GetFieldsByName(&type_objs, obj, mem_name) || type_objs.size() != 1) {
       return false;
    }
    
    oi = type_objs.front().field;
    if (oi.GetTypeFamily() == eTypeFamilyPrimitive && oi.GetPrimitiveValueType() == ePrimitiveValueString) {
        if (m_Constraint->Match(oi.GetPrimitiveValueString())) { // whether the subtype value matches the constraint
            return true;
        }
    } else {
        string msg = oi.GetName() + " class member's type is " + NStr::IntToString(oi.GetTypeFamily());
        NCBI_THROW(CMacroExecException, eVarNotResolved, msg);
    }

    return false;
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
