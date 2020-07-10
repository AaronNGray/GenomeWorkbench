/*  $Id: macro_exec.cpp 44959 2020-04-28 18:49:47Z asztalos $
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
 * Authors: Anatoly Osipov
 *
 * File Description: Macro execution
 *      
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistd.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_exec.hpp>

#undef _TRACE
#define _TRACE(arg) ((void)0)

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

template <typename T>
static bool BinOpsFunc(CQueryParseNode::EType op, const T& a1, const T& a2)
{
    _TRACE("====");
    switch(op) {
    case CQueryParseNode::eAnd:
        return a1 && a2;
    case CQueryParseNode::eOr:
        return a1 || a2;
    case CQueryParseNode::eXor:
        return (!a1 && a2) || (a1 && !a2);
    case CQueryParseNode::eSub:
        return a1 && !a2;
    case CQueryParseNode::eEQ:
        return a1 == a2;
    case CQueryParseNode::eGT:
        return a1 > a2;
    case CQueryParseNode::eGE:
        return a1 >= a2;
    case CQueryParseNode::eLT:
        return a1 < a2;
    case CQueryParseNode::eLE:
        return a1 <= a2;
    default: {
        // Should be unreachable
        string err_msg = "Invalid operation: " + CQueryParseNode::GetNodeTypeAsString(op);
        NCBI_THROW(CMacroExecException, eInternalError, err_msg);
    }
    }
    return false;
}

static bool BinOpsFuncString(CQueryParseNode::EType op, 
                             const string& s1, 
                             const string& s2, 
                             bool case_sensitive)
{
    _TRACE("====");
    Int4 res = (case_sensitive) ? NStr::CompareCase(s1, s2) : NStr::CompareNocase(s1, s2);
    return BinOpsFunc(op, res, 0);
}

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryNodeValue
///
void CMQueryNodeValue::Dereference()
{
    while (GetDataType() == eRef) {
        CRef<CMQueryNodeValue> tmp_obj = m_Ref;
        *this = *tmp_obj;
    }
}

bool CMQueryNodeValue::AssignToRef(const CMQueryNodeValue& source)
{
    if ( m_DataType != eRef || !m_Ref)
        return false;

    *m_Ref = source;
    return true;
}

void CMQueryNodeValue::AssignFromObjectInfo(const CObjectInfo& objinfo)
{
    SetDataType(eNotSet);

    if (objinfo.GetTypeFamily() != eTypeFamilyPrimitive)
        return;

    switch (objinfo.GetPrimitiveValueType()) {
    case ePrimitiveValueString: 
        m_String = objinfo.GetPrimitiveValueString();
        SetDataType(eString);
        break;
    case ePrimitiveValueEnum:
        // getting a string from an enum can cause an exception if
        // the number does not correspond to any defined string,
        // use the number converted into a string in this case
        try {
            m_String = objinfo.GetPrimitiveValueString();
        } catch(const CException&) {
            m_String = NStr::NumericToString(objinfo.GetPrimitiveValueInt4());
        }
        SetDataType(eString);
        break;
    case ePrimitiveValueBool: 
        m_Bool = objinfo.GetPrimitiveValueBool();
        SetDataType(eBool);
        break;
    case ePrimitiveValueReal: 
        m_Double = objinfo.GetPrimitiveValueDouble();
        SetDataType(eFloat);
        break;
    case ePrimitiveValueInteger:
    case ePrimitiveValueChar: 
        m_Int = objinfo.GetPrimitiveValueInt();
        SetDataType(eInt);
        break;
    default:
        ; // This type is not supported
    }
}

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionOps
///
void CMQueryFunctionOps::x_InitReferences(CQueryParseTree::TNode& node)
{
    m_Result = x_GetUserObject(node);
    _ASSERT(m_Result);

    TArgVector m_argNodes;
    // Consider to rewrite taking subnodes from the tree without MakeArgVector
    // However, qparse update is needed to get subnodes number to reserve
    // space in vector without result relocation.
    MakeArgVector(node, m_argNodes);
    m_Arguments.clear();
    m_Arguments.reserve(m_argNodes.size());
    TArgVector::const_iterator iter = m_argNodes.begin();
    while(iter != m_argNodes.end()) {
        CRef<CMQueryNodeValue> user_obj = x_GetUserObject(**iter);
        _ASSERT(user_obj.GetPointerOrNull());
        if (user_obj) {
            m_Arguments.push_back(user_obj);
            ++iter;
        }
    }
}

bool CMQueryFunctionOps::x_ValidateAll(Uint4 number, CMQueryNodeValue::EType type)
{
    if (m_Arguments.size() != number)
        return false;
    
    for (auto& it : m_Arguments) {
        if (it->GetDataType() != type)
            return false;
    }
    return true;
}

bool CMQueryFunctionOps::x_Validate(Uint4 number, CMQueryNodeValue::EType type1, CMQueryNodeValue::EType type2)
{
    if (m_Arguments.size() != number)
        return false;

    for (auto& it : m_Arguments) {
        if (it->GetDataType() != type1 && it->GetDataType() != type2) {
            return false;
        }
    }
    return true;
}

bool CMQueryFunctionOps::x_FindNotSet()
{
    for(auto& iter : m_Arguments) {
        if (iter->GetDataType() == CMQueryNodeValue::eNotSet) {
            return true;
        }
    }
    return false;
}

bool CMQueryFunctionOps::x_IsCaseSensitive()
{
    CMacroExec* macro_exec = GetContext();
    _ASSERT(macro_exec);
    if (macro_exec) {
        return macro_exec->IsCaseSensitive();
    }
    return false;
}

void CMQueryFunctionOps::x_ProcessInvalidParams(CQueryParseTree::TNode& node)
{
    if (x_FindNotSet()) {
        //if variable was unresolved
        m_Result->SetDataType(CMQueryNodeValue::eNotSet);
    }
    else {
        ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eWrongArguments,
            "Wrong number or type of argument", &node);
    }
}

/// functor classes. 
/// See CMacroExec::x_Init to get a match between tree node types and 
/// its corresponding functor class. Note that these are only for processing 
/// predefined node types (CQueryParseNode::EType) and not for processing 
/// specific user functions.
///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionOps_BooleanBinary
///
class CMQueryFunctionOps_BooleanBinary : public CMQueryFunctionOps
{
    virtual void Evaluate(CQueryParseTree::TNode& node)
    {
    _TRACE("<<<< node: " << &node.GetValue() << " type: " << node.GetValue().GetType() << " orig text: " << node.GetValue().GetOrig());
        x_InitReferences(node);
        if(x_ValidateAll(2, CMQueryNodeValue::eBool))
            m_Result->SetBool(BinOpsFunc(node.GetValue().GetType(), m_Arguments[0]->GetBool(), m_Arguments[1]->GetBool()) );
        else
            x_ProcessInvalidParams(node);
        _TRACE(">>>>");
    }
};

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionOps_BooleanNot
///
class CMQueryFunctionOps_BooleanNot : public CMQueryFunctionOps
{
    virtual void Evaluate(CQueryParseTree::TNode& node)
    {
        x_InitReferences(node);
        if(x_ValidateAll(1, CMQueryNodeValue::eBool))
            m_Result->SetBool( !m_Arguments[0]->GetBool() );
        else
            x_ProcessInvalidParams(node);
    }
};

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionOps_Comparison
///
class CMQueryFunctionOps_Comparison : public CMQueryFunctionOps
{
    virtual void Evaluate(CQueryParseTree::TNode& node)
    {
        _TRACE("<<<< node: " << &node.GetValue() << " type: " << node.GetValue().GetType() << " orig text: " << node.GetValue().GetOrig());
        x_InitReferences(node);

        // either of the parameters can be a not just a value, but also a reference,
        // so dereference both before use
        m_Arguments[0]->Dereference();
        m_Arguments[1]->Dereference();
        
        bool valid = false; // below is an assignment and not a comparison of 'valid'
        if (!valid && x_ValidateAll(2, CMQueryNodeValue::eInt) && (valid = true)) {
            m_Result->SetBool(BinOpsFunc(node.GetValue().GetType(),
                            m_Arguments[0]->GetInt(),
                            m_Arguments[1]->GetInt()));
        }
        if (!valid && x_ValidateAll(2, CMQueryNodeValue::eFloat) && (valid = true)) {
            m_Result->SetBool(BinOpsFunc(node.GetValue().GetType(),
                            m_Arguments[0]->GetDouble(),
                            m_Arguments[1]->GetDouble()));
        }
        if (!valid && x_Validate(2, CMQueryNodeValue::eInt, CMQueryNodeValue::eFloat) 
            && x_ConvertIntArgsToDouble() 
            && (valid = true)) {
            m_Result->SetBool(BinOpsFunc(node.GetValue().GetType(),
                            m_Arguments[0]->GetDouble(),
                            m_Arguments[1]->GetDouble()));
        }
        if (!valid && x_ValidateAll(2, CMQueryNodeValue::eString) && (valid = true)) {
            m_Result->SetBool(BinOpsFuncString(node.GetValue().GetType(),
                            m_Arguments[0]->GetString(),
                            m_Arguments[1]->GetString(),
                            x_IsCaseSensitive()));
        }
        if (!valid && x_ValidateAll(2, CMQueryNodeValue::eBool) && (valid = true)) {
            m_Result->SetBool(BinOpsFunc(node.GetValue().GetType(),
                            m_Arguments[0]->GetBool(),
                            m_Arguments[1]->GetBool()));
        }
        if (!valid)
            x_ProcessInvalidParams(node);
        _TRACE(">>>>");
    }

    /// Converts the integer arguments to double in order to perform comparisons
    bool x_ConvertIntArgsToDouble()
    {
        for (auto& it : m_Arguments) {
            if (it->GetDataType() == CMQueryNodeValue::eInt) {
                it->SetDouble(double(it->GetInt()));
            }
        }
        return x_ValidateAll((Uint4)m_Arguments.size(), CMQueryNodeValue::eFloat);
    }
};

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionOps_Like
///
class CMQueryFunctionOps_Like : public CMQueryFunctionOps
{
    virtual void Evaluate(CQueryParseTree::TNode& node)
    {
        x_InitReferences(node);
        
        bool valid = false; // below is an assignment and not a comparison of 'valid'
        if(!valid && x_ValidateAll(2, CMQueryNodeValue::eString)  && (valid = true))
            m_Result->SetBool ( NStr::MatchesMask(m_Arguments[0]->GetString(), 
                                                m_Arguments[1]->GetString(), 
                                                (x_IsCaseSensitive())?NStr::eCase:NStr::eNocase) );

        if(!valid)
            x_ProcessInvalidParams(node);
    }
};

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionOps_In
///
class CMQueryFunctionOps_In : public CMQueryFunctionOps
{
    virtual void Evaluate(CQueryParseTree::TNode& node)
    {
        bool valid = false;
        x_InitReferences(node);

        auto arg_num = m_Arguments.size();
        size_t index = 0;
        if (arg_num >= 2) {
            // below is an assignment and not a comparison of 'valid'
            if (!valid && x_ValidateAll(Uint4(arg_num), CMQueryNodeValue::eInt) && (valid = true)) {
                for (index = 1; index < arg_num; ++index) {
                    if (m_Arguments[0]->GetInt() == m_Arguments[index]->GetInt()) {
                        break;
                    }
                }
            }
            if (!valid && x_ValidateAll(Uint4(arg_num), CMQueryNodeValue::eString) && (valid = true)) {
                for (index = 1; index < arg_num; ++index) {
                    if (BinOpsFuncString(CQueryParseNode::eEQ,
                        m_Arguments[0]->GetString(),
                        m_Arguments[index]->GetString(),
                        x_IsCaseSensitive())) {
                        break;
                    }
                }
            }
        }

        if (!valid) {
            x_ProcessInvalidParams(node);
        } else {
            m_Result->SetBool(index < arg_num);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionOps_Between
///
class CMQueryFunctionOps_Between : public CMQueryFunctionOps
{
    virtual void Evaluate(CQueryParseTree::TNode& node)
    {
        x_InitReferences(node);
        
        bool valid = false;
        if (!valid && x_ValidateAll(3, CMQueryNodeValue::eInt)) {
            valid = true;
            m_Result->SetBool ( BinOpsFunc(CQueryParseNode::eGE,
                                       m_Arguments[0]->GetInt(), 
                                       m_Arguments[1]->GetInt()) );
            m_Result->SetBool ( m_Result->GetBool() && 
                             BinOpsFunc(CQueryParseNode::eLE,
                                       m_Arguments[0]->GetInt(), 
                                       m_Arguments[2]->GetInt()) );
        }
            
        if (!valid && x_ValidateAll(3, CMQueryNodeValue::eString)) {
            valid = true;
            m_Result->SetBool ( BinOpsFuncString(CQueryParseNode::eGE,
                                              m_Arguments[0]->GetString(), 
                                              m_Arguments[1]->GetString(),
                                              x_IsCaseSensitive()) );
            m_Result->SetBool (  m_Result->GetBool() && 
                             BinOpsFuncString(CQueryParseNode::eLE,
                                              m_Arguments[0]->GetString(), 
                                              m_Arguments[2]->GetString(),
                                              x_IsCaseSensitive()) );
        }

        if (!valid)
            x_ProcessInvalidParams(node);
    }
};

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionFunction implementation
///
void CMQueryFunctionFunction::Evaluate(CQueryParseTree::TNode& node)
{
    _TRACE("<<<< node: " << &node.GetValue() << " type: " << node.GetValue().GetType() << " orig text: " << node.GetValue().GetOrig());

    CMacroExec* macro_exec = GetContext();
    _ASSERT(macro_exec);
    IResolver* resolver = macro_exec->GetResolver();
    _ASSERT(resolver);

    // return information also about the parent of the current node
    // if the parent is a function, the current function should return a CObjectInfo instead of a standard data type 
    // This is useful mostly for functions used in the WHERE clause of the macro script
    resolver->CallFunction(node->GetOriginalText(), node);
    _TRACE(">>>>");
}

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionDummy implementation
///
void CMQueryFunctionDummy::Evaluate(CQueryParseTree::TNode& node)
{
    ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eIncorrectNodeType, 
        "Node type is not supported: " + node.GetValue().GetNodeTypeAsString(), &node);
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroExec implementation
///
void CMacroExec::x_Init()
{
    m_Boolean = false;

    int lo = CQueryParseNode::eNotSet;
    int hi = CQueryParseNode::eMaxType; // array does not include this 
    for (int i = lo; i < hi; i++)
        AddFunc(static_cast<CQueryParseNode::EType>(i), new CMQueryFunctionDummy());

    // create or reset user objects (node values)
    CMQueryNodePreprocessor pre_exec;
    TreeDepthFirstTraverse(*m_QTree->GetQueryTree(), pre_exec);

    // Logical operators:
    AddFunc(CQueryParseNode::eAnd, new CMQueryFunctionOps_BooleanBinary());
    AddFunc(CQueryParseNode::eOr, new CMQueryFunctionOps_BooleanBinary());
    AddFunc(CQueryParseNode::eSub, new CMQueryFunctionOps_BooleanBinary());
    AddFunc(CQueryParseNode::eXor, new CMQueryFunctionOps_BooleanBinary());

    AddFunc(CQueryParseNode::eNot, new CMQueryFunctionOps_BooleanNot());

    // Constants:
    AddFunc(CQueryParseNode::eIntConst, new CMQueryFunctionValue());
    AddFunc(CQueryParseNode::eFloatConst, new CMQueryFunctionValue());
    AddFunc(CQueryParseNode::eBoolConst, new CMQueryFunctionValue());
    AddFunc(CQueryParseNode::eString, new CMQueryFunctionValue());

    // Comparison operators:
    AddFunc(CQueryParseNode::eEQ, new CMQueryFunctionOps_Comparison());
    AddFunc(CQueryParseNode::eGT, new CMQueryFunctionOps_Comparison());
    AddFunc(CQueryParseNode::eGE, new CMQueryFunctionOps_Comparison());
    AddFunc(CQueryParseNode::eLT, new CMQueryFunctionOps_Comparison());
    AddFunc(CQueryParseNode::eLE, new CMQueryFunctionOps_Comparison());
    
    AddFunc(CQueryParseNode::eIn, new CMQueryFunctionOps_In());
    AddFunc(CQueryParseNode::eBetween, new CMQueryFunctionOps_Between());
    AddFunc(CQueryParseNode::eLike, new CMQueryFunctionOps_Like());
    
    /* TBD: needs to be implemented
    AddFunc(CQueryParseNode::eLike, new CMQueryFunctionCompare());
    */

    // For functions implementation:
    AddFunc(CQueryParseNode::eIdentifier, new CMQueryFunctionValue());
    AddFunc(CQueryParseNode::eFunction, new CMQueryFunctionFunction());

    // Assignment operator implementaion.
    // Unusual mapping was chosen:
    //  Run time variable ->  CQueryParseNode::eSelect
    //  Assignment        ->  CQueryParseNode::eFrom
    // This is done to preserve CQueryParseNode from modification.
    AddFunc(CQueryParseNode::eSelect, new CMQueryFunctionRTVar); // RT VAR
    AddFunc(CQueryParseNode::eFrom, new CMQueryFunctionAssignment);   // Assignment operator
}

