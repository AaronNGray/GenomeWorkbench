#ifndef GUI_OBJUTILS___MACRO_EX__HPP
#define GUI_OBJUTILS___MACRO_EX__HPP

/*  $Id: macro_ex.hpp 44980 2020-05-01 13:48:03Z asztalos $
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
 * File Description: Macro language processing exception class
 *
 */

/// @file macro_ex.hpp
/// Macro exceptions

#include <sstream>

#include <corelib/ncbiexpt.hpp>
#include <util/qparse/query_parse.hpp>
#include <serial/serialbase.hpp>
#include <gui/gui_export.h>
/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

///////////////////////////////////////////////////////////////////////////////
/// Macro exception
///
class NCBI_GUIOBJUTILS_EXPORT CMacroParseException : EXCEPTION_VIRTUAL_BASE public CException {
public:
    // Error codes for parsing the macro script
    //
    enum EErrCode {
        eParseKeywordExpected,
        eParseAsnSelectorExpected,
        eParseNamedAnnotExpected,
        eParseMacroNameExpected,

        eParseVariableDefined,
        eParseVarDefinitionExpected,
        eParseUndefinedVariable,
        eParseIdentifierNameExpected,
        eParseWrongVariableUsage,
        eParseIntegerExpected,
        eParseRangeInvalid,
        eParseThreadCountInvalid,
        
        eParseAssignmentExpected,
        eParseAssignmentToConst,
        
        eParseLeftBraceExpected,
        eParseCommaOrRightBraceExpected,
        eParseLeftParenthesisExpected,
        eParseRightParenthesisExpected,
        eParseLeftBracketExpected,
        eParseRightBracketExpected,

        eParseWhereClauseExpected,
        eParseWhereClauseInvalid,
        eParseFunctionOrObjectNameExpected,
        eParseFunctionExpected,
        eParseFunctionOrDoneExpected,
        eParseEndOfFunction,

        eParseNothingExpected,
        eParseSemicolonExpected,
        eParseSeparatorExpected,
        eParseCommaExpected
    };

    // Convertion of error code to user domain message
    // 
    virtual const char* GetErrCodeString(void) const override {
        switch (GetErrCode()) {
        case eParseKeywordExpected:              return "eKeywordExpected";
        case eParseAsnSelectorExpected:          return "eSelectorExpected";

        case eParseMacroNameExpected:            return "eMacroNameExpected";

        case eParseVariableDefined:              return "eVarDefined";
        case eParseVarDefinitionExpected:        return "eVarDefinitionExpected";
        case eParseUndefinedVariable:            return "eUndefinedVar";
        case eParseIdentifierNameExpected:       return "eIdentifierNameExpected";
        case eParseWrongVariableUsage:           return "eWrongVariableUsage";

        case eParseAssignmentExpected:           return "eAssignmentExpected";
        case eParseAssignmentToConst:            return "eWrongVarAssignment";
        

        case eParseLeftBraceExpected:            return "eLeftBraceExpected";
        case eParseCommaOrRightBraceExpected:    return "eCommaOrRightBraceExpected";
        case eParseLeftParenthesisExpected:      return "eLeftParanthExpected";
        case eParseRightParenthesisExpected:     return "eRightParanthExpected";
        
        case eParseWhereClauseExpected:          return "eWhereSectionExpected";
        case eParseWhereClauseInvalid:           return "eInvalidWhereSection";
        
        case eParseFunctionOrObjectNameExpected: return "eFncOrObjectexpected";
        case eParseFunctionExpected:             return "eFncNameExpected";
        case eParseFunctionOrDoneExpected:       return "eFncOrDONEExpected";
        case eParseEndOfFunction:                return "eEndOfFunction";

        case eParseNothingExpected:              return "eNoTokenExpected";
        case eParseSemicolonExpected:            return "eSemicolonExpected";
        case eParseSeparatorExpected:            return "eSeparatorExpected";
            
        default: return CException::GetErrCodeString();
        }
    }

    NCBI_EXCEPTION_DEFAULT(CMacroParseException, CException);
};


///////////////////////////////////////////////////////////////////////////////
/// class CMacroExecException
///
/// Exception that carries information about the data, that caused the error in the form of CSerialObject 
class NCBI_GUIOBJUTILS_EXPORT CMacroDataException : EXCEPTION_VIRTUAL_BASE public CException
{
public:
    /// Constructor.
    CMacroDataException(const CDiagCompileInfo& info,
        const CException* prev_exception, EErrCode err_code,
        const string& message, CConstRef<CSerialObject> data,
        EDiagSev severity = eDiag_Error)
        : CException(info, prev_exception, message, severity),
        m_Data(data)
        NCBI_EXCEPTION_DEFAULT_IMPLEMENTATION(CMacroDataException, CException);

public:
    virtual void ReportExtra(ostream& out) const override
    {
        if (!m_Data)
            return;

        out << "Data context:" << endl;
        out << MSerial_AsnText << *(m_Data.GetPointer()) << endl;
    }

