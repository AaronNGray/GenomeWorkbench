/*  $Id: macro_parse.cpp 45086 2020-05-28 14:46:27Z asztalos $ 
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
 * File Description:  Macro parser
 *
 */

#include <ncbi_pch.hpp>
#include <util/qparse/parse_utils.hpp>
#include <gui/objutils/macro_parse.hpp>
#include <gui/objutils/macro_rep.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <functional>
#include <math.h>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

/////////////////////////////////////////////////////////////////////////////
//
//  CScanner
//

void CScanner::x_InitScanner()
{
    m_CurrentLexem.m_Token = eTokenILLEGAL;
    m_CurrentLexem.m_Position = SLocation(0, 1, 1);
    m_CurrentLexem.m_Data.Init();
}

CScanner::EToken CScanner::GetToken()
{
    return m_CurrentLexem.m_Token;
}

int CScanner::x_GetQuotedString()
{
    Int4& pos = m_CurrentLexem.m_Position.m_Pos;
    
    char ch = m_StrMacro[pos];
    m_CurrentLexem.m_Position.IncColumn();
    
    int i;
    for (i=0; (m_StrMacro[pos] != '\0') && 
              (m_StrMacro[pos] != ch)   && 
              (i < MAX_STRING_LEXEM_LENGTH); i++) {

        switch (m_StrMacro[pos]) {
        case '\n':
            m_CurrentLexem.m_Position.IncLine(); 
            break;
        case '\\':
            if (m_StrMacro[pos+1] == ch)
                m_CurrentLexem.m_Position.IncColumn();
            break;
        default:
            ; // intentionally no operators here
        }

        m_CurrentLexem.m_Data.m_Str[i] = m_StrMacro[pos];
        m_CurrentLexem.m_Position.IncColumn();
    }
            
    m_CurrentLexem.m_Data.m_Str[i] = '\0';
            
    if (i >= MAX_STRING_LEXEM_LENGTH) {
        m_CurrentLexem.m_ErrorMsg.assign("String is too long");
        return m_CurrentLexem.m_Token = eTokenILLEGAL;
    }

    if (m_StrMacro[pos] == '\0') {
        m_CurrentLexem.m_ErrorMsg.assign("String is not closed");
        return m_CurrentLexem.m_Token = eTokenILLEGAL;
    }
            
    m_CurrentLexem.m_Position.IncColumn(); // this eats " symbol
    return m_CurrentLexem.m_Token = (ch == '"') ? eTokenSTRING : eTokenASK;
}

void CScanner::x_InitForNextLexem()
{
    m_CurrentLexem.m_ErrorMsg.clear();
    m_CurrentLexem.m_Data.Init();

    Int4& pos = m_CurrentLexem.m_Position.m_Pos;
    if (m_StrMacro) {
        // Skip Delimeters
        while ((m_StrMacro[pos] == ' ')  || 
               (m_StrMacro[pos] == '\t') || 
               (m_StrMacro[pos] == '\n')) {
            
            if (m_StrMacro[pos] != '\n')
                m_CurrentLexem.m_Position.IncColumn();
            else    
                m_CurrentLexem.m_Position.IncLine();
        }
        m_CurrentLexem.m_Start = m_CurrentLexem.m_Position;
        m_CurrentLexem.m_ErrorLoc = m_CurrentLexem.m_Position;
    }
}

int CScanner::NextASNSelector()
{
    Int4& pos = m_CurrentLexem.m_Position.m_Pos;

    // It's to decide should we report a problem if:
    // 1) ** or .. is encountered
    // 2) dot is the first or last symbol in selector
    
    if (!m_StrMacro)
        return m_CurrentLexem.m_Token = eTokenILLEGAL;

    x_InitForNextLexem();
    if (isalpha(m_StrMacro[pos]) || (m_StrMacro[pos]=='*')) {
        int i;
        for ( i = 0; (  isdigit(m_StrMacro[pos]) || 
                        isalpha(m_StrMacro[pos]) || 
                        (m_StrMacro[pos] == '_') || 
                        (m_StrMacro[pos] == '-') || 
                        (m_StrMacro[pos] =='*')  || 
                        (m_StrMacro[pos] =='.')
                     ) && (i < MAX_STRING_LEXEM_LENGTH); i++) {

            m_CurrentLexem.m_Data.m_Str[i] = m_StrMacro[pos];
            m_CurrentLexem.m_Position.IncColumn();
        }
        m_CurrentLexem.m_Data.m_Str[i] = '\0';

        if (i >= MAX_STRING_LEXEM_LENGTH) {
            m_CurrentLexem.m_ErrorMsg.assign("String is too long");
            return m_CurrentLexem.m_Token = eTokenILLEGAL;
        }
        else {
            if (NStr::EqualNocase(m_CurrentLexem.m_Data.m_Str, "where")
                || NStr::EqualNocase(m_CurrentLexem.m_Data.m_Str, "do")) {
                return m_CurrentLexem.m_Token = eTokenILLEGAL;
            }
        }
        return m_CurrentLexem.m_Token = eTokenASNSELECTOR;
    }
    else {
        m_CurrentLexem.m_ErrorMsg.assign("ASN.1 selector expected");
        return m_CurrentLexem.m_Token = eTokenILLEGAL;
    }
}

int CScanner::NextNamedAnnot()
{
    Int4& pos = m_CurrentLexem.m_Position.m_Pos;

    if (!m_StrMacro)
        return m_CurrentLexem.m_Token = eTokenILLEGAL;

    x_InitForNextLexem();
    if (m_StrMacro[pos] == 'N' && m_StrMacro[pos + 1] == 'A') {
        size_t i = 0;
        for (i; (  isdigit(m_StrMacro[pos]) || 
                   isalpha(m_StrMacro[pos]) || 
                   (m_StrMacro[pos] == '.') ||
                   (m_StrMacro[pos] == '#')
                 ) && (i < MAX_STRING_LEXEM_LENGTH); i++) {
            m_CurrentLexem.m_Data.m_Str[i] = m_StrMacro[pos];
            m_CurrentLexem.m_Position.IncColumn();
        }
        m_CurrentLexem.m_Data.m_Str[i] = '\0';

        if (i >= MAX_STRING_LEXEM_LENGTH) {
            m_CurrentLexem.m_ErrorMsg.assign("String is too long");
            return m_CurrentLexem.m_Token = eTokenILLEGAL;
        }
        else {
            if (NStr::EqualNocase(m_CurrentLexem.m_Data.m_Str, "where")
                || NStr::EqualNocase(m_CurrentLexem.m_Data.m_Str, "do")) {
                return m_CurrentLexem.m_Token = eTokenILLEGAL;
            }
        }
        return m_CurrentLexem.m_Token = eTokenNAMEDANNOT;
    }

    m_CurrentLexem.m_ErrorMsg.assign("Named annotation (NA number) expected");
    return m_CurrentLexem.m_Token = eTokenILLEGAL;
}

