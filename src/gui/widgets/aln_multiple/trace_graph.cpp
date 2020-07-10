/*  $Id: trace_graph.cpp 41823 2018-10-17 17:34:58Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a 'United States Government Work' under the
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
#include <corelib/ncbistd.hpp>

#include <gui/widgets/aln_multiple/aln_vec_iterator.hpp>
#include <gui/widgets/aln_multiple/trace_graph.hpp>

#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/types.hpp>

#include <math.h>

#include <memory>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CTraceGraphProperties::CTraceGraphProperties()
:   m_SignalStyle(eCurve),
    m_ConfGraphState(eExpanded),
    m_SignalGraphState(eExpanded),
    m_bReverseColors(true)
{
}

CTraceGraphProperties&
    CTraceGraphProperties::operator=(const CTraceGraphProperties& orig)
{
    m_SignalStyle = orig.m_SignalStyle;
    m_ConfGraphState = orig.m_ConfGraphState;
    m_SignalGraphState = orig.m_SignalGraphState;
    return *this;
}


////////////////////////////////////////////////////////////////////////////////
/// CTraceGraph

const static int kGradColors = 32;

CTraceGraph::CTraceGraph(const CBioseq_Handle& handle, bool b_neg_strand)
:   m_DataProxy(handle, b_neg_strand),
    m_Data(NULL),
    m_Font(CGlTextureFont::eFontFace_Helvetica, 10),
    m_TextColor(0.6f, 0.6f, 0.6f)
{

}


CTraceGraph::~CTraceGraph()
{
    Destroy();
}


// vertical spacing between graph area border and graph
static const int kGraphOffsetY = 1;

static const int kConfGraphPrefH = 24;
static const int kSignalGraphPrefH = 40;
static const int kCollapsedGraphH = 11;


int CTraceGraph::x_GetConfGraphH() const
{
    switch(m_Props.m_ConfGraphState)    {
    case CTraceGraphProperties::eCollapsed: return kCollapsedGraphH;
    case CTraceGraphProperties::eExpanded: return kConfGraphPrefH;
    case CTraceGraphProperties::eHidden: return 0;
    }
    return 0;
}


int CTraceGraph::x_GetSignalGraphH() const
{
    switch(m_Props.m_SignalGraphState)    {
    case CTraceGraphProperties::eCollapsed: return kCollapsedGraphH;
    case CTraceGraphProperties::eExpanded: return kSignalGraphPrefH;
    case CTraceGraphProperties::eHidden: return 0;
    }
    return 0;
}


void CTraceGraph::Render(CGlPane& pane)
{
}


static const int kTextOff = 2;

TVPPoint CTraceGraph::PreferredSize()
{
    TVPUnit h = 0;
    if(m_Data) {
        h = x_GetConfGraphH();
        h += x_GetSignalGraphH();
        h += 2;
    } else {
        // Don't display meaningless text signs
        // h = (TVPUnit) m_Font.TextHeight() + 2 * kTextOff;
    }
    return TVPPoint(0, h);
}


bool CTraceGraph::NeedTooltip(CGlPane& /*pane*/, int vp_x, int vp_y)
{
    _VERIFY(m_VPRect.PtInRect(vp_x, vp_y));

    return false; //TODO
}


string CTraceGraph::GetTooltip()
{
    return "";
}


bool    CTraceGraph::IsCreated() const
{
    return (m_Data != NULL);
}


bool    CTraceGraph::Create()
{
    m_Data = m_DataProxy.LoadData();
    if(m_Data) {
        SetConfGraphState(CTraceGraphProperties::eExpanded);

        bool b_ch = m_Data->GetSamplesCount() > 0;
        SetSignalGraphState(b_ch    ? CTraceGraphProperties::eExpanded
                                    : CTraceGraphProperties::eHidden);

        m_vSignalColors.resize(4 * kGradColors);
        float k = 1.0f / kGradColors;

        for( int j = 0; j < kGradColors; j++ )  {
            float v = 1.0f - k * j;
            m_vSignalColors[j] = CRgbaColor(1.0f, v, v); //red
            m_vSignalColors[kGradColors + j] = CRgbaColor(v, 1.0f, v); //green
            m_vSignalColors[2 * kGradColors + j] = CRgbaColor(v, v, 1.0f); //blue
            m_vSignalColors[3 * kGradColors + j] = CRgbaColor(0.5f * (1 + v), v, 0.5f * (1 + v)); //purple
        }

        m_Data->CalculateMax();
        return true;
    } else return false;
}


