/*  $Id: query_func_promote.cpp 39115 2017-08-01 18:58:09Z asztalos $
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

#include <gui/objutils/query_func_promote.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistd.hpp>

#include <objects/seq/seq_id_handle.hpp>
#include <gui/objutils/utils.hpp>

#include <cmath>

BEGIN_NCBI_SCOPE

/******************************************************************************
 *
 * CQueryFuncPromoteBase Implementation
 *
 *****************************************************************************/ 

CQueryFuncPromoteBase::TEvalResult* 
CQueryFuncPromoteBase::GetQueryNodeValue(CQueryParseTree::TNode& qnode)
{
    IQueryParseUserObject* uo = qnode.GetValue().GetUserObject();
    return dynamic_cast<TEvalResult*>(uo);
}
    
CQueryFuncPromoteBase::TEvalResult* 
CQueryFuncPromoteBase::MakeQueryNodeValue(CQueryParseTree::TNode& qnode)
{
    TEvalResult* c = this->GetQueryNodeValue(qnode);
    if (!c) {
        qnode.GetValue().SetUserObject((c = new TEvalResult(&qnode)));
    }
    return c;
}

bool CQueryFuncPromoteBase::ResolveFieldValue(TEvalResult& tree_val)
{
    // Type information for query tree elements that are not data fields
    // are set during pre-processing of the query since they are static.
    // Only field data types need to be set dymamically (where querying
    // each individual data element)
    if (tree_val.IsDataField()) { 
        if (tree_val.GetDataType() == QueryValueType::eFieldString ||
            tree_val.GetDataType() == QueryValueType::eFieldSeqID ||
            tree_val.GetDataType() == QueryValueType::eUndefined ) {
            if (!m_QExec->ResolveIdentifier(tree_val.GetFieldID(),
                                            tree_val.m_String)) {
                tree_val.m_String = "";                
                return false;
            }
        }
        else if (tree_val.GetDataType() == QueryValueType::eFieldInt) {
            if (!m_QExec->ResolveIdentifier(tree_val.GetFieldID(),
                                            tree_val.m_Int)) {
                tree_val.m_Int = 0;
                return false;
            }
        }
        else if (tree_val.GetDataType() == QueryValueType::eFieldBool) {
            if (!m_QExec->ResolveIdentifier(tree_val.GetFieldID(),
                                            tree_val.m_Bool)) {
                tree_val.m_Bool = false;
                return false;
            }
        }
        else {
            if (!m_QExec->ResolveIdentifier(tree_val.GetFieldID(),
                                            tree_val.m_Double)) {
                tree_val.m_Double = 0.0;
                return false;
            }
        }
    }

    return true;
}

// From NStr::StringToBool but faster because it doesn't throw an exception
static const char* s_kTrueString = "true";
static const char* s_kFalseString = "false";
static const char* s_kTString = "t";
static const char* s_kFString = "f";
static const char* s_kYesString = "yes";
static const char* s_kNoString = "no";
static const char* s_kYString = "y";
static const char* s_kNString = "n";
bool FastStringToBool(const CTempString str, bool& isbool)
{
    isbool = true;
    if (AStrEquiv(str, s_kTrueString, PNocase()) ||
        AStrEquiv(str, s_kTString, PNocase()) ||
        AStrEquiv(str, s_kYesString, PNocase()) ||
        AStrEquiv(str, s_kYString, PNocase())) {
        errno = 0;
        return true;
    }
    if (AStrEquiv(str, s_kFalseString, PNocase()) ||
        AStrEquiv(str, s_kFString, PNocase()) ||
        AStrEquiv(str, s_kNoString, PNocase()) ||
        AStrEquiv(str, s_kNString, PNocase())) {
        errno = 0;
        return false;
    }

    isbool = false;
    return false;
}

bool CQueryFuncPromoteBase::SetCompareType(TEvalResult& tree_val)
{
    // Type info for query tree elements that are not data fields
    // is set during pre-processing of the query since they are static.
    // Only field data types may need to be set dynamically (while querying
    // each individual data element)
    if (tree_val.IsDataField()) {

        // If the data field was identified (during pre-processing) as a seq-id, 
        // do not try to convert it to another (non-string) type
        if (tree_val.GetDataType() == QueryValueType::eFieldSeqID) {
            return ResolveFieldValue(tree_val);
        }

        // We normally use this function (SetCompareType) when we are not
        // certain of the underlying type for the field. For that reason
        // we (re)set the field type to string here since it is the most
        // general.
        tree_val.SetDataType(QueryValueType::eFieldString);

        // Look up the field using the data source
        if (!ResolveFieldValue(tree_val)) {
            return false;
        }

        // Get first non-blank character - this speeds up checking
        // for possible conversions
        std::size_t first_char = tree_val.m_String.find_first_not_of(" \t\r\n");
        if (first_char == std::string::npos)
            return true;

        int ch = tolower(tree_val.m_String[first_char]);

        // Check for boolean.  For performance, use function that does not throw
        // an exception when not found
        if (ch == 'y' || ch == 'n' || ch == 't' || ch == 'f') {
            bool found = false;
            tree_val.m_Bool = FastStringToBool(tree_val.m_String, found);

            if (found) {
                tree_val.SetDataType(QueryValueType::eFieldBool);
            }
        }
        // See if it can be an number. StringToInt/Double() are slow so
        // avoid them if possible. Valid numbers: 0.5  +5  -5  .5  0x5A      
        else if (isdigit(ch) || ch == '-' || ch == '+' || ch == '.') {
            // Try integer convert. This will not convert floats, e.g. 1.0         
            tree_val.m_Int = NStr::StringToInt(tree_val.m_String, NStr::fConvErr_NoThrow);
            if (errno != 0 && !tree_val.m_Int) {
                // See if it can be a float/double:
                tree_val.m_Double = NStr::StringToDouble(tree_val.m_String, NStr::fConvErr_NoThrow);
                if (!(errno != 0 && 
                    (tree_val.m_Double == HUGE_VAL || tree_val.m_Double == -HUGE_VAL || !tree_val.m_Double)))
                        tree_val.SetDataType(QueryValueType::eFieldFloat);
            }
            else {
                tree_val.SetDataType(QueryValueType::eFieldInt);            
            }
        }
    }

    return true;
}

QueryValueType::EBaseType  
CQueryFuncPromoteBase::GetPromotedType(const CPromoteRule& pr)
{
    // Search for the promotion rule based on the types of the two data items and
    // the operator.
    std::vector<CPromoteRule>::iterator iter = 
        std::lower_bound(m_PromoteRules.begin(), 
        m_PromoteRules.end(), pr);

    // All items should be covered (and if not, we can't compare the items).
    if (iter == m_PromoteRules.end() ||
        !(*iter ==  pr)) {
            //_TRACE("Error finding promote type");
            return QueryValueType::eUndefined;
    }

    return (*iter).m_PromotedType; 
}

void CQueryFuncPromoteBase::AddPromoteTypes(CQueryParseNode::EType op,
                                            QueryValueType::EBaseType type1,
                                            QueryValueType::EBaseType type2,
                                            QueryValueType::EBaseType ptype)
{


    // Add promotion rule for both 'type1 op type2' and 'type2 op type1'.   
    m_PromoteRules.push_back(CPromoteRule(op, type1, type2, ptype));
    m_PromoteRules.push_back(CPromoteRule(op, type2, type1, ptype));
}

