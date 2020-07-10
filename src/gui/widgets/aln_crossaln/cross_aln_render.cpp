/*  $Id: cross_aln_render.cpp 42766 2019-04-10 20:45:59Z katargir $
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
 * Authors:  Vlad Lebedev
 *
 */

#include <ncbi_pch.hpp>
#include "cross_aln_render.hpp"
#include <gui/widgets/aln_crossaln/cross_aln_ds.hpp>

#include <gui/opengl/glutils.hpp>
#include <math.h>


BEGIN_NCBI_SCOPE



const TVPUnit   kMaxPixelsBase  = 12;

CCrossAlnRenderer::CCrossAlnRenderer()
{
    m_DS         = NULL;
    m_MinZoomX   = log(1.0f / kMaxPixelsBase);  // Enougth to fit seq. letters

    m_RP_Q.SetMinScaleX(1.0f / kMaxPixelsBase);
    m_RP_Q.EnableZoom(true, false);
    m_RP_Q.SetAdjustToLimits(true, false);
    m_RP_Q.EnableOffset();

    m_CP.EnableZoom(false, false);
    //m_CP.EnableOffset();

    m_RP_S.SetMinScaleX(1.0f / kMaxPixelsBase);
    m_RP_S.EnableZoom(true, false);
    m_RP_S.SetAdjustToLimits(true, false);
    m_RP_S.EnableOffset();

    m_RulerPanel_Q.SetHorizontal(true, CRuler::eTop);

    m_RulerPanel_Q.SetColor(CRuler::eBackground, CRgbaColor(1.0f, 1.0f, 1.0f));
    m_RulerPanel_S.SetColor(CRuler::eBackground, CRgbaColor(1.0f, 1.0f, 1.0f));
}

CCrossAlnRenderer::~CCrossAlnRenderer()
{
}



void CCrossAlnRenderer::Update()
{
    if (!m_DS) return;

    CBioseq_Handle hndl1 = m_DS->GetQueryHandle();
    CBioseq_Handle hndl2 = m_DS->GetSubjectHandle();

    if ( !hndl1 ) {
        NCBI_THROW(CException, eUnknown, "The query sequence can't be resolved!");
    }

    if ( !hndl2 ) {
        NCBI_THROW(CException, eUnknown, "The subject sequence can't be resolved!");
    }

    m_SeqLength_Q = hndl1.GetBioseqLength();
    m_SeqLength_S = hndl2.GetBioseqLength();

    x_SetupModelLimits();
    x_AdjustCrossPane();
}


void CCrossAlnRenderer::SetDataSource(const TVPRect& rc, ICrossAlnDataSource* ds)
{
    m_DS = ds;

    m_CrossPanel.SetDataSource(ds);
    x_SetupViewPorts(rc);
}


void CCrossAlnRenderer::x_SetupViewPorts(const TVPRect& rc)
{
    TVPUnit ruler_height = m_RulerPanel_Q.GetVPRect().Height();

    m_RP_Q.SetViewport( TVPRect(rc.Left(), rc.Top() - ruler_height + 1,
                        rc.Right(), rc.Top()) );

    m_CP.SetViewport( TVPRect(rc.Left(), rc.Bottom() + ruler_height, rc.Right(), rc.Top() - ruler_height) );

    m_RP_S.SetViewport( TVPRect(rc.Left(), rc.Bottom(),
                        rc.Right(), rc.Bottom() + ruler_height - 1) );

    m_MaxZoomX_Q = log(m_RP_Q.GetModelLimitsRect().Width() / rc.Width());
    m_MaxZoomX_S = log(m_RP_S.GetModelLimitsRect().Width() / rc.Width());
}



