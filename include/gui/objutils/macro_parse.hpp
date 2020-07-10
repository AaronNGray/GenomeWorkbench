#ifndef GUI_OBJUTILS___MACRO_PARSER__HPP
#define GUI_OBJUTILS___MACRO_PARSER__HPP

/*  $Id: macro_parse.hpp 44980 2020-05-01 13:48:03Z asztalos $ 
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
 * File Description: Macro parser
 *
 */

/// @file macro_parse.hpp
/// Macro parser components

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>

#include <util/qparse/query_exec.hpp>

#include <gui/gui_export.h>
#include <gui/objutils/macro_rep.hpp>

#include <set>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

/// Class for lexical tokens extraction from macro string
///
/// sequentially gets macro lexems from the input string
class CScanner
{
private:    
    /// Max buffer size for lexem string info
    static const int MAX_STRING_LEXEM_LENGTH = 4094;
public:
    enum EToken {
        eTokenILLEGAL        = -1, ///< Error
        eTokenEOS            =  0, ///< If next token is eos

        eTokenKWMACRO           , ///< Keyword macro
        eTokenKWVARS            , ///< Keyword var or vars
        eTokenKWFOR             , ///< Keyword for as a part of for each
        eTokenKWEACH            , ///< Keyword each as a part of for each
        eTokenKWFROM            , ///< Keyword from for specifying named annotation
        eTokenKWCHOICE          , ///< Keyword choice
        eTokenKWRANGE           , ///< Keyword range
        eTokenKWWHERE           , ///< Keyword where
        eTokenKWDO              , ///< Keyword do
        eTokenKWDONE            , ///< Keyword done
        eTokenKWDOPARL          , ///< Keyword do_p(arallel)
        
        eTokenFUNCTION           , ///< Built-in function
        eTokenIDENT              , ///< Identifier
        eTokenASNSELECTOR        , ///< ASN selector
        eTokenNAMEDANNOT         , ///< Named annotation
        eTokenINTNUMBER          , ///< Integer number
        eTokenFLOATNUMBER        , ///< Floating point number
        eTokenSTRING             , ///< String in double quotes
        eTokenASK                , ///< String enclosed in % symbols
        eTokenBOOLCONST          , ///< Boolean const value
        
        eTokenASSIGNMENT         , ///< '=' token
        eTokenLEFTPAREN          , ///< '(' token
        eTokenRIGHTPAREN         , ///< ')' token
        eTokenLEFTBRACKET        , ///< '[' token
        eTokenRIGHTBRACKET       , ///< ']' token
        eTokenLEFTBRACE          , ///< '{' token
        eTokenRIGHTBRACE         , ///< '}' token
        eTokenCOMMA              , ///< ',' token
        eTokenSEMICOLON          , ///< ';' token

        eTokenMINUS              , ///< '-' token
        eTokenPLUS               , ///< '+' token

        eTokenDOT                , ///< '.' token
        eTokenMULT               , ///< '*' token
        eTokenDIV                , ///< '/' token
        eTokenGREATER            , ///< '>' token
        eTokenLESS               , ///< '<' token
        eTokenNOTLESS            , ///< '>=' token
        eTokenNOTGREATER         , ///< '<=' token
        eTokenNOTEQUAL           , ///< '<>' token

        eTokenCOMMENT              ///< Comment read on request
    };

    //Types
    struct SLocation {
        SLocation() : m_Pos(-1), m_Line(0), m_Column(0) {} // set invalid
        SLocation(Int4 pos, int line, int col) : m_Pos(pos), m_Line(line), m_Column(col) {}

        void Reset() { m_Pos = -1; m_Line = 0; m_Column = 0; } // set invalid
        void IncLine() { m_Pos++; m_Line++; m_Column = 1; }
        void IncColumn() { m_Pos++; m_Column++; }

        operator CQueryParseNode::SSrcLoc() const
        {
            return CQueryParseNode::SSrcLoc(m_Line - 1, m_Column - 1); // SSrcLoc positions are 0 based
        }

        Int4 m_Pos;   // Position in string. Zero based.
        unsigned m_Line;   // The first is 1.   line
        unsigned m_Column; // The first is 1.
    };

    struct SData {
        bool   m_BoolValue;
        Int4   m_IntNumber;
        double m_FloatNumber;
        char   m_Str[MAX_STRING_LEXEM_LENGTH + 1];

