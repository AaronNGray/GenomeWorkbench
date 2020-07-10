/*  $Id: phylo_tree_ds.cpp 43693 2019-08-14 18:24:50Z katargir $
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
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>

#include <gui/widgets/phylo_tree/phylo_selection_set.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/biotree/FeatureDictSet.hpp>
#include <objects/biotree/FeatureDescr.hpp>
#include <objects/biotree/NodeSet.hpp>
#include <objects/biotree/Node.hpp>
#include <objects/biotree/NodeFeatureSet.hpp>
#include <objects/biotree/NodeFeature.hpp>

#include <util/xregexp/regexp.hpp>


BEGIN_NCBI_SCOPE


void CTreeLabel::InitializeFromUserObject(const CBioTreeContainer_Base::TUser& uo)
{
    if (uo.HasField("view-label") &&
        uo.GetField("view-label").GetData().IsObject()) {

            const CUser_field::C_Data::TObject& label_obj = uo.GetField("view-label").GetData().GetObject();

            if (label_obj.HasField("label") &&
                label_obj.GetField("label").GetData().IsStr()) {
                   m_Label = label_obj.GetField("label").GetData().GetStr();
            }

            if (label_obj.HasField("font-name") &&
                label_obj.GetField("font-name").GetData().IsStr()) {
                    m_FontName = label_obj.GetField("font-name").GetData().GetStr();
            }

            if (label_obj.HasField("font-size") &&
                label_obj.GetField("font-size").GetData().IsInt()) {
                    m_FontSize = label_obj.GetField("font-size").GetData().GetInt();
            }

            if (label_obj.HasField("posx") &&
                label_obj.GetField("posx").GetData().IsInt()) {
                    m_XPos = label_obj.GetField("posx").GetData().GetInt();
            }

            if (label_obj.HasField("posy") &&
                label_obj.GetField("posy").GetData().IsInt()) {
                    m_YPos = label_obj.GetField("posy").GetData().GetInt();
            }

            if (label_obj.HasField("color") &&
                label_obj.GetField("color").GetData().IsStr()) {
                    m_Color.FromString(label_obj.GetField("color").GetData().GetStr());
            }
    }
}

void CTreeLabel::SaveToUserObject(CBioTreeContainer_Base::TUser& uo)
{
    CRef<CUser_field::C_Data::TObject>  label_object;

    if (uo.HasField("view-label") && 
        uo.GetField("view-label").GetData().IsObject()) {
            label_object.Reset(&(uo.SetField("view-label").SetData().SetObject()));               
    }
    else {                      
        CUser_object* label_uo = new CUser_object();
        CRef<CObject_id> uo_id;
        uo_id.Reset(new CObject_id());
        uo_id->SetStr("label-parameters");
        label_uo->SetType(*uo_id);

        label_object.Reset(label_uo);
        uo.AddField("view-label", *label_uo);
    }    

    if (label_object->HasField("label") && 
        label_object->GetField("label").GetData().IsStr()) {
            label_object->SetField("label").SetData().SetStr(m_Label);
    }
    else {
        label_object->AddField("label", m_Label);
    }  

    if (label_object->HasField("font-name") && 
        label_object->GetField("font-name").GetData().IsStr()) {
            label_object->SetField("font-name").SetData().SetStr(m_FontName);
    }
    else {
        label_object->AddField("font-name", m_FontName);
    }  

    if (label_object->HasField("font-size") && 
        label_object->GetField("font-size").GetData().IsInt()) {
            label_object->SetField("font-size").SetData().SetInt(m_FontSize);
    }
    else {
        label_object->AddField("font-size", m_FontSize);
    } 

    if (label_object->HasField("posx") && 
        label_object->GetField("posx").GetData().IsInt()) {
            label_object->SetField("posx").SetData().SetInt(m_XPos);
    }
    else {
        label_object->AddField("posx", m_XPos);
    }  

    if (label_object->HasField("posy") && 
        label_object->GetField("posy").GetData().IsInt()) {
            label_object->SetField("posy").SetData().SetInt(m_YPos); 
    }
    else {
        label_object->AddField("posy", m_YPos);
    } 

    if (label_object->HasField("color") && 
        label_object->GetField("color").GetData().IsStr()) {
            label_object->SetField("color").SetData().SetStr(m_Color.ToString());
    }
    else {
        label_object->AddField("color", m_Color.ToString());
    }
}

CPhyloTree CPhyloTreeDataSource::m_sTreeClipboard;

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
CPhyloTreeDataSource::CPhyloTreeDataSource(const objects::CBioTreeContainer& tree, 
                                           objects::CScope& scope,
                                           bool expand_all)
: m_Calc(NULL)
{
    Init(tree, scope, expand_all);
}

CPhyloTreeDataSource::~CPhyloTreeDataSource()
{
    delete m_Calc;
    m_Calc = NULL;
}

void CPhyloTreeDataSource::Init(const objects::CBioTreeContainer& tree, 
                                objects::CScope& scope,
                                bool expand_all)
{
    // Make sure we have a valid container - it can't be empty.
    if (tree.GetNodeCount() == 0 ) {
        NCBI_THROW(CException, eUnknown, "Cannot initialize CPhyloTreeDataSource with an empty tree");
    }

    if (!m_TreeModel.IsNull())
        m_TreeModel->Clear();
    else
        m_TreeModel.Reset(new CPhyloTree());


    m_Scope.Reset(&scope);

    BioTreeConvertContainer2Tree(m_TreeModel.GetNCObject(), tree, &m_TreeModel->GetFeatureDict(), true, expand_all);
    m_TreeModel->UpdateNodesMapping();

    if (tree.IsSetUser()) {
        const CBioTreeContainer_Base::TUser& uo = tree.GetUser();
        m_TreeLabel.InitializeFromUserObject(uo);

        m_TreeModel->GetSelectionSets().Clear();
        m_TreeModel->GetSelectionSets().InitFromUserObject(m_TreeModel.GetPointer(), uo);
    }

    TreeDepthFirst(m_TreeModel.GetNCObject(), 
        visitor_read_properties(m_TreeModel->GetColorTable()));

    MeasureTree();
    m_Calc->DumpStats();
}

void CPhyloTreeDataSource::Clear() 
{
    m_TreeModel->Clear();

    m_SearchCurrentNode = CPhyloTree::Null();
}

void CPhyloTreeDataSource::SetColorIndices(CPhyloTreeScheme* scheme)
{
    if (scheme != NULL)
        scheme->UpdateColorTable(m_TreeModel->GetColorTable());

    m_TreeModel->GetColorTable()->LoadTexture();
}

float CPhyloTreeDataSource::GetClosestLen(float pct) const
{ 
    size_t idx = (size_t)(pct*float(m_LenDistribution.size()));
    return m_LenDistribution[std::min(idx, m_LenDistribution.size() - 1)];
}

void CPhyloTreeDataSource::MeasureTree(TTreeIdx node)
{
    if (!m_Calc)
        m_Calc = new CPhyloTreeCalculator(m_TreeModel.GetPointer(), m_TreeModel->GetColorTable());
    m_Calc->Init(m_TreeModel->GetColorTable());
    *m_Calc = TreeDepthFirst(m_TreeModel.GetNCObject(), node, *m_Calc);

    // Force lengths to be recomputed if tree changed (but do not compute by 
    // default since they are not always needed).
    m_LenDistribution.clear();
}

void CPhyloTreeDataSource::ComputeLengthsFromRoot()
{
    // Can't compute lengths without distance parameter.
    if (!m_TreeModel->GetFeatureDict().HasFeature("dist"))
        return;

    // The array is only needed for visualization based on edge length of 
    // large trees. Only compute if needed. If tree changes, MeasureTree()
    // clears the array so we know its needed.
    if (m_LenDistribution.size() != 0)
        return;

    CPhyloTreeMaxChildDist child_dist =
        TreeDepthFirst(m_TreeModel.GetNCObject(),
        m_TreeModel->GetRootIdx(),
        CPhyloTreeMaxChildDist(m_TreeModel.GetNCPointer()));

    float min_dist = child_dist.GetMinDist();
    float max_dist = child_dist.GetMaxDist();

    float dist_scaler = 1.0f / (max_dist - min_dist);
    m_LenDistribution.swap(child_dist.GetDistances());
    for (size_t i = 0; i<m_LenDistribution.size(); ++i) {
        m_LenDistribution[i] = (m_LenDistribution[i] - min_dist)*dist_scaler;
        m_TreeModel->GetNode(i)->SetEdgeScore(m_LenDistribution[i]);
    }

    std::sort(m_LenDistribution.begin(), m_LenDistribution.end());
}

void CPhyloTreeDataSource::MeasureTree()
{
    MeasureTree(m_TreeModel->GetRootIdx());
}

string CPhyloTreeDataSource::GenerateTooltipFormat()
{
    string ttf = "";
    
    if (!m_TreeModel.IsNull()) {
        CBioTreeFeatureDictionary dict = m_TreeModel->GetFeatureDict();
        
        ITERATE(CBioTreeFeatureDictionary::TFeatureDict, it, dict.GetFeatureDict()) {
            string sName = it->second;
            string sKey  = "$(" + it->second + ")";
           
            ttf += ttf.empty()?"" : "\n";
            ttf += (sName + ": " + sKey);            
        }
    }

    return ttf.empty()?"Tree Node":ttf;
}


void CPhyloTreeDataSource::ApplyAttributes(CBioTreeAttrReader::TAttrTable & attrs,
                                           CPhyloTreeScheme* scheme,
                                           const string& labelfmt)
{
    m_Calc->Init(m_TreeModel->GetColorTable());
    m_Calc->SetAttrTable(attrs);
    if (!labelfmt.empty())
        m_Calc->SetLabelFormat(labelfmt);
    *m_Calc = TreeDepthFirst(m_TreeModel.GetNCObject(), m_TreeModel->GetRootIdx(), *m_Calc);
    m_Calc->ClearAttrTable();


    
    // Update all colors in the color table (scheme colors, colors attached to nodes
    // and cluster colors)
    m_TreeModel->GetColorTable()->ClearColors();

    TreeDepthFirst(m_TreeModel.GetNCObject(), visitor_read_properties(m_TreeModel->GetColorTable()));

    Clusterize(scheme);  
}

// clusters check
bool CPhyloTreeDataSource::HasClusters()
{
    return (m_Calc != NULL && !m_Calc->GetClusters().empty()); 
}


// clusterizing with colors
void CPhyloTreeDataSource::Clusterize(CPhyloTreeScheme* scheme)
{   
    if (!m_Calc)
        return;

    // seed color
    CRgbaColor          color(204, 153, 102);
    CRgbaColor          white(255,255,255,255);

    size_t color_idx;
    if (!m_TreeModel->GetColorTable()->FindColor(color, color_idx)) {
        color_idx = m_TreeModel->GetColorTable()->AddColor(color);                                            
    }

    m_ClusterToColorMap.clear();

    // COLOR 'STEP'
    int   nmbClusters = m_Calc->GetClusters().size();
    float colorStep = nmbClusters? (180.0 / nmbClusters): 0;
    //unsigned contrast = 1;
    // for narrow color angle we will try an alternative color assignment 
    bool narrow_angle = colorStep < 5.0; 

    // Get cluster ids from selection sets -these have their own assigned colors so
    // we should not generate them. Only active (selected) selection sets need to
    // be considered here.
    map<int, size_t>  cluster_id_to_selection_map = m_TreeModel->GetSelectionSets().GetClusterToSelectionMap();


    // Find colors for each cluster
    ITERATE (CPhyloTreeCalculator::TClusterHash, cl, m_Calc->GetClusters()) {
        int cluster_id = cl->first;

        // Get color from table of selected node sets OR generate it for the cluster.
        // We generate a new color each time though the loop even if not all are used
        // because we want to not change cluster colors when selection changes. (disconcerting
        // for user)
        CRgbaColor cluster_color = color;
        if (cluster_id_to_selection_map.find(cluster_id) != cluster_id_to_selection_map.end()) {
            size_t select_set_idx = cluster_id_to_selection_map[cluster_id];
            cluster_color = m_TreeModel->GetSelectionSets().GetSets()[select_set_idx].GetColor();
        }
        else {
            // This code is to protect us from colors too bright (almost white)           
            float c_dist = CRgbaColor::ColorDistance(white, cluster_color);
            while (c_dist < 0.2f) {
                cluster_color.Darken(0.15f);
                c_dist = CRgbaColor::ColorDistance(white, cluster_color);
            }
        }

        if (!m_TreeModel->GetColorTable()->FindColor(cluster_color, color_idx)) {
            size_t closest_idx;
            float dist = m_TreeModel->GetColorTable()->FindClosestColor(cluster_color, closest_idx);            

            if (dist > 0.01f) {
                // Add a new color to the color table
                color_idx = m_TreeModel->GetColorTable()->AddColor(cluster_color);
            }
            else {
                // Use an existing color (to avoid letting the table get large - since it's stored in a texture
                // max size may be 4096 or 8192.
                color_idx = closest_idx;
            }
        }

        m_ClusterToColorMap[cluster_id] = color_idx;

        float rotate_angle = colorStep + (narrow_angle ? 90.0 : 180.0);
        color = CRgbaColor::RotateColor(color, rotate_angle); 
    } // ITERATE


    // propagate colors
    const CPhyloTreeCalculator::TClusterHash& cluster_map = m_Calc->GetClusters();
    ITERATE(CPhyloTreeCalculator::TClusterHash, cl, cluster_map) {
        // getting min x of each cluster
        int minX = m_Calc->GetWidth();       

        ITERATE (vector<TTreeIdx>, node_iter, cl->second) {
            if ((*m_TreeModel)[*node_iter].HasParent()) {
                CPhyloTreeNode& parent = m_TreeModel->GetParent((*m_TreeModel)[*node_iter]);

                if ((*parent).IDX().first < minX) {
                    minX = (*parent).IDX().first;    
                }
            }
        }

        color_idx = m_ClusterToColorMap[cl->first];
        bool selection_cluster = (cluster_id_to_selection_map.find(cl->first) != cluster_id_to_selection_map.end());

        // Selection clusters propogate all the way to the root.  Standard clusters
        // (cluster-id property) propogate to nearest common ancester.  Also standard clusters
        // are only seen when coloration mode is 'cluster'
        if (selection_cluster) 
            minX = 0;
        else if (scheme->GetColoration()!=CPhyloTreeScheme::eClusters)
            continue;

        ITERATE (vector<TTreeIdx>, node_iter, cl->second) {          
            TTreeIdx parent_idx = *node_iter;

            // if it is not lowest common node, mark parents       
            for (; (*m_TreeModel)[parent_idx].HasParent(); 
                parent_idx = (*m_TreeModel)[parent_idx].GetParent()) {

                    CPhyloTreeNode& cursor = ((*m_TreeModel)[parent_idx]);
                    if ((*cursor).IDX().first > minX) {

                        // If its the primary cluster-id, update cluster colors.
                        // If there are multiple cluster id's in the node add a marker
                        // if were are visiting parents for > 1 time... ignore.  Only multi-mark
                        // the selected nodes themselves, and only visit parents of dominant
                        // color. (first sel color or, if none, cluster-id).
                        int primary_cluster_id = (*cursor).GetPrimaryCluster();                        
                        if (parent_idx == *node_iter) {                            
                            if (primary_cluster_id == cl->first) {
                                (*cursor).SetClusterColorIdx(color_idx);                          
                            }
                            if (cursor.IsLeaf() && (*cursor).GetNumClusters() > 1) {
                                // only include selection colors in the set of colors for
                                // markers, and only apply markers to leaf nodes.
                                if (selection_cluster) {
                                    // Marker size is a multiple of node size, so just use 2 here (double node size):
                                    (*cursor).SetMarkerSize(2.0f);
                                    (*cursor).GetMarkerColors().push_back(
                                         m_TreeModel->GetColorTable()->GetColor(color_idx));
                                 }
                            }
                        }
                        else {
                            (*cursor).SetClusterColorIdx(color_idx);                          
                        }
                    }
                    else {                     
                        break;
                    }
            }
        }
    }

    SetColorIndices(scheme); 
}

void CPhyloTreeDataSource::ReRoot(TTreeIdx root_idx)
{
    m_TreeModel->ReRoot(root_idx);

    // root node has 0 distance since it has no parent (and distance stored in a node is
    // the node's distance from it's parent)
    m_TreeModel->GetNode(root_idx).GetValue().SetDistance(0.0f);
    m_TreeModel->GetNode(root_idx).SetParent(CPhyloTree::Null());
    m_TreeModel->GetNode(root_idx).GetValue().Sync(m_TreeModel->GetFeatureDict());
}

void CPhyloTreeDataSource::ReRootEdge(TTreeIdx edge_child_node)
{
    // We can still re-root if there is not distance attribute. But
    // if there is, we need to update it and split the distance 50%
    // on either side of the new node.
    float dist = 0.0f;
    if (m_TreeModel->GetFeatureDict().HasFeature("dist"))
        dist = m_TreeModel->GetNode(edge_child_node).GetValue().GetDistance();
    
    // Create the new node to be the root:
    m_TreeModel->SetCurrentNode(edge_child_node);
    TTreeIdx new_node_idx = NewNode(false);

    // If there is a distance attribute, allocate 1/2 the distance from the child
    // to the new node, and the other half from the new node to the child's parent
    if (m_TreeModel->GetFeatureDict().HasFeature("dist")) {
        m_TreeModel->GetNode(new_node_idx).GetValue().SetDistance(dist/2.0f);
        m_TreeModel->GetNode(new_node_idx).GetValue().Sync(m_TreeModel->GetFeatureDict());
        m_TreeModel->GetNode(edge_child_node).GetValue().SetDistance(dist/2.0f);
        m_TreeModel->GetNode(edge_child_node).GetValue().Sync(m_TreeModel->GetFeatureDict());
    }
    ReRoot(new_node_idx);
    m_TreeModel->SetCurrentNode(CPhyloTree::Null());
}

void CPhyloTreeDataSource::ReRootMidpoint()
{
    if (!m_TreeModel->GetFeatureDict().HasFeature("dist"))
        return;

    CPhyloTreeDistFromRoot child_dist = TreeDepthFirst(
            m_TreeModel.GetNCObject(),
            m_TreeModel->GetRootIdx(),
            CPhyloTreeDistFromRoot(m_TreeModel.GetNCPointer()));

    CPhyloTreeMidpointDist midpoint_dist = TreeDepthFirst(m_TreeModel.GetNCObject(),
        m_TreeModel->GetRootIdx(),
        CPhyloTreeMidpointDist(m_TreeModel.GetNCPointer(), 
                               child_dist.GetDistances(),
                               child_dist.GetMaxDistNode()));

    float dist;
    std::vector<CPhyloTree::TTreeIdx> path;

    midpoint_dist.GetLongest(path, dist);
    float midpoint = dist / 2.0f;
    float d = 0.0f;

    if (path.size() > 1) {
        TTreeIdx midpoint_parent = CPhyloTree::Null();        
        TTreeIdx midpoint_child = CPhyloTree::Null();
        float dist_from_parent = 0.0f;
        float dist_from_child = 0.0f;

        for (size_t i=0; i < path.size(); ++i) {
            TNodeType& node = m_TreeModel->GetNode(path[i]);

            // Find middle of path, based on distances. Distances are stored in the child node as distance 
            // to the parent, so we have to check at each node along the path which is parent and which is 
            // child since it switches at the least common ancestor - the highest node in the tree that
            // the two leaves at either end of 'path' have in common.  Once we identify the midpoint,
            // we may stop this loop. (but we run to the end to highlight nodes for debugging)
            if (i < path.size() - 1 && midpoint_child == CPhyloTree::Null()) {

                TNodeType& next_node = m_TreeModel->GetNode(path[i + 1]);

                // If next node is parent of this node, distance is in this node.
                if (node.GetParent() == path[i + 1]) {
                    d += node.GetValue().GetDistance();

                    if (d >= midpoint) {
                        midpoint_child = path[i];
                        midpoint_parent = path[i + 1];

                        float base_distance = d - node.GetValue().GetDistance();
                        dist_from_parent = d - midpoint;
                        dist_from_child = midpoint - base_distance;
                    }
                }
                // If this node is parent of the next node, distance is in the next node
                else if (next_node.GetParent() == path[i]) {
                    d += next_node.GetValue().GetDistance();

                    if (d >= midpoint) {
                        midpoint_parent = path[i];
                        midpoint_child = path[i + 1];

                        float base_distance = d - next_node.GetValue().GetDistance();
                        dist_from_parent = midpoint - base_distance;
                        dist_from_child = d - midpoint;
                    }
                }
                else {
                    /// Error - either one or the other is a parent
                    _ASSERT(0);
                }
            }

            /*  For debugging: highlight the path we found
            node.GetValue().SetFeature(
                m_TreeModel->GetFeatureDict(), "$NODE_COLOR", "[255 0 0 255]");
            node.GetValue().InitFeatures(
                m_TreeModel->GetFeatureDict(), m_TreeModel->GetColorTable());
            */
        }


        if (midpoint_child != CPhyloTree::Null()) {
            // It is possible the tree is already mid-point rooted. If the 
            // midpoint is 'close' to an existing node, make
            // that the root if it is not already (and write info message)
            // Now that node has at least 2 children by definition. If it
            // has more I think we accept that - it is possible to have
            // the midpoint lie at an n-node intersection
            
            // What is a 'close'? Let's say 1/100 of average edge length. 
            // Since we now know the maximum distance between 2 leaves and the
            // number of edges between them, we could set 'close' to be 1/100
            // of the length of an average edge, i.e. given 10 edges in the path
            // and the total path length of 0.7, 'small' is < (0.7/10)/100
            float close = (dist/float(path.size() - 1))/100.0f;
            TTreeIdx close_idx = CPhyloTree::Null();
            if (dist_from_parent <= close)
                close_idx = midpoint_parent;
            else if (dist_from_child <= close)
                close_idx = midpoint_child;

            if (close_idx != CPhyloTree::Null()) {
                TNodeType& midpoint = m_TreeModel->GetNode(close_idx);
                if (!midpoint.HasParent()) {
                    LOG_POST(Info << "Tree already rooted at midpoint");
                    return;
                }
                else {
                    ReRoot(close_idx);
                    LOG_POST(Info << "Re-rooted tree at existing midpoint");
                    return;
                }
            }
            else {
                // Set current node to child node (midpoint child)
                // Call newnode with false to insert between parent and child
                // set new nodes distance to dist_from_parent
                // set midpoint_child's distance to dist_from_child
                // Re-root tree at new node.
                m_TreeModel->SetCurrentNode(midpoint_child);
                TTreeIdx new_node_idx = NewNode(false);
                m_TreeModel->GetNode(new_node_idx).GetValue().SetDistance(dist_from_parent);
                m_TreeModel->GetNode(new_node_idx).GetValue().Sync(m_TreeModel->GetFeatureDict());
                m_TreeModel->GetNode(midpoint_child).GetValue().SetDistance(dist_from_child);
                // This also can be done via  CPhyloNodeData::Sync(feature_dict) but we don't want to sync the
                // label which may or may not be initialized at this point
                string str_dist;
                NStr::DoubleToString(str_dist, dist_from_child);
                m_TreeModel->GetNode(midpoint_child).GetValue().SetFeature(m_TreeModel->GetFeatureDict(), "dist", str_dist);
                ReRoot(new_node_idx);
                m_TreeModel->SetCurrentNode(CPhyloTree::Null());
            }
        }
    }

}

