    /*  $Id: phylo_tree_render.cpp 43849 2019-09-10 14:59:34Z evgeniev $
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
#include <gui/widgets/phylo_tree/phylo_tree_render.hpp>

#include <util/image/image.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>
#include <gui/widgets/gl/mouse_zoom_handler.hpp>
#include <gui/widgets/wx/gui_event.hpp>
#include <gui/opengl/glframebuffer.hpp>
#include <gui/widgets/wx/glcanvas.hpp>
#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/glexception.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/ftglfontmanager.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/print/glrenderpdf.hpp>
#include <gui/widgets/gl/attrib_menu.hpp>
#include <gui/utils/vect2.hpp>
#include <gui/widgets/gl/mouse_zoom_handler.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <math.h>
#include <algorithm>
#include <limits>
#include <cmath>


#define PTRENDER_EFFECTS_TIMER_ID 63200
#define PHYLO_MOUSE_CLICK_TIMER_ID 63201
#define M_NCBI_E 2.71828182845904523536f


BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE(IPhyloTreeRender, wxEvtHandler)
    EVT_LEFT_DOWN(IPhyloTreeRender::OnLeftDown)
    EVT_LEFT_DCLICK(IPhyloTreeRender::OnLeftDblClick)
    EVT_LEFT_UP(IPhyloTreeRender::OnLeftUp)
    EVT_RIGHT_DOWN(IPhyloTreeRender::OnRightDown)
    EVT_MOTION(IPhyloTreeRender::OnMotion)
    EVT_TIMER(PTRENDER_EFFECTS_TIMER_ID, IPhyloTreeRender::OnTimer)
    EVT_TIMER(PHYLO_MOUSE_CLICK_TIMER_ID, IPhyloTreeRender::OnTimerLeftUp)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

IPhyloTreeRender::IPhyloTreeRender()
    : m_EffectsTimer(this, PTRENDER_EFFECTS_TIMER_ID)
    , m_MouseSingleTimer(this, PHYLO_MOUSE_CLICK_TIMER_ID)
{
    m_DS            = NULL;
    m_pLabelFont    = NULL;
    m_DistMode     = false;
    m_UseSplines = false;
    m_RotatedLabels = false;
    m_ForceSquare = false;
    m_RegenerateTexture = false;
    m_LabelViewPct = 0.6f;
    m_RenderScale = true;
    m_TreePixels = 0.0f;
    m_LabelPixels = 0.0f;
    m_State = eIdle;
    m_pMouseZoomHandler = NULL;
    m_ActiveTooltipNode = NULL_TREE_IDX;
    m_SwitchedLayout = false;
    m_ValidLayout = eNeedLayoutAndSize;

    // raster rect is setup to default hardcoded values
    // it can be changed by Layout()
    m_DimX      = 10000;
    m_DimY      = 10000;
    m_DistanceBarLowerLeft = true;
    m_RasterRect.Init(0, 0, m_DimX, m_DimY);
    m_SL.Reset(new CPhyloTreeScheme()); 
    m_BufferdRenderTime = 0.0f;
    m_LayoutTime = 0.0f;
    m_HighlightLongerEdges = false;
    m_HighlightFunctionK = 50.0f;
    m_HighlightFunctionMidpoint = 90.0f;
    m_HighlightIntensity = 1.0f;

#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenu& m = CAttribMenuInstance::GetInstance();
    CAttribMenu* sub_menu = m.AddSubMenuUnique("Phylo Render", this);
    sub_menu->AddFloatReadOnly("Buffered Render Time", &m_BufferdRenderTime);
    sub_menu->AddFloatReadOnly("VBO Sync time", &m_VboSyncTime);    
    sub_menu->AddFloatReadOnly("Layout Time", &m_LayoutTime);
    sub_menu->AddFloat("Highlight K", &m_HighlightFunctionK, 50.0f, 1.0f, 500.0f, 10.0f);
    sub_menu->AddFloatReadOnly("Highlight Midpoint", &m_HighlightFunctionMidpoint);
#endif
}

IPhyloTreeRender::IPhyloTreeRender(double width, double height)
    : m_DimX(width), m_DimY(height)
    , m_EffectsTimer(this, PTRENDER_EFFECTS_TIMER_ID)
    , m_MouseSingleTimer(this, PHYLO_MOUSE_CLICK_TIMER_ID)
{
    m_pPane         = NULL;
    m_DS            = NULL;
    m_pLabelFont    = NULL;
    m_DistMode     = false;
    m_UseSplines = false;
    m_RotatedLabels = false;
    m_ForceSquare = false;
    m_RegenerateTexture = false;
    m_LabelViewPct = 0.6f;
    m_RenderScale = true;
    m_TreePixels = 0.0f;
    m_LabelPixels = 0.0f;
    m_State = eIdle;
    m_DistanceBarLowerLeft = true;
    m_RasterRect.Init(0, 0, m_DimX, m_DimY);
    m_SL.Reset(new CPhyloTreeScheme());
    m_ActiveTooltipNode = NULL_TREE_IDX;
    m_SwitchedLayout = false;
    m_ValidLayout = eNeedLayoutAndSize;
    m_BufferdRenderTime = 0.0f;
    m_LayoutTime = 0.0f;
    m_HighlightLongerEdges = false;
    m_HighlightFunctionK = 50.0f;
    m_HighlightFunctionMidpoint = 90.0f;
    m_HighlightIntensity = 1.0f;

#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenu& m = CAttribMenuInstance::GetInstance();
    CAttribMenu* sub_menu = m.AddSubMenuUnique("Phylo Render", this);
    sub_menu->AddFloatReadOnly("Buffered Render Time", &m_BufferdRenderTime);
    sub_menu->AddFloatReadOnly("VBO Sync time", &m_VboSyncTime);
    sub_menu->AddFloatReadOnly("Layout Time", &m_LayoutTime);
    sub_menu->AddFloat("Highlight K", &m_HighlightFunctionK, 50.0f, 1.0f, 500.0f, 10.0f);
    sub_menu->AddFloatReadOnly("Highlight Midpoint", &m_HighlightFunctionMidpoint);
#endif
}

IPhyloTreeRender::~IPhyloTreeRender()
{
#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenuInstance::GetInstance().RemoveMenuR("Phylo Render", this);
#endif
}

void IPhyloTreeRender::SetHost(IPhyloTreeRenderHost* pHost)
{
    m_pHost = pHost;
}

void IPhyloTreeRender::SetPane(CGlPane* pane)
{
    if (pane) {
        m_pPane = pane;
    }
}

void IPhyloTreeRender::x_RenderSelection(CGlPane& pane)
{
    IRender& gl = GetGl();

    CPhyloTree::TTreeIdx current_idx = m_DS->GetTree()->GetCurrentNodeIdx();

    // Draw a stippled line around the currently selected node, if any
    if (current_idx != CPhyloTree::Null()) {
        CPhyloTreeNode& node = m_DS->GetTree()->GetNode(current_idx);

        // Render a alpha-blended circular overlay to emphasize node more
        float node_size = GetNodeSize(&node);
        if (node_size > 0.0f) {
            CRgbaColor c(0.1f, 0.1f, 1.0f, 0.3f);
            x_RenderNodeHighlight(pane, current_idx, c);
        }

        if (node_size > 0.0f) {
            CGlAttrGuard AttrGuard(GL_POLYGON_BIT | GL_LINE_BIT);
            pane.OpenPixels();

            float node_size_scaler = 1.0f;
            // Use smaller circle for collapsed nodes (which are triangles, not circles)
            if (!node.Expanded() && node_size > 2.0f) {
                node_size /= 2.0f;
                node_size_scaler = 0.5f;
            }

            float r;
            CVect2<float> pos = node->GetNodePosEx(CVect3<float>(pane.GetScale().X(), pane.GetScale().Y(), 1.0f),
                                                   m_SL.GetPointer(), r, node_size_scaler);
            TVPPoint p = pane.Project(pos.X(), pos.Y());

            gl.MatrixMode(GL_MODELVIEW);
            gl.Disable(GL_BLEND);
            gl.Disable(GL_TEXTURE_2D);

            gl.PushMatrix();
            gl.LoadIdentity();
            gl.Translatef((float)p.X(), (float)p.Y(), 0.0);
            
            gl.LineWidth(1.0f);
            gl.Color3f(0.0f, 0.0f, 0.0f);
            gl.LineStipple(3, 0xAAAA);
            gl.Enable(GL_LINE_STIPPLE);
            gl.Disable(GL_LINE_SMOOTH);

            node_size += 1.0f;
            gl.Begin(GL_LINES);
            gl.Vertex2f(-node_size, -node_size);
            gl.Vertex2f(node_size,  -node_size);

            gl.Vertex2f(-node_size,  node_size);
            gl.Vertex2f(node_size,   node_size);

            gl.Vertex2f(-node_size, -node_size);
            gl.Vertex2f(-node_size,  node_size);

            gl.Vertex2f(node_size,  -node_size);
            gl.Vertex2f(node_size,   node_size);
            gl.End();

            gl.Disable(GL_LINE_STIPPLE);
            gl.PopMatrix();
            pane.Close();
        }
    }

    // Draw the selection rectangle, if user is currently doing a drag-selection
    if(m_State == eSelRect) {
        _ASSERT(m_pHost);

        CGlAttrGuard AttrGuard(GL_POLYGON_BIT | GL_LINE_BIT);
        pane.OpenPixels();

        gl.LineWidth(1.0f);
        gl.Color3f(0.0f, 0.0f, 0.0f);

        gl.LineStipple(1, 0x0F0F);
        gl.Enable(GL_LINE_STIPPLE);
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        gl.Disable(GL_LINE_SMOOTH);
        gl.Disable(GL_BLEND);
        gl.Disable(GL_TEXTURE_2D);

        int x1 = m_StartPoint.x;
        int y1 = m_pHost->HMGH_GetVPPosByY(m_StartPoint.y);
        int x2 = m_DragPoint.x;
        int y2 = m_pHost->HMGH_GetVPPosByY(m_DragPoint.y);
        if(x2 < x1)
            swap(x1, x2);
        if(y2 < y1)
            swap(y1, y2);

        gl.Begin(GL_LINES);
        gl.Vertex2d(x1, y2);
        gl.Vertex2d(x2, y2);

        gl.Vertex2d(x2, y2);
        gl.Vertex2d(x2, y1);

        gl.Vertex2d(x1, y2);
        gl.Vertex2d(x1, y1);

        gl.Vertex2d(x1, y1);
        gl.Vertex2d(x2, y1);
        gl.End();

        gl.Disable(GL_LINE_STIPPLE);

        pane.Close();
    }

    if (m_SelectedEdge.size() > 0) {
        CGlAttrGuard AttrGuard(GL_POLYGON_BIT | GL_LINE_BIT);
        pane.OpenOrtho();

        gl.LineWidth(5.0f);
        gl.Color4f(1.0f, 0.0f, 0.0f, 0.3f);

        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl.Disable(GL_TEXTURE_2D);

        gl.Begin(GL_LINES);
        for (size_t i = 0; i < m_SelectedEdge.size(); ++i) {
            gl.Vertex2fv(m_SelectedEdge[i].GetData());
        }
        gl.End();

        pane.Close();
    }
}

// Compute node distance for a fixed pixel offset using
// unproject.
double IPhyloTreeRender::ComputeDistFromPixels(CGlPane& pane,
                                               const CVect2<TModelUnit>& pixpos1, 
                                               const CVect2<TModelUnit>& pixpos2) const {
    CVect2<TModelUnit> pt1 = pane.UnProjectEx(pixpos1);
    CVect2<TModelUnit> pt2 = pane.UnProjectEx(pixpos2);

    // m_DimX is a factor used during layout to force
    // the graph to project to a virtual window of width m_DimX (labels not
    // included) so we divide it out here.  GetNormDistance is the
    // underlying graph width that gets scaled (at layout) to m_DimX.
    return (pt2.X() - pt1.X())*(m_DS->GetNormDistance()/m_DimX);
}

// Compute the number (rounded to the nearest significant digit) that will be displayed
// for the scale (and this will also determine the length of the scale marker)
double IPhyloTreeRender::ComputeScale(double default_scale, int& precision) const {                                        
    double rounded = default_scale;
    double multiplier;
    precision = 0;
    if (default_scale < 1.0) { 
        multiplier = 1.0;
        while (rounded < 1.0) {
            multiplier *= 10.0;
            ++precision;
            rounded = default_scale*multiplier;
        }
        precision = std::max(precision, 1);
        rounded = (double)((int)(floor(rounded) + 0.5));
        rounded = rounded / multiplier;
    }
    else {
        precision = 1;
        multiplier = 1.0;
        while (rounded > 10.0) {
            multiplier /= 10.0f;
            rounded = default_scale*multiplier;
        }
        rounded = (double)((int)(floor(rounded) + 0.5));
        rounded = rounded / multiplier;
    }

    return rounded;
}

void IPhyloTreeRender::SetHighlightEdges(bool enable)
{
    // Edges don't have length to highlight if we are not in distance mode
    if (!m_DistMode)
        m_HighlightLongerEdges = false;
    else 
        m_HighlightLongerEdges = enable;
}

void IPhyloTreeRender::x_ComputeEdgeHighlight()
{
    if (m_HighlightLongerEdges) {
        // Use zoom level, size of tree and length data distribution to compute
        // parameters for the edge length highlight function

        float tree_size = (float)m_DS->GetSize();

        // Spacing between highlighted nodes. Smaller=>more highlighted nodes
        float kSpacing = 4.0f;

        // visible_highlighted_nodes is the # of nodes we want to highlight, so
        // we compute that then turn it into a precent.
        float vp_height = (float)m_pPane->GetViewport().Height();
        float pct_visible = m_pPane->GetVisibleRect().Height() / m_pPane->GetModelLimitsRect().Height();
        float total_height = vp_height * (1.0 / pct_visible);
        float visible_highlighted_nodes = std::min(total_height / kSpacing, tree_size);

        // The midpoint of the s-curve in the logistics function representing
        // the approximate precentage of nodes highlighted when highlighting is
        // enabled. 0=> 100% and 1 would be 0%. (so result always < 1.0)
        m_HighlightFunctionMidpoint = 1.0f - std::min(1.0f, (visible_highlighted_nodes / tree_size) + 0.01f);

        float dist_between_nodes = (m_yStep*m_pPane->GetOutputUnitScaler()) / m_pPane->GetScaleY();
        if (dist_between_nodes < 0.5f)
            m_HighlightIntensity = 0.9f;
        else
            m_HighlightIntensity = std::max(0.0f, ((1.0f - dist_between_nodes) / 0.5f)-0.1f);
    }

    // Need to have different blending modes when highlighting is enabled (with alpha
    // blending overlapping edges become very dark)
    // Set drawing options that are specific to slanted cladogram
    CGlVboNode* edge_node = m_DS->GetModel().FindGeomNode("TreeEdges");
    CGlVboNode* sel_edge_node = m_DS->GetModel().FindGeomNode("SelectedTreeEdges");
    CGlVboNode* filler_points_node = m_DS->GetModel().FindGeomNode("FillerPoints");
    CGlVboNode* sel_filler_points_node = m_DS->GetModel().FindGeomNode("SelectedFillerPoints");

    if (m_HighlightLongerEdges && m_HighlightIntensity > 0.0f) {
        // computes lengths Only if not already computed.
        m_DS->ComputeLengthsFromRoot();

        if (edge_node != NULL) {
            edge_node->GetState().Enable(GL_BLEND);
            edge_node->GetState().BlendEquation(GL_MIN);
            edge_node->GetState().BlendFunc(GL_ONE, GL_ONE);
        }
        if (sel_edge_node != NULL) {
            sel_edge_node->GetState().Enable(GL_BLEND);
            sel_edge_node->GetState().BlendEquation(GL_MIN);
            sel_edge_node->GetState().BlendFunc(GL_ONE, GL_ONE);
        }
        if (filler_points_node != NULL) {
            filler_points_node->GetState().Enable(GL_BLEND);
            filler_points_node->GetState().BlendEquation(GL_MIN);
            filler_points_node->GetState().BlendFunc(GL_ONE, GL_ONE);
        }
        if (sel_filler_points_node != NULL) {
            sel_filler_points_node->GetState().Enable(GL_BLEND);
            sel_filler_points_node->GetState().BlendEquation(GL_MIN);
            sel_filler_points_node->GetState().BlendFunc(GL_ONE, GL_ONE);
        }
    }
    else {
        if (edge_node != NULL) {
            edge_node->GetState().Enable(GL_BLEND);
            edge_node->GetState().BlendEquation(GL_FUNC_ADD);
            edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (sel_edge_node != NULL) {
            sel_edge_node->GetState().Enable(GL_BLEND);
            sel_edge_node->GetState().BlendEquation(GL_FUNC_ADD);
            sel_edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (filler_points_node != NULL) {
            filler_points_node->GetState().Enable(GL_BLEND);
            filler_points_node->GetState().BlendEquation(GL_FUNC_ADD);
            filler_points_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (sel_filler_points_node != NULL) {
            sel_filler_points_node->GetState().Enable(GL_BLEND);
            sel_filler_points_node->GetState().BlendEquation(GL_FUNC_ADD);
            sel_filler_points_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }
}

void IPhyloTreeRender::x_RenderScaleMarker(CGlPane& pane, IRender& gl)
{
    // Can turn off scale rendering explicitly, and if we are not in distance
    // mode it is also turned off automatically since redering scales without
    // distance makes no sense.
    if (!m_RenderScale || !m_DistMode)
        return;

    // target length in pixels of the distance bar
    float target_len = pane.GetPixelDist(TModelUnit(100)); 
    // actual length of distance bar, after rounding
    float final_len = target_len;

    float base_xpos;
    float base_ypos = 20.0f; 
    float bar_height = pane.GetPixelDist(TModelUnit(20)); 
    float delta =  pane.GetPixelDist(TModelUnit(5)); 

    // Don't try to draw if there is not enough room for
    // the error bar.  This also avoids some divide-by-0 errors
    // for when the viewport width goes to 0.
    if (pane.GetViewport().Width() <= target_len || 
        pane.GetViewport().Height() < bar_height)
        return;
    
    //
    // Section is a bit strange for pdf... - is the viewport
    // the combined viewport??
    //
    pane.OpenOrtho();   

    if (IsDistanceBarLowerLeft()) {
        base_xpos = 2.0f*delta;
    }
    else {
        base_xpos = (float)pane.GetViewport().Width() -
                    (1.5f*target_len + 20.0f); 
        base_xpos = std::max(0.0f, base_xpos);
    }

    // Get node distance for a fixed pixel offset
    double default_scale = ComputeDistFromPixels(pane, CVect2<TModelUnit>(TModelUnit(base_xpos), TModelUnit(base_ypos)),
        CVect2<TModelUnit>(TModelUnit(base_xpos+target_len), TModelUnit(base_ypos)));

    pane.Close();

    // a 0 value or something very close to it won't work.  this
    // happens if a tree doesn't have distances specified for nodes.
    if (default_scale < std::numeric_limits<double>::epsilon())
        return;

    pane.OpenPixels();

    // Represent the distance bar with only 1 significant digit (first non-zero).
    // This seems a little simplistic but it seems to work OK. 
    int precision = 0;
    double rounded = ComputeScale(default_scale, precision);
    final_len = target_len * rounded/default_scale;    
    char buf_round[32];
    sprintf(buf_round, "%.*f", precision, rounded);
    

    // Drawing options
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.Disable(GL_LINE_SMOOTH);
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.ColorC(CRgbaColor(0.7f, 0.7f, 0.7f, 0.4f));

    TModelRect r(base_xpos-delta, base_ypos-delta, base_xpos + final_len + delta, base_ypos + bar_height + delta);

    // Draw a quad for the distance bar background
    gl.RectC(r);

    // Draw the scale (distance) bar
    gl.ColorC(CRgbaColor(0.0f, 0.0f, 0.0f, 0.6f));
    gl.LineWidth((GLfloat)pane.GetPixelDist(TModelUnit(2.0)));
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl.Begin(GL_LINES);
        gl.Vertex2f(base_xpos, base_ypos);
        gl.Vertex2f(base_xpos, base_ypos + bar_height);

        gl.Vertex2f(base_xpos, base_ypos + bar_height/2.0f);
        gl.Vertex2f(base_xpos + final_len, base_ypos + bar_height/2.0f);
    
        gl.Vertex2f(base_xpos + final_len, base_ypos);
        gl.Vertex2f(base_xpos + final_len, base_ypos + bar_height);
    gl.End();

    // draw the length represented by the distance bar
    const CGlTextureFont& font = m_SL->GetFont();    
    float tx = base_xpos + (float)delta;
    float ty = base_ypos + bar_height/2.0f + (float)delta;
       
    gl.BeginText(&font, CRgbaColor(0.0f, 0.0f, 0.0f, 0.6f));
    gl.WriteText(tx, ty, buf_round);
    gl.EndText();      

    pane.Close();  
}


void IPhyloTreeRender::x_RenderTreeLabel(CGlPane& pane, IRender& gl)
{
    CTreeLabel tl = m_DS->GetTreeLabel();

    // If no label, return
    if (tl.GetLabel().length() == 0)
        return;

    pane.OpenPixels();

    // Label position is in % 0..100 
    float tx = (float(pane.GetViewport().Width())) * ((float)tl.GetXPos())/100.0f;
    float ty = (float(pane.GetViewport().Height())) * ((float)tl.GetYPos())/100.0f;
 

    CGlTextureFont::EFontFace ff = CGlTextureFont::FaceFromString(tl.GetFontName());

    // If bug:
    if (ff == CGlTextureFont::eFontFace_LastFont)
        ff = CGlTextureFont::eFontFace_Helvetica;

    CGlTextureFont tf(ff, tl.GetFontSize());

    float width = (float)tf.TextWidth(tl.GetLabel().c_str());
    float height = (float)tf.TextHeight();

    tx -= width/2.0f;
    ty -= height/2.0f;

    // draw the label:       
    gl.BeginText(&tf, tl.GetColor());
    gl.WriteText(tx, ty, tl.GetLabel().c_str());
    gl.EndText();      

    pane.Close();  
}

void IPhyloTreeRender::x_RenderTooltipHints(CGlPane& pane)
{
    // If mouse is currently over tooltip for this node...    
    if (m_ActiveTooltipNode != NULL_TREE_IDX) {
        CRgbaColor c = m_SL->SetColor(CPhyloTreeScheme::eNode,
            CPhyloTreeScheme::eTipSelColor);

        x_RenderNodeHighlight(pane, m_ActiveTooltipNode, c);

    }

  
    // Render temporary arrows, if any currently active
    std::vector<NodePointer>::iterator niter, erase_iter = m_NodePointers.end();
    for (niter=m_NodePointers.begin(); niter!=m_NodePointers.end(); ++niter) {
        
        if ((*niter).m_NodeIdx != CPhyloTreeNode::Null()) {
            CPhyloTreeNode& node = m_DS->GetTree()->GetNode((*niter).m_NodeIdx);

            NodePointer np = *niter;
            float alpha = 0.5f;

            double x = (*node).X();
            double y = (*node).Y();

            // Check if this arrow has timed out.  Can erase 1 at a time (ok because
            // there will be very few arrows)
            if (np.m_Duration > 0.0) {
                if (np.m_Timer.Elapsed() > np.m_Duration) {
                    alpha = 0.0f;
                    erase_iter = niter;                   
                    break;
                }
                else alpha *= 1.0f - np.m_Timer.Elapsed()/np.m_Duration;
            }

            if (alpha > 0.0f) {                     
                CRgbaColor c = m_SL->SetColor(CPhyloTreeScheme::eNode,
                                              CPhyloTreeScheme::eTipSelColor);

                IRender& gl = GetGl();

                gl.Enable(GL_BLEND);
                gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                c.SetAlpha(alpha);
                gl.Color4fv(c.GetColorArray());

                // Render in screen space:
                gl.MatrixMode(GL_MODELVIEW);
                gl.PushMatrix();
                gl.LoadIdentity();
                gl.MatrixMode(GL_PROJECTION);
                gl.PushMatrix();
                gl.LoadIdentity();
                gl.Ortho(0, m_pPane->GetViewport().Width(), 
                         0, m_pPane->GetViewport().Height(), -1.0f, 1.0f);

                // Center of the tooltip in screen (pixel) coordinates:
                CVect2<float> tip_center(np.m_TipCenter.X(), np.m_TipCenter.Y());
                CVect2<float> node_pos(x, y);
                TVPPoint tvp = m_pPane->Project(x, y);
                // Node position in screen (pixel) coordinates
                CVect2<float> node_proj_pos(tvp.X(), tvp.Y());
                CVect2<float> proj_dir(node_proj_pos - tip_center);

                float proj_len = proj_dir.Length();
                proj_dir.Normalize();
                CVect2<float> proj_dir_perp(proj_dir.Y(), -proj_dir.X());

                // Arrowhead that points at node:
                CVect2<float> arrowhead_base(node_proj_pos.X() - proj_dir.X()*16.0f,
                                             node_proj_pos.Y() - proj_dir.Y()*16.0f);

                // Draw shaft of arrow (unless tip is so close no room for shaft)
                float shaft_len = proj_len - 16.0f;
                if (shaft_len > 0.0f) {
                    CVect2<float> dir_minus_arrowhead(proj_dir*shaft_len);

                    CVect2<float>  q1 = tip_center + proj_dir_perp*3.0f;
                    CVect2<float>  q2 = q1 + dir_minus_arrowhead;
                    CVect2<float>  q3 = q2 - proj_dir_perp*6.0f;
                    CVect2<float>  q4 = q3 - dir_minus_arrowhead;
               
                    gl.Begin(GL_TRIANGLE_STRIP);
                        gl.Vertex2fv(q1.GetData());
                        gl.Vertex2fv(q2.GetData());
                        gl.Vertex2fv(q4.GetData());
                        gl.Vertex2fv(q3.GetData());
                    gl.End();
                }

                // Draw arrowhead
                CVect2<float>  t1(arrowhead_base + proj_dir_perp*8.0f);
                CVect2<float>  t2(node_proj_pos);             
                CVect2<float>  t3(arrowhead_base - proj_dir_perp*8.0f);
                
                gl.Begin(GL_TRIANGLES);
                    gl.Vertex2fv(t1.GetData());
                    gl.Vertex2fv(t2.GetData());
                    gl.Vertex2fv(t3.GetData());
                gl.End();

                gl.PopMatrix();
                gl.MatrixMode(GL_MODELVIEW);
                gl.PopMatrix();

                gl.Disable(GL_BLEND);
            }            
        }
    }

    // If a temporary arrow timed out, erase it here:
    if (erase_iter !=  m_NodePointers.end())
        m_NodePointers.erase(erase_iter);
}

void IPhyloTreeRender::x_RenderNodeHighlight(CGlPane& pane, 
                                             CPhyloTree::TTreeIdx node_idx,
                                             const CRgbaColor& c)
{
    // If mouse is currently over tooltip for this node...    
    if (node_idx != NULL_TREE_IDX) {
        IRender& gl = GetGl();

        CPhyloTree::TNodeType& active_node = m_DS->GetTree()->GetNode(node_idx);

        float  s = (float)GetNodeSize(&active_node);
        float node_size_scaler = 1.0f;
        // Use smaller circle for collapsed nodes (which are triangles, not circles)
        if (!active_node.Expanded() && s > 2.0f ) {
            s /= 2.0f;
            node_size_scaler = 0.5f;
        }

        s += 4.0f;
        int num_lines = int(std::max(2.0f, GetDefaultNodeSize(&active_node)) * 10.0f);

        pane.OpenPixels();

        float r;
        CVect2<float> pos = active_node->GetNodePosEx(CVect3<float>(pane.GetScale().X(), pane.GetScale().Y(), 1.0f),
                                                      m_SL.GetPointer(), r, node_size_scaler);
        TVPPoint p = pane.Project(pos.X(), pos.Y());

        gl.MatrixMode(GL_MODELVIEW);
        gl.PushMatrix();
        gl.Translatef((float)p.X(), (float)p.Y(), 0.0);

        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        gl.Color4fv(c.GetColorArray());

        num_lines /= 2;
        float delta = 3.141592653589793238463f / num_lines;

        gl.Begin(GL_TRIANGLE_STRIP);
        gl.Vertex2f(1.0f, 0.0f);
        float angle = delta;
        for (int i = 0; i < num_lines - 1; i++) {
            float sina = sin(angle), cosa = cos(angle);
            gl.Vertex2f(s*cosa, s*sina);
            gl.Vertex2f(s*cosa, -s*sina);
            angle += delta;
        }
        gl.Vertex2f(-1.0f, 0.0f);
        gl.End();

        gl.PopMatrix();

        pane.Close();
    }
}



////////////////////////////////////////////////////////////////////////////////
/// event handlers

void IPhyloTreeRender::OnLeftDown(wxMouseEvent& event)
{   
    if (m_DS == NULL) {
        event.Skip();
        return; 
    }

    // Don't try to do any selection if current scaling (z key is down)
    if (m_pMouseZoomHandler->IsScaleMode())
        return;

    m_MoveDuringSelection = false;

    m_StartPoint  = event.GetPosition();
    m_DragPoint = m_StartPoint;

    float x  = (float)m_pPane->UnProjectX(event.GetX());
    float y  = (float)m_pPane->UnProjectY(m_pHost->HMGH_GetVPPosByY(event.GetY()));

    // set current node or, if mouse is not on a node, leave it alone.
    CPhyloTree::TTreeIdx n = m_DS->GetTree()->TestForNode(CVect2<float>(x, y), m_Label.IsVisible(this), m_RotatedLabels);
    bool current = (n != CPhyloTree::Null());
    if (current)
        m_DS->GetTree()->SetCurrentNode(n);

    // select point if we hit a node, otherwise select rect if we are in an extended selection mode
    // (!eSelectState).
    if (m_pMouseZoomHandler->GetPanMode() == CMouseZoomHandler::eLmouse) {
        if (current) {
            m_State = eSelPoint;
        }
        else if (CGUIEvent::wxGetSelectState(event) != CGUIEvent::eSelectState) {
            m_State = eSelRect;
            GetGenericHost()->GHH_CaptureMouse();
        }
        else {
            m_State = eIdle;
            event.Skip(); 
        }
    }
    else {
        if (current)
            m_State = eSelPoint;
        else 
            m_State = eSelRect;
    }

    if (m_State == eSelPoint)
        m_pHost->HMGH_OnRefresh();
}

void IPhyloTreeRender::OnRightDown(wxMouseEvent& event)
{ 
   if (m_DS == NULL) {
        event.Skip();
        return; 
    }

    m_State = eSelPoint;

    m_StartPoint  = event.GetPosition();
    m_DragPoint = m_StartPoint;
  
    float x  = (float)m_pPane->UnProjectX(event.GetX());
    float y  = (float)m_pPane->UnProjectY(m_pHost->HMGH_GetVPPosByY(event.GetY()));
    bool node_selected = m_DS->GetTree()->SetCurrentNode(
        CVect2<float>(x,y), m_Label.IsVisible(this), m_RotatedLabels, false);

    if (node_selected) {
        m_State = eSelPoint;
    }
    else {
        m_State = eIdle;
        m_RightClick.Set(x, y);
        BufferedRender(*m_pPane, *m_DS, false, true);
    }

    m_pHost->HMGH_OnRefresh();
    
     // Skip so that the context menu can come up as well. (Right Down on Mac)
    event.Skip();
}


void IPhyloTreeRender::OnLeftDblClick(wxMouseEvent& event)
{
   if (m_DS == NULL) {
        event.Skip();
        return; 
    }

    if (m_MouseSingleTimer.IsRunning()) {
        m_MouseSingleTimer.Stop();
    }

    // Don't try to do any selection if current scaling (z key is down)
    if (m_pMouseZoomHandler->IsScaleMode())
        return;

    float x  = (float)m_pPane->UnProjectX(event.GetX());
    float y  = (float)m_pPane->UnProjectY(m_pHost->HMGH_GetVPPosByY(event.GetY()));
    m_DS->GetTree()->SetCurrentNode(CVect2<float>(x,y), m_Label.IsVisible(this), m_RotatedLabels);

    CPhyloTree::TTreeIdx idx = m_DS->GetTree()->GetCurrentNodeIdx();
    if (idx != CPhyloTree::Null()) {
        CPhyloTree::TNodeType& current = m_DS->GetTree()->GetNode(idx);

        if ((*current).GetDisplayChildren() == CPhyloNodeData::eHideChildren) {
            if (current.CanExpandCollapse(CPhyloNodeData::eShowChildren)) {                
                m_pHost->FireEditEvent(eCmdNodeExpandCollapse);
            }
        }
        else if ((*current).GetDisplayChildren() == CPhyloNodeData::eShowChildren) {      
            if (current.CanExpandCollapse(CPhyloNodeData::eHideChildren)) {
                m_DS->SetCollapsedLabel(idx);
                m_pHost->FireEditEvent(eCmdNodeExpandCollapse);
            }        
        }
    }
}

void IPhyloTreeRender::OnMotion(wxMouseEvent& event)
{
    if(event.Dragging())    {
        m_MoveDuringSelection = true;

        if(m_State == eSelRect)   {
            wxPoint ms_pos = event.GetPosition();

            if(ms_pos != m_DragPoint)  {
                m_State = eSelRect;
                m_DragPoint = ms_pos;               
                m_pHost->HMGH_OnRefresh();
            }
        }
        else {
            event.Skip();
        }
    } 
    else {
        event.Skip();
    }

    m_LastPos = event.GetPosition();
}

void IPhyloTreeRender::OnTimer(wxTimerEvent& /* event */)
{
    if (m_NodePointers.size() == 0) {
        m_EffectsTimer.Stop();
        return;
    }

    m_pHost->HMGH_OnChanged();
}

