/*  $Id: phylo_tree_algorithm.cpp 38346 2017-04-27 13:24:57Z falkrb $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistd.hpp>

#include <algorithm>
#include <functional>
#include <cmath>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_algorithm.hpp>


#include <algo/phy_tree/bio_tree.hpp>
#include <algo/phy_tree/bio_tree_conv.hpp>
#include <util/bitset/bitset_debug.hpp>
#include <util/bitset/ncbi_bitset.hpp>

#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/biotree/FeatureDescr.hpp>
#include <objects/biotree/FeatureDictSet.hpp>
#include <objects/biotree/NodeFeatureSet.hpp>
#include <objects/biotree/NodeFeature.hpp>
#include <objects/biotree/NodeSet.hpp>
#include <objects/biotree/Node.hpp>

BEGIN_NCBI_SCOPE


// IPhyloTreeVisitor
ETreeTraverseCode IPhyloTreeVisitor::operator()(TTreeType& tree, TTreeType::TTreeIdx tree_node, int delta)
{
    return x_OnStep(tree_node, delta);
}

ETreeTraverseCode IPhyloTreeVisitor::x_OnStep(TTreeIdx x_node, int delta)
{
    switch (delta) {
    case 0:  return x_OnStepDown(x_node);
    case 1:  return x_OnStepRight(x_node);
    case -1: return x_OnStepLeft(x_node);
    default: break;
    }
    return eTreeTraverse;
}

ETreeTraverseCode IPhyloTreeVisitor::x_OnStepDown(TTreeIdx /* x_node */)
{
    return eTreeTraverse;
}

ETreeTraverseCode IPhyloTreeVisitor::x_OnStepRight(TTreeIdx /* x_node */)
{
    return eTreeTraverse;
}

ETreeTraverseCode IPhyloTreeVisitor::x_OnStepLeft(TTreeIdx /* x_node */)
{
    return eTreeTraverse;
}



// CPhyloTreeCalculator
CPhyloTreeCalculator::CPhyloTreeCalculator(TTreeType* tree, 
                                           CRgbaGradColorTable* ct)
    : IPhyloTreeVisitor(tree)
{
    Init(ct);
}

void CPhyloTreeCalculator::Init(CRgbaGradColorTable* ct)
{
    m_PamlID = 0;
    m_ID   = 0;
    m_NodeCount = 0;
    m_VisibleNodes = 0;
    m_PosX = 0;
    m_PosY = 0;
    m_Leaves = 0;
    m_Childs = 0;
    m_Width = 0;
    m_Height = 0;
    m_MaxDistance = 0;
    m_MinDistance = 0;
    m_MaxLabelLen = 0;
    m_Clusters.clear();
    m_MaxClusterID = (TClusterID)-1;
    m_DistFromRoot = 0;
    m_LastDist = 0;
    m_CollapsedParentIdx = CPhyloTreeNode::Null();
    m_AttrTable = NULL;
    m_AttrKeyId = -1;
    m_LabelFormat = "";
    m_ColorTable = ct;
}

void  CPhyloTreeCalculator::DumpStats()
{
    _TRACE("Calc Results:");
    _TRACE("GetNumNodes(): " << m_NodeCount );
    _TRACE("GetVisibleNodes(): " << m_VisibleNodes );
    _TRACE("GetNumEdges(): " << m_NodeCount-1 );
    _TRACE("GetWidth(): " << m_Width );
    _TRACE("GetHeight(): " << m_Height );
    _TRACE("GetMaxDistance(): " << m_MaxDistance );
    _TRACE("GetMinDistance(): " << m_MinDistance );
    _TRACE("GetMaxLabelLen(): " << m_MaxLabelLen );
}

