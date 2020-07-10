/*  $Id: seq_glyph.cpp 44204 2019-11-14 20:16:52Z filippov $
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
 * Authors:  Mike Dicuccio, Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/object_based_glyph.hpp>
#include <corelib/ncbiutil.hpp>
#include <wx/event.h>

BEGIN_NCBI_SCOPE

using objects::CSeq_loc;
using objects::CSeq_loc_CI;

static const TModelUnit kUndefined = -1.0;

CSeqGlyph::CSeqGlyph()
    : m_Parent(NULL)
    , m_Context(NULL)
    , m_Neighbours(eNghbrs_None)
    , m_Height(kUndefined)
    , m_Width(kUndefined)
    , m_Pos(kUndefined, kUndefined)
    , m_Selected(false)
    , m_Highlighted(false)
    , m_Tearline(0)
    , m_RowNumber(0)
    , m_Visible(true)
    , m_Level(-1)
//    , m_isDead(false)
{
}


bool CSeqGlyph::OnLeftDown(const TModelPoint& /*p*/)
{
    return false;
}


bool CSeqGlyph::OnLeftUp(const TModelPoint& /*p*/)
{
    return false;
}


bool CSeqGlyph::OnLeftDblClick(const TModelPoint& /*p*/)
{
    return false;
}

bool CSeqGlyph::OnContextMenu(wxContextMenuEvent& /*event*/)
{
    return false;
}

void CSeqGlyph::Update(bool /*layout_only*/)
{
    x_UpdateBoundingBox();
}


void CSeqGlyph::Draw() const
{
    if (m_Renderer) {
        m_Renderer->Draw(this);
    } else {
        x_Draw();
    }

    if (IsHighlighted()) {
        x_DrawHighlight();
    }
}


CRef<CSeqGlyph> CSeqGlyph::HitTest(const TModelPoint& p)
{
    CRef<CSeqGlyph> obj;
    if (IsIn(p)) {
        obj.Reset(this);
    }
    return obj;
}

bool CSeqGlyph::HitTestHor(TSeqPos x, const CObject *obj)
{
    return IsInHor(x);
}

bool CSeqGlyph::Intersects(const TModelRect& rect, TConstObjects& objs) const
{
    if (rect.Intersects(GetModelRect())) {
        objs.push_back(CConstRef<CSeqGlyph>(this));
        return true;
    }
    return false;
}


bool CSeqGlyph::NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& t_title) const
{
    return false;
}


void CSeqGlyph::GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& tt, string& /*t_title*/) const
{

}


bool CSeqGlyph::IsIn(TModelUnit x, TModelUnit y) const
{
    TModelUnit pix_size = m_Context->ScreenToSeq(1.0);
    return x > GetLeft() - pix_size  &&  x < GetRight() + pix_size &&
        y > GetTop() - 1.0  &&  y < GetBottom() + 1.0;
}
    
bool CSeqGlyph::IsInHor(TModelUnit x) const
{
    TModelUnit pix_size = m_Context->ScreenToSeq(1.0);
    return x > GetLeft() - pix_size  &&  x < GetRight() + pix_size;
}


void CSeqGlyph::SetHighlightsColor(const string &hl_color_str)
{
    if (hl_color_str.empty())
        return;
    try {
        m_HighlightsColor.reset(new CRgbaColor(hl_color_str));
    }
    catch (const CException &err)
    {
        LOG_POST(Error << "Invalid highlighting color specified, falling back to default. " << err);
    }
}


void CSeqGlyph::x_DrawHighlight() const
{
    if (!m_HighlightsColor.get())
        m_Context->DrawHighlight(GetModelRect());
    else
        m_Context->DrawHighlight(GetModelRect(), m_HighlightsColor.get());
}