void CMacroExec::EvaluateTree(CQueryParseTree& Qtree, IResolver& resolver, bool query_tree, bool cs)
{
    m_QTree = &Qtree;
    m_Resolver = &resolver;
    m_CaseSensitive = cs;
    x_Init();

    if (query_tree) {
        x_EvaluateQueryTree(*m_QTree->GetQueryTree());
    }
    else {
        TBase::Evaluate(*m_QTree);
    }

    m_QTree = &Qtree; // call to Evaluate resets m_QTree pointer

    m_Result = false;
    m_Boolean = false;
    m_NotSet = false;

    CRef<CMQueryNodeValue> pNodeValue = x_GetTopUserNodeValue();
    if (pNodeValue) {
        m_NotSet = (pNodeValue->GetDataType() == CMQueryNodeValue::eNotSet);
        m_Boolean = (pNodeValue->GetDataType() == CMQueryNodeValue::eBool);
    }
    m_Result = (m_Boolean) ? pNodeValue->GetBool() : false;
}

CRef<CMQueryNodeValue> CMacroExec::GetOrCreateRTVar(const string& name)
{
    return GetResolver()->GetOrCreateRTVar(name);
}

// Variables resolver (consider to make it virtual)
bool CMacroExec::PassIdentifierToResolver(const string& identifier,
                                   CMQueryNodeValue& v, const CQueryParseTree::TNode* parent) 
{
    return m_Resolver->ResolveIdentifier(identifier, v, parent);
}

