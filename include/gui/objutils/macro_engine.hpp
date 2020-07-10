#ifndef GUI_OBJUTILS___MACRO_ENGINE__HPP
#define GUI_OBJUTILS___MACRO_ENGINE__HPP
/*  $Id: macro_engine.hpp 45047 2020-05-15 19:30:14Z asztalos $
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
 * File Description: Macro engine for macro processing
 *
 */

/// @file macro_engine.hpp
/// Macro engine for macro execution
#include <gui/gui_export.h>
#include <gui/objutils/macro_biodata.hpp>
#include <gui/objutils/macro_lib.hpp>
#include <gui/objutils/macro_parse.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/objutils/macro_cmd_composite.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

///////////////////////////////////////////////////////////////////////////////
/// CMacroStat - collecting statistics about a single macro 
///
/// collector for macro execution statistics, 
/// such as execution time and number of changed qualifiers
/// everything is logged via LOG_POST so all standard logging conventions apply
class NCBI_GUIOBJUTILS_EXPORT CMacroStat : public CObject 
{
public:
    CMacroStat() { Reset(); }
    /// reset the macro name, qualifier and iteration counts
    void Reset();


    ~CMacroStat() {}
    class CMacroLog 
    {
    public:
        CMacroLog() :  m_MacroName(kEmptyStr), m_Log(kEmptyStr) {}
        ~CMacroLog() {}

        const string& GetName() const { return m_MacroName; }
        const string& GetLog() const { return m_Log; }

        void SetName(const string& name)
        {
            if (name.empty()) {
                m_Log.clear();
            }
            m_MacroName.assign(name);
        }
        
        void BuildReport(const string& rep)
        {
            if (!m_MacroName.empty()) {
                m_Log.append(rep);
            }
        }

        void ClearAndBuildReport(const string& rep)
        {
            if (!m_MacroName.empty()){
                m_Log = rep;
            }
        }
        void Reset() 
        { 
            m_MacroName.resize(0);
            m_Log.resize(0);
        } 
    private:
        /// name of macro
        string m_MacroName; 
        /// Logging all activities during this macro
        string m_Log;
    };

    
    const CMacroLog& GetMacroReport() const { return m_MacroReport; }
    
    /// function gradually builds the log report
    void AddToReport(const string& report);

    /// log start of macro execution
    /// @param macro_name
    ///   name of the macro which is started
    ///   is printed out and memorized until the next Log_Start()/Reset() call
    void LogStart(const string& macro_name);

    /// log end of macro execution
    /// this includes time of execution, status, count of changed qualifiers
    /// and an error message when applicable
    /// result also goes to the GBench status bar
    /// @param status
    ///    true if no errors occured, false otherwise
    /// @param err_message
    ///    additional error message which is printed when status is false
    void LogStop(bool status, const string& err_message);

    long GetExecutionTime() const { return static_cast<long>(m_StopTime - m_StartTime); }
    /// increment number of changed qualifiers by a given number
    void LogQualChanged(Int4 increment);
    /// increment number of iterations
    void LogIteration();
    size_t GetCounter() const {return m_counter;}
private:
    time_t m_StartTime;
    time_t m_StopTime;
    CMacroLog m_MacroReport;
    size_t m_counter{ 0 };

    map<string, Int4> m_ChangedQuals;

private:
    CMacroStat(const CMacroStat&);
    CMacroStat& operator=(const CMacroStat&);
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroEngine
///
///	Provides interfaces for:
/// - parsing single macro into its binary representation
/// - parsing library (file) of macros
/// - executing macro binary representation
/// - resolving identifier met in the macro into variable value or biodata object (qualifier) value
/// - calling function by name met in the macro.
///
/// Additionally, this class keeps the run-time variables that are assigned a value in the Do-clause of a macro.
///
/// This class keeps the context for execution. All variables in macro representation are to be resolved by GUI if required.

class NCBI_GUIOBJUTILS_EXPORT CMacroEngine : public IResolver
{
public:
    /// Constructor
    CMacroEngine(): m_Status(false), 
                    m_ErrorMessage("Not executed"), 
                    m_Line(0), 
                    m_Column(0) { x_InitSetOfBuiltInFunctions(); }

    virtual ~CMacroEngine();

    /// Parses a file containing macros and appends the results to a map.
    /// 
    /// @param filename
    ///   Full name (with path) for a file to be parsed. 
    /// @param lib
    ///   The map that internally stores macros
    /// @return
    ///   - true, if the file is read and parsed successfully
    ///   - false, otherwise.
    bool AppendToLibrary(const string& filename, CMacroLib::TMacroLibrary& lib);
    /// Parses a file containing macros and stores them in a list
    ///@param[in] filename
    ///  Name of the file to be parsed
    ///@param[in,out] macro_list
    ///  Map in which the parsed macros to be stored
    bool ReadAndParseMacros(const string& filename, vector<CRef<CMacroRep>>& macro_list);

