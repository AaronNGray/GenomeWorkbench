/*  $Id: map_control.cpp 44189 2019-11-14 14:51:36Z katargir $
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

#include <gui/widgets/wx/map_control.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/popup_event.hpp>

#include <wx/dcbuffer.h>
#include <wx/settings.h>

#include <math.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CMapControl

static const int kDefaultColumnW = 160; /// default column width
static const int kMaxItemH = 64;

BEGIN_EVENT_TABLE(CMapControl, CMapControl::TParent)
    EVT_PAINT(CMapControl::OnPaint)
    EVT_SIZE(CMapControl::OnSize)
    EVT_CONTEXT_MENU(CMapControl::OnContextMenu)    
    EVT_SET_FOCUS(CMapControl::OnFocusChanged)
    EVT_KILL_FOCUS(CMapControl::OnFocusChanged)

    EVT_LEFT_DOWN(CMapControl::OnMouseDown)
    EVT_RIGHT_DOWN(CMapControl::OnMouseDown)
    EVT_LEFT_DCLICK(CMapControl::OnLeftDoubleClick)
    EVT_KEY_DOWN(CMapControl::OnKeyDown)
    EVT_MOUSEWHEEL(CMapControl::OnMouseWheel)
    EVT_MOTION(CMapControl::OnMouseMove)
    EVT_LEAVE_WINDOW(CMapControl::OnMouseLeave)
END_EVENT_TABLE()


CMapControl::SProperties::SProperties()
:   m_ColumnWidth(kDefaultColumnW),
    m_MaxItemHeight(kMaxItemH),
    m_SepLineWidth(2),
    m_SepLineVertOffset(2),
    m_ItemOffsetX(2),
    m_ItemOffsetY(2),
    m_SeparateGroups(true),
    m_SingleColumn(false),
    m_SizePolicy(eAdjustVertSize)
{
}


CMapControl::CMapControl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                             const wxSize& size, long style, const wxString& name)
:   TParent(parent, id, pos, size, style, name),
    m_HotItem(-1)
{
    x_Init();
}


void CMapControl::x_Init()
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    SetScrollRate( 16, 16 );

    m_UpdateLockCounter = 0;

    wxColour cl_back = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    SetBackgroundColour(cl_back);

    // Setup item rendering properties
    m_ItemProps.m_Font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    m_ItemProps.m_MinorFont = m_ItemProps.m_Font;
    m_ItemProps.m_Font.SetWeight(wxFONTWEIGHT_BOLD);

    m_ItemProps.m_TextColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    m_ItemProps.m_BackColor = GetAverage(m_ItemProps.m_TextColor, cl_back, 0.07f);
}


CMapControl::~CMapControl()
{
    LockUpdates(true);

    DeleteAllItems();

    LockUpdates(false);
}


void CMapControl::SetColumnWidth(int w, bool update)
{
    _ASSERT(w >= 0);
    m_Properties.m_ColumnWidth = max(0, w);

    if(update)  {
        Layout();
    }
}

void CMapControl::SetMaxItemHeight(int h, bool update)
{
    _ASSERT(h >= 0);
    m_Properties.m_MaxItemHeight = max(0, h);

    if(update)  {
        Layout();
    }
}


SwxMapItemProperties& CMapControl::GetMapItemProperties()
{
    return m_ItemProps;
}


void CMapControl::LockUpdates(bool lock)
{
    m_UpdateLockCounter += (lock ? 1 : -1);
    if(m_UpdateLockCounter == 0)    {
        Layout();
    }
}


// apply default action to all selected items
void CMapControl::x_DoDefaultActionOnSelected()
{
    //LOG_POST("CMapControl::x_DoDefaultActionOnSelected()");
    // assuming that single click was handled by CSelectionControl
    TIndexVector indexes;
    GetSelectedIndexes(indexes);
    for( size_t i = 0; i < indexes.size();  i++ )   {
        TIndex ind = indexes[i];
        IwxMapItem* item  = x_GetItem(ind);
        if(item)    {
            item->OnDefaultAction();
        }
    }
}

void CMapControl::x_HorzMoveSelectionBy(int shift, CGUIEvent::EGUIState state)
{
    TIndex i_focused = GetFocusedIndex();
    if(i_focused >= 0)  {
        wxRect rc_foc;
        m_Items[i_focused]->GetRect(rc_foc);

        // find an element in the columnt to the right (left)
        TIndex col_start_index  = -1, to_index = -1;
        if(shift > 0)   {
            wxRect rc;
            for( TIndex i = i_focused;  i < (TIndex) m_Items.size(); i++ ) {
                m_Items[i]->GetRect(rc);
                if(rc.GetLeft() >= rc_foc.GetRight())    {
                    col_start_index = i;
                    break;
                }
            }
        } else {
            // find item to the left that is not lower than focused one
            wxRect rc;
            for( TIndex i = i_focused;  i >= 0; i-- ) {
                m_Items[i]->GetRect(rc);
                if(rc.GetRight() < rc_foc.GetLeft()  &&  rc.GetTop() <= rc_foc.GetTop())    {
                    col_start_index = i;
                    break;
                }
            }
        }

        if(col_start_index >= 0)   {
            // have found a column - now skip down to the element the intersects
            // the focused element vertically
            to_index = col_start_index;
            wxRect rc;
            for(  ;  to_index < (TIndex) m_Items.size();  to_index++  )    {
                m_Items[to_index]->GetRect(rc);
                if(! (rc.GetTop() > rc_foc.GetBottom()  ||  rc.GetBottom() < rc_foc.GetTop()))  {
                    break; // intersects rec vertically
                }
            }
            if(to_index == (int)m_Items.size())   {
                to_index = (int)(m_Items.size() - 1);
            }
        }

        if(to_index != -1)  {
            // we found appropriate item - select it (or the whole range)
            x_SelectTo(to_index, state);
        }
    }
}


void CMapControl::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);

    // fill background
    wxRect rc = GetClientSize();
    wxBrush brush(GetBackgroundColour());
    dc.SetBrush(brush);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(rc.x, rc.y, rc.width, rc.height);

	PrepareDC( dc );

	x_InitItemProperties();

    x_DrawSeparationLines(dc);

    x_DrawItemsRange(dc, 0, (TIndex)(m_Items.size() - 1));

    dc.SetDeviceOrigin(0, 0);
}


void CMapControl::x_InitItemProperties()
{
    bool focused = (wxWindow::FindFocus() == this);

    wxSystemColour type = focused ? wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT;
    m_ItemProps.m_SelTextColor = wxSystemSettings::GetColour(type);

    //type = focused ? eSysColor_FocusedBack : eSysColor_SelectedBack;
    type = focused ? wxSYS_COLOUR_HIGHLIGHT : wxSYS_COLOUR_BTNFACE;
    m_ItemProps.m_SelBackColor = wxSystemSettings::GetColour(type);
}


void CMapControl::x_DrawSeparationLines(wxDC& dc)
{
    if(m_Properties.m_SingleColumn) {
        return; // do not need to draw
    }

    int sep_w = m_Properties.m_SepLineWidth;
    if(sep_w > 0) {
        int origin_x = 0, origin_y = 0, size_x, size_y;
        GetViewStart(&origin_x, &origin_y);
        GetVirtualSize(&size_x, &size_y);

        int step = m_Properties.m_ColumnWidth;
        int start = step * (int) (floor( double(origin_x + step) / step));
        int end = step * (int) (floor( double(origin_x + size_x - 1) / step));

        start  -= sep_w;
        end  -= sep_w;

        wxRect rc = GetClientRect();
        int off_y = m_Properties.m_SepLineVertOffset;
        int y1 = rc.y + off_y;
        int line_h = rc.height - 2 * off_y;

        wxColour cl = GetAverage(GetBackgroundColour(), m_ItemProps.m_TextColor, 0.8f);
        wxBrush brush(cl);
        dc.SetBrush(brush);
        dc.SetPen(*wxTRANSPARENT_PEN);

        for( int xx = start;  xx <= end;  xx += step  ) {
            dc.DrawRectangle(xx, y1, sep_w, line_h);
        }
    }
}


void CMapControl::OnSize(wxSizeEvent& event)
{
    Layout();
}


CMapControl::TIndex CMapControl::GetItemsCount() const
{
    return (CMapControl::TIndex)m_Items.size();
}


CMapControl::TItemRef CMapControl::GetItem(TIndex index)
{
    bool valid = x_AssertIndexValid(index);
    return valid ? m_Items[index] : TItemRef();
}


CMapControl::TCItemRef CMapControl::GetItem(TIndex index) const
{
    TCItemRef ref;
    if(x_AssertIndexValid(index))   {
        ref = m_Items[index];
    }
    return ref;
}


void CMapControl::AddItem(IwxMapItem* item)
{
    if(item)    {
        m_Items.push_back(TItemRef(item));
        TIndex index = (CMapControl::TIndex)(m_Items.size() - 1);
        CSelectionControl::x_InsertItem(index, item, false);

        if(! x_IsUpdatesLocked())   {
            Layout();
            x_UpdateItemsRange(index, index);
        }
    }
}


void CMapControl::InsertItem(TIndex index, IwxMapItem* item)
{
    bool valid = (index >=0  && index <= (TIndex) m_Items.size());
    _VERIFY(valid);

    if(item)    {
        m_Items.insert(m_Items.begin() + index, TItemRef(item));
        CSelectionControl::x_InsertItem(index, item, false);

        if(! x_IsUpdatesLocked())   {
            Layout();
            x_UpdateItemsRange(index, (CMapControl::TIndex)m_Items.size() - 1);
        }
    }
}


void CMapControl::DeleteItem(TIndex index)
{
    bool valid = x_AssertIndexValid(index);
    if(valid)    {
        if(m_HotItem == index)  {
            m_HotItem = -1;
            SetCursor(wxCursor(wxCURSOR_DEFAULT));
        }
        x_DeleteItem(index, false); // delete from CSelectionControl

        m_Items.erase(m_Items.begin() + index);

        if(! x_IsUpdatesLocked())   {
            Layout();
            x_UpdateItemsRange(index, (TIndex)m_Items.size() - 1);
        }
    }
}


void CMapControl::DeleteItem(IwxMapItem& item)
{
    TIndex index = GetItemIndex(item);
    DeleteItem(index);

    if(! x_IsUpdatesLocked())   {
        Layout();
    }
}


void CMapControl::DeleteAllItems()
{
    m_HotItem = -1;
    SetCursor(wxCursor(wxCURSOR_DEFAULT));

    m_Items.clear();
    x_DeleteAllItems();

    if(! x_IsUpdatesLocked())   {
        Layout();
    }
}


CMapControl::TIndex CMapControl::GetItemIndex(IwxMapItem& item) const
{
    for( size_t i = 0;  i < m_Items.size();  i++ )   {
        if(x_GetItem((TIndex)i).GetPointer() == &item)    {
            return (TIndex)i;
        }
    }
    return -1;
}


CMapControl::TItemRef CMapControl::x_GetItem(TIndex index)
{
    TItemRef ref;
    if(x_AssertIndexValid(index))   {
        ref = m_Items[index];
    }
    return ref;
}


CMapControl::TCItemRef CMapControl::x_GetItem(TIndex index) const
{
    TCItemRef ref;
    if(x_AssertIndexValid(index))   {
        ref = m_Items[index];
    }
    return ref;
}


bool CMapControl::x_IsUpdatesLocked() const
{
    return m_UpdateLockCounter > 0;
}


void CMapControl::x_DrawItemsRange(wxDC& dc, TIndex from, TIndex to)
{
    int focused = (wxWindow::FindFocus() == this) ?
                  CSelectionControl::fWidgetFocused : 0;

    int origin_x, origin_y;
    GetViewStart(&origin_x, &origin_y);
    wxRect rc;

    for( TIndex i = from;  i <= to;  i++ )    {
        int state = GetItemState(i) | focused;
        TItemRef item = m_Items[i];
        item->GetRect(rc);
        item->Draw(dc/*, rc*/, state, m_ItemProps);
    }
}


