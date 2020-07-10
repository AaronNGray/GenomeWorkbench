/*  $Id: align_statistics_glyph.cpp 42972 2019-05-01 15:58:38Z shkeda $
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
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/align_statistics_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/widgets/seq_graphic/layout_conf.hpp>
#include "wx_aln_stat_dlg.hpp"

//#include <gui/widgets/wx/message_box.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static int kSmearRowHeight = 10;

static const string kScale         = "GraphScale";
static const string kPileUpLabel = "Pile-up";
static const string kPileUpColor = "0,0,255";


///////////////////////////////////////////////////////////////////////////////
///   CAlnStatGlyph

CAlnStatGlyph::CAlnStatGlyph(TSeqPos start, TModelUnit scale)
    : m_DlgHost(NULL)
    , m_StartPos(start)
    , m_ZoomScale(scale)
{
}


bool CAlnStatGlyph::OnLeftDblClick(const TModelPoint& /*p*/)
{
    CwxAlnStatOptionsDlg dlg(NULL);
    dlg.SetContent(m_Config->ShowAGTC());
    dlg.SetDisplay(m_Config->IsBarGraph());
    dlg.SetValueType(m_Config->ShowCount());
    dlg.SetGraphHeight(m_GraphParams->m_Height);
    dlg.SetZoomLevel(m_Config->m_StatZoomLevel);
    if (m_DlgHost) {
        m_DlgHost->PreDialogShow();
    }
    if(dlg.ShowModal() == wxID_OK) {
        m_Config->SetDisplayFlag(CAlnStatConfig::fBarGraph, dlg.IsBarGraph());
        m_Config->SetDisplayFlag(CAlnStatConfig::fShowMismatch, !dlg.IsAGTC());
        m_Config->SetDisplayFlag(CAlnStatConfig::fShowCount, dlg.IsCount());
        m_Config->m_StatZoomLevel = dlg.GetZoomLevel();
        m_GraphParams->m_Height = max(dlg.GetGraphHeight(), 10);
        x_OnLayoutChanged();
    }
    if (m_DlgHost) {
        m_DlgHost->PostDialogShow();
    }
    return true;
}


bool CAlnStatGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    GetTooltip(p, tt, t_title);
    return true;
}


void CAlnStatGlyph::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& /*t_title*/) const
{
    if (p.X() >= 0) {
        TSeqPos seq_pos = (TSeqPos)p.X();
        tt.AddSectionRow("Alignment statistics");
        
        tt.AddRow("Base position:", NStr::UIntToString(seq_pos + 1, NStr::fWithCommas));
        
        int idx = seq_pos - m_StartPos;
        if (m_Context->GetScale() > 1.0) {
            idx = (int)(idx / m_Context->GetScale());
        }

        char buf[255];
        if (idx >= 0  &&  idx < (int)m_StatVec.size()) {
            const SStatStruct& stat = m_StatVec[idx];
            int total = stat.m_Data[CAlnStatConfig::eStat_Total];
            
            tt.AddRow("Total count:", NStr::Int8ToString(total));

            int num = stat.m_Data[CAlnStatConfig::eStat_Match];
            string value = NStr::Int8ToString(num);
            if (total > 0) {
                sprintf(buf, "%2.1f", 100.0 * num / total);
                value += " (";
                value += buf;
                value += "%)";
            }
            tt.AddRow("Matches:", value);

            num = stat.m_Data[CAlnStatConfig::eStat_Mismatch];
            value = NStr::Int8ToString(num);
            if (total > 0) {
                sprintf(buf, "%2.1f", 100.0 * num / total);
                value += " (";
                value += buf;
                value += "%)";
            }
            tt.AddRow("Mismatches:", value);

            num = stat.m_Data[CAlnStatConfig::eStat_Gap];
            value = NStr::Int8ToString(num);
            if (total > 0) {
                sprintf(buf, "%2.1f", 100.0 * num / total);
                value += " (";
                value += buf;
                value += "%)";
            }
            tt.AddRow("Gaps:", value);

            num = stat.m_Data[CAlnStatConfig::eStat_Intron];
            if (num > 0) {
                value = NStr::Int8ToString(num);
                if (total > 0) {
                    sprintf(buf, "%2.1f", 100.0 * num / total);
                    value += " (";
                    value += buf;
                    value += "%)";
                }
                tt.AddRow("Introns:", value);
            }
            
            num = stat.m_Data[CAlnStatConfig::eStat_A];
            value = NStr::Int8ToString(num);
            if (total > 0) {
                sprintf(buf, "%2.1f", 100.0 * num / total);
                value += " (";
                value += buf;
                value += "%)";
            }
            tt.AddRow("A:", value);
            
            num = stat.m_Data[CAlnStatConfig::eStat_T];
            value = NStr::Int8ToString(num);
            if (total > 0) {
                sprintf(buf, "%2.1f", 100.0 * num / total);
                value += " (";
                value += buf;
                value += "%)";
            }
            tt.AddRow("T:", value);
            
            num = stat.m_Data[CAlnStatConfig::eStat_G];
            value = NStr::Int8ToString(num);
            if (total > 0) {
                sprintf(buf, "%2.1f", 100.0 * num / total);
                value += " (";
                value += buf;
                value += "%)";
            }
            tt.AddRow("G:", value);

            num = stat.m_Data[CAlnStatConfig::eStat_C];
            value = NStr::Int8ToString(num);
            if (total > 0) {
                sprintf(buf, "%2.1f", 100.0 * num / total);
                value += " (";
                value += buf;
                value += "%)";
            }
            tt.AddRow("C:", value);
        }
        tt.AddRow();
        tt.AddRow("Double click on the statistics graph to change settings...");
    }
}