    /// Parses macro script into its binary representation. This may need to be preprocessed to
    /// let user resolve "ask" and "choice" variables.
    CMacroRep* Parse(const string& macro_text);

    /// Executes a macro
    ///@param[in] macro_rep 
    ///  Macro in its binary representation. It should be free of unresolved "ask" and "choice" variables
    ///@param[in] data
    ///  Data (top seq-entry and optionally seq-submit) which will be edited by the macro
    ///@param[in,out] CmdComposite
    ///  Storage for editing commands
    ///@param[in] throw_on_error
    ///  Flag, indicating whether to throw exceptions on errors
    bool Exec(CMacroRep& macro_rep, CMacroBioData& data, CRef<CMacroCmdComposite> CmdComposite, bool throw_on_error = false, CNcbiOstream* ostream = nullptr);

    /// Get status of most recent command
    bool GetStatus() const { return m_Status; }

    /// Get error message in case previous command was unsuccessful.
    const string& GetErrorMessage() const { return m_ErrorMessage; }

    /// Get error location in case previous command was unsuccessful.
    /// Numbering starts from 1.
    /// Horizontal tabs are treated to be of length 1.
    Uint4 GetErrorLine()   const { return (m_Status)? 0 : m_Line; }
    Uint4 GetErrorColumn() const { return (m_Status)? 0 : m_Column; }

    /// Gets the most recently executed macro statistics
    const CMacroStat& GetStatistics() const { return m_MacroStat; }

    /// get one of the "where" sub-clauses which are indentified by an index in the 
    /// main parsed tree
    virtual CQueryParseTree* GetAssignmentWhereClause(int index) const;

    /// Gets or creates run-time vars (used in assignment in Do clause)
    virtual CRef<CMQueryNodeValue> GetOrCreateRTVar(const string& name);

    /// Returns true if the run-time variable, identified by its name, was defined
    virtual bool ExistRTVar(const string& name);

    /// Temporary run-time variables used in evaluating subqueries in the DO section,
    /// used together with the Assignment Operator (CMQueryFunctionAssignment)
    virtual void AddTmpRTVarObject(const string& name, CObjectInfo& oi);
    virtual bool GetTmpRTVarObject(const string& name, CObjectInfo& oi);
    virtual void ResetTmpRTVarObjects();

private:
    bool x_AppendToLibrary(const string& filename, CMacroLib::TMacroLibrary& lib);
    
    /// Gets the value of RT variable
    bool x_ResolveRTVar(const string& identifier, CMQueryNodeValue& v, const CQueryParseTree::TNode* parent);
    /// Gets the pointer to RT variable
    CRef<CMQueryNodeValue> x_LocateRTVar(const string& identifier);

    /// Function resolves identifiers
    virtual bool ResolveIdentifier(const string& identifier, CMQueryNodeValue& v, const CQueryParseTree::TNode* parent) ;

    /// Function call the function specified by name, passing args as parameters and 
    /// it can update nv as a result.
    virtual void CallFunction(const string& name, CQueryParseTree::TNode &qnode);

    /// Reset status and error related private data
    void x_ResetStatus(void);

    /// Reset status and error related private data
    void x_InitSetOfBuiltInFunctions(void);
    
    /// Locates function by name
    IEditMacroFunction* x_ResolveFunctionName(const string& name) const;

    /// Sets m_Status, error_message and its location.
    void x_SetError(const string& message, Uint4 line = 0, Uint4 column = 0);
    
    /// Initializes do/where lists of functions in parser.
    void x_SetFunctions(CMacroParser& parser);

private:
    bool m_Status;          ///< execution status of most recent command
    string m_ErrorMessage;  ///< error message
    Uint4 m_Line, m_Column; ///< location of problem in macro text
    CMacroStat m_MacroStat; ///< Most recent executed macro statistics

    struct SKey_Less
    {
        bool operator()(const string& str1, const string& str2) const
        {
            return NStr::CompareNocase(str1, str2) < 0;
        }
    };

    typedef map<string, IEditMacroFunction*, SKey_Less> TBuiltInFunctionsMap;
    /// Do section functions
    TBuiltInFunctionsMap m_BuiltInFunctions;

    /// temporary data available while Exec is working. It is used by resolver.
    CMacroRep*               m_MacroRep;
    CIRef<IMacroBioDataIter> m_DataIter;
    CRef<CMacroCmdComposite> m_CmdComposite;

    typedef map< string, CRef<CMQueryNodeValue> > TRTVarsMap;
    TRTVarsMap m_RTVars;

    typedef map<string, CObjectInfo> TTempRTVarsMap;
    TTempRTVarsMap m_TempRTVars;

    bool m_EvalDo; ///< Flag indicates if the work should be performed on the Do clause.
};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_ENGINE__HPP