bool CMacroExec::x_EvaluateQueryTree(CQueryParseTree::TNode& node)
{
    switch (node.GetValue().GetType()) {
    case CQueryParseNode::eAnd:
    {{
        bool res = false;
        CQueryParseTree::TNode::TNodeList_I iter = node.SubNodeBegin();
        if (iter != node.SubNodeEnd()) {
            res = x_EvaluateQueryTree(**iter);
            ++iter;
        }

        for (; iter != node.SubNodeEnd() && res; ++iter) {
            res &= x_EvaluateQueryTree(**iter);
        }

        CMQueryNodeValue* user_obj = dynamic_cast<CMQueryNodeValue*>(node->GetUserObject());
        _ASSERT(user_obj);
        if (user_obj) {
            user_obj->SetBool(res);
        }
        return res;
    }}
    case CQueryParseNode::eOr:
    {{
        bool res = false;
        CQueryParseTree::TNode::TNodeList_I iter = node.SubNodeBegin();
        if (iter != node.SubNodeEnd()) {
            res = x_EvaluateQueryTree(**iter);
            ++iter;
        }

        for (; iter != node.SubNodeEnd() && !res; ++iter) {
            res |= x_EvaluateQueryTree(**iter);
        }

        CMQueryNodeValue* user_obj = dynamic_cast<CMQueryNodeValue*>(node->GetUserObject());
        _ASSERT(user_obj);
        if (user_obj) {
            user_obj->SetBool(res);
        }
        return res;
    }}
    case CQueryParseNode::eNot:
    {{
        bool res = false;
        CQueryParseTree::TNode::TNodeList_I iter = node.SubNodeBegin();
        if (iter != node.SubNodeEnd()) {
            res = !x_EvaluateQueryTree(**iter);
        }

        CMQueryNodeValue* user_obj = dynamic_cast<CMQueryNodeValue*>(node->GetUserObject());
        _ASSERT(user_obj);
        if (user_obj) {
            user_obj->SetBool(res);
        }
        return res;
    }}
    default:
    {{
        CQueryParseTree* orig_qtree = m_QTree;
        TBase::Evaluate(*m_QTree, node);   // call to Evaluate resets m_QTree pointer
        m_QTree = orig_qtree;
           
        bool res = false;
        IQueryParseUserObject* user_object = node.GetValue().GetUserObject();
        if (user_object) {
            CMQueryNodeValue* pTopUserNodeValue = dynamic_cast<CMQueryNodeValue*>(user_object);
            if (pTopUserNodeValue) {
                bool boolean = (pTopUserNodeValue->GetDataType() == CMQueryNodeValue::eBool);
                res = (boolean) ? pTopUserNodeValue->GetBool() : false;
            }
        }

        return res;
    }}

    }
    return false;
}

