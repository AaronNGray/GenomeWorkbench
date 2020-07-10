#ifndef GUI_WIDGETS_PHY_TREE___PHYLO_RENDER_MODEL__HPP
#define GUI_WIDGETS_PHY_TREE___PHYLO_RENDER_MODEL__HPP

/*  $Id: phylo_tree.hpp 42900 2019-04-25 14:54:04Z katargir $
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
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <gui/utils/vect2.hpp>
#include <gui/opengl/glrect.hpp>
#include <gui/widgets/phylo_tree/tree_model.hpp>
#include <gui/widgets/phylo_tree/tree_graphics_model.hpp>
#include <gui/widgets/phylo_tree/phylo_selection_set.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
///
///    Tree subclass also has functions and data needed for rendering and
///    selection.
///
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloTree : public CTreeModel<CPhyloTreeNode>
{
public:
    typedef CPhyloNodeData::TClusterID TClusterID;
    typedef CPhyloNodeData::TID TID;
    typedef pair<TTreeIdx, CPhyloNodeData::TSelectedState> TSelStateValue;
    typedef vector<TSelStateValue> TSelState;

public:

    /// ctor - need to add nodes to make tree valid
    CPhyloTree();

    /// Clear the tree nodes, graphics model and selected nodes
    void Clear();

    /// Get model for rendering
    CTreeGraphicsModel& GetGraphicsModel();

    /// Set graphics model - may be needed if we want to render
    /// to different graphics contexts since graphics model
    /// has context-specific data (e.g. OpenGL buffer Ids)
    void SetGraphicsModelRef(CRef<CTreeGraphicsModel> gm) { m_GraphicsModel = gm; }
    CRef<CTreeGraphicsModel> GetGraphicsModelRef() { return m_GraphicsModel; }
    CRgbaGradColorTable* GetColorTable();

    /// Sort the children of a node based on the number of children they have
    ///
    /// @param 
    ///    idx - index of the node whose children will be sorted
    ///    order - if true order nodes in descending order of subtree size   
    void Sort(TTreeIdx idx, bool order);

    /// Sort the children of a node based on length of longest subtree
    ///
    /// @param 
    ///    idx - index of the node whose children will be sorted
    ///    order - if true order nodes in descending order of distance
    void SortSubtreeDist(TTreeIdx idx, const vector<float>& distances, bool order);

    /// Sort the children of a node based on label comparison (alphabetical order)
    ///
    /// @param 
    ///    idx - index of the node whose children will be sorted
    ///    order - if true order nodes in descending alphabetical order
    void SortLabel(TTreeIdx idx, bool order);

    /// Sort the children of a node based on the alphanumeric range of
    /// their child nodes.  
    ///
    /// @param 
    ///    idx - index of the node whose children will be sorted
    ///    order - if true order nodes in descending alphabeitcal label range
    void SortLabelRange(TTreeIdx idx, 
                        const vector<pair<string,string> >& subtree_labels, 
                        bool order);

    /// Select or deselect the node at the specified index and, optionally, its
    /// parents and children as well.  Parent selection state is eTraced or eShared.
    ///
    /// @param 
    ///    idx - the node to be selected or unselected (eSelected or eNotSelected)
    ///    sel - if true the node should be selected, false - deselected
    ///    sel_children - if true, all chidren recursively are selected/deselected
    ///    sel_parents - if true all ancestors are selected/deselected
    void  SetSelection(TTreeIdx idx,
                       bool sel,
                       bool sel_children = true,
                       bool sel_parents = true);

    /// Given the selection of node 'idx', store the selection state
    /// (selected, traced, shared) of that node and its parents. This
    /// function does Not update selection state of the actual nodes.
    /// 
    /// @param 
    ///    sel_state - vector to return the selection info
    ///    idx - the node whose selection state we are retrieving
    ///    sel_children - if true, all chidren recursively are added to sel state
    ///    sel_parents - if true all ancestors are added to sel state
    void GetSelState(TSelState& sel_state,
        TTreeIdx idx,
        bool sel_children = true,
        bool sel_parents = true) const;

    /// Sets selection state of all nodes to eNotSelected and clears m_Selected
    void ClearSelection(); 

	/// Returns indices of selected nodes
    ///
    /// @return - vector of all nodes with state eSelected
    void GetSelected(vector<TTreeIdx>& sel) const;

    /// Returns the node ids of selected nodes
    ///
    /// @return - vector of all nodes with state eSelected
    void GetSelectedIDs(vector<TID>& sel) const;
	
    /// Returns the number of selected nodes
    ///
    /// @return - number of selected nodes with state eSelected
    size_t GetNumSelected() const  { return m_Selected.size(); }

    /// Returns only indices of nodes explicitly selected, not their parents
    /// or children, i.e. nodes passed as first parameter to SetSelection(...)
    ///
    /// @return - vector of all explicitly selected nodes
    void GetExplicitlySelected(vector<TTreeIdx>& esel) const;

    /// Returns only indices of nodes explicitly selected, but when a node
    /// is underneath a collapsed node, the collapsed parent is returned
    ///
    /// @return - vector of all explicitly selected nodes
    void GetExplicitlySelectedAndNotCollapsed(vector<TTreeIdx>& esel) const;

    /// Returns only ids of nodes explicitly selected, not their parents
    /// or children, i.e. nodes passed as first parameter to SetSelection(...)
    ///
    /// @return - vector of ids of all explictly selected nodes
    void GetExplicitlySelectedIDs(vector<TID>& esel) const;

    /// Reset explicitly selected nodes to those currently selected
    /// nodes that are also found in the vector 'esel'
    ///
    /// @param
    ///   esel - vector of node indices which, if they are Also in the current
    ///   selection set, will marked as explicitly selected.
    void SetExplicitlySelected(const vector<TID>& esel);

    /// Set the nodes used for iteration to default to the leaf nodes. This 
    /// is used when we set the selection via broadcasting from another
    /// tree such that we don't know which nodes were explicitly selected in
    /// the other tree.
    void SetLeavesExplicitlySelected();

    /// Select a set of nodes - equivalent to calling 
    /// SetSelection(idx, true, true, true) on each of the nodes.
    ///
    /// @param 
    ///    sel - the set of nodes to be selected
    void SetSelection(const vector<TTreeIdx>& sel, 
                      bool sel_children = true, 
                      bool sel_parents = true);

    /// Select a set of nodes - equivalent to calling 
    /// SetSelection(idx, true, true, true) on each of the nodes.
    ///
    /// @param 
    ///    sel - the set of node IDs to be selected
    void SetSelectionIDs(const vector<TID>& ids, bool sel=true, bool sel_children=true);

    bool HasSelection() const { return (m_Selected.size() > 0 ? true : false); }

    /// Return bounding rectangle from lower-left to upper-right of selected
    /// nodes (eSelected) and false if no nodes are currently selected.
    ///
    /// @param
    ///    ll - min x and y coordinates for selected set
    ///    ur - max x and y coordinates for selected set
    /// @return - true if one or more nodes are selected, false otherwise
    bool GetSelectedBoundary(CVect2<float>& ll, CVect2<float>& ur);

    /// Determine if the point pt is close enough to a node to select or
    /// deselect it.  If it is, select or deselect according to sel and toggle
    ///
    /// @param
    ///    pt - the selection position
    ///    sel - if true we are doing normal (vs. incremental) selection
    ///    toggle - if true we should toggle node's selection state
    //     labels_visible - if true node labels are currently displayed
    //     rotated_lables - if true labels are drawn rotated as needed
    /// @return - true if a node's selections state was updated
    bool SelectByPoint(const CVect2<float>& pt, bool sel, bool toggle, 
        bool labels_visible, bool rotated_labels=false);

    /// Select or deselect all nodes in the rectangle bounded by ll and ur
    ///
    /// @param
    ///    ll - lower left corner of selection rectangle
    ///    ur - upper right corner of selection rectangle
    ///    toggle - if true we should toggle node's selection state
    /// @return - true if one or more node's selection state was updated
    bool SelectByRect(const CVect2<float>& ll, 
                      const CVect2<float>& ur,  
                      bool toggle);    

    /// Returns indices of nodes thate are currently collapsed
    ///
    /// @return - vector of all collapsed nodes
    vector<TTreeIdx> GetAllCollapsed() const;

    /// Return the index of the given node (node_idx) if none of its parent
    /// nodes up to the root are collapsed, otherwise return the node index
    /// of the collapsed parent node closest to the root
    ///
    /// @param
    ///    node_idx - the node to check
    /// @return - Index of the node itself or of the collapsed parent node, if
    ///           any, closest to the root.
    TTreeIdx GetCollapsedParentIdx(TTreeIdx node_idx) const;
    
    /// Set the index of the currently active node. Active nodes may have 
    /// edits or property updates applied to them.
    ///
    /// @param
    ///    node_idx - index of the active node within the tree (may be Null())
    void SetCurrentNode(TTreeIdx node_idx);

    /// If a node is close enough to the given position, set it to be the 
    /// currently active node.
    ///
    /// @param
    ///    node_idx - index of the active node within the tree (may be Null())
    /// @return -true if a node was selected (even if it was already current)
    bool SetCurrentNode(const CVect2<float>& pt, bool labels_visible, bool rotated_labels, 
        bool unselect=true);

    /// Return the index of the currently active node (may be Null()).
    TTreeIdx GetCurrentNodeIdx() const { return m_CurrentNode; }

    /// Get reference to currently active node. Throws exception if it's Null()
    CPhyloTreeNode& GetCurrentNode();

    /// Return true if the currently active node is not Null()
    bool HasCurrentNode() const { return m_CurrentNode != Null(); }

    /// Set/Get the edge between the two specified nodes as current. May
    /// be Null (but if one value is null, both are)
    ///
    /// @param
    ///    child_idx - index of the child node of the edge (may be Null())
    ///    parent_idx - index of the parent node of the edge (may be NULL())
    void SetCurrentEdge(TTreeIdx child_idx, TTreeIdx parent_idx);
    void GetCurrentEdge(TTreeIdx& child_idx, TTreeIdx& parent_idx) const;
    bool HasCurrentEdge() const;

    /// Search nodes to find and return closest node to 'pt' that is
    /// within the max. selection distance (used for clicking on nodes)
    ///
    /// @param
    ///    pt - the position against which we are testing
    //     rotated_lables - true if lables are rotated (since we also test against the label)
    /// @return - index of the closest node within selection distance or Null()
    TTreeIdx TestForNode(const CVect2<float>& pt, bool labels_visible, bool rotated_labels=false);

    /// Determine if the node at node_idx is within a collapsed subtree or not
    ///
    /// @param
    ///    node_idx - the node to check
    /// @return - true if no parents (recursively) of node_idx are collapsed
    bool IsVisible(TTreeIdx node_idx);

    /// Return index of the node with the given id or Null(). ID's are unique.
    ///
    /// @param
    ///    id - the id to search for
    /// @return - the node with the given ID or Null().
    TTreeIdx FindNodeById(TID id) const
    {
        TTIDPhyloTreeNodeMap::const_iterator itNode = m_NodesMap.find(id);
        return (itNode != m_NodesMap.end()) ? itNode->second : Null();
    }

    /// Return true if the given position in the array is not currently 
    /// included as a node in the tree. Can happen if nodes are deleted, for
    /// example
    ///
    /// @param
    ///    idx - the index to check
    /// @return - true if the position is empty
    bool IsUnused(TTreeIdx idx) const { return m_Nodes[idx].IsUnused(); }

    /// Return feature dictionary
    CBioTreeFeatureDictionary& GetFeatureDict() { return m_FeatureDict; }  
    const CBioTreeFeatureDictionary& GetFeatureDict() const { return m_FeatureDict; }

    CPhyloSelectionSetMgr& GetSelectionSets() { return m_SelectionSets; }
    const CPhyloSelectionSetMgr& GetSelectionSets() const { return m_SelectionSets; }

    void UpdateNodesMapping();
protected:

    /// Set selected state of anode and add/remove it from m_Selected as needed
    void x_SetSelState(TTreeIdx idx, 
                       CPhyloNodeData::TSelectedState state, 
                       bool override_explicit=true);
    /// Propogate selection operation (selecte/deselect) to a nodes children
    void x_SelectChildren(CPhyloTreeNode& node, bool sel);
    /// Propogate selection (eTraced) to a nodes parents and if children
    /// aren't be selected, don't override explicit parent selection
    void x_SelectParents(CPhyloTreeNode& node, bool sel, bool children_selected = true);
    
    /// Check recursively parents of node and set selection state to eShared if
    /// all of their children are selected
    TTreeIdx x_CommonNodeSearch(TTreeIdx idx);
    void x_PropagateCommonState(CPhyloTreeNode& node);

    /// Get (and return) selection state for parents of a given node
    void x_GetParentState(TSelState& sel_state,
        TTreeIdx idx) const;
    /// Get (and return) selection state for children of a given node
    void x_GetChildState(TSelState& sel_state,
        TTreeIdx idx) const;

    /// Convert parents of node_idx to be its children. Also moves edge data
    /// associated with node children to parents when parent/child role is reversed.
    virtual void x_ConvertUpstream(TTreeIdx node_idx);


    /// Indices of all the nodes with selection state eSelected
    typedef set<TTreeIdx>  TTreeIdxSet;
    TTreeIdxSet  m_Selected;

    /// Often when you select a node, it's children or parents may also
    /// join the selection set.  But in case of queries, user may want to
    /// only see (or iterate over) nodes actually meeting query criteria
    /// or explictly clicked on.
    TTreeIdxSet  m_ExplicitlySelected;

    /// Node ID to tree index map
    typedef map<TID, TTreeIdx> TTIDPhyloTreeNodeMap;
    TTIDPhyloTreeNodeMap m_NodesMap;

    /// Currently active node (for operations like edit), or Null()
    TTreeIdx m_CurrentNode;

    /// Currently active edge (to allow re-rooting by edge). Since we do not 
    /// explicitly store edges, save it as the edges parent and child nodes.
    TTreeIdx m_CurrentEdgeChild;
    TTreeIdx m_CurrentEdgeParent;

    /// Model for rendering (updated from current tree state)
    CRef<CTreeGraphicsModel>  m_GraphicsModel;
    /// Color table to store colors by index
    CRef<CRgbaGradColorTable> m_ColorTable;

    /// Feature dictionary
    CBioTreeFeatureDictionary  m_FeatureDict;

    /// Selection sets - sets of selected nodes, which are saved in the 
    /// biotreecontainer in user data (visualized kind of like clusters)
    CPhyloSelectionSetMgr m_SelectionSets;

private:

    /// Disable copy
    CPhyloTree(const CPhyloTree&);
    CPhyloTree& operator=(const CPhyloTree&);
};

/////////////////////////////////////////////////////////////////////////////
//
//  Tree traversal
//

/// Depth-first tree traversal that skips collapsed nodes.  
///
/// Identical to TreeDepthFirst except for the skipping of collapsed nodes.
///
/// Takes tree and visitor function and calls function for every 
/// node in the subtree rooted at node_idx
///
/// Functor should have the next prototype: 
/// ETreeTraverseCode Func(TTreeType& tree, TTreeType::TTreeIdx node, int delta)
///  where node is a reference to the visited node index and delta_level 
///  reflects the current traverse direction(depth wise) in the tree, 
///   0  - algorithm stays is on the same level
///   1  - we are going one level deep into the tree (from the root)
///  -1  - we are traveling back by one level (getting close to the root)
///
/// The specificts of the algorithm is that it calls visitor both on the 
/// way from the root to leafs and on the way back
/// Using this template we can implement both variants of tree 
/// traversal (pre-order and post-order)
/// Visitor controls the traversal by returning ETreeTraverseCode
///
/// @sa ETreeTraverseCode
///
template<class TTreeModel, class Fun>
void TreeDepthFirstEx(TTreeModel& tree_model, typename TTreeModel::TTreeIdx node_idx, Fun& func)
{
    typedef typename TTreeModel::TNodeType TNodeType;

    int delta_level = 0;
    ETreeTraverseCode stop_scan;

    stop_scan = func(tree_model, node_idx, delta_level);
    switch (stop_scan) {
        case eTreeTraverseStop:
        case eTreeTraverseStepOver:
            return;
        case eTreeTraverse:
            break;
    }

    delta_level = 1;
    TNodeType* tr = &tree_model[node_idx];

    typedef typename TNodeType::TNodeList_I TTreeNodeIterator;

    TTreeNodeIterator it = tr->SubNodeBeginEx();
    TTreeNodeIterator it_end = tr->SubNodeEndEx();

    if (it == it_end)
        return;

    stack<TTreeNodeIterator> tree_stack;

    while (true) {
        tr =  &tree_model[*it];       
        stop_scan = func(tree_model, *it, delta_level);
        switch (stop_scan) {
            case eTreeTraverseStop:
                return;
            case eTreeTraverse:
            case eTreeTraverseStepOver:
                break;
        }
        if ( (stop_scan != eTreeTraverseStepOver) &&
             (delta_level >= 0) && 
             (!tr->IsLeafEx())) {  // sub-node, going down
            tree_stack.push(it);
            it = tr->SubNodeBeginEx();
            it_end = tr->SubNodeEndEx();
            delta_level = 1;
            continue;
        }
        ++it;
        if (it == it_end) { // end of level, going up
            if (tree_stack.empty()) {
                break;
            }
            it = tree_stack.top();
            tree_stack.pop();
            tr =  &tree_model[*it];
            it_end = tree_model[tr->GetParent()].SubNodeEndEx();
            delta_level = -1;
            continue;
        }
        // same level 
        delta_level = 0;

    } // while

    func(tree_model, node_idx, -1);
}

/// Calls TreeDepthFirst with the root node of 'tree_model'
///
template<class TTreeModel, class Fun>
void TreeDepthFirstEx(TTreeModel& tree_model, Fun& func)
{
    TreeDepthFirstEx(tree_model, tree_model.GetRootIdx(), func);
}

///////////////////////////////////////////////////////////////////////////////
/// Breadth-first tree traversal that skips collapsed nodes.  
///
/// Traverse the tree in breadth-first order, skipping
/// collapsed nodes.
///
/// Takes tree and visitor function and calls function for every 
/// node in the subtree rooted at node_idx
///
/// Functor should have the next prototype: 
/// ETreeTraverseCode Func(TTreeType& tree, TTreeType::TTreeIdx node, int delta)
///  where node is a reference to the visited node index and delta_level 
///  reflects the current traverse direction(depth wise) in the tree, 
///   0  - algorithm stays on the same level
///   1  - we are going one level deep into the tree (from the root)
///
///
/// @sa ETreeTraverseCode
///
template<class TTreeModel, class Fun>
void TreeBreadthFirstEx(TTreeModel& tree_model, typename TTreeModel::TTreeIdx node_idx, Fun& func)
{
    typedef typename TTreeModel::TNodeType TNodeType;
    typedef typename TTreeModel::TTreeIdx  TTreeIdx;

    int delta_level = 0;
    ETreeTraverseCode stop_scan;

    stop_scan = func(tree_model, node_idx, delta_level);
    switch (stop_scan) {
        case eTreeTraverseStop:
        case eTreeTraverseStepOver:
            return;
        case eTreeTraverse:
            break;
    }

    typedef typename TNodeType::TNodeList_I TTreeNodeIterator;

    queue<TTreeIdx> node_queue;
    node_queue.push(node_idx);
    int level_count = 1;
    delta_level = 1;

    while (!node_queue.empty()) {
        TTreeIdx node_idx = node_queue.front();
        node_queue.pop();

        TNodeType* tr =  &tree_model[node_idx];

        TTreeNodeIterator it = tr->SubNodeBeginEx();
        TTreeNodeIterator it_end = tr->SubNodeEndEx();
        for (; it != it_end; ++it) {           
            stop_scan = func(tree_model, *it, delta_level);
            switch (stop_scan) {
                case eTreeTraverseStop:
                    return;
                case eTreeTraverse:
                case eTreeTraverseStepOver:
                        break;
            }            
            if ( (stop_scan != eTreeTraverseStepOver) &&
                 (!tr->IsLeafEx())) {  // sub-node, que for next pass
                node_queue.push(*it);
            }
            // same level 
            delta_level = 0;
        }
        
        // track current level so that we can set delta_level correctly
        if (--level_count == 0) {
            level_count = node_queue.size();
            delta_level = 1;
        }

    } // while

    func(tree_model, node_idx, -1);
}

/// Calls TreeBreadthFirstEx with the root node of 'tree_model'
///
template<class TTreeModel, class Fun>
void TreeBreadthFirstEx(TTreeModel& tree_model, Fun& func)
{
    TreeBreadthFirstEx(tree_model, tree_model.GetRootIdx(), func);
}

END_NCBI_SCOPE

#endif
