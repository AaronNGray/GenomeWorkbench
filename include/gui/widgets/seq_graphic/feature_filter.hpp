#ifndef GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_FILTER__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_FILTER__HPP

/*  $Id: feature_filter.hpp 44767 2020-03-06 22:00:40Z rudnev $
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
 * Authors: Liangshou Wu
 *
 * File Description: 
 * Header file for classes needed to imeplement query execution for  
 * feature filtering.
 *
 */

#include <gui/gui_export.h>
#include <objmgr/scope.hpp>
#include <objmgr/mapped_feat.hpp>
#include <util/qparse/query_exec.hpp>

BEGIN_NCBI_SCOPE

typedef map<string, CQueryParseNode::EType> TIdentifierDict;

///////////////////////////////////////////////////////////////////////////////
/// Query parser exceptions
/// class CFeatFilterQueryException
/// 
/// The different types of errors that can occur after the query is parsed while
/// it is being executed.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CFeatFilterQueryException : public CException
{
public:
    enum EErrCode {
        eIncorrectNodeType,     // a given query node is incorrect, e.g. "abc"
        eInvalidQuery,          // a given query or part of query is invalid or
                                //   doesn't make sense, e.g. 0 < "abc", 1 = 1
        eWrongArgumentCount,    // Number of arguments for an operand are incorrect,
                                //   e.g. '==' has 3 operands.  The initial parser
                                //   may catch all of these.
        eNullFeature,           // evaluation done before a feature is assigned
        eUnknownError
    };

    virtual const char* GetErrCodeString(void) const override
    {
        switch (GetErrCode())
        {
        case eIncorrectNodeType:       return "eIncorrectNodeType";
        case eInvalidQuery:            return "eInvalidQuery";
        case eWrongArgumentCount:      return "eWrongArgumentCount";
        case eUnknownError:            return "eUnknownError";
        case eNullFeature:             return "eNullFeature";
            
        default: return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT(CFeatFilterQueryException, CException);
};

///////////////////////////////////////////////////////////////////////////////
///  class CFeatureFilter
///
///  Subclass of CQueryExec that adds functions spcific to the feature
///  filtering that allow it to extract data from the feature lists of 
///  individual nodes so that those values can be used in queries.
/// 

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CFeatureFilter : public CQueryExec
{
public:
    CFeatureFilter();

    virtual ~CFeatureFilter() {}

    void Init(const string& filter);

    bool Pass(const objects::CMappedFeat* feat);

    void AddId(const string& id, CQueryParseNode::EType type);

    const TIdentifierDict* GetIdDictionary() const;

    bool NeedFiltering() const;

    /// The following functions search in the feature asn for a value with
    /// the name 'identifier' and, if found, try to convert it to the 
    /// specified type, returning true if it is found and converted 
    /// successfully.
    
    virtual bool ResolveIdentifier(const std::string& identifier,
                                   bool& value);

    /// Search for integer 'identifier' in feature list and return if found
    virtual bool ResolveIdentifier(const std::string& id, Int8& value);

    virtual bool ResolveIdentifier(const std::string& /*identifier*/, 
                                   double& /*value*/) { return false; }

    /// Search for string-value 'identifier' in feature list and return if found
    virtual bool ResolveIdentifier(const std::string& id, std::string& value);
    
    /// Search for 'identifier' in nodes feature list and true if it exists
    virtual bool HasIdentifier(const std::string& id);
    
private:
    CQueryParseTree         m_Qtree;

    /// Evaluated object.
    /// A mapped feature in this case.
    const objects::CMappedFeat*    m_EvalFeat;

    /// Identifier dictionary.
    TIdentifierDict         m_IdDict;

    ///
    bool                    m_NeedFiltering;
};


///////////////////////////////////////////////////////////////////////////////
///  class CQueryNodeValue
///
///  Subclass of the IQueryParseUserObject which is held as the user-defined object
///  in each CQueryParseNode.  Holds a EStatus value that tells us if the
///  subexpression evaluted to true, false, or invalid, so tht that value can be
///  passed up the tree.  Also keeps track of whether that value has been set.
///
class CSGQueryNodeValue : public IQueryParseUserObject
{
public:

    CSGQueryNodeValue() 
        : m_Node(NULL)
        , m_DataType(CQueryParseNode::eNotSet)
        , m_Valid(true)
        , m_IsDataField(false)
    {}

    CSGQueryNodeValue(CQueryParseTree::TNode* n) 
        : m_Node(n)
        , m_DataType(CQueryParseNode::eNotSet)
        , m_Valid(true)
        , m_IsDataField(false)
    {}

    virtual void Reset()
    {
        // do nothing
    }
    
    bool IsValid() const { return  m_Valid; }
    void SetValid(bool f) { m_Valid = f; }

    /// Set/get underlying data type
    void SetDataType(CQueryParseNode::EType dt) {m_DataType = dt; }
    CQueryParseNode::EType GetDataType() const { return m_DataType; }

    /// Get to indicate if this is a field from the tree or simple string
    bool IsDataField() const  { return m_IsDataField; }
    void SetDataField(bool f) { m_IsDataField = f; }

    /// promote this value to the given type.
    /// if doen't make sense to do promotion, data type will be e_NotSet.
    bool PromoteTo(CQueryParseNode::EType type);

    /// Node from parsed query tree
    CQueryParseTree::TNode* m_Node;

    /// String data, if data came from a string or data fied in the tree
    std::string m_String;

    /// Bool data, if data base a constant boolean or converted into one
    bool m_Bool;

    /// Int data, if data was an integer or converted into one
    Int8  m_Int;

    /// Floating point data, if data was a double or converted into one
    double m_Double;

private:

    /// Node value data type.
    /// The node value data type used for evaluation or comparison. It need to
    /// be one of the followings: eIntConst, eFloatConst, eBoolConst, and eString
    CQueryParseNode::EType m_DataType;

    /// Evaluation status.
    /// true, false.
    bool        m_Valid;

    /// Flag indicating if this value is from query objects.
    bool        m_IsDataField;
};


///////////////////////////////////////////////////////////////////////////////
///  class CSGQueryNodePreprocessor
///
///  This class applies an operation to each notde of the query tree to do 
///  any necessary preprocessing, initialization, and optiminzation prior 
///  to execution.
///
class CSGQueryNodePreprocessor
{
public:
    CSGQueryNodePreprocessor(TIdentifierDict* d);
          
    ETreeTraverseCode operator()(CTreeNode<CQueryParseNode>& tr, int delta);

private:
    TIdentifierDict* m_Dictionary;
};


///////////////////////////////////////////////////////////////////////////////
///  class CSGQueryFunctionValue
///
///  Query execution function for simple atomic values.
///
class CSGQueryFunctionValue : public CQueryFunctionBase
{
public:
    typedef CQueryFunctionBase TParent;

public:
    CSGQueryFunctionValue() {}
    virtual void Evaluate(CQueryParseTree::TNode& node);
};



///////////////////////////////////////////////////////////////////////////////
///  class CSGQueryFunctionLogic
///
///  Query execution function for logical operators like AND, OR, etc.
///
class CSGQueryFunctionLogic : public CQueryFunctionBase
{
public:
    typedef CQueryFunctionBase TParent;

public:
    CSGQueryFunctionLogic() {}

    virtual void Evaluate(CQueryParseTree::TNode& node);
};



///////////////////////////////////////////////////////////////////////////////
///  class CSGQueryFunctionCompare
///
///  Query execution function for comparison operators like <, ==, >.
///
////
class CSGQueryFunctionCompare : public CQueryFunctionBase
{    
public:
    typedef CQueryFunctionBase TParent;

public:
    /// Ctors for compare must include op_type for loading promotion rules
    CSGQueryFunctionCompare() {};

    virtual void Evaluate(CQueryParseTree::TNode& node);
};



END_NCBI_SCOPE


#endif  // GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_FILTER__HPP

