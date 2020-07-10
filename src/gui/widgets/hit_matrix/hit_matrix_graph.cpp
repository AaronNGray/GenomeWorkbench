/*  $Id: hit_matrix_graph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistl.hpp>

#include <gui/widgets/hit_matrix/hit_matrix_graph.hpp>

#include <gui/widgets/wx/gui_event.hpp>

#include <gui/graph/igraph_utils.hpp>
#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>
#include <math.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
///
static const int kDefIndex = -1;
static const int kSelectedIndex = -2;
static const int kHiddenIndex = -3;


CHitElemGlyph::CHitElemGlyph(const IHitElement& hit_elem)
:   m_HitElem(&hit_elem),
    m_ColorIndex(kDefIndex),
    m_bSelected(false)
{
}


void CHitElemGlyph::GetModelRect(TModelRect& rc) const
{
    rc.Init(m_HitElem->GetSubjectStart(), m_HitElem->GetQueryStart());
    TSeqPos s_size = m_HitElem->GetSubjectLength();
    TSeqPos q_size = m_HitElem->GetQueryLength();
    rc.SetSize(s_size, q_size);
}


void CHitElemGlyph::Render(CGlPane& pane, ERenderingPass what)
{
    TModelUnit x1 = m_HitElem->GetSubjectStart() - pane.GetOffsetX();
    TModelUnit y1 = m_HitElem->GetQueryStart() - pane.GetOffsetY();
    TModelUnit s_l = m_HitElem->GetSubjectLength();
    TModelUnit q_l = m_HitElem->GetQueryLength();
    TModelUnit x2 = x1 + s_l;
    TModelUnit y2 = y1 + q_l;

    if(m_HitElem->GetSubjectStrand() == eNa_strand_minus)    {
        swap(x1, x2);
    }
    if(m_HitElem->GetQueryStrand() == eNa_strand_minus)    {
        swap(y1, y2);
    }

    TModelUnit scale_x = pane.GetScaleX();
    TModelUnit scale_y = pane.GetScaleY();

    if(fabs(x2 - x1) < scale_x  &&  fabs(y2 - y1) < scale_y){
        // too small, we need to draw at least one pixel
        x1 = 0.5 * (x2 + x1 - scale_x);
        x2 = x1 + scale_x;

        y1 = 0.5 * (y2 + y1 - scale_y);
        y2 = y1 + scale_y;
    }

    IRender& gl = GetGl();
    switch(what)    {
    case eEndPoints:
    case eHitElemLines:    {
        gl.Vertex2d(x1, y1);
        gl.Vertex2d(x2, y2);
    }; break;
    case eProjLines:    {
        TModelUnit left = min(x1, pane.GetVisibleRect().Left() - pane.GetOffsetX());
        TModelUnit bottom = min(y1, pane.GetVisibleRect().Bottom() - pane.GetOffsetY());

        gl.Vertex2d(x1, y1);
        gl.Vertex2d(left, y1);

        gl.Vertex2d(left, y2);
        gl.Vertex2d(x2, y2);

        gl.Vertex2d(x1, y1);
        gl.Vertex2d(x1, bottom);

        gl.Vertex2d(x2, bottom);
        gl.Vertex2d(x2, y2);
    }; break;
    default: break;
    }
}


void CHitElemGlyph::StartVertex()
{
    IRender& gl = GetGl();
    TModelUnit x1 = m_HitElem->GetSubjectStart();
    if(m_HitElem->GetSubjectStrand() == eNa_strand_minus)    {
        x1 += m_HitElem->GetSubjectLength();
    }

    TModelUnit y1 = m_HitElem->GetQueryStart();
    if(m_HitElem->GetQueryStrand() == eNa_strand_minus)    {
        y1 += m_HitElem->GetQueryLength();
    }
    gl.Vertex2d(x1, y1);
}


void CHitElemGlyph::EndVertex()
{
    TModelUnit x1 = m_HitElem->GetSubjectStart();
    if(m_HitElem->GetSubjectStrand() != eNa_strand_minus)    {
        x1 += m_HitElem->GetSubjectLength();
    }

    TModelUnit y1 = m_HitElem->GetQueryStart();
    if(m_HitElem->GetQueryStrand() != eNa_strand_minus)    {
        y1 += m_HitElem->GetQueryLength();
    }
    IRender& gl = GetGl();
    gl.Vertex2d(x1, y1);
}


inline double square(double x)  {    return pow(x, 2);  }

double  CHitElemGlyph::GetDistPixels(CGlPane& pane, const TVPPoint& pt) const
{
    TModelUnit x1 = m_HitElem->GetSubjectStart();// - pane.GetOffsetX();
    TModelUnit y1 = m_HitElem->GetQueryStart();// - pane.GetOffsetY();
    TModelUnit s_l = m_HitElem->GetSubjectLength();
    TModelUnit q_l = m_HitElem->GetQueryLength();
    TModelUnit x2 = x1 + s_l;
    TModelUnit y2 = y1 + q_l;

    if(m_HitElem->GetSubjectStrand() == eNa_strand_minus)    {
        swap(x1, x2);
    }
    if(m_HitElem->GetQueryStrand() == eNa_strand_minus)    {
        swap(y1, y2);
    }

    TVPPoint p1 = pane.Project(x1, y1);
    TVPPoint p2 = pane.Project(x2, y2);

    double alpha = atan2((double)(p2.Y() - p1.Y()), (double)(p2.X() - p1.X()));
    double max_proj = sqrt( square(p2.Y() - p1.Y()) + square(p2.X() - p1.X()));

    double beta = atan2((double)(pt.Y() - p1.Y()), (double)(pt.X() - p1.X()));
    double hyp = sqrt( square(pt.Y() - p1.Y()) + square(pt.X() - p1.X()));
    double angle = beta - alpha;

    double proj = hyp * cos(angle);
    if(proj >= 0  &&  proj <= max_proj)  { // projection point is on hit
        double ortho_d = fabs( hyp * sin(angle));
        return ortho_d;
    } else { // calculate min of distances to hit's ends
        double d2 = sqrt( square(pt.Y() - p2.Y()) + square(pt.X() - p2.X()));
        return min(hyp, d2);
    }
}

bool  CHitElemGlyph::InRect(CGlPane& pane, const TVPRect& rc) const
{
    TModelUnit x1 = m_HitElem->GetSubjectStart();
    TModelUnit y1 = m_HitElem->GetQueryStart();
    TVPPoint p1 = pane.Project(x1, y1);

    if(rc.PtInRect(p1)) {
        TModelUnit s_l = m_HitElem->GetSubjectLength();
        TModelUnit q_l = m_HitElem->GetQueryLength();
        TModelUnit x2 = x1 + s_l;
        TModelUnit y2 = y1 + q_l;

        TVPPoint p2 = pane.Project(x2, y2);
        return rc.PtInRect(p2);
    }
    return false;
}


inline bool InRange(TModelUnit z, TModelUnit z1, TModelUnit z2)
{
    return z >= std::min(z1, z2)  &&  z <= std::max(z1, z2);
}


bool CHitElemGlyph::Intersects(CGlPane& pane, const TVPRect& rc) const
{
    TModelUnit x1 = m_HitElem->GetSubjectStart();
    TModelUnit y1 = m_HitElem->GetQueryStart();

    TModelUnit s_l = m_HitElem->GetSubjectLength();
    TModelUnit q_l = m_HitElem->GetQueryLength();
    TModelUnit x2 = x1 + s_l;
    TModelUnit y2 = y1 + q_l;

    // unporject the rectangle to the model space
    TModelPoint p1 = pane.UnProject(rc.Left(), rc.Bottom());
    TModelPoint p2 = pane.UnProject(rc.Right(), rc.Top());

    // if one of the endpoint (x1, y1) and (x2, y2) is inside the rectangle
    // we return true
    if(InRange(x1, p1.X(), p2.X())  &&  InRange(y1, p1.Y(), p2.Y())) {
        return true;
    }
    if(InRange(x2, p1.X(), p2.X())  &&  InRange(y2, p1.Y(), p2.Y())) {
        return true;
    }

    // no luck, now check if this hit element intersects the rectangle
    // y = k * x + b;
    double k = double(y2 - y1) / (x2 - x1);
    double b = y1 - k * x1;

    // calculate coordinates of the points where y = kx + b intersects
    // the lines froming the rectangle rc
    TModelUnit xc_1 = (p1.Y() - b) / k;
    if(InRange(xc_1, x1, x2)  &&  InRange(xc_1, p1.X(), p2.X()))    {
        return true;
    }

    TModelUnit xc_2 = (p2.Y() - b) / k;
    if(InRange(xc_2, x1, x2)  &&  InRange(xc_2, p1.X(), p2.X()))    {
        return true;
    }

    TModelUnit yc_1 = k * p1.X() + b;
    if(InRange(yc_1, y1, y2)  &&  InRange(yc_1, p1.Y(), p2.Y()))    {
        return true;
    }

    TModelUnit yc_2 = k * p2.X() + b;
    if(InRange(yc_2, y1, y2)  &&  InRange(yc_2, p1.Y(), p2.Y()))    {
        return true;
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////
/// CHitGlyph

CHitGlyph::CHitGlyph(const IHit& hit)
: m_Hit(hit)
{
    // create Elem glyphs
    size_t size = hit.GetElemsCount();
    m_Elems.reserve(size);

    bool bounds = false;  // initialized

    for( size_t i = 0; i < size;  i++ ) {
        const IHitElement& elem = hit.GetElem(i);

        if(elem.GetQueryStart() >=0  &&  elem.GetSubjectStart() >= 0 )   {
            // not gap - create glyph
            m_Elems.push_back(CHitElemGlyph(elem));

            // extend bounds rectangle
            int start_s = elem.GetSubjectStart();
            int start_q = elem.GetQueryStart();
            int len_s = elem.GetSubjectLength();
            int len_q = elem.GetQueryLength();
            int end_s = start_s + len_s;
            int end_q = start_q + len_q;

            if(bounds)    {
                start_s = min(m_Bounds.Left(), start_s);
                end_s = max(m_Bounds.Right(), end_s);
                start_q = min(m_Bounds.Bottom(), start_q);
                end_q = max(m_Bounds.Top(), end_q);
            }
            m_Bounds.SetLeft(start_s);
            m_Bounds.SetRight(end_s);
            m_Bounds.SetBottom(start_q);
            m_Bounds.SetTop(end_q);
            bounds = true;
        }
    }
}


CHitGlyph::TElemGlyphCont&  CHitGlyph::GetElems()
{
    return m_Elems;
}


void CHitGlyph::SetColorIndex(int index)
{
    NON_CONST_ITERATE(TElemGlyphCont, it, m_Elems)  {
        it->SetColorIndex(index);
    }
}


void CHitGlyph::Render(CGlPane& pane, CHitElemGlyph::ERenderingPass what)
{
    int size = (int)m_Elems.size();
    switch(what)    {
    case CHitElemGlyph::eConnectionLines:   {
        for( int i = 0; i < size - 1; )   {
            m_Elems[i].EndVertex();
            m_Elems[++i].StartVertex();
        }
    }; break;
    default:    {
        for( int i = 0; i < size; i++ )   {
            m_Elems[i].Render(pane, what);
        }
    }; //default
    }// switch
}


///////////////////////////////////////////////////////////////////////////////
/// CHitMatrixGraph

BEGIN_EVENT_TABLE(CHitMatrixGraph, wxEvtHandler)
    EVT_LEFT_DOWN(CHitMatrixGraph::OnLeftDown)
    EVT_LEFT_UP(CHitMatrixGraph::OnLeftUp)
    EVT_MOTION(CHitMatrixGraph::OnMotion)
    EVT_MOUSE_CAPTURE_LOST(CHitMatrixGraph::OnMouseCaptureLost)
    EVT_KEY_DOWN(CHitMatrixGraph::OnKeyEvent)
    EVT_KEY_UP(CHitMatrixGraph::OnKeyEvent)
END_EVENT_TABLE()


static float    kHitAlpha = 0.8f;

CHitMatrixGraph::CHitMatrixGraph()
:   m_CurrColorIndex(0),
    m_ColoringParams(NULL),
    m_ColorTable(1),
    m_DefaultColor(0.25f, 0.25f, 0.25f, kHitAlpha),
    m_SelColor(0.5f, 0.5f, 1.0f, 1.0f),
    m_HighLightColor(0.8f, 0.8f, 1.0f, 0.5),
    m_PathColor(0.2f, 0.2f, 0.2f, 1.0f),
    m_ProjLinesColor(0.8f, 0.8f, 1.0f, 0.25f),
    m_ProjBackColor(0.8f, 0.8f, 1.0f, 0.25f),
    m_DrawConnections(false),
    m_Host(NULL),
    m_Pane(NULL),
    m_State(eIdle)
{
}


CHitMatrixGraph::~CHitMatrixGraph()
{
    DeleteGlyphs();
}


void CHitMatrixGraph::DrawConnections(bool draw)
{
    m_DrawConnections = draw;
}


void CHitMatrixGraph::DeleteGlyphs()
{
    ResetGlyphSelection();

    destroy_and_erase_elems(m_vGlyphs);
}


void CHitMatrixGraph::CreateGlyph(const IHit& hit_elem)
{
    m_vGlyphs.push_back(new CHitGlyph(hit_elem));
}



void CHitMatrixGraph::ResetGlyphSelection()
{
    NON_CONST_ITERATE(TElemGlyphSet, it, m_SelGlyphs)   {
        (*it)->SetSelected(false);
    }
    m_SelGlyphs.clear();
}


void CHitMatrixGraph::SelectGlyph(const CHitGlyph& glyph)
{
    // argument is const, but the method is not const, so with
    // can legitimately access all glyphs in the graph
    CHitGlyph& the_glyph = *const_cast<CHitGlyph*>(&glyph);

    NON_CONST_ITERATE(CHitGlyph::TElemGlyphCont, it, the_glyph.GetElems())    {
        CHitElemGlyph& elem = *it;
        if(! elem.IsSelected())     {
            elem.SetSelected(true);
            m_SelGlyphs.insert(&elem);
        }
    }
}


void CHitMatrixGraph::Render(CGlPane& pane)
{
    if (pane.GetViewport().Width() < 10 ||
        pane.GetViewport().Height() < 10)
            return;

    {
        CGlPaneGuard GUARD(pane, CGlPane::eOrtho);
        x_RenderPath(pane);
        x_RenderSelection(pane);
        x_RenderHits(pane);

        //m_Clustering.Build(m_vGlyphs, pane.GetScaleX(), pane.GetScaleY());
        //x_RenderClustering(pane);
    }

    x_RenderEventHandler(pane);
}


static const float kHitLineWidth = 2.0;

void CHitMatrixGraph::x_RenderHits(CGlPane& pane)
{
    IRender& gl = GetGl();
//    gl.Flush();

    CGlAttrGuard guard(GL_LINE_BIT |  GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT );

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl.Enable(GL_LINE_SMOOTH);
    //gl.Hint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    const TModelRect& rc_V = pane.GetVisibleRect();

    // precomputation
    TModelUnit scale_x = pane.GetScaleX();
    TModelUnit scale_y = pane.GetScaleY();

    TModelUnit offset_x = pane.GetOffsetX();
    TModelUnit offset_y = pane.GetOffsetY();

    // draw Hit Element Lines
    gl.ColorC(GetColorByIndex(m_CurrColorIndex));

    gl.LineWidth(kHitLineWidth);
    gl.Begin(GL_LINES);
        NON_CONST_ITERATE(THitGlyphVector, it, m_vGlyphs)    { // for each Hit
            CHitGlyph::TElemGlyphCont& elems = (*it)->GetElems();

            // filter out invisible hits
            const TVPRect& rc_b = (*it)->m_Bounds;
            if((rc_b.Left() > rc_V.Right()  ||  rc_b.Right() < rc_V.Left())
                && (rc_b.Bottom() >rc_V.Top()  ||  rc_b.Top() < rc_V.Bottom())) {
                continue;
            }

            bool tiny_x =  rc_b.Width() < scale_x;
            bool tiny_y = rc_b.Height() < scale_y;

            if( ! x_IsColoringEnabled() &&  tiny_x  &&  tiny_y)  {
                // render hit as dot
                TModelUnit x1 = 0.5 * (rc_b.Left() + rc_b.Right() - scale_x) - offset_x;
                TModelUnit x2 = x1 + scale_x;

                TModelUnit y1 = 0.5 * (rc_b.Bottom() + rc_b.Top() - scale_y) - offset_y;
                TModelUnit y2 = y1 + scale_y;

                // get the color
                if(m_CurrColorIndex != -1)  {
                    m_CurrColorIndex = -1;
                    gl.Color4f(m_DefaultColor.GetRed(), m_DefaultColor.GetGreen(),
                              m_DefaultColor.GetBlue(), kHitAlpha);
                }

                // draw dot
                gl.Vertex2d(x1, y1);
                gl.Vertex2d(x2, y2);
            } else {
                // render hit the usual way
                // for each Hit Element
                NON_CONST_ITERATE(CHitGlyph::TElemGlyphCont, itE, elems)    {
                    // Colors - TODO
                    if(itE->IsSelected()) {
                        gl.ColorC(m_SelColor);
                        m_CurrColorIndex = kSelectedIndex;
                    } else {
                        int ind = itE->GetColorIndex();
                        if(ind != m_CurrColorIndex) {
                            m_CurrColorIndex = ind;
                            const CRgbaColor& color = GetColorByIndex(m_CurrColorIndex);

                            // make it semi-transparent
                            gl.Color4f(color.GetRed(), color.GetGreen(), color.GetBlue(), kHitAlpha);
                        }
                    }
                    if(m_CurrColorIndex != kHiddenIndex)    {
                        itE->Render(pane, CHitElemGlyph::eHitElemLines);
                    }
                }
            }
        }
    gl.End();

    gl.LineWidth(1.0f);
    gl.Disable(GL_LINE_SMOOTH);

    if(m_DrawConnections)  {   // draw connecting lines between Hit Elements
        gl.ColorC(m_DefaultColor);

        gl.Enable(GL_LINE_STIPPLE);
        gl.LineWidth(1.0f);
        gl.LineStipple(1, 0x5555);

        gl.Begin(GL_LINES);
            NON_CONST_ITERATE(THitGlyphVector, it, m_vGlyphs)    {
                (*it)->Render(pane, CHitElemGlyph::eConnectionLines);
            }
        gl.End();
    }

    //gl.Flush();
    //LOG_POST("x_RenderHits()  hits  = " << count << ", elems  = " << e_count <<
    //         ", Clipped hits " << clipped << ", Dots " << dots << ", time = " << (w.Elapsed() * 1000) << " ms");
}


/*void CHitMatrixGraph::x_RenderClustering(CGlPane& pane)
{
    CStopWatch w;
    w.Start();

    CGlAttrGuard guard(GL_LINE_BIT |  GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glLineWidth(6.0f);
    glColor4d(0.8, 0.8, 0.0, 0.5); //###

    glBegin(GL_LINES);

    TModelUnit off_x = pane.GetOffsetX();
    TModelUnit off_y = pane.GetOffsetY();

    int hits = 0;

    const vector<CHitCluster*>& clusters = m_Clustering.GetClusters();
    for( size_t i = 0;  i < clusters.size();  i++ ) {
        const CHitCluster* cl = clusters[i];
        const TVector& v = cl->m_Vector;

        glVertex2d(v.m_From.X() - off_x, v.m_From.Y() - off_y);
        glVertex2d(v.m_To.X() - off_x, v.m_To.Y() - off_y);

        hits += cl->m_Elems.size();
    }

    glEnd();
    glLineWidth(1.0f);

    double t = w.Elapsed();
    LOG_POST("x_RenderClustering()  clusters " << clusters.size() <<
             ", cl hits " << hits << ", time " << t * 1000 << " ms");
}*/