void CSeqGlyph::x_DrawInnerLabelColumns(TModelUnit base, const string &label, const CRgbaColor &color, const CGlTextureFont& font, bool side_label_visible) const
{
    IRender& gl = GetGl();

    TModelRange visible_bar = m_Context->IntersectVisible(GetRange());

    // Genome Workbench displays one column of labels
    unsigned labelsCount(1);
    if ((m_Context->GetViewWidth() != 0) && (m_Context->GetViewWidth() != m_Context->GetGlPane()->GetViewport().Width())) {
        // SV displays three columns - one in the middle and one on the left and right parts of the image, which are not visible on the screen
        labelsCount = 3;
    }

    //if (side_label_visible || visible_bar.GetLength() < m_Context->GetMinLabelWidthPos())
    if (visible_bar.GetLength() < m_Context->GetMinLabelWidthPos())
        return;

    // label text that will be shown
    string sLabelTextOut(label);

    // widths are in screen units
    TModelUnit label_width = gl.TextWidth(&font, sLabelTextOut.c_str());
    TModelUnit max_width = min(m_Context->SeqToScreen(visible_bar.GetLength()), m_Context->GetMaxLabelWidth(font));
    if (label_width > max_width) {
        label_width = max_width;
        sLabelTextOut = font.Truncate(sLabelTextOut.c_str(), label_width);
    }

    TModelUnit font_height = gl.TextHeight(&font);
    TModelUnit label_base = base + (font_height * 0.5);

    gl.ColorC(color);

    TModelUnit part_size = m_Context->GetVisibleRange().GetLength() / labelsCount;
    TModelUnit label_x;
    for (unsigned i = 0; i < labelsCount; ++i) {
        label_x = m_Context->GetVisibleRange().GetFrom() + part_size / 2.0 + i*part_size;
        TModelRange label_range(label_x - label_width / 2.0, label_x + label_width / 2.0);
        TModelRange label_range2 = label_range.IntersectionWith(visible_bar);
        if (label_range != label_range2)
            continue;
        m_Context->TextOut(&font, sLabelTextOut.c_str(), label_x, label_base, true, true);
    }
}

void CSeqGlyph::x_DrawInnerLabels(TModelUnit base, const string &label, const CRgbaColor &color, const CGlTextureFont& font, bool side_label_visible, bool inside_only, vector<TModelRange> *labels_ranges, TSeqRange* interval, bool XOR_mode) const
{
    if (side_label_visible && !m_Context->GetViewWidth())
        return;

    IRender& gl = GetGl();

    TModelRange visible_bar = m_Context->IntersectVisible(GetRange());
    //TModelUnit view_width = m_Context->ScreenToSeq(m_Context->GetViewWidth() != 0 ? m_Context->GetViewWidth() : m_Context->GetGlPane()->GetViewport().Width());
    TModelUnit seg_width(m_Context->GetGlPane()->GetViewport().Width());
    if (m_Context->GetViewWidth() && (m_Context->GetViewWidth() != seg_width))
        seg_width /= 5.0;
    seg_width = m_Context->ScreenToSeq(seg_width);

    int labelsCount = round(visible_bar.GetLength() / seg_width);
    if (0 == labelsCount) {
        if (!inside_only) {
            return;
        }
        else {
            labelsCount = 1;
        }
    }

    if (visible_bar.GetLength() < m_Context->GetMinLabelWidthPos()) {
        return;
    }

    // label text that will be shown
    string sLabelTextOut(label);

    // widths are in screen units
    TModelUnit label_width = gl.TextWidth(&font, sLabelTextOut.c_str());
    TModelUnit max_width = m_Context->SeqToScreen(visible_bar.GetLength() / (double)labelsCount);
    if (label_width > max_width) {
        label_width = max_width;
        sLabelTextOut = font.Truncate(sLabelTextOut.c_str(), label_width);
    }
    TModelUnit half_label = m_Context->ScreenToSeq(label_width / 2.0);

    TModelUnit font_height = gl.TextHeight(&font);
    TModelUnit label_base = base + (font_height * 0.5);

    if (XOR_mode) {
        glAlphaFunc(GL_GREATER, 0.2f); // Reject fragments with alpha < 0.2
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_XOR);
    }

    gl.ColorC(color);

    if (interval) {
        TModelUnit label_x;
        string sTruncatedLabel(sLabelTextOut);
        int labelsPerExon = round(interval->GetLength() / seg_width);
            if (!labelsPerExon) {
                // Small exon (draw single label)
                label_x = interval->GetFrom() + interval->GetLength() / 2.0;
                x_DrawTruncatedLabel(label, font, interval->GetLength(), label_x, label_base, labels_ranges);
            }
            else {
                // Wider exon
                TModelUnit part_size = interval->GetLength() / labelsPerExon;
                for (int i = 0; i < labelsPerExon; ++i) {
                    label_x = interval->GetFrom() + part_size / 2.0 + i*part_size;
                    x_DrawTruncatedLabel(label, font, part_size, label_x, label_base, labels_ranges);
                }
        }
    }
    else {
        TModelUnit part_size = visible_bar.GetLength() / labelsCount;
        TModelUnit label_x;
        for (int i = 0; i < labelsCount; ++i) {
            label_x = visible_bar.GetFrom() + part_size / 2.0 + i*part_size;
            m_Context->TextOut(&font, sLabelTextOut.c_str(), label_x, label_base, true, true);
            if (labels_ranges)
                labels_ranges->push_back(TModelRange(label_x - half_label, label_x + half_label));
        }
    }

    if (XOR_mode) {
        glDisable(GL_COLOR_LOGIC_OP);
        glDisable(GL_ALPHA_TEST);
    }
}

