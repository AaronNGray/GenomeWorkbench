/*  $Id: ruler.cpp 45024 2020-05-09 02:03:16Z evgeniev $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/opengl/glstate.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/widgets/gl/ruler.hpp>

#include <math.h>


BEGIN_NCBI_SCOPE

CRuler::CRuler(bool horz)
:   m_Visible(true),
    m_FontRotate(CGlTextureFont::fFontRotateBase),
    m_FontRotateDegrees(0),
    m_AutoRange(true),
    m_BaseWidth(1),
    m_BaseOffset(0.5),
    m_DisplayOptions(fDefaultDisplayOptions),    
    m_Font(CGlTextureFont::eFontFace_Helvetica, 12),
    m_TextColor(0.1f, 0.2f, 0.1f),
    m_RulerColor(0.2f, 0.2f, 0.2f),
    m_BackColor(0.95f, 0.95f, 0.95f),
    m_MajorTickSize(6),
    m_MinorTickSize(3),
    m_LabelTickSize(6), 
    m_OppMajorTickSize(0),
    m_OppMinorTickSize(0),
    m_OppLabelTickSize(6), 
    m_Dirty(true),
    m_ScaleX(0),
    m_ScaleY(0),
    m_MaxLabelW(0),
    m_MaxLabelH(0),
    m_BaseStep(0),
    m_PosLabelsStep(-1),
    m_TickSpace(0)
{
    SetHorizontal(horz);
    m_LabelPlace = m_Horz ? eBottom : eLeft;
    m_LabelAln = eAln_Center;
}


CRuler::~CRuler()
{
}


const static int kMinTickStepPixels = 5;
const static int kLabelSepPixX = 12; // min distance between labels
const static int kLabelSepPixY = 12; // min distance between labels


// change Ruler orientation
void CRuler::SetHorizontal(bool horz, ELabelPlacement place, ELabelAlign aln)
{
    m_Horz = horz;

    if(m_Horz) {
        switch(place) {
            case eLeft:
            case eRight:    _ASSERT(false);
            case eDefault:
            case eBottom: m_LabelPlace = eBottom; break;
            case eTop: m_LabelPlace = eTop; break;
        }
    } else {
        switch(place)   {
            case eBottom:
            case eTop:    _ASSERT(false);
            case eDefault:
            case eLeft: m_LabelPlace = eLeft; break;
            case eRight: m_LabelPlace = eRight; break;
        }
    }

    switch (aln) {
        case eAln_Right:
        case eAln_Top:
            m_LabelAln = horz ? eAln_Right : eAln_Top;
            break;
        case eAln_Left:
        case eAln_Bottom:
            m_LabelAln = horz ? eAln_Left : eAln_Bottom;
            break;
        case eAln_Center:
        default:
            m_LabelAln = eAln_Center;
            break;
    }

    m_Dirty = true;
}


void CRuler::SetColor(EColorType type, const CRgbaColor& color)
{
    switch(type)    {
    case eRuler: m_RulerColor = color; break;
    case eText: m_TextColor = color; break;
    case eBackground: m_BackColor = color; break;
    default: _ASSERT(false);
    }
    m_Dirty = true;
}


void CRuler::SetFont(CGlTextureFont::EFontFace font_type,
                     unsigned int font_size)
{
    m_Font.SetFontFace(font_type);
    m_Font.SetFontSize(font_size);

    m_Dirty = true;
}


void CRuler::SetLabelOrientation(CGlTextureFont::EFontRotateFlags rotate, 
                                 int rotate_degrees)
{
    m_FontRotate = rotate;
    m_Font.SetFontRotate(m_FontRotate);

    m_FontRotateDegrees = rotate_degrees;
}


void CRuler::SetDisplayOptions(int options)
{
    m_DisplayOptions = options;
}


void CRuler::SetAutoRange()
{
    m_AutoRange = true;
}


// keep this functions for simplicity and backward compatibility
void CRuler::SetRange(int start, int end, int seq_start, bool reverse)
{
    m_AutoRange = false;

    m_Mapping.clear();

    int len = end - start + 1;
    TAlignRange range(start, seq_start, len, ! reverse);
    m_Mapping.insert(range);
}


void CRuler::SetMapping(const TAlignColl& coll)
{
    m_AutoRange = false;
    m_Mapping = coll;
}


void CRuler::SetTextLabel(const string& label)
{
    m_TextLabel = label;
}


void CRuler::SetGeometryParam(EGeometryParam geom, int value)
{
    switch(geom)    {
    case eMinorTickHeight:    m_MinorTickSize = value;    break;
    case eMajorTickHeight:
        m_MajorTickSize = value;
        if (m_MajorTickSize > m_LabelTickSize) {
            m_LabelTickSize = m_MajorTickSize;
        }
        break;
    case eLabelTickHeight:    m_LabelTickSize = value;    break;
    case eOppMinorTickHeight: m_OppMinorTickSize = value;    break;
    case eOppMajorTickHeight:
        m_OppMajorTickSize = value;
        if (m_OppMajorTickSize > m_OppLabelTickSize) {
            m_OppLabelTickSize = m_OppMajorTickSize;
        }
        break;
    case eOppLabelTickHeight: m_OppLabelTickSize = value;    break;
    default: _ASSERT(false);
    }

    m_Dirty = true;
}


/// spacing between text and borders or between text and other graphics
static int  kTextSpaceX = 2;
static int  kTextSpaceY = 2;


bool CRuler::x_TextAlongAxis() const
{
    bool horz_text = (m_FontRotateDegrees == 0) ||
                     (m_FontRotateDegrees == 180);
    return m_Horz == horz_text;
}

/// Ruler contains two layers. The first layer displays the axis with Ticks and
/// optional Labels, the second displays Metric and Origin.

/// Returns V size necessary for drawing axis ticks and labels.
/// V is the size in the direction normal to the axis
int CRuler::x_GetTicksLabelsSizeV(int max_num) const
{
    IRender& gl = GetGl();

    int text_space = m_Horz ? kTextSpaceY : kTextSpaceX;
    int v = m_MajorTickSize + m_OppMajorTickSize + text_space;
    if((m_DisplayOptions & fHideLabels) == 0) {
        double text_v = x_TextAlongAxis() ? gl.TextHeight(&m_Font) :
            gl.GetMaxWidth(&m_Font, max_num);
        if (m_LabelAln == eAln_Center) {
            v = m_LabelTickSize + m_OppLabelTickSize +
                2 * text_space + (int) ceil(text_v);
        } else {
            v = m_LabelTickSize;
            v += max(m_OppLabelTickSize, m_OppMajorTickSize + 3 * text_space + (int) ceil(text_v));
        }
    }

    return v;
}


int CRuler::x_GetOriginMetricSizeV() const
{
    int v = 0;

    IRender& gl = GetGl();

    if(m_DisplayOptions & (fShowOrigin | fShowMetric))  {
        // metric and origin text is always dispalyed along the axis
        int text_v = (int) ceil(gl.TextHeight(&m_Font));
        int text_space = m_Horz ? kTextSpaceY : kTextSpaceX;

        v = max(text_v, m_MajorTickSize) + text_space * 2;
    }
    return v;
}


// "max_num" - is the biggest number that needs to be represented by the ruler
TVPPoint CRuler::GetPreferredSize(int max_num) const
{
    int text_v = x_GetTicksLabelsSizeV(max_num);
    int metric_v = x_GetOriginMetricSizeV();
    int v = text_v + metric_v;
    return m_Horz ? TVPPoint(0, v) : TVPPoint(v, 0);
}


TVPRect CRuler::GetVPRect() const
{
    TVPPoint pt = GetPreferredSize(0);
    return TVPRect(0, 0, pt.X(), pt.Y());
}


void CRuler::SetVPRect(const TVPRect&)
{
    // do nothing
}


TModelRect CRuler::GetModelRect() const
{
    return TModelRect();
}


void CRuler::SetModelRect(const TModelRect&)
{
    _ASSERT(false); // not supported
}


TVPPoint CRuler::PreferredSize()
{
    return TVPPoint(0, 0);
}


bool CRuler::IsVisible()
{
    return m_Visible;
}


void CRuler::SetVisible(bool set)
{
    m_Visible = set;
}


bool CRuler::NeedTooltip(CGlPane& /*pane*/, TVPUnit /*vp_x*/, TVPUnit /*vp_y*/)
{
    return false;
}