bool CMapControl::Layout()
{
    wxSize old_virt_size = GetVirtualSize();
    wxSize sz = GetClientSize();

    wxSize new_virt_sz = x_CalculateLayout(sz.x, sz.y);
    if(new_virt_sz != old_virt_size)    {
        SetVirtualSize(new_virt_sz);
        Refresh();
    }

    return true;
}


wxSize CMapControl::x_CalculateLayout(int width, int height)
{
    int item_x = 0;
    int item_y = 0;
    int max_y = height - 1;

    const int full_sep_w = m_Properties.m_SepLineWidth + 2 * m_Properties.m_ItemOffsetX;
    const int item_w = (m_Properties.m_SingleColumn)
            ?   width - 2 * m_Properties.m_ItemOffsetX
            :   m_Properties.m_ColumnWidth - full_sep_w;

    wxSize sz(0, 0);
    wxClientDC dc(this);

    for( size_t i = 0;  i < m_Items.size();  i++ )   {
        IwxMapItem& item = *m_Items[i];

        int pref_h = item.PreferredHeight(dc, m_ItemProps, item_w);
        int real_h = min(pref_h, m_Properties.m_MaxItemHeight);

        bool column_full = (item_y + real_h > max_y)  &&  (item_y > 0);
        bool new_group = (item.IsGroupSeparator())  &&  (i > 0);

        // start a new column if needed
        if( ! m_Properties.m_SingleColumn)  {
            if(column_full  ||  (new_group  &&  m_Properties.m_SeparateGroups))  {
                // start a new column
                item_y = 0;
                item_x += m_Properties.m_ColumnWidth;
            }
        }

        wxRect rc;
        rc.SetLeftTop(wxPoint(item_x + m_Properties.m_ItemOffsetX,
                              item_y + m_Properties.m_ItemOffsetY));
        rc.SetSize(wxSize(item_w, real_h));
        item.SetRect(rc);

        item.Layout(dc, m_ItemProps);

        sz.x = max(sz.x, item_x + m_Properties.m_ColumnWidth);
        sz.y = max(sz.y, item_y + real_h);

        item_y += real_h + 2 * m_Properties.m_ItemOffsetY;
    }
    return sz;
}