    /// Returns the ASN.1 data that caused the error
    CConstRef<CSerialObject> GetData() const throw() { return m_Data; }

    /// Returns the text encoded ASN.1 data that caused the error
    string GetDataAsString() const throw()
    {
        if (!m_Data)
            return "";

        stringstream strm;
        strm << MSerial_AsnText << *(m_Data.GetPointer()) << endl;
        return strm.str();
    }

protected:
    virtual void x_Assign(const CException& src) override
    {
        CException::x_Assign(src);
        m_Data = dynamic_cast<const CMacroDataException&>(src).m_Data;
    }

private:
    CConstRef<CSerialObject>    m_Data;
};


///////////////////////////////////////////////////////////////////////////////
/// class CMacroExecException
///
class NCBI_GUIOBJUTILS_EXPORT CMacroExecException : EXCEPTION_VIRTUAL_BASE public CException {
public:
    enum EErrCode {
        eInternalError,         // Internal error, usually indicates attempt to derefence 
                                // non-initialized pointer
        eWrongArguments,        // Wrong argument count or types
        eFloatNotSupported,     // Float is not yet supported
        eIncorrectIdentifier,   // identifier could not be resolved (i.e. it is undefined)
        eIncorrectNamedAnnot,   // NA number could not be resolved
        eFunctionNotImplemented,// function not implemented
        eWrongFunctionScope,    // function is used in the wrong do/where clause scope
        eInvalidQuery,          // a given query or part of query coming from qparser library is invalid
        eIncorrectNodeType,     // a given query node is incorrect, e.g. "abc"
        eVarNotResolved,        // Variable not resolved 
        eInvalidData,           // Indicates errors, caused by invalid data
        eFileNotFound           // File does not exist
    };

    CMacroExecException(const CDiagCompileInfo &info,
        const CException *prev_exception, EErrCode err_code,
        const string &message, unsigned lineNo = 0, unsigned columnNo = 0, EDiagSev severity = eDiag_Error)
        : CException(info, prev_exception, (CException::EErrCode) CException::eInvalid, (message)),
        m_LineNo(lineNo),
        m_ColumnNo(columnNo)
        NCBI_EXCEPTION_DEFAULT_IMPLEMENTATION(CMacroExecException, CException);

public:
    virtual const char* GetErrCodeString(void) const override
    {
        switch (GetErrCode())
        {
        case eInternalError:           return "Internal error";
        case eWrongArguments:          return "Wrong arguments";
        case eIncorrectIdentifier:     return "Incorrect identifier";
        case eFunctionNotImplemented:  return "Function was not implemented";
        case eWrongFunctionScope:      return "Wrong function scope";
        case eInvalidQuery:            return "Invalid query";
        case eIncorrectNodeType:       return "Incorrect node type";
        case eVarNotResolved:          return "Variable not resolved";
        case eInvalidData:             return "Invalid data";

        default: return "";
        }
    }

    /// Returns the line number where error occurred
    unsigned GetLineNo(void) const throw() { return m_LineNo; }

    /// Sets the line number where error occurred
    void SetLineNo(unsigned lineNo) throw() { m_LineNo = lineNo; }

    /// Returns the column number where error occurred
    unsigned GetColumnNo(void) const throw() { return m_ColumnNo; }
    /// Sets the column number where error occurred
    void SetColumnNo(unsigned columnNo) throw() { m_ColumnNo = columnNo; }

    virtual void ReportExtra(ostream& out) const override
    {
        out << "[Error] Line " << m_LineNo << ", Pos " << m_ColumnNo << endl;
    }

protected:
    virtual void x_Assign(const CException& src) override
    {
        CException::x_Assign(src);
        const CMacroExecException& src_macro = dynamic_cast<const CMacroExecException&>(src);
        m_LineNo = src_macro.m_LineNo;
        m_ColumnNo = src_macro.m_ColumnNo;
    }

protected:
    /// Line where the error occured
    unsigned m_LineNo;
    /// Column where the error occured
    unsigned m_ColumnNo;
};

/// Throws CMacroExecException with the specified message and error location from the TNode
inline void ThrowCMacroExecException(const CDiagCompileInfo &info, CMacroExecException::EErrCode code, const string &message, const CQueryParseTree::TNode *treeNode, const CException* previous = nullptr)
{
    unsigned lineNo(0);
    unsigned columnNo(0);
    if (treeNode) {
        const CQueryParseNode::SSrcLoc &loc = treeNode->GetValue().GetLoc();
        // SSrcLoc positions are 0 based
        lineNo = loc.line + 1;
        columnNo = loc.pos + 1;
    }
    throw CMacroExecException(info, previous, code, message, lineNo, columnNo);
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_EX__HPP