string CRuler::GetTooltip()
{
    return "Error";
}


// returns the maximal absolute number (having maximum number of digits) that needs
// to be displayed on the Ruler.
// This number can be used to deterime how much space do we need for displaying labels
int CRuler::x_GetMaxNum()
{
    int total_max = 0;
    ITERATE(TAlignColl, it, m_Mapping)  {
        const TAlignRange& r = *it;
        int mod_start = abs(x_ToDisplay(r, r.GetFirstFrom()));
        int mod_end = abs(x_ToDisplay(r, r.GetFirstTo()));
        int max_int = max(mod_start, mod_end);
        total_max = max(total_max, max_int);
    }
    return total_max;
}


// calculates distance in pixels between position labels
void CRuler::x_CalculatePosLabelsStep(CGlPane& pane)
{
    IRender& gl = GetGl();
    m_MaxLabelH = gl.TextHeight(&m_Font);

    // determining maximal number of characters in a label
    int max_num = x_GetMaxNum();

    double char_w = gl.GetMetric(&m_Font, CGlTextureFont::eMetric_AvgCharWidth);
    double comma_w = gl.TextWidth(&m_Font, ",");

    int sep_pix = m_Horz ? kLabelSepPixX : kLabelSepPixY;
    m_MaxLabelW = sep_pix + (int) ceil(gl.GetMaxWidth(&m_Font, max_num));

    // calculate size of the longest label in model coords
    double scale = m_Horz ? pane.GetScaleX() : pane.GetScaleY();
    double max_label_sym = 0;  // in symbols

    if(x_TextAlongAxis()) {
        max_label_sym = scale * m_MaxLabelW;
    } else {
        max_label_sym = 2 * scale * m_MaxLabelH;
    }

    // choosing step in model coords
    double log = (max_label_sym >= 1.0) ? log10(max_label_sym) : 0;
    log = ceil(log);
    double step = pow((double)10, log) * m_BaseWidth;
    double base_step = step;
    if(step > 10.001)   {
        // try to mimimize step without intersecting labels
        if(m_Horz)   {
            // adjusting order
            int groups_n = 0;
            step = step * 10; // to compensate effect of the first iteration
            double max_label_w = m_MaxLabelW;
            do
            {
                _ASSERT(step > 0);
                step = step / 10;
                log = ceil(log10(step));
                groups_n = (int) (log / 3); // number of comma-separated groups (111,222,333)
                if(groups_n)  {
                    int d_digits =  3 * groups_n - 2;
                    max_label_w = m_MaxLabelW - d_digits * char_w + groups_n * comma_w;
                    max_label_sym = scale * max_label_w;
                }
            } while(groups_n  &&  step > max_label_sym * 10);
            m_MaxLabelW = max_label_w;
            base_step = step;
        }
        // currently step has 10^X form, lets check if we can choose
        // a smaller step in a form  K * 10^(X-1), where K = 2, 5
        // this adjusment does not affect labels size
        if(step > max_label_sym * 5)    {
            base_step = step / 10;  // 10^(X-1)
            step = step / 5;        // 2 * 10^(X-1)
        }
        else if(step > max_label_sym * 2)   {
            base_step = step / 10;  // 10^(X-1)
            step = step / 2;        // 5 * 10^(X-1)
        }
    }

    m_BaseStep = (int) base_step;
    m_BaseStep = max(m_BaseStep, 1);

    m_PosLabelsStep = (int) step;
    m_PosLabelsStep  = max(m_PosLabelsStep , 1);

    x_ChooseTickSpace(scale);

    m_Dirty = false;
}