static void s_GetLabel(CHistParams::EScale scale, string& label)
{
    label  = kPileUpLabel;
    if (scale != CHistParams::eLinear) {
        label += ", ";
        label += CHistParams::ScaleValueToName(scale);
        label += " scaled";
    }
}

void CAlnStatGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{

    if (m_Config->IsBarGraph()) { 
        string label = kPileUpLabel;
        if (m_Config->ShowCount())
            s_GetLabel(m_GraphParams->m_Scale, label);

        CHTMLActiveArea area;
        CSeqGlyph::x_InitHTMLActiveArea(area);
        area.m_PositiveStrand = true;
        area.m_Flags =
            CHTMLActiveArea::fNoSelection |
            CHTMLActiveArea::fNoTooltip |
            CHTMLActiveArea::fNoHighlight |
            CHTMLActiveArea::fZoomOnDblClick;
        area.m_Descr = label;

        Uint8 total = 0;
        for (auto&& v : m_StatVec) {
            total ^= v.m_Data[CAlnStatConfig::eStat_Total];
        }
        string id = "stat_" + NStr::NumericToString(total);
        area.m_Signature = id;
        CRef<CTrackConfig> config(new CTrackConfig);
        // mandatory options
        config->SetName("Alignment statistics");
        config->SetOrder(0);
        config->SetKey(id);
        config->SetShown(true);
        // Actual settings
        config->SetChoice_list().push_back(CHistParams::CreateScaleOptions(kScale, m_GraphParams->m_Scale));
        config->SetChoice_list().push_back(CreateDisplayOptions("StatDisplay", m_Config->m_Display));
        area.m_Config = config;
        
        p_areas->push_back(area);
        
        {
            TModelUnit top = 0;
            {
                TModelUnit left = 0;
                x_Local2World(left, top);
            }
            CGlTextureFont font(CGlTextureFont::eFontFace_Courier, 11);
            IRender& gl = GetGl();

            TModelRange vis_r = m_Context->IntersectVisible(this);
            int view_width = m_Context->GetViewWidth();
            if (view_width == 0)
                view_width = m_Context->SeqToScreen(vis_r.GetLength());
            auto right = view_width;
            right -= (gl.TextWidth(&font, label.c_str()) + 4);

            CHTMLActiveArea label_area;
            
            CSeqGlyph::x_InitHTMLActiveArea(label_area);
            label_area.m_Bounds.SetTop(top);
            label_area.m_Bounds.SetBottom(top + gl.TextHeight(&font) + 2);
            label_area.m_Bounds.SetLeft(-1);
            label_area.m_Bounds.SetRight(right);
            label_area.m_Flags = CHTMLActiveArea::fComment
                | CHTMLActiveArea::fNoSelection 
                | CHTMLActiveArea::fNoTooltip 
                | CHTMLActiveArea::fNoPin;
            label_area.m_Color = kPileUpColor + ",1"; // +alpha
            label_area.m_ID = label;

            // required, but nonsense fields
            label_area.m_PositiveStrand = true;
            p_areas->emplace_back(label_area);
        }

    } else {

        TModelUnit left = 0;
        TModelUnit top = 0;
        x_Local2World(left, top);
        //        top -= 3.0;

        vector<int> rows = x_GetShowList();
        vector<int>::reverse_iterator iter = rows.rbegin();
        for (; iter != rows.rend(); ++iter) {
            CHTMLActiveArea area;
            area.m_Bounds.SetTop(top);
            top += kSmearRowHeight;
            area.m_Bounds.SetBottom(top);
            area.m_Bounds.SetLeft(-1);
            area.m_Bounds.SetRight(0);
            area.m_Flags = CHTMLActiveArea::fComment
                | CHTMLActiveArea::fDrawBackground
                | CHTMLActiveArea::fNoHighlight
                | CHTMLActiveArea::fNoSelection
                | CHTMLActiveArea::fNoTooltip;
            
            area.m_ID = CAlnStatConfig::GetStatLabel(*iter);
            if ( !m_Config->ShowCount()  &&  *iter != CAlnStatConfig::eStat_Total) {
                area.m_ID += "(%)";
            }

            // required, but nonsense fields
            area.m_PositiveStrand = true;
            area.m_SeqRange.SetFrom(0);
            area.m_SeqRange.SetTo(0);
            
            p_areas->push_back(area);
        }
    }
}


