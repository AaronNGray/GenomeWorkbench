/*  $Id: custom_tree.cpp 42735 2019-04-08 17:41:06Z kachalos $
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
*  and reliability of the software and data,  the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties,  express or implied,  including
*  warranties of performance,  merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Sema
*/

#include <ncbi_pch.hpp>
#include <gui/packages/pkg_sequence_edit/custom_tree.hpp>
#include <wx/dcclient.h>
#include <wx/settings.h>
#include <wx/renderer.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE


CCustomTreeItem::~CCustomTreeItem()
{
    for (size_t i = 0; i < m_List.size(); i++) {
        delete m_List[i];
    }
}


BEGIN_EVENT_TABLE(CCustomTree, wxWindow)
    EVT_PAINT(CCustomTree::OnPaint)
    EVT_SIZE(CCustomTree::OnSize)
    EVT_SCROLLWIN(CCustomTree::OnScroll)
    EVT_MOUSEWHEEL(CCustomTree::OnWheel)
    EVT_SET_FOCUS(CCustomTree::OnSetFocus)
    EVT_KILL_FOCUS(CCustomTree::OnKillFocus)
    EVT_LEFT_DOWN(CCustomTree::OnClick)
    EVT_LEFT_DCLICK(CCustomTree::OnDblClick)
END_EVENT_TABLE()


void CCustomTree::Clear()
{
    for (size_t i = 0; i < m_List.size(); i++) {
        delete m_List[i];
    }
    m_List.clear();
    m_All.clear();
    m_Visible.clear();
    m_Start = 0;
}


void CCustomTree::CollectAll()
{
    m_All.clear();
    for (size_t i = 0; i < m_List.size(); i++) {
        CollectAll(m_List[i]);
    }
}


void CCustomTree::CollectAll(CCustomTreeItem* item)
{
    m_All.push_back(item);
    for (size_t i = 0; i < item->m_List.size(); i++) {
        CollectAll(item->m_List[i]);
    }
}


void CCustomTree::CollectVisible()
{
    m_Visible.clear();
    for (size_t i = 0; i < m_List.size(); i++) {
        CollectVisible(m_List[i]);
    }
}


void CCustomTree::CollectVisible(CCustomTreeItem* item)
{
    m_Visible.push_back(item);
    if (item->IsExpanded()) {
        for (size_t i = 0; i < item->m_List.size(); i++) {
            CollectVisible(item->m_List[i]);
        }
    }
}


void CCustomTree::QueryMetrix()
{
    wxRendererNative& rnd = wxRendererNative::GetDefault();
    wxSize cbSize = rnd.GetCheckBoxSize(this);
    m_CB_width = cbSize.GetWidth();
    m_CB_height = cbSize.GetHeight();
    m_SB_width = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
    wxPaintDC dc(this);
    m_LN_height = dc.GetTextExtent("!").GetY();
    m_MIN_width = m_CB_width + 2;
    m_MIN_height = wxSystemSettings::GetMetric(wxSYS_SMALLICON_Y);
    if (m_MIN_height < m_CB_height + 2) {
        m_MIN_height = m_CB_height + 2;
    }
    if (m_MIN_height < m_LN_height) {
        m_MIN_height = m_LN_height;
    }
}


vector<string> CCustomTree::Split(string str, wxDC& dc, int width)
{
    vector<string> v;
    vector<string> w;
    string q;
    NStr::Split(str, " ", v);
    for (size_t i = 0; i < v.size(); i++) {
        string p = q.empty() ? v[i] : q + " " + v[i];
        if (!q.empty() && dc.GetTextExtent(p).GetWidth() > width) {
            w.push_back(q);
            q = v[i];
            continue;
        }
        q = p;
    }
    if (!q.empty()) {
        w.push_back(q);
    }
    return w;
}


int CCustomTree::ItemHeight(const CCustomTreeItem& item, wxDC& dc, int width)
{
    int x = item.GetLevel() * m_MIN_width + 1;
    x += m_MIN_width;
    x += m_MIN_width + 2;
    vector<string> v = Split(item.GetText(), dc, width - x);
    return m_MIN_height + m_LN_height * (v.size() - 1);
}