void IPhyloTreeRender::OnLeftUp(wxMouseEvent& event)
{
    if (m_DS == NULL) {
        event.Skip();
        return; 
    }

    m_LastMouseEvent = event;

    if (m_MouseSingleTimer.IsRunning()) {
        return;
    }

    if (m_State == eIdle) {
        event.Skip();      
    }

    m_MouseSingleTimer.Start(100, true);
}

void IPhyloTreeRender::OnTimerLeftUp(wxTimerEvent& /* event */)
{
    // Do normal processing for left up. This gets cancelled when the
    // user doubleclicks instead

    // Don't try to do any selection if current scaling (z key is down)
    if (m_pMouseZoomHandler->IsScaleMode())
        return;

    float x  = (float)m_pPane->UnProjectX(m_LastMouseEvent.GetX());
    float y  = (float)m_pPane->UnProjectY(m_pHost->HMGH_GetVPPosByY(m_LastMouseEvent.GetY()));
    CVect2<float> pt(x,y);                  

    CGUIEvent::EGUIState state =
        CGUIEvent::wxGetSelectState(m_LastMouseEvent);

    // not doing incremental only selection
    //bool neg = (state == CGUIEvent::eSelectExtState);
    bool inc =  (state == CGUIEvent::eSelectIncState);
    bool sel =  (state == CGUIEvent::eSelectState);

    switch(m_State) {
    case eSelPoint:
    {
        CPhyloTree::TTreeIdx idx = m_DS->GetTree()->TestForNode(pt, m_Label.IsVisible(this), m_RotatedLabels);

        if (idx == m_DS->GetTree()->GetCurrentNodeIdx()) {
            if (m_DS->GetTree()->SelectByPoint(pt, sel, inc, m_Label.IsVisible(this), m_RotatedLabels)) {
                m_pHost->HMGH_OnChanged();
            }
        }

        m_State = eIdle;

        break;
    }
    case eSelRect: {       
        int dist = std::abs(m_StartPoint.x - m_DragPoint.x) +
                   std::abs(m_StartPoint.y - m_DragPoint.y); 
        bool sel_changed = false;
        
        // Don't select for tiny rectangles.  Allow de-selection on mouse up if
        // control key was down (if no key was down, we would be panning instead).
        if (dist > 4) {
            sel_changed = x_SelectByRect(inc );
        }
        else {
            if ((m_pMouseZoomHandler->GetPanMode() == CMouseZoomHandler::eLmouse && inc) ||
                (m_pMouseZoomHandler->GetPanMode() == CMouseZoomHandler::ePkey && sel)) {
                    m_DS->GetTree()->ClearSelection();
                    sel_changed = true;
            }
        }

        m_State = eIdle;

        GetGenericHost()->GHH_ReleaseMouse();

        if (sel_changed)
            m_pHost->HMGH_OnChanged();
        else
            m_pHost->HMGH_OnRefresh();

        break;
    }
    default:
        // clear selection if user left clicks without moving (panning) the mouse
        if ( !m_MoveDuringSelection && 
             m_pMouseZoomHandler->GetPanMode() == CMouseZoomHandler::eLmouse && 
             sel) {
                m_DS->GetTree()->ClearSelection();
                m_pHost->HMGH_OnChanged();
            }
    }
}

