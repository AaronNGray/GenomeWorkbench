/*  $Id: mate_pair_glyph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
 *   CMatePairGlyph -- utility class to layout mate pairs (a special type of
 *      pairwise alignments and hold a set of CAlignGlyph
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/mate_pair_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/opengl/irender.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objtools/alnmgr/alnvec.hpp>



BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CMatePairGlyph::CMatePairGlyph(const TAlignList& aligns)
    : m_SeqAligns(aligns)
    , m_LibraryId(eLibrary_NotSet)
    , m_ErrorType(eError_NotSet)
{
    if (m_SeqAligns.size() != 1) {
        m_ErrorType = eError_NoError;
    }

    const CSeq_id* id = NULL;
    TSeqRange range;
    ITERATE (TAlignList, iter, m_SeqAligns) {
        const CAlignGlyph& pw = **iter;
        TSeqRange curr_r = pw.GetLocation().GetTotalRange();
        range += curr_r;
        id = &pw.GetAlignMgr().GetSeqId(pw.GetAlignMgr().GetAnchor());

        const CSeq_align_Handle& align = pw.GetOrigAlignment();
        int type = 0;
        if (m_ErrorType != eError_NoError  &&
            align.GetSeq_align()->GetNamedScore("bad matepair code", type)) {
            m_ErrorType = static_cast<EErrorType>(type);
        }
        align.GetSeq_align()->GetNamedScore("matepair library", m_LibraryId);
        m_Intervals.push_back(curr_r);
    }

    if (m_LibraryId == eLibrary_NotSet) {
        m_LibraryId = eLibrary_NotFound;
    }

    if (m_ErrorType == eError_NotSet) {
        m_ErrorType = eError_NoError;
    }

    // if not set, set it based on pairs' orientation
    if (m_SeqAligns.size() == 2  &&  m_ErrorType == eError_NoError) {
        if (m_SeqAligns[0]->IsNegative() == m_SeqAligns[1]->IsNegative()) {
            m_ErrorType = eError_CoAlign;
        } else {
            m_ErrorType = eError_ContraAlign;
        }
    }

    if (id) {
        m_Location.Reset(new CSeq_loc());
        m_Location->SetInt().SetFrom(range.GetFrom());
        m_Location->SetInt().SetTo  (range.GetTo());
        m_Location->SetId(*id);
    }
}


bool CMatePairGlyph::NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& /*t_title*/) const
{
    return true;
}


void CMatePairGlyph::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    TModelPoint lcl_p(p);
    x_World2Local(lcl_p);
/*
    ITERATE(TAlignList, iter, m_SeqAligns) {
        TModelPoint t = (*iter)->GetPos();
        if ((*iter)->IsIn(lcl_p)) {
            (*iter)->GetTooltip(p, tt, t_title);
            break;
        }
    }
*/
    TModelUnit bar_h = m_SeqAligns.front()->GetBarHeight();
    TModelUnit verticalOffset(0.0);
    // In case two mate pair ends overlap, the second one is put right under the first one.
    if (2 == m_SeqAligns.size()) {
        if (m_SeqAligns[0]->GetRange().IntersectingWith(m_SeqAligns[1]->GetRange()))
            verticalOffset = bar_h;
    }
    TSeqRange r(GetLeft(), GetRight());
    TModelUnit t_y = 0;
    bool show_label = m_Config->m_ShowLabel  &&  m_Context->WillLabelFit(r);
    if (show_label) {
        IRender& gl = GetGl();
        t_y += gl.TextHeight(&(m_Config->m_LabelFont)) + 4;
    }

    for (const auto& aln_g : m_SeqAligns) {
        if (lcl_p.m_Y >= t_y && lcl_p.m_Y <= t_y + bar_h) {
            const auto& r = aln_g->GetRange();
            if (lcl_p.m_X >= r.GetFrom() && lcl_p.m_X <= r.GetTo()) {
                aln_g->GetTooltip(p, tt, t_title);
                break;
            }
        }
        t_y += verticalOffset;
    }

}


void CMatePairGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    TModelUnit bar_h = m_SeqAligns.front()->GetBarHeight();
    TModelUnit verticalOffset(0.0);
    // In case two mate pair ends overlap, the second one is put right under the first one.
    if (2 == m_SeqAligns.size()) {
        if (m_SeqAligns[0]->GetRange().IntersectingWith(m_SeqAligns[1]->GetRange()))
            verticalOffset = bar_h;
    }

    TModelUnit t_x = 0;
    TModelUnit t_y = 0;
    x_Local2World(t_x, t_y);

    t_y += GetHeight();

    REVERSE_ITERATE (TAlignList, iter, m_SeqAligns) {
        CHTMLActiveArea area;
        TVPUnit x1 = m_Context->SeqToScreenXClipped((*iter)->GetLeft());
        TVPUnit x2 = m_Context->SeqToScreenXClipped((*iter)->GetRight());
        if (m_Context->IsFlippedStrand()) {
            x1 = -x1;
            x2 = -x2;
        }

        TVPUnit y1 = TVPUnit(t_y - bar_h);
        TVPUnit y2 = TVPUnit(t_y);

        t_y -= verticalOffset;

        area.m_Bounds.Init(x1, y2, x2, y1);
        area.m_SeqRange = (*iter)->GetRange();
        area.m_Signature = (*iter)->GetSignature();
        p_areas->push_back(area);
    }
}

TSeqRange CMatePairGlyph::GetRange(void) const
{
    return m_Location->GetTotalRange();
}


bool CMatePairGlyph::IsClickable() const
{
    return true;
}


const objects::CSeq_loc& CMatePairGlyph::GetLocation(void) const
{
    return *m_Location;
}


CConstRef<CObject> CMatePairGlyph::GetObject(TSeqPos pos) const
{
    ITERATE (TAlignList, iter, m_SeqAligns) {
        CConstRef<CObject> obj = (*iter)->GetObject(pos);
        if (obj.NotEmpty()) {
            return obj;
        }
    }
    return CConstRef<CObject>();
}


void CMatePairGlyph::GetObjects(vector<CConstRef<CObject> >& objs) const
{
    ITERATE (TAlignList, iter, m_SeqAligns) {
        (*iter)->GetObjects(objs);
    }
}


bool CMatePairGlyph::HasObject(CConstRef<CObject> obj) const
{
    ITERATE(TAlignList, iter, m_SeqAligns) {
        if ( (*iter)->HasObject(obj) ) {
            m_ObjSel.push_back(obj);
            return true;
        }
    }
    return false;
}


string CMatePairGlyph::GetSignature() const
{
    // always return the signature of the first alignment
    ITERATE (TAlignList, iter, m_SeqAligns) {
        return (*iter)->GetSignature();
    }
    return string();
}


const CMatePairGlyph::TIntervals& CMatePairGlyph::GetIntervals(void) const
{
    return m_Intervals;
}


bool CMatePairGlyph::IsPairSelected(const CAlignGlyph* pw_aln) const
{
    ITERATE(vector<CConstRef<CObject> >, iter, m_ObjSel) {
        if (pw_aln->HasObject(*iter))
            return true;
    }
    return false;
}


