/*  $Id: phylo_tree_boundary_shapes.cpp 42852 2019-04-19 20:43:49Z katargir $
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
#include <gui/widgets/phylo_tree/phylo_tree_boundary_shapes.hpp>
#include <gui/opengl.h>

#include <cmath>

BEGIN_NCBI_SCOPE


/*********************** IBoundaryShape **************************************/

IBoundaryShape* IBoundaryShape::CreateBoundary(const string& boundary_type)
{
    IBoundaryShape* shape = NULL;

    if (boundary_type == "Rectangle") {
        shape = new CBoundaryShapeRect();
    }
    else if (boundary_type == "RoundedRectangle") {
        shape = new CBoundaryShapeRoundedRect();
    }
    else if (boundary_type == "Triangle") {
        shape = new CBoundaryShapeTri();
    }
    else if (boundary_type == "Ellipse") {
        shape = new BoundaryShapeEllipse();
    }

    return shape;
}

void IBoundaryShape::Render(const CVect2<float>& scale, 
                            float alpha_mod,
                            bool include_labels)
{
    if (m_Hidden)
        return;

    // Needed when view is scaled (not necessarily every time)
    ComputeShapeWithLabels(scale, include_labels);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    CRgbaColor c(m_Parms.GetColor());
    c.SetAlpha(c.GetAlpha()*alpha_mod);
    glColor4fv(c.GetColorArray());
   
    vector<CVect2<float> > verts;
    x_GetTris(verts, scale);

    glBegin(GL_TRIANGLES);
    {
        for (size_t i=0; i<verts.size(); i+=3) {
            glVertex2fv(verts[i].GetData());
            glVertex2fv(verts[i+1].GetData());
            glVertex2fv(verts[i+2].GetData());
        }
    }
    glEnd();

    //Draw the border around polygons as a line
    if (m_Parms.GetDrawBoundaryEdge()) {
        glLineWidth(m_Parms.GetBorderWidth());
        CRgbaColor c(m_Parms.GetBoundaryEdgeColor());
        c.SetAlpha(c.GetAlpha()*alpha_mod);
        glColor4fv(c.GetColorArray());
               
        verts.clear();
        x_GetEdges(verts, scale);

        glBegin(GL_LINES);
        {
            for (size_t i=0; i<verts.size(); i+=2) {
                glVertex2fv(verts[i].GetData());
                glVertex2fv(verts[i+1].GetData());
            }
        }
        glEnd();
    }

    glDisable(GL_BLEND);
}

void IBoundaryShape::RenderVbo(CRef<CGlVboNode>& tri_node,
                               CRef<CGlVboNode>& edge_node,                           
                               const CVect2<float>& scale, 
                               float alpha_mod,
                               bool include_labels)
{
    if (m_Hidden)
        return;

    // Needed when view is scaled (not necessarily every time)
    ComputeShapeWithLabels(scale, include_labels);
  
    //tri_node.Reset(new CGlVboNode(GL_TRIANGLES));
    CRgbaColor c(m_Parms.GetColor());
    c.SetAlpha(c.GetAlpha()*alpha_mod);  

    tri_node->GetState().ColorC(c);
    tri_node->GetState().Enable(GL_BLEND);
    tri_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    tri_node->GetState().PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    vector<CVect2<float> > verts;
    x_GetTris(verts, scale);

    // Get bounding rectangle for boundary area. 
    TModelUnit tmax = std::numeric_limits<TModelUnit>::max();
    m_Extent.Init(tmax, tmax, -tmax, -tmax);

    for (size_t i = 0; i < verts.size(); ++i) {
        TModelUnit x = (TModelUnit)verts[i].X();
        TModelUnit y = (TModelUnit)verts[i].Y();

        m_Extent.SetLeft(std::min(x, m_Extent.Left()));
        m_Extent.SetRight(std::max(x, m_Extent.Right()));
        m_Extent.SetBottom(std::min(y, m_Extent.Bottom()));
        m_Extent.SetTop(std::max(y, m_Extent.Top()));
    }
    tri_node->SetVertexBuffer2D(verts);


    //Draw the border around the outside of the object (if border is given)
    if (m_Parms.GetDrawBoundaryEdge()) {
        CRgbaColor c(m_Parms.GetBoundaryEdgeColor());
        c.SetAlpha(c.GetAlpha()*alpha_mod);

        verts.clear();
        x_GetEdges(verts, scale);

        //edge_node.Reset(new CGlVboNode(GL_LINES));
        edge_node->GetState().ColorC(c);
        edge_node->GetState().Enable(GL_BLEND);
        edge_node->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        edge_node->GetState().LineWidth(m_Parms.GetBorderWidth());
        edge_node->GetState().LineJoinStyle(CGlState::eRoundedJoin);
        edge_node->GetState().LineCapStyle(CGlState::eRoundCap);

        edge_node->SetVertexBuffer2D(verts);
    }
}

/*********************** CBoundaryShapeRectBase **********************************/

CBoundaryShapeRectBase::CBoundaryShapeRectBase()
: m_PixelDeltaNegX(0.0f)
, m_PixelDeltaNegY(0.0f)
, m_PixelDeltaPosX(0.0f)
, m_PixelDeltaPosY(0.0f)
{
    for (size_t i=0; i<4; ++i)
        m_Points[i] = 0.0f;
}

void CBoundaryShapeRectBase::ComputeShape(const CVect2<float>& /* scale */,
                                          const CVect2<float>& /* base_node_pos */,
                                          const std::vector<CVect2<float> >& pts,                                      
                                          const vector<std::pair<CVect2<float>, CGlRect<float> > >& pixel_pts)
{
    float minx = 1e10f;
    float miny = 1e10f;
    float maxx = -1e10f;
    float maxy = -1e10f;

    for (size_t i=0; i<pts.size(); ++i) {
        minx = std::min(minx, pts[i].X());
        maxx = std::max(maxx, pts[i].X());
        miny = std::min(miny, pts[i].Y());
        maxy = std::max(maxy, pts[i].Y());
    }

    if (m_Parms.GetIncludeTextArea()) {
        for (size_t i=0; i<pixel_pts.size(); ++i) {
            minx = std::min(minx, pixel_pts[i].first.X());
            maxx = std::max(maxx, pixel_pts[i].first.X());
            miny = std::min(miny, pixel_pts[i].first.Y());
            maxy = std::max(maxy, pixel_pts[i].first.Y());
        }
    }

    m_Points[0].X() = minx;
    m_Points[0].Y() = miny;

    m_Points[1].X() = maxx;
    m_Points[1].Y() = miny;

    m_Points[2].X() = maxx;
    m_Points[2].Y() = maxy;

    m_Points[3].X() = minx;
    m_Points[3].Y() = maxy;

    m_PixelPoints = pixel_pts;
}