void CQueryFuncPromoteBase::PreProcess(CQueryParseTree::TNode& tr,
                                       objects::CScope* scope)
{        
     CQueryParseNode& qnode = tr.GetValue();

     // Each node in the query parse tree (one element in tree for each token in
     // the query) has a corresponding user value which we create and insert here,
     // during query pre-processing.
     IQueryParseUserObject* uo = qnode.GetUserObject();
     TEvalResult* tree_val = dynamic_cast<TEvalResult*>(uo);
     if (!tree_val) {
         tree_val = new TEvalResult(&tr);
         qnode.SetUserObject(tree_val);
     }

     //
     // Do any required analyisis or type-conversions to the individual query nodes
     // here during query pre-processing. This is more efficient since conversions or 
     // lookups done here only need to be done once.  
     //

     tree_val->SetIsDataField(false); 
     tree_val->SetScope(scope);
     tree_val->SetDataType(QueryValueType::eUndefined);

     // If this query token is a string, determine if it is the name of a data field
     // or (if not) if the string can also be converted to another type, e.g. bool.
     if (qnode.GetType() == CQueryParseNode::eString) {          
         //
         // If the string value (before parsing) is enclosed in quotes, then don't 
         // try to look the string up in the data - it's just a string. 
         if ((qnode.GetOrig().length() >= 2) &&
             ((qnode.GetOrig()[0] == '\"' && 
             qnode.GetOrig()[qnode.GetOrig().length()-1] == '\"'))) {
           
             tree_val->m_String = qnode.GetStrValue();
         }
         // Also check to see if string was enclosed in single quotes.  Single quotes
         // are (currently) not stripped, so we check for them in the parsed token
         // and strip them if they are there.
         else if ((qnode.GetOrig().length() >= 2) &&
             ((qnode.GetOrig()[0] == '\'' && 
             qnode.GetOrig()[qnode.GetOrig().length()-1] == '\''))) {                
                 // If the single quotes were not stripped by the parser, strip
                 // them here
                 if (qnode.GetStrValue().length() == qnode.GetOrig().length())
                     tree_val->m_String = qnode.GetStrValue().substr(1, 
                     qnode.GetStrValue().length()-2);
                 else
                     tree_val->m_String = qnode.GetStrValue();
         }
         // No enclosing quotes - look it up to see if it is an identfier for a 
         // data field
         else if (m_QExec->HasIdentifier(qnode.GetStrValue())) {
             tree_val->SetIsDataField(true);
             tree_val->SetFieldID(m_QExec->GetIdentifier(qnode.GetStrValue()));

             // Check name to see if it is a seq-id field:
             if (NStr::Compare(qnode.GetStrValue(), "seq-id", NStr::eNocase) == 0) {
                 tree_val->SetDataType(QueryValueType::eFieldSeqID);
             }

             // look up field type.  Not all query execution environments will 
             // have this (schema) information available.
             CQueryParseNode::EType field_type = 
                 m_QExec->IdentifierType(qnode.GetStrValue());

             switch (field_type) {
                 case CQueryParseNode::eBoolConst:
                     tree_val->SetDataType(QueryValueType::eFieldBool);
                     break;
                 case CQueryParseNode::eIntConst:
                     tree_val->SetDataType(QueryValueType::eFieldInt);
                     break;
                 case CQueryParseNode::eFloatConst:
                     tree_val->SetDataType(QueryValueType::eFieldFloat);
                     break;
                 case CQueryParseNode::eString:
                     tree_val->SetDataType(QueryValueType::eFieldString);
                     break;
                 default:
                     // Field type not available (we will have to determine it
                     // at evaluation time)
                     break;
             }
         }
         // String without quotes that is not a data field
         else {            
             tree_val->m_String = qnode.GetStrValue();
         }

         //
         // If the string is not a data field, determine if it can
         // be converted to a boolean, integer, or float in that order
         // of priority (bool is defined as: {true,false,yes,no,y,n,t,f},
         // not 1 or 0 - those are integers). If it can't be converted,
         // its type is just string.  If it can be converted, its type is 
         // String{Bool, Int, Float}. This allows us, if needed, to have 
         // different promotion rules for a > 27  vs a > "27" (type Int
         // vs. StringInt)
         //
         if (!tree_val->IsDataField()) {
             try {
                 tree_val->m_Bool = NStr::StringToBool(tree_val->m_String);
                 tree_val->SetDataType(QueryValueType::eStringBool);
             }
             catch (CStringException&) {
                 // See if it can be an integer (this will not convert floats, e.g.
                 // 1.0 will not convert)
                 try {
                     tree_val->m_Int = NStr::StringToInt(tree_val->m_String);
                     tree_val->SetDataType(QueryValueType::eStringInt);
                 }
                 catch (CStringException&) {
                     // See if it can be a float:
                     try {
                         tree_val->m_Double = NStr::StringToDouble(tree_val->m_String);
                         tree_val->SetDataType(QueryValueType::eStringFloat);
                     }
                     catch (CStringException&) {
                         // It's just a string:
                         tree_val->SetDataType(QueryValueType::eString);
                     }
                 }
             }
         }
     }
     //
     // Node in tree was not a string, so it can't be a data field.  Check if
     // it was one of the other primitive data types.
     else if (qnode.GetType() == CQueryParseNode::eBoolConst) {
         tree_val->SetDataType(QueryValueType::eBool);
         tree_val->m_Bool = qnode.GetBool();
     }     
     else if (qnode.GetType() == CQueryParseNode::eIntConst) {
         tree_val->SetDataType(QueryValueType::eInt);
         tree_val->m_Int = qnode.GetInt();
     }
     else if (qnode.GetType() == CQueryParseNode::eFloatConst) {
         tree_val->SetDataType(QueryValueType::eFloat);
         tree_val->m_Double = qnode.GetDouble();
     }
     else if (qnode.GetType() == CQueryParseNode::eIdentifier) {
        if (m_QExec->HasIdentifier(qnode.GetStrValue())) {
            tree_val->SetIsDataField(true);
            tree_val->SetFieldID(m_QExec->GetIdentifier(qnode.GetStrValue()));

            // Check name to see if it is a seq-id field:
            if (NStr::Compare(qnode.GetStrValue(), "seq-id", NStr::eNocase) == 0) {
                 tree_val->SetDataType(QueryValueType::eFieldSeqID);
            }

            // look up field type.  Not all query execution environments will 
            // have this (schema) information available.
            CQueryParseNode::EType field_type =
                 m_QExec->IdentifierType(qnode.GetStrValue());

             switch (field_type) {
             case CQueryParseNode::eBoolConst:
                 tree_val->SetDataType(QueryValueType::eFieldBool);
                 break;
             case CQueryParseNode::eIntConst:
                 tree_val->SetDataType(QueryValueType::eFieldInt);
                 break;
             case CQueryParseNode::eFloatConst:
                 tree_val->SetDataType(QueryValueType::eFieldFloat);
                 break;
             case CQueryParseNode::eString:
                 tree_val->SetDataType(QueryValueType::eFieldString);
                 break;
             default:
                 // Field type not available (we will have to determine it
                 // at evaluation time)
                 break;
             }
         }
     }
     //
     // If a node in the query tree is not a primitive data type, then
     // it must be a logical or comparison operator.  These query objects
     // have a type of BoolConst since they are the boolean result of a
     // subexpression.  (we could do additional validation checks
     // here, or check for expressions supported by our query execution
     // object, throwing an error for any expression not included...)
     else if (!qnode.IsValue() && 
              qnode.GetType() != CQueryParseNode::eFunction &&
              qnode.GetType() != CQueryParseNode::eSelect &&
              qnode.GetType() != CQueryParseNode::eFrom &&
              qnode.GetType() != CQueryParseNode::eWhere &&
              qnode.GetType() != CQueryParseNode::eList) {
         tree_val->SetDataType(QueryValueType::eBoolResult);

        CQueryFunctionBase::TArgVector args;
        this->MakeArgVector(tr, args); 

        // Check if operator has a valid number of parameters.  Normal query
        // parsing seems to catch these so this is more of a verification.
        if (args.size() < GetArgCountMin(qnode.GetType()) || 
            args.size() > GetArgCountMax(qnode.GetType()) ) {
            NCBI_THROW(CQueryExecException, eWrongArgumentCount, 
                "Error: operator " +
                qnode.GetNodeTypeAsString() +
                " has " + NStr::SizetToString(args.size()) + " operands.");
        }        

        // Many expressions have just 2 arguments (==, !=, <, >, <=, >=, etc)
        // The two expressions that have > 2 args are 'between' and 'in'
        // and for both of those, we evaluate the first argument against each 
        // of the subsequent arguments.  We also have the NOT expression.
        // Also AND and OR can have (after tree flattening) > 2 operands.
        TEvalResult* val1 = GetQueryNodeValue(*args[0]);
        for (size_t i=1; i<args.size(); ++i) {
            TEvalResult* val2 = GetQueryNodeValue(*args[i]);

            if (val1->GetDataType() != QueryValueType::eUndefined &&
                val2->GetDataType() != QueryValueType::eUndefined) {

                if (qnode.IsLogic()) {
                    
                    // These could throw an exception if a (static)
                    // type cannot be converted to a boolean.
                    if (val1->GetQueryNode()->GetValue().IsValue()) {
                        if (!val1->IsDataField())
                            val1->PromoteTo(QueryValueType::eBool);
                        else
                            val1->SetDataType(QueryValueType::eFieldBool);
                    }

                    if (val2->GetQueryNode()->GetValue().IsValue()) {
                        if (!val2->IsDataField())
                            val2->PromoteTo(QueryValueType::eBool);
                        else
                            val2->SetDataType(QueryValueType::eFieldBool);
                    }

                    tree_val->AddPromotedType(CPromoteRule(qnode.GetType(), 
                                                           val1->GetDataType(),
                                                           val2->GetDataType(),
                                                           QueryValueType::eBool));
                }
                else {                                   
                    CPromoteRule pr(qnode.GetType(), 
                                    val1->GetDataType(), 
                                    val2->GetDataType(),
                                    QueryValueType::eUndefined);

                    QueryValueType::EBaseType promoted_type = 
                        GetPromotedType(pr);
                       
                    // Both data types are defined (otherwise we would not be here) but
                    // there is no valid type-basis for comparison.
                    if (promoted_type == QueryValueType::eUndefined) {
                        string msg = "Unable to compare: " +
                            args[0]->GetValue().GetOriginalText() + 
                            " with: " +
                            args[i]->GetValue().GetOriginalText() + 
                            " using operator: " + 
                            qnode.GetNodeTypeAsString();                  
                        NCBI_THROW(CQueryExecException, 
                            eIncompatibleType, 
                            msg);                        
                    }
                    else {
                        // This can throw an exception that a (static) type from
                        // the query string cannot be promoted to the target
                        // promotion type.
                        pr.m_PromotedType = promoted_type;
                        tree_val->AddPromotedType(pr);
                        //applies to 1 or many??
                        if (!val1->IsDataField())
                            val1->PromoteTo(promoted_type);
                        if (!val2->IsDataField())
                            val2->PromoteTo(promoted_type);
                    }
                }
            }
        }
    }
}