void CMatePairGlyph::x_Draw() const
{
    if (!GetVisible())
        return;
    NON_CONST_ITERATE(TAlignList, iter, m_SeqAligns)
    {
        (*iter)->SetVisible(true);
        (*iter)->SetTearline(GetTearline());
        (*iter)->SetRowNum(GetRowNum());
    }
    IRender& gl = GetGl();

    TModelRect rcm = GetModelRect();
    TSeqRange range = GetRange();
    TModelRange inrc = m_Context->IntersectVisible(range);

    TModelUnit yy = rcm.Top();
    bool show_label = m_Config->m_ShowLabel  &&  m_Context->WillLabelFit(inrc);
    if (show_label) {
        yy +=  gl.TextHeight(&(m_Config->m_LabelFont)) + 4;
    }

    TModelUnit bar_h = m_SeqAligns.front()->GetBarHeight();

    // Set Colors based on Error code for Mate Pairs
    CRgbaColor c_fg, c_seq, c_mis;
    switch (GetError()) {
    case eError_Orientation:
        c_fg = m_Config->m_FGOrientation;
        c_seq = m_Config->m_SeqOrientation;
        c_mis = m_Config->m_SeqMismatchOrientation;
        break;
    case eError_NonUnique:
        c_fg  = m_Config->m_FGNonUnique;
        c_seq = m_Config->m_SeqNonUnique;
        c_mis = m_Config->m_SeqMismatchNonUnique;
        break;
    case eError_Distance:
        c_fg  = m_Config->m_FGDistance;
        c_seq = m_Config->m_SeqDistance;
        c_mis = m_Config->m_SeqMismatchDistance;
        break;
    case eError_CoAlign:
        c_fg  = m_Config->m_FGCoAlign;
        c_seq = m_Config->m_SeqNo;
        c_mis = m_Config->m_SeqMismatchNo;
        break;
    case eError_ContraAlign:
        c_fg  = m_Config->m_FGContraAlign;
        c_seq = m_Config->m_SeqNo;
        c_mis = m_Config->m_SeqMismatchNo;
        break;
    default:
        c_fg  = m_Config->m_FGNo;
        c_seq = m_Config->m_SeqNo;
        c_mis = m_Config->m_SeqMismatchNo;
        break;
    }

    // Draw each Pairwise Alignment inside
    // If two mates overlap, move the second one below the first one
    TModelUnit y_off = 0.0;
    if (m_SeqAligns.size() == 2  &&
        m_SeqAligns[0]->GetRange().IntersectingWith(m_SeqAligns[1]->GetRange())) {
            y_off = bar_h;
    }
    TModelUnit yTop = yy;
    ITERATE(TAlignList, iter, m_SeqAligns) {
        TSeqRange l_range = (*iter)->GetRange();
        TModelRect rcm_pw(l_range.GetFrom(), yTop + bar_h, l_range.GetTo(), yTop);
        (*iter)->ApplyFading();
        (*iter)->DrawPWAlignElem(rcm_pw, false, false, c_fg, c_seq, c_mis);
        yTop += y_off;
    }

    
    auto fade_factor = GetFadeFactor();
    if (fade_factor != 1.) {
        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    // draw link between mate pairs
    if (m_SeqAligns.size() == 2) {
        CRgbaColor c(m_Config->m_FGLink, fade_factor);
        gl.ColorC(c);
        gl.LineWidth(1.0f);
        gl.Disable(GL_LINE_SMOOTH);
        gl.LineStipple(2, 0xAAAA);
        gl.Enable(GL_LINE_STIPPLE);

        //TSeqRange r1 = m_SeqAligns[0]->GetRange();
        //TSeqRange r2 = m_SeqAligns[1]->GetRange();
        TSeqRange r1(m_SeqAligns[0]->GetLeft(), m_SeqAligns[0]->GetLeft() + m_SeqAligns[0]->GetWidth());
        TSeqRange r2(m_SeqAligns[1]->GetLeft(), m_SeqAligns[1]->GetLeft() + m_SeqAligns[1]->GetWidth());
        TSeqRange inter_r = r1.IntersectionWith(r2);
        if (inter_r.Empty()) {
            TSeqPos from = r1.GetFrom() < r2.GetFrom() ? r1.GetTo() : r2.GetTo();
            TSeqPos to   = r1.GetFrom() < r2.GetFrom() ? r2.GetFrom() : r1.GetFrom();
            _ASSERT(from <= to);
            auto gap = m_Context->ScreenToSeq(1);
            m_Context->DrawLine(from + gap, yy + bar_h * 0.5f - 1.0, to + gap, yy + bar_h * 0.5f - 1.0);
        } else {
            m_Context->DrawLine(inter_r.GetFrom(), yy + bar_h, inter_r.GetTo(), yy + bar_h);
        }
        gl.Disable(GL_LINE_STIPPLE);
    }

    /*
    // Highlight background based on Library ID
    if (m_ConfigSettings->GetShowAlignBg()  &&
        pair->GetLibraryId() > 0  &&
        m_ConfigSettings->GetShowPWAlignLabels()) {
        CRgbaColor color = m_ColorTable.GetColor(pair->GetLibraryId());
        gl.Color4f(color.GetRed(), color.GetGreen(), color.GetBlue(), 0.1f);
        x_DrawQuad(rcm, horz);
    }
    */

    // draw label
    if (show_label) {
        string title;
        x_GetTitle(&title, CLabel::eDefault);

        TModelUnit widthP  = m_Context->SeqToScreen(inrc.GetLength());
        title = m_Config->m_LabelFont.Truncate(title.c_str(), widthP);

        CRgbaColor text_color(IsSelected() ? 
                              m_Context->GetSelLabelColor() : m_Config->m_Label,
                              fade_factor);
        gl.ColorC(text_color);

        TModelUnit xM = (inrc.GetFrom() + inrc.GetTo()) * 0.5;
        m_Context->TextOut(&m_Config->m_LabelFont, title.c_str(), xM, yy - 2, true);
    } // done with labels

    if (IsSelected()) {
        m_Context->DrawSelection(rcm);
    }
}


void CMatePairGlyph::x_UpdateBoundingBox()
{
    IRender& gl = GetGl();

    bool overlap = m_SeqAligns.size() == 2 &&
        m_SeqAligns[0]->GetRange().IntersectingWith(m_SeqAligns[1]->GetRange());

    TAlignList::iterator iter = m_SeqAligns.begin();
    while (iter != m_SeqAligns.end()) {
        (*iter)->Update(true);
        ++iter;
    }

    iter = m_SeqAligns.begin();
    TModelRange range((*iter)->GetLeft(), (*iter)->GetRight());
    ++iter;
    if (iter != m_SeqAligns.end()) {
        range.CombineWith(TModelRange((*iter)->GetLeft(), (*iter)->GetRight()));
    }
    SetLeft(range.GetFrom());
    SetWidth(range.GetLength());

    SetHeight(m_SeqAligns.front()->GetBarHeight());

    // in case two mate pair ends overlap, we put the second one
    // right under the first one.
    if (overlap) {
        SetHeight(2*GetHeight());
    }
    TModelRange inrc = m_Context->IntersectVisible(range);
    bool show_label = m_Config->m_ShowLabel  &&  m_Context->WillLabelFit(inrc);
    TModelUnit yy = 0;// GetTop();

    if (show_label) {
        auto label_height = gl.TextHeight(&(m_Config->m_LabelFont)) + 4;
        SetHeight(GetHeight() + label_height);
        yy += label_height;
    }

    // Adjust the coordinates of the alignments

    TModelUnit bar_h = m_SeqAligns.front()->GetBarHeight();
    // If two mates overlap, move the second one below the first one
    TModelUnit y_off = overlap ? bar_h : 0.0;
    TModelUnit yTop = yy;
    NON_CONST_ITERATE(TAlignList, iter, m_SeqAligns) {
        (*iter)->SetTop(yTop);
        yTop += y_off;
    }
}


void CMatePairGlyph::x_GetTitle(string* title, CLabel::ELabelType type) const
{
    *title = "Mate Pair: ";
    string tmp = "";

    ITERATE(TAlignList, iter, m_SeqAligns) {
        if (tmp.length() > 0) *title = *title + " / ";
        tmp.erase();
        (*iter)->GetTitle(&tmp, type);
        *title = *title + tmp;
    }
}

END_NCBI_SCOPE

