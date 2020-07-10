/*  $Id: translation_glyph.cpp 43675 2019-08-14 14:10:58Z katargir $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/translation_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/objutils/tooltip.hpp>

#include <cmath>

BEGIN_NCBI_SCOPE

CTranslationGlyph::CTranslationGlyph(EFrame frame) : m_Frame(frame)
{}

CTranslationGlyph::CTranslationGlyph(TSeqPos start, TSeqPos stop,EFrame frame)
        : m_Frame(frame)
{
    if (start < stop) {
        SetLeft(start);
        SetWidth(stop - start);
    } else {
        SetLeft(stop);
        SetWidth(start - stop);
    }
}


bool CTranslationGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    GetTooltip(p, tt, t_title);
    return !tt.IsEmpty();
}


void CTranslationGlyph::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& /*t_title*/) const
{
    bool negative = NegativeStrand();
    ITERATE (TORFs, iter, m_Orfs) {
        TSeqPos f = iter->GetFrom();
        TSeqPos t = iter->GetTo();
        if (negative) swap(f, t);
        t += 2;
        if (p.X() >= f  &&  p.X() <= t) {
            tt.AddSectionRow("Sequence translation");

            string value = NStr::IntToString(m_Frame % 3 + 1) + " in ";
            value +=  NegativeStrand() ? "negative strand" : "positive strand";
            tt.AddRow("Reading frame:", value);

            tt.AddRow("Total length:", NStr::IntToString(t - f + 1, NStr::fWithCommas));
            
            tt.AddRow("Start:", NStr::UIntToString(negative ? t : f, NStr::fWithCommas));
            
            tt.AddRow("Stop:", NStr::UIntToString(negative ? f : t, NStr::fWithCommas));
            
            tt.AddRow("Position:", NStr::UIntToString((TSeqPos)p.X() + 1, NStr::fWithCommas));
            break;
        }
    }
}


void CTranslationGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    CHTMLActiveArea area;
    CSeqGlyph::x_InitHTMLActiveArea(area);

    area.m_Bounds.SetLeft(-1);
    area.m_Bounds.SetRight(0);
    //    area.m_Bounds.Offset(0, -2);
    area.m_Flags = CHTMLActiveArea::fComment
        | CHTMLActiveArea::fDrawBackground
        | CHTMLActiveArea::fNoHighlight
        | CHTMLActiveArea::fNoSelection
        | CHTMLActiveArea::fTooltipEmbedded;

    area.m_Descr = "Sequence translation<br>";
    area.m_Descr += "Reading frame: " + NStr::IntToString(m_Frame % 3 + 1) + " in "; 
    area.m_Descr +=  NegativeStrand() ? "negative strand" : "positive strand";
    area.m_ID = x_GetFrameLabel();

    // required, but nonsense fields
    area.m_PositiveStrand = true;
    area.m_SeqRange.SetFrom(0);
    area.m_SeqRange.SetTo(0);

    p_areas->push_back(area);
}


bool CTranslationGlyph::IsClickable() const
{
    return false;
}


