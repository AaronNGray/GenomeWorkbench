/*  $Id: tree_collision_model2d.cpp 38377 2017-05-01 18:00:23Z falkrb $
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
#include <corelib/ncbistl.hpp>

#include <gui/widgets/phylo_tree/tree_collision_model2d.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/tree_graphics_model.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>

#include <gui/widgets/gl/attrib_menu.hpp> 
#include <corelib/ncbitime.hpp>


BEGIN_NCBI_SCOPE

CTreeCollisionModel2D::CTreeCollisionModel2D()
: m_Model(NULL)
{
#ifdef ATTRIB_MENU_SUPPORT
    m_VisUpdateTime = 0.0f;
    m_ScaleUpdateTime = 0.0f;

    CAttribMenu& m = CAttribMenuInstance::GetInstance();

    CAttribMenu* sub_menu = m.AddSubMenuUnique("Collision", this);

    sub_menu->AddIntReadOnly("CD Width", &m_Width);
    sub_menu->AddIntReadOnly("CD Height", &m_Height);

    sub_menu->AddFloatReadOnly("UpdateVis T", &m_VisUpdateTime);
    sub_menu->AddFloatReadOnly("UpdateScale T", &m_ScaleUpdateTime);

    sub_menu->AddFloat("Resolution Scaler", &m_ResolutionScaler, 1.0f, 0.1f, 10.0f, 0.05f);
#endif 
}

CTreeCollisionModel2D::~CTreeCollisionModel2D()
{
#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenuInstance::GetInstance().RemoveMenuR("Collision", this);
#endif
}

void CTreeCollisionModel2D::Update(const CGlPane& pane, const CVect2<float>& scale)
{
    Clear();

    m_Scale = scale;

    UpdateScaled(pane, m_Scale);    
}


void CTreeCollisionModel2D::UpdateScaled(const CGlPane& pane, const CVect2<float>& scale)
{
    if (m_Model == NULL)
        return;

    CStopWatch  t;
    t.Start();

    Clear();

    m_Scale = scale;
   

    vector<size_t>& node_indices = m_Model->GetNodeIndices();
    CPhyloTree* tree = m_Model->GetTree();

    vector<size_t>::const_iterator iter;
    for (iter=node_indices.begin(); iter!=node_indices.end(); ++iter) {
        CVect2<float> ll, ur;
        size_t idx = *iter;

        CPhyloTreeNode& n = (*tree)[idx];
       
        if (m_Model->GetRotateLabels()) {
            bool visible;
            float default_angle = n->GetAngle();
            float scaled_angle = m_Model->GetCurrentRotationAngle(pane, n, visible);

            n->SetAngle(scaled_angle);
            (*tree)[idx]->GetBoundingRect(m_Scale, ll, ur, m_Model->GetRotateLabels());
            n->SetAngle(default_angle);
        }
        else {
            (*tree)[idx]->GetBoundingRect(m_Scale, ll, ur, m_Model->GetRotateLabels());
        }
        
        CVect2<int> posi_ll(int(floorf(ll[0] / m_ResolutionX)),
                            int(floorf(ll[1] / m_ResolutionY)));
        CVect2<int> posi_ur(int(floorf(ur[0] / m_ResolutionX)),
                            int(floorf(ur[1] / m_ResolutionY)));
                           

        // Make sure we only look at sections inside our spatial subdivision (which
        // may not include regions when geometry is larger than the un-zoomed viewing
        // area)
        posi_ll.X() = std::max(posi_ll.X(), m_MinIdx.X());
        posi_ll.Y() = std::max(posi_ll.Y(), m_MinIdx.Y());
        posi_ur.X() = std::min(posi_ur.X(), m_MaxIdx.X());
        posi_ur.Y() = std::min(posi_ur.Y(), m_MaxIdx.Y());

        posi_ll -= m_MinIdx;
        posi_ur -= m_MinIdx;

        // Guarantee no out-of-bounds errors:
        posi_ll.X() = std::min(posi_ll.X(), m_Width - 1);
        posi_ll.Y() = std::min(posi_ll.Y(), m_Height - 1);
        posi_ur.X() = std::min(posi_ur.X(), m_Width - 1);
        posi_ur.Y() = std::min(posi_ur.Y(), m_Height - 1);

        CSpatialHash2D<CollisionEntry>::TVeci grid_pos;
        for (grid_pos.Y() = posi_ll.Y(); grid_pos.Y()<=posi_ur.Y(); ++grid_pos.Y()) {
            for (grid_pos.X() = posi_ll.X(); grid_pos.X()<=posi_ur.X(); ++grid_pos.X()) {
                Get(grid_pos).m_Labels.push_back(idx);
            }
        }
    }    

    m_ScaleUpdateTime = (float)t.Elapsed();
}

CPhyloTree::TTreeIdx CTreeCollisionModel2D::TestForNode(float x, float y, bool labels_visible, bool rotated_labels)
{
    CPhyloTree::TTreeIdx tree_idx = CPhyloTree::Null();
    if (m_Model == NULL)
        return tree_idx;

    /// Get index for cell that holds (x,y)
    TVeci idx(int(floorf(x/m_ResolutionX)), 
              int(floorf(y/m_ResolutionY)));  

    idx -= m_MinIdx;

    /// Get the set of nodes (indices) in the cell that (x,y) falls into
    std::vector<size_t> cell_nodes = GetSafe(idx).m_Labels;
    
    int best_vis_idx = -1;
    float best_vis_label_dist = FLT_MAX;

    int best_hidden_idx = -1;
    float best_hidden_label_dist = FLT_MAX;

    // iterate over all the nodes in the cell for (x,y) and check
    // to see if one is close enough to (x,y) to select it
    CPhyloTree* tree = m_Model->GetTree();

    for (size_t i=0; i<cell_nodes.size(); ++i) {
        CPhyloTreeNode& node = (*tree)[cell_nodes[i]];

        float r;
        CVect2<float> p = node->GetNodePosEx(CVect3<float>(m_Scale.X(), m_Scale.Y(), 1.0f), 
                                             m_SL.GetPointer(), r);
        // For selection need  a minimal size of about 3 pixel radius regardless of how large node is displayed
        // (nodes may not be displayed giving them a 0 size)
        if (r < 3.0f)
            r = 3.0f;
         
        // The node size is in pixels.  add a bit to make selection easier...   
        r += 2.0f;

        // A 'hit' should be within a couple of pixels...
        float xdelta = (x-p.X()) * 1.0f/m_Scale.X();
        float ydelta = (y-p.Y()) * 1.0f/m_Scale.Y();

        float dist = sqrtf( xdelta*xdelta + ydelta*ydelta );

        if (dist < r) {
            bool vis_label = node->GetVisible() && labels_visible;

            // Favor nodes with visible labels over ones without:
            if (vis_label && dist < best_vis_label_dist) {
                best_vis_idx = cell_nodes[i];
                best_vis_label_dist = dist;
            }
            else if (!vis_label && dist < best_hidden_label_dist) {
                best_hidden_idx = cell_nodes[i];
                best_hidden_label_dist = dist;
            }
        }
        // Allow selection to occur by clicking on label too (when visible)
        else if (node->GetVisible() && labels_visible) { 
            // Check if user clicked inside label rectangle
            CVect2<float> ll;
            CVect2<float> ur;

            if (node->PointInTextBox(m_Scale, CVect2<float>(x,y), rotated_labels)) {
                // How good of a hit?  pretty binary if you click inside the label. Just
                // evaluate is if you clicked on the edge of the node.
                best_vis_idx = cell_nodes[i];
                best_vis_label_dist = node->GetNodeSize();
            }
        }
    }
  
    if (best_vis_idx > -1) {
        tree_idx = best_vis_idx;
    }
    else if (best_hidden_idx != -1) {
        tree_idx = best_hidden_idx;
    }

    return tree_idx;
}

vector<CPhyloTree::TTreeIdx> CTreeCollisionModel2D::SelectNodes(float x1, float y1, float x2, float y2)
{
    vector<CPhyloTree::TTreeIdx> sel_nodes;

    if (m_Model == NULL)
        return sel_nodes;

    // Get index for lower-left corner cell in which (x1,y1) falls.
    TVeci idx1(int(floorf(x1/m_ResolutionX)), 
               int(floorf(y1/m_ResolutionY)));  
    idx1 -= m_MinIdx;

    // Get index for upper-right corner cell in which (x2,y2) falls
    TVeci idx2(int(floorf(x2/m_ResolutionX)), 
               int(floorf(y2/m_ResolutionY)));
    idx2 -= m_MinIdx;

    // Iterate over all the cells (idx1-idx2)
    // and for each cell iterate over all the nodes in it too
    // see if they fall within (x1,x2)-(y1,y2)

    CPhyloTree* tree = m_Model->GetTree();

    for (int x = idx1.X(); x <= idx2.X(); ++x) {
        for (int y = idx1.Y(); y <= idx2.Y(); ++y) {
            TVeci idx(x,y);

            std::vector<size_t> cell_nodes = GetSafe(idx).m_Labels;

            // Find collisions between nodes hashed in this entry
            for (size_t i = 0; i<cell_nodes.size(); ++i) {
                CPhyloTreeNode& node = (*tree)[cell_nodes[i]];
                CVect2<float> p = node->XY() - node->GetNodeOffset();

                // A 'hit' will include the center of the node in the bounding box
                if (p.X() >= x1 && p.X() <= x2 &&
                    p.Y() >= y1 && p.Y() <= y2) {

                    sel_nodes.push_back(cell_nodes[i]);
                }
            }
        }
    }

    // A node may overalp more than one cell in the sptial data structure, so
    // we eliminate duplicates here 
    sort(sel_nodes.begin(), sel_nodes.end());
    sel_nodes.erase(std::unique(sel_nodes.begin(), sel_nodes.end()), sel_nodes.end());

    return sel_nodes;   
}

void CTreeCollisionModel2D::Sync(const CGlPane& pane, CTreeGraphicsModel* m)
{
    m_Model = m;

    Update(pane, m_Scale);
}

void CTreeCollisionModel2D::SetScheme(CPhyloTreeScheme& sl)
{
    m_SL.Reset(&sl);
}

bool CTreeCollisionModel2D::RectIntersect(const CVect2<float>& ll1,
                                          const CVect2<float>& ur1,
                                          const CVect2<float>& ll2,
                                          const CVect2<float>& ur2)
{    
    bool xoverlap = !( (ll2.X() > ur1.X() || ur2.X() < ll1.X()) );
    bool yoverlap = !( (ll2.Y() > ur1.Y() || ur2.Y() < ll1.Y()) );

    return (xoverlap && yoverlap);
}

struct CompareLabelX {
    CompareLabelX(CPhyloTree* tree,
                  const CVect2<float>& s) 
    : m_Tree(tree)
    , m_Scale(s) {}

    bool operator()(size_t lhs, size_t rhs) 
    { 
        return ((*m_Tree)[lhs]->GetMinX(m_Scale) <
                (*m_Tree)[rhs]->GetMinX(m_Scale)); 
    }

    CPhyloTree* m_Tree;
    CVect2<float> m_Scale;
};


void CTreeCollisionModel2D::UpdateVisibility(const TModelRect& r)
{
    if (m_Model == NULL)
        return;

    CStopWatch  t;
    t.Start();

    // Lower left and upper right corners of currently visible rectangle
    CVect2<float> ll((float)r.Left(), (float)r.Bottom());
    CVect2<float> ur((float)r.Right(), (float)r.Top());

    // Indices in the datastructure for lower-left and upper right corners of
    // current rectangle 
    CVect2<int> posi_ll(int(floorf(ll[0]/m_ResolutionX)) - m_MinIdx.X(), 
                        int(floorf(ll[1]/m_ResolutionY)) - m_MinIdx.Y());
    CVect2<int> posi_ur(int(floorf(ur[0]/m_ResolutionX)) - m_MinIdx.X(), 
                        int(floorf(ur[1]/m_ResolutionY)) - m_MinIdx.Y());


    m_VisMinIdx = posi_ll;
    m_VisMaxIdx = posi_ur;

    // Guarantee no out-of-bounds errors:
    posi_ll.X() = std::min(posi_ll.X(), m_Width-1);
    posi_ll.X() = std::max(posi_ll.X(), 0);

    posi_ll.Y() = std::min(posi_ll.Y(), m_Height-1);
    posi_ll.Y() = std::max(posi_ll.Y(), 0);

    posi_ur.X() = std::min(posi_ur.X(), m_Width-1);
    posi_ur.X() = std::max(posi_ur.X(), 0);

    posi_ur.Y() = std::min(posi_ur.Y(), m_Height-1);
    posi_ur.Y() = std::max(posi_ur.Y(), 0);

    // Indices in the datastructure for lower-left and upper right corners of
    // rectangle visible at previous update
    CVect2<int> prev_posi_ll(int(floorf(m_PrevVisMin[0]/m_ResolutionX)) - m_MinIdx.X(), 
                             int(floorf(m_PrevVisMin[1]/m_ResolutionY)) - m_MinIdx.Y());
    CVect2<int> prev_posi_ur(int(floorf(m_PrevVisMax[0]/m_ResolutionX)) - m_MinIdx.X(),
                             int(floorf(m_PrevVisMax[1]/m_ResolutionY)) - m_MinIdx.Y());

    // Guarantee no out-of-bounds errors:
    prev_posi_ll.X() = std::min(prev_posi_ll.X(), m_Width-1);
    prev_posi_ll.X() = std::max(prev_posi_ll.X(), 0);

    prev_posi_ll.Y() = std::min(prev_posi_ll.Y(), m_Height-1);
    prev_posi_ll.Y() = std::max(prev_posi_ll.Y(), 0);

    prev_posi_ur.X() = std::min(prev_posi_ur.X(), m_Width-1);
    prev_posi_ur.X() = std::max(prev_posi_ur.X(), 0);

    prev_posi_ur.Y() = std::min(prev_posi_ur.Y(), m_Height-1);
    prev_posi_ur.Y() = std::max(prev_posi_ur.Y(), 0);

    CPhyloTree* tree = m_Model->GetTree();

    // Since an individual element may be in multiple grid cells, we 
    // iterate twice - once to turn off visibility for any element that
    // overlaps a grid cell visible on the previous update, then again
    // to turn on visibility for all overlapped cells.

    // A last interation finds overlaps between labels and picks labels to
    // not draw.

    // Iterate over the whole grid subregion that was previously displayed
    // and update all labels to be 'not visible' in that region.
    for (int x = prev_posi_ll.X(); x <= prev_posi_ur.X(); ++x) {
        for (int y = prev_posi_ll.Y(); y <= prev_posi_ur.Y(); ++y) {
            TVeci idx(x,y);

            std::vector<size_t>& cell_nodes = Get(idx).m_Labels;
            // Set individual labels to be not visible
            for (size_t i=0; i<cell_nodes.size(); ++i) {
                (*tree)[cell_nodes[i]]->SetVisible(false);
            }
        }
    }

    // Iterate over the currently-visible grid which subdivides 
    // the space occupied by the tree into individual cells, 
    // turning visibility on for all non-empty labels in the region
    for (int x = posi_ll.X(); x <= posi_ur.X(); ++x) {
        for (int y = posi_ll.Y(); y <= posi_ur.Y(); ++y) {
            TVeci idx(x,y);
            std::vector<size_t>& cell_nodes = Get(idx).m_Labels;

            // Maintain sorted order here (could be moved to sync, but this will
            // be fast for subsequent (already sorted..) updates...
            // Sorting is desired so that we favor in overlaps the node starting
            // further to the left (picking a standard makes display more consistent).
            CompareLabelX  cmp(tree, m_Scale);
            std::sort(cell_nodes.begin(), cell_nodes.end(), cmp);

            /// Could also be more efficient here to update bounding rect data
            /// now rather than do it in inner AND outer loops below. (but we
            /// have to add data to label class, making array larger...

            // Turn on visibility for non-blank cells
            for (size_t i=0; i<cell_nodes.size(); ++i) {
                if ((*tree)[cell_nodes[i]]->GetDisplayLabel() != "")
                    (*tree)[cell_nodes[i]]->SetVisible(true);
            }
        }
    }

    // Find overlaps between visible labels and when that happens,
    // set the second label to not be visible (second label, due to sorting,
    // should be to the right of the first label).
    // We do not do this for rotated labels as that would require overlap checking 
    // with non-axis-aligned bounding boxes and curerntly we only store a labels
    // min/max x&&y extents.
    if (!m_Model->GetRotateLabels()) {
        for (int x = posi_ll.X(); x <= posi_ur.X(); ++x) {
            for (int y = posi_ll.Y(); y <= posi_ur.Y(); ++y) {
                TVeci idx(x,y);

                std::vector<size_t> cell_nodes = Get(idx).m_Labels;

                // Find collisions between nodes hashed in this entry   
                for (size_t i=0; i<cell_nodes.size(); ++i) {
                    CPhyloTreeNode& l1 = (*tree)[cell_nodes[i]];

                    // labels can be blank - e.g. if it is not a leaf and rendering is turned off
                    // for interior nodes
                    CVect2<float> ll1, ur1;
                    bool label1_visible = (l1->GetVisible() && l1->GetDisplayLabel().length() > 0);
                    if (label1_visible)
                        l1->GetLabelRect(m_Scale, ll1, ur1, m_Model->GetRotateLabels());

                    CVect2<float> nll1, nur1, nll2, nur2;
                    float scaled_nodesize_x1 = l1->GetNodeDisplaySize()*m_Scale.X();
                    float scaled_nodesize_y1 = l1->GetNodeDisplaySize()*m_Scale.Y();

                    nll1.X() = l1->X()-scaled_nodesize_x1;
                    nll1.Y() = l1->Y()-scaled_nodesize_y1;
                    nur1.X() = l1->X()+scaled_nodesize_x1;
                    nur1.Y() = l1->Y()+scaled_nodesize_y1;

                    for (size_t j=i+1; j<cell_nodes.size(); ++j) {
                        CPhyloTreeNode& l2 = (*tree)[cell_nodes[j]];
                        bool label2_visible = (l2->GetVisible() && l2->GetDisplayLabel().length() > 0);                             

                        // If labels overlap, set second label (on the right of the first) to not visible
                        if (label2_visible) {
                            CVect2<float> ll2, ur2;
                            l2->GetLabelRect(m_Scale, ll2, ur2,  m_Model->GetRotateLabels());
                            if (label1_visible && RectIntersect(ll1, ur1, ll2, ur2)) {
                                l2->SetVisible(false);
                            }
                            // check if second (to the right) label overlaps first node
                            else if (RectIntersect(ll2, ur2, nll1, nur1)) {
                                l2->SetVisible(false);
                            }
                        }

                        // check if first label overlaps second node
                        if (label1_visible) {
                            float scaled_nodesize_x2 = l1->GetNodeDisplaySize()*m_Scale.X();
                            float scaled_nodesize_y2 = l1->GetNodeDisplaySize()*m_Scale.Y();
                            nll2.X() = l2->X()-scaled_nodesize_x2;
                            nll2.Y() = l2->Y()-scaled_nodesize_y2;
                            nur2.X() = l2->X()+scaled_nodesize_x2;
                            nur2.Y() = l2->Y()+scaled_nodesize_y2;

                            if (RectIntersect(ll1, ur1, nll2, nur2)) {
                                l1->SetVisible(false);
                            }
                        }
                    }
                }
            }
        }
    }

    m_PrevVisMin = ll;
    m_PrevVisMax = ur;

    m_VisUpdateTime = t.Elapsed();
}



END_NCBI_SCOPE
