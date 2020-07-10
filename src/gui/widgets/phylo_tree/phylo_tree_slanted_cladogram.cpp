/*  $Id: phylo_tree_slanted_cladogram.cpp 43692 2019-08-14 18:15:15Z katargir $
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
#include <gui/widgets/phylo_tree/phylo_tree_slanted_cladogram.hpp>


BEGIN_NCBI_SCOPE

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////


CPhyloSlantedCladogram::CPhyloSlantedCladogram()
{
    m_DistMode = false;
}

CPhyloSlantedCladogram::CPhyloSlantedCladogram(double w, double h)
    : IPhyloTreeRender(w, h)
{
    m_DistMode = false;
}


CPhyloSlantedCladogram::~CPhyloSlantedCladogram()
{
}


void  CPhyloSlantedCladogram::x_Layout(CPhyloTreeDataSource& ds)
{
    m_Leafs = ds.GetSize();
    m_Width = ds.GetWidth();

    m_yStep = m_DimY / ((m_Leafs>1)?(m_Leafs-1):2);
    m_xStep = m_yStep;

    ComputePixelHeight ph = TreeDepthFirst(*ds.GetTree(), ComputePixelHeight(this));
    m_TreePixels = ph.m_PixelHeight;
    m_LabelPixels = ph.m_TotalHeight;

    CBoundaryPoints boundary_pts;
    m_DistMode = false;
    x_Calculate(ds.GetTree());

    if (ds.GetNormDistance() > 0){
        m_NormDistance = m_DimX /  ds.GetNormDistance();
    }
    else {
        m_NormDistance = 1.0;
    }

    // changing raster - x_Calculate for sure changed size in unpredictable way
    TModelRect newRect = ds.GetBoundRect();      

    m_RasterRect.Init(newRect.Left(),
                      newRect.Bottom(),
                      newRect.Right(),
                      newRect.Top());

    m_pPane->SetModelLimitsRect(m_RasterRect);
    m_pPane->SetVisibleRect(m_RasterRect);

    m_ValidLayout = eValidLayout;
    ComputeViewingLimits(*m_pPane, m_ForceSquare);
}

void CPhyloSlantedCladogram::x_SetGlRenderOptions(CPhyloTreeDataSource& ds)
{
    // Set drawing options that are specific to slanted cladogram
    CGlVboNode* edge_node = ds.GetModel().FindGeomNode("TreeEdges");
    CGlVboNode* narrow_edge_node = ds.GetModel().FindGeomNode("NarrowTreeEdges");
    CGlVboNode* filler_points_node = m_DS->GetModel().FindGeomNode("FillerPoints");

    if (edge_node != NULL && narrow_edge_node != NULL) {
        edge_node->GetState().Enable(GL_LINE_SMOOTH);
        edge_node->GetState().Enable(GL_BLEND);
        edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        narrow_edge_node->GetState().Enable(GL_LINE_SMOOTH);
        narrow_edge_node->GetState().Enable(GL_BLEND);
        narrow_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (filler_points_node != NULL)
        filler_points_node->SetVisible(false);
}

void  CPhyloSlantedCladogram::x_RenderVbo(CPhyloTreeDataSource& ds)
{    
    CGlVboNode* edge_node = m_DS->GetModel().FindGeomNode("TreeEdges");
    CGlVboNode* narrow_edge_node = m_DS->GetModel().FindGeomNode("NarrowTreeEdges");
    CGlVboNode* point_node = m_DS->GetModel().FindGeomNode("NodePoints");
    _ASSERT(edge_node != NULL && narrow_edge_node != NULL);

    //
    GLsizei edge_count = (GLsizei)ds.GetNumEdges(); 
    GLsizei node_count = (GLsizei)ds.GetNumNodes();

    CStopWatch timer;
    timer.Start();

    if (edge_count > 0) {        
        vector<float> color_coords;
        color_coords.reserve(node_count);

        vector<CVect2<float> > edge_node_coords;
        edge_node_coords.reserve(edge_count * 2);

        vector<CVect4<unsigned char> > edge_node_colors;
        edge_node_colors.reserve(edge_count * 2);

        x_DrawTreeVbo(edge_node_coords, edge_node_colors, color_coords, ds.GetTree());

        point_node->SetTexCoordBuffer1D(color_coords);

        edge_node->SetVertexBuffer2D(edge_node_coords);
        edge_node->SetColorBufferUC(edge_node_colors);

        point_node->ClearPositions();
        point_node->SetVisible(true);

        edge_node->SetDefaultPosition();
        edge_node->SetVisible(true);
        narrow_edge_node->SetDefaultPosition();
        narrow_edge_node->SetVisible(true);
    }
    else if (node_count == 1) {
        edge_node->SetVisible(false);
        narrow_edge_node->SetVisible(false);

        vector<float> color_coords;
        x_RenderNodeVbo(&ds.GetTree()->GetRoot(), ds.GetTree()->GetRootIdx(), color_coords);
        point_node->SetTexCoordBuffer1D(color_coords);

        point_node->ClearPositions();
        point_node->SetVisible(true);
    }
    else {
        edge_node->SetVisible(false);
        narrow_edge_node->SetVisible(false);
        point_node->SetVisible(false);
    }
}

class CCalcSlantedTree
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CCalcSlantedTree(CPhyloSlantedCladogram* clad,
                     CPhyloTreeDataSource* ds)
    : m_Clad(clad)
    , m_ComputeBoundary(0)
    {
        m_BoundaryPoints.push(CBoundaryPoints());
        m_Clad->InitExtents();

        m_yStep = m_Clad->GetDimY() / ((double)m_Clad->GetTreeHeight());
        m_xStep = m_yStep;
        m_SubtreeWidth = 0.0;
        m_yStepDefault = 2.0*std::max(m_Clad->GetScheme().GetNodeSize(), m_Clad->GetScheme().GetLeafNodeSize())*m_yStep;

        m_SubtreeExtentY.resize(ds->GetTree()->GetSize(), CVect2<double>(-std::numeric_limits<double>::max(), 
                                                                         std::numeric_limits<double>::max()));
        m_LastStepY = 0.0;
    }

    ETreeTraverseCode operator()(CPhyloTree& tree, 
                                 TTreeIdx node_idx, int delta)
    {   
        if (delta == 1) {
            m_BoundaryPoints.push(CBoundaryPoints());
        }

        CPhyloTree::TNodeType& node = tree[node_idx];
        if (delta==1 || delta==0){

            if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded)
                m_ComputeBoundary += 1;

            node.GetValue().SetAngle(0); // for correct labeling

            m_Clad->InitLabel(&tree, node);
            if (node.IsLeafEx()) {              
                double step_size = 2.0*double(m_Clad->GetNodeLayoutSize(&node))*m_yStep;

                // Need to keep track of the yextent (top to bottom range)
                // assigned to each node. For a leaf that is just the nodes y value
                m_SubtreeWidth += m_LastStepY / 2.0 + step_size / 2.0;
                double ypos = m_Clad->GetDimY() - m_SubtreeWidth;
                m_SubtreeExtentY[node_idx].X() = ypos;
                m_SubtreeExtentY[node_idx].Y() = ypos;

                node.GetValue().Y() = ypos;
                node.GetValue().X() = m_Clad->GetDimX();  // all leafs on the right

                m_LastStepY = step_size;

                m_Clad->CalculateExtents(&tree, node);

                CBoundaryPoints node_boundary_pts;
                m_Clad->x_ComputeNodeBoundary(&tree, node, node_boundary_pts, "SlantedCladogram");
                if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded)
                    m_ComputeBoundary -= 1;
            
                if (m_ComputeBoundary > 0) {
                    m_BoundaryPoints.top().AddBoundedPoints(node_boundary_pts);
                }
            }					
        }
        else if (delta == -1) {
            // We are skipping nodes with one child unless they are the root. Their 
            // positions are computed when we get to their first multi-child parent
            if (node.GetChildren().size() != 1 || node.GetParent() == CPhyloTree::Null()) {               
                bool single_children = false;

                // Compute the maximum and minimum y values of this node's immediate
                // children and save those in m_SubtreeExtentY.  If any of the children
                // only have 1 child of their own, descend into that subtree since we 
                // will need to compute the positions of any nodes that only have 1 child
                // because they can't be positioned the normal way (as the average of 
                // their children's positions)
                CPhyloTreeNode::TNodeList_I iter;
                for (iter = node.SubNodeBeginEx(); iter < node.SubNodeEndEx(); ++iter) {
                    size_t child_idx =*iter;

                    // Always make sure we take collapsed nodes into account
                    // here (collapsed nodes have 0 children)
                    size_t num_children = 0;
                    if (tree.GetNode(child_idx).Expanded())
                        num_children = tree.GetNode(child_idx).GetChildren().size();

                    // If any of the child nodes have themselves only 1 child node, follow down
                    // until we find a leaf or a node with more than one child and use that
                    // nodes position to properly space the current node.
                    while (num_children == 1) {                     
                        single_children = true;
                        // Get single child node of current child node
                        CPhyloTreeNode& n = tree.GetNode(child_idx);
                        child_idx = n.GetChildren()[0];

                        num_children = 0;
                        if (tree.GetNode(child_idx).Expanded())
                            num_children = tree.GetNode(child_idx).GetChildren().size();
                    }

                    m_SubtreeExtentY[node_idx].X() = std::max(m_SubtreeExtentY[node_idx].X(), m_SubtreeExtentY[child_idx].X());
                    m_SubtreeExtentY[node_idx].Y() = std::min(m_SubtreeExtentY[node_idx].Y(), m_SubtreeExtentY[child_idx].Y());
                }

                double yrange = m_SubtreeExtentY[node_idx].X() - m_SubtreeExtentY[node_idx].Y();

                // Set the nodes x && y position. Special case needed for root
                // to handle possibility that it only has 1 child.
                node.GetValue().Y() = m_SubtreeExtentY[node_idx].Y() + yrange / 2.0;
                if (node.GetParent() == CPhyloTree::Null()) {
                    double root_offset = 0.0;
                    if (node.Expanded() && node.GetChildren().size() == 1)
                        ++root_offset;
                    node.GetValue().X() = m_Clad->GetDimX() - (yrange + root_offset*2.0*m_yStep);
                }
                else {
                    node.GetValue().X() = m_Clad->GetDimX() - yrange;
                }

                m_Clad->CalculateExtents(&tree, node);

                // Calculate the positions iteratively of any single children
                // of the current node. Positions will be interpolated between
                // the position of the current node whose position we just
                // computed and the first child node that is either a leaf
                // or has more than one child (because leaves and nodes with more
                // than one child will have a position defined already)
                if (single_children) {
                    CPhyloTreeNode::TNodeList_I iter;
                    for (iter = node.SubNodeBeginEx(); iter < node.SubNodeEndEx(); ++iter) {
                        size_t child_idx = *iter;

                        size_t num_children = 0;
                        if (tree.GetNode(child_idx).Expanded())
                            num_children = tree.GetNode(child_idx).GetChildren().size();

                        double num_single_children = 0.0;

                        while (num_children == 1) {
                            ++num_single_children;

                            // Get single child node of current child node
                            CPhyloTreeNode& n = tree.GetNode(child_idx);
                            child_idx = n.GetChildren()[0];
                            num_children = 0;
                            if (tree.GetNode(child_idx).Expanded())
                                num_children = tree.GetNode(child_idx).GetChildren().size();

                            m_BoundaryPoints.push(CBoundaryPoints());
                            if (n.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded) {
                                m_ComputeBoundary += 1;
                            }
                        }

                        // If we have single-child nodes between current node and ultimate child_idx
                        // (child_idx being the first node that has > 1 children OR is a leaf).
                        if (child_idx != *iter) {
                            size_t intermediate_idx = *iter;
                            ++num_single_children;
                            double child_num = num_single_children - 1.0;

                            double x2 = tree.GetNode(child_idx).GetValue().X();
                            double x1 = node.GetValue().X();
                            double y2 = tree.GetNode(child_idx).GetValue().Y();
                            double y1 = node.GetValue().Y();
                
                            while (intermediate_idx != child_idx) {
                                child_idx = (tree)[child_idx].GetParent();
                                CPhyloTreeNode& n = tree.GetNode(child_idx);

                                // Set single child position as interpolated between current node and position
                                // of leaf node or last single child node (child_idx).
                                n.GetValue().Y() = y2*(child_num / num_single_children) +
                                    y1*(num_single_children - child_num) / num_single_children;
                                n.GetValue().X() = x2*(child_num / num_single_children) +
                                    x1*(num_single_children - child_num) / num_single_children;

                                // Add boundary information for current node in case
                                // nodes have bounding area
                                CBoundaryPoints pts = m_BoundaryPoints.top();
                                m_BoundaryPoints.pop();
                                pts.AddBoundedPoints(m_CurrentBoundaryPoints);
                                m_Clad->x_ComputeNodeBoundary(&tree, n, pts, "SlantedCladogram");

                                if (n.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded) {
                                    m_ComputeBoundary -= 1;
                                }

                                if (m_ComputeBoundary > 0) {
                                    m_BoundaryPoints.top().AddBoundedPoints(pts);
                                }

                                --child_num;
                                m_Clad->CalculateExtents(&tree, n);
                            }
                        }
                    }
                }

                // Each node that computes a boundary has to add in the points (and text rectangles)
                // for all nodes below it in the tree
                CBoundaryPoints pts = m_BoundaryPoints.top();
                m_BoundaryPoints.pop();
                m_Clad->x_ComputeNodeBoundary(&tree, node, pts, "SlantedCladogram");

                if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded) {
                    m_ComputeBoundary -= 1;
                }

                if (m_ComputeBoundary > 0) {
                    m_BoundaryPoints.top().AddBoundedPoints(pts);
                }
            }
            // If node has 1 child, just handle boundary info. position will 
            // be computed later.
            else {
                CBoundaryPoints pts = m_BoundaryPoints.top();
                m_BoundaryPoints.pop();

                if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded) {
                    m_ComputeBoundary -= 1;
                }
                if (m_ComputeBoundary > 0) {
                    m_CurrentBoundaryPoints = pts;
                    m_BoundaryPoints.top().AddBoundedPoints(pts);
                }
            }
        }

        return eTreeTraverse;
    }

    double GetYStep() const { return m_yStepDefault; }

private:
    CPhyloSlantedCladogram* m_Clad;
    int m_ComputeBoundary;
    stack<CBoundaryPoints> m_BoundaryPoints;
    double  m_xStep, m_yStep;
    double m_yStepDefault;
    double m_SubtreeWidth;
    double m_LastStepY;

    // boundary points to remember value for single-child nodes because they
    // are processed out-of-order (they are processed when the root or
    // their first multi-child parent is encountered)
    CBoundaryPoints m_CurrentBoundaryPoints;

    /// for each node the width in pixels of the nodes underneath that node
    /// the y value is the lower bound and x is the upper bound
    vector<CVect2<double> > m_SubtreeExtentY;
};

void CPhyloSlantedCladogram::x_Calculate(CPhyloTree* tree)
{
    CCalcSlantedTree  calc_tree(this, m_DS);
  
    TreeDepthFirstEx(*tree, calc_tree);
    m_yStep = calc_tree.GetYStep();
}

class CDrawSlantedTreeVbo
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CDrawSlantedTreeVbo(CPhyloSlantedCladogram* clad,
                        const CPhyloTreeScheme* sl,
                        vector<CVect2<float> >& edge_node_coords,
                        vector<CVect4<unsigned char> >& edge_node_colors,
                        vector<float>& color_coords)
    : m_Clad(clad)
    , m_SL(clad->GetScheme())
    , m_EdgeNodeCoords(edge_node_coords)
    , m_EdgeNodeColors(edge_node_colors)
    , m_ColorCoords(color_coords)
    , m_NodeSingleSelection(clad->GetSingleSelection())
    {
    }

    ETreeTraverseCode operator()(CPhyloTree& tree, 
                                 TTreeIdx node_idx, int delta)
    {
        if (delta==1 || delta==0) {
            CPhyloTree::TNodeType& node = tree[node_idx];
            m_Clad->x_RenderNodeVbo(&node, node_idx, m_ColorCoords);

            // draw edges from node to parent
            if (node.HasParent()){
                CPhyloTree::TNodeType& parent = (tree)[node.GetParent()];

                m_Clad->x_RenderLineVbo(node_idx,
                    &node,
                    &parent,
                    m_EdgeNodeCoords,
                    m_EdgeNodeColors,
                    parent.GetValue().X(),
                    parent.GetValue().Y(),
                    node.GetValue().X(),
                    node.GetValue().Y());
            }
        }
        return eTreeTraverse;
    }

private:
    CPhyloSlantedCladogram* m_Clad;
    const CPhyloTreeScheme& m_SL;
    vector<CVect2<float> >& m_EdgeNodeCoords;
    vector<CVect4<unsigned char> >& m_EdgeNodeColors;
    vector<float>& m_ColorCoords;

    // When user only wants to display a single, current selection from
    // the selection set (including traced and commmon nodes).
    CPhyloTree::TSelState& m_NodeSingleSelection;
};

void CPhyloSlantedCladogram::x_DrawTreeVbo(vector<CVect2<float> >& edge_node_coords,
                                           vector<CVect4<unsigned char> >& edge_node_colors,
                                           vector<float>& color_coords,
                                           CPhyloTree* tree)
{
    CDrawSlantedTreeVbo  draw_tree(this, m_SL.GetNCPointer(),
        edge_node_coords, edge_node_colors,
        color_coords);
  
    TreeDepthFirstEx(*tree, draw_tree);
}


string CPhyloSlantedCladogram::GetDescription(void)
{
    return "Slanted Cladogram";
}

END_NCBI_SCOPE