void IPhyloTreeRender::x_OnSelectCursor(void)
{
    switch(m_State)    {
    case eIdle:
    case eSelPoint:
        m_CursorId = wxCURSOR_ARROW;
        break;
    case eSelRect:
        m_CursorId = wxCURSOR_CROSS;
        break;
    default:
        break;
    }
    GetGenericHost()->GHH_SetCursor(wxCursor(m_CursorId));
}

IGenericHandlerHost* IPhyloTreeRender::GetGenericHost()
{
    return dynamic_cast<IGenericHandlerHost*>(m_pHost);
}


/*************************************************************************************/


int IPhyloTreeRender::x_OnMouseMove(void)
{
    return (m_State == eIdle) ? 0 : 1;
}

int IPhyloTreeRender::x_OnKeyDown(void)
{
    return (m_State == eIdle) ? 0 : 1;
}

int IPhyloTreeRender::x_OnKeyUp(void)
{
    return (m_State == eIdle) ? 0 : 1;
}


bool IPhyloTreeRender::x_GetEdgeDist(const CVect2<float>& from, const CVect2<float>& to)
{
    // Given a previous right-click, calculate distance of click-point from
    // edge in model coordinates
    CVect2<float> dir = to - from;
    float len = dir.Length();
    dir /= len;
    float proj = (m_RightClick - from).Dot(dir);
    
    float dist;
    CVect2<float> delta;
    if (proj < 0.0f) {
        delta = m_RightClick - from;    
    }
    else if (proj > len) {
        delta = m_RightClick - to;     
    }
    else {
        delta = m_RightClick - (from + dir*proj);      
    }
    dist = delta.Length2();

    if (dist < m_ClosestEdgeDistSq) {
        m_ClosestEdgeDistSq = dist;
        m_ClosestEdgeDir = delta;
        return true;
    }
    return false;
}

void IPhyloTreeRender::x_RenderLineVbo(TTreeIdx child_node_idx,
                                       CPhyloTreeNode* child_node,
                                       CPhyloTreeNode* parent_node, 
                                       vector<CVect2<float> >& line_coords,
                                       vector<CVect4<unsigned char> >& line_colors,
                                       double x1,
                                       double y1,
                                       double x2,
                                       double y2)
{
    CRgbaColor c;
    CRgbaColor parent_color;
    GetEdgeColor(child_node, parent_node, c, parent_color);

    // parent color supports gradient coloring - caller decides whether to enable that or not.
    line_colors.push_back(parent_color.GetRgbaUC());
    line_colors.push_back(c.GetRgbaUC());
    CVect2<float> p1((float)x1, (float)y1);
    CVect2<float> p2((float)x2, (float)y2);
    line_coords.push_back(p1);
    line_coords.push_back(p2);

    if (m_SelEdge && x_GetEdgeDist(p1,p2)) {
        m_SelectedEdge.clear();
        m_SelectedEdge.push_back(p1);
        m_SelectedEdge.push_back(p2);
        m_DS->GetTree()->SetCurrentEdge(child_node_idx, child_node->GetParent());
    }
}

void IPhyloTreeRender::x_RenderLineVbo(TTreeIdx child_node_idx,
                                       CPhyloTreeNode* child_node,
                                       CPhyloTreeNode* parent_node,
                                       vector<CVect2<float> >& line_coords,
                                       vector<CVect4<unsigned char> >& line_colors,
                                       vector<CVect2<float> >& sel_line_coords,
                                       vector<CVect4<unsigned char> >& sel_line_colors,
                                       double x1,
                                       double y1,
                                       double x2,
                                       double y2)
{
    CRgbaColor c;
    CRgbaColor parent_color;

    bool selected_edge = GetEdgeColor(child_node, parent_node, c, parent_color);
    
    CVect2<float> p1((float)x1, (float)y1);
    CVect2<float> p2((float)x2, (float)y2);

    // parent color supports gradient coloring - caller decides whether to enable that or not.
    if (!selected_edge) {
        line_colors.push_back(parent_color.GetRgbaUC());
        line_colors.push_back(c.GetRgbaUC());
        line_coords.push_back(p1);
        line_coords.push_back(p2);
    }
    else {
        sel_line_colors.push_back(parent_color.GetRgbaUC());
        sel_line_colors.push_back(c.GetRgbaUC());
        sel_line_coords.push_back(p1);
        sel_line_coords.push_back(p2);
    }

    if (m_SelEdge && x_GetEdgeDist(p1, p2)) {
        m_SelectedEdge.clear();
        m_SelectedEdge.push_back(p1);
        m_SelectedEdge.push_back(p2);
        m_DS->GetTree()->SetCurrentEdge(child_node_idx, child_node->GetParent());
    }
}

void IPhyloTreeRender::x_RenderSplineVbo(TTreeIdx child_node_idx,
                                         CPhyloTreeNode* child_node,
                                         CPhyloTreeNode* parent_node,
                                         vector<CVect2<float> >& edge_node_coords,
                                         vector<CVect4<unsigned char> >& edge_node_colors,
                                         vector<CVect2<float> >& sel_edge_node_coords,
                                         vector<CVect4<unsigned char> >& sel_edge_node_colors,
                                         const CVect3<float>& pt1,
                                         const CVect3<float>& pt2,
                                         const CVect3<float>& pt3,
                                         const CVect3<float>& pt4)
{       
    CRgbaColor c;
    CRgbaColor parent_color;                        
    bool selected_edge = GetEdgeColor(child_node, parent_node, c, parent_color);

    // Add edges that are part of this spline to the vector for selected or non-selected
    // edges as approrpriate. Selected edges are drawn after non-selected edges.
    vector<CVect2<float> >& tree_edge_vec = selected_edge ? sel_edge_node_coords : edge_node_coords;
    vector<CVect4<unsigned char> >& colors = selected_edge ? sel_edge_node_colors : edge_node_colors;

    m_Curve.SetPoint(0, pt1);
    m_Curve.SetPoint(1, pt2);
    m_Curve.SetPoint(2, pt3);
    m_Curve.SetPoint(3, pt4);
    m_Curve.Recalc();

    vector<CVect3<float> > buf;
    float len = m_Curve.DrawBuffered(buf, 0.002f, 8);
    float cur_len = 0.0f;
    CRgbaColor col = parent_color;
    bool closest = false;

    size_t i;
    for (i=0; i<buf.size()-1; ++i) {
        colors.push_back(col.GetRgbaUC());

        cur_len += (buf[i+1]-buf[i]).Length();
        float pct_copied = cur_len/len;
        col = parent_color*(1.0f-pct_copied) + c*pct_copied;

        colors.push_back(col.GetRgbaUC());
        CVect2<float> p1(buf[i].X(), buf[i].Y());
        CVect2<float> p2(buf[i + 1].X(), buf[i + 1].Y());

        tree_edge_vec.push_back(p1);
        tree_edge_vec.push_back(p2);

        if (m_SelEdge && x_GetEdgeDist(p1, p2)) {
            closest = true;
        }
    }

    if (closest) {
        m_SelectedEdge.clear();
        std::copy(tree_edge_vec.end() - (buf.size() - 1) * 2, tree_edge_vec.end(), std::back_inserter(m_SelectedEdge));
        m_DS->GetTree()->SetCurrentEdge(child_node_idx, child_node->GetParent());
    }
}

void IPhyloTreeRender::x_RenderCircularVbo(TTreeIdx child_node_idx,
                                           CPhyloTreeNode* child_node,
                                           CPhyloTreeNode* parent_node,
                                           vector<CVect2<float> >* trace_line_coords,
                                           vector<CVect4<unsigned char> >* trace_line_colors,
                                           vector<CVect2<float> >* trace_point_coords,
                                           vector<CVect4<unsigned char> >* trace_point_colors,
                                           vector<CVect2<float> >& edge_node_coords,
                                           vector<CVect4<unsigned char> >& edge_node_colors,
                                           vector<CVect2<float> >& sel_edge_node_coords,
                                           vector<CVect4<unsigned char> >& sel_edge_node_colors,
                                           float radius,
                                           const CVect2<float>& center,
                                           const CVect2<float>& offset,
                                           float resolution)
{   
    CRgbaColor c;
    CRgbaColor parent_color;                      
    bool selected_edge = GetEdgeColor(child_node, parent_node, c, parent_color);

    // Add edges that are part of this spline to the vector for selected or non-selected
    // edges as approrpriate. Selected edges are drawn after non-selected edges.
    vector<CVect2<float> >& tree_edge_vec = selected_edge ? sel_edge_node_coords : edge_node_coords;
    vector<CVect4<unsigned char> >& colors = selected_edge ? sel_edge_node_colors : edge_node_colors;

    vector<CVect2<float> > buf;

    float angle1 = (*parent_node)->GetAngle();
    float angle2 = (*child_node)->GetAngle();

    // resolution is larger for larger trees (since you zoom in more)
    int steps = std::abs((int)floorf((angle1-angle2)*resolution));
    // Need at least a couple arc-points to make an edge to the second node
    if (steps < 2 || radius == 0.0f)
        steps = 2;
    float tdelta = -1.0f*(angle1-angle2)/(float)steps;
    float a = angle1;

    for (int i=0; i<=steps; ++i) {            
        CVect2<float> p(radius*cosf(a), radius*sinf(a));
        p += center;
        buf.push_back(p);
        a += tdelta;
    }

    bool closest = false;

    CVect2<float> pt2 = (*child_node)->XY();
    if (trace_line_coords != NULL && offset.Length2() > 0.0f) {
        buf.push_back(pt2 - offset);
        CVect4<unsigned char> parent_color_uc = parent_color.GetRgbaUC();

        size_t i;
        for (i = 0; i < buf.size() - 1; ++i) {
            colors.push_back(parent_color_uc);
            if (i == buf.size() - 2)
                colors.push_back(c.GetRgbaUC());
            else
                colors.push_back(parent_color_uc);

            CVect2<float> p1(buf[i].X(), buf[i].Y());
            CVect2<float> p2(buf[i + 1].X(), buf[i + 1].Y());

            tree_edge_vec.push_back(p1);
            tree_edge_vec.push_back(p2);

            if (m_SelEdge && x_GetEdgeDist(p1, p2)) {
                closest = true;
            }
        }

        // If it's a distance tree and this is a leaf node, we add a light grey line to the 
        // trees outer perimeter and put the node there. The lines are in trace_lines, and
        // trace_points holds points that cap off the end of the line to which the narrow
        // grey line attaches
        CRgbaColor trace_color = c;
        trace_color.Lighten(0.7f);
        CVect4<unsigned char> trace_color_uc = trace_color.GetRgbaUC();
        trace_line_colors->push_back(trace_color_uc);
        trace_line_colors->push_back(trace_color_uc);
        trace_line_coords->push_back(pt2 - offset);
        trace_line_coords->push_back(pt2);

        if (m_SelEdge) {
            // need to call x_GetEdgeDist for every edge to compute closest
            // edge accurately
            closest = x_GetEdgeDist(pt2 - offset, pt2) || closest;

            if (closest) {
                m_SelectedEdge.clear();
                std::copy(tree_edge_vec.end() - (buf.size() - 1) * 2, tree_edge_vec.end(), std::back_inserter(m_SelectedEdge));
                m_SelectedEdge.push_back(pt2 - offset);
                m_SelectedEdge.push_back(pt2);
                m_DS->GetTree()->SetCurrentEdge(child_node_idx, child_node->GetParent());
            }
        }

        trace_point_coords->push_back(pt2 - offset);
        trace_point_colors->push_back(parent_color_uc);
    }
    else {
        buf.push_back(pt2);
        CVect4<unsigned char> parent_color_uc = parent_color.GetRgbaUC();

        size_t i;
        for (i = 0; i < buf.size() - 1; ++i) {
            colors.push_back(parent_color_uc);
            if (i == buf.size() - 2)
                colors.push_back(c.GetRgbaUC());
            else
                colors.push_back(parent_color_uc);

            CVect2<float> p1(buf[i].X(), buf[i].Y());
            CVect2<float> p2(buf[i + 1].X(), buf[i + 1].Y());

            tree_edge_vec.push_back(p1);
            tree_edge_vec.push_back(p2);

            if (m_SelEdge && x_GetEdgeDist(p1, p2)) {
                closest = true;
            }
        }

        if (closest) {
            m_SelectedEdge.clear();
            std::copy(tree_edge_vec.end() - (buf.size() - 1) * 2, tree_edge_vec.end(), std::back_inserter(m_SelectedEdge));
            m_DS->GetTree()->SetCurrentEdge(child_node_idx, child_node->GetParent());
        }
    }
}