void CMapControl::x_SendSelectionEvent(TIndex index)
{
    wxCommandEvent event(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
    event.SetEventObject(this);
    event.SetInt(index);

    (void)GetEventHandler()->ProcessEvent(event);
}


void CMapControl::x_UpdateItems(TIndexVector& indexes)
{
    Refresh(); //TODO optimize
}


void CMapControl::x_UpdateItemsRange(TIndex start, TIndex end)
{
    wxRect rc_draw;
    for( TIndex i = start; i <= end; i++ )  {
        wxRect rc;
        m_Items[i]->GetRect(rc);
        if(i == start)  {
            rc_draw = rc;
        } else {
            rc_draw.Union(rc);
        }
    }

    wxPoint pos = GetPosition();
    int origin_x, origin_y;
    GetViewStart(&origin_x, &origin_y);

    rc_draw.Offset(pos.x - origin_x, pos.y - origin_x );

    if( ! rc_draw.IsEmpty())   {
        Refresh(); // TODO - optimize
    }
}


int CMapControl::x_GetIndexByWindowPos(int x, int y, bool clip) //TODO - clip
{
    CalcUnscrolledPosition(x, y, &x, &y);

    wxRect rc;
    for( size_t i = 0; i < m_Items.size(); i++ ){
        m_Items[i]->GetRect(rc);
        if( rc.Contains( x, y ) ){
            return (TIndex)i;
        }
    }
    return -1;
}


void CMapControl::x_MakeVisible(TIndex index)
{
    bool valid = x_AssertIndexValid(index);
    if(valid)   {
        wxRect rc;
        m_Items[index]->GetRect(rc);

        bool update = false;

        wxSize sz = GetSize();
        int origin_x, origin_y;
        GetViewStart(&origin_x, &origin_y);

        if(rc.GetLeft() < origin_x)    {
            origin_x = rc.GetLeft();
            update = true;
        } else if(rc.GetRight() >= origin_x + sz.x)  {
            origin_y  = rc.GetRight() - sz.x + 1;
            update = true;
        }
        if(rc.GetTop() < origin_y)    {
            origin_y = rc.GetTop();
            update = true;
        } else if(rc.GetBottom() >= origin_y + sz.y)  {
            origin_y  = rc.GetBottom() - sz.y + 1;
            update = true;
        }
        Scroll(origin_x, origin_y);
    }
}

/* TODO this functionality needs to be rewritten wxWidgets-style
void CMapControl::x_OnShowPopupMenu()
{
    CMenuItem* root = x_CreatePopupMenu();
    if(root  &&  ! root->IsSubmenuEmpty())  {
        CPopupMenu  menu(root, this);
        menu.Popup();
    } else {
        delete root;
    }
}


// override this function in derived classes
CMenuItem* CMapControl::x_CreatePopupMenu()
{
    CPopupMenuEvent evt(this, new CMenuItem("Root")); // no menu by default

    // let parents provide the menu
    Send(&evt, eDispatch_Default, ePool_Parent);
    return evt.GetRootItem();
}
*/

void CMapControl::OnContextMenu(wxContextMenuEvent& event)
{
    LOG_POST("CMapControl::OnContextMenu()");
}


void CMapControl::OnMouseDown(wxMouseEvent& event)
{
    SetFocus();
    CSelectionControl::OnMouseDown(event);

    wxPoint ms_pos = event.GetPosition();
    int index = x_GetIndexByWindowPos(ms_pos.x, ms_pos.y, true);
    CalcUnscrolledPosition(ms_pos.x, ms_pos.y, &ms_pos.x, &ms_pos.y);

    if(index != -1) {
        IwxMapItem* item  = x_GetItem(index);
        item->OnMouseDown(ms_pos);
    }

    x_UpdateHotItem(event.GetPosition());
}


void CMapControl::OnLeftDoubleClick(wxMouseEvent& event)
{
    wxPoint ms_pos = event.GetPosition();
    x_UpdateHotItem(ms_pos);

    int index = x_GetIndexByWindowPos(ms_pos.x, ms_pos.y, true);
    CalcUnscrolledPosition(ms_pos.x, ms_pos.y, &ms_pos.x, &ms_pos.y);

    if(index != -1) {
        IwxMapItem* item  = x_GetItem(index);
        item->OnLeftDoubleClick(ms_pos);
    }
}


void CMapControl::OnMouseUp(wxMouseEvent& event)
{
    //CSelectionControl::OnMouseUp(event);
    event.Skip();
}


void CMapControl::OnMouseWheel(wxMouseEvent& event)
{
    int shift = event.GetWheelRotation() / event.GetWheelDelta();

    int origin_x, origin_y;
    GetViewStart(&origin_x, &origin_y);

    int old_origin_x = origin_x;
    int old_origin_y = origin_y;

    switch(m_Properties.m_SizePolicy)   {
    case eAdjustHorzSize:    {{
        if(m_Properties.m_SingleColumn) {
            int y = origin_y;

            if(shift > 0)   {
                wxRect rc;
                for( size_t i = 0;  i < m_Items.size();  i++ )   {
                    m_Items[i]->GetRect(rc);
                    if(rc.GetBottom() > y) {
                        // get next item and scroll to it
                        if(++i < m_Items.size()) {
                            wxRect rc_next;
                            m_Items[i]->GetRect(rc_next);
                            origin_y += rc_next.GetTop();
                        }
                        break;
                    }
                }
            } else {
                wxRect rc;
                for( size_t i = m_Items.size();  i > 0;  )   {
                    --i;
                    m_Items[i]->GetRect(rc);
                    if(rc.GetTop() < y) {
                        origin_y = rc.GetTop();
                        break;
                    }
                }
            }
        }
        break;
    }}
    case eAdjustVertSize:
        CSelectionControl::MoveSelectionBy(-shift, CGUIEvent::eSelectState);
        return;
/*
        if(shift > 0)   {
            origin_x += m_Properties.m_ColumnWidth;
        } else {
            origin_x -= m_Properties.m_ColumnWidth;
        }
        break;
*/
    default:
        break;
    }

    if(origin_x != old_origin_x  ||  origin_y != old_origin_y)  {
        Scroll(origin_x, origin_y);
        Refresh();
    }
}

CSelectionControl::TIndex CMapControl::GetSelectedIndex() const
{
    return CSelectionControl::GetSelectedIndex();
}

void CMapControl::OnMouseMove(wxMouseEvent& event)
{
    x_UpdateHotItem(event.GetPosition());
}

void CMapControl::x_UpdateHotItem(wxPoint ms_pos)
{
    int index = x_GetIndexByWindowPos(ms_pos.x, ms_pos.y, true);
    CalcUnscrolledPosition(ms_pos.x, ms_pos.y, &ms_pos.x, &ms_pos.y);
    if(index != -1) {
        IwxMapItem* item  = x_GetItem(index);
        bool hot = item->OnHotTrack(ms_pos);
        if(hot) {
            if(index != m_HotItem)  {
                // clear old hot track
                if(m_HotItem != -1) {
                    GetItem(m_HotItem)->OnHotTrack(ms_pos);
                }
                m_HotItem = index;
                Refresh();
                SetCursor(wxCursor(wxCURSOR_HAND));
                UpdateSelection();
            }
            return;
        }
    }

    if (m_HotItem != -1) {
        GetItem(m_HotItem)->OnHotTrack(ms_pos);
        m_HotItem = -1;
        Refresh();
        SetCursor(wxCursor(wxCURSOR_DEFAULT));
        UpdateSelection();
    }
}

void CMapControl::OnMouseLeave(wxMouseEvent& WXUNUSED(event))
{
    x_UpdateHotItem(wxPoint(-1, -1));
}

void CMapControl::UpdateSelection()
{
    x_SendSelectionEvent(m_HotItem);
}

void CMapControl::OnKeyDown(wxKeyEvent& event)
{
    int key = event.GetKeyCode();

    switch(key) {
    case WXK_LEFT:
    case WXK_RIGHT: {
        CGUIEvent::EGUIState state = CGUIEvent::wxGetSelectState(event);
        x_HorzMoveSelectionBy((key == WXK_LEFT) ? -1 : 1, state);
        break;
    }
    case WXK_RETURN:
    case WXK_NUMPAD_ENTER:
        x_DoDefaultActionOnSelected();
        break;
    case WXK_TAB:   {
        int flags = 0;
        if (!event.ShiftDown())
            flags |= wxNavigationKeyEvent::IsForward ;
        if (event.ControlDown())
            flags |= wxNavigationKeyEvent::WinChange ;
        Navigate(flags);
        break;
    }
    default:
        CSelectionControl::OnKeyDown(event);
        break;
    }
}


void CMapControl::OnFocusChanged(wxFocusEvent& event)
{
    Refresh();
}


END_NCBI_SCOPE