// choosing optimal distance between ticks
void CRuler::x_ChooseTickSpace(double scale)
{
    m_TickSpace = m_BaseStep;
    int ar_K[] = { 10, 5 ,2 };
    for( int i = 0; i < 3; i++ )    {
        int space = m_TickSpace / ar_K[i];
        if(space >= 1  &&  space / scale > kMinTickStepPixels)   {
            m_TickSpace = space;
            break;
        }
    }
}


// Renders the Ruler
void CRuler::Render(CGlPane& pane)
{
    if (!m_Visible)
        return;

    if(m_AutoRange)    {
        x_UpdateMappingByPane(pane);
    }
    x_UpdatePosLabelsStep(pane);

    // clean background
    CGlAttrGuard AttrGuard(GL_POLYGON_BIT | GL_LINE_BIT);

    IRender& gl = GetGl();
    
    if (gl.IsPrinterFriendly()) {
        TVPRect viewport = pane.GetViewport();
        gl.BeginClippingRect(viewport.Left(), viewport.Top(), viewport.Width(), viewport.Height());
    }
    
    gl.LineWidth(1.0f);
       
    if(m_PosLabelsStep > 0) {
        // step is valid - calculating range to draw
        TModelRect rc_v = pane.GetVisibleRect();

        // [first_elem, last_elem] - is a range being rendered in model coords (clipping)
        int first_elem = (int) floor(m_Horz ? rc_v.Left() : rc_v.Bottom());
        int last_elem = (int) ceil(m_Horz ? rc_v.Right() : rc_v.Top()) -1;

        // find iterators that define the set of ranges that are visible
        TAlignColl::const_iterator it_first = m_Mapping.find_2(first_elem).first;
        pair<TAlignColl::const_iterator, bool> res = m_Mapping.find_2(last_elem);
        TAlignColl::const_iterator it_end = res.second ? ++res.first : res.first;

        // iterate with in the visible range and render things
        for( TAlignColl::const_iterator it = it_first;  it != it_end; it++)   {
            const TAlignRange& range = *it;

            TRange clip_r = range.GetFirstRange();
            clip_r.SetFrom( max(clip_r.GetFrom(), first_elem) );
            clip_r.SetTo( min(clip_r.GetTo(), last_elem) );

            if( ! clip_r.Empty()  &&  (!(m_DisplayOptions & fHideNegative)  ||  range.GetSecondFrom() >= 0)) {
                x_RenderRange(pane, range, clip_r);
            }
        }

    }

    if (gl.IsPrinterFriendly()) {
        gl.EndClippingRect();
    }
}


// if Auto Mode is enabled - update m_Mapping so that it represents
// the model limits rect of the pane
void CRuler::x_UpdateMappingByPane(CGlPane& pane)
{
    _ASSERT(m_AutoRange);
    const TModelRect& rc_lim = pane.GetModelLimitsRect();

    int from = (int) (m_Horz ? rc_lim.Left() : rc_lim.Bottom());
    int to = (int) (m_Horz ? rc_lim.Right() : rc_lim.Top()) - 1;

    bool update = false;
    if(m_Mapping.size() != 1)   {
        update = true;
    } else {
        const TAlignRange& range = *m_Mapping.begin();
        int old_from_1 = range.GetFirstFrom();
        int old_from_2 = range.GetSecondFrom();
        int old_to_1 = range.GetFirstTo();
        int old_to_2 = range.GetSecondTo();
        update = (from != old_from_1  ||  from != old_from_2  ||  to != old_to_1  ||  to != old_to_2);
    }
    if(update)  {
        m_Mapping.clear();
        m_Mapping.insert(TAlignRange(from, from, to - from + 1, true));
    }
}


