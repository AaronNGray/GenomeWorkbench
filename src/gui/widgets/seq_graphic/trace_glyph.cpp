/* 
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
* Authors:  Andrei Shkeda
*
* File Description:
*    CTraceGlyph -- utility class for 
*                      drawing Trace chromatograms 
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/trace_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include "wx_histogram_config_dlg.hpp"
#include <gui/widgets/wx/message_box.hpp>

#include <gui/opengl/glpoint.hpp>
#include <gui/widgets/gl/attrib_menu.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#define M_NCBI_E 2.71828182845904523536

const static int kGradColors = 32;


CRgbaColor s_GradientToWhite(const CRgbaColor& rgb, float degree)
{
    if (degree == 0.)
        return rgb;
    _ASSERT(degree >= 0. && degree <= kGradColors);
    float h, s, v;
    CRgbaColor::RgbToHsv(rgb, h, s, v);
    float k_s = s / kGradColors;
    s -= k_s * degree;
    float k_v =  (1. - v) / kGradColors;
    v += k_v * degree;
    return CRgbaColor::HsvToRgb(h, s, v);
}

//
// CTraceGlyph
//
CTraceGlyph::CTraceGlyph(const CRef<CSGTraceData>& data, const CRef<CTraceGraphConfig>& config)
    : m_DlgHost(NULL)
    , m_Config(config)
    , m_Data(data)
    , m_ShowTitle(false)
{

    bool b_ch = m_Data->GetSamplesCount() > 0;
    m_Config->m_SignalGraphState = b_ch ? 
        CTraceGraphConfig::eExpanded : CTraceGraphConfig::eHidden;
    m_Data->CalculateMax();
}


CTraceGlyph::~CTraceGlyph()
{
}

void CTraceGlyph::x_InitColors() const
{
    m_vSignalColors.resize(4 * kGradColors);
    for (int j = 0; j < kGradColors; j++) {
        float v = kGradColors - j;
        for (int ch = 0; ch < 4; ++ch) {
            m_vSignalColors[kGradColors * ch + j] = s_GradientToWhite(m_Config->m_colors[ch], v);
        }
    }
}
// vertical spacing between graph area border and graph
static const int kGraphOffsetY = 1;

static const int kCollapsedGraphH = 11;

static const double kConfGraphPrefHR = 0.4;
static const double kSignalGraphPrefHR = 0.6;


int CTraceGlyph::x_GetConfGraphH() const
{
    switch (m_Config->m_ConfGraphState) {
    case CTraceGraphConfig::eCollapsed: 
        return kCollapsedGraphH;
    case CTraceGraphConfig::eExpanded: 
        return floor(GetHeight() * kConfGraphPrefHR);
    case CTraceGraphConfig::eHidden: 
        return 0;
    }
    return 0;
}


int CTraceGlyph::x_GetSignalGraphH() const
{
    switch (m_Config->m_SignalGraphState) {
    case CTraceGraphConfig::eCollapsed: 
        return kCollapsedGraphH;
    case CTraceGraphConfig::eExpanded: 
        return floor(GetHeight() * kSignalGraphPrefHR);
    case CTraceGraphConfig::eHidden: 
        return 0;
    }
    return 0;
}



bool CTraceGlyph::OnLeftDblClick(const TModelPoint& /*p*/)
{
    return true;
}


bool CTraceGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    GetTooltip(p, tt, t_title);
    return true;
}

void CTraceGlyph::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& /*t_title*/) const
{
    tt.AddRow("DNA Sequencing Chromatograms");
}


void CTraceGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    _ASSERT(p_areas);
}


bool CTraceGlyph::IsClickable() const
{
    return false;
}


