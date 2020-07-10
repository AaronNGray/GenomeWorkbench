/*  $Id: alignment_glyph.cpp 44759 2020-03-05 19:38:06Z shkeda $
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
 *   CAlignGlyph -- utility class to layout alignments
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <objmgr/util/sequence.hpp>
#include <algo/sequence/consensus_splice.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seqalign/Spliced_seg.hpp>
#include <objects/seqalign/Spliced_exon.hpp>
#include <objects/seqalign/Splice_site.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>

#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/alignment_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/gui_object_info_seq_align.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/utils.hpp>

#include <gui/opengl/irender.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/rasterizer.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <util/sequtil/sequtil_manip.hpp>

#include <gui/widgets/gl/attrib_menu.hpp>

#include <objects/seqfeat/Genetic_code.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// How closely pack align. sequence when showing DNA alignments for proteins
// (pack 3 alignment bases into 1 sequence base)
static const TModelUnit kPWAlignPackLevel = 0.333;
static const TSeqPos kTailGlyphLen = 12;
static const TSeqPos kCompactTailGlyphLen = 2;

static const int kAlignRowSpace = 3;
static const int kLabelSpace_v = 2;

static const int kLabelSpace_h = 5;
static const int kCompactLabelSpace_h = 1;

CAlignGlyph::CAlignGlyph(CConstRef<IAlnGraphicDataSource> aln_mgr,
    const CSeq_align_Handle& orig_align,
    const CSeq_align& mapped_align,
    int orig_anchor)
    : m_AlnMgr(aln_mgr)
    , m_SeqAlign(&mapped_align)
    , m_OrigSeqAlign(orig_align)
    , m_OrigAnchor(orig_anchor)
    , m_ShowScore(false)
    , m_UnalignedRegions(false)
    , m_FadeFactor(1.0f)
{
    IAlnGraphicDataSource::TNumrow anchor = m_AlnMgr->GetAnchor();
    TSeqPos base_width_anchor = m_AlnMgr->GetBaseWidth(anchor);

    TSignedSeqPos start = m_AlnMgr->GetSeqStart(anchor) / base_width_anchor;
    TSignedSeqPos stop  = m_AlnMgr->GetSeqStop (anchor) / base_width_anchor;

    m_Location.Reset(new CSeq_loc());
    m_Location->SetInt().SetFrom(start);
    m_Location->SetInt().SetTo  (stop);
    if (x_IsPWAlign()) {
        if (m_AlnMgr->IsNegativeStrand(0) == m_AlnMgr->IsNegativeStrand(1)) {
            m_Location->SetStrand(eNa_strand_plus);
        } else {
            m_Location->SetStrand(eNa_strand_minus);
        }
    }

    CConstRef<CSeq_id> id(&m_AlnMgr->GetSeqId(anchor));
    m_Location->SetId(*id);
}


bool CAlignGlyph::NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& /*t_title*/) const
{
    return true;
}


void CAlignGlyph::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    if (!GetVisible())
        return;
    CScope& scope = m_Context->GetScope();
    SConstScopedObject scoped_obj(&GetAlignment(), &scope);
    CIRef<IGuiObjectInfo> gui_info(
        CreateObjectInterface<IGuiObjectInfo>(scoped_obj, NULL));

    if ( !gui_info ) return;

    CGuiObjectInfoSeq_align* gui_info_aln =
        dynamic_cast<CGuiObjectInfoSeq_align*>(gui_info.GetPointer());
    if (gui_info_aln) {
        gui_info_aln->SetAnchorRow(m_OrigAnchor);
        gui_info_aln->SetFlippedStrands(m_Context->IsFlippedStrand());
    }

    TSeqPos at_p = (TSeqPos)-1;
    if (p.X() >= 0) {
        at_p = (TSeqPos)p.X();
    }
   
    gui_info->GetToolTip(tt, t_title, at_p);
    gui_info->GetLinks(tt, false);

    if (IsLastRow()) {        
        tt.AddRow("");
        tt.AddRow("Last displayed alignment");
        tt.AddRow("Alignment rows truncated for 'Adaptive' display");
        tt.AddRow("To see all alignments select 'Show All'");
    }
}


void CAlignGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    if (!GetVisible())
        return;
    CHTMLActiveArea area;
    CSeqGlyph::x_InitHTMLActiveArea(area);
    if (m_AlnMgr->GetNumRows() == 2) {
        area.m_PositiveStrand =
            m_AlnMgr->IsNegativeStrand(0) == m_AlnMgr->IsNegativeStrand(1);
    }
    area.m_Signature = GetSignature();
    if (m_UnalignedRegions)
        area.m_Flags |= CHTMLActiveArea::fNoCaching;
    p_areas->push_back(area);
}


TSeqRange CAlignGlyph::GetRange(void) const
{
    if (m_Intervals.size() == 1) {
        return m_Intervals.front();
    }

    return m_Location->GetTotalRange();
}


bool CAlignGlyph::IsClickable() const
{
    return true;
}


CConstRef<CObject> CAlignGlyph::GetObject(TSeqPos) const
{
    return CConstRef<CObject>(m_OrigSeqAlign.GetSeq_align());
}


void CAlignGlyph::GetObjects(vector<CConstRef<CObject> >& objs)  const
{
    objs.push_back( CConstRef<CObject>(m_OrigSeqAlign.GetSeq_align()));
}


bool CAlignGlyph::HasObject(CConstRef<CObject> obj) const
{
    return m_OrigSeqAlign.GetSeq_align() == obj.GetPointer();
}


const objects::CSeq_loc& CAlignGlyph::GetLocation(void) const
{
    return *m_Location;
}


string CAlignGlyph::GetSignature() const
{
    if (x_IsSRA()) {
        int aligned_seq = m_AlnMgr->GetAnchor() == 0 ? 1 : 0;
        CSeq_loc csra_loc;
        csra_loc.Assign(*m_Location);
        csra_loc.SetId(m_AlnMgr->GetSeqId(aligned_seq));
        return CObjFingerprint::GetAlignSignature(
                    GetOrigAlignment().GetSeq_align().GetObject(),
                    csra_loc, &m_Context->GetScope(), m_OrigSeqAlign.GetAnnot());
    }
    else {
        return CObjFingerprint::GetAlignSignature(
                    GetOrigAlignment().GetSeq_align().GetObject(),
                    GetLocation(), &m_Context->GetScope(), m_OrigSeqAlign.GetAnnot());
    }
}


const CAlignGlyph::TIntervals& CAlignGlyph::GetIntervals(void) const
{
    return m_Intervals;
}


inline double CompressLength(double len)
{
    if(len <= 10.0) {
        return len;
    } else {
        double log = log10(len) * 10;
        return log;
    }
}


void CAlignGlyph::x_DrawGaps(const SDrawContext& ctx) const
{
    if (ctx.seq_range.Empty())
        return;

    IRender& gl = GetGl();
    int anchor = m_AlnMgr->GetAnchor();
    int row_num = m_AlnMgr->GetNumRows();

    // color the gap differently
    //CGlAttrGuard AttrGuard(GL_LINE_BIT);
    gl.Disable(GL_LINE_SMOOTH);

    const TModelUnit y_center = ctx.yy + ctx.half;
    CRgbaColor gap_color_min(0.78f, 0.5f, 0.5f, m_FadeFactor);
    CRgbaColor gap_color_max(0.6f, 0.0f, 0.0f, m_FadeFactor);

    for (int row = 0;  row < row_num;  ++row) {
        if (row == anchor) {
            continue;  // skip master seq
        }
        auto_ptr<IAlnSegmentIterator> p_it(
            m_AlnMgr->CreateSegmentIterator(row,
            ctx.aln_range, IAlnSegmentIterator::eAllSegments));

        for (IAlnSegmentIterator& it = *p_it;  it;  ++it) {
            const IAlnSegment& seg = *it;
            if ( !(seg.GetType() & IAlnSegment::fIndel)  ||
                    !seg.GetRange().Empty() ) {
                        continue;
            }

            const IAlnSegment::TSignedRange& curr_aln_r = seg.GetAlnRange();
            TModelUnit curr_seq_from =
                m_AlnMgr->GetSeqPosFromAlnPos(anchor, curr_aln_r.GetFrom()) /
                ctx.base_width_anchor;
            TModelUnit curr_seq_to =
                m_AlnMgr->GetSeqPosFromAlnPos(anchor, curr_aln_r.GetTo()) /
                ctx.base_width_anchor;
            if (curr_seq_from > curr_seq_to) {
                swap (curr_seq_from, curr_seq_to);
            }
            curr_seq_to += 1.0 / ctx.base_width_anchor;
            TModelUnit gap_size = m_Context->SeqToScreen(curr_seq_to - curr_seq_from);
            if (gap_size > 1.5) {
                gl.LineWidth(2.0);
                gl.ColorC(CRgbaColor(m_Config->m_Gap, m_FadeFactor));
                const IAlnGraphicDataSource::TCigarPair *cigar = m_AlnMgr->GetCigarAtAlnPos(row, curr_aln_r.GetFrom());
                if ((nullptr != cigar) && (IAlnGraphicDataSource::eCigar_N == (*cigar).first)) {
                    gl.ColorC(CRgbaColor(m_Config->m_Intron, m_FadeFactor));
                }
                m_Context->DrawLine(curr_seq_from, y_center, curr_seq_to, y_center);
            } else {
                /*
                if (gap_size > 1.0) {
                    gap_size = 1.0;
                } else if (gap_size < 0.1) {
                    gap_size = 0.1;
                }
                if (gap_size > 1.0) {
                    CRgbaColor gap_color_ins(CRgbaColor::Interpolate(gap_color_max,
                        gap_color_min, gap_size));
                    gl.ColorC(gap_color_ins);
                    gl.LineWidth(1.0);
                    TModelUnit c_x = (curr_seq_from + curr_seq_to) * 0.5;
                    m_Context->DrawLine_NoAA(c_x, ctx.yy, c_x, ctx.yy + ctx.bar_h);
                }
                */
            }
        }
    }
    gl.LineWidth(1.0f);
}

