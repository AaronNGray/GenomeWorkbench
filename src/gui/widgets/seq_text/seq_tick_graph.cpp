/*  $Id: seq_tick_graph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
 * Authors:  Colleen Bollin (adapted from a file by Andrey Yazhuk)
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistl.hpp>
#include <gui/widgets/seq_text/seq_tick_graph.hpp>

#include <gui/objutils/label.hpp>
#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/irender.hpp>

#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>

#include <math.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CSequenceTickGraph::CSequenceTickGraph()
:   m_pSeqFont(NULL),
    m_BackColor(0.9f, 0.9f, 0.9f),
    m_TextColor(0.0f, 0.0f, 0.0f),
    m_ShowAbsolutePosition (true),
    m_pGeometry (NULL)
{
}

void CSequenceTickGraph::SetFont(CGlTextureFont* seq_font)
{
    m_pSeqFont = seq_font;
}

void CSequenceTickGraph::SetDataSource(CSeqTextDataSource* p_ds)
{
    m_DataSource = p_ds;
}

static const int kMargin = 2;
static const int kTextOffset = 2;

TVPPoint CSequenceTickGraph::PreferredSize()
{
    int spacing = 2 * kMargin;
    TVPPoint size(spacing, spacing);
    int num_chars = 15;
    TSeqPos last_val;

    TModelUnit seq_h = m_pSeqFont->GetMetric(CGlTextureFont::eMetric_FullCharHeight);
    if (m_DataSource) {
        last_val = m_DataSource->GetDataLen();
        if (m_ShowAbsolutePosition) {
            last_val = m_DataSource->SourcePosToSequencePos(last_val - 1);
        }
        num_chars = CTextUtils::GetCharsCount (last_val);
    }

    size.m_X += (int)((num_chars + kMargin) * m_pSeqFont->GetMetric (CGlTextureFont::eMetric_MaxCharWidth));

    size.m_Y += (int) ceil(seq_h);
    return size;
}


void CSequenceTickGraph::Render(CGlPane& pane)
{
    if (m_DataSource)
    {
        _ASSERT(m_pSeqFont);
        _ASSERT(m_pGeometry);

        IRender& gl = GetGl();

        TSeqPos chars_in_line, lines_in_seq;
        m_pGeometry->STG_GetLineInfo(chars_in_line, lines_in_seq);;
        if (chars_in_line < 1) {
            return;
        }

        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //const   TVPRect&    rc_VP = pane.GetViewport();
        // rc_model is a rectangle that describes the visible portion of the pane
        TModelRect& rc_model = pane.GetModelLimitsRect();
        TModelRect& rc_vis = pane.GetVisibleRect();
        TVPRect rc_vp = pane.GetViewport();
        TVPPoint pref_size = PreferredSize();

        rc_vp.SetRight (rc_vp.Left() + pref_size.m_X);
        pane.SetViewport(rc_vp);

        rc_model.SetHorz (0, pref_size.m_X / m_pSeqFont->GetMetric (CGlTextureFont::eMetric_MaxCharWidth));
        rc_vis.SetHorz (0, pref_size.m_X / m_pSeqFont->GetMetric (CGlTextureFont::eMetric_MaxCharWidth));
        pane.SetModelLimitsRect (rc_model);
        pane.SetVisibleRect (rc_vis);

        TModelUnit scale_x = pane.GetScaleX();
        TModelUnit scale_y = pane.GetScaleY();

        // fill background
        TVPRect rc_back = m_VPRect;
        rc_back.Inflate(0, -kMargin);

        {
            CGlPaneGuard GUARD(pane, CGlPane::ePixels);
            gl.ColorC(m_BackColor);
            gl.Rectd(rc_back.Left(), rc_back.Bottom(), rc_back.Right(), rc_back.Top());
        } 

        CGlPaneGuard GUARD(pane, CGlPane::eOrtho);
        gl.ColorC(m_TextColor);

        // draw sequence start numbers
        TSeqPos vis_start, vis_stop;
        m_pGeometry->STG_GetVisibleRange (vis_start, vis_stop);

        const TModelRect& rc_limits = pane.GetModelLimitsRect ();

        string seq;
        TModelUnit print_start;

        while (vis_start < vis_stop && vis_start < m_DataSource->GetDataLen()) {
            TModelPoint display_pos = m_pGeometry->STG_GetModelPointBySourcePos(vis_start);
            TSeqPos seq_start = vis_start;
            if (m_ShowAbsolutePosition) {
                seq_start = m_DataSource->SourcePosToSequencePos (vis_start);
            }
            seq = CTextUtils::FormatSeparatedNumber((int) seq_start + 1, false);

            print_start = rc_limits.Right() - seq.length() - kMargin;
            m_pSeqFont->ArrayTextOut((float) print_start, (float) display_pos.m_Y, 1.0f, 0.0f,
                                     seq.c_str(), NULL,
                                     (float) scale_x, (float) scale_y);
            vis_start += chars_in_line;
        }
    }
}


bool CSequenceTickGraph::NeedTooltip(CGlPane& /*pane*/, int vp_x, int vp_y)
{
    return m_VPRect.PtInRect(vp_x, vp_y);
}


string  CSequenceTickGraph::GetTooltip()
{
    if (m_DataSource)   {
        return m_DataSource->GetTitle();
    } else {
        return "No sequence loaded";
    }
}


TModelRect CSequenceTickGraph::GetModelRect() const
{
    TSeqPos len = 0;

    if (m_DataSource) {
     CSequenceTickGraph* nc_this = const_cast<CSequenceTickGraph*>(this);
        len = nc_this->m_DataSource->GetDataLen();
    }
    TModelRect rc_model(0.0, 0.0, len, 1.0);
    return rc_model;
}


void CSequenceTickGraph::SetModelRect(const TModelRect& rc)
{
    _ASSERT(false); // not supported
}


void CSequenceTickGraph::ShowAbsolutePosition (bool show_abs)
{
    m_ShowAbsolutePosition = show_abs;
}


END_NCBI_SCOPE