void  IPhyloTreeRender::x_RenderPseudoSplineVbo(TTreeIdx child_node_idx,
                                                CPhyloTreeNode* child_node,
                                                CPhyloTreeNode* parent_node,
                                                vector<CVect2<float> >& edge_node_coords,
                                                vector<CVect4<unsigned char> >& edge_node_colors,
                                                vector<CVect2<float> >& sel_edge_node_coords,
                                                vector<CVect4<unsigned char> >& sel_edge_node_colors,
                                                const CVect2<float>&  pt1,
                                                const CVect2<float>&  mid_point,                                                
                                                const CVect2<float>&  pt2)
{
    CRgbaColor c;
    CRgbaColor parent_color;
    bool selected_edge = GetEdgeColor(child_node, parent_node, c, parent_color);

    CVect4<unsigned char> parent_color_uc = parent_color.GetRgbaUC();

    // parent color supports gradient coloring - For simplicity in this view,
    // only apply gradient to horizontal component of edge.
    if (!selected_edge) {
        edge_node_colors.push_back(parent_color_uc);
        edge_node_colors.push_back(parent_color_uc);
        edge_node_colors.push_back(parent_color_uc);
        edge_node_colors.push_back(c.GetRgbaUC());

        edge_node_coords.push_back(pt1);
        edge_node_coords.push_back(mid_point);
        edge_node_coords.push_back(mid_point);
        edge_node_coords.push_back(pt2);
    }
    else {
        sel_edge_node_colors.push_back(parent_color_uc);
        sel_edge_node_colors.push_back(parent_color_uc);
        sel_edge_node_colors.push_back(parent_color_uc);
        sel_edge_node_colors.push_back(c.GetRgbaUC());

        sel_edge_node_coords.push_back(pt1);
        sel_edge_node_coords.push_back(mid_point);
        sel_edge_node_coords.push_back(mid_point);
        sel_edge_node_coords.push_back(pt2);
    }

    // Have to call for each edge:

    if (m_SelEdge && 
        (x_GetEdgeDist(pt1, mid_point) || x_GetEdgeDist(mid_point, pt2))) {
        m_SelectedEdge.clear();
        m_SelectedEdge.push_back(pt1);
        m_SelectedEdge.push_back(mid_point);
        m_SelectedEdge.push_back(mid_point);
        m_SelectedEdge.push_back(pt2);
        m_DS->GetTree()->SetCurrentEdge(child_node_idx, child_node->GetParent());
    }
}

static void s_GenerateCollapsedNode(vector<CVect2<float> >& vertices, size_t circleSegments)
{
    vertices.push_back(CVect2<float>(1.0f, -0.5f));
    vertices.push_back(CVect2<float>(1.0f,  0.5f));

    float cap_end = 100.0f / 512.0f;
    float a = 3.141592653589793238463
        - acosf(cap_end / sqrtf((1.0f - cap_end)*(1.0f - cap_end) + 0.5f * 0.5f))
        - atan2(0.5f, 1.0f - cap_end);

    size_t steps = max((size_t)1, circleSegments / 2);
    float delta = a / steps;

    for (size_t i = 0; i < steps; ++i) {
        float cosa = cosf(a);
        float sina = sinf(a);
        vertices.push_back(CVect2<float>(cap_end*(1 - cosa), -cap_end * sina));
        vertices.push_back(CVect2<float>(cap_end*(1 - cosa), cap_end*sina));
        a -= delta;
    }

    vertices.push_back(CVect2<float>(0.0f, 0.0f));
}

void IPhyloTreeRender::x_AddCollapsedGeomNode(CPhyloTreeNode* node,
                                              const CVect2<float> delta,
                                              CRgbaColor color,
                                              float alpha,
                                              size_t circleSegments,
                                              bool skip_opengl)
{
    CMatrix4<float> m;
    float angle = 0.0f;
    float node_scale;
    string geomName = "collapsed" + NStr::NumericToString(circleSegments);

    CIRef<IVboGeom> geom = m_DS->GetModel().FindGeomObject(geomName);
    if (geom.IsNull()) {
        geom = m_DS->GetModel().AddGeomObject(GL_TRIANGLE_STRIP, geomName);

        vector<CVect2<float> > vb;
        s_GenerateCollapsedNode(vb, circleSegments);
        geom->SetVertexBuffer2D(vb);
    }

    // Since PDF has anti-aliasing built-in, we skip it for all but the most opaque node
    CGlVboNode* n = m_DS->GetModel().AddTempGeomNode("CollapsedNode", true);
    if (skip_opengl) {   
        n->SkipTarget(eOpenGL20, true);
        n->SkipTarget(eOpenGL11, true);
    }
    n->SetVBOGeom(geom);

    // Set rotation for this glyph in the node. It will be applied
    // in a scale dependent manner (angle changes appropriately when
    // view is expanded/compressed in x/y)
    if (GetRotatedLabels() || GetDescription() == "Circular Cladogram")
        angle = node->GetValue().GetAngle();
    n->SetRotationZ(angle);

    // GetDefaultNodeSize() returns a radius (that's how circular
    // nodes are drawn) so we multiply by 2 here:
    node_scale = 2.0f*GetDefaultNodeSize(node);

    // In trees without distance this will be equal to node_scale (could also consider using
    // tree depth as a proxy for distance)
    float width_scale = node->GetValue().GetNodeWidthScaler(m_SL.GetPointer());

    m.Identity();
    m(0, 0) = width_scale;
    m(1, 1) = node_scale;
    m(2, 2) = node_scale;
    m(0, 3) = (**node).X() - delta.X();
    m(1, 3) = (**node).Y() - delta.Y();
    n->SetPosition(m);
    // node needs to know offset for selection and selection highlights
    node->GetValue().SetNodeOffset(delta);

    // Need to multiply pixel offset by inverse of any scale factor in
    // position matrix.
    if (!SupportsRotatedLabels()) {
        n->SetPixelOffset(CVect2<float>(-2.0f / width_scale, 0.0f));
    }
    else {
        n->SetPixelOffset(CVect2<float>(-4.0f / width_scale, 0.0f));
    }
    // tree node needs offset to to assist with label placement
    (*node)->SetNodePixelOffset(n->GetPixelOffset());

    n->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    n->GetState().Enable(GL_BLEND);
    n->GetState().Disable(GL_TEXTURE_2D);
    n->GetState().Disable(GL_TEXTURE_1D);   
    n->GetState().PdfShadeStyle(CGlState::eFlat);
    color.SetAlpha(alpha);
    n->GetState().ColorC(color);
    n->GetState().ScaleInvarient(true);
}


bool IPhyloTreeRender::x_AddCollapsedTextureNode(CPhyloTreeNode* node,
                                                 const CVect2<float> delta,
                                                 CRgbaColor color,
                                                 float alpha)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    vector<CVect2<float> > vb;
    s_GenerateCollapsedNode(vb, 24);

    CGlVboNode* n = m_DS->GetModel().AddTempCollapsedNode(vb);

    if (n == NULL) {
        glPopAttrib();
        CGlUtils::CheckGlError();
        return false;
    }

    CMatrix4<float> m;
    float angle = 0.0f;
    float node_scale;

    // Set rotation for this glyph in the node. It will be applied
    // in a scale dependent manner (angle changes appropriately when
    // view is expanded/compressed in x/y)
    if (GetRotatedLabels() || GetDescription() == "Circular Cladogram") {
        angle = node->GetValue().GetAngle();
    }
    else if (SupportsRotatedLabels()) {
        // switch between rotation of 0 or 180: (node facing right or left)
        if (cosf((**node).GetAngle()) < 0.0f)
            angle = (float)M_PI;
    }
    n->SetRotationZ(angle);

    // GetDefaultNodeSize() really returns a radius (that's how circular
    // nodes are drawn) so we multiply by 2 here:
    node_scale = 2.0f*GetDefaultNodeSize(node);

    float width_scale = node->GetValue().GetNodeWidthScaler(m_SL.GetPointer());
    // If tree did not have distance values
    if (width_scale == 0.0f)
        width_scale = node_scale;

    m.Identity();
    m(0, 0) = width_scale;
    m(1, 1) = node_scale;
    m(2, 2) = node_scale;
    m(0, 3) = (**node).X() - delta.X();
    m(1, 3) = (**node).Y() - delta.Y();
    n->SetPosition(m);
    // node needs to know offset for selection and selection highlights
    node->GetValue().SetNodeOffset(delta);

    // Need to multiply pixel offset by inverse of any scale factor in
    // position matrix.
    if (!SupportsRotatedLabels()) {
        n->SetPixelOffset(CVect2<float>(-2.0f / width_scale, 0.0f));
    }
    else {
        n->SetPixelOffset(CVect2<float>(-4.0f / width_scale, 0.0f));
    }
    // tree node needs offset to to assist with label placement
    (*node)->SetNodePixelOffset(n->GetPixelOffset());

    n->GetState().Color4f(color.GetRed(), color.GetGreen(), color.GetBlue(), alpha);
    n->GetState().ScaleInvarient(true);

    glPopAttrib();
    CGlUtils::CheckGlError();
    return true;
}

void IPhyloTreeRender::x_RenderNodeVbo(CPhyloTreeNode* node,
                                       TTreeIdx node_idx, 
                                       vector<float>& color_coords,
                                       const CVect2<float> delta)
{
    // colors
    float node_alpha = SetNodeColoration(node, node_idx);
    short color_idx = GetNodeColorIdx(node);

    float def_node_size = (float)GetDefaultNodeSize(node);
    float this_node_size = def_node_size;

    // will give good circle - size * approx(2pi)
    int num_lines = int(std::max(2.0f, def_node_size) * 10.0f);
       
    bool collapsed_node = (**node).GetDisplayChildren() == CPhyloNodeData::eHideChildren;
    (*node)->SetNodePixelOffset(CVect2<float>(0.0f, 0.0f));

    if (collapsed_node) {
        CRgbaColor color = m_DS->GetModel().GetColorTable()->GetColor((int) color_idx);   

        switch (GetGl().GetApi()) {
        case eRenderPDF :
        case eRenderSVG:
            x_AddCollapsedGeomNode(node, delta, color, node_alpha, 12, true);
            break;
        case eMetal :
        case eApiUndefined :
            x_AddCollapsedGeomNode(node, delta, color, node_alpha, 2, false);
            break;
        case eOpenGL20 :
        case eOpenGL11 :
        case eRenderDebug :
            if (!x_AddCollapsedTextureNode(node, delta, color, node_alpha))
                x_AddCollapsedGeomNode(node, delta, color, node_alpha, 2, false);
            break;
        }
    }
    else {
        node->GetValue().SetNodeOffset(CVect2<float>(0.0f, 0.0f));
        //Use triangle fan for efficiency
        CTreeTriFanNode* node_fan = m_DS->GetModel().GetTriFanNode();

        if (node_fan->GetVertexCount() == 0) {
            node_fan->CreateVbo(num_lines);
            node_fan->ClearPositions(); 
            node_fan->GetState().Disable(GL_BLEND);
            node_fan->GetState().ScaleInvarient(true);
        }

        // This is only used for color when rendering nodes as Points
        float color_coord = m_DS->GetModel().GetColorTable()->GetColorTexCoord(color_idx, node->IsLeafEx() ?  node_alpha : 0.0f);
        color_coords.push_back(color_coord);
    }

    // Add any node markers. These get updated each time new vbos are created,
    // which is a bit inefficient, since the underlying geometry does not change
    // unless the specific node marker is edited (which is rare)
    if ((**node).HasNodeMarker()) {
        int num_lines = 32;

        // Use triangles instead of triangle fan because each triangle in circle
        // can have a separate color from the one next to it - therefore they
        // can't share the center node
        CGlVboNode* render_node = m_DS->GetModel().AddTempGeomNode(GL_TRIANGLES,
                                                                   "MarkerNode", 
                                                                   true);
        CMatrix4<float> m;
        m.Identity();             
        m(0,3) = (**node).X();
        m(1,3) = (**node).Y(); 
        render_node->SetPosition(m);

        render_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        render_node->GetState().Enable(GL_BLEND);        
        render_node->GetState().Disable(GL_TEXTURE_2D);
        render_node->GetState().Disable(GL_TEXTURE_1D);
        render_node->GetState().ScaleInvarient(true);

        vector<CVect2<float> > vb;
        vector<CRgbaColor> cb;

        float target_alpha = 1.0f;
        if (m_SL->GetSelectionVisibility()==CPhyloTreeScheme::eHighlightSelection &&
            (**node).GetSelectedState()==CPhyloNodeData::eNotSelected) {
                target_alpha = m_SL->GetNonSelectedAlpha();
        }

        size_t color_count = (**node).GetMarkerColors().size();
        float color_countf = (float)color_count;

        // Don't want to tie markers to leaf node sizes which can be 0 or to sizes
        // of collapsed nodes which grow with # of children
        float s2 = m_SL->GetNodeSize();
        if ((**node).GetNodeSize() > 0)
            s2 = (**node).GetNodeSize();

        s2 *= (**node).GetMarkerSize();
         
        float delta = 6.28f/(float)num_lines;
        float angle = 3.14159f/2.0f;
        for(int i=0; i<num_lines; i++){
            size_t color_idx = (size_t)((((float)i)/(float)(num_lines)) * color_countf);
            color_idx = std::min(color_idx, color_count-1);
            CRgbaColor c = (**node).GetMarkerColors()[color_idx];
            c.SetAlpha(target_alpha);           
            cb.push_back(c);

            vb.push_back(CVect2<float>(0.0f, 0.0f));          
            cb.push_back(c);
            vb.push_back(CVect2<float>(s2*cosf(angle), 
                s2*sinf(angle)));

            cb.push_back(c);
            vb.push_back(CVect2<float>(s2*cosf(angle+delta), 
                s2*sinf(angle+delta)));

            angle += delta;
        }

        render_node->SetVertexBuffer2D(vb);
        render_node->SetColorBuffer(cb);
    }

    if ((**node).GetBoundedDisplay()) {        
        m_DS->GetModel().GetBoundaryNode()->GetNodes().push_back(node_idx);
    }

    // rendering labels
    short label_color_idx = GetLabelColorIdx(node);
   
    node->GetValue().SetVisible(true);

    node->GetValue().SetNodeDisplaySize((float)this_node_size);

    // Currently using this for actual node display color.  
    // Since we are using destination alpha to simplify drawing, we
    // lighten the color rather than using alpha
    CRgbaColor c =  m_DS->GetModel().GetColorTable()->GetColor(color_idx);
    c.Lighten(1.0f - node_alpha);
    node->GetValue().SetNodeDisplayColor(c);

    c =  m_DS->GetModel().GetColorTable()->GetColor(label_color_idx);
    c.SetAlpha(node_alpha);
    node->GetValue().SetTextDisplayColor(c);

    node->GetValue().SetAlreadyDrawn(false);    

    m_DS->GetModel().GetNodeIndices().push_back(node_idx);
}

void IPhyloTreeRender::InitLabel(CPhyloTree* tree,
                                 CPhyloTree::TNodeType& node)
{
    string label = "";

    if ((m_SL->GetLabelVisibility() == CPhyloTreeScheme::eLabelsVisible) ||
        ((m_SL->GetLabelVisibility() == CPhyloTreeScheme::eLabelsForLeavesOnly) &&
        node.IsLeafEx())) {

        CVect2<float> po;
        CVect2<float> pixel_size;
        label = m_Label.GetLabel(*tree, node);

        // for long strings, GetNodeLabelOffset() is expensive, so avoid it if we don't need it.
        bool recompute = (label != (*node).GetDisplayLabel());

        if (recompute) {
            pixel_size = m_Label.GetNodeLabelOffset(label, &node, this, po);
            CGlRect<float> r(po.X(), po.Y(), po.X() + pixel_size.X(), po.Y() + pixel_size.Y());

            (*node).SetLabelRect(r);
            (*node).SetDisplayLabel(label);
        }
        else {
            CGlRect<float> r = (*node).GetLabelRect();
            pixel_size = m_Label.GetNodeLabelOffset(label, &node, this, 
                CVect2<float>(r.Width(), r.Height()), po);
            r.Init(po.X(), po.Y(), po.X() + pixel_size.X(), po.Y() + pixel_size.Y());

            (*node).SetLabelRect(r);
            (*node).SetDisplayLabel(label);
        }
    }
    else {
        (*node).SetDisplayLabel("");
        (*node).SetLabelRect(CGlRect<float>(0.0f, 0.0f, 0.0f, 0.0f));
    }
}