void CPhyloTreeCalculator::SetAttrTable(const TAttrTable  & attr) 
{ 
    // Just use first column as key (we used to look for a seq-id, but
    // users wanted more flexibility in key selection)
    unsigned int key_x = 0;
    m_AttrKeyName = attr.Column(0);    

    // Tree has to have attribute as feature..
    if (!m_Tree->GetFeatureDict().HasFeature(m_AttrKeyName)) {
        LOG_POST(Error << "Error - attribute file key: '" + m_AttrKeyName + "' not found in tree features.");
        return;
    }

    m_AttrTable = &attr;

    m_AttrKeys.clear();
    m_AttrKeys.reserve(attr.Rows());

    // Initialize a separate vector of sorted (usually seq-id) strings
    // from the table.  This can be used to look up table entries
    // much faster when node ids and table ids match via
    // string compare, or when the table entry is a shortened
    // version of the node entry, e.g. AAT66223 vs AAT66223.1
    for (unsigned int i=0; i<m_AttrTable->Rows(); i++) {
        const string&  id_str = m_AttrTable->GetCell(i, key_x);

        // Do not create an index for the entry if there is no seq-id
        if (id_str.empty()) continue;

        AttrKey akey(id_str, i);
        m_AttrKeys.push_back(akey);
    }

    std::sort(m_AttrKeys.begin(), m_AttrKeys.end());
    m_AttrKeyId = m_Tree->GetFeatureDict().GetId(m_AttrKeyName);
}

void CPhyloTreeCalculator::ClearAttrTable()
{
    m_AttrTable = NULL;
    m_AttrKeys.clear();
    m_AttrKeyId = -1;
}

void CPhyloTreeCalculator::x_UpdateProperties(TNodeType& node, int row_idx)
{
    /// Go through all the properties in 'row_idx' of the attribute table
    /// and either update the corresponding value in the node, or add 
    /// a new value if one doesn't already exist.  Skip first col since it is the id.
    for (unsigned int j=1; j<m_AttrTable->Cols(); ++j) {
        string attr_name = m_AttrTable->Column(j);
        string attr_value = m_AttrTable->GetCell(row_idx, j);

        (*node).SetFeature(m_Tree->GetFeatureDict(), attr_name, attr_value);

        if (attr_name == "label") {
            (*node).SetLabel(attr_value);
        }
    }

    (*node).InitFeatures(m_Tree->GetFeatureDict(), m_ColorTable);
}