int CCustomTree::FullHeight()
{
    if (!m_Fullheight) {
        wxClientDC dc(this);
        const int height = GetClientSize().GetY();
        int width = GetClientSize().GetX();
        if (m_ScrollBar) {
            width += m_SB_width;
        }
        for (size_t i = 0; i < m_Visible.size() && m_Fullheight <= height; i++) {
            CCustomTreeItem& item = *m_Visible[i];
            m_Fullheight += ItemHeight(item, dc, width);
        }
        if (m_Fullheight > height) {
            width -= m_SB_width;
            m_Fullheight = 0;
            for (size_t i = 0; i < m_Visible.size(); i++) {
                CCustomTreeItem& item = *m_Visible[i];
                m_Fullheight += ItemHeight(item, dc, width);
            }
        }
    }
    return m_Fullheight;
}


void CCustomTree::OnPaint(wxPaintEvent & evt)
{
    wxPaintDC dc(this);
    Paint(dc);
}


void CCustomTree::Paint(wxDC& dc)
{
    GetMetrix();
    if (!m_Visible.size()) {
        CollectVisible();
    }
    UpdateScrollbar();
    wxRendererNative& rnd = wxRendererNative::GetDefault();

    int h = 0;
    int yy = - m_Start;
    const int width = GetClientSize().GetX();
    const int height = GetClientSize().GetY();
    for (size_t i = 0; i < m_Visible.size() && yy < height; yy += h, i++) {
        CCustomTreeItem& item = *m_Visible[i];
        int x = item.GetLevel() * m_MIN_width + 1;
        x += m_MIN_width;
        x += m_MIN_width + 2;
        vector<string> v = Split(item.GetText(), dc, width - x);
        h = m_MIN_height + m_LN_height * (v.size() - 1);
        if (yy + h <= 0) {
            continue;
        }
        bool highlited = (&item == m_Current);
        if (&item == m_Current) {
            dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
            wxBrush brush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
            dc.SetBrush(brush);
            dc.DrawRectangle(0, yy, width, h);
        }
        else {
            dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
        }
        x = item.GetLevel() * m_MIN_width + 1;
        int y = yy + (m_MIN_height - m_CB_height) / 2;
        if (item.m_List.size()) {
            wxRect rect(x, y, m_CB_width, m_CB_height);
            DrawExpander(dc, rect, item.IsExpanded(), highlited);
        }

		if (m_DrawCB) {
			x += m_MIN_width;
			int flags = item.IsChecked() ? wxCONTROL_CHECKED : 0;
			flags |= item.IsActive() ? 0 : wxCONTROL_DISABLED;
			rnd.DrawCheckBox(this, dc, wxRect(x, y, m_CB_width, m_CB_height), flags);
		}

        x += m_MIN_width + 2;
        y = yy + (m_MIN_height - m_LN_height) / 2;

        for (auto& s : v) {
            dc.DrawText(s, x, y);
            y += m_LN_height;
        }
    }
}


void CCustomTree::DrawExpander(wxDC& dc, wxRect& rect, bool open, bool highlited)
{
    wxPen pen(highlited ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) : wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    dc.SetPen(pen);
    if (open) {
        int x0 = rect.GetLeft() + 1;
        int x1 = rect.GetRight() - 1;
        int xm = (x0 + x1) / 2;
        int y0 = rect.GetTop() + (rect.GetHeight() + 2) / 4;
        int y1 = y0 + rect.GetHeight() / 2;
        dc.DrawLine(x0, y0, x1, y0);
        dc.DrawLine(x1, y0, xm, y1);
        dc.DrawLine(xm, y1, x0, y0);
    }
    else {
        int x0 = rect.GetLeft() + (rect.GetWidth() + 2) / 4;
        int x1 = x0 + rect.GetWidth() / 2;
        int y0 = rect.GetTop() + 1;
        int y1 = rect.GetBottom() - 1;
        int ym = (y0 + y1) / 2;
        dc.DrawLine(x0, y0, x1, ym);
        dc.DrawLine(x1, ym, x0, y1);
        dc.DrawLine(x0, y1, x0, y0);
    }
}


void CCustomTree::OnSetFocus(wxFocusEvent& evt)
{

}

void CCustomTree::OnKillFocus(wxFocusEvent& evt)
{

}