void CCrossAlnRenderer::x_SetupModelLimits()
{
    TSeqRange rQ = m_DS->GetQueryHitsRange();
    TSeqRange rS = m_DS->GetSubjectHitsRange();

    // 3% offset on each side of each sequence
    TModelUnit pQ = m_RP_Q.GetViewport().Width() * 3.0 / 100.0;
    TModelUnit pS = m_RP_S.GetViewport().Width() * 3.0 / 100.0;


    TModelRect rc2( max(TModelUnit(0.0), rQ.GetFrom()-pQ), 0, min(m_SeqLength_Q + TModelUnit(1.0),
            rQ.GetTo() + TModelUnit(1) + pQ), m_RP_Q.GetViewport().Height());
    TModelRect rc3( max(TModelUnit(0.0), rS.GetFrom()-pS), TModelUnit(0), min(m_SeqLength_S+TModelUnit(1.0),
            rS.GetTo() + TModelUnit(1) + pS), m_RP_S.GetViewport().Height());

    TModelRect rcc(TModelUnit(0), TModelUnit(100), TModelUnit(1), TModelUnit(0));

    m_RulerPanel_Q.SetAutoRange();
    m_RulerPanel_S.SetAutoRange();

    m_RP_Q.SetModelLimitsRect(rc2);
    m_RP_Q.SetVisibleRect    (rc2);

    m_CP.SetModelLimitsRect(rcc);
    m_CP.SetVisibleRect    (rcc);

    m_RP_S.SetModelLimitsRect(rc3);
    m_RP_S.SetVisibleRect    (rc3);

    x_AdjustCrossPane();
}


void CCrossAlnRenderer::ResetObjectSelection()
{
    m_CrossPanel.ResetObjectSelection();
}


void CCrossAlnRenderer::GetObjectSelection(TConstObjects& objs) const
{
    m_CrossPanel.GetObjectSelection(objs);
}


void CCrossAlnRenderer::SetObjectSelection(const vector<const CSeq_align*> sel_aligns)
{
    m_CrossPanel.SetObjectSelection(sel_aligns);
}


void CCrossAlnRenderer::Render()
{
    if ( !m_DS ) {
        return;
    }

    // draw all panels
    m_RulerPanel_Q.Render(m_RP_Q);  // Query
    m_CrossPanel.Render(m_CP);      // Cross Panel
    m_RulerPanel_S.Render(m_RP_S);  // Subject

    CGlUtils::CheckGlError();
}



void CCrossAlnRenderer::SetViewport(const TVPRect& rc)
{
    x_SetupViewPorts(rc);
}


void CCrossAlnRenderer::ColorBySegments()
{
    m_CrossPanel.ColorBySegments();
}


void CCrossAlnRenderer::ColorByScore(CConstRef<CObject_id> score_id)
{
    m_CrossPanel.ColorByScore(score_id);
}


CConstRef<CObject_id> CCrossAlnRenderer::GetScoreId() const
{
    return m_CrossPanel.GetScoreId();
}


TModelRect CCrossAlnRenderer::SelectHitTest(int x, int y)
{
    return m_CrossPanel.HitTest(m_CP, x, y, true);
}



TModelRect CCrossAlnRenderer::HitTest(int x, int y)
{
    return m_CrossPanel.HitTest(m_CP, x, y);
}


void CCrossAlnRenderer::GetTooltip(int x, int y, string* title)
{
    *title = "";
}



float CCrossAlnRenderer::GetZoomX_Q(void) const
{
    TModelUnit scale_x = log(m_RP_Q.GetScaleX());
    return (float)(1.0f - (scale_x - m_MinZoomX) / (m_MaxZoomX_Q - m_MinZoomX));
}


void CCrossAlnRenderer::SetZoomX_Q(float value)
{
    TModelUnit scale = m_MinZoomX + (m_MaxZoomX_Q - m_MinZoomX)
                    * (1.0f - value);

    m_RP_Q.SetScale(exp(scale), m_RP_Q.GetScaleY(),
                    m_RP_Q.GetVisibleRect().CenterPoint());

    x_AdjustCrossPane();
}


