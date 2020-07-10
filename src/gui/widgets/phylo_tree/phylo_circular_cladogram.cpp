/*  $Id: phylo_circular_cladogram.cpp 42903 2019-04-25 15:57:26Z katargir $
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
#include <gui/widgets/phylo_tree/phylo_circular_cladogram.hpp>

#include <gui/widgets/gl/attrib_menu.hpp>
#include <corelib/ncbi_limits.h>

BEGIN_NCBI_SCOPE

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////


CPhyloCircularCladogram::CPhyloCircularCladogram()
: m_UseMinimalRotation(true)
{
    SetRotatedLabels(true);
}

CPhyloCircularCladogram::CPhyloCircularCladogram(double w, double h)
: IPhyloTreeRender(w, h)
, m_UseMinimalRotation(true)
{
    SetRotatedLabels(true);
}


CPhyloCircularCladogram::~CPhyloCircularCladogram()
{
}


void  CPhyloCircularCladogram::x_Layout(CPhyloTreeDataSource& ds)
{
#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenuInstance::GetInstance().SynchToUserValue();
#endif

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

    if (float(m_DS->GetNormDistance()) > kMin_Float)
        m_NormDistance = m_DimX /  ds.GetNormDistance();
    else
        m_NormDistance = 1.0;

    ComputePixelHeight ph = TreeDepthFirst(*ds.GetTree(), ComputePixelHeight(this));
    m_TreePixels = ph.m_PixelHeight;
    m_LabelPixels = ph.m_TotalHeight;
    
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

void CPhyloCircularCladogram::x_SetGlRenderOptions(CPhyloTreeDataSource& ds)
{
    // Set drawing options that are specific to slanted cladogram
    CGlVboNode* edge_node = ds.GetModel().FindGeomNode("TreeEdges");
    CGlVboNode* narrow_edge_node = ds.GetModel().FindGeomNode("NarrowTreeEdges");
    
    CGlVboNode* sel_edge_node = ds.GetModel().FindGeomNode("SelectedTreeEdges");
    CGlVboNode* sel_narrow_edge_node = ds.GetModel().FindGeomNode("SelectedNarrowTreeEdges");

    CGlVboNode* filler_points_node = m_DS->GetModel().FindGeomNode("FillerPoints");

    if (edge_node != NULL && narrow_edge_node != NULL &&
        sel_edge_node != NULL && sel_narrow_edge_node != NULL) {
        edge_node->GetState().Enable(GL_LINE_SMOOTH);
        edge_node->GetState().Enable(GL_BLEND);
        edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   

        sel_edge_node->GetState().Enable(GL_LINE_SMOOTH);
        sel_edge_node->GetState().Enable(GL_BLEND);
        sel_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        narrow_edge_node->GetState().Enable(GL_LINE_SMOOTH);
        narrow_edge_node->GetState().Enable(GL_BLEND);
        narrow_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        sel_narrow_edge_node->GetState().Enable(GL_LINE_SMOOTH);
        sel_narrow_edge_node->GetState().Enable(GL_BLEND);
        sel_narrow_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (filler_points_node != NULL)
        filler_points_node->SetVisible(false);
}

void  CPhyloCircularCladogram::x_RenderVbo(CPhyloTreeDataSource& ds)
{
    // Enable circular label processing...
    if (m_UseMinimalRotation || !GetRotatedLabels()) {
        m_DS->GetModel().EnableCircularLabelTrimming(m_Center);
    }

    CGlVboNode* edge_node = m_DS->GetModel().FindGeomNode("TreeEdges");
    CGlVboNode* narrow_edge_node = m_DS->GetModel().FindGeomNode("NarrowTreeEdges");
    CGlVboNode* sel_edge_node = ds.GetModel().FindGeomNode("SelectedTreeEdges");
    CGlVboNode* point_node = m_DS->GetModel().FindGeomNode("NodePoints");
    _ASSERT(edge_node != NULL && 
            narrow_edge_node != NULL &&
            sel_edge_node != NULL &&
            point_node != NULL);

    GLsizei edge_count = (GLsizei)ds.GetNumEdges();
    GLsizei node_count = (GLsizei)ds.GetNumNodes();    

    int estimatedSize = edge_count * 155;

    if (edge_count > 0) {
        vector<float> color_coords;
        color_coords.reserve(node_count);

        vector<CVect2<float> > edge_node_coords, sel_edge_node_coords;
        edge_node_coords.reserve(estimatedSize);
        sel_edge_node_coords.reserve(estimatedSize);

        vector<CVect4<unsigned char> > edge_node_colors, sel_edge_node_colors;
        edge_node_colors.reserve(estimatedSize);
        sel_edge_node_colors.reserve(estimatedSize);

        x_DrawTree(ds.GetTree(), color_coords, edge_node_coords, edge_node_colors, sel_edge_node_coords, sel_edge_node_colors);
        point_node->SetTexCoordBuffer1D(color_coords);

        point_node->ClearPositions();
        point_node->SetVisible(true);

        edge_node->SetVertexBuffer2D(edge_node_coords);
        edge_node->SetColorBufferUC(edge_node_colors);

        sel_edge_node->SetVertexBuffer2D(sel_edge_node_coords);
        sel_edge_node->SetColorBufferUC(sel_edge_node_colors);
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

// Compute node distance for a fixed pixel offset using unproject
double CPhyloCircularCladogram::ComputeDistFromPixels(CGlPane& pane,
                                                      const CVect2<TModelUnit>& pixpos1,
                                                      const CVect2<TModelUnit>& pixpos2) const {
    CVect2<TModelUnit> pt1 = pane.UnProjectEx(pixpos1);
    CVect2<TModelUnit> pt2 = pane.UnProjectEx(pixpos2);

    // m_DimX is a factor used during layout to force
    // the graph to project to a virtual window of width m_DimX (labels not
    // included) so we divide it out here.  GetNormDistance is the
    // underlying graph width that gets scaled (at layout) to m_DimX.
    return (pt2.X() - pt1.X())*(4.0*m_DS->GetNormDistance() / m_DimX);
}

class CCalcCircularTree
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CCalcCircularTree(CPhyloCircularCladogram* clad,
                      CPhyloTreeDataSource* ds,
                      const CVect2<float>& center)
        : m_Clad(clad)
        , m_DS(ds)
        , m_ComputeBoundary(0)
        , m_Center(center)
        , m_LeafPixelsVisited(0.0f)
        , m_LevelsFromRoot(0)
        , m_DistFromRoot(0.0f)
        , m_LastDist(0.0f)
    {
        m_BoundaryPoints.push(CBoundaryPoints());
        m_Clad->InitExtents();

        m_MaxRadius = (float)(m_Clad->GetDimX() * 0.25);

        if (float(m_DS->GetNormDistance()) > kMin_Float)
            m_NormDist = 1.0f / (float)m_DS->GetNormDistance();
        else
            m_NormDist = 1.0f;

        m_TotalLeaves = (ds->GetTree()->GetRoot()->GetNumLeavesEx() > 0) ? ds->GetTree()->GetRoot()->GetNumLeavesEx() : 1.0f;

        // Leave a small gap so top and bottom do not blend together. 
        m_TotalRadius = float(M_PI*2.0 - M_PI / 32.0);

        /// distance between adjacent nodes - this is both the y step and the xstep
        /// at their maximum values (angle=0/180 and angle=90/270) (of course this is
        /// radial distance along circle, not linear distance)
        m_NodeDelta = (m_TotalRadius*m_MaxRadius) / m_TotalLeaves;
    }

    ETreeTraverseCode operator()(CPhyloTree& tree,
        TTreeIdx node_idx, int delta)
    {
        CPhyloTree::TNodeType& node = tree[node_idx];

        if (delta == 1)  {
            m_BoundaryPoints.push(CBoundaryPoints());
            m_LevelsFromRoot += 1;
            m_DistFromRoot += (*node).GetDistance();
        }
        else if (delta == 0 && node.HasParent()) {
            m_DistFromRoot = m_DistFromRoot - m_LastDist +
                node.GetValue().GetDistance();
        }

        if (delta == 1 || delta == 0){

            if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded)
                m_ComputeBoundary += 1;

            // radius is current depth in tree/total depth * max-dimension           
            if (node.IsLeafEx())  {
                float node_pixels = (float)m_Clad->GetNodeLayoutSize(&node);

                // add 1/2 of the pixels before the node and 1/2 after
                m_LeafPixelsVisited += node_pixels;
                float avg_angle = m_TotalRadius*(m_LeafPixelsVisited / m_Clad->GetTreeHeight());              
                node->SetAngle(avg_angle);                
                m_LeafPixelsVisited += node_pixels;

                // Push leaf nodes to the perimeter Except for collapsed nodes
                float radius = m_MaxRadius;

                (*node).X() = m_Center.X() + (radius * cosf(avg_angle));
                (*node).Y() = m_Center.Y() + (radius * sinf(avg_angle));

                m_Clad->InitLabel(&tree, node);
                m_Clad->CalculateExtents(&tree, node);

                CBoundaryPoints node_boundary_pts;
                m_Clad->x_ComputeNodeBoundary(&tree, node, node_boundary_pts, "CircularCladogram");
                if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded)
                    m_ComputeBoundary -= 1;

                if (m_ComputeBoundary > 0) {
                    m_BoundaryPoints.top().AddBoundedPoints(node_boundary_pts);
                }
            }
        }
        else if (delta == -1) {
            // Compute the position of the node based on the average angle around
            // the circle of its immediate children
            m_LevelsFromRoot -= 1;
            m_DistFromRoot -= m_LastDist;

            float angle = 0.0f;
            if (!node.HasParent()) { // root
                (*node).XY() = m_Center;
            }
            else {
                float num_children = 0.0f;
                for (CPhyloTreeNode::TNodeList_I  it = node.SubNodeBeginEx();
                    it != node.SubNodeEndEx(); it++)  {
                    angle += tree[*it]->GetAngle();
                    ++num_children;
                }

                angle /= num_children;
            }
            node->SetAngle(angle);

            float radius;
            if (m_Clad->GetDistRendering())
                radius = m_NormDist * m_DistFromRoot * m_MaxRadius;
            else
                radius = ((float)m_LevelsFromRoot) / ((float)m_DS->GetWidth()) * m_MaxRadius;

            (*node).X() = m_Center.X() + (radius * cosf(angle));
            (*node).Y() = m_Center.Y() + (radius * sinf(angle));
            if (!node.IsLeafEx())
                m_Clad->InitLabel(&tree, node); // this needs the angle (node->SetAngle())
            m_Clad->CalculateExtents(&tree, node);

            // Each node that computes a boundary has to add in the points (and text rectangles)
            // for all nodes below it in the tree
            CBoundaryPoints pts = m_BoundaryPoints.top();
            m_BoundaryPoints.pop();
            m_Clad->x_ComputeNodeBoundary(&tree, node, pts, "CircularCladogram");

            if (node.GetValue().GetBoundedDisplay() == CPhyloNodeData::eBounded) {
                m_ComputeBoundary -= 1;
            }

            if (m_ComputeBoundary > 0) {
                m_BoundaryPoints.top().AddBoundedPoints(pts);
            }
        }

        m_LastDist = node.GetValue().GetDistance();
        return eTreeTraverse;
    }

    double GetYStep() const { return m_NodeDelta; }

private:
    CPhyloCircularCladogram* m_Clad;
    CPhyloTreeDataSource* m_DS;
    int m_ComputeBoundary;
    stack<CBoundaryPoints> m_BoundaryPoints;
    double m_NodeDelta;
    float m_MaxRadius;
    CVect2<float> m_Center;
    float m_LeafPixelsVisited;
    float m_TotalLeaves;
    int m_LevelsFromRoot;
    float m_DistFromRoot;
    float m_NormDist;
    float m_LastDist;
    float m_TotalRadius;
};

void CPhyloCircularCladogram::x_Calculate(CPhyloTree* tree)
{
    m_Center.Set(((float)(GetDimX() / 2.0)), ((float)(GetDimY() / 2.0)));
    CCalcCircularTree  calc_tree(this, m_DS, m_Center);
    TreeDepthFirstEx(*tree, calc_tree);

    m_yStep = calc_tree.GetYStep();
    m_xStep = m_yStep;
}



struct CompareBrightness {
    CompareBrightness(CPhyloTree* tree) : m_Tree(tree) {}

    bool operator()(const CPhyloTree::TTreeIdx& lhs, const CPhyloTree::TTreeIdx& rhs) {
        // 4-way comparison
        switch ((*m_Tree)[lhs]->GetSelectedState()) {
        case CPhyloNodeData::eNotSelected:
            {{
                return ((*m_Tree)[rhs]->GetSelectedState()!=CPhyloNodeData::eNotSelected);
            }}
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

class CDrawCircularTreeVbo
{
public:
    typedef CPhyloTree::TTreeIdx TTreeIdx;

public:
    CDrawCircularTreeVbo(CPhyloCircularCladogram* clad,
                         CPhyloTreeDataSource* ds,
                         vector<CVect2<float> >* trace_line_coords,
                         vector<CVect4<unsigned char> >* trace_line_colors,
                         vector<CVect2<float> >* trace_point_coords,
                         vector<CVect4<unsigned char> >* trace_point_colors,
                        vector<CVect2<float> >& edge_node_coords,
                        vector<CVect4<unsigned char> >& edge_node_colors,
                        vector<CVect2<float> >& sel_edge_node_coords,
                        vector<CVect4<unsigned char> >& sel_edge_node_colors,
                        vector<float>& color_coords)
    : m_Clad(clad)
    , m_SL(clad->GetScheme())
    , m_DS(ds)
    , m_EdgeNodeCoords(edge_node_coords)
    , m_EdgeNodeColors(edge_node_colors)
    , m_SelEdgeNodeCoords(sel_edge_node_coords)
    , m_SelEdgeNodeColors(sel_edge_node_colors)
    , m_ColorCoords(color_coords)
    , m_TraceLineCoords(trace_line_coords)
    , m_TraceLineColors(trace_line_colors)
    , m_TracePointCoords(trace_point_coords)
    , m_TracePointColors(trace_point_colors)
    , m_NodeSingleSelection(clad->GetSingleSelection())
    {   
        // When drawing arcs, the resolution determines the number
        // of segments.  For larger trees (more leaves) you can zoom
        // in more so there are more segments. Overall, the circular 
        // cladogram draws the leaves around the circle, so the number
        // of leaves is a good proxy for resolution.
        if (m_DS->GetSize()/4 < 360) 
            m_Resolution = 360.0f;
        else 
            m_Resolution = (float)m_DS->GetSize()/4;

        m_Center = m_DS->GetTree()->GetRoot()->XY();
        m_MaxRadius = (float)(m_Clad->GetDimX() * 0.25);

        if (float(m_DS->GetNormDistance()) > kMin_Float)
            m_NormDist = 1.0f / (float)m_DS->GetNormDistance();
        else
            m_NormDist = 1.0f;
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

            float radius = 0.0f;
            for (CPhyloTreeNode::TNodeList_I it = sorted_nodes.begin();
                it!=sorted_nodes.end(); it++) {

                    CPhyloTreeNode& sub_node = tree.GetNode(*it);

                    // Only get radius once
                    if (radius==0.0f)
                        radius = (node->XY() - m_Center).Length();
                    float angle = sub_node->GetAngle();                        

                    CVect2<float> delta(0.0f, 0.0f);
                    if (sub_node.IsLeafEx() && m_Clad->GetDistRendering()) {
                        float r = (*sub_node).GetDistance()*m_NormDist*m_MaxRadius + ((*node).XY() - m_Center).Length();

                        CVect2<float> pt(m_Center.X() + (r * cosf(angle)),
                                         m_Center.Y() + (r * sinf(angle)));
                        delta = ((*sub_node).XY() - pt);
                    }

                    m_Clad->x_RenderNodeVbo(&sub_node, *it, m_ColorCoords, delta);

                    m_Clad->x_RenderCircularVbo(*it, &sub_node, &node,
                                                m_TraceLineCoords, m_TraceLineColors,
                                                m_TracePointCoords, m_TracePointColors,
                                                m_EdgeNodeCoords, m_EdgeNodeColors,
                                                m_SelEdgeNodeCoords, m_SelEdgeNodeColors,
                                                radius, m_Center, delta, m_Resolution);
            }
        }
        return eTreeTraverse;
    }

private:
    CPhyloCircularCladogram* m_Clad;
    const CPhyloTreeScheme& m_SL;
    CPhyloTreeDataSource* m_DS;
    vector<CVect2<float> >& m_EdgeNodeCoords;
    vector<CVect4<unsigned char> >& m_EdgeNodeColors;
    vector<CVect2<float> >& m_SelEdgeNodeCoords;
    vector<CVect4<unsigned char> >& m_SelEdgeNodeColors;
    vector<float>& m_ColorCoords;
    float m_Resolution;

    float m_MaxRadius;
    float m_NormDist;
    CVect2<float> m_Center;
    vector<CVect2<float> >* m_TraceLineCoords;
    vector<CVect4<unsigned char> >* m_TraceLineColors;
    vector<CVect2<float> >* m_TracePointCoords;
    vector<CVect4<unsigned char> >* m_TracePointColors;

    // When user only wants to display a single, current selection from
    // the selection set (including traced and commmon nodes).
    CPhyloTree::TSelState& m_NodeSingleSelection;
};


void CPhyloCircularCladogram::x_DrawTree(CPhyloTree* tree, 
        vector<float>& color_coords,
        vector<CVect2<float> >& edge_node_coords,
        vector<CVect4<unsigned char> >& edge_node_colors,
        vector<CVect2<float> >& sel_edge_node_coords,
        vector<CVect4<unsigned char> >& sel_edge_node_colors)
{
    CGlVboNode* trace_lines = m_DS->GetModel().FindGeomNode("CircularTraces");
    CGlVboNode* trace_points = m_DS->GetModel().FindGeomNode("CircularTracePoints");
    int num_leaves = m_DS->GetTree()->GetRoot()->GetNumLeavesEx();

    // With distance rendering we still place nodes at perimeter. To do this we add
    // extra thin lines from end of distance-based edge to node on perimiter (trace_lines)
    // We also add individual points (trace_points) to the points where the main (thicker)
    // edge and thin edge to perimeter intersect. These points 'cap off' the thicker
    // edge (except in PDF mode where edges can have a cap-style).
    if (GetDistRendering()) {
        CMatrix4<float> m;
        m.Identity();

        if (trace_lines == NULL) {
            trace_lines = m_DS->GetModel().AddGeomNode(GL_LINES, "CircularTraces", 0, true);            

            trace_lines->GetState().Enable(GL_BLEND);
            trace_lines->GetState().Enable(GL_LINE_SMOOTH);
            trace_lines->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            trace_lines->GetState().Disable(GL_TEXTURE_2D);
            trace_lines->GetState().Disable(GL_TEXTURE_1D);
            trace_lines->GetState().LineWidth(1.0f);
        }
        trace_lines->SetPosition(m);
        
        if (trace_points == NULL) {
            trace_points = m_DS->GetModel().AddGeomNode(GL_POINTS, "CircularTracePoints", true);
            trace_points->GetState().Disable(GL_BLEND);
            trace_points->GetState().Disable(GL_TEXTURE_2D);
            trace_points->GetState().Disable(GL_TEXTURE_1D);
            // Don't draw points in pdf mode (pdf fills line elbows differently)  
            trace_points->SkipTarget(eRenderPDF, true);
        }
        trace_points->SetPosition(m);
 
        vector<CVect2<float> > trace_line_coords, trace_point_coords;
        trace_line_coords.reserve(num_leaves * 2);
        trace_point_coords.reserve(num_leaves);

        vector<CVect4<unsigned char> > trace_line_colors, trace_point_colors;
        trace_line_colors.reserve(num_leaves * 2);
        trace_point_colors.reserve(num_leaves);

        CDrawCircularTreeVbo  draw_tree(this, m_DS,
            &trace_line_coords, &trace_line_colors,
            &trace_point_coords, &trace_point_colors, 
            edge_node_coords,
            edge_node_colors,
            sel_edge_node_coords,
            sel_edge_node_colors,
            color_coords);

        TreeDepthFirstEx(*tree, draw_tree);

        trace_lines->SetVertexBuffer2D(trace_line_coords);
        trace_lines->SetColorBufferUC(trace_line_colors);

        trace_points->SetVertexBuffer2D(trace_point_coords);
        trace_points->SetColorBufferUC(trace_point_colors);

        trace_lines->SetVisible(true);
        trace_points->SetVisible(true);
    }
    else {
        CDrawCircularTreeVbo  draw_tree(this, m_DS,
            nullptr, nullptr, 
            nullptr, nullptr, 
            edge_node_coords,
            edge_node_colors,
            sel_edge_node_coords,
            sel_edge_node_colors,
            color_coords);
        TreeDepthFirstEx(*tree, draw_tree);
    }
}


string CPhyloCircularCladogram::GetDescription(void)
{
    return "Circular Cladogram";
}

float CPhyloCircularCladogram::GetNodeLabelDist(const CPhyloTreeNode * node) const
{
    // Different if collapsed nodes are inside distance tree and therefore
    // not "pushing" text out.
    float d = 0;
    if (!node->Expanded()) {       
        if (GetDistRendering()) {
            d = ((node->GetValue().GetNodeWidthScaler(m_SL.GetPointer())) / 2.0f);
        }
        else {
            d = (node->GetValue().GetNodeWidthScaler(m_SL.GetPointer())) - 2.0f;
        }
    }
    else {
        d = GetDefaultNodeSize(node);
    }

    return d;
}

bool CPhyloCircularCladogram::x_WideLines()
{
    TModelUnit line_width =
        m_SL->SetSize(CPhyloTreeScheme::eLineWidth);

    // Require lines to be twice as far apart as normal. This means that lines 
    // until they are closer to the center of the circle
    if (DistanceBetweenNodes() > line_width*4.0) {
        return true;
    }
    return false;
}

END_NCBI_SCOPE
