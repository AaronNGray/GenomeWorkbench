#ifndef GUI_OBJUTILS___QUERY_FUNC_PROMOTE_HPP
#define GUI_OBJUTILS___QUERY_FUNC_PROMOTE_HPP
/*  $Id: query_func_promote.hpp 39115 2017-08-01 18:58:09Z asztalos $
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
 * Authors: Bob Falk
 *
 * File Description: 
 * Header file for classes needed to implement query execution using a
 * type-promotion approach for comparisons
 *
 */
#include <corelib/ncbistl.hpp>

#include <gui/gui_export.h>

#include <objmgr/scope.hpp>

#include <util/qparse/query_exec.hpp>
#include <gui/objutils/macro_query_exec.hpp>
#include <gui/objutils/string_matching.hpp>
#include <gui/objutils/query_node_value.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncPromoteBase
///
/// Base class for query execution functions.  The base class holds
/// the type-promotion rules needed to convert types appropriately to allow
/// for different kinds of comparisons.
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncPromoteBase : public CQueryFunctionBase
{
public:
    typedef CQueryNodeValue TEvalResult;

    CQueryFuncPromoteBase() 
        : m_CaseSensitive(NStr::eCase)
        , m_StringMatchAlgo(CStringMatching::ePlainSearch) {}

    /// Get/set case sensitivity for all string compares for this function
    void SetCaseSensitive(NStr::ECase e) {m_CaseSensitive = e;}
    NStr::ECase GetCaseSensitive() const { return m_CaseSensitive; }

    /// Get/set string matching algorithm
    void SetStringMatching(CStringMatching::EStringMatching m) {m_StringMatchAlgo = m;}
    CStringMatching::EStringMatching GetStringMatching() const { return m_StringMatchAlgo; }

    /// Get the user object for node 'qnode' or NULL if not created
    TEvalResult* GetQueryNodeValue(CQueryParseTree::TNode& qnode);
    
    /// Get user object for 'qnode' or create a new object if it doesn't exist
    TEvalResult* MakeQueryNodeValue(CQueryParseTree::TNode& qnode);

    /// Resolve, if possible, the data type for the query node tr and,
    /// for operator nodes (e.g. ==, <), determine if possible types
    /// to be used for comparison (not always possible for untyped data
    /// from data source)
    void PreProcess(CQueryParseTree::TNode& tr, objects::CScope* scope);

    /// For the operator in 'qnode', retrive from data source (if ncessary)
    /// the data values arg1 and arg2 and promote the values, if needed, to 
    /// the required type needed for comparison.
    QueryValueType::EBaseType
        ResolveAndPromote(size_t comparison_idx,
                          CQueryParseTree::TNode& qnode,
                          CQueryParseTree::TNode* arg1,
                          CQueryParseTree::TNode* arg2);                       

    /// Given data from two nodes and a comparison operator (e.g. <, ==, >..)
    /// determine the best type for comparing them (e.g. if one is a string
    /// and one is an integer should they be compared as strings, integers, or 
    /// not at all).
    QueryValueType::EBaseType  
        GetPromotedType(const CPromoteRule& pr);

    /// Get the value of a field from the data source and store the
    /// result in the CQueryNodeValue object.  Returns true on success.
    bool ResolveFieldValue(TEvalResult& tree_val);

    /// Given data for a node, determine its underlying data type (e.g. if 
    /// its a string, can it also be converted to a bool? int? float?).
    /// Returns false when field not present in data source.
    bool SetCompareType(TEvalResult& tree_value);

    // functions which do not take 2 arguments (e.g. 'In', 'Not'), should
    // override these functions.
    virtual size_t GetArgCountMin(CQueryParseNode::EType /* t */) { return 2; }
    virtual size_t GetArgCountMax(CQueryParseNode::EType /* t */) { return 2; }

protected:

    /// Add the promtion rules for both 'type1 op type2' and 'type2 op type1'
    void AddPromoteTypes(CQueryParseNode::EType op,
                         QueryValueType::EBaseType type1,
                         QueryValueType::EBaseType type2,
                         QueryValueType::EBaseType ptype);

    /// Vector of sorted promotion rules
    std::vector<CPromoteRule> m_PromoteRules;
    
    /// If true, all string comparisons by subclasses will use case-sensitive
    /// comparisons, otherwise not.
    NStr::ECase m_CaseSensitive;

    /// Determines how string equality comaparisons will be handled (wildcards, regex, exact..)
    CStringMatching::EStringMatching m_StringMatchAlgo;
};

///////////////////////////////////////////////////////////////////////////////
///  Query execution function for run-time variables.
///  It represents variable which can be met in left and right side of expression.
///
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncRTVar : public CQueryFuncPromoteBase
{
public:
    /// Function is a placeholder for run-time variable processing
    virtual void Evaluate(CQueryParseTree::TNode& qnode);
};

///////////////////////////////////////////////////////////////////////////////
///  Query execution function for assignment operator.
///
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncAssignment : public CQueryFuncPromoteBase
{
public:
    /// Function implements the assignment operator
    virtual void Evaluate(CQueryParseTree::TNode& node);
};

