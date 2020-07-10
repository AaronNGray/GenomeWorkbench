/*  $Id: phylo_tree.cpp 39527 2017-10-05 15:12:08Z falkrb $
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

#include <ncbi_pch.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/tree_graphics_model.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
// CPhyloTree
///////////////////////////////////////////////////////////////////////////////

CPhyloTree::CPhyloTree()
    : m_CurrentNode(Null())
{
    Clear();
}

void CPhyloTree::Clear()
{
    // Delete nodes and set root index to null
    CTreeModel<CPhyloTreeNode>::Clear();

    // Do not create a new model if it doesn't exist at this point
    // Model requires opengl so we don't want to accidently create
    // at startup (prior to context creation)
    if (!m_GraphicsModel.IsNull()) {
        m_GraphicsModel->ClearAll();
        m_GraphicsModel->Init(this);
    } 

    ClearSelection();
}

CRgbaGradColorTable* CPhyloTree::GetColorTable()
{ 
    if (m_ColorTable.IsNull()) {
        m_ColorTable.Reset(new CRgbaGradColorTable());
    }

    return m_ColorTable.GetPointer(); 
}


CTreeGraphicsModel& CPhyloTree::GetGraphicsModel() 
{ 
    if (m_GraphicsModel.IsNull()) {
        m_GraphicsModel.Reset(new CTreeGraphicsModel());
        m_GraphicsModel->Init(this);
    }    

    return m_GraphicsModel.GetObject(); 
}


class CNodeOrderPredNumChildren
{
public:
    CNodeOrderPredNumChildren(const CPhyloTree& m, bool order) 
    : m_Tree(m), m_Order(order) {}

    bool operator()(const CPhyloTree::TTreeIdx& lhs, 
                    const CPhyloTree::TTreeIdx& rhs) const {
        if (m_Order) {
			return m_Tree[lhs].GetValue().GetNumLeavesEx() < m_Tree[rhs].GetValue().GetNumLeavesEx();
        }
        else {
			return m_Tree[lhs].GetValue().GetNumLeavesEx() > m_Tree[rhs].GetValue().GetNumLeavesEx();
        }
    }

protected:
    const CPhyloTree& m_Tree;
    bool m_Order;
};

class CNodeOrderPredDist
{
public:
    CNodeOrderPredDist(const CPhyloTree& m, bool order) 
    : m_Tree(m), m_Order(order) {}

    bool operator()(const CPhyloTree::TTreeIdx& lhs, 
                    const CPhyloTree::TTreeIdx& rhs) const {   
        if (m_Order) {
            return  m_Tree[lhs].GetValue().GetDistance() < m_Tree[rhs].GetValue().GetDistance();
        }
        else {
            return  m_Tree[lhs].GetValue().GetDistance() > m_Tree[rhs].GetValue().GetDistance();
        }
    }

protected:
    const CPhyloTree& m_Tree;
    bool m_Order;
};

class CNodeOrderPredSubtreeDist
{
public:
    CNodeOrderPredSubtreeDist(const CPhyloTree& m, 
                              const vector<float>& distances, 
                              bool order) 
    : m_Tree(m), m_Distances(distances), m_Order(order) {}

    bool operator()(const CPhyloTree::TTreeIdx& lhs, 
                    const CPhyloTree::TTreeIdx& rhs) const { 

        if (m_Order) {
            return m_Distances[lhs] < m_Distances[rhs];
        }
        else {
            return m_Distances[lhs] > m_Distances[rhs];
        }
    }

protected:
    const CPhyloTree& m_Tree;
    const vector<float>& m_Distances;
    bool m_Order;
};

class CNodeOrderPredLabel
{
public:
    CNodeOrderPredLabel(const CPhyloTree& m, bool order) 
    : m_Tree(m), m_Order(order) {}

    bool operator()(const CPhyloTree::TTreeIdx& lhs, 
                    const CPhyloTree::TTreeIdx& rhs) const {
        const string &l1 =  m_Tree[lhs].GetValue().GetLabel();
		const string &l2 = m_Tree[rhs].GetValue().GetLabel();

        // Nodes with blank labels, including interior nodes, will come first.
        if (m_Order) {
            return l1 < l2;
        }
        else {
            return l1 > l2;
        }
    }

protected:
    const CPhyloTree& m_Tree;
    bool m_Order;
};

class CNodeOrderPredSubtreeLabel
{
public:
    CNodeOrderPredSubtreeLabel(const CPhyloTree& m, 
                               const vector<pair<string,string> >& labels,
                               bool order)
    : m_Tree(m), m_SubtreeLabels(labels), m_Order(order) {}

    bool operator()(const CPhyloTree::TTreeIdx& lhs, 
                    const CPhyloTree::TTreeIdx& rhs) const {

        // There are multiple ways that subtree labels can be compared.  We are doing
        // the simplest one here of just picking the node whose label range has the 
        // alphabetically smallest value.
        if (m_Order) {
            return m_SubtreeLabels[lhs].first < m_SubtreeLabels[rhs].first;
        }
        else {
            return m_SubtreeLabels[lhs].first > m_SubtreeLabels[rhs].first;
        }
    }

protected:
    const CPhyloTree& m_Tree;
    // Each pair is the lexicographical min and max from a nodes subtree labels
    const vector<pair<string,string> >& m_SubtreeLabels;
    bool m_Order;
};



void CPhyloTree::Sort(TTreeIdx idx, bool order)
{
    CNodeOrderPredNumChildren  pred(*this, order);

    sort(m_Nodes[idx].GetChildren().begin(),
         m_Nodes[idx].GetChildren().end(), pred);
}

void CPhyloTree::SortSubtreeDist(TTreeIdx idx, const vector<float>& distances, bool order)
{
    CNodeOrderPredSubtreeDist  pred(*this, distances, order);

    sort(m_Nodes[idx].GetChildren().begin(),
         m_Nodes[idx].GetChildren().end(), pred);
}

void CPhyloTree::SortLabel(TTreeIdx idx, bool order)
{
    CNodeOrderPredLabel  pred(*this, order);

    sort(m_Nodes[idx].GetChildren().begin(),
         m_Nodes[idx].GetChildren().end(), pred);
}

void CPhyloTree::SortLabelRange(TTreeIdx idx, 
                                const vector<pair<string,string> >& subtree_labels, 
                                bool order)
{
    CNodeOrderPredSubtreeLabel  pred(*this, subtree_labels, order);

    sort(m_Nodes[idx].GetChildren().begin(),
         m_Nodes[idx].GetChildren().end(), pred);
}

void CPhyloTree::ClearSelection()
{ 
    TNodeVecType::iterator iter;

    for (iter=m_Nodes.begin(); iter!=m_Nodes.end(); ++iter) {
        (*iter).GetValue().SetSelectedState(CPhyloNodeData::eNotSelected);
    }

    m_Selected.clear(); 
    m_ExplicitlySelected.clear();

    m_CurrentNode = Null();

    m_CurrentEdgeChild = Null();
    m_CurrentEdgeParent = Null();
}

void CPhyloTree::GetSelected(vector<TTreeIdx>& sel) const
{
    sel.reserve(sel.size() + m_Selected.size());
    std::copy(m_Selected.begin(), m_Selected.end(), back_inserter(sel));
}

void CPhyloTree::GetSelectedIDs(vector<TID>& sel) const
{
    sel.reserve(sel.size() + m_Selected.size());
    ITERATE(TTreeIdxSet, it, m_Selected) {
        sel.push_back(m_Nodes[*it].GetValue().GetId());
    }
}

void CPhyloTree::GetExplicitlySelected(vector<TTreeIdx>& esel) const
{
    esel.reserve(esel.size() + m_ExplicitlySelected.size());
    std::copy(m_ExplicitlySelected.begin(), m_ExplicitlySelected.end(), back_inserter(esel));
}

void CPhyloTree::GetExplicitlySelectedAndNotCollapsed(vector<TTreeIdx>& esel) const
{
    esel.reserve(m_ExplicitlySelected.size());

    for (auto iter = m_ExplicitlySelected.begin(); iter != m_ExplicitlySelected.end(); ++iter) {
        // Check if any of the parent nodes of the current node, up to the root, are collapsed
        // and, if so, return the collapsed node closest to the root.
        const TNodeType& n = m_Nodes[*iter];
        TTreeIdx parent = n.GetParent();
        TTreeIdx ex_sel_node = *iter;

        while (parent != TNodeType::Null()) {
            const TNodeType& p = m_Nodes[parent];
            if (!p.Expanded())
                ex_sel_node = parent;
            parent = p.GetParent();
        }

        // If a node is a parent node, check if it's a duplicate before adding it to the array.
        // Most duplicates will be adjacent because trees indicies will almost always be 
        // in depth-first order
        if (ex_sel_node != *iter && esel.size() > 0) {
            if (esel[esel.size() - 1] != ex_sel_node) {
                if (std::find(esel.begin(), esel.end(), ex_sel_node) == esel.end()) {
                    esel.push_back(ex_sel_node);
                }
            }
        }
        else {
            esel.push_back(ex_sel_node);
        }
    }
}

void CPhyloTree::GetExplicitlySelectedIDs(vector<TID>& esel) const
{
    esel.reserve(esel.size() + m_ExplicitlySelected.size());
    ITERATE(TTreeIdxSet, it, m_ExplicitlySelected) {
        esel.push_back(m_Nodes[*it].GetValue().GetId());
    }
}

void CPhyloTree::SetExplicitlySelected(const vector<TID>& esel)
{
    m_ExplicitlySelected.clear();
    if (esel.size() == 0)
        return;

    // For each ID, find its corresponding index then check if it is in
    // the set of selected nodes. If it is, mark it as explicitly selected.
    ITERATE(vector<TID>, it, esel) {
        TTreeIdx idx = FindNodeById(*it);

        if (m_Selected.find(idx) != m_Selected.end())
            m_ExplicitlySelected.insert(idx);
    }
}

void CPhyloTree::SetLeavesExplicitlySelected()
{
    m_ExplicitlySelected.clear();
    ITERATE(TTreeIdxSet, it, m_Selected) {
        if (m_Nodes[*it].IsLeaf())
            m_ExplicitlySelected.insert(*it);
    }
}

void CPhyloTree::SetSelectionIDs(const vector<TID>& ids, bool sel, bool sel_children)
{
    TTreeIdx idx;
    ITERATE(vector<TID>,iter,ids) {
        idx = FindNodeById(*iter);
        if (Null() == idx)
            continue;
        SetSelection(idx, sel, sel_children);
    }
}

void CPhyloTree::SetSelection(const vector<TTreeIdx>& sel,    
                              bool sel_children,
                              bool sel_parents)
{
    vector<TTreeIdx>::const_iterator iter;

    for (iter=sel.begin(); iter!=sel.end(); ++iter) {    
        SetSelection(*iter, true, sel_children, sel_parents);
    }
}

vector<CPhyloTree::TTreeIdx> CPhyloTree::GetAllCollapsed() const
{ 
    vector<TTreeIdx> collapsed_nodes;

    for (size_t i=0; i<m_Nodes.size(); ++i) {
        if (m_Nodes[i]->GetDisplayChildren() == CPhyloNodeData::eHideChildren &&
            !m_Nodes[i].IsUnused()) {
                collapsed_nodes.push_back(TTreeIdx(i));
        }
    }

    return collapsed_nodes;
}

TTreeIdx CPhyloTree::GetCollapsedParentIdx(TTreeIdx node_idx) const
{
    TTreeIdx result = node_idx;

    CPhyloTree::TTreeIdx parent_idx = GetNode(node_idx).GetParent();
    while (parent_idx != CPhyloTreeNode::Null()) {
        if (!GetNode(parent_idx).Expanded()) {
            result = parent_idx;
        }
        parent_idx = GetNode(parent_idx).GetParent();
    }

    return result;
}

bool CPhyloTree::GetSelectedBoundary(CVect2<float>& ll, 
                                     CVect2<float>& ur)
{
    ll = numeric_limits<float>::max();
    ur = -numeric_limits<float>::max();

    ITERATE(TTreeIdxSet, iter, m_Selected) {
        CVect2<float> p = m_Nodes[*iter]->XY();
        ll.X() = std::min(ll.X(), p.X());
        ll.Y() = std::min(ll.Y(), p.Y());

        ur.X() = std::max(ur.X(), p.X());
        ur.Y() = std::max(ur.Y(), p.Y());
    }

    return m_Selected.size()>0;
}

void CPhyloTree::SetCurrentNode(TTreeIdx node_idx)
{
    m_CurrentNode = node_idx;
}

bool CPhyloTree::SetCurrentNode(const CVect2<float>& pt, bool labels_visible, bool rotated_labels,
    bool unselect)
{
    if (unselect)
        m_CurrentNode = Null();

    TTreeIdx new_current_node = TestForNode(pt, labels_visible, rotated_labels);
    if (new_current_node != Null()) {
        m_CurrentNode = new_current_node;
        return true;
    }
    return false;
}

CPhyloTreeNode& CPhyloTree::GetCurrentNode()  
{ 
    if (!HasCurrentNode())
         NCBI_THROW(CException, eUnknown, "Attempt to get current node from tree when there is none");
    
    return m_Nodes[m_CurrentNode]; 
}

void CPhyloTree::SetCurrentEdge(TTreeIdx child_idx, TTreeIdx parent_idx)
{
    m_CurrentEdgeChild = child_idx;
    m_CurrentEdgeParent = parent_idx;
}

void CPhyloTree::GetCurrentEdge(TTreeIdx& child_idx, TTreeIdx& parent_idx) const
{
    child_idx = m_CurrentEdgeChild;
    parent_idx = m_CurrentEdgeParent;
}

bool CPhyloTree::HasCurrentEdge() const
{
    // It should never be the case that only one of these is Null
    return (m_CurrentEdgeChild != Null() && m_CurrentEdgeParent != Null());
}

CPhyloTree::TTreeIdx CPhyloTree::TestForNode(const CVect2<float>& pt, bool labels_visible, bool rotated_labels)
{
    CPhyloTree::TTreeIdx idx(TNodeType::Null());

    idx = GetGraphicsModel().GetCollisionData().TestForNode(pt.X(), pt.Y(), labels_visible, rotated_labels);

    return idx;
}

bool CPhyloTree::IsVisible(TTreeIdx node_idx)
{
    TNodeType* node = &m_Nodes[node_idx];
    TNodeType* parent = NULL;
    TTreeIdx parent_idx;

    while ((parent_idx=node->GetParent())!=TNodeType::Null()) {
        parent = &m_Nodes[parent_idx];
        if ((**parent).GetDisplayChildren()==CPhyloNodeData::eHideChildren){
            return false;
        }
        node = parent;
    }

    return true;
}

bool CPhyloTree::SelectByPoint(const CVect2<float>& pt, bool sel, bool toggle, 
    bool labels_visible, bool rotated_labels)
{
    TTreeIdx idx = TestForNode(pt, labels_visible, rotated_labels);
        
    if (idx != Null()) {
        CPhyloTreeNode& node = m_Nodes[idx];

        // If it is not an incremental selection and we clicked on a node, clear
        // previous selection
        if (sel) {
            ClearSelection();
        }

        if (toggle)
            SetSelection(idx, !((*node).GetSelectedState()==CPhyloNodeData::eSelected),  true);
        else
            SetSelection(idx, CPhyloNodeData::eSelected,  true);

        m_CurrentNode = idx;

        return true;
    }
    return false;
}

bool CPhyloTree::SelectByRect(const CVect2<float>& ll, 
                              const CVect2<float>& ur,  
                              bool toggle)
{
    vector<CPhyloTree::TTreeIdx> selected;

    selected = GetGraphicsModel().GetCollisionData().SelectNodes(ll.X(), ll.Y(), ur.X(), ur.Y());

    for (size_t i=0; i<selected.size(); ++i) {
        TNodeType& node = m_Nodes[size_t(selected[i])];

        bool sel;
        if (toggle) 
            sel = !((*node).GetSelectedState()==CPhyloNodeData::eSelected);
        else
            sel = true;

        SetSelection(selected[i], sel, true);
    }    

    return (selected.size() > 0);
}

void CPhyloTree::UpdateNodesMapping()
{
    m_NodesMap.clear();
    ITERATE(TNodeVecType, it, m_Nodes) {
        m_NodesMap[(*it).GetValue().GetId()] = (TTreeIdx)(it-m_Nodes.begin());
    }
}

void CPhyloTree::x_SetSelState(TTreeIdx idx,
                               CPhyloNodeData::TSelectedState state,
                               bool override_explicit)
{
    CPhyloTreeNode& node = m_Nodes[idx];

    if (state != CPhyloNodeData::eSelected &&
        (*node).GetSelectedState() == CPhyloNodeData::eSelected) {
            TTreeIdxSet::iterator sel_it = m_ExplicitlySelected.find(idx);
            if (sel_it != m_ExplicitlySelected.end()){
                // This allows us to not allow parent-selection/de-selection to 
                // remove selection of parent that was selected explicitly (e.g. 
                // clicked on directly or met results of a query)
                if (!override_explicit)
                    return;

                // And we also preserve explicitly selected nodes even if the
                // de-selection as a child node will now show the node as traced,
                // not selected.
                m_ExplicitlySelected.erase(sel_it);
            }
            sel_it = m_Selected.find(idx);    
            if (sel_it!=m_Selected.end()){
                m_Selected.erase(sel_it);
            }  
    }
    else if (state == CPhyloNodeData::eSelected &&  
             (*node).GetSelectedState() != CPhyloNodeData::eSelected) {            
                 m_Selected.insert(idx);
    }

    (*node).SetSelectedState(state);
}

void  CPhyloTree::SetSelection(TTreeIdx idx,
                               bool sel,
                               bool sel_children,
                               bool sel_parents)
{
    x_SetSelState(idx, sel ? CPhyloNodeData::eSelected :
                             CPhyloNodeData::eNotSelected);

    // This function is called for nodes user clicks on or selects through a query
    // - explicitly selected nodes so keep that set up-to-date here. Children
    // and parent nodes of the selected set may also be selected based on
    // sel_children/sel_parents.
    if (!sel) {
        TTreeIdxSet::iterator sel_it = m_ExplicitlySelected.find(idx);
        if (sel_it != m_ExplicitlySelected.end()){
            m_ExplicitlySelected.erase(sel_it);
        }
    }
    else {
        m_ExplicitlySelected.insert(idx);
    }
    
    CPhyloTreeNode& node = m_Nodes[idx];

    if (sel_children) {
        x_SelectChildren(node, sel);
    }

    if (sel_parents) {
        x_SelectParents(node, sel, sel_children);
        TTreeIdx common_idx = x_CommonNodeSearch(idx);
        if (common_idx != Null())
            x_PropagateCommonState(m_Nodes[common_idx]);
    }
}

void CPhyloTree::GetSelState(TSelState& sel_state,
                             TTreeIdx idx,
                             bool sel_children,
                             bool sel_parents) const
{

    const CPhyloTreeNode& node = m_Nodes[idx];

    if ((*node).GetSelectedState() == CPhyloNodeData::eSelected) {
        sel_state.push_back(TSelStateValue(idx, CPhyloNodeData::eSelected));
    } 
    // If a selected node is not a leaf, sometimes it is set as shared rather than selected,
    // so see if it might be in explicitly selected set too.
    else if (!node.IsLeaf()) {
        TTreeIdxSet::iterator sel_it = m_ExplicitlySelected.find(idx);
        if (sel_it != m_ExplicitlySelected.end()){
            sel_state.push_back(TSelStateValue(idx, CPhyloNodeData::eSelected));
        }
    }

    if (sel_children) {
        x_GetChildState(sel_state, idx);
    }

    if (sel_parents && node.HasParent()) {
        x_GetParentState(sel_state, node.GetParent());
    }
}

void  CPhyloTree::x_SelectChildren(CPhyloTreeNode& node,
                                   bool sel)
{     
    CPhyloTreeNode::TNodeList_I it;
    for (it=node.SubNodeBegin(); it != node.SubNodeEnd(); ++it) {
        TNodeType& child_node = m_Nodes[*it];
        x_SetSelState(*it, sel ? CPhyloNodeData::eSelected : 
                                 CPhyloNodeData::eNotSelected);

        x_SelectChildren(child_node, sel);
    }
}

void  CPhyloTree::x_SelectParents(CPhyloTreeNode& node, bool sel, bool children_selected)
{
    if (!node.HasParent())
        return;

    CPhyloTreeNode& parent = m_Nodes[node.GetParent()];

    bool all_selected = true;
    bool some_selected = false;

    CPhyloTreeNode::TNodeList_I  it;
    for (it = parent.SubNodeBegin(); it != parent.SubNodeEnd(); ++it ) {
        TNodeType& child_node = m_Nodes[*it];

        bool sel_state   = ((*child_node).GetSelectedState() == CPhyloNodeData::eSelected);
        bool trace_state = ((*child_node).GetSelectedState() != CPhyloNodeData::eNotSelected);

        all_selected &= sel_state;
        some_selected |= trace_state;
    }
    if (all_selected) {
        x_SetSelState(node.GetParent(), sel ? CPhyloNodeData::eSelected :
            CPhyloNodeData::eNotSelected);
    }
    else {
        // When we don't selecte children, selecting a group of nodes can cause previously
        // selected parents to be unselelected so we disable un-selection of parents in this case
        x_SetSelState(node.GetParent(), some_selected ? CPhyloNodeData::eTraced :
            CPhyloNodeData::eNotSelected, children_selected);
    }

    x_SelectParents(parent, sel, children_selected);
}

CPhyloTree::TTreeIdx CPhyloTree::x_CommonNodeSearch(TTreeIdx idx)
{
    // finding common node
    TTreeIdx common_idx = Null();
    TTreeIdx cursor_idx = idx;

    while (cursor_idx != Null()){
        int iMarkedNodes=0;
        CPhyloTreeNode& cursor = m_Nodes[cursor_idx];

        CPhyloTreeNode::TNodeList_I  it;
        for (it = cursor.SubNodeBegin(); it != cursor.SubNodeEnd(); ++it ) {
            if (m_Nodes[*it].GetValue().GetSelectedState()!=
                    CPhyloNodeData::eNotSelected) {
               ++iMarkedNodes;
            }
        }
        if (iMarkedNodes > 1) {
            common_idx = cursor_idx;
        }

        cursor_idx = cursor.GetParent();
    }

    if (common_idx != Null() && !m_Nodes[common_idx].IsLeaf())
        return common_idx;
    return Null();
}

void  CPhyloTree::x_PropagateCommonState(CPhyloTreeNode& node)
{
    if ((*node).GetSelectedState()==CPhyloNodeData::eTraced) {
        (*node).SetSelectedState(CPhyloNodeData::eShared);
            
        CPhyloTreeNode::TNodeList_I  it;
        for(it=node.SubNodeBegin(); it != node.SubNodeEnd(); ++it)  {
            x_PropagateCommonState(m_Nodes[*it]);
        }
    }
}

void  CPhyloTree::x_GetParentState(TSelState& sel_state,
                                   TTreeIdx idx) const
{
    const CPhyloTreeNode& node = m_Nodes[idx];

    if ((*node).GetSelectedState() != CPhyloNodeData::eNotSelected) {
        sel_state.push_back(TSelStateValue(idx, (*node).GetSelectedState()));

        if (node.HasParent())
            x_GetParentState(sel_state, node.GetParent());
    }
}

void  CPhyloTree::x_GetChildState(TSelState& sel_state,
                                  TTreeIdx idx) const
{
    const CPhyloTreeNode& node = m_Nodes[idx];

    CPhyloTreeNode::TNodeList_CI  it;
    for (it = node.SubNodeBegin(); it != node.SubNodeEnd(); ++it)  {
        const CPhyloTreeNode& n = m_Nodes[*it];
        if ((*n).GetSelectedState() != CPhyloNodeData::eNotSelected) {
            sel_state.push_back(TSelStateValue(*it, (*node).GetSelectedState()));
            x_GetChildState(sel_state, *it);
        }
    }
}

void CPhyloTree::x_ConvertUpstream(TTreeIdx node_idx)
{
    TTreeIdx parent_idx = m_Nodes[node_idx].GetParent();

    if (parent_idx != Null()) {
        RemoveChild(parent_idx, node_idx);
        x_ConvertUpstream(parent_idx);
        AddChild(node_idx, parent_idx);

        // Disance values are stored in child nodes as distance of the child from
        // the parent, so when we swap child and parent nodes, we need to move the 
        // distance from the former child to the former parent.        
        GetNode(parent_idx).GetValue().SetDistance(
            GetNode(node_idx).GetValue().GetDistance());
        GetNode(parent_idx).GetValue().Sync(m_FeatureDict);
    }
}


END_NCBI_SCOPE