void CHitMatrixGraph::x_RenderPath(CGlPane& pane)
{
    IRender& gl = GetGl();
    CGlAttrGuard guard(GL_LINE_BIT |  GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT );

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl.Enable(GL_LINE_SMOOTH);
//    gl.Hint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    gl.LineWidth(6.0f);
    gl.Color4d(0.8, 0.8, 0.8, 0.5); //###

    gl.Begin(GL_LINES);
        NON_CONST_ITERATE(TPathCont, it, m_Paths)    { // for each Hit
            TPath& path = *it;

            // for each Hit Elem in the path
            NON_CONST_ITERATE(TPath, itE, path)    {
                if(! (*itE)->IsSelected()) {
                    int ind = (*itE)->GetColorIndex();
                    const CRgbaColor* pC = & GetColorByIndex(ind);
                    gl.Color4d(pC->GetRed(), pC->GetGreen(), pC->GetBlue(), 0.25);

                    (*itE)->Render(pane, CHitElemGlyph::eHitElemLines);
                }
            }
        }

    // draw connecting lines
    NON_CONST_ITERATE(TPathCont, it, m_Paths)    { // for each Hit
        TPath& path = *it;

        // for each Hit Elem in the path
        TPath::const_iterator it1 = path.begin();
        TPath::const_iterator it2 = ++it1;
        while( it2 != path.end() )  {
            (*it1)->EndVertex();
            (*it2)->StartVertex();
            it1 = it2;
            it2++;
        }
    }
    gl.End();
    gl.LineWidth(1.0f);
}


