/*  $Id: map_item.cpp 40274 2018-01-19 17:05:09Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/map_item.hpp>
#include <gui/widgets/wx/selection_control.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/utils/command.hpp>

#include <wx/dc.h>

#if defined(NCBI_OS_DARWIN)
// include Cocoa header here
#endif


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CMapItem

CMapItem::CMapItem()
:   
    m_Separator(false),
    m_Cmd(eCmdInvalid),
    m_CmdTarget(NULL)
{
    x_Init();
}

CMapItem::CMapItem(const string& label, wxBitmap& image, const string& descr, bool separator)
:   m_Label(label),
    m_Icon(image),
    m_Descr(descr),
    m_Separator(separator),
    m_Cmd(eCmdInvalid),
    m_CmdTarget(NULL)
{
    x_Init();
}


CMapItem::~CMapItem()
{
}


void CMapItem::x_Init()
{
    m_PrefsDirty = true;
    m_LabelPrefH = 0;
}

void CMapItem::SetIcon(wxBitmap& image)
{
    m_Icon = image;
    m_PrefsDirty = true;
}


void CMapItem::SetCommand(TCmdID cmd, wxEvtHandler* target)
{
    m_Cmd = cmd;
    m_CmdTarget = target;
}


static const int kSeparatorH = 9;
static const int kSepHorzOffset = 2;
static const int kSepVertOffset = 2;

static const int kDefIconWidth  = 16;
static const int kDefIconHeight = 16;


// Layout internal areas based on the current m_Rect
void CMapItem::Layout(wxDC& dc, SwxMapItemProperties& props)
{
    int i_width  = kDefIconWidth;
    int i_height = kDefIconHeight;

    // safeguard against carbon-specific issues
    try {
        if(m_Icon.Ok()) {
            i_width  = m_Icon.GetWidth();
            i_height = m_Icon.GetHeight();
        }
    }
    catch(std::exception&)
    {
        _TRACE("CMapItem::Layout - unable to determine icon size (platform has no support)");
    }

    if(m_PrefsDirty)    {
        x_UpdatePreferredHeights(dc, props, m_Rect.width);
    }

    wxRect rc_back(m_Rect);
    //rc_back.Inflate(-props.m_HorzMargin, -props.m_VertMargin);
    rc_back.Inflate(-props.m_Border, -props.m_Border);
    int x = rc_back.GetLeft() + x_GetItemShift();
    int y = rc_back.GetTop() + props.m_VertMargin + (m_Separator ? x_PreferredSeparatorHeight(dc) : 0);

    m_IconPos = wxPoint(0, 0); // default
    m_LabelRect = wxRect(0, 0, -1, -1); // default

    int h1 = std::max(i_height, m_LabelPrefH);
    bool center = props.m_CenterLabelAndIcon;

    if(m_Icon.Ok()) {
        m_IconPos.x = x + props.m_HorzMargin;
        int icon_off_y = center ? ((h1 - i_height) / 2) : 0;
        m_IconPos.y = y + icon_off_y;
        x += i_width + props.m_HorzMargin; // shift position to the right
    }
    // Labels
    if( ! m_Label.empty()  &&  x < m_Rect.GetRight())  {
        // calculate the rectangle that will accomodate both Label and Description
        int label_off_y = center ? ((h1 - m_LabelPrefH) / 2) : 0;
        wxRect rc_text(x, y + label_off_y, 0, rc_back.height);
        rc_text.SetRight(rc_back.GetRight());
        rc_text.Inflate(-props.m_HorzMargin, 0);

        m_LabelRect = rc_text;
        m_LabelRect.height = std::min(rc_text.height, m_LabelPrefH);
    }
}

// return horizontal shift of the item in pixels
int CMapItem::x_GetItemShift() const
{
    return 0;
}


// calculates the space available for text output based on the given full item width
int CMapItem::x_GetTextAreaWidth(SwxMapItemProperties& props, int width) const
{
    int w = width - 2 * (props.m_Border + props.m_HorzMargin);
    if(m_Icon.Ok()) {
        w -= m_Icon.GetWidth() + 2 * props.m_HorzMargin;
    }
    return std::max(0, w);
}


// calculates preferred text heights based on the given full item width
void CMapItem::x_UpdatePreferredHeights(wxDC& dc, SwxMapItemProperties& props, int width)
{
    int text_w = x_GetTextAreaWidth(props, width);
    m_LabelPrefH = x_PreferredLabelHeight(dc, props, text_w);
    m_PrefsDirty = false;
}


int  CMapItem::PreferredHeight(wxDC& dc, SwxMapItemProperties& props, int width)
{
    if(m_PrefsDirty)    {
        x_UpdatePreferredHeights(dc, props, width);
    }

    int icon_h = m_Icon.Ok() ? m_Icon.GetHeight() : 0;
    int pref_h = 0;

    if(props.m_CenterLabelAndIcon)  {
        pref_h = std::max(icon_h, m_LabelPrefH);
    } else {
        int text_h = m_LabelPrefH;
        pref_h = max(icon_h, text_h);
    }

    pref_h += (props.m_Border + props.m_VertMargin) * 2;
    if(m_Separator) {
        pref_h += x_PreferredSeparatorHeight(dc);
    }
    return pref_h;
}


int CMapItem::x_PreferredSeparatorHeight(wxDC& dc) const
{
    return kSeparatorH;
}


int CMapItem::x_PreferredLabelHeight(wxDC& dc, SwxMapItemProperties& props, int text_w) const
{
    int label_h = 0;
    if(m_Label.size())  {
        dc.SetFont(props.m_Font);

        vector<int> line_lens;
        WrapText(dc, m_Label, text_w, line_lens);

        label_h = (int)(line_lens.size() * abs(props.m_Font.GetPixelSize().y));
        if(label_h > 0)  {
            label_h += 2 * props.m_VertMargin;
        }
    }
    return label_h;
}


int CMapItem::x_CalculateRealTextWidth(wxDC& dc, SwxMapItemProperties& props)
{
    string text = m_Label;
    wxRect rc = m_LabelRect;
    int max_w = 0;

    int y = rc.GetTop() + props.m_VertMargin;
    int text_bottom = rc.GetBottom() - props.m_VertMargin;;

    int d_h = abs(dc.GetFont().GetPixelSize().y);
    int w = rc.GetWidth()- 2 * props.m_HorzMargin;

    // wrap text
    vector<int> line_lens;
    WrapText(dc, text, w, line_lens);

    int max_lines = max(0, rc.GetHeight() / d_h);
    max_lines = min(max_lines, (int) line_lens.size());

    y -= d_h / 5;
    bool text_truncated = (max_lines < (int) line_lens.size());
    int pos = 0;
    for( int i = 0;  i < max_lines  &&  y + d_h < text_bottom;  i++ )  {
        int len = line_lens[i]; // line length in chars
        const char* line_s = text.c_str() + pos; // line text

        bool last_line = (y + 2 * d_h > text_bottom);
        wxString wx_text;
        if(text_truncated  &&  last_line)   {
            wxString wx_s = ToWxString(line_s, len);
            wx_text = TruncateText(dc, wx_s, w, ewxTruncate_EllipsisAlways);
        } else {
            wx_text = ToWxString(line_s, len);
        }
        wxSize size = dc.GetTextExtent(wx_text);
        max_w = std::max(max_w, size.x);

        y += d_h;
        pos += len;
    }
    return max_w + 2 * props.m_HorzMargin;
}


bool CMapItem::IsGroupSeparator() const
{
    return m_Separator;
}


void CMapItem::SetRect(const wxRect& rc)
{
    m_Rect = rc;
}


void CMapItem::GetRect(wxRect& rc) const
{
    rc = m_Rect;
}


void CMapItem::Draw(wxDC& dc, int state, SwxMapItemProperties& props)
{
    wxRect rc_back(m_Rect);
    //rc_back.Inflate(-props.m_HorzMargin , -props.m_VertMargin);
    rc_back.Inflate(-props.m_Border, -props.m_Border);

    // draw focus rectangle
    x_DrawFocusRect(dc, m_Rect, state, props);

    // Fill background
    x_DrawBackground(dc, rc_back, state, props);

    bool selected = (state & CSelectionControl::fItemSelected) != 0;

    // Draw separator
    if(m_Separator) {
        wxRect rc_sep(rc_back);
        rc_sep.SetHeight(kSeparatorH);
        x_DrawSeparator(dc, rc_sep, props);
    }

    // Draw Icon
    if(m_Icon.Ok()) {
        dc.DrawBitmap(m_Icon, m_IconPos.x, m_IconPos.y, true);
    }

    // draw the Label
    if( ! m_Label.empty()  &&  m_LabelRect.width > 0  &&  m_LabelRect.height > 0)  {
        bool widget_focused = (state & CSelectionControl::fWidgetFocused) != 0;

        if(selected)    {
            // draw background
            dc.SetPen(*wxTRANSPARENT_PEN);
            wxBrush brush(props.m_SelBackColor);
            dc.SetBrush(brush);
            wxRect rc_sel(m_LabelRect);
            dc.DrawRectangle(rc_sel.x, rc_sel.y, rc_sel.width, rc_sel.height);
        }

        dc.SetFont(props.m_Font);
        x_DrawText(dc, m_Label, m_LabelRect, selected, widget_focused, false, props);
    }
}


void CMapItem::x_DrawFocusRect(wxDC& dc, const wxRect& rc, int state, SwxMapItemProperties& props)
{
    if(state & CSelectionControl::fItemFocused) {
        const wxBrush& old_brush = dc.GetBrush();
        wxBrush brush(props.m_TextColor, wxBRUSHSTYLE_STIPPLE);
        dc.SetBrush(brush);
        dc.DrawRectangle(m_Rect.x, m_Rect.y, m_Rect.width, m_Rect.height);
        dc.SetBrush(old_brush);
    }
}


void CMapItem::x_DrawBackground(wxDC& dc, const wxRect& rc, int state, SwxMapItemProperties& props)
{
#if defined(NCBI_OS_DARWIN) & 0
    //! TODO Draw theme-compliant background for the item
    /*
    Rect bounds;
    SetRect (&bounds, rc_back.Left(), rc_back.Top(),
                      rc_back.Right(), rc_back.Bottom());
    DrawThemePlacard(&bounds, kThemeStateActive);
    */
