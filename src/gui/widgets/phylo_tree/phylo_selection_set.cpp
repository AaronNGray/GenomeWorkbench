/*  $Id: phylo_selection_set.cpp 36928 2016-11-15 14:28:10Z falkrb $
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
#include <gui/widgets/phylo_tree/phylo_selection_set.hpp>

#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>


BEGIN_NCBI_SCOPE

using namespace objects;

bool CPhyloSelectionSet::operator==(const CPhyloSelectionSet& rhs) const
{
    // We don't care about m_ClusterID and selected status for purposes
    // of equality
    return (m_SelectedIDs == rhs.m_SelectedIDs &&
        m_SelectionName == rhs.m_SelectionName &&
        m_SelectionColor == rhs.m_SelectionColor);
}


void CPhyloSelectionSet::RemoveDuplicateIDs()
{
    std::sort(m_SelectedIDs.begin(), m_SelectedIDs.end());
    m_SelectedIDs.erase(std::unique(m_SelectedIDs.begin(), m_SelectedIDs.end()),
        m_SelectedIDs.end());
}

void CPhyloSelectionSet::InitFromUserObject(CRef< CUser_object > sel_set)
{
    if (sel_set->HasField("selection-name") &&
        sel_set->GetField("selection-name").GetData().IsStr()) {
        m_SelectionName = sel_set->GetField("selection-name").GetData().GetStr();
    }
    if (sel_set->HasField("color-red") && sel_set->HasField("color-green") && sel_set->HasField("color-blue") &&
        sel_set->GetField("color-red").GetData().IsReal() &&
        sel_set->GetField("color-green").GetData().IsReal() &&
        sel_set->GetField("color-blue").GetData().IsReal()) {
        m_SelectionColor.Set(float(sel_set->GetField("color-red").GetData().GetReal()),
            float(sel_set->GetField("color-green").GetData().GetReal()),
            float(sel_set->GetField("color-blue").GetData().GetReal()));
    }
    if (sel_set->HasField("selected") &&
        sel_set->GetField("selected").GetData().IsBool()) {
        m_Selected = sel_set->GetField("selected").GetData().GetBool();
    }
    if (sel_set->HasField("cluster-id") &&
        sel_set->GetField("cluster-id").GetData().IsInt()) {
        m_ClusterID = sel_set->GetField("cluster-id").GetData().GetInt();
    }
    if (sel_set->HasField("selection-ids") &&
        sel_set->GetField("selection-ids").GetData().IsInts()) {
        vector<int> node_ids = sel_set->GetField("selection-ids").GetData().GetInts();

        m_SelectedIDs.clear();
        for (size_t i = 0; i<node_ids.size(); ++i) {
            m_SelectedIDs.push_back(node_ids[i]);
        }
    }
}

CRef< CUser_object > CPhyloSelectionSet::SaveToUserObject() const
{
    CRef<CUser_object>  uo_sel_set;
    uo_sel_set.Reset(new CUser_object());

    CRef<CObject_id> uo_id;
    uo_id.Reset(new CObject_id());
    uo_id->SetStr("selection-set");
    uo_sel_set->SetType(*uo_id);

    uo_sel_set->AddField("selection-name", GetName());
    uo_sel_set->AddField("color-red", GetColor().GetRed());
    uo_sel_set->AddField("color-green", GetColor().GetGreen());
    uo_sel_set->AddField("color-blue", GetColor().GetBlue());
    uo_sel_set->AddField("selected", GetSelected());
    uo_sel_set->AddField("cluster-id", GetClusterID());

    // need ints, not size_t's:
    vector<int> selected;
    for (size_t j = 0; j<GetSelectionSet().size(); ++j)
        selected.push_back(int(GetSelectionSet()[j]));

    uo_sel_set->AddField("selection-ids", selected);

    return uo_sel_set;
}

bool CPhyloSelectionSetMgr::operator==(const CPhyloSelectionSetMgr& rhs) const
{
    if (m_SelectionSets.size() != rhs.m_SelectionSets.size())
        return false;

    for (size_t i = 0; i < m_SelectionSets.size(); ++i) {
        if (!(m_SelectionSets[i] == rhs.m_SelectionSets[i]))
            return false;
    }

    return true;
}

void CPhyloSelectionSetMgr::InitFromUserObject(CPhyloTree* tree_model,
                                               const CBioTreeContainer_Base::TUser& uo)
{
    if (uo.HasField("selection-sets") &&
        uo.GetField("selection-sets").GetData().IsObjects()) {
        vector< CRef< CUser_object > > sel_sets = uo.GetField("selection-sets").GetData().GetObjects();

        m_SelectionSets.resize(sel_sets.size());
        for (size_t i = 0; i < sel_sets.size(); ++i) {
            m_SelectionSets[i].InitFromUserObject(sel_sets[i]);
        }
        // sync with current model immediately after loading
        x_SyncSelectionSets(tree_model);
        // set selection
        SetSelectionSetProperty(tree_model); 
    }
}

void CPhyloSelectionSetMgr::SaveToUserObject(CBioTreeContainer_Base::TUser& uo)
{
    if (uo.HasField("selection-sets")) {
        CUser_object::TData::iterator it = uo.SetData().begin();
        while (it != uo.SetData().end()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr() &&
                (*it)->GetLabel().GetStr() == "selection-sets") {
                uo.SetData().erase(it);
                break;
            }
            ++it;
        }
    }

    if (m_SelectionSets.size() > 0) {
        // create a cuserobject with appropriate fields for each
        // entry, and add that under the name 'selection-sets' to uo:
        vector< CRef<CUser_object> > uo_sets;

        for (size_t i = 0; i < m_SelectionSets.size(); ++i) {
            CRef<CUser_object>  uo_sel_set = m_SelectionSets[i].SaveToUserObject();

            uo_sets.push_back(uo_sel_set);
        }
        uo.AddField("selection-sets", uo_sets);
    }
}

void CPhyloSelectionSetMgr::RemoveDuplicateIDs()
{
    for (auto& sel_set : m_SelectionSets) {
        sel_set.RemoveDuplicateIDs();
    }
}

map<int, size_t> CPhyloSelectionSetMgr::GetClusterToSelectionMap()
{
    map<int, size_t>  cluster_id_to_selection_map;

    for (size_t i = 0; i<m_SelectionSets.size(); ++i) {
        if (m_SelectionSets[i].GetSelected()) {
            int id = m_SelectionSets[i].GetClusterID();
            cluster_id_to_selection_map[id] = i;
        }
    }

    return cluster_id_to_selection_map;
}


void CPhyloSelectionSetMgr::SetSelectionSetProperty(CPhyloTree* tree_model)
{
    //  First clear any existing selection set cluster ids.
    size_t num_nodes = tree_model->GetSize();
    TBioTreeFeatureId cluster_id = tree_model->GetFeatureDict().Register("cluster-id");
    TBioTreeFeatureId sel_feat_id = tree_model->GetFeatureDict().Register("$SEL_CLUSTERS");
    TBioTreeFeatureId marker_feat_id = tree_model->GetFeatureDict().Register("marker");

    // Clear cluseter info before re-adding it following loop
    //------------------------------------------------------------------------------------
    if (sel_feat_id != (TBioTreeFeatureId)-1) {
        for (size_t i = 0; i<num_nodes; ++i) {
            CPhyloTreeNode& n = tree_model->GetNode(i);

            // clear all selection cluster info - that will be re-added below if there is any
            if (n.GetValue().HasSelClusters())
                n.GetValue().GetSelClusters().clear();
            n.GetValue().GetBioTreeFeatureList().RemoveFeature(sel_feat_id);

            // reset cluster id from feature list, if available
            string cid = n.GetValue().GetBioTreeFeatureList().GetFeatureValue(cluster_id);
            int cluster_id = -1;
            if (cid != "") {
                int sel_id = NStr::StringToInt(cid, NStr::fConvErr_NoThrow);
                if (sel_id || errno == 0)
                    cluster_id = sel_id;
            }
            n.GetValue().SetClusterID(cluster_id);
            n.GetValue().SetClusterColorIdx(-1);

            // Reset node marker from feature (if available) since it can also be overridden by
            // selection clusters
            if (n.GetValue().HasNodeMarker())
                n.GetValue().GetMarkerColors().clear();
            const string& marker = n.GetValue().GetBioTreeFeatureList().GetFeatureValue(marker_feat_id);
            try {
                if (!marker.empty()) {
                    n.GetValue().SetMarkerColors(marker);
                }
            }
            catch (std::exception&){}
        }
    }

    // Set selection-cluster property in feature list of all selected nodes 
    // in active selection sets.
    for (size_t i = 0; i<m_SelectionSets.size(); ++i) {
        if (m_SelectionSets[i].GetSelected()) {
            int cluster_id = m_SelectionSets[i].GetClusterID();
            string cid;
            NStr::IntToString(cid, cluster_id);

            for (size_t j = 0; j<m_SelectionSets[i].GetSelectionSet().size(); ++j) {
                objects::CNode::TId node_id = m_SelectionSets[i].GetSelectionSet()[j];
                CPhyloTree::TTreeIdx node_idx = tree_model->FindNodeById(node_id);

                // Node could be NULL if user edited (removed nodes from) tree or if
                // this is a subtree which would have copied only some nodes but 
                // inherits all user data - so some ids in selection-sets may not exist.
                // (cleaned up if missing_ids gets set to true)
                if (node_idx != CPhyloTree::Null()) {
                    CPhyloTreeNode& node = tree_model->GetNode(node_idx);
                    node.GetValue().GetSelClusters().push_back(cluster_id);
                    string sel_clusters = node.GetValue().GetBioTreeFeatureList().GetFeatureValue(sel_feat_id);
                    if (sel_clusters.length() > 0)
                        sel_clusters += " ";
                    sel_clusters += cid;
                    node.GetValue().GetBioTreeFeatureList().SetFeature(sel_feat_id, sel_clusters);
                }
            }
        }
    }
}

size_t CPhyloSelectionSetMgr::GetSelectionSet(const string& set_name)
{
    for (size_t i = 0; i < m_SelectionSets.size(); ++i) {
        if (m_SelectionSets[i].GetName() == set_name) {
            return i;
        }
    }

    return size_t(-1);
}

CPhyloSelectionSet& CPhyloSelectionSetMgr::AddSet(const string& set_name)
{
    size_t set_idx = GetSelectionSet(set_name);

    if (set_idx != size_t(-1))
        return m_SelectionSets[set_idx];

    vector<CRgbaColor> current_colors;
    int max_cluster_id = 500;
    for (size_t i = 0; i < m_SelectionSets.size(); ++i) {
        current_colors.push_back(m_SelectionSets[i].GetColor());
        max_cluster_id = std::max(m_SelectionSets[i].GetClusterID(), max_cluster_id);
    }

    CRgbaColor c = PickBestColor(current_colors);

    vector<objects::CNode::TId> ids;
    CPhyloSelectionSet new_set(ids, set_name);
    new_set.SetColor(c);
    new_set.SetClusterID(max_cluster_id);

    m_SelectionSets.push_back(new_set);
    return m_SelectionSets[m_SelectionSets.size() - 1];
}

CPhyloSelectionSet&  CPhyloSelectionSetMgr::AddSet(const string& set_name, const CRgbaColor& c)
{
    size_t set_idx = GetSelectionSet(set_name);

    if (set_idx != size_t(-1))
        return m_SelectionSets[set_idx];

    vector<objects::CNode::TId> ids;
    CPhyloSelectionSet new_set(ids, set_name);
    new_set.SetColor(c);

    m_SelectionSets.push_back(new_set);
    return m_SelectionSets[m_SelectionSets.size() - 1];
}

CRgbaColor CPhyloSelectionSetMgr::PickBestColor(const vector<CRgbaColor>& current_colors)
{
    CRgbaColor  color(102, 153, 204);
    CRgbaColor  white(255, 255, 255, 255);

    float num_colors = (float)current_colors.size();
    float color_step = num_colors ? (180.0f / num_colors) : 0;
    float rotate_angle = color_step + 100.0f;

    float max_color_dist = 0.0f;
    CRgbaColor best_color = color;

    // Find a color that is as far away visually as possible from the other
    // selection set colors and colors of normal clusters in the tree.
    for (size_t i = 0; i < 2 * current_colors.size(); ++i) {
        // protect from bright colors (almost white) since they are harder to see
        float c_dist = CRgbaColor::ColorDistance(white, color);
        while (c_dist < 0.7f) {
            color.Darken(0.1f);
            c_dist = CRgbaColor::ColorDistance(white, color);
        }

        float min_color_dist = 1e10f;

        // Find the closest color from among all current selection/cluster colors in use
        for (size_t j = 0; j < current_colors.size(); ++j) {
            float dist = CRgbaColor::ColorDistance(current_colors[j], color);
            if (dist < min_color_dist) {
                min_color_dist = dist;
            }
        }

        // If the closet color is further away than the closest color
        // for all previous colors wever tried, save the current color.
        if (min_color_dist > max_color_dist) {
            max_color_dist = min_color_dist;
            best_color = color;
        }

        color = CRgbaColor::RotateColor(color, rotate_angle);
    }

    return best_color;
}

void CPhyloSelectionSetMgr::x_SyncSelectionSets(CPhyloTree* tree_model)
{
    // Selection sets may be out of sync with actual tree if the 
    // tree was created as a subtree of another tree but inherited
    // the other trees full selections sets, or also if user
    // directly edits selection set.

    // First remove any ids that are not in the current tree
    for (size_t i = 0; i<m_SelectionSets.size(); ++i) {

        vector<objects::CNode::TId>  selected = m_SelectionSets[i].GetSelectionSet();
        m_SelectionSets[i].GetSelectionSet().clear();

        for (size_t j = 0; j<selected.size(); ++j) {
            objects::CNode::TId node_id = selected[j];
            CPhyloTree::TTreeIdx node_idx = tree_model->FindNodeById(node_id);

            if (node_idx != CPhyloTree::Null())
                m_SelectionSets[i].GetSelectionSet().push_back(node_id);
        }
    }

    // Now remove any empty selection sets
    vector<CPhyloSelectionSet> selection_sets = m_SelectionSets;
    m_SelectionSets.clear();

    for (size_t i = 0; i<selection_sets.size(); ++i) {
        if (selection_sets[i].GetSelectionSet().size() > 0) {
            m_SelectionSets.push_back(selection_sets[i]);
        }
    }
}

void CPhyloSelectionSetMgr::RenumberClusterIDs(int start_id)
{
    // set ids, first highest then lower.
    NON_CONST_ITERATE(vector<CPhyloSelectionSet>, sel_set, m_SelectionSets) {
        sel_set->SetClusterID(start_id--);
    }
}


END_NCBI_SCOPE