void CHitMatrixGraph::x_RenderSelection(CGlPane& pane)
{
    IRender& gl = GetGl();
    CGlAttrGuard guard(GL_LINE_BIT |  GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT );

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw projection quads
    gl.ColorC(m_ProjBackColor);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.Begin(GL_QUADS);
         NON_CONST_ITERATE(TElemGlyphSet, it, m_SelGlyphs)    {
           (*it)->Render(pane, CHitElemGlyph::eProjLines);
        }
    gl.End();

    // draw projection lines
    gl.LineWidth(0.5f);
    gl.ColorC(m_ProjLinesColor);

    gl.Begin(GL_LINES);
         NON_CONST_ITERATE(TElemGlyphSet, it, m_SelGlyphs)    {
           (*it)->Render(pane, CHitElemGlyph::eProjLines);
        }
    gl.End();

    // draw highlight
    gl.Enable(GL_LINE_SMOOTH);
//    gl.Hint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    gl.LineWidth(8.0f);
    gl.ColorC(m_HighLightColor);
    gl.Begin(GL_LINES);
         NON_CONST_ITERATE(TElemGlyphSet, it, m_SelGlyphs)    {
           (*it)->Render(pane, CHitElemGlyph::eHitElemLines);
        }
    gl.End();
    gl.LineWidth(1.0f);
}