        void Init()
        {
            m_IntNumber = 0;
            m_FloatNumber = 0.0;
            *m_Str = '\0';
        }
    };

    struct SLexem {
        EToken m_Token;
        SLocation m_Position;  // actual position of the token (it can be also white space)

        SLocation m_Start;   // starting position of the token

        SData m_Data;
        
        SLocation m_ErrorLoc;  // the position of the error
        string m_ErrorMsg;

        string GetStringRepr() const
        {
            CNcbiOstrstream oss;
            switch (m_Token) {
            case eTokenBOOLCONST:
                oss << m_Data.m_BoolValue;
                break;
            case eTokenINTNUMBER:
                oss << m_Data.m_IntNumber;
                break;
            case eTokenFLOATNUMBER:
                oss << m_Data.m_FloatNumber;
                break;
            default:
                oss << m_Data.m_Str;
            }
            return string(CNcbiOstrstreamToString(oss));
        }
    };

public:
    // Constructors
    CScanner(const char* sMacroText) : m_StrMacro(sMacroText)
        { x_InitScanner(); }
    // Destructors
    virtual ~CScanner(){}

    void Reset(const char* sMacroText) { m_StrMacro = sMacroText; x_InitScanner(); }

    // Read the lexem but ASN selector.
    //
    int Next(bool parseComment = false);              
    // Read ASN selector lexem.
    //
    int NextASNSelector(); 
    // Read Named annotation lexem
    //
    int NextNamedAnnot();
    // Get previously read lexem code.
    //
    CScanner::EToken GetToken();
    // Get meta info about lexem.
    //
    const SLexem& GetLexemInfo() const { return m_CurrentLexem; }
    // Get error location in input string.
    //
    SLocation GetErrorLocation() const { return m_CurrentLexem.m_ErrorLoc; }

private:
    // Reset scanner to the initial state. Scanned string is untouched.
    // 
    void x_InitScanner();

    // Get the content of the string limited by the first symbol.
    // 
    int x_GetQuotedString();
    // Skip whitespaces and reset lexem meta info.
    // 
    void x_InitForNextLexem();

    int x_SetCurrentLexem(Int4& pos, CScanner::EToken token);
    // Pointer to the string being scanned.
    // 
    const char* m_StrMacro;
    // Pointer to the lexem meta info.
    // 
    SLexem m_CurrentLexem;

private:
    /// Prohibit copy constructor and assignment operator
    CScanner(const CScanner&);
    CScanner& operator=(const CScanner&);
};



/// Class provides macro language interface for bulk bio-editing
///
/// Parses text containing macro(s) into binary macro representation 
/// (instance of CMacroRep). 
/// While parsing it uses CScanner for extracting lexems from a string and 
/// it uses QParse library (util/qparse) for parsing Where-clause of a macro
class NCBI_GUIOBJUTILS_EXPORT CMacroParser
{
public:
    /// Type for the list of functions in Where/Do clauses
    ///
    typedef CQueryParseTree::TFunctionNames TFunctionNamesList;

    enum EParsingErrReportType {
        eErrorLineAndColumn,   ///< Report the line and column for the error (default)
        eErrorAbsolutePos      ///< Report the position of the error from the beginning of the script
    };

public:
    /// Constructor
    ///
    CMacroParser() : m_StrMacro(0), m_Scanner(0), m_MacroRep(nullptr), m_ErrorReportType(eErrorLineAndColumn)
        { x_InitParser(); }
    /// Destructor
    ///
    virtual ~CMacroParser() { if (!m_MacroRep) delete m_MacroRep; }

    /// initializes lists of known functions that should be recognized by the parser when 
    /// going through WHERE and DO clauses
    ///
    /// @param wh_funcs
    ///    functions that should be recognized in WHERE
    /// @param do_funcs
    ///    functions that should be recognized in DO
    void SetFunctionNames(const TFunctionNamesList& wh_funcs, const TFunctionNamesList& do_funcs);
    /// get list of where functions
    ///
    const TFunctionNamesList& GetWhereFunctions() const { return m_WhereFunctionsList; }
    /// get list of do functions
    ///
    const TFunctionNamesList& GetDoFunctions() const { return m_DoFunctionsList; }

    /// Reset the parser and sets macro text to be parsed
    ///
    void SetSource(const char* sMacroText)
        { m_StrMacro = sMacroText; x_InitParser(); m_Scanner.Next(); }

