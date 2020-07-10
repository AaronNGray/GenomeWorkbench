#ifndef GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_QUERY_EXEC__HPP
#define GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_QUERY_EXEC__HPP

/*  $Id: phylo_tree_query_exec.hpp 39021 2017-07-20 16:37:55Z falkrb $
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

#include <objmgr/scope.hpp>

#include <util/qparse/query_exec.hpp>
#include <gui/objutils/query_func_promote.hpp>

#include <gui/objutils/macro_rep.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_fn_base.hpp>
#include <gui/objutils/macro_cmd_composite.hpp>

#include <gui/objutils/macro_query_exec.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_query_macro_fns.hpp>

#include <algo/phy_tree/bio_tree.hpp>

#include <stack>

BEGIN_NCBI_SCOPE

struct CFeatureEdit;
struct CSelectionSetEdit;

////////////////////////////////////////////////////////////////////////////////
/// class CTreeQueryExec
///
///  Subclass of CQueryExec that adds functions spcific to the phylogenetic
///  tree that allow it to extract data from the feature lists of 
///  individual nodes so that those values can be used in queries.
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CTreeQueryExec : public CMacroQueryExec
{
public:
    typedef CPhyloTree TTreeType;
    typedef CPhyloTree::TTreeIdx TTreeIdx;
    typedef CPhyloTree::TNodeType TNodeType;

public:
    CTreeQueryExec(const CBioTreeFeatureDictionary* d) 
        : m_EvalNode(TTreeType::Null()) 
        , m_Tree(NULL)
        , m_Dictionary(d)  
        , m_TopologyChange(false)
        , m_NodeBranchDepth(0)
        , m_NodeMaxChildBranchDepth(-1)
        , m_DisableUndo(false)
        , m_EvalDo(false) {}

    virtual ~CTreeQueryExec();

    /// Set current node for query execution
    void SetTree(CPhyloTree *t);

    /// Returns id of a registred tree feature based on case-insensitive compare
    /// or -1 if feature does not exist.
    TBioTreeFeatureId 
        GetFeatureIdNoCase(const string& feature_name,
                           const CBioTreeFeatureDictionary* dict) const;

    /// @name Find and convert feature-list values in individual nodes
    /// @{    
    /// Search for bool 'identifier' in feature list and return if found
    virtual bool ResolveIdentifier(const std::string& identifier, 
                                   bool& value);    

    /// Search for integer 'identifier' in feature list and return if found
    virtual bool ResolveIdentifier(const std::string& identifier, 
                                   Int8& value);

    /// Search for float 'identifier' in feature list and return if found
    virtual bool ResolveIdentifier(const std::string& identifier, 
                                   double& value);

    /// Search for string 'identifier' in feature list and return if found
    virtual bool ResolveIdentifier(const std::string& identifier, 
                                   std::string& value);

    /// Following functions look up field based on a biotree feature id
    virtual bool ResolveIdentifier(const TFieldID& id, bool& value);
    virtual bool ResolveIdentifier(const TFieldID& id, Int8& value);
    virtual bool ResolveIdentifier(const TFieldID& id, double& value);
    virtual bool ResolveIdentifier(const TFieldID& id, std::string& value);
    /// @}

    /// Get biotree feature ID for identifier or return TFieldID(-1)
    virtual TFieldID GetIdentifier(const std::string& identifier);
    
    /// Search for 'identifier' in dictionary and return true if it exists. 
    /// This does not mean that the value is defined for a particular node.
    virtual bool HasIdentifier(const std::string& identifier);

    /// Some applications may know the type of an identifier.  This hook
    /// should be overriden to return an identifier's type, when available.
    /// Return one of eIntConst, eBoolConst, eFloatConst, eString, or eNotSet.
    virtual CQueryParseNode::EType IdentifierType(const std::string& /* identifier */)
        { return CQueryParseNode::eNotSet; }

    virtual void CallFunction(const string& name, CQueryParseTree::TNode &node);

    /// Move to the first row for eval, return false if table empty
    virtual void EvalStart();
    /// Move to the next row for eval, return false if table size < m_EvalRow+1
    virtual bool EvalNext(CQueryParseTree& qtree);
    virtual bool EvalNext(macro::CMacroRep& m);
    virtual bool EvalComplete();

    // Add the names of the 'do' and 'where' functions from m_BuiltInFunctions 
    // to parser  
    virtual void GetFunctionNames(macro::CMacroParser& parser) const;

    //std::vector<CPhyloTreeNode*> GetTreeSelected() const { return m_Selected; }
    std::vector<TTreeIdx> GetTreeSelected() const { return m_Selected; }
    TTreeIdx GetCurrentIdx() { return m_EvalNode; }

    CFeatureEdit* GetFeatureEdit() { return m_FeatureEdit; }
    CSelectionSetEdit* GetSelectionEdit() { return m_SelectionSetEdit; }

    bool GetTopologyChange() const { return m_TopologyChange; }

    void DisableUndo(bool b) { m_DisableUndo = b; }

protected:
    typedef map<string, ITreeMacroFunction*> TBuiltInFunctionsMap;
    ///functions
    TBuiltInFunctionsMap m_BuiltInFunctions;

    /// Locates function by name
    ITreeMacroFunction* x_ResolveFunctionName(const string& name) const;
    /// Deletes function objects
    void x_ClearBuiltInFunctions();

    /// Moves to next node for 'EvalNext' functions
    void x_EvalAdvance();

    CRef<CMacroCmdComposite> m_CmdComposite;

    /// Node currently being evaluated
    TTreeIdx m_EvalNode;
    CPhyloTree* m_Tree;

    /// for iterating through the node tree without recursion
    struct PhyloTreePointer {
        PhyloTreePointer(TNodeType* n)
            : m_Node(n)
            , m_Iterator(n->SubNodeBegin())
            , m_Index(0)
            , m_MaxChildBranchDepth(-1) {}
        TNodeType* m_Node;
        TNodeType::TNodeList_I m_Iterator;
        size_t m_Index;
        int m_MaxChildBranchDepth;
    };

    /// stack of visited nodes up the tree
    std::stack<PhyloTreePointer> m_NodeStack;

    /// Dictionary of 'features' for the tree
    const CBioTreeFeatureDictionary* m_Dictionary;

    /// current set of selected nodes from the query
    std::vector<TTreeIdx> m_Selected;

    /// The undo/redo buffer for node properties. We don't want to update this
    /// in CGI since it is not needed and would slow things down
    CRef<CFeatureEdit> m_FeatureEdit;

    /// If true topology changed (e.g. a node was deleted)
    bool m_TopologyChange;

    /// Depth in tree as an integer - number of branches between current node and root
    int m_NodeBranchDepth;
    /// Number of branches between the current node and it's most distant (by branch
    /// count) child
    int m_NodeMaxChildBranchDepth;

    /// Undo-redo buffer for selection sets (which are stored in biotreecontainer
    /// user data)
    CRef<CSelectionSetEdit> m_SelectionSetEdit;

    /// To disable use of undo buffer (m_FeatureEdit)
    bool m_DisableUndo;

    /// Flag indicates if the work should be performed on the Do clause.
    bool m_EvalDo; 
};



END_NCBI_SCOPE


#endif  // GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_QUERY_EXEC__HPP

