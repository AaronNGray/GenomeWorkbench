/*  $Id: alignment_smear_glyph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *   CAlignSmearGlyph -- utility class to layout mate pairs (a special type of
 *      pairwise alignments and hold a set of CAlignGlyph
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/alignment_smear_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/objutils/tooltip.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CAlignSmearGlyph::CAlignSmearGlyph(
    const objects::CBioseq_Handle& handle,
    TSeqPos start, TSeqPos stop, float window,
    CAlignmentSmear::EAlignSmearStrand strand_type)
    : m_AlignSmear(new CAlignmentSmear(handle, start, stop, window, strand_type))
{}

CAlignSmearGlyph::CAlignSmearGlyph(
    const objects::CBioseq_Handle& handle,
    TSeqPos start, TSeqPos stop, float window,
    CAlignmentSmear::EAlignSmearStrand strand_type,
    const objects::CSeq_annot& seq_annot)
    : m_AlignSmear(new CAlignmentSmear(handle, start, stop, window, strand_type))

{
    m_AlignSmear->AddAnnot(seq_annot);
}


bool CAlignSmearGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    GetTooltip(p, tt, t_title);
    return true;
}


void CAlignSmearGlyph::GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& tt, string& /*t_title*/) const
{
    tt.AddRow(GetAlignSmear().GetLabel());
}


bool CAlignSmearGlyph::IsClickable() const
{
    return true;
}


void CAlignSmearGlyph::x_Draw() const
{
    IRender& gl = GetGl();

    const CAlignmentSmear& smear = GetAlignSmear();
    TModelRect rcm = GetModelRect();
    TSeqRange r = GetRange();
    TModelRange inrc = m_Context->IntersectVisible(r);

    TModelUnit fs = 0;
    if (m_Config->m_ShowLabel) {
        fs = gl.TextHeight(&(m_Config->m_LabelFont));
    }
    TModelUnit base    = rcm.Top();
    TModelUnit line_y1 = base + fs + 4;
    TModelUnit line_y2 = line_y1 + m_Config->m_BarHeight;
    TModelUnit line_ym = line_y1 + m_Config->m_BarHeight * 0.5;

    // Draw smear
    CAlignmentSmear::runlen_iterator seg_it = smear.SmearSegBegin();
    for (; seg_it; seg_it.Advance()) {
        TModelUnit  value = seg_it.GetValue();
        if (value == 0)
            continue;

        TModelUnit f = seg_it.GetSeqPosition();
        TModelUnit t = f + seg_it.GetSeqRunLength();

        // skip invisible bars
        if (f > m_Context->GetVisSeqTo()  ||
            t < m_Context->GetVisSeqFrom())
            continue;

        // base color
        //gl.Color4f(maxColor.GetRed(), maxColor.GetGreen(), maxColor.GetBlue(), 1.0f);
        //m_Context->DrawLine(f, line_ym, t, line_ym, horz);

        // set color of segment.
        float score = (float)(value / smear.GetMaxValue());
        CRgbaColor color(CRgbaColor::Interpolate(m_Config->m_SmearColorMax, m_Config->m_SmearColorMin, score));
        gl.Color4f(color.GetRed(), color.GetGreen(), color.GetBlue(), 1.0f);

        // draw segment as quad
        m_Context->DrawQuad(f, line_y1, t, line_y2);
    }

    // Draw gaps
    CAlignmentSmear::runlen_iterator gap_it = smear.SmearGapBegin();
    for (; gap_it; gap_it.Advance()) {
        TModelUnit  value = gap_it.GetValue();
        if (value == 0)
            continue;

        TModelUnit f = gap_it.GetSeqPosition();
        TModelUnit t = f + gap_it.GetSeqRunLength();

        gl.Color4f(m_Config->m_SmearColorMax.GetRed(),
                   m_Config->m_SmearColorMax.GetGreen(),
                   m_Config->m_SmearColorMax.GetBlue(), 1.0f);
        m_Context->DrawLine(f, line_ym, t, line_ym);
    }

    // Draw label
    if (m_Config->m_ShowLabel  &&  m_Context->WillLabelFit(inrc)) {
        if (IsSelected()) {
            gl.ColorC(m_Context->GetSelLabelColor());
        } else {
            gl.ColorC(m_Config->m_Label);
        }

        string label = smear.GetLabel();
        if (smear.GetStrandType() != CAlignmentSmear::eSmearStrand_Both) {
            if (smear.GetStrandType() == CAlignmentSmear::eSmearStrand_Neg)
                label += " (negative strand)";
            else
                label += " (positive strand)";
        }

        TModelUnit xM = (inrc.GetFrom() + inrc.GetTo()) * 0.5;
        TModelUnit yM = rcm.Top() + fs;
        m_Context->TextOut(&m_Config->m_LabelFont, label.c_str(), xM, yM, true);
    }
}


void CAlignSmearGlyph::x_UpdateBoundingBox()
{
    IRender& gl = GetGl();

    const TModelRange& range = m_Context->GetVisibleRange();
    SetWidth(range.GetLength());
    SetLeft(range.GetFrom());
    SetHeight(m_Config->m_BarHeight + 2.0);
    if (m_Config->m_ShowLabel) {
        SetHeight(GetHeight() + gl.TextHeight(&(m_Config->m_LabelFont)) + 4);
    }
}


END_NCBI_SCOPE