/// Collapse, based on distance, enough nodes in the tree to get the total
/// number of leaves down to the requested number. (or just over if exact 
/// match is not possible).  We do not expand nodes that may already be
/// collapsed.
set<CPhyloNodeData::TID> 
CPhyloTreeDataSource::CollapseByDistance(int leaf_count_target, SCollapsable* collapse_func)
{
    set<CPhyloNodeData::TID> collapsed_nodes;

    if (!m_TreeModel->GetFeatureDict().HasFeature("dist"))
        return collapsed_nodes;

    // Get distance of each node from root to determine which should be collapsed.
    // distance here is max distance of any direct child of the node from root
    CPhyloTreeMaxDirectChildDist child_dist =
        TreeDepthFirst(m_TreeModel.GetNCObject(),
        m_TreeModel->GetRootIdx(),
        CPhyloTreeMaxDirectChildDist(m_TreeModel.GetNCPointer(), collapse_func));

    vector<SChildMaxDist> distances = child_dist.GetDistances();

    // sort by distance (least to greatest)
    std::sort(distances.begin(), distances.end());
    int current_leaf_count = m_TreeModel->GetRoot()->GetNumLeavesEx();

    // 
    while (current_leaf_count > leaf_count_target) {
        vector<SChildMaxDist>::reverse_iterator riter = distances.rbegin();

        // pick the node with the most distant direct child (exclude any nodes
        // with subtrees).
        for (; riter != distances.rend(); ++riter) {
            TNodeType& n = m_TreeModel->GetNode((*riter).m_NodeIdx);

            bool has_subtree = false;
            // make sure node only has direct children (leaf nodes or collapsed nodes)
            // and do not include the root node.
            for (size_t i = 0; i < n.GetChildren().size(); ++i) {
                TNodeType& child_node = m_TreeModel->GetNode(n.GetChildren()[i]);
                if (child_node.IsLeafEx())
                    continue;
                else if (collapsed_nodes.find(child_node->GetId()) == collapsed_nodes.end()) {
                    has_subtree = true;
                    break;
                }
            }
            if (!has_subtree)
                break;
        }

        if (riter == distances.rend())
            break;

        TNodeType& n = m_TreeModel->GetNode((*riter).m_NodeIdx);

        // Collapse nodes in order of most distant to least.  Distance is based on
        // the maximum distance of any of the nodes immediate children (whether or
        // not they are leaves).  Using this approach, we should never collapse 
        // a node that has a non-leaf child that is not already collapsed.
        if (n.CanExpandCollapse(CPhyloNodeData::eHideChildren)) {
            // We only want to show top-level nodes as collapsed, so remove
            // from the set of collapsed nodes any children of the node
            // we have chosen to collapse.
            for (size_t i = 0; i < n.GetChildren().size(); ++i) {
                TNodeType& child_node = m_TreeModel->GetNode(n.GetChildren()[i]);

                set<CPhyloNodeData::TID>::iterator iter;
                iter = collapsed_nodes.find(child_node->GetId());
                if (iter != collapsed_nodes.end()) {
                    collapsed_nodes.erase(iter);
                }
            }

            collapsed_nodes.insert(n->GetId());

            // When you collapse a node that only has leaves or collapsed nodes as children,
            // You reduce the number of leaves by the number of its leaves-1 because it
            // is now a leaf.
            current_leaf_count -= n.GetChildren().size() - 1;
        }

        distances.erase(--riter.base());
    }

    return collapsed_nodes;
}