void CBoundaryShapeRectBase::ComputeShapeWithLabels(const CVect2<float>& scale,
                                                    bool labels_visible)
{
    // This function is needed because when the view scales, text fields
    // move to a new position(translate) but do not scale.  This means
    // that the size of the bounding area cannot simply be scaled by the
    // same amount as the underlying tree.
    
    // Recompute the size of the boudary taking into account the extent
    // of text fields associated with specific nodes (positions) in the tree.
    // The contribution of these fields is saved in the m_PixelDelta* vars.
    m_PixelDeltaNegX = 0.0f;
    m_PixelDeltaNegY = 0.0f;
    m_PixelDeltaPosX = 0.0f;    
    m_PixelDeltaPosY = 0.0f;

    if (!labels_visible)
        return;

    float sx = scale.X();
    float sy = scale.Y();

    // Iterate over the node points and their associated text
    // rectangles.  Node positions are in normal (scaled) coordinates
    // and text field dimensions are in absolute (pixel) coordinates.
    for (size_t i=0; i<m_PixelPoints.size(); ++i) {
        // a position in model coordinates:
        CVect2<float> base_pos = m_PixelPoints[i].first;
        // area of rectangle for the text field (in pixels)
        CGlRect<float> pix_rect = m_PixelPoints[i].second;

        // For each of the 4 corners of the text field rectangle, determine
        // if it outside the bounds of the current boundary and if so save
        // that (scaled) amount in m_PixelDeltaNeg*
        float negx = 1e10f;
        float negy = 1e10f;
        float posx = -1e10f;
        float posy = -1e10f;

        for (int i=0; i<4; ++i) {
            CGlPoint<float> pix_pos(pix_rect.GetCorner(i));

            negx = std::min(negx, pix_pos.X());
            posx = std::max(posx, pix_pos.X());
            negy = std::min(negy, pix_pos.Y());
            posy = std::max(posy, pix_pos.Y());
        }

        m_PixelDeltaNegX = std::max(m_PixelDeltaNegX, m_Points[0].X() - base_pos.X() - negx*sx );
        m_PixelDeltaNegY = std::max(m_PixelDeltaNegY, m_Points[0].Y() - base_pos.Y() - negy*sy);
        m_PixelDeltaPosX = std::max(m_PixelDeltaPosX, base_pos.X() + posx*sx - m_Points[2].X());
        m_PixelDeltaPosY = std::max(m_PixelDeltaPosY, base_pos.Y() + posy*sy - m_Points[2].Y());
    }
}

/*********************** CBoundaryShapeRect **********************************/

void CBoundaryShapeRect::x_GetTris(vector<CVect2<float> >& tris, 
                                   const CVect2<float>& scale)
{
    float border_neg_x = m_PixelDeltaNegX + scale.X()*(m_Parms.GetBorderWidth());
    float border_neg_y = m_PixelDeltaNegY + scale.Y()*(m_Parms.GetBorderWidth());
    float border_pos_x = m_PixelDeltaPosX + scale.X()*(m_Parms.GetBorderWidth());
    float border_pos_y = m_PixelDeltaPosY + scale.Y()*(m_Parms.GetBorderWidth());

    tris.push_back(CVect2<float>(m_Points[0].X() - border_neg_x, 
                                 m_Points[0].Y() - border_neg_y));
    tris.push_back(CVect2<float>(m_Points[1].X() + border_pos_x, 
                                 m_Points[1].Y() - border_neg_y));
    tris.push_back(CVect2<float>(m_Points[3].X() - border_neg_x, 
                                 m_Points[3].Y() + border_pos_y));

    tris.push_back(CVect2<float>(m_Points[1].X() + border_pos_x, 
                                 m_Points[1].Y() - border_neg_y));
    tris.push_back(CVect2<float>(m_Points[2].X() + border_pos_x, 
                                 m_Points[2].Y() + border_pos_y));
    tris.push_back(CVect2<float>(m_Points[3].X() - border_neg_x, 
                                 m_Points[3].Y() + border_pos_y));
}

void CBoundaryShapeRect::x_GetEdges(vector<CVect2<float> >& edges, 
                                    const CVect2<float>& scale)
{
    //Draw the border around the outside of the object (if border is given)
    if (m_Parms.GetDrawBoundaryEdge()) {
        float border_neg_x = m_PixelDeltaNegX + scale.X()*(m_Parms.GetBorderWidth() + 1);
        float border_neg_y = m_PixelDeltaNegY + scale.Y()*(m_Parms.GetBorderWidth() + 1);
        float border_pos_x = m_PixelDeltaPosX + scale.X()*(m_Parms.GetBorderWidth() + 1);
        float border_pos_y = m_PixelDeltaPosY + scale.Y()*(m_Parms.GetBorderWidth() + 1);

        edges.push_back(CVect2<float>(m_Points[0].X() - border_neg_x, 
                                      m_Points[0].Y() - border_neg_y));
        edges.push_back(CVect2<float>(m_Points[1].X() + border_pos_x, 
                                      m_Points[1].Y() - border_neg_y));

        edges.push_back(CVect2<float>(m_Points[1].X() + border_pos_x, 
                                      m_Points[1].Y() - border_neg_y));
        edges.push_back(CVect2<float>(m_Points[2].X() + border_pos_x, 
                                      m_Points[2].Y() + border_pos_y));

        edges.push_back(CVect2<float>(m_Points[2].X() + border_pos_x, 
                                      m_Points[2].Y() + border_pos_y));
        edges.push_back(CVect2<float>(m_Points[3].X() - border_neg_x, 
                                      m_Points[3].Y() + border_pos_y));

        edges.push_back(CVect2<float>(m_Points[3].X() - border_neg_x, 
                                      m_Points[3].Y() + border_pos_y));
        edges.push_back(CVect2<float>(m_Points[0].X() - border_neg_x, 
                                      m_Points[0].Y() - border_neg_y));
    }
}


/*********************** CBoundaryShapeRoundedRect ***************************/

void CBoundaryShapeRoundedRect::x_GetRoudedCornerTris(vector<CVect2<float> >& tris,
                                                      const CVect2<float>& pos,
                                                      float corner_width_x,
                                                      float corner_width_y,
                                                      float start_angle)
{     
    float tri_count = 10.0f;
    float delta = (3.1415926535f/2.0f)/tri_count;
    float angle = start_angle;
    CVect2<float> prev_pt, pt;

    for(int i =0; i<=tri_count; i++){
        prev_pt = pt;
        pt.Set(pos.X() + corner_width_x*cosf(angle), 
               pos.Y() + corner_width_y*sinf(angle));

        if (i>0) {
            tris.push_back(pos);
            tris.push_back(prev_pt);
            tris.push_back(pt);
        }
        angle += delta;
    }
}

void CBoundaryShapeRoundedRect::x_GetRoudedCornerEdges(vector<CVect2<float> >& edges,
                                                       const CVect2<float>& pos,
                                                       float corner_width_x,
                                                       float corner_width_y,
                                                       float start_angle)
{
    float tri_count = 10.0f;
    float delta = (3.1415926535f/2.0f)/tri_count;
    float angle = start_angle;
    CVect2<float> prev_pt, pt;

    for(int i =0; i<=tri_count; i++){
        prev_pt = pt;
        pt.Set(pos.X() + corner_width_x*cosf(angle), 
               pos.Y() + corner_width_y*sinf(angle));

        if (i>0) {
            edges.push_back(prev_pt);
            edges.push_back(pt);
        }
        angle += delta;
    }
}