// recalculate step only if needed
void CRuler::x_UpdatePosLabelsStep(CGlPane& pane)
{
    bool b_update = m_Dirty || m_PosLabelsStep <= 0;
    if(! b_update)  {
        const TModelRect& rc_lim = pane.GetModelLimitsRect();
        bool b_scale_changed = m_Horz ? (m_ScaleX != pane.GetScaleX())
                                       : (m_ScaleY != pane.GetScaleY());
        b_update =  b_scale_changed  ||  ! (m_rcLimits == rc_lim);

        m_rcLimits = rc_lim;
        m_ScaleX = pane.GetScaleX();
        m_ScaleY = pane.GetScaleY();
    }

    if(b_update) {
        x_CalculatePosLabelsStep(pane);
    }
}


// renders a segment of the Ruler
void CRuler::x_RenderRange(CGlPane& pane, const TAlignRange& range, const TRange& clip_r)
{
    _ASSERT( ! clip_r.Empty());

    pane.OpenOrtho();

    if((m_DisplayOptions & fFillBackground))    {
        const TModelRect& rc_v = pane.GetVisibleRect();
        x_RenderBackground(pane, rc_v, clip_r);
    }

    x_RenderScale(pane, range, clip_r); // render ticks and labels

    if((m_DisplayOptions & fHideLabels) == 0) {
        x_RenderAllPosLabels(pane, range, clip_r);
    }

    x_RenderOriginAndMetric(pane, clip_r);

    pane.Close();
}


// returns a number displayed on screen (1-based biologist's coordinate system)
int CRuler::x_ToDisplay(const TAlignRange& range, int model) const
{
    // prg - 0-based programmer's coordinates
    int prg = range.GetSecondPosByFirstPos(model);
    if (m_BaseWidth > 1)
        prg /= m_BaseWidth;
    return prg >= 0 ? (prg + 1) : prg;
}


// converts display coordinates to model coordinates
TModelUnit CRuler::x_ToModel(const TAlignRange& range, int display) const
{
    TModelUnit prg = (display > 0) ? (display - 1) : display;
    if (m_BaseWidth > 1) {
        prg *= m_BaseWidth;
        prg += m_BaseOffset;
    }
    TModelUnit model = range.GetFirstPosBySecondPos(prg);
    return model;
}


void CRuler::x_RenderBackground(CGlPane& pane, const TModelRect& rc, const TRange& clip_r)
{
    IRender& gl = GetGl();

    gl.LineWidth(1.0f);
    gl.ColorC(m_BackColor);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    TModelUnit from = (TModelUnit) clip_r.GetFrom();
    TModelUnit to_open = (TModelUnit) clip_r.GetToOpen();
    TModelRect rc_back(rc);

    if(m_Horz)  {
        rc_back.SetLeft(max(rc_back.Left(), from));
        rc_back.SetRight(min(rc_back.Right(), to_open));
    } else {
        rc_back.SetBottom(max(rc_back.Bottom(), from));
        rc_back.SetTop(min(rc_back.Top(), to_open));
    }

    TModelUnit offset_x = pane.GetOffsetX();
    TModelUnit offset_y = pane.GetOffsetY();
    rc_back.Offset(-offset_x, -offset_y);

    gl.RectC(rc_back);
}

void CRuler::x_CalcStartStopOffsets(const TAlignRange& range, TModelUnit& from_offset, TModelUnit& to_offset)
{
    from_offset = m_BaseOffset;
    to_offset = m_BaseOffset;
    if (m_BaseWidth > 1) {
        int start_pos = range.GetSecondPosByFirstPos(range.GetFirstFrom());
        int stop_pos = range.GetSecondPosByFirstPos(range.GetFirstTo());
        if (start_pos > 0) {
            if (start_pos < stop_pos) {
                from_offset = m_BaseWidth - start_pos % m_BaseWidth;
            } else {
                from_offset = (start_pos + 1) % m_BaseWidth;
            }
            from_offset = from_offset > 0 ? from_offset * 0.5 : m_BaseOffset;
        }
        if (stop_pos > 0) {
            if (start_pos < stop_pos) {
                to_offset = (stop_pos + 1) % m_BaseWidth;
            } else {
                to_offset = m_BaseWidth - stop_pos % m_BaseWidth;
            }

            to_offset = to_offset > 0 ? to_offset * 0.5 : m_BaseOffset;
        }
    } 
}