int CScanner::x_SetCurrentLexem(Int4& pos, CScanner::EToken token)
{
    char ch = tolower(m_StrMacro[pos]);
    strncat(m_CurrentLexem.m_Data.m_Str, &ch, 1);
    m_CurrentLexem.m_Position.IncColumn();
    m_CurrentLexem.m_Token = token;
    return m_CurrentLexem.m_Token;
}

int CScanner::Next(bool parseComment)
{
    Int4& pos = m_CurrentLexem.m_Position.m_Pos;

    if (!m_StrMacro)
        return m_CurrentLexem.m_Token = eTokenILLEGAL;

    x_InitForNextLexem();
    switch (tolower(m_StrMacro[pos])) {
    case '\0':
        // We reached the end of string.
        // Do not increment position.
        return m_CurrentLexem.m_Token = eTokenEOS;
    case '=':
        return x_SetCurrentLexem(pos, eTokenASSIGNMENT);
    case '(':
        return x_SetCurrentLexem(pos, eTokenLEFTPAREN);
    case ')':
        return x_SetCurrentLexem(pos, eTokenRIGHTPAREN);
    case '{':
        return x_SetCurrentLexem(pos, eTokenLEFTBRACE);
    case '}':
        return x_SetCurrentLexem(pos, eTokenRIGHTBRACE);
    case '[':
        return x_SetCurrentLexem(pos, eTokenLEFTBRACKET);
    case ']':
        return x_SetCurrentLexem(pos, eTokenRIGHTBRACKET);
    case ',':
        return x_SetCurrentLexem(pos, eTokenCOMMA);
    case ';':
        return x_SetCurrentLexem(pos, eTokenSEMICOLON);

    case '-': // unary - in front of a number or start of a comment
        m_CurrentLexem.m_Position.IncColumn();
        if (m_StrMacro[pos] != '-')
            return m_CurrentLexem.m_Token = eTokenMINUS;

		m_CurrentLexem.m_Position.IncColumn();

		if (parseComment) {
			int i = 0;
			for (; i < MAX_STRING_LEXEM_LENGTH; i++) {
				if (m_StrMacro[pos] == '\0' || m_StrMacro[pos] == '\n')
					break;

				m_CurrentLexem.m_Data.m_Str[i] = m_StrMacro[pos];
				m_CurrentLexem.m_Position.IncColumn();
			}
			m_CurrentLexem.m_Data.m_Str[i] = '\0';

			while ((m_StrMacro[pos] != '\0') && (m_StrMacro[pos] != '\n'))
				m_CurrentLexem.m_Position.IncColumn();

			return m_CurrentLexem.m_Token = eTokenCOMMENT;
		}

        // it is a comment, skip it till the end of line.
        while ((m_StrMacro[pos] != '\0')  && (m_StrMacro[pos] != '\n'))
            m_CurrentLexem.m_Position.IncColumn();
        return Next();

    case '+': // unary + in front of a number
        return x_SetCurrentLexem(pos, eTokenPLUS);
    case '.': // FOR WHERE CLAUSE
        return x_SetCurrentLexem(pos, eTokenDOT);
    case '*': // FOR WHERE CLAUSE
        return x_SetCurrentLexem(pos, eTokenMULT);
    case '/': // FOR WHERE CLAUSE
        return x_SetCurrentLexem(pos, eTokenDIV);

    case '>': // FOR WHERE CLAUSE
        m_CurrentLexem.m_Position.IncColumn();
        if (m_StrMacro[pos] != '=')
            return m_CurrentLexem.m_Token = eTokenGREATER;
            
        m_CurrentLexem.m_Position.IncColumn();
        return m_CurrentLexem.m_Token = eTokenNOTLESS;
        
    case '<': // FOR WHERE CLAUSE
        m_CurrentLexem.m_Position.IncColumn();
        if ((m_StrMacro[pos] != '>') && (m_StrMacro[pos] != '='))
            return m_CurrentLexem.m_Token = eTokenLESS;

        m_CurrentLexem.m_Position.IncColumn();
        if (m_StrMacro[pos] == '=')
            return m_CurrentLexem.m_Token = eTokenNOTGREATER;
        return m_CurrentLexem.m_Token = eTokenNOTEQUAL;

    case '0': // can numbers start with 0?
    case '1':case '2':case '3':
    case '4':case '5':case '6':
    case '7':case '8':case '9': 
    {
        Int4 num = m_StrMacro[pos] - '0';
        m_CurrentLexem.m_Position.IncColumn();

        while (isdigit(m_StrMacro[pos])) {
            num = num * 10 + (m_StrMacro[pos] - '0');
            m_CurrentLexem.m_Position.IncColumn();
        }
        
        if (m_StrMacro[pos] == '.') {
            m_CurrentLexem.m_Position.IncColumn();
            double fl_num = (double)num;
            short index = -1;
            while (isdigit(m_StrMacro[pos])) {
                fl_num += (m_StrMacro[pos] - '0')*pow(10.0, index); 
                index--;
                m_CurrentLexem.m_Position.IncColumn();
    }
            m_CurrentLexem.m_Data.m_FloatNumber = fl_num;
            return m_CurrentLexem.m_Token = eTokenFLOATNUMBER;
        }
        else {
            m_CurrentLexem.m_Data.m_IntNumber = num;
            return m_CurrentLexem.m_Token = eTokenINTNUMBER;
        }
    }
    case '"':
    case '%':
        return x_GetQuotedString();
    default: 
        if (isalpha(m_StrMacro[pos])) {
            int i;
            // * and . are included for asn selectors
            for ( i = 0; (  isdigit(m_StrMacro[pos]) || 
                            isalpha(m_StrMacro[pos]) || 
                            // .
                            // -
                            (m_StrMacro[pos] == '_')
                         ) && (i < MAX_STRING_LEXEM_LENGTH); i++) {

                m_CurrentLexem.m_Data.m_Str[i] = m_StrMacro[pos];
                m_CurrentLexem.m_Position.IncColumn();
            }
            m_CurrentLexem.m_Data.m_Str[i] = '\0';

            if (i >= MAX_STRING_LEXEM_LENGTH) {
                m_CurrentLexem.m_ErrorMsg.assign("Identifier is too long");
                return m_CurrentLexem.m_Token = eTokenILLEGAL;
            }

            CTempString keyword(m_CurrentLexem.m_Data.m_Str);
            // Look up keyword names
            if (NStr::EqualNocase(keyword, "macro")) {
                return m_CurrentLexem.m_Token = eTokenKWMACRO;
            } else if (NStr::EqualNocase(keyword, "var")) {
                return m_CurrentLexem.m_Token = eTokenKWVARS;
            } else if (NStr::EqualNocase(keyword, "vars")) {
                return m_CurrentLexem.m_Token = eTokenKWVARS;
            } else if (NStr::EqualNocase(keyword, "for")) {
                return m_CurrentLexem.m_Token = eTokenKWFOR;
            } else if (NStr::EqualNocase(keyword, "each")) {
                return m_CurrentLexem.m_Token = eTokenKWEACH;
            } else if (NStr::EqualNocase(keyword, "from")) {
                return m_CurrentLexem.m_Token = eTokenKWFROM;
            } else if (NStr::EqualNocase(keyword, "choice")) {
                return m_CurrentLexem.m_Token = eTokenKWCHOICE;
            } else if (NStr::EqualNocase(keyword, "range")) {
                return m_CurrentLexem.m_Token = eTokenKWRANGE;
            } else if (NStr::EqualNocase(keyword, "where")) {
                return m_CurrentLexem.m_Token = eTokenKWWHERE;
            } else if (NStr::EqualNocase(keyword, "do")) {
                return m_CurrentLexem.m_Token = eTokenKWDO;
            } else if (NStr::EqualNocase(keyword, "done")) {
                return m_CurrentLexem.m_Token = eTokenKWDONE;
            } else if (NStr::EqualNocase(keyword, "do_p")) {
                return m_CurrentLexem.m_Token = eTokenKWDOPARL;
            } else if (NStr::EqualNocase(keyword, "true")) {
                m_CurrentLexem.m_Data.m_BoolValue = true;
                return m_CurrentLexem.m_Token = eTokenBOOLCONST;
            } else if (NStr::EqualNocase(keyword, "false")) {
                m_CurrentLexem.m_Data.m_BoolValue = false;
                return m_CurrentLexem.m_Token = eTokenBOOLCONST;
            } else {
                /* TEMPORARY DISABLE THIS 
                // Search for built-in function names
                for (i = 0; m_FuncNames[i] != NULL; i++) {
                    if (!NStr::CompareNocase(m_CurrentLexem.m_Data.m_Str, m_FuncNames[i]))
                    break;
                }
                if (m_FuncNames[i] != NULL)
                    return m_CurrentLexem.m_Token = eTokenFUNCTION;
                */
                return m_CurrentLexem.m_Token = eTokenIDENT;
            }
        }
        else {
            m_CurrentLexem.m_ErrorMsg.assign("Symbol is not allowed");
            m_CurrentLexem.m_ErrorLoc = m_CurrentLexem.m_Position;
            return m_CurrentLexem.m_Token = eTokenILLEGAL;
        }
    }
}