ETreeTraverseCode CPhyloTreeCalculator::x_OnStep(TTreeIdx x_node, int delta)
{
    ETreeTraverseCode traverse_code =
        IPhyloTreeVisitor::x_OnStep(x_node, delta);

    TNodeType& node = (*m_Tree)[x_node];
    
    // Account for trees without distance parameter as well
    if (delta == -1) {
        m_DistFromRoot -= m_LastDist;

        float max_branch_dist = 0.0f;
        for (auto iter = node.GetChildren().begin(); iter != node.GetChildren().end(); ++iter) {
            max_branch_dist = std::max(max_branch_dist, (*m_Tree)[*iter]->GetDistance() + (*m_Tree)[*iter]->GetMaxChildDistance());
        }
        
        node->SetDistanceFromRoot(m_DistFromRoot);
        node->SetMaxChildDistance(max_branch_dist);
    }
    else if (delta==1 || delta==0) {            
        if (delta==1)
            m_DistFromRoot += node.GetValue().GetDistance();
        else if (delta==0 && node.HasParent()) {
            // If the root has a distance value, ignore it
            m_DistFromRoot = m_DistFromRoot - m_LastDist + 
                node.GetValue().GetDistance();
        }

        // If the optional attribute table has been provided, see if it has
        // an entry with a seq-id matching the current node and, if so, 
        // update/add current node properties with properties from the table.
        if (m_AttrTable && 
            m_AttrTable->Rows()>0) {

            //string key_id = "seq-id";
            //unsigned key_x = m_AttrTable->ColumnIdx(key_id);
            unsigned int key_x = -1;
            AttrKey comp_key;
            bool node_has_key = true;

            if (m_AttrKeyName == "seq-id") {
                if ((*node).GetSeqID().IsNull()) {
                    node_has_key = false;
                }
                else {
                    // We now use first column always so seq-ids in columns other than first
                    // will not be used.
                    key_x = m_AttrTable->ColumnIdx(m_AttrKeyName);
                    comp_key.m_IDStr = (*node).GetSeqID()->GetSeqIdString();
                }
            }
            else {
                // No seq-id - just try to use first column as the key
                key_x = 0;
                comp_key.m_IDStr = (*node).GetBioTreeFeatureList()[m_AttrKeyId];
            
                if (comp_key.m_IDStr == "")
                    node_has_key = false;
            }

            if (node_has_key) {
                bool found = false;

                // First try to look up the matching entry based on simple string 
                // comparison using the sorted array of table seq-ids placed in
                // m_AttrKeys.  If that fails, we do a full (inefficient) scan
                // of all table entries to try to find a match.
                std::vector<AttrKey>::iterator iter;

                iter = std::lower_bound(m_AttrKeys.begin(), m_AttrKeys.end(), 
                    comp_key);

                // iter is closet string match, not necssarily exact (but
                // we check it either way).
                if (iter != m_AttrKeys.end()) {
                    try {
                        // put more efficient check first...
                        if (comp_key.m_IDStr == (*iter).m_IDStr) {
                            found = true;                            
                        }
                        else if (m_AttrKeyName == "seq-id") {
                            objects::CSeq_id  sid((*iter).m_IDStr);
                            if ((*node).GetSeqID()->Match(sid))
                                found = true;                            
                        }

                        if (found)
                            x_UpdateProperties(node, (*iter).m_AttrTableIdx);                   
                    }
                    // May have errors from mal-formed seq-ids in file - ignore those.
                    catch(...)
                    {
                        found = false;
                    }
                }

                // Not found.  Check all table entries one at a time if key is a seq-id
                if (!found && m_AttrKeyName == "seq-id") {
                    for (unsigned i=0; i<m_AttrTable->Rows(); i++) {
                        try {
                            string seqid_str = m_AttrTable->GetCell(i, key_x);

                            // Ignore attribute rows without a key (seq-id).
                            // (could also use node-id potentially as a lookup - but 
                            //  it would have to be exported from the tree first)
                            if (seqid_str.empty()) continue;

                            objects::CSeq_id sid(seqid_str);

                            if ((*node).GetSeqID()->Match(sid)) {
                                x_UpdateProperties(node, i);
                                break;
                            }
                        }
                        // May have errors from mal-formed seq-ids in file - ignore those.
                        catch (...){break;}
                    }
                }
            }
        }

        /// Perform all other node calculations        
        if (node.HasParent()) {
            if (m_MaxDistance < m_DistFromRoot) {
                m_MaxDistance = m_DistFromRoot;
            }
            if (m_MinDistance==0 || (m_MinDistance > m_DistFromRoot)){
                m_MinDistance = m_DistFromRoot;
            }
        }
       
		(*node).SetNumLeaves(0);
		(*node).SetNumLeavesEx(0);
        
        // Under current algorithm, this will be same as ID
        ++m_NodeCount;

        if (m_CollapsedParentIdx == CPhyloTreeNode::Null())
            ++m_VisibleNodes;

        if (!(node).IsLeaf()) {
            (*node).SetPamlCounter(m_PamlID++);
        }

        // update label corresponding to format
        if (!m_LabelFormat.empty()) {
            (*node).SetLabel(m_PTL.GetLabelForNode(*m_Tree, node, m_LabelFormat));
            (*node).SetDisplayLabel("");
        }


        if (node.IsLeaf() && ((*node).GetLabel().length() > (unsigned) m_MaxLabelLen)) {
           m_MaxLabelLen = (*node).GetLabel().length();
        }       

        TClusterID clusterID = (*node).GetClusterID();

        // cluster ID
        if (clusterID != -1) {
            m_Clusters[clusterID].push_back(x_node);
            m_MaxClusterID = std::max(m_MaxClusterID, clusterID);
        }

        if ((*node).HasSelClusters()) {
            string sel_ids;
			vector<int> selection_clusters = (*node).GetSelClusters();

            for (size_t i=0; i<selection_clusters.size(); ++i) {
                string sel_id;
                //NStr::IntToString(sel_id, selection_clusters[i]);
                m_Clusters[selection_clusters[i]].push_back(x_node);
                // selected nodes only show selection markers.  Note that any 
                // marker colors saved in node properties are still there and
                // will be displayed if node is not part of a selection set.
                (*node).GetMarkerColors().clear();
            }
        }
        
        // Disable all cluster colors for now. they will be reset
        // as needed in the CPhyloTreeDS::Clusterize() function
        (*node).SetClusterColorIdx(-1);
    }

    if (node.IsLeaf()) {
        node->SetDistanceFromRoot(m_DistFromRoot);
        node->SetMaxChildDistance(0.0f);
    }

    m_LastDist = node.GetValue().GetDistance();
    return traverse_code;
}