void CPhyloTreeDataSource::Relabel(CPhyloTreeScheme* scheme, 
                                   string labelFmt)
{
    m_Calc->Init(m_TreeModel->GetColorTable());
    m_Calc->SetLabelFormat(labelFmt);
    *m_Calc = TreeDepthFirst(m_TreeModel.GetNCObject(), m_TreeModel->GetRootIdx(), *m_Calc);
	m_TreeModel->SetNumNodes(m_Calc->GetNumNodes());

    Clusterize(scheme);

    int max_children = std::max(10, m_Calc->GetHeight() / 2);
    scheme->SetMaxNumChildren(GLdouble(max_children));
    scheme->SetMaxBranchDist(m_Calc->GetMaxDistance());
}

void CPhyloTreeDataSource::Sort(bool ascending)
{
    TreeDepthFirst(m_TreeModel.GetNCObject(), 
        m_TreeModel->GetRootIdx(), 
        CPhyloTreeSorter(m_TreeModel.GetPointer(), ascending));

    MeasureTree();
}

void CPhyloTreeDataSource::SortDist(bool ascending)
{
    if (!m_TreeModel->GetFeatureDict().HasFeature("dist"))
        return;

    CPhyloTreeMaxChildDist child_dist = 
        TreeDepthFirst(m_TreeModel.GetNCObject(), 
                       m_TreeModel->GetRootIdx(), 
                       CPhyloTreeMaxChildDist(m_TreeModel.GetNCPointer()));

    TreeDepthFirst(m_TreeModel.GetNCObject(), 
        m_TreeModel->GetRootIdx(), 
        CPhyloTreeSorterSubtreeDist(m_TreeModel.GetNCPointer(), child_dist.GetDistances(), ascending));

    MeasureTree();
}