void IPhyloTreeRender::x_ComputeNodeBoundary(CPhyloTree* tree,
                                             CPhyloTree::TNodeType& node,
                                             CBoundaryPoints& boundary_pts,
                                             const string& layout_type)
{
    CVect2<float>  node_pos((*node).XY());

    boundary_pts.AddBoundedPoint(node_pos);

    // Get pixel rectangle for label
    if (!(*node).GetLabel().empty()) {
        CVect2<float> ll, ur;
        CGlRect<float> r = (*node).GetLabelRect(
            CVect2<float>(float(m_pPane->GetScaleX()), float(m_pPane->GetScaleY())), m_RotatedLabels);

        // This is imperfect for rotated labels since their extents vary depending on changes in viewing
        // scale (e.g. changing the viewport).
        boundary_pts.AddPixelRect(node_pos, r);
    }

    if ((*node).GetBoundedDisplay() == CPhyloNodeData::eBounded) {
        CVect2<float> scale(m_pPane->GetScaleX(), m_pPane->GetScaleY());
        (*node).GetSubtreeBoundary()->ComputeShapes(
            boundary_pts, scale, node_pos, layout_type);
    }
}

void IPhyloTreeRender:: x_DrawBoundingAreas(CPhyloTreeNode * node)
{
}

bool IPhyloTreeRender::x_SelectByRect(bool toggle)
{
    float x1 = (float)
        m_pPane->UnProjectX(m_StartPoint.x);
    float y1 = (float)
        m_pPane->UnProjectY(m_pHost->HMGH_GetVPPosByY(m_StartPoint.y));
    float x2 = (float)
        m_pPane->UnProjectX(m_DragPoint.x);
    float y2 = (float)
        m_pPane->UnProjectY(m_pHost->HMGH_GetVPPosByY(m_DragPoint.y));

    if (m_DS == NULL)
        return false;

    if (x1 > x2) swap(x1, x2);
    if (y1 > y2) swap(y1, y2);

    CVect2<float> ll(x1, y1);
    CVect2<float> ur(x2, y2);

    return m_DS->GetTree()->SelectByRect(ll, ur, toggle);
}
    

void IPhyloTreeRender::UpdateDataSource(CPhyloTreeDataSource& ds, CGlPane& p)
{
    m_pPane = &p;
    m_DS = &ds;
}

void IPhyloTreeRender::Layout(CPhyloTreeDataSource& ds, CGlPane& p)
{
    CStopWatch timer;
    timer.Start();

    m_pPane = &p;
    m_DS = &ds;
    x_Layout(ds);

    m_LayoutTime = timer.Elapsed();

    // Mark the fact that we have a valid layout, which Render function can check.
    // Only really need this for first time, since it may not be possible to do
    // layout prior to first call to Render().
    m_ValidLayout = eValidLayout;

    // New layout establised. (flag is for swtiching between types of layout,
    // not updates to current layout like hide/show node.)
    m_SwitchedLayout = false;
}

///
/// Compute viewing area limits that will best get the entire tree to
/// fit within the boundaries of the viewport currently defined in m_pPane.
/// The limits are returned in 'limits'
/// The node_extents and text_extents return the pixel coordinates to which
/// the node (resizable) and text (non-resizable) parts of the tree project.
/// The text_extents are always the same size or larger than the node_extents.
/// These two extents are useful when iterations is 1 which gives us a viewport
/// size instead of the viewing area limits (limits) which takes multiple 
/// iterations.
CVect2<float>  IPhyloTreeRender::x_FindBestSize(CGlPane& pane,
                                      std::vector<ProjectionElement>& xexts,
                                      std::vector<ProjectionElement>& yexts,
                                      TModelRect& limits,
                                      TModelRect& node_extents,
                                      TModelRect& text_extents,
                                      int max_iterations)
{
    float goal = ((float)(limits.Right()-limits.Left()))/1e04f;
    goal = std::min(goal, ((float)(limits.Top()-limits.Bottom()))/1e04f);
    //goal = 0.1f;
    float delta = 1e10f;
    float prev_delta = 1e10f;
    //float pct_non_label = 1.0f;

    float viewport_x, viewport_y;
    float vp_width = (float)pane.GetViewport().Width();
    float vp_height = (float)pane.GetViewport().Height();
    float left = limits.Left();
    float right = limits.Right();
    float top = limits.Top();
    float bottom = limits.Bottom();

    if (left==right) {
        left -= 1.0f;
        right += 1.0f;
    }
    if (top==bottom) {
        bottom -= 1.0f;
        top += 1.0f;
    }

    int count = 0;
    CVect2<float> vp_extent(0.0f, 0.0f);
 
    while (delta > goal && count++ < max_iterations /*&& pct_non_label>0.3f*/) {
        prev_delta = delta;
        // Given current limits (which are used to create the orthographic projection),
        // find the max/min x and y viewport positions to which the labels will project.
        // This allows us to iteratively search for the optimum limits for which the 
        // tree will exactly occupy the given viewport.
        float viewport_maxx = -1e10f; 
        float viewport_minx = 1e10f;
        float viewport_maxy = -1e10f;
        float viewport_miny = 1e10f;

        // Viewport projection limits for the nodes of the tree given the 
        // the current right/left/bottom/top for the visible rect
        float node_maxx = -1e10f; 
        float node_minx = 1e10f;
        float node_maxy = -1e10f; 
        float node_miny = 1e10f;

        // We set the limits of the viewing area differently based on the zoom
        // behavior.  We want to make sure that when the user zooms  in
        // to the point labels appear, that there is room for the full label
        // within the viewing limits.  This means that if we are zooming in y
        // only, there must be space for the full labels at max. zoom level,
        // whereas for zooming in x or proportionally, we don't need the full
        // label space until we have zoomed in to the point they are visible.
        float label_zoom = 1.0f;

        if (m_SL->GetZoomBehavior(GetDescription()) == CPhyloTreeScheme::eZoomX ||
            m_SL->GetZoomBehavior(GetDescription()) == CPhyloTreeScheme::eZoomXY) {
                // Get the vertical distance between nodes for current projection
                // (in pixels) at maximum resolution (visible-rect==model-limits-rect)      
                // (DistanceBetweenNodes() returns dist for current zoom level)
                //float distance_between_nodes = DistanceBetweenNodes();
                float distance_between_nodes = 
                    (TModelUnit(pane.GetViewport().Height())*x_GetVerticalSeparationFactor()*m_pPane->GetOutputUnitScaler())/
                            ((TModelUnit)m_DS->GetSize());

                // Determine how much zoom (reduction in visible rect), if any, is 
                // required to get the distance between nodes to a value that allows
                // labels to be drawn without overlapping each other (vertically);
                float height = (float)m_Label.GetMinVerticalSeparation();
                label_zoom = std::min(1.0f, distance_between_nodes/height);
        }
        else if (m_SL->GetZoomBehavior(GetDescription()) == CPhyloTreeScheme::eZoomY) {
            // scale room for label width based on current zoom in horizontal
            label_zoom = (((float)pane.GetVisibleRect().Width())/
                           (float)pane.GetModelLimitsRect().Width());
        }


        // Check the projection in each direction by computing the pixel
        // offset as it will computed through the orthographic 
        // projection-to-viewport mapping.  Add to this value the pixel
        // offset of the label, "offset".
        //  
        //  w = viewport width, h = viewport height, x,y = node position,
        //  l,r,t,b = limits(left, right, top, bottom)
        //   
        //  (scale model to viewport)*(tranlate model to (0,0))*(model point)
        //  --                -- --        -- -- --
        //  | w/(r-l)  0    0  | | 1  0  -l | | x |
        //  |                  | |          | |   |
        //  | 0     h/(t-b) 0  | | 0  1  -b | | y |
        //  |                  | |          | |   |
        //  | 0        0    1  | | 0  0   1 | | 1 |
        //  --                -- --        -- -- --  
        for (size_t i=0; i<xexts.size(); ++i) {
            // Get position node projects onto viewport:         
            viewport_x =
                (xexts[i].m_NodeCoord*vp_width - left*vp_width)/(right-left);

            // In this first (initial) loop over the extents, just compute the
            // region (minx..maxx) occupied by the nodes projected onto the viewport.            
            node_maxx = std::max(viewport_x, node_maxx);
            node_minx = std::min(viewport_x, node_minx);

            xexts[i].m_ProjectionCood = viewport_x;
        }

        viewport_maxx = node_maxx;
        viewport_minx = node_minx;
        node_extents.SetRight(viewport_maxx);
        node_extents.SetLeft(viewport_minx);

        // % of area occupied by tree (nodes and branches) rather than labels. We
        // do not want the tree part to shrink to (near) 0 when the labels are very long
        // (very long means long enough to occupy the majority of the viewport)
        //pct_non_label = (node_maxx-node_minx)/vp_width;

        // Now that we have the extent that the nodes project onto the viewport
        // (node_minx..node_maxx), determine how the viewport needs to be adjusted
        // to accomodate pixel-dimensioned (label) attachements to the nodes.
        for (size_t i=0; i<xexts.size(); ++i) {
            // Get position node projects onto viewport:         
            viewport_x = xexts[i].m_ProjectionCood;          

            // Pixel extent (which come from labels) attached to the current node
            float offsetx = xexts[i].m_PixelOffset*label_zoom;

            // If we do have a pixel attachment it may go either right (+) or left(-).
            // Figure out how many pixels of that extent (label) go beyond the right
            // or left edge of the overall tree (if the label is on a right-most or
            // left-most tree node, that value is 0, otherwise it is the length of the label
            // in pixels minus the extent that those pixels overlap with the range 
            // (node_minx, node_maxx)
            if (std::abs(offsetx) > 0.0f) {
                float overlap_x;

                // overlap_x is the extent of the label that overlaps (in x)
                // the tree (node/edges) extent.
                if (offsetx > 0.0f)
                    overlap_x = (node_maxx - viewport_x);
                else
                    overlap_x = (viewport_x - node_minx);

                // non_overlap_x is extent of pixel attachment (label) that goes
                // beyond the rightmost or leftmost node in the tree
                float non_overlap_x = std::max(std::abs(offsetx) - overlap_x, 0.0f);

                // scaled_width is the amount of viewport available for labels (otherwise a long
                // label could push the whole tree into a single vertical line)
                float scaled_width =  vp_width*GetLabelViewPct();

                // Portion of viewport occupied by labels only (beyond tree) can't 
                // exceed scaled_width (thats non_overlap_x).  Part inside tree
                // is always included.
                float usable_offsetx = std::min(overlap_x, std::abs(offsetx)) + 
                                       std::min(non_overlap_x, scaled_width);
                offsetx = (offsetx >= 0.0f) ? usable_offsetx : -usable_offsetx;
            }         

            // Add the pixel offset to the overall viewport size.
            if (offsetx != 0.0f) {
                viewport_x += offsetx;
                viewport_maxx = std::max(viewport_x, viewport_maxx);
                viewport_minx = std::min(viewport_x, viewport_minx);
            }           
        }
        
        text_extents.SetRight(viewport_maxx);
        text_extents.SetLeft(viewport_minx);

        for (size_t i=0; i<yexts.size(); ++i) {
            // Get position node projects onto viewport:                    
            viewport_y =
                (yexts[i].m_NodeCoord*vp_height - bottom*vp_height)/(top-bottom);
            viewport_maxy = std::max(viewport_y, viewport_maxy);
            viewport_miny = std::min(viewport_y, viewport_miny);

            // In this first (initial) loop over the extents, just compute the
            // region (miny..maxy) occupied by the nodes projected onto the viewport.            
            node_maxy = std::max(viewport_y, node_maxy);
            node_miny = std::min(viewport_y, node_miny);

            yexts[i].m_ProjectionCood = viewport_y;
        }

        viewport_maxy = node_maxy;
        viewport_miny = node_miny;
        node_extents.SetTop(viewport_maxy);
        node_extents.SetBottom(viewport_miny);

        for (size_t i=0; i<yexts.size(); ++i) {
            // Get position node projects onto viewport:         
            viewport_y = yexts[i].m_ProjectionCood;          

            // Pixel extent (which come from labels) attached to the current node
            float offsety = yexts[i].m_PixelOffset*label_zoom;

            if (std::abs(offsety) > 0.0f) {
                float overlap_y;

                if (offsety > 0.0f)
                    overlap_y = (node_maxy - viewport_y);
                else
                    overlap_y = (viewport_y - node_miny);

                float non_overlap_y = std::max(std::abs(offsety) - overlap_y, 0.0f);

                // Portion of viewport occupied by labels only (beyond tree) can't 
                // exceed scaled_width (thats non_overlap_x).  Part inside tree
                // is always included.
                float usable_offsety = std::min(overlap_y, std::abs(offsety)) + 
                                       std::min(non_overlap_y, vp_height*GetLabelViewPct()); //node_maxy-node_miny);
                offsety = (offsety >= 0.0f) ? usable_offsety : -usable_offsety;
            }         

            // Add the pixel offset to the overall viewport size.
            if (offsety != 0.0f) {
                viewport_y += offsety;
                viewport_maxy = std::max(viewport_y, viewport_maxy);
                viewport_miny = std::min(viewport_y, viewport_miny);
            }           
        }
        
        text_extents.SetTop(viewport_maxy);
        text_extents.SetBottom(viewport_miny);

        ////////////////////////////////////////////////////////////////////////////// 
        // For the viewport directions that had projections by labels, re-estimate the
        // best size for the limits in that dimension.
        //////////////////////////////////////////////////////////////////////////////
        delta = 0.0f;
        // Pixels could go in both directions (min and max) so we
        // just double
        if (viewport_maxx != -1e10f) {
            float rd = viewport_maxx - (float)pane.GetViewport().Right();
            if (fabs(rd) > 0.0f) {
                delta = std::max(delta, (float)fabs(rd));
                float off = (rd/vp_width)*(right-left);
                right += off;
            }
        }

        if (viewport_minx != 1e10f) {
            float ld = viewport_minx - (float)pane.GetViewport().Left();
            if (fabs(ld) > 0.0f) {
                delta = std::max(delta, (float)fabs(ld));
                float off =  (ld/vp_width)*(right-left);
                left += off;
            }
        }

        if ( viewport_maxy != -1e10f) {
            float td = viewport_maxy - (float)pane.GetViewport().Top();
            if (fabs(td) > 0.0f) {
                delta = std::max(delta, (float)fabs(td));
                float off = (td/vp_height)*(top-bottom);
                top += off;
            }
        }

        if ( viewport_miny != 1e10f) {
            float bd = viewport_miny - (float)pane.GetViewport().Bottom();
            if (fabs(bd) > 0.0f) {
                delta = std::max(delta, (float)fabs(bd));
                float off = (bd/vp_height)*(top-bottom);
                bottom += off;
            }
        }        

        vp_extent.Set(viewport_maxx-viewport_minx,
                      viewport_maxy-viewport_miny);

        // not improving much - break
        if (std::abs(delta-prev_delta) < goal)
            break;

        // Sometimes projections limits keep growing because no matter how
        // big it is, the labels can still not fit in the viewport.  previous
        // delta check should catch this, but without this we could get NaNs
        if (right-left > 1000000 || top-bottom > 1000000)
            break;
    }

    limits.SetRight(right);
    limits.SetLeft(left);
    limits.SetBottom(bottom);
    limits.SetTop(top);

    return vp_extent;
}