/////////////////////////////////////////////////////////////////////////////
//
//  CMacroParser
//
bool CMacroParser::Parse(bool bSingleMacroMode, CQueryExec* exec)
{
    m_RTVars.clear();

    if (m_MacroRep) {
        delete m_MacroRep;
    }
    m_MacroRep = new CMacroRep(exec);

    if ( m_Scanner.GetToken() == CScanner::eTokenEOS ) {
        delete m_MacroRep; 
        m_MacroRep = 0;
        return false;
    }

    Int4 pos_start = m_Scanner.GetLexemInfo().m_Start.m_Pos;

    x_ParseScript();

    Int4 pos_end = m_Scanner.GetLexemInfo().m_Start.m_Pos;
    m_MacroRep->SetSource( string (m_StrMacro + pos_start, pos_end - pos_start) );

    if (bSingleMacroMode) {
        if (m_Scanner.GetToken() != CScanner::eTokenEOS) {
            string msg = x_GetError();
            msg += "Unexpected token: '" + m_Scanner.GetLexemInfo().GetStringRepr() + "'";
            NCBI_THROW(CMacroParseException, eParseNothingExpected, msg);
        }
    }
    return true;
}

string CMacroParser::x_GetErrorLocation() const
{
    _ASSERT(m_ErrorLocation.m_Pos != -1);

    string msg = "[Parsing error] ";
    if (m_ErrorReportType == eErrorLineAndColumn) {
        msg += "Line ";
        msg += NStr::IntToString(m_ErrorLocation.m_Line);
        msg += ", Pos ";
        msg += NStr::IntToString(m_ErrorLocation.m_Column);
        msg += ": ";
    }
    else {
        msg += "Pos ";
        msg += NStr::IntToString(m_ErrorLocation.m_Pos + 1);
        msg += ": ";
    }

    return msg;
}


void CMacroParser::x_UpdateErrorLocation()
{
    if (m_ErrorLocation.m_Pos == -1) {
        m_ErrorLocation = m_Scanner.GetErrorLocation();
    }
}

string CMacroParser::x_GetError()
{
    x_UpdateErrorLocation();
    return x_GetErrorLocation();
}

// Inits parser to its initial state
void CMacroParser::x_InitParser()
{
    m_Scanner.Reset(m_StrMacro);
    m_ErrorLocation.Reset();
}

