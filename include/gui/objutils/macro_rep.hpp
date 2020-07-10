#ifndef GUI_OBJUTILS___MACRO_REP__HPP
#define GUI_OBJUTILS___MACRO_REP__HPP

/*  $Id: macro_rep.hpp 44980 2020-05-01 13:48:03Z asztalos $
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
 * File Description: Macro representation
 *
 */

/// @file macro_bin_rep.hpp
/// Macro representation

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>

#include <util/qparse/query_exec.hpp>
#include <util/range.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/query_macro_user_object.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */
 
BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

/// Abstract interface for variable representation
///
class NCBI_GUIOBJUTILS_EXPORT IMacroVar
{
public:
    /// Constructor & destructor
    IMacroVar(const string& var_name) : m_VarName(var_name) {}
    virtual ~IMacroVar() {};

    /// Fill in IQueryMacroUserObject-derived node
    /// Return true if successful
    bool GetNodeValue(IQueryMacroUserObject& v) const;
    /// Get the name of stored variable
    const string& GetName() const { return m_VarName; }
    /// Returns true is the variable can be resolved via GUI
    virtual bool IsGUIResolvable() const = 0;
    /// Sets GUI resolved value
    virtual bool SetGUIResolvedValue(const string& new_value) = 0;
    /// Prints variable
    virtual void Print(CNcbiOstream& os) const
    {
        os << m_VarName << " = <" << x_GetValue().AsString() << ">" << endl;
    }
    string GetGUIResolvedValue() {return x_GetValue().AsString();}
protected:
    /// Name of variable
    string m_VarName;    

    /// Type of stored value (not the type of variable)
    enum EValueType { 
        eValueTypeString = 0,
        eValueTypeInt,
        eValueTypeFloat,
        eValueTypeBool,
        eValueTypeNotSet
    };

    /// Variable value storage structure
    struct SValue
    {
        SValue(const string& str);
        SValue(Int8 val); 
        SValue(bool val); 
        SValue(double val);

        const string& AsString() const { return m_String; }
        void Reset(void);
         
        void Set(const string& str);
        void Set(Int8 val);
        void Set(double val);
        void Set(bool val);
        
        string m_String;
        Int8   m_Int;
        bool   m_Bool;
        double m_Double;

        EValueType m_Type;
    private:
        void x_WriteAsString(void)
        {
            switch (m_Type) {
            case eValueTypeInt:
                m_String = NStr::Int8ToString(m_Int);
                break;
            case eValueTypeFloat:
                m_String = NStr::DoubleToString(m_Double);
                break;
            case eValueTypeBool:
                m_String = (m_Bool) ? "true" : "false";
                break;
            case eValueTypeString:
            default:
                break;
            }
        }
    };
private:
    /// Gets the variable value from derived classes
    virtual SValue x_GetValue() const = 0;
};

/// derived classes for representation of specific variable types. 

/// Class for simple variable representation (i.e. name = "value")
///
class NCBI_GUIOBJUTILS_EXPORT CMacroVarSimple : public IMacroVar
{
public:
    // Constructors
    CMacroVarSimple(const string& name, bool value) : IMacroVar(name), m_Value(value) {} 
    CMacroVarSimple(const string& name, Int8 value) : IMacroVar(name), m_Value(value) {}
    CMacroVarSimple(const string& name, const string& value) : IMacroVar(name), m_Value(value) {}
    CMacroVarSimple(const string& name, double value) : IMacroVar(name), m_Value(value) {}

    /// Destructor
    virtual ~CMacroVarSimple() {}

    /// Returns true is the variable can be resolved via GUI
    virtual bool IsGUIResolvable() const { return false; }
    /// Sets GUI resolved value
    virtual bool SetGUIResolvedValue(const string& new_value) { return false; }

private:
    /// Gets the variable value
    virtual SValue x_GetValue() const { return m_Value; };

    /// Value of the variable
    SValue m_Value;
};

/// Class for "ask" variable representation (i.e. name = %default value%)
///
/// If the variable is resolved through the GUI, the class can hold the resolved value.
class NCBI_GUIOBJUTILS_EXPORT CMacroVarAsk : public IMacroVar
{
public:
    /// Constructor
    CMacroVarAsk(const string& name, const string& def_value);
    /// Destructor
    virtual ~CMacroVarAsk() {};
    
    // /// Sets the value of variable after interaction with user
    // void SetValue(const string& Value);

    /// Returns true is the variable can be resolved via GUI
    virtual bool IsGUIResolvable() const { return true; }
    /// Sets GUI resolved value
    virtual bool SetGUIResolvedValue(const string& new_value);
    /// Get default value
    const string& GetDefaultValue() const { return m_DefaultValue.AsString(); }
    /// Prints variable
    virtual void Print(CNcbiOstream& os) const;

private:
    /// Gets the variable value
    virtual SValue x_GetValue() const { return m_NewValue; }
    