float CCrossAlnRenderer::GetZoomX_S(void) const
{
    TModelUnit scale_x = log(m_RP_S.GetScaleX());
    return (float)(1.0f - (scale_x - m_MinZoomX) / (m_MaxZoomX_S - m_MinZoomX));
}


void CCrossAlnRenderer::SetZoomX_S(float value)
{
    TModelUnit scale = m_MinZoomX + (m_MaxZoomX_S - m_MinZoomX)
                    * (1.0f - value);

    m_RP_S.SetScale(exp(scale), m_RP_S.GetScaleY(),
                    m_RP_S.GetVisibleRect().CenterPoint());

    x_AdjustCrossPane();
}



void CCrossAlnRenderer::x_AdjustCrossPane()
{
    m_CrossPanel.SetLimits(m_RP_Q.GetViewport(), m_RP_Q.GetVisibleRect(),
                           m_RP_S.GetViewport(), m_RP_S.GetVisibleRect());
}


void CCrossAlnRenderer::SetScaleRef_Q(TModelUnit scale_x, TModelUnit scale_y,
                                     const TModelPoint& point)
{
    if(scale_x > m_RP_Q.GetMinScaleX())  {
        TModelPoint ref_p(point.X(),  m_RP_Q.GetVisibleRect().Top());

        m_RP_Q.SetScaleRefPoint(scale_x, 1, ref_p); // -1
        x_AdjustCrossPane();
    }
}



void CCrossAlnRenderer::SetScaleRef_S(TModelUnit scale_x, TModelUnit scale_y,
                                     const TModelPoint& point)
{
    if(scale_x > m_RP_S.GetMinScaleX())  {
        TModelPoint ref_p(point.X(),  m_RP_S.GetVisibleRect().Top());

        m_RP_S.SetScaleRefPoint(scale_x, 1, ref_p); // -1
        x_AdjustCrossPane();
    }
}


void CCrossAlnRenderer::SetScaleRef_C(TModelUnit scale_x, TModelUnit scale_y,
                                     const TModelPoint& point)
{
}


void CCrossAlnRenderer::SquareOnRect(TModelRect rc)
{
    // calculate model space limits
    TModelUnit sMl = m_RP_S.GetModelLimitsRect().Left();
    TModelUnit sMr = m_RP_S.GetModelLimitsRect().Right();

    TModelUnit qMl = m_RP_Q.GetModelLimitsRect().Left();
    TModelUnit qMr = m_RP_Q.GetModelLimitsRect().Right();

    // our accuracy is within 5%
    TModelUnit s5l = sMl / 100.0f * 5.0f;
    TModelUnit s5r = sMr / 100.0f * 5.0f;

    TModelUnit q5l = qMl / 100.0f * 5.0f;
    TModelUnit q5r = qMr / 100.0f * 5.0f;

    // calculate scale limits
    TModelUnit qSx  = m_RP_Q.GetScaleX();
    TModelUnit sSx  = m_RP_S.GetScaleX();

    // scale limits for subject sequence
    TModelUnit s5s = sSx / 100.0f * 5.0f;
    TModelUnit minAVs = sSx - s5s;
    TModelUnit maxAVs = sSx + s5s;

    // scale limits for query sequence
    TModelUnit q5s = qSx / 100.0f * 5.0f;
    TModelUnit minAVq = qSx - q5s;
    TModelUnit maxAVq = qSx + q5s;

    // our new zoom factor
    m_RP_S.SetZoomFactor(1.05f);
    m_RP_Q.SetZoomFactor(1.05f);

    // initially, zoom all the way
    ZoomOnRange_Q(rc.Left(), rc.Right());
    ZoomOnRange_S(rc.Bottom(), rc.Top());

    // "slowly" zoom out until the scales are correct or until nowhere to zoom
    do {
        TModelUnit sVl = m_RP_S.GetVisibleRect().Left();
        TModelUnit qVl = m_RP_Q.GetVisibleRect().Left();
        TModelUnit sVr = m_RP_S.GetVisibleRect().Right();
        TModelUnit qVr = m_RP_Q.GetVisibleRect().Right();

        TModelUnit qSx  = m_RP_Q.GetScaleX();
        TModelUnit sSx  = m_RP_S.GetScaleX();

        // check limits first, and then the scales
        if (sVl <= sMl + s5l ||  qVl <= qMl + q5l  ||
            sVr >= sMr - s5r ||  qVr >= qMr - q5r  ||
            (qSx >= minAVq  &&  qSx <= maxAVq) ||
            (sSx >= minAVs  &&  sSx <= maxAVs) ) {
            break;
        } else {
            m_RP_S.ZoomOutCenter();
            m_RP_Q.ZoomOutCenter();
        }
    } while (true);

    x_AdjustCrossPane();
}