void CCustomTree::OnClick(wxMouseEvent& evt)
{
    wxClientDC dc(this);
    int x = evt.GetX();
    int y = evt.GetY();
    int yy = - m_Start;
    int h = 0;
    const int width = GetClientSize().GetX();
    const int height = GetClientSize().GetY();
    bool check = false;
    const CCustomTreeItem* current = m_Current;
    for (size_t i = 0; i < m_Visible.size() && yy < height; yy += h, i++) {
        CCustomTreeItem& item = *m_Visible[i];
        h = ItemHeight(item, dc, width);
        if (y < yy || y >= yy + h) {
            continue;
        }
        int cx = item.GetLevel() * m_MIN_width + 1;
        int cy = yy + (m_MIN_height - m_CB_height) / 2;
        if (x >= cx && x < cx + m_CB_width && y >= cy && y < cy + m_CB_height) {
            ExpandItem(&item, !item.m_Expanded);
        }
        else if (m_DrawCB && x >= cx + m_MIN_width && x < cx + m_MIN_width + m_CB_width && y >= cy && y < cy + m_CB_height) {
            CheckItem(&item, !item.m_Checked);
            check = true;
        }
        m_Current = &item;
        Refresh();
        break;
    }

    if (check || current != m_Current) {
        wxCommandEvent e(wxEVT_LISTBOX, GetId());
        e.SetEventObject(GetParent());
        ProcessEvent(e);
    }
}


void CCustomTree::OnDblClick(wxMouseEvent& evt)
{
    wxCommandEvent e(wxEVT_LISTBOX_DCLICK, GetId());
    e.SetEventObject(GetParent());
    ProcessEvent(e);
}


void CCustomTree::ExpandItem(CCustomTreeItem* item, bool expand)
{
    item->m_Expanded = expand;
    m_Fullheight = 0;
    CollectVisible();
    UpdateScrollbar();
}


void CCustomTree::CheckItem(CCustomTreeItem* item, bool check, bool propagate)
{
    item->m_Checked = check;
    if (propagate) {
        PropagateCheck(item, check);
        CheckParent(item->m_Parent, check);
    }
}


void CCustomTree::PropagateCheck(CCustomTreeItem* item, bool check)
{
    item->m_Checked = check;
    for (auto& it : item->m_List) {
        PropagateCheck(it, check);
    }
}


void CCustomTree::CheckParent(CCustomTreeItem* item, bool check)
{
    if (!item) {
        return;
    }
    if (!check) {
        item->m_Checked = false;
        CheckParent(item->m_Parent, false);
        return;
    }
    for (auto& it : item->m_List) {
        if (!it->m_Checked) {
            return;
        }
    }
    item->m_Checked = true;
    CheckParent(item->m_Parent, true);
}


void CCustomTree::UpdateScrollbar()
{
    const int height = GetClientSize().GetY();
    const int fullheight = FullHeight();
    if (fullheight <= height) {
        m_Start = 0;
        if (m_ScrollBar) {
            m_ScrollBar = false;
            SetScrollbar(wxVERTICAL, 0, 0, 0);
        }
    }
    else {
        if (m_Start + height > fullheight) {
            m_Start = fullheight - height;
        }
        if (m_Start < 0) {
            m_Start = 0;
        }
        m_ScrollBar = true;
        SetScrollbar(wxVERTICAL, m_Start, height, fullheight);
    }
}


void CCustomTree::OnScroll(wxScrollWinEvent& evt)
{
    const wxEventType& type = evt.GetEventType();
    const int height = GetClientSize().GetY();
    if (type == wxEVT_SCROLLWIN_TOP) {
        m_Start = 0;
        Refresh();
    }
    else if (type == wxEVT_SCROLLWIN_BOTTOM) {
        m_Start = FullHeight() - height;
        Refresh();
    }
    else if (type == wxEVT_SCROLLWIN_LINEUP) {
        m_Start -= m_MIN_height;
        Refresh();
    }
    else if (type == wxEVT_SCROLLWIN_LINEDOWN) {
        m_Start += m_MIN_height;
        Refresh();
    }
    else if (type == wxEVT_SCROLLWIN_PAGEUP) {
        m_Start -= height;
        Refresh();
    }
    else if (type == wxEVT_SCROLLWIN_PAGEDOWN) {
        m_Start += height;
        Refresh();
    }
    else if (type == wxEVT_SCROLLWIN_THUMBTRACK || type == wxEVT_SCROLLWIN_THUMBRELEASE) {
        if (evt.GetPosition() >= 0) {
            m_Start = evt.GetPosition();
            Refresh();
        }
    }
}


void CCustomTree::OnWheel(wxMouseEvent& evt)
{
    int delta = evt.GetWheelDelta();
    m_Start -= m_MIN_height * (delta ? evt.GetWheelRotation() / delta : evt.GetWheelRotation());
    UpdateScrollbar();
    Refresh();
}


void CCustomTree::SetCurrentItem(CCustomTreeItem* p)
{
    m_Current = p;
    for (CCustomTreeItem* up = p->m_Parent; up; up = up->m_Parent) {
        up->m_Expanded = true;
    }
    CollectVisible();
    UpdateScrollbar();
    Refresh();
}


END_NCBI_SCOPE
