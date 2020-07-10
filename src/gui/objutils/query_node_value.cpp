/*  $Id: query_node_value.cpp 39115 2017-08-01 18:58:09Z asztalos $
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
 * Authors:  Bob Falk
 *
 * File Description:
 * Implementation for query execution using a type-promotion approach
 * for comparisons.
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/query_node_value.hpp>

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistd.hpp>

#include <objects/seq/seq_id_handle.hpp>
#include <gui/objutils/utils.hpp>

#include <cmath>

BEGIN_NCBI_SCOPE


/******************************************************************************
 *
 * QueryValueType namespace Implementation
 *
 *****************************************************************************/ 

 namespace QueryValueType {

     string GetTypeAsString(EBaseType et)
     {
         switch (et) {
            case eBoolResult: return "boolean expression result";
            case eBool: return "boolean const";
            case eInt: return "int const";
            case eFloat: return "float const";
            case eString: return "non-convertible string";
            case eSeqID: return "possible seq-id identifier";
            case eStringBool: return "bool converted from a string";
            case eStringInt: return "integer converted form a string";
            case eStringFloat: return "float converted from a string";
            case eFieldSeqID: return "possible seq-id field value";
            case eFieldString: return "string field value";      
            case eFieldBool: return "boolean field value";
            case eFieldFloat: return "float field value";
            case eFieldInt: return "int field value";
            case eUndefined: return "undefined field value";
            default: return "";
         };
     }

}

/******************************************************************************
 *
 * CQueryNodeValue namespace Implementation
 *
 *****************************************************************************/ 

string CQueryNodeValue::GetVisibleValue() const
{ 
    string str = "Type: " + QueryValueType::GetTypeAsString(m_DataType) +
                 string("Data Field: ") + (IsDataField() ? "True " : "False ") +
                 string("Bool Value: ") + (GetValue() ? "True" : "False"); 

    return str;
}

void CQueryNodeValue::PromoteTo(QueryValueType::EBaseType pt)
{
    if (m_DataType == QueryValueType::eRef) {
        Dereference();
    }

    switch (m_DataType) {
    case QueryValueType::eBool:
    case QueryValueType::eBoolResult:       
    case QueryValueType::eStringBool:
    case QueryValueType::eFieldBool:
        {
            switch (pt) {               
                case QueryValueType::eInt:
                    m_Int = m_Bool ? 1 : 0;
                    break;
                case QueryValueType::eFloat:
                    m_Double = m_Bool ? 1.0 : 0.0;
                    break;
                case QueryValueType::eString:
                    if (m_DataType != QueryValueType::eStringBool)
                        m_String = m_Bool ? "true" : "false";
                    break;
                case QueryValueType::eSeqID:
                    // Can't convert a bool to a seq-id.  Throw an exception
                    // if we are not in query evaluation mode or if the
                    // field in error was not a data field (if it was taken
                    // taken directly from the query string).
                    if (m_DataType != QueryValueType::eFieldBool) {                           
                        NCBI_THROW(CQueryExecException, eIncompatibleType,
                            "Query error. Unable to promote boolean to type: " + 
                            GetTypeAsString(pt));
                    }
                    break;
                default:
                    break;
            }
        }
        break;          
    case QueryValueType::eInt:
    case QueryValueType::eStringInt:
    case QueryValueType::eFieldInt:
        {
            switch (pt) {
                case QueryValueType::eBool:
                    m_Bool = (m_Int==0) ? false : true;
                    break;
                case QueryValueType::eFloat:
                    m_Double = (double)m_Int;
                    break;
                case QueryValueType::eString:
                    if (m_DataType!=QueryValueType::eStringInt)
                        m_String = NStr::Int8ToString(m_Int);
                    break;
                case QueryValueType::eSeqID:
                    // We use the string field for seq-id compares so 
                    // convert the int to a string (the string value will
                    // already be set if type is eStringInt or eFieldInt).
                    if (m_DataType!=QueryValueType::eStringInt && 
                        m_DataType!=QueryValueType::eFieldInt)
                        m_String = NStr::Int8ToString(m_Int);
                    break;
                default:
                    break;
            }
        }
        break;
    case QueryValueType::eFloat:
    case QueryValueType::eStringFloat:
    case QueryValueType::eFieldFloat:
        {
            switch (pt) {
                case QueryValueType::eBool:
                    m_Bool = (m_Double==0.0) ? false : true;
                    break;
                case QueryValueType::eInt:
                    m_Int = (int)m_Double;
                    break;               
                case QueryValueType::eString:
                    if (m_DataType!=QueryValueType::eStringFloat)
                        m_String = NStr::DoubleToString(m_Double);
                    break;
                case QueryValueType::eSeqID:
                    // We use the string field for seq-id compares so 
                    // convert the double to a string (the string value will
                    // already be set if type is eStringFloat or eFieldFloat).
                    //??Should we truncate here? double to int? floor?
                    if (m_DataType!=QueryValueType::eStringFloat && 
                        m_DataType!=QueryValueType::eFieldFloat)
                        m_String = NStr::DoubleToString(m_Double);
                    break;
                default:
                    break;
            }
        }
        break;
    case QueryValueType::eString:
        // leave it as a string if its a seq-id
        if (pt == QueryValueType::eSeqID) {
            break;
        }
        else if (pt != QueryValueType::eString) {
            // If the type is string, and the string does not come from
            // a field, it will not convert for any data, so throw error.
            NCBI_THROW(CQueryExecException, eNotPromotable, 
                "Query error. Unable to promote string " + 
                m_String + " to type: " + GetTypeAsString(pt));
        }
        break;  
    case QueryValueType::eFieldString:
        if (pt != QueryValueType::eString &&
            pt != QueryValueType::eSeqID) {
 
            //If this is the type, we already feel it is not promotable
            NCBI_THROW(CQueryExecException, eIncompatibleType, 
                "Query error. Unable to promote string " + 
                m_String + " to type: " + GetTypeAsString(pt));
                
        }
        break;       
    default:
        break;
    }
}