void CTranslationGlyph::x_Draw() const
{
    IRender& gl = GetGl();

    bool show_seq = m_Context->WillTransLettersFit();
    TSeqPos vis_from = m_Context->GetVisSeqFrom();
    TSeqPos vis_to = m_Context->GetVisSeqTo();
  
    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
    vis_to = min(vis_to, seq_ds->GetSequenceLength() - 1);
    bool negative = NegativeStrand();
    int strand_factor = negative ? -1 : 1;
    TModelUnit off = 0.5;

    TModelUnit half_h = floor(m_Config->GetBarHeight() * 0.5 + 0.5);
    TModelUnit center_y = GetTop() + half_h;
    TModelUnit end_len = m_Context->ScreenToSeq(half_h);

    half_h -= 2.0;

    ITERATE (TORFs, iter, m_Orfs) {
        TSeqPos orf_start = iter->GetFrom();
        TSeqPos orf_stop = iter->GetTo();

        //  early termination
        if ( (negative  &&  orf_start < vis_from)  ||
              (!negative  &&  orf_start > vis_to) ) {
            // no need to do anything for the rest ORFs.
            break;
        }

        // check if it is in the visible range
        if ( !x_Intersecting(orf_start, orf_stop, vis_from, vis_to) ) {
            // skip this ORF, go to next one.
            continue;
        }

        // highlight ORF
        // orf_start or orf_stop is at the middle base. Need to
        // compensate the first base and last base
        int orf_len = (orf_stop - orf_start) * strand_factor + 1 + 2;
        if (orf_len > m_Config->GetOrfThreshold() * 3) {
            gl.ColorC(m_Config->GetOrfHighlightColor());
            TModelUnit x1 = orf_start + (negative ? 2.0 : -1.0);
            TModelUnit x2 = x1 + orf_len * strand_factor;
            m_Context->DrawQuad(x1, center_y - half_h, x2, center_y + half_h);
            // draw strand indicator
            // do not draw strand when sequence is shown
            const CGlTextureFont& font = m_Config->GetStrandFont();
            if (!show_seq  &&  orf_len > m_Context->ScreenToSeq(gl.TextWidth(&font, "<<"))) {
                TModelUnit xM = (x1 + x2) * 0.5;
                TModelUnit yM = center_y +  gl.TextHeight(&font) * 0.5 - 1.0;
                char strandLabel[2] = {'>', '\0'};
                if (m_Context->IsFlippedStrand()){
                    strandLabel[0] = negative ? '>' : '<';
                } else {
                    strandLabel[0] = negative ? '<' : '>';
                }
                gl.Color4f(0.0f, 0.0f, 0.0f, 1.0f);  // Black
                m_Context->TextOut(&font, strandLabel, xM, yM, true);
            }
        }

        // draw ORF sequence
        if (show_seq) {
            if (orf_len > m_Config->GetOrfThreshold() * 3) {
                gl.ColorC(m_Config->GetOrfSeqColor());
            } else {
                gl.ColorC(m_Config->GetSeqColor());
            }
            x_DrawSequence(orf_start, orf_stop, vis_from, vis_to, center_y, negative);
        }
    }

    // highlight codons
    if (m_Config->GetHighlightCodons()) {
        TModelUnit half_len = 1.5;

        // highlight start codons
        ITERATE (TCodons, iter, m_StartCodons) {
            // early termination
            if ( (negative  &&  *iter < vis_from)  ||
                  (!negative  &&  *iter > vis_to) ) {
                break;
            }

            // check if it is in the visible range
            if (*iter < vis_from  ||  *iter > vis_to) {
                continue;
            }

            gl.ColorC(m_Config->GetStartCodonColor());
            TModelUnit x1 = *iter - half_len + off;
            TModelUnit x2 = *iter + half_len + off;
            m_Context->DrawQuad(x1, center_y - half_h, x2, center_y + half_h);
        }

        // highlight stop codon
        ITERATE (TCodons, iter, m_StopCodons) {
            // early termination
            if ( (negative  &&  *iter < vis_from)  ||
                  (!negative  &&  *iter > vis_to) ) {
                break;
            }

            // check if it is in the visible range
            if (*iter < vis_from  ||  *iter > vis_to) {
                continue;
            }

            gl.ColorC(m_Config->GetStopCodonColor());
            TModelUnit x1 = *iter - half_len + off;
            TModelUnit x2 = *iter + half_len + off;
            m_Context->DrawQuad(x1, center_y - half_h, x2, center_y + half_h);
        }
    }

    // draw translated sequence
    if (show_seq) {
        TSeqPos pre_stop = negative ? vis_to + 3 : 0;
        ITERATE (TORFs, iter, m_Orfs) {
            TSeqPos orf_start = iter->GetFrom();
            TSeqPos orf_stop = iter->GetTo();

            //  early termination
            if ( (negative  &&  orf_start < vis_from)  ||
                (!negative  &&  orf_start > vis_to) ) {
                    // no need to do anything for the rest ORFs.
                    break;
            }

            // check if it is in the visible range
            if ( !x_Intersecting(orf_start, orf_stop, vis_from, vis_to) ) {
                // skip this ORF, go to next one.
                continue;
            }

            // draw translation sequence between ORFs
            if (x_Intersecting(pre_stop, orf_start, vis_from, vis_to)) {
                gl.ColorC(m_Config->GetSeqColor());
                x_DrawSequence(pre_stop, orf_start, vis_from, vis_to, center_y, negative);
            }
            pre_stop = orf_stop + strand_factor * 3;

            // draw ORF sequence
            int orf_len = (orf_stop - orf_start) * strand_factor + 1 + 2;
            if (orf_len > m_Config->GetOrfThreshold() * 3) {
                gl.ColorC(m_Config->GetOrfSeqColor());
            } else {
                gl.ColorC(m_Config->GetSeqColor());
            }
            x_DrawSequence(orf_start, orf_stop, vis_from, vis_to, center_y, negative);
        }

        TSeqPos end_pos = negative ? 0 : vis_to + 3;
        if (x_Intersecting(pre_stop, end_pos, vis_from, vis_to)) {
            gl.ColorC(m_Config->GetSeqColor());
            // draw translation sequence at the end
            x_DrawSequence(pre_stop, end_pos, vis_from, vis_to, center_y, negative);
        }
    }

    if (m_Config->ShowLabel()) {
        // draw translation label
        string label = x_GetFrameLabel();
        TModelUnit left = m_Context->GetVisibleFrom();
        TModelUnit y = center_y + gl.TextHeight(&(m_Config->GetSeqFont())) * 0.5;
        gl.ColorC(m_Config->GetLabelBgColor());
        m_Context->DrawQuad(left, GetTop(), left + end_len * 3.0, GetBottom());
        gl.ColorC(m_Config->GetOrfSeqColor());
        m_Context->TextOut(&m_Config->GetSeqFont(),
                           label.c_str(), left + end_len * 1.5, y, true);
    }
}