QueryValueType::EBaseType
CQueryFuncPromoteBase::ResolveAndPromote(size_t comparison_idx,
                                         CQueryParseTree::TNode& qnode,
                                         CQueryParseTree::TNode* arg1,
                                         CQueryParseTree::TNode* arg2)
{
    TEvalResult* node_value = GetQueryNodeValue(qnode);
    TEvalResult* val1 = GetQueryNodeValue(*arg1);
    TEvalResult* val2 = GetQueryNodeValue(*arg2);

    // Get underlying data and data type.  May require extracting
    // data from data source (if value node is an identifier).  Data 
    // type for comparison depends potentially on comparison type 
    // and underlying type of both nodes.
    QueryValueType::EBaseType promoted_type = QueryValueType::eUndefined;

    // This returns false if types don't match - maybe we should force
    // promotion to the matching promote entry, if available.
    if ( !node_value->HasPromoteType(comparison_idx, 
                                     val1->GetDataType(),
                                     val2->GetDataType()) )  {
        // Determine the underlying type for each argument (e.g. is it a boolean, a
        // string that can be converted to a boolean, etc.)
        bool data_available;

        data_available = SetCompareType(*val1);
        if (data_available)
            data_available = SetCompareType(*val2);

        // Not every element of every data source, e.g. node in a tree
        // has every property. When properties or fields are not available
        // return undefined so that the comparison will evaluate to false.
        if (!data_available) {
            return QueryValueType::eUndefined;
        }

        CPromoteRule pr;
        pr.m_CompareOperator = qnode.GetValue().GetType();
        pr.m_Type1 = val1->GetDataType();
        pr.m_Type2 = val2->GetDataType();

        promoted_type = GetPromotedType(pr);

        if (promoted_type == QueryValueType::eUndefined) {
            // Can't convert - not necessarily a real error e.g., a 
            // field may have a blank value where usually it as an int.
            // So just set result to false here if either of the values
            // came from a data field. If neither came from a field,
            // this has bad syntax so throw an exception
            if (!val1->IsDataField() && !val2->IsDataField()) {                                               
                string msg = "Unable to compare: " +
                    arg1->GetValue().GetOriginalText() + 
                    " with: " +
                    arg2->GetValue().GetOriginalText() + 
                    " using operator: " + 
                    qnode.GetValue().GetNodeTypeAsString();                  
                NCBI_THROW(CQueryExecException, 
                    eIncompatibleType, 
                    msg);
            }               

            TEvalResult* eval_res = GetQueryNodeValue(qnode);
            eval_res->SetValue(qnode->IsNot() ? true : false);
            return QueryValueType::eUndefined;
        }                
    }
    else {
        promoted_type = node_value->GetPromoteType(comparison_idx);

        // If the node represents an identifier, this retrieves
        // the corresponding value from the data source.
        bool found;
        found = ResolveFieldValue(*val1);

        // Not every element of every data source, e.g. node in a tree
        // has every property. When properties or fields are not available
        // return undefined so that the comparison will evaluate to false.
        if (!found) {
            return QueryValueType::eUndefined;
        }

        found = ResolveFieldValue(*val2);

        if (!found) {
            return QueryValueType::eUndefined;
        }
    }

    // Promote both data elements to the required comparison (may not require a
    // change if types were already compatible)
    val1->PromoteTo(promoted_type);
    val2->PromoteTo(promoted_type);

    return promoted_type;
}