void    CTraceGraph::Destroy()
{
    if(m_Data) {
        delete m_Data;
        m_Data = NULL;

        m_vSignalColors.clear();
    }
}


const IAlnRowGraphProperties* CTraceGraph::GetProperties() const
{
    return &m_Props;
}


void    CTraceGraph::SetProperties(IAlnRowGraphProperties* props)
{
    CTraceGraphProperties* trace_props =
        dynamic_cast<CTraceGraphProperties*>(props);
    if(trace_props) {
        m_Props = *trace_props;
    }
}


void    CTraceGraph::SetConfGraphState(EGraphState state)
{                                                                                                        
    m_Props.m_ConfGraphState = state;
}


void    CTraceGraph::SetSignalGraphState(EGraphState state)
{
    m_Props.m_SignalGraphState = state;
}


// renders both Confidence graph and Chromatograms
void    CTraceGraph::Render(CGlPane& pane, IAlnSegmentIterator& it)
{
    //LOG_POST(Info << "CTraceGraph::Render()  ");
    //LOG_POST(Info << "    Viewport " << m_VPRect.ToString() << "\n    Visible " << m_ModelRect.ToString());

    CGlAttrGuard AttrGuard(GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT | GL_HINT_BIT
                            | GL_LINE_SMOOTH | GL_POLYGON_MODE | GL_LINE_BIT);

    if( ! m_Data)   {
        // Don't display meaningless text signs
        // x_RenderText(pane);
        return;
    }

    IRender& gl = GetGl();

    int conf_h = x_GetConfGraphH();
    int sig_h = x_GetSignalGraphH();
    int top_y = int(m_ModelRect.Top() - pane.GetOffsetY());

    pane.OpenOrtho();

    auto_ptr<IAlnSegmentIterator> it_1(it.Clone());
    x_RenderContour(pane, top_y, sig_h, conf_h + sig_h, *it_1);  // render background

    if(m_Props.m_SignalGraphState == CTraceGraphProperties::eExpanded)    {
        if(pane.GetScaleX() < 1.0)   { // render signal graph
            if(m_Props.m_SignalStyle == CTraceGraphProperties::eCurve)   { // render curves
                gl.Enable(GL_BLEND);
                gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                gl.Enable(GL_LINE_SMOOTH);
                glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
                gl.LineWidth(0.5);

                auto_ptr<IAlnSegmentIterator> it_3(it.Clone());
                x_RenderSignalGraph(pane, top_y, sig_h, *it_3);
            } else {            // render intensity bands
                pane.Close();
                pane.OpenPixels();

                gl.Disable(GL_BLEND);
                gl.Disable(GL_LINE_SMOOTH);
                gl.LineWidth(1.0);

                auto_ptr<IAlnSegmentIterator> it_4(it.Clone());
                x_RenderIntensityGraphs(pane, top_y, sig_h, *it_4);

                pane.Close();
                pane.OpenOrtho();
            }
        }
        top_y += sig_h;
    }

    if(m_Props.m_ConfGraphState == CTraceGraphProperties::eExpanded)   {
        // render confidence graph
        auto_ptr<IAlnSegmentIterator> it_2(it.Clone());
        x_RenderConfGraph(pane, top_y, conf_h, *it_2);
    }

    pane.Close();
}


void CTraceGraph::x_RenderText(CGlPane& pane)
{
    pane.OpenPixels();

    IRender& gl = GetGl();

    TVPRect rc = m_VPRect;
    rc.Inflate(-kTextOff, -kTextOff);

    gl.ColorC(m_TextColor);
    m_Font.TextOut(rc.Left(), rc.Bottom(), rc.Right(), rc.Top(),
                   "Trace Graph - no data available", IGlFont::eAlign_Left);

    pane.Close();
}

// Functor for coordinate translation align <->seq
struct SChunkTranslator
{
public:
    SChunkTranslator()
    : m_SeqRange(NULL), m_AlnRange(NULL), m_bNegative(false)
    {
    }