void CHitMatrixGraph::x_RenderEventHandler(CGlPane& pane)
{
    if(m_State == eSelRect) {
        _ASSERT(m_Host);

        IRender& gl = GetGl();
        CGlAttrGuard AttrGuard(GL_POLYGON_BIT | GL_LINE_BIT);
        CGlPaneGuard GUARD(pane, CGlPane::ePixels);

        gl.LineWidth(1.0f);
        gl.Color3f(0.0f, 0.0f, 0.0f);

        gl.LineStipple(1, 0x0F0F);
        gl.Enable(GL_LINE_STIPPLE);
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        int x1 = m_StartPoint.x;
        int y1 = m_Host->HMGH_GetVPPosByY(m_StartPoint.y);
        int x2 = m_DragPoint.x;
        int y2 = m_Host->HMGH_GetVPPosByY(m_DragPoint.y);
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
    }
}


bool CHitMatrixGraph::x_IsColoringEnabled()
{
    return m_ColoringParams != NULL;
}


void CHitMatrixGraph::AssignColorsByScore( const SHitColoringParams* params )
{
    m_ColoringParams = params;

    size_t num_glyphs = m_vGlyphs.size();
    if( m_ColoringParams ){
        m_ColorTable.SetSize( m_ColoringParams->m_Steps );
        m_ColorTable.FillGradient( 
            m_ColoringParams->m_MinColor,
            m_ColoringParams->m_MaxColor
        );

        // assigne colors
        int colors_n = (int)m_ColorTable.GetSize();
        const string& score = m_ColoringParams->m_ScoreName;

        double min_v = m_ColoringParams->GetMin( false );
        double max_v = m_ColoringParams->GetMax( false );
        for( size_t i = 0; i < num_glyphs;  i++ ){
            double value = m_vGlyphs[i]->GetHit().GetScoreValue( score );
            int ind = kDefIndex;

            if( value >= min_v  &&  value <= max_v ){
                double norm = m_ColoringParams->GetColorNorm( value, false );
                ind = (int) floor( norm * colors_n );
                ind = min( ind, colors_n - 1 );
            } else {
                ind = kHiddenIndex;
            }
            m_vGlyphs[i]->SetColorIndex( ind );
        }
    } else {
         for( size_t i = 0; i < num_glyphs;  i++ ){
            m_vGlyphs[i]->SetColorIndex(-1);
        }
    }
}


