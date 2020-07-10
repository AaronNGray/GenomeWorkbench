#ifndef GUI_OBJUTILS___QUERY_NODE_VALUE_HPP
#define GUI_OBJUTILS___QUERY_NODE_VALUE_HPP
/*  $Id: query_node_value.hpp 43436 2019-06-28 16:26:08Z katargir $
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
#include <gui/objutils/query_macro_user_object.hpp>

BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// class CQueryExecException
///
/// The different types of errors that can occur after the query is parsed while
/// it is being executed.
class NCBI_GUIOBJUTILS_EXPORT CQueryExecException : public CException
{
public:
    enum EErrCode {
        eNotPromotable,         // Used if a type can't be promoted to another
                                //   type, e.g. 'dog' can't be prmoted to float.
        eIncompatibleType,      // types which can't be compared, e.g. true AND "cat"
        eWrongArgumentCount,    // Number of arguments for an operand are incorrect,
                                //   e.g. '==' has 3 operands.  The initial parser
                                //   should catch all of these.
        eExecParseError,        // If there are strings with multiple tokens during
                                //   execution, those may be parsed and evaluted, e.g.
                                //   in "dist>(2.0 * OtherDist)", (2 * OtherDist)
                                //   may be further parsed and found to be valid,
                                //   or it may throw this error if not.
        eObjManagerError,       // Error when looking up/comparing seq-ids
                                //   during query execution
        eUnableToResolveData,   // Unable to retrieve a field value from the data
                                //    source for a particular entry
        eFunctionExedError      // Called a function but function execution failed
    };

    virtual const char* GetErrCodeString(void) const override
    {
        switch (GetErrCode())
        {
        case eNotPromotable:            return "eNotPromotable";
        case eIncompatibleType:         return "eIncompatibleType";
        case eWrongArgumentCount:       return "eWrongArgumentCount";
        case eExecParseError:           return "eExecParseError";
        case eObjManagerError:          return "eObjManagerError";
        case eUnableToResolveData:      return "eUnableToResolveData";
        case eFunctionExedError:        return "eFunctionExedError";
            
        default: return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT(CQueryExecException, CException);
};

namespace QueryValueType {

    /// Set of all possible types for nodes.  Basic types are repeated based
    /// on their source to allow us to include the source of the data when
    /// deciding on how to promote something (e.g. did an int get parsed from
    /// a string in the query (e.g. the number "27" in (a=="27")), or was the
    /// int retrieved from a data field (e.g. in dist>0.4, the value
    /// for 'dist' comes from the current record/node being evaluated).
    enum EBaseType {
        eBoolResult,  // result value from computing results in query sub-tree
        eBool,        // boolean const
        eInt,         // int const
        eFloat,       // float const
        eString,      // string which cannot be converted to another type
        eSeqID,       // possible seq-id identifier
        eStringBool,  // bool converted from a string in the query
        eStringInt,   // integer converted form a string in the query
        eStringFloat, // float converted from a string in the query
        eFieldSeqID,  // possible seq-id field value
        eFieldString, // string field value
        eFieldBool,   // boolean field value
        eFieldFloat,  // float field value
        eFieldInt,    // integer field value
        eRef,         // reference type to a similar object
        eUndefined    // incompatible types or other error
    };

    NCBI_GUIOBJUTILS_EXPORT string GetTypeAsString(EBaseType et);
};


////////////////////////////////////////////////////////////////////////////////
/// class CPromoteRule
///
/// This is a simple class meant to be an entry in a table representing
/// promotion rules. Given the table, you can determine for any two types
/// (and type information incldudes the source of the data such as whether it
/// was a constant in the query or comes from the data source) and their
/// operator which type should be used as the basis for comparison, or if the
/// comparison does not make any sense. (An example of a non-promotable pair 
/// would be a boolean and a string where that string is not converible to a 
/// boolean or numeric value).
class NCBI_GUIOBJUTILS_EXPORT CPromoteRule {
public:
    /// Default ctor initializes to undefined values
    CPromoteRule() 
        : m_CompareOperator(CQueryParseNode::eNotSet)
        , m_Type1(QueryValueType::eUndefined)
        , m_Type2(QueryValueType::eUndefined)
        , m_PromotedType(QueryValueType::eUndefined) {}

    /// Set the values for the type and their comparison operator
    CPromoteRule(CQueryParseNode::EType  op,
                 QueryValueType::EBaseType type1,
                 QueryValueType::EBaseType type2,
                 QueryValueType::EBaseType ptype)
        : m_CompareOperator(op)
        , m_Type1(type1)
        , m_Type2(type2)
        , m_PromotedType(ptype) {}

    
    /// Allow table to be sorted for faster lookup
    bool operator<(const CPromoteRule& rhs) const
    { 
        if (m_Type1 < rhs.m_Type1)
            return true;
        else if (m_Type1 == rhs.m_Type1 &&
                 m_Type2 < rhs.m_Type2)
            return true;
        else if (m_Type1 == rhs.m_Type1 &&
                 m_Type2 == rhs.m_Type2 &&
                 m_CompareOperator < rhs.m_CompareOperator)
            return true;

        return false;
    }

    /// Allows us to check to see if we found requested table entry
    bool operator==(const CPromoteRule& rhs) const
    { 
        return ((m_Type1 == rhs.m_Type1) &&
                (m_Type2 == rhs.m_Type2) &&
                (m_CompareOperator == rhs.m_CompareOperator));         
    }

    /// The comparison operator applied to the two values, e.g. ==, <, ...
    CQueryParseNode::EType  m_CompareOperator;
    /// The extended type for the first element to compare
    QueryValueType::EBaseType m_Type1;
    /// The extended type for the second element to compare
    QueryValueType::EBaseType m_Type2;

    /// The type to be used for the comparison.  This is the type from the
    /// CQueryParseNode class since it does not care where the data came from 
    QueryValueType::EBaseType  m_PromotedType;
};



////////////////////////////////////////////////////////////////////////////////
/// class CQueryNodeValue
///
/// Subclass of the IQueryParseUserObject which is held as the user-defined object
///  in each CQueryParseNode.  Holds a boolean value that tells us if the
///  subexpression evaluted to true or not, so that value can be passed up the 
///  tree.  Also keeps track of whether that value has been set.
///
/// This class also holds data similar to CQueryParseNode, but with more information
/// about the type (includes both the underlying type and source of the data). 
/// More than one data element may be valid, e.g. if the data is an integer
/// parsed from a string (eStringInt), both the string and integer fields will
/// be set.
class NCBI_GUIOBJUTILS_EXPORT CQueryNodeValue : public IQueryMacroUserObject
{
public:
    typedef CQueryExec::TFieldID  TFieldIDType;

public:
    CQueryNodeValue() 
        : m_Node(NULL)
        , m_DataType(QueryValueType::eUndefined)
        , m_IsField(false)
        , m_FieldID(TFieldIDType(-1))
        , m_Scope(NULL)
        , m_Value(false) 
        , m_NodeBranchDepth(0)
        , m_NodeMaxChildBranchDepth(-1)
        {}
    CQueryNodeValue(CQueryParseTree::TNode* n) 
        : m_Node(n)
        , m_DataType(QueryValueType::eUndefined)
        , m_IsField(false)
        , m_FieldID(TFieldIDType(-1))
        , m_Scope(NULL)
        , m_Value(false)
        , m_NodeBranchDepth(0)
        , m_NodeMaxChildBranchDepth(-1)
    {}

    // specifically does not reset m_IsField - that sticks between evaluations
    virtual void Reset() { m_Value = false; }
    virtual string GetVisibleValue() const;
    
    /// Set boolean result value (result of (sub)expression).
    bool GetValue() const { return  m_Value; }
    void SetValue(int v) { m_Value = v; }

    /// Set/Get number of branches between node and root
    void SetBranchDepth(int bd) { m_NodeBranchDepth = bd; }
    int GetBranchDepth() const { return m_NodeBranchDepth; }
    
    /// Set/Get number of branches between node and most distant child
    void SetMaxChildBranchDepth(int cbd) { m_NodeMaxChildBranchDepth = cbd; }
    int GetMaxChildBranchDepth() const { return m_NodeMaxChildBranchDepth; }

    /// Convert current value to the type 'pt'. Does not update m_DataType
    void PromoteTo(QueryValueType::EBaseType pt);

    /// Get corresponding query node
    CQueryParseTree::TNode* GetQueryNode() { return m_Node; }

    /// Set/get underlying data type
    void SetDataType(QueryValueType::EBaseType dt) {m_DataType = dt; }
    QueryValueType::EBaseType GetDataType() const { return m_DataType; }

    /// Set/Get to indicate if this is a field from the data source or simple 
    /// string. The type of field may not yet be avialable (if it has to be
    /// determined for each separate data element)
    void SetIsDataField(bool b) { m_IsField = b; }
    bool IsDataField() const { return m_IsField; }

    // Set/Get biotree feature ID if m_IsField is true (makes looking up
    // feature values within nodes faster).
    void SetFieldID(TFieldIDType fid) { m_FieldID = fid; }
    TFieldIDType GetFieldID() const { return m_FieldID; }

    /// Set/Get CScope used for comparing seq-ids
    void SetScope(objects::CScope* s) {m_Scope = s;}
    objects::CScope* GetScope() { return m_Scope; }

    /// Return promotion rule(s) defined for this operator.
    vector<CPromoteRule>& GetPromoteRules() { return m_PromoteRules; }
    /// Get the promotion type for a specific argument pair, or eUndefined
    /// if no rule is available.     
    QueryValueType::EBaseType GetPromoteType(size_t arg_idx);
    /// Return true if there is a promote entry defined for the specified
    /// argument pair at 'idx' only if the types for the rule match t1 and t2.
    bool HasPromoteType(size_t arg_idx, 
                        QueryValueType::EBaseType t1, 
                        QueryValueType::EBaseType t2);
    /// Append a new promote rule
    void AddPromotedType(const CPromoteRule& pr) { m_PromoteRules.push_back(pr); }

    void Dereference();
    
    /// String data, if data came from a string or data field in the tree
    string m_String;

    /// Bool data, if data base a constant boolean or converted into one
    bool m_Bool;

    /// Int data, if data was an integer or converted into one
    Int8 m_Int;

    /// Floating point data, if data was a double or converted into one
    double m_Double;

    /// Reference to similar object
    CRef<CQueryNodeValue> m_Ref;

    /// Set/get underlying data type
    virtual void SetString(const string& data);
    virtual void SetBool(bool data);
    virtual void SetDouble(double data);
    virtual void SetInt(Int8 data);
    void SetRef(CRef<CQueryNodeValue> node);
    bool AssignToRef(const CQueryNodeValue& source);

    virtual const string&  GetString()  const { return m_String; }
    virtual bool           GetBool()    const { return m_Bool; }
    virtual double         GetDouble()  const { return m_Double; }
    virtual Int8           GetInt()     const { return m_Int; }
    
protected:
    /// Node from parsed query tree
    CQueryParseTree::TNode* m_Node;

    /// Data type, including source of the data (const, string field, or tree)
    QueryValueType::EBaseType m_DataType;

    /// True if the data comes from field in the tree
    bool m_IsField;

    /// If it is a field, this is the ID to look it up (efficiently)
    TFieldIDType m_FieldID;

    /// Used for comparing seq-ids
    objects::CScope* m_Scope;

    /// The promote rules defined for the current operator.  This will be empty
    /// for atomic types.  For operator type query nodes (e.g. ==, <, etc) this
    /// will have the comparison (promotion) type for each pair of operands. If
    /// one or more types will be defined at runtime, e.g. an untyped field 
    /// from the data source, this will be empty    
    vector<CPromoteRule> m_PromoteRules;
        
    /// Boolean result of the (sub)expression attached to the corresponding
    /// query node.
    bool m_Value;

    /// Mechanism to pass current position of node within the tree in
    /// terms of the number of branches between the node and the parent
    /// and between the node and it's most distant (by branch count)
    /// child
    int m_NodeBranchDepth;
    int m_NodeMaxChildBranchDepth;
};

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___QUERY_NODE_VALUE_HPP