void CCrossAlnRenderer::ZoomOnRange_Q(TModelUnit from, TModelUnit to)
{
    TModelRect& rc = m_RP_Q.GetVisibleRect();
    rc.SetHorz(from, to);
    m_RP_Q.ZoomRect(rc);

    // Now add five pixels offset on each side
    m_RP_Q.OpenOrtho();
    TModelUnit pQ = m_RP_Q.UnProjectWidth(5);
    m_RP_Q.Close();

    rc.SetHorz(from - pQ, to + pQ);
    m_RP_Q.ZoomRect(rc);

    x_AdjustCrossPane();
}


void CCrossAlnRenderer::ZoomOnRange_S(TModelUnit from, TModelUnit to)
{
    TModelRect& rc = m_RP_S.GetVisibleRect();
    rc.SetHorz(from, to);
    m_RP_S.ZoomRect(rc);

    // Now add five pixels offset on each side
    m_RP_S.OpenOrtho();
    TModelUnit pS = m_RP_S.UnProjectWidth(5);
    m_RP_S.Close();

    rc.SetHorz(from - pS, to + pS);
    m_RP_S.ZoomRect(rc);

    x_AdjustCrossPane();
}



// Query
void CCrossAlnRenderer::ZoomIn_Q()
{
    m_RP_Q.ZoomInCenter();
    x_AdjustCrossPane();
}


void CCrossAlnRenderer::ZoomOut_Q()
{
    m_RP_Q.ZoomOutCenter();
    x_AdjustCrossPane();
}


void CCrossAlnRenderer::ZoomAll_Q(void)
{
    SetZoomX_Q(0.0f);
}

void CCrossAlnRenderer::ZoomToSeq_Q(void)
{
    SetZoomX_Q(1.0f);
}


// Subject
void CCrossAlnRenderer::ZoomIn_S()
{
    m_RP_S.ZoomInCenter();
    x_AdjustCrossPane();
}


void CCrossAlnRenderer::ZoomOut_S()
{
    m_RP_S.ZoomOutCenter();
    x_AdjustCrossPane();
}


void CCrossAlnRenderer::ZoomAll_S(void)
{
    SetZoomX_S(0.0f);
}


void CCrossAlnRenderer::ZoomToSeq_S(void)
{
    SetZoomX_S(1.0f);
}


CGlPane& CCrossAlnRenderer::GetCP()
{
    return m_CP;
}

const CGlPane& CCrossAlnRenderer::GetCP() const
{
    return m_CP;
}


CGlPane& CCrossAlnRenderer::GetRP_Q()
{
    return m_RP_Q;
}


CGlPane& CCrossAlnRenderer::GetRP_S()
{
    return m_RP_S;
}


void CCrossAlnRenderer::Scroll(TModelUnit dxQ, TModelUnit dxS)
{
    m_RP_Q.Scroll(dxQ, 0.0f);
    m_RP_S.Scroll(dxS, 0.0f);

    x_AdjustCrossPane();
}


END_NCBI_SCOPE