void CBoundaryShapeRoundedRect::x_GetTris(vector<CVect2<float> >& tris, 
                                          const CVect2<float>& scale)
{
    // If border is not equal to at least the corner size, then elements may 
    // wind up outside the bounded area.
    float border_width = std::max(m_Parms.GetBorderWidth(), m_Parms.GetCornerWidth());

    float border_neg_x = m_PixelDeltaNegX + scale.X()*(border_width);
    float border_neg_y = m_PixelDeltaNegY + scale.Y()*(border_width);
    float border_pos_x = m_PixelDeltaPosX + scale.X()*(border_width);
    float border_pos_y = m_PixelDeltaPosY + scale.Y()*(border_width);

    float corner_width_x = scale.X()*m_Parms.GetCornerWidth();
    float corner_width_y = scale.Y()*m_Parms.GetCornerWidth();

    // Maximum size for the rounded corners is 1/2 distance in x or y direction
    // otherwise we get overlap (which looks bad)
    // Get min dimension (x or y)
    float xmin = std::min(m_Points[0].X(), m_Points[2].X()) - border_neg_x;
    float xmax = std::max(m_Points[0].X(), m_Points[2].X()) + border_pos_x;
    float ymin = std::min(m_Points[0].Y(), m_Points[2].Y()) - border_neg_y;
    float ymax = std::max(m_Points[0].Y(), m_Points[2].Y()) + border_pos_y;

    float xw = xmax-xmin;
    float yw = ymax-ymin;

    if (corner_width_x * 2.0f > xw)
        corner_width_x = xw*0.5f;
    if (corner_width_y * 2.0f > yw)
        corner_width_y = yw*0.5f;


    /* draw rounded rectangle in 7 parts: 3 rectangles and 4 corners
     * 
     *     /-----------------------\
     *    /4 |                   |7 \
     *   |---|    1.             |---|
     *   |_2_|                   |3__| 
     *    \5 |                   |6 /
     *     \-----------------------/
     */

    // First quad (two tris, lower left and upper right)
    tris.push_back(CVect2<float>(xmin + corner_width_x, ymin));
    tris.push_back(CVect2<float>(xmax - corner_width_x, ymin));
    tris.push_back(CVect2<float>(xmax - corner_width_x, ymax));

    tris.push_back(CVect2<float>(xmax - corner_width_x, ymax));
    tris.push_back(CVect2<float>(xmin + corner_width_x, ymax));
    tris.push_back(CVect2<float>(xmin + corner_width_x, ymin));

    // Second quad (as two tris)
    tris.push_back(CVect2<float>(xmin, ymin + corner_width_y));
    tris.push_back(CVect2<float>(xmin + corner_width_x, ymin + corner_width_y));
    tris.push_back(CVect2<float>(xmin + corner_width_x, ymax - corner_width_y));

    tris.push_back(CVect2<float>(xmin + corner_width_x, ymax - corner_width_y));
    tris.push_back(CVect2<float>(xmin, ymax - corner_width_y));
    tris.push_back(CVect2<float>(xmin, ymin + corner_width_y));

    // third quad (as two tris)
    tris.push_back(CVect2<float>(xmax - corner_width_x, ymin + corner_width_y));
    tris.push_back(CVect2<float>(xmax, ymin + corner_width_y));
    tris.push_back(CVect2<float>(xmax, ymax - corner_width_y));

    tris.push_back(CVect2<float>(xmax, ymax - corner_width_y));
    tris.push_back(CVect2<float>(xmax - corner_width_x, ymax - corner_width_y));
    tris.push_back(CVect2<float>(xmax - corner_width_x, ymin + corner_width_y));


    float pi = 3.14159265358979f;

    CVect2<float> pos(xmin + corner_width_x, ymin + corner_width_y);
    x_GetRoudedCornerTris(tris, pos, corner_width_x, corner_width_y, pi);

    pos.X() = xmax - corner_width_x;
    x_GetRoudedCornerTris(tris, pos, corner_width_x, corner_width_y, (3.0f*pi)/2.0f);
    
    pos.Y() = ymax - corner_width_y;
    x_GetRoudedCornerTris(tris, pos, corner_width_x, corner_width_y, 0.0f);

    pos.X() = xmin + corner_width_x;
    x_GetRoudedCornerTris(tris, pos, corner_width_x, corner_width_y, pi/2.0f);
}

void CBoundaryShapeRoundedRect::x_GetEdges(vector<CVect2<float> >& edges, 
                                           const CVect2<float>& scale)
{
    // If border is not equal to at least the corner size, then elements may 
    // wind up outside the bounded area.
    float border_width = std::max(m_Parms.GetBorderWidth(), m_Parms.GetCornerWidth());

    float border_neg_x = m_PixelDeltaNegX + scale.X()*(border_width);
    float border_neg_y = m_PixelDeltaNegY + scale.Y()*(border_width);
    float border_pos_x = m_PixelDeltaPosX + scale.X()*(border_width);
    float border_pos_y = m_PixelDeltaPosY + scale.Y()*(border_width);

    float corner_width_x = scale.X()*m_Parms.GetCornerWidth();
    float corner_width_y = scale.Y()*m_Parms.GetCornerWidth();

    // Maximum size for the rounded corners is 1/2 distance in x or y direction
    // otherwise we get overlap (which looks bad)
    // Get min dimension (x or y)
    float xmin = std::min(m_Points[0].X(), m_Points[2].X()) - border_neg_x;
    float xmax = std::max(m_Points[0].X(), m_Points[2].X()) + border_pos_x;
    float ymin = std::min(m_Points[0].Y(), m_Points[2].Y()) - border_neg_y;
    float ymax = std::max(m_Points[0].Y(), m_Points[2].Y()) + border_pos_y;

    float xw = xmax-xmin;
    float yw = ymax-ymin;

    if (corner_width_x * 2.0f > xw)
        corner_width_x = xw*0.5f;
    if (corner_width_y * 2.0f > yw)
        corner_width_y = yw*0.5f;


    /* draw rounded rectangle in 7 parts: 3 rectangles and 4 corners
     * 
     *     /-----------------------\
     *    /4 |                   |7 \
     *   |---|    1.             |---|
     *   |_2_|                   |3__| 
     *    \5 |                   |6 /
     *     \-----------------------/
     */

    // First quad - top and bottom edges
    edges.push_back(CVect2<float>(xmin + corner_width_x, ymin));
    edges.push_back(CVect2<float>(xmax - corner_width_x, ymin));

    edges.push_back(CVect2<float>(xmax - corner_width_x, ymax));
    edges.push_back(CVect2<float>(xmin + corner_width_x, ymax));

    // Second quad - left edge
    edges.push_back(CVect2<float>(xmin , ymax - corner_width_y));
    edges.push_back(CVect2<float>(xmin, ymin + corner_width_y));

    // third quad - right edge
    edges.push_back(CVect2<float>(xmax, ymin + corner_width_y));
    edges.push_back(CVect2<float>(xmax, ymax - corner_width_y));

    float pi = 3.14159265358979f;

    CVect2<float> pos(xmin + corner_width_x, ymin + corner_width_y);
    x_GetRoudedCornerEdges(edges, pos, corner_width_x, corner_width_y, pi);

    pos.X() = xmax - corner_width_x;
    x_GetRoudedCornerEdges(edges, pos, corner_width_x, corner_width_y, (3.0f*pi)/2.0f);
    
    pos.Y() = ymax - corner_width_y;
    x_GetRoudedCornerEdges(edges, pos, corner_width_x, corner_width_y, 0.0f);

    pos.X() = xmin + corner_width_x;
    x_GetRoudedCornerEdges(edges, pos, corner_width_x, corner_width_y, pi/2.0f);
}


/*********************** CBoundaryShapeTri ***********************************/

CBoundaryShapeTri::CBoundaryShapeTri()
{
}

void CBoundaryShapeTri::ComputeShape(const CVect2<float>& /* scale */,
                                     const CVect2<float>& base_node_pos,                                  
                                     const std::vector<CVect2<float> >& pts,                    
                                     const vector<std::pair<CVect2<float>, CGlRect<float> > >& pixel_pts) 
{
    // Pixel points are the points that define text box areas
    if (m_Parms.GetIncludeTextArea() || m_Parms.GetTextBox()) {
        m_PixelPoints = pixel_pts;
    }

    m_NodePoints = pts;
    m_BaseNodePos = base_node_pos;

    // Remove the base (apex) node from the main set of nodes since it has a fixed position
    // in the triangle and it can mess up the math (gets subtracted from itself and creates
    // 0 length vector. Its probably the first node, but check all.
    std::vector<CVect2<float> >::iterator iter;
    for (iter=m_NodePoints.begin(); iter!=m_NodePoints.end(); ++iter) {
        if ( ((*iter)-m_BaseNodePos).Length() < std::numeric_limits<float>::epsilon())
            break;
    }

    if (iter != m_NodePoints.end() )
        m_NodePoints.erase(iter);
}