void CSeqGlyph::x_DrawTruncatedLabel(const string &label, const CGlTextureFont& font, TModelUnit width, TModelUnit x, TModelUnit y, vector<TModelRange> *labels_ranges) const
{
    if (width < m_Context->GetMinLabelWidthPos())
        return;

    string label_text(label);

    IRender& gl = GetGl();
    TModelUnit available_width = m_Context->SeqToScreen(width);
    TModelUnit label_width = gl.TextWidth(&font, label.c_str());
    if (label_width > available_width) {
        label_width = available_width;
        label_text = font.Truncate(label.c_str(), label_width);
        // Minimum length of truncated labels.
        // Truncated labels end in '...', so we want at least 4 meaningful characters
        const size_t min_length = 7;
        if (label_text.size() < min_length)
            return;
    }

    TModelUnit half_label = m_Context->ScreenToSeq(label_width / 2.0);
    if (labels_ranges)
        labels_ranges->push_back(TModelRange(x - half_label, x + half_label));

    m_Context->TextOut(&font, label_text.c_str(), x, y, true, true);
}

void CSeqGlyph::x_OnLayoutChanged()
{
    x_UpdateBoundingBox();
    if (m_Parent) {
        m_Parent->x_OnLayoutChanged();
    }
}


void CSeqGlyph::x_World2Local(TModelUnit& x, TModelUnit& y) const
{
    if (m_Parent) {
        m_Parent->x_World2Local(x, y);
        x_Parent2Local(x, y);
    }
}


void CSeqGlyph::x_Local2World(TModelUnit& x, TModelUnit& y) const
{
    y += GetTop();
    if (m_Parent) {
        m_Parent->x_Local2World(x, y);
    }
}


void CSeqGlyph::x_InitHTMLActiveArea(CHTMLActiveArea& area) const
{
    TVPUnit x1 = m_Context->SeqToScreenXClipped(GetLeft());
    TVPUnit x2 = m_Context->SeqToScreenXClipped(GetRight());
    if (m_Context->IsFlippedStrand()) {
        x1 = -x1;
        x2 = -x2;
    }

    TModelUnit t_x = 0;
    TModelUnit t_y = 0;
    x_Local2World(t_x, t_y);

    // Round y values to nearest integer and add 2 to the height
    // to give some space around the glyph
    TVPUnit y1 = TVPUnit(floor(t_y + 0.5));
    TVPUnit y2 = TVPUnit(floor(t_y + GetHeight() + 2.5));

    area.m_Bounds.Init(x1, y2, x2, y1);
    area.m_SeqRange = GetRange();
}


bool CSeqGlyph::x_Visible_V() const
{
    TModelUnit x = 0.0, t = 0.0, b = GetHeight();
    x_Local2World(x, t);
    x_Local2World(x, b);
    return !m_Context->IntersectVisible_Y(TModelRange(t, b)).Empty();
}

END_NCBI_SCOPE