void CHitMatrixGraph::SetHost(IHitMatrixGraphHost* host)
{
    m_Host = host;
}


IGenericHandlerHost* CHitMatrixGraph::GetGenericHost()
{
    return dynamic_cast<IGenericHandlerHost*>(m_Host);
}


void CHitMatrixGraph::SetPane(CGlPane* pane)
{
    m_Pane = pane;
}


wxEvtHandler* CHitMatrixGraph::GetEvtHandler()
{
    return this;
}


////////////////////////////////////////////////////////////////////////////////
/// event handlers

void CHitMatrixGraph::OnLeftDown(wxMouseEvent& event)
{
    m_State = eIdle;
    m_MoveDuringSelection = false;

    m_StartPoint  = event.GetPosition();
    m_DragPoint = m_StartPoint;

    CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);

    m_IncSelection = state == CGUIEvent::eSelectIncState  ||
                     state == CGUIEvent::eSelectExtState;
    m_ToggleSelection = state == CGUIEvent::eSelectIncState;

    bool selected = x_SelectTest();

    // If there is no glyph selected on mouse down, unselect on mouse up
    // if not an incremental selection and mouse didn't move during selection.
    if (selected)
        x_SelectByPoint(m_IncSelection, m_ToggleSelection);

    if (selected)
        m_State = eSelPoint;
    else if (m_IncSelection)
        m_State = eSelRect;
    else 
        m_State = eIdle;

    if (m_State != eIdle) {
        m_Host->HMGH_OnChanged();
        x_OnSelectCursor();

        if(m_State == eSelRect) {
            GetGenericHost()->GHH_CaptureMouse();
        }
    }
    else {
        event.Skip();
    }
}


