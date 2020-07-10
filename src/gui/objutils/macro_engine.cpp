/*  $Id: macro_engine.cpp 45086 2020-05-28 14:46:27Z asztalos $
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
 * Authors:  Anatoly Osipov, Dmitry Rudnev, Andrea Asztalos
 *
 * File Description: Macro engine for macro processing
 *
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/macro_engine.hpp>
#include <gui/objutils/macro_parse.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_fn_base.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/objutils/macro_fn_entry.hpp>
#include <gui/objutils/macro_fn_feature.hpp>
#include <gui/objutils/macro_fn_where.hpp>
#include <gui/objutils/macro_fn_pubfields.hpp>
#include <gui/objutils/macro_fn_lookup.hpp>
#include <gui/objutils/macro_fn_string_constr.hpp>
#include <gui/objutils/macro_fn_loc_constr.hpp>
#include <gui/objutils/macro_fn_seq_constr.hpp>
#include <gui/objutils/macro_field_resolve.hpp>

#include <util/line_reader.hpp>

#undef _TRACE
#define _TRACE(arg) ((void)0)

/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// class CMacroEngine
///

CMacroEngine::~CMacroEngine() {
    TBuiltInFunctionsMap::iterator it(m_BuiltInFunctions.begin());

    for (; it != m_BuiltInFunctions.end(); ++it)
        delete it->second;
}

bool CMacroEngine::AppendToLibrary(const string& filename, CMacroLib::TMacroLibrary& lib)
{
    // library is not cleared
    return x_AppendToLibrary(filename, lib);
}

CQueryParseTree* CMacroEngine::GetAssignmentWhereClause(int index) const
{
    return m_MacroRep->GetAssignmentWhereClause( index );
}

bool CMacroEngine::ReadAndParseMacros(const string& filename, vector<CRef<CMacroRep>>& macro_list)
{
    x_ResetStatus();

    string filetext;
    try {
        CRef<ILineReader> line_reader(ILineReader::New(filename));
        while (!line_reader->AtEOF()) {
            line_reader->ReadLine();
            filetext += line_reader->GetCurrentLine();
            filetext += "\n";
        }
    }
    catch (const CException& ex) {
        ERR_POST(ex.ReportAll());
        x_SetError(string("Problem with reading file:") + filename);
        return m_Status;
    }

    CMacroParser parser;
    macro_list.resize(0);
    try {
        x_SetFunctions(parser);
        parser.SetSource(filetext.c_str());
        while (parser.Parse(false)) {
            CRef<CMacroRep> mr(parser.DetachMacroRep());
            macro_list.push_back(mr);
        }

        m_Status = true;
    }
    catch (const CMacroParseException& ex) {
        m_Status = false;
        
        // if there is an error, none of the macros will be read
        macro_list.resize(0);

        m_ErrorMessage = ex.GetMsg();
        if (!m_ErrorMessage.empty()) {
            //m_Line = parser.GetErrorLocation().m_Line;
            //m_Column = parser.GetErrorLocation().m_Column;
        }
        else
            m_ErrorMessage = ex.what();
        return m_Status;
    }

    return m_Status;
}

bool CMacroEngine::x_AppendToLibrary(const string& filename, CMacroLib::TMacroLibrary& lib)
{
    x_ResetStatus();

    string filetext;
    try {
        CRef<ILineReader> line_reader(ILineReader::New(filename));
        while (!line_reader->AtEOF()) {
            line_reader->ReadLine();
            filetext += line_reader->GetCurrentLine();
            filetext += "\n";
        }
    }
    catch (const CException& ex) {
        ERR_POST(ex.ReportAll());
        x_SetError(string("Problem with reading file:") + filename);
        return m_Status;
    }

    CMacroParser parser;
    vector<CRef<CMacroRep>> parsed_vec;
    try {
        x_SetFunctions(parser);
        parser.SetSource(filetext.c_str());
        
        while (parser.Parse(false)) {
            CRef<CMacroRep> mr(parser.DetachMacroRep());
            parsed_vec.push_back(mr);
        }
        m_Status = true;
    }
    catch (const CMacroParseException& ex) {
        m_Status = false;
        m_ErrorMessage = ex.GetMsg();
        if (!m_ErrorMessage.empty()) {
            //m_Line = parser.GetErrorLocation().m_Line;
            //m_Column = parser.GetErrorLocation().m_Column;
        }
        else
            m_ErrorMessage = ex.what();
        return m_Status;
    }

    if (m_Status) {
        // save the macro in the library only when all steps have been successfully parsed
        lib.reserve(lib.size() + parsed_vec.size());
        lib.insert(lib.end(), parsed_vec.begin(), parsed_vec.end());
    }

    return m_Status;
}

void CMacroEngine::x_SetError(const string& message, Uint4 line, Uint4 column)
{
    m_ErrorMessage = message;  
    m_Line = line;
    m_Column = column; 

    m_Status = false;
}

void CMacroEngine::x_ResetStatus(void)
{
    m_Status = true;
    m_ErrorMessage.resize(0);
    m_Line = m_Column = 0;

    m_MacroStat.Reset();
    // clearing the run time variables takes place in Exec() member
}

CMacroRep* CMacroEngine::Parse(const string& macro_text)
{
    x_ResetStatus();

    if (macro_text.empty()) {
        m_Status = false;
        m_ErrorMessage = "The macro is empty.";
        return nullptr;
    }

    CMacroParser parser;
    try {
        x_SetFunctions(parser);
        parser.SetSource(macro_text.c_str());
        parser.Parse();
        return parser.DetachMacroRep();
    }
    catch (const CMacroParseException& ex) {
        m_Status = false;
        m_ErrorMessage = ex.GetMsg();  // this contains the error location
        if (m_ErrorMessage.empty()) {
            m_ErrorMessage = ex.what();
        }
    }

    return nullptr;
}

bool CMacroEngine::Exec(CMacroRep& macro_rep, CMacroBioData& data, CRef<CMacroCmdComposite> CmdComposite, bool throw_on_error, CNcbiOstream* ostream)
{
/*
#ifdef _DEBUG
    macro_rep.Print(NcbiCout);
#endif
*/
    x_ResetStatus();
    m_MacroStat.LogStart(macro_rep.GetName());

    m_RTVars.clear();
    m_TempRTVars.clear();
    m_MacroRep = &macro_rep;

    m_CmdComposite.Reset(CmdComposite);
    
    m_DataIter.Reset(data.CreateIterator(
        m_MacroRep->GetForEachString(), 
        m_MacroRep->GetNamedAnnot(),
        m_MacroRep->GetSeqRange(),
        ostream));

    if (!m_DataIter) {
        m_Status = false;
        m_ErrorMessage = "Missing FOR EACH statement or incorrect selector specified or incorrect NA provided";
        m_Line = 0;   // TBD: implement error location
        m_Column = 0; // TBD: implement error location
        if (throw_on_error) {
            string msg;
            if (m_MacroRep->GetNamedAnnot().empty()) {
                msg = "Missing FOR EACH statement, or unsupported selector";
            }
            else {
                msg = "Incorrect NA provided or missing FOR EACH statement or unsupported selector specified";
            }
            NCBI_THROW(CException, eUnknown, msg);
        }
        return m_Status;
    }

    CMacroExec macro_exec;
    try {
        m_DataIter->Begin();
//        m_MacroStat.AddToReport("Processing features count: " + NStr::NumericToString(m_DataIter->GetCount()) + "\n");
//        m_MacroStat.AddToReport("Best description: " + m_DataIter->GetBestDescr() + "\n");
        while (!m_DataIter->IsEnd()) {
            m_EvalDo = true;
                
            CQueryParseTree* tree = m_MacroRep->GetWhereClause();
            if (tree) {
                m_EvalDo = false;

                macro_exec.EvaluateTree(*tree, *this, true);
                if (!macro_exec.IsBoolType() && !macro_exec.IsNotSetType()) 
                    ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eInternalError, 
                    "Wrong type of computed WHERE clause", tree->GetQueryTree());
                                        
                if (macro_exec.IsBoolType() && macro_exec.GetBoolValue()) {
                    m_EvalDo = true;
                } else {
                    m_DataIter->Next();
                }
            }
                
            if (m_EvalDo) {
                tree = m_MacroRep->GetDoTree();
                _ASSERT(tree);
                m_DataIter->BuildEditedObject();
                // make all the necessary changes to the asn selector object and
                // execute commands on the dependent objects
                macro_exec.EvaluateTree(*tree, *this, false);

                if (m_DataIter->IntendToDelete()) {
                    m_DataIter->RunDeleteCommand(m_CmdComposite);
                }
                else {
                    if (m_DataIter->IsModified()) {
                        m_DataIter->RunEditCommand(m_CmdComposite);
                    }
                    m_DataIter->Next();
                }
            }
        }
        m_Status = true;

        m_MacroStat.AddToReport(m_DataIter->GetReport());
    } 
    catch (const CMacroExecException& ex) {
        m_Status = false;

        m_Line = ex.GetLineNo();
        m_Column = ex.GetColumnNo();
        m_ErrorMessage = ex.GetMsg();
        if (m_ErrorMessage.empty()) {
            m_ErrorMessage = ex.what();
        }

        CNcbiOstrstream oss;
        oss << "[Error] Line " << m_Line << ", Pos " << m_Column;
        oss << ": " << m_ErrorMessage;
        m_ErrorMessage.assign(CNcbiOstrstreamToString(oss));

        if (throw_on_error)
            throw;
    }