void CBoundaryShapeTri::x_ComputeTriParms(const std::vector<CVect2<float> >& pts,  
                                          const CVect2<float>& base_pos,
                                          const CVect2<float>& dir,
                                          const CVect2<float>& perp_dir,
                                          float& max_len,
                                          float& max_angle_top,
                                          float& max_angle_bottom)
{
    // Compute angle of all points from base line segment and save result in
    // max_angle_top and max_angle_bottom.  Top/bottom is relative to vector
    // perpendicular to main direction of triangle (perp_dir)
    for (size_t i=0; i<pts.size(); ++i) {     
        CVect2<float> pt_dir = pts[i] - base_pos;       
        float proj_len = pt_dir.Dot(dir);
        max_len = std::max(max_len, proj_len);

        // If the offset from the base position is < 0, then some of the points
        // are behind the apex of the triangle, and the shape won't work.  Throw
        // out nodes behind the triangle so we can show something (this could occur
        // in physically-based graph layout)

        if (max_len > 0.0f) {
            pt_dir.Normalize();
            float cos_angle = pt_dir.Dot(dir);

            // clamp to -1.0 .. 1.0 to prevent numeric precision problems
            cos_angle = cos_angle < -1.0f ? -1.0f : 1.0f < cos_angle ? 1.0f : cos_angle;

            float angle = acosf(cos_angle);
            bool pos_angle = (perp_dir.Dot(pt_dir) > 0.0f);
            if (pos_angle) {
                max_angle_top = std::max(angle, max_angle_top);
                max_angle_bottom = std::max(-angle, max_angle_bottom);
            }
            else {
                max_angle_bottom = std::max(angle, max_angle_bottom);
                max_angle_top = std::max(-angle, max_angle_top);
            }
        }
    }
}

CBoundaryShapeTri::RoundedCorner 
CBoundaryShapeTri::x_ComputeRoundedCorner(const CVect2<float>& pt_in,
                                          const CVect2<float>& prev_pt_in,
                                          const CVect2<float>& next_pt_in,
                                          const CVect2<float>& scale,
                                          CVect2<float>& pt_out1,
                                          CVect2<float>& pt_out2)
{
    CVect2<float> pdir;
    CVect2<float> s1, s2;
    float c, scale_modifier, theta;
    float a = m_Parms.GetCornerWidth();

    // Get vectors going away from the vertex being 'rounded' (these are vectors to
    // adjacent vertices).
    s1 = prev_pt_in-pt_in;
    float s1_len = s1.Length();
    s1 /= s1_len;   

    s2 = next_pt_in-pt_in;
    float s2_len = s2.Length();
    s2 /= s2_len;

    // Compute the angle as it appears in pixel coordinates (scaled) not using model
    // coordinates which would be acos(s1.Dot(s2)).   
    CVect2<float> s1_scaled = CVect2<float>(s1.X()*1.0f/scale.X(), s1.Y()*1.0f/scale.Y());
    CVect2<float> s2_scaled = CVect2<float>(s2.X()*1.0f/scale.X(), s2.Y()*1.0f/scale.Y());    
    s1_scaled.Normalize();
    s2_scaled.Normalize();
    theta = acos(s1_scaled.Dot(s2_scaled));

    RoundedCorner rc;   
    
    // To round the corner, we find the two points where that same perpendicular
    // segment of length 'a' will intersect the two vectors to neighboring 
    // vertices.  The distance is based on the right triangle formed by
    // the vector to the adjacent vertex, the line segemnt of length 'a'
    // and the line bisecting the angle of length 'b'.  So the distance
    // to the intersection is 'c' from:  sin(theta/2) = (a/2)/c
    c = (a/2.0f)/sin(theta/2.0f);

    float scale_modifier1 = 1.0f/CVect2<float>(s1.X()/scale.X(), s1.Y()/scale.Y()).Length();
    float scale_modifier2 = 1.0f/CVect2<float>(s2.X()/scale.X(), s2.Y()/scale.Y()).Length();

    if (c*scale_modifier1 > s1_len*0.5f)
        c = (s1_len*0.5f)/scale_modifier1;
    if (c*scale_modifier2 > s2_len*0.5f)
        c = (s2_len*0.5f)/scale_modifier2;
   
    // Use c (scaled for pixel coordinates) to get intersection along the 
    // two vectors that make up the angle:
    pt_out1 = pt_in + s1*c*scale_modifier1; 
    pt_out2 = pt_in + s2*c*scale_modifier2;

    // We need to find the center and radius of the circle which, when drawn,
    // will create the rounded corner.  This is the circle that is tangent to
    // the interior of the angle at points pt_out1 and pt_out2, so the vector
    // to the center of this circle goes through the midpoint of the line
    // segment (pt_out1, pt_out2).  The length of the vector OP where P is
    // pt_in and O is the circle center is: cos(theta/2)=c/OP and radius
    // is the other side of the triangle: sin(theta/2)=r/OP.
    pdir = ((pt_out1+pt_out2)*0.5f)-pt_in;
    float OP = c/cosf(theta/2.0f);
    float r = sinf(theta/2)*OP;
    pdir.Normalize();
    scale_modifier = 1.0f/CVect2<float>(pdir.X()/scale.X(), pdir.Y()/scale.Y()).Length();    
   
    rc.pos = pt_in + pdir*OP*scale_modifier;
    rc.initial_vertex = pt_in;
    rc.intersection1 = pt_out1;
    rc.intersection2 = pt_out2;
    rc.corner_width_x = scale.X()*r;
    rc.corner_width_y = scale.Y()*r;

    return rc;
}