void CHitMatrixGraph::OnMotion(wxMouseEvent& event)
{
    m_MoveDuringSelection = true;

    if(event.Dragging())    {
        if(m_State == eSelRect)   {
            wxPoint ms_pos = event.GetPosition();

            if(ms_pos != m_DragPoint)  {
                m_State = eSelRect;
                m_DragPoint = ms_pos;

                GetGenericHost()->GHH_Redraw();
                x_OnSelectCursor();
            }
        }
        else {
            event.Skip();
        }
    } else {
        if(m_State == eIdle) {
            event.Skip();
        }
    }
}


void CHitMatrixGraph::OnLeftUp(wxMouseEvent& event)
{
    switch(m_State) {
    case eSelPoint:
        break;

    case eSelRect: {
        x_SelectByRect(m_IncSelection, 
                       m_ToggleSelection);

        m_Host->HMGH_OnChanged();
        x_CancelSelRectMode();
        break;
    }
    default:
        // Left mouse click on the background undoes selection
        // unless mouse is moved, in which case it is panning.
        if (!m_MoveDuringSelection && !m_IncSelection) {
            ResetGlyphSelection();
            m_Host->HMGH_OnChanged();
        }
        
        event.Skip();       
    }

    m_State = eIdle;
}


void CHitMatrixGraph::OnMouseCaptureLost(wxMouseCaptureLostEvent& evt)
{
    if(m_State == eSelRect) {
        x_CancelSelRectMode();
    }
}