void CMacroParser::x_ParseScript()
{
    // Macro heading section
    if (m_Scanner.GetToken() != CScanner::eTokenKWMACRO) {
        string msg = x_GetError();
        msg += "Keyword 'MACRO' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseKeywordExpected, msg);
    }
    
    m_Scanner.Next();
    if (m_Scanner.GetToken() != CScanner::eTokenIDENT) {
        string msg = x_GetError();
        msg += "Macro name expected";
        NCBI_THROW(CMacroParseException, eParseMacroNameExpected, msg);
    }
    else {
        m_MacroRep->SetName(m_Scanner.GetLexemInfo().m_Data.m_Str);
    }

    m_Scanner.Next(true);
    // description of the macro is optional
    if (m_Scanner.GetToken() == CScanner::eTokenSTRING) {
        m_MacroRep->SetTitle(m_Scanner.GetLexemInfo().m_Data.m_Str);
        m_Scanner.Next(true);
    }

	while (m_Scanner.GetToken() == CScanner::eTokenCOMMENT) {
		x_ParseMetaData();
		m_Scanner.Next(true);
	}

	//LOG_POST(Error << m_MacroRep->GetMetaKeywords());

    // Vars section(s) is optional
    while (m_Scanner.GetToken() == CScanner::eTokenKWVARS) {
        m_Scanner.Next();
        x_ParseVariables();
    }

/*
    // to identify which macro had a parsing error
    LOG_POST("Parsing: " << m_MacroRep->GetName());
*/
    // Macro body section
    x_ParseBody();
};

void CMacroParser::x_ParseVariables()
{
    m_MacroRep->SetNewVarBlock();

    if (m_Scanner.GetToken() != CScanner::eTokenIDENT) {
        string msg = x_GetError();
        msg += "Identifier name expected";
        NCBI_THROW(CMacroParseException, eParseIdentifierNameExpected, msg);
    }

    do {
        // Store Variable Name
        string var_name = m_Scanner.GetLexemInfo().m_Data.m_Str;
        if (m_MacroRep->FindVar(var_name)) {
            string msg = x_GetError();
            msg += "Variable '" + m_Scanner.GetLexemInfo().GetStringRepr();
            msg += "' is already defined";
            NCBI_THROW(CMacroParseException, eParseVariableDefined, msg);
        }

        m_Scanner.Next();
        if (m_Scanner.GetToken() != CScanner::eTokenASSIGNMENT) {
            string msg = x_GetError();
            msg += "Assignment expected";
            NCBI_THROW(CMacroParseException, eParseAssignmentExpected, msg);
        }

        m_Scanner.Next();
        switch (m_Scanner.GetToken()) {
        case CScanner::eTokenSTRING:            
            // Store variable's value as a string
            m_MacroRep->SetVarString(var_name, m_Scanner.GetLexemInfo().m_Data.m_Str);
            m_Scanner.Next();
            break;
        case CScanner::eTokenASK:
            // Store ASK's value as a string
            m_MacroRep->SetVarAsk(var_name, m_Scanner.GetLexemInfo().m_Data.m_Str);
            m_Scanner.Next();
            break;
        case CScanner::eTokenKWCHOICE:
            // Store variables's name
            m_MacroRep->SetVarChoiceName(var_name);
            m_Scanner.Next();
            x_ParseChoice();
            break;
        case CScanner::eTokenBOOLCONST:
            // Store bool const value
            m_MacroRep->SetVarBool(var_name, m_Scanner.GetLexemInfo().m_Data.m_BoolValue);
            m_Scanner.Next();
            break;
        default:
            {
                x_ParseNumericVarValue(var_name, x_ParseSign());
            }
        }
    }
    while (m_Scanner.GetToken() == CScanner::eTokenIDENT);
}

int CMacroParser::x_ParseSign()
{
    int sign = 1;
    if (m_Scanner.GetToken() == CScanner::eTokenMINUS) {
        sign = -1;
    }
    if ((m_Scanner.GetToken() == CScanner::eTokenMINUS) ||
        (m_Scanner.GetToken() == CScanner::eTokenPLUS)) {

        m_Scanner.Next();
    }
    return sign;
}

void CMacroParser::x_ParseNumericVarValue(const string& var_name, int sign)
{
    switch (m_Scanner.GetToken()) {
    case CScanner::eTokenINTNUMBER:
    {
        Int4 value = sign * m_Scanner.GetLexemInfo().m_Data.m_IntNumber;
        m_MacroRep->SetVarInt(var_name, value);
        break;
    }
    case CScanner::eTokenFLOATNUMBER:
    {
        double value = sign * m_Scanner.GetLexemInfo().m_Data.m_FloatNumber;
        m_MacroRep->SetVarFloat(var_name, value);
        break;
    }
    default: {
        string msg = x_GetError();
        msg += "Definition of '" + var_name + "' variable is incomplete";
        NCBI_THROW(CMacroParseException, eParseVarDefinitionExpected, msg);
    }
    }
    
    m_Scanner.Next();
}

void CMacroParser::x_ParseChoice()
{
    if (m_Scanner.GetToken() != CScanner::eTokenLEFTBRACE) {
        string msg = x_GetError();
        msg += "Symbol '{' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseLeftBraceExpected, msg);
    }

    do {
        // Parse var value
        m_Scanner.Next();

        switch (m_Scanner.GetToken()) {
        case CScanner::eTokenSTRING:
            // Store choice variable's value as a string
            m_MacroRep->SetVarChoiceString(m_Scanner.GetLexemInfo().m_Data.m_Str);
            m_Scanner.Next();
            break;
        case CScanner::eTokenBOOLCONST:
            m_MacroRep->SetVarChoiceBool(m_Scanner.GetLexemInfo().m_Data.m_BoolValue);
            m_Scanner.Next();
            break;
        default:
            {
                x_ParseNumericChoiceValue(x_ParseSign());
            }
        }
    }
    while (m_Scanner.GetToken() == CScanner::eTokenCOMMA);
    
    if (m_Scanner.GetToken() != CScanner::eTokenRIGHTBRACE) {
        string msg = x_GetError();
        msg += "Comma or '}' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseCommaOrRightBraceExpected, msg);
    }
    
    m_Scanner.Next();
}

void CMacroParser::x_ParseNumericChoiceValue(int sign)
{
    switch (m_Scanner.GetToken()) {
    case CScanner::eTokenINTNUMBER:
    {
        Int8 value = sign * m_Scanner.GetLexemInfo().m_Data.m_IntNumber;
        m_MacroRep->SetVarChoiceInt(value);
        break;
    }
    case CScanner::eTokenFLOATNUMBER:
    {
        double value = sign * m_Scanner.GetLexemInfo().m_Data.m_FloatNumber;
        m_MacroRep->SetVarChoiceFloat(value);
        break;
    }
    default: {
        string msg = x_GetError();
        msg += "Variable definition expected";
        NCBI_THROW(CMacroParseException, eParseVarDefinitionExpected, msg);
    }
    }

    m_Scanner.Next();
}