///////////////////////////////////////////////////////////////////////////////
///  class CQueryFuncFunction
///
///  Class implements functions calls in the do section of macro
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncFunction : public CQueryFuncPromoteBase
{
public:
    /// Function implements functions calls in the do section of macro
    virtual void Evaluate(CQueryParseTree::TNode& qnode);
};


////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncPromoteValue
///
/// Query execution function for simple atomic values. These are 
/// 'leaf' nodes of the query tree.
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncPromoteValue : public CQueryFuncPromoteBase
{
public:
    CQueryFuncPromoteValue() {}
  
    ///
    /// Create the user object for the node if it does not yet exist
    /// and reset the boolean 'result' value for the node evaluation
    virtual void Evaluate(CQueryParseTree::TNode& qnode);

    virtual size_t GetArgCountMin(CQueryParseNode::EType /* t */) { return 0; }
    virtual size_t GetArgCountMax(CQueryParseNode::EType /* t */) { return 0; }
};

////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncPromoteIdentifier
///
/// Query execution function for variables
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncPromoteIdentifier : public CQueryFuncPromoteBase
{
public:
    CQueryFuncPromoteIdentifier() {}

    ///
    /// Create the user object for the node if it does not yet exist
    /// and reset the boolean 'result' value for the node evaluation
    virtual void Evaluate(CQueryParseTree::TNode& qnode);

    virtual size_t GetArgCountMin(CQueryParseNode::EType /* t */) { return 0; }
    virtual size_t GetArgCountMax(CQueryParseNode::EType /* t */) { return 0; }
};



////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncPromoteAndOr
///
/// Query execution function for logical operators like AND, OR, etc.
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncPromoteAndOr : public CQueryFuncPromoteBase
{
public:
    CQueryFuncPromoteAndOr() : CQueryFuncPromoteBase() {}

    virtual bool EvaluateChildrenFirst() const { return false; }
    
    /// Try to  convert the arguments into boolean values and, if successful,
    /// apply the specified boolean operator to those values.
    virtual void Evaluate(CQueryParseTree::TNode& qnode);

    virtual size_t GetArgCountMin(CQueryParseNode::EType t) { return 2; }
        
    /// Can string togeather multiple ands/ors into one combined and/or, e.g.
    /// a or b or c or d==or:a,b,c,d. So we allow essentially unlimited operands
    virtual size_t GetArgCountMax(CQueryParseNode::EType t) { return 65536; }
};

////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncPromoteLogic
///
/// Query execution function for logical operators like AND, OR, etc.
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncPromoteLogic : public CQueryFuncPromoteBase
{
public:
    CQueryFuncPromoteLogic() : CQueryFuncPromoteBase() {}

    /// Try to  convert the arguments into boolean values and, if successful,
    /// apply the specified boolean operator to those values.
    virtual void Evaluate(CQueryParseTree::TNode& qnode);

    virtual size_t GetArgCountMin(CQueryParseNode::EType t)
    {
        return (t == CQueryParseNode::eNot ? 1 : 2);
    }
    /// xor, sub take 2 args, not takes 1
    virtual size_t GetArgCountMax(CQueryParseNode::EType t)
    {
        return (t == CQueryParseNode::eNot ? 1 : 2);
    }
};


////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncPromoteCompare
///
/// Query execution function base class for all comparison classes including:
/// >, <, >=, <=, ==, 'In', 'Between', and Like.
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncPromoteCompare : public CQueryFuncPromoteBase
{  
public:
    /// Ctors for compare must include op_type for loading promotion rules
    CQueryFuncPromoteCompare(CQueryParseNode::EType op_type,                               
                             NStr::ECase c = NStr::eCase,
                             CStringMatching::EStringMatching matching=(CStringMatching::ePlainSearch));

    /// Subclass to create vector of promotion rules in m_PromoteRules
    virtual void InitTypePromotionRules(CQueryParseNode::EType /*op_type*/) {}
};


////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncEqualityCompares
///
/// Base class for query execution functions that use comparison operators 
/// (for now that is == and In).  These functions both use the same type
/// promotion rules
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncEqualityCompares : public CQueryFuncPromoteCompare
{    
public:
    /// Ctor
    CQueryFuncEqualityCompares(CQueryParseNode::EType op_type,                               
                               NStr::ECase c = NStr::eCase, 
                               CStringMatching::EStringMatching matching=CStringMatching::ePlainSearch)
    : CQueryFuncPromoteCompare(op_type, c, matching) {}

    /// Initialize promotion table for eqaulity comparisons (e.g. == or In)
    virtual void InitTypePromotionRules(CQueryParseNode::EType op_type);
};


////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncPromoteEq
///
/// Query execution function for equality comparison: ==
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncPromoteEq : public CQueryFuncEqualityCompares
{    
public:
    /// Ctor
    CQueryFuncPromoteEq(NStr::ECase c = NStr::eCase, 
                        CStringMatching::EStringMatching matching=CStringMatching::ePlainSearch);
   
    /// Evaluate the node to see if '==' returns true
    virtual void Evaluate(CQueryParseTree::TNode& qnode);
};

