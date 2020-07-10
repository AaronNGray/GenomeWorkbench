/*  $Id: tree_graphics_model.cpp 43454 2019-07-02 17:24:35Z katargir $
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

#include <gui/widgets/phylo_tree/tree_graphics_model.hpp>
#include <gui/opengl/glframebuffer.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_node.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_boundary_shapes.hpp>
#include <gui/widgets/phylo_tree/phylo_tree.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_label.hpp>


#include <gui/opengl/glresmgr.hpp>

#include <gui/print/pdf.hpp>
#include <gui/print/print_media.hpp>
#include <gui/print/pdf_object.hpp>

#include <gui/widgets/gl/attrib_menu.hpp>

#include <gui/utils/matrix3.hpp>
#include <gui/utils/matrix4.hpp>

#include <corelib/ncbitime.hpp>

#include <cmath>

BEGIN_NCBI_SCOPE


void CTreeBoundaryNode::x_Render()
{
    if (!IsVisible())
        return;

    CVect2<float> scale((float)m_State->GetScaleFactor().X(),
                        (float)m_State->GetScaleFactor().Y());

    for (size_t i=0; i<m_BoundaryNodes.size(); ++i) {
        CPhyloTreeNode& node = (*m_Tree)[m_BoundaryNodes[i]];      

        if ((*node).GetDisplayChildren() == CPhyloNodeData::eHideChildren)
            continue;
      
        float alpha_mod = 1.0f;

        if ((*node).GetSelectedState()!=CPhyloNodeData::eSelected) {
            if (m_SL->GetSelectionVisibility() == 
                CPhyloTreeScheme::eHighlightSelection) {
                    alpha_mod = m_SL->GetNonSelectedAlpha();
            }
	    } 
       
        (*node).GetSubtreeBoundary()->RenderBoundaryVbo(scale, alpha_mod, m_IncludeLabels);

        if ((*node).GetSubtreeBoundary()->Overlaps(m_Pane.GetVisibleRect())) {
            (*node).GetSubtreeBoundary()->RenderVbo();
        }
    }
}

void CTreeBoundaryNode::RenderPdf(CRef<CPdf>& pdf)
{
    if (!IsVisible())
        return;

    for (size_t i=0; i<m_BoundaryNodes.size(); ++i) {
        CPhyloTreeNode& node = (*m_Tree)[m_BoundaryNodes[i]];      

        if ((*node).GetDisplayChildren() == CPhyloNodeData::eHideChildren)
            continue;
      
        //glTranslated((**n).X(), (**n).Y(), 0.0);
        float alpha_mod = 1.0f;

        if ((*node).GetSelectedState() != CPhyloNodeData::eSelected) {
            if (m_SL->GetSelectionVisibility() == 
                CPhyloTreeScheme::eHighlightSelection) {
                    alpha_mod = m_SL->GetNonSelectedAlpha();
            }
	    } 

        CVect2<float> scale2d((float)m_State->GetScaleFactor().X(),
                              (float)m_State->GetScaleFactor().Y());
      
        (*node).GetSubtreeBoundary()->RenderBoundaryVbo(scale2d, alpha_mod);

        pdf->PrintTriBuffer((*node).GetSubtreeBoundary()->GetBoundaryTris(), m_Pane.GetViewport());
       
        CRef<CGlVboNode> boundaryEdges = node->GetSubtreeBoundary()->GetBoundaryEdges();
        if (boundaryEdges && boundaryEdges->GetVertexFormat() == IVboGeom::kVertexFormatVertex2D)
            pdf->PrintLineBuffer((*node).GetSubtreeBoundary()->GetBoundaryEdges(), m_Pane.GetViewport());
    }
}

void CTreeTriFanNode::CreateVbo(size_t numSements)
{
    m_VBOGeom.Reset(CGlResMgr::Instance().CreateTreeNodeVboGeom(numSements));
}

CTreeGraphicsModel::CTreeGraphicsModel()
: m_BoundaryNode(NULL)
, m_TriFanNode(NULL)
, m_Tree(NULL)
, m_RotateLabels(false)
, m_RenderPdfTooltips(false)
, m_RenderTime(0.0f)
, m_LineRenderTime(0.0f)
, m_NodesRenderTime(0.0f)
, m_BoundaryRenderTime(0.0f)
, m_NodeRenderCount(0)
{
#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenu& m = CAttribMenuInstance::GetInstance();
    CAttribMenu* sub_menu = m.AddSubMenuUnique("Model", this);

    sub_menu->AddFloatReadOnly("Render Time", &m_RenderTime);
    sub_menu->AddFloatReadOnly("Line Render Time", &m_LineRenderTime);
    sub_menu->AddFloatReadOnly("Nodes Render Time", &m_NodesRenderTime);
    sub_menu->AddFloatReadOnly("Boundary Render Time", &m_BoundaryRenderTime);
    sub_menu->AddIntReadOnly("Nodes Render Count", &m_NodeRenderCount);
    sub_menu->AddFloatReadOnly("Total Time", &m_TotalTime);
    sub_menu->AddFloatReadOnly("Collision Update Time", &m_CDUpdateTime);
#endif
}

CTreeGraphicsModel::~CTreeGraphicsModel()
{
    delete m_BoundaryNode;
    m_BoundaryNode = NULL;

    delete m_TriFanNode;
    m_TriFanNode = NULL;

#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenuInstance::GetInstance().RemoveMenuR("Model", this);
#endif
}


void CTreeGraphicsModel::Init(CPhyloTree* t)
{
    m_Tree = t;
    m_ColorTable.Reset(t->GetColorTable());

    // Order that the CGlVboNode's are created is significant, as it 
    // determines their drawing order (and thus what is drawn on top of what)

    CGlVboNode* filler_points_node = FindGeomNode("FillerPoints");
    if (!filler_points_node) {
        filler_points_node = AddGeomNode("FillerPoints", true);    
        filler_points_node->GetState().Enable(GL_BLEND);
        filler_points_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        filler_points_node->GetState().Disable(GL_TEXTURE_2D);
        filler_points_node->GetState().Disable(GL_TEXTURE_1D);
        filler_points_node->GetState().ColorC(CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f));

        // Don't draw points in pdf mode (pdf fills line elbows differently)
        filler_points_node->SkipTarget(eRenderPDF, true);

        // off by default. only used in rectangular pseudo-spline layout
        filler_points_node->SetVisible(false);
    }

    CGlVboNode* sel_filler_points_node = FindGeomNode("SelectedFillerPoints");
    if (!sel_filler_points_node) {
        sel_filler_points_node = AddGeomNode("SelectedFillerPoints", true);
        sel_filler_points_node->GetState().Enable(GL_BLEND);
        sel_filler_points_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        sel_filler_points_node->GetState().Disable(GL_TEXTURE_2D);
        sel_filler_points_node->GetState().Disable(GL_TEXTURE_1D);
        sel_filler_points_node->GetState().ColorC(CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f));

        // Don't draw points in pdf mode (pdf fills line elbows differently)
        sel_filler_points_node->SkipTarget(eRenderPDF, true);

        // off by default. only used in rectangular pseudo-spline layout
        sel_filler_points_node->SetVisible(false);
    }

    CGlVboNode* edge_node = FindGeomNode("TreeEdges");
    if (!edge_node) {
        edge_node = AddGeomNode(GL_LINES, "TreeEdges", true);
        edge_node->GetState().Disable(GL_TEXTURE_2D);
        edge_node->GetState().Disable(GL_TEXTURE_1D);
        edge_node->GetState().LineJoinStyle(CGlState::eRoundedJoin);
        edge_node->GetState().LineCapStyle(CGlState::eRoundCap);
        edge_node->GetState().ColorC(CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f));
    }

    CGlVboNode* boundary_node = FindGeomNode("BoundaryNodes");
    if (!boundary_node) {
        boundary_node = AddGeomNode(GL_TRIANGLES, "BoundaryNodes", true);
        boundary_node->GetState().Disable(GL_BLEND);
        boundary_node->GetState().Disable(GL_TEXTURE_2D);
        boundary_node->GetState().Disable(GL_TEXTURE_1D);
    }

    // These edges are inside wider edges (same geometry buffer as 'TreeEdges')
    CGlVboNode* narrow_edge_node = FindGeomNode("NarrowTreeEdges");
    if (!narrow_edge_node) {        
        narrow_edge_node = AddGeomNode("NarrowTreeEdges", true);
        narrow_edge_node->SetVBOGeom(CGlResMgr::Instance().CreateNarrowTreeEdgeVboGeom(edge_node->GetVBOGeom()));
        
        narrow_edge_node->GetState().LineWidth(1.0f);
        // color to be used for lighting in blending equation
        narrow_edge_node->GetState().ColorC(CRgbaColor(1.0f, 1.0f, 1.0f, 0.7f));
        narrow_edge_node->GetState().Disable(GL_TEXTURE_2D);
        narrow_edge_node->GetState().Disable(GL_TEXTURE_1D);
        narrow_edge_node->GetState().LineJoinStyle(CGlState::eMiteredJoin);
        narrow_edge_node->GetState().LineCapStyle(CGlState::eButtCap);
    }

    // We draw selected edges last so they are always on top of other edges.
    // This is easiest to do if we put them in their own vbo. This is only
    // needed for distance-based rendering though since edges should never
    // overlap if layout has full control of spacing
    CGlVboNode* sel_edge_node = FindGeomNode("SelectedTreeEdges");
    if (!sel_edge_node) {
        sel_edge_node = AddGeomNode(GL_LINES, "SelectedTreeEdges", true);
        sel_edge_node->GetState().Disable(GL_TEXTURE_2D);
        sel_edge_node->GetState().Disable(GL_TEXTURE_1D);
        sel_edge_node->GetState().LineJoinStyle(CGlState::eRoundedJoin);
        sel_edge_node->GetState().LineCapStyle(CGlState::eRoundCap);
        sel_edge_node->GetState().ColorC(CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // Narrow edges for selected edges (when using distance-based rendering)
    CGlVboNode* sel_narrow_edge_node = FindGeomNode("SelectedNarrowTreeEdges");
    if (!sel_narrow_edge_node) {
        sel_narrow_edge_node = AddGeomNode("SelectedNarrowTreeEdges", true);
        sel_narrow_edge_node->SetVBOGeom(CGlResMgr::Instance().CreateNarrowTreeEdgeVboGeom(sel_edge_node->GetVBOGeom()));

        sel_narrow_edge_node->GetState().LineWidth(1.0f);
        // color to be used for lighting in blending equation
        sel_narrow_edge_node->GetState().ColorC(CRgbaColor(1.0f, 1.0f, 1.0f, 0.7f));
        sel_narrow_edge_node->GetState().Disable(GL_TEXTURE_2D);
        sel_narrow_edge_node->GetState().Disable(GL_TEXTURE_1D);
        sel_narrow_edge_node->GetState().LineJoinStyle(CGlState::eMiteredJoin);
        sel_narrow_edge_node->GetState().LineCapStyle(CGlState::eButtCap);
    }

    // Used by some renderings to fill in joint where two lines with 
    // width > 1 meet (otherwise you get a gap at the corner,as shown
    // below where two lines of width 3 pixels meet but leave the position
    // marked by ? blank.
    //  ?xxxxxxx
    //  xxxxxxxx  <- line 2
    //  xxxxxxxx
    //  xxx
    //  xxx <-line 1
    //  xxx
    // (faster):
    // Draw the edge buffer twice, but second time call it a 
    // point buffer and set the offset to only draw the corner points
    // (preferably without using an index buffer. (Can't draw end points
    // since that would extend the line)
    if (filler_points_node != NULL) {
        filler_points_node->SetVBOGeom(CGlResMgr::Instance().CreateFillerPointVboGeom(edge_node->GetVBOGeom()));
        filler_points_node->GetState().Disable(GL_TEXTURE_1D);
        filler_points_node->GetState().Disable(GL_TEXTURE_2D);
    }

    if (sel_filler_points_node != NULL) {
        sel_filler_points_node->SetVBOGeom(CGlResMgr::Instance().CreateFillerPointVboGeom(sel_edge_node->GetVBOGeom()));
        sel_filler_points_node->GetState().Disable(GL_TEXTURE_1D);
        sel_filler_points_node->GetState().Disable(GL_TEXTURE_2D);
    }

    CGlVboNode* point_node = FindGeomNode("NodePoints");
    if (!point_node) {
        point_node = AddGeomNode(GL_POINTS, "NodePoints", true);
        point_node->GetState().Enable(GL_TEXTURE_1D);
        point_node->GetState().Disable(GL_TEXTURE_2D);
        point_node->GetState().ColorC(CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f));
        point_node->GetState().PointSize(1.0f);
    }

    if (m_BoundaryNode == NULL) {
        m_BoundaryNode = new CTreeBoundaryNode(t);
        if (m_SL.GetPointerOrNull() != NULL)
            m_BoundaryNode->SetScheme(m_SL.GetObject());
        m_BoundaryNode->GetState().Disable(GL_TEXTURE_1D);
        m_BoundaryNode->GetState().Disable(GL_TEXTURE_2D);
    }

    if (m_TriFanNode == NULL) {
        m_TriFanNode = new CTreeTriFanNode();
        m_TriFanNode->GetState().Disable(GL_TEXTURE_2D);
        m_TriFanNode->GetState().Disable(GL_TEXTURE_1D);
        m_TriFanNode->GetState().Disable(GL_BLEND);
        m_TriFanNode->GetState().ShadeModel(GL_SMOOTH);
        m_TriFanNode->GetState().PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

CGlVboNode* CTreeGraphicsModel::AddTempCollapsedNode(const vector<CVect2<float> >& vb)
{
    if (!m_CollapseTexture) {
        CGlAttrGuard guard(GL_ALL_ATTRIB_BITS);

        CIRef<I3DFrameBuffer> tmp_framebuffer(CGlResMgr::Instance().CreateFrameBuffer(512));

        tmp_framebuffer->SetTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
        tmp_framebuffer->SetClearColor(0, 0, 0, 0);
        tmp_framebuffer->CreateFrameBuffer();

        if (tmp_framebuffer->IsValid()) {
            auto renderer = [&vb]() {
                IRender& gl = GetGl();

                gl.Viewport(0, 0, 512, 512);

                GLdouble saveMVMatrix[16], saveProjMatrix[16];
                gl.GetModelViewMatrix(saveMVMatrix);
                gl.GetProjectionMatrix(saveProjMatrix);

                gl.MatrixMode(GL_MODELVIEW);
                gl.LoadIdentity();

                gl.MatrixMode(GL_PROJECTION);
                gl.LoadIdentity();
                gl.Ortho(0.0, 1.0, -0.5, 0.5, -1.0, 1.0);

                gl.Disable(GL_BLEND);
                gl.Disable(GL_TEXTURE_2D);
                gl.Disable(GL_TEXTURE_1D);
                gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                gl.Color4f(1.0f, 1.0f, 1.0f, 1.0f);

                gl.Begin(GL_TRIANGLE_STRIP);
                for (const auto& v : vb)
                    gl.Vertex3f(v.X(), v.Y(), 0.0f);
                gl.End();

                gl.MatrixMode(GL_PROJECTION);
                gl.LoadMatrixd(saveProjMatrix);

                gl.MatrixMode(GL_MODELVIEW);
                gl.LoadMatrixd(saveMVMatrix);
            };

            tmp_framebuffer->Render(renderer);
            tmp_framebuffer->GenerateMipMaps();

            // Set rendering target back
            m_CollapseTexture.Reset(tmp_framebuffer->Get3DTexture());
            m_CollapseTexture->SetTexEnv(GL_MODULATE);
            m_CollapseTexture->SetFilterMag(GL_LINEAR);
            m_CollapseTexture->SetFilterMin(GL_LINEAR_MIPMAP_LINEAR);
            m_CollapseTexture->SetParams();
            tmp_framebuffer->ReleaseTexture();
        }
    }

    if (!m_CollapseTexture)
        return nullptr;

    CIRef<IVboGeom> geom_node = FindGeomObject("collapsed_tex");
    if (!geom_node) {
        geom_node = AddGeomObject(GL_TRIANGLE_STRIP, "collapsed_tex");

        vector<CVect2<float> > vb;

        vb.push_back(CVect2<float>(0.0f, -0.5f));
        vb.push_back(CVect2<float>(1.0f, -0.5f));
        vb.push_back(CVect2<float>(0.0f, 0.5f));
        vb.push_back(CVect2<float>(1.0f, 0.5f));

        geom_node->SetVertexBuffer2D(vb);

        vector<CVect2<float> > tb;
        tb.push_back(CVect2<float>(0.0f, 0.0f));
        tb.push_back(CVect2<float>(1.0f, 0.0f));
        tb.push_back(CVect2<float>(0.0f, 1.0f));
        tb.push_back(CVect2<float>(1.0f, 1.0f));

        geom_node->SetTexCoordBuffer(tb);
    }

    // Since PDF has anti-aliasing built-in, we skip it for all but the most opaque node
    CGlVboNode* n = AddTempGeomNode("CollapsedNode", true);
    n->SetVBOGeom(geom_node);

    n->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    n->GetState().Disable(GL_LIGHTING);
    n->GetState().Enable(GL_BLEND);
    n->GetState().Enable(GL_TEXTURE_2D);
    n->GetState().Disable(GL_TEXTURE_1D);
    n->GetState().SetTexture(m_CollapseTexture.GetNCPointer());
    n->GetState().ScaleInvarient(true);

    return n;
}

void CTreeGraphicsModel::SyncBuffers()
{
    // Make sure color class is right size (nobody has added virtual functions
    // or other data).
    _ASSERT(sizeof(CRgbaColor) == 16);

    CGlVboNode* point_node = FindGeomNode("NodePoints");
    CGlVboNode* filler_point_node = FindGeomNode("FillerPoints"); 
    CGlVboNode* sel_filler_point_node = FindGeomNode("SelectedFillerPoints");
    CGlVboNode* edge_node = FindGeomNode("TreeEdges");
    CGlVboNode* narrow_edge_node = FindGeomNode("NarrowTreeEdges");
    CGlVboNode* sel_edge_node = FindGeomNode("SelectedTreeEdges");
    CGlVboNode* sel_narrow_edge_node = FindGeomNode("SelectedNarrowTreeEdges");
    
    
    if (m_NodeIndices.size() > 0) {
        point_node->GetState().SetTexture(m_ColorTable->GetTexture());

        point_node->SetVisible(true);
        // Gets cleared in clear arrays.  Seems like a bit of a hack...
        point_node->SetDefaultPosition();

        vector<CVect2<float> > coords;
        coords.reserve(m_NodeIndices.size());

        for (size_t i=0; i<m_NodeIndices.size(); ++i) {
            CPhyloTreeNode& n = (*m_Tree)[m_NodeIndices[i]];
            if (n->GetDisplayChildren() != CPhyloNodeData::eHideChildren)
                coords.push_back(n->XY());
        }

        point_node->SetVertexBuffer2D(coords);
    }
    else if (point_node != NULL) {
        point_node->SetVisible(false);
    }   

    if (edge_node != NULL) {
        size_t vb_size = edge_node->GetVertexCount();  
        
        if (vb_size > 0) {                     
            // The filler points are set to visible only for pseudo-spline (right-angle
            // edges) rendering.  Otherwise visible will be set to false.
            if (filler_point_node->IsVisible())
                filler_point_node->SetDefaultPosition();
  
            edge_node->SetVisible(true);
            // Gets cleared in clear arrays
            edge_node->SetDefaultPosition();

            if (narrow_edge_node != NULL) {
                narrow_edge_node->SetDefaultPosition();
            }
        }
        else {
            edge_node->SetVisible(false);
        }

        // Selected edges are in their own buffer so they can be drawn after (and show up on top of)
        // non-selected edges
        vb_size = sel_edge_node->GetVertexCount();

        if (vb_size > 0) {
            // The filler points are set to visible only for pseudo-spline (right-angle
            // edges) rendering.  Otherwise visible will be set to false.
            if (sel_filler_point_node->IsVisible())
                sel_filler_point_node->SetDefaultPosition();

            sel_edge_node->SetVisible(true);
            // Gets cleared in clear arrays
            sel_edge_node->SetDefaultPosition();

            if (sel_narrow_edge_node != NULL) {
                sel_narrow_edge_node->SetDefaultPosition();
            }
        }
        else {
            sel_edge_node->SetVisible(false);
        }
    }
}

void CTreeGraphicsModel::ClearArrays()
{
    CGlModel2D::ClearArrays();

    m_NodeIndices.clear();

    if (m_BoundaryNode != NULL)
        m_BoundaryNode->Clear();

    if (m_TriFanNode != NULL)
        m_TriFanNode->ClearPositions();

    CGlVboNode* edge_node = FindGeomNode("TreeEdges");
    if (edge_node)
        edge_node->SetVertexBuffer2D(vector<CVect2<float> >());

    CGlVboNode* sel_edge_node = FindGeomNode("SelectedTreeEdges");
    if (sel_edge_node)
        sel_edge_node->SetVertexBuffer2D(vector<CVect2<float> >());

    // Default this to false before re-creating buffers, so that only the circular
    // layout has to call EnableCircularLabelTrimming()
    m_EnableCircularLabelTrimming = false;
}

void CTreeGraphicsModel::EnableCircularLabelTrimming(const CVect2<float>& cladogram_center)
{
    m_EnableCircularLabelTrimming = true;
    m_CladogramCenter = cladogram_center;
    // Magic number pi/32.0 is gap left in circular layout so that top and bottom do not blend together.
    m_DeltaTheta = (float(M_PI*2.0 - M_PI / 32.0)) / float(m_Tree->GetRoot().GetValue().GetNumLeavesEx());
}

void  CTreeGraphicsModel::SetScheme(CPhyloTreeScheme& sl) 
{ 
    m_SL.Reset(&sl); 
    m_CollisionData.SetScheme(sl);

    if (m_BoundaryNode != NULL)
        m_BoundaryNode->SetScheme(sl);
}

void  CTreeGraphicsModel::x_CircularLabelTrim(const CGlPane& pane,
                                              const CVect3<float>& scale,
                                              CPhyloTreeNode &n,
                                              bool truncate_labels)
{
    float a1 = n->GetAngle();
    float a2;

    // Get expected angle of leaf next leaf node on the perimiter that is closer
    // to the centerline (y=0)
    if (a1 <= float(M_PI / 2.0) || a1 > float(3.0*M_PI / 2.0))
        a2 = a1 - m_DeltaTheta;
    else
        a2 = a1 + m_DeltaTheta;

    // Angle saved in nodes assume square viewport. This scales angle to match 
    // current (possibly non-square) viewport
    a1 = n->GetScaledAngle(scale, a1, false);
    a2 = n->GetScaledAngle(scale, a2, false);

    TVPPoint p1 = pane.Project(n->X(), n->Y());
    TVPPoint p2 = pane.Project(m_CladogramCenter.X(), m_CladogramCenter.Y());

    // This does not give correct result ... (maybe warp the angle the way we
    // do in node class?)
    float r = CVect2<float>(p1.X() - p2.X(), p1.Y() - p2.Y()).Length();

    // Normally node labels are set to 'not visible' when they overlap. We
    // disable that here since we compute overlap based on position around the circle
    n->SetVisible(true);
    // compute vertical distance between nodes:
    float vdist = std::abs(r*sin(a1) - r*sin(a2));
    if (vdist < m_SL->GetFont().TextHeight()) {
        // Compute horizontal distance between nodes. Since labels should not overlap the nodes
        // themselves, we also subtract out the node radius * 2 here, and subtract 2 pixels to 
        // guarantee some minimal spacing between labels and adjancent nodes
        float hdist = std::abs(r*cos(a1) - r*cos(a2)) - (2.0f*n->GetNodeDisplaySize() + 2.0f);
        if (hdist > 0.0f)
            n->Render(pane, m_SL->GetFont(), m_ColorTable.GetPointer(), scale, truncate_labels, (TModelUnit)hdist);
    }
    else {
        n->Render(pane, m_SL->GetFont(), m_ColorTable.GetPointer(), scale, truncate_labels);
    }
}

float CTreeGraphicsModel::x_CircularLabelMinAngle(const CGlPane& pane,
                                                  const CVect3<float>& scale,
                                                  const CPhyloTreeNode &n,
                                                  bool& visible) const
{

    // Angle of node around circle (0..2*pi)
    float a1 = n->GetAngle();
    float a2;

    visible = true;

    // Wherever a1 is around circle (0..2pi) we re-map it to the equivalent
    // angle in the first quadrant (0..pi/2) to make our calculations consistent.
    // Since our goal is to find the rotation amount for the label, for some
    // quadrants we will reverse that rotation (via reverse_rotation).
    float reverse_rotation = 1.0f;
    if (a1 > float(M_PI / 2.0)) {
        if (a1 < float(M_PI)) {
            a1 = float(M_PI) - a1;
            reverse_rotation = -1.0f;
        }
        else if (a1 < float(3.0*M_PI / 2.0)) {
            a1 = a1 - float(M_PI);
        }
        else {
            a1 = float(2.0*M_PI) - a1;
            reverse_rotation = -1.0f;
        }
    }

    // a2 is the presumed angle of the leaf node that is closest to 'n'
    // on the side closer to the zero-angle.  m_DeltaTheta was computed
    // during layout of the view. We use this as a proxy for the adjacent
    // node, which we do not have here.
    a2 = a1 - m_DeltaTheta;

    // distance from the center of the circular cladogram to current node:    
    float r = (n->XY() - m_CladogramCenter).Length();

    // Get horizontal and vertical distance from current to adjacent
    // node in screen coordinates
    float vdist = r*sin(a1) - r*sin(a2);
    float hdist = r*cos(a2) - r*cos(a1);
    vdist /= scale.Y(); // Dividing by scale gives distance in pixels
    hdist /= scale.X();

    // Get maximum text height (include descender) + some extra (1.0)
    // to keep rotated labels from overlapping
    TModelUnit th = m_SL->GetFont().GetMetric(IGlFont::eMetric_CharHeight) -
        m_SL->GetFont().GetMetric(IGlFont::eMetric_Descender) + 1.0;

    TModelUnit th_2 = th / 2.0;

    // Length of collapsed nodes can extend up to 4*m_SL->GetMaxNodeSize()
    // see:  CPhyloNodeData::GetNodeWidthScaler
    TModelUnit adjacent_offset = std::max(th, 4.0*m_SL->GetMaxNodeSize());
    TModelUnit adjacent_offset2 = adjacent_offset / 2.0;

    // Get positions of the current node and the expected location of the
    // adjacent node in screen (pixel) coordinates
    TVPPoint p1 = pane.Project(n->X(), n->Y());
    CVect2<float> current_node_lower_left_label_pos(p1.X(), p1.Y() - th_2);
    CVect2<float> adjacent_node_pos(p1.X() + hdist, p1.Y() - vdist);

    // This is the offset from the adjacent node position to the position
    // on the sphere centered on the adjacent node at 3*pi/4:
    CVect2<float> u2_offset(cos(3.0*M_PI / 4.0), sin(3.0*M_PI / 4.0));
    u2_offset *= adjacent_offset2;

    float required_angle = 0.0f;

    // When horizontal distance is very small between the nodes, we 
    // artificially increase since adjacent node Has to be to the right
    // of the current node by the radius of the circle (which is based on
    // text height) to work.  (And this makes the transition from non-rotated
    // to rotated smoother)
    if (hdist < th) {
        u2_offset.X() += (th / 2.0)*(th - hdist) / th;   // hdist 0..th_2 => th_2/2 .. 0.0
    }

    // Get vector from lower left corner of this node's label to the point on a
    // circle around the adjacent node at 3pi/4 (halfway from 90 and 180 degrees)
    CVect2<float> circle_pos(adjacent_node_pos + u2_offset);
    CVect2<float> v = circle_pos - current_node_lower_left_label_pos;

    // If nodes are far enough apart in vertical direction that text will
    // not overlap, the vector from the circle position to the lower-left
    // label position of our node will be negative (and we won't do anything)
    if (v.Y() > 0.0f) {
        // Convert vector to world (from screen) coordinates
        v.X() *= scale.X();
        v.Y() *= scale.Y();
        v.Normalize();

        // Skip label if nodes are so close that text will overlap no matter
        // what. This is for cases where viewing area is too narrow in X. If 
        // too narrow in Y, we hide all labels
        if ((CVect2<float>(p1.X(), p1.Y()) - adjacent_node_pos).Length() < th_2) {
            visible = false;
            return 0.0f;
        }

        // Dot product of the vectors is the cosine of the angle between them.
        // Since the lower edge of our text box vector is (1,0), the
        // dot product (v.dot(1,0)) is v.X()
        float dp = v.X();
        required_angle = acos(dp); // inverse cos to get angle

        // Don't let angle be > than angle to center of cladogram. sometimes when nodes 
        // are alomost on top of each other, angle will be too large.
        if (required_angle > a1)
            required_angle = a1;

        // reverse rotation if needed to match correct quadrant
        required_angle *= reverse_rotation;
    }

    return required_angle;
}


void  CTreeGraphicsModel::x_CircularLabelMinRotate(const CGlPane& pane,
                                                   const CVect3<float>& scale,
                                                   CPhyloTreeNode &n,
                                                   bool truncate_labels)
{
    // Normally node labels are set to 'not visible' when they overlap. Disable
    // that here since we compute overlap based on position around the circle
    n->SetVisible(true);

    bool visible;
    float required_angle = x_CircularLabelMinAngle(pane, scale, n, visible);

    if (visible) {
        // Render the node. Update the angle to the computed value first (then undo that)
        float a = n->GetAngle();
        n->SetAngle(required_angle);
        n->RenderRotated(pane, m_SL->GetFont(), m_ColorTable.GetPointer(), scale, a, truncate_labels);
        n->SetAngle(a);
    }
}


float CTreeGraphicsModel::GetCurrentRotationAngle(const CGlPane& pane,
                                                  const CPhyloTreeNode& node,
                                                  bool& visible) const
{
    if (!m_RotateLabels) {
        return 0.0f;
    }
    else {
        if (m_EnableCircularLabelTrimming && node.IsLeafEx()) {
            CVect3<float> scale(pane.GetScaleX(), pane.GetScaleY(), 1.0f);
            return x_CircularLabelMinAngle(pane, scale, node, visible);
        }
        else {
            return node->GetAngle();
        }
    }
}

void CTreeGraphicsModel::x_RenderNodes(CGlPane& pane,
                                       const CVect3<float>& scale)
{
    /// Visibility for the node is based on visibility of the geometry (triangle fan)
    /// not the associated label, but it would look strange to have the label (which
    /// is larger than the node) without the node.
    if (!m_TriFanNode->IsVisible())
        return;

    // Indices in the datastructure for lower-left and upper right corners of
    // current rectangle
    CTreeCollisionModel2D::TVeci posi_ll = m_CollisionData.GetMinVisIndex();
    CTreeCollisionModel2D::TVeci posi_ur = m_CollisionData.GetMaxVisIndex();

    m_NodeRenderCount = 0;

    CMatrix4<float> mat;
    mat.Identity();  

    // Set flag for all nodes in visible area that they have not been drawn
    // yet.  Must do this since node labels are alpha-blended and so look
    // different if they are drawn multiple tims.
    for (int x = posi_ll.X(); x <= posi_ur.X(); ++x) {
        for (int y = posi_ll.Y(); y <= posi_ur.Y(); ++y) {
            CTreeCollisionModel2D::TVeci idx(x,y);

            // Should always be true unless vis rectangle not intialized
            if (!m_CollisionData.InBounds(idx))
                continue;

            std::vector<size_t>& cell_nodes = m_CollisionData.Get(idx).m_Labels;

            // Flag each node in this cell as not drawn
            for (size_t i=0; i<cell_nodes.size(); ++i) {
                (*m_Tree)[cell_nodes[i]]->SetAlreadyDrawn(false);
            }
        }
    }

    bool truncate_labels = (m_SL->GetLabelTruncation() == 
                            CPhyloTreeScheme::eLabelsTruncated);

    // Iterate over the currently-visible grid which subdivides 
    // the space occupied by the tree into individual cells
    for (int x = posi_ll.X(); x <= posi_ur.X(); ++x) {
        for (int y = posi_ll.Y(); y <= posi_ur.Y(); ++y) {
            CTreeCollisionModel2D::TVeci idx(x,y);

            // Should always be true unless vis rectangle not intialized
            if (!m_CollisionData.InBounds(idx))
                continue;

            std::vector<size_t>& cell_nodes = m_CollisionData.Get(idx).m_Labels;

            // Find collisions between nodes hashed in this entry   
            for (size_t i=0; i<cell_nodes.size(); ++i) {
                // Label draw checks visibility (updated in UpdateVisibility)

                CPhyloTreeNode &n = (*m_Tree)[cell_nodes[i]];


                if (m_TriFanNode->GetLabelVisibility()) {
                    if (!n->GetAlreadyDrawn()) {
                        if (!m_RotateLabels) {
                            if (m_EnableCircularLabelTrimming && n.IsLeafEx()) {
                                x_CircularLabelTrim(pane, scale, n, truncate_labels);
                            }
                            else {
                                n->Render(pane, m_SL->GetFont(), m_ColorTable.GetPointer(), scale, truncate_labels);
                            }                            
                        }
                        else {
                            if (m_EnableCircularLabelTrimming && n.IsLeafEx()) {
                                x_CircularLabelMinRotate(pane, scale, n, truncate_labels);
                            }
                            else {
                                n->RenderRotated(pane, m_SL->GetFont(), m_ColorTable.GetPointer(), scale, n->GetAngle(), truncate_labels);
                            }
                        }
                        n->SetAlreadyDrawn(true);
                    }
                }
                ++m_NodeRenderCount;

                bool vis = false;
                vis =  (n->GetDisplayChildren() != CPhyloNodeData::eHideChildren) &&
                        !n->HasNodeMarker();

                if (vis) {
                    mat[3] = n->X();  // mat[3] = mat(0,3)
                    mat[7] = n->Y();  // mat[7] = mat(1,3)  

                    mat[0] = n->GetNodeDisplaySize();    // mat[0] == mat(0,0)
                    mat[5] = n->GetNodeDisplaySize();    // mat[5] == mat(1,1)  

                    CRgbaColor c = n->GetNodeDisplayColor();
                    m_TriFanNode->SetPosition(mat);
                    m_TriFanNode->GetState().ColorC(c);
                    m_TriFanNode->Render();
                }
            }
        }
    }
}

void CTreeGraphicsModel::Render(CGlPane& pane)
{ 
    CStopWatch  render_total;
    render_total.Start();

    CVect3<float> scale(pane.GetScaleX(), pane.GetScaleY(), 1.0f);

    pane.OpenOrtho();
    CGlUtils::CheckGlError();

    IRender& gl = GetGl();

    // Don't want to combine texture with color.  Why does glcolor matter if
    // mode is decal?
    glTexEnvf(GL_TEXTURE_ENV,  GL_TEXTURE_ENV_MODE, GL_DECAL);
    CRgbaColor c(1.0f, 1.0f, 1.0f, 1.0f);
    glColorC(c);

    CStopWatch  t;
    t.Start();

    if (m_BoundaryNode != NULL) {
        m_BoundaryNode->SetIncludeLabels(m_TriFanNode->GetLabelVisibility());
        m_BoundaryNode->SetPane(pane);
        m_BoundaryNode->GetState().ScaleInvarient(true, pane.GetScale());
        m_BoundaryNode->Render();
    }

    m_BoundaryRenderTime = t.Restart();

    // Set as not visible any CGlVboNode objects that render marker nodes
    // if nodes in general are not visible.
    for (size_t i=0; i<m_TempGeomNodes.size(); ++i) {
        if (m_TempGeomNodes[i]->GetName() == "MarkerNode") {
            m_TempGeomNodes[i]->SetVisible(m_TriFanNode->IsVisible());
        }
    }

    CGlModel2D::Render(pane);

    m_LineRenderTime = (float)t.Restart();

    if (m_TriFanNode != NULL) {
        x_RenderNodes(pane, scale);
    }

    m_NodesRenderTime = (float)t.Restart();    

    m_RenderTime = render_total.Elapsed();

    CGlUtils::CheckGlError();
    pane.Close();

    // other views don't concern themselves with textures
    // so make sure these are disabled before returning
    gl.Disable(GL_TEXTURE_1D);
    gl.Disable(GL_TEXTURE_2D);
}

void CTreeGraphicsModel::x_RenderNodeLabelsPdf(CGlPane& pane,
                                               CRef<CPdf>& pdf,
                                               CVect3<float> scale)
{
    if (!m_TriFanNode->GetLabelVisibility())
        return;

    // This is only in case we want tooltips in the pdf file
    CPhyloTreeLabel label;
    label.SetScheme(m_SL.GetObject(), m_Tree);

    IRender& gl = GetGl();

    bool truncate_labels = (m_SL->GetLabelTruncation() == 
                            CPhyloTreeScheme::eLabelsTruncated);

    // Not easy to clip rotated labels
    if (GetRotateLabels()) 
        truncate_labels = false;

    float uu_inv = 1.0f/pdf->GetOptions().GetUserUnit();
    scale *= uu_inv;

    // Do all backgrounds first since they are separate objects (in pdf)
    // and we're putting all the text into a single object.
    size_t i;
    gl.Enable(GL_BLEND);
    for (i=0; i<m_NodeIndices.size(); ++i) {
        CPhyloTreeNode& n = (*m_Tree)[m_NodeIndices[i]];

        if (!n->GetAlreadyDrawn() && n->GetVisible()) {
            string txt;
            CVect2<float> pos = n->GetTextOut(pane, m_SL->GetFont(), scale, truncate_labels, txt);

            gl.LoadIdentity();
            if (n->GetLabelBgColorIdx() != -1 && txt.length() > 0) {
                CRgbaColor c = m_ColorTable->GetColor((int)n->GetLabelBgColorIdx());
                gl.Color4fv((GLfloat*)c.GetColorArray());

                // There seems to be some kind of off-by-one error in here in that I have to specify
                // - pane.GetScaleX/Y() to get the correct position.  And in the rotated case this works
                // but not up until the largest (zoomed in until uu_inv is very small) images.
                if (!m_RotateLabels) {    
                    TModelUnit text_h = gl.GetMetric(&(m_SL->GetFont()), IGlFont::eMetric_CharHeight);
                    TModelUnit text_w = gl.TextWidth(&(m_SL->GetFont()), txt.c_str());

                    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    gl.RectC(TModelRect(pos.X() - pane.GetScaleX(),
                                        pos.Y() - pane.GetScaleY(),
                                        pos.X() + (text_w + 1.0) * pane.GetScaleX()*double(uu_inv) - pane.GetScaleX(),
                                        pos.Y() + (text_h + 1.0) * pane.GetScaleY()*double(uu_inv) - pane.GetScaleY()));

                }
                else {
                    float angle;
                    CVect4<CVect2<float> > corners = n->GetRotatedTextBox(scale, angle);
                    gl.Begin(GL_TRIANGLE_STRIP);
                    gl.Vertex2f(corners[3].X() - pane.GetScaleX(), corners[3].Y() - pane.GetScaleY());
                    gl.Vertex2f(corners[0].X() - pane.GetScaleX(), corners[0].Y() - pane.GetScaleY());
                    gl.Vertex2f(corners[2].X() - pane.GetScaleX(), corners[2].Y() - pane.GetScaleY());
                    gl.Vertex2f(corners[1].X() - pane.GetScaleX(), corners[1].Y() - pane.GetScaleY());
                    gl.End();
                }
            } 
        }
    }


    // Now print out all the text
    CRef<CPdfObject> content = pdf->BeginContent(CPdf::ePdfText); 

    for (i=0; i<m_NodeIndices.size(); ++i) {
        CPhyloTreeNode &n = (*m_Tree)[m_NodeIndices[i]];

        if (!n->GetAlreadyDrawn()) {
            if (n->GetVisible()) {
                string txt;
                CVect2<float> pos;
                CVect2<float> posp;  
                double px, py;
                double dummyz;

                CGlTextureFont::EFontFace face = m_SL->GetFont().GetFontFace();
                float font_size = (float)m_SL->GetFont().GetFontSize();

                CRgbaColor c = n->GetTextDisplayColor();

                if (!m_RotateLabels) {
                    pos = n->GetTextOut(pane, m_SL->GetFont(), scale, truncate_labels, txt);

                    CGlRect<float> r = n->GetLabelRect();                  
                
                    pos.X() = (float)(n->XY().X());
                    pos.Y() = (float)(n->XY().Y());

                    gluProject(pos.X(), pos.Y(), 0.0, 
                        m_ModelviewMatrix, m_ProjectionMatrix, m_Viewport, 
                        &px, &py, &dummyz);
                    posp.X() = px + r.Left()*uu_inv;
                    posp.Y() = py + r.Bottom()*uu_inv;

                    pdf->PrintText(pdf->GetFontHandler(), face, font_size, posp, 
                        txt.c_str(), c);
                }
                else {
                    float a;
                    pos = n->GetRotatedTextPos(scale, a);
                    float cos_a = cosf(a);
                    float sin_a = sinf(a);

                    gluProject(pos.X(), pos.Y(), 0.0, 
                        m_ModelviewMatrix, m_ProjectionMatrix, m_Viewport, 
                        &px, &py, &dummyz);
                    posp.X() = px;
                    posp.Y() = py;

                    // Matrix for transforming text - rotates around the point posp.  Matrix is
                    // given in column major order (so sin_a/-sin_a are switched)
                    CMatrix3<double> m(cos_a, -sin_a, posp.X(), sin_a, cos_a, posp.Y(), 0.0f, 0.0f, 1.0f);

                    pdf->PrintText(pdf->GetFontHandler(), face, font_size, m,
                        n->GetDisplayLabel().c_str(), c);
                }
            }


            if (m_RenderPdfTooltips) {
                std::string final_text = label.GetToolTipForNode(
                    *m_Tree, n, m_SL->SetTooltipFormat(), "\n");
                CVect2<float> lower_left;
                CVect2<float> upper_right;
                n->GetBoundingRect(CVect2<float>(scale.X(), scale.Y()), lower_left, upper_right, m_RotateLabels);
                if (final_text.length() == 0) {
                    lower_left -= 2.0f;
                    upper_right += 2.0f; 
                }
                else {
                    lower_left.Y() += 3.0f;
                    upper_right.Y() -= 3.0f; 
                }
                CVect4<float> bound(lower_left.X(), lower_left.Y(), upper_right.X(), upper_right.Y());

                // These tips use javascript alert boxes (less attractive...)
                //pdf->AddJSTooltip(pane, final_text, n->GetDisplayLabel(), bound);
                pdf->AddTooltip(pane, final_text, bound);
            }

            n->SetAlreadyDrawn(true);
        }
    }
    pdf->EndContent();
}

CVect2<float> CTreeGraphicsModel::x_Project(CVect2<float>& p)
{
    double px, py;
    double dummyz;    

    gluProject(p.X(), p.Y(), 0.0, 
        m_ModelviewMatrix, m_ProjectionMatrix, m_Viewport, 
        &px, &py, &dummyz);

    return CVect2<float>((float)px, (float)py);  
}

    
void CTreeGraphicsModel::x_RenderNodesPdf(CGlPane& pane,
                                          CRef<CPdf>& pdf,
                                          CVect3<float> scale)
{
    if (!m_TriFanNode->IsVisible())
        return;

    map<int, string> shader_ids;

    // For rendering to the screen, I do not use a color buffer since there are only two colors
    // (I use a blending trick instead).  Here create the needed color buffer for pdf rendering.
    vector<CRgbaColor> colors, prev_colors;

    m_TriFanNode->GetColorBuffer(prev_colors);

    CRgbaColor cback(1.0f, 1.0f, 1.0f);
    float uu_inv = 1.0f/pdf->GetOptions().GetUserUnit();
    scale *= uu_inv;

    IRender& gl = GetGl();

    // for each node, check its color and see if we have already drawn a node
    // of the same color to the pdf file.  If so, just add a new shader 
    // reference to the existing pdf triangle shader with an appropriate 
    // transformation.  If not, create a new pdf shader and add it to the
    // shading dictionary.
    for (size_t i=0; i<m_NodeIndices.size(); ++i) {
        CPhyloTreeNode& n = (*m_Tree)[m_NodeIndices[i]];

        bool vis = false;
        vis =  (n->GetDisplayChildren() != CPhyloNodeData::eHideChildren) &&
               !n->HasNodeMarker();

        if (vis) {
                // create a unique identifer (unsigned int) for the color
                CRgbaColor c = n->GetNodeDisplayColor();

                unsigned int id = c.GetRedUC()<<16;
                id |= c.GetGreenUC()<<8;
                id |= c.GetBlueUC();

                // see if we've already created a node of this color
                string shader = shader_ids[id];

                // If not, create a new triangle shader node and map its identifer
                // to the colors unique id value
                if (shader == "") {
                    // Fill color buffer before calling pdf to render tris:
                    colors.clear();
                    colors.resize(prev_colors.size(), c);
                    colors[0] = cback;
                    m_TriFanNode->SetColorBuffer(colors);

                    //shader = pdf->AddShadedTris(m_TriFanNode);
                    shader = pdf->AddShadedTris(m_TriFanNode, pane.GetViewport(), 
                        CPdfObject::e16Bit, m_ColorTable.GetPointer(), -200, 200, -200, 200);

                    shader_ids[id] = shader;
                }

                // Pdf render function gets transformation values from opengl               
                CMatrix3<double> mat;
                mat.Identity();

                gl.MatrixMode(GL_MODELVIEW);
                gl.PushMatrix();
                 
                gl.Translatef(n->X(), n->Y(), 0.0f);                

                gl.Scalef(n->GetNodeDisplaySize() * scale.X(), n->GetNodeDisplaySize() * scale.Y(), 1.0f);

                // create a reference in the pdf file to the shader (tris) at the nodes position
                pdf->RenderShaderInstance(m_TriFanNode, shader, pane.GetViewport());
                gl.PopMatrix();
        }
    }

    // Restore previous colors for node (so that it will work correctly
    // in non-pdf drawing)
    m_TriFanNode->SetColorBuffer(prev_colors);
}

void CTreeGraphicsModel::RenderPDF(CGlPane& pane, CRef<CPdf>& pdf)
{  
    pane.OpenOrtho();

    // get viewport directly from pane since size may exceed
    // opengl max viewport size (but not max pdf viewable page size)
    //glGetIntegerv(GL_VIEWPORT, m_Viewport);
    m_Viewport[0] = pane.GetViewport().Left();
    m_Viewport[1] = pane.GetViewport().Bottom();
    m_Viewport[2] = pane.GetViewport().Width();
    m_Viewport[3] = pane.GetViewport().Height();

    IRender& gl = GetGl();
    gl.GetModelViewMatrix(m_ModelviewMatrix);
    gl.GetProjectionMatrix(m_ProjectionMatrix);

    CVect3<float> scale((float)pane.GetScaleX(), (float)pane.GetScaleY(), 1.0f);

    if (m_BoundaryNode != NULL) {
        m_BoundaryNode->SetIncludeLabels(m_TriFanNode->GetLabelVisibility());
        m_BoundaryNode->SetPane(pane);
        m_BoundaryNode->GetState().ScaleInvarient(true, pane.GetScale());
        m_BoundaryNode->RenderPdf(pdf);
    }

    // Set as not visible any CGlVboNode objects that render marker nodes
    // if nodes in general are not visible.
    for (size_t i = 0; i<m_TempGeomNodes.size(); ++i) {
        if (m_TempGeomNodes[i]->GetName() == "MarkerNode")
            m_TempGeomNodes[i]->SetVisible(m_TriFanNode->IsVisible());
    }
    
    pdf->PrintModel(pane, *this, m_ColorTable.GetPointer());
    x_RenderNodesPdf(pane, pdf, scale);
    x_RenderNodeLabelsPdf(pane, pdf, scale);  

    pane.Close();
}


END_NCBI_SCOPE