///////////////////////////////////////////////////////////////////////////////
/// class CQueryFunctionRTVar implementation
///
void CQueryFuncRTVar::Evaluate(CQueryParseTree::TNode& qnode)
{
    _ASSERT(qnode.GetValue().GetType() == CQueryParseNode::eSelect);
    CMacroQueryExec* macro_query_exec = dynamic_cast<CMacroQueryExec*>(m_QExec);
    CQueryNodeValue* node_value = dynamic_cast<CQueryNodeValue*>(qnode->GetUserObject());
    _ASSERT(macro_query_exec && node_value);

    if (macro_query_exec && node_value) {
        node_value->SetRef(macro_query_exec->GetOrCreateRTVar(qnode.GetValue().GetOriginalText()));
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CQueryFuncAssignment
///
void CQueryFuncAssignment::Evaluate(CQueryParseTree::TNode& qnode)
{
    CQueryFunctionBase::TArgVector user_args;
    MakeArgVector(qnode, user_args);

    vector<CRef<CQueryNodeValue>> arguments;
    arguments.reserve(user_args.size());
    for (auto&& it : user_args) {
        CRef<CQueryNodeValue> user_obj = Ref(dynamic_cast<CQueryNodeValue*>((*it)->GetUserObject()));
        if (user_obj) {
            arguments.push_back(user_obj);
        }
    }

    size_t arg_size = arguments.size();
    if (arg_size < 2 || arguments[0]->GetDataType() != QueryValueType::eRef) {
        for (auto& iter : arguments) {
            if (iter->GetDataType() == QueryValueType::eUndefined) {
                TEvalResult* eval_res = GetQueryNodeValue(qnode);
                eval_res->SetDataType(QueryValueType::eUndefined);
                return;
            }
        }
        NCBI_THROW(macro::CMacroExecException, eWrongArguments,
            "Wrong type of argument in function: " + qnode.GetValue().GetOrig());
    }

    CRef<CQueryNodeValue> node_value = arguments[1];
    arguments[0]->AssignToRef(*node_value);
}

///////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncFunction implementation
///
void CQueryFuncFunction::Evaluate(CQueryParseTree::TNode& qnode)
{
    string fname = qnode->GetOriginalText();

    // Get vector of arguments to function and resolve values for any which come
    // from the data source (value types and macro variables would already be
    // resolved at this point).
    CQueryFunctionBase::TArgVector args;
    this->MakeArgVector(qnode, args);

    // for each function argument:
    for (size_t i = 0; i < args.size(); ++i) {
        TEvalResult* val = GetQueryNodeValue(*args[i]);

        // we don't check for success here, but if we do not find a 
        // parameter for a function the function could fail, but 
        // that is better reported by the function.
        SetCompareType(*val);
    }

    m_QExec->CallFunction(fname, qnode);
}


/******************************************************************************
 *
 * CQueryFuncPromoteValue Implementation
 *
 *****************************************************************************/ 

void CQueryFuncPromoteValue::Evaluate(CQueryParseTree::TNode& qnode)
{
    TEvalResult* res = MakeQueryNodeValue(qnode);

    // We only use one instance of the query parse tree to evaluate all
    // the nodes, so that means we inherit the value in 'res' from
    // the previous execution.  That's why we reset the value here
    // in the queries 'leaf' (value) nodes.
    res->Reset();
}


/******************************************************************************
*
* CQueryFuncPromoteIdentifier Implementation
*
*****************************************************************************/

void CQueryFuncPromoteIdentifier::Evaluate(CQueryParseTree::TNode& qnode)
{
    TEvalResult* res = MakeQueryNodeValue(qnode);

    // We only use one instance of the query parse tree to evaluate all
    // the nodes, so that means we inherit the value in 'res' from
    // the previous execution.  That's why we reset the value here
    // in the queries 'leaf' (value) nodes.
    res->Reset();

    // Resolve any node values that are varaibles here.  If an identifier
    // is a value type, it is already set and if it comes from the data
    // source (a field type), then it is (looked up and) resolved when an
    // operator is applied to it
    string identifier = qnode.GetValue().GetOriginalText();
    CMacroQueryExec* mqe = dynamic_cast<CMacroQueryExec*>(m_QExec);
    
    if (mqe && mqe->GetMacroRep()) {
        // This will also set the type if the variable is unidentified
        bool found = mqe->GetMacroRep()->GetNodeValue(identifier, *res);
        if (!found) {
            // Try to look it up in the run time variable list
            mqe->ResolveRTVar(identifier, *res);
        }
    }
}


/******************************************************************************
 *
 * CQueryFuncPromoteLogic Implementation
 *
 *****************************************************************************/ 
void CQueryFuncPromoteLogic::Evaluate(CQueryParseTree::TNode& qnode)
{
    CQueryFunctionBase::TArgVector args;
    this->MakeArgVector(qnode, args);

    if (args.size() == 0) {
        // If a logical function has no arguments.... Is it truely logical?
        NCBI_THROW(CQueryExecException, eWrongArgumentCount, 
                    "No arguments for logical function " + 
                    qnode->GetNodeTypeAsString());                      
    }

    // operation interpretor
    //        
    CQueryParseNode::EType node_type = qnode.GetValue().GetType();

    // check if it is a binary operator:
    if (node_type == CQueryParseNode::eSub ||
        node_type == CQueryParseNode::eXor) {

            if (args.size() != 2) {
                NCBI_THROW(CQueryExecException, eWrongArgumentCount, 
                    "Error: Binary operator: " +
                    qnode.GetValue().GetNodeTypeAsString() +
                    " has: " + NStr::SizetToString(args.size()) + " operands."); 
            }

            // Since this is a logical operator, both arguments must be the
            // boolean result of a previous expression or promotable to
            // boolean.  Static types may have already been promoted to bool,
            // but in thas case "PromoteTo(eBool)" is a no-op.
            TEvalResult* val1 = GetQueryNodeValue(*args[0]);
            TEvalResult* val2 = GetQueryNodeValue(*args[1]);

            if (val1->IsDataField()) {              
                if (!m_QExec->ResolveIdentifier(val1->GetFieldID(),
                                                val1->m_Bool)) {                  
                    NCBI_THROW(CQueryExecException, eIncompatibleType, 
                               "Error: Unable to convert field: " + 
                               args[0]->GetValue().GetStrValue() +
                               " to boolean value.");                              
                }
                val1->SetValue(val1->m_Bool);
            } else if (args[0]->GetValue().IsValue()) {
                val1->PromoteTo(QueryValueType::eBool);
                val1->SetValue(val1->m_Bool);
            }

            if (val2->IsDataField()) {              
                if (!m_QExec->ResolveIdentifier(val2->GetFieldID(),
                                                val2->m_Bool)) {                  
                    NCBI_THROW(CQueryExecException, eIncompatibleType, 
                               "Error: Unable to convert field: " + 
                               args[1]->GetValue().GetStrValue() +
                               " to boolean value.");                              
                }
                val2->SetValue(val1->m_Bool);
            } else if (args[1]->GetValue().IsValue()) {
                val2->PromoteTo(QueryValueType::eBool);
                val2->SetValue(val2->m_Bool);
            }

            TEvalResult* res = MakeQueryNodeValue(qnode);
            switch (node_type) {   
                case CQueryParseNode::eSub:
                    res->SetValue(val1->GetValue() && !val2->GetValue());
                    break;
                case CQueryParseNode::eXor:
                    res->SetValue(val1->GetValue() != val2->GetValue());
                    break;
                default:
                    break;
            }

            // not can be an attribute of a node, but this is for !=.  I don't
            // think it can show up as part of a logical expression, since
            // we have the separate 'NOT' operator for that.
            if (qnode.GetValue().IsNot())
                res->SetValue(!res->GetValue());
    }
    else if (node_type == CQueryParseNode::eNot) {
        if (args.size() != 1) {
            NCBI_THROW(CQueryExecException, eWrongArgumentCount, 
                "Error: Unary NOT operator has: " +
                NStr::SizetToString(args.size()) + " operands."); 
        }

        TEvalResult* val1 = GetQueryNodeValue(*args[0]);
        TEvalResult* res = MakeQueryNodeValue(qnode);

        if (val1->IsDataField()) {              
            if (!m_QExec->ResolveIdentifier(val1->GetFieldID(),
                val1->m_Bool)) {                  
                    NCBI_THROW(CQueryExecException, eIncompatibleType, 
                        "Error: Unable to convert field: " + 
                        args[0]->GetValue().GetStrValue() +
                        " to boolean value.");                              
            }
            val1->SetValue(val1->m_Bool);
        } else if (args[0]->GetValue().IsValue()) {
            val1->PromoteTo(QueryValueType::eBool);
            val1->SetValue(val1->m_Bool);
        }

        res->SetValue(!val1->GetValue());
    }
    else {        
        NCBI_THROW(CQueryExecException, eExecParseError, 
            "Error: Unexpected logical operand: " +
            qnode->GetOriginalText());
    }
}

/******************************************************************************
*
* CQueryFuncPromoteAndOr Implementation
*
*****************************************************************************/
void CQueryFuncPromoteAndOr::Evaluate(CQueryParseTree::TNode& qnode)
{
    CQueryFunctionBase::TArgVector args;
    this->MakeArgVector(qnode, args);

    if (args.size() == 0) {
        // If a logical function has no arguments.... Is it truely logical?
        NCBI_THROW(CQueryExecException, eWrongArgumentCount,
            "No arguments for logical function " +
            qnode->GetNodeTypeAsString());
    }

    // operation interpretor
    //        
    CQueryParseNode::EType node_type = qnode.GetValue().GetType();

 
    // check if it is an n-ary operator (trees can be flattened to combine 
    // adjacent ands and ors into a single operator (a or b or c) == (or: a,b,c)
    if (node_type == CQueryParseNode::eAnd ||
        node_type == CQueryParseNode::eOr) {

        if (args.size() < 2) {
            NCBI_THROW(CQueryExecException, eWrongArgumentCount,
                "Error: n-ary operator: " +
                qnode.GetValue().GetNodeTypeAsString() +
                " has: " + NStr::SizetToString(args.size()) + " operands.");
        }

        TEvalResult* res = MakeQueryNodeValue(qnode);

        // Set default answer based on logical operation type:
        bool result;
        if (node_type == CQueryParseNode::eAnd)
            result = true;
        else if (node_type == CQueryParseNode::eOr)
            result = false;

        // Check arguments 1 at a time. For AND, every argument must be true.
        // For OR, every argument must be false (or we break early)
        for (size_t i = 0; i < args.size(); ++i) {

            // Since these are logical operators, all arguments must be the
            // boolean result of a previous expression or promotable to
            // boolean.  Static types may have already been promoted to bool,
            // but in that case "PromoteTo(eBool)" is a no-op.
            
            // For efficiency, with the AND and OR logical operators only, 
            // we evalute any subexpressions one at a time since either all must
            // be true (AND) or none (OR).  So if node is not a simple value, 
            // traverse child subtree here:
            if (!args[i]->GetValue().IsValue()) {
                CQueryExecEvalFunc visitor(*m_QExec);
                TreeDepthFirstTraverse(*args[i], visitor);
            }

            TEvalResult* val = GetQueryNodeValue(*args[i]);

            if (val->IsDataField()) {
                if (!m_QExec->ResolveIdentifier(val->GetFieldID(),
                    val->m_Bool)) {
                    NCBI_THROW(CQueryExecException, eIncompatibleType,
                        "Error: Unable to convert field: " +
                        args[i]->GetValue().GetStrValue() +
                        " to boolean value.");
                }
                val->SetValue(val->m_Bool);
            }
            else if (args[i]->GetValue().IsValue() || args[i]->GetValue().GetType() == CQueryParseNode::eFunction) {
                val->PromoteTo(QueryValueType::eBool);
                val->SetValue(val->m_Bool);
            }

            // We can break early if any AND parameters are false
            // or any OR parameters are true:
            if (node_type == CQueryParseNode::eAnd) {
                if (!val->GetValue()) {
                    result = false;
                    break;
                }
            }
            else if (node_type == CQueryParseNode::eOr) {
                if (val->GetValue()) {
                    result = true;
                    break;
                }
            }
        }
        res->SetValue(result);

        // not can be an attribute of a node, but this is for !=.  I don't
        // think it can show up as part of a logical expression, since
        // we have the separate 'NOT' operator for that.
        if (qnode.GetValue().IsNot())
            res->SetValue(!res->GetValue());
    }
    else {
        NCBI_THROW(CQueryExecException, eExecParseError,
            "Error: Unexpected logical operand: " +
            qnode->GetOriginalText());
    }
}



/******************************************************************************
 *
 * CQueryFuncPromoteCompare Implementation
 *
 *****************************************************************************/ 

CQueryFuncPromoteCompare::CQueryFuncPromoteCompare(CQueryParseNode::EType,
                                                   NStr::ECase c,
                                                   CStringMatching::EStringMatching matching)
: CQueryFuncPromoteBase()
{ 
    SetCaseSensitive(c);
    SetStringMatching(matching);
}

/******************************************************************************
 *
 * CQueryFuncEqualityCompares Implementation
 *
 *****************************************************************************/ 
void CQueryFuncEqualityCompares::InitTypePromotionRules(CQueryParseNode::EType op_type)
{    
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eBoolResult, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eInt, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eStringBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eStringInt, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eStringFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFieldString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFieldFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFieldInt, QueryValueType::eBool);

    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eInt, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eStringBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eStringInt, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eStringFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFieldString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFieldFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFieldInt, QueryValueType::eBool);

    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eInt, QueryValueType::eInt);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eSeqID, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eStringBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eStringInt, QueryValueType::eInt);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eStringFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFieldSeqID, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFieldFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFieldInt, QueryValueType::eInt);

    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eStringBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eStringInt, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eStringFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFieldFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFieldInt, QueryValueType::eFloat);

    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eSeqID, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eString, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eStringBool, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eStringInt, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eStringFloat, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eFieldSeqID, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eFieldString, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eFieldBool, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eFieldFloat, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eFieldInt, QueryValueType::eSeqID);

    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eStringBool, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eStringInt, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eStringFloat, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eFieldSeqID, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eFieldBool, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eFieldFloat, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eFieldInt, QueryValueType::eString);

    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eStringBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eStringInt, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eStringFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eFieldFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eFieldInt, QueryValueType::eBool);

    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eStringInt, QueryValueType::eInt);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eStringFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eFieldSeqID, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eFieldFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eFieldInt, QueryValueType::eInt);

    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eStringFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eFieldFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eFieldInt, QueryValueType::eFloat);

    AddPromoteTypes(op_type, QueryValueType::eFieldSeqID, QueryValueType::eFieldSeqID, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eFieldSeqID, QueryValueType::eFieldString, QueryValueType::eSeqID);
    AddPromoteTypes(op_type, QueryValueType::eFieldSeqID, QueryValueType::eFieldBool, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eFieldSeqID, QueryValueType::eFieldFloat, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eFieldSeqID, QueryValueType::eFieldInt, QueryValueType::eSeqID);

    AddPromoteTypes(op_type, QueryValueType::eFieldString, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFieldString, QueryValueType::eFieldBool, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFieldString, QueryValueType::eFieldFloat, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFieldString, QueryValueType::eFieldInt, QueryValueType::eString);

    AddPromoteTypes(op_type, QueryValueType::eFieldBool, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eFieldBool, QueryValueType::eFieldFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eFieldBool, QueryValueType::eFieldInt, QueryValueType::eBool);

    AddPromoteTypes(op_type, QueryValueType::eFieldFloat, QueryValueType::eFieldFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eFieldFloat, QueryValueType::eFieldInt, QueryValueType::eFloat);

    AddPromoteTypes(op_type, QueryValueType::eFieldInt, QueryValueType::eFieldInt, QueryValueType::eInt);

    std::sort(m_PromoteRules.begin(), m_PromoteRules.end());
    m_PromoteRules.erase(std::unique(m_PromoteRules.begin(), m_PromoteRules.end()), 
        m_PromoteRules.end());
}