ETreeTraverseCode CPhyloTreeCalculator::x_OnStepDown(TTreeIdx x_node)
{
    TNodeType& node = (*m_Tree)[x_node];

    // Determine visibility by keeping track of whether any of the (recursive) parents
    // is collapsed.
    CPhyloTreeNode::TTreeIdx parent_idx = node.GetParent();
    if (m_CollapsedParentIdx == CPhyloTreeNode::Null()) {
        if (parent_idx != CPhyloTreeNode::Null() && !(*m_Tree)[parent_idx].Expanded()) {
            m_CollapsedParentIdx = parent_idx;
        }
    }

    // add 1 if node is visible (does not have a collapsed parent)
    m_PosY += ((m_CollapsedParentIdx == CPhyloTreeNode::Null()) ? 1 : 0);

    (*node).IDX().first  = m_PosX;
    (*node).IDX().second = m_PosY;

    // adjusting height
    if (m_Height < m_PosY) { m_Height = m_PosY; }

    return eTreeTraverse;
}

ETreeTraverseCode CPhyloTreeCalculator::x_OnStepRight(TTreeIdx x_node)
{
    TNodeType& node = (*m_Tree)[x_node];

    // Determine visibility by keeping track of whether any of the (recursive) parents
    // is collapsed.
    CPhyloTreeNode::TTreeIdx parent_idx = node.GetParent();
    if (m_CollapsedParentIdx == CPhyloTreeNode::Null()) {
        if (parent_idx != CPhyloTreeNode::Null() && !(*m_Tree)[parent_idx].Expanded()) {
            m_CollapsedParentIdx = parent_idx;
        }
    }

    // add 1 if node is visible (does not have a collapsed parent)
    m_PosX += (m_CollapsedParentIdx == CPhyloTreeNode::Null()) ? 1 : 0;

    // position
    (*node).IDX().first  = m_PosX;
    (*node).IDX().second = m_PosY;
    
    // adjusting width
    if (m_Width < m_PosX) { m_Width = m_PosX; }

    return eTreeTraverse;
}

ETreeTraverseCode CPhyloTreeCalculator::x_OnStepLeft(TTreeIdx x_node)
{
   TNodeType& node = (*m_Tree)[x_node];

   // Track visibility
   if (!node.Expanded() && m_CollapsedParentIdx == x_node) {
       m_CollapsedParentIdx = CPhyloTreeNode::Null();
   }

    // Add 1 if node is visible (does not have a collapsed parent)
    m_PosX -= (( (m_CollapsedParentIdx == CPhyloTreeNode::Null()) && node.Expanded()) ? 1 : 0);

    // number of leaves/childs calculation
    for (TTreeType::TNodeList_I it = node.SubNodeBegin();
            it != node.SubNodeEnd();
            it++){
        TNodeType& nn = (*m_Tree)[*it];

        // our leaves
        if (node.Expanded()){
            if (nn.IsLeafEx()){
				(*node).SetNumLeavesEx((*node).GetNumLeavesEx() + 1);
            }
            else {
				(*node).SetNumLeavesEx((*node).GetNumLeavesEx() + (*nn).GetNumLeavesEx());
            }
        }

        // 'real leaves'
        if (nn.IsLeaf()){
			(*node).SetNumLeaves((*node).GetNumLeaves() + 1);
        }
        else {
			(*node).SetNumLeaves((*node).GetNumLeaves() + (*nn).GetNumLeaves());
        }        
    }

    return eTreeTraverse;
}