#else
    wxBrush brush( props.m_BackColor );
    dc.SetBrush(brush);
    dc.DrawRectangle( rc.x, rc.y, rc.width, rc.height );
#endif
}


void CMapItem::x_DrawSeparator(wxDC& dc, const wxRect& rc, SwxMapItemProperties& props)
{
    wxColour sep_color = GetAverage(props.m_BackColor, props.m_TextColor, 0.5);
    wxPen pen(sep_color);
    dc.SetPen(pen);

    int y_line = rc.GetTop() + kSepVertOffset;
    int left = rc.GetLeft() + kSepHorzOffset;
    int right = rc.GetRight() - kSepHorzOffset;

    // draw 3 horizontal lines
    dc.DrawLine(left, y_line, right, y_line);
    y_line += 2;
    dc.DrawLine(left, y_line, right, y_line);
    y_line += 2;
    dc.DrawLine(left, y_line, right, y_line);
}


void CMapItem::x_DrawText(wxDC& dc, const string& text, const wxRect& rc,
                          bool selected, bool focused, bool highlighted, SwxMapItemProperties& props)
{
    const int x_off = props.m_HorzMargin;
    int x = rc.GetLeft() + x_off;
    int y = rc.GetTop() + props.m_VertMargin;
    int text_bottom = rc.GetBottom() - props.m_VertMargin;

    int d_h = abs(dc.GetFont().GetPixelSize().y);

    int w = rc.GetWidth()- 2 * x_off;

    // wrap text
    vector<int> line_lens;
    WrapText(dc, text, w, line_lens);

    int max_lines = max(0, rc.GetHeight() / d_h);
    max_lines = min(max_lines, (int) line_lens.size());
    //int height = max_lines * d_h;

    wxColour color_text = props.m_TextColor;
    if (selected  &&  focused)
        color_text = props.m_SelTextColor;
    else if (highlighted) {
        color_text = props.m_HiTextColor;
        dc.SetPen(wxPen(props.m_HiTextColor, 1));
    }

    dc.SetTextForeground(color_text);

    y -= d_h / 5;
    bool text_truncated = (max_lines < (int) line_lens.size());
    int pos = 0;
    for( int i = 0;  i < max_lines  &&  y + d_h < text_bottom;  i++ )  {
        int len = line_lens[i]; // line length in chars
        const char* line_s = text.c_str() + pos; // line text

        bool last_line = (y + 2 * d_h > text_bottom);
        wxString wx_s = ToWxString(line_s, len);
        if(text_truncated  &&  last_line) {
            wx_s = TruncateText(dc, wx_s, w, ewxTruncate_EllipsisAlways);
        }
        dc.DrawText(wx_s, x, y);

        if (highlighted) {
            wxCoord w, h;
            dc.GetTextExtent(wx_s, &w, &h);
            dc.DrawLine(x, y + h - 1, x + w - 1, y + h - 1);
        }

        y += d_h;
        pos += len;
    }

    if (highlighted)
        dc.SetPen(*wxTRANSPARENT_PEN);
}


void CMapItem::OnDefaultAction()
{
}

END_NCBI_SCOPE