void CBoundaryShapeTri::ComputeShapeWithLabels(const CVect2<float>& scale,
                                               bool labels_visible)
{
    // Get all text positions based on current scale factor
    std::vector<CVect2<float> > text_pts;

    // Iterate over the node points and their associated text
    // rectangles.  Node positions are in normal (scaled) coordinates
    // and text field dimensions are in absolute (pixel) coordinates.
    size_t i;

    if (labels_visible) {
        for (i = 0; i < m_PixelPoints.size(); ++i) {
            // a position in model coordinates:
            CVect2<float> base_pos = m_PixelPoints[i].first;
            // area of rectangle for the text field (in pixels)
            CGlRect<float> pix_rect = m_PixelPoints[i].second;

            for (int i = 0; i < 4; ++i) {
                CGlPoint<float> pix_pos(pix_rect.GetCorner(i));
                text_pts.push_back(CVect2<float>(base_pos.X() + pix_pos.X()*scale.X(),
                    base_pos.Y() + pix_pos.Y()*scale.Y()));
            }
        }
    }

    //  Shouldn't be called with an empty area
    if (m_NodePoints.size() == 0 && text_pts.size() == 0)
        return;

    //*************************************************************************
    // Need to find main axis of triangle.  If the the triangle is axis aligned
    // use the centroid to pick the major axis direction. If not axis aligned,
    // pick the axis to be the direction to the parent node (if direction to 
    // parent node is not defined, as is the case for the root node of the tree,
    // also use the centroid).     
    //*************************************************************************
    CVect2<float> axis_dir;
    CVect2<float> centroid(0.0f);
    CVect2<float> minpos(m_BaseNodePos);
    CVect2<float> maxpos(m_BaseNodePos);

    if (m_NodePoints.size() > 0) {
        for (i=0; i<m_NodePoints.size(); ++i) {
            centroid += m_NodePoints[i];
            minpos.X() = std::min(minpos.X(), m_NodePoints[i].X());
            minpos.Y() = std::min(minpos.Y(), m_NodePoints[i].Y());

            maxpos.X() = std::max(maxpos.X(), m_NodePoints[i].X());
            maxpos.Y() = std::max(maxpos.Y(), m_NodePoints[i].Y());
        }
    }

    for (i=0; i<text_pts.size(); ++i) {
        centroid += text_pts[i];
    }

    centroid /= (float)(m_NodePoints.size() + text_pts.size());

    // Compute position of base line segment (non-horizontal case) as b1..b2
    axis_dir = (centroid - m_BaseNodePos);
    axis_dir.Normalize();

    // find bounding rectangle.  main vertex is at (closest) to top/bottom/left or right
    // and primary axis is in opposite direction

    //  When there is only text the wrong axis may be picked.
    if (m_Parms.GetAxisAligned() && m_NodePoints.size() > 0) {
        // Make the primary axis of the triangle based on whether m_BaseNodePos
        // is closer to the top, bottom, left or right side of the triangle.
        float d1 = std::abs(m_BaseNodePos.X()-minpos.X());
        float d2 = std::abs(m_BaseNodePos.X()-maxpos.X());
        float d3 = std::abs(m_BaseNodePos.Y()-minpos.Y());
        float d4 = std::abs(m_BaseNodePos.Y()-maxpos.Y());

        if (d1 < d2 && d1 < d3 && d1 < d4) {
            axis_dir = CVect2<float>(1.0f, 0.0f);
        }
        else if (d2 < d3 && d2 < d4) {
            axis_dir = CVect2<float>(-1.0f, 0.0f);
        }
        else if (d3 < d4) {
            axis_dir = CVect2<float>(0.0f, 1.0f);
        }
        else {
            axis_dir = CVect2<float>(0.0f, -1.0f);
        }
    }

    // Get a perpendicular vector that points in a counter-clockwise 
    // direction with respect to axis_dir. This is always true if
    // we pick (-y,x).
    CVect2<float> perp_dir(-axis_dir.Y(), axis_dir.X());

    CVect2<float> base_dir = axis_dir*m_Parms.GetTriOffset();
    //float scale_modifier = 1.0f/CVect2<float>(base_dir.X()/scale.X(), base_dir.Y()/scale.Y()).Length();
    CVect2<float> base_pos = m_BaseNodePos - base_dir;


    // Compute angle of all points relative to the axis direction of the
    // triangle and return the maximum values in max_angle_[top,bottom].
    float max_len = -1e10f;
    float max_angle_top = 0.0f;
    float max_angle_bottom = 0.0f;
    
    if (!m_Parms.GetTextBox()) {
        x_ComputeTriParms(text_pts, 
                          base_pos, 
                          axis_dir, 
                          perp_dir,
                          max_len,
                          max_angle_top, 
                          max_angle_bottom);
    }

    x_ComputeTriParms(m_NodePoints, 
                      base_pos, 
                      axis_dir, 
                      perp_dir,
                      max_len,
                      max_angle_top,
                      max_angle_bottom);
    

    CVect2<float> p1, p2, p3;

    p1 = base_pos;  
    float a;

    if (m_Parms.GetAxisAligned() && m_NodePoints.size() > 0) {
        float angle = std::max(max_angle_top, max_angle_bottom);

        a = max_len*tanf(angle);
        p2 = base_pos + axis_dir*max_len - perp_dir*a;
        p3 = base_pos + axis_dir*max_len + perp_dir*a;
    }
    else  {
        a = tanf(max_angle_bottom);
        p2 = base_pos + axis_dir - perp_dir*a;

        a = tanf(max_angle_top);   
        p3 = base_pos + axis_dir + perp_dir*a;

        CVect2<float> v1 = p3-p1;
        v1.Normalize();
        CVect2<float> v2 = p2-p1;
        v2.Normalize();

        CVect2<float> bisector = ((p1+v1) + (p1+v2))*0.5f - p1;
        bisector.Normalize();

        max_len = 0.0f;
        for (i=0; i<m_NodePoints.size(); ++i) {     
            CVect2<float> pt_dir = m_NodePoints[i] - p1;    
            float proj_len = pt_dir.Dot(bisector);
            max_len = std::max(max_len, proj_len); 
        }

        if (m_Parms.GetIncludeTextArea() && 
            !m_Parms.GetTextBox()) {

            for (i=0; i<text_pts.size(); ++i) {     
                CVect2<float> pt_dir = text_pts[i] - p1;    
                float proj_len = pt_dir.Dot(bisector);
                max_len = std::max(max_len, proj_len); 
            }
        }

        float c = max_len/v1.Dot(bisector);
        p3 = p1 + v1*c;

        c = max_len/v2.Dot(bisector);
        p2 = p1 + v2*c;
    }


    //*************************************************************************
    // Expand triangle by border width
    //*************************************************************************
    // find tri center and translate
    CVect2<float> tri_centroid = (p1 + p2 + p3)*0.3333f;
    p1 -= tri_centroid;
    p2 -= tri_centroid;
    p3 -= tri_centroid;

    m_Center = tri_centroid;

    // Expand each vertex away from the center by 'border' PIXELS (scale
    // each vector to convert distance to pixels)
    CVect2<float> bdir;
    float scaled_border;

    bdir = p1;
    bdir.Normalize();
    scaled_border = m_Parms.GetBorderWidth()/CVect2<float>(bdir.X()/scale.X(), bdir.Y()/scale.Y()).Length(); 
    p1 += bdir*scaled_border;

    bdir = p2;
    bdir.Normalize();
    scaled_border = m_Parms.GetBorderWidth()/CVect2<float>(bdir.X()/scale.X(), bdir.Y()/scale.Y()).Length();
    p2 += bdir*scaled_border;

    bdir = p3;
    bdir.Normalize();
    scaled_border = m_Parms.GetBorderWidth()/CVect2<float>(bdir.X()/scale.X(), bdir.Y()/scale.Y()).Length();
    p3 += bdir*scaled_border;

    p1 += tri_centroid;
    p2 += tri_centroid;
    p3 += tri_centroid;

    //*************************************************************************
    // Define rounded corners for triangle
    // Corners will not appear rounded if m_Parms.GetCornerWidth() == 0,
    // but we do the same processing either way since the duplicate
    // points won't hurt.
    //*************************************************************************
    m_RoundedCorners.clear();
    RoundedCorner rc;
    CVect2<float> clipped_corner1, clipped_corner2;

    // If the text is not added, two quads will define bounding area, with each
    // vertex split into 2 to that we can optionally have rounded corners.
    if (!m_Parms.GetTextBox()) {                              
        rc = x_ComputeRoundedCorner(p1, p3, p2, scale, clipped_corner1, clipped_corner2);
        m_RoundedCorners.push_back(rc);
        m_Rect1[0] = clipped_corner1;
        m_Rect1[1] = clipped_corner2;

        rc = x_ComputeRoundedCorner(p2, p1, p3, 
                                    scale, clipped_corner1, clipped_corner2);
        m_Rect1[2] = clipped_corner1;
        m_Rect1[3] = clipped_corner2;
        m_Rect2[0] = clipped_corner2;
        m_RoundedCorners.push_back(rc);

        rc = x_ComputeRoundedCorner(p3, p2, p1,
                                    scale, clipped_corner1, clipped_corner2);
        m_RoundedCorners.push_back(rc);
        m_Rect2[1] = clipped_corner1;
        m_Rect2[2] = clipped_corner2;
        m_Rect2[3] = m_Rect1[0];
    }
    else {
        text_pts.push_back(p2);
        text_pts.push_back(p3);

        float text_minx = 1e10f;
        float text_miny = 1e10f;
        float text_maxx = -1e10f;
        float text_maxy = -1e10f;

        for (i=0; i<text_pts.size(); ++i) {
            text_minx = std::min(text_minx, text_pts[i].X());
            text_maxx = std::max(text_maxx, text_pts[i].X());
            text_miny = std::min(text_miny, text_pts[i].Y());
            text_maxy = std::max(text_maxy, text_pts[i].Y());
        }
        
        float border_x = scale.X()*(m_Parms.GetBorderWidth());
        float border_y = scale.Y()*(m_Parms.GetBorderWidth());

        text_minx -= border_x;
        text_maxx += border_x;
        text_miny -= border_y;
        text_maxy += border_y;

        // Try to merge triangle corners (but not the parent nodes corner) with
        // the text box corners that are closest. Will not work well for vertically
        // oriented trees (should probably not include text in that case)
        if (axis_dir.X() >= 0.0f) {
            m_TextBox[0].X() = text_minx;
            m_TextBox[0].Y() = text_miny;
    
            m_TextBox[1].X() = text_maxx;
            m_TextBox[1].Y() = text_miny;

            m_TextBox[2].X() = text_maxx;
            m_TextBox[2].Y() = text_maxy;

            m_TextBox[3].X() = text_minx;
            m_TextBox[3].Y() = text_maxy;

            if (text_maxy > p3.Y())
                p3.Y() = text_maxy;
            else
                text_maxy = p3.Y();

            if (text_miny < p2.Y())
                p2.Y() = text_miny;
            else
                 text_miny = p2.Y();
               
            m_TextBox[0].X() = p2.X();
            m_TextBox[3].X() = p3.X();
        }
        else {
            m_TextBox[0].X() = text_maxx;
            m_TextBox[0].Y() = text_maxy;
    
            m_TextBox[1].X() = text_minx;
            m_TextBox[1].Y() = text_maxy;

            m_TextBox[2].X() = text_minx;
            m_TextBox[2].Y() = text_miny;

            m_TextBox[3].X() = text_maxx;
            m_TextBox[3].Y() = text_miny;

            if (text_maxy > p2.Y())
                p2.Y() = text_maxy;
            else
                text_maxy = p2.Y();

            if (text_miny < p3.Y())
                p3.Y() = text_miny;
            else
                 text_miny = p3.Y();
               
            m_TextBox[0].X() = p2.X();
            m_TextBox[3].X() = p3.X();
        }
      
        rc = x_ComputeRoundedCorner(p1, p3, p2, scale, clipped_corner1, clipped_corner2);
        m_Rect1[0] = clipped_corner1;
        m_Rect1[1] = clipped_corner2;
        m_Rect1[2] = p2;
        m_Rect1[3] = p3;
        m_RoundedCorners.push_back(rc);

        rc = x_ComputeRoundedCorner(m_TextBox[1], m_TextBox[0], m_TextBox[2], 
            scale, clipped_corner1, clipped_corner2);
        m_Rect2[0] = p2;
        m_Rect2[1] = clipped_corner1;
        m_Rect2[2] = clipped_corner2;
        m_RoundedCorners.push_back(rc);      

        rc = x_ComputeRoundedCorner(m_TextBox[2], m_TextBox[1], m_TextBox[3], 
            scale, clipped_corner1, clipped_corner2);
        m_Rect2[3] = clipped_corner1;
        m_Rect3[0] = clipped_corner1;
        m_Rect3[1] = clipped_corner2;
        m_Rect3[2] = p3;
        m_Rect3[3] = p2;
        m_RoundedCorners.push_back(rc); 
    }    

    // Compute the center and area of the shape, including text box if 
    // applicable.  This information is used to scale the shape when 
    // drawing the (optional) border
    float minx = 1e10f;
    float miny = 1e10f;
    float maxx = -1e10f;
    float maxy = -1e10f;

    for (int idx=0; idx<4; ++idx) {
        minx = std::min(minx, m_Rect1[idx].X());
        minx = std::min(minx, m_Rect2[idx].X());          
        maxx = std::max(maxx, m_Rect1[idx].X());
        maxx = std::max(maxx, m_Rect2[idx].X());

        miny = std::min(miny, m_Rect1[idx].Y());
        miny = std::min(miny, m_Rect2[idx].Y());          
        maxy = std::max(maxy, m_Rect1[idx].Y());
        maxy = std::max(maxy, m_Rect2[idx].Y());

        if (m_Parms.GetTextBox()) {
            minx = std::min(minx, m_Rect3[idx].X());
            maxx = std::max(maxx, m_Rect3[idx].X());

            miny = std::min(miny, m_Rect3[idx].Y());
            maxy = std::max(maxy, m_Rect3[idx].Y()); 
        }
    }
    m_Center.X() = (maxx + minx)/2.0f;
    m_Center.Y() = (maxy + miny)/2.0f;
    m_Width = maxx-minx;
    m_Height = maxy-miny;

}