void CPhyloTreeDataSource::SortLabel(bool ascending)
{
    TreeDepthFirst(m_TreeModel.GetNCObject(), 
        m_TreeModel->GetRootIdx(), 
        CPhyloTreeSorterLabel(m_TreeModel.GetNCPointer(), ascending));

    MeasureTree();
}

void CPhyloTreeDataSource::SortLabelRange(bool ascending)
{
    CPhyloTreeLabelRange label_ranges = 
        TreeDepthFirst(m_TreeModel.GetNCObject(), 
                       m_TreeModel->GetRootIdx(), 
                       CPhyloTreeLabelRange(m_TreeModel.GetNCPointer(), ascending));

    TreeDepthFirst(m_TreeModel.GetNCObject(), 
        m_TreeModel->GetRootIdx(), 
        CPhyloTreeSorterLabelRange(m_TreeModel.GetNCPointer(), label_ranges.GetLabelRanges(), ascending));

    MeasureTree();
}

TModelRect CPhyloTreeDataSource::GetBoundRect(void)
{
    // rectangle calculator
    CPhyloTreeRectCalculator rect_calc(m_TreeModel);
    rect_calc.Init();
    rect_calc = TreeDepthFirst(m_TreeModel.GetNCObject(), m_TreeModel->GetRootIdx(), rect_calc);
    return rect_calc.GetRect();
}

void CPhyloTreeDataSource::Clean()
{
    // Convert to biocontainer to get user data
    CRef<objects::CBioTreeContainer> btc(new objects::CBioTreeContainer());
    CRef<objects::CBioTreeContainer> new_container(new CBioTreeContainer());
    TreeConvert2Container(*new_container, *GetTree());

    // Now convert tree datastructure to biotreecontainer, skipping over any
    // single child nodes
    TreeConvertNonSingleChild2Container(*btc, *GetTree(), GetTree()->GetRootIdx());

    // Add user data to new biotreecontainer
    if (new_container->IsSetUser()) {
        btc->SetUser().Assign(new_container->GetUser());
    }

    // Initialize tree with new biotreecontainer
    Init(*btc, *m_Scope, false);
    GetModel().GetCollisionData().Clear();
}