/******************************************************************************
 *
 * CQueryFuncPromoteEq Implementation
 *
 *****************************************************************************/ 
CQueryFuncPromoteEq::CQueryFuncPromoteEq(NStr::ECase c, CStringMatching::EStringMatching matching)
: CQueryFuncEqualityCompares(CQueryParseNode::eEQ, c, matching) 
{ 
    InitTypePromotionRules(CQueryParseNode::eEQ); 
}

void CQueryFuncPromoteEq::Evaluate(CQueryParseTree::TNode& qnode)
{
    CQueryFunctionBase::TArgVector args;
    this->MakeArgVector(qnode, args);   

    // get operation type
    CQueryParseNode::EType node_type = qnode.GetValue().GetType();

    // handle equality comparison
    if (node_type == CQueryParseNode::eEQ) {

        TEvalResult* eval_res = GetQueryNodeValue(qnode);

        if (args.size() != 2) {
            NCBI_THROW(CQueryExecException, eWrongArgumentCount, 
                "Error: Binary operator " +
                qnode.GetValue().GetNodeTypeAsString() +
                " has " + NStr::SizetToString(args.size()) + " operands.");
        }


        // Get underlying data and data type.  Data my require extracting
        // data from current record (tree node).  Data type for comparison
        // depends potentially on comparison type and underlying type of both
        // nodes.
        QueryValueType::EBaseType promoted_type = 
            ResolveAndPromote(0, qnode, args[0], args[1]);

        TEvalResult* val1 = GetQueryNodeValue(*args[0]);
        TEvalResult* val2 = GetQueryNodeValue(*args[1]);

        // If a field was not present in the data, the equivalent of a NULL
        // value in SQL, we return by default true for 'not equal' comparisons
        // and false for 'equal' comparisons.  But as a special case, if the
        // field is being compared to the (constant) empty string "", we
        // we reverse that since users would expect empty (blank) and missing 
        // (null) to be equivalent.
        // This is a bit of a hack since we don't support IS NULL
        // keyword like SQL does (and IS NULL is not intuitive for users)
        if (promoted_type == QueryValueType::eUndefined) {                       
            // Add special case for prop="" and prop!="" (again because we don't
            // support null).  A user would expect that field!=""
            // returns true for null fields, and that field="" returns
            // false for null fields.
            if (val1->IsDataField() &&
                val2->GetDataType() == QueryValueType::eString &&
                val2->m_String == "") {
                eval_res->SetValue(qnode->IsNot() ? false : true);
            }
            else if (val2->IsDataField() &&
                val1->GetDataType() == QueryValueType::eString &&
                val1->m_String == "") {
                eval_res->SetValue(qnode->IsNot() ? false : true);
            }
            else {
                // not comparing to blank, return true for '!=' and false for '='
                eval_res->SetValue(qnode->IsNot() ? true : false);
            }

            return;
        }

        if (promoted_type == QueryValueType::eBool) {
            eval_res->SetValue(val1->m_Bool == val2->m_Bool);
        }
        else if (promoted_type == QueryValueType::eInt) {
            eval_res->SetValue(val1->m_Int == val2->m_Int);
        }
        else if (promoted_type == QueryValueType::eFloat) {
            eval_res->SetValue(val1->m_Double == val2->m_Double);
        }
        else if (promoted_type == QueryValueType::eString) {
            // Support phonetic, wildcard and reg-ex comparisons
            // We need to pass the pattern (if there is one) to the
            // CStringMatching ctor, and then pass the field to 
            // CStringMatching::MatchString(). If neither is a field or
            // both are fields, do simple string comparison.
            // Also ePlainSearch in CStringMatching looks for substrings but
            // here for 'name'=value, we only want exact matches.
            if (val1->IsDataField() && !val2->IsDataField() && 
                (m_StringMatchAlgo != CStringMatching::ePlainSearch)) {
                    CStringMatching  sm(val2->m_String.c_str(), m_StringMatchAlgo, GetCaseSensitive());
                    bool result = sm.MatchString(val1->m_String.c_str());
                    eval_res->SetValue(result);             
            }
            else if (val2->IsDataField() && !val1->IsDataField() && 
                (m_StringMatchAlgo != CStringMatching::ePlainSearch)) {
                    CStringMatching  sm(val1->m_String.c_str(), m_StringMatchAlgo, GetCaseSensitive());
                    bool result = sm.MatchString(val2->m_String.c_str());
                    eval_res->SetValue(result); 
            }
            else {
                int result = NStr::Compare(val1->m_String.c_str(), 
                                           val2->m_String.c_str(), 
                                           GetCaseSensitive());            
                eval_res->SetValue(result==0);
            }  
        }
        else if (promoted_type == QueryValueType::eSeqID) {
            try {
                // For efficiency, do a simple string check first, and
                // if true, do not continue.  Also, if either field is blank,
                // do not continue past the string check.
                int result = NStr::CompareNocase(val1->m_String.c_str(), 
                                                 val2->m_String.c_str());                                       
                if (result == 0) {
                    eval_res->SetValue(true);
                }
                else if (val1->m_String=="" || val2->m_String=="") {
                    eval_res->SetValue(false);
                }
                else {               
                    objects::CSeq_id seq_id1(val1->m_String);
                    objects::CSeq_id seq_id2(val2->m_String);

                    bool match = CSeqUtils::Match(seq_id1, seq_id2, val1->GetScope());

                    eval_res->SetValue(match);
                }
            }
            catch (CException& e) {
                NCBI_THROW(CQueryExecException, eObjManagerError, 
                    "Object Manager Error: " +
                    e.GetMsg()); 
            }            
        }
        // !=
        if (qnode->IsNot())
            eval_res->SetValue(!eval_res->GetValue());


    }
    else {
        NCBI_THROW(CQueryExecException, eExecParseError, 
            "Error - Unhandled comparison operator: " +
            qnode.GetValue().GetNodeTypeAsString());  
    }
}   