void CTraceGlyph::x_Draw() const
{
    IRender& gl = GetGl();
    CGlPane& pane = *m_Context->GetGlPane();

    _ASSERT(m_Context);

    TModelRect m_ModelRect = GetModelRect();

    //LOG_POST(Info << "    Viewport " << m_VPRect.ToString() << "\n    Visible " << m_ModelRect.ToString());

    CGlAttrGuard AttrGuard(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_HINT_BIT
                           | GL_LINE_SMOOTH | GL_POLYGON_MODE | GL_LINE_BIT);


    int conf_h = x_GetConfGraphH();
    int sig_h = (conf_h == 0) ? GetHeight() : x_GetSignalGraphH();

    //int top_y = int(m_ModelRect.Top() - pane.GetOffsetY());
    int top_y = m_ModelRect.Top();

    //pane.OpenOrtho();

    x_RenderContour(pane, top_y, sig_h, conf_h + sig_h);  // render background

    if (m_Config->m_SignalGraphState == CTraceGraphConfig::eExpanded) {
        //        if (pane.GetScaleX() < 1.0) { // render signal graph
            if (m_Config->m_SignalStyle == CTraceGraphConfig::eCurve) { // render curves
                gl.Enable(GL_BLEND);
                gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                gl.Enable(GL_LINE_SMOOTH);
                glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
                gl.LineWidth(0.5);

                x_RenderSignalGraph(pane, top_y, sig_h);
            } else {            // render intensity bands
                TVPRect rcVP = pane.GetViewport();
                pane.Close();
                TVPRect flipped_rect(rcVP.Left(), rcVP.Top(), rcVP.Right(), rcVP.Bottom());
                pane.SetViewport(flipped_rect);
                pane.OpenPixels();

                gl.Disable(GL_BLEND);
                gl.Disable(GL_LINE_SMOOTH);
                gl.LineWidth(1.0);
                x_RenderIntensityGraphs(pane, top_y, sig_h);

                pane.Close();
                pane.SetViewport(rcVP);
                pane.OpenOrtho();
            }
            //        }
        top_y += sig_h;
    }

    if (m_Config->m_ConfGraphState == CTraceGraphConfig::eExpanded) {
        // render confidence graph
        x_RenderConfGraph(pane, top_y, conf_h);
    }
    //pane.Close();


}

void CTraceGlyph::x_UpdateBoundingBox()
{
    SetHeight(m_Config->m_Height);
    const TModelRange& vr = m_Context->GetVisibleRange();
    SetLeft(vr.GetFrom());
    SetWidth(vr.GetLength() - 1.0);
}


void CTraceGlyph::x_RenderContour(CGlPane& pane, int y, int top_h, int total_h) const
{
    IRender& gl = GetGl();

    gl.Disable(GL_BLEND);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.Color3d(0.9, 0.9, 0.9);

    TModelUnit offset_x = pane.GetOffsetX();
    TModelUnit y1 = y;
    TModelUnit y2 = y + top_h - 1;
    TModelUnit y3 = y + top_h + 1;
    TModelUnit y4 = y + total_h - 1;
    TModelRect r = GetModelRect();
    TModelUnit x1 = r.Left() - offset_x;
    TModelUnit x2 = r.Right() - offset_x;
    gl.Rectd(x1, y1, x2, y2);
    gl.Rectd(x1, y3, x2, y4);

}