void CPhyloTreeDataSource::Filter()
{
    // not currently hooked up to UI (and would have problems - needs to fix child pointers..)
    CPhyloTreeFilter_Selector selector(m_TreeModel.GetNCPointer());
    selector = TreeDepthFirstInvarient(m_TreeModel.GetNCObject(), m_TreeModel->GetRootIdx(), selector);

    MeasureTree();
}


void CPhyloTreeDataSource::FilterDistances(double x_dist)
{
    // not currently hooked up to UI (and would have problems - needs to fix child pointers..)
    CPhyloTreeFilter_Distance dfilter(m_TreeModel.GetNCPointer(), x_dist);
    dfilter = TreeDepthFirstInvarient(m_TreeModel.GetNCObject(), m_TreeModel->GetRootIdx(), dfilter);

    MeasureTree();
}



string CPhyloTreeDataSource::GetColumnLabel(size_t col) const
{ 
    const CBioTreeFeatureDictionary:: TFeatureDict& fdict = 
        m_TreeModel->GetFeatureDict().GetFeatureDict();
    CBioTreeFeatureDictionary::TFeatureDict::const_iterator fiter;

    size_t count = 0;
    for (fiter=fdict.begin(); fiter != fdict.end(); ++fiter, ++count) {
        if (count == col) {
            return (*fiter).second; 
        }
    }

    return "";
}

size_t CPhyloTreeDataSource::GetColsCount() const  
{ 
    return m_TreeModel->GetFeatureDict().GetFeatureDict().size(); 
}

CMacroQueryExec* CPhyloTreeDataSource::GetQueryExec(bool casesensitive, CStringMatching::EStringMatching matching)
{
    CTreeQueryExec* qexec = new CTreeQueryExec(&(this->GetDictionary()));
       
    qexec->SetTree(this->GetTree());

    // Logical operators:
    qexec->AddFunc(CQueryParseNode::eAnd,
            new CQueryFuncPromoteAndOr());
    qexec->AddFunc(CQueryParseNode::eOr,
            new CQueryFuncPromoteAndOr());
    qexec->AddFunc(CQueryParseNode::eSub,
            new CQueryFuncPromoteLogic());
    qexec->AddFunc(CQueryParseNode::eXor,
            new CQueryFuncPromoteLogic());
    qexec->AddFunc(CQueryParseNode::eNot,
            new CQueryFuncPromoteLogic());

    // Constants:
    qexec->AddFunc(CQueryParseNode::eIntConst,
            new CQueryFuncPromoteValue());
    qexec->AddFunc(CQueryParseNode::eFloatConst,
            new CQueryFuncPromoteValue());
    qexec->AddFunc(CQueryParseNode::eBoolConst,
            new CQueryFuncPromoteValue());
    qexec->AddFunc(CQueryParseNode::eString,
            new CQueryFuncPromoteValue());

    NStr::ECase cs = casesensitive ? NStr::eCase  : NStr::eNocase;

    // Comparison operators:
    qexec->AddFunc(CQueryParseNode::eEQ,
            new CQueryFuncPromoteEq(cs, matching));
    qexec->AddFunc(CQueryParseNode::eGT,
            new CQueryFuncPromoteGtLt(CQueryParseNode::eGT, cs));
    qexec->AddFunc(CQueryParseNode::eGE,
            new CQueryFuncPromoteGtLt(CQueryParseNode::eGE, cs));
    qexec->AddFunc(CQueryParseNode::eLT,
            new CQueryFuncPromoteGtLt(CQueryParseNode::eLT, cs));
    qexec->AddFunc(CQueryParseNode::eLE,
            new CQueryFuncPromoteGtLt(CQueryParseNode::eLE, cs));
    qexec->AddFunc(CQueryParseNode::eIn,
            new CQueryFuncPromoteIn(cs, matching));
    qexec->AddFunc(CQueryParseNode::eBetween,
            new CQueryFuncPromoteBetween(cs));
    qexec->AddFunc(CQueryParseNode::eLike,
            new CQueryFuncLike(cs));

    // Functions
    qexec->AddFunc(CQueryParseNode::eFunction,
            new CQueryFuncFunction());
    qexec->AddFunc(CQueryParseNode::eIdentifier,
        new CQueryFuncPromoteIdentifier());
    // Unusual mapping: Run time vars ->  CQueryParseNode::eSelect
    // This is done to preserve CQueryParseNode from modification.
    qexec->AddFunc(CQueryParseNode::eSelect,  // RT Variable
            new CQueryFuncRTVar());
    qexec->AddFunc(CQueryParseNode::eFrom,   // Assignment operator
            new CQueryFuncAssignment());

    return qexec;
}

void CPhyloTreeDataSource::SetQueryResults(CMacroQueryExec* q)
{
    CTreeQueryExec *e = dynamic_cast<CTreeQueryExec*>(q);

    ClearQueryResults();
    m_TreeModel->ClearSelection();
    m_TreeModel->SetSelection(e->GetTreeSelected(), false, true);
}

void CPhyloTreeDataSource::ClearQueryResults()
{ 
    m_SearchCurrentNode = CPhyloTree::Null(); 
}

void CPhyloTreeDataSource::ExecuteStringQuery(const string &query, 
                                              size_t& num_selected, 
                                              size_t& num_queried, 
                                              CStringMatching::EStringMatching string_matching, 
                                              NStr::ECase use_case)
{
    // Re-run the query if options that can change query results have changed
    m_SearchCurrentNode = CPhyloTree::Null();
    num_queried = 0;
    num_selected = 0;

    m_StringQueryIDs.clear();
        
    // simple string queries are completed synchronously since
    // they can never get hung up in network access.
    if (query != "") {
        vector<TTreeIdx> sel = FindNodes(query, num_queried, string_matching, use_case);
        m_TreeModel->ClearSelection();

        // Because of selecting parents this can lose the fact that some
        // non leaf nodes were selected.
        m_TreeModel->SetSelection(sel, false, true);
        num_selected = sel.size();

        // Be a bit more efficient to get these during query if we decide we have to keep this.
        m_StringQueryIDs.reserve(sel.size());
        for (auto iter=sel.begin(); iter!=sel.end(); ++iter) {
            m_StringQueryIDs.push_back(m_TreeModel->GetNode(*iter).GetValue().GetId());
        }

        LOG_POST(Info << "String Query: " << query << " Num selected: " << sel.size());
    }
}

class visitor_stringmatch_query
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    visitor_stringmatch_query(CStringMatching  &stringMatching, vector<TTreeIdx> &selNodes, size_t &numQueried) :  
        m_StringMatching(stringMatching),
        m_SelNodes(selNodes),
        m_NumQueried(numQueried)
    {}
    ETreeTraverseCode operator()(CPhyloTree& tree, 
                                 TTreeIdx node, int delta)
    {
        if (delta==1 || delta==0){
            ++m_NumQueried;
            CBioTreeFeatureList::TFeatureList f_list = 
                tree[node]->GetBioTreeFeatureList().GetFeatureList();


            ITERATE(CBioTreeFeatureList::TFeatureList, it, f_list) {
                if (m_StringMatching.MatchString(it->value)) {
                    m_SelNodes.push_back(node);
                    break;
                }                     
            }    
        }
        return eTreeTraverse;
    }

private:
    CStringMatching  &m_StringMatching;
    vector<TTreeIdx> &m_SelNodes;
    size_t           &m_NumQueried;
};

vector<CPhyloTree::TTreeIdx> 
CPhyloTreeDataSource::FindNodes(const string &query, size_t& num_queried, CStringMatching::EStringMatching string_matching, NStr::ECase use_case) const
{
    vector<TTreeIdx> selNodes;
    CStringMatching stringMatching(query, string_matching, use_case);
    visitor_stringmatch_query finder(stringMatching, selNodes, num_queried);
    TreeDepthFirst(m_TreeModel.GetNCObject(), finder);
    return selNodes;
}