/******************************************************************************
 *
 * CQueryFuncPromoteIn Implementation
 *
 *****************************************************************************/ 
CQueryFuncPromoteIn::CQueryFuncPromoteIn(NStr::ECase c, CStringMatching::EStringMatching matching)
: CQueryFuncEqualityCompares(CQueryParseNode::eIn, c, matching)
{ 
    InitTypePromotionRules(CQueryParseNode::eIn); 
}

void CQueryFuncPromoteIn::Evaluate(CQueryParseTree::TNode& qnode)
{
    CQueryFunctionBase::TArgVector args;
    this->MakeArgVector(qnode, args);   

    // get operation type
    CQueryParseNode::EType node_type = qnode.GetValue().GetType();

    if (node_type == CQueryParseNode::eIn) {
        TEvalResult* eval_res = GetQueryNodeValue(qnode);

        // In operator can have any number of operands > 1:
        if (args.size() <= 1) {
            NCBI_THROW(CQueryExecException, eWrongArgumentCount, 
                "Error: 'IN' operator has " +
                NStr::SizetToString(args.size()) + " operands.");
        }

        // Get underlying data and data type. Since there may be any number
        // of operands, iterate through them and check each time if they
        // need to be promoted. This allows the user to mix types between 
        // the parms, e.g. : dist IN (0.57, "", 1) without throwing an 
        // exception.
        TEvalResult* val1 = GetQueryNodeValue(*args[0]);

        bool found = false;
        for (size_t i=1; i<args.size() && !found; ++i) {
            TEvalResult* val2 = GetQueryNodeValue(*args[i]);

            QueryValueType::EBaseType promoted_type = 
                ResolveAndPromote(i-1, qnode, args[0], args[i]);

            if (promoted_type != QueryValueType::eUndefined) {
                // Promote both data elements to the required comparison 
                //val1->PromoteTo(promoted_type, m_ExecEval);
                //val2->PromoteTo(promoted_type, m_ExecEval);

                if (promoted_type == QueryValueType::eBool) {
                    found = (val1->m_Bool == val2->m_Bool);
                }
                else if (promoted_type == QueryValueType::eInt) {
                    found = (val1->m_Int == val2->m_Int);
                }
                else if (promoted_type == QueryValueType::eFloat) {
                    found = (val1->m_Double == val2->m_Double);
                }
                else if (promoted_type == QueryValueType::eString) {              
                    // Support phonetic, wildcard and reg-ex comparisons
                    if (val1->IsDataField() && !val2->IsDataField() &&
                        (m_StringMatchAlgo != CStringMatching::ePlainSearch)) {
                            CStringMatching  sm(val2->m_String.c_str(), m_StringMatchAlgo, GetCaseSensitive());
                            found = sm.MatchString(val1->m_String.c_str());          
                    }
                    else if (val2->IsDataField() && !val1->IsDataField() &&
                        (m_StringMatchAlgo != CStringMatching::ePlainSearch)) {
                            CStringMatching  sm(val1->m_String.c_str(), m_StringMatchAlgo, GetCaseSensitive());
                            found = sm.MatchString(val2->m_String.c_str());
                    }
                    else {
                        int result = NStr::Compare(val1->m_String.c_str(), 
                                                   val2->m_String.c_str(), 
                                                   GetCaseSensitive());            
                        found = (result==0);
                    }          
                }
                else if (promoted_type == QueryValueType::eSeqID) {
                    try {
                        // For efficiency, do a simple string check first, and
                        // if true, do not continue.  Also, if either field is blank,
                        // do not continue past the string check.
                        int result = NStr::CompareNocase(val1->m_String.c_str(), 
                            val2->m_String.c_str());                                       
                        if (result == 0) {
                            found = true;
                        }
                        else if (val1->m_String!="" && val2->m_String!="") {
                            objects::CSeq_id seq_id1(val1->m_String);
                            objects::CSeq_id seq_id2(val2->m_String);

                            found = CSeqUtils::Match(seq_id1, seq_id2, val1->GetScope());
                        }
                    }
                    catch (CException& e) {
                        NCBI_THROW(CQueryExecException, eObjManagerError, 
                            "Object Manager Error: " +
                            e.GetMsg()); 
                    }            
                }                        
            }
        }

        if (qnode->IsNot())            
            eval_res->SetValue(!found); 
        else 
            eval_res->SetValue(found);
    }
    else {
        NCBI_THROW(CQueryExecException, eExecParseError, 
            "Error - Unhandled comparison operator: " +
            qnode.GetValue().GetNodeTypeAsString());  
    }
}   



/******************************************************************************
 *
 * CQueryFuncLike Implementation
 *
 *****************************************************************************/ 
CQueryFuncLike::CQueryFuncLike(NStr::ECase c)
: CQueryFuncPromoteCompare(CQueryParseNode::eLike, c)
{ 
    InitTypePromotionRules(CQueryParseNode::eLike); 
}

