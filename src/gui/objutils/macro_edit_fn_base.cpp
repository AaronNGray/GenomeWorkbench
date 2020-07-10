/*  $Id: macro_edit_fn_base.cpp 44444 2019-12-19 15:54:41Z asztalos $
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
#include <gui/objutils/macro_engine.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_field_resolve.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

void IEditMacroFunction::x_InitCall(CIRef<IMacroBioDataIter>& data)
{
    m_DataIter = data;
    m_QualsChangedCount = 0;

    _ASSERT(m_CmdComposite && m_DataIter);
    m_Result->SetDataType(CMQueryNodeValue::eNotSet);

    if (!x_IsNestedFunctionReturnValid()) {
        return;
    }

    if (!x_ValidArguments()) {
        NCBI_THROW(CMacroExecException, eWrongArguments, "Wrong number or type of arguments passed to '" + m_FuncName + "' function");
    }

    x_ResetState();
}

void IEditMacroFunction::x_SetUserObjects(CQueryParseTree::TNode& qnode)
{
    m_Result = Ref(dynamic_cast<CMQueryNodeValue*>(qnode->GetUserObject()));
    _ASSERT(m_Result);

    m_Args.resize(0);
    CTreeNode<CQueryParseNode>::TNodeList_I it = qnode.SubNodeBegin();
    CTreeNode<CQueryParseNode>::TNodeList_I it_end = qnode.SubNodeEnd();
    for (; it != it_end; ++it) {
        CRef<CMQueryNodeValue> arg = Ref(dynamic_cast<CMQueryNodeValue*>((*it)->GetValue().GetUserObject()));
        _ASSERT(arg.GetPointerOrNull());
        if (arg) {
            m_Args.push_back(arg);
        }
    }
    m_FuncName = qnode->GetOriginalText();
}

void IEditMacroFunction::x_AssignReturnValue(const CObjectInfo& oi, const string& field_name)
{
    if (m_Nested == eNotNested) { // return a standard type value
        CMQueryNodeValue::TObs res_oi;
        if ( !GetFieldsByName(&res_oi, oi, field_name) || res_oi.size() != 1) {
            return;
        }
        m_Result->AssignFromObjectInfo(res_oi.front().field);

    } else {
        ResolveIdentToObjects(oi, field_name, m_Result.GetNCObject());
    }
}

void IEditMacroFunction::x_AssignReturnValueFromContainer(const CObjectInfo& oi, const string& container, const string& field_name)
{
    m_Result->SetNotSet();

    CMQueryNodeValue::TObs res_oi;
    GetFieldsByName(&res_oi, oi, container);
    if (res_oi.empty()) {
        return;
    }

    CObjectInfo mod_obj = res_oi.front().field;
    if (mod_obj.GetTypeFamily() != eTypeFamilyContainer) {
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

        if (type_oi.GetPrimitiveValueType() == ePrimitiveValueString) {
            string sValue = type_oi.GetPrimitiveValueString();
            if (NStr::EqualNocase(sValue, field_name)) {
                CObjectInfo val_oi = objInfo.GetClassMemberIterator(++index).GetMember();
                if (m_Nested == eNotNested) {
                    m_Result->AssignFromObjectInfo(val_oi);
                }
                else {
                    if (m_Result->GetDataType() == CMQueryNodeValue::eNotSet) {
                        m_Result->SetDataType(CMQueryNodeValue::eObjects);
                    }
                    m_Result->SetObjects().push_back(CMQueryNodeValue::SResolvedField(objInfo, val_oi));
                }
            }
        }
        else if (type_oi.GetPrimitiveValueType() == ePrimitiveValueEnum) {
            string sValue;
            try {
                sValue = type_oi.GetPrimitiveValueString();
            }
            catch (const CException&) {
                sValue = NStr::NumericToString(type_oi.GetPrimitiveValueInt4());
            }
            if (NStr::EqualNocase(sValue, field_name)) {
                CObjectInfo val_oi = objInfo.GetClassMemberIterator(++index).GetMember();
                if (m_Nested == eNotNested) {
                    m_Result->AssignFromObjectInfo(val_oi);
                }
                else {
                    if (m_Result->GetDataType() == CMQueryNodeValue::eNotSet) {
                        m_Result->SetDataType(CMQueryNodeValue::eObjects);
                    }
                    m_Result->SetObjects().push_back(CMQueryNodeValue::SResolvedField(objInfo, val_oi));
                }
            }
        }
        ++elem;
    }
}

void IEditMacroFunction::x_GetObjectsFromRef(CMQueryNodeValue::TObs& objects, const size_t& index)
{
    _ASSERT(index < m_Args.size());
    if (index < m_Args.size()) {
        objects.clear();

        CMQueryNodeValue& objs = m_Args[index].GetNCObject();
        objs.Dereference();
        if (objs.GetDataType() != CMQueryNodeValue::eObjects) {
            return;
        }
        objects = objs.GetObjects();
    }
}

void IEditMacroFunction::x_GetOptionalArgs(string& delimiter, bool& remove_field, size_t& index)
{
    delimiter.clear();
    remove_field = false;
    if (++index < m_Args.size()) {
        if (m_Args[index]->IsString()) {
            delimiter = m_Args[index]->GetString();
        }
        else if (m_Args[index]->IsBool()) {
            remove_field = m_Args[index]->GetBool();
        }
    }
    if (++index < m_Args.size()) {
        remove_field = m_Args[index]->GetBool();
    }
}

bool IEditMacroFunction::SetQualStringValue(CObjectInfo& oi, const string& value)
{
    if (!NStr::EqualCase(oi.GetPrimitiveValueString(), value)) {
        oi.SetPrimitiveValueString(value);
        m_QualsChangedCount++;
        return true;
    }
    return false;
}

/// CMacroFunction_TopLevel
/// It is the same as CMacroRep::m_TopFuncName
DEFINE_MACRO_FUNCNAME(CMacroFunction_TopLevel, "-TOP-DO-SECTION-")
void CMacroFunction_TopLevel::TheFunction()
{
}

bool CMacroFunction_TopLevel::x_ValidArguments() const
{
    return true;
}


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
