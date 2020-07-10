#ifndef GUI_OBJUTILS___MACRO_ENGINE_PARALLEL__HPP
#define GUI_OBJUTILS___MACRO_ENGINE_PARALLEL__HPP
/*  $Id: macro_engine_parallel.hpp 45086 2020-05-28 14:46:27Z asztalos $
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
 * Authors: Anatoly Osipov, Andrea Asztalos
 *
 * File Description: Macro engine for macro processing (refactored)
 *
 */

/// @file macro_engine.hpp
/// Macro engine for macro execution
#include <gui/gui_export.h>
#include <gui/objutils/macro_biodata.hpp>
#include <gui/objutils/macro_lib.hpp>
#include <gui/objutils/macro_parse.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>
#include <gui/objutils/macro_cmd_composite.hpp>
#include <gui/objutils/macro_engine.hpp>
#include <gui/utils/command_processor.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)


class IMacroFunctionContext : public CObject
{
public:
    virtual ~IMacroFunctionContext();

    /// Return pointer to macro function identified by its name
    IEditMacroFunction* ResolveFunctionName(const string& name) const;

    void GetFunctionNames(CMacroParser::TFunctionNamesList& where_funcs, CMacroParser::TFunctionNamesList& do_funcs) const;

protected:
    IMacroFunctionContext() {}

    using TBuiltInFunctionsMap = map<string, IEditMacroFunction*, PNocase>;
    TBuiltInFunctionsMap m_BuiltInFunctions;

};

/// Defines the full set of macro functions
class CMacroContextForEditing : public IMacroFunctionContext
{
public:
    CMacroContextForEditing() : IMacroFunctionContext() { x_InitSetOfBuiltInFunctions(); }
private:
    void x_InitSetOfBuiltInFunctions();
};

// Defines the set of functions used for printing and some auxiliary macro functions
class CMacroContextForListing : public IMacroFunctionContext
{
public:
    CMacroContextForListing() : IMacroFunctionContext() { x_InitSetOfBuiltInFunctions(); }
private:
    void x_InitSetOfBuiltInFunctions();
};



///////////////////////////////////////////////////////////////////////////////
/// CMacroResolver
/// Variable and function resolver 
/// Provides interfaces for:
/// - resolving identifier met in the macro into variable value or biodata object (qualifier) value
/// - calling macro function identified by its name 
/// It also keeps the run-time (RT) variables that are assigned a value in the DO-clause of a macro.
///

class CMacroResolver : public IResolver
{
public:
    CMacroResolver(const CMacroRep& macro_rep, CIRef<IMacroBioDataIter> data_iter,
        CRef<CMacroCmdComposite> CmdComposite, CConstIRef<IMacroFunctionContext> context, CMacroStat& stat)
        : m_MacroRep(&macro_rep),
        m_DataIter(data_iter),
        m_MacroStat(&stat),
        m_CmdComposite(CmdComposite),
        m_FuncContext(context) {}

    /// Resolve identifier and store the value in @val
    virtual bool ResolveIdentifier(const string& identifier, CMQueryNodeValue& val, const CQueryParseTree::TNode* parent);
    /// Function call the function specified by name, passing args as parameters and 
    /// it can update nv as a result.
    virtual void CallFunction(const string& name, CQueryParseTree::TNode& qnode);

    /// Return the "where" sub-query from the DO-DONE section identified by the index in the main parsed tree
    virtual CQueryParseTree* GetAssignmentWhereClause(int index) const;

    /// Get or create run-time variable
    virtual CRef<CMQueryNodeValue> GetOrCreateRTVar(const string& name);

    /// Return true if the run-time variable, identified by its name, is defined
    virtual bool ExistRTVar(const string& name);

    // Temporary run-time variables used in evaluating subqueries in the DO section,
    /// used together with the Assignment Operator (CMQueryFunctionAssignment)
    virtual void AddTmpRTVarObject(const string& name, CObjectInfo& oi);
    virtual bool GetTmpRTVarObject(const string& name, CObjectInfo& oi);
    virtual void ResetTmpRTVarObjects();

private:
    /// Return the value of Run-Time (RT) variable
    bool x_ResolveRTVar(const string& identifier, CMQueryNodeValue& val, const CQueryParseTree::TNode* parent);

    /// Return data associated with the RT variable based on its name
    CRef<CMQueryNodeValue> x_LocateRTVar(const string& identifier);
    
    CConstRef<CMacroRep>     m_MacroRep;
    