ETreeTraverseCode CPhyloTreeRectCalculator::x_OnStep(TTreeIdx node_idx, int delta)
{
    TNodeType& node = (*m_Tree)[node_idx];

    if (delta == 0 || delta == 1) {
        const CPhyloNodeData::TPoint & xy = (*node).XY();
        /// m_Rect.IsEmpty() returns true even when rect is line
        if ((m_Rect.Left()==m_Rect.Right()) && (m_Rect.Top()==m_Rect.Bottom())){
            m_Rect.Init(xy.X()-1e-04, xy.Y()-1e-04, xy.X()+1e-04, xy.Y()+1e-04);
        }
        else {
            if (m_Rect.Left() > xy.X())    m_Rect.SetLeft(xy.X());
            if (m_Rect.Right() < xy.X())   m_Rect.SetRight(xy.X());
            if (m_Rect.Bottom() > xy.Y()) m_Rect.SetBottom(xy.Y());
            if (m_Rect.Top() < xy.Y())    m_Rect.SetTop(xy.Y());  
        }
    }

    if (!node.Expanded())
        return eTreeTraverseStepOver;

    return eTreeTraverse;
}

void CPhyloTreeRectCalculator::Init()
{
    m_Rect.Init();
}

ETreeTraverseCode CPhyloTreePriorityNode::x_OnStep(TTreeIdx node_idx, int delta)
{
    TNodeType& node = (*m_Tree)[node_idx];

    if (delta == 0 || delta == 1) {
        if (node.IsLeaf()) {
            // Get priority feature for current node, convert it to int, and then 
            // compare it to m_MaxPriority
            string priority_str = (*node).GetBioTreeFeatureList().GetFeatureValue(m_PriorityId);
            int priority = -1;

            if (priority_str != "") {
                try {
                    priority = NStr::StringToInt(priority_str);
                }
                catch (CStringException&) {
                    priority = -1;
                }
            }
            if (priority > m_MaxPriority) {
                m_MaxPriority = priority;
                m_MaxPriorityLeafNum = m_LeafCount;
                m_PriorityLeafIdx = node_idx;
            }
            // Choose the (leaf) node that is closer to the center leaf node
            // of the subtree as the maximum priority node
            else if (priority == m_MaxPriority) {
                if (std::abs(float(m_LeafCount) - float(m_LeafMidpoint)) < 
                    std::abs(float(m_MaxPriorityLeafNum) - float(m_LeafMidpoint))) {
                    m_MaxPriorityLeafNum = m_LeafCount;
                    m_PriorityLeafIdx = node_idx;
                }
            }

            ++m_LeafCount;
        }
    }

    if (!node.Expanded())
        return eTreeTraverseStepOver;

    return eTreeTraverse;
}

void CPhyloTreePriorityNode::Init(TTreeIdx node_idx)
{
    size_t num_leaves = (*m_Tree)[node_idx].GetValue().GetNumLeaves();
    m_LeafMidpoint = (((float)num_leaves) / 2.0f) - 0.5f;
    m_PriorityId = (*m_Tree).GetFeatureDict().GetId("$PRIORITY");
}

ETreeTraverseCode CPhyloTreeMaxIdCalculator::x_OnStep(TTreeIdx node_idx, int delta)
{
    TNodeType& node = (*m_Tree)[node_idx];

    if (delta == 0 || delta == 1) {
        m_Id = std::max(m_Id, (*node).GetId());
    }
    return eTreeTraverse;
}

ETreeTraverseCode CPhyloTreeSorter::x_OnStep(TTreeIdx node_idx, int delta)
{
    if (delta == 0 || delta == 1) {
        m_Tree->Sort(node_idx, m_Order);
    }

    return eTreeTraverse;
}