void CHitMatrixGraph::OnKeyEvent(wxKeyEvent& event)
{
    if(m_State == eIdle) {
        event.Skip();
    }
}


void CHitMatrixGraph::x_CancelSelRectMode()
{
    m_State = eIdle;
    x_OnSelectCursor();
    GetGenericHost()->GHH_ReleaseMouse();
}


void CHitMatrixGraph::x_OnSelectCursor(void)
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


static int kHitTestPrecision = 5;

bool CHitMatrixGraph::x_SelectTest() const
{
    double min_d = -1;
    const CHitElemGlyph*  p_hit_elem = NULL;

    TVPPoint pt(m_StartPoint.x, m_Host->HMGH_GetVPPosByY(m_StartPoint.y));

    ITERATE(THitGlyphVector, it, m_vGlyphs)    { // for each Hit
        CHitGlyph::TElemGlyphCont& elems = (*it)->GetElems();

        // for each Hit Element
        ITERATE(CHitGlyph::TElemGlyphCont, itE, elems)    {
            double d = itE->GetDistPixels(*m_Pane, pt);
            if(! p_hit_elem  ||  d < min_d )  {
                min_d = d;
                p_hit_elem = &(*itE);
            }
        }
    }
    if(p_hit_elem  &&  min_d <= kHitTestPrecision)  {      
        return true;
    }
    return false;
}