void CBoundaryShapeTri::x_GetTris(vector<CVect2<float> >& tris, 
                                  const CVect2<float>& scale)
{
    // Front quad with clipped front tri if we have a text box.
    // If not, this quad has the 4 vertices formed by the clipped
    // polygon tip and clipped lower corner with top of front
    // clip being first vertex.
    //  /|
    // | |
    //  \|
    tris.push_back(CVect2<float>(m_Rect1[0].X(), m_Rect1[0].Y()));
    tris.push_back(CVect2<float>(m_Rect1[1].X(), m_Rect1[1].Y()));
    tris.push_back(CVect2<float>(m_Rect1[2].X(), m_Rect1[2].Y()));

    tris.push_back(CVect2<float>(m_Rect1[2].X(), m_Rect1[2].Y()));
    tris.push_back(CVect2<float>(m_Rect1[3].X(), m_Rect1[3].Y()));
    tris.push_back(CVect2<float>(m_Rect1[0].X(), m_Rect1[0].Y()));

    // With a text box, this is the the lower right
    // triangle (with lower right corner clipped to 
    // make it a quad) of the text box.
    // without a text box it is the upper (large) quad
    // of the main clipped triangle (3 clips make 6 vertices)
    tris.push_back(CVect2<float>(m_Rect2[0].X(), m_Rect2[0].Y()));
    tris.push_back(CVect2<float>(m_Rect2[1].X(), m_Rect2[1].Y()));
    tris.push_back(CVect2<float>(m_Rect2[2].X(), m_Rect2[2].Y()));

    tris.push_back(CVect2<float>(m_Rect2[2].X(), m_Rect2[2].Y()));
    tris.push_back(CVect2<float>(m_Rect2[3].X(), m_Rect2[3].Y()));
    tris.push_back(CVect2<float>(m_Rect2[0].X(), m_Rect2[0].Y()));

    // This is the upper left 'triangle' of the text box.  Acutally 
    // a quad because the included upper right corner is clipped
    if (m_Parms.GetTextBox()) {         
        tris.push_back(CVect2<float>(m_Rect3[0].X(), m_Rect3[0].Y()));
        tris.push_back(CVect2<float>(m_Rect3[1].X(), m_Rect3[1].Y()));
        tris.push_back(CVect2<float>(m_Rect3[2].X(), m_Rect3[2].Y()));

        tris.push_back(CVect2<float>(m_Rect3[2].X(), m_Rect3[2].Y()));
        tris.push_back(CVect2<float>(m_Rect3[3].X(), m_Rect3[3].Y()));
        tris.push_back(CVect2<float>(m_Rect3[0].X(), m_Rect3[0].Y()));
    }   

    // If corners not rounded, return.
    if (m_Parms.GetCornerWidth() == 0.0f)
        return;

    // Draw the rounded corners.  If there is a text box,
    // two corners are the back of the text box.     
    for (size_t i=0; i<m_RoundedCorners.size(); ++i) {
        RoundedCorner c = m_RoundedCorners[i];

        vector<CVect2<float> > arc;        
        x_GetArc(c, scale, arc);

        CVect2<float>  base((c.intersection1+c.intersection2)*0.5f);

        // arc returns edge pairs usable as GL_LINES     
        for (size_t i=0; i<arc.size(); i+=2) {
            tris.push_back(base);
            tris.push_back(arc[i]);
            tris.push_back(arc[i+1]);
        }
    }
}