    void Init(const TSignedSeqRange& seq_range,
              const TSignedSeqRange& aln_range, bool negative)
    {
        m_SeqRange = &seq_range;
        m_AlnRange = &aln_range;
        m_bNegative = negative;
        _ASSERT(m_SeqRange->GetLength() == m_AlnRange->GetLength());
    }
    inline TSignedSeqPos GetAlnPosFromSeqPos(TSignedSeqPos seq_pos)
    {
        _ASSERT(m_SeqRange  && m_AlnRange);

        TSignedSeqPos seq_off = seq_pos - m_SeqRange->GetFrom();
        if(m_bNegative)    {
          return m_AlnRange->GetTo() - seq_off;
        } else return seq_off + m_AlnRange->GetFrom();
    }
    inline double GetAlnPosFromSeqPos(double seq_pos)
    {
        _ASSERT(m_SeqRange  && m_AlnRange);

        double seq_off = seq_pos - m_SeqRange->GetFrom();
        if(m_bNegative)    {
          return m_AlnRange->GetTo() - seq_off;
        } else return seq_off + m_AlnRange->GetFrom();
    }
    inline TSignedSeqPos GetSeqPosFromAlnPos(TSignedSeqPos aln_pos)
    {
        _ASSERT(m_SeqRange  && m_AlnRange);

        TSignedSeqPos aln_off = aln_pos - m_AlnRange->GetFrom();
        if(m_bNegative)    {
          return m_SeqRange->GetTo() - aln_off;
        } else return aln_off + m_SeqRange->GetFrom();
    }
    inline double GetSeqPosFromAlnPos(double aln_pos)
    {
        _ASSERT(m_SeqRange  && m_AlnRange);

        double aln_off = aln_pos - m_AlnRange->GetFrom();
        if(m_bNegative)    {
          return m_SeqRange->GetTo() - aln_off;
        } else return aln_off + m_SeqRange->GetFrom();
    }
protected:
    const TSignedSeqRange* m_SeqRange;
    const TSignedSeqRange* m_AlnRange;
    bool    m_bNegative;
};


void    CTraceGraph::x_RenderContour(CGlPane& pane, int y, int top_h, int total_h,
                                     IAlnSegmentIterator& it)
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

    TSignedSeqRange gr_range = TSignedSeqRange(m_Data->GetSeqFrom(), m_Data->GetSeqTo());
    SChunkTranslator    trans;

    // iterate by segments
    for( ;  it; ++it )  {
      const IAlnSegment& seg = *it;
      if(seg.IsAligned()) {
            TSignedSeqRange ch_range = seg.GetRange();
            const TSignedSeqRange& ch_aln_range = seg.GetAlnRange();
            trans.Init(ch_range, ch_aln_range, m_Data->IsNegative());

            ch_range.IntersectWith(gr_range);

            if(ch_range.NotEmpty())   {
                double x1 = trans.GetAlnPosFromSeqPos(ch_range.GetFrom()) - offset_x;
                double x2 = trans.GetAlnPosFromSeqPos(ch_range.GetTo()) - offset_x;
                if(x1 > x2) {
                    swap(x1, x2);
                }
                x2 += 1.0;
                gl.Rectd(x1, y1, x2, y2);
                gl.Rectd(x1, y3, x2, y4);
            }
        }
    }
}


const static float kEps = 0.000001f; // precision for float point comparisions