// renders confidence graph
void CTraceGlyph::x_RenderConfGraph(CGlPane& pane, int y, int h) const
{
    const TVPRect& rc_vp = pane.GetViewport();
    const TModelRect rc_vis = pane.GetVisibleRect();

    IRender& gl = GetGl();

    gl.Disable(GL_BLEND);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    TModelUnit amp = 0.0;
    // guard against the situation where confidence data is not available
    // or fails to load
    if (m_Data->GetMaxConfidence() != 0.0) {
        amp = ((TModelUnit)h - 2 * kGraphOffsetY - 1)
            / m_Data->GetMaxConfidence();
    }

    TModelUnit base_y = y + kGraphOffsetY;
    TModelUnit scale_x = abs(pane.GetScaleX());

    TSignedSeqPos gr_from = m_Data->GetSeqFrom();
    TSignedSeqPos gr_to = m_Data->GetSeqTo();

    bool b_average = (scale_x > 0.5);
    b_average = false; // doesn't work yet
    
    if (b_average) {
        TVPRect rcVP = pane.GetViewport();
        pane.Close();
        TVPRect flipped_rect(rcVP.Left(), rc_vis.Bottom(), rcVP.Right(), rc_vis.Top());
        pane.SetViewport(flipped_rect);
        pane.OpenPixels();
        TModelUnit  left = floor(rc_vis.Left());

        //calculate visible range in seq coords
        TVPUnit range_pix_start = rc_vp.Left();
        TVPUnit range_pix_end = rc_vp.Right();

        TModelUnit  top_y = rc_vp.Bottom() + (y + kGraphOffsetY);

        TModelUnit pos_pix_left, pos_pix_right;

        TSignedSeqPos pos = gr_from, pos_inc = 1;
        double v = 0, v_min = 0, v_max = 0;

        TSignedSeqPos vis_from = (TSignedSeqPos)floor(rc_vis.Left());
        TSignedSeqPos vis_to = (TSignedSeqPos)ceil(rc_vis.Right());
        if (vis_from > vis_to) {
            _ASSERT(m_Context->IsFlippedStrand());
            swap(vis_from, vis_to);
        }

        // calculate intersection of the segment in seq_coords with graph range
        TSignedSeqPos from = max<TSignedSeqPos>(vis_from, gr_from);
        TSignedSeqPos to = min<TSignedSeqPos>(vis_to, gr_to);

        if (m_Context->IsFlippedStrand()) {
            left = pos = to;
            pos_inc = -1;
        } else {
            left = pos = from;
            pos_inc = 1;
        }


        gl.Begin(GL_LINES);
        for (int pix = range_pix_start; pix <= range_pix_end;) { // iterate by pixels

            if (pos < gr_from || pos > gr_to)
                break;

            bool b_first = true;
            if (m_Context->IsFlippedStrand()) {
                pos_pix_left = range_pix_start + (left - pos) / scale_x;
            } else {
                pos_pix_left = range_pix_start + (pos - left) / scale_x;
            }
            pos_pix_right = pos_pix_left + 1 / scale_x;

            pos_pix_left = max((TModelUnit)pix, pos_pix_left);

            _ASSERT(pos_pix_left >= pix);

            while (pos_pix_left < pix + 1) {
                if (pos < from || pos > to)
                    break;
                // calculate overlap with pixel and integrate
                v = amp * m_Data->GetConfidence(pos);
                if (b_first) {
                    v_min = v_max = v;
                    b_first = false;
                } else {
                    v_min = min(v_min, v);
                    v_max = max(v_max, v);
                }

                if (pos_pix_right >= pix + 1)
                    break;
                pos += pos_inc; // advance to next pos
                if (m_Context->IsFlippedStrand()) {
                    pos_pix_left = range_pix_start + (left - pos) / scale_x;
                } else {
                    pos_pix_left = range_pix_start + (pos - left) / scale_x;
                }
                pos_pix_right = pos_pix_left + 1 / scale_x;

            }

            gl.ColorC(m_Config->m_colorConfMin);
            gl.Vertex2d(pix, top_y);
            gl.Vertex2d(pix, top_y + v_min);

            gl.ColorC(m_Config->m_colorConfMax);
            gl.Vertex2d(pix, top_y + v_min);
            gl.Vertex2d(pix, top_y + v_max);

            pix++;
            v = v_min = v_max = 0;
        }
        gl.End();

        pane.Close();
        pane.SetViewport(rcVP);
        pane.OpenOrtho();
    } else { // render without averaging
        _ASSERT(pane.GetProjMode() == CGlPane::eOrtho);

        TModelUnit offset_x = pane.GetOffsetX();
        gl.ColorC(m_Config->m_colorConfMin); //###
        TSignedSeqPos from = floor(rc_vis.Left());
        TSignedSeqPos to = ceil(rc_vis.Right());
        if (from > to) {
            swap(from, to);
        }

        from = max<TSignedSeqPos>(gr_from, from);
        to = min<TSignedSeqPos>(gr_to, to);

        for (TSignedSeqPos pos = from; pos <= to; pos++) {
            double v = m_Data->GetConfidence(pos);
            v *= amp;
            TSignedSeqPos aln_pos = pos;
            double x = aln_pos - offset_x;
            gl.Rectd(x, base_y, x + 1.0, base_y + v);
        }
    }
}