/*
#ifdef _DEBUG
    m_MacroStat.LogStop(m_Status, m_ErrorMessage);
#endif
*/
    return m_Status;
}

bool CMacroEngine::ResolveIdentifier(const string& identifier, CMQueryNodeValue& v, const CQueryParseTree::TNode* parent)
{
    _TRACE("<<<<");
    _TRACE("Resolving identifier: " << identifier);
    bool res = false;

    _TRACE("Trying to get it as node value (IMacroVar) from m_MacroRep: " << m_MacroRep);
    if (!res && m_MacroRep) {
        res = m_MacroRep->GetNodeValue(identifier, v);
    }

    if (!res) {
        _TRACE("Trying to get it as an RT Var");
        res = x_ResolveRTVar(identifier, v, parent);
    } else 
        _TRACE("Successfully resolved as node value");

    if (!res) {
        // resolving variables like WHERE biomol = "peptide", example Autofix_008
        _TRACE("Trying to get it via ResolveIdentToSimple() from m_DataIter: " << m_DataIter);
        if(m_DataIter) {
            CObjectInfo obj = m_DataIter->GetEditedObject();
            res = ResolveIdentToSimple(obj, identifier, v);
        }
        if(res)
            _TRACE("Successfully resolved via ResolveIdentToSimple() from m_DataIter");
    }

    if (!res) {
        _TRACE("All attempts to resolve failed");
        v.SetDataType(CMQueryNodeValue::eNotSet);
    }

    _TRACE("Identifier resolved successfully: " << res);
    _TRACE(">>>>");
    return res;
}

IEditMacroFunction* CMacroEngine::x_ResolveFunctionName(const string& name) const
{
    IEditMacroFunction* func = 0;

    auto it(m_BuiltInFunctions.find(name));
    if ( it != m_BuiltInFunctions.end() )
        func = it->second;

    return func;
}