QueryValueType::EBaseType CQueryNodeValue::GetPromoteType(size_t arg_idx)
{ 
    return (arg_idx < m_PromoteRules.size() ? 
        m_PromoteRules[arg_idx].m_PromotedType : QueryValueType::eUndefined);
}

bool CQueryNodeValue::HasPromoteType(size_t arg_idx,
                                     QueryValueType::EBaseType t1, 
                                     QueryValueType::EBaseType t2)
{
    if (arg_idx < m_PromoteRules.size()) {
        return  (m_PromoteRules[arg_idx].m_Type1 == t1 &&
                 m_PromoteRules[arg_idx].m_Type2 == t2);
    }

    return false;
}
void CQueryNodeValue::SetString(const string& data) 
{ 
    m_String = data; 
    if (m_DataType == QueryValueType::eUndefined) 
        m_DataType = QueryValueType::eString; 
}

void CQueryNodeValue::SetBool(bool data) 
{ 
    m_Bool = data;
    if (m_DataType == QueryValueType::eUndefined)
        m_DataType = QueryValueType::eBool;
}

void CQueryNodeValue::SetDouble(double data) 
{ 
    m_Double = data; 
    if (m_DataType == QueryValueType::eUndefined)
        m_DataType = QueryValueType::eFloat;
}

void CQueryNodeValue::SetInt(Int8 data) 
{ 
    m_Int = data; 
    if (m_DataType == QueryValueType::eUndefined)
        m_DataType = QueryValueType::eInt;
}

void CQueryNodeValue::SetRef(CRef<CQueryNodeValue> node)
{
    m_Ref = node;
    m_DataType = QueryValueType::eRef;
}


void CQueryNodeValue::Dereference()
{
    while (m_DataType == QueryValueType::eRef) {
        CRef<CQueryNodeValue> tmp_obj = m_Ref;
        *this = *tmp_obj;
    }
}

bool CQueryNodeValue::AssignToRef(const CQueryNodeValue& source)
{
    if (m_DataType != QueryValueType::eRef || !m_Ref)
        return false;

    *m_Ref = source;
    return true;
}

END_NCBI_SCOPE
