#ifndef GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_QUERY_MACRO_FNS__HPP
#define GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_QUERY_MACRO_FNS__HPP

/*  $Id: phylo_tree_query_macro_fns.hpp 39133 2017-08-04 16:09:53Z asztalos $
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
 * Authors: Bob Falk
 *
 * File Description: 
 * Header file for classes needed to implement query execution.
 *
 */

#include <util/qparse/query_exec.hpp>
#include <gui/objutils/query_func_promote.hpp>

#include <gui/objutils/macro_rep.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_fn_base.hpp>
#include <gui/objutils/macro_cmd_composite.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// Base class for functions which can be called from the WHERE or DO
/// clauses of macros/queries using the CQueryFuncPromote classes
/// (phylogenetic tree, table query)
///
class ITreeMacroFunction : public macro::IMacroFunction<CPhyloTreeNode>
{
public:

    /// Ctor
    ITreeMacroFunction(EScopeEnum func_scope, CPhyloTree* tree)
        : macro::IMacroFunction<CPhyloTreeNode>(func_scope)
        , m_Tree(tree) {}

    /// Functions make the action and collect statistics
    //bool SetQualStringValue(CObjectInfo& oi, const string& value);


protected:
    virtual void x_InitCall(CPhyloTreeNode& data)
    {
        m_Node = &data;
    }

    virtual void x_SetUserObjects(CQueryParseTree::TNode& qnode)
    {
        m_Result = Ref(dynamic_cast<CQueryNodeValue*>(qnode->GetUserObject()));
        if (!m_Result) {
            NCBI_THROW(macro::CMacroExecException, eInternalError, "Error - non-initialized pointer");
        }

        m_Args.resize(0);
        CTreeNode<CQueryParseNode>::TNodeList_I it = qnode.SubNodeBegin();
        CTreeNode<CQueryParseNode>::TNodeList_I it_end = qnode.SubNodeEnd();
        for (; it != it_end; ++it) {
            CRef<CQueryNodeValue> arg = Ref(dynamic_cast<CQueryNodeValue*>((*it)->GetValue().GetUserObject()));
            if (!arg) {
                NCBI_THROW(macro::CMacroExecException, eInternalError, "Error - non-initialized pointer");
            }
            m_Args.push_back(arg);
        }
    }

    /// Filter out java/javascript keywords that could possibly be used in
    /// xss attacks, particularly event handler names and < and > symbols
    static string JSFilter(const string& str);

    /// function useful mostly for functions in the WHERE clause
    void x_AssignReturnValue(const CObjectInfo& oi, const string& field_name);

    CPhyloTreeNode* m_Node;
    CPhyloTree* m_Tree;

    // since macros may come from client, we want to filter out javascript code
    // that might get inserted into out tooltips (TV-525)
    static string s_JSFilterdKeywords[];
    static pair<string, string> s_JSSwapKeywords[];

