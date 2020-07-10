/*  $Id: macro_field_resolve.cpp 44065 2019-10-18 15:32:55Z asztalos $
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
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbimisc.hpp>
#include <gui/objutils/macro_field_resolve.hpp>

#undef _TRACE
#define _TRACE(arg) ((void)0)

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

static const char* kdot = ".";

static void s_Start_GetFields(string& path, const CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found);

static void s_ResolveFieldsOfClass(string& path, const CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found)
{
    string member, remainder;
    NStr::SplitInTwo(path, kdot, member, remainder);

    CObjectInfoMI mem = info.FindClassMember(member);
    if (!mem.Valid()) {
        found = false;
        return;
    }

    if (NStr::IsBlank(remainder)) {
        found = true;
        if (mem.IsSet()) {
            results->push_back(CMQueryNodeValue::SResolvedField(info, mem.GetMember()));
        } 
    } else {
        if (mem.IsSet()) {
            path = remainder;
            s_Start_GetFields(path, mem.GetMember(), results, found);
        }
    }
}


static void s_ResolveFieldsOfChoice(string& path, const CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found)
{
    string member, remainder;
    NStr::SplitInTwo(path, kdot, member, remainder);

    CObjectInfoCV curr_var = info.GetCurrentChoiceVariant();
    string variant = curr_var.GetVariantInfo()->GetId().GetName();

    if (NStr::IsBlank(remainder)) {
        if (NStr::EqualNocase(member, variant)) {
            found = true;
            results->push_back(CMQueryNodeValue::SResolvedField(info, curr_var.GetVariant()));
        }
    } else {
        if (NStr::EqualNocase(member, variant)) {
            path = remainder;
            s_Start_GetFields(path, curr_var.GetVariant(), results, found);
        }
    }
}

static void s_ResolveFieldsOfContainer(string& path, const CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found)
{
    CObjectInfoEI e = info.BeginElements();
    string previous_path = path;
    while (e.Valid()) {
        path = previous_path;
        s_Start_GetFields(path, e.GetElement(), results, found);
        ++e;
    }
}

static void s_ResolveFieldsOfPointer(string& path, const CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found)
{
    s_Start_GetFields(path, info.GetPointedObject(), results, found);
}

// static
void s_Start_GetFields(string& path, const CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found)
{
    if (NStr::IsBlank(path)) {
        return;
    }

    switch (info.GetTypeFamily()) {
    default:
        LOG_POST("ERROR: unknown family type");
        break;
    case eTypeFamilyPrimitive:
        break;
    case eTypeFamilyClass:
        s_ResolveFieldsOfClass(path, info, results, found);
        break;
    case eTypeFamilyChoice:
        s_ResolveFieldsOfChoice(path, info, results, found);
        break;
    case eTypeFamilyContainer:
        s_ResolveFieldsOfContainer(path, info, results, found);
        break;
    case eTypeFamilyPointer:
        s_ResolveFieldsOfPointer(path, info, results, found);
        break;
    }
}


static void s_Start_SetFields(string& path, CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found);

static void s_SetFieldsOfClass(string& path, CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found)
{
    string member, remainder;
    NStr::SplitInTwo(path, kdot, member, remainder);

    CObjectInfoMI mem_it = info.FindClassMember(member);
    if (!mem_it.Valid()) {
        // there is no such member
        found = false;
        return;
    }

    if (NStr::IsBlank(remainder)) {
        CObjectInfo tmp_info(info.SetClassMember(mem_it.GetMemberIndex()));
        results->push_back(CMQueryNodeValue::SResolvedField(info, tmp_info));
        found = true;
    } else {
        path = remainder;
        CObjectInfo tmp_info(info.SetClassMember(mem_it.GetMemberIndex()));
        s_Start_SetFields(path, tmp_info, results, found);
    }
}

static void s_SetFieldsOfChoice(string& path, CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found)
{
    string member, remainder;
    NStr::SplitInTwo(path, kdot, member, remainder);

    TMemberIndex index = info.GetCurrentChoiceVariantIndex();
    const CChoiceTypeInfo* choice_type = CTypeConverter<CChoiceTypeInfo>::SafeCast(info.GetTypeInfo());
    if (index < choice_type->GetVariants().FirstIndex()) {
        CObjectInfo tmp_info(info.SetChoiceVariant(info.FindVariantIndex(member)));
        if (NStr::IsBlank(remainder)) {
            found = true;
            results->push_back(CMQueryNodeValue::SResolvedField(info, tmp_info));
        }
        else {
            path = remainder;
            s_Start_SetFields(path, tmp_info, results, found);
        }
    } else {
        CObjectInfoCV curr_var = info.GetCurrentChoiceVariant();
        string variant = curr_var.GetVariantInfo()->GetId().GetName();
        if (NStr::IsBlank(remainder)) {
            if (NStr::EqualNocase(member, variant)) {
                found = true;
                results->push_back(CMQueryNodeValue::SResolvedField(info, curr_var.GetVariant()));
            }
        } else {
            if (NStr::EqualNocase(member, variant)) {
                path = remainder;
                CObjectInfo tmp(curr_var.GetVariant());
                s_Start_SetFields(path, tmp, results, found);
            }
        }
    }
}


static void s_SetFieldsOfContainer(string& path, CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found)
{
    // first, check if the element already exists in the container, otherwise add a new one
    // this is the case when the elements are choices
    CObjectInfoEI elem = info.BeginElements();
    while (elem.Valid()) {
        CObjectInfo oi = elem.GetElement();
        if (oi.GetTypeFamily() == eTypeFamilyPointer) {
            oi = oi.GetPointedObject();
        }

        if (oi.GetTypeFamily() == eTypeFamilyChoice) {
            CObjectInfoCV var = oi.GetCurrentChoiceVariant();
            string variant = var.GetVariantInfo()->GetId().GetName();
            if (NStr::StartsWith(path, variant)) {
                s_Start_SetFields(path, oi, results, found);
                return;
            }
        }
        else if (oi.GetTypeFamily() == eTypeFamilyClass) {
            s_Start_SetFields(path, oi, results, found);
            return;
        }
        ++elem;
    }

    // add a new one
    if (info.GetElementType().GetTypeFamily() == eTypeFamilyPointer) {
        CObjectInfo tmp_info(info.AddNewPointedElement());
        s_Start_SetFields(path, tmp_info, results, found);
    }
    else {
        CObjectInfo tmp_info(info.AddNewElement());
        s_Start_SetFields(path, tmp_info, results, found);
    }
}

static void s_SetFieldsOfPointer(string& path, CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found)
{
    CObjectInfo tmp_info(info.SetPointedObject());
    s_Start_SetFields(path, tmp_info, results, found);
}


//static
void s_Start_SetFields(string& path, CObjectInfo& info, CMQueryNodeValue::TObs* results, bool& found)
{
    if (found) {
        return;
    }
    switch (info.GetTypeFamily()) {
    default:
        LOG_POST("ERROR: unknown family type");
        break;
    case eTypeFamilyPrimitive:
        break;
    case eTypeFamilyClass:
        s_SetFieldsOfClass(path, info, results, found);
        break;
    case eTypeFamilyChoice:
        s_SetFieldsOfChoice(path, info, results, found);
        break;
    case eTypeFamilyContainer:
        s_SetFieldsOfContainer(path, info, results, found);
        break;
    case eTypeFamilyPointer:
        s_SetFieldsOfPointer(path, info, results, found);
        break;
    }
}

bool GetFieldsByName(CMQueryNodeValue::TObs* results, const CObjectInfo& oi_i, const string& field_name)
{
    if (NStr::IsBlank(field_name)) {
        results->push_back(CMQueryNodeValue::SResolvedField(CObjectInfo(), CObjectInfo(oi_i)));
        return true;
    }

    string path = field_name;
    bool found = false;

    s_Start_GetFields(path, oi_i, results, found);

    if (found && results->empty()) {
        return true; // the choice variant or the class member is not set
    }
    if (!results->empty()) {
        return true;
    }
    return false;
}

bool SetFieldsByName(CMQueryNodeValue::TObs* results, CObjectInfo& oi_i, const string& field_name)
{
    if (NStr::IsBlank(field_name)) {
        results->push_back(CMQueryNodeValue::SResolvedField(CObjectInfo(), CObjectInfo(oi_i)));
        return true;
    }

    string path = field_name;
    bool found = false;

    s_Start_SetFields(path, oi_i, results, found);

    if (found && results->empty()) {
        return true; // the choice variant or the class member is not set
    }
    if (!results->empty()) {
        return true;
    }
    return false;
}

static bool s_RemovePrimitiveValue(CObjectInfo& obj)
{
    // remove only enum type of values
    _ASSERT(obj.GetTypeFamily() == eTypeFamilyPrimitive);
    _ASSERT(obj.GetPrimitiveValueType() == ePrimitiveValueEnum);

    const CEnumeratedTypeValues& values = obj.GetEnumeratedTypeValues();
    CEnumeratedTypeValues::TValues all = values.GetValues();
    CEnumeratedTypeValues::TValues::const_iterator it = all.begin();
    obj.SetPrimitiveValueInt(it->second);
    return true;
}

static bool s_RemoveClassMember(CMQueryNodeValue::SResolvedField& obj)
{
    _ASSERT(obj.parent.GetTypeFamily() == eTypeFamilyClass);

    CObjectInfoMI mem = obj.parent.BeginMembers();
    CObjectInfo field = obj.field;
    bool rval(false);

    while (mem.Valid()) {
        if (mem.IsSet() 
            //&& NStr::EqualNocase(mem.GetMember().GetName(), field.GetName())
            && mem.GetMember() == field) {
            if (mem.GetMemberInfo()->Optional()) {
                if (field.GetTypeFamily() == eTypeFamilyPrimitive &&
                    field.GetPrimitiveValueType() == ePrimitiveValueEnum) {
                    rval = s_RemovePrimitiveValue(field);
                } else {
                    mem.Reset();
                    rval = true;
                }
            }
        }
        ++mem;
    }
    return rval;
}

static bool s_ResetChoiceVariant(CMQueryNodeValue::SResolvedField& obj)
{
    _ASSERT(obj.parent.GetTypeFamily() == eTypeFamilyChoice);

    CObjectInfoCV var = obj.parent.GetCurrentChoiceVariant();
    _TRACE("Current variant: " << var.GetVariantInfo()->GetId().GetName());

    // cannot reset choice variant
    //obj.parent.SetChoiceVariant(0); does not work
    // TO DO: rewrite it to delete the choice variable
    if (obj.field.GetTypeFamily() == eTypeFamilyPrimitive &&
        obj.field.GetPrimitiveValueType() == ePrimitiveValueString) {
        obj.field.SetPrimitiveValueString(kEmptyStr);
    }
    return true;
}

static bool s_RemoveContainerElement(CMQueryNodeValue::SResolvedField& obj)
{
    _ASSERT(obj.parent.GetTypeFamily() == eTypeFamilyContainer);

    CObjectInfoEI elem = obj.parent.BeginElements();
    while (elem.Valid()) {
        if (elem.GetElement().GetTypeFamily() == eTypeFamilyPointer) {
            CObjectInfo oi = elem.GetElement().GetPointedObject();
            if (oi == obj.field) {
                elem.Erase();
                return true;
            }
        } else if (elem.GetElement() == obj.field) {
            elem.Erase();
            return true;
        }
        ++elem;
    }
    return false;
}

bool RemoveFieldByName(CMQueryNodeValue::SResolvedField& obj)
{
    CObjectInfo parent = obj.parent;
    CObjectInfo field = obj.field;

    switch (obj.parent.GetTypeFamily()) {
    case eTypeFamilyPrimitive:
        return false;
    case eTypeFamilyClass:
        return s_RemoveClassMember(obj);
    case eTypeFamilyChoice:
        return s_ResetChoiceVariant(obj);
    case eTypeFamilyContainer:
        return s_RemoveContainerElement(obj);
    case eTypeFamilyPointer:
        CMQueryNodeValue::SResolvedField new_obj(obj.parent.GetPointedObject(), obj.field);
        return RemoveFieldByName(new_obj);
    }

    return false;
}

bool SetSimpleTypeValue(CObjectInfo& oi, const CMQueryNodeValue& value, edit::EExistingText existing_text)
{
    _TRACE("<<<< oi: " << &oi);
    if (oi.GetTypeFamily() != eTypeFamilyPrimitive) {
        _TRACE(">>>> false");
        return false;
    }
    bool isOK(false);

    EPrimitiveValueType     oi_t = oi.GetPrimitiveValueType();
    CMQueryNodeValue::EType v_t  = value.GetDataType();
    
    if ( oi_t == ePrimitiveValueString && v_t == CMQueryNodeValue::eString ) {
        // string|char*|const char* ?? TBD
        string orig_val = oi.GetPrimitiveValueString();
        string new_val = value.GetString();
        if (edit::AddValueToString (orig_val, new_val, existing_text)) {
            oi.SetPrimitiveValueString(orig_val); 
        }
        isOK = true;
    }
    else if ( oi_t == ePrimitiveValueBool && v_t == CMQueryNodeValue::eBool ) {
        oi.SetPrimitiveValueBool(value.GetBool());
        isOK = true;
    } 
    else if ( oi_t == ePrimitiveValueReal && v_t == CMQueryNodeValue::eFloat ) {
        // float|double ?? TBD
        oi.SetPrimitiveValueDouble(value.GetDouble());
        isOK = true;
    }
    else if ( oi_t == ePrimitiveValueInteger && v_t == CMQueryNodeValue::eInt ) {
        // (signed|unsigned) (char|short|int|long) ?? TBD
        // oi.GetPrimitiveTypeInfo()->GetSize();
        // oi.GetPrimitiveTypeInfo()->IsSigned();
        oi.SetPrimitiveValueInt8(value.GetInt());
        isOK = true;
    }
    else if ( oi_t == ePrimitiveValueChar && v_t == CMQueryNodeValue::eInt ) {
        oi.SetPrimitiveValueChar(char(value.GetInt()));
        isOK = true;
    }
    else if ( oi_t == ePrimitiveValueEnum && v_t == CMQueryNodeValue::eString ) {
        try {
            oi.SetPrimitiveValueString(value.GetString());
            isOK = true;
        } catch(const CException&) {
            // if this fails, try to convert the string to int and assign again
            try {
                oi.SetPrimitiveValueInt4(NStr::StringToLong(value.GetString()));
                isOK = true;
            } catch(const CException&) {}
        }
    }
    else if( oi_t == ePrimitiveValueEnum && v_t == CMQueryNodeValue::eInt ) {
        oi.SetPrimitiveValueInt4(Int4(value.GetInt()));
        isOK = true;
    }

    _TRACE(">>>> isOK: " << isOK);
    return isOK;
}

bool ResolveAndSetSimpleTypeValue(CObjectInfo& oi, 
    const string& field_name, 
    const CMQueryNodeValue& value, 
    edit::EExistingText existing_text)
{
    _TRACE("<<<< oi: " << &oi << " type family: " << oi.GetTypeFamily() << " type name: " << oi.GetName());
    bool isOK(false);
    if ( field_name.empty() ) {
        isOK = SetSimpleTypeValue (oi, value, existing_text);
    } else {
        CMQueryNodeValue::TObs obs_list;
        if (SetFieldsByName(&obs_list, oi, field_name) ) {
            CMQueryNodeValue::TObs::iterator it = obs_list.begin();
            for ( ; it != obs_list.end(); ++it ) {
                CObjectInfo oi_loc = it->field;
                if ( !SetSimpleTypeValue ( oi_loc, value, existing_text )) {
                    _TRACE(">>>> false");
                    return false;
                } else {
                    isOK = true;
                }
            }
        }
    }
    _TRACE(">>>> " << isOK);
    return isOK;
}

bool GetSimpleTypeValue(CObjectInfo& oi, const string& field_name, CMQueryNodeValue& value)
{
    _TRACE("<<<< oi: " << &oi);
    bool isOK(false);
    if(field_name.empty()) {
        // when the field name is empty, then oi can only be primitive
        if(oi.GetTypeFamily() == eTypeFamilyPrimitive) {
            isOK = GetPrimitiveValue(oi, value);
        }
    } else {
        if(oi.GetTypeFamily() != eTypeFamilyPrimitive) {
            isOK = ResolveIdentToSimple(oi, field_name, value);
        }
    }
    _TRACE(">>>> result: " << isOK);
    return isOK;
}

bool GetPrimitiveValue(const CObjectInfo& oi, CMQueryNodeValue& value)
{
    _TRACE("<<<< oi: " << &oi);
    bool isOK(false);
    if(oi.GetTypeFamily() == eTypeFamilyPrimitive) {
        switch (oi.GetPrimitiveValueType()) {
        case ePrimitiveValueString: 
            value.SetString (oi.GetPrimitiveValueString());
            isOK = true;
            break;
        case ePrimitiveValueBool: 
            value.SetBool (oi.GetPrimitiveValueBool());
            isOK = true;
            break;
        case ePrimitiveValueReal: 
            value.SetDouble ( oi.GetPrimitiveValueDouble() );
            isOK = true;
            break;
        case ePrimitiveValueInteger:
        case ePrimitiveValueChar: 
            value.SetInt ( oi.GetPrimitiveValueInt8() );
            isOK = true;
            break;
        case ePrimitiveValueEnum:
            // getting a string from an enum can cause an exception if
            // the number does not correspond to any defined string,
            // use the number converted into a string in this case
            {
                string sValue;
                try {
                    sValue = oi.GetPrimitiveValueString();
                } catch (const CException&) {
                    sValue = NStr::NumericToString(oi.GetPrimitiveValueInt4());
                }
                value.SetString(sValue);
                isOK = true;
            }
            break;
        default:
            break;
        }
    }
    _TRACE(">>>> result: " << isOK);
    return isOK;
}

static bool s_ResolveIdentOICommon(const CObjectInfo& oi, const string& identifier, CMQueryNodeValue::TObs& nodes)
{
    CMQueryNodeValue::TObs tmp_nodes;
    if(GetFieldsByName(&tmp_nodes, oi, identifier)) { // brings back container
        ITERATE(CMQueryNodeValue::TObs, i_tmp_nodes, tmp_nodes) {
            CObjectInfo member_oi(i_tmp_nodes->field);
            if(member_oi.GetTypeFamily() == eTypeFamilyContainer) {
                CObjectInfoEI ei(member_oi.BeginElements());
                while(ei.Valid()) {
                    _TRACE("Processing element index: " << ei.GetIndex());
                    CObjectInfo element_oi(ei.GetElement());
                    switch(element_oi.GetTypeFamily()) {
                        case eTypeFamilyPointer:
                        {
                            CObjectInfo pointed_oi(element_oi.GetPointedObject());
                            nodes.push_back(CMQueryNodeValue::SResolvedField(member_oi, pointed_oi));
                            break;
                        }
                        case eTypeFamilyClass:
                            nodes.push_back(CMQueryNodeValue::SResolvedField(member_oi, element_oi));
                            break;
                        case eTypeFamilyPrimitive:
                            if (element_oi.GetPrimitiveValueType() == ePrimitiveValueString) 
                                nodes.push_back(CMQueryNodeValue::SResolvedField(member_oi, element_oi));
                            break;
                        default:
                            //!! any other types are not supported to appear in the container
                            break;
                    }
                    ++ei;
                }
            } else {
                nodes.push_back(CMQueryNodeValue::SResolvedField(i_tmp_nodes->parent, member_oi));
            }
        }
        return true;
    } else {
        return false;
    }
}

bool ResolveIdentToSimple(const CObjectInfo& obj, const string& identifier, CMQueryNodeValue& v)
{
    CMQueryNodeValue::TObs nodes;
    if ( s_ResolveIdentOICommon(obj, identifier, nodes) ) {
        if (nodes.empty()) {
            // Typical case of choice variables
            v.SetNotSet();
            return true;
        }
        else {
            // pick the first one if there are multiple (GB-7181)
            CObjectInfo oi = nodes.front().field;
            return (oi.GetTypeFamily() == eTypeFamilyPrimitive) ? GetPrimitiveValue(oi, v) : false;
            // return false if oi was not set or has no primitive type value
        }
    }

    return false; // This identifier was not found
}

bool ResolveIdentToObjects(const CObjectInfo& oi, const string& identifier, CMQueryNodeValue& v)
{
    CMQueryNodeValue::TObs nodes;

    bool resolved = s_ResolveIdentOICommon(oi, identifier, nodes);

    if (!resolved && v.GetDataType() == CMQueryNodeValue::eNotSet) {
         v.SetNotSet();
    } else if (resolved){
        if (v.GetDataType() == CMQueryNodeValue::eNotSet) {
            v.SetObjects(nodes);
        } else if (v.GetDataType() == CMQueryNodeValue::eObjects) {
            ITERATE (CMQueryNodeValue::TObs, it, nodes) {
                v.SetObjects().push_back(*it);
            }
        }
    }
    
    return resolved;
}


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
