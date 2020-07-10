/*  $Id: phylo_tree_radial.cpp 43692 2019-08-14 18:15:15Z katargir $
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
#include <gui/widgets/phylo_tree/phylo_tree_radial.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_render.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>


BEGIN_NCBI_SCOPE

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////


CPhyloRadial::CPhyloRadial()
{
}

CPhyloRadial::CPhyloRadial(double w, double h)
    : IPhyloTreeRender(w, h)
{
}

void  CPhyloRadial::x_Layout(CPhyloTreeDataSource& ds)
{
    Int4 leafs = ds.GetSize();
    Int4 width = ds.GetWidth();   

    m_xStep = m_DimX /  width;
    m_yStep = m_DimY / ((leafs>1)?(leafs-1):2);

    ComputePixelHeight ph = TreeDepthFirst(*ds.GetTree(), ComputePixelHeight(this));
    m_TreePixels = ph.m_PixelHeight;
    m_LabelPixels = ph.m_TotalHeight;

    if (ds.GetNormDistance() > 0){
        m_NormDistance = m_DimX /  ds.GetNormDistance();
    }
    else {
        m_NormDistance = 1.0;
    }

    x_Calculate(ds.GetTree());

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

void CPhyloRadial::x_SetGlRenderOptions(CPhyloTreeDataSource& ds)
{
    // Set drawing options that are specific to slanted cladogram
    CGlVboNode* edge_node = ds.GetModel().FindGeomNode("TreeEdges");
    CGlVboNode* narrow_edge_node = ds.GetModel().FindGeomNode("NarrowTreeEdges");
    CGlVboNode* sel_edge_node = ds.GetModel().FindGeomNode("SelectedTreeEdges");
    CGlVboNode* sel_narrow_edge_node = ds.GetModel().FindGeomNode("SelectedNarrowTreeEdges");
    CGlVboNode* filler_points_node = m_DS->GetModel().FindGeomNode("FillerPoints");
    CGlVboNode* sel_filler_points_node = m_DS->GetModel().FindGeomNode("SelectedFillerPoints");

    if (edge_node != NULL && narrow_edge_node != NULL) {
        edge_node->GetState().Enable(GL_LINE_SMOOTH);
        edge_node->GetState().Enable(GL_BLEND);
        edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        narrow_edge_node->GetState().Enable(GL_LINE_SMOOTH);
        narrow_edge_node->GetState().Enable(GL_BLEND);
        narrow_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        sel_edge_node->GetState().Enable(GL_LINE_SMOOTH);
        sel_edge_node->GetState().Enable(GL_BLEND);
        sel_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        sel_narrow_edge_node->GetState().Enable(GL_LINE_SMOOTH);
        sel_narrow_edge_node->GetState().Enable(GL_BLEND);
        sel_narrow_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (filler_points_node != NULL)
        filler_points_node->SetVisible(false);

    if (sel_filler_points_node != NULL)
        sel_filler_points_node->SetVisible(false);
}

void  CPhyloRadial::x_RenderVbo(CPhyloTreeDataSource& ds)
{    
    CGlVboNode* edge_node = m_DS->GetModel().FindGeomNode("TreeEdges");
    CGlVboNode* narrow_edge_node = m_DS->GetModel().FindGeomNode("NarrowTreeEdges");
    CGlVboNode* sel_edge_node = ds.GetModel().FindGeomNode("SelectedTreeEdges");
    CGlVboNode* sel_narrow_edge_node = ds.GetModel().FindGeomNode("SelectedNarrowTreeEdges");
    CGlVboNode* point_node = m_DS->GetModel().FindGeomNode("NodePoints");
    _ASSERT(edge_node != NULL && narrow_edge_node != NULL);

    //
    GLsizei edge_count = (GLsizei)ds.GetNumEdges();
    GLsizei node_count = (GLsizei)ds.GetNumNodes();

    if (edge_count > 0) {
        vector<float> color_coords;
        color_coords.reserve(node_count);

        vector<CVect2<float> > edge_node_coords, sel_edge_node_coords;
        edge_node_coords.reserve(edge_count * 2);
        sel_edge_node_coords.reserve(edge_count * 2);

        vector<CVect4<unsigned char> > edge_node_colors, sel_edge_node_colors;
        edge_node_colors.reserve(edge_count * 2);
        sel_edge_node_colors.reserve(edge_count * 2);
        x_DrawTreeVbo(edge_node_coords, edge_node_colors, sel_edge_node_coords, sel_edge_node_colors, color_coords, ds.GetTree());

        point_node->SetTexCoordBuffer1D(color_coords);

        edge_node->SetVertexBuffer2D(edge_node_coords);
        edge_node->SetColorBufferUC(edge_node_colors);

        sel_edge_node->SetVertexBuffer2D(sel_edge_node_coords);
        sel_edge_node->SetColorBufferUC(sel_edge_node_colors);

        point_node->ClearPositions();
        point_node->SetVisible(true);

        edge_node->SetVisible(true);
        edge_node->SetDefaultPosition();  

        narrow_edge_node->SetDefaultPosition();
        narrow_edge_node->SetVisible(true);

        sel_edge_node->SetVisible(true);
        sel_edge_node->SetDefaultPosition();

        sel_narrow_edge_node->SetDefaultPosition();
        sel_narrow_edge_node->SetVisible(true);
    }
    else if (node_count == 1) {
        edge_node->SetVisible(false);
        narrow_edge_node->SetVisible(false);
        sel_edge_node->SetVisible(false);
        sel_narrow_edge_node->SetVisible(false);
       
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

// Function to calculate node positions in tree. Essentially a recursive
// function (implemented as a loop) using TreeDepthFirstEx. This prevents
// very deep trees from the risk of stack overflow.
class CCalcRadialTree
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CCalcRadialTree(CPhyloRadial* clad,
                    CPhyloTreeDataSource* ds)
    : m_Clad(clad)
    , m_ComputeBoundary(0)
    {
        m_BoundaryPoints.push(CBoundaryPoints());
        m_Clad->InitExtents();

        m_xStep = m_Clad->GetDimX() /  (TModelUnit)ds->GetWidth();
        m_yStep = m_Clad->GetDimY() / (TModelUnit)((ds->GetSize()>1) ? (ds->GetSize() - 1) : 2);

        if (ds->GetNormDistance() > 0){
            m_NormDistance = m_Clad->GetDimX() /  ds->GetNormDistance();
        }
        else {
            m_NormDistance = 1.0;
        }

        m_AngleWedge.resize(ds->GetTree()->GetSize(), 0.0f);
        m_Radius = (float)sqrt(m_xStep*m_xStep + m_yStep*m_yStep);
        m_StartDegree = 0.0f;
    }

    ETreeTraverseCode operator()(CPhyloTree& tree, 
                                 TTreeIdx node_idx, int delta)
    {   
        CPhyloTree::TNodeType& node = tree[node_idx];

        if (delta==1)  {
            m_BoundaryPoints.push(CBoundaryPoints());
        }
        
        if (delta==1 || delta==0) {
            if (!node.HasParent()) { // root
                (*node).X() = m_Clad->GetDimX() / 2;
                (*node).Y() = m_Clad->GetDimY() / 2;
                (*node).SetAngle(0.);
                m_AngleWedge[node_idx] = 6.28f;
            }
            else {
                CPhyloTree::TNodeType& parent = tree.GetParent(node);

                (*node).X() = parent.GetValue().X();
                (*node).Y() = parent.GetValue().Y();

                float num_leaves = (float)(*node).GetNumLeavesEx();
                if (node.IsLeafEx())
                    num_leaves = 1.0f;
                float num_parent_leaves = (float)(parent.GetValue().GetNumLeavesEx());

                m_AngleWedge[node_idx] = m_AngleWedge[node.GetParent()] *
                                        (num_leaves / num_parent_leaves);										


                // annulus wedge degree
                float awd = m_AngleWedge[node_idx];

                // start degree
                if (node_idx == *parent.SubNodeBeginEx()){
                    m_StartDegree =
                        parent.GetValue().GetAngle() -
                        m_AngleWedge[node.GetParent()]/2.0;
                }

                // setting angle and position for this vertex
                double angle = m_StartDegree + (awd / 2.0);

                (*node).SetAngle(angle);

                if (m_Clad->GetDistRendering()){
                    m_Radius = (*node).GetDistance() * m_NormDistance;
                }
                // When adjacent leaves have dist==0, then they are written on top
                // of each other in distance-mode.  This forces them to have a mininmal
                // distance (1 node diameter which is 2*node size).  But this is also
                // not correct, so we'll leave it out for now.  User can always choose
                // not to use distances.
                //m_Radius = std::max(m_Radius, 
                //    2.0*(double)GetDefaultNodeSize(node)*(double)m_pPane->GetScaleX() );

                (*node).X() += m_Radius * cos(angle);
                (*node).Y() += m_Radius * sin(angle);

                m_StartDegree += awd;
            }

            if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded)
                m_ComputeBoundary += 1;

            // Compute position for leaves first, then on the
            // way up the tree, for interior nodes
            m_Clad->InitLabel(&tree, node);
            if (node.IsLeafEx())  {
                CBoundaryPoints node_boundary_pts;
                m_Clad->x_ComputeNodeBoundary(&tree, node, node_boundary_pts, "Radial");
                if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded)
                    m_ComputeBoundary -= 1;
            
                if (m_ComputeBoundary>0) {
                    m_BoundaryPoints.top().AddBoundedPoints(node_boundary_pts);
                }
            }			
			m_Clad->CalculateExtents(&tree, node);
        }
        else if (delta == -1) {          
            // Each node that computes a boundary has to add in the points (and text rectangles)
            // for all nodes below it in the tree
            CBoundaryPoints pts = m_BoundaryPoints.top();
            m_BoundaryPoints.pop();
            m_Clad->x_ComputeNodeBoundary(&tree, node, pts, "Radial");

            if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded) {
                m_ComputeBoundary -= 1;
            }

            if (m_ComputeBoundary>0) {
                m_BoundaryPoints.top().AddBoundedPoints(pts);
            }
        }

        return eTreeTraverse;
    }

    double GetYStep() const { return m_yStep; }

private:
    CPhyloRadial* m_Clad;
    int m_ComputeBoundary;
    stack<CBoundaryPoints> m_BoundaryPoints;
    float m_NormDistance;
    float m_Radius;
    float m_StartDegree;
    TModelUnit m_xStep, m_yStep;
 
    /// for each node - angle info. Only needed for layout so we put it 
    /// here rather than in node class
    vector<float> m_AngleWedge;
};

void CPhyloRadial::x_Calculate(CPhyloTree* tree)
{
    CCalcRadialTree  calc_tree(this, m_DS);
    TreeDepthFirstEx(*tree, calc_tree);

    // ystep is used to compute distance between nodes.  Since nodes are radial, about
    // twice the size gives a better approximation of average distance between nodes.
    m_yStep = calc_tree.GetYStep()*2.0;
}

class CDrawRadialTreeVbo
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CDrawRadialTreeVbo(CPhyloRadial* clad,
                       const CPhyloTreeScheme* sl,
                       vector<CVect2<float> >& edge_node_coords,
                       vector<CVect4<unsigned char> >& edge_node_colors,
                       vector<CVect2<float> >& sel_edge_node_coords,
                       vector<CVect4<unsigned char> >& sel_edge_node_colors,
                       vector<float>& color_coords)
    : m_Clad(clad)
    , m_SL(clad->GetScheme())
    , m_EdgeNodeCoords(edge_node_coords)
    , m_EdgeNodeColors(edge_node_colors)
    , m_SelEdgeNodeCoords(sel_edge_node_coords)
    , m_SelEdgeNodeColors(sel_edge_node_colors)
    , m_ColorCoords(color_coords)
    , m_NodeSingleSelection(clad->GetSingleSelection())
    {
    }

    ETreeTraverseCode operator()(CPhyloTree& tree, 
                                 TTreeIdx node_idx, int delta)
    {
        if (delta==1 || delta==0){
            CPhyloTree::TNodeType& node = tree[node_idx];
            m_Clad->x_RenderNodeVbo(&node, node_idx, m_ColorCoords);

            // draw edges from node to parent
            if (node.HasParent()){
                CPhyloTree::TNodeType& parent = (tree)[node.GetParent()];

                m_Clad->x_RenderLineVbo(node_idx,
                    &node,
                    &parent,
                    m_EdgeNodeCoords, m_EdgeNodeColors,
                    m_SelEdgeNodeCoords, m_SelEdgeNodeColors,
                    parent.GetValue().X(),
                    parent.GetValue().Y(),
                    node.GetValue().X(),
                    node.GetValue().Y());
            }
        }
        return eTreeTraverse;
    }

private:
    CPhyloRadial* m_Clad;
    const CPhyloTreeScheme& m_SL;
    vector<CVect2<float> >& m_EdgeNodeCoords;
    vector<CVect4<unsigned char> >& m_EdgeNodeColors;
    vector<CVect2<float> >& m_SelEdgeNodeCoords;
    vector<CVect4<unsigned char> >& m_SelEdgeNodeColors;
    vector<float>& m_ColorCoords;

    // When user only wants to display a single, current selection from
    // the selection set (including traced and commmon nodes).
    CPhyloTree::TSelState& m_NodeSingleSelection;
};

void CPhyloRadial::x_DrawTreeVbo(vector<CVect2<float> >& edge_node_coords,
                                 vector<CVect4<unsigned char> >& edge_node_colors,
                                 vector<CVect2<float> >& sel_edge_node_coords,
                                 vector<CVect4<unsigned char> >& sel_edge_node_colors,
                                 vector<float>& color_coords,
                                 CPhyloTree* tree)
{
    CDrawRadialTreeVbo  draw_tree(this, m_SL.GetNCPointer(),
        edge_node_coords, edge_node_colors,
        sel_edge_node_coords, sel_edge_node_colors,
        color_coords);
  
    TreeDepthFirstEx(*tree, draw_tree);
}


string CPhyloRadial::GetDescription(void)
{
    return "Radial Tree";
}

END_NCBI_SCOPE