void CMacroParser::x_ParseBody()
{
    if (m_Scanner.GetToken() == CScanner::eTokenKWFOR) {
        m_Scanner.Next();
        if (m_Scanner.GetToken() != CScanner::eTokenKWEACH) {
            string msg = x_GetError();
            msg += "Keyword 'FOR EACH' expected instead of '";
            msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
            NCBI_THROW(CMacroParseException, eParseKeywordExpected, msg);
        }

        m_Scanner.NextASNSelector();
        if (m_Scanner.GetToken() != CScanner::eTokenASNSELECTOR) {
            string msg = x_GetError();
            msg += "ASN.1 selector expected instead of '";
            msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
            NCBI_THROW(CMacroParseException, eParseAsnSelectorExpected, msg);
        }

        // Store ASN selector
        m_MacroRep->SetForEachItem(m_Scanner.GetLexemInfo().m_Data.m_Str);
        m_Scanner.Next();
    }

    // Code below commented out, because only one ASN selector is allowed
    //
    // while ( m_Scanner.GetToken() == CScanner::eTokenCOMMA ) {
    //    m_Scanner.NextASNSelector();
    //    if ( m_Scanner.GetToken() != CScanner::eTokenASNSELECTOR )
    //        NCBI_THROW(CMacroParseException, EErCode_ASN_SELECTOR_EXPECTED, "Macro Exception");
    //
    //   m_Scanner.Next();
    // }

    if (m_Scanner.GetToken() == CScanner::eTokenKWFROM) {
        m_Scanner.NextNamedAnnot();
        if (m_Scanner.GetToken() != CScanner::eTokenNAMEDANNOT) {
            string msg = x_GetError();
            msg += "Named annotation expected instead of '";
            msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
            NCBI_THROW(CMacroParseException, eParseNamedAnnotExpected, msg);
        }
        
        m_MacroRep->SetNamedAnnot(m_Scanner.GetLexemInfo().m_Data.m_Str);
        m_Scanner.Next();
    }

    if (m_Scanner.GetToken() == CScanner::eTokenKWRANGE) {
        x_ParseRange();
    }

    if ( m_Scanner.GetToken() == CScanner::eTokenKWWHERE )
        x_ParseWhere();

    x_ParseDo();
}

void CMacroParser::x_ParseRange()
{
    if (m_Scanner.GetToken() != CScanner::eTokenKWRANGE) {
        string msg = x_GetError();
        msg += "Keyword 'RANGE' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseKeywordExpected, msg);
    }

    m_Scanner.Next();
    auto token = m_Scanner.GetToken();
    if ((token == CScanner::eTokenKWWHERE) ||
        (token == CScanner::eTokenKWDO) ||
        (token == CScanner::eTokenKWDOPARL) ||
        (token == CScanner::eTokenILLEGAL) ||
        (token == CScanner::eTokenEOS)) {
        string msg = x_GetError();
        msg += "Sequence range expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseWhereClauseExpected, msg);
    }

    if (m_Scanner.GetToken() != CScanner::eTokenLEFTBRACKET) {
        string msg = x_GetError();
        msg += "Symbol '[' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseLeftBracketExpected, msg);
    }

    Int8 start = 0, stop = 0;
    bool found_start = false, found_stop = false, found_comma = false;
    int sign = 1;

    m_Scanner.Next();
    token = m_Scanner.GetToken();
    while ((token != CScanner::eTokenRIGHTBRACKET) &&
        (token != CScanner::eTokenKWDO) &&
        (token != CScanner::eTokenKWWHERE) &&
        (token != CScanner::eTokenILLEGAL) &&
        (token != CScanner::eTokenEOS)) {

        switch (token) {
        case CScanner::eTokenINTNUMBER:
            if (!found_start && !found_comma) {
                found_start = true;
                start = sign * m_Scanner.GetLexemInfo().m_Data.m_IntNumber;
            }
            else if (found_comma && !found_stop) {
                found_stop = true;
                stop = sign * m_Scanner.GetLexemInfo().m_Data.m_IntNumber;
            }
            break;
        case CScanner::eTokenCOMMA:
            found_comma = true;
            break;
        case CScanner::eTokenFLOATNUMBER:
        case CScanner::eTokenSTRING:
        case CScanner::eTokenBOOLCONST:
        {
            string msg = x_GetError();
            msg += "Positive integer number expected instead of '";
            msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
            NCBI_THROW(CMacroParseException, eParseIntegerExpected, msg);
            break;
        }
        case CScanner::eTokenSEMICOLON:
        case CScanner::eTokenDOT:
        {
            string msg = x_GetError();
            msg += "Comma expected to separate two positive integers instead of '";
            msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
            NCBI_THROW(CMacroParseException, eParseCommaExpected, msg);
            break;
        }
        case CScanner::eTokenRIGHTBRACE:
        case CScanner::eTokenRIGHTPAREN:
        {
            string msg = x_GetError();
            msg += "Symbol ']' expected instead of '";
            msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
            NCBI_THROW(CMacroParseException, eParseRightBracketExpected, msg);
            break;
        }
        default:
            break;
        }
        m_Scanner.Next();
        sign = x_ParseSign();
        token = m_Scanner.GetToken();
    } 

    if (token != CScanner::eTokenRIGHTBRACKET || !found_start || !found_stop) {
        string msg = "Range was not specified as expected: '[start, stop]'";
        NCBI_THROW(CMacroParseException, eParseRangeInvalid, msg);
    }

    if (start < 0 || stop < 0) {
        string msg = "Start and Stop positions are expected to be positive integers";
        NCBI_THROW(CMacroParseException, eParseRangeInvalid, msg);
    }

    if (start > stop) {
        string msg = "Start position should be less than or equal to the Stop position";
        NCBI_THROW(CMacroParseException, eParseRangeInvalid, msg);
    }

    m_MacroRep->SetSeqRange(TSeqRange((unsigned)start, (unsigned)stop));
    m_Scanner.Next();
}

void CMacroParser::x_GetWhereClause(Int4 istart, Int4 iend, string& result) const
{
    int len = static_cast<int>(iend - istart);
    if ( len > 0 ) {
        int i = 0;
        result.resize(len + 1);
        while (i<len)
            result[i++] = m_StrMacro[istart++];
        result[len] = '\0';
    } else {
        result.clear();
    }
}