void CBoundaryShapeTri::x_GetEdges(vector<CVect2<float> >& edges, 
                                   const CVect2<float>& scale)
{
    // Front quad with clipped front tri if we have a text box.
    // If not, this quad has the 4 vertices formed by the clipped
    // polygon tip and clipped lower corner with top of front
    // clip being first vertex.
    //  /|
    // | |
    //  \|
    if (m_Parms.GetTextBox()) { 
        edges.push_back(m_Rect1[1]);
        edges.push_back(m_Rect1[2]);
        edges.push_back(m_Rect1[3]);
        edges.push_back(m_Rect1[0]);
    }
    else {
        edges.push_back(m_Rect1[1]);
        edges.push_back(m_Rect1[2]);
    }


    // With a text box, this is the the lower right
    // triangle (with lower right corner clipped to 
    // make it a quad) of the text box.
    // without a text box it is the upper (large) quad
    // of the main clipped triangle (3 clips make 6 vertices) 
    if (m_Parms.GetTextBox()) { 
        edges.push_back(m_Rect2[0]);
        edges.push_back(m_Rect2[1]);

        edges.push_back(m_Rect2[2]);
        edges.push_back(m_Rect2[3]);
    }
    else {
        edges.push_back(m_Rect2[0]);
        edges.push_back(m_Rect2[1]);

        edges.push_back(m_Rect2[2]);
        edges.push_back(m_Rect2[3]);
    }

    // This is the upper left 'triangle' of the text box.  Acutally 
    // a quad because the included upper right corner is clipped
    if (m_Parms.GetTextBox()) {  
        edges.push_back(m_Rect3[1]);
        edges.push_back(m_Rect3[2]);
    }

    // If corners not rounded, return.
    if (m_Parms.GetCornerWidth() == 0.0f)
        return;

    // Draw Lines around the rounded corners  
    for (size_t i=0; i<m_RoundedCorners.size(); ++i) {
        x_GetArc(m_RoundedCorners[i], scale, edges);
    }
}

void CBoundaryShapeTri::x_GetArc(const RoundedCorner& c,
                                 const CVect2<float>& scale,
                                 vector<CVect2<float> >& edges)
{
    vector<CVect2<float> > arc;

    float corner_x = c.pos.X();
    float corner_y = c.pos.Y();
   
    CVect2<float> p0 = c.intersection1;
    CVect2<float> p1 = c.intersection2;

    // Get directions to start and stop of arc
    CVect2<float> dir0 = (c.intersection1-c.pos);
    CVect2<float> dir1 = (c.intersection2-c.pos);
    dir0.X() /= scale.X();
    dir0.Y() /= scale.Y();
    dir1.X() /= scale.X();
    dir1.Y() /= scale.Y();

    dir0.Normalize();
    dir1.Normalize();

    // Get angles for start and stop of arc, and put those angles
    // in order so that a1>a0 
    float a0 = atan2f(dir0.Y(), dir0.X());
    if (a0 < 0.0f) 
        a0 = 2.0f*3.14159f + a0;
    float a1 = atan2f(dir1.Y(), dir1.X());
    if (a1 < 0.0f) 
        a1 = 2.0f*3.14159f + a1;

    if (a0 > a1) {
        std::swap(a0, a1);
        std::swap(p0, p1);
    }

    // special case if a0/a1 cross 0-boundary (all angles here
    // will be < 180).  Add 2pi to a0 and swap a1, a0.
    if (a1-a0 > 3.14149f) {
        a0 += 2.0f*3.14159f;
        std::swap(a0, a1);
        std::swap(p0, p1);
    }

    int vert_count = 10;
    float delta = (a1-a0)/(float)vert_count;      
    float angle = a0;

    // Add arc. Push back vertices in pairs so they
    // can be drawn as GL_LINEs
    CVect2<float> prev_vert;

    for(int i=0; i<=vert_count; i++){     
        if (i==0) {
            prev_vert = p0;
        }
        else if (i==vert_count) {
            edges.push_back(prev_vert);
            edges.push_back(p1);            
        }
        else {
            edges.push_back(prev_vert);
            edges.push_back(CVect2<float>(corner_x + c.corner_width_x*cosf(angle), 
                                          corner_y + c.corner_width_y*sinf(angle)));
            prev_vert = edges[edges.size()-1];
        }
        angle += delta;
    }
}

/*********************** CBoundaryPoints ***********************************/
void CBoundaryPoints::AddBoundedPoint(const CVect2<float>& pt)
{
    m_GraphPoints.push_back(pt);
}

void CBoundaryPoints::AddPixelRect(const CVect2<float>& pt, 
                                   const CGlRect<float>& offset)
{
    m_PixelPoints.push_back(std::pair<CVect2<float>, CGlRect<float> >(pt, offset));
}

void CBoundaryPoints::AddBoundedPoints(CBoundaryPoints& pts)
{
    m_GraphPoints.insert(m_GraphPoints.begin(), 
                         pts.m_GraphPoints.begin(), 
                         pts.m_GraphPoints.end());
    m_PixelPoints.insert(m_PixelPoints.begin(), 
                         pts.m_PixelPoints.begin(), 
                         pts.m_PixelPoints.end());


    pts.ClearBoundedPoints();
}

void CBoundaryPoints::ClearBoundedPoints() 
{
    m_GraphPoints.clear(); 
    m_PixelPoints.clear();
}

/*********************** CSubtreeBoundary ***********************************/
CSubtreeBoundary::~CSubtreeBoundary()
{
}


void CSubtreeBoundary::RenderBoundary(const CVect2<float>& scale, 
                                      float alpha_mod,
                                      bool include_labels)
{
    if (!m_BoundaryShape.Empty())
        m_BoundaryShape->Render(scale, alpha_mod, include_labels);
}