void IPhyloTreeRender::CExtentDimension::UpdateExtent(float node_coord,
                                                      float pixel_offset,
                                                      bool has_label)
{

    // If there was no label, the new coordinate can only be inserted
    // at the head of the list.  It will replace the first element in
    // the list if that element also does not have a label (pixel_offset
    // is still non-zero however, because all elements have a pixel margin
    // around themseleves) 
    if ( !has_label ) {    
        // List empty, just add
        if (m_Extents.size() == 0) {
            m_Extents.insert(m_Extents.begin(), 
                ProjectionElement(node_coord, pixel_offset));
        }
        // List not empty - decide whether or not the first element will
        // be replaced
        else if (node_coord > m_Extents[0].m_NodeCoord) {
            if (m_Extents[0].m_PixelOffset <= pixel_offset) {
                m_Extents[0] = ProjectionElement(node_coord, pixel_offset);
            }
            else {
                m_Extents.insert(m_Extents.begin(), 
                    ProjectionElement(node_coord, pixel_offset));
            }
        }
    }
    // Current element has a label. Determine if it should be inserted,
    // where to insert it, whether it will overwrite an existing entry,
    // and whether elements in the list with smaller coordnates need to 
    // be erased since they no longer can effect max. projection extent
    else {
        float max_pixel_offset = -FLT_MAX;
        bool resolved = false;       
        int erase_from = -1;

        // Iterate over list until we find where to insert (list is generally
        // short (1-10 elements), so a binary search could be longer)
        for (size_t ext_idx=0; ext_idx<m_Extents.size() && !resolved; ++ext_idx) {
            max_pixel_offset = std::max(max_pixel_offset, m_Extents[ext_idx].m_PixelOffset);

            // Coord > than current element in list, so insert it
            if (node_coord > m_Extents[ext_idx].m_NodeCoord) {
                if (pixel_offset >= max_pixel_offset) {                   
                    m_Extents[ext_idx].m_PixelOffset = pixel_offset;
                    m_Extents[ext_idx].m_NodeCoord = node_coord;
                    //m_Extents[ext_idx].m_Text = label;
                    erase_from = ext_idx;   
                }
                else {
                    ProjectionElement ne(node_coord, pixel_offset);
                    //ne.m_Text = label;
                    m_Extents.insert(m_Extents.begin() + ext_idx, ne);
                }
                resolved = true;                    
            }
            // Coord == coord of current list element. Replace the current
            // element with the new IF new element has larger pixel offset
            else if (node_coord == m_Extents[ext_idx].m_NodeCoord) {                        
                if (pixel_offset > max_pixel_offset) {                  
                    //ne.m_Text = label;                  
                    m_Extents[ext_idx].m_PixelOffset = pixel_offset;
                    erase_from = ext_idx;
                }
                resolved = true;
            }
            //node_coord < m_Extents[ext_idx].m_NodeCoord
            else {
                if (pixel_offset <= max_pixel_offset) {
                    resolved = true;
                }                 
            }
        }

        // Coordinate was smaller than any element in the list,
        // but the label (pixel offset) was larger than any element
        // in the list so insert the element at the end
        if (!resolved && pixel_offset > max_pixel_offset) {
            ProjectionElement ne(node_coord, pixel_offset);
            //ne.m_Text = label;
            m_Extents.push_back(ne);
        }

        // Element was inserted into the list and its label (pixel offset) was
        // larger than all of the elements up to its insertion point.  Now erase
        // any elements after the newly inserted element that have a pixel offset
        // that is smaller than the pixel offset of the inserted element since
        // they will no longer effect the projection size (at any zoom level)
        if (erase_from >=0) {
            size_t erase_to = erase_from + 1;
            for (; erase_to < m_Extents.size(); ++erase_to)
                if (pixel_offset < m_Extents[erase_to].m_PixelOffset)
                    break;

            if (erase_to > (size_t)(erase_from + 1))
                m_Extents.erase(m_Extents.begin() +(size_t)( erase_from + 1),
                m_Extents.begin() + erase_to);               
        }            
    }
}

void IPhyloTreeRender::CExtentDimension::DumpExtent(const string& dim) {
    for (size_t i=0; i<m_Extents.size(); ++i) {
        _TRACE("Value: " << 
            //m_Extents[i].m_Text << 
            dim << m_Extents[i].m_NodeCoord << " ) Pix: ( " <<
            m_Extents[i].m_PixelOffset << " ) ");
    }
}

void IPhyloTreeRender::InitExtents()
{
    m_ZoomableExtent.SetBottom(1e10);
    m_ZoomableExtent.SetLeft(1e10f);
    m_ZoomableExtent.SetRight(-1e10);
    m_ZoomableExtent.SetTop(-1e10);

    m_ProjectionExtents.Clear();
}

void IPhyloTreeRender::CalculateExtents(CPhyloTree* tree, CPhyloTree::TNodeType& node)
{   
    CVect2<float> pos = (*node).XY();

    GLdouble left_margin, top_margin, right_margin, bottom_margin;
    m_SL->GetMargins(left_margin, top_margin, right_margin, bottom_margin);

    // The zoomable_extent just holds the x/y extents without considering 
    // labels.
    m_ZoomableExtent.SetLeft(std::min(m_ZoomableExtent.Left(), (TModelUnit)pos.X()));
    m_ZoomableExtent.SetRight(std::max(m_ZoomableExtent.Right(), (TModelUnit)pos.X()));
    m_ZoomableExtent.SetBottom(std::min(m_ZoomableExtent.Bottom(), (TModelUnit)pos.Y()));
    m_ZoomableExtent.SetTop(std::max(m_ZoomableExtent.Top(), (TModelUnit)pos.Y()));

    // If no label, add to extent without considering label
    if (node->GetDisplayLabel().empty()) {
        CVect2<float> pos = (*node).XY();

        m_ProjectionExtents.m_MaxX.UpdateExtent(pos.X(), right_margin, false);
        m_ProjectionExtents.m_MaxY.UpdateExtent(pos.Y(), top_margin, false);
        m_ProjectionExtents.m_MinX.UpdateExtent(-pos.X(), left_margin, false);
        m_ProjectionExtents.m_MinY.UpdateExtent(-pos.Y(), bottom_margin, false);
    }
    // There is a label - add to each extent based on the labels maximum
    // projection distance (in pixels) in that direction
    else {
        CGlRect<float> r = node->GetLabelRect();

        float right = -std::numeric_limits<float>::max();
        float left = std::numeric_limits<float>::max();
        float top = -std::numeric_limits<float>::max();
        float bottom = std::numeric_limits<float>::max();

        if (!m_RotatedLabels) {
            right = r.Right();
            left = r.Left();
            top = r.Top();
            bottom = r.Bottom();
        }
       else {
            // Get rotation angle (which is relative to negative x axis
            // for angles between 90 and 270)
            double a = (double)node->GetAngle();

            if (a > (M_PI / 2.0) && a < ((3.0*M_PI) / 2.0)) {
                a = a - M_PI;
            }

            float cos_a = (float)cos(a);
            float sin_a = (float)sin(a);

            // Current rotation algorithm changes the amount of rotation from
            // not rotated at all (if the nodes are more spread out) up to
            // the specified angle. So the maximum right/left values are the
            // unrotated values so we start with those.
            right = r.Right();
            left = r.Left();

            // Compute the positions of the four corners of the rotated text
            // (in pixel coordinates relative to the node position) and extract
            // maximum and minimum x and y coordinates. Note that here we need
            // positions unscaled (unlike CPhyloNodeData::GetRotatedTextBox)
            for (int corner = 0; corner < 4; ++corner) {
                CGlPoint<float> pos = r.GetCorner(corner);

                // multiply corner by standard 2D rotation matrix:
                CVect2<float> rpos(pos.X()*cos_a - pos.Y()*sin_a, pos.X()*sin_a + pos.Y()*cos_a);
                right = std::max(right, rpos.X());
                left = std::min(left, rpos.X());
                top = std::max(top, rpos.Y());
                bottom = std::min(bottom, rpos.Y());
            }
        }

        m_ProjectionExtents.m_MaxX.UpdateExtent(pos.X(), right + right_margin, true);
        m_ProjectionExtents.m_MaxY.UpdateExtent(pos.Y(), top + top_margin, true);
        // note that we negate minimum values for x,y so that we can use
        // the same function (UpdateExtents()) for all 4 directions
        m_ProjectionExtents.m_MinX.UpdateExtent(-pos.X(), -left + left_margin, true);
        m_ProjectionExtents.m_MinY.UpdateExtent(-pos.Y(), -bottom + bottom_margin, true);
    }
}

void IPhyloTreeRender::
CProjectionExtents::ConsolidateExtents(vector<ProjectionElement>& xexts,
                                       vector<ProjectionElement>& yexts)
{
    /// Combine m_MaxX and m_MinX.  All coordinates in m_MinX are
    /// flipped (negated) so we have to reverse that here.
    xexts = m_MaxX.m_Extents;
    for (size_t i=0; i<m_MinX.m_Extents.size(); ++i) {
        ProjectionElement ne(m_MinX.m_Extents[i]);
        ne.m_NodeCoord = -ne.m_NodeCoord;
        ne.m_PixelOffset = -ne.m_PixelOffset;

        xexts.push_back(ne);
    }

    /// Combine m_MaxY and m_MinY.  All coordinates in m_MinY are
    /// flipped (negated) so we have to reverse that here.
    yexts = m_MaxY.m_Extents;
    for (size_t i=0; i<m_MinY.m_Extents.size(); ++i) {
        ProjectionElement ne(m_MinY.m_Extents[i]);
        ne.m_NodeCoord = -ne.m_NodeCoord;
        ne.m_PixelOffset = -ne.m_PixelOffset;

        yexts.push_back(ne);
    }
}

void IPhyloTreeRender::CProjectionExtents::DumpExtents() {
    _TRACE("");
    _TRACE("Dumping + X Dimension:");
    m_MaxX.DumpExtent(" X Pos: ( ");

    _TRACE("");
    _TRACE("Dumping + Y Dimension:");
    m_MaxY.DumpExtent(" Y Pos: ( ");

    _TRACE("");
    _TRACE("Dumping - X Dimension:");
    m_MinX.DumpExtent(" -X Pos: ( ");

    _TRACE("");
    _TRACE("Dumping - Y Dimension:");
    m_MinY.DumpExtent(" -Y Pos: ( ");
}

void IPhyloTreeRender::ComputeViewingLimits(CGlPane& pane, 
                                            bool force_square,
                                            bool init_collision_info)
{
    GLdouble top, bottom, left, right;
    m_SL->GetMargins(left, top, right, bottom);

    // This adds label size to overall viewing area, but does so incorrectly - too much space
    // is added for the label probably because the current (visible) rect already includes
    // the label from the previous call to this function? Maybe we could differentiate between 
    // nodes and edges size and that size + text size by saving the text origins with the nodes
    // and edges, and using those to compute (at each change) the size with labels...
    
    TModelRect visible = pane.GetVisibleRect();
    TModelRect limits = pane.GetModelLimitsRect();

    TModelRect extent_limits;   
    extent_limits = m_ZoomableExtent;

    // Consolidate the extent data into two vectors - one for min/max x values and one 
    // for min/max y values.
    vector<ProjectionElement> xexts, yexts;
    m_ProjectionExtents.ConsolidateExtents(xexts, yexts);

    //extents.DumpExtents();

    //_TRACE("Visible Nodes: " << m_DS->GetVisibleNodes() );
    //  If there is only 1 node force a small window
    bool too_small = false;
    if (m_DS->GetVisibleNodes() == 1) {
        m_ZoomableExtent.Inflate(1.0f, 1.0f);
        too_small = true;
    }

    // Get numerical problems as viewport size goes to 0 so we just use the model
    // extent in this (not-visually-usable) case:
    if (pane.GetViewport().Height() < 20 ||
        pane.GetViewport().Width() < 20) {
        pane.SetModelLimitsRect(m_ZoomableExtent);
        pane.SetVisibleRect(m_ZoomableExtent);
        return;
    }

    if (!too_small) {           
        TModelRect node_rect, text_rect;
        x_FindBestSize(pane, xexts, yexts, m_ZoomableExtent, node_rect, text_rect);
        // Guard against numerical issues (very small extents - maybe nodes are in a line)
        if (m_ZoomableExtent.Width() < 1.0f)
            m_ZoomableExtent.Inflate(1.0f, 0.0f);
        if (m_ZoomableExtent.Height() < 1.0f)
            m_ZoomableExtent.Inflate(0.0f, 1.0f);
    }          

    TModelRect mr(m_ZoomableExtent.Left() - left,
        m_ZoomableExtent.Bottom() - bottom,
        m_ZoomableExtent.Right() + right,
        m_ZoomableExtent.Top() + top);

    // If user wants width and height to match - primary reason would
    // be to force circular cladogram to be a true circle rather than
    // an ellipses
    if (force_square) {
        if (mr.Width() > mr.Height())
            mr.Inflate(0.0, (mr.Width()-mr.Height())/2.0f);
        else if (mr.Height() > mr.Width())
            mr.Inflate((mr.Height()-mr.Width())/2.0f, 0.0);
    }       

    pane.SetModelLimitsRect(mr);

    float left_pct = 0.0f;
    float right_pct = 0.0f;
    float bottom_pct = 0.0f;
    float top_pct = 0.0f;

    if (limits.Width() > 0.0 && limits.Height() > 0.0) {
        left_pct = (visible.Left() - limits.Left())/limits.Width();
        right_pct = (visible.Right() - limits.Left())/limits.Width();
        bottom_pct = (visible.Bottom() - limits.Bottom())/limits.Height();
        top_pct = (visible.Top() - limits.Bottom())/limits.Height();
    }

    visible.SetLeft(mr.Left() + mr.Width()*left_pct);
    visible.SetRight(mr.Left() + mr.Width()*right_pct);
    visible.SetBottom(mr.Bottom() + mr.Height()*bottom_pct);
    visible.SetTop(mr.Bottom() + mr.Height()*top_pct);

    // Visible rect will be updated based on zoom. 
    pane.SetVisibleRect(visible);

    BufferedRender(pane, *m_DS, init_collision_info);
}

void IPhyloTreeRender::BufferedRender(CGlPane& pane, 
                                      CPhyloTreeDataSource& ds,
                                      bool init_collision_info,
                                      bool select_edge)
{
    // Depending on initialization order (for a newly created view) this may 
    // get called before ComputeViewingLimits has been called for the first time.
    // Do not render in that case.
    if (m_ValidLayout != eValidLayout)
        return;

    // compute edge highlights (if enabled)
    x_ComputeEdgeHighlight();

    m_pPane     = &pane;
    m_DS        = &ds;

    CStopWatch  render_total;
    render_total.Start();

    // Set visible selection set which may be smaller (one node plus parents
    // and children) than the full selection set if user is iterating over
    // the nodes
    CPhyloTree::TTreeIdx current_idx = ds.GetTree()->GetCurrentNodeIdx();
    m_NodeSingleSelection.clear();
    if (!m_SL->GetShowAllSelected() && current_idx != CPhyloTree::Null()) {
        ds.GetTree()->GetSelState(m_NodeSingleSelection, current_idx, true, true);
    }

    m_DS->GetModel().SetRotateLabels(m_RotatedLabels); 
    x_SetGlRenderOptions(ds);

    // Edge selection can only happen during x_RenderVbo since edges are 
    // not explicitly stored on c++ side but are generated during x_RenderVbo
    // and saved in OpenGL buffers.
    m_SelectedEdge.clear();
    m_SelEdge = select_edge;
    m_ClosestEdgeDistSq = std::numeric_limits<float>::max();   
    m_ClosestEdgeDir.Set(0.0f, 0.0f);
    m_DS->GetTree()->SetCurrentEdge(CPhyloTree::Null(), CPhyloTree::Null());

    CGlUtils::CheckGlError();
    m_DS->GetModel().ClearArrays();
    x_RenderVbo(*m_DS);
    CGlUtils::CheckGlError();

    // If we were looking for an edge to select, see if the closest edge found
    // was close enough. If not, erase any selected edge.
    if (m_SelEdge) {       
        m_ClosestEdgeDir.X() *= (1.0f / (float)pane.GetScaleX());
        m_ClosestEdgeDir.Y() *= (1.0f / (float)pane.GetScaleY());
        m_SelEdge = false;

        if (m_ClosestEdgeDir.Length() > 4) {
            m_DS->GetTree()->SetCurrentEdge(CPhyloTree::Null(), CPhyloTree::Null());
            m_SelectedEdge.clear();
        }
    }

    // Caller can elect to skip this if they know they will be
    // doing it themselves later (performance).  It is done again
    // later if the pane's visible rect is updated.
    if (init_collision_info)
      InitCollisionInfo();
    
    CStopWatch stimer;
    stimer.Start();
    m_DS->GetModel().SyncBuffers();
    m_VboSyncTime = stimer.Elapsed();

    m_BufferdRenderTime = render_total.Elapsed();
}

