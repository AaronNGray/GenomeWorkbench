/*  $Id: gradient_panel.cpp 24939 2011-12-29 14:52:50Z wuliangs $
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

#include <gui/utils/rgba_color.hpp>

#include <gui/widgets/hit_matrix/gradient_panel.hpp>

#include <gui/widgets/wx/wx_utils.hpp>


#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/dcclient.h>


BEGIN_NCBI_SCOPE


BEGIN_EVENT_TABLE(CGradientPanel, wxWindow)
    EVT_PAINT(CGradientPanel::OnPaint)
END_EVENT_TABLE()

static const int kGradH = 16;
static const int kTickH = 8;
static const int kOffset = 2;
static const int kSpace = 8;


CGradientPanel::CGradientPanel(wxWindow* parent,
                               wxWindowID id,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style,
                               const wxString& name)
:   wxPanel(parent, id, pos, size, style, name),
    m_Params(NULL)
{
//     SetBackgroundStyle(wxBG_STYLE_CUSTOM); // do not erase it

    m_Font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

}


wxSize CGradientPanel::DoGetBestSize() const
{
    int text_h = abs(m_Font.GetPixelSize().y);

    wxSize sz(200, kGradH + kTickH + text_h + 5 * kOffset);
    CacheBestSize(sz);
    return sz;
}


void CGradientPanel::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    wxRect rc = GetClientRect();

    if(m_Params  &&  IsEnabled())    {
        rc.Deflate(kOffset);

        int left = rc.GetLeft();
        int top = rc.GetTop();
        int width = rc.GetWidth();

        int right = left + width - 1;

        // draw gradient bar
        int n = m_Params->m_Steps;

        double grad_start = m_Params->m_EnableMinGrad ? m_Params->m_MinGrad : m_Params->m_MinValue;
        double start = min(m_Params->m_MinValue, grad_start);
        double grad_end = m_Params->m_EnableMaxGrad ? m_Params->m_MaxGrad : m_Params->m_MaxValue;
        double end = max(m_Params->m_MaxValue, grad_end);

        double norm_start = m_Params->GetColorNorm(grad_start);
        double norm_end = m_Params->GetColorNorm(grad_end);
        norm_start = max(0.0, norm_start);
        norm_end = min(1.0, norm_end);

        dc.SetPen(*wxTRANSPARENT_PEN);  

        double d = ((double) (norm_end - norm_start) * width) / n;
        int grad_x = left + (int)(norm_start * width);
        for(int i = 0; i < n ; i++)    {
            int x1 = (int) floor(d * i);
            int x2 = (int) ceil(d * (i + 1));
            float k = ((float) i) / (n - 1);
            CRgbaColor  color = CRgbaColor::Interpolate(m_Params->m_MaxColor, m_Params->m_MinColor, k);

            wxBrush brush(ConvertColor(color));
            dc.SetBrush(brush);
            dc.DrawRectangle(grad_x + x1, top, x2 - x1, kGradH);
        }

        // draw min max labels
        wxColour cl_text = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
        dc.SetTextForeground(cl_text);
        dc.SetFont(m_Font);

        int tick_y = top + kGradH + kOffset;
        int text_y = top + kGradH + kTickH + 2 * kOffset;

        int clip_left = left;
        int clip_right = right;
        wxAlignment align = (m_Params->m_MinValue > start) ? wxALIGN_CENTER : wxALIGN_LEFT;
        x_DrawNumber(dc, m_Params->m_MinValue, left, right, clip_left, clip_right, text_y, align);

        align = (m_Params->m_MaxValue < end) ? wxALIGN_CENTER : wxALIGN_RIGHT;
        clip_left = clip_right + kSpace;
        x_DrawNumber(dc, m_Params->m_MaxValue, left, right, clip_left, clip_right, text_y, align);

        // draw tick marks
        dc.SetPen(wxPen(cl_text));

        double value_range = m_Params->m_MaxValue - m_Params->m_MinValue;
        double d_value = value_range / 4;
        for( int i = 0; i < 5; i++ )    {
            double v = m_Params->m_MinValue + i * d_value;
            double norm = m_Params->GetColorNorm(v);
            int line_x = left + (int) (norm * (width - 1));
            dc.DrawLine(line_x, tick_y, line_x, tick_y + kTickH);
        }
    }
}

// draws a label for the given value, left and right params specify limits of the
// drawing area
void    CGradientPanel::x_DrawNumber(wxDC& dc, double value, int left, int right,
                                   int& clip_left, int& clip_right, int y, wxAlignment align)
{
    double norm = m_Params->GetColorNorm(value);
    int x = left + (int)(norm * (right - left));
    int text_w = 0, text_h = 0;

    wxString str = wxString::Format( ToWxString(m_Params->GetPrecisionFormat()), value );
    dc.GetTextExtent(str, &text_w, &text_h);

    switch(align)   {
    case wxALIGN_CENTER:
        x -= text_w / 2;
        break;
    case wxALIGN_RIGHT:
        x -= text_w;
        break;
    default:
        break;
    }

    // adjust position if necessary
    if(x + text_w > right)  {
        x = right - text_w;
    }
    if(x < left)    {
        x = left;
    }

    if(x >= left  &&  (x + text_w - 1) <= right)    {   // draw
        dc.DrawText(str, x, y);

        clip_left = x;
        clip_right = x + text_w - 1;
    } else {
        clip_left = clip_right = left;
    }
}


void CGradientPanel::SetParams(const SHitColoringParams* params)
{
    m_Params = params;
    Refresh();
}


END_NCBI_SCOPE