bool CAlnStatGlyph::IsClickable() const
{
    return true;
}


void CAlnStatGlyph::x_Draw() const
{
    if (m_Config->IsBarGraph()) {
        x_DrawBarGraph();
    } else {
        x_DrawDensityTable();
    }
}


void CAlnStatGlyph::x_UpdateBoundingBox()
{
    SetLeft(m_Context->GetVisibleFrom());
    SetWidth(m_Context->GetVisibleRange().GetLength());
    if (m_Config->IsBarGraph()) {
        SetHeight(m_GraphParams->m_Height);
    } else {
        int row = (int)(x_GetShowList().size());
        SetHeight(kSmearRowHeight * row);
    }
}

static
double s_log_value(double value, CHistParams::EScale scale)
{
    
    switch (scale) {
    case CHistParams::eLog10:
        value = log10(value);
        break;
    case CHistParams::eLog2:
        value = log(value)/log(2.);
        break;
    case CHistParams::eLoge:
        value = log(value);
        break;
    default:
        break;
    }
    return value;

}

//#define ALNSTAT_USE_LOG_SCALE 
void CAlnStatGlyph::x_DrawBarGraph() const
{
    IRender& gl = GetGl();

    TModelUnit bottom = GetBottom();
    TModelUnit top = GetTop();
    TModelUnit left = GetLeft();
    TModelUnit right = GetRight();
    TModelUnit bar_h = m_GraphParams->m_Height;
    double max_t = x_GetMaxTotal();
    double max_value = max_t;
    bool show_count = m_Config->ShowCount();

    TModelUnit h_factor = bar_h;
    if (show_count) {
        max_t = max(1., s_log_value(max_t, m_GraphParams->m_Scale));
        //max_t = ceil(max_t);
        h_factor /= max_t;
    }

    vector<int> show_list = x_GetShowList();
    TModelUnit x = (TModelUnit)m_StartPos;

    // When the zoom level is greater than 1.0, the statistics data are computed
    // based on screen pixel (one set of data per pixel), not the nucleotide base.
    // So we need to use the correct offset (original scale when data are calculated),
    // not the the current zoom scale (m_Context->GetScale()).
    // When zoom level is less than or equal to 1.0, the statistics data are
    // computed based on nucleotide base (on set of data per nucleotide base).
    // So in that case, the offset will always be 1.
    TModelUnit off = m_ZoomScale > 1.0 ? m_ZoomScale : 1.0;
    ITERATE (TStatVec, iter, m_StatVec) {
        x += off;
        double total = iter->m_Data[CAlnStatConfig::eStat_Total];
        if (total == 0) continue;
        double total_factor = 1.0;
        double log_total = 1.;
        if ( m_GraphParams->m_Scale == CHistParams::eLinear) {
            if (!show_count) 
                total_factor /= total;
        } else {
            if (show_count) {
                if (total <=1) {
                    log_total = 0.5;
                } else {
                    log_total = max(1., s_log_value(total, m_GraphParams->m_Scale));
                }
            }
            total_factor /= total;
            total_factor *= log_total;
        }
        TModelUnit y = bottom;
        double curr_total= 0;
        // in the statistics data 
        // total != (matches + mismatches + intron + gaps)
        // but total = (matches + errors)
        // where errors is max of mismatches, intron or  gaps
        // however, we want to preserve the rendering order as per show_list
        // i.e. matches, mismatches, gaps, introns
        // So we render in that order until we reach max values for the total rows
        ITERATE (vector<int>, v_iter, show_list) {
            double val = iter->m_Data[*v_iter];
            if (val == 0) continue;
            if (curr_total + val > total)
                val = total - curr_total;
            curr_total += val;
            val *= total_factor;
            TModelUnit height = val * h_factor;
            gl.ColorC(m_Config->m_Colors[*v_iter]);
            m_Context->DrawQuad(x - off, y, x, y - height);
            y -= height;
            if (curr_total >= total)
                break;
        }
        //if (m_Context->WillSeqLetterFit()) {
        //    // show border
        //    gl.Color4f(0.2f, 0.2f, 0.2f, 0.8f);
        //    m_Context->DrawLine(x - 1.0, bottom, x - 1.0, y);
        //    m_Context->DrawLine(x, bottom, x, y);
        //}
    }

    // draw background scales.
    // we do this only when we show the count for each individual statistics,
    // not when we show the percentage since it will be 100% always.
    if (show_count) {
        double tick_dist = 30.0;
        int tick_n = (int)(floor(bar_h / tick_dist));
        tick_dist = bar_h / tick_n;

        CGlAttrGuard AttrGuard(GL_LINE_BIT);
        gl.Disable(GL_LINE_SMOOTH);
        gl.LineWidth(1.0f);
        gl.ColorC(m_GraphParams->m_RulerColor);
        
        //CRgbaColor line_color(0.7f, 0.7f, 0.7f, 0.4f);
        //gl.ColorC(line_color);
        m_Context->DrawLine(left, bottom, right, bottom);
        if (max_t == 0) return;

        for (int i = 0; i < tick_n; ++i) {
            m_Context->DrawLine(left, bottom - tick_dist * (i + 1), right, bottom - tick_dist * (i + 1));
        }

        int ruler_n = 6;
        TModelUnit dist_x = GetWidth() / ruler_n;
        TModelUnit min_dist = m_Context->ScreenToSeq(400.0);
        if (dist_x < min_dist) {
            dist_x = min_dist;
            ruler_n = (int)(GetWidth() / dist_x);
        }
        TModelUnit label_off = m_Context->ScreenToSeq(2.0);
        CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica_Bold, 10);
        TModelUnit x = left + dist_x * 0.1;
        string max_str = NStr::NumericToString(max_value);
        for (;  x < right;  x += dist_x) {
            gl.ColorC(m_GraphParams->m_RulerColor);
            m_Context->DrawLine(x, top, x, bottom);
            gl.ColorC(m_GraphParams->m_LabelColor);
            m_Context->TextOut(&font, max_str.c_str(), x + label_off, top + gl.TextHeight(&font) + 1, false);
        }
    }

    if (!m_CgiMode) {
        string label = kPileUpLabel;
        if (show_count)
            s_GetLabel(m_GraphParams->m_Scale, label);

        auto pane = m_Context->GetGlPane();
        CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica, 11);
        auto w = gl.TextWidth(&font, label.c_str());
        auto h = gl.TextHeight(&font);
        const TModelRect& vr = pane->GetVisibleRect();
        auto one_px = m_Context->ScreenToSeq(1);
        auto x = vr.Right() - ((w + 16) * one_px);
        auto y = vr.Top() + h + 2;
        CRgbaColor c(kPileUpColor);
        gl.ColorC(c);
        m_Context->TextOut(&font, label.c_str(), x, y, false);
    }
    /*    if we need a boundary 
    {
        const auto& pane = m_Context->GetGlPane();
        TModelUnit y = 0, l = 0;
        x_Local2World(l, y);
        {
            CGlPaneGuard GUARD(*pane, CGlPane::eOrtho);
            y = pane->UnProjectY(y);
        }
        CGlPaneGuard GUARD(*pane, CGlPane::ePixels);
        gl.ColorC(m_GraphParams->m_RulerColor);
        const auto& vp = pane->GetViewport();
        m_Context->DrawLine(vp.Left(), y, vp.Right(), y);
    }
    */

}


