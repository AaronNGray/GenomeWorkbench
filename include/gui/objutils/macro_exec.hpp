#ifndef GUI_OBJUTILS___MACRO_EXEC__HPP
#define GUI_OBJUTILS___MACRO_EXEC__HPP

/*  $Id: macro_exec.hpp 44959 2020-04-28 18:49:47Z asztalos $
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

#include <gui/gui_export.h>
#include <util/qparse/query_exec.hpp>
#include <serial/objectiter.hpp>
#include <gui/objutils/query_macro_user_object.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

// forward declarations
class CMacroExec;

///////////////////////////////////////////////////////////////////////////////
///  This class applies an operation to each node of the query tree to do 
///  necessary initialization (construction of user objects for tree nodes).
///
class CMQueryNodePreprocessor {
public:
    /// Operator builds node values for each CQueryParseNode
    ETreeTraverseCode operator()(CTreeNode<CQueryParseNode>& tr, int delta);
};

///////////////////////////////////////////////////////////////////////////////
/// Subclass of the IQueryParseUserObject which is held as the user-defined 
/// object in each CQueryParseNode. It defines the tree node user object for 
/// storing results of the underlying tree evaluation.
/// It can define:
////  1)  Simple data type (EType::eInt, eBool, eString, eFloat);
///   2)  Objects (list of references to bioobjects);
///   3)  References to run-time variables stored in CMacroEngine class.

class CMQueryNodeValue : public IQueryMacroUserObject
{
public:
    enum EType {
        eNotSet,
        eInt, 
        eFloat,
        eBool,
        eString,
        eStrings,
        eObjects,
        eRef
    };

    struct SResolvedField
    {
        CObjectInfo parent;
        CObjectInfo field;
        SResolvedField(CObjectInfo parent, CObjectInfo field)
            : parent(parent), field(field) {}
    };

    typedef list<SResolvedField> TObs;

    CMQueryNodeValue() 
        : m_Node(nullptr)
        , m_DataType(eNotSet)
        , m_Ref(0)
    {}

    CMQueryNodeValue(CQueryParseTree::TNode* n)
        : m_Node(n)
        , m_DataType(eNotSet)
        , m_Ref(0)
    {}

    virtual void Reset()
    {
        m_DataType = eNotSet;
        m_Obs.clear();
        m_Ref = 0;
    }

    bool IsSimpleType() const { return m_DataType & (eInt | eBool | eString | eFloat); }

    /// Assigns data from objinfo. It is used when storing evaluation result
    void AssignFromObjectInfo(const CObjectInfo& objinfo);

    /// If it is a reference it is resolved to the first non reference type in the hierarchy.
    void Dereference();

    void SetDataType(EType dt) { Reset(); m_DataType = dt; }
    EType GetDataType() const { return m_DataType; }

    // Set/get underlying data and data type
    void SetString(const string& data)              { Reset(); m_String = data; m_DataType = eString;   }
    void SetStrings(const vector<string>& data)     { Reset(); m_Strings = data; m_DataType = eStrings; }
    void SetBool(bool data)                         { Reset(); m_Bool   = data; m_DataType = eBool;     }
    void SetInt(Int8 data)                          { Reset(); m_Int    = data; m_DataType = eInt;      }
    void SetDouble(double data)                     { Reset(); m_Double = data; m_DataType = eFloat;    }
    void SetRef(CRef<CMQueryNodeValue> node)        { Reset(); m_Ref    = node; m_DataType = eRef;      }
    void SetObjects(const TObs& obs)                { Reset(); m_Obs    = obs;  m_DataType = eObjects;  }

    /// @note It assumes that the underlying datatype is eObjects. It will not reset it.
    TObs& SetObjects(void)                          { _ASSERT(m_DataType == eObjects); return m_Obs; }
    void SetNotSet()                                { Reset();                  m_DataType  = eNotSet;  }

    bool AssignToRef(const CMQueryNodeValue& source);

    const string&              GetString()  const { return m_String;  }
    const vector<string>&      GetStrings() const { return m_Strings; }
    bool                       GetBool()    const { return m_Bool;    }
    Int8                       GetInt()     const { return m_Int;     }
    double                     GetDouble()  const { return m_Double;  }
    const TObs&                GetObjects() const { return m_Obs;     }

    bool IsString() const { return m_DataType == eString; }
    bool AreStrings() const { return m_DataType == eStrings; }
    bool IsBool() const { return m_DataType == eBool; }
    bool IsInt() const { return m_DataType == eInt; }
    bool IsDouble() const { return m_DataType == eFloat; }
    bool IsRef() const { return m_DataType == eRef; }
    bool AreObjects() const { return m_DataType == eObjects; }
    bool IsNotSet() const { return m_DataType == eNotSet; }

    CQueryParseTree::TNode* GetNode() const { return m_Node; }

private:
    /// Node from parsed query tree
    CQueryParseTree::TNode* m_Node;

    /// Node value data type.
    /// The node value data type used for evaluation or comparison. It need to
    /// be one of the following: eIntConst, eFloatConst, eBoolConst, and eString
    EType m_DataType;

    /// Data
    /// String data, if data came from a string or data field in the tree
    string m_String;

    /// Vector of strings
    vector<string> m_Strings;

    /// For data that is a boolean or converted into one
    bool m_Bool;

    /// For data that is an integer or converted into one
    Int8  m_Int;

    /// For data that is a double or converted into one
    double m_Double;

    /// Reference to similar object
    CRef<CMQueryNodeValue> m_Ref;

    /// List of objects
    TObs m_Obs;
};

///////////////////////////////////////////////////////////////////////////////
///  Query execution proxy class to add common operations and 
///  avoid modification of CQueryFunctionBase.
///
class CMQueryFunctionBase : public CQueryFunctionBase
{
public:
    CMacroExec* GetContext();
};

///////////////////////////////////////////////////////////////////////////////
///  Query execution function for simple atomic values.
///  It represents leaf constants and resolved identifier values.
///
class CMQueryFunctionValue : public CMQueryFunctionBase
{
public:

    CMQueryFunctionValue() {}
    /// Calculates node values for constants and resolved identifiers
    virtual void Evaluate(CQueryParseTree::TNode& node);
};

///////////////////////////////////////////////////////////////////////////////
///  Query execution function for run-time variables.
///  It represents variable which can be met in left and right side of expression.
///
class CMQueryFunctionRTVar : public CMQueryFunctionBase
{
public:
    /// Function is a placeholder for run-time variable processing
    virtual void Evaluate(CQueryParseTree::TNode& node);
};

class CMQueryFunctionOps : public CMQueryFunctionBase
{
public:
    typedef vector<CRef<CMQueryNodeValue>> TUserArgs;

    virtual ~CMQueryFunctionOps() {}
    /// Abstract node evaluation function
    virtual void Evaluate(CQueryParseTree::TNode& node) = 0;

protected:
    /// Ininializes protected members to be used in derived classes
    void x_InitReferences(CQueryParseTree::TNode& node);
    /// Checks the number and type of arguments
    bool x_ValidateAll(Uint4 number, CMQueryNodeValue::EType type);
    /// Checks the number of arguments, also their type should be one of the given types
    bool x_Validate(Uint4 number, CMQueryNodeValue::EType type1, CMQueryNodeValue::EType type2);
    /// Find not set value
    bool x_FindNotSet();
    /// Gets case sensitiveness flag from environment
    bool x_IsCaseSensitive();
    /// Throws exception for invalid number of types of arguments
    void x_ProcessInvalidParams(CQueryParseTree::TNode& node);

    TUserArgs m_Arguments;            ///< Vector of argument node values
    CRef<CMQueryNodeValue> m_Result;  ///< Node for result storage

private:
    /// Gets user node value from CQueryParseTree node
    CRef<CMQueryNodeValue> x_GetUserObject (CQueryParseTree::TNode& node)
    {
        return Ref(dynamic_cast<CMQueryNodeValue*>(node->GetUserObject()));
    }
};

///////////////////////////////////////////////////////////////////////////////
///  Query execution function for assignment operator.
///
class CMQueryFunctionAssignment : public CMQueryFunctionOps
{
public:
    /// Function implements the assignment operator
    virtual void Evaluate(CQueryParseTree::TNode& node);
};

///////////////////////////////////////////////////////////////////////////////
///  Class implements functions calls in the do section of macro
///
class CMQueryFunctionFunction : public CMQueryFunctionOps
{
public:
    /// Function implements functions calls in the do section of macro
    virtual void Evaluate(CQueryParseTree::TNode& node);
};

///////////////////////////////////////////////////////////////////////////////
///  Default node processor. It is set if not redefined by something else.
///
class CMQueryFunctionDummy : public CMQueryFunctionBase
{
public:
    /// Function throws an exception about not implmented operation for the node
    virtual void Evaluate(CQueryParseTree::TNode& node);
};

//////////////////////////////////////////////////////////////////
/// Interface class for resolving entities during function execution
///
class IResolver
{
public:
    virtual ~IResolver() {}
    virtual bool ResolveIdentifier(const string& identifier, CMQueryNodeValue& vi, const CQueryParseTree::TNode* parent) = 0;
    virtual void CallFunction(const string& name, CQueryParseTree::TNode& qnode) = 0;

    virtual CQueryParseTree* GetAssignmentWhereClause(int index) const = 0;
    virtual CRef<CMQueryNodeValue> GetOrCreateRTVar(const string& name) = 0;
    virtual bool ExistRTVar(const string& name) = 0;

    virtual void AddTmpRTVarObject(const string& name, CObjectInfo& oi) = 0;
    virtual bool GetTmpRTVarObject(const string& name, CObjectInfo& oi) = 0;
    virtual void ResetTmpRTVarObjects() = 0;
};

///////////////////////////////////////////////////////////////////////////////
///  Subclass of CQueryExec that adds:
///   1) Macro identifiers resolution
///   2) Where clause evaluation
///
///  It implements EvaluateTree method for execution of both Where and Do clauses. 
///  Execution model is derived from its parent CQueryExec from QParse library (util/qparse). 
///  This model is based on bottom-up iteration through tree nodes. 
///  Result of node evaluation is stored in the same node as the node user object. 
///  Nodes evaluation is done by function-classes which are registered by x_Init. 
///  If node is an identifier or a function then their resolving is redirected to 
///  CMacroEngine class as an execution context.
class NCBI_GUIOBJUTILS_EXPORT CMacroExec : public CQueryExec
{
    typedef class CQueryExec TBase;

public:
    CMacroExec()
        : CQueryExec(), 
        m_CaseSensitive(false), 
        m_NotSet(false), 
        m_Boolean(false),
        m_Result(false) {}

    virtual ~CMacroExec() {}

    /// Evaluates tree
    void EvaluateTree(CQueryParseTree& Qtree, IResolver& resolver, bool query_tree, bool case_sensitive = false);

    CRef<CMQueryNodeValue> GetOrCreateRTVar(const string& name);

    /// Check/get functions result from the top node after calculation
    bool IsNotSetType() const { return m_NotSet; }
    bool IsBoolType() const { return m_Boolean; }
    bool GetBoolValue() const { return m_Result; }
    
    /// Get function to access case sensitivity flag
    bool IsCaseSensitive() const { return m_CaseSensitive; }

    /// Identifier resolver function
    bool PassIdentifierToResolver(const string& identifier, CMQueryNodeValue& NodeValue, const CQueryParseTree::TNode* parent);
    IResolver* GetResolver() const { return m_Resolver; }

    /// Stubs for virtual functions
    virtual bool ResolveIdentifier(const string& ident, bool& value) 
        {return false;}
    virtual bool ResolveIdentifier(const string& ident, Int8& value) 
        {return false;}
    virtual bool ResolveIdentifier(const string& ident, double& value) 
        {return false;}
    virtual bool ResolveIdentifier(const string& ident, string& value)
        {return false;}
private:
    CRef<CMQueryNodeValue> x_GetTopUserNodeValue();

    void x_Init();

    bool x_EvaluateQueryTree(CQueryParseTree::TNode& node);

    IResolver* m_Resolver;    ///< Variables resolver
    
    /// Case sensitive flag for string comparisons
    bool m_CaseSensitive;

    // Result of calculation
    bool m_NotSet;  ///< Type of result is not set
    bool m_Boolean; ///< Type of result is boolean
    bool m_Result;  ///< Boolean result of calculation
};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_EXEC__HPP