namespace {
    size_t s_NodeCost(const CQueryParseTree::TNode node)
    {
        switch (node->GetType()){
        case CQueryParseNode::eEQ:
        {
            CQueryParseTree::TNode::TNodeList_CI iter = node.SubNodeBegin();
            if ((*iter)->GetValue().GetType() == CQueryParseNode::eFunction) {
                const string& name = (*iter)->GetValue().GetOrig();
                if (NStr::StartsWith(name, "Sequence_for", NStr::eNocase)) {
                    return 100;
                }
            }
            return 1;
        }
        case CQueryParseNode::eIn:
        {
            CQueryParseTree::TNode::TNodeList_CI iter = node.SubNodeBegin();
            if ((*iter)->GetValue().GetType() == CQueryParseNode::eFunction) {
                const string& name = (*iter)->GetValue().GetOrig();
                if (NStr::StartsWith(name, "Sequence_for", NStr::eNocase)) {
                    return 100;
                }
            }
            return 5;
        }
        default:
            return 10;
        }
    }

    struct SNode_Less
    {
        bool operator() (const CQueryParseTree::TNode* item1, const CQueryParseTree::TNode* item2) const
        {
            return s_NodeCost(*item1) < s_NodeCost(*item2);
        }
    };
} // end of namespace

void CMacroParser::x_SortParseTree(CQueryParseTree::TNode& node) const
{
    switch (node->GetType()) {
    case CQueryParseNode::eAnd:
    case CQueryParseNode::eOr:
    {{
            vector<CQueryParseTree::TNode*> nodes;
            if (node.CountNodes() > 1) {
                CQueryParseTree::TNode::TNodeList_I iter = node.SubNodeBegin();
                for (; iter != node.SubNodeEnd();) {
                    CQueryParseTree::TNode* ptr = node.DetachNode(iter++);
                    nodes.push_back(ptr);
                }

                std::function<bool(const CQueryParseTree::TNode*, const CQueryParseTree::TNode*)> sorter = SNode_Less();
                sort(nodes.begin(), nodes.end(), sorter);
                ITERATE(vector<CQueryParseTree::TNode*>, iter, nodes) {
                    node.AddNode(*iter);
                }
            }
        }}
    default:
        break;
    }

    CQueryParseTree::TNode::TNodeList_I iter = node.SubNodeBegin();
    for (; iter != node.SubNodeEnd(); ++iter) {
        x_SortParseTree(**iter);
    }
}

CQueryParseTree* CMacroParser::x_QParseWhere(const string& str, unsigned line, unsigned linePos) const
{       
    auto_ptr<CQueryParseTree> ap_qparse_tree(new CQueryParseTree());
    ap_qparse_tree->Parse(str.c_str(), 
                            CQueryParseTree::eCaseInsensitive,
                            CQueryParseTree::eSyntaxCheck,
                            false,
                            m_WhereFunctionsList,
                            line,
                            linePos
                           );

    Flatten_ParseTree(*ap_qparse_tree->GetQueryTree());

    x_SortParseTree(*ap_qparse_tree->GetQueryTree());

    CQueryParseTree* p_where_tree = ap_qparse_tree.release();
    ap_qparse_tree.reset(NULL);
    return p_where_tree;
}

void CMacroParser::x_InternalParseWhere(const CScanner::EToken terminator, string& str, CQueryParseTree*& tree)
{
    tree = 0;
    str.empty();

    vector<CScanner::EToken> end_tokens{ CScanner::eTokenILLEGAL, CScanner::eTokenEOS, terminator };
    if (terminator == CScanner::eTokenKWDO) {
        end_tokens.push_back(CScanner::eTokenKWDOPARL);
    }

    if (m_Scanner.GetToken() != CScanner::eTokenKWWHERE) {
        string msg = x_GetError();
        msg += "Keyword 'WHERE' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseKeywordExpected, msg);
    }
    
    m_Scanner.Next();
    if (find(end_tokens.begin(), end_tokens.end(), m_Scanner.GetToken()) != end_tokens.end()) {
        string msg = x_GetError();
        msg += "'WHERE' clause expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseWhereClauseExpected, msg);
    }

    // store the potential error location
    CScanner::SLocation er_loc(m_Scanner.GetLexemInfo().m_Start); 
    const CScanner::SLocation whereStart = m_Scanner.GetLexemInfo().m_Start;

    Int4 istart = m_Scanner.GetLexemInfo().m_Start.m_Pos;
    while (find(end_tokens.begin(), end_tokens.end(), m_Scanner.GetToken()) == end_tokens.end()) {
        m_Scanner.Next();
    }
    Int4 iend = m_Scanner.GetLexemInfo().m_Start.m_Pos;
    
    x_GetWhereClause(istart , iend, str);

    if(!str.empty()) {
        try {
            tree = x_QParseWhere(str, whereStart.m_Line - 1, whereStart.m_Column - 1);
        } 
        catch(CQueryParseException&)
        {
            m_ErrorLocation = er_loc;
            string msg = x_GetErrorLocation();
            msg += "Invalid 'WHERE' clause";
            NCBI_THROW(CMacroParseException, eParseWhereClauseInvalid, msg);
        }
    } else {
        string msg = x_GetError();
        msg += "Invalid 'WHERE' clause: ";
        msg += m_Scanner.GetLexemInfo().GetStringRepr();
        NCBI_THROW(CMacroParseException, eParseWhereClauseInvalid, msg);
    }
}

void CMacroParser::x_ParseWhere()
{
    string where_clause;
    CQueryParseTree* where_tree;

    x_InternalParseWhere(CScanner::eTokenKWDO,
                           where_clause, 
                           where_tree);

    //where_tree->Print(NcbiCout);
    m_MacroRep->AttachWhereTree(where_tree);
}

void CMacroParser::x_ParseFunctionWhere()
{
    string where_clause;
    CQueryParseTree* where_tree;

    x_InternalParseWhere(CScanner::eTokenSEMICOLON,
                           where_clause, 
                           where_tree);

    //where_tree->Print(NcbiCout);
    m_MacroRep->SetAssignmentWhereClause(where_clause, where_tree, m_Scanner.GetLexemInfo().m_Start);
}