void CAlnStatGlyph::x_DrawDensityTable() const
{
    IRender& gl = GetGl();

    TModelUnit bottom = GetBottom();
    TModelUnit top = GetTop();
    TModelUnit left = GetLeft();
    TModelUnit right = GetRight();
    
    // draw boundary lines
    {
        CGlAttrGuard AttrGuard(GL_LINE_BIT);
        gl.Disable(GL_LINE_SMOOTH);
        gl.LineWidth(1.0f);
        gl.Color4f(0.6f, 0.6f, 0.8f, 0.5f);
        m_Context->DrawLine(left, bottom, right, bottom);
        m_Context->DrawLine(left, top, right, top);
    }

    int max_t = x_GetMaxTotal();
    if (max_t == 0) return;

    TModelUnit d_factor = 1.0;
    bool show_count = m_Config->ShowCount();
    if (show_count) {
        d_factor /= max_t;
    }

    CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica_Bold, 8);

    vector<int> show_list = x_GetShowList();
    TModelUnit x = (TModelUnit)m_StartPos;

    // See comment in CAlnStatGlyph::x_DrawBarGraph() on why offsets are different
    // for different zoom scales.
    TModelUnit off = m_ZoomScale > 1.0 ? m_ZoomScale : 1.0;

    ITERATE (TStatVec, iter, m_StatVec) {
        x += off;
        int total = iter->m_Data[CAlnStatConfig::eStat_Total];
        if (total == 0) continue;

        double total_factor = 1.0;
        if ( !show_count ) {
            total_factor /= total;
        }
        TModelUnit y = bottom;
        ITERATE (vector<int>, v_iter, show_list) {
            y -= kSmearRowHeight;
            if (iter->m_Data[*v_iter] == 0) continue;

            TModelUnit num = iter->m_Data[*v_iter];
            TModelUnit c_factor = num;
            if (*v_iter == CAlnStatConfig::eStat_Total) {
                c_factor /= max_t;
            } else {
                c_factor *= total_factor * d_factor;
                num *= total_factor;
                if ( !show_count ) num *= 100;
            }
            CRgbaColor color_max = m_Config->m_Colors[*v_iter];
            CRgbaColor color_min = color_max;
            color_min.SetAlpha(0.3f);
            CRgbaColor color(CRgbaColor::Interpolate(color_max, color_min, c_factor));
            gl.ColorC(color);
            m_Context->DrawQuad(x - off, y + kSmearRowHeight, x, y);

            // show count
            if (m_Context->GetScale() <= 1.0f / 16.0) {
                string out_text;
                if (num > 1000.0) {
                    int num_digit = 0;
                    while (num > 10.0) {
                        ++num_digit;
                        num *= 0.1;
                    }
                    out_text = NStr::IntToString((int)num) + "e" + NStr::IntToString(num_digit);
                } else if (num < 1.0) {
                    out_text = "<1";
                } else {
                    out_text = NStr::IntToString((int)num);
                }
                color = color.ContrastingColor();
                gl.ColorC(color);
                m_Context->TextOut(&font, out_text.c_str(),
                    x - 0.5, y + (kSmearRowHeight + gl.TextHeight(&font)) * 0.5, true);

            }
        }
    }


    if (m_Config->ShowLabel()) {
        // draw labels (left-most)
        vector<string> labels;
        TModelUnit max_label_w = 0.0;
        ITERATE (vector<int>, v_iter, show_list) {
            string label = CAlnStatConfig::GetStatLabel(*v_iter);
            if ( !show_count  &&  *v_iter != CAlnStatConfig::eStat_Total) {
                label += "(%)";
            }
            labels.push_back(label);
            max_label_w = max(max_label_w, gl.TextWidth(&font, label.c_str()));
        }

        x = left;
        TModelUnit y = bottom;

        gl.Color4f(0.8f, 1.0f, 0.8f, 1.0f);
        max_label_w += 4.0;
        max_label_w = m_Context->ScreenToSeq(max_label_w);
        m_Context->DrawQuad(x, y, x + max_label_w, top);

        gl.Color3f(0.0f, 0.0f, 0.0f);
        ITERATE (vector<string>, l_iter, labels) {
            const string& label = *l_iter;
            TModelUnit label_w = gl.TextWidth(&font, label.c_str()) + 2.0;
            label_w = m_Context->ScreenToSeq(label_w);
            m_Context->TextOut(&font, label.c_str(),
                               x + max_label_w - label_w, y - (kSmearRowHeight - gl.TextHeight(&font)) * 0.5, false);
            y -= kSmearRowHeight;
        }
    }
}