////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncPromoteIn
///
/// Query execution function for the 'In' comparison
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncPromoteIn : public CQueryFuncEqualityCompares
{    
public:
    /// ctor
    CQueryFuncPromoteIn(NStr::ECase c = NStr::eCase, 
                        CStringMatching::EStringMatching matching=CStringMatching::ePlainSearch);
   
    /// Evaluate the node to see if 'In' returns true
    virtual void Evaluate(CQueryParseTree::TNode& qnode);
    
    /// In can have just one argument (a in ()) or a very large number
    /// a in (dog, cat, bird mouse......)
    virtual size_t GetArgCountMin(CQueryParseNode::EType /* t */) { return 1; }
    virtual size_t GetArgCountMax(CQueryParseNode::EType /* t */) { return 4096; }
};

////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncLike
///
/// Query execution function for 'Like' comparison.
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncLike : public CQueryFuncPromoteCompare
{    
public:
    /// Ctor
    CQueryFuncLike(NStr::ECase c = NStr::eCase);
    
    /// Initialize promotion rules for 'Like' comparison
    virtual void InitTypePromotionRules(CQueryParseNode::EType op_type);
   
    /// Evaluate the node to see if 'Like' returns true
    virtual void Evaluate(CQueryParseTree::TNode& qnode);
};

////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncGtLtCompares
///
/// Base class for query execution functions that use greater than/ less 
/// than comparisons (for now that is >,<,>=,<= and Between).  These functions
/// all use the same promotion rules.
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncGtLtCompares : public CQueryFuncPromoteCompare
{    
public:
    /// Ctor
    CQueryFuncGtLtCompares(CQueryParseNode::EType op_type,                               
                           NStr::ECase c = NStr::eCase)
    : CQueryFuncPromoteCompare(op_type, c) {}

    /// Initialize promotion table for the comparisons.
    virtual void InitTypePromotionRules(CQueryParseNode::EType op_type);
};

////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncPromoteGtLt
///
/// Query execution function for the >,<,>= and <= operators
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncPromoteGtLt : public CQueryFuncGtLtCompares
{    
public:
    /// Ctor includes op_type since we will create a separate
    /// instance for each of >,<, >= and <=.
    CQueryFuncPromoteGtLt(CQueryParseNode::EType op_type,                               
                          NStr::ECase c = NStr::eCase);
   
    /// Evaluate the node to see if 'greater than/less than' returns true
    virtual void Evaluate(CQueryParseTree::TNode& qnode);
};

////////////////////////////////////////////////////////////////////////////////
/// class CQueryFuncPromoteBetween
///
/// Query execution function for the 'Between' operator
class NCBI_GUIOBJUTILS_EXPORT CQueryFuncPromoteBetween : public CQueryFuncGtLtCompares
{    
public:
    /// Ctor
    CQueryFuncPromoteBetween(NStr::ECase c = NStr::eCase);

    /// Evaluate the node to see if 'Between' returns true
    virtual void Evaluate(CQueryParseTree::TNode& qnode);

    /// 27 between 10 and 30 => 3 arguments for between operator
    virtual size_t GetArgCountMin(CQueryParseNode::EType /* t */) { return 3; }
    virtual size_t GetArgCountMax(CQueryParseNode::EType /* t */) { return 3; }
};

////////////////////////////////////////////////////////////////////////////////
/// class CQueryExecPreProcessFunc
///
/// This class applies a function to the query to do any needed optiminzation
/// or preprocessing of the query prior to execution.
class CQueryExecPreProcessFunc
{
public: 
    CQueryExecPreProcessFunc(objects::CScope* scope,
                             CQueryExec& exec)
    : m_Exec(exec)
    , m_Scope(scope)
    {}

    ETreeTraverseCode 
    operator()(CTreeNode<CQueryParseNode>& tr, int delta) 
    {
        CQueryParseNode& qnode = tr.GetValue();
        if (delta == 0 || delta == 1) {
            // If node has children, we skip it and process on the way back
            if (!tr.IsLeaf()) {
                return eTreeTraverse;
            }
        }
        CQueryParseNode::EType func_type = qnode.GetType();
        CQueryFuncPromoteBase* func = 0;
        
        func = dynamic_cast<CQueryFuncPromoteBase*>( m_Exec.GetFunc(func_type) ); 
        
        if (func == 0) { // function not registered
            NCBI_THROW(CQueryParseException, eUnknownFunction, 
               "Query pre-processing faild. Unknown function:" + qnode.GetOrig());
        }        
       
        func->PreProcess(tr, m_Scope);       
        
        return eTreeTraverse;        
    }
private:
    
    CQueryExec& m_Exec;
    objects::CScope* m_Scope;
};



END_NCBI_SCOPE


#endif  // GUI_OBJUTILS___QUERY_FUNC_PROMOTE_HPP