CRef<CMQueryNodeValue> CMacroExec::x_GetTopUserNodeValue()
{
    CQueryParseTree::TNode* query_parsenode = m_QTree->GetQueryTree();
    if (query_parsenode) {
        IQueryParseUserObject* query_userobject = query_parsenode->GetValue().GetUserObject();
        if (query_userobject) {
            CMQueryNodeValue* top_user_nodevalue = dynamic_cast<CMQueryNodeValue*>(query_userobject);
            return CRef<CMQueryNodeValue>(top_user_nodevalue);
        }
    }
    return CRef<CMQueryNodeValue>();
}

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryNodePreprocessor implementation
///
ETreeTraverseCode
CMQueryNodePreprocessor::operator()(CTreeNode<CQueryParseNode>& tr, int delta)
{
    if (delta == 0 || delta == 1) {
        // If node has children, we skip it and process on the way back
        if (!tr.IsLeaf()) {
            return eTreeTraverse;
        }
    }

    CMQueryNodeValue* v = dynamic_cast<CMQueryNodeValue*>(tr->GetUserObject());
    if(v)
        tr->ResetUserObject();
    else
        tr->SetUserObject(new CMQueryNodeValue(&tr));

    return eTreeTraverse;
}


///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionValue
///
void CMQueryFunctionValue::Evaluate(CQueryParseTree::TNode& node)
{
    _TRACE("<<<< node: " << &node.GetValue() << " type: " << node.GetValue().GetType() << " orig text: " << node.GetValue().GetOrig());
    CMQueryNodeValue* v = dynamic_cast<CMQueryNodeValue*>(node->GetUserObject());
    _ASSERT(v);

    switch(node->GetType()) {
    case CQueryParseNode::eIntConst:
        v->SetInt ( node.GetValue().GetInt() );    
        break;
    case CQueryParseNode::eFloatConst:
        v->SetDouble( node.GetValue().GetDouble() );
        break;
    case CQueryParseNode::eBoolConst:
        // TODO: the parser does not parse 'true' and 'false' values as boolean constants
        v->SetBool ( node.GetValue().GetBool() );
        break;
    case CQueryParseNode::eString:
        {{
            const string& str = node.GetValue().GetStrValue();
            const string& strOrig = node.GetValue().GetOriginalText();

            if (str == strOrig) {
                // Check for number
                // Negative numbers are parsed as strings

                if (NStr::EqualNocase(str, "true") || NStr::EqualNocase(str, "false")) {
                    // the variable is boolean
                    v->SetBool(NStr::StringToBool(str));
                } 
                else {
                    bool is_int = false, is_float = false;

                    Int8 data_int = NStr::StringToInt8(str, NStr::fConvErr_NoThrow);
                    if (errno != 0 && !data_int) {
                        double data_dbl = NStr::StringToDouble(str, NStr::fConvErr_NoThrow);
                        if (errno == 0) {
                            v->SetDouble(data_dbl);
                            is_float = true;
                        }
                    }
                    else {
                        v->SetInt(data_int);
                        is_int = true;
                    }

                    if (!is_int && !is_float) {
                        // It is an identifier
                        // return simple data when the parent is not eFunction
                        // return eobject when the parent is eFunction
                        if (!GetContext()->PassIdentifierToResolver(str, *v, node.GetParent())) {
                            ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eIncorrectIdentifier,
                                "Unknown identifier: '" + str + "'", &node);
                        }
                    }
                }
            } else if ((strOrig.length() - str.length() > 1) && 
                       (strOrig[0] == '"')                   && // first char
                       (strOrig[strOrig.length()-1] == '"')) {     // last char
                
                string root, subfield;
                NStr::SplitInTwo(str, ".", root, subfield);
                if (NStr::FindNoCase(str, ".") != NPOS && GetContext()->GetResolver()->ExistRTVar(root)) {
                    if (!GetContext()->PassIdentifierToResolver(str, *v, node.GetParent()))
                        ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eIncorrectIdentifier,
                            "Unknown identifier: '" + str + "'", &node);
                } else {
                    // It is a string
                    v->SetString ( str );
                }
            } else {
                // It is something unknown
                ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eInvalidQuery, 
                    "Invalid string: '" + node->GetOriginalText() + "'", &node);
            }
        }}
        break;
    case CQueryParseNode::eIdentifier:
        {{
            const string& strOrig = node.GetValue().GetOriginalText();
            if (!GetContext()->PassIdentifierToResolver(strOrig, *v, node.GetParent()))
                ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eIncorrectIdentifier, 
                            "Unknown identifier: '" + strOrig + "'", &node);
        }}
        break;
    default:
        // It is something unknown
        ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eIncorrectNodeType, 
            "Incorrect node type: " + node.GetValue().GetNodeTypeAsString(), &node);
    }
    _TRACE(">>>>");
}

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionRTVar
///
void CMQueryFunctionRTVar::Evaluate(CQueryParseTree::TNode& node) 
{
    _TRACE("<<<< node: " << &node.GetValue() << " type: " << node.GetValue().GetType() << " orig text: " << node.GetValue().GetOrig());
    _ASSERT(node.GetValue().GetType() == CQueryParseNode::eSelect);

    CMacroExec* context = GetContext();
    CMQueryNodeValue* node_value = dynamic_cast<CMQueryNodeValue*>(node->GetUserObject());
    _ASSERT(context && node_value);
    _TRACE("User data type: " << node_value->GetDataType());

    node_value->SetRef( context->GetOrCreateRTVar(node.GetValue().GetOriginalText()) );
    _TRACE(">>>>");
}

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionAssignment
///
void CMQueryFunctionAssignment::Evaluate(CQueryParseTree::TNode& node)
{
    _TRACE("<<<< node: " << &node.GetValue() << " type: " << node.GetValue().GetType() << " orig text: " << node.GetValue().GetOrig());
    x_InitReferences(node);
    size_t arg_size = m_Arguments.size();
    if ((arg_size < 2) 
        || (m_Arguments[0]->GetDataType() != CMQueryNodeValue::eRef)
        || (arg_size > 2 && m_Arguments[2]->GetDataType() != CMQueryNodeValue::eInt) ) {
        
        x_ProcessInvalidParams(node);
        return;
    }

    CRef<CMQueryNodeValue> node_value = m_Arguments[1];
    _TRACE("number of arguments: " << arg_size);
    _TRACE("For filtering to work, it must be 3");
    _TRACE("Arg 1 user data type: " << node_value->GetDataType());
    _TRACE("For filtering to work, it must be eObjects: " << CMQueryNodeValue::eObjects);

    // --------------------------------------
    // Next, filter the found objects
    if (arg_size == 3 && node_value->GetDataType() == CMQueryNodeValue::eObjects) {
        _TRACE("Proceeding to filtering");
        CMQueryNodeValue::TObs objs = node_value->GetObjects();

        _TRACE("Going through # objects: " << objs.size());
        CMQueryNodeValue::TObs updated_objs;
        for (auto objs_iter = objs.begin(); objs_iter != objs.end(); ++objs_iter) {
            // --------------------------------------
            // Copy context for execution
            CMacroExec* exec_context = GetContext();
            CMacroExec me;

            IResolver* resolver = exec_context->GetResolver();
            CQueryParseTree* where_tree = resolver->GetAssignmentWhereClause(int(m_Arguments[2]->GetInt()));

            // To properly evaluate the do-where clause, we may need to evaluate the RT-var to the left of assignment
            // since it is not yet assigned to anything, we create a temp RT-var that can be used
            CObjectInfo tmp_obj = objs_iter->field;
            CQueryParseTree::TNode* arg_node = m_Arguments[0]->GetNode();
            if (arg_node) {
                const string tmp_obj_name = arg_node->GetValue().GetOrig();
                resolver->AddTmpRTVarObject(tmp_obj_name, tmp_obj);
            }

/*
#ifdef _DEBUG 
            p_where_tree->Print(NcbiCout);
#endif
*/
            _TRACE("Evaluating where tree id: " << m_Arguments[2]->GetInt());
            me.EvaluateTree ( *where_tree, *resolver, true, me.IsCaseSensitive() );
            resolver->ResetTmpRTVarObjects();

            if (!me.IsBoolType() && !me.IsNotSetType()) {
                NCBI_THROW(CMacroExecException, eInternalError, "Wrong type of computed WHERE clause");
            }
            _TRACE("Evaluated to " << (me.IsBoolType() && me.GetBoolValue()));
            
            if (me.IsBoolType() && me.GetBoolValue()) {
                updated_objs.push_back(CMQueryNodeValue::SResolvedField(objs_iter->parent, tmp_obj));
            }
        }
        node_value->SetObjects(updated_objs);
    }
    // End of filtering
    m_Arguments[0]->AssignToRef(*node_value);
    _TRACE(">>>>");
}

///////////////////////////////////////////////////////////////////////////////
/// class CMQueryFunctionBase
///
CMacroExec* CMQueryFunctionBase::GetContext()
{ 
    _ASSERT(m_QExec);
    CMacroExec* macro_exec = dynamic_cast<CMacroExec*>(m_QExec);
    _ASSERT(macro_exec);
    return macro_exec;
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