ETreeTraverseCode CPhyloTreeMaxDirectChildDist::x_OnStep(TTreeIdx node_idx, int delta)
{
    ETreeTraverseCode traverse_code =
        IPhyloTreeVisitor::x_OnStep(node_idx, delta);

    TNodeType& node = (*m_Tree)[node_idx];
    m_Distances[node_idx].m_NodeIdx = node_idx;
    float node_dist = std::max(0.0f, node.GetValue().GetDistance());

    if (delta == -1) {
        m_DistFromRoot -= m_LastDist;
        float max_child_dist = 0.0f;
        bool do_not_collapse = false;

        for (size_t i = 0; i < node.GetChildren().size(); ++i) {
            TNodeType& child_node = (*m_Tree)[node.GetChildren()[i]];
            float child_dist = std::max(0.0f, child_node->GetDistance());
            max_child_dist = std::max(max_child_dist, m_DistFromRoot + child_dist);

            if (!(*m_CheckCollapseFunc)(child_node))
                do_not_collapse = true;
        }

        if (do_not_collapse)
            max_child_dist = 0.0f;

        m_Distances[node_idx].m_MaxChildDist = max_child_dist;
        char buf[128];
        sprintf(buf, "%f", max_child_dist);
        (*node).SetFeature(m_Tree->GetFeatureDict(), "mcd", buf);
    }
    else if (delta == 1 || delta == 0) {
        if (delta == 1) {
            m_DistFromRoot += node_dist;
        }
        else if (delta == 0 && node.HasParent()) {
            // If the root has a distance value, ignore it
            m_DistFromRoot = m_DistFromRoot - m_LastDist + node_dist;
        }
    }
    if (node.IsLeaf()) {
        m_Distances[node_idx].m_MaxChildDist = -1.0f;
        char buf[128];
        sprintf(buf, "%f", m_DistFromRoot);
        (*node).SetFeature(m_Tree->GetFeatureDict(), "mcd", buf);
    }

    m_LastDist = node_dist;
    return traverse_code;
}

ETreeTraverseCode CPhyloTreeMaxChildDist::x_OnStep(TTreeIdx node_idx, int delta)
{
    ETreeTraverseCode traverse_code =
        IPhyloTreeVisitor::x_OnStep(node_idx, delta);

    TNodeType& node = (*m_Tree)[node_idx];
    
    if (delta==-1) {        
        m_DistFromRoot -= m_LastDist;
        float max_branch_dist = 0.0f;
        for (size_t i = 0; i < node.GetChildren().size(); ++i) {
            max_branch_dist = std::max(max_branch_dist, m_Distances[node.GetChildren()[i]]);
        }

        m_Distances[node_idx] = max_branch_dist;
    }
    else if (delta==1 || delta==0) {
        if (delta==1) {
            m_DistFromRoot += node.GetValue().GetDistance();
        }
        else if (delta==0 && node.HasParent()) {
            // If the root has a distance value, ignore it
            m_DistFromRoot = m_DistFromRoot - m_LastDist + 
                node.GetValue().GetDistance();
        }
    }
    if (node.IsLeaf()) {
        m_Distances[node_idx] = m_DistFromRoot;
        m_MaxDist = std::max(m_MaxDist, m_DistFromRoot);
        m_MinDist = std::min(m_MinDist, m_DistFromRoot);
    }

    m_LastDist = node.GetValue().GetDistance();
    return traverse_code;
}

ETreeTraverseCode CPhyloTreeDistFromRoot::x_OnStep(TTreeIdx node_idx, int delta)
{
    ETreeTraverseCode traverse_code =
        IPhyloTreeVisitor::x_OnStep(node_idx, delta);

    TNodeType& node = (*m_Tree)[node_idx];

    // Just for debugging (remove any highlighted path previously found)
    /*
    node.GetValue().RemoveFeature(
        m_Tree->GetFeatureDict(), "$NODE_COLOR");
    node.GetValue().InitFeatures(m_Tree->GetFeatureDict(), m_ColorTable);
    */

    if (delta == -1) {
        m_DistFromRoot -= m_LastDist;
        m_Distances[node_idx] = m_DistFromRoot;
    }
    else if (delta == 1 || delta == 0) {
        if (delta == 1) {
            m_DistFromRoot += node.GetValue().GetDistance();
            m_MaxBranchDist = m_DistFromRoot;
        }
        else if (delta == 0 && node.HasParent()) {
            // If the root has a distance value, ignore it
            m_DistFromRoot = m_DistFromRoot - m_LastDist +
                node.GetValue().GetDistance();
            m_MaxBranchDist = std::max(m_MaxBranchDist, m_DistFromRoot);
        }
    }
    if (node.IsLeaf()) {
        m_Distances[node_idx] = m_MaxBranchDist;

        /// Save idx of node with furthest distance from root
        if (m_MaxDistNode == CPhyloTree::Null() ||
            m_MaxBranchDist > m_Distances[m_MaxDistNode]) {
            m_MaxDistNode = node_idx;
        }
    }

    m_LastDist = node.GetValue().GetDistance();
    return traverse_code;
}