void CRuler::x_RenderScale(CGlPane& pane, const TAlignRange& range, const TRange& clip_r)
{
    int clip_from = clip_r.GetFrom();
    int clip_to = clip_r.GetTo();

    if( ! clip_r.Empty())  {
        TModelUnit offset_x = pane.GetOffsetX();
        TModelUnit offset_y = pane.GetOffsetY();

        int from = range.GetFirstFrom();
        int to = range.GetFirstTo();
        double MinU = from;
        double MaxU = to + 1;
        TModelUnit from_offset = 0;
        TModelUnit to_offset = 0;
        x_CalcStartStopOffsets(range, from_offset, to_offset);
        MinU += from_offset;
        MaxU -= to_offset;

        // setup rendering parameters

        double u1 = max(MinU, (double) clip_from);
        double u2 = min(MaxU, (double) clip_to + 1);

        TModelUnit v0 = 0.0, v1 = 0.0, v2 = 0.0, v3 = 0.0, v4 = 0.0;
        switch(m_LabelPlace) {
        case eTop:
        case eRight:
            {{
                v0 = m_LabelTickSize;
                v1 = v0 + m_OppMinorTickSize;
                v2 = v0 - m_MinorTickSize;
                v3 = v0 + m_OppMajorTickSize;
                v4 = v0 - m_MajorTickSize;;
            }}
            break;
        case eLeft:
            {{
                v0 = pane.GetVisibleRect().Width() - m_LabelTickSize - 1;
                v1 = v0 - m_OppMinorTickSize;
                v2 = v0 + m_MinorTickSize;
                v3 = v0 - m_OppMajorTickSize;
                v4 = v0 + m_MajorTickSize;
            }}
            break;
        case eBottom:
            {{
                v0 = pane.GetVisibleRect().Height() - m_LabelTickSize - 1;
                v1 = v0 - m_OppMinorTickSize;
                v2 = v0 + m_MinorTickSize;
                v3 = v0 - m_OppMajorTickSize;
                v4 = v0 + m_MajorTickSize;
            }}
            break;
        default:
            _ASSERT(false);
            break;
        }

        IRender& gl = GetGl();

        gl.LineWidth(1.0f);
        gl.ColorC(m_RulerColor);
        gl.Disable(GL_LINE_SMOOTH);
        gl.Begin(GL_LINES);   
    

        if(m_Horz) {
            gl.Vertex2d(u1 - offset_x, v0 - offset_y);
            gl.Vertex2d(u2 - offset_x, v0 - offset_y);
        } else {
           gl.Vertex2d(v0 - offset_x, u1 - offset_y);
           gl.Vertex2d(v0 - offset_x, u2 - offset_y);
        }

        // draw Minor ticks
        if(m_TickSpace) {
            int i_first_disp = (x_ToDisplay(range, clip_from) / m_TickSpace ) * m_TickSpace;
            int i_last_disp = (x_ToDisplay(range, clip_to) / m_TickSpace) * m_TickSpace;

            if (i_last_disp < i_first_disp)  
                swap(i_last_disp, i_first_disp);

            for( int  i = i_first_disp;  i <= i_last_disp;  i += m_TickSpace)   {
                bool major = (m_BaseStep > 1)  &&  (i % m_BaseStep) == 0;
                double v_f = major ? v4 : v2;
                double v_t = major ? v3 : v1;
                double model = x_ToModel(range, i);

                if(model >= u1 &&  model < u2) {
                    double u = model + 0.5;
                    // render tick
                    if(m_Horz) {
                        gl.Vertex2d(u - offset_x, v_f - offset_y);
                        gl.Vertex2d(u - offset_x, v_t - offset_y);
                    } else {
                        gl.Vertex2d(v_f - offset_x, u - offset_y);
                        gl.Vertex2d(v_t - offset_x, u - offset_y);
                    }
                }
            }
        }
        gl.End();
    }
}


// This function fills given vector with indices of the Alignment elements
// for which position labels should be shown
void  CRuler::x_GenerateLabelPositions(const TAlignRange& range,
                                       int first_elem, int last_elem, vector<TModelUnit>& vElemPos)
{
    vElemPos.clear();
    const int& step = m_PosLabelsStep;

    int i_first_disp = (x_ToDisplay(range, first_elem) / step ) * step;
    int i_last_disp = (x_ToDisplay(range, last_elem) / step) * step;

    // labels shall be generated from left to right
    if(i_last_disp < i_first_disp)  {
        for( int  i = i_first_disp;  i >= i_last_disp;  i -= step)   {
            TModelUnit model = x_ToModel(range, i);
            if(model >= first_elem  &&  model <= last_elem) {
                vElemPos.push_back(model);
            }
        }
    } else {
        for( int  i = i_first_disp;  i <= i_last_disp;  i += step)   {
            TModelUnit model = x_ToModel(range, i);
            if(model >= first_elem  &&  model <= last_elem) {
                vElemPos.push_back(model);
            }
        }
    }
}

// returns label size in ruler coordinate system (U, V)
TModelPoint CRuler::x_GetLabelSize(CGlPane& pane, const string& label)
{
    IRender& gl = GetGl();
    double w = gl.TextWidth(&m_Font, label.c_str());
    double h = gl.TextHeight(&m_Font);

    double scale_u = pane.GetScaleX();
    double scale_v = pane.GetScaleY();

    return  TModelPoint(w * scale_u, h * scale_v);
}


TModelPoint CRuler::x_GetLabelSizeUnscaled(CGlPane& pane, const string& label)
{
    IRender& gl = GetGl();
    double w = gl.TextWidth(&m_Font, label.c_str());
    double h = gl.TextHeight(&m_Font);

    return  TModelPoint(w, h);
}

//TODO refactor this function so that it can render several intervals and
// apply the existing algorithm to every interval

