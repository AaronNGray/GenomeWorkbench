#ifndef GUI_WIDGETS_PHY_TREE___TREE_MODEL__HPP
#define GUI_WIDGETS_PHY_TREE___TREE_MODEL__HPP

/*  $Id: tree_model.hpp 38346 2017-04-27 13:24:57Z falkrb $
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
#include <corelib/ncbi_tree.hpp>

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
///
///    Bi-directionaly linked N way tree allocated in a contiguous memory 
///    block.  Links are represented as indexes into the array.
///


/// Index type for tree
typedef size_t TTreeIdx;

/// Global define for a NULL link in the tree used for comparison
#define NULL_TREE_IDX static_cast<TTreeIdx>(-1)

/// Base class for nodes in the tree.  Takes a data template parameter to allow
/// additional storage.
template <class TData>
class CTreeModelNode
{
public:
    typedef TData TValueType;
    typedef size_t TTreeIdx;

    typedef vector<TTreeIdx>                           TNodeList; 
    typedef typename TNodeList::iterator               TNodeList_I;
    typedef typename TNodeList::const_iterator         TNodeList_CI;
    typedef typename TNodeList::reverse_iterator       TNodeList_RI;
    typedef typename TNodeList::const_reverse_iterator TNodeList_CRI;


public:
    /// Construct an empty node (null parent)
    CTreeModelNode() : m_ParentNode(Null()) {}
    
    /// Set index of nodes parent
    ///
    /// @param 
    ///    parent_idx  index of parent node in the array
    void SetParent(TTreeIdx parent_idx) {m_ParentNode = parent_idx;}
    
    /// Get node's parent
    ///
    /// @return index of parent node or null index if root
    TTreeIdx GetParent() const { return m_ParentNode; }
    
    /// Set the parent index to Null() e.g. for root node
    void RemoveParent() { m_ParentNode = Null(); }
    
    /// Check if the node has a parent
    ///
    /// @return index of parent node or null index if root
    bool HasParent() const { return (m_ParentNode != Null()); }
    
    /// Add a child node
    ///
    /// @param 
    ///    child_idx  index of parent node in the array
    void AddChild(TTreeIdx child_idx);

    /// Remove child node if it is a child of this node
    ///
    /// @param 
    ///    child_idx  index of child node to be removed
    void RemoveChild(TTreeIdx child_idx);

    /// Check if another node is a child of this node
    ///
    /// @param 
    ///    child_idx  index of node to be checked
    /// @return true if child_idx is a child of this node
    bool HasChild(TTreeIdx child_idx) const;

    /// Static function that returns the null value.  NULL_TREE_IDX
    /// can also be used as the null value.
    ///
    /// @return the numeric index value used to represent NULL
    static TTreeIdx Null();
    
    /// Report whether this is a leaf node
    ///
    /// @return TRUE if this is a leaf node (has no children),
    /// false otherwise
    bool IsLeaf() const { return m_ChildNodes.empty(); };

    /// Return the indices of this node's child nodes.
    ///
    /// @return list of the children of this node
    TNodeList& GetChildren() { return m_ChildNodes; }
    const TNodeList& GetChildren() const { return m_ChildNodes; }    

    /// Return const iterator to first subnode index.
    TNodeList_CI SubNodeBegin() const { return m_ChildNodes.begin(); }

    /// Return iterator to first subnode index.
    TNodeList_I SubNodeBegin() { return m_ChildNodes.begin(); }

    /// Return const iterator to end of subnode list
    TNodeList_CI SubNodeEnd() const { return m_ChildNodes.end(); }

    /// Return iterator to end of subnode list
    TNodeList_I SubNodeEnd() { return m_ChildNodes.end(); }

    /// Return const reverse iterator to (reverse) of begin of subnode array
    TNodeList_CRI SubNodeRBegin() const { return m_ChildNodes.rbegin(); }

    /// Return reverse iterator to (reverse) begin of subnode array
    TNodeList_RI SubNodeRBegin() { return m_ChildNodes.rbegin(); }

    /// Return const reverse iterator to (reverse) end of subnode array
    TNodeList_CRI SubNodeREnd() const { return m_ChildNodes.rend(); }

    /// Return reverse iterator to (reverse) end of subnode array
    TNodeList_RI SubNodeREnd() { return m_ChildNodes.rend(); }

    /// Remove connections to parent and children of this node
    void ClearConnections() { m_ChildNodes.clear(); m_ParentNode = Null(); }

    /// Set the value-object for the node
    void SetValue(const TData& data) { m_Data = data; }
    /// Return the value object for the node
    TData& GetValue() { return m_Data; }
    const TData& GetValue() const { return m_Data; }

    /// Return the value object for the node using de-referenceing semantics
    TData& operator*() { return m_Data; }
    const TData& operator*() const { return m_Data; }

    TData* operator->() { return &m_Data; }
    const TData* operator->() const { return &m_Data; }

protected:
    /// Index of parent node to this node. ==Null() if this is the root node
    TTreeIdx m_ParentNode;
    /// Indices of all the children of this node
    vector<TTreeIdx> m_ChildNodes;
    /// Data object
    TData m_Data;
};


template<class TNode>
class CTreeModel : public CObject
{
public:   
    typedef TNode TNodeType;
    typedef typename std::vector<TNodeType> TNodeVecType;
    typedef typename TNode::TValueType TValueType;
    typedef typename TNode::TTreeIdx TTreeIdx;

    typedef typename TNodeType::TNodeList TNodeList;
    typedef typename TNodeType::TNodeList_I TNodeList_I;
    typedef typename TNodeType::TNodeList_CI TNodeList_CI;
    typedef typename TNodeType::TNodeList_RI TNodeList_RI;
    typedef typename TNodeType::TNodeList_CRI TNodeList_CRI;

public:
    /// Create empty tree. Tree is not valid at this point (no nodes)
    CTreeModel() : m_RootIdx(TTreeIdx(0)) {}
    virtual ~CTreeModel() {}

    /// Remove all nodes (empty array) and set root index to Null
    void Clear();

    /// Return a reference to the node at the given index
    ///
    /// @param 
    ///    idx  index of the node to be returned
    /// @return reference to the node at index 'idx'
    TNodeType& GetNode(TTreeIdx idx) { return m_Nodes[size_t(idx)]; }
    const TNode& GetNode(TTreeIdx idx) const { return m_Nodes[size_t(idx)]; }

    /// Use operator[] to return a reference to the node at 'idx'.
    TNodeType& operator[](TTreeIdx idx) { return m_Nodes[size_t(idx)]; }
    const TNode& operator[](TTreeIdx idx) const { return m_Nodes[size_t(idx)]; }


    /// Get the number of nodes currently in the array
    ///
    /// @return - the number of nodes (but some may be collapsed)
    size_t GetSize() const { return m_Nodes.size(); }

	/// Get the number of displayed nodes in current tree layout
	///
	/// @return - the number of displayed (active) nodes
	size_t GetNumNodes() const { return m_NumNodes; }
	
	/// Set the number of displayed nodes in current tree layout
	///
	/// @param 
	///    count - number of nodes
	void SetNumNodes(int count)  { m_NumNodes=count; }


    /// Return a reference to the parent node of the given node. Throws 
    /// exception if the given node is root
    ///
    /// @param 
    ///    node - the node whose parent is to be returned
    /// @return - reference to node's parent, if node is not root
    TNodeType& GetParent(TNodeType& node);
    const TNodeType& GetParent(TNodeType& node) const;

    /// Return a reference to the 'value' object of a node
    ///
    /// @param 
    ///    idx - index of the node
    /// @return - reference to the value object held by the node at 'idx'
    TValueType& GetNodeValue(TTreeIdx idx) 
        { return m_Nodes[size_t(idx)].GetValue(); }
    const TValueType& GetNodeValue(TTreeIdx idx) const 
        { return m_Nodes[size_t(idx)].GetValue(); }

    /// Return a reference to the root node of the tree
    ///
    /// @return - reference to the root node
    TNodeType& GetRoot() { return m_Nodes[m_RootIdx]; }
    const TNode& GetRoot() const { return m_Nodes[m_RootIdx]; }

    /// Set the index of the root node of the tree. This is initially
    /// 0 but the root index can move around with sorting, edits etc.
    ///
    /// @param
    ///    idx - the index for the tree's root node 
    void SetRootIdx(TTreeIdx idx) { m_RootIdx = idx; }

    /// Return the index of the root node
    ///
    /// @return the root node index
    TTreeIdx GetRootIdx() const { return m_RootIdx; }

    /// Remove the node at 'child_idx' from its parent 'parent_idx'
    /// Nothing is done if the node 'child_idx' is not a child of that parent
    ///
    /// @param
    ///    parent_idx - the index of the parent node
    ///    child_idx - the index of the child node
    void RemoveChild(TTreeIdx parent_idx, TTreeIdx child_idx);
    
    /// Add the node at 'child_idx' to the children 'parent_idx'.
    /// This does not check to see if the node is already a child.
    ///
    /// @param
    ///    parent_idx - the index of the parent node
    ///    child_idx - the index of the child node
    void AddChild(TTreeIdx parent_idx, TTreeIdx child_idx);

    /// Sets the root idx to be 'idx' and updates the tree so that
    /// all nodes above the new root become children of that node
    ///
    /// @param
    ///    idx - the new root node  
    void ReRoot(TTreeIdx idx);

    /// Add a new default node to the tree and return its index
    ///
    /// @return - the index of the new node
    TTreeIdx AddNode();

    /// Add a copy of node 'node' to the tree and return its index
    ///
    /// @param
    ///    node - the node from which to copy the new node
    /// @return - the index of the new node
    TTreeIdx AddNode(const TNode& node);

    /// Allocate the memory in advance, if you know how big the tree will be
    ///
    /// @param
    ///    target_size - the number of nodes that will be in the tree
    void Reserve(size_t target_size);

    /// Return the index value that represents a NULL node
    static TTreeIdx Null();

protected:
    /// Convert parents of node_idx to be its children
    virtual void x_ConvertUpstream(TTreeIdx node_idx);

    /// The list of nodes in the tree
    TNodeVecType m_Nodes;

	/// Number of nodes in tree (not including collapsed/hidden nodes)
	int m_NumNodes;

    /// The index of the root node within the tree
    TTreeIdx m_RootIdx;
};

/////////////////////////////////////////////////////////////////////////////
//
//  Tree algorithms
//


/// Depth-first tree traversal algorithm.
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
Fun TreeDepthFirst(TTreeModel& tree_model, typename TTreeModel::TTreeIdx node_idx, Fun func)
{
    typedef typename TTreeModel::TNodeType TNodeType;

    int delta_level = 0;
    ETreeTraverseCode stop_scan;

    stop_scan = func(tree_model, node_idx, delta_level);
    switch (stop_scan) {
        case eTreeTraverseStop:
        case eTreeTraverseStepOver:
            return func;
        case eTreeTraverse:
            break;
    }

    delta_level = 1;
    TNodeType* tr = &tree_model[node_idx];

    typedef typename TNodeType::TNodeList_I TTreeNodeIterator;

    TTreeNodeIterator it = tr->SubNodeBegin();
    TTreeNodeIterator it_end = tr->SubNodeEnd();

    if (it == it_end)
        return func;

    stack<TTreeNodeIterator> tree_stack;

    while (true) {
        tr =  &tree_model[*it];
        stop_scan = func(tree_model, *it, delta_level);
        switch (stop_scan) {
            case eTreeTraverseStop:
                return func;
            case eTreeTraverse:
            case eTreeTraverseStepOver:
                break;
        }        
        if ( (stop_scan != eTreeTraverseStepOver) &&
             (delta_level >= 0) && 
             (!tr->IsLeaf())) {  // sub-node, going down
            tree_stack.push(it);
            it = tr->SubNodeBegin();
            it_end = tr->SubNodeEnd();
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
            it_end = tree_model[tr->GetParent()].SubNodeEnd();
            delta_level = -1;
            continue;
        }
        // same level 
        delta_level = 0;

    } // while

    func(tree_model, node_idx, -1);
    return func;
}


/// Calls TreeDepthFirst with the root node of 'tree_model'
///
template<class TTreeModel, class Fun>
Fun TreeDepthFirst(TTreeModel& tree_model, Fun func)
{
    return TreeDepthFirst(tree_model, tree_model.GetRootIdx(), func);
}

/// Depth-first tree traversal which allows the traversed tree to update the
/// list of child nodes at the current node while iterating.  Otherwise 
/// identical to TreeDepthFirst
///
template<class TTreeModel>
struct NodeListIter {
    typedef typename TTreeModel::TNodeType TNodeType;
    typedef typename TTreeModel::TNodeList TNodeList;
    typedef typename TNodeType::TNodeList_I TTreeNodeIterator;

    NodeListIter(TNodeType* tr) 
        : m_node_list(tr->GetChildren()), m_it(m_node_list.begin()) {}

    NodeListIter(const NodeListIter& rhs) { *this = rhs; }

    NodeListIter& operator=(const NodeListIter& rhs) {
        size_t off = rhs.m_it - rhs.m_node_list.begin();
        m_node_list = rhs.m_node_list;
        m_it = m_node_list.begin() + off;
        return *this;
    }

    void SetNode(TNodeType* tr) 
        { m_node_list = tr->GetChildren(); m_it = m_node_list.begin(); }

    TNodeList m_node_list;
    TTreeNodeIterator m_it;     
};

template<class TTreeModel, class Fun>
Fun TreeDepthFirstInvarient(TTreeModel& tree_model, typename TTreeModel::TTreeIdx node_idx, Fun func)
{
    typedef typename TTreeModel::TNodeType TNodeType;
    typedef typename TNodeType::TNodeList_I TTreeNodeIterator;

    int delta_level = 0;
    ETreeTraverseCode stop_scan;

    stop_scan = func(tree_model, node_idx, delta_level);
    switch (stop_scan) {
        case eTreeTraverseStop:
        case eTreeTraverseStepOver:
            return func;
        case eTreeTraverse:
            break;
    }

    delta_level = 1;
    TNodeType* tr = &tree_model[node_idx];

    NodeListIter<TTreeModel> node_iter(tr);

    TTreeNodeIterator it_end = node_iter.m_node_list.end();

    if (node_iter.m_it == it_end)
        return func;

    stack<NodeListIter<TTreeModel> > tree_stack;

    while (true) {
        tr =  &tree_model[*node_iter.m_it];       
        stop_scan = func(tree_model, *node_iter.m_it, delta_level);
        switch (stop_scan) {
            case eTreeTraverseStop:
                return func;
            case eTreeTraverse:
            case eTreeTraverseStepOver:
                break;
        }
        if ( (stop_scan != eTreeTraverseStepOver) &&
             (delta_level >= 0) && 
             (!tr->IsLeaf())) {  // sub-node, going down
            tree_stack.push(node_iter);
            node_iter.SetNode(tr);
            //it = tr->SubNodeBegin();
            it_end = node_iter.m_node_list.end();
            delta_level = 1;
            continue;
        }
        ++node_iter.m_it;
        if (node_iter.m_it == it_end) { // end of level, going up
            if (tree_stack.empty()) {
                break;
            }
            node_iter = tree_stack.top();
            tree_stack.pop();
            tr =  &tree_model[*node_iter.m_it];
            it_end = node_iter.m_node_list.end();
            delta_level = -1;
            continue;
        }
        // same level 
        delta_level = 0;

    } // while

    func(tree_model, node_idx, -1);
    return func;
}

/// Calls TreeDepthFirstInvarient with the root node of 'tree_model'
///
template<class TTreeModel, class Fun>
Fun TreeDepthFirstInvarient(TTreeModel& tree_model, Fun func)
{
    return TreeDepthFirstInvarient(tree_model, tree_model.GetRootIdx(), func);
}

///////////////////////////////////////////////////////////////////////////////
/// Breadth-first tree traversall 
///
/// Traverse the tree in breadth-first order
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
///
/// @sa ETreeTraverseCode
template<class TTreeModel, class Fun>
Fun TreeBreadthFirst(TTreeModel& tree_model, typename TTreeModel::TTreeIdx node_idx, Fun func)
{
    typedef typename TTreeModel::TNodeType TNodeType;
    typedef typename TTreeModel::TTreeIdx  TTreeIdx;

    int delta_level = 0;
    ETreeTraverseCode stop_scan;

    stop_scan = func(tree_model, node_idx, delta_level);
    switch (stop_scan) {
        case eTreeTraverseStop:
        case eTreeTraverseStepOver:
            return func;
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

        TTreeNodeIterator it = tr->SubNodeBegin();
        TTreeNodeIterator it_end = tr->SubNodeEnd();
        for (; it != it_end; ++it) {           
            stop_scan = func(tree_model, *it, delta_level);
            switch (stop_scan) {
                case eTreeTraverseStop:
                    return func;
                case eTreeTraverse:
                case eTreeTraverseStepOver:
                        break;
            }            
            if (stop_scan != eTreeTraverseStepOver) {  // sub-node, queue for next pass
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
    return func;
}

/// Calls TreeBreadthFirst with the root node of 'tree_model'
///
template<class TTreeModel, class Fun>
Fun TreeBreadthFirst(TTreeModel& tree_model, Fun func)
{
    return TreeBreadthFirst(tree_model, tree_model.GetRootIdx(), func);
}



/////////////////////////////////////////////////////////////////////////////
//
//  CTreeModelNode<TData> Implementation
//

template <class TData>
typename CTreeModelNode<TData>::TTreeIdx  CTreeModelNode<TData>::Null() { return NULL_TREE_IDX; }


template <class TData>
void CTreeModelNode<TData>::AddChild(TTreeIdx child_idx)
{
#ifdef _DEBUG
    if (HasChild(child_idx)) {
        LOG_POST("Trying to add duplicate child node: " << child_idx << " to another node");
        return;
    }
#endif

    m_ChildNodes.push_back(child_idx);
}

template <class TData>
void CTreeModelNode<TData>::RemoveChild(TTreeIdx child_idx)
{
    vector<TTreeIdx>::iterator iter = find(m_ChildNodes.begin(), m_ChildNodes.end(), child_idx);
    
    if (iter==m_ChildNodes.end()) {
        return;
    }

    m_ChildNodes.erase(iter);
}

template <class TData>
bool CTreeModelNode<TData>::HasChild(size_t child_idx) const
{
    return ( std::find(m_ChildNodes.begin(), m_ChildNodes.end(), child_idx) != m_ChildNodes.end());
}


/////////////////////////////////////////////////////////////////////////////
//
//  CTreeModel<TNode> Implementation
//


template <class TNode>
typename CTreeModel<TNode>::TTreeIdx  CTreeModel<TNode>::Null() { return NULL_TREE_IDX; }

template<class TNode>
void CTreeModel<TNode>::Clear()
{
    m_Nodes.clear();
    m_NumNodes = 0;

    m_RootIdx = Null();
}

template<class TNode>
typename CTreeModel<TNode>::TNodeType& CTreeModel<TNode>::GetParent(TNodeType& node)
{
    if (node.GetParent() == TNodeType::Null())
         NCBI_THROW(CException, eUnknown, "Attempt to retrieve NULL parent in tree");

    return m_Nodes[node.GetParent()];
}

template<class TNode>
const typename CTreeModel<TNode>::TNodeType& CTreeModel<TNode>::GetParent(TNodeType& node) const
{
    if (node.GetParent() == TNodeType::Null())
         NCBI_THROW(CException, eUnknown, "Attempt to retrieve NULL parent in tree");

    return m_Nodes[node.GetParent()];
}

template<class TNode>
void CTreeModel<TNode>::RemoveChild(TTreeIdx parent_idx, TTreeIdx child_idx)
{
    m_Nodes[size_t(parent_idx)].RemoveChild(child_idx);
    m_Nodes[size_t(child_idx)].SetParent(Null());
}

template<class TNode>
void CTreeModel<TNode>::AddChild(TTreeIdx parent_idx, TTreeIdx child_idx)
{
    m_Nodes[size_t(parent_idx)].AddChild(child_idx);
    m_Nodes[size_t(child_idx)].SetParent(parent_idx);
}

template<class TNode>
void CTreeModel<TNode>::ReRoot(TTreeIdx idx)
{
    if (m_RootIdx == idx)
        return;
    
    if (idx >= m_Nodes.size())
        return;

    x_ConvertUpstream(idx);

    m_RootIdx = idx;
}

template<class TNode>
void CTreeModel<TNode>::x_ConvertUpstream(TTreeIdx node_idx)
{
    TTreeIdx parent_idx = m_Nodes[node_idx].GetParent();

    if (parent_idx != Null()) { 
        RemoveChild(parent_idx, node_idx);
        x_ConvertUpstream(parent_idx);
        AddChild(node_idx, parent_idx);
    }
}


template<class TNode>
typename CTreeModel<TNode>::TTreeIdx CTreeModel<TNode>::AddNode()
{
    m_Nodes.push_back(TNode());    
    size_t idx = m_Nodes.size()-1;

    return TTreeIdx(idx);
}

template<class TNode>
typename CTreeModel<TNode>::TTreeIdx CTreeModel<TNode>::AddNode(const TNode& node)
{
    size_t idx = m_Nodes.size();

    m_Nodes.push_back(node);
    return TTreeIdx(idx);
}

template<class TNode>
void CTreeModel<TNode>::Reserve(size_t target_size)
{
    // ensure size is at least equal to target size
    m_Nodes.reserve(target_size);
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_PHY_TREE___TREE_MODEL__HPP