ETreeTraverseCode CPhyloTreeMidpointDist::x_OnStep(TTreeIdx node_idx, int delta)
{
    ETreeTraverseCode traverse_code =
        IPhyloTreeVisitor::x_OnStep(node_idx, delta);

    TNodeType& node = (*m_Tree)[node_idx];

    if (node.IsLeaf() && node_idx != m_MaxDistNode) {
        
        /// Compare distance from this leaf to m_MaxDistNode. For each leaf,
        /// find first common node with m_MaxDistPathToRoot. The distance 
        /// between the nodes is then the distance back to the common node.
        vector<TTreeIdx> path;
        TTreeIdx idx = node_idx;      

        float leaf_dist = (*m_Tree)[idx].GetValue().GetDistance();
        path.push_back(idx);

        bool found = false;
        while (!found) {
            TTreeIdx parent_idx = (*m_Tree)[idx].GetParent();

            // Check if current node is shared (if it is in the current node's
            // path-to-root).
            vector<TTreeIdx>::iterator iter = std::lower_bound(m_SortedMaxDistPathToRoot.begin(),
                m_SortedMaxDistPathToRoot.end(), parent_idx);

            idx = parent_idx;
            path.push_back(idx);

            // We don't add distance stored in lowest common ancestor since that
            // distance is from the lca to the lca's parent, and that's not part
            // of the path between the two leaf nodes.
            if (iter != m_SortedMaxDistPathToRoot.end() && *iter == idx)
                found = true;
            else
                leaf_dist += (*m_Tree)[idx].GetValue().GetDistance();
        }

        // idx should now be equal to the lowest common ancestor between
        // m_MaxDistNode and 'node'.  Total distance is then the distance
        // from 'idx' to 'node' minus the distance to 'idx' from the root
        // plus leaf_dist (distance from m_MaxDistNode to 'idx')
        float total_dist = (m_MaxDistFromRoot - m_Distances[idx]) + leaf_dist;
        if (total_dist > m_MaxDist) {
            m_MaxDist = total_dist;
            vector<TTreeIdx>::iterator iter2 = std::find(m_MaxDistPathToRoot.begin(),
                                                         m_MaxDistPathToRoot.end(), idx);
            m_MaxPath.clear();

            // The last node in path2 and the first node in m_PathToRoot is 
            // lowest common ancestor node. (pointed to by iter2).  Do not add
            // this to m_MaxPath twice.
            m_MaxPath.insert(m_MaxPath.end(), path.begin(), path.end());
            m_MaxPath.insert(m_MaxPath.end(), iter2 + 1, m_MaxDistPathToRoot.end());
        }
    }

    return traverse_code;
}

void CPhyloTreeMidpointDist::GetLongest(vector<TTreeIdx>& path, float& length)
{
    path = m_MaxPath;
    length = m_MaxDist;
}

ETreeTraverseCode CPhyloTreeLabelRange::x_OnStep(TTreeIdx node_idx, int delta)
{
    ETreeTraverseCode traverse_code =
        IPhyloTreeVisitor::x_OnStep(node_idx, delta);

    TNodeType& node = (*m_Tree)[node_idx];
    
    if (delta==-1) {
        // get min/max. special case for blank since otherwise it would always be low value
        TNodeType::TNodeList_CI iter = node.SubNodeBeginEx();
        for (; iter!=node.SubNodeEndEx(); ++iter) {
            TTreeIdx sub_node_idx = *iter;

            if ( m_LabelRanges[node_idx].first == "" )
                m_LabelRanges[node_idx].first = m_LabelRanges[sub_node_idx].first;
            else
                m_LabelRanges[node_idx].first = std::min(m_LabelRanges[node_idx].first, 
                                                         m_LabelRanges[sub_node_idx].first);
            m_LabelRanges[node_idx].second = std::max(m_LabelRanges[node_idx].second, 
                                                      m_LabelRanges[sub_node_idx].second);
        }
    }
    else if (delta==1 || delta==0) {
        if (node->GetLabel() != "") {
            m_LabelRanges[node_idx].first = node->GetLabel();
            m_LabelRanges[node_idx].second = node->GetLabel();
        }
    }

    return traverse_code;
}