// Renders all labels on the ruler
void  CRuler::x_RenderAllPosLabels(CGlPane& pane, const TAlignRange& range, const TRange& clip_r)
{
    vector<TModelUnit> vLabelsPos;
    x_GenerateLabelPositions(range, clip_r.GetFrom() + 1, clip_r.GetTo() - 1, vLabelsPos);

    double scale =  m_Horz ? pane.GetScaleX() : pane.GetScaleY();
    double label_size = 0;
    double label_size_scaled = 0.;
    //const TModelRect& rc_m = pane.GetVisibleRect();

    double low_limit = clip_r.GetFrom();
    double high_limit = clip_r.GetToOpen();

    int sep_pix = m_Horz ? kLabelSepPixX : kLabelSepPixY;

    if(m_DisplayOptions & fShowTextLabel)    {
        label_size = x_GetLabelSizeUnscaled(pane, m_TextLabel).X()*scale + sep_pix * scale;
        const TModelRect& rc_vis = pane.GetVisibleRect();

        double pos = m_Horz? rc_vis.Left() : rc_vis.Bottom();
        x_RenderPosLabel(pane, 0, pos, m_TextLabel, false);
        low_limit = pos + label_size;
    }

    int from = range.GetFirstFrom();
    int to = range.GetFirstTo();

    TModelUnit from_offset = 0;
    TModelUnit to_offset = 0;
    x_CalcStartStopOffsets(range, from_offset, to_offset);

    // setup rendering parameters
    double label_u = from_offset + from; // label position along the Ruler
    string S = kEmptyStr;

    // draw the first Label
    if (!(m_DisplayOptions & fHideFirstLabel) ) {
        S = x_GetPositionLabel(range, from);
        if (m_DisplayOptions & fFirstLabelHasText && !m_TextLabel.empty()) {
            S += " ";
            S += m_TextLabel;
        }
        label_size_scaled = x_GetLabelSizeUnscaled(pane, S).X()*scale;
        label_size = label_size_scaled + sep_pix * scale;

        double label_right = label_u + label_size;
        if(label_u > low_limit) {
            if(label_right > high_limit || label_size_scaled >= range.GetLength())  {
                S.erase();
            } else {
                low_limit = label_u + label_size + sep_pix * scale;
            }
            //double shift = -min(label_size_scaled * 0.5, (double)label_u - from);
            x_RenderPosLabel(pane, label_u, 0, S);
        }
    }

    label_u = (to + 1) - to_offset; // label position along the Ruler
    if(label_u) {
        // draw the Last label
        if (!(m_DisplayOptions & fHideLastLabel) ) {
            S = x_GetPositionLabel(range, to);
            // there is enough space at least for the last label
            label_size = x_GetLabelSizeUnscaled(pane, S).X()*scale;
            if (label_u - label_size < low_limit
                || label_u > high_limit 
                || label_size >= range.GetLength())    {
                S.erase();
            } else {
                high_limit = label_u - (label_size + sep_pix * scale);
            }
            double shift = label_size;
            if ((label_u + label_size * 0.5) < (to + 1))
                shift *= 0.5;
            x_RenderPosLabel(pane, label_u, -label_size, S.c_str());
        }

        // draw regular labels
        int labels_n = (int)vLabelsPos.size();
        for( int i_label = labels_n - 1; i_label >= 0; i_label-- )  {
            TModelUnit pos = vLabelsPos[i_label];
            S = x_GetPositionLabel(range, pos);
            label_u = pos + 0.5; 

            label_size = x_GetLabelSizeUnscaled(pane, S).X()*scale;

            double shift = -label_size * 0.5;
            if (m_LabelAln == eAln_Left  ||  m_LabelAln == eAln_Bottom) {
                shift = -kTextSpaceX * scale - label_size;
            } else if (m_LabelAln == eAln_Right  ||  m_LabelAln == eAln_Top) {
                shift = kTextSpaceX * scale;
            }

            bool b_draw_text = (label_u + shift + label_size < high_limit)
                                &&  (label_u + shift > low_limit);
            if(b_draw_text)   {
                high_limit = label_u + shift;
            } else {
                S = "";
            }

            if ( !S.empty()  ||  (label_u < high_limit && label_u > low_limit)) {
                x_RenderPosLabel(pane, label_u, shift, S);
            }
        }
    }
}


/// distance in pixels between left side of the ruler and origin labels
const static int kOriginOffsetX = 6;

/// minimal size of the metric in pixels
const static int kMinMetricPix = 20;