void CMacroEngine::CallFunction(const string& name, CQueryParseTree::TNode& qnode)
{
    IEditMacroFunction* func = x_ResolveFunctionName(name);
    if (!func)
        ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eFunctionNotImplemented, 
                        "Function '" + name + "' not implemented", &qnode);

    if ((m_EvalDo && func->GetFuncScope() == IEditMacroFunction::eWhere) ||
        (!m_EvalDo && func->GetFuncScope() == IEditMacroFunction::eDo))
        ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eWrongFunctionScope, 
                        "Function '" + name + "' used in the wrong DO/WHERE clause scope", &qnode);

    _ASSERT(m_DataIter); 
    _ASSERT(m_CmdComposite.GetNCPointerOrNull());

    const CQueryParseTree::TNode* parent = qnode.GetParent();
    if (parent 
        && (parent->GetValue().GetType() == CQueryParseNode::eFunction  // used in the Where clause
            || parent->GetValue().GetType() == CQueryParseNode::eFrom)) {  // used in the rhs of the assignment operator
        func->SetNestedState(IEditMacroFunction::eNested);
    } else {
        func->SetNestedState(IEditMacroFunction::eNotNested);
    }

    try {
        (*func)(m_CmdComposite.GetNCObject(), m_DataIter, qnode);
    }
    catch (const CMacroDataException &err) {
        string message("Function ");
        message += name;
        message += " failed";
        ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eInvalidData, message, &qnode, &err);
    }
    catch (CMacroExecException &error) {
        // Set the macro position information
        const CQueryParseNode::SSrcLoc &loc = qnode.GetValue().GetLoc();
        // SSrcLoc positions are 0 based
        error.SetLineNo(loc.line + 1);
        error.SetColumnNo(loc.pos + 1);
        throw;
    }
    catch (const CException& e) {
        LOG_POST(Info << "Error in calling function " << name << ": " << e.GetMsg());
        ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eInternalError, CTempString("Error in function ") + name, &qnode, &e);
    }
    catch (const std::exception& e) {
        LOG_POST(Info << "Error in calling function " << name << ": " << e.what());
        ThrowCMacroExecException(DIAG_COMPILE_INFO, CMacroExecException::eInternalError, CTempString("Error in function ") + name, &qnode);
    }
    
    // TODO
    // log the activities of this function
    m_MacroStat.AddToReport((*func).GetFuncReport());
}

void CMacroEngine::x_SetFunctions(CMacroParser& parser)
{
    CMacroParser::TFunctionNamesList where_funcs, do_funcs;

    for (auto& iter : m_BuiltInFunctions) {
        string name = iter.first;
        IEditMacroFunction* func = iter.second;
        if ( func ) {
            switch (func->GetFuncScope()) {
            case IEditMacroFunction::eDo:
                do_funcs.push_back(name); 
                break;
            case IEditMacroFunction::eWhere:
                where_funcs.push_back(name); 
                break;
            case IEditMacroFunction::eBoth:
                do_funcs.push_back(name); 
                where_funcs.push_back(name); 
                break;
            default:
                ; // unknown case
            }
        }
    }

    parser.SetFunctionNames(where_funcs, do_funcs);
}

CRef<CMQueryNodeValue> CMacroEngine::GetOrCreateRTVar(const string& name)
{
    _TRACE("<<<<");
    _TRACE("Looking for: " << name);
    if ( name.empty() )
        return CRef<CMQueryNodeValue>();

    auto rt_var = x_LocateRTVar( name );
    if ( rt_var ) {
        _TRACE("found!");
        _TRACE(">>>>");
        return rt_var;
    }

    CRef<CMQueryNodeValue> node(new CMQueryNodeValue());
    m_RTVars[name] = node;
    _TRACE("created as node: " << node.GetPointerOrNull());
    _TRACE(">>>>");
    return node;
}

bool CMacroEngine::ExistRTVar(const string& name)
{
    _TRACE("<<<<");
    _TRACE("Looking for (whether it exists) :" << name);
    auto it = m_RTVars.find(name);
    if (it != m_RTVars.end()) {
        _TRACE("found!");
        return true;
    }

    _TRACE("not found!");
    return false;
}

CRef<CMQueryNodeValue> CMacroEngine::x_LocateRTVar(const string& name)
{
    _TRACE("<<<<");
    _TRACE("Looking for: " << name);
/*
#ifdef _DEBUG
    _TRACE("Current m_RTVars snapshot:");
    ITERATE(TRTVarsMap, iRTVars, m_RTVars) {
        _TRACE(iRTVars->first);
    }
#endif
*/
    auto it = m_RTVars.find(name);
    if (it != m_RTVars.end()) {
        _TRACE("found!");
        _TRACE(">>>>");
        return (*it).second;
    } else {
        _TRACE("not found!");
        _TRACE(">>>>");
        return CRef<CMQueryNodeValue>();
    }
}

bool CMacroEngine::x_ResolveRTVar(const string& identifier, CMQueryNodeValue& v, const CQueryParseTree::TNode* parent)
{
    _TRACE("<<<<");
    _TRACE("Looking for: " << identifier);
    bool isOK(false);

    // check for a "." inside the RTVar name -- 
    // it may mean that a field of the var is required
    string sRTVarRoot, sRTVarFields;
    NStr::SplitInTwo(identifier, ".", sRTVarRoot, sRTVarFields);
    CRef<CMQueryNodeValue> rt_var = x_LocateRTVar(sRTVarRoot);

    if ( rt_var ) {
        CObjectInfo tmpObj;
        if(!GetTmpRTVarObject(sRTVarRoot, tmpObj)) {
            _TRACE("datatype of rt_var: " << rt_var->GetDataType());
            if (NStr::IsBlank(sRTVarFields)) {  // EditStringQual(o, find_text, ...) where o = Resolve("org.taxname");
                v.SetRef( rt_var );
                _TRACE("set reference from node: " << rt_var << " to node: " << &v);
                isOK = true;
            } else {  // EditStringQual("o.taxname", find_text, ...), where o = Resolve("org");
                if (rt_var->GetDataType() == CMQueryNodeValue::eObjects) {
                    const CMQueryNodeValue::TObs& objs = rt_var->GetObjects();
                    if (!objs.empty()) { 
                        ITERATE (CMQueryNodeValue::TObs, obj_it, objs) {
                            isOK |= ResolveIdentToObjects(obj_it->field, sRTVarFields, v);
                        }
                        _TRACE("resolved subfields of run-time vars");
                    } else { // if there are no resolved objects
                        v.SetRef( rt_var );
                        _TRACE("set reference from node: " << rt_var << " to node: " << &v);
                        isOK = true;
                    }
                } else if (rt_var->GetDataType() == CMQueryNodeValue::eNotSet) { // when the run-time variable is not set
                    v.SetRef(rt_var);
                    _TRACE("set reference from node: " << rt_var << " to node: " << &v);
                    isOK = true;
                }
            }
        } else if (m_DataIter){ // used in the WHERE clause
            if (parent && parent->GetValue().GetType() == CQueryParseNode::eFunction) { // EQUALS(o, match_text)
                isOK = ResolveIdentToObjects(tmpObj, sRTVarFields, v);
                _TRACE("got value using iterator in form of objects (from tmpObj)");
            } else { // o.subtype (= "isolate")
                isOK = GetSimpleTypeValue(tmpObj, sRTVarFields, v);
                _TRACE("got simple value using iterator");
            }
        }
        _TRACE(">>>> " << isOK);
        return isOK;
    }
    _TRACE("RT Var not found");
    _TRACE(">>>> false");
    return false;
}