void CAlignGlyph::x_DrawInserts(const SDrawContext& ctx) const
{
    if (ctx.seq_range.Empty())
        return;

    IRender& gl = GetGl();
    TModelUnit over = 2.0;
    TModelUnit down = 3.0;
    const TModelUnit y1 = ctx.yy + down;
    const TModelUnit y2 = ctx.yy + ctx.bar_h - down;

    CRgbaColor color_max = CRgbaColor(m_Config->m_Insertion, m_FadeFactor);
    CRgbaColor color_min = color_max;
    color_min.Lighten(0.6f);
    int anchor = m_AlnMgr->GetAnchor();
    int row_num = m_AlnMgr->GetNumRows();
    CGlAttrGuard AttrGuard(GL_LINE_BIT);
    gl.Disable(GL_LINE_SMOOTH);

    for (int row = 0;  row < row_num;  ++row) {
        if (row == anchor) {
            continue;  // skip master seq
        }
        auto_ptr<IAlnSegmentIterator> p_it(
            m_AlnMgr->CreateSegmentIterator(row,
            ctx.aln_range, IAlnSegmentIterator::eAllSegments));

        int last_aligned_seg_to_anchor = -2;
        int last_aligned_seg_to_curr = -2;
        int last_aligned_seg_from_anchor = -2;
        int last_aligned_seg_from_curr = -2;

        int last_large_ins = -100000;  // >= 5 bases

        int seg_iter_dir = 0;  // 0: unknown, 1: forward (anchor), -1: reverse (anchor)
        for (IAlnSegmentIterator& it = *p_it;  it;  ++it)  {
            const IAlnSegment& seg = *it;
            TSeqPos start = 0;
            int ins_len = 0;
            if ( !(seg.GetType() & IAlnSegment::fIndel)  ||
                seg.GetRange().Empty() ) {
                // it is not marked as insertion
                // check if it is intron
                if (seg.GetType() & IAlnSegment::fAligned) {
                    if (seg_iter_dir == 0) {
                        int anchor_to =
                            m_AlnMgr->GetSeqPosFromAlnPos(anchor, seg.GetAlnRange().GetTo(), IAlnExplorer::eRight);
                        int anchor_from =
                            m_AlnMgr->GetSeqPosFromAlnPos(anchor, seg.GetAlnRange().GetFrom(), IAlnExplorer::eRight);
                        if (last_aligned_seg_to_anchor < 0) {
                            // It must be the first segment (unknow iterator direction)
                            last_aligned_seg_to_anchor = anchor_to;
                            last_aligned_seg_from_anchor = anchor_from;
                            last_aligned_seg_to_curr = seg.GetRange().GetTo();
                            last_aligned_seg_from_curr = seg.GetRange().GetFrom();
                        } else {
                            if (anchor_to < last_aligned_seg_to_anchor || anchor_from < last_aligned_seg_to_anchor) {
                                seg_iter_dir = -1;
                            } else {
                                seg_iter_dir = 1;
                            }
                        }
                    }

                    if (seg_iter_dir == -1) {
                        TSeqPos t_pos = m_AlnMgr->GetSeqPosFromAlnPos(anchor,
                            seg.GetAlnRange().GetTo(), IAlnExplorer::eRight);
                        if ((int)t_pos == last_aligned_seg_from_anchor - 1) {
                            start = t_pos / ctx.base_width_anchor;
                            if (ctx.reverse_strand) {
                                ins_len = seg.GetRange().GetFrom() - last_aligned_seg_to_curr;
                            } else {
                                ins_len = last_aligned_seg_from_curr - seg.GetRange().GetTo();
                            }
                            ins_len -= 1;
                        }
                        last_aligned_seg_to_anchor = t_pos;
                        last_aligned_seg_from_anchor = m_AlnMgr->GetSeqPosFromAlnPos(anchor,
                            seg.GetAlnRange().GetFrom(), IAlnExplorer::eRight);
                    } else if (seg_iter_dir == 1) {
                        TSeqPos f_pos = m_AlnMgr->GetSeqPosFromAlnPos(anchor,
                            seg.GetAlnRange().GetFrom(), IAlnExplorer::eRight);
                        if ((int)f_pos == last_aligned_seg_to_anchor + 1) {
                            start = f_pos / ctx.base_width_anchor;
                            if (ctx.reverse_strand) {
                                ins_len = last_aligned_seg_from_curr - seg.GetRange().GetTo();
                            } else {
                                ins_len = seg.GetRange().GetFrom() - last_aligned_seg_to_curr;
                            }
                            ins_len -= 1;
                        }
                        last_aligned_seg_from_anchor = f_pos;
                        last_aligned_seg_to_anchor = m_AlnMgr->GetSeqPosFromAlnPos(anchor,
                            seg.GetAlnRange().GetTo(), IAlnExplorer::eRight);
                    }
                    last_aligned_seg_from_curr = seg.GetRange().GetFrom();
                    last_aligned_seg_to_curr = seg.GetRange().GetTo();
                }
            } else {
                // The left/right/backward/forward flag works very strangely.
                // We need to hack it by looking at the strand flag.
                start = m_AlnMgr->GetSeqPosFromAlnPos(anchor,
                    seg.GetAlnRange().GetFrom(),
                    m_AlnMgr->IsNegativeStrand(anchor) ? IAlnExplorer::eBackwards : IAlnExplorer::eForward);

                start /= ctx.base_width_anchor;
                ins_len = seg.GetRange().GetLength();
                // reset to avoid drawing insertion twice
                last_aligned_seg_to_anchor = -2;
                last_aligned_seg_from_anchor = -2;
            }
            if (ins_len <= 0) continue;

            TModelUnit ins_len_pix = m_Context->SeqToScreen(ins_len);
            // adjust it to the anchor sequence base length
            ins_len_pix /= ctx.base_width_anchor;

            if (ins_len < 5) {
                if (((int)start - last_large_ins) < m_Context->ScreenToSeq(1.0)) {
                    // skip the minor ones that overlap with the last major one
                    continue;
                }
            } else if (ins_len_pix > 2.0) {
                // store the last major insert ( > 5 bases) location
                last_large_ins = (int)start;
            }
            if (ins_len_pix < 0.1f)
                continue;

            if (ins_len_pix > 1.5  &&  m_Config->m_CompactMode == CAlignmentConfig::eNormal) {
                TModelUnit scale = min(10.0 * ins_len/ m_Context->GetScale(), 1.0);
                CRgbaColor color_ins(CRgbaColor::Interpolate(color_max, color_min, scale));
                gl.ColorC(color_ins);
                m_Context->DrawLine(start, ctx.yy, start, ctx.yy + ctx.bar_h);
                TModelUnit size_in_model
                    = min(m_Context->ScreenToSeq(4), (TModelUnit)ins_len);
                m_Context->DrawTriangle(start, y1,
                    start + size_in_model, ctx.yy - over,
                    start - size_in_model, ctx.yy - over);
                m_Context->DrawTriangle(start, y2,
                    start - size_in_model, ctx.yy + ctx.bar_h + over,
                    start + size_in_model, ctx.yy + ctx.bar_h + over);
                TModelUnit len = max(CompressLength(ins_len), size_in_model * 2.0);
                TModelUnit x1 = start - len * 0.5;
                TModelUnit x2 = x1 + len;
                m_Context->DrawLine(x1, ctx.yy - over, x2, ctx.yy - over);
                m_Context->DrawLine(x1, ctx.yy + ctx.bar_h + over, x2, ctx.yy + ctx.bar_h + over);
            }
        }
    }
}

void CAlignGlyph::x_DrawIntronsAndInternalUnalignedRegions(const SDrawContext& ctx, TSignedSeqPos from, TSignedSeqPos to) const
{
    
    TMUnitRanges unaligned_regions;
    x_DrawInternalUnalignedRegions(ctx, unaligned_regions);
    sort(unaligned_regions.begin(), unaligned_regions.end(),
        [](TModelRange const& a, TModelRange const& b) -> bool
    {
        return a.GetFrom() < b.GetFrom();
    });
    TModelUnit y = ctx.yy + ctx.half;
    double barh = abs(m_Context->GetGlPane()->GetScaleY() * ctx.bar_h);
    // if bar height i sless than 2pix
    // set y to top so that the following alignment bar covers it
    // (blue band issue)
    if (barh < 2.)
        y = ctx.yy;
    if (unaligned_regions.empty()) {
        m_Context->DrawLine_NoAA(from, y, to, y);
    } else {
        TModelUnit ln_start = from;
        for (const auto& unaligned_rng : unaligned_regions) {
            m_Context->DrawLine_NoAA(ln_start, y, unaligned_rng.GetFrom() - 1, y);
            ln_start = unaligned_rng.GetToOpen();
        }
        m_Context->DrawLine_NoAA(ln_start, y, to, y);
    }
}

void CAlignGlyph::x_DrawInternalUnalignedRegions(const SDrawContext& ctx, TMUnitRanges &unaligned_regions) const
{
    if (ctx.seq_range.Empty())
        return;

    m_UnalignedRegions = false;

    TModelUnit seg_width(m_Context->GetGlPane()->GetViewport().Width());
    if (m_Context->GetViewWidth() && (m_Context->GetViewWidth() != seg_width))
        seg_width /= 5.0;
    seg_width = m_Context->ScreenToSeq(seg_width);

    IRender& gl = GetGl();
    // Don't draw the labels, if they are taller than the bar
    const CGlTextureFont& l_font = m_Config->m_LabelFont;
    TModelUnit h_text = gl.TextHeight(&l_font);

    int anchor = m_AlnMgr->GetAnchor();
    int row_num = m_AlnMgr->GetNumRows();

    gl.ColorC(CRgbaColor(m_Config->m_Label, m_FadeFactor));

    for (int row = 0; row < row_num; ++row) {
        if (row == anchor) {
            continue;  // skip master seq
        }

        TSignedSeqRange aln_range = ctx.aln_range;
        if (aln_range.GetFrom() > 0) {
            aln_range.SetFrom(aln_range.GetFrom() - 1);
        }

        auto_ptr<IAlnSegmentIterator> p_it(
            m_AlnMgr->CreateSegmentIterator(row,
            aln_range, IAlnSegmentIterator::eAllSegments));

        for (IAlnSegmentIterator& it = *p_it; it; ++it) {
            const IAlnSegment& seg = *it;

            if (!(seg.GetType() & IAlnSegment::fUnaligned)) {
                continue;
            }

            const IAlnSegment::TSignedRange& curr_aln_r = seg.GetAlnRange();
            TSignedSeqPos curr_seq_from =
                m_AlnMgr->GetSeqPosFromAlnPos(anchor, curr_aln_r.GetFrom()) /
                ctx.base_width_anchor;
            TSignedSeqPos curr_seq_to =
                m_AlnMgr->GetSeqPosFromAlnPos(anchor, curr_aln_r.GetTo()) /
                ctx.base_width_anchor;
            if (curr_seq_from > curr_seq_to) {
                swap(curr_seq_from, curr_seq_to);
            }

            TSeqRange curr_seq(curr_seq_from, curr_seq_to - 1);
            if (curr_seq.GetLength() <= 1)
                continue;

            TModelRange inrc = m_Context->IntersectVisible(curr_seq);
            TSeqRange visible_range((TSeqPos)inrc.GetFrom(), (TSeqPos)inrc.GetTo());
            if (visible_range.Empty())
                continue;

            unaligned_regions.push_back(TModelRange(visible_range.GetFrom() + 1, visible_range.GetTo() + 1));
            m_UnalignedRegions = true;

            TModelUnit y = ctx.yy + ctx.half;

            if (h_text > ctx.bar_h) {
                m_Context->DrawStippledLine(visible_range.GetFrom() + 1, y, visible_range.GetTo() + 1, y);
                continue;
            }

            // label text that will be shown
            string sLabelTextOut = NStr::IntToString(seg.GetRange().GetLength(), NStr::fWithCommas);
            sLabelTextOut += " bp unaligned";

            TModelUnit label_width = m_Context->ScreenToSeq(gl.TextWidth(&l_font, sLabelTextOut.c_str()));
            TModelUnit half_label = label_width / 2.0;

            int labelsCount = round(visible_range.GetLength() / seg_width);
            if (!labelsCount) {
                if ((label_width + label_width*0.5) >= visible_range.GetLength()) {
                    m_Context->DrawStippledLine(visible_range.GetFrom() + 1, y, visible_range.GetTo() + 1, y);
                    continue;
                }
                labelsCount = 1;
            }
                                    
            TModelUnit yM = ctx.yy + ctx.bar_h - (ctx.bar_h - h_text) / 2;
            

            TModelUnit part_size = visible_range.GetLength() / labelsCount;
            TModelUnit xM;
            TModelUnit xLF, xLT;
            xLF = visible_range.GetFrom();
            for (int i = 0; i < labelsCount; ++i) {
                xM = visible_range.GetFrom() + part_size / 2.0 + i*part_size;
                xLT = visible_range.GetFrom() + part_size / 2.0 + i*part_size - half_label;
                m_Context->DrawStippledLine(xLF, y, xLT, y);
                m_Context->TextOut(&l_font, sLabelTextOut.c_str(), xM, yM, true, true);
                xLF = visible_range.GetFrom() + part_size / 2.0 + i*part_size + half_label;
            }
            m_Context->DrawStippledLine(xLF, y, visible_range.GetToOpen(), y);
        }
    }
}



void CAlignGlyph::x_DrawTailGlyphs(const SDrawContext& ctx, bool show_label, const CRgbaColor& c_fg) const
{
    IRender& gl = GetGl();
    if (ctx.bar_h <= 2)
        show_label = false;

    int aligned_seq = m_AlnMgr->GetAnchor() == 0 ? 1 : 0;
    TModelUnit size = m_Context->ScreenToSeq(show_label ? kTailGlyphLen : kCompactTailGlyphLen);
    TModelUnit gap = m_Context->ScreenToSeq(show_label ? kLabelSpace_h : kCompactLabelSpace_h);
    TModelUnit y_base = ctx.yy;

    TSignedSeqPos tail_5(0);
    TSignedSeqPos tail_3(0);
    TSignedSeqPos from(ctx.f);
    TSignedSeqPos to(ctx.t);
    m_AlnMgr->GetUnalignedTails(tail_5, tail_3, false);

    if (CAlignmentConfig::eTails_ShowSequence == m_Config->m_UnalignedTailsMode) {
        from = x_GetAlnTailRange(e5_Prime, ctx).GetFrom();
        to = x_GetAlnTailRange(e3_Prime, ctx).GetTo();
        if (tail_5 > ctx.tail_5)
            tail_5 -= ctx.tail_5;
        else
            tail_5 = 0;
        if (tail_3 > ctx.tail_3)
            tail_3 -= ctx.tail_3;
        else
            tail_3 = 0;
    }

    // starting tail
    if (ctx.tail_5 > 0) {
        TModelUnit x_base = from - gap;
        gl.ColorC(c_fg);
        TSeqRange tail_5_rng = x_ConvertAlnTaillRngToSeqRng(e5_Prime, ctx, x_GetAlnTailRange(e5_Prime, ctx));
        bool is_polya = CGuiObjectInfoSeq_align::IsPolyA(m_AlnMgr->GetBioseqHandle(aligned_seq), tail_5_rng.GetFrom(), tail_5_rng.GetToOpen());
        m_Context->DrawUnalignedTail(x_base - size, x_base, y_base,
            y_base + ctx.bar_h, is_polya, CRgbaColor(m_Config->m_TailColor, m_FadeFactor));
        if (show_label) {
            const CGlTextureFont& l_font = m_Config->m_LabelFont;
            string len_str = NStr::IntToString(tail_5, NStr::fWithCommas);
            TModelUnit num_len = gl.TextWidth(&l_font, len_str.c_str());
            
            TModelUnit xM = x_base - size - gap - m_Context->ScreenToSeq(num_len);
            TModelUnit yM = y_base + ctx.bar_h - 2.0;

            TModelUnit h_text = gl.TextHeight(&l_font);
            
            if (h_text < ctx.bar_h) {
                gl.ColorC(CRgbaColor(m_Config->m_Label, m_FadeFactor));
                m_Context->TextOut(&l_font, len_str.c_str(), xM, yM, false);
            }
        }
    }

    // ending tail
    if (tail_3 > 0) {
        TModelUnit x_base = to + gap + 1;
        gl.ColorC(c_fg);
        TSeqRange tail_3_rng = x_ConvertAlnTaillRngToSeqRng(e3_Prime, ctx, x_GetAlnTailRange(e3_Prime, ctx));
        bool is_polya = CGuiObjectInfoSeq_align::IsPolyA(m_AlnMgr->GetBioseqHandle(aligned_seq), tail_3_rng.GetFrom(), tail_3_rng.GetToOpen());
        m_Context->DrawUnalignedTail(x_base, x_base + size, y_base,
            y_base + ctx.bar_h, is_polya, CRgbaColor(m_Config->m_TailColor, m_FadeFactor));
        if (show_label) {
            const CGlTextureFont& l_font = m_Config->m_LabelFont;
            string len_str = NStr::IntToString(tail_3, NStr::fWithCommas);

            TModelUnit xM = x_base + size + gap;
            TModelUnit yM = y_base + ctx.bar_h - 2.0;;
            
            TModelUnit h_text = gl.TextHeight(&l_font);

            if (h_text < ctx.bar_h) {
                gl.ColorC(CRgbaColor(m_Config->m_Label, m_FadeFactor));
                m_Context->TextOut(&l_font, len_str.c_str(), xM, yM, false);
            }
        }
    }
}