void CMacroParser::x_ParseDo()
{
    auto token = m_Scanner.GetToken();
    if (token != CScanner::eTokenKWDO && 
        token != CScanner::eTokenKWDOPARL) {
        string msg = x_GetError();
        msg += "Keyword 'DO' or keyword 'DO_P' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseKeywordExpected, msg);
    }

    if (token == CScanner::eTokenKWDOPARL) {
        x_ParseThreadCount();
    }

    CQueryParseTree *doTree = m_MacroRep->GetDoTree();
    _ASSERT(doTree);
    CQueryParseTree::TNode *doNode = doTree->GetQueryTree();
    _ASSERT(doNode);
    doNode->GetValue().SetLoc(m_Scanner.GetLexemInfo().m_Start);

    m_Scanner.Next();
    do {
        CQueryParseTree::TNode* assignmentNode = nullptr;
        if (m_Scanner.GetToken() != CScanner::eTokenIDENT) {
            string msg = x_GetError();
            msg += "Function or object name is expected instead of '";
            msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
            NCBI_THROW(CMacroParseException, eParseFunctionOrObjectNameExpected, msg);
        }

        string ident = m_Scanner.GetLexemInfo().m_Data.m_Str;
        if ( !x_IsFunction(ident) ) {

            m_ErrorLocation = m_Scanner.GetLexemInfo().m_Start;

            if (m_MacroRep->FindVar(ident)) {
                string msg = x_GetError();
                msg += "Assignment to non-modifiable variable '";
                msg += ident + "' in this context";
                NCBI_THROW(CMacroParseException, eParseAssignmentToConst, msg);
            }

            // Create the variable node
            auto_ptr<CQueryParseTree::TNode> aptrVar (doTree->CreateNode(CQueryParseNode::eSelect, 0, 0, ident));
            aptrVar->GetValue().SetLoc(m_Scanner.GetLexemInfo().m_Start);

            m_RTVars.insert(ident);

            m_Scanner.Next();

            if (m_Scanner.GetToken() == CScanner::eTokenLEFTPAREN) {
                // the function name ('ident') was probably misspelled
                string msg = x_GetErrorLocation();
                msg += "Unknown function name: '" + ident + "'";
                NCBI_THROW(CMacroParseException, eParseFunctionExpected, msg);
            } else if (m_Scanner.GetToken() != CScanner::eTokenASSIGNMENT) {
                string msg = x_GetError();
                msg += "Assignment operator expected instead of '";
                msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
                NCBI_THROW(CMacroParseException, eParseAssignmentExpected, msg);
            }
            
            // Create the assignment node
            assignmentNode = doTree->CreateNode(CQueryParseNode::eFrom, aptrVar.release(), 0, "ASSIGNMENT OPERATOR");
            assignmentNode->GetValue().SetLoc(m_Scanner.GetLexemInfo().m_Start);

            // Attach the assignment to the tree
            doTree->GetQueryTree()->AddNode(assignmentNode);
            
            m_Scanner.Next();
        }
        m_ErrorLocation.Reset();

        x_ParseFunctionCall(assignmentNode);
    }
    while ( (m_Scanner.GetToken() != CScanner::eTokenKWDONE) && 
            (m_Scanner.GetToken() != CScanner::eTokenILLEGAL) && 
            (m_Scanner.GetToken() != CScanner::eTokenEOS));

    if (m_Scanner.GetToken() != CScanner::eTokenKWDONE) {
        string msg = x_GetError();
        msg += "Function name or 'DONE' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseFunctionOrDoneExpected, msg);
    }

    m_Scanner.Next();
}

void CMacroParser::x_ParseThreadCount()
{
    if (m_Scanner.GetToken() != CScanner::eTokenKWDOPARL) {
        string msg = x_GetError();
        msg += "Keyword 'DO_P' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseKeywordExpected, msg);
    }

    m_Scanner.Next();
    if (m_Scanner.GetToken() != CScanner::eTokenLEFTPAREN) {
        string msg = x_GetError();
        msg += "Symbol '(' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseLeftParenthesisExpected, msg);
    }

    Int8 threads = 0; // number of threads should always be specified in the macro
    int sign = 1;

    m_Scanner.Next();
    auto token = m_Scanner.GetToken();
    
    while ((token != CScanner::eTokenRIGHTPAREN) &&
        (token != CScanner::eTokenKWDO) &&
        (token != CScanner::eTokenKWWHERE) &&
        (token != CScanner::eTokenIDENT) &&
        (token != CScanner::eTokenILLEGAL) &&
        (token != CScanner::eTokenEOS)) {

        switch (token) {
        case CScanner::eTokenINTNUMBER:
            threads = sign * m_Scanner.GetLexemInfo().m_Data.m_IntNumber;
            break;
        case CScanner::eTokenFLOATNUMBER:
        case CScanner::eTokenSTRING:
        case CScanner::eTokenBOOLCONST:
        {
            string msg = x_GetError();
            msg += "Positive integer number expected instead of '";
            msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
            NCBI_THROW(CMacroParseException, eParseIntegerExpected, msg);
            break;
        }
        case CScanner::eTokenRIGHTBRACKET:
        case CScanner::eTokenRIGHTBRACE:
        {
            string msg = x_GetError();
            msg += "Symbol ')' expected instead of '";
            msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
            NCBI_THROW(CMacroParseException, eParseCommaOrRightBraceExpected, msg);
            break;
        }
        case CScanner::eTokenMINUS:
            sign = -1;
            break;
        default:
            break;
        }

        m_Scanner.Next();
        token = m_Scanner.GetToken();
    }

    if (m_Scanner.GetToken() != CScanner::eTokenRIGHTPAREN) {
        string msg = x_GetError();
        msg += "Symbol ')' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseRightParenthesisExpected, msg);
    }

    if (threads <= 0) {
        string msg = "Number of threads is expected to be a positive, non-zero integer";
        NCBI_THROW(CMacroParseException, eParseThreadCountInvalid, msg);
    }

    m_MacroRep->SetThreadCount((unsigned)threads);
}

bool CMacroParser::x_IsFunction(const string& name) const
{
    auto it = find_if(m_DoFunctionsList.begin(), m_DoFunctionsList.end(),
        [&name](const string& func) { return NStr::EqualNocase(func, name); } );
    return (it != m_DoFunctionsList.end());
}