void CTraceGlyph::x_RenderSignalGraph(CGlPane& pane, int y, int h) const
{
    IRender& gl = GetGl();

    CSGTraceData::TSignalValue MaxSignal = 0;
    for (int ch = CSGTraceData::eA; ch <= CSGTraceData::eG; ch++) {
        MaxSignal = max(MaxSignal, m_Data->GetMax((CSGTraceData::EChannel) ch));
    }
    TModelUnit amp = ((TModelUnit)(h - 2 * kGraphOffsetY)) / MaxSignal;
    int bottom_y = y + (h - 1) - kGraphOffsetY;

    for (int i_ch = 0; i_ch < 4; i_ch++) {   // for every channel (i_ch - channel index)
        int ch_index = (m_Config->m_bReverseColors  &&  m_Context->IsFlippedStrand()) ? (i_ch ^ 2) : i_ch;
        int ch = CSGTraceData::eA + ch_index;

        const CSGTraceData::TPositions& positions = m_Data->GetPositions();

        CSGTraceData::EChannel channel = (CSGTraceData::EChannel) ch;
        const CSGTraceData::TValues& values = m_Data->GetValues(channel);

        CRgbaColor c(m_Config->m_colors[ch]);
        c.SetAlpha(0.5f);
        gl.ColorC(c);
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        _ASSERT(m_Config->m_SignalStyle == CTraceGraphConfig::eCurve);

        x_RenderCurveSegment(pane, positions, values, bottom_y, h, (int) amp);

        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}


// Renders chromatogram corresponding to given segment of sequence.
// This function renders data as a curve for a single channel specified by "values".
void    CTraceGlyph::x_RenderCurveSegment(CGlPane& pane,
                                          const CSGTraceData::TPositions& positions,
                                          const CSGTraceData::TValues& values,
                                          int bottom_y, int /*h*/, int amp) const
{
    IRender& gl = GetGl();

    const TModelRect rc_vis = pane.GetVisibleRect();

    bool b_neg = m_Context->IsFlippedStrand();

    // [from, to] - is sequence range for which graph is rendered
    double from = m_Data->GetSeqFrom();
    double to = m_Data->GetSeqTo();

    double vis_from = rc_vis.Left();
    double vis_to = rc_vis.Right();
    if (vis_to < vis_from) {
        _ASSERT(b_neg);
        swap(vis_from, vis_to);
    }

    from = max(from, vis_from);
    to = min(to, vis_to);

    if (from <= to) {
        int sm_start = x_FindSampleToLeft(from);
        int sm_end = x_FindSampleToRight(to + 1);
        sm_start = max(0, sm_start);
        sm_end = min(sm_end, m_Data->GetSamplesCount() - 1);

        if (sm_start <= sm_end) {
            gl.Begin(GL_TRIANGLE_STRIP);
            TModelUnit offset_x = pane.GetOffsetX();

            // check if start interpolation is needed
            CSGTraceData::TFloatSeqPos sm_start_seqpos = positions[sm_start];
            if (sm_start_seqpos < from) {
                if (sm_start + 1 < sm_end) {
                    double v1 = values[sm_start];
                    double v2 = values[sm_start + 1];
                    double x1 = sm_start_seqpos;
                    double x2 = positions[sm_start + 1];
                    double v = v1 + ((from - x1) * (v2 - v1) / (x2 - x1));
                    v *= amp;

                    double aln_from = from + (b_neg ? -1 : 0);
                    gl.Vertex2d(aln_from - offset_x, bottom_y - v);
                    //gl.Vertex2d(aln_from - offset_x, bottom_y);
                }
                sm_start++;
            }
            // render regular samples
            for (int i_sm = sm_start; i_sm < sm_end; i_sm++) {
                TModelUnit seqpos = positions[i_sm];
                double v = values[i_sm];
                v *= amp;

                double aln_pos = seqpos + (b_neg ? -1 : 0);
                gl.Vertex2d(aln_pos - offset_x, bottom_y - v);
                //gl.Vertex2d(aln_pos - offset_x, bottom_y);
            }
            // render end point
            if (sm_end - 1 > sm_start) { // interpolate end point
                double v1 = values[sm_end - 1];
                double v2 = values[sm_end];
                double x1 = positions[sm_end - 1];
                double x2 = positions[sm_end];
                double v = v1 + ((to + 1 - x1) * (v2 - v1) / (x2 - x1));
                v *= amp;

                double aln_to = to + (b_neg ? 0 : 1);
                gl.Vertex2d(aln_to - offset_x, bottom_y - v);
                //gl.Vertex2d(aln_to - offset_x, bottom_y);
            } else {
                TModelUnit seqpos = positions[sm_end];
                double v = values[sm_end];
                v *= amp;

                double aln_pos = seqpos;
                gl.Vertex2d(aln_pos - offset_x, bottom_y - v);
                //gl.Vertex2d(aln_pos - offset_x, bottom_y);
            }
            gl.End();
        }
    }
}


/// Render signals for all channels as gradient-color bands with color intensity
/// proprotional to signal strength.
void    CTraceGlyph::x_RenderIntensityGraphs(CGlPane& pane, int y, int h) const
{
    IRender& gl = GetGl();

    //_TRACE("\nx_RenderIntensityGraphs");
    const CSGTraceData::TPositions& positions = m_Data->GetPositions();

    const TVPRect& rc_vp = pane.GetViewport();
    const TModelRect rc_vis = pane.GetVisibleRect();

    // calculate layout                
    int av_h = h - 2 * kGraphOffsetY; // height available for intensity bands
    int band_h = av_h / 4;
    int off = (av_h - 4 * band_h) / 2; // rounding error compensation
    int top_y = rc_vp.Bottom() + (y + kGraphOffsetY + off);

    TModelUnit  left = rc_vis.Left();
    TModelUnit  scale_x = abs(pane.GetScaleX());

    TVPUnit range_pix_start = rc_vp.Left();
    TVPUnit range_pix_end = rc_vp.Right();

    gl.Begin(GL_LINES);

    // calculate samples range by segment

    double vis_from = rc_vis.Left();
    double vis_to = rc_vis.Right();
    if (vis_to < vis_from) {
        _ASSERT(m_Context->IsFlippedStrand());
        swap(vis_from, vis_to);
    }

    // [from, to] - is sequence range for which graph is rendered
    double from = max<TVPUnit>(m_Data->GetSeqFrom(), floor(vis_from));
    double to = min<TVPUnit>(m_Data->GetSeqTo(), ceil(vis_to));

    // [sm_start, sm_end] - samples range being rendered
    int sm_start = x_FindSampleToLeft(from);
    int sm_end = x_FindSampleToRight(to + 1);
    sm_start = max(sm_start, 0);
    sm_end = min(sm_end, m_Data->GetSamplesCount() - 1);

    // calculate pixels range to render
    double aln_from = from;
    double aln_to = to;
    aln_to += 1;


    TVPUnit pix_start = range_pix_start + (aln_from - floor(vis_from)) / scale_x;;
    TVPUnit pix_end = range_pix_start + (TVPUnit)ceil((aln_to - floor(vis_from)) / scale_x);

    pix_start = max(pix_start, range_pix_start);
    pix_end = min(pix_end, range_pix_end);

    int sm_inc = m_Context->IsFlippedStrand() ? -1 : +1;

    int band_y = top_y;
    for (int i_ch = 0; i_ch < 4; i_ch++) {   // for every channel (i_ch - channel index)
        int ch_index = (m_Config->m_bReverseColors  &&
                        m_Context->IsFlippedStrand()) ? (i_ch ^ 2) : i_ch;
        int ch = CSGTraceData::eA + ch_index;

        const CSGTraceData::TValues& values = m_Data->GetValues((CSGTraceData::EChannel) ch);
        CSGTraceData::TSignalValue MaxSignal = m_Data->GetMax((CSGTraceData::EChannel) ch);

        double x1 = 0.0, x2 = 0.0, pix_x1 = 0.0, pix_x2 = 0.0;
        double v1, v2, s, dx, sum, sum_pix_x;

        int sample = m_Context->IsFlippedStrand() ? sm_end : sm_start;
        _ASSERT(sample >= 0);

        for (TVPUnit pix = pix_start; pix <= pix_end; pix++) { // for each pixel
            // calculate average value for "pix" pixel by integrating values
            // over the range [pix, pix+1]
            sum = 0; // integral from values by pix_x
            sum_pix_x = 0; // length of integrated range

            x1 = positions[sample];
            pix_x1 = range_pix_start;
            TModelUnit dist = m_Context->IsFlippedStrand() ? left - x1 : x1 - left;
            if (dist > 0)
                pix_x1 += dist / scale_x;

            v1 = v2 = values[sample]; //#####

            if (pix_x1 < pix + 1) {

                bool b_next_point = m_Context->IsFlippedStrand() ? (sample > sm_start) : (sample < sm_end);
                if (b_next_point) { // there is second point available
                    x2 = positions[sample + sm_inc];
                    pix_x2 = range_pix_start;
                    TModelUnit dist = m_Context->IsFlippedStrand() ? left - x2 : x2 - left;
                    if (dist > 0)
                        pix_x2 += dist / scale_x;

                    v2 = values[sample + sm_inc];

                    if (pix_x1 < pix) { // fisrt sample is to the left of this pixel
                        // replace it fake interpolated sample at x = "pix"
                        v1 += (v2 - v1) * (pix - pix_x1) / (pix_x2 - pix_x1);
                        pix_x1 = pix;
                    }
                }

                while (b_next_point  &&  pix_x2 <= pix + 1) // while second point is inside pixel
                {
                    dx = pix_x2 - pix_x1;
                    s = 0.5 * (v1 + v2) * dx;
                    _ASSERT(s >= 0 && dx >= 0);

                    sum += s;
                    sum_pix_x += dx;

                    sample += sm_inc; // advance, x2 becomes x1
                    pix_x1 = pix_x2;
                    v1 = v2;

                    b_next_point = m_Context->IsFlippedStrand() ? (sample > sm_start) : (sample < sm_end);
                    if (!b_next_point)
                        break;
                    x2 = positions[sample + sm_inc];
                    pix_x2 = range_pix_start;
                    TModelUnit dist = m_Context->IsFlippedStrand() ? left - x2 : x2 - left;
                    if (dist > 0)
                        pix_x2 += dist / scale_x;
                    v2 = values[sample + sm_inc];
                }
                _ASSERT(pix_x1 <= pix + 1);

                if (b_next_point  &&  pix_x2 > pix + 1) { // second point is outside pixel
                    dx = pix + 1 - pix_x1;
                    _ASSERT(dx >= 0);

                    double v = v1 + (v2 - v1) * dx / (pix_x2 - pix_x1);
                    s = 0.5 * (v1 + v) * dx;

                    _ASSERT(s >= 0 && dx >= 0);

                    sum += s;
                    sum_pix_x += dx;
                }
                double av_v = (sum_pix_x) > 0 ? sum / sum_pix_x : 0;

                // render pixel
                double norm = (MaxSignal == 0) ? 0 : (av_v / MaxSignal);
                const CRgbaColor& col = GetColorByValue(norm, i_ch);
                gl.ColorC(col);
                gl.Vertex2d(pix, band_y);
                gl.Vertex2d(pix, band_y + band_h - 1);
            } // if(pix < pix + 1)
        }
        band_y += band_h;
    }
    gl.End();
}


// returns gradient color corresponding to normalized [0, 1.0] value "value"
// for channel specified by "signal"
const CRgbaColor& CTraceGlyph::GetColorByValue(double value, int signal) const
{
    _ASSERT(value >= 0 && value <= 1.0);
    _ASSERT(signal >= 0 && signal <= 3);

    int i = (int)(value * kGradColors);
    i = min(i, kGradColors);
    int index = signal * kGradColors + i;
    if (m_vSignalColors.empty())
        x_InitColors();
    _ASSERT(index < m_vSignalColors.size());
    return m_vSignalColors[index];
}

/// returns index of rightmost sample having m_SeqPos less then "pos".
/// if "pos" is to the left of the trace range function returns -1,
/// if "pos" is to the right of the trace range functions returns "n_samples"
int CTraceGlyph::x_FindSampleToLeft(double pos) const
{
    int n_samples = m_Data->GetSamplesCount();
    if (pos < m_Data->GetSeqFrom() || n_samples == 0) {
        return -1;
    } else if (pos > m_Data->GetSeqTo()) {
        return n_samples;
    } else {
        const CSGTraceData::TPositions& positions = m_Data->GetPositions();
        double scale = ((double)n_samples) / m_Data->GetSeqLength();

        // calculate approximate sample index
        int i = (int)(scale * (pos - m_Data->GetSeqFrom()));
        i = min(i, n_samples - 1);
        i = max(i, 0);

        if (positions[i] > pos) {
            for (; i > 0 && positions[i] > pos; i--) {
            }
        } else {
            for (; ++i < n_samples && positions[i] < pos;) {
            }
            i--;
        }
        return i;
    }
}


/// returns index of the leftmost sample having m_SeqPos greater than "pos"
/// if "pos" is to the left of the trace range function returns -1,
/// if "pos" is to the right of the trace range functions returns "n_samples"
int CTraceGlyph::x_FindSampleToRight(double pos) const
{
    int n_samples = m_Data->GetSamplesCount();
    if (pos < m_Data->GetSeqFrom() || n_samples == 0) {
        return -1;
    } else if (pos > m_Data->GetSeqTo()) {
        return n_samples;
    } else {
        const CSGTraceData::TPositions& positions = m_Data->GetPositions();
        double scale = ((double)n_samples) / m_Data->GetSeqLength();

        // calculate approximate sample index
        int i = (int)(scale * (pos - m_Data->GetSeqFrom()));
        i = min(i, n_samples - 1);
        i = max(i, 0);

        if (positions[i] > pos) {
            for (; i > 0 && positions[i] > pos; i--) {
            }
            i++;
        } else {
            for (; ++i < n_samples && positions[i] < pos;) {
            }
        }
        return i;
    }
}




END_NCBI_SCOPE
