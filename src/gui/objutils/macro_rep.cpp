/*  $Id: macro_rep.cpp 44800 2020-03-19 16:32:09Z asztalos $ 
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
 * File Description:  Macro representation
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistr.hpp>

#include <gui/objutils/macro_rep.hpp>
#include <gui/objutils/macro_ex.hpp>

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)

const string CMacroRep::m_TopFuncName = "-TOP-DO-SECTION-";

/////////////////////////////////////////////////////////////////////////////
//
//  IMacroVar
//
bool IMacroVar::GetNodeValue(IQueryMacroUserObject& v) const
{
    SValue value = x_GetValue();

    switch (value.m_Type) {
    case eValueTypeString:
        v.SetString(value.m_String);
        return true;
    case eValueTypeInt:
        v.SetInt(value.m_Int);
        return true;
    case eValueTypeFloat:
        v.SetDouble(value.m_Double);
        return true;
    case eValueTypeBool:
        v.SetBool(value.m_Bool);
        return true;
    default:
        return false;
    }
}

IMacroVar::SValue::SValue(const string& str)
    : m_String(str), m_Type(eValueTypeString)
{
    m_Bool = false;
    m_Int = 0;
    m_Double = 0.0;
    x_WriteAsString();
}

IMacroVar::SValue::SValue(Int8 val)
    : m_Int(val), m_Type(eValueTypeInt)
{
    m_Bool = false;
    m_Double = 0.0;
    x_WriteAsString();
}

IMacroVar::SValue::SValue(bool val)
    : m_Bool(val), m_Type(eValueTypeBool)
{
    m_Int = 0;
    m_Double = 0.0;
    x_WriteAsString();
}

IMacroVar::SValue::SValue(double val)
    : m_Double(val), m_Type(eValueTypeFloat)
{
    m_Bool = false;
    m_Int = 0;
    x_WriteAsString();
}

void IMacroVar::SValue::Reset()
{
    m_String.clear(); 
    m_Int = 0; 
    m_Double = 0.0; 
    m_Bool = false;
    m_Type = eValueTypeNotSet;
}

void IMacroVar::SValue::Set(const string& str)
{
    Reset();
    m_String = str;
    m_Type = eValueTypeString;
    x_WriteAsString();
}

void IMacroVar::SValue::Set(Int8 val)
{
    Reset();
    m_Int = val;
    m_Type = eValueTypeInt;
    x_WriteAsString();
}

void IMacroVar::SValue::Set(double val)
{
    Reset();
    m_Double = val;
    m_Type = eValueTypeFloat;
    x_WriteAsString();
}

void IMacroVar::SValue::Set(bool val)
{
    Reset();
    m_Bool = val;
    m_Type = eValueTypeBool;
    x_WriteAsString();
}

/////////////////////////////////////////////////////////////////////////////
//
//  CMacroVarAsk
//
CMacroVarAsk::CMacroVarAsk(const string& name, const string& def_value)
    : IMacroVar(name), m_DefaultValue(def_value), m_NewValue(def_value)
{
    x_ParseString(m_DefaultValue.m_String);
}

void CMacroVarAsk::x_ParseString(const string& str, bool new_value)
{
    m_NewValue.Set(str);
    if (NStr::EqualNocase(str, "true")) {
        m_NewValue.Set(true);
    }
    else if (NStr::EqualNocase(str, "false")) {
        m_NewValue.Set(false);
    }
    else {
        Int8 data_int = NStr::StringToInt8(str, NStr::fConvErr_NoThrow);
        if (errno != 0 && !data_int) {
            double data_dbl = NStr::StringToDouble(str, NStr::fConvErr_NoThrow);
            if (errno == 0) {
                m_NewValue.Set(data_dbl);
            }
        }
        else {
            m_NewValue.Set(data_int);
        }
    }

    if (!new_value) {
        m_DefaultValue = m_NewValue;
    }
}


bool CMacroVarAsk::SetGUIResolvedValue(const string& new_value)
{
    x_ParseString(new_value, true);
    return true;
}

void CMacroVarAsk::Print(CNcbiOstream& os) const
{
    IMacroVar::Print(os);
    os << "        " << "Default is <" << m_DefaultValue.AsString() << ">" << endl;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CMacroVarChoice
//
bool CMacroVarChoice::SetGUIResolvedValue(const string& new_value)
{
    m_Selection = 0;
    for (size_t index = 0; index < m_Choices.size(); ++index) {
        if (m_Choices[index].AsString() == new_value) {
            m_Selection = index;
            return true;
        }
    }
    return false;
}

const string* CMacroVarChoice::GetFirstChoice()
{
    m_ReturnedChoice = 0;
    return GetNextChoice();
}
const string* CMacroVarChoice::GetNextChoice()
{
    if (++m_ReturnedChoice <= m_Choices.size()) {
        return &(m_Choices[m_ReturnedChoice - 1].AsString());
    }
    return NULL;
}

void CMacroVarChoice::Print(CNcbiOstream& os) const
{
    IMacroVar::Print(os);
    if (!m_Choices.empty()) {
        os << "        " << "Choices are: " << endl;
        for (auto& it : m_Choices) {
            os << it.AsString() << endl;
        }
    }
}

IMacroVar::SValue CMacroVarChoice::x_GetValue() const
{ 
    // ensures that selection is within its domain
    _ASSERT(m_Selection < m_Choices.size());
    return m_Choices[m_Selection];
}



/////////////////////////////////////////////////////////////////////////////
//
//  CMacroRep
//
CMacroRep::CMacroRep(CQueryExec* exec)
    : m_Exec(exec)
    , m_SeqRange(0, 0)
    , m_WhereTree(NULL)
    , m_DoTree(new CQueryParseTree)
    , m_NumOfVarBlocks(0)
    , m_ActiveNode(NULL)
{
    m_GUIResolvable = false;
    m_FuncWhereClauses.clear();
    // Top level node for all function calls
    m_DoTree->SetQueryTree(m_DoTree->CreateFuncNode(m_TopFuncName)); 
}

CMacroRep::~CMacroRep()
{
    TVarBlocks::iterator it_b = m_VarBlocks.begin();
    for (; it_b != m_VarBlocks.end(); ++it_b) {
        TVariables::iterator it_v = (*it_b).begin();
        for (; it_v != (*it_b).end(); ++it_v) {
            IMacroVar* pVar = *it_v;
            delete pVar;
        }
    }

    vector<CQueryParseTree*>::iterator it = m_FuncWhereClauses.begin();
    for (; it != m_FuncWhereClauses.end(); ++it)
        delete *it;
}

CQueryParseTree* CMacroRep::GetAssignmentWhereClause(int index) const
{
    return (index < 0 || index >= int(m_FuncWhereClauses.size())) ? 0 : m_FuncWhereClauses[index];
}

void CMacroRep::SetFunction(const string& name, const CQueryParseNode::SSrcLoc &pos, CQueryParseTree::TNode *parentNode)
{ 
    CQueryParseTree::TNode* top_level_node = parentNode ? parentNode : m_DoTree->GetQueryTree();

    m_ActiveNode = m_DoTree->CreateFuncNode(name);
    m_ActiveNode->GetValue().SetLoc(pos);
    top_level_node->AddNode(m_ActiveNode);
}

void CMacroRep::SetFunctionParameterAsIdent(const string& name, const CQueryParseNode::SSrcLoc &pos)
{ 
    CQueryParseTree::TNode* created_node;
    created_node = m_DoTree->CreateNode(CQueryParseNode::eIdentifier, 0, 0, name);
    created_node->GetValue().SetLoc(pos);

    _ASSERT(m_ActiveNode);
    m_ActiveNode->AddNode(created_node);
}

void CMacroRep::SetFunctionParameterAsInt(Int8 value, const CQueryParseNode::SSrcLoc &pos)
{
    CQueryParseTree::TNode* created_node;
    created_node = m_DoTree->CreateNode(value, "not defined");
    created_node->GetValue().SetLoc(pos);

    _ASSERT(m_ActiveNode);
    m_ActiveNode->AddNode(created_node);
}

void CMacroRep::SetFunctionParameterAsFloat(double value, const CQueryParseNode::SSrcLoc &pos)
{
    CQueryParseTree::TNode* created_node;
    created_node = m_DoTree->CreateNode(value, "not defined");
    created_node->GetValue().SetLoc(pos);

    _ASSERT(m_ActiveNode);
    m_ActiveNode->AddNode(created_node);
}

void CMacroRep::SetFunctionParameterAsBool(bool value, const CQueryParseNode::SSrcLoc &pos)
{
    CQueryParseTree::TNode* created_node;
    created_node = m_DoTree->CreateNode(value, "not defined");
    created_node->GetValue().SetLoc(pos);
    
    _ASSERT(m_ActiveNode);
    m_ActiveNode->AddNode(created_node);
}

void CMacroRep::SetFunctionParameterAsString(const string& value, const CQueryParseNode::SSrcLoc &pos)
{
    CQueryParseTree::TNode* created_node;
    created_node = m_DoTree->CreateNode(value, string("\"") + value + "\"", false);
    created_node->GetValue().SetLoc(pos);
    
    _ASSERT(m_ActiveNode);
    m_ActiveNode->AddNode(created_node);
}

void CMacroRep::SetAssignmentWhereClause(const string& where_str, 
                                         CQueryParseTree* where_tree, const CQueryParseNode::SSrcLoc &pos)
{
    _ASSERT(m_ActiveNode);
    CQueryParseTree::TNode* assignment_node = m_ActiveNode->GetParent();
    
    size_t func_size = m_FuncWhereClauses.size();
    m_FuncWhereClauses.push_back(where_tree);
    
    CQueryParseTree::TNode* ref_node;
    ref_node = m_DoTree->CreateNode(Int8(func_size), where_str);
    ref_node->GetValue().SetLoc(pos);

    assignment_node->AddNode(ref_node);
}

void CMacroRep::AttachWhereTree(CQueryParseTree* pWhereTree) 
{
    m_WhereTree.reset(pWhereTree);
}

CMacroRep::TVariables& CMacroRep::x_GetOrCreateLastBlock()
{
    if( m_NumOfVarBlocks > Int4(m_VarBlocks.size()) ) {
        m_VarBlocks.resize ( m_VarBlocks.size() + 1 );
    }

    return  m_VarBlocks.back();
}

void CMacroRep::SetVarInt(const string& name, Int8 value)
{
    TVariables& var_block = x_GetOrCreateLastBlock();
    IMacroVar* var = new CMacroVarSimple (name, value);
    var_block.push_back(var);

    if (!m_GUIResolvable) {
        m_GUIResolvable = var->IsGUIResolvable();
    }
}

void CMacroRep::SetVarFloat(const string& name, double value)
{
    TVariables& var_block = x_GetOrCreateLastBlock();
    IMacroVar* var = new CMacroVarSimple(name, value);
    var_block.push_back(var);

    if (!m_GUIResolvable) {
        m_GUIResolvable = var->IsGUIResolvable();
    }
}

void CMacroRep::SetVarBool(const string& name, bool value)
{
    TVariables& var_block = x_GetOrCreateLastBlock();
    IMacroVar* var = new CMacroVarSimple (name, value);
    var_block.push_back(var);

    if (!m_GUIResolvable) {
        m_GUIResolvable = var->IsGUIResolvable();
    }
}

void CMacroRep::SetVarString(const string& name, const string& value)
{
    TVariables& var_block = x_GetOrCreateLastBlock();
    IMacroVar* var = new CMacroVarSimple(name, value);
    var_block.push_back(var);

    if (!m_GUIResolvable) {
        m_GUIResolvable = var->IsGUIResolvable();
    }
}

void CMacroRep::SetVarAsk(const string& name, const string& value)
{
    TVariables& var_block = x_GetOrCreateLastBlock();
    IMacroVar* var = new CMacroVarAsk(name, value);
    var_block.push_back(var);

    if (!m_GUIResolvable) {
        m_GUIResolvable = var->IsGUIResolvable();
    }
}

void CMacroRep::SetVarChoiceName(const string& name)
{
    TVariables& var_block = x_GetOrCreateLastBlock();
    IMacroVar* var = new CMacroVarChoice (name);
    var_block.push_back(var);

    if (!m_GUIResolvable) {
        m_GUIResolvable = var->IsGUIResolvable();
    }
}

void CMacroRep::SetVarChoiceInt(Int8 value)
{
    TVariables& var_block = m_VarBlocks.back();
    CMacroVarChoice* choice_var = dynamic_cast<CMacroVarChoice*>(var_block.back());
    
    _ASSERT(choice_var);
    choice_var->AddChoiceInt(value);
}

void CMacroRep::SetVarChoiceFloat(double value)
{
    TVariables& var_block = m_VarBlocks.back();
    CMacroVarChoice* choice_var = dynamic_cast<CMacroVarChoice*>(var_block.back());

    _ASSERT(choice_var);
    choice_var->AddChoiceFloat(value);
}

void CMacroRep::SetVarChoiceBool(bool value)
{
    TVariables& var_block = m_VarBlocks.back();
    CMacroVarChoice* choice_var = dynamic_cast<CMacroVarChoice*>(var_block.back());
    
    _ASSERT(choice_var);
    choice_var->AddChoiceBool(value);
}

void CMacroRep::SetVarChoiceString(const string& value)
{
    TVariables& var_block = m_VarBlocks.back();
    CMacroVarChoice* choice_var = dynamic_cast<CMacroVarChoice*>(var_block.back());
    
    _ASSERT(choice_var);
    choice_var->AddChoiceString(value);
}

bool CMacroRep::FindVar(const string& name) const
{
    return (x_FindVar(name)) ? true : false;
}

IMacroVar* CMacroRep::x_FindVar(const string& name) const
{
    for (auto& block_it : m_VarBlocks) {
        for (auto& var_iter : block_it) {
            if (var_iter->GetName() == name)
                return var_iter;
        }
    }
    return 0;
}

bool CMacroRep::GetNodeValue(const string& name, IQueryMacroUserObject& v) const
{
    IMacroVar* var = x_FindVar(name);
    if (var) {
        return var->GetNodeValue(v);
    }
    return false;
}

IMacroVar* CMacroRep::GetFirstVar(Int4& block_num)
{
    m_BlockNumber = block_num = 0;

    m_BlockIter = m_VarBlocks.begin();
    if (m_BlockIter != m_VarBlocks.end()) {
        m_VarIter = (*m_BlockIter).begin();
        if (m_VarIter != (*m_BlockIter).end()) {
            m_BlockNumber = block_num = 1;
            return *m_VarIter;
        }
    }
    return NULL;
}

IMacroVar* CMacroRep::GetNextVar(Int4& block_num)
{
    block_num = 0;
    ++m_VarIter;
    if (m_VarIter != (*m_BlockIter).end()) {
        block_num = m_BlockNumber;
        return *m_VarIter;
    }

    ++m_BlockIter;
    ++m_BlockNumber;

    if (m_BlockIter != m_VarBlocks.end()) {
        m_VarIter = (*m_BlockIter).begin();
        if (m_VarIter != (*m_BlockIter).end()) {
            block_num = m_BlockNumber;
            return *m_VarIter;
        }
    }
    return NULL;
}

bool CMacroRep::HasIdentifier(const string& ident) const 
{
    if (m_Exec != NULL) {
        return m_Exec->HasIdentifier(ident);
    }
    
    return false;
}

void CMacroRep::Print(CNcbiOstream& os) const
{
    string separator(40, '-'), main_sep(40, '=');

    os << main_sep << endl;

    os << "Name: " << m_Name << endl << endl;
    os << "Title: " << m_Title << endl << endl;
    os << "\"Foreach\" selector: " << m_Foreach << endl << endl;
    // -----------------------------------
    os << "Defined variables: ";
    if (m_VarBlocks.size() > 0) {
        os << endl;
        TVarBlocks::const_iterator itb = m_VarBlocks.begin();
        for (; itb != m_VarBlocks.end(); ++itb) {
            const TVariables& vars = *itb;
            TVariables::const_iterator itv = vars.begin();
            for (; itv != vars.end(); ++itv)
                (*itv)->Print(os);
        }
    }
    else
        os << "None." << endl;
    os << endl;
    // -----------------------------------
    x_PrintTree(os, m_WhereTree.get(), "Where-clause tree: ", separator);
    os << endl;
    x_PrintTree(os, m_DoTree.get(), "Do-clause tree: ", separator);
    os << endl;

    int i = -1, vsize = int(m_FuncWhereClauses.size());
    while (i++, i < vsize) {
        x_PrintTree(os,
            m_FuncWhereClauses[i],
            string("Where tree for assignment <") + NStr::IntToString(i) + ">: ",
            separator);
    }
    os << endl;
    os << main_sep << endl;
}

void CMacroRep::x_PrintTree(CNcbiOstream& os, CQueryParseTree* tree,
    const string& title, const string& separator) const
{
    os << separator << endl;
    os << title;
    if (tree) {
        os << endl;
        tree->Print(os);
    }
    else
        os << "None" << endl;
}


END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