void CTranslationGlyph::x_UpdateBoundingBox()
{
    SetHeight(m_Config->GetBarHeight());
}


void CTranslationGlyph::x_DrawSequence(TSeqPos start,
                                       TSeqPos stop,
                                       TSeqPos vis_from,
                                       TSeqPos vis_to,
                                       TModelUnit center_y,
                                       bool negative) const
{
    if (m_TransSeq.empty()) return;
    IRender& gl = GetGl();

    if (start > stop) swap(start, stop);
    start = max(start, vis_from);
    stop = min(stop + 1, vis_to);

    int len = m_TransSeq.size();
    TSeqPos shift = m_Frame % 3;
    int i = negative ? (TSeqPos)GetRight() - start : start - (TSeqPos)GetLeft();
    int stop_i = negative ? (TSeqPos)GetRight() - stop : stop - (TSeqPos)GetLeft();
    i = (i < (int)shift ? 0 : i - shift) / 3;
    stop_i = (stop_i < (int)shift ? 0 : stop_i - shift) / 3;
    i = min(i, len - 1);
    stop_i = min(stop_i, len - 1);

    char seq_char[2];
    seq_char[1] = '\0';
    TModelUnit start_x = negative ? GetRight() - shift - 1 : GetLeft() + shift + 1;
    TModelUnit y = center_y + gl.TextHeight(&(m_Config->GetSeqFont())) * 0.5;
    int step = negative ? -1 : 1;
    while ( (negative  &&  i >= stop_i)  ||  (!negative  &&  i <= stop_i)) {
        seq_char[0] = m_TransSeq[i];
        m_Context->TextOut(&m_Config->GetSeqFont(), seq_char,
            start_x + (negative ? -3.0 : 3.0) * i + 0.5, y, true);
        i += step;
    }
}


bool CTranslationGlyph::x_Intersecting(TSeqPos f1, TSeqPos t1,
                                       TSeqPos f2, TSeqPos t2) const
{
    if (f1 > t1) swap(f1, t1);
    if (f2 > t2) swap(f2, t2);
    return TSeqRange(f1, t1).IntersectingWith(TSeqRange(f2, t2));
}


string CTranslationGlyph::x_GetFrameLabel() const
{
    char label[3];
    label[2] = '\0';
    label[1] = m_Frame % 3 + 48 + 1;
    if (NegativeStrand()) label[0] = '-';
    else label[0] = '+';
    return string(label);
}

END_NCBI_SCOPE