    void x_ParseString(const string& str, bool new_value = false);
    
    /// Default value of "ask" variable
    SValue m_DefaultValue;
    /// User defined value of "ask" variable
    SValue m_NewValue;
};

/// Class for "choice" variable representation (i.e. name = {"choice value 1", "choice value 2"})
///
/// If the variable is resolved through the GUI, the class can hold the resolved value.
class NCBI_GUIOBJUTILS_EXPORT CMacroVarChoice : public IMacroVar
{
public:
    /// Constructor
    CMacroVarChoice(const string& name) : IMacroVar(name), m_Selection(0) {}
    /// Destructor
    virtual ~CMacroVarChoice() {};
    
    // Functions add "choice" variable values
    void AddChoiceBool(bool value) { m_Choices.push_back(SValue(value)); }
    void AddChoiceInt(Int8 value) { m_Choices.push_back(SValue(value)); }
    void AddChoiceFloat(double value) { m_Choices.push_back(SValue(value)); }
    void AddChoiceString(const string& value) { m_Choices.push_back(SValue(value)); }
    
    /// Returns true is the variable can be resolved via GUI
    virtual bool IsGUIResolvable() const { return true; }
    /// Sets GUI resolved value
    virtual bool SetGUIResolvedValue(const string& new_value);
    /// Get functions for stored choices
    const string* GetFirstChoice();
    const string* GetNextChoice();
    /// Prints variable
    virtual void Print(CNcbiOstream& os) const;

private:
    /// Returns the variable value
    virtual SValue x_GetValue() const;
    
    /// Storage for "choice" variable values
    vector<SValue> m_Choices;
    /// User defined selection of choice value
    size_t         m_Selection;
    
private:
    /// Internal state for get choices functions
    size_t m_ReturnedChoice;
};



/// Class for parsed macro representation
///
/// stores internal binary macro representation and 
/// implements an interface for access to:
///    Macro name
///    Macro title (description)  - optional
///....Set of variables
///....“foreach” selector   - optional
///....Which-clause tree
///....Do-clause tree
class NCBI_GUIOBJUTILS_EXPORT CMacroRep : public CObject
{
public:
    /// Typedef vector of variables
    typedef list<IMacroVar*> TVariables;
    /// Typedef vector of blocks of variables
    typedef list<TVariables> TVarBlocks;
    
    /// Name for top level function
    static const string m_TopFuncName;
public:
    /// Constructor
    CMacroRep(CQueryExec* exec=NULL);
    /// Destructor
    ~CMacroRep();

    /// Output class content to NcbiCout
    void Print(CNcbiOstream& os) const;

    // Set of building functions to be called during parsing

    /// Set the name of the macro
    void SetName(const string& name) { m_Name = name; }
    /// Set the title of the macro
    void SetTitle(const string& title) { m_Title = title; }

    void AddMetaKeywords(const string& keyWords)
    {
        if (m_KeyWords.empty())
            m_KeyWords = keyWords;
        else {
            m_KeyWords += ",";
            m_KeyWords += keyWords;
        }
    }

    const string& GetMetaKeywords() const { return m_KeyWords; }

    /// Set the new variable block
    void SetNewVarBlock() { ++m_NumOfVarBlocks; }

    // Functions for setting variables' names and values
    void SetVarInt(const string& name, Int8 value);
    void SetVarFloat(const string& name, double value);
    void SetVarBool(const string& name, bool value);
    void SetVarString(const string& name, const string& value);
    
    void SetVarAsk(const string& name, const string& value);

    void SetVarChoiceName(const string& name);
    void SetVarChoiceInt(Int8 value);
    void SetVarChoiceFloat(double value);
    void SetVarChoiceBool(bool value);
    void SetVarChoiceString(const string& value);
    
    /// Set "for each" asn selector
    void SetForEachItem(const string& name) { m_Foreach = name; };
    /// Set named annotation 
    void SetNamedAnnot(const string& name) { m_NamedAnnot = name; }
    /// Set the sequence range
    void SetSeqRange(const TSeqRange& range) { m_SeqRange = range; }
    /// Set number of threads
    void SetThreadCount(unsigned count) { m_NrThreads = count; }
    /// Attach where tree after its constrution in qParser
    void AttachWhereTree(CQueryParseTree* pWhereTree);

    /// Set function name
    void SetFunction(const string& name, const CQueryParseNode::SSrcLoc &pos, CQueryParseTree::TNode *parentNode = 0);

