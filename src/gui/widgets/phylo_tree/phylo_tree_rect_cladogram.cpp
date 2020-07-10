/*  $Id: phylo_tree_rect_cladogram.cpp 43692 2019-08-14 18:15:15Z katargir $
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
#include <gui/widgets/phylo_tree/phylo_tree_rect_cladogram.hpp>

#include <gui/opengl/glutils.hpp>


BEGIN_NCBI_SCOPE

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////


CPhyloRectCladogram::CPhyloRectCladogram()
{
    m_DistMode = true;
    m_DistanceBarLowerLeft = false;
}

CPhyloRectCladogram::CPhyloRectCladogram(double w, double h)
: IPhyloTreeRender(w, h)
{
    m_DistMode = true;
    m_DistanceBarLowerLeft = false;
}


CPhyloRectCladogram::~CPhyloRectCladogram()
{
}

void  CPhyloRectCladogram::x_Layout(CPhyloTreeDataSource& ds)
{
    Int4 leafs = ds.GetSize();
    Int4 width = ds.GetWidth();

    if (width>0){
        m_xStep = m_DimX /  width;
    }
    else {
        m_xStep = m_DimX /  2;
    }

    if (leafs>1){
        m_yStep = m_DimY / leafs;
    }
    else {
        m_yStep = m_DimY / 2;
    }

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

void CPhyloRectCladogram::x_SetGlRenderOptions(CPhyloTreeDataSource& ds)
{
    // Set drawing options that are specific to slanted cladogram
    CGlVboNode* edge_node = ds.GetModel().FindGeomNode("TreeEdges");
    CGlVboNode* narrow_edge_node = ds.GetModel().FindGeomNode("NarrowTreeEdges");
    CGlVboNode* sel_edge_node = ds.GetModel().FindGeomNode("SelectedTreeEdges");
    CGlVboNode* sel_narrow_edge_node = ds.GetModel().FindGeomNode("SelectedNarrowTreeEdges");
    CGlVboNode* filler_points_node = m_DS->GetModel().FindGeomNode("FillerPoints");
    CGlVboNode* sel_filler_points_node = m_DS->GetModel().FindGeomNode("SelectedFillerPoints");
    CGlVboNode* point_node = m_DS->GetModel().FindGeomNode("NodePoints");

    if (edge_node != NULL &&
        narrow_edge_node != NULL &&
        sel_edge_node != NULL &&
        sel_narrow_edge_node != NULL &&
        filler_points_node != NULL &&
        sel_filler_points_node != NULL) {
        if (!m_UseSplines){
            edge_node->GetState().Disable(GL_LINE_SMOOTH);
            // blending still needed for the selectionvisibility option that lowers
            // alpha for non-selected elements
            edge_node->GetState().Enable(GL_BLEND);
            edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            sel_edge_node->GetState().Disable(GL_LINE_SMOOTH);           
            sel_edge_node->GetState().Enable(GL_BLEND);
            sel_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            filler_points_node->SetVisible(true);
            sel_filler_points_node->SetVisible(true);

            narrow_edge_node->GetState().Disable(GL_LINE_SMOOTH);
            narrow_edge_node->GetState().Enable(GL_BLEND);
            narrow_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            sel_narrow_edge_node->GetState().Disable(GL_LINE_SMOOTH);
            sel_narrow_edge_node->GetState().Enable(GL_BLEND);
            sel_narrow_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else {
            // Curved lines - need to smooth them
            edge_node->GetState().Enable(GL_LINE_SMOOTH);
            edge_node->GetState().Enable(GL_BLEND);
            edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            sel_edge_node->GetState().Enable(GL_LINE_SMOOTH);
            sel_edge_node->GetState().Enable(GL_BLEND);
            sel_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            filler_points_node->SetVisible(false);
            sel_filler_points_node->SetVisible(false);

            narrow_edge_node->GetState().Enable(GL_LINE_SMOOTH);
            narrow_edge_node->GetState().Enable(GL_BLEND);
            narrow_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            sel_narrow_edge_node->GetState().Enable(GL_LINE_SMOOTH);
            sel_narrow_edge_node->GetState().Enable(GL_BLEND);
            sel_narrow_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        if (point_node != NULL) {
            point_node->GetState().Enable(GL_BLEND);
            point_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }
}

void  CPhyloRectCladogram::x_RenderVbo(CPhyloTreeDataSource& ds)
{
    CGlVboNode* point_node = m_DS->GetModel().FindGeomNode("NodePoints");
    _ASSERT(point_node != NULL);

    GLsizei node_count = (GLsizei)ds.GetNumNodes();    

    if (node_count > 0) {
        if (m_UseSplines)
            x_RenderVboSplines(ds);
        else
            x_RenderVboPseudoSplines(ds);

        point_node->ClearPositions();
        point_node->SetVisible(true);
    }
    else {
        point_node->SetVisible(false);
    }
}

void  CPhyloRectCladogram::x_RenderVboSplines(CPhyloTreeDataSource& ds)
{
    CGlVboNode* edge_node = m_DS->GetModel().FindGeomNode("TreeEdges");
    _ASSERT(edge_node != NULL);
    CGlVboNode* sel_edge_node = m_DS->GetModel().FindGeomNode("SelectedTreeEdges");
    _ASSERT(sel_edge_node != NULL);

    CGlVboNode* point_node = m_DS->GetModel().FindGeomNode("NodePoints");
    _ASSERT(point_node != NULL);

    GLsizei node_count = (GLsizei)ds.GetNumNodes();
    GLsizei edge_count = (GLsizei)ds.GetNumEdges();

    int estimatedSize = edge_count * 45;

    if (ds.GetNumEdges() > 0) {
        vector<float> color_coords;
        color_coords.reserve(node_count);

        vector<CVect2<float> > edge_node_coords, sel_edge_node_coords;
        edge_node_coords.reserve(estimatedSize);
        sel_edge_node_coords.reserve(estimatedSize);

        vector<CVect4<unsigned char> > edge_node_colors, sel_edge_node_colors;
        edge_node_colors.reserve(estimatedSize);
        sel_edge_node_colors.reserve(estimatedSize);

        x_DrawSplineTree(color_coords, edge_node_coords, edge_node_colors, sel_edge_node_coords, sel_edge_node_colors);
        point_node->SetTexCoordBuffer1D(color_coords);

        edge_node->SetVertexBuffer2D(edge_node_coords);
        edge_node->SetColorBufferUC(edge_node_colors);

        sel_edge_node->SetVertexBuffer2D(sel_edge_node_coords);
        sel_edge_node->SetColorBufferUC(sel_edge_node_colors);
    }
    else {
        // Render single node:
        if (ds.GetTree() != NULL) {
            vector<float> color_coords;
            x_RenderNodeVbo(&ds.GetTree()->GetRoot(), ds.GetTree()->GetRootIdx(), color_coords);
            point_node->SetTexCoordBuffer1D(color_coords);
        }
    }
}

void  CPhyloRectCladogram::x_RenderVboPseudoSplines(CPhyloTreeDataSource& ds)
{
    CGlVboNode* edge_node = m_DS->GetModel().FindGeomNode("TreeEdges");
    _ASSERT(edge_node != NULL);
    CGlVboNode* sel_edge_node = m_DS->GetModel().FindGeomNode("SelectedTreeEdges");
    _ASSERT(sel_edge_node != NULL);

    CGlVboNode* point_node = m_DS->GetModel().FindGeomNode("NodePoints");
    _ASSERT(point_node != NULL);

    // The pseudo spline is drawn as 2 lines per edge
    GLsizei edge_count = (GLsizei)ds.GetNumEdges();
    GLsizei node_count = (GLsizei)ds.GetNumNodes();

    int vertCount = edge_count * 4;

    if (edge_count > 0) {
        // Since any number from 0 to the total number of edges may be selected, we allocate
        //both the nodes for (non-selected) edges and edges to be the same size.
        vector<float> color_coords;
        color_coords.reserve(node_count);

        vector<CVect2<float> > edge_node_coords, sel_edge_node_coords;
        edge_node_coords.reserve(vertCount);
        sel_edge_node_coords.reserve(vertCount);

        vector<CVect4<unsigned char> > edge_node_colors, sel_edge_node_colors;
        edge_node_colors.reserve(vertCount);
        sel_edge_node_colors.reserve(vertCount);

        x_DrawPseudoSplineTree(color_coords, edge_node_coords, edge_node_colors, sel_edge_node_coords, sel_edge_node_colors);
        point_node->SetTexCoordBuffer1D(color_coords);

        edge_node->SetVertexBuffer2D(edge_node_coords);
        edge_node->SetColorBufferUC(edge_node_colors);

        sel_edge_node->SetVertexBuffer2D(sel_edge_node_coords);
        sel_edge_node->SetColorBufferUC(sel_edge_node_colors);
    }
    else {
        // Render single node:
        if (ds.GetTree() != NULL) {
            vector<float> color_coords;
            x_RenderNodeVbo(&ds.GetTree()->GetRoot(), ds.GetTree()->GetRootIdx(), color_coords);
            point_node->SetTexCoordBuffer1D(color_coords);
        }
    }
    CGlUtils::CheckGlError();
}

void  CPhyloRectCladogram::x_SetElementVisibility()
{
    // Want to turn filler points on/off based on zoom (but only when using
    // non-spline rendering - spline doesn't use them)
    if (!m_UseSplines) {
        CGlVboNode* filler_points_node = m_DS->GetModel().FindGeomNode("FillerPoints");
        CGlVboNode* sel_filler_points_node = m_DS->GetModel().FindGeomNode("SelectedFillerPoints");

        TModelUnit line_width = 1.0;

        if (x_WideLines()) {
            line_width = m_SL->SetSize(CPhyloTreeScheme::eLineWidth);
        }

        if (filler_points_node != NULL &&
            sel_filler_points_node != NULL) {
            filler_points_node->GetState().PointSize((float)line_width);
            filler_points_node->SetVisible(line_width > 1.0f);

            sel_filler_points_node->GetState().PointSize((float)line_width);
            sel_filler_points_node->SetVisible(line_width > 1.0f);
        }
    }


    IPhyloTreeRender::x_SetElementVisibility();
}

// Function to calculate node positions in tree. Essentially a recursive
// function (implemented as a loop) using TreeDepthFirstEx. This prevents
// very deep trees from the risk of stack overflow.
class CCalcRectangularTree
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CCalcRectangularTree(CPhyloRectCladogram* clad,
                         CPhyloTreeDataSource* ds)
    : m_Clad(clad)
    , m_ComputeBoundary(0)
    , m_DistFromRoot(0.0f)
    , m_LastDist(0.0f)
    {
        m_BoundaryPoints.push(CBoundaryPoints());

		m_Clad->InitExtents();

        int leaves = ds->GetSize();
        int width = ds->GetWidth();
        if (width>0){
            m_xStep = m_Clad->GetDimX() /  width;
        }
        else {
            m_xStep = m_Clad->GetDimX() /  2;
        }

        if (leaves > 1){
            m_yStep = m_Clad->GetDimY() / leaves;
        }
        else {
            m_yStep = m_Clad->GetDimY() / 2;
        }

        if (ds->GetNormDistance() > 0){
            m_NormDistance = m_Clad->GetDimX() /  ds->GetNormDistance();
        }
        else {
            m_NormDistance = 1.0;
        };

        m_yDelta = 0.0;
        m_DefaultNodeSize = std::max(m_Clad->GetScheme().GetLeafNodeSize(), m_Clad->GetScheme().GetNodeSize());

        m_PixelHeight = 0.0f;
        m_TotalHeight = 0.0f;
        m_FontHeight = (float)clad->GetScheme().GetFont().GetFontSize();
    }

    ETreeTraverseCode operator()(CPhyloTree& tree, 
                                 TTreeIdx node_idx, int delta)
    {   
        CPhyloTree::TNodeType& node = tree[node_idx];

        if (delta==1)  {
            m_BoundaryPoints.push(CBoundaryPoints());
        }
        
        if (delta==1 || delta==0) {
            node->SetAngle(0.0f);
            if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded)
                m_ComputeBoundary += 1;

            if (delta == 0) {
                m_DistFromRoot -=  m_LastDist;
            }

            m_LastDist = (*node).GetDistance();
            m_DistFromRoot +=  m_LastDist;

            // Compute position for leaves first, then on the
            // way up the tree, for interior nodes
            m_Clad->InitLabel(&tree, node);
            if (node.IsLeafEx())  {
                if (m_Clad->GetDistRendering()) {
                    (*node).X()  = m_NormDistance * m_DistFromRoot;
                }
                else {
                    (*node).X()  = m_Clad->GetDimX();   // all leafs on the right
                }
                double this_node_size = (double)m_Clad->GetNodeLayoutSize(&node);
                
                m_PixelHeight += this_node_size*2.0;
                if (m_FontHeight > this_node_size)
                    m_TotalHeight += m_FontHeight;
                else
                    m_TotalHeight += this_node_size*2.0;

                double ydelta = 0.0;
                if (this_node_size > m_DefaultNodeSize) {
                    ydelta = 0.5*(((this_node_size / m_DefaultNodeSize) - 1.0) * m_yStep);
                    m_yDelta += ydelta;            
                }
                
                (*node).Y() = (m_Clad->GetDimY() - (m_yDelta + (*node).IDX().second * m_yStep));     // y coordinate    
                m_Clad->CalculateExtents(&tree, node);

                m_yDelta += ydelta;

                CBoundaryPoints node_boundary_pts;
                m_Clad->x_ComputeNodeBoundary(&tree, node, node_boundary_pts, "RectCladogram");
                if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded)
                    m_ComputeBoundary -= 1;
            
                if (m_ComputeBoundary>0) {
                    m_BoundaryPoints.top().AddBoundedPoints(node_boundary_pts);
                }
            }			            
        }
        else if (delta == -1) {          
            CPhyloTreeNode::TNodeList_I fst = node.SubNodeBegin();
            CPhyloTreeNode::TNodeList_I lst = node.SubNodeEnd();
            lst--;

            m_DistFromRoot -= m_LastDist;

            double yf = (tree)[*fst]->XY().Y();
            double yl = (tree)[*lst]->XY().Y();

            if (m_Clad->GetDistRendering()){
                (*node).XY().X()  = m_NormDistance * m_DistFromRoot; 
            }
            else {
                (*node).XY().X()  = ((*node).IDX().first * m_xStep);
            }

            (*node).XY().Y() = (yl + (yf - yl)/2);
            m_Clad->CalculateExtents(&tree, node);

            // Each node that computes a boundary has to add in the points (and text rectangles)
            // for all nodes below it in the tree
            CBoundaryPoints pts = m_BoundaryPoints.top();
            m_BoundaryPoints.pop();
            m_Clad->x_ComputeNodeBoundary(&tree, node, pts, "RectCladogram");

            if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded) {
                m_ComputeBoundary -= 1;
            }

            if (m_ComputeBoundary>0) {
                m_BoundaryPoints.top().AddBoundedPoints(pts);
            }

            m_LastDist = (*node).GetDistance();
        }

        return eTreeTraverse;
    }

    float GetTreeHeight() { return (float)m_PixelHeight; }
    float GetLabelHeight() { return (float)m_TotalHeight; }
    double GetYStep() const { return m_yStep; }

private:
    CPhyloRectCladogram* m_Clad;
    int m_ComputeBoundary;
    stack<CBoundaryPoints> m_BoundaryPoints;
    double  m_xStep, m_yStep;
    float m_DistFromRoot;
    float m_LastDist;
    float m_NormDistance;
    double m_yDelta;
    double m_DefaultNodeSize;
    double m_PixelHeight;
    double m_TotalHeight;
    double m_FontHeight;
};

void CPhyloRectCladogram::x_Calculate(CPhyloTree* tree)
{
    CCalcRectangularTree  calc_tree(this, m_DS);
    TreeDepthFirstEx(*tree, calc_tree);
    m_TreePixels = calc_tree.GetTreeHeight();
    m_LabelPixels = calc_tree.GetLabelHeight();
    m_yStep = calc_tree.GetYStep();
}

struct CompareBrightness {
    CompareBrightness(CPhyloTree* tree) : m_Tree(tree) {}

    bool operator()(const CPhyloTree::TTreeIdx& lhs, const CPhyloTree::TTreeIdx& rhs) {
        // 4-way comparison
        switch ((*m_Tree)[lhs]->GetSelectedState()) {
        case CPhyloNodeData::eNotSelected:
        case CPhyloNodeData::eTraced:
            {{
                return ((*m_Tree)[rhs]->GetSelectedState()!=CPhyloNodeData::eNotSelected);
            }}
        case CPhyloNodeData::eShared:
            {{
                return ((*m_Tree)[rhs]->GetSelectedState()==CPhyloNodeData::eSelected);
            }}
        case CPhyloNodeData::eSelected:
            {{
                return false;
            }}
        default:
            {{
                return false;
            }}
        }
   }

protected:
    CPhyloTree* m_Tree;
};

class CDrawSplineTreeVbo
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CDrawSplineTreeVbo(CPhyloRectCladogram* clad,
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
        if (delta==1 || delta==0) {
            CPhyloTree::TNodeType& node = tree[node_idx];

            // Other than root, render node info when looping over children, since
            // we use some selection info (computed there) of that node for edges too
            if (node.GetParent() == CPhyloTreeNode::Null())
                m_Clad->x_RenderNodeVbo(&node, node_idx, m_ColorCoords);

            if (!node.Expanded())
                return eTreeTraverse;

            vector<CPhyloTree::TTreeIdx> sorted_nodes = node.GetChildren();
            CompareBrightness  cb(&tree);

            std::sort(sorted_nodes.begin(), sorted_nodes.end(), cb);

            for (CPhyloTreeNode::TNodeList_I it = sorted_nodes.begin();
                it!=sorted_nodes.end(); it++) {

                    CPhyloTreeNode& sub_node = tree.GetNode(*it);

                    CVect3<float> pt1((*node).X(), (*node).Y(), 0);
                    CVect3<float> pt2((*node).X(), (*sub_node).Y(), 0);
                    CVect3<float> pt3((*sub_node).X(), (*sub_node).Y(), 0);

                    m_Clad->x_RenderNodeVbo(&sub_node, *it, m_ColorCoords);
                   
                    m_Clad->x_RenderSplineVbo(*it, &sub_node, &node, 
                                              m_EdgeNodeCoords, m_EdgeNodeColors,
                                              m_SelEdgeNodeCoords, m_SelEdgeNodeColors,
                                              pt1, pt2, pt2, pt3);
            }

        }
        return eTreeTraverse;
    }

private:
    CPhyloRectCladogram* m_Clad;
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

void CPhyloRectCladogram::x_DrawSplineTree(
        vector<float>& color_coords,
        vector<CVect2<float> >& edge_node_coords,
        vector<CVect4<unsigned char> >& edge_node_colors,
        vector<CVect2<float> >& sel_edge_node_coords,
        vector<CVect4<unsigned char> >& sel_edge_node_colors)
{
    CDrawSplineTreeVbo  draw_tree(this, m_SL.GetNCPointer(),
        edge_node_coords, edge_node_colors,
        sel_edge_node_coords, sel_edge_node_colors,
        color_coords);
    TreeBreadthFirstEx(*m_DS->GetTree(), draw_tree);
}

class CDrawPseudoSplineTreeVbo
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CDrawPseudoSplineTreeVbo(CPhyloRectCladogram* clad,
        CPhyloTree& tree,
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
        if (delta==1 || delta==0) {
            CPhyloTree::TNodeType& node = tree[node_idx];

            if (node.GetParent() == CPhyloTreeNode::Null())
                m_Clad->x_RenderNodeVbo(&node, node_idx, m_ColorCoords);

            if (!node.Expanded())
                return eTreeTraverse;

            vector<CPhyloTree::TTreeIdx> sorted_nodes = node.GetChildren();
            CompareBrightness  cb(&tree);

            std::sort(sorted_nodes.begin(), sorted_nodes.end(), cb);

            for (CPhyloTreeNode::TNodeList_I it = sorted_nodes.begin();
                it != sorted_nodes.end(); ++it) {
                CPhyloTreeNode& sub_node = tree.GetNode(*it);

                CVect2<float> mid_point((*node).X(), (*sub_node).Y());

                m_Clad->x_RenderNodeVbo(&sub_node, *it, m_ColorCoords);

                m_Clad->x_RenderPseudoSplineVbo(*it, &sub_node, &node,                     
                    m_EdgeNodeCoords, m_EdgeNodeColors,
                    m_SelEdgeNodeCoords, m_SelEdgeNodeColors,
                    (node)->XY(), mid_point, (*sub_node).XY());
            }            
        }
        return eTreeTraverse;
    }

private:
    CPhyloRectCladogram* m_Clad;
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

void CPhyloRectCladogram::x_DrawPseudoSplineTree(
    vector<float>& color_coords, 
    vector<CVect2<float> >& edge_node_coords,
    vector<CVect4<unsigned char> >& edge_node_colors,
    vector<CVect2<float> >& sel_edge_node_coords,
    vector<CVect4<unsigned char> >& sel_edge_node_colors)
{
    CDrawPseudoSplineTreeVbo  draw_tree(this, *m_DS->GetTree(),
        edge_node_coords, edge_node_colors,
        sel_edge_node_coords, sel_edge_node_colors,
        color_coords);
  
    TreeDepthFirstEx(*m_DS->GetTree(), draw_tree);
}

string CPhyloRectCladogram::GetDescription(void)
{
    return "Rectangular Cladogram";
}

END_NCBI_SCOPE