void CRuler::x_RenderOriginAndMetric(CGlPane& pane, const TRange& clip_r)
{
    // Metric and Origin can only be displayed in horizontal Ruler
    bool can_draw = m_Horz &&  (m_Mapping.size() == 1); // mapping is linear

    if(can_draw) {
         IRender& gl = GetGl();
    
        // translate clipping region into viewport coordinates
        TVPUnit clip_vp_left = pane.ProjectX(clip_r.GetFrom());
        TVPUnit clip_vp_right = pane.ProjectX(clip_r.GetToOpen());
        TVPUnit clip_vp_len = clip_vp_right - clip_vp_left + 1;

        double t_h = gl.GetMetric(&m_Font, CGlTextureFont::eMetric_FullCharHeight);
        int text_h = (int) ceil(t_h);

        bool top_label = (m_LabelPlace == eTop);
        int max_num = x_GetMaxNum();
        TModelUnit off = x_GetTicksLabelsSizeV(max_num);
        const TVPRect& rc_vp = pane.GetViewport();
        TModelUnit y = top_label ? (rc_vp.Bottom() + off) : (rc_vp.Top() - off);

        int origin_right = clip_vp_left; // rightmost point of the origin label

        pane.Close();
        pane.OpenPixels();

        if(m_DisplayOptions & fShowOrigin )  {   // render Origin label
            const TAlignRange& range = *m_Mapping.begin(); // the only segment
            int origin = -1;
            if(range.IsDirect()) {
                origin = range.GetFirstFrom() - range.GetSecondFrom();
            } else {
                origin = range.GetSecondTo() + range.GetFirstFrom();
            }

            string s = "Origin : " + CTextUtils::FormatSeparatedNumber(origin + 1, true);
            int or_text_w = (int) ceil(gl.TextWidth(&m_Font, s.c_str()));

            or_text_w = min(or_text_w, clip_vp_len - kOriginOffsetX);
            int x = origin_right + kOriginOffsetX;
            TModelUnit text_y = (float) (top_label ? y : (y - text_h));
            gl.BeginText(&m_Font, m_TextColor);
                gl.WriteText(x, text_y, x + or_text_w, text_y + text_h, s.c_str());
            gl.EndText();

            origin_right += (x + or_text_w); // advance origin_right to point at the end of the label
        }

        if(m_DisplayOptions & fShowMetric)  {
            // choose metric size
            TModelUnit scale_x = pane.GetScaleX();
            TModelUnit step = m_BaseStep;
            while(step / scale_x < kMinMetricPix)  {
                step *= 10;
            }

            int pix_l = (int) ceil(step / scale_x); //length of metric in pixels
            string s = CTextUtils::FormatSeparatedNumber((int) step, true);
            s += "  ";
            int text_w = (int) ceil(gl.TextWidth(&m_Font, s.c_str()));

            int metric_w = max(pix_l, text_w);
            if(origin_right + metric_w + kOriginOffsetX < clip_vp_right) {
                // there is enough space for rendering metric

                TModelUnit x = clip_vp_right - kOriginOffsetX - metric_w;
                int half_h = max(text_h, m_MajorTickSize) / 2;
                TModelUnit yc = y + (top_label ? half_h : -half_h);

                TModelUnit y1 = yc + m_MajorTickSize / 2;
                TModelUnit y2 = yc - m_MajorTickSize / 2;
            
                gl.Begin(GL_LINES);
                    gl.Vertex2d(x, y1);
                    gl.Vertex2d(x, y2);

                    gl.Vertex2d(x, yc);
                    gl.Vertex2d(x + pix_l, yc);

                    gl.Vertex2d(x + pix_l, y1);
                    gl.Vertex2d(x + pix_l, y2);
                gl.End();

                TModelUnit x1 = x - text_w;
                gl.BeginText(&m_Font, m_TextColor);
                    gl.WriteText(x1, y1, x, y2, s.c_str(),
                                 IGlFont::eAlign_Right);
                gl.EndText();
            }
        }

    }
}


// "pos_u" is the coordinate of the point in the model space
// "u_label_offset" - offset of the label origin relative to this point
void  CRuler::x_RenderPosLabel(CGlPane& pane, double pos_u,
                               double u_label_offset, const string& s_text,
                               bool draw_tick)
{
    if(m_Horz)  {
        x_RenderHorzPosLabel(pane, pos_u, u_label_offset, s_text, draw_tick);
    }   else {
        x_RenderVertPosLabel(pane, pos_u, u_label_offset, s_text, draw_tick);
    }
}


// renders a label on horizontal ruler
void  CRuler::x_RenderHorzPosLabel(CGlPane& pane, double pos_u,
                                   double u_label_offset, const string& s_text,
                                   bool draw_tick)
{
    TModelPoint size = x_GetLabelSize(pane, s_text);
    TModelUnit text_u = size.X();
    TModelUnit text_v = size.Y();

    TModelUnit tick_h = m_LabelTickSize + m_OppLabelTickSize;
    TModelUnit x = pos_u + u_label_offset;

    // don't render label if label is only partially visible
    if (x + text_u > pane.GetVisibleRect().Right()) return;

    TModelUnit label_bottom, tick_bottom;

    bool bottom_label = (m_LabelPlace == eBottom);
    if(bottom_label)  {
        tick_bottom = pane.GetVisibleRect().Top() - tick_h;
        if (m_LabelAln == eAln_Center) {
            label_bottom = tick_bottom - kTextSpaceY - text_v;
        } else {
            label_bottom = tick_bottom - text_v;
            label_bottom += m_LabelTickSize - kTextSpaceY - m_MajorTickSize;
        }
    } else {
        tick_bottom = pane.GetVisibleRect().Bottom();
        if (m_LabelAln == eAln_Center) {
            label_bottom = tick_bottom + tick_h + kTextSpaceY;
        } else {
            label_bottom = tick_bottom + m_LabelTickSize + m_OppMajorTickSize + kTextSpaceY;
        }
    }

    IGlFont::TAlign text_align = IGlFont::eAlign_Center;
    switch(m_FontRotateDegrees)    {
    case 0:
        text_align = bottom_label ? IGlFont::eAlign_Top : IGlFont::eAlign_Bottom;
        break;
    case 90:
        text_align = bottom_label ? IGlFont::eAlign_Right : IGlFont::eAlign_Left;
        break;
    case 180:
        text_align = bottom_label ? IGlFont::eAlign_Bottom : IGlFont::eAlign_Top;
        break;
    case 270:
        text_align = bottom_label ? IGlFont::eAlign_Left : IGlFont::eAlign_Right;
        break;
    default:
        break;
    }

    TModelUnit offset_x = pane.GetOffsetX();//(TModelUnit)((int)pane.GetOffsetX()+TModelUnit(0.5));
    TModelUnit offset_y = pane.GetOffsetY();//(TModelUnit)((int)pane.GetOffsetY()+TModelUnit(0.5));

    // render the label
    IRender& gl = GetGl();
    if (!s_text.empty()) {
        TModelUnit x1 = x - offset_x;
        TModelUnit y1 = label_bottom - offset_y;

        // not forcing text to integer x coords makes it slide more smoothly with
        // the ruler tic marks
        if (m_Font.GetFontSize() >= 12)
            m_Font.SetSnapToPixel(false, true);
        else
            m_Font.SetSnapToPixel(true, true);

        gl.BeginText(&m_Font, m_TextColor);
            gl.WriteText(x1, y1, text_u, text_v, s_text.c_str(), text_align, 
                IGlFont::eTruncate_None, TModelUnit(m_FontRotateDegrees));
        gl.EndText();
    }

    if(draw_tick)   {
        gl.LineWidth(1.0f);
        gl.ColorC(m_RulerColor);
        gl.Disable(GL_LINE_SMOOTH);

        gl.Begin(GL_LINES);
            gl.Vertex2d(pos_u - offset_x, tick_bottom - offset_y);
            gl.Vertex2d(pos_u - offset_x, tick_bottom + tick_h - offset_y);
        gl.End();
    }
}