bool CHitMatrixGraph::x_SelectByPoint(bool b_inc, bool toggle)
{
    double min_d = -1;
    CHitElemGlyph*  p_hit_elem = NULL;

    TVPPoint pt(m_StartPoint.x, m_Host->HMGH_GetVPPosByY(m_StartPoint.y));

    NON_CONST_ITERATE(THitGlyphVector, it, m_vGlyphs)    { // for each Hit
        CHitGlyph::TElemGlyphCont& elems = (*it)->GetElems();

        // for each Hit Element
        NON_CONST_ITERATE(CHitGlyph::TElemGlyphCont, itE, elems)    {
            double d = itE->GetDistPixels(*m_Pane, pt);
            if(! p_hit_elem  ||  d < min_d )  {
                min_d = d;
                p_hit_elem = &(*itE);
            }
        }
    }
    if(! b_inc)  { // operation not incremental - clear selection
        NON_CONST_ITERATE(TElemGlyphSet, it, m_SelGlyphs)   {
            (*it)->SetSelected(false);
        }
        m_SelGlyphs.clear();
    }
    if(p_hit_elem  &&  min_d <= kHitTestPrecision)  {
        if(toggle  &&  p_hit_elem->IsSelected()) {
            p_hit_elem->SetSelected(false);
            m_SelGlyphs.erase(p_hit_elem);
        } else {
            p_hit_elem->SetSelected(true);
            m_SelGlyphs.insert(p_hit_elem);
        }
        return true;
    }
    return false;
}


/// inc - incremental, "neg" - negative (toggle select)
void CHitMatrixGraph::x_SelectByRect(bool inc, bool neg)
{
    if(! inc)  { // operation is not incremental - reset selection
        ResetGlyphSelection();
    }

    int x1 = m_StartPoint.x;
    int y1 = m_Host->HMGH_GetVPPosByY(m_StartPoint.y);
    int x2 = m_DragPoint.x;
    int y2 = m_Host->HMGH_GetVPPosByY(m_DragPoint.y);
    TVPRect rc( min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2));

    NON_CONST_ITERATE(THitGlyphVector, it, m_vGlyphs)    { // for each Hit
        CHitGlyph::TElemGlyphCont& elems = (*it)->GetElems();

        // for each Hit Element
        NON_CONST_ITERATE(CHitGlyph::TElemGlyphCont, itE, elems)    {
            bool ok = itE->Intersects(*m_Pane, rc);
            if(ok)  {
                if(neg && (itE->IsSelected())) {
                    itE->SetSelected(false);
                    m_SelGlyphs.erase(&(*itE));                    
                } else {
                    itE->SetSelected(true);
                    m_SelGlyphs.insert(&(*itE));
                }
            }
        }
    }
}


END_NCBI_SCOPE