void IPhyloTreeRender::InitCollisionInfo()
{
    /// For large models, track the resolution to be relative to a 
    /// full screen of nodes with labels since that is our unit of drawing
    float collision_resolution = max(ceilf(sqrtf(m_DS->GetSize())),
        ((float)m_DS->GetSize()) / 4000.0f);

    CPhyloTreeScheme::TZoomBehavior zb = m_SL->GetZoomBehavior(GetDescription());
    m_DS->GetModel().GetCollisionData().ResizeGrid(*m_pPane,
        collision_resolution,
        (zb != CPhyloTreeScheme::eZoomY),
        (zb != CPhyloTreeScheme::eZoomX));

    // >> This also calls UpdateScaled. Just make sure scale is right..
    m_DS->GetModel().GetCollisionData().Sync(*m_pPane, &m_DS->GetModel());

    // initiate visibility computation
    TModelUnit optional_pdf_scaler = 1.0 / m_pPane->GetOutputUnitScaler();
    CVect2<float> scale2d((float)(m_pPane->GetScaleX()*optional_pdf_scaler),
                          (float)(m_pPane->GetScaleY()* optional_pdf_scaler));
    m_DS->GetModel().GetCollisionData().UpdateScaled(*m_pPane, scale2d);
}

bool IPhyloTreeRender::LabelsVisible() {
    if (( (m_SL->GetLabelVisibility()==CPhyloTreeScheme::eLabelsVisible) ||
        (m_SL->GetLabelVisibility()==CPhyloTreeScheme::eLabelsForLeavesOnly) ) &&
        m_Label.IsVisible(this) ){
            return true;
    }
    else {
        return false;
    }
}

float IPhyloTreeRender::DefaultNodeSize() 
{
    float node_size = 0.0f;
    float def_node_size = (float)m_SL->SetSize(CPhyloTreeScheme::eNodeSize);
    if (DistanceBetweenNodes() > def_node_size * 2.0f) {
        node_size = def_node_size;
    }

    return node_size;
}

bool IPhyloTreeRender::x_WideLines()
{
    TModelUnit line_width =
        m_SL->SetSize(CPhyloTreeScheme::eLineWidth);
    
    if (DistanceBetweenNodes() > line_width) {
        return true;
    }
    return false;
}


void IPhyloTreeRender::x_SetElementVisibility()
{
    TModelUnit optional_pdf_scaler = 1.0 / m_pPane->GetOutputUnitScaler();

    float x = (float)m_pPane->GetScaleX()*optional_pdf_scaler;
    float y = (float)m_pPane->GetScaleY()*optional_pdf_scaler;

    CVect3<float> scale(x, y, 0.0f);
    CVect2<float> scale2d(x, y);

    float node_size = DefaultNodeSize();

    // This doesn't handle nodes that are forced to be <0 in size...
    CGlVboNode* point_node = m_DS->GetModel().FindGeomNode("NodePoints");
    if (point_node) {
        point_node->SetVisible(node_size <= 0.0f);
    }

    TModelUnit line_width = 1.0;

    if (x_WideLines()) {
        line_width = m_SL->SetSize(CPhyloTreeScheme::eLineWidth);
    }

    CGlVboNode* edge_node = m_DS->GetModel().FindGeomNode("TreeEdges");
    CGlVboNode* narrow_edge_node = m_DS->GetModel().FindGeomNode("NarrowTreeEdges");
    CGlVboNode* sel_edge_node = m_DS->GetModel().FindGeomNode("SelectedTreeEdges");
    CGlVboNode* sel_narrow_edge_node = m_DS->GetModel().FindGeomNode("SelectedNarrowTreeEdges");

    if (edge_node != NULL && 
        narrow_edge_node != NULL) {
        edge_node->GetState().LineWidth((float)line_width);
        sel_edge_node->GetState().LineWidth((float)line_width);

        narrow_edge_node->GetState().LineWidth(1.0f);
        sel_narrow_edge_node->GetState().LineWidth(1.0f);
        narrow_edge_node->SetVisible(line_width > 1.0f);
        sel_narrow_edge_node->SetVisible(line_width > 1.0f);
    }

    m_DS->GetModel().SetRotateLabels(m_RotatedLabels);

    CStopWatch  cdtime;
    cdtime.Start();

    CTreeTriFanNode* node_fan = m_DS->GetModel().GetTriFanNode();
    if (node_fan) {
        node_fan->GetState().ScaleFactor(m_pPane->GetScale());

        node_fan->SetVisible(node_size > 0.0f);

        node_fan->SetLabelVisibility(LabelsVisible());

        // Set visibility for collapsed nodes too...
        vector<CGlVboNode*>& nodes = m_DS->GetModel().GetTempNodes();
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i]->GetName() == "CollapsedNode") {
                nodes[i]->SetVisible(node_size > 0.0f);
                nodes[i]->GetState().ScaleFactor(m_pPane->GetScale());
            }
        }

        // Determines if labels and associated nodes may be visible (if they are in
        // the viewing area).  Only needed if the nodes or labels are visible.
        if (node_fan->IsVisible() || node_fan->GetLabelVisibility()) {
            CVect2<float> s = m_DS->GetModel().GetCollisionData().GetScale();
            if (s.X() != scale2d.X() || s.Y() != scale2d.Y())
                m_DS->GetModel().GetCollisionData().UpdateScaled(*m_pPane, scale2d);
            m_DS->GetModel().GetCollisionData().UpdateVisibility(m_pPane->GetVisibleRect());
        }
    }
    m_DS->GetModel().m_CDUpdateTime = cdtime.Elapsed();
}

bool IPhyloTreeRender::Render(CGlPane& pane, CPhyloTreeDataSource& ds)
{
    CStopWatch  render_total;
    render_total.Start();

    m_pPane     = &pane;
    m_DS        = &ds;

    x_SetElementVisibility();

    CIRef<IRender> rmgrgl = CGlResMgr::Instance().GetCurrentRenderer();
    rmgrgl->Render(pane, &m_DS->GetModel());

    // Render selection box
    x_RenderSelection(pane);
    x_RenderTooltipHints(pane);
    // Render distance scale at bottom of screen
    x_RenderScaleMarker(pane, rmgrgl.GetNCObject());
    // Render the (optional) tree label
    x_RenderTreeLabel(pane, rmgrgl.GetNCObject());

    m_DS->GetModel().m_RenderTime = render_total.Elapsed();

    // other views don't concern themselves with textures
    // so make sure these are disabled before returning
    rmgrgl->Disable(GL_TEXTURE_1D);
    rmgrgl->Disable(GL_TEXTURE_2D);

    return true;
}

void IPhyloTreeRender::RenderForHardcopy(CGlPane& pane, CPhyloTreeDataSource& ds)
{
    m_pPane = &pane;
    m_DS = &ds;

    CRgbaColor bgColor(m_SL->SetColor(CPhyloTreeScheme::eTree,
        CPhyloTreeScheme::eBgColor));

    glClearColor(bgColor.GetRed(), bgColor.GetGreen(),
        bgColor.GetBlue(), bgColor.GetAlpha());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    Render(pane, ds);
}


void IPhyloTreeRender::RenderPdf(CGlPane& pane,
    CPhyloTreeDataSource& ds,
    CRef<CPdf> pdf)
{
    m_pPane = &pane;
    m_DS = &ds;

    CRgbaColor bgColor(m_SL->SetColor(CPhyloTreeScheme::eTree,
        CPhyloTreeScheme::eBgColor));

    glClearColor(bgColor.GetRed(), bgColor.GetGreen(),
        bgColor.GetBlue(), bgColor.GetAlpha());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // save current render manager
    CIRef<IRender>  prev_mgr = CGlResMgr::Instance().GetCurrentRenderer();

    // Set a render manager to write to a pdf object:
    CIRef<IRender> mgr = CGlResMgr::Instance().GetRenderer(eRenderPDF);
    CGlRenderPdf* rpdf = NULL;
    if (mgr.IsNull()) {
        rpdf = new CGlRenderPdf();
        mgr.Reset(rpdf);
        CGlResMgr::Instance().AddRenderer(mgr);
    }
    else {
        rpdf = dynamic_cast<CGlRenderPdf*>(mgr.GetPointerOrNull());
        if (rpdf == NULL) {
            LOG_POST(Error << "RenderPdf: unable to cast render manager to CGlRenderPdf");
            return;
        }
    }
    rpdf->SetPdf(pdf);


    bool bitmap_font = m_SL->GetFont().IsBitmapFont();
    CGlTextureFont::EFontFace current_font_face = m_SL->GetFont().GetFontFace();
    CGlTextureFont::EFontFace pdf_font_face;

    CGlResMgr::Instance().SetCurrentRenderer(mgr);

    // We can't use bitmap fonts for PDF rendering since the metrics
    // don't match the metrics of the corresponding PDF fonts
    if (m_SL->GetFont().IsBitmapFont()) {
        CGlTextureFont f;
        f.SetFontSize(m_SL->GetFont().GetFontSize());
        f.SetFontFace(m_SL->GetFont().GetFontFace(), false);
        m_SL->SetFont(f);
        m_DS->Relabel(m_SL, m_SL->SetLabelFormat());
        
    }
    // Also some of our standard fonts are not available in PDF (we rely on built-in PDF fonts which only
    // includes times, helvetica and courier and we match other fonts to the closest of these)
    else {
        CGlTextureFont::PdfBaseFontName(current_font_face, pdf_font_face);

        if (pdf_font_face != current_font_face) {
            CGlTextureFont f;
            f.SetFontSize(m_SL->GetFont().GetFontSize());
            f.SetFontFace(pdf_font_face, false);
            m_SL->SetFont(f);
            m_DS->Relabel(m_SL, m_SL->SetLabelFormat());
        }
    }

    Layout(*m_DS, *m_pPane);

    x_SetElementVisibility();

    rpdf->Render(pane, &m_DS->GetModel());
    x_RenderScaleMarker(pane, *rpdf);
    // Render the (optional) tree label
    x_RenderTreeLabel(pane, *rpdf);

    // Set the previous render manager:
    CGlResMgr::Instance().SetCurrentRenderer(prev_mgr);

    // Switch back to bitmap fonts, if that's what we had before:
    if (bitmap_font) {
        CGlTextureFont f;
        f.SetFontSize(m_SL->GetFont().GetFontSize());
        f.SetFontFace(m_SL->GetFont().GetFontFace(), true);
        m_SL->SetFont(f);
    }
    // Or to our pdf-equivalent font:
    else if (pdf_font_face != current_font_face) {
        CGlTextureFont f;
        f.SetFontSize(m_SL->GetFont().GetFontSize());
        f.SetFontFace(current_font_face, true);
        m_SL->SetFont(f);
    }

    m_DS->Relabel(m_SL, m_SL->SetLabelFormat());
    Layout(*m_DS, *m_pPane);
}

void IPhyloTreeRender::Redraw(void)
{
    CRgbaColor bgColor(m_SL->SetColor(CPhyloTreeScheme::eTree,
                                    CPhyloTreeScheme::eBgColor));

    glClearColor(bgColor.GetRed(), bgColor.GetGreen(),
        bgColor.GetBlue(), bgColor.GetAlpha());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    Render(*m_pPane, *m_DS);
}

void IPhyloTreeRender::RemoveCurrentDataSource() 
{
    m_ValidLayout = eNeedLayoutAndSize;
}


void IPhyloTreeRender::x_GenerateTexture(void)
{
    GLint w = m_pPane->GetViewport().Width();
    GLint h = m_pPane->GetViewport().Height();

    if (!m_MinimapBuffer) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        m_MinimapBuffer.Reset(CGlResMgr::Instance().CreateFrameBuffer(1024));
        m_MinimapBuffer->SetTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
        m_MinimapBuffer->CreateFrameBuffer();
        glPopAttrib();
    }

    if (m_MinimapBuffer->IsValid()) {

        auto renderer = [this, w, h]() {
            // Set viewport and draw:
            GLint vp_width, vp_height;
            if (w > h) {
                vp_width = (int)m_MinimapBuffer->GetFrameSize();
                vp_height = (int)(((float)vp_width)*((float)h) / (float)w);
            }
            else {
                vp_height = (int)m_MinimapBuffer->GetFrameSize();
                vp_width = (int)(((float)vp_height)*((float)w) / (float)h);
            }

            // Get current viewport and visible region so we can
            // restore them later
            TVPRect saved_viewport = m_pPane->GetViewport();
            TModelRect saved_rect = m_pPane->GetVisibleRect();
            TModelRect saved_limits_rect = m_pPane->GetModelLimitsRect();

            // Set visible rect to draw the whole tree
            m_pPane->SetVisibleRect(m_pPane->GetModelLimitsRect());
            m_pPane->GetViewport().Init(0, 0, vp_width - 1, vp_height - 1);

            ComputeViewingLimits(*m_pPane, m_ForceSquare);
            Redraw();

            // Set viewport and model region back
            m_pPane->SetViewport(saved_viewport);
            m_pPane->SetVisibleRect(saved_rect);
            m_pPane->SetModelLimitsRect(saved_limits_rect);
        };

        m_MinimapBuffer->Render(renderer);

        m_MinimapBuffer->GenerateMipMaps();

        // Update the minimap texture with the texture id from the framebuffer
        m_Texture.Reset(m_MinimapBuffer->Get3DTexture());
        m_Texture->SetFilterMag(GL_LINEAR);
        m_Texture->SetFilterMin(GL_LINEAR_MIPMAP_LINEAR);
        m_Texture->SetParams();


        CGlUtils::CheckGlError();
    }
    else {
        // If system does not support framebuffers, generate a (worse) texture the 
        // old-fashioned way.
        TModelRect saved_rect = m_pPane->GetVisibleRect();
        m_pPane->SetVisibleRect(m_pPane->GetModelLimitsRect());
        glDrawBuffer(GL_BACK);
        Redraw();

        try {
            CImage * image = new CImage(w, h, 4);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);

            glReadBuffer(GL_BACK);
            glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image->SetData());
            m_Texture.Reset(CGlResMgr::Instance().CreateTexture(image));

            m_Texture->SetFilterMag(GL_LINEAR);
            m_Texture->SetFilterMin(GL_LINEAR_MIPMAP_LINEAR);
            m_Texture->SetParams();
        }
        catch(COpenGLException&)
        {
            m_RegenerateTexture = false;
        }
        catch(std::exception&)
        {
            m_RegenerateTexture = false;
        }

        m_pPane->SetVisibleRect(saved_rect);
        Redraw();
    }
}

I3DTexture* IPhyloTreeRender::GetTexture(float& xcoord_limit,
                                           float& ycoord_limit)
{
    if (m_RegenerateTexture) {
        x_GenerateTexture();
    }
    
    // The minimap is created from a square texture, but when we
    // draw to the texture the whole texture is not used.  The limits
    // represent the part of the textture that should be shown.
    GLint w = m_pPane->GetViewport().Width();
    GLint h = m_pPane->GetViewport().Height();
    if (w > h) {
        xcoord_limit = 1.0f;
        ycoord_limit = ((float)h)/(float)w;
    }
    else {
        ycoord_limit = 1.0f;
        xcoord_limit = ((float)w)/(float)h;
    }

    return m_RegenerateTexture ? m_Texture.GetPointer() : NULL;
}

CPhyloTreeNode* IPhyloTreeRender::GetHoverNode(CVect2<float> pt)
{
    if (m_DS != NULL) {

        pt.X()  = (float)m_pPane->UnProjectX(pt.X());
        pt.Y()  = (float)m_pPane->UnProjectY(m_pHost->HMGH_GetVPPosByY(pt.Y())); 

        CPhyloTree::TTreeIdx node_idx = 
            m_DS->GetTree()->TestForNode(pt,  m_Label.IsVisible(this), m_RotatedLabels);
    
        if (node_idx==CPhyloTree::Null())
            return NULL;
        else
            return &(m_DS->GetTree()->GetNode(node_idx));
    }

    return NULL;
}

CPhyloTree::TTreeIdx IPhyloTreeRender::GetHoverNodeIdx(CVect2<float> pt)
{
    if (m_DS != NULL) {
        pt.X()  = (float)m_pPane->UnProjectX(pt.X());
        pt.Y()  = (float)m_pPane->UnProjectY(m_pHost->HMGH_GetVPPosByY(pt.Y())); 

        CPhyloTree::TTreeIdx node_idx = 
            m_DS->GetTree()->TestForNode(pt, m_Label.IsVisible(this), m_RotatedLabels);
    
        if (node_idx==CPhyloTree::Null())
            return CPhyloTree::Null();
        else
            return node_idx;
    }

    return CPhyloTree::Null();
}

string IPhyloTreeRender::TTHH_NeedTooltip(const wxPoint & pt)
{
    // Since this can be checked with a timer, we may get
    // here while user is using rectangle-select and we don't 
    // want to do anything in that case.
    if (m_DS == NULL || m_State == eSelRect)
        return "";

    CPhyloTreeNode* node = GetHoverNode(CVect2<float>(pt.x, pt.y));
    if (node != NULL) {
        return NStr::IntToString(node->GetValue().GetId());
    }

    return "";
}

