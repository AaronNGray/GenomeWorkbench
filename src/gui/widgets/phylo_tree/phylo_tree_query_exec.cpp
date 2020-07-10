/*  $Id: phylo_tree_query_exec.cpp 39133 2017-08-04 16:09:53Z asztalos $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *      Implementation for tree query execution
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_query_exec.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>

#include <gui/objutils/query_func_promote.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <gui/objutils/macro_ex.hpp>

#include <objects/seq/seq_id_handle.hpp>

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistd.hpp>

BEGIN_NCBI_SCOPE


/******************************************************************************
 *
 * CPhyloQueryExec Implementation
 *
 *****************************************************************************/ 


 ///
class  CMacroFunction_TopLevel2 : public ITreeMacroFunction
{
public:
    CMacroFunction_TopLevel2(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}
    virtual ~CMacroFunction_TopLevel2() {};

    virtual void TheFunction() {}
};

CTreeQueryExec::~CTreeQueryExec() 
{
    x_ClearBuiltInFunctions();
}

///////////////////////////////////////////////////////////////////////////////
/// Setting up the functions
///
void CTreeQueryExec::SetTree(CPhyloTree *t) 
{ 
    m_Tree = t; 
    x_ClearBuiltInFunctions();

    m_BuiltInFunctions[macro::CMacroRep::m_TopFuncName] = new CMacroFunction_TopLevel2(ITreeMacroFunction::eDo, m_Tree);
    m_BuiltInFunctions["ISLEAF"] = new CMacroFunction_IsLeaf(ITreeMacroFunction::eWhere, m_Tree);
    m_BuiltInFunctions["NUMCHILDREN"] = new CMacroFunction_NumChildren(ITreeMacroFunction::eWhere, m_Tree);
    m_BuiltInFunctions["BRANCHDEPTH"] = new CMacroFunction_BranchDepth(ITreeMacroFunction::eWhere, m_Tree);
    m_BuiltInFunctions["MAXCHILDBRANCHDEPTH"] = new CMacroFunction_MaxChildBranchDepth(ITreeMacroFunction::eWhere, m_Tree);
    m_BuiltInFunctions["ISNULL"] = new CMacroFunction_IsNull(ITreeMacroFunction::eWhere, m_Tree); 
    m_BuiltInFunctions["DATE"] = new CMacroFunction_Date(ITreeMacroFunction::eWhere, m_Tree);
    m_BuiltInFunctions["SETCOLOR"] = new CMacroFunction_SetColor(ITreeMacroFunction::eDo, m_Tree);
    m_BuiltInFunctions["SETSIZE"] = new CMacroFunction_SetSize(ITreeMacroFunction::eDo, m_Tree);
    m_BuiltInFunctions["SETPROPERTY"] = new CMacroFunction_SetProperty(ITreeMacroFunction::eDo, m_Tree);
    m_BuiltInFunctions["COLLAPSENODE"] = new CMacroFunction_CollapseNode(ITreeMacroFunction::eDo, m_Tree);
    m_BuiltInFunctions["EXPANDNODE"] = new CMacroFunction_ExpandNode(ITreeMacroFunction::eDo, m_Tree);
    m_BuiltInFunctions["SETMARKER"] = new CMacroFunction_SetMarker(ITreeMacroFunction::eDo, m_Tree);
    m_BuiltInFunctions["SETBOUNDING"] = new CMacroFunction_SetBounding(ITreeMacroFunction::eDo, m_Tree);
    m_BuiltInFunctions["ADDTOSELECTIONSET"] = new CMacroFunction_AddToSelectionSet(ITreeMacroFunction::eDo, m_Tree);
    m_BuiltInFunctions["DELETENODE"] = new CMacroFunction_DeleteNode(ITreeMacroFunction::eDo, m_Tree);
    m_BuiltInFunctions["SUBTREESUM"] = new CMacroFunction_SubtreeSum(ITreeMacroFunction::eDo, m_Tree);

    // This should be reset on a per-macro-set basis (for undo), so move this to an appropriate location
    // to implement undo.
    m_CmdComposite.Reset(new CMacroCmdComposite("TreeMacro"));
}