void CAlignGlyph::x_DrawTailSequences(const SDrawContext& ctx, const CRgbaColor& c_fg) const
{
    if (ctx.tail_5 > 0) {
        x_DrawUnalignedBar(e5_Prime, ctx, CRgbaColor(m_Config->m_UnalignedFG, m_FadeFactor));
    }
    if (ctx.tail_3 > 0) {
        x_DrawUnalignedBar(e3_Prime, ctx, CRgbaColor(m_Config->m_UnalignedFG, m_FadeFactor));
    }
    TSignedSeqPos tail_5 = 0;
    TSignedSeqPos tail_3 = 0;
    m_AlnMgr->GetUnalignedTails(tail_5, tail_3, false);
    if ((tail_5 > ctx.tail_5) || (tail_3 > ctx.tail_3))
        x_DrawTailGlyphs(ctx, true, c_fg);
}

void CAlignGlyph::x_DrawUnalignedBar(const ETail tail, const SDrawContext& ctx, const CRgbaColor& c_fg) const
{
    IRender& gl = GetGl();

    TSeqRange range(x_GetAlnTailRange(tail, ctx));

    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    gl.PushMatrix();
    gl.Scalef(m_Context->GetScale(), 1.0f, 1.0f);

    TModelRange inrc = m_Context->IntersectVisible(range);
    TSeqRange visible_range((TSeqPos)inrc.GetFrom(), (TSeqPos)inrc.GetTo());
    if (!visible_range.Empty()) {
        gl.Color4d(c_fg.GetRed(), c_fg.GetGreen(), c_fg.GetBlue(), m_FadeFactor);
        TVPUnit start = m_Context->SeqToScreenX(visible_range.GetFrom());
        TVPUnit stop = m_Context->SeqToScreenX(visible_range.GetToOpen());
        gl.Rectd(start, ctx.yy, stop, ctx.yy + ctx.bar_h);
    }

    gl.PopMatrix();
}

void CAlignGlyph::x_DrawUnalignedRowSequence(const ETail tail, const SDrawContext& ctx, const CRgbaColor& c_fg) const
{
    IRender& gl = GetGl();

    int anchor = m_AlnMgr->GetAnchor();
    int aligned_seq = anchor == 0 ? 1 : 0;

    TSeqRange range(x_GetAlnTailRange(tail, ctx));

    TSeqRange visible_unaligned_range(range);
    TModelRange inrc = m_Context->IntersectVisible(visible_unaligned_range);

    if (inrc.Empty()) {
        return;
    }

    visible_unaligned_range.SetFrom((TSeqPos)inrc.GetFrom());
    visible_unaligned_range.SetTo((TSeqPos)inrc.GetTo());

    TSeqRange visible_sequence_range(x_ConvertAlnTaillRngToSeqRng(tail, ctx, visible_unaligned_range));
    
    gl.ColorC(c_fg);

    const CBioseq_Handle& mhndl = m_AlnMgr->GetBioseqHandle(anchor);
    const CBioseq_Handle& hndl = m_AlnMgr->GetBioseqHandle(aligned_seq);

    bool ab = mhndl.IsAa();
    bool rb = hndl.IsAa();

    EAlignSpacing spacing(eOne2One);
    if (!ab && rb) spacing = eOne2Three;
    if (ab && !rb) spacing = eThree2One;
    
        try {

            CSeqVector s_vec = hndl.GetSeqVector(CBioseq_Handle::eCoding_Iupac);

            string seq, tmp_seq;
            s_vec.GetSeqData(visible_sequence_range.GetFrom(), visible_sequence_range.GetTo(), tmp_seq);

            if (ctx.reverse_strand) {
                if (rb) {
                    // protein sequence, just reverse it
                    CSeqManip::Reverse(tmp_seq, CSeqUtil::e_Iupacaa, 0, tmp_seq.length(), seq);
                }
                else {
                    // nucleotide sequence, get reverse and complement
                    if (m_Context->IsFlippedStrand()) {
                        CSeqManip::Reverse(tmp_seq, CSeqUtil::e_Iupacna, 0, tmp_seq.length(), seq);
                    }
                    else {
                        CSeqManip::ReverseComplement(tmp_seq, CSeqUtil::e_Iupacna, 0, tmp_seq.length(), seq);
                    }
                }
            }
            else {
                if (!rb && m_Context->IsFlippedStrand()) {
                    CSeqManip::Complement(tmp_seq, CSeqUtil::e_Iupacna, 0, tmp_seq.length(), seq);
                }
                else {
                    swap(seq, tmp_seq);
                }
            }

            string translated_seq;
            switch (spacing) {
                case eOne2Three:
                    translated_seq = seq;
                    break;
                case eThree2One: {
                    CGenetic_code gcode;
                    gcode.SetId(m_AlnMgr->GetGenCode(aligned_seq));
                    CSeqTranslator::Translate(seq, translated_seq, CSeqTranslator::fDefault, &gcode);
                    break;
                }
                default:
                    break;
            }

            char bases[2];
            bases[1] = '\0';
            TVPUnit font_w = (TVPUnit)(gl.TextWidth(&(m_Config->m_SeqFont), "G") * 0.5f);
            TModelUnit addon = 0.5 - m_Context->ScreenToSeq(font_w);

            TSeqPos sStart = visible_unaligned_range.GetFrom();

            TModelUnit fs = gl.TextHeight(&m_Config->m_SeqFont);
            // Draw characters (actg)
            for (TSeqPos bp = 0; bp < seq.size(); bp++) {
                TModelUnit pos = 0;
                switch (spacing) {
                case eOne2One:
                    pos = sStart + bp + addon;
                    break;
                case eOne2Three:
                    pos = sStart + bp * 3 + addon + 1;
                    break;
                case eThree2One:
                    pos = sStart + bp * kPWAlignPackLevel + addon - kPWAlignPackLevel;
                    break;
                default:
                    break;
                }
                               

                bases[0] = seq[bp];

                m_Context->TextOut(&m_Config->m_SeqFont, bases, pos, ctx.center_y + fs * 0.5, false);
            } // for TSeqPos
        }
        catch (CException&) {
            /// ignore exceptions - these are all objmgr related
        }
}

void CAlignGlyph::x_DrawNonConsensusSpliceSites(const SDrawContext& ctx) const
{
    if (ctx.seq_range.Empty())
        return;

    if (!(m_SeqAlign->GetSegs().Which() == CSeq_align::TSegs::e_Spliced && m_OrigAnchor == 1))
        return;
    IRender& gl = GetGl();
    const CSpliced_seg& spliced_seg = m_SeqAlign->GetSegs().GetSpliced();
    string splice5;
    TSeqPos pre_stop = 0;
    bool neg_genomic = (spliced_seg.GetSeqStrand(1) == eNa_strand_minus);
    ITERATE (CSpliced_seg::TExons, exon_it, spliced_seg.GetExons()) {
        const CSpliced_exon& exon = **exon_it;
        string splice3;
        if (exon.IsSetAcceptor_before_exon()) {
            splice3 = exon.GetAcceptor_before_exon().GetBases();
        }

        // render a bold red bar for non-consensus splice site
        if ( !splice5.empty()  &&  !splice3.empty()  &&
            !IsConsensusSplice(splice5, splice3) ) {
            gl.LineWidth(4.0f);
            gl.ColorC(CRgbaColor(m_Config->m_NonConsensus, m_FadeFactor));
            m_Context->DrawLine(pre_stop, ctx.center_y,
                neg_genomic ? exon.GetGenomic_end() + 1 :
                exon.GetGenomic_start(), ctx.center_y);
            gl.LineWidth(1.0f);
        }
        if (exon.IsSetDonor_after_exon()) {
            splice5 = exon.GetDonor_after_exon().GetBases();
        } else {
            splice5 = kEmptyStr;
        }
        pre_stop = neg_genomic ?
            exon.GetGenomic_start() : exon.GetGenomic_end() + 1;
    }
}

void CAlignGlyph::x_DrawNoScoreAlign(const SDrawContext& ctx, const CRgbaColor& c_fg) const
{
    if (ctx.seq_range.Empty())
        return;
    CRgbaColor c;
    if (ctx.override_color)
        c = ctx.read_status_color;
    else
        c = c_fg;

    // Don't use blending here - just overwrite but take any alpha into account
    if (m_FadeFactor < 1.0) {
        c.Lighten(1.0f - m_FadeFactor);
    }

    // m_Intervals - are calculated in UpdateBoundingBox
    ITERATE(vector<TSeqRange>, it, m_Intervals) {
        if (ctx.override_color)
            m_Context->Draw3DQuad(it->GetFrom(), ctx.yy, it->GetTo(), ctx.yy + ctx.bar_h, c, true);
        else
            m_Context->Draw3DQuad(it->GetFrom(), ctx.yy, it->GetTo(), ctx.yy + ctx.bar_h, c, true);
    }
}

void CAlignGlyph::x_DrawStrandIndicators(const SDrawContext& ctx) const
{
    IRender& gl = GetGl();
    // draw strand indicators
    if (m_Config->m_CompactMode != CAlignmentConfig::eExtremeCompact  &&
        m_Context->SeqToScreen(ctx.whole_range.GetLength()) > 8.0) {
        TModelUnit xM = (ctx.whole_range.GetFrom() + ctx.whole_range.GetTo()) * 0.5;
        if (m_Context->WillSeqLetterFit()) {
            // make sure we don't step on the sequence letter
            // when sequence is shown
            xM = ceil(xM);
        }

        TModelUnit yM = ctx.yy + ctx.half;
        gl.Color4f(0.0f, 0.0f, 0.0f, m_FadeFactor);  // Black
        m_Context->DrawGreaterLessSign(xM, yM, ctx.bar_h, ctx.reverse_strand);
    }
}

void CAlignGlyph::x_DrawSimplified(const SDrawContext& ctx, const CRgbaColor& c_fg) const
{
    IRender& gl = GetGl();
    if (m_ShowScore  &&  m_AlnMgr->IsRegular()) {
        double identity = 100.0;
            if ( !m_SeqAlign->GetNamedScore(CSeq_align::eScore_PercentIdentity, identity) ) {
            TSeqPos align_length = m_SeqAlign->GetAlignLength();
            int identities = (int)align_length;
            if ( !m_SeqAlign->GetNamedScore(CSeq_align::eScore_IdentityCount, identities) ) {
                //    CScoreBuilder builder;
                //    identities = builder.GetIdentityCount(*scope, align);
            }
            identity = identities * 100 / align_length;
        }
        identity *= 0.01;
        if (identity > 0.6) {
            identity = 1.75 * identity - 0.75;
        } else {
            identity /= 2.0;
        }
        CRgbaColor color_max(0.78f, 0.78f, 0.78f, m_FadeFactor);
        CRgbaColor color_min(1.0f, 0.0f, 0.0f, m_FadeFactor);
        CRgbaColor color(CRgbaColor::Interpolate(color_max, color_min, identity));
        if (ctx.override_color)
            color = ctx.read_status_color;

        gl.ColorC(color);
        m_Context->DrawQuad(ctx.seq_range.GetFrom(), ctx.yy, ctx.seq_range.GetTo(), ctx.yy + ctx.bar_h, false);
    } else {
        m_Context->Draw3DQuad(ctx.seq_range.GetFrom(), ctx.yy, ctx.seq_range.GetTo(), ctx.yy + ctx.bar_h, c_fg, true);
    }
    x_DrawStrandIndicators(ctx);
}