void CMacroEngine::AddTmpRTVarObject(const string& name, CObjectInfo& oi)
{
    m_TempRTVars[name] = oi;
}

bool CMacroEngine::GetTmpRTVarObject(const string& name, CObjectInfo& oi)
{
    auto it = m_TempRTVars.find(name);
    if (it != m_TempRTVars.end()) {
        _TRACE("found!");
        _TRACE(">>>> true");
        oi = it->second;
        return true;
    } else {
        _TRACE("not found!");
        _TRACE(">>>> false");
        return false;
    }
}

void CMacroEngine::ResetTmpRTVarObjects()
{
    m_TempRTVars.clear();
}

///////////////////////////////////////////////////////////////////////////////
/// Setting up the functions
///
void CMacroEngine::x_InitSetOfBuiltInFunctions(void)
{
    // Regarding Function Names: where clause function names have to be in uppercase! (limitation of qparse lib)
    m_BuiltInFunctions.emplace(CMacroFunction_TopLevel::GetFuncName(), new CMacroFunction_TopLevel(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetStringQual::GetFuncName(), new CMacroFunction_SetStringQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_EditStringQual::GetFuncName(), new CMacroFunction_EditStringQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_CopyStringQual::GetFuncName(), new CMacroFunction_CopyStringQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_CopyFeatQual::GetFuncName(), new CMacroFunction_CopyFeatQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ConvertStringQual::GetFuncName(), new CMacroFunction_ConvertStringQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ConvertFeatQual::GetFuncName(), new CMacroFunction_ConvertFeatQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SwapQual::GetFuncName(), new CMacroFunction_SwapQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SwapRelFeatQual::GetFuncName(), new CMacroFunction_SwapRelFeatQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SwapRelatedFeaturesQual_Depr::GetFuncName(), new CMacroFunction_SwapRelatedFeaturesQual_Depr(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ParseStringQual::GetFuncName(), new CMacroFunction_ParseStringQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ParseFeatQual::GetFuncName(), new CMacroFunction_ParseFeatQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_AddDBLink::GetFuncName(), new CMacroFunction_AddDBLink(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ParseToStructComm::GetFuncName(), new CMacroFunction_ParseToStructComm(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ApplyStrucVoucherPart::GetFuncName(), new CMacroFunction_ApplyStrucVoucherPart(IEditMacroFunction::eDo));
    
    m_BuiltInFunctions.emplace(CMacroFunction_AddParsedText::GetFuncName(), new CMacroFunction_AddParsedText(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace("AECRParseStringQual", new CMacroFunction_AddParsedText(IEditMacroFunction::eDo)); //deprecated
    m_BuiltInFunctions.emplace(CMacroFunction_AddParsedToFeatQual::GetFuncName(), new CMacroFunction_AddParsedToFeatQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ParsedText::GetFuncName(), new CMacroFunction_ParsedText(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveQual::GetFuncName(), new CMacroFunction_RemoveQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveOutside::GetFuncName(), new CMacroFunction_RemoveOutside(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RmvOutsideRelFeatQual::GetFuncName(), new CMacroFunction_RmvOutsideRelFeatQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_Resolve::GetFuncName(), new CMacroFunction_Resolve(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ResolveBioSourceQuals::GetFuncName(), new CMacroFunction_ResolveBioSourceQuals(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ResolveNASeq::GetFuncName(), new CMacroFunction_ResolveNASeq(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_LocalID::GetFuncName(), new CMacroFunction_LocalID(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetQual::GetFuncName(), new CMacroFunction_SetQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetRnaProduct::GetFuncName(), new CMacroFunction_SetRnaProduct(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveRnaProduct::GetFuncName(), new CMacroFunction_RemoveRnaProduct(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_TrimStringQual::GetFuncName(), new CMacroFunction_TrimStringQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveOrgName::GetFuncName(), new CMacroFunction_RemoveOrgName(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetStructCommField::GetFuncName(), new CMacroFunction_SetStructCommField(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetStructCommDb::GetFuncName(), new CMacroFunction_SetStructCommDb(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetRelFeatQual::GetFuncName(), new CMacroFunction_SetRelFeatQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_EditRelFeatQual::sm_FunctionName, new CMacroFunction_EditRelFeatQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RmvRelFeatQual::GetFuncName(), new CMacroFunction_RmvRelFeatQual(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_FixSourceQualCaps::GetFuncName(), new CMacroFunction_FixSourceQualCaps(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_TrimJunkFromPrimers::GetFuncName(), new CMacroFunction_TrimJunkFromPrimers(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_FixIInPrimers::GetFuncName(), new CMacroFunction_FixIInPrimers(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_FixMouseStrain::GetFuncName(), new CMacroFunction_FixMouseStrain(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_EditSubfield::GetFuncName(), new CMacroFunction_EditSubfield(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveSubfield::GetFuncName(), new CMacroFunction_RemoveSubfield(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_RemoveDescriptor::GetFuncName(), new CMacroFunction_RemoveDescriptor(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveSequence::GetFuncName(), new CMacroFunction_RemoveSequence(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveAlignment::GetFuncName(), new CMacroFunction_RemoveAlignment(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_UnculturedTaxLookup::sm_FunctionName, new CMacroFunction_UnculturedTaxLookup(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_CulturedTaxLookup::sm_FunctionName, new CMacroFunction_CulturedTaxLookup(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_CorrectGeneticCodes::GetFuncName(), new CMacroFunction_CorrectGeneticCodes(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_RemoveModifier::GetFuncName(), new CMacroFunction_RemoveModifier(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_AddorSetBsrcModifier::GetFuncName(), new CMacroFunction_AddorSetBsrcModifier(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_AddorSetGbQual::GetFuncName(), new CMacroFunction_AddorSetGbQual(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_FixUSAandStatesAbbrev::GetFuncName(), new CMacroFunction_FixUSAandStatesAbbrev(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_FixFormat::GetFuncName(), new CMacroFunction_FixFormat(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveLineageSourceNotes::GetFuncName(), new CMacroFunction_RemoveLineageSourceNotes(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_ChangeSeqInst::GetFuncName(), new CMacroFunction_ChangeSeqInst(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_FixAuthorCaps::GetFuncName(), new CMacroFunction_FixAuthorCaps(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_FixPubCapsTitle::GetFuncName(), new CMacroFunction_FixPubCapsTitle(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_FixPubCapsAuthor::GetFuncName(), new CMacroFunction_FixPubCapsAuthor(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_FixPubCapsAffil::GetFuncName(), new CMacroFunction_FixPubCapsAffil(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_FixPubCapsAffilWithExcept::GetFuncName(), new CMacroFunction_FixPubCapsAffilWithExcept(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_FixPubCapsAffilCountry::GetFuncName(), new CMacroFunction_FixPubCapsAffilCountry(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_RemoveSegGaps::GetFuncName(), new CMacroFunction_RemoveSegGaps(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RmvDuplStructComments::GetFuncName(), new CMacroFunction_RmvDuplStructComments(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ReorderStructComment::GetFuncName(), new CMacroFunction_ReorderStructComment(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_MakeBoldXrefs::GetFuncName(), new CMacroFunction_MakeBoldXrefs(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_AddProteinTitles::GetFuncName(), new CMacroFunction_AddProteinTitles(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetCodonsRecognized::GetFuncName(), new CMacroFunction_SetCodonsRecognized(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_TrimTerminalNs::GetFuncName(), new CMacroFunction_TrimTerminalNs(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ToUnknownLengthGap::GetFuncName(), new CMacroFunction_ToUnknownLengthGap(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_ISOJTALookup::GetFuncName(), new CMacroFunction_ISOJTALookup(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_LookupPub::GetFuncName(), new CMacroFunction_LookupPub(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_DOILookup::GetFuncName(), new CMacroFunction_DOILookup(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ApplyPmidToEntry::GetFuncName(), new CMacroFunction_ApplyPmidToEntry(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ApplyDOIToEntry::GetFuncName(), new CMacroFunction_ApplyDOIToEntry(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_MoveMiddleName::GetFuncName(), new CMacroFunction_MoveMiddleName(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_StripSuffix::GetFuncName(), new CMacroFunction_StripSuffix(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_TruncateMI::GetFuncName(), new CMacroFunction_TruncateMI(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ReverseAuthNames::GetFuncName(), new CMacroFunction_ReverseAuthNames(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_AddGeneXref::GetFuncName(), new CMacroFunction_AddGeneXref(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveGeneXref::GetFuncName(), new CMacroFunction_RemoveGeneXref(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_JoinShorttRNAs::GetFuncName(), new CMacroFunction_JoinShorttRNAs(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_TrimStopFromCompleteCDS::GetFuncName(), new CMacroFunction_TrimStopFromCompleteCDS(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SynchronizeCDSPartials::GetFuncName(), new CMacroFunction_SynchronizeCDSPartials(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_AdjustConsensusSpliceSites::GetFuncName(), new CMacroFunction_AdjustConsensusSpliceSites(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RetranslateCDS::GetFuncName(), new CMacroFunction_RetranslateCDS(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ReplaceStopWithSelenocysteine::GetFuncName(), new CMacroFunction_ReplaceStopWithSelenocysteine(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ConvertFeature::GetFuncName(), new CMacroFunction_ConvertFeature(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ConvertCDS::GetFuncName(), new CMacroFunction_ConvertCDS(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ApplyCDSFrame::GetFuncName(), new CMacroFunction_ApplyCDSFrame(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_FixProteinFormat::GetFuncName(), new CMacroFunction_FixProteinFormat(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveInvalidECNumbers::GetFuncName(), new CMacroFunction_RemoveInvalidECNumbers(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_UpdateReplacedECNumbers::GetFuncName(), new CMacroFunction_UpdateReplacedECNumbers(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_UpdatemRNAProduct::GetFuncName(), new CMacroFunction_UpdatemRNAProduct(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_CopyNameToCDSNote::GetFuncName(), new CMacroFunction_CopyNameToCDSNote(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveFeature::GetFuncName(), new CMacroFunction_RemoveFeature(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveAllFeatures::GetFuncName(), new CMacroFunction_RemoveAllFeatures(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveDuplFeatures::GetFuncName(), new CMacroFunction_RemoveDuplFeatures(IEditMacroFunction::eDo));
    
    m_BuiltInFunctions.emplace(CMacroFunction_RenormalizeNucProtSet::GetFuncName(), new CMacroFunction_RenormalizeNucProtSet(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_Autodef::GetFuncName(), new CMacroFunction_Autodef(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_AutodefId::GetFuncName(), new CMacroFunction_AutodefId(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveSingleItemSet::GetFuncName(), new CMacroFunction_RemoveSingleItemSet(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_FixSpelling::GetFuncName(), new CMacroFunction_FixSpelling(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_DiscrepancyAutofix::GetFuncName(), new CMacroFunction_DiscrepancyAutofix(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_TaxLookup::GetFuncName(), new CMacroFunction_TaxLookup(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_CreateProteinFeats::GetFuncName(), new CMacroFunction_CreateProteinFeats(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ConvertRawToDeltabyNs::GetFuncName(), new CMacroFunction_ConvertRawToDeltabyNs(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_DeltaSeqToRaw::GetFuncName(), new CMacroFunction_DeltaSeqToRaw(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_Extend5Feature::GetFuncName(), new CMacroFunction_Extend5Feature(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_Extend3Feature::GetFuncName(), new CMacroFunction_Extend3Feature(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetBothPartials::GetFuncName(), new CMacroFunction_SetBothPartials(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemoveBothPartials::GetFuncName(), new CMacroFunction_RemoveBothPartials(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ConvertLocType::GetFuncName(), new CMacroFunction_ConvertLocType(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ConvertLocStrand::GetFuncName(), new CMacroFunction_ConvertLocStrand(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_Set5Partial::GetFuncName(), new CMacroFunction_Set5Partial(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_Set3Partial::GetFuncName(), new CMacroFunction_Set3Partial(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_Clear5Partial::GetFuncName(), new CMacroFunction_Clear5Partial(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_Clear3Partial::GetFuncName(), new CMacroFunction_Clear3Partial(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_MakeInterval::GetFuncName(), new CMacroFunction_MakeInterval(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_MakeWholeSeqInterval::GetFuncName(), new CMacroFunction_MakeWholeSeqInterval(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_MakePoint::GetFuncName(), new CMacroFunction_MakePoint(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ApplyCDS::GetFuncName(), new CMacroFunction_ApplyCDS(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ApplyRNA::GetFuncName(), new CMacroFunction_ApplyRNA(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ApplyGene::GetFuncName(), new CMacroFunction_ApplyGene(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ApplyOtherFeature::GetFuncName(), new CMacroFunction_ApplyOtherFeature(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_ApplyPublication::sm_FunctionName, new CMacroFunction_ApplyPublication(IEditMacroFunction::eDo, CPub::e_Sub));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubAffil_Depr::sm_FunctionName, new CMacroFunction_SetPubAffil_Depr(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubDate_Depr::sm_FunctionName, new CMacroFunction_SetPubDate_Depr(IEditMacroFunction::eDo));
    
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubTitle::GetFuncName(), new CMacroFunction_SetPubTitle(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubAffil::GetFuncName(), new CMacroFunction_SetPubAffil(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubAuthor::GetFuncName(), new CMacroFunction_SetPubAuthor(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_AddPubAuthor::GetFuncName(), new CMacroFunction_AddPubAuthor(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_AddAuthorList::GetFuncName(), new CMacroFunction_AddAuthorList(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubJournal::GetFuncName(), new CMacroFunction_SetPubJournal(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubVolIssuePages::sm_FuncVolume, new CMacroFunction_SetPubVolIssuePages(IEditMacroFunction::eDo, "volume"));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubVolIssuePages::sm_FuncIssue, new CMacroFunction_SetPubVolIssuePages(IEditMacroFunction::eDo, "issue"));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubVolIssuePages::sm_FuncPages, new CMacroFunction_SetPubVolIssuePages(IEditMacroFunction::eDo, "pages"));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubCitation::GetFuncName(), new CMacroFunction_SetPubCitation(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetSerialNumber::GetFuncName(), new CMacroFunction_SetSerialNumber(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubDate::GetFuncName(), new CMacroFunction_SetPubDate(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubDateField::GetFuncName(), new CMacroFunction_SetPubDateField(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubStatus::GetFuncName(), new CMacroFunction_SetPubStatus(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubPMID::GetFuncName(), new CMacroFunction_SetPubPMID(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_SetPubAuthorMI::GetFuncName(), new CMacroFunction_SetPubAuthorMI(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_RemovePubAuthorMI::GetFuncName(), new CMacroFunction_RemovePubAuthorMI(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_RemovePubAuthors::GetFuncName(), new CMacroFunction_RemovePubAuthors(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_ValueFromTable::GetFuncName(), new CMacroFunction_ValueFromTable(IEditMacroFunction::eDo));

    // printing functions
    m_BuiltInFunctions.emplace(CMacroFunction_PrintCSV::GetFuncName(), new CMacroFunction_PrintCSV(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_PrintTSV::GetFuncName(), new CMacroFunction_PrintTSV(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_PrintBankit::GetFuncName(), new CMacroFunction_PrintBankit(IEditMacroFunction::eDo));
    m_BuiltInFunctions.emplace(CMacroFunction_PrintLiteral::GetFuncName(), new CMacroFunction_PrintLiteral(IEditMacroFunction::eDo));

    m_BuiltInFunctions.emplace(CMacroFunction_SatelliteType::GetFuncName(), new CMacroFunction_SatelliteType(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_SatelliteName::GetFuncName(), new CMacroFunction_SatelliteName(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_GeneQual::GetFuncName(), new CMacroFunction_GeneQual(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_StructVoucherPart::GetFuncName(), new CMacroFunction_StructVoucherPart(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_MobileElementType::GetFuncName(), new CMacroFunction_MobileElementType(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_MobileElementName::GetFuncName(), new CMacroFunction_MobileElementName(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_GetRnaProduct::GetFuncName(), new CMacroFunction_GetRnaProduct(IEditMacroFunction::eBoth));

    // SNP related functions
    m_BuiltInFunctions.emplace(CMacroFunction_VariationType::GetFuncName(), new CMacroFunction_VariationType(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_RefAllele::GetFuncName(), new CMacroFunction_RefAllele(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_AltAlleles::GetFuncName(), new CMacroFunction_AltAlleles(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_Consequence::GetFuncName(), new CMacroFunction_Consequence(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_SnpID::GetFuncName(), new CMacroFunction_SnpID(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_VcfSevenCol::GetFuncName(), new CMacroFunction_VcfSevenCol(IEditMacroFunction::eBoth));
    
    m_BuiltInFunctions.emplace(CMacroFunction_ChoiceType::GetFuncName(), new CMacroFunction_ChoiceType(IEditMacroFunction::eWhere));
    m_BuiltInFunctions.emplace(CMacroFunction_Features_For_Object::GetFuncName(), new CMacroFunction_Features_For_Object(IEditMacroFunction::eWhere));
    m_BuiltInFunctions.emplace(CMacroFunction_CDSTranslation::GetFuncName(), new CMacroFunction_CDSTranslation(IEditMacroFunction::eWhere));
    m_BuiltInFunctions.emplace(CMacroFunction_SeqID::GetFuncName(), new CMacroFunction_SeqID(IEditMacroFunction::eWhere));
    m_BuiltInFunctions.emplace(CMacroFunction_Accession::GetFuncName(), new CMacroFunction_Accession(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_InconsistentTaxa::GetFuncName(), new CMacroFunction_InconsistentTaxa(IEditMacroFunction::eWhere));
    m_BuiltInFunctions.emplace(CMacroFunction_InTable::GetFuncName(), new CMacroFunction_InTable(IEditMacroFunction::eWhere));
   
    m_BuiltInFunctions.emplace(CMacroFunction_GetSeqdesc::sm_BsrcForMolinfo, new CMacroFunction_GetSeqdesc(IEditMacroFunction::eWhere, CSeqdesc::e_Source));
    m_BuiltInFunctions.emplace(CMacroFunction_GetSeqdesc::sm_MolinfoForBsrc, new CMacroFunction_GetSeqdesc(IEditMacroFunction::eWhere, CSeqdesc::e_Molinfo));
    m_BuiltInFunctions.emplace(CMacroFunction_GetSeqdesc::sm_BsrcForSeq, new CMacroFunction_GetSeqdesc(IEditMacroFunction::eBoth, CSeqdesc::e_Source));
    m_BuiltInFunctions.emplace(CMacroFunction_GetSeqdesc::sm_BsrcForFeat, new CMacroFunction_GetSeqdesc(IEditMacroFunction::eWhere, CSeqdesc::e_Source));
    m_BuiltInFunctions.emplace(CMacroFunction_GetSeqdesc::sm_MolinfoForFeat, new CMacroFunction_GetSeqdesc(IEditMacroFunction::eWhere, CSeqdesc::e_Molinfo));
    m_BuiltInFunctions.emplace(CMacroFunction_GetSeqdesc::sm_BsrcForSeqdesc, new CMacroFunction_GetSeqdesc(IEditMacroFunction::eWhere, CSeqdesc::e_Source));
    m_BuiltInFunctions.emplace(CMacroFunction_GetDBLink::GetFuncName(), new CMacroFunction_GetDBLink(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_GetSequence::sm_SeqForDescr, new CMacroFunction_GetSequence(IEditMacroFunction::eWhere));
    m_BuiltInFunctions.emplace(CMacroFunction_GetSequence::sm_SeqForFeat, new CMacroFunction_GetSequence(IEditMacroFunction::eWhere));

    // Location constraints
    m_BuiltInFunctions.emplace(CMacroFunction_LocPartialTest::sm_PartialStart, new CMacroFunction_LocPartialTest(IEditMacroFunction::eWhere, ELocEndType::eStart));
    m_BuiltInFunctions.emplace(CMacroFunction_LocPartialTest::sm_PartialStop, new CMacroFunction_LocPartialTest(IEditMacroFunction::eWhere, ELocEndType::eStop));
    m_BuiltInFunctions.emplace(CMacroFunction_LocEnd::sm_Start, new CMacroFunction_LocEnd(IEditMacroFunction::eBoth, ELocEndType::eStart));
    m_BuiltInFunctions.emplace(CMacroFunction_LocEnd::sm_Stop, new CMacroFunction_LocEnd(IEditMacroFunction::eBoth, ELocEndType::eStop));
    m_BuiltInFunctions.emplace(CMacroFunction_LocationStrand::GetFuncName(), new CMacroFunction_LocationStrand(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_LocationStrandSymbol::GetFuncName(), new CMacroFunction_LocationStrandSymbol(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_LocationType::GetFuncName(), new CMacroFunction_LocationType(IEditMacroFunction::eWhere));
    m_BuiltInFunctions.emplace(CMacroFunction_LocationSeqType::sm_SeqNa, new CMacroFunction_LocationSeqType(IEditMacroFunction::eWhere, eSeqtype_constraint_nuc));
    m_BuiltInFunctions.emplace(CMacroFunction_LocationSeqType::sm_SeqAa, new CMacroFunction_LocationSeqType(IEditMacroFunction::eWhere, eSeqtype_constraint_prot));
    m_BuiltInFunctions.emplace(CMacroFunction_LocationDistConstraint::sm_FromStart, new CMacroFunction_LocationDistConstraint(IEditMacroFunction::eWhere, ELocEndType::eStart));
    m_BuiltInFunctions.emplace(CMacroFunction_LocationDistConstraint::sm_FromStop, new CMacroFunction_LocationDistConstraint(IEditMacroFunction::eWhere, ELocEndType::eStop));

    m_BuiltInFunctions.emplace(CMacroFunction_FeatStrandednessConstraint::sm_FunctionName, new CMacroFunction_FeatStrandednessConstraint(IEditMacroFunction::eWhere));
    m_BuiltInFunctions.emplace(CMacroFunction_NumberOfFeatures::sm_FunctionName, new CMacroFunction_NumberOfFeatures(IEditMacroFunction::eWhere));
    m_BuiltInFunctions.emplace(CMacroFunction_Contained::GetFuncName(), new CMacroFunction_Contained(IEditMacroFunction::eWhere));
    m_BuiltInFunctions.emplace(CMacroFunction_GeneType::GetFuncName(), new CMacroFunction_GeneType(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_Label::GetFuncName(), new CMacroFunction_Label(IEditMacroFunction::eBoth));

    // Both WHERE and DO clause functions:
    // String constraints
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_Upper, new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, kEmptyStr, CMacroFunction_StringConstraints::eUpper));
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_Lower, new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, kEmptyStr, CMacroFunction_StringConstraints::eLower));
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_Punct, new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, kEmptyStr, CMacroFunction_StringConstraints::ePunct));
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_UpperAll, 
        new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, kEmptyStr, CMacroFunction_StringConstraints::eUpper, CMacroFunction_StringConstraints::eAND));
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_LowerAll, 
        new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, kEmptyStr, CMacroFunction_StringConstraints::eLower, CMacroFunction_StringConstraints::eAND));
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_PunctAll, 
        new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, kEmptyStr, CMacroFunction_StringConstraints::ePunct, CMacroFunction_StringConstraints::eAND));
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_Start, new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, "starts"));
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_End, new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, "ends"));
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_Equal, new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, "equals"));
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_Contain, new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, "contains"));
    m_BuiltInFunctions.emplace(CMacroFunction_StringConstraints::sm_InList, new CMacroFunction_StringConstraints(IEditMacroFunction::eBoth, "inlist"));

    m_BuiltInFunctions.emplace(CMacroFunction_StringLength::GetFuncName(), new CMacroFunction_StringLength(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_IsPresent::GetFuncName(), new CMacroFunction_IsPresent(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_RelatedFeatures::GetFuncName(), new CMacroFunction_RelatedFeatures(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_StructCommField::GetFuncName(), new CMacroFunction_StructCommField(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_StructCommDatabase::GetFuncName(), new CMacroFunction_StructCommDatabase(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_StructCommFieldname::GetFuncName(), new CMacroFunction_StructCommFieldname(IEditMacroFunction::eBoth));
    m_BuiltInFunctions.emplace(CMacroFunction_FirstOrLastItem::sm_First, new CMacroFunction_FirstOrLastItem(IEditMacroFunction::eBoth, true));
    m_BuiltInFunctions.emplace(CMacroFunction_FirstOrLastItem::sm_Last, new CMacroFunction_FirstOrLastItem(IEditMacroFunction::eBoth, false));
    m_BuiltInFunctions.emplace(CMacroFunction_IllegalDbXref::GetFuncName(), new CMacroFunction_IllegalDbXref(IEditMacroFunction::eBoth));
    

    // Publication fields
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubAuthors, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_authors));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubCit, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_cit));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubSerialNumber, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_serial_number));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubJournal, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_journal));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubVolume, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_volume));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubIssue, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_issue));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubPages, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_pages));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubDate, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_date));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubTitle, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_title));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubAffil, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_affiliation));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubPMID, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_pmid));
    m_BuiltInFunctions.emplace(CMacroFunction_PubFields::sm_PubClass, new CMacroFunction_PubFields(IEditMacroFunction::eBoth, ePublication_field_pub_class));
}

///////////////////////////////////////////////////////////////////////////////
/// class CMacroStat
///

void CMacroStat::Reset()
{
    m_MacroReport.Reset();
    m_StartTime = 0;
    m_StopTime = 0;
    m_counter = 0;
    m_ChangedQuals.clear();
}

void CMacroStat::AddToReport(const string& report)
{
    if (!report.empty()) {
        if (report.find(";") != NPOS) {
            vector<string> fields;
            NStr::Split(report, ";", fields);
            auto it = m_ChangedQuals.find(fields[1]);
            try {
                if (it == m_ChangedQuals.end()) {
                    m_ChangedQuals.emplace(fields[1], NStr::StringToInt(fields[0]));
                }
                else {
                    it->second += NStr::StringToInt(fields[0]);
                }
            }
            catch (const CException&) {}
        }

        if (!m_ChangedQuals.empty()) {
            string msg = "Changed ";
            for (auto& it : m_ChangedQuals) {
                msg += NStr::IntToString(it.second);
                msg += " fields during " + it.first;
            }
            m_MacroReport.ClearAndBuildReport(msg);
        }
        else {
            m_MacroReport.BuildReport(report);
        }
        m_counter++;
    }
}

void CMacroStat::LogStart(const string& macro_name)
{
    m_MacroReport.SetName(macro_name);

#ifdef _DEBUG
    CTime::GetCurrentTimeT(&m_StartTime);
    string s = "Macro (";
    s += m_MacroReport.GetName();
    s += ") execution started.";
    LOG_POST(Info << s);
#endif
}

void CMacroStat::LogStop(bool status, const string& err_message)
{
    CTime::GetCurrentTimeT(&m_StopTime);

    string s = "Macro (";
    s += m_MacroReport.GetName();
    s += ") execution ";
    s += ((status) ? "was successful" : "failed");
    s += ". Elapsed time - ";
    s += NStr::LongToString(static_cast<long>(m_StopTime - m_StartTime));
    s += " seconds.";

    if (status)
        LOG_POST(Info << s);
    else {
        LOG_POST(s);

        s = "Macro (";
        s += m_MacroReport.GetName();
        s += ") error message: '";
        s += err_message;
        s += "'";
        LOG_POST(s);
    }
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