int CAlnStatGlyph::x_GetMaxTotal() const
{
    int max_t = 0;
    ITERATE (TStatVec, iter, m_StatVec) {
        max_t = max(max_t, iter->m_Data[CAlnStatConfig::eStat_Total]);
    }
    return max_t;
}


vector<int> CAlnStatGlyph::x_GetShowList() const
{
    vector<int> show_list;
    if ( !m_Config->IsBarGraph()  &&  m_Config->ShowTotal() ) {
        show_list.push_back(CAlnStatConfig::eStat_Total);
    }

    if (m_Config->ShowAGTC()) {
        show_list.push_back(CAlnStatConfig::eStat_A);
        show_list.push_back(CAlnStatConfig::eStat_T);
        show_list.push_back(CAlnStatConfig::eStat_G);
        show_list.push_back(CAlnStatConfig::eStat_C);
    } else {
        show_list.push_back(CAlnStatConfig::eStat_Match);
        show_list.push_back(CAlnStatConfig::eStat_Mismatch);
        if (m_Config->m_HasAmbigBases)
            show_list.push_back(CAlnStatConfig::eStat_Ambig);
    }
    show_list.push_back(CAlnStatConfig::eStat_Gap);
    if (m_ShowIntrons) {
        show_list.push_back(CAlnStatConfig::eStat_Intron);
    }

    return show_list;
}