void CAlignGlyph::DrawPWAlignElem(const TModelRect& rcm,
                                  bool selected,
                                  bool show_label,
                                  const CRgbaColor& c_fg,
                                  const CRgbaColor& c_seq,
                                  const CRgbaColor& c_mis) const
{
    IRender& gl = GetGl();

    int anchor = m_AlnMgr->GetAnchor();
    int aligned_seq = m_AlnMgr->GetQuery();
    if (aligned_seq < 0) {
        aligned_seq = 0;
    }
    SDrawContext ctx;
    ctx.base_width_anchor = m_AlnMgr->GetBaseWidth(anchor);
    ctx.base_width_curr = m_AlnMgr->GetBaseWidth(aligned_seq);
    ctx.f = m_AlnMgr->GetSeqStart(anchor);
    ctx.f /= ctx.base_width_anchor;
    ctx.t = m_AlnMgr->GetSeqStop(anchor);
    ctx.t /= ctx.base_width_anchor;
    ctx.reverse_strand = m_AlnMgr->IsNegativeStrand(aligned_seq) != m_AlnMgr->IsNegativeStrand(anchor);

    ctx.seq_range.Set(ctx.f, ctx.t);
    if (CAlignmentConfig::eTails_ShowSequence == m_Config->m_UnalignedTailsMode)
        ctx.whole_range = m_AlnMgr->GetAlnRangeWithTails();
    else
        ctx.whole_range = ctx.seq_range;
    
    if (ctx.seq_range.Empty()) {
        LOG_POST(Error 
            << "CAlignGlyph::DrawPWAlignElem: zero alignment range!");
        return;
    }
    {
        TModelRange inrc = m_Context->IntersectVisible(ctx.whole_range);
        if (inrc.Empty())
            return;
        ctx.whole_range.SetOpen((TSeqPos)inrc.GetFrom(), (TSeqPos)inrc.GetToOpen());
        inrc = m_Context->IntersectVisible(ctx.seq_range);
        ctx.seq_range.SetOpen((TSeqPos)inrc.GetFrom(), (TSeqPos)inrc.GetToOpen());
    }

    // Special colors for sra reads with errors: failed reads red,
    // pcr duplicates pale gray.  SV-2153
    ctx.override_color = false;
    x_GetOverrideColor(ctx.override_color, ctx.read_status_color);

    ctx.bar_h = GetBarHeight();
    ctx.half = ctx.bar_h * 0.5;
    ctx.center_y = rcm.Bottom() - ctx.half - 1.0;
    ctx.yy = ctx.center_y - ctx.half;
    
    if (IsSimplified()) {
        x_DrawSimplified(ctx, c_fg);
        // Draw selection
        if (selected)
            m_Context->DrawSelection(rcm);
        return;
    }

    TSignedSeqPos aln_r_s = m_AlnMgr->GetAlnPosFromSeqPos(
        anchor, ctx.seq_range.GetFrom() * ctx.base_width_anchor, IAlnExplorer::eRight);
    TSignedSeqPos aln_r_e = m_AlnMgr->GetAlnPosFromSeqPos(
        anchor, ctx.seq_range.GetTo() * ctx.base_width_anchor, IAlnExplorer::eLeft);
    if (aln_r_s > aln_r_e) {
        swap(aln_r_s, aln_r_e);
    }

    ctx.aln_range = TSignedSeqRange(aln_r_s, aln_r_e);
    bool show_score = x_ShowScore();
    gl.ColorC(c_fg);

    TModelUnit align_len = m_Context->SeqToScreen(ctx.whole_range.GetLength());
    bool show_unaligned_tails = x_ShowUnalignedTails(align_len);

    // get tail sizes
    if (show_unaligned_tails) {
        m_AlnMgr->GetUnalignedTails(ctx.tail_5, ctx.tail_3);
    }
    double barh = abs(m_Context->GetGlPane()->GetScaleY() * ctx.bar_h);
    if ((m_Context->GetScale() < 1  &&  !show_score)  ||
        (show_score  &&  m_Context->WillSeqLetterFit() && ctx.tail_3)) {
        TModelUnit over7 = m_Context->ScreenToSeq(7);
        TModelUnit over2 = m_Context->ScreenToSeq(2);
        TModelUnit y = ctx.yy + ctx.half;
        if (ctx.reverse_strand) {
            x_DrawIntronsAndInternalUnalignedRegions(ctx, ctx.f - over2, ctx.t + 1);
        }
        else {
            x_DrawIntronsAndInternalUnalignedRegions(ctx, ctx.f, ctx.t + over2 + 1);
        }

        // ignore strand signs if bar is less that 2 pix
        if (barh >= 2.) {
           
            if (show_score) {
                // make the color for the strand indicator matches well with
                // the alignment bar color
                gl.Color4f(0.5f, 0.5f, 0.5f, m_FadeFactor);
            }
            if (ctx.reverse_strand && !(show_unaligned_tails && ctx.tail_5)) {
                m_Context->DrawTriangle(ctx.f - over7, y,
                                        ctx.f - over2, ctx.yy - 1, ctx.f - over2, ctx.yy + ctx.bar_h + 1);
            }
            else if (!(show_unaligned_tails && ctx.tail_3)) {
                m_Context->DrawTriangle(ctx.t + 1 + over7, y,
                                        ctx.t + 1 + over2, ctx.yy - 1, ctx.t + 1 + over2, ctx.yy + ctx.bar_h + 1);
            }
        }
    } else {
        x_DrawIntronsAndInternalUnalignedRegions(ctx, ctx.f, ctx.t);
    }  // arrows

    // Draw Sequence and quads for segments   
    if (show_score) {
        // Disable blending for scores since they overwrite the alignment bar
        if (m_FadeFactor != 1.0f)
            gl.Disable(GL_BLEND);

        // do score coloration
        int row_num = m_AlnMgr->GetNumRows();
        for (int row = 0;  row < row_num;  ++row) {
            if (row == anchor) {
                continue;  // skip master seq
            }
            if (m_Score->IsAverageable()) {
                x_RenderAveragableScores(ctx.yy, ctx.aln_range, ctx.seq_range, ctx.bar_h, row,
                    ctx.override_color, ctx.read_status_color);
            } else {
                x_RenderNonAveragableScores(ctx.yy, ctx.aln_range, ctx.seq_range, ctx.bar_h, row,
                    ctx.override_color, ctx.read_status_color);
            } 
        }

        if (m_FadeFactor != 1.0f)
            gl.Enable(GL_BLEND);       

        //        if (m_Config->m_CompactMode == CAlignmentConfig::eNormal)
        x_DrawGaps(ctx);
        x_DrawInserts(ctx);
        
        // draw unaligned tails
        if (show_unaligned_tails) {
            switch (m_Config->m_UnalignedTailsMode)
            {
            case CAlignmentConfig::eTails_Hide:
                break;
            case CAlignmentConfig::eTails_ShowGlyph:
                x_DrawTailGlyphs(ctx, true, c_fg);
                break;
            case CAlignmentConfig::eTails_ShowSequence:
                x_DrawTailSequences(ctx, c_fg);
                break;
            }           
        }
        
    } else {
        // Disable blending since we want overwrite here for the fading effect,
        // (if any) not blending with other elements
        if (m_FadeFactor != 1.0f)
            gl.Disable(GL_BLEND);

        x_DrawNoScoreAlign(ctx, c_fg);

        if (m_FadeFactor != 1.0f)
            gl.Enable(GL_BLEND);
    }


    // special case
    // reflect non-consensus splice sites for spliced-seg alignment
    // we only show non-consensus site when showing in genomic sequence
    // context, not product sequence context.
    x_DrawNonConsensusSpliceSites(ctx);

    if ((m_Context->WillSeqLetterFit()  &&  ctx.base_width_anchor <= ctx.base_width_curr)
        || (m_Context->GetScale() <= 0.05  &&  ctx.base_width_anchor > ctx.base_width_curr)) {
        try {
            x_DrawAlignRowSequence(ctx.center_y, 1 - anchor, c_fg, c_seq, c_mis);
        } catch (const CException& e) {
            LOG_POST(Warning << "Can't show alignment sequence due to an error: " << e.GetMsg());
        }
        if (CAlignmentConfig::eTails_ShowSequence == m_Config->m_UnalignedTailsMode) {
            CRgbaColor c_letters(m_Config->m_UnalignedSequence, m_FadeFactor);
            if (ctx.tail_5 > 0) {
                try {
                    x_DrawUnalignedRowSequence(e5_Prime, ctx, c_letters);
                }
                catch (const CException& e) {
                    LOG_POST(Warning << "Can't show 5' tail sequence due to an error: " << e.GetMsg());
                }
            }
            if (ctx.tail_3 > 0) {
                try {
                    x_DrawUnalignedRowSequence(e3_Prime, ctx, c_letters);
                }
                catch (const CException& e) {
                    LOG_POST(Warning << "Can't show 3' tail sequence due to an error: " << e.GetMsg());
                }
            }
        }
    }

    x_DrawStrandIndicators(ctx);

    // Draw selection
    if (selected) {
        m_Context->DrawSelection(rcm);
    }
}


void CAlignGlyph::GetTitle(string* title, CLabel::ELabelType type) const
{
    title->erase();

    if ( !x_IsPWAlign()) {
        *title = "[Alignment] " +
            NStr::UIntToString(m_AlnMgr->GetNumRows()) + " sequences";
        return;
    }

    // pairwise align
    switch (type) {
    case CLabel::eType:
    case CLabel::eContent:
    case CLabel::eUserTypeAndContent:
        {
            // display the name of the aligned sequence.
            int row = 1 - m_OrigAnchor;
            CLabel::GetLabel(m_OrigSeqAlign.GetSeq_align()->GetSeq_id(row),
                title, type, &m_Context->GetScope());

            // make title/label shorter incase it is a cSRA alignment on panfs
            if (title->length() > 20  &&  title->find("\\panfs\\") != string::npos) {
                size_t pos = title->find_last_of("\\");
                if (pos != string::npos) {
                    *title = title->substr(pos + 1);
                    if (title->length() > 20) {
                        pos = title->rfind('.');
                        if (pos != string::npos  &&  pos > 0) {
                            pos = title->rfind('.', pos - 1);
                            if (pos != string::npos) {
                                *title = title->substr(pos + 1);
                            }
                        }
                    }
                }
            }
        };
        break;
    case CLabel::eDescriptionBrief:
    case CLabel::eDescription:
        CLabel::GetLabel(*GetOrigAlignment().GetSeq_align(),
            title, type, &m_Context->GetScope());
        break;
    default:
        break;
    }
}