    /// Set the function parameters
    void SetFunctionParameterAsIdent(const string& name, const CQueryParseNode::SSrcLoc &pos);
    void SetFunctionParameterAsInt(Int8 value, const CQueryParseNode::SSrcLoc &pos);
    void SetFunctionParameterAsFloat(double value, const CQueryParseNode::SSrcLoc &pos);
    void SetFunctionParameterAsBool(bool value, const CQueryParseNode::SSrcLoc &pos);
    void SetFunctionParameterAsString(const string& value, const CQueryParseNode::SSrcLoc &pos);

    /// Set the function Where clause as a part of an assignment operator
    void SetAssignmentWhereClause(const string& where_str, CQueryParseTree* where_tree, const CQueryParseNode::SSrcLoc &pos);

    /// Work with macro source text
    void SetSource (const string& source_text) { m_SourceText = source_text; }
    const string& GetSource() const { return m_SourceText; }

    // Access functions

    /// Look up the variable among the stored ones
    bool FindVar(const string& name) const;
    
    /// Function fills in IQueryMacroUserObject-derived node out of stored variable
    /// @param name
    ///   A constant string name of variable being searched for
    /// @param name
    ///   A IQueryMacroUserObject-derived node to be filled in
    /// @return 
    ///   - true if node was filled in.
    ///   - false if variable was not found or problem occured.
    bool GetNodeValue(const string& name, IQueryMacroUserObject& v) const;

    /// Return macro name
    const string& GetName() const { return m_Name; }
    /// Return macro description
    const string& GetTitle() const {return m_Title; }

    /// Return "for each" string
    const string& GetForEachString() const { return m_Foreach; }
    /// Return "from" named annotation
    const string& GetNamedAnnot() const { return m_NamedAnnot;  }
    /// Return sequence range
    const TSeqRange& GetSeqRange() const { return m_SeqRange; }
    /// Return "where" clause
    CQueryParseTree* GetWhereClause() const { return m_WhereTree.get(); }
    /// Return "do" clause
    CQueryParseTree* GetDoTree() const { return m_DoTree.get(); }
    /// Return number of threads
    unsigned GetThreadCount() const { return m_NrThreads; }

    /// Return functions for stored vars
    IMacroVar* GetFirstVar(Int4& block_num);
    IMacroVar* GetNextVar(Int4& block_num);

    /// Return true if identifier is in datasource (only works if m_Exec not NULL)
    bool HasIdentifier(const string& ident) const;

    CQueryParseTree* GetAssignmentWhereClause(int index) const; 

    /// Return true if there are any GUI variables
    bool AreThereGUIResolvableVars() const { return m_GUIResolvable; };
    
    /// Get number of variable blocks
    Int4 GetBlocksCount() const { return m_NumOfVarBlocks; }

private:
    /// Print a tree with a given header and separator
    void x_PrintTree(CNcbiOstream& os, 
                        CQueryParseTree* tree, 
                                 const string& title, 
                        const string& separator) const;

    /// Locate the variable if it was stored.
    IMacroVar* x_FindVar(const string& name) const;

    /// Helper function. It finds or creates a block for new variable
    TVariables& x_GetOrCreateLastBlock();

    /// Execution objection associated with this macro (may be NULL)
    CQueryExec*               m_Exec{ nullptr };

    string                    m_Name;           ///< Macro name
    string                    m_Title;          ///< Macro title

    string                    m_KeyWords;       ///< Meta keywords for search

    TVarBlocks                m_VarBlocks;      ///< Variables
    string                    m_Foreach;        ///< For each string
    string                    m_NamedAnnot;     ///< Named annotation
    TSeqRange                 m_SeqRange;       ///< Sequence range
    auto_ptr<CQueryParseTree> m_WhereTree;      ///< Parsed Where-clause
    auto_ptr<CQueryParseTree> m_DoTree;         ///< Parsed function calls
    unsigned                  m_NrThreads{ 1 }; ///< Number of execution threads

    /// Vector of parsed function Where clauses.
    /// They are stored here with indexes to them from eWhere nodes from the tree.
    /// Due to limitation of CQueryParseTree::TNode, they can't be inserted easily in the tree itself.
    vector<CQueryParseTree*> m_FuncWhereClauses;

    string                    m_SourceText;     ///< Macro source text   

    // Internal states
    Int4                      m_NumOfVarBlocks; ///< Number of blocks of variables
    CQueryParseTree::TNode*   m_ActiveNode;     ///< Active tree node in where clause
    bool                      m_GUIResolvable;  ///< True if there is at least one GUI resolvable var

    // Iterators for get vars functions
    TVariables::iterator      m_VarIter;
    TVarBlocks::iterator      m_BlockIter;
    Int4                      m_BlockNumber;

    /// Prohibit copy constructor and assignment operator
    CMacroRep(const CMacroRep&);
    CMacroRep& operator=(const CMacroRep&);
};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
 
#endif  // GUI_OBJUTILS___MACRO_REP__HPP