    vector<CRef<CQueryNodeValue>> m_Args;
    CRef<CQueryNodeValue> m_Result;
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_SetColor
/// Takes one string parameter and uses it to sets the color for the node
/// to that value via the property $NODE_COLOR
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_SetColor : public ITreeMacroFunction  //do
{
public:
    CMacroFunction_SetColor(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}
    virtual ~CMacroFunction_SetColor() {};

    virtual void TheFunction();

};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_SetSize
/// Takes one integer parameter and uses it to sets the radius for the node
/// to that value via the property $NODE_SIZE
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_SetSize : public ITreeMacroFunction  //do
{
public:
    CMacroFunction_SetSize(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_SetMarker
/// Takes one parameter (string) and sets that as the marker property 
/// for the node to create a marker for the node.  See 'marker' in the tutorial:
/// https://www.ncbi.nlm.nih.gov/tools/gbench/tutorial3/
/// for details.
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_SetMarker : public ITreeMacroFunction  //do
{
public:
    CMacroFunction_SetMarker(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_SetBounding
/// Takes one parameter (string) and sets that as the $NODE_BOUNDED property 
/// for the node to create a bounding area around the node and its children.
/// See $NODE_BOUNDED in the tutorial:
/// https://www.ncbi.nlm.nih.gov/tools/gbench/tutorial3/
/// for details.
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_SetBounding : public ITreeMacroFunction  //do
{
public:
    CMacroFunction_SetBounding(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_SetProperty
/// Takes a property name (string) and property value (string) and sets or
/// updates the corresponding name to the specified value.  Since the first
/// parameter name is a property name, not a property value, it should be in 
/// quotes, e.g. SetProperty("creation_date", "10/12/2016")
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_SetProperty : public ITreeMacroFunction  //do
{
public:
    CMacroFunction_SetProperty(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_SubtreeSum
/// Calculates the sum of node property (single parameter) of all the nodes that form the subtree
/// rooted at the specific node (including or excluding itself)
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_SubtreeSum : public ITreeMacroFunction  //do
{
public:
    CMacroFunction_SubtreeSum(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_CollapseNode
/// Collapses the current node. This is equivalent to:
/// SetProperty("$NODE_COLLAPSED", 1);
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_CollapseNode : public ITreeMacroFunction  //do
{
public:
    CMacroFunction_CollapseNode(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_ExpandNode
/// Expands the current node. This is equivalent to:
/// SetProperty("$NODE_COLLAPSED", 0);
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_ExpandNode : public ITreeMacroFunction  //do
{
public:
    CMacroFunction_ExpandNode(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};


///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_AddToSelectionSet
///
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_AddToSelectionSet : public ITreeMacroFunction  //do
{
public:
    CMacroFunction_AddToSelectionSet(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();

protected:

    void x_AddSetInfo(CPhyloSelectionSet& s);

    /// Base cluster id for selection sets. Selection set and cluster-id ids
    /// can't overlap. 
    int m_SelectionSetClusterID;
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_DeleteNode
/// Deletes the current node
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_DeleteNode : public ITreeMacroFunction  // do
{
public:
    CMacroFunction_DeleteNode(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};


///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_IsLeaf
/// Returns true if child is a leaf, so you can do queries that exclude non-leaf
/// nodes, e.g. IsLeaf("") AND IsNull("host_disease") checks for any leaf nodes
/// that do not have a value (even blank) set for "host_disease"
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_IsLeaf : public ITreeMacroFunction  // where
{
public:
    CMacroFunction_IsLeaf(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_NumChildren
/// Returns the number of children of the current node so that you can search
/// for nodes based on that factor, e.g. NumChildren("") > 2
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_NumChildren : public ITreeMacroFunction  // where
{
public:
    CMacroFunction_NumChildren(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_BranchDepth
/// Returns the number of branches between the current node and the root node.
/// 0=>Root node, 1=>immediate children of root node, etc.
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_BranchDepth : public ITreeMacroFunction  // where
{
public:
    CMacroFunction_BranchDepth(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_MaxChildBranchDepth
/// Returns the number of branches between the current node and its most
/// distant (with regards to number of edges) child.
/// 0=>Leaf node, 1=>node which only has leaf nodes as childrent, etc.
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_MaxChildBranchDepth : public ITreeMacroFunction  // where
{
public:
    CMacroFunction_MaxChildBranchDepth(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_IsNull
/// Returns true of the passed in property is not set at all for the current
/// node.  The name of the property being checked must be passed as a string
/// (in quotes), e.g. IsNull("host_disease") rather than the normal way of 
/// passing a field value, e.g. IsNull(host_disease).  The property name is
/// case sensitive!
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_IsNull : public ITreeMacroFunction  // where
{
public:
    CMacroFunction_IsNull(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

///////////////////////////////////////////////////////////////////////////////
/// CMacroFunction_Date
/// Takes a date/time string as a parameter and returns it as an integer number
/// of seconds elapsed since midnight (00:00:00), January 1, 1970 UTC. Do not 
/// use this function if the year is before 1970.
///
/// This allows us to add date-based comparisons to queries, e.g.:
/// target_creation_date > "10/01/2015" where target_creation_date has a format 
/// like: "2016-02-24" would use the following query:
/// Date(target_creation_date, "Y-M-D") > Date("10/01/2015")
///
/// For valid date/time formats see CTimeFormat. Default format is M/D/Y h:m:s
/// and it's ok if trailing values are not provided, e.g. 06/04/91 is ok
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CMacroFunction_Date : public ITreeMacroFunction  // where
{
public:
    CMacroFunction_Date(EScopeEnum func_scope, CPhyloTree* tree)
        : ITreeMacroFunction(func_scope, tree) {}

    virtual void TheFunction();
};

END_NCBI_SCOPE


#endif  // GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_QUERY_MACRO_FNS__HPP