// renders confidence graph
void    CTraceGraph::x_RenderConfGraph(CGlPane& pane, int y, int h,
                                       IAlnSegmentIterator& it_seg)
{
    const TVPRect& rc_vp = pane.GetViewport();
    const TModelRect rc_vis = pane.GetVisibleRect();

    IRender& gl = GetGl();

    gl.Disable(GL_BLEND);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    TModelUnit amp  = 0.0;
    // guard against the situation where confidence data is not available
    // or fails to load
    if (m_Data->GetMaxConfidence() != 0.0) {
        amp = ((TModelUnit) h - 2 * kGraphOffsetY -1)
                        / m_Data->GetMaxConfidence();
    }

    TModelUnit base_y = y + kGraphOffsetY;
    TModelUnit scale_x = pane.GetScaleX();

    TSignedSeqPos gr_from = m_Data->GetSeqFrom();
    TSignedSeqPos gr_to = m_Data->GetSeqTo();

    SChunkTranslator trans;

    bool b_average = (scale_x > 0.3);
    if(b_average)    {
        pane.Close();
        pane.OpenPixels();

        //calculate visible range in seq coords
        TVPUnit range_pix_start = rc_vp.Left();
        TVPUnit range_pix_end = rc_vp.Right();

        TModelUnit  left = rc_vis.Left();
        scale_x = pane.GetScaleX();
        TModelUnit  top_y = rc_vp.Bottom() - (y + kGraphOffsetY);

        gl.Begin(GL_LINES);

        bool it_seg_moved = true;
        const IAlnSegment* seg = NULL;

        TModelUnit pos_pix_left, pos_pix_right;

        TSignedSeqPos from = -1, to = -2;
        TSignedSeqPos pos = gr_from, pos_inc = 1;
        double v = 0, v_min = 0, v_max = 0;

        TModelUnit ch_pix_end = range_pix_start - 1;

        for( int pix = range_pix_start;
             pix <= range_pix_end  &&  it_seg  &&  pos <= gr_to; )   { // iterate by pixels
            bool b_first = true;

            // if segment ends before pixel
            if(it_seg_moved)  {
                // advance segment
                 seg = &*it_seg;
                if(seg->IsAligned())    {
                    const TSignedSeqRange& ch_range = seg->GetRange();
                    const TSignedSeqRange& ch_aln_range = seg->GetAlnRange();

                    trans.Init(ch_range, ch_aln_range, m_Data->IsNegative());

                    // calculate intersection of the segment in seq_coords with graph range
                    from = max(ch_range.GetFrom(), gr_from);
                    to = min(ch_range.GetTo(), gr_to);

                    // clip with visible align range (expanding integer clip)
                    TSignedSeqPos vis_from = trans.GetSeqPosFromAlnPos((TSignedSeqPos) floor(rc_vis.Left()));
                    TSignedSeqPos vis_to = trans.GetSeqPosFromAlnPos((TSignedSeqPos) ceil(rc_vis.Right()));
                    if(vis_to < vis_from)   {
                        _ASSERT(m_Data->IsNegative());
                        swap(vis_from, vis_to);
                    }
                    from = max(from, vis_from);
                    to = min(to, vis_to);
                    if(m_Data->IsNegative()) {
                        ch_pix_end = range_pix_start + (trans.GetAlnPosFromSeqPos(from) + 1 - left) / scale_x;
                        pos = to;
                        pos_inc = -1;
                    } else {
                        ch_pix_end = range_pix_start + (trans.GetAlnPosFromSeqPos(to) + 1 - left) / scale_x;
                        pos = from;
                        pos_inc = 1;
                    }
                }
                it_seg_moved = false;
            }

            if(seg->IsAligned()) {
                TSignedSeqPos aln_pos = trans.GetAlnPosFromSeqPos(pos);
                pos_pix_left = range_pix_start + (aln_pos - left) / scale_x;
                pos_pix_right = pos_pix_left + 1 / scale_x;

                pos_pix_left = max((TModelUnit) pix, pos_pix_left);

                _ASSERT(pos_pix_left >= pix);

                while(pos >= from  &&  pos <= to  &&  pos_pix_left < pix + 1)   {
                    // calculate overlap with pixel and integrate
                    v = amp * m_Data->GetConfidence(pos);

                    v_min = b_first ? v : min(v_min, v);
                    v_max = b_first ? v : max(v_max, v);
                    b_first = false;

                    if(pos_pix_right < pix +1)  {
                        pos +=pos_inc; // advance to next pos
                        aln_pos = trans.GetAlnPosFromSeqPos(pos);
                        pos_pix_left = range_pix_start + (aln_pos - left) / scale_x;
                        pos_pix_right = pos_pix_left + 1 / scale_x;
                    } else break;
                }
            }
            if(ch_pix_end < pix + 1) {
                ++it_seg;
                it_seg_moved = true;
            } else  {
                if(seg->IsAligned()) {
                    gl.Color3d(0.0f, 0.5f, 0.0f);
                    gl.Vertex2d(pix, top_y);
                    gl.Vertex2d(pix, top_y - v_min);

                    gl.Color3d(0.0f, 0.75f, 0.25f);
                    gl.Vertex2d(pix, top_y - v_min);
                    gl.Vertex2d(pix, top_y - v_max);

                }
                pix++;
                v = v_min = v_max = 0;
            }
        }
        gl.End();
    } else { // render without averaging
        _ASSERT(pane.GetProjMode() == CGlPane::eOrtho);

        TModelUnit offset_x = pane.GetOffsetX();
        gl.Color3d(0.0f, 0.5f, 0.0f); //###

        for( ;  it_seg;  ++it_seg  ) {
            const IAlnSegment& seg = *it_seg;
            if(seg.IsAligned()) {
                const TSignedSeqRange& ch_range = seg.GetRange();
                const TSignedSeqRange& ch_aln_range = seg.GetAlnRange();
                trans.Init(ch_range, ch_aln_range, m_Data->IsNegative());

                TSignedSeqPos from = max(gr_from, ch_range.GetFrom());
                TSignedSeqPos to = min(gr_to, ch_range.GetTo());

                for( TSignedSeqPos pos = from; pos <= to; pos ++ ) {
                    double v = m_Data->GetConfidence(pos);
                    v *= amp;
                    TSignedSeqPos aln_pos = trans.GetAlnPosFromSeqPos(pos);
                    double x = aln_pos - offset_x;
                    gl.Rectd(x , base_y, x + 1.0, base_y + v);
                }
            }
        }
    }
}