void CQueryFuncLike::InitTypePromotionRules(CQueryParseNode::EType op_type)
{
    //
    // Like: second parameter has to be string{,int,float,bool} and promotion is always to string for
    // comparison.  First argument can be anything except the result of a subexpression (boolean
    // result). Seq-ids in like expressions are treated as regular strings.
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eBool, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eBool, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eBool, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eBool, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eInt, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eInt, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eInt, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eInt, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFloat, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFloat, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFloat, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFloat, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eString, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eString, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eString, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eString, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eSeqID, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eSeqID, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eSeqID, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eSeqID, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringBool, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringBool, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringBool, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringBool, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringInt, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringInt, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringInt, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringInt, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringFloat, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringFloat, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringFloat, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eStringFloat, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldString, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldString, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldString, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldString, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldSeqID, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldSeqID, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldSeqID, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldSeqID, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldBool, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldBool, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldBool, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldBool, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldFloat, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldFloat, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldFloat, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldFloat, QueryValueType::eStringFloat, QueryValueType::eString));

    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldInt, QueryValueType::eString, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldInt, QueryValueType::eStringBool, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldInt, QueryValueType::eStringInt, QueryValueType::eString));
    m_PromoteRules.push_back(CPromoteRule(CQueryParseNode::eLike, 
        QueryValueType::eFieldInt, QueryValueType::eStringFloat, QueryValueType::eString));


    std::sort(m_PromoteRules.begin(), m_PromoteRules.end());
    m_PromoteRules.erase(std::unique(m_PromoteRules.begin(), m_PromoteRules.end()), 
        m_PromoteRules.end());
}

void CQueryFuncLike::Evaluate(CQueryParseTree::TNode& qnode)
{
    CQueryFunctionBase::TArgVector args;
    this->MakeArgVector(qnode, args);   

    // get operation type
    CQueryParseNode::EType node_type = qnode.GetValue().GetType();

    // handle all the binary comparisons
    if (node_type == CQueryParseNode::eLike) {

        TEvalResult* eval_res = MakeQueryNodeValue(qnode);

        if (args.size() != 2) {
            NCBI_THROW(CQueryExecException, eWrongArgumentCount, 
                "Error: Binary operator " +
                qnode.GetValue().GetNodeTypeAsString() +
                " has " + NStr::SizetToString(args.size()) + " operands.");
        }

        // Get underlying data and data type.  Data my require extracting
        // data from current record (tree node).  Data type for comparison
        // depends potentially on comparison type and underlying type of both
        // nodes.
        QueryValueType::EBaseType promoted_type = 
            ResolveAndPromote(0, qnode, args[0], args[1]);

        if (promoted_type == QueryValueType::eUndefined) {
            eval_res->SetValue(false);
            return;
        }

        TEvalResult* val1 = GetQueryNodeValue(*args[0]);
        TEvalResult* val2 = GetQueryNodeValue(*args[1]);

        if (promoted_type == QueryValueType::eString) {
            bool result = NStr::MatchesMask(val1->m_String.c_str(), 
                val2->m_String.c_str(), 
                GetCaseSensitive());
            eval_res->SetValue(result);
        }
        // !=
        if (qnode->IsNot())
            eval_res->SetValue(!eval_res->GetValue());

    }
    else {
        NCBI_THROW(CQueryExecException, eExecParseError, 
            "Error - Unhandled comparison operator: " +
            qnode.GetValue().GetNodeTypeAsString());  
    }
}   


/******************************************************************************
 *
 * CQueryFuncGtLtCompares Implementation
 *
 *****************************************************************************/ 
void CQueryFuncGtLtCompares::InitTypePromotionRules(CQueryParseNode::EType op_type)
{    
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eBoolResult, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eBoolResult, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eInt, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eStringBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eStringInt, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eStringFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFieldString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFieldFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBoolResult, QueryValueType::eFieldInt, QueryValueType::eBool);

    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eInt, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eStringBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eStringInt, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eStringFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFieldString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFieldFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eBool, QueryValueType::eFieldInt, QueryValueType::eBool);

    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eInt, QueryValueType::eInt);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eStringBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eStringInt, QueryValueType::eInt);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eStringFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFieldFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eInt, QueryValueType::eFieldInt, QueryValueType::eInt);

    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eString, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eStringBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eStringInt, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eStringFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFieldFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eFloat, QueryValueType::eFieldInt, QueryValueType::eFloat);

    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eSeqID, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eStringBool, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eStringInt, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eStringFloat, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eFieldBool, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eFieldFloat, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eSeqID, QueryValueType::eFieldInt, QueryValueType::eString);

    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eStringBool, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eStringInt, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eStringFloat, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eFieldBool, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eFieldFloat, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eString, QueryValueType::eFieldInt, QueryValueType::eString);

    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eStringBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eStringInt, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eStringFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eFieldFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringBool, QueryValueType::eFieldInt, QueryValueType::eBool);

    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eStringInt, QueryValueType::eInt);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eStringFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eFieldFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eStringInt, QueryValueType::eFieldInt, QueryValueType::eInt);

    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eStringFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eFieldSeqID, QueryValueType::eUndefined);
    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eFieldFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eStringFloat, QueryValueType::eFieldInt, QueryValueType::eFloat);

    AddPromoteTypes(op_type, QueryValueType::eFieldSeqID, QueryValueType::eFieldSeqID, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFieldSeqID, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFieldSeqID, QueryValueType::eFieldBool, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFieldSeqID, QueryValueType::eFieldFloat, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFieldSeqID, QueryValueType::eFieldInt, QueryValueType::eString);

    AddPromoteTypes(op_type, QueryValueType::eFieldString, QueryValueType::eFieldString, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFieldString, QueryValueType::eFieldBool, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFieldString, QueryValueType::eFieldFloat, QueryValueType::eString);
    AddPromoteTypes(op_type, QueryValueType::eFieldString, QueryValueType::eFieldInt, QueryValueType::eString);

    AddPromoteTypes(op_type, QueryValueType::eFieldBool, QueryValueType::eFieldBool, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eFieldBool, QueryValueType::eFieldFloat, QueryValueType::eBool);
    AddPromoteTypes(op_type, QueryValueType::eFieldBool, QueryValueType::eFieldInt, QueryValueType::eBool);

    AddPromoteTypes(op_type, QueryValueType::eFieldFloat, QueryValueType::eFieldFloat, QueryValueType::eFloat);
    AddPromoteTypes(op_type, QueryValueType::eFieldFloat, QueryValueType::eFieldInt, QueryValueType::eFloat);

    AddPromoteTypes(op_type, QueryValueType::eFieldInt, QueryValueType::eFieldInt, QueryValueType::eInt);

    std::sort(m_PromoteRules.begin(), m_PromoteRules.end());
    m_PromoteRules.erase(std::unique(m_PromoteRules.begin(), m_PromoteRules.end()), 
        m_PromoteRules.end());
}




/******************************************************************************
 *
 * CQueryFuncPromoteGtLt Implementation
 *
 *****************************************************************************/ 
CQueryFuncPromoteGtLt::CQueryFuncPromoteGtLt(CQueryParseNode::EType op_type,
                                             NStr::ECase c)
: CQueryFuncGtLtCompares(op_type, c)
{ 
    InitTypePromotionRules(op_type); 
}