void CMacroParser::x_ParseFunctionCall(CQueryParseTree::TNode *assignmentNode)
{
    if (m_Scanner.GetToken() != CScanner::eTokenIDENT) {
        string msg = x_GetError();
        msg += "Function name expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseFunctionExpected, msg);
    }

    string fname = m_Scanner.GetLexemInfo().m_Data.m_Str;
    if (!x_IsFunction(fname)) {
        string msg = x_GetError();
        msg += "Unknown function name: '" + fname + "'";
        NCBI_THROW(CMacroParseException, eParseFunctionExpected, msg);
    }

    m_MacroRep->SetFunction(fname, m_Scanner.GetLexemInfo().m_Start, assignmentNode);
    
    m_Scanner.Next();
    if (m_Scanner.GetToken() != CScanner::eTokenLEFTPAREN) {
        string msg = x_GetError();
        msg += "Symbol '(' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseLeftParenthesisExpected, msg);
    }

    m_Scanner.Next();
    if ( m_Scanner.GetToken() != CScanner::eTokenRIGHTPAREN ) {
        bool bNextPar=true;
        while (bNextPar) {
            switch (m_Scanner.GetToken())
            {
            case CScanner::eTokenIDENT:
                // Identifier
                {
                    string ident = m_Scanner.GetLexemInfo().m_Data.m_Str;
                    if (!m_MacroRep->FindVar(ident) && (m_RTVars.find(ident) == m_RTVars.end()) &&
                        !m_MacroRep->HasIdentifier(ident)) {
                        string msg = x_GetError();
                        msg += "Undefined variable: '" + ident + "'";
                        NCBI_THROW(CMacroParseException, eParseUndefinedVariable, msg);
                    }

                    m_MacroRep->SetFunctionParameterAsIdent(ident, m_Scanner.GetLexemInfo().m_Start);
                    m_Scanner.Next();
                    break;
                }
            case CScanner::eTokenSTRING:
                // String 
                m_MacroRep->SetFunctionParameterAsString(m_Scanner.GetLexemInfo().m_Data.m_Str, m_Scanner.GetLexemInfo().m_Start);
                m_Scanner.Next();
                break;
            case CScanner::eTokenBOOLCONST:
                // String 
                m_MacroRep->SetFunctionParameterAsBool(m_Scanner.GetLexemInfo().m_Data.m_BoolValue, m_Scanner.GetLexemInfo().m_Start);
                m_Scanner.Next();
                break;
            default:
                {
                    x_ParseNumericParameterValue(x_ParseSign(), m_Scanner.GetLexemInfo().m_Start);
                }
            }

            if ( m_Scanner.GetToken() == CScanner::eTokenCOMMA )
                m_Scanner.Next();
            else
                bNextPar = false;
        }
    }
    
    if (m_Scanner.GetToken() == CScanner::eTokenDOT) {
        string msg = x_GetError();
        msg += "Symbol '\"' expected around the argument of '" + fname + "'";
        NCBI_THROW(CMacroParseException, eParseWrongVariableUsage, msg);
    } else if (m_Scanner.GetToken() != CScanner::eTokenRIGHTPAREN) {
        string msg = x_GetError();
        msg += "Symbol ')' expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseRightParenthesisExpected, msg);
    }
    m_Scanner.Next();

    if (!assignmentNode && m_Scanner.GetToken() != CScanner::eTokenSEMICOLON) {
        string msg = x_GetError();
        msg += "Symbol ';' expected";
        NCBI_THROW(CMacroParseException, eParseSemicolonExpected, msg);
    }

    switch ( m_Scanner.GetToken() ) {
    case CScanner::eTokenKWWHERE:
        x_ParseFunctionWhere();
        m_Scanner.Next();
        break;
    case CScanner::eTokenSEMICOLON:
        m_Scanner.Next();
        break;
    default:{
        string msg = x_GetError();
        msg += "Keyword 'WHERE' or semicolon expected instead of '";
        msg += m_Scanner.GetLexemInfo().GetStringRepr() + "'";
        NCBI_THROW(CMacroParseException, eParseEndOfFunction, msg);
    }
    }
}

void CMacroParser::x_ParseNumericParameterValue(int sign, CScanner::SLocation start)
{
    switch (m_Scanner.GetToken()) {
    case CScanner::eTokenINTNUMBER:
    {
        Int4 value = sign * m_Scanner.GetLexemInfo().m_Data.m_IntNumber;
        m_MacroRep->SetFunctionParameterAsInt(value, start);
        break;
    }
    case CScanner::eTokenFLOATNUMBER:
    {
        double value = sign * m_Scanner.GetLexemInfo().m_Data.m_FloatNumber;
        m_MacroRep->SetFunctionParameterAsFloat(value, start);
        break;
    }
    default: {
        string msg = x_GetError();
        msg += "Variable definition expected";
        NCBI_THROW(CMacroParseException, eParseVarDefinitionExpected, msg);
    }
    }

    m_Scanner.Next();
}

void CMacroParser::SetFunctionNames(const TFunctionNamesList& wh_funcs, const TFunctionNamesList& do_funcs) {
    m_WhereFunctionsList = wh_funcs;
    m_DoFunctionsList = do_funcs;
}

void CMacroParser::x_ParseMetaData()
{
	if (m_Scanner.GetToken() != CScanner::eTokenCOMMENT) {
		string msg = "COMMENT token expected";
		NCBI_THROW(CMacroParseException, eParseFunctionExpected, msg);
	}

	const char kKeywords[] = "#Keywords:";

	string comment = m_Scanner.GetLexemInfo().GetStringRepr();
	size_t pos = comment.find(kKeywords);
	if (pos == string::npos)
		return;

	string keywords = comment.substr(pos + sizeof(kKeywords) - 1);

	string delim, result;
	pos = 0;
	for(;;) {
		size_t pos2 = keywords.find(',', pos);
		size_t len = (pos2 == string::npos) ? pos2 : pos2 - pos;
		if (len > 0) {
			string w = NStr::TruncateSpaces(keywords.substr(pos, len));
			if (!w.empty()) {
				result += delim;
				delim = ",";
				result += w;
			}
		}

		if (pos2 == string::npos)
			break;

		pos = pos2 + 1;
	}

	if (!result.empty())
		m_MacroRep->AddMetaKeywords(result);
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