// This sorts nodes by their position (depth-first order) in the tree using
// the node index which represents the position of the node relative to other 
// nodes.
struct NodeIdxSort {
    NodeIdxSort(const CPhyloTree& t) : m_Tree(t) {};
    bool operator()(CPhyloTree::TTreeIdx lhs, CPhyloTree::TTreeIdx rhs) const {
        const CPhyloTree::TNodeType& n1 = m_Tree[lhs];
        const CPhyloTree::TNodeType& n2 = m_Tree[rhs];

        if (n1.GetValue().IDX().second < n2.GetValue().IDX().second)
            return true;
        else if (n1.GetValue().IDX().second > n2.GetValue().IDX().second)
            return false;
        // first value was equal:
        else if (n1.GetValue().IDX().first < n2.GetValue().IDX().first)
            return true;
        else return false;
    }

protected:
    const CPhyloTree& m_Tree;
};

CPhyloTree::TTreeIdx 
CPhyloTreeDataSource::IterateOverSelNodes(int direction, bool highlight)
{
    // highlight
    if (m_TreeModel->GetRootIdx() == CPhyloTree::Null())
        return CPhyloTree::Null();

    // Get all nodes user 'explicitly' selected - so either the user clicked on
    // them directly (does not count if they are the child of a selected node) or
    // the node was directly selected by a query
    vector<TTreeIdx> search_cache;
    m_TreeModel->GetExplicitlySelectedAndNotCollapsed(search_cache);

    // Sort the nodes into depth-first order
    std::sort(search_cache.begin(), search_cache.end(), NodeIdxSort(m_TreeModel.GetObject()));

    if (search_cache.empty())
        return CPhyloTree::Null(); 

    // cursor
    if (direction==0) {
        if (m_SearchCurrentNode == CPhyloTree::Null()) {
            m_SearchCurrentNode = (*search_cache.begin());
        }
    }
    else if (direction>0) {
        if (m_SearchCurrentNode == CPhyloTree::Null()) {
            m_SearchCurrentNode = (*search_cache.begin());
        }
        else {
            vector<TTreeIdx>::iterator itt = 
                find(search_cache.begin(), search_cache.end(), m_SearchCurrentNode);

            // If not found (unexpected) set cursor to beginning.
            if (itt == search_cache.end()) {
                m_SearchCurrentNode = (*search_cache.begin());
            }
            // else increment unless we are already at the end.
            else {
                ++itt;
                if (itt != search_cache.end())
                    m_SearchCurrentNode = *itt;
                else // wrap around to the beginning
                    m_SearchCurrentNode = (*search_cache.begin());
            }
        }
    }
    else {
        if (m_SearchCurrentNode == CPhyloTree::Null()) {
            m_SearchCurrentNode = *(search_cache.begin() + (search_cache.size() - 1));
        }
        else {
            vector<TTreeIdx>::iterator itt = 
                find(search_cache.begin(), search_cache.end(), m_SearchCurrentNode);

            // If not found (unexpected) set cursor to the end
            if (itt == search_cache.end()) {
                m_SearchCurrentNode = *(search_cache.begin() + (search_cache.size() - 1));
            }
            // else decrement cursor if not already at the beginning
            else {                
                if (itt != search_cache.begin())
                    m_SearchCurrentNode = *(--itt);
                else // wrap around to the end
                    m_SearchCurrentNode = search_cache[search_cache.size() - 1];                
            }
        }
    }

    // Make the node we have iterated to the current node -this will give it
    // a small visual marker (square) around the node so user knows where they are.
    if (m_SearchCurrentNode)
        m_TreeModel->SetCurrentNode(m_SearchCurrentNode);

    return m_SearchCurrentNode;
}

void CPhyloTreeDataSource::UpdateSelectionSets(CPhyloTreeScheme* scheme)
{
    // renumber selection cluster IDs in set to make sure the selection cluster IDs
    // don't overlap regular cluster IDs:
    // pick cluster id larger than existing ids:
    CPhyloTree::TClusterID max_id =
        GetMaxClusterID() + TClusterID(500 + m_TreeModel->GetSelectionSets().GetSets().size());
    // set ids, first highest then lower.
    GetSelectionSets().RenumberClusterIDs(max_id);

    m_TreeModel->GetSelectionSets().SetSelectionSetProperty(m_TreeModel.GetPointer());
    MeasureTree();

    // Check max-id one more time. We do this because MeasureTree() updates the value
    // returned by GetMaxClusterID() and there is a small chance that this value has
    // increased since the tree was created. (change would be via editing of node properties)
    if (GetMaxClusterID() >= max_id - TClusterID(m_TreeModel->GetSelectionSets().GetSets().size())) {
        GetSelectionSets().RenumberClusterIDs(GetMaxClusterID() + 500 + m_TreeModel->GetSelectionSets().GetSets().size());
        m_TreeModel->GetSelectionSets().SetSelectionSetProperty(m_TreeModel.GetPointer());
        MeasureTree();
    }

    Clusterize(scheme);
}

class visitor_detach_subtree
{
public:
    visitor_detach_subtree(){}
    ETreeTraverseCode operator()(CPhyloTree&  tree, CPhyloTree::TTreeIdx node, int delta)
    {
        if (delta==1 || delta==0){
            tree[node].GetValue().SetId(CPhyloNodeData::TID(-1));
        }
        return eTreeTraverse;
    }
};

class visitor_copy_subtree
{
public:
    typedef CPhyloTree TTreeType;
    typedef CPhyloTree::TTreeIdx TTreeIdx;
    typedef CPhyloTree::TNodeType TNodeType;

public:
    visitor_copy_subtree(CPhyloTree& tree, CPhyloTree& target)
    : m_Target(target)
    {
        m_Target.GetFeatureDict() = tree.GetFeatureDict();
        m_Target.SetRootIdx(0);
    }
    ETreeTraverseCode operator()(CPhyloTree& tree, 
                                 CPhyloTree::TTreeIdx node_idx, 
                                 int delta)
    {
        if (delta < 0) {
            return eTreeTraverse;
        }

        TNodeType& node = tree[node_idx];
        CPhyloNodeData::TID id = node.GetValue().GetId();
        
        // Make sure not already in tree (but traversal should guarantee)
        TTreeIdx idx = m_Target.FindNodeById(id);
        if (idx != CPhyloTreeNode::Null()) {
            return eTreeTraverse;
        }

        // Adds a copy of node.  Need to update indices to match the target tree
        TTreeIdx new_node_idx = m_Target.AddNode(node);
        TNodeType& new_node = m_Target[new_node_idx];
        new_node.ClearConnections();
        
        // Look up the parent index of the new node using the 
        // node id
        CPhyloNodeData::TID parent_id = tree[node.GetParent()]->GetId();
        TTreeIdx parent_idx = m_Target.FindNodeById(parent_id);
        
        // This will only happen for the top (first) node of the 
        // subtree being cut since its parent won't be in the subtree
        if (parent_idx == CPhyloTreeNode::Null()) {            
            return eTreeTraverse;
        }
        m_Target.AddChild(parent_idx, new_node_idx);

        return eTreeTraverse;
    }

    CPhyloTree& m_Target;
};