CTooltipInfo IPhyloTreeRender::TTHH_GetTooltip(const wxRect & rect)
{
    CTooltipInfo  tip;

    // Should not call this while selecting, but just in case, check m_State
    if (m_DS == NULL || m_State == eSelRect)
        return tip;

    m_StartPoint.x = rect.x;  m_StartPoint.y = rect.y;
    CPhyloTreeNode* n = GetHoverNode(CVect2<float>(m_StartPoint.x, m_StartPoint.y));
    
    if (n != NULL) {
        tip.SetTipID(NStr::IntToString(n->GetValue().GetId()));

        std::string final_text = m_LabelExt.GetToolTipForNode(
            *(m_DS->GetTree()), *n, m_SL->SetTooltipFormat(), "\n");

        /*string info;
        CPhyloTree::TTreeIdx idx = GetHoverNodeIdx(CVect2<float>(m_StartPoint.x, m_StartPoint.y));
        if (idx != CPhyloTree::Null()) {
            info = "ID: " + NStr::NumericToString((*n)->GetId());
            info += " idx: " + NStr::NumericToString(idx);
        }*/
        tip.SetTipText(final_text);
        tip.SetTitleText(/*info +*/ (*n)->GetLabel());
    }
   
    return tip;
}

void IPhyloTreeRender::SetActiveTooltipNode(int id) 
{ 
    m_ActiveTooltipNode = m_DS->GetTree()->FindNodeById(TTreeIdx(id));
}

void IPhyloTreeRender::PointToNode(int id, wxRect tip_rect, float sec)
{         
    if (m_DS == NULL)
        return;

    // Create an entry for an arrow to point from a tooltip window to node 'id'
    TTreeIdx tip_node_idx  = m_DS->GetTree()->FindNodeById(TTreeIdx(id));
    if (tip_node_idx != NULL_TREE_IDX) {           
        NodePointer np;
        np.m_NodeID = id;
        np.m_NodeIdx = tip_node_idx;

        np.m_TipCenter.m_X = (TModelUnit)(tip_rect.x + tip_rect.width/2);
        np.m_TipCenter.m_Y = (TModelUnit)(tip_rect.y + tip_rect.height/2);

        np.m_TipRect = tip_rect;

        np.m_Duration = sec;
        if (np.m_Duration != (TModelUnit)-1)
            np.m_Timer.Start(); 

        if (!m_EffectsTimer.IsRunning())
            m_EffectsTimer.Start(30);

        // If we are already pointing to this node, reset it
        for (unsigned int i=0; i<m_NodePointers.size(); ++i) {
            if (m_NodePointers[i].m_NodeID == (CPhyloTree::TID)id) {
                m_NodePointers[i] = np;
                return;
            }
        }

        m_NodePointers.push_back(np);
    }
}

TVPUnit IPhyloTreeRender::DistanceBetweenNodes(void) const
{
    TModelUnit ydist = (m_yStep*m_pPane->GetOutputUnitScaler())/m_pPane->GetScaleY();

    return TVPUnit(ydist);
}

void  IPhyloTreeRender::SetScheme(CPhyloTreeScheme & sl)
{
    m_SL.Reset(&sl);
    m_Label.SetScheme(sl, (m_DS != NULL) ? m_DS->GetTree() : NULL);
    m_LabelExt.SetScheme(sl, (m_DS != NULL) ? m_DS->GetTree() : NULL);
}

float IPhyloTreeRender::SetNodeColoration(CPhyloTreeNode* node,
                                          TTreeIdx node_idx)
{
    float node_alpha = 1.0f;

    CPhyloNodeData::TSelectedState sel_state = (**node).GetSelectedState();

    // This handles the case where the user wants to hide any selected
    // nodes other than the current node (if it is one of the selected nodes.)
    // Allows for a visual iteration that hides all but the current node.
    if (!m_SL->GetShowAllSelected() && sel_state != CPhyloNodeData::eNotSelected) {
        CPhyloTree::TSelState::iterator iter =
            find_if(m_NodeSingleSelection.begin(), m_NodeSingleSelection.end(),
            IPhyloTreeRender::FindSelState(node_idx));

        if (iter != m_NodeSingleSelection.end())
            sel_state = (*iter).second;
        else
            sel_state = CPhyloNodeData::eNotSelected;
    }

    /// Cluster color overrides all states except selected:
    if (sel_state == CPhyloNodeData::eSelected) {
        (**node).SetColoration(CPhyloTreeScheme::eSelColor);
    }
    else {
        if (sel_state == CPhyloNodeData::eShared) {
            (**node).SetColoration(CPhyloTreeScheme::eSharedColor);
        }
        else if (sel_state == CPhyloNodeData::eTraced) {
            (**node).SetColoration(CPhyloTreeScheme::eTraceColor);
        }
        else {
            (**node).SetColoration(CPhyloTreeScheme::eColor);

            if (m_SL->GetSelectionVisibility() == CPhyloTreeScheme::eHighlightSelection) {
                node_alpha = m_SL->GetNonSelectedAlpha();
            }
        }
    }

    return node_alpha;
}


short IPhyloTreeRender::GetNodeColorIdx(CPhyloTreeNode* node)
{
    CPhyloTreeScheme::TPhyloTreeColors c = (**node).GetColoration();

    if (c == CPhyloTreeScheme::eSelColor) {
        return m_SL->GetColorIdx(CPhyloTreeScheme::eNode, CPhyloTreeScheme::eSelColor);
    }
    else {
        short node_color_idx = (**node).GetNodeFgColorIdx();
        if (node_color_idx != -1)
            return node_color_idx;
        else if ((**node).GetClusterColorIdx() != -1) //if (c == CPhyloTreeScheme::eClusterColor)
            return ((**node).GetClusterColorIdx());
        else
            return  m_SL->GetColorIdx(CPhyloTreeScheme::eNode, c);
    }
}

short IPhyloTreeRender::GetLabelColorIdx(CPhyloTreeNode* node)
{
    short label_color_idx = (**node).GetLabelFgColorIdx();
    if (label_color_idx != -1)
        return label_color_idx;

    return  m_SL->GetColorIdx(CPhyloTreeScheme::eLabel, (**node).GetColoration());
}

bool IPhyloTreeRender::GetEdgeColor(CPhyloTreeNode* child_node,
    CPhyloTreeNode* parent_node,
    CRgbaColor& c,
    CRgbaColor& parent_color)
{
    short color_idx;
    short parent_color_idx = -1;
    float alpha = 1.0f;

    bool selected_state = false;
    CPhyloTreeScheme::TPhyloTreeColors beg_state = (**parent_node).GetColoration();
    CPhyloTreeScheme::TPhyloTreeColors end_state = (**child_node).GetColoration();

    // If end state is selected or shared, use beginning state, otherwise use
    // end state (which may be the same as beginnnig state)
    CPhyloTreeScheme::TPhyloTreeColors state = end_state;

    if (beg_state != end_state &&
        (state == CPhyloTreeScheme::eSelColor || state == CPhyloTreeScheme::eSharedColor)){
        state = beg_state;
    }

    if (state == CPhyloTreeScheme::eColor) {
        color_idx = (**child_node).GetClusterColorIdx();
        if (color_idx == -1) {
            color_idx = (**child_node).GetNodeEdColorIdx();
            parent_color_idx = (**parent_node).GetNodeEdColorIdx();
        }

        if (color_idx == -1){
            color_idx = m_SL->GetColorIdx(CPhyloTreeScheme::eLine,
                                          CPhyloTreeScheme::eColor);
            c = m_DS->GetModel().GetColorTable()->GetColor(color_idx);

            // If the node is not selected or a cluster and user is highlighting longer
            // edges, lighten the color besed on the computed lenght of its edge And
            // the current edge density (highlight is applied to overlappin edges)
            if (m_HighlightLongerEdges && m_HighlightIntensity > 0.0f && m_DS->HasLenDistribution()) {
                float len = child_node->GetValue().GetEdgeScore();

                // Not used in alpha blending since blending is disabled when many edges
                // overlap - just lighten the color directly
                float transparency = 1.0f / (1.0f + powf(M_NCBI_E,
                    -m_HighlightFunctionK*(len - m_DS->GetClosestLen(m_HighlightFunctionMidpoint))));

                // m_HighlightIntensity goes from 0 (no highlight) to 1 (max highlight)
                transparency += (1.0f - transparency)*(1.0f - m_HighlightIntensity);

                c.Lighten(1.0f - transparency);
            }
        }
        else {
            c = m_DS->GetModel().GetColorTable()->GetColor(color_idx);
        }

        if (m_SL->GetSelectionVisibility() == CPhyloTreeScheme::eHighlightSelection)
            alpha = m_SL->GetNonSelectedAlpha();
    }
    else {
        color_idx = m_SL->GetColorIdx(CPhyloTreeScheme::eLine, state);
        c = m_DS->GetModel().GetColorTable()->GetColor(color_idx);
        if (state == CPhyloTreeScheme::eSelColor ||
            state == CPhyloTreeScheme::eTraceColor ||
            state == CPhyloTreeScheme::eSharedColor) {
            selected_state = true;
        }
    }
    
    c.SetAlpha(alpha);

    parent_color = c;
    if (parent_color_idx != -1 && (**child_node).GetEdgeColorGradient()) {
        parent_color = m_DS->GetModel().GetColorTable()->GetColor(parent_color_idx);
        parent_color.SetAlpha(alpha);
    }

    return selected_state;
}

TVPRect IPhyloTreeRender::GetMinDimensions(CGlPane& pane,
                                           const CPhyloTreeDataSource& ds,                             
                                           const CPhyloTreeScheme& sc,
                                           int& label_height,
                                           TModelRect& node_rect,
                                           TModelRect& text_rect,
                                           float width_factor,
                                           float height_factor,
                                           float wh_ratio)
{
    // Consolidate the extent data into two vectors - one for min/max x values and one 
    // for min/max y values.
    vector<ProjectionElement> xexts, yexts;
    m_ProjectionExtents.ConsolidateExtents(xexts, yexts);

    // Initial target height is based on the size of the model which comes from the number
    // of leaves and font size, and then that is scaled by a separation factor based on 
    // tree layout type and another separation factor that can be passed in by the client. 
    TModelUnit min_height = (m_LabelPixels*height_factor) / x_GetVerticalSeparationFactor();
    label_height = (int)min_height;
  
    // Set the initial width based on the tree height.  Width will also need to accomdate
    // labels, but those are added in by x_FindBestSize.  The 'width_factor' is a tuning
    // parameter to control how compact the nodes are horizontally (5-15 is a reasonable range)
    float tree_height = m_DS->GetWidth();
    float width = ((float)m_SL->GetNodeSize())*width_factor*tree_height;

    // Expand the width or height as needed to force the node layout to a 
    // particular ratio (including forcing the image to be square when 
    // a circular cladogram is desired)
    if (wh_ratio > 0) {
        float computed_wh_ratio = width/min_height;
        if (computed_wh_ratio < wh_ratio) {
            width = wh_ratio*min_height;
        }
        else if (computed_wh_ratio > wh_ratio) {
            min_height = width/wh_ratio;
        }
    }

    TVPRect r(0, 0, width, min_height);

    CGlPane tmp_pane = pane;
    tmp_pane.SetViewport(r);

    // LabelViewPct forces the viewport to preserve some room for geometry 
    // even with very  long labels.  But since we want to get out the viewport size
    // from x_FindBestSize rather than the visible area, we 'disable' it by
    // setting it to a large value
    float tmp = m_LabelViewPct;
    m_LabelViewPct = 1000.0f;
  
    // Now call x_FindBestSize(..) which will iterate over all the nodes, compute
    // their (x,y) viewport projection and then find how far beyond the nodes
    // the textual labels project (text_rect).
    //TModelRect node_rect;  // Returns bounding rect for all nodes
    //TModelRect text_rect;  // Returns bounding rect for all labels
    TModelRect vr = m_ZoomableExtent;
    x_FindBestSize(tmp_pane, xexts, yexts, vr, node_rect, text_rect, 1);
    m_LabelViewPct = tmp;
   
    // Get margins (blank areas) for edges of viewport (may be 0):
    GLdouble top_margin, bottom_margin, left_margin, right_margin;
    m_SL->GetMargins(top_margin, bottom_margin, left_margin, right_margin);

    // Compute the new viewport size as the size we computed before the
    // call to x_FindBestSize plus the additional pixels required on the
    // right,left,top and bottom to accomodate labels and margin (the added
    // label size is the difference in size between text_rect and node_rect)
    TModelUnit left_pixels = node_rect.Left()-text_rect.Left();
    TModelUnit right_pixels = text_rect.Right()-node_rect.Right();
    TModelUnit bottom_pixels = node_rect.Bottom()-text_rect.Bottom();
    TModelUnit top_pixels = text_rect.Top()-node_rect.Top();
    TModelUnit w = width + left_pixels + right_pixels + left_margin + right_margin; 
    TModelUnit h = min_height + top_pixels + bottom_pixels + top_margin  + bottom_margin;
   
    // If the caller wants the viewport in a particular ratio, expand
    // the width or height as needed (we do not contract since the space 
    // computed above is the minimal space for the tree in x and y) 
    if (wh_ratio > 0) {
        float computed_wh_ratio = w/h;
        if (computed_wh_ratio < wh_ratio) {
            w = wh_ratio*h;
        }
        else if (computed_wh_ratio > wh_ratio) {
            h = w/wh_ratio;
        }
    }

    r.SetRight(int(w*1.05));
    r.SetTop(int(h*1.05));

    return r;
}

int IPhyloTreeRender::GetMaxLeavesVisible()
{
    // Get required distance between nodes (based on font size)
    TModelUnit mvs = (TModelUnit)m_Label.GetMinVerticalSeparation();
    int vp_height = m_pPane->GetViewport().Height();

    GLdouble left, bottom, right, top;
    m_SL->GetMargins(left, bottom, right, top);

    // Subtract bottom and top margins from viewport height
    vp_height -= (int)(bottom + top);

    // Divide viewport height by pixels (vertical) required for each leaf node
    // and subtract a few to make sure labels are visible (otherwise exactly on edge)
    return int(TModelUnit(vp_height*x_GetVerticalSeparationFactor()) / mvs) - 5;
}


wxEvtHandler* IPhyloTreeRender::GetEvtHandler()
{
    return this;
}

float IPhyloTreeRender::GetDefaultNodeSize(const CPhyloTreeNode * node) const
{
    return node->GetValue().GetDefaultNodeSize(m_SL);
}

float IPhyloTreeRender::GetNodeLayoutSize(const CPhyloTreeNode * node) const
{
    return node->GetValue().GetNodeLayoutSize(m_SL);
}

float IPhyloTreeRender::GetNodeLabelDist(const CPhyloTreeNode* node) const
{
    float d = 0.0f;

    // For standard nodes 'd' represents the radius, but for collapsed nodes
    // it is the full size (width). That works out OK here becuase round 
    // nodes are centered on the node position but collapsed nodes only place
    // their tip there and project away. We subtract an amount that represents
    // the shift of the collapsed node toward (on top of) the node position.
    if (!node->Expanded()) {
        d = node->GetValue().GetNodeWidthScaler(m_SL.GetPointer());
        // Circular views with collapsed nodes are a bit trickier to line
        // up label and node. If distance is enabled, most collapsed nodes
        // are inside and should be placed at tip of edge. If not distance,
        // we want to retract (push in) collapsed nodes more to make labels
        // display more cleanly
        if (!GetDistRendering() && SupportsRotatedLabels()) {
            d = d - (d / 4.0f) + 1.0;
        }
        else {
            d -= 2.0f;
        }
    }
    else {
        d = GetDefaultNodeSize(node);
    }

    return d;
}

float IPhyloTreeRender::GetNodeSize(const CPhyloTreeNode* node) const
{
    float actual_size = 0.0f;

    if (m_pPane) {
        float node_size = GetDefaultNodeSize(node);
        float def_node_size = (float)(node->IsLeaf() ? m_SL->SetSize(CPhyloTreeScheme::eLeafNodeSize) :
                                                       m_SL->SetSize(CPhyloTreeScheme::eNodeSize));

        if ((m_SL->GetBoaNodes() && node->IsLeafEx() && !node->IsLeaf()) || 
            DistanceBetweenNodes()> def_node_size*2.0f) {
            actual_size = node_size;
        }
        else {
            actual_size = def_node_size;
        }
    }
    
    return actual_size;
}

END_NCBI_SCOPE