    CIRef<IMacroBioDataIter> m_DataIter;
    CMacroStat* m_MacroStat;
    CRef<CMacroCmdComposite> m_CmdComposite;

    CConstIRef<IMacroFunctionContext>  m_FuncContext;

    typedef map<string, CRef<CMQueryNodeValue>> TRTVarsMap;
    TRTVarsMap m_RTVars;

    typedef map<string, CObjectInfo> TTempRTVarsMap;
    TTempRTVarsMap m_TempRTVars;
};



/// CMacroEngineParallel
///
///	Provides interfaces for:
/// - parsing single macro into its binary representation
/// - parsing library (file) of macros
/// - executing macro binary representation
/// 
/// This class keeps the context for execution. 
/// All variables in macro representation are to be resolved by GUI if required.

class NCBI_GUIOBJUTILS_EXPORT CMacroEngineParallel
{
public:
    CMacroEngineParallel() : m_EngineFuncContext(new CMacroContextForEditing()) {}

    /// Parse a file containing macros and append the results to the map that stores these macros
    /// 
    /// @param filename
    ///   Full name (with path) for a file to be parsed. 
    /// @param lib
    ///   The map that internally stores macros
    /// @return
    ///   - true, if the file is read and parsed successfully
    ///   - false, otherwise.
    bool AppendToLibrary(const string& filename, CMacroLib::TMacroLibrary& lib);
    /// Parse a file containing macros and store them in a vector
    ///@param[in] filename
    ///  Name of the file to be parsed
    ///@param[in,out] macro_list
    ///  Vector containing the parsed macros
    bool ReadAndParseMacros(const string& filename, vector<CRef<CMacroRep>>& macro_list);

    /// Parse the macro script into its binary representation. This may need to be preprocessed to
    /// let user resolve "ask" and "choice" variables.
    CMacroRep* Parse(const string& macro_text);

    /// Execute a macro
    ///@param[in] macro_rep 
    ///  Macro in its binary representation. It should be free of unresolved "ask" and "choice" variables
    ///@param[in] data
    ///  Data (top seq-entry and optionally seq-submit) which will be edited by the macro
    ///@param[in,out] CmdComposite
    ///  Storage for editing commands
    ///@param[in] throw_on_error
    ///  Flag, indicating whether to throw exceptions on errors
    ///@param[in,out] ostream
    ///  Pointer to an output stream used by the listing macro functions
    bool Exec(const CMacroRep& macro_rep, const CMacroBioData& data, CRef<CMacroCmdComposite> CmdComposite, bool throw_on_error = false, CNcbiOstream* ostream = nullptr);

    /// Get status of most recent command
    bool GetParsingStatus() const { return m_ParsingInfo.m_Status; }

    /// Get error message in case previous command was unsuccessful.
    const string& GetParsingErrorMessage() const { return m_ParsingInfo.m_ErrorMessage; }

    /// Get error location in case previous command was unsuccessful.
    /// Numbering starts from 1.
    /// Horizontal tabs are treated to be of length 1.
    Uint4 GetParsingErrorLine()   const { return (m_ParsingInfo.m_Status) ? 0 : m_ParsingInfo.m_Line; }
    Uint4 GetParsingErrorColumn() const { return (m_ParsingInfo.m_Status) ? 0 : m_ParsingInfo.m_Column; }

    /// Gets the most recently executed macro statistics
    const CMacroStat& GetStatistics() const { return m_MacroStat; }

    struct SMacroInfo
    {
        bool m_Status{ true };    ///< status of the activity (parsing or execution)
        string m_ErrorMessage;     ///< error message
        Uint4 m_Line{ 0 }, m_Column{ 0 };  ///< location of problem within the macro script

        void Reset()
        {
            m_Status = true;
            m_ErrorMessage.resize(0);
            m_Line = m_Column = 0;
        }

        void SetError(const string& message, Uint4 line, Uint4 column)
        {
            m_ErrorMessage = message;
            m_Line = line;
            m_Column = column;
            m_Status = false;
        }
    };

private:
    bool x_AppendToLibrary(const string& filename, CMacroLib::TMacroLibrary& lib);

    /// Initialize do/where lists of functions in parser.
    void x_SetFunctions(CMacroParser& parser);

private:
    SMacroInfo m_ParsingInfo;       ///< Status, error message related to parsing a macro
    CMacroStat m_MacroStat;         ///< Most recent executed macro statistics
    CIRef<IMacroFunctionContext>  m_EngineFuncContext;

};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_ENGINE_PARALLEL__HPP