void CSubtreeBoundary::RenderBoundaryVbo(const CVect2<float>& scale,
                                         float alpha_mod,
                                         bool include_labels)
{
    if (!m_BoundaryShape.Empty() && 
        (m_LastScale - scale).Length2() > FLT_EPSILON) {

        m_LastScale = scale;
        if (m_BoundaryTris.IsNull()) {
            m_BoundaryTris.Reset(new CGlVboNode(GL_TRIANGLES));
            m_BoundaryTris->GetState().Enable(GL_BLEND);
            m_BoundaryTris->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (m_BoundaryEdges.IsNull()) {
            m_BoundaryEdges.Reset(new CGlVboNode(GL_LINES));
            m_BoundaryEdges->GetState().Enable(GL_BLEND);
            m_BoundaryEdges->GetState().BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        m_BoundaryShape->RenderVbo(m_BoundaryTris, m_BoundaryEdges, scale, alpha_mod, include_labels);
    }
}

void CSubtreeBoundary::RenderVbo()
{
    m_BoundaryTris->Render();
    m_BoundaryEdges->Render();
}

void CSubtreeBoundary::ComputeShapes(const CBoundaryPoints& boundary_pts,
                                     const CVect2<float>& scale,
                                     const CVect2<float>& base_node_pos,
                                     const string& layout_type)
{
    // This will force a redraw of the shape
    m_LastScale.Set(0.0f, 0.0f);

    if (m_Shapes.find(layout_type) != m_Shapes.end())
        m_BoundaryShape = m_Shapes[layout_type];
    else
        m_BoundaryShape = m_Shapes["Default"];

    if (!m_BoundaryShape.Empty()) {
        m_BoundaryShape->ComputeShape(scale, 
                                      base_node_pos,                                   
                                      boundary_pts.GetGraphPoints(), 
                                      boundary_pts.GetPixelPoints());
        m_BoundaryShape->Hide(false);
    }
}

void CSubtreeBoundary::Hide(bool b) 
{
    if (!m_BoundaryShape.Empty()) {
        m_BoundaryShape->Hide(b);
    }
}

bool CSubtreeBoundary::Overlaps(TModelRect& r) const
{
    if (!m_BoundaryShape.Empty()) {
        return (r.Intersects(m_BoundaryShape->GetExtent()));
    }

    return false;
}

string CSubtreeBoundary::x_GetParameter(const string& features, const string& parm)
{
    string parm_lower = parm;
    parm_lower = NStr::ToLower(parm_lower) + "=";

    string features_lower = features;
    features_lower = NStr::ToLower(features_lower);

    size_t idx = features_lower.find(parm_lower);
    if (idx != string::npos) {
        size_t end_idx = features.find_first_of(" ,\t", idx);
        if (end_idx == string::npos)
            end_idx = features.size();

        return features.substr(idx + parm_lower.length(), end_idx-(idx+parm_lower.length()));
    }

    return "";
}

bool CSubtreeBoundary::x_GetColorParameter(const string& features, 
                                           const string& parm,
                                           bool parm_required,
                                           CRgbaColor& c)
{
    string parm_lower = parm;
    parm_lower = NStr::ToLower(parm_lower) + "=[";

    string features_lower = features;
    features_lower = NStr::ToLower(features_lower);

    string::size_type idx1 = string::npos;
    
    if (!parm_required) {
        if (features[0] == '[')
            idx1 = 0;
        else {
            idx1 = features.find(" [");
            if (idx1 != string::npos) 
                idx1 += 2;
        }
    }

    if (idx1 == string::npos)  {
        idx1 = features_lower.find(parm_lower);
        if (idx1 != string::npos) {
            idx1 += parm_lower.length();
        }
    }

    if (idx1 != string::npos) {
        string::size_type idx2 = features.find(']', idx1 + 2);
        if (idx2 != string::npos) {
            // There is a color array.  
            string color_str = features.substr(idx1, idx2-idx1);
            c.FromString(color_str);

            // Was the alpha value provided? if not use default.
           // list<string> toks;
           // NStr::Split(color_str.substr(1, idx2-idx1-1), ", ", toks);
           // if (toks.size() < 4)
           //     c.SetAlpha(0.25f);

            return true;
        }
    }

    return false;
}


void CSubtreeBoundary::x_AddBoundaryType(const CBoundaryParms& parms, 
                                         const string& boundary_type,
                                         const string& layout_type)
{
    if (boundary_type != "") {       
        IBoundaryShape* s = IBoundaryShape::CreateBoundary(boundary_type);
        if (s != NULL) {
            s->SetBoundaryParms(parms);
            CRef<IBoundaryShape> sr(s);
            m_Shapes[layout_type] = sr;
        }
    }
}


void CSubtreeBoundary::CreateShapes(const std::string& features)
{
    // Avoid unnecessary updates:
    if (m_BoundaryStr == features)
        return;

    m_BoundaryStr = features;

    CBoundaryParms parms;

    // default to red if color not provided
    parms.GetColor().Set(1.0f, 0.0f, 0.0f, 0.25f);

    // find color string in feature in form [...]
    // may be given without a name or as color=[...]
    string color_str;
    
    //string::size_type idx1 = features.find_first_of('[');

    // If there is a qualifier name of the form name=[..] and 
    // the qualifier is "color=" then this is the color we want.
    // Also if there is no qualifier (no name=), then that is
    // the color we want.
    x_GetColorParameter(features, "Color", false, parms.GetColor());

    //
    // shape=RoundedRect CladogramShape=Rect RadialShape=Ellipse ForceShape=Triangle
    // Color=[128, 128, 128, 32]
    // Border=22

    m_Shapes.clear();

    string parm;
    
    parm = x_GetParameter(features, "Border");
    if (parm != "")
        parms.SetBorderWidth(NStr::StringToDouble(parm, NStr::fConvErr_NoThrow));

    parm = x_GetParameter(features, "Corner");
    if (parm != "")
        parms.SetCornerWidth(NStr::StringToDouble(parm, NStr::fConvErr_NoThrow));

    // valid values: true t yes y | false f no n
    try {
        parm = x_GetParameter(features, "DrawEdge");
        if (parm != "")
            parms.SetDrawBoundaryEdge(NStr::StringToBool(parm));
    }
    catch (CStringException&) {
        // just use default
    };

    x_GetColorParameter(features, "EdgeColor", true, parms.GetBoundaryEdgeColor());

    try {
        parm = x_GetParameter(features, "IncludeText");
        if (parm != "")
            parms.SetIncludeTextArea(NStr::StringToBool(parm));
    }
    catch (CStringException&) {
        // just use default
    };

    try {
        parm = x_GetParameter(features, "AxisAligned");
        if (parm != "")
            parms.SetAxisAligned(NStr::StringToBool(parm));
    }
    catch (CStringException&) {
        // just use default
    };

    try {
        parm = x_GetParameter(features, "TextBox");
        if (parm != "")
            parms.SetTextBox(NStr::StringToBool(parm));
    }
    catch (CStringException&) {
        // just use default
    };

    parm = x_GetParameter(features, "TriOffset");
    if (parm != "")
        parms.SetTriOffset(NStr::StringToDouble(parm, NStr::fConvErr_NoThrow));

    parm = x_GetParameter(features, "Shape");
    if (parm == "")
        parm = "RoundedRectangle";

    IBoundaryShape* s = IBoundaryShape::CreateBoundary(parm);
    if (s != NULL) {
        s->SetBoundaryParms(parms);

        CRef<IBoundaryShape> sr(s);
        m_Shapes["Default"] = sr;
    }

    x_AddBoundaryType(parms, x_GetParameter(features, "SlantedCladogram"), 
                      "SlantedCladogram");
    x_AddBoundaryType(parms, x_GetParameter(features, "RectCladogram"),
                      "RectCladogram");
    x_AddBoundaryType(parms, x_GetParameter(features, "Radial"),
                      "Radial");
    x_AddBoundaryType(parms, x_GetParameter(features, "ForceLayout"),
                      "ForceLayout");

    m_BoundaryShape = m_Shapes["Default"];
}

END_NCBI_SCOPE