ETreeTraverseCode CPhyloTreeSorterSubtreeDist::x_OnStep(TTreeIdx node_idx, int delta)
{
    if (delta == 0 || delta == 1) {
        m_Tree->SortSubtreeDist(node_idx, m_Distances, m_Order);
    }

    return eTreeTraverse;
}

ETreeTraverseCode CPhyloTreeSorterLabel::x_OnStep(TTreeIdx node_idx, int delta)
{
    if (delta == 0 || delta == 1) {
        m_Tree->SortLabel(node_idx, m_Order);
    }

    return eTreeTraverse;
}

ETreeTraverseCode CPhyloTreeSorterLabelRange::x_OnStep(TTreeIdx node_idx, int delta)
{
    if (delta == 0 || delta == 1) {
        m_Tree->SortLabelRange(node_idx, m_LabelRanges, m_Order);
    }

    return eTreeTraverse;
}



ETreeTraverseCode IPhyloTreeFilter::x_OnStep(TTreeIdx x_node, int delta)
{
    TNodeType& node = (*m_Tree)[x_node];

    bool allowed = x_Allowed(node);

    if (m_TreeStack.size() == 0) {
        // first node to begin conversion with
        if (allowed) {
            //auto_ptr<TTreeType> pnode(MakeNewTreeNode(x_node));
            m_TreeStack.push_back(x_node);
            m_Tree->SetRootIdx(x_node);
        }
        return eTreeTraverse;
    }

    // Mark disallowed nodes - use an invalid ID (-1)
    if (!allowed)
        node.GetValue().SetId(CPhyloNodeData::TID(-1));

    if (delta == 0) {
        TTreeIdx back_node = CPhyloTree::Null();

        if (m_TreeStack.size() > 0) {
            back_node = m_TreeStack.back();
            m_TreeStack.pop_back();
        }

        if (!allowed) {
            if (m_TreeStack.size() == 0)
                m_TreeStack.push_back(back_node);
            else
                m_TreeStack.push_back(m_TreeStack.back());
            return eTreeTraverse;
        }


        TTreeIdx parent_idx = m_TreeStack.back();

        // Only set parents when we visit leafs or we are going
        // back up the tree (delta == -1)
        if (node.IsLeaf())  {
            node.SetParent(parent_idx);

            // maintain order?
            TNodeType& parent = (*m_Tree)[parent_idx];
            if (!parent.HasChild(x_node))
                parent.AddChild(x_node);
        }


        m_TreeStack.push_back(x_node);
        return eTreeTraverse;
    }

    if (delta == 1) {
        // adjusting stack
        if (!allowed) {
            m_TreeStack.push_back(m_TreeStack.back());
            return eTreeTraverse;
        }

        TTreeIdx parent_idx = m_TreeStack.back();

        // Only set parents when we visit leafs or we are going
        // back up the tree (delta == -1)
        if (node.IsLeaf())  {
            node.SetParent(parent_idx);

            // maintain order?
            TNodeType& parent = (*m_Tree)[parent_idx];
            if (!parent.HasChild(x_node))
                parent.AddChild(x_node);
        }

        m_TreeStack.push_back(x_node);
        return eTreeTraverse;
    }
    if (delta == -1) {
        // returned to first node = stop traversal
        if (m_TreeStack.empty()) {
            return eTreeTraverseStop;
        }        
        m_TreeStack.pop_back();
        
        if (allowed) {
            if (m_TreeStack.size() > 1) {
                TTreeIdx parent_idx = m_TreeStack[m_TreeStack.size()-2];

                node.SetParent(parent_idx);

                // Does this maintain order?
                TNodeType& parent = (*m_Tree)[parent_idx];
                if (!parent.HasChild(x_node))
                    parent.AddChild(x_node);
            }
        }
        else {
            m_Tree->GetParent(node).RemoveChild(x_node);
            node.SetParent(CPhyloTree::Null());
        }
    }

    return eTreeTraverse;
}

END_NCBI_SCOPE