    /// Parse the macro and build its representation
    /// Function throws CMacroException if parsing is unsuccessful
    ///
    /// @param bParseSingleMacro
    ///   if it's true then function will throw an exception if anything but EOF 
    ///   exists after one parsed macro.
    /// @return
    ///   false if already at EOF and nothing was parsed
    bool Parse(bool bSingleMacroMode = true, CQueryExec* exec=NULL);

    /// Detach macro representation for futher processing
    /// Caller is responsible for deletion of returned object
    //!! maybe use CRef instead [?]
    CMacroRep* DetachMacroRep() { 
        CMacroRep* temp = m_MacroRep; 
        m_MacroRep = 0; 
        return temp;
    }

    /// Get the error location from the most recent parsing
    ///
    //CScanner::SLocation GetErrorLocation();
    /// Set the report type of error location
    ///
    void SetErrLocReportType(EParsingErrReportType type)  { m_ErrorReportType = type; }

private:
    /// string to be parsed
    ///
    const char* m_StrMacro; 
    /// scanner for extracting tokens and meta info
    ///
    CScanner m_Scanner;
    /// list of functions available for Where clause
    ///
    TFunctionNamesList m_WhereFunctionsList;
    /// list of functions available for Do clause
    ///
    TFunctionNamesList m_DoFunctionsList;
    /// list of run-time variables met in the left side of assignment in DO clause
    ///
    set<string> m_RTVars;
    /// binary representation of macro while parsing
    ///
    CMacroRep* m_MacroRep;

    // for error reporting:
    /// error location
    ///
    CScanner::SLocation m_ErrorLocation;
    /// Specifies the type of error reporting
    ///
    EParsingErrReportType m_ErrorReportType;

private:
    /// Resets parser to initial state. String to be parsed is not reset.
    ///
    void x_InitParser();
    /// Entry point to parse whole script.
    ///
    void x_ParseScript();
    /// Entry point to parse vars section.
    ///
    void x_ParseVariables();
    /// Entry point to parse choice section.
    ///
    void x_ParseChoice();
    /// Entry point to parse range
    ///
    void x_ParseRange();
    /// Entry point to parse number of threads to be used
    ///
    void x_ParseThreadCount();
    /// Entry point to parse body section.
    ///
    void x_ParseBody();
    /// Entry point to parse where clause.
    ///
    void x_ParseWhere();
    /// Entry point to parse where clause in function call.
    ///
    void x_ParseFunctionWhere();
    /// Entry point to parse DO-DONE section.
    ///
    void x_ParseDo();
    /// Entry point to parse function call.
    ///
    void x_ParseFunctionCall(CQueryParseTree::TNode *assignmentNode);

	void x_ParseMetaData();

    int x_ParseSign();
    /// Entry point to parse numeric var value.
    ///
    void x_ParseNumericVarValue(const string& var_name, int sign);
    /// Parses numeric value of a choice variable
    ///
    void x_ParseNumericChoiceValue(int sign);
    /// Parses numeric value of a function parameter
    ///
    void x_ParseNumericParameterValue(int sign, CScanner::SLocation start);
    /// Checks whether its parameter is a name of the registered functions
    ///
    bool x_IsFunction(const string& name) const;
    /// Extracts where clause as a string
    ///
    void x_GetWhereClause(Int4 istart, Int4 iend, string& result) const;
    /// Translates Where clause into a tree.
    /// User is responsible for deletion of its result.
    ///
    CQueryParseTree* x_QParseWhere(const string& str, unsigned line, unsigned linePos) const;
    /// Sorts the nodes according to a weight function for faster tree evaluation
    ///
    void x_SortParseTree(CQueryParseTree::TNode& node) const;
    /// Extracts Where clause string and builds Where clause tree;
    /// it uses terminator token to determine the end of Where clause.
    /// User is responsible for deletion of a resulting tree.
    ///
    void x_InternalParseWhere(const CScanner::EToken terminator, string& str, CQueryParseTree*& tree);

    void x_UpdateErrorLocation();

    string x_GetErrorLocation() const;

    string x_GetError();
private:
    /// Prohibit copy constructor and assignment operator
    CMacroParser(const CMacroParser&);
    CMacroParser& operator=(const CMacroParser&);
};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_PARSER__HPP