void CTreeQueryExec::x_ClearBuiltInFunctions()
{
    TBuiltInFunctionsMap::iterator it(m_BuiltInFunctions.begin());

    for (; it != m_BuiltInFunctions.end(); ++it)
        delete it->second;

    m_BuiltInFunctions.clear();
}

void CTreeQueryExec::GetFunctionNames(macro::CMacroParser& parser) const
{
    macro::CMacroParser::TFunctionNamesList where_funcs, do_funcs;

    TBuiltInFunctionsMap::const_iterator cit = m_BuiltInFunctions.begin();
    for (; cit != m_BuiltInFunctions.end(); ++cit) {
        string name = (*cit).first;
        ITreeMacroFunction* p_func = (*cit).second;
        if (p_func) {
            switch (p_func->GetFuncScope()) {
            case ITreeMacroFunction::eDo:
                do_funcs.push_back(name);
                break;
            case ITreeMacroFunction::eWhere:
                where_funcs.push_back(name);
                break;
            case ITreeMacroFunction::eBoth:
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

TBioTreeFeatureId 
CTreeQueryExec::GetFeatureIdNoCase(const string& feature_name,
                                   const CBioTreeFeatureDictionary* dict) const
{
    const CBioTreeFeatureDictionary::TFeatureDict& fd = dict->GetFeatureDict();
    CBioTreeFeatureDictionary::TFeatureDict::const_iterator it;

    for(it = fd.begin(); it != fd.end(); ++it) {
        int cmp = NStr::CompareNocase(feature_name.c_str(), 
                                      (*it).second.c_str());
        if (cmp==0)
            return it->first;
    }

    return (TBioTreeFeatureId)-1;
}

bool CTreeQueryExec::ResolveIdentifier(const std::string& identifier, 
                                        bool& value) 
{ 
    const CBioTreeFeatureDictionary* dict = 
        &(m_Tree->GetFeatureDict());
    TBioTreeFeatureId id = GetFeatureIdNoCase(identifier, dict);

    return ResolveIdentifier(id, value);
}

bool CTreeQueryExec::ResolveIdentifier(const std::string& identifier, 
                                        Int8& value) 
{ 
    const CBioTreeFeatureDictionary* dict = 
        &(m_Tree->GetFeatureDict());
    TBioTreeFeatureId id = GetFeatureIdNoCase(identifier, dict);

    return ResolveIdentifier(id, value);
}

bool CTreeQueryExec::ResolveIdentifier(const std::string& identifier, 
                                        double& value) 
{ 
    const CBioTreeFeatureDictionary* dict = 
        &(m_Tree->GetFeatureDict());
    TBioTreeFeatureId id = GetFeatureIdNoCase(identifier, dict);

    return ResolveIdentifier(id, value);
}

bool CTreeQueryExec::ResolveIdentifier(const std::string& identifier, 
                                        std::string& value) 
{ 
    const CBioTreeFeatureDictionary* dict = 
        &(m_Tree->GetFeatureDict());
    TBioTreeFeatureId id = GetFeatureIdNoCase(identifier, dict);

    return ResolveIdentifier(id, value);
}

bool CTreeQueryExec::ResolveIdentifier(const TFieldID& id,
                                       bool& value)
{
    if (id != (TBioTreeFeatureId)-1) {
        string feature_value;
        bool has_value = m_Tree->GetNode(m_EvalNode)->GetBioTreeFeatureList().
            GetFeatureValue(id, feature_value);

        if (has_value) {
            try {
                value = NStr::StringToBool(feature_value);
                return true;
            }
            catch (CStringException&) {
                return false;
            }
        }
    }

    return false;
}

bool CTreeQueryExec::ResolveIdentifier(const TFieldID& id,
                                       Int8& value)
{
    if (id != (TBioTreeFeatureId)-1) {
        string feature_value;
        bool has_value = m_Tree->GetNode(m_EvalNode)->GetBioTreeFeatureList().
            GetFeatureValue(id, feature_value);

        if (has_value) {
            try {
                value = NStr::StringToInt8(feature_value);
                return true;
            }
            catch (CStringException&) {
                return false;
            }
        }
    }

    return false;
}
bool CTreeQueryExec::ResolveIdentifier(const TFieldID& id,
                                       double& value)
{
    if (id != (TBioTreeFeatureId)-1) {
        string feature_value;
        bool has_value = m_Tree->GetNode(m_EvalNode)->GetBioTreeFeatureList().
            GetFeatureValue(id, feature_value);

        if (has_value) {
            try {
                value = NStr::StringToDouble(feature_value);
                return true;
            }
            catch (CStringException&) {
                return false;
            }
        }
    }

    return false;
}
bool CTreeQueryExec::ResolveIdentifier(const TFieldID& id,
                                       std::string& value)
{
    if (id != (TBioTreeFeatureId)-1) {
        return m_Tree->GetNode(m_EvalNode)->GetBioTreeFeatureList().GetFeatureValue(id, value);
    }

    return false;
}

/// @}

/// Get biotree feature ID for identifier or return TFieldID(-1)
CTreeQueryExec::TFieldID CTreeQueryExec::GetIdentifier(const std::string& identifier)
{
    return GetFeatureIdNoCase(identifier, m_Dictionary);
}

bool CTreeQueryExec::HasIdentifier(const std::string& identifier)
{ 
    TBioTreeFeatureId id = GetFeatureIdNoCase(identifier, m_Dictionary);

    return (id != (TBioTreeFeatureId)-1);
}

ITreeMacroFunction* CTreeQueryExec::x_ResolveFunctionName(const string& name) const {
    ITreeMacroFunction* func = 0;
    string uname = name;
    NStr::ToUpper(uname);

    TBuiltInFunctionsMap::const_iterator it(m_BuiltInFunctions.find(uname));
    if (it != m_BuiltInFunctions.end())
        func = it->second;

    return func;
}

void CTreeQueryExec::CallFunction(const string& name, CQueryParseTree::TNode &node) 
{
    ITreeMacroFunction* func = x_ResolveFunctionName(name);
    if (!func)
        macro::ThrowCMacroExecException(DIAG_COMPILE_INFO, macro::CMacroExecException::eFunctionNotImplemented,
        "Error - function \"" + name + "\" not implemented", &node);

    if ((m_EvalDo && func->GetFuncScope() == ITreeMacroFunction::eWhere) ||
        (!m_EvalDo && func->GetFuncScope() == ITreeMacroFunction::eDo))
        macro::ThrowCMacroExecException(DIAG_COMPILE_INFO, macro::CMacroExecException::eWrongFunctionScope,
        "Error - function \"" + name + "\" is used in the wrong do/where clause scope", &node);

    if (m_Tree == NULL) {
        macro::ThrowCMacroExecException(DIAG_COMPILE_INFO, macro::CMacroExecException::eInternalError,
            "Error - tree pointer was not set", &node);
    }

    const CQueryParseTree::TNode* parent = node.GetParent();
    if (parent &&
        (parent->GetValue().GetType() == CQueryParseNode::eFunction ||  // used in the Where clause
        parent->GetValue().GetType() == CQueryParseNode::eFrom)) {  // used in the rhs of the assignment operator
        func->SetNestedState(ITreeMacroFunction::eNested);
    }
    else {
        func->SetNestedState(ITreeMacroFunction::eNotNested);
    }

    // Add branch/depth info to query node value object as a way to pass 
    // contextual (where we are int the tree)  info to the functions
    CQueryNodeValue*  v = dynamic_cast<CQueryNodeValue*>(
        node.GetValue().GetUserObject());
    v->SetBranchDepth(m_NodeBranchDepth);
    v->SetMaxChildBranchDepth(m_NodeMaxChildBranchDepth);

    try {
        (*func)(m_CmdComposite.GetObject(), m_Tree->GetNode(m_EvalNode), node);
    }
    catch (const macro::CMacroDataException &err) {
        string message("Function ");
        message += name;
        message += " failed";
        macro::ThrowCMacroExecException(DIAG_COMPILE_INFO, macro::CMacroExecException::eInvalidData, message, &node, &err);
    }
    catch (macro::CMacroExecException &error) {
        // Set the macro position information
        const CQueryParseNode::SSrcLoc &loc = node.GetValue().GetLoc();
        // SSrcLoc positions are 0 based
        error.SetLineNo(loc.line + 1);
        error.SetColumnNo(loc.pos + 1);
        throw;
    }
    catch (const CException& e) {
        LOG_POST(Info << "Error in calling function " << name << ": " << e.GetMsg());
        macro::ThrowCMacroExecException(DIAG_COMPILE_INFO, macro::CMacroExecException::eInternalError, CTempString("Error - in the function ") + name, &node, &e);
    }
}

void CTreeQueryExec::EvalStart()
{
    m_ExceptionCount = 0;
    m_QueriedCount = 0;
    m_TopologyChange = false;
    m_NodeBranchDepth = 0;
    m_NodeMaxChildBranchDepth = -1;
    m_EvalNode = m_Tree->GetRootIdx();
    m_Selected.clear();
    m_RTVars.clear();

    /// For macro queries (holds undo buffer)
    m_FeatureEdit.Reset(new CFeatureEdit());
    m_SelectionSetEdit.Reset(new CSelectionSetEdit());

    // Save current state now and get updated state at end
    m_SelectionSetEdit->GetPrevSet() = m_Tree->GetSelectionSets();

    // We are doing post order traversal so we don't want to start with the root.
    // Push the root onot the stack and then advance (to first leaf)
    m_NodeStack.push(PhyloTreePointer(&(m_Tree->GetRoot())));
    x_EvalAdvance();
}

bool CTreeQueryExec::EvalComplete()
{
    return m_NodeStack.empty();
}

bool CTreeQueryExec::EvalNext(CQueryParseTree& qtree) {
    bool selected = false;

    try {
        ++m_QueriedCount;
        Evaluate(qtree);

        CQueryParseTree::TNode* top_node = qtree.GetQueryTree();
        CQueryNodeValue*  v = dynamic_cast<CQueryNodeValue*>(
            top_node->GetValue().GetUserObject());

        if (v != NULL) {
            if (v->GetValue()) {
                m_Selected.push_back(m_EvalNode);
                selected = true;
            }
        }
    }
    // Could avoid flooding log since many may be identical, but
    // maybe the last one is the one you need to see (for debugging...)
    catch(CQueryParseException &pe) {
        LOG_POST("Query parse error during execution: " << pe.GetMsg());
        ++m_ExceptionCount;
    }
    catch(CQueryExecException &ex) {
        LOG_POST(Info << "Query execution error: " << ex.GetMsg());
        ++m_ExceptionCount;
    }
    catch (CException &ex) {
        LOG_POST(Info << "Query error: " << ex.GetMsg());
        ++m_ExceptionCount;
    }

    x_EvalAdvance();

    return selected;
}

bool CTreeQueryExec::EvalNext(macro::CMacroRep& m)
{
    bool selected = false;
    SetMacroRep(&m);

    try {
        m_EvalDo = false;
        CQueryParseTree* qtree = m.GetWhereClause();
        if (qtree) {
            ++m_QueriedCount;

            Evaluate(*qtree);

            CQueryParseTree::TNode* top_node = qtree->GetQueryTree();
            CQueryNodeValue*  v = dynamic_cast<CQueryNodeValue*>(
                top_node->GetValue().GetUserObject());

            if (v != NULL) {
                if (v->GetValue()) {
                    m_Selected.push_back(m_EvalNode);
                    selected = true;
                }
            }
        }
        else {
            m_Selected.push_back(m_EvalNode);
            selected = true;
        }

        if (selected) {
            m_EvalDo = true;

            qtree = m.GetDoTree();

            if (!qtree)
                NCBI_THROW(macro::CMacroExecException, eInternalError, "Error - no functions found in the do section");
            try {
                if (m_DisableUndo) {
                    // execute commands on the selected node
                    Evaluate(*qtree);
                }
                else {
                    CUpdatedFeature f;

                    // Node updates are all changes to biotree features or 
                    // the tree's user data
                    // Record current (pre-update) features here.
                    CPhyloTreeNode& node = m_Tree->GetNode(m_EvalNode);                    
                    f.GetPrevFeatures() = node->GetBioTreeFeatureList();

                    // execute commands on the selected node
                    Evaluate(*qtree);

                    // Check if node was deleted. Topology changes require a different type
                    // of update to the project biotreecontainer (replace rather than update)
                    CPhyloNodeData::TID node_id = node.GetValue().GetId();
                    if (node_id == CPhyloNodeData::TID(-1)) {
                        m_TopologyChange = true;
                    }

                    // Determine if any features actually changed by comparing previous
                    // and current feature lists (if none changed, no need for undo)                
                    f.GetFeatures() = node->GetBioTreeFeatureList();
                    bool node_updated = false;
                    if (f.GetPrevFeatures().GetFeatureList().size() != f.GetFeatures().GetFeatureList().size()) {
                        node_updated = true;
                    }
                    else {
                        for (size_t i = 0; i < f.GetFeatures().GetFeatureList().size(); ++i) {
                            if (f.GetFeatures().GetFeatureList()[i].id != f.GetPrevFeatures().GetFeatureList()[i].id ||
                                f.GetFeatures().GetFeatureList()[i].value != f.GetPrevFeatures().GetFeatureList()[i].value) {
                                node_updated = true;
                                break;
                            }
                        }
                    }

                    // If any features changed, add a new 'updatedfeature' object to the undo/redo object
                    // Note that evaluating the "Do" section has already updated the biotree featurelist
                    // so the feature update will update the underlying biotreecontainer object but the
                    // changes propagated back to the tree will not change any values.  (But the previous
                    // features we captured will support undo)
                    if (node_updated) {
                        f.SetNode(node->GetId(), m_EvalNode);
                        m_FeatureEdit->GetUpdated().push_back(f);
                    }
                }
            }
            catch (const CException& e) {
                LOG_POST("Error while executing macro: " << e.GetMsg());
                ++m_ExceptionCount;
            }
        }
    }
    catch (CQueryParseException &pe) {
        // Could avoid flooding log since many may be identical, but
        // maybe the last one is the one you need to see (for debugging...)
        LOG_POST("Query parse error during execution: " << pe.GetMsg());
        ++m_ExceptionCount;
    }
    catch (CQueryExecException &ex) {
        // Could avoid flooding log since many may be identical, but
        // maybe the last one is the one you need to see (for debugging...)
        LOG_POST(Info << "Query execution error: " << ex.GetMsg());
        ++m_ExceptionCount;
    }

    x_EvalAdvance();

    return selected;
}

void CTreeQueryExec::x_EvalAdvance()
{
    PhyloTreePointer p = m_NodeStack.top();

    if (p.m_Node->IsUnused()) {
        m_NodeStack.pop();
        --m_NodeBranchDepth;
        if (!m_NodeStack.empty()) {
            m_NodeStack.top().m_Iterator = m_NodeStack.top().m_Node->SubNodeBegin() + m_NodeStack.top().m_Index;
            p = m_NodeStack.top();
        }
    }
    else {
        if (p.m_Node->IsLeaf() || p.m_Iterator == p.m_Node->SubNodeEnd()) {
            if (p.m_Node->IsLeaf()) {
                m_NodeMaxChildBranchDepth = 0;              
            }

            m_NodeStack.pop();
            --m_NodeBranchDepth;

            if (!m_NodeStack.empty()) {
                if (m_NodeStack.top().m_Iterator != m_NodeStack.top().m_Node->SubNodeEnd()) {
                    ++(m_NodeStack.top().m_Iterator);
                    ++(m_NodeStack.top().m_Index);
                }
                m_NodeMaxChildBranchDepth = std::max(++m_NodeMaxChildBranchDepth, m_NodeStack.top().m_MaxChildBranchDepth);
                m_NodeStack.top().m_MaxChildBranchDepth = m_NodeMaxChildBranchDepth;

                p = m_NodeStack.top();
            }
        }
    }

    while (!p.m_Node->IsLeaf() && p.m_Iterator != p.m_Node->SubNodeEnd()) {
        m_NodeStack.push(PhyloTreePointer(&(m_Tree->GetNode(*p.m_Iterator))));
        p = m_NodeStack.top();
        ++m_NodeBranchDepth;

        m_NodeMaxChildBranchDepth = -1;
        if (p.m_Node->IsLeaf()) {
            m_NodeMaxChildBranchDepth = 0;
            m_NodeStack.top().m_MaxChildBranchDepth = 0;
        }
    }

    if (!m_NodeStack.empty()) {
        m_EvalNode = m_Tree->FindNodeById(m_NodeStack.top().m_Node->GetValue().GetId());
    }    
}

END_NCBI_SCOPE