const static int kIntBandSpace = 1;

void CTraceGraph::x_RenderSignalGraph(CGlPane& pane, int y, int h,
                                         IAlnSegmentIterator& it)
{
    IRender& gl = GetGl();

    CTraceData::TSignalValue MaxSignal = 0;
    for( int ch = CTraceData::eA; ch <= CTraceData::eG; ch++ )   {
        MaxSignal = max(MaxSignal, m_Data->GetMax( (CTraceData::EChannel) ch));
    }
    TModelUnit amp  = ((TModelUnit) (h - 2 * kGraphOffsetY)) / MaxSignal;
    int bottom_y = y + (h - 1) - kGraphOffsetY;

    for( int i_ch = 0; i_ch < 4; i_ch++ )   {   // for every channel (i_ch - channel index)
        int ch_index = (m_Props.m_bReverseColors  &&  m_Data->IsNegative()) ? (i_ch ^ 2) : i_ch;
        int ch = CTraceData::eA + ch_index;

        const CTraceData::TPositions& positions = m_Data->GetPositions();

        CTraceData::EChannel channel = (CTraceData::EChannel) ch;
        const CTraceData::TValues& values = m_Data->GetValues(channel);

        gl.ColorC(m_vSignalColors[kGradColors * (i_ch + 1) -1]);
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        _ASSERT(m_Props.m_SignalStyle == CTraceGraphProperties::eCurve);

        auto_ptr<IAlnSegmentIterator> p_it_1(it.Clone());
        for(  IAlnSegmentIterator& it_1(*p_it_1);  it_1;  ++it_1 ) {
            if(it_1->IsAligned()) {
                x_RenderCurveSegment(pane, *it_1, positions, values, bottom_y, h, (int) amp);
            }
        }

		gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}


// Renders chromatogram corresponding to given segment of sequence.
// This function renders data as a curve for a single channel specified by "values".
void    CTraceGraph::x_RenderCurveSegment(CGlPane& pane,
                                        const IAlnSegment& seg,
                                        const CTraceData::TPositions& positions,
                                        const CTraceData::TValues& values,
                                        int bottom_y, int /*h*/, int amp)
{
    IRender& gl = GetGl();

    const TModelRect rc_vis = pane.GetVisibleRect();

    const TSignedSeqRange& ch_range = seg.GetRange();
    const TSignedSeqRange& ch_aln_range = seg.GetAlnRange();

    SChunkTranslator trans;
    bool b_neg = m_Data->IsNegative();
    trans.Init(ch_range, ch_aln_range, m_Data->IsNegative());

    // [from, to] - is sequence range for which graph is rendered
    double from = max(m_Data->GetSeqFrom(), ch_range.GetFrom());
    double to = min(m_Data->GetSeqTo(), ch_range.GetTo());

    double vis_from = trans.GetSeqPosFromAlnPos(rc_vis.Left());
    double vis_to = trans.GetSeqPosFromAlnPos(rc_vis.Right());
    if(vis_to < vis_from)   {
        _ASSERT(m_Data->IsNegative());
        swap(vis_from, vis_to);
    }

    from = max(from, vis_from);
    to = min(to, vis_to);

    if(from <= to)  {
        int sm_start = x_FindSampleToLeft(from);
        int sm_end = x_FindSampleToRight(to + 1);
        sm_start = max(0, sm_start);
        sm_end = min(sm_end, m_Data->GetSamplesCount() - 1);

        if(sm_start <= sm_end)  {
            gl.Begin(GL_QUAD_STRIP);
            TModelUnit offset_x = pane.GetOffsetX();

            // check if start interpolation is needed
            CTraceData::TFloatSeqPos sm_start_seqpos = positions[sm_start];
            if(sm_start_seqpos < from)   {
                if(sm_start + 1 < sm_end)   {
                    double v1 = values[sm_start];
                    double v2 = values[sm_start + 1];
                    double x1 = sm_start_seqpos;
                    double x2 = positions[sm_start + 1];
                    double v = v1 + ((from - x1) * (v2 - v1) / (x2 - x1));
                    v *= amp;

                    double aln_from = trans.GetAlnPosFromSeqPos(from + (b_neg ? -1 : 0));
                    gl.Vertex2d(aln_from - offset_x, bottom_y - v);
                    //gl.Vertex2d(aln_from - offset_x, bottom_y);
                }
                sm_start++;
            }
            // render regular samples
            for( int i_sm = sm_start; i_sm < sm_end; i_sm++  ) {
                TModelUnit seqpos = positions[i_sm];
                double v = values[i_sm];
                v *= amp;

                double aln_pos = trans.GetAlnPosFromSeqPos(seqpos + (b_neg ? -1 : 0));
                gl.Vertex2d(aln_pos - offset_x, bottom_y - v);
                //gl.Vertex2d(aln_pos - offset_x, bottom_y);
            }
            // render end point
            if( sm_end - 1 > sm_start) { // interpolate end point
                double v1 = values[sm_end -1];
                double v2 = values[sm_end];
                double x1 = positions[sm_end - 1];
                double x2 = positions[sm_end];
                double v = v1 + ((to + 1 - x1) * (v2 - v1) / (x2 - x1));
                v *= amp;

                double aln_to = trans.GetAlnPosFromSeqPos(to + (b_neg ? 0 : 1));
                gl.Vertex2d(aln_to - offset_x, bottom_y - v);
                //gl.Vertex2d(aln_to - offset_x, bottom_y);
            } else {
               TModelUnit seqpos = positions[sm_end];
                double v = values[sm_end];
                v *= amp;

                double aln_pos = trans.GetAlnPosFromSeqPos(seqpos);
                gl.Vertex2d(aln_pos - offset_x, bottom_y - v);
                //gl.Vertex2d(aln_pos - offset_x, bottom_y);
            }
            gl.End();
        }
    }
}


/// Render signals for all channels as gradient-color bands with color intensity
/// proprotional to signal strength.
void    CTraceGraph::x_RenderIntensityGraphs(CGlPane& pane, int y, int h,
                                             IAlnSegmentIterator& it_seg)
{
    IRender& gl = GetGl();

    //_TRACE("\nx_RenderIntensityGraphs");
    const CTraceData::TPositions& positions = m_Data->GetPositions();

    const TVPRect& rc_vp = pane.GetViewport();
    const TModelRect rc_vis = pane.GetVisibleRect();

    // calculate layout
    int av_h = h - 2 * kGraphOffsetY; // height available for intensity bands
    int band_h = av_h / 4;
    int off = (av_h - 4 * band_h) / 2; // rounding error compensation
    int top_y = rc_vp.Bottom() - (y + kGraphOffsetY + off);

    TModelUnit  left = rc_vis.Left();
    TModelUnit  scale_x = pane.GetScaleX();

    TVPUnit range_pix_start = rc_vp.Left();
    TVPUnit range_pix_end = rc_vp.Right();

    SChunkTranslator trans;

    gl.Begin(GL_LINES);

    // iterate by pixels,samples and chunks
    for( int pix = range_pix_start; pix <= range_pix_end  &&  it_seg;  ++it_seg)   {
        const IAlnSegment& seg = *it_seg;
        if(seg.IsAligned()) {
            // calculate samples range by segment
            const TSignedSeqRange& ch_range = seg.GetRange();
            const TSignedSeqRange& ch_aln_range = seg.GetAlnRange();
            trans.Init(ch_range, ch_aln_range, m_Data->IsNegative());

            // [from, to] - is sequence range for which graph is rendered
            double from = max(m_Data->GetSeqFrom(), ch_range.GetFrom());
            double to = min(m_Data->GetSeqTo(), ch_range.GetTo());

            double vis_from = trans.GetSeqPosFromAlnPos(rc_vis.Left());
            double vis_to = trans.GetSeqPosFromAlnPos(rc_vis.Right());
            if(vis_to < vis_from)   {
                _ASSERT(m_Data->IsNegative());
                swap(vis_from, vis_to);
            }

            from = max(from, vis_from);
            to = min(to, vis_to);

            // [sm_start, sm_end] - samples range being rendered
            int sm_start = x_FindSampleToLeft(from);
            int sm_end = x_FindSampleToRight(to + 1);
            sm_start = max(sm_start, 0);
            sm_end = min(sm_end, m_Data->GetSamplesCount() - 1);

            // calculate pixels range to render
            double aln_from = trans.GetAlnPosFromSeqPos(from);
            double aln_to = trans.GetAlnPosFromSeqPos(to);
            if(m_Data->IsNegative())   {
                swap(aln_from, aln_to);
            }
            aln_to += 1;

            TVPUnit pix_start = range_pix_start + (TVPUnit) floor((aln_from - left) / scale_x);
            TVPUnit pix_end = range_pix_start + (TVPUnit) ceil((aln_to - left) / scale_x);
            pix_start = max(pix_start, range_pix_start);
            pix_end = min(pix_end, range_pix_end);

            int sm_inc = m_Data->IsNegative() ? -1 : +1;

            int band_y = top_y;
            for( int i_ch = 0; i_ch < 4; i_ch++ )   {   // for every channel (i_ch - channel index)
                int ch_index = (m_Props.m_bReverseColors  &&
                                m_Data->IsNegative()) ? (i_ch ^ 2) : i_ch;
                int ch = CTraceData::eA + ch_index;

                const CTraceData::TValues& values = m_Data->GetValues((CTraceData::EChannel) ch);
                CTraceData::TSignalValue MaxSignal = m_Data->GetMax((CTraceData::EChannel) ch);

                double x1 = 0.0, x2 = 0.0, pix_x1 = 0.0, pix_x2 = 0.0;
                double v1, v2, s, dx, sum, sum_pix_x;

                int sample = m_Data->IsNegative() ? sm_end : sm_start;
                _ASSERT(sample >= 0);

                for( TVPUnit pix = pix_start; pix <= pix_end;  pix++ )  { // for each pixel
                    // calculate average value for "pix" pixel by integrating values
                    // over the range [pix, pix+1]
                    sum = 0; // integral from values by pix_x
                    sum_pix_x = 0; // length of integrated range

                    x1 = positions[sample];
                    if(m_Data->IsNegative())
                        x1 -= 1.0;
                    pix_x1 = range_pix_start + (trans.GetAlnPosFromSeqPos(x1) - left) / scale_x;
                    v1 = v2 = values[sample]; //#####

                    if(pix_x1 < pix + 1)    {

                        bool b_next_point = m_Data->IsNegative() ? (sample > sm_start) : (sample < sm_end);
                        if(b_next_point) { // there is second point available
                            x2 = positions[sample + sm_inc];
                            if(m_Data->IsNegative())
                                x2 -= 1.0;
                            pix_x2 = range_pix_start + (trans.GetAlnPosFromSeqPos(x2) - left) / scale_x;
                            v2 = values[sample + sm_inc];

                            if(pix_x1 < pix)    { // fisrt sample is to the left of this pixel
                                // replace it fake interpolated sample at x = "pix"
                                v1 += (v2 - v1) * (pix - pix_x1) / (pix_x2 - pix_x1);
                                pix_x1 = pix;
                            }
                        }

                        while(b_next_point  &&  pix_x2 <= pix + 1) // while second point is inside pixel
                        {
                            dx = pix_x2 - pix_x1;
                            s = 0.5 * (v1 + v2) * dx;
                            _ASSERT(s >=0  &&  dx >= 0);

                            sum += s;
                            sum_pix_x += dx;

                            sample += sm_inc; // advance, x2 becomes x1
                            pix_x1 = pix_x2;
                            v1 = v2;

                            b_next_point = m_Data->IsNegative() ? (sample > sm_start) : (sample < sm_end);
                            if(b_next_point) {
                                x2 = positions[sample + sm_inc];
                                if(m_Data->IsNegative())
                                    x2 -= 1.0;
                                pix_x2 = range_pix_start + (trans.GetAlnPosFromSeqPos(x2) - left) / scale_x;
                                v2 = values[sample + sm_inc];
                            } else break;
                        }
                        _ASSERT(pix_x1 <= pix + 1);

                        if(b_next_point  &&  pix_x2 > pix + 1)    { // second point is outside pixel
                            dx = pix + 1 - pix_x1;
                            _ASSERT(dx >= 0);

                            double v = v1 + (v2 - v1) * dx / (pix_x2 - pix_x1);
                            s = 0.5 * (v1 + v) * dx;

                            _ASSERT(s >=0  &&  dx >= 0);

                            sum += s;
                            sum_pix_x += dx;
                        }
                        double av_v = (sum_pix_x) > 0  ? sum / sum_pix_x : 0;

                        // render pixel
                        double norm = (MaxSignal == 0) ? 0 : (av_v / MaxSignal);
                        const CRgbaColor& col = GetColorByValue(norm, i_ch);
                        gl.ColorC(col);
                        gl.Vertex2d(pix, band_y);
                        gl.Vertex2d(pix, band_y - band_h - 1);
                    } // if(pix < pix + 1)
                }
                band_y -= band_h;
            }
        }
    }
    gl.End();
}

// returns gradient color corresponding to normalized [0, 1.0] value "value"
// for channel specified by "signal"
const CRgbaColor& CTraceGraph::GetColorByValue(double value, int signal) const
{
    _ASSERT(value >= 0  &&  value <= 1.0);
    _ASSERT(signal >= 0  &&  signal <= 3);

    int i = (int) (value * kGradColors);
    i = min(i, kGradColors);
    int index = signal * kGradColors + i;
    return m_vSignalColors[index];
}

/// returns index of rightmost sample having m_SeqPos less then "pos".
/// if "pos" is to the left of the trace range function returns -1,
/// if "pos" is to the right of the trace range functions returns "n_samples"
int CTraceGraph::x_FindSampleToLeft(double pos) const
{
    int n_samples = m_Data->GetSamplesCount();
    if(pos < m_Data->GetSeqFrom()  || n_samples == 0)  {
        return -1;
    } else if(pos > m_Data->GetSeqTo())   {
        return n_samples;
    } else {
        const CTraceData::TPositions& positions = m_Data->GetPositions();
        double scale = ((double) n_samples) / m_Data->GetSeqLength();

        // calculate approximate sample index
        int i = (int) (scale * (pos - m_Data->GetSeqFrom()));
        i = min(i, n_samples - 1);
        i = max(i, 0);

        if(positions[i] > pos)  {
            for(  ; i > 0  &&  positions[i] > pos;  i-- )  {
            }
        } else {
            for(  ; ++i < n_samples  &&  positions[i] < pos;  )  {
            }
            i--;
        }
        return i;
    }
}


/// returns index of the leftmost sample having m_SeqPos greater than "pos"
/// if "pos" is to the left of the trace range function returns -1,
/// if "pos" is to the right of the trace range functions returns "n_samples"
int CTraceGraph::x_FindSampleToRight(double pos) const
{
    int n_samples = m_Data->GetSamplesCount();
    if(pos < m_Data->GetSeqFrom()  || n_samples == 0)  {
        return -1;
    } else if(pos > m_Data->GetSeqTo())   {
        return n_samples;
    } else {
        const CTraceData::TPositions& positions = m_Data->GetPositions();
        double scale = ((double) n_samples) / m_Data->GetSeqLength();

        // calculate approximate sample index
        int i = (int) (scale * (pos - m_Data->GetSeqFrom()));
        i = min(i, n_samples - 1);
        i = max(i, 0);

        if(positions[i] > pos)  {
            for(  ; i > 0  &&  positions[i] > pos;  i-- ) {
            }
            i++;
        } else {
            for(  ; ++i < n_samples  &&  positions[i] < pos;  ) {
            }
        }
        return i;
    }
}


END_NCBI_SCOPE