CRef<CChoice> CAlnStatGlyph::CreateDisplayOptions(const string& option_name, int display_flag)
{
    auto choice = CTrackConfigUtils::CreateChoice
        (option_name, "Pileup Display", NStr::IntToString(display_flag),
         "Alignment Pileup Display");

    /// For bam/csra track, show alignment statistics always.
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   ("15",
                    "Match/Mismatch graph (count)",
                    "Show matches, mismatches, and gaps count as bar graph",
                    ""));
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   ("13",
                    "Match/Mismatch graph (percentage)",
                    "Show matches, mismatches, and gaps percentage as bar graph",
                    ""));
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   ("11",
                    "ATGC graph (count)",
                    "Show A, T, G, C, and gaps count as bar graph",
                    ""));
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   ("9",
                    "ATGC graph (percentage)",
                    "Show A, T, G, C, and gaps percentage as bar graph",
                    ""));
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   ("14",
                    "Match/Mismatch table (count)",
                    "Show matches, mismatches, and gaps count as table",
                    ""));
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   ("12",
                    "Match/Mismatch table (percentage)",
                    "Show matches, mismatches, and gaps percentage as table",
                    ""));
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   ("10",
                    "ATGC table (count)",
                    "Show A, T, G, C, and gaps count as table",
                    ""));
    choice->SetValues()
        .push_back(CTrackConfigUtils::CreateChoiceItem
                   ("8",
                    "ATGC table (percentage)",
                    "Show A, T, G, C, and gaps percentage as table",
                    ""));
    return choice;

}
END_NCBI_SCOPE