class visitor_paste_subtree
{
public:
    typedef CPhyloTree TTreeType;
    typedef CPhyloTree::TTreeIdx TTreeIdx;
    typedef CPhyloTree::TNodeType TNodeType;

public:
    visitor_paste_subtree(CPhyloTree& source, CPhyloTree& target, TTreeIdx target_idx)
    : m_Target(target)
    , m_Source(source)
    , m_TargetIdx(target_idx)
    {
        CPhyloTreeMaxIdCalculator next_id(&m_Target);
        next_id = TreeDepthFirst(m_Target, next_id);
        m_TargetMaxId = next_id.GetMaxId();
    }
    // This is iterating over the source tree adding each 
    // element to the target tree (m_Target)
    ETreeTraverseCode operator()(CPhyloTree& tree, 
                                 CPhyloTree::TTreeIdx node_idx, 
                                 int delta)
    {
        if (delta < 0) {
            return eTreeTraverse;
        }

        // Node from source tree (the tree we are copying into the target tree)
        TNodeType& node = tree[node_idx];
        CPhyloNodeData::TID source_id = node.GetValue().GetId();

        // Adds a copy of node.  Need to update indices to match the target tree
        TTreeIdx new_node_idx = m_Target.AddNode(node);
        TNodeType& new_node = m_Target[new_node_idx];
        new_node.ClearConnections();  

        CPhyloNodeData::TID target_id = ++m_TargetMaxId;
        m_IdMap[source_id] = target_id;

        new_node->SetId(target_id);
        new_node.GetValue().GetBioTreeFeatureList() = CBioTreeFeatureList();

        // Add any features in in dictionary of source but not target to target.
        // Get the id for the name each time too since ids between the two
        // trees may not match.
        ITERATE(CBioTreeFeatureList::TFeatureList, it,
                node->GetBioTreeFeatureList().GetFeatureList()) {
            // Use the id for the feature to get the feature name     
            string feature_name = m_Source.GetFeatureDict().GetName(it->id);
            TBioTreeFeatureId id = m_Target.GetFeatureDict().Register(feature_name);
            new_node.GetValue().GetBioTreeFeatureList().SetFeature(id, it->value);
        }

        // If this is the root of the source tree, attach it to the node
        // m_TargetIdx in the target tree. Otherwise attach it to the
        // corresponding node copied from the source tree using the id-to-id map
        if (node.GetParent() == CPhyloTreeNode::Null()) {
            m_Target.AddChild(m_TargetIdx, new_node_idx);
        }
        else {
            CPhyloNodeData::TID parent_id = tree[node.GetParent()]->GetId();
            CPhyloNodeData::TID target_tree_parent_id = m_IdMap[parent_id];
            TTreeIdx parent_idx = m_Target.FindNodeById(target_tree_parent_id);

            if (parent_idx != CPhyloTreeNode::Null()) {
                m_Target.AddChild(parent_idx, new_node_idx);
            }
            else {
                LOG_POST(Info << "Error - did not find parent node as expected.");
            }
        }        
        
        return eTreeTraverse;
    }

    CPhyloTree& m_Target;
    CPhyloTree& m_Source;
    TTreeIdx m_TargetIdx;

    /// mapping from Ids in source tree to target tree
    map<CPhyloNodeData::TID, CPhyloNodeData::TID> m_IdMap;

    CPhyloNodeData::TID m_TargetMaxId;
};



void CPhyloTreeDataSource::Cut()
{
    TTreeIdx node_idx = m_TreeModel->GetCurrentNodeIdx();

    if (node_idx == CPhyloTree::Null()) {
        _TRACE("CPhyloTreeDataSource::Cut - Nothing is selected");
        return;
    }

    if (node_idx == m_TreeModel->GetRootIdx()) {
        _TRACE("CPhyloTreeDataSource::Cut - Removing root item is illegal");
        return;
    }
    
    m_sTreeClipboard.Clear();   

    CPhyloTree::TNodeType& n = m_TreeModel->GetCurrentNode();

    m_TreeModel->SetSelection(n.GetParent(), true, true);

    TreeDepthFirst(m_TreeModel.GetNCObject(), node_idx, 
        visitor_copy_subtree(m_TreeModel.GetNCObject(), m_sTreeClipboard));

    m_TreeModel->ClearSelection();
    m_TreeModel->RemoveChild(n.GetParent(), node_idx);      

    m_TreeModel->UpdateNodesMapping();
    MeasureTree();   
}

void CPhyloTreeDataSource::Paste(void)
{
    TTreeIdx node_idx = m_TreeModel->GetCurrentNodeIdx();

    if (node_idx == CPhyloTree::Null()) {
        _TRACE("CPhyloTreeDataSource::Paste - Nothing is selected");
        return;
    }

    if (m_sTreeClipboard.GetRootIdx() == CPhyloTreeNode::Null()) {
        _TRACE("CPhyloTreeDataSource::Paste - Clipboard is empty");
        return;
    }

    TreeDepthFirst( m_sTreeClipboard,
        visitor_paste_subtree(m_sTreeClipboard, m_TreeModel.GetNCObject(), node_idx));

    m_TreeModel->UpdateNodesMapping();
    MeasureTree();
}

TTreeIdx CPhyloTreeDataSource::NewNode(bool after)
{
    TTreeIdx current_node_idx = m_TreeModel->GetCurrentNodeIdx();

    if (current_node_idx == CPhyloTree::Null()) {
        _TRACE("CPhyloTreeDataSource::NewNode - Nothing is selected");
        return CPhyloTree::Null();
    }

    TTreeIdx parent_node_idx = m_TreeModel->GetNode(current_node_idx).GetParent();

    if (!after && parent_node_idx == CPhyloTree::Null()) {
        _TRACE("CPhyloTreeDataSource::NewNode - New root cannot be added");
        return CPhyloTree::Null();
    }

    TTreeIdx new_node_idx = m_TreeModel->AddNode();
    TNodeType& new_node = m_TreeModel->GetNode(new_node_idx);

    // Set node id to current maximim node id value +1.  IDs needed during export.
    CPhyloTreeMaxIdCalculator next_id(m_TreeModel);
    next_id = TreeDepthFirst(m_TreeModel.GetNCObject(), next_id);
    new_node->SetId(next_id.GetMaxId() + 1);    

    if (after) { // insert as a child
        TNodeType& current_node = m_TreeModel->GetCurrentNode();
        current_node.GetChildren().insert(current_node.SubNodeBegin(), new_node_idx);
        new_node.SetParent(m_TreeModel->GetCurrentNodeIdx());
    }
    else {
        m_TreeModel->RemoveChild(parent_node_idx, current_node_idx);
        m_TreeModel->AddChild(parent_node_idx, new_node_idx);
        m_TreeModel->AddChild(new_node_idx, current_node_idx);
    }
    
	// Set dictionary and a blank feature table with all values from dictionary.
    //new_node.GetValue().SetDictionaryPtr(&(m_TreeModel->GetFeatureDict()));
    // Do we need this - I don't see why... maybe dist?
    ITERATE(CBioTreeFeatureDictionary::TFeatureDict, it,
            m_TreeModel->GetFeatureDict().GetFeatureDict()) {
        new_node.GetValue().SetFeature(m_TreeModel->GetFeatureDict(), it->second, "");
    }

    new_node->Init(m_TreeModel->GetFeatureDict(), m_TreeModel->GetColorTable());

    m_TreeModel->UpdateNodesMapping();
    MeasureTree();
   
    return new_node_idx;
}


void CPhyloTreeDataSource::Remove(bool subtree)
{
    TTreeIdx current_node_idx = m_TreeModel->GetCurrentNodeIdx();
    if (current_node_idx == CPhyloTree::Null()) {
        _TRACE("CPhyloTreeDataSource::Remove - Nothing is selected");
        return;
    }

    TTreeIdx parent_node_idx = m_TreeModel->GetNode(current_node_idx).GetParent();
    if (parent_node_idx == CPhyloTree::Null()) {
        _TRACE("CPhyloTreeDataSource::Remove - The root node can't be removed");
        return;
    }

    m_TreeModel->ClearSelection();
    
    if (subtree) {
        // Remove the node and all it's subnodes
        TreeDepthFirst(m_TreeModel.GetNCObject(), current_node_idx, visitor_detach_subtree());
        m_TreeModel->RemoveChild(parent_node_idx, current_node_idx);      
    }
    else {
        // Remove the node and add all its children to its parent
        TNodeType& current_node = m_TreeModel->GetNode(current_node_idx);

        for(TNodeType::TNodeList_I  it = current_node.SubNodeBegin();
                                    it != current_node.SubNodeEnd(); it++ )  {
            m_TreeModel->AddChild(parent_node_idx, *it);
        }

        current_node.ClearConnections();
        m_TreeModel->RemoveChild(parent_node_idx, current_node_idx);
        current_node.GetValue().SetId(CPhyloNodeData::TID(-1));  
    }

    m_TreeModel->UpdateNodesMapping();
    MeasureTree(); 
}