void CQueryFuncPromoteGtLt::Evaluate(CQueryParseTree::TNode& qnode)
{
    CQueryFunctionBase::TArgVector args;
    this->MakeArgVector(qnode, args); 

    // get operation type
    CQueryParseNode::EType node_type = qnode.GetValue().GetType();

    // handle all the binary comparisons
    if (node_type == CQueryParseNode::eGT ||
        node_type == CQueryParseNode::eGE ||
        node_type == CQueryParseNode::eLT ||
        node_type == CQueryParseNode::eLE) {

            TEvalResult* eval_res = GetQueryNodeValue(qnode); 

            if (args.size() != 2) {
                NCBI_THROW(CQueryExecException, eWrongArgumentCount, 
                    "Error: Binary operator " +
                    qnode.GetValue().GetNodeTypeAsString() +
                    " has " + NStr::SizetToString(args.size()) + " operands.");
            }

            // Get underlying data and data type.  May require extracting
            // data from data source (if value node is an identifier).  Data 
            // type for comparison depends potentially on comparison type 
            // and underlying type of both nodes.
            QueryValueType::EBaseType promoted_type = 
                ResolveAndPromote(0, qnode, args[0], args[1]);

            if (promoted_type == QueryValueType::eUndefined) {
                eval_res->SetValue(false);
                return;
            }

            TEvalResult* val1 = GetQueryNodeValue(*args[0]);
            TEvalResult* val2 = GetQueryNodeValue(*args[1]);

            /// Apply each of the possible comparison types for eache of the possible
            /// promotion types
            switch (node_type) {
                case CQueryParseNode::eGT:
                    if (promoted_type == QueryValueType::eBool) {
                        eval_res->SetValue(val1->m_Bool > val2->m_Bool);
                    }
                    else if (promoted_type == QueryValueType::eInt) {
                        eval_res->SetValue(val1->m_Int > val2->m_Int);
                    }
                    else if (promoted_type == QueryValueType::eFloat) {
                        eval_res->SetValue(val1->m_Double > val2->m_Double);
                    }
                    else if (promoted_type == QueryValueType::eString) {
                        int result = NStr::Compare(val1->m_String.c_str(), 
                            val2->m_String.c_str(), 
                            GetCaseSensitive());
                        eval_res->SetValue(result>0);
                    }
                    break;        

                case CQueryParseNode::eGE:
                    if (promoted_type == QueryValueType::eBool) {
                        eval_res->SetValue(val1->m_Bool >= val2->m_Bool);
                    }
                    else if (promoted_type == QueryValueType::eInt) {
                        eval_res->SetValue(val1->m_Int >= val2->m_Int);
                    }
                    else if (promoted_type == QueryValueType::eFloat) {
                        eval_res->SetValue(val1->m_Double >= val2->m_Double);
                    }
                    else if (promoted_type == QueryValueType::eString) {
                        int result = NStr::Compare(val1->m_String.c_str(), 
                            val2->m_String.c_str(), 
                            GetCaseSensitive());
                        eval_res->SetValue(result>=0);
                    }
                    break;

                case CQueryParseNode::eLT:
                    if (promoted_type == QueryValueType::eBool) {
                        eval_res->SetValue(val1->m_Bool < val2->m_Bool);
                    }
                    else if (promoted_type == QueryValueType::eInt) {
                        eval_res->SetValue(val1->m_Int < val2->m_Int);
                    }
                    else if (promoted_type == QueryValueType::eFloat) {
                        eval_res->SetValue(val1->m_Double < val2->m_Double);
                    }
                    else if (promoted_type == QueryValueType::eString) {
                        int result = NStr::Compare(val1->m_String.c_str(), 
                            val2->m_String.c_str(), 
                            GetCaseSensitive());
                        eval_res->SetValue(result<0);
                    }
                    break;

                case CQueryParseNode::eLE:
                    if (promoted_type == QueryValueType::eBool) {
                        eval_res->SetValue(val1->m_Bool  <= val2->m_Bool );
                    }
                    else if (promoted_type == QueryValueType::eInt) {
                        eval_res->SetValue(val1->m_Int <= val2->m_Int);
                    }
                    else if (promoted_type == QueryValueType::eFloat) {
                        eval_res->SetValue(val1->m_Double <= val2->m_Double);
                    }
                    else if (promoted_type == QueryValueType::eString) {
                        int result = NStr::Compare(val1->m_String.c_str(), 
                            val2->m_String.c_str(), 
                            GetCaseSensitive());
                        eval_res->SetValue(result<=0);
                    }
                    break;

                default:
                    // We already check for this wih previous 'if' so should not happen 
                    NCBI_THROW(CQueryExecException, eExecParseError, 
                        "Error - Unhandled operator: " +
                        qnode.GetValue().GetNodeTypeAsString());                   
                    break;
            }
    }
    else {
        NCBI_THROW(CQueryExecException, eExecParseError, 
            "Error - Unhandled comparison operator: " +
            qnode.GetValue().GetNodeTypeAsString());  
    }
}   

/******************************************************************************
 *
 * CQueryFuncPromoteBetween Implementation
 *
 *****************************************************************************/ 
CQueryFuncPromoteBetween::CQueryFuncPromoteBetween(NStr::ECase c)
: CQueryFuncGtLtCompares(CQueryParseNode::eBetween, c)
{ 
    InitTypePromotionRules(CQueryParseNode::eBetween); 
}

void CQueryFuncPromoteBetween::Evaluate(CQueryParseTree::TNode& qnode)
{
    CQueryFunctionBase::TArgVector args;
    this->MakeArgVector(qnode, args);   

    // get operation type (should be between)
    CQueryParseNode::EType node_type = qnode.GetValue().GetType();

    if (node_type == CQueryParseNode::eBetween) {
        TEvalResult* eval_res = GetQueryNodeValue(qnode);

        if (args.size() != 3) {
            NCBI_THROW(CQueryExecException, eWrongArgumentCount, 
                "Error: Ternary operator " +
                qnode.GetValue().GetNodeTypeAsString() +
                " has " + NStr::SizetToString(args.size()) + " operands.");
        }


        // Get underlying data and data type.  This is a ternary operator
        // and is equivalent to (a>b && a<c) || a==b || a==c.  So get a 
        // separate promotion type for  (a,b) and (a,c) and do the comparisons 
        // separately.
        QueryValueType::EBaseType promoted_type1 = 
            ResolveAndPromote(0, qnode, args[0], args[1]);

        if (promoted_type1 == QueryValueType::eUndefined) {
            eval_res->SetValue(false);
            return;
        }

        TEvalResult* val1 = GetQueryNodeValue(*args[0]);
        TEvalResult* val2 = GetQueryNodeValue(*args[1]);
        TEvalResult* val3 = GetQueryNodeValue(*args[2]);


        // Set is_gt to true if val1 > val2, and set
        // is_eq to true if val1 == val2.
        bool is_gt = false;
        bool is_eq = false;
        if (promoted_type1 == QueryValueType::eInt) {
            is_gt = val1->m_Int > val2->m_Int;
            is_eq = val1->m_Int == val2->m_Int;
        }
        else if (promoted_type1 == QueryValueType::eFloat) {
            is_gt = val1->m_Double > val2->m_Double;
            is_eq = val1->m_Double == val2->m_Double;
        }
        else if (promoted_type1 == QueryValueType::eString) {
            int result = NStr::Compare(val1->m_String.c_str(), 
                                       val2->m_String.c_str(), 
                                       GetCaseSensitive());
            is_gt = (result > 0);
            is_eq = (result == 0);
        }

        // If val1==val2 (is_eq), we are done, otherwise 
        // now set is_lt to true if val1 < val3.
        bool is_lt = false;
        if (!is_eq) {            
            QueryValueType::EBaseType promoted_type2 = 
                ResolveAndPromote(1, qnode, args[0], args[2]);

            if (promoted_type2 == QueryValueType::eUndefined) {
                eval_res->SetValue(false);
                return;
            }

            if (promoted_type2 == QueryValueType::eInt) {
                is_lt = val1->m_Int < val3->m_Int;
                is_eq = val1->m_Int == val3->m_Int;
            }
            else if (promoted_type2 == QueryValueType::eFloat) {
                is_lt = val1->m_Double < val3->m_Double;
                is_eq = val1->m_Double == val3->m_Double;
            }
            else if (promoted_type2 == QueryValueType::eString) {
                int result = NStr::Compare(val1->m_String.c_str(), 
                    val3->m_String.c_str(), 
                    GetCaseSensitive());
                is_lt = (result < 0);
                is_eq = (result == 0);
            }
        }

        // 'a' between 1 and 5 should give the same result as
        // 'a' between 5 and 1, so we check if the value 'a' is
        // greater than one value and less than the other, e.g.
        // check if is_gt == is_lt.  The number is also between if
        // it is equal to either of the two bounding values.
        // a >= 1 and a <= 5;
        // a >= 5 and a <= 1;
        bool is_between = (is_gt == is_lt) || is_eq;
        if (qnode->IsNot())            
            eval_res->SetValue(!is_between); 
        else 
            eval_res->SetValue(is_between);
    }
    else {
        NCBI_THROW(CQueryExecException, eExecParseError, 
            "Error - Unhandled comparison operator: " +
            qnode.GetValue().GetNodeTypeAsString());  
    }
}   


END_NCBI_SCOPE