void CAlignGlyph::ApplyFading() const
{
    m_FadeFactor = GetFadeFactor();
    if (m_FadeFactor != 1.) {
        IRender& gl = GetGl();
        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void CAlignGlyph::x_Draw() const
{
    if (!GetVisible())
        return;
    auto vis_rect = m_Context->GetGlPane()->GetVisibleRect();
    if (GetTop() > vis_rect.Bottom() && GetBottom() < vis_rect.Top())
        return;

    if (x_IsPWAlign()) {
        ApplyFading();
        x_DrawPWAlign();
    } else {
        // No fade out for multiple align drawing for now. Since all the rows in
        // this case are inside this one glyph and drawn in a loop, we would need
        // to handle things a bit differently.
        m_FadeFactor = 1.f;
        x_DrawMultiAlign();
    }
}


void CAlignGlyph::x_UpdateBoundingBox()
{
    IRender& gl = GetGl();

    if (m_Intervals.empty()  && x_IsPWAlign()) {
        x_CalcIntervals(m_Intervals);
    }

    TSeqRange range = GetRange();
    SetWidth(range.GetLength());
    SetLeft(range.GetFrom());

    TModelUnit bar_h = GetBarHeight();
    if (x_IsPWAlign()) {
        SetHeight(bar_h + (m_Config->m_CompactMode == CAlignmentConfig::eNormal ? 2 : 0));
    } else {
        SetHeight((bar_h + kAlignRowSpace) * (m_AlnMgr->GetNumRows() - 1));
    }

    bool show_label = x_ShowLabel();
    // don't show labels and unaligned tails for simplified rendering
    if (IsSimplified()  &&  !show_label) {
        return;
    }

    bool top_label = m_Config->m_LabelPos == CAlignmentConfig::ePos_Above;
    const CGlTextureFont& font = m_Config->m_LabelFont;
    int anchor = m_AlnMgr->GetAnchor();
    bool reverse_strand = m_AlnMgr->IsNegativeStrand(0) != m_AlnMgr->IsNegativeStrand(1);
    TSignedSeqPos visible_tail_5 = 0;
    TSignedSeqPos visible_tail_3 = 0;
    TSignedSeqPos tail_5 = 0;
    TSignedSeqPos tail_3 = 0;
    TSignedSeqPos base_width_curr = m_AlnMgr->GetBaseWidth(anchor == 0 ? 1 : 0);
    TModelUnit align_len = m_Context->SeqToScreen(GetRange().GetLength());

    // get tail sizes
    if (x_ShowUnalignedTails(align_len)) {
        
        m_AlnMgr->GetUnalignedTails(tail_5, tail_3, false);
        m_AlnMgr->GetUnalignedTails(visible_tail_5, visible_tail_3);
        
        TModelUnit tail_len = 0;
        if (tail_5 > 0) {
            tail_len = x_GetTailWidth(tail_5, visible_tail_5, base_width_curr);
            
            SetWidth(GetWidth() + tail_len);
            SetLeft(GetLeft() - tail_len);
        }
        if (tail_3 > 0) {
            tail_len = x_GetTailWidth(tail_3, visible_tail_3, base_width_curr);

            SetWidth(GetWidth() + tail_len);
        }
    }

    if (show_label) {
        if (top_label) {
            if (x_IsPWAlign()) {
                // vertical space for alignment label
                SetHeight(GetHeight() + gl.TextHeight(&font) + 3);
            } else {
                SetHeight(GetHeight() + (gl.TextHeight(&font) + kLabelSpace_v) *
                    (m_AlnMgr->GetNumRows() - 1));
            }
        } else { // not top label (side label)
            if (x_IsPWAlign()) {
                // horizontal space for alignment label
                string label;
                GetTitle(&label, CLabel::eDefault);
                TModelUnit label_w = gl.TextWidth(&font, label.c_str());
                label_w = m_Context->ScreenToSeq(label_w + kLabelSpace_h);
                SetWidth(GetWidth() + label_w);
                if ( !reverse_strand ) {
                    SetLeft(GetLeft() - label_w);
                }
            } else {
                TModelUnit max_label_w = 0.0;
                for (int row = 0;  row < m_AlnMgr->GetNumRows();  ++row) {
                    if (row == anchor) {
                        continue;  // skip master seq
                    }
                    string label;
                    CLabel::GetLabel(m_AlnMgr->GetSeqId(row), &label,
                        CLabel::eContent, &m_Context->GetScope());
                    if (m_AlnMgr->IsNegativeStrand(anchor) !=
                        m_AlnMgr->IsNegativeStrand(row)) {
                        label += " (Neg)";
                    }
                    TModelUnit label_w = gl.TextWidth(&font, label.c_str());
                    max_label_w = max(max_label_w, label_w);
                }
                max_label_w += kLabelSpace_h;
                max_label_w = m_Context->ScreenToSeq(max_label_w);
                SetWidth(GetWidth() + max_label_w);
                SetLeft(GetLeft() - max_label_w);
            }
        }
    }
}


void CAlignGlyph::x_DrawPWAlign() const
{
    IRender& gl = GetGl();

    TModelRect rcm = GetModelRect();
    bool show_label = x_ShowLabel();

    DrawPWAlignElem(rcm, IsSelected(), show_label, 
        CRgbaColor(m_Config->m_FG, m_FadeFactor),
        CRgbaColor(m_Config->m_Sequence, m_FadeFactor),
        CRgbaColor(m_Config->m_SeqMismatch, m_FadeFactor));

    if (show_label) {
        string label;
        GetTitle(&label, CLabel::eDefault);
        const CGlTextureFont& l_font = m_Config->m_LabelFont;
        bool top_label = m_Config->m_LabelPos == CAlignmentConfig::ePos_Above;
        TModelUnit label_h = gl.TextHeight(&l_font);
        TModelRange inrc = m_Context->IntersectVisible(this);
        TModelUnit xM = inrc.GetFrom() + inrc.GetLength() * 0.5;
        TModelUnit yM = rcm.Top() + label_h + 1.0;
        TModelUnit max_label_width_px = m_Context->SeqToScreen(inrc.GetLength() - 1);
        if (max_label_width_px > 0.0) {
            //if (max_label_width_px > 2.0) max_label_width_px -= 2.0;
            TModelUnit label_width_px = gl.TextWidth(&l_font, label.c_str());
            if (label_width_px > max_label_width_px) {
                label_width_px = max_label_width_px;
                label = l_font.Truncate(label.c_str(), label_width_px);
                if ((string::npos != label.find("...")) && (label.length() <= 5))
                    return;
            }

            if ( !top_label ) {
                TModelUnit t_w = m_Context->ScreenToSeq(label_width_px);
                bool reverse_strand = m_AlnMgr->IsNegativeStrand(0) !=
                    m_AlnMgr->IsNegativeStrand(1);
                yM = GetBottom() - 2.0;
                if (reverse_strand) {
                    xM = inrc.GetTo() - t_w;
                    if (inrc.GetTo() < GetRight()) {
                        gl.ColorC(CRgbaColor(m_Config->m_BG, m_FadeFactor));
                        m_Context->DrawBackground( TModelRect(xM, yM + 1.0,
                            inrc.GetTo(), yM - label_h - 1.0), 0);
                    }
                } else {
                    xM = inrc.GetFrom();
                    if (inrc.GetFrom() > GetLeft()) {
                        gl.ColorC(CRgbaColor(m_Config->m_BG, m_FadeFactor));
                        m_Context->DrawBackground( TModelRect(xM, yM + 1.0,
                            xM + t_w, yM - label_h - 1.0), 0);
                    }
                }
            }

            if (IsSelected()) {
                gl.ColorC(CRgbaColor(m_Context->GetSelLabelColor(), m_FadeFactor));
            } else {
                gl.ColorC(CRgbaColor(m_Config->m_Label, m_FadeFactor));
            }
            m_Context->TextOut(&l_font, label.c_str(), xM, yM, top_label);
        }
    }
}


void CAlignGlyph::x_DrawMultiAlign() const
{
    IRender& gl = GetGl();

    int anchor = m_AlnMgr->GetAnchor();
    TSeqPos base_width_anchor = m_AlnMgr->GetBaseWidth(anchor);
    TSeqRange anchor_range(
        m_AlnMgr->GetSeqStart(anchor) / base_width_anchor,
        m_AlnMgr->GetSeqStop(anchor) / base_width_anchor);
    if (anchor_range.Empty()) {
        LOG_POST(Error
            << "CAlignGlyph::x_DrawMultiAlign: zero alignment range!");
        return;
    }

    TModelRange inrc = m_Context->IntersectVisible(anchor_range);
    anchor_range.Set((TSeqPos)inrc.GetFrom(), (TSeqPos)inrc.GetTo());
    if (anchor_range.Empty()) {
        return;
    }

    // Special colors for sra reads with errors: failed reads red,
    // pcr duplicates pale gray.  SV-2153
    bool override_color = false;
    CRgbaColor read_status_color;
    x_GetOverrideColor(override_color, read_status_color);

    TSignedSeqPos aln_r_s = m_AlnMgr->GetAlnPosFromSeqPos(anchor,
        anchor_range.GetFrom() * base_width_anchor, IAlnExplorer::eRight);
    TSignedSeqPos aln_r_e = m_AlnMgr->GetAlnPosFromSeqPos(anchor,
        anchor_range.GetTo() * base_width_anchor, IAlnExplorer::eLeft);
    if (aln_r_s > aln_r_e) {
        swap(aln_r_s, aln_r_e);
    }
    TSignedSeqRange aln_total_range(aln_r_s, aln_r_e);


    TModelRect rcm = GetModelRect();
    TModelUnit base = rcm.Top();
    TModelUnit bar_h = GetBarHeight();

    bool show_label = x_ShowLabel();
    TModelUnit yy = base + 1;
    for (int row = 0;  row < m_AlnMgr->GetNumRows();  ++row) {
        if (row == anchor) {
            continue;  // skip master seq
        }

        TSignedSeqRange aln_range =
            aln_total_range.IntersectionWith(m_AlnMgr->GetSeqAlnRange(row));
        TModelUnit xM = (inrc.GetFrom() + inrc.GetTo()) * 0.5;
        bool reverse_strand =
            m_AlnMgr->IsNegativeStrand(row) !=
            m_AlnMgr->IsNegativeStrand(anchor);

        // Draw label
        if (show_label  &&
            m_Config->m_LabelPos == CAlignmentConfig::ePos_Above) {
            const CGlTextureFont& l_font = m_Config->m_LabelFont;
            yy += gl.TextHeight(&l_font);
            x_DrawMultiAlignLabel(row, yy);
            yy += kLabelSpace_v;
        }

        TModelUnit mid = yy + bar_h * 0.5;
        if (x_ShowScore()) {
            // do score coloration
            if (m_Score->IsAverageable()) {
                x_RenderAveragableScores(yy, aln_range, anchor_range, bar_h, row,
                                         override_color, read_status_color);
            } else {
                x_RenderNonAveragableScores(yy, aln_range, anchor_range, bar_h, row,
                                            override_color, read_status_color);
            }

            // color the gap differently
            gl.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
            CGlAttrGuard AttrGuard(GL_LINE_BIT);
            gl.Disable(GL_LINE_SMOOTH);

            auto_ptr<IAlnSegmentIterator> p_it(
                m_AlnMgr->CreateSegmentIterator(row, aln_range,
                IAlnSegmentIterator::eAllSegments));

            gl.LineWidth(2.0f);
            for (IAlnSegmentIterator& it = *p_it;  it;  ++it)  {
                const IAlnSegment& seg = *it;
                if ( !(seg.GetType() & IAlnSegment::fIndel)  ||
                    !seg.GetRange().Empty() ) {
                        continue;
                }

                TSeqPos start = m_AlnMgr->GetSeqPosFromAlnPos(anchor, seg.GetAlnRange().GetFrom());
                start /= base_width_anchor;
                TSeqPos stop = start + seg.GetAlnRange().GetLength() / base_width_anchor;
                m_Context->DrawLine(start, mid, stop, mid);
            }
            gl.LineWidth(1.0f);

            // draw inserts
            gl.ColorC(CRgbaColor(m_Config->m_Insertion, m_FadeFactor));
            const TModelUnit barOff =
                m_AlnMgr->IsNegativeStrand(anchor) ? 1.0 : 0.0;
            const TModelUnit kMinLen = m_Context->ScreenToSeq(4); // 4 pixels in model units

            TModelUnit y1 = yy;
            TModelUnit y2 = yy + bar_h;
            p_it.reset(m_AlnMgr->CreateSegmentIterator(
                row, aln_range, IAlnSegmentIterator::eAllSegments));

            for (IAlnSegmentIterator& it = *p_it;  it;  ++it)  {
                const IAlnSegment& seg = *it;

                if ( !(seg.GetType() & IAlnSegment::fIndel)  ||
                    seg.GetRange().Empty() ) {
                        continue;
                }

                TSeqPos start = m_AlnMgr->GetSeqPosFromAlnPos(anchor,
                    seg.GetAlnRange().GetTo(),
                    IAlnExplorer::eRight);
                start /= base_width_anchor;
                TModelUnit len = CompressLength(seg.GetRange().GetLength());
                len = max(len, kMinLen);
                TModelUnit x1 = start + barOff - len * 0.5;
                TModelUnit x2 = x1 + len;

                m_Context->DrawLine_NoAA(start + barOff, y1, start + barOff, y2);
                m_Context->DrawLine_NoAA(x1, y1, x2, y1);
                m_Context->DrawLine_NoAA(x1, y2, x2, y2);
            }
        } else {  // use default scheme
            gl.ColorC(CRgbaColor(m_Config->m_FG, m_FadeFactor));

            // draw connection line in the middle
            m_Context->DrawLine_NoAA(anchor_range.GetFrom(), mid, anchor_range.GetTo(),  mid);
            TIntervals intervals;
            x_CalcIntervals(intervals, row);
            ITERATE(TIntervals, it, intervals) {
                m_Context->Draw3DQuad(it->GetFrom(), yy, it->GetTo(), yy + bar_h, CRgbaColor(m_Config->m_FG, m_FadeFactor), true);
            }
        }

        if (m_Context->WillSeqLetterFit()) {
            try {
                x_DrawAlignRowSequence(mid, row, 
                    CRgbaColor(m_Config->m_FG, m_FadeFactor),
                    CRgbaColor(m_Config->m_Sequence, m_FadeFactor),
                    CRgbaColor(m_Config->m_SeqMismatch, m_FadeFactor));
            } catch (CException& e) {
                LOG_POST(Warning << "Can't show alignment sequence due to an error: "
                    << e.GetMsg());
            }

        } else if (m_Context->WillLabelFit(inrc)) {
            // draw strand indicators
            // do not draw strand when sequence is shown
            gl.Color4f(0.0f, 0.0f, 0.0f, m_FadeFactor);  // Black
            gl.Color4f(0.0f, 0.0f, 0.0f, m_FadeFactor);  // Black
            TModelUnit fs = gl.TextHeight(&(m_Config->m_SeqFont)) - 1;
            TModelUnit yM = yy + fs + 1;
            char strandLabel[2] = {'>', '\0'};
            if (m_Context->IsHorizontal()){
                if (m_Context->IsFlippedStrand()){
                    strandLabel[0] = reverse_strand ? '>' : '<';
                } else {
                    strandLabel[0] = reverse_strand ? '<' : '>';
                }
            } else {
                if (m_Context->IsFlippedStrand()){
                    strandLabel[0] = reverse_strand ? '<' : '>';
                } else {
                    strandLabel[0] = reverse_strand ? '>' : '<';
                }
            }
            m_Context->TextOut(&m_Config->m_SeqFont, strandLabel, xM, yM, true);
        }

        if (show_label  &&
            m_Config->m_LabelPos == CAlignmentConfig::ePos_Side) {
            const CGlTextureFont& l_font = m_Config->m_LabelFont;
            TModelUnit yM = yy + gl.TextHeight(&l_font);
            x_DrawMultiAlignLabel(row, yM);
        }
        // advance to next row: 1 pixel between
        yy += bar_h + kAlignRowSpace;
    }

    // Draw selection
    if (IsSelected()) {
        m_Context->DrawSelection(rcm);
    }
}


void CAlignGlyph::x_DrawMultiAlignLabel(int row, TModelUnit y) const
{
    IRender& gl = GetGl();

    string label;
    CLabel::GetLabel(m_AlnMgr->GetSeqId(row), &label,
        CLabel::eContent, &m_Context->GetScope());
    if (m_AlnMgr->IsNegativeStrand(m_AlnMgr->GetAnchor()) !=
        m_AlnMgr->IsNegativeStrand(row)) {
        label += " (Neg)";
    }
    const CGlTextureFont& l_font = m_Config->m_LabelFont;

    TModelRange inrc = m_Context->IntersectVisible(this);
    TModelUnit max_label_width_px = m_Context->SeqToScreen(inrc.GetLength());
    TModelUnit label_w = gl.TextWidth(&l_font, label.c_str());
    if (label_w > max_label_width_px) {
        label_w = max_label_width_px;
        label = l_font.Truncate(label.c_str(), max_label_width_px);
    }
    bool top_label = m_Config->m_LabelPos == CAlignmentConfig::ePos_Above;
    TModelUnit xM = (inrc.GetFrom() + inrc.GetTo()) * 0.5;
    if ( !top_label ) {
        xM = inrc.GetFrom();
        if (inrc.GetFrom() > GetLeft()) {
            gl.ColorC(CRgbaColor(m_Config->m_BG, m_FadeFactor));
            label_w = m_Context->ScreenToSeq(label_w);
            m_Context->DrawBackground( TModelRect(xM, y + 1.0,
                xM + label_w, y - gl.TextHeight(&l_font) - 1.0), 0);
        }
    }

    if (IsSelected()) {
        gl.ColorC(CRgbaColor(m_Context->GetSelLabelColor(), m_FadeFactor));
    } else {
        gl.ColorC(CRgbaColor(m_Config->m_Label, m_FadeFactor));
    }
    m_Context->TextOut(&l_font, label.c_str(), xM, y, top_label);
}


void CAlignGlyph::x_DrawAlignRowSequence(TModelUnit center_y,
                                         int row,
                                         const CRgbaColor& c_fg,
                                         const CRgbaColor& c_seq,
                                         const CRgbaColor& c_mis) const
{
    IRender& gl = GetGl();
    TModelUnit fs = gl.TextHeight(&m_Config->m_SeqFont);

    int anchor = m_AlnMgr->GetAnchor();
    TSeqPos base_width_anchor = m_AlnMgr->GetBaseWidth(anchor);
    TSeqPos base_width_curr = m_AlnMgr->GetBaseWidth(row);
    TSeqRange anchor_range(
        m_AlnMgr->GetSeqStart(anchor) / base_width_anchor,
        m_AlnMgr->GetSeqStop(anchor) / base_width_anchor);
    TModelRange inrc = m_Context->IntersectVisible(anchor_range);
    anchor_range.Set((TSeqPos)inrc.GetFrom(), (TSeqPos)inrc.GetTo());
    if (anchor_range.Empty()) {
        return;
    }
    const CBioseq_Handle& mhndl = m_AlnMgr->GetBioseqHandle(anchor);
    if (!mhndl)
        return;
    const CBioseq_Handle& hndl = m_AlnMgr->GetBioseqHandle(row);
    if (!hndl)
        return;

    TSignedSeqPos aln_r_s = m_AlnMgr->GetAlnPosFromSeqPos(anchor,
            anchor_range.GetFrom() * base_width_anchor, IAlnExplorer::eRight);
    TSignedSeqPos aln_r_e = m_AlnMgr->GetAlnPosFromSeqPos(anchor,
            anchor_range.GetTo() * base_width_anchor, IAlnExplorer::eLeft);
    if (aln_r_s > aln_r_e) {
        swap(aln_r_s, aln_r_e);
    }
    TSignedSeqRange aln_range(aln_r_s, aln_r_e);

    if (aln_range.GetFrom() > 2) {
        aln_range.SetFrom(aln_range.GetFrom() - 3);
    }

    gl.ColorC(c_fg);

    bool reverse_strand = m_AlnMgr->IsNegativeStrand(row) !=
        m_AlnMgr->IsNegativeStrand(anchor);

    auto_ptr<IAlnSegmentIterator> p_it(
        m_AlnMgr->CreateSegmentIterator(row,
        aln_range, IAlnSegmentIterator::eSkipGaps));

    bool ab = mhndl.IsAa();
    bool rb = hndl.IsAa();
    CSeqVector ms_vec =
        mhndl.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
    CSeqVector s_vec =
        hndl.GetSeqVector(CBioseq_Handle::eCoding_Iupac);

    EAlignSpacing spacing(eOne2One);
    if (!ab && rb) spacing = eOne2Three;
    if (ab && !rb) spacing = eThree2One;

    for ( IAlnSegmentIterator& it(*p_it);  it;  ++it )  {
        const IAlnSegment& seg = *it;

        // We need to deal with some tricky genomic-to-protein alignments
        // described in both GB-2336 and GB-2324. Sometimes, the insertions
        // on the protein sequence row are one-(nucleotide)-base insertions.
        // That means two nucleiotide bases on genomic sewquence correspond
        // to one residue on the protein sequence at that insertion postion.
        // Normally, it one-to-three or three-to-one relation. That introduces
        // all sort of troubles including:
        //    - showing correct sequence
        //    - positioning the sequence correctly,
        //    - retrieving the correct chunk of sequences for both protein
        //      translation and protein-to-genomic comparison

        TSeqPos start = seg.GetRange().GetFrom();
        TSeqPos stop  = seg.GetRange().GetTo();
        TSeqPos sStart = (TModelUnit)m_AlnMgr->GetSeqPosFromAlnPos(anchor, seg.GetAlnRange().GetFrom());
        TSeqPos sStop  = (TModelUnit)m_AlnMgr->GetSeqPosFromAlnPos(anchor, seg.GetAlnRange().GetTo());
        

        TSeqPos frame_shift_f = start % base_width_curr;
        TSeqPos frame_shift_t = 2 - stop % base_width_curr;
        if (base_width_anchor == 3) {
            frame_shift_f = sStart % base_width_anchor;
            frame_shift_t = 2 - sStop % base_width_anchor;
        }
        if (sStart > sStop) {
            swap(sStart, sStop);
            swap(frame_shift_f, frame_shift_t);
        }
        stop /= base_width_curr;
        start /= base_width_curr;
        sStart /= base_width_anchor;
        sStop /= base_width_anchor;

        TSeqPos seg_start = sStart;
        TSeqPos seg_stop = sStop;

        // Make sure nucleotide coordinate aligns protein codon's
        // starting/ending base. We need to do protein
        if (spacing == eOne2Three) {
            // top/anchor sequence is nucleotide, and and aligned sequence
            // is protein.
            sStop += frame_shift_t;
            sStart -= frame_shift_f;
        } else if (spacing == eThree2One) {
            // make sure it aligns codon's starting base
            stop += frame_shift_t;
            start -= frame_shift_f;
        }

        // check if sStop is larger than the sequence lenght
        sStop = min(sStop, mhndl.GetBioseqLength());

        try {

            string seq, mseq, tmp_seq, visible_seq;
            s_vec.GetSeqData (start, stop + 1, tmp_seq);
            ms_vec.GetSeqData(sStart, sStop + 1, mseq);
            visible_seq = tmp_seq;

            if (reverse_strand) {

                if (rb) {
                    // protein sequence, just reverse it
                    CSeqManip::Reverse(tmp_seq, CSeqUtil::e_Iupacaa,
                        0, tmp_seq.length(), seq);
                    visible_seq = seq;
                } else {
                    // nucleotide sequence, get reverse and complement
                    CSeqManip::ReverseComplement(tmp_seq, CSeqUtil::e_Iupacna,
                                                 0, tmp_seq.length(), seq);

                    if (m_Context->IsFlippedStrand()) {
                        CSeqManip::Reverse(tmp_seq, CSeqUtil::e_Iupacna,
                                           0, tmp_seq.length(), visible_seq);
                    } else {
                        visible_seq = seq;
                    }

                }

            } else {
                if (!rb && m_Context->IsFlippedStrand()) {
                    CSeqManip::Complement(tmp_seq, CSeqUtil::e_Iupacna,
                                          0, tmp_seq.length(), visible_seq);
                }
                swap(seq, tmp_seq);
            }
            string translated_seq;
            switch (spacing) {
            case eOne2Three:
                {
                    string seq_candidate = mseq;
                    if (reverse_strand) {
                        CSeqManip::ReverseComplement(mseq, CSeqUtil::e_Iupacna,
                            0, mseq.length(), seq_candidate);
                    }
                    CGenetic_code gcode;
                    gcode.SetId(m_AlnMgr->GetGenCode(anchor));
                    CSeqTranslator::Translate(seq_candidate, translated_seq, CSeqTranslator::fIs5PrimePartial, &gcode);
                    if (reverse_strand) {
                        // don't use the same string as the input and output string.
                        CSeqManip::Reverse(translated_seq, CSeqUtil::e_Iupacaa,
                            0, translated_seq.length(), tmp_seq);
                        translated_seq = tmp_seq;
                    }
                }
                break;
            case eThree2One:
                {
                    auto r = seg.GetAlnRange();
                    r.SetFrom(r.GetFrom() - frame_shift_f);
                    r.SetTo(r.GetTo() + frame_shift_t);
                    m_AlnMgr->GetAlnSeqString(row, seq, r);
                    CGenetic_code gcode;
                    gcode.SetId(m_AlnMgr->GetGenCode(row));
                    CSeqTranslator::Translate(seq, translated_seq, CSeqTranslator::fIs5PrimePartial, &gcode);
                }
                break;
            default:
                break;
            }

            char bases[2];
            bases[1] = '\0';
            TVPUnit font_w =
                (TVPUnit)(gl.TextWidth(&(m_Config->m_SeqFont),"G") * 0.5f);
            TModelUnit addon = 0.5 - m_Context->ScreenToSeq(font_w);

            TSeqPos start_base = frame_shift_f % base_width_anchor;
            TSeqPos stop_base = min(stop - start + 1 - frame_shift_t % base_width_anchor,
                (TSeqPos)seq.length());            

            // Draw characters (actg)
            for (TSeqPos bp = start_base;  bp < stop_base;  bp++) {
                TModelUnit pos = 0;
                bool match = true;
                switch (spacing) {
                    case eOne2One:
                        pos = sStart + bp + addon;
                        match = bp < seq.size() && bp < mseq.size() 
                            && seq[bp] == mseq[bp];
                        break;
                    case eOne2Three:
                        pos =  sStart + bp * 3 + addon + 1;
                        match = bp < seq.size() && bp < translated_seq.size() 
                            && seq[bp] == translated_seq[bp];
                        break;
                    case eThree2One: {
                        pos = sStart + bp * kPWAlignPackLevel + addon - kPWAlignPackLevel;
                        int bp_pos = bp / 3;
                        match = bp_pos < seq.size() && bp_pos < translated_seq.size()
                            && mseq[bp_pos] == translated_seq[bp_pos];
                        break;
                    }
                    default:
                        break;
                }
                if (pos < (seg_start - 0.5)) 
                    continue;
                if (pos > (seg_stop + 1))
                    continue;

                // Highlight mismatch?
                match ? gl.ColorC(c_seq) : gl.ColorC(c_mis);
                if (match && !m_Config->m_ShowIdenticalBases) {
                    CRgbaColor c = c_seq;
                    c.Lighten(0.65f);
                    gl.ColorC(c);
                    TModelUnit bh = GetBarHeight();
                    m_Context->DrawDisk2(TModelPoint(pos, center_y), (bh / 2.0) + 0.5, c);
                }
                else {
                    // set scoring if avaliable
                    if (m_Score  &&  m_Score->HasScores()) {
                        if (m_Score->GetSupportedColorTypes() & IScoringMethod::fForeground) {
                            auto_ptr<IAlnScoreIterator> score_iter(m_Score->GetScoreIterator(row));
                            score_iter->MoveTo(bp);
                            IScoringMethod::TScore score = score_iter->GetAttr();
                            CRgbaColor cl = m_Score->GetColorForScore(score,
                                IScoringMethod::fForeground);
                            cl.SetAlpha(m_FadeFactor);
                            gl.ColorC(cl);  // use scoreing color
                        }
                        else {
                            gl.ColorC(c_seq);  // use default color
                        }
                    }

                    bases[0] = visible_seq[bp];
                                        
                    m_Context->TextOut(&m_Config->m_SeqFont, bases,
                        pos, center_y + fs * 0.5, false);
                }
            } // for TSeqPos
        }
        catch (CException&) {
            /// ignore exceptions - these are all objmgr related
        }
    } // for i = 0

    if (IsLastRow()) {
        gl.ColorC(CRgbaColor(c_seq, 1.0f));
        const char* long_msg = (GetTearline() < eMaxRowLimit) ?
            "Alignment rows truncated for 'Adaptive' display - select 'Show All' to see more"
            : "The number of alignment rows exceeds the supported limit";
        const char* short_msg = (GetTearline() < eMaxRowLimit) ?
            "Select 'Show All' to see all alignments"
            : "Number of rows exceeds limit";
        const char* char_msg = "X";
        TModelUnit long_msg_width = gl.TextWidth(&m_Config->m_LabelFont, long_msg);
        TModelUnit short_msg_width = gl.TextWidth(&m_Config->m_LabelFont, short_msg);
        TModelUnit char_msg_width = gl.TextWidth(&m_Config->m_LabelFont, char_msg);
        
        TModelUnit width = m_Context->SeqToScreen(GetWidth());
        TModelUnit seq_char_width = m_Context->ScreenToSeq(char_msg_width);

        const char* msg = NULL;
        TModelUnit msg_width = 0.0;

        if (width > long_msg_width) {
            msg = long_msg;
            msg_width = long_msg_width;
        }
        else if (width > short_msg_width) {
            msg = short_msg;
            msg_width = short_msg_width;
        }
        else if (width > char_msg_width) {
            msg = char_msg;
            msg_width = char_msg_width;
        }
        else {
        }

        TModelUnit msg_spacing = char_msg_width * 4.0;

        if (msg != NULL) {
            // Only room for the message - no filler on the sides
            if (width < msg_width + msg_spacing*2.0) {
                TModelUnit w = (width - msg_width)*0.5;
                m_Context->TextOut(&m_Config->m_LabelFont, msg, 
                    GetLeft() + m_Context->ScreenToSeq(w), center_y + fs * 0.5, false);
            }
            // Room for message + filler:
            else {
                TModelUnit w = (width - msg_width)*0.5;
                TModelUnit seq_w = m_Context->ScreenToSeq(w);

                m_Context->TextOut(&m_Config->m_LabelFont, msg, 
                    GetLeft() + m_Context->ScreenToSeq(w), center_y + fs * 0.5, false);

                // Draw jagged lines on either side of the message
                gl.Color3f(0.5, 0.5, 0.5f);
                vector<TModelPoint> pts;
                bool bottom_y = true;
                TModelUnit pos = GetLeft();
                while (pos < GetLeft() + seq_w - seq_char_width) {
                    if (bottom_y)
                        pts.push_back(TModelPoint(pos, center_y + fs * 0.5));                    
                    else 
                        pts.push_back(TModelPoint(pos, center_y - fs * 0.5));
                    
                    bottom_y = !bottom_y;
                    pos += seq_char_width;
                }
                m_Context->DrawLineStrip(pts);
                pts.clear();

                bottom_y = true;
                pos = GetRight();
                while (pos > GetRight() - (seq_w - seq_char_width)) {
                    if (bottom_y) 
                        pts.push_back(TModelPoint(pos, center_y + fs * 0.5));                    
                    else 
                        pts.push_back(TModelPoint(pos, center_y - fs * 0.5));
                    
                    bottom_y = !bottom_y;
                    pos -= seq_char_width;
                }
                m_Context->DrawLineStrip(pts);
            }
        }
    }   
}

// Scoring structures/functions start
/// represents a pixel with a score, if several intervals cover a pixel, the interval
/// with the maximum coverage will "own" the pixel (i.e. the pixel will be colored
/// according to the score of this interval)
/// This is used for non-averageable scores.
struct SScPix   {
    float   m_Score;    /// score assigned to the pixel
    float   m_MaxCoverage; /// fraction of the pixel corresponding to the m_Score
    bool    m_Gap; /// true if this is a gap interval

    bool operator==(const SScPix& other) const
    {
        return m_Score == other.m_Score  &&
            m_MaxCoverage == other.m_MaxCoverage  &&
            m_Gap == other.m_Gap;
    }
};


/// a pixel with the Max coverage (fraction) overrides all attributes
static void FSetPixMaxCoverage(SScPix& dst, const SScPix& src,
                               float fraction, bool)
{
    if(fraction > dst.m_MaxCoverage)   {
        dst.m_Score = src.m_Score;
        dst.m_Gap = src.m_Gap;
        dst.m_MaxCoverage = fraction;
    }
}


void CAlignGlyph::x_RenderNonAveragableScores(TModelUnit yy,
                                              TSignedSeqRange aln_range,
                                              TSeqRange seq_range,
                                              TModelUnit height,
                                              int row,
                                              bool override_color,
                                              const CRgbaColor& read_status_color) const
{
    IRender& gl = GetGl();

    TModelUnit fromM = (TModelUnit)seq_range.GetFrom();
    TModelUnit toM   = (TModelUnit)seq_range.GetToOpen();
    int anchor = m_AlnMgr->GetAnchor();
    TSeqPos base_width_anchor = m_AlnMgr->GetBaseWidth(anchor);

    // coloration range in the viewport coordinate system
    TVPUnit scoreStart = m_Context->SeqToScreenX(fromM);
    TVPUnit scoreEnd = m_Context->SeqToScreenX(toM);

    typedef CRasterizer<SScPix> TR;
    TR rasterizer(scoreStart, scoreEnd, fromM, toM);
    SScPix pix = { 0.0f, 0.0f, false };

    auto_ptr<IAlnScoreIterator> score_iter(m_Score->GetScoreIterator(row));
    IAlnScoreIterator& it = *score_iter;
    auto_ptr<IAlnSegmentIterator> p_it(m_AlnMgr->CreateSegmentIterator(row,
        aln_range, IAlnSegmentIterator::eSkipGaps));

    IAlnSegmentIterator& aln_seg_it(*p_it);
    while (aln_seg_it  &&  it) {
        const IAlnSegment& seg = *aln_seg_it;
        const TSignedSeqRange& r = seg.GetAlnRange();

        TSignedSeqPos start1 = r.GetFrom();
        TSignedSeqPos stop1 = r.GetTo();
        TSignedSeqPos start2 = it.GetFrom();
        TSignedSeqPos stop2 = it.GetTo();

        while (it  &&  start1 > stop2) {
            ++it;
            start2 = it.GetFrom();
            stop2 = it.GetTo();
        }

        if (start1 > stop2) {
            break;
        }

        int from, to;
        from = std::max<int>(start1, start2);
        to   = std::min<int>(stop1,  stop2);

        TModelUnit act_from = m_AlnMgr->GetSeqPosFromAlnPos(anchor, from);
        act_from /= base_width_anchor;
        TModelUnit act_to = m_AlnMgr->GetSeqPosFromAlnPos(anchor, to);
        act_to /= base_width_anchor;
        if (act_from > act_to) {
            swap(act_from, act_to);
        }

        if (act_from <= act_to) {//! seg.IsGap()) {
            pix.m_Score = it.GetAttr();
            pix.m_Gap = seg.IsGap();
            rasterizer.AddInterval(act_from, act_to + 1.0 / base_width_anchor, pix, FSetPixMaxCoverage);
        }

        /// advance iterators
        if (to == stop1) {
            ++aln_seg_it;
        }
        if (to == stop2) {
            ++it;
        }
    }

    // Rendering
    const TR::TRaster& raster = rasterizer.GetRaster();
    static float eps = 0.0001f;
    double shift_x = scoreStart;

    gl.PushMatrix();
    gl.Scalef(m_Context->GetScale(), 1.0f, 1.0f);
    for (size_t i = 0;  i < raster.size(); ) {
        double start = i + shift_x;
        const SScPix& sc_pix = raster[i];
        while (++i < raster.size()  &&  raster[i] == sc_pix);

        if (sc_pix.m_MaxCoverage > eps)    {
            CRgbaColor color = m_Score->GetColorForScore(
                sc_pix.m_Score, IScoringMethod::fBackground);
            if (override_color)
                color = read_status_color;

            // Lighten color if we are fading out (instead of using
            // blending which we don't want for the rectanges which should
            // overwrite (not blend with) what is behind them.
            if (m_FadeFactor < 1.0f)
                color.Lighten(1.0 - m_FadeFactor);
            gl.ColorC(color);

            // vertical offset from the border of the alignment band
            // to the border of gap band
            int GapOffset = 2;
            int off_y = sc_pix.m_Gap ? GapOffset : 0;

            double x2 = i + shift_x;
            if (start == x2) {
                gl.Begin(GL_LINES);
                gl.Vertex2d(start, yy + off_y);
                gl.Vertex2d(start, yy + height - off_y);
                gl.End();
            } else {
                gl.Rectd(start, yy + off_y, x2, yy + height - off_y);
            }
        }
    }
    gl.PopMatrix();
}


struct SScPix2   {
    float m_IntScore;
    float m_IntLen;
    float m_GapScore;
    float m_GapLen;
    float m_MinScore;
    bool  m_Init;

    bool    operator==(const SScPix2& other) const
    {
        return m_IntScore == other.m_IntScore  &&  m_IntLen == other.m_IntLen &&
        m_GapScore == other.m_GapScore  &&  m_GapLen == other.m_GapLen &&
        m_MinScore == other.m_MinScore;
    }
};


/// accumulate weighted scores
static void FSetPix2(SScPix2& dst, const SScPix2& src, float fraction, bool long_seg)
{
    dst.m_IntScore += src.m_IntScore * fraction;
    dst.m_IntLen += src.m_IntLen * fraction;
    dst.m_GapScore += src.m_GapScore * fraction;
    dst.m_GapLen += src.m_GapLen * fraction;

    if( ! long_seg)  {
        // only for small segments
        static const float eps = 0.00001f;
        if( ! dst.m_Init)   {
            if(src.m_IntLen > eps)  {
                dst.m_MinScore = src.m_IntScore;
                dst.m_Init = true;
            } else if(src.m_GapLen > eps)  {
                dst.m_MinScore = src.m_GapScore;
                dst.m_Init = true;
            }
        } else {
            if(src.m_IntLen > eps)  {
                dst.m_MinScore = min(dst.m_MinScore, src.m_IntScore);
            }
            if(src.m_GapLen > eps)  {
                dst.m_MinScore = min(dst.m_MinScore, src.m_GapScore);
            }
        }
    }
}


void CAlignGlyph::x_RenderAveragableScores(TModelUnit yy,
                                           TSignedSeqRange aln_range,
                                           TSeqRange seq_range,
                                           TModelUnit height,
                                           int row,
                                           bool override_color,
                                           const CRgbaColor& read_status_color) const
{
    if (seq_range.Empty())
        return;

    IRender& gl = GetGl();

    TModelUnit fromM = (TModelUnit)seq_range.GetFrom();
    TModelUnit toM   = (TModelUnit)seq_range.GetToOpen();
    int anchor = m_AlnMgr->GetAnchor();
    TSeqPos base_width_anchor = m_AlnMgr->GetBaseWidth(anchor);

    // coloration range in the viewport coordinate system
    TVPUnit scoreStart = m_Context->SeqToScreenXInModelUnit(fromM);
    TVPUnit scoreEnd = TVPUnit(ceil(m_Context->SeqToScreenXInModelUnit(toM)));

    typedef CRasterizer<SScPix2>  TR;
    TR rasterizer(scoreStart, scoreEnd, fromM, toM);
    CRange<float> value_limits(numeric_limits<float>::max(), numeric_limits<float>::min());
    auto_ptr<IAlnScoreIterator> score_it(m_Score->GetScoreIterator(row));
    IAlnScoreIterator& it = *score_it;
    auto_ptr<IAlnSegmentIterator> p_it(m_AlnMgr->CreateSegmentIterator(row,
        aln_range, IAlnSegmentIterator::eAllSegments));

    IAlnSegmentIterator& aln_seg_it(*p_it);
    while (aln_seg_it && it) {
        const IAlnSegment& seg = *aln_seg_it;
        const TSignedSeqRange& r = seg.GetAlnRange();

        TSignedSeqPos start1 = r.GetFrom();
        TSignedSeqPos stop1 = r.GetTo();
        TSignedSeqPos start2 = it.GetFrom();
        TSignedSeqPos stop2 = it.GetTo();

        while (it && start1 > stop2) {
            ++it;
            if (it) {
                start2 = it.GetFrom();
                stop2 = it.GetTo();
            }
        }

        if (start1 > stop2) {
            break;
        }

        TSignedSeqPos from, to;
        from = std::max<int>(start1, start2);
        to = std::min<int>(stop1, stop2);

        SScPix2 pix = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false };
        TModelUnit act_from = m_AlnMgr->GetSeqPosFromAlnPos(anchor, from, IAlnExplorer::eBackwards);
        act_from /= base_width_anchor;
        TModelUnit act_to = m_AlnMgr->GetSeqPosFromAlnPos(anchor, to, IAlnExplorer::eForward);
        act_to /= base_width_anchor;
        if (act_from > act_to) {
            swap(act_from, act_to);
        }
        if (act_from <= act_to && act_from <=toM) {
            if (seg.IsAligned()) {
                pix.m_IntLen = 1.;
                pix.m_IntScore = it.GetAttr();
                if (pix.m_IntScore < value_limits.GetFrom())
                    value_limits.SetFrom(pix.m_IntScore);
                else if (pix.m_IntScore > value_limits.GetTo())
                    value_limits.SetTo(pix.m_IntScore);
                rasterizer.AddInterval(act_from, act_to + 1.0 / base_width_anchor, pix, FSetPix2);
            //}  else if (seg.IsIndel()) {
                //++aln_seg_it;
                //continue;
            } else if (seg.IsIndel() || seg.IsGap()) {
                int len = (act_to - act_from) + 1;// / base_width_anchor;
                if (m_Context->SeqToScreen(len) <= 1.5) {
                    pix.m_GapLen = 1.;
                    pix.m_GapScore = it.GetAttr();
                    rasterizer.AddInterval(act_from, act_to + 1.0 / base_width_anchor, pix, FSetPix2);
                }
            }
        }

        /// advance iterators
        if (to == stop1) {
            ++aln_seg_it;
        }
        if (to == stop2) {
            ++it;
        }
    }

    // Rendering
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    const TR::TRaster& raster = rasterizer.GetRaster();

    double barh = abs(m_Context->GetGlPane()->GetScaleY() * height);

    double shift_x = scoreStart;
    static const float eps = 0.001f;
    float value_scale = value_limits.GetTo() - value_limits.GetFrom();
    if (value_scale <= 0)
        value_scale = 1;
    gl.PushMatrix();
    gl.Scalef(m_Context->GetScale(), 1.0f, 1.0f);
    for (size_t i = 0;  i < raster.size(); )    {
        double start = i + shift_x;
        const SScPix2& sc_pix = raster[i];
        while (++i < raster.size()  &&  raster[i] == sc_pix);
        CRgbaColor color = m_Score->GetColorForScore(sc_pix.m_MinScore, IScoringMethod::fBackground);
        bool draw = false;
        if (sc_pix.m_IntLen > eps) {
            draw = true;
            float sc = sc_pix.m_IntScore / sc_pix.m_IntLen;
            //float sc = min(1.0f, float(sc_pix.m_IntScore / sc_pix.m_IntLen));
            color = override_color ? 
                   read_status_color : m_Score->GetColorForScore(sc, IScoringMethod::fBackground);
            if (sc_pix.m_Init) {
                sc = min(1.0f, ((sc - value_limits.GetFrom()) / value_scale));
                CRgbaColor color2 = m_Score->GetColorForScore(sc_pix.m_MinScore,
                                                              IScoringMethod::fBackground);
                float alpha = 1.0f - sc * 0.75f;
                color = CRgbaColor::Interpolate(color2, color, alpha);
            }
            // m_GapLen is and insertion length
            if (sc_pix.m_GapLen > eps) {
                //float sc = min(1.0f, sc_pix.m_GapScore / sc_pix.m_GapLen);
                // make lower score more prominent
                float alpha = min(1.0f, 0.25f + sc_pix.m_GapLen * 0.75f);
                color = CRgbaColor::Interpolate(m_Config->m_Insertion, color, alpha);
            }
        }
        if (draw) {
            color.Lighten(1.0 - color.GetAlpha() * m_FadeFactor);
            gl.Color4d(color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha()*m_FadeFactor);
            double x2 = i + shift_x;
            if (start == x2) {
                gl.Begin(GL_LINES);
                gl.Vertex2d(start, yy);
                gl.Vertex2d(start, yy + height);
                gl.End();
            } else {

                if (barh >= 2.) {
                    gl.Rectd(start, yy, x2, yy + height);
                } else {
                    // if bar height is less than 2px
                    // blue thin line that goes through the whole
                    // alignment glyph occasianally gets visible
                    // when rendered by MESA (blue band issue).
                    // Setting linewidth to 2 fixes that
                    
                    gl.LineWidth(2);
                    gl.Begin(GL_LINES);
                    gl.Vertex2d(start, yy);
                    gl.Vertex2d(x2, yy);
                    gl.End();
                    gl.LineWidth(1.);
                }
            }
        }

    }
    gl.PopMatrix();
}

static float kMinSegDistance = 1.5f;

void CAlignGlyph::x_CalcIntervals(TIntervals& intervals, int target_row) const
{
    intervals.clear();

    int anchor = m_AlnMgr->GetAnchor();
    TSeqPos base_width_anchor = m_AlnMgr->GetBaseWidth(anchor);
    TSeqRange r(
        m_AlnMgr->GetSeqStart(anchor) / base_width_anchor,
        m_AlnMgr->GetSeqStop(anchor) / base_width_anchor);

    if (r.Empty()) {
        LOG_POST(Error
            << "CAlignGlyph::x_CalcIntervals: zero alignment range!");
        return;
    }

    {
        TModelRange inrc = m_Context->IntersectVisible(r);
        if (inrc.Empty())
            return;
        //r.SetOpen((TSeqPos)inrc.GetFrom(), (TSeqPos)inrc.GetToOpen());
        //if (r.Empty())
          //  return;
    }
    // optimization
    // Only use the total range if the size is small
    if (IsSimplified()) {
        intervals.push_back(r);
        return;
    }
    TSignedSeqPos aln_r_s = m_AlnMgr->GetAlnPosFromSeqPos(
        anchor, r.GetFrom() * base_width_anchor, IAlnExplorer::eRight);
    TSignedSeqPos aln_r_e = m_AlnMgr->GetAlnPosFromSeqPos(
        anchor, r.GetTo() * base_width_anchor, IAlnExplorer::eLeft);
    if (aln_r_s > aln_r_e) {
        swap(aln_r_s, aln_r_e);
    }
    CRange<TSignedSeqPos> aln_range(aln_r_s, aln_r_e);

    int row = target_row;
    if (row < 0)
        row = max(0, 1 - anchor);

    auto_ptr<IAlnSegmentIterator> p_it(m_AlnMgr->CreateSegmentIterator(row,
        aln_range,
        IAlnSegmentIterator::eSkipGaps));
    TSeqRange rrr;
    for ( IAlnSegmentIterator& it(*p_it);  it;  ++it )  {
        const IAlnSegment& seg = *it;
        if (seg.GetRange().Empty())
            continue;
        if (!seg.IsAligned() || seg.IsIndel())
            continue;
        const TSignedSeqRange& aln_r = seg.GetAlnRange();
        if (aln_r.Empty())
            continue;
        TSeqPos start = m_AlnMgr->GetSeqPosFromAlnPos(anchor, aln_r.GetFrom());
        start /= base_width_anchor;
        TSeqPos stop = m_AlnMgr->GetSeqPosFromAlnPos(anchor, aln_r.GetTo());
        stop /= base_width_anchor;
        if (start > stop)
            swap(start, stop);
        bool add = true;
        if (!intervals.empty()) {
            TModelUnit dist = m_Context->SeqToScreen(start - intervals.back().GetTo());
            if (dist < kMinSegDistance)  {
                intervals.back().SetTo(stop);
                add = false;
            }
        }
        if (add)
            intervals.push_back( TSeqRange(start, stop) );
    }
}

bool CAlignGlyph::x_ShowLabel() const
{
    bool shown = false;
    if (m_Config->m_ShowLabel  &&
        m_Config->m_LabelPos != CAlignmentConfig::ePos_NoLabel) {
        if (m_Config->m_LabelPos == CAlignmentConfig::ePos_Above) {
            TModelRange r = m_Context->IntersectVisible(GetRange());
            if (m_Context->WillLabelFit(r)) {
                shown = true;
            }
        } else {
            shown = true;
        }
    }
    return shown;
}


bool CAlignGlyph::x_ShowScore() const
{
    return !m_Context->IsOverviewMode()  &&  m_Score  &&  m_Score->HasScores();
}


bool CAlignGlyph::x_ShowUnalignedTails(TModelUnit align_len) const
{
    // Only for pairwise alignments with a minimal screen size
    return m_AlnMgr->HasPolyATail() != IAlnGraphicDataSource::ePolyA_No  &&
        !m_Context->IsOverviewMode() &&
        align_len > kTailGlyphLen &&
        ((m_Config->m_UnalignedTailsMode != CAlignmentConfig::eTails_Hide) ||
        (m_Config->m_ShowUnalignedTailsForTrans2GenomicAln  &&  m_SeqAlign->GetSegs().IsSpliced()))  &&
        x_IsPWAlign();
}

TSeqRange CAlignGlyph::x_GetAlnTailRange(const ETail tail, const SDrawContext& ctx) const
{
    if (e5_Prime == tail) 
        return TSeqRange(ctx.f - ctx.tail_5*ctx.base_width_curr, ctx.f - 1);
    else
        return TSeqRange(ctx.t + 1, ctx.t + ctx.tail_3*ctx.base_width_curr);
}

TSeqRange CAlignGlyph::x_ConvertAlnTaillRngToSeqRng(const ETail tail, const SDrawContext& ctx, const TSeqRange &aln_range) const
{
    int aligned_seq = m_AlnMgr->GetQuery();
    TSignedSeqPos aln_start = m_AlnMgr->GetSeqStart(aligned_seq) / ctx.base_width_curr;
    TSignedSeqPos aln_stop = m_AlnMgr->GetSeqStop(aligned_seq) / ctx.base_width_curr;

    if (ctx.reverse_strand) {
        if (e5_Prime == tail) {
            TSignedSeqPos start = (ctx.f - aln_range.GetToOpen()) / ctx.base_width_curr;
            start *= ctx.base_width_anchor;
            start += aln_stop + 1;
            TSignedSeqPos end = (ctx.f - aln_range.GetFrom()) / ctx.base_width_curr;
            end *= ctx.base_width_anchor;
            end += aln_stop + 1;
            return TSeqRange(start, end);
        }
        else { // e3_Prime
            TSignedSeqPos start = (aln_range.GetToOpen() - ctx.t - 1) / ctx.base_width_curr;
            start *= ctx.base_width_anchor;
            start = aln_start - start;
            TSignedSeqPos end = (aln_range.GetFrom() - ctx.t - 1) / ctx.base_width_curr;
            end *= ctx.base_width_anchor;
            end = aln_start - end;
            return TSeqRange(start, end);
        }
    }
    else {
        if (e5_Prime == tail) {
            TSignedSeqPos start = (aln_range.GetFrom() - ctx.f) / ctx.base_width_curr;
            start *= ctx.base_width_anchor;
            start += aln_start;
            TSignedSeqPos end = (aln_range.GetToOpen() - ctx.f) / ctx.base_width_curr;
            end *= ctx.base_width_anchor;
            end += aln_start;
            return TSeqRange(start, end);
        }
        else { // e3_Prime
            TSignedSeqPos start = (aln_range.GetFrom() - ctx.t) / ctx.base_width_curr;
            start *= ctx.base_width_anchor;
            start += aln_stop;
            TSignedSeqPos end = (aln_range.GetToOpen() - ctx.t) / ctx.base_width_curr;
            end *= ctx.base_width_anchor;
            end += aln_stop;
            return TSeqRange(start, end);
        }
    }
}

TModelUnit CAlignGlyph::x_GetTailLabelWidth(const TSignedSeqPos tail) const
{
    IRender& gl = GetGl();

    TModelUnit h_text = gl.TextHeight(&m_Config->m_LabelFont);
    if (h_text >= GetBarHeight())
        return 0.0;

    string len_str = NStr::IntToString(tail, NStr::fWithCommas);
    return m_Context->ScreenToSeq(gl.TextWidth(&m_Config->m_LabelFont, len_str.c_str()));
}

TModelUnit CAlignGlyph::x_GetTailWidth(const TSignedSeqPos tail, const TSignedSeqPos visible_tail, const TSignedSeqPos base_width_curr) const
{
    TModelUnit tail_len = 0;
    TModelUnit tail_label_len = 0;
    TModelUnit bar_h = GetBarHeight();
    bool compact = (GetBarHeight() <= 2);  // tiny bars, compact mode 

    int label_space_pix = compact ? kCompactLabelSpace_h : kLabelSpace_h;
    int tail_len_pix = compact ? kCompactTailGlyphLen  : kTailGlyphLen;
    tail_len_pix += label_space_pix;
            
    switch (m_Config->m_UnalignedTailsMode) {
    case CAlignmentConfig::eTails_ShowGlyph:
        // Preserve horizontal space for the unaligned tail glyph
        tail_len = m_Context->ScreenToSeq(tail_len_pix);
        tail_label_len = x_GetTailLabelWidth(tail);
        if (tail_label_len > 0) {
            // Preserve horizontal space for the unaligned tail length label
            tail_len += tail_label_len + m_Context->ScreenToSeq(label_space_pix);
        }
        break;
    case CAlignmentConfig::eTails_ShowSequence:
        // Preserve space for the unaligned sequence
        tail_len = visible_tail*base_width_curr;
        if (visible_tail < tail) { // The visible unaligned sequence is truncated
            // Preserve horizontal space for the unaligned tail glyph
            tail_len += m_Context->ScreenToSeq(tail_len_pix);
            tail_label_len = x_GetTailLabelWidth(tail - visible_tail);
            if (tail_label_len > 0) {
                // Preserve horizontal space for the unaligned tail length label
                tail_len += tail_label_len + m_Context->ScreenToSeq(label_space_pix);
            }
        }
        break;
    default:
        break;
    }
    return tail_len;
}

bool CAlignGlyph::x_IsSRA() const
{
    try {
        int aligned_seq = m_AlnMgr->GetAnchor() == 0 ? 1 : 0;
        CSeq_id_Handle shdl = sequence::GetId(m_AlnMgr->GetSeqId(aligned_seq), m_Context->GetScope(), sequence::eGetId_Canonical);
        string id_str = shdl.GetSeqId()->AsFastaString();
        return ((id_str.find("gnl|SRA") != string::npos) && (CSeqUtils::IsVDBAccession(m_OrigSeqAlign.GetAnnot().GetName())));
    }
    catch (const CException&) {

    }
    return false;
}

void CAlignGlyph::x_GetOverrideColor(bool& override_color,
                                     CRgbaColor& read_status_color) const
{
    // Special colors for sra reads with errors: failed reads red,
    // pcr duplicates pale gray.  SV-2153
    override_color = false;

    if (m_SeqAlign->IsSetExt()) {
        const CSeq_align_Base::TExt& exts = m_SeqAlign->GetExt();
        ITERATE (CSeq_align_Base::TExt, iter, exts) {
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == "PCR duplicate") {
                    override_color = true;
                    read_status_color.Set(0.9f, 0.67f, 0.9f, m_FadeFactor);
            }
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == "Poor sequence quality") {
                    override_color = true;
                    read_status_color.Set(1.0f, 0.17f, 0.0f, m_FadeFactor);
            }
        }
    }
}

bool CAlignGlyph::LessBySeqPos(const CSeqGlyph& obj) const
{
    const CAlignGlyph* align = dynamic_cast<const CAlignGlyph*>(&obj);
    if (align) try {
        int anchor1 = align->GetAlignMgr().GetAnchor();
        int from1 = align->GetAlignMgr().GetSeqStart(anchor1);

        int anchor0 = m_AlnMgr->GetAnchor();
        int from0 = m_AlnMgr->GetSeqStart(anchor0);
        if (from0 < from1)
            return true;
        if (from0 > from1)
            return false;
        int to1 = align->GetAlignMgr().GetSeqStop(anchor1);
        int to0 = m_AlnMgr->GetSeqStop(anchor0);
        return to0 < to1;
    } catch (CException&) {
    }
    // otherwise, using default, compare by SeqRanges
    return CSeqGlyph::LessBySeqPos(obj);
}

END_NCBI_SCOPE