void CPhyloTreeDataSource::RemoveSelected()
{
    vector<CPhyloNodeData::TID> sel;
    m_TreeModel->GetExplicitlySelectedIDs(sel);

    if (sel.size() == 0) {
        _TRACE("CPhyloTreeDataSource::RemoveSelected() - Nothing is selected");
        return;
    }

    // Make sure root node is not selected
    for (size_t i = 0; i < sel.size(); ++i) {
        CPhyloTree::TTreeIdx node_idx = m_TreeModel->FindNodeById(sel[i]);
        if (node_idx != CPhyloTree::Null()) {
            TTreeIdx parent_node_idx = m_TreeModel->GetNode(node_idx).GetParent();
            if (parent_node_idx == CPhyloTree::Null()) {
                _TRACE("CPhyloTreeDataSource::Remove - The root node can't be removed");
                return;
            }
        }
    }

    m_TreeModel->ClearSelection();

    // Delete nodes one at a time based on ID since indices may change in deletion
    // process.  Also, a node may disappear if it is the child of a node that has 
    // already been deleted.
    for (size_t i = 0; i < sel.size(); ++i)
    {
        CPhyloTree::TTreeIdx node_idx = m_TreeModel->FindNodeById(sel[i]);
        if (node_idx != CPhyloTree::Null()) {
            TTreeIdx parent_node_idx = m_TreeModel->GetNode(node_idx).GetParent();            
            TreeDepthFirst(m_TreeModel.GetNCObject(), node_idx, visitor_detach_subtree());
            m_TreeModel->RemoveChild(parent_node_idx, node_idx);
        }
    }

    m_TreeModel->UpdateNodesMapping();
    MeasureTree();
}

void CPhyloTreeDataSource::SetCollapsedLabel(CPhyloTree::TTreeIdx idx)
{
    /*  We update labels for nodes being collapsed only if the tree
    *  support the $PRIORITY (integer) parameter. In this case, we find the
    *  subnode with the highest $PRIORITY value and assign its label to
    *  the collapsed node. In the case that multiple sub-nodes have the same
    *  (highest) value for $PRIORITY, we pick the label of the subnode
    *  closest to the center of the subtree (center being the middle leaf node)
    */
    if (!m_TreeModel->GetFeatureDict().HasFeature("$PRIORITY") ||
        !m_TreeModel->GetFeatureDict().HasFeature("label"))
        return;

    if (idx != CPhyloTree::Null()) {
        CPhyloTree::TNodeType& current = m_TreeModel->GetNode(idx);

        if (current.CanExpandCollapse(CPhyloNodeData::eHideChildren)) {
            CBioTreeFeatureList& features = (*current).GetBioTreeFeatureList();
            TBioTreeFeatureId label_feat_id = m_TreeModel->GetFeatureDict().GetId("label");
            // We do not update labels here.  Only set the label if it is not already set
            if (features.GetFeatureValue(label_feat_id) != "")
                return;

            CPhyloTreePriorityNode  pnode(m_TreeModel, idx);
            pnode = TreeDepthFirst(*m_TreeModel, idx, pnode);
            if (pnode.GetMaxPriorityNode() != CPhyloTree::Null())
            {                
                CBioTreeFeatureList& priority_node_features = 
                    m_TreeModel->GetNode(pnode.GetMaxPriorityNode())->GetBioTreeFeatureList();

                string priority_label = priority_node_features.GetFeatureValue(label_feat_id);
                current->SetLabel(priority_label);

                if (m_TreeModel->GetFeatureDict().HasFeature("label")) {                                        
                    features.SetFeature(label_feat_id, priority_label);
                }
            }
        }
    }
}

void CPhyloTreeDataSource::SetCollapsedLabels(const vector<CPhyloNodeData::TID>& node_ids)
{
    for (size_t i = 0; i < node_ids.size(); ++i) {
        CPhyloTree::TTreeIdx idx = m_TreeModel->FindNodeById(node_ids[i]);
        if (idx != CPhyloTree::Null()) {
            CPhyloTree::TNodeType& current =
                m_TreeModel->GetNode(idx);

            if (current.CanExpandCollapse(CPhyloNodeData::eHideChildren)) {
                SetCollapsedLabel(idx);
            }
        }
    }
}

void CPhyloTreeDataSource::CollapseSelected()
{
    vector<TTreeIdx> sel;
    m_TreeModel->GetExplicitlySelected(sel);
    TTreeIdx root_idx = m_TreeModel->GetRootIdx();

    if (sel.size() == 0) {
        _TRACE("CPhyloTreeDataSource::RemoveSelected() - Nothing is selected");
        return;
    }
   
    // Collapse all  selected nodes.  Do not collapse nodes if they
    // have a collapsed parent or a parent in 'sel' which is also going to be
    // collapsed.
    for (size_t i = 0; i < sel.size(); ++i)
    {
        CPhyloTree::TTreeIdx node_idx = sel[i];
        bool collapse_node = true;

        if (m_TreeModel->GetNode(node_idx).CanExpandCollapse(CPhyloNodeData::eHideChildren)) {
            TTreeIdx parent_node_idx = m_TreeModel->GetNode(node_idx).GetParent();
            while (parent_node_idx != root_idx) {
                // If parent node is already collapsed:
                if (!m_TreeModel->GetNode(parent_node_idx).CanExpandCollapse(CPhyloNodeData::eHideChildren)) {
                    collapse_node = false;
                    break;
                }
                
                // if parent node is also in the list, sel, of nodes to be collapsed:
                if (std::find(sel.begin(), sel.end(), parent_node_idx) != sel.end()) {
                    collapse_node = false;
                    break;
                }

                parent_node_idx = m_TreeModel->GetNode(parent_node_idx).GetParent();
            }

            // no parent nodes were already collapsed, so this node can be collapsed:
            if (collapse_node) {
                SetCollapsedLabel(node_idx);
                m_TreeModel->GetNode(node_idx).ExpandCollapse(m_TreeModel->GetFeatureDict(),
                    CPhyloNodeData::eHideChildren);
            }
        }
    }

    MeasureTree();
}

void CPhyloTreeDataSource::MoveNode(bool up)
{
    TTreeIdx current_node_idx = m_TreeModel->GetCurrentNodeIdx();

    if (current_node_idx == CPhyloTree::Null()) {
        _TRACE("CPhyloTreeDataSource::MoveNode - Nothing is selected");
        return;
    }

    TTreeIdx parent_node_idx = m_TreeModel->GetNode(current_node_idx).GetParent();

    if (parent_node_idx == CPhyloTree::Null()) {
        _TRACE("CPhyloTreeDataSource::MoveNode - The root node can't be moved");
        return;
    }

    TNodeType& parent = m_TreeModel->GetNode(parent_node_idx);

    for(TNodeType::TNodeList_I  it = parent.SubNodeBegin();
                                it != parent.SubNodeEnd(); it++ )  {
        if (*it == current_node_idx) {
            if (up && (it!=parent.SubNodeBegin())) {
                TNodeType::TNodeList_I pre = it--;
                swap(*pre, *it);
                break;
            }
            else {
                TNodeType::TNodeList_I last = parent.SubNodeEnd(); last--;
                if (it != last) {
                    TNodeType::TNodeList_I pst = it++;
                    swap(*pst, *it);
                    break;
                }
            }
        }
    }
}



END_NCBI_SCOPE