// renders a label on vertical ruler
void  CRuler::x_RenderVertPosLabel(CGlPane& pane, double pos_u,
                                   double u_label_offset, const string& s_text,
                                   bool draw_tick)
{
    TModelPoint size = x_GetLabelSizeUnscaled(pane, s_text);
    TModelUnit text_u = size.X()*pane.GetScaleX();
    TModelUnit text_v = size.Y()*pane.GetScaleY();

    TModelUnit tick_w = m_LabelTickSize + m_OppLabelTickSize;
    TModelUnit y = pos_u + u_label_offset;

    TModelUnit label_left, tick_left;

    bool left_label = (m_LabelPlace == eLeft);
    if(left_label)  {
        tick_left = pane.GetVisibleRect().Right() - tick_w;
        if (m_LabelAln == eAln_Center) {
            label_left = tick_left - kTextSpaceX;
        } else {
            label_left = tick_left - text_v;
            label_left += m_LabelTickSize - kTextSpaceY - m_MajorTickSize;
        }
    } else {
        tick_left = pane.GetVisibleRect().Left();
        if (m_LabelAln == eAln_Center) {
            label_left = tick_left + tick_w + kTextSpaceX;
        } else {
            label_left = tick_left + m_LabelTickSize +
                m_OppMajorTickSize + kTextSpaceY + 8;
        }
    }

    IGlFont::TAlign text_align = IGlFont::eAlign_Center;
    switch(m_FontRotateDegrees) {
    case 0:
        text_align = left_label ? IGlFont::eAlign_Right : IGlFont::eAlign_Left;
        break;
    case 90:
        text_align = left_label ? IGlFont::eAlign_Bottom : IGlFont::eAlign_Top;
        break;
    case 180:
        text_align = left_label ? IGlFont::eAlign_Left : IGlFont::eAlign_Right;
        break;
    case 270:
        text_align = left_label ? IGlFont::eAlign_Top : IGlFont::eAlign_Bottom;
        break;
    default:
        break;
    }

    TModelUnit offset_x = pane.GetOffsetX();
    TModelUnit offset_y = pane.GetOffsetY();

    // render Label
    IRender& gl = GetGl();

    TModelUnit x1 = label_left - offset_x;
    TModelUnit y1 = y - offset_y;

    // not forcing text to integer y coords makes it slide more smoothly with
    // the ruler tic marks
    if (m_Font.GetFontSize() >= 12)
        m_Font.SetSnapToPixel(true, false);
    else
        m_Font.SetSnapToPixel(true, true);

    gl.BeginText(&m_Font, m_TextColor);
        gl.WriteText(x1, y1, text_u, text_v, s_text.c_str(), text_align, 
            IGlFont::eTruncate_None, TModelUnit(m_FontRotateDegrees));
    gl.EndText();

    if(draw_tick)   { 
        gl.ColorC(m_RulerColor);
        gl.LineWidth(1.0f);
        gl.Disable(GL_LINE_SMOOTH);

        gl.Begin(GL_LINES);
            gl.Vertex2d(tick_left - offset_x, pos_u - offset_y);
            gl.Vertex2d(tick_left + tick_w - offset_x, pos_u - offset_y);
        gl.End();
    }
}


// i_elem is in model coords
string  CRuler::x_GetPositionLabel(const TAlignRange& range, int i_elem)
{
    int pos = x_ToDisplay(range, i_elem);
    string S = CTextUtils::FormatSeparatedNumber(pos, true);
    return S;
}


END_NCBI_SCOPE
