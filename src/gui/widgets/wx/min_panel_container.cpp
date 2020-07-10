/*  $Id: min_panel_container.cpp 44189 2019-11-14 14:51:36Z katargir $
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

#include <gui/widgets/wx/min_panel_container.hpp>

#include <gui/widgets/wx/window_manager.hpp>

#include <gui/widgets/wx/dock_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>


#include <wx/dc.h>
#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/menu.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


////////////////////////////////////////////////////////////////////////////////
/// CMinPanelContainer

static const int kRestoreWindowFirst = 15000;
static const int kRestoreWindowLast  = 15100;

BEGIN_EVENT_TABLE(CMinPanelContainer, CMinPanelContainer::TParent)
    EVT_PAINT(CMinPanelContainer::OnPaint)
    EVT_SIZE(CMinPanelContainer::OnSize)
    EVT_LEFT_UP(CMinPanelContainer::OnLeftClick)
    EVT_CONTEXT_MENU(CMinPanelContainer::OnContextMenu) 
    EVT_MENU_RANGE(eCmdCloseDockPanel, eCmdWindowRestore,
                   CMinPanelContainer::OnDockPanelCommand)
    EVT_MENU_RANGE(kRestoreWindowFirst, kRestoreWindowLast,
                   CMinPanelContainer::OnRestoreWindow)
    EVT_UPDATE_UI_RANGE(eCmdCloseDockPanel, eCmdWindowRestore,
                   CMinPanelContainer::OnUpdateWindowCommand)

    EVT_MENU(eCmdShowWindowsDlg, CMinPanelContainer::OnShowWindowsDlg)
END_EVENT_TABLE()


CMinPanelContainer::CMinPanelContainer(wxWindow* parent, CDockManager& manager)
:   TParent(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, _("min_panel_container")),
    m_DockManager(manager),
    m_PrefSize(0, 0),
    m_MenuPanel(NULL)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    m_Font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
}


CMinPanelContainer::~CMinPanelContainer()
{
}


void CMinPanelContainer::AddClient(CDockPanel& panel)
{
    SItem* item = new SItem;
    item->m_DockPanel = &panel;
    m_Items.push_back(item);

    m_MenuPanel = &panel;

    panel.Hide();
    panel.Reparent(this);

    InvalidateBestSize();

    Layout();
    GetParent()->Layout();
    Refresh();
}


void CMinPanelContainer::RemoveClient(CDockPanel& panel)
{
    NON_CONST_ITERATE(TItems, it, m_Items)    {
        SItem& item = **it;
        if(item.m_DockPanel == &panel) {
            // delete this item
            panel.Hide();
            m_Items.erase(it);

			if (m_MenuPanel != NULL && m_MenuPanel == &panel) {
				m_MenuPanel = NULL;

				if (!m_Items.empty()) {
					m_MenuPanel = m_Items.back()->m_DockPanel;
				}
			}

            InvalidateBestSize();

            Layout();
            GetParent()->Layout();
            Refresh();
            return;
        }
    }

    _ASSERT(false); // cannot find the panel !
}


bool CMinPanelContainer::HasClient(CDockPanel& panel) const
{
    const SItem* item = x_FindItemByPanel(&panel);
    return item != NULL;
}


void CMinPanelContainer::OnSize(wxSizeEvent& event)
{
    Layout();
    Refresh();
}


const CMinPanelContainer::SItem*
    CMinPanelContainer::x_FindItemByPanel(CDockPanel* panel) const
{
    ITERATE(TItems, it, m_Items)    {
        const SItem* item = *it;
        if(item->m_DockPanel == panel)
            return item;
    }
    return NULL;
}


CMinPanelContainer::SItem* CMinPanelContainer::x_FindItemByPos(const wxPoint& pt)
{
    NON_CONST_ITERATE(TItems, it, m_Items)    {
        SItem* item = *it;
        if(item->m_Rect.Contains(pt))
            return item;
    }
    return NULL;
}


static const int kOffsetX = 2;
static const int kOffsetY = 2;
static const int kMinW = 50;
static const int kMaxW = 200;
static const int kMinH = 20;
static const int kLabelOffsetX = 4;
static const int kLabelOffsetY = 3;
static const int kIconOffX = 3;
static const int kIconOffY = 3;


bool CMinPanelContainer::Layout()
{
    m_PrefSize = wxSize(0, 0);

    wxRect bounds = GetRect();
    bounds.x = bounds.y = 0;
    bounds.Deflate(kOffsetX, kOffsetY);

    bool new_row = true;
    int x = bounds.x, y = bounds.y;

    wxClientDC dc(this);
    dc.SetFont(m_Font);

    // iterate by items
    for( size_t i = 0; i < m_Items.size();  i++  )    {
        SItem& item = *m_Items[i];

        wxSize size = x_MeasureItem(dc, item);

        int right = x + size.x + kOffsetX;
        if(right > bounds.GetRight()  &&  ! new_row) {
            // start a new row
            x = bounds.x;
            y += kMinH + kOffsetY;
            new_row = true;
        }

        // set rectangle
        item.m_Rect = wxRect(x, y, size.x, size.y);
        x = item.m_Rect.GetRight() + kOffsetX;

        m_PrefSize.x = std::max(m_PrefSize.x, x);
        m_PrefSize.y = std::max(m_PrefSize.y, item.m_Rect.GetBottom());

        new_row = false;
    }
    m_PrefSize.x += kOffsetX * 2;
    if(m_PrefSize.y > 0)    {
        m_PrefSize.y += kOffsetY * 2;
    }
    return true;
}


wxSize CMinPanelContainer::x_MeasureItem(wxDC& dc, const SItem& item)
{
    wxSize size;

    const IWMClient* client = item.m_DockPanel->GetClient();

    string alias = client->GetIconAlias();
    wxBitmap bmp = wxArtProvider::GetBitmap(ToWxString(alias));
    if(bmp.IsOk())  {
        size.x += bmp.GetWidth() + kIconOffX * 2;
        size.y += bmp.GetHeight() + kIconOffY * 2;
    }

    /// measure item text
    wxString label = ToWxString(client->GetClientLabel());
    int w = 0, h = 0;
    dc.GetTextExtent(label, &w, &h);
    // add extra spacing around the text
    w += kLabelOffsetX * 2;
    h += kLabelOffsetY * 2;
    size.x = max(size.x, w);
    size.y = max(size.y, h);

    // apply min / max constraints
    size.x = max(kMinW, size.x);
    size.x = min(kMaxW, size.x);
    size.y = min(kMinH, size.y);
    return size;
}


wxSize CMinPanelContainer::DoGetBestSize() const
{
    return m_PrefSize;
}


void CMinPanelContainer::OnPaint(wxPaintEvent& event)
{
    wxRect rc = GetRect();

    wxAutoBufferedPaintDC dc(this);

    // fill background
    wxColour back_cl = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
    wxBrush brush(back_cl);
    dc.SetBrush(brush);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, rc.width, rc.height);

    // draw items
    for( size_t i = 0; i < m_Items.size();  i++  )    {
        SItem& item = *m_Items[i];
        x_DrawItem(dc, item);
    }
}


void CMinPanelContainer::x_DrawItem(wxDC& dc, SItem& item)
{
    const IWMClient* client = item.m_DockPanel->GetClient();
    wxRect& rc = item.m_Rect;

    wxColor cl_text = wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW);
    wxColor cl_line = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
    wxColour back_cl = wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT);

    wxBrush brush(back_cl);
    wxPen pen(cl_line);
    dc.SetBrush(brush);
    dc.SetPen(pen);

    dc.DrawRoundedRectangle(rc.x, rc.y, rc.width, rc.height, 3);

    int pos_x = rc.x;

    // draw Client icon
    string alias = client->GetIconAlias();
    wxBitmap bmp = wxArtProvider::GetBitmap(ToWxString(alias));
    if(bmp.IsOk())  {
        pos_x += kIconOffX;
        int pos_y = rc.y + (rc.height - bmp.GetHeight()) / 2;
        dc.DrawBitmap(bmp, pos_x, pos_y);
        pos_x += bmp.GetWidth() + kIconOffX;
    }

    dc.SetFont(m_Font);

    // truncate label if needed
    pos_x += kLabelOffsetX;
    int av_w = rc.x + rc.width - pos_x - kLabelOffsetX;
    wxString label = ToWxString(client->GetClientLabel());
    wxString s = TruncateText(dc, label, av_w);

    int w = 0, h = 0;
    dc.GetTextExtent(s, &w, &h);
    int pos_y = rc.y + (rc.height - h) / 2;

    dc.SetTextForeground(cl_text);
    dc.DrawText(s, pos_x, pos_y);
}


void CMinPanelContainer::OnLeftClick(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    SItem* item = x_FindItemByPos(pt);

    if(item)   {
        // restore the window associated with the item
        m_DockManager.OnDockPanelCommand(*item->m_DockPanel, eCmdWindowRestore);
    }
}


void CMinPanelContainer::OnContextMenu(wxContextMenuEvent& event)
{
    static const size_t kMaxLabelChars = 40;

    m_MenuPanel = NULL;
    auto_ptr<wxMenu> menu;
    m_CmdToPanel.clear();

    // hit testing
    wxPoint sc_pt = event.GetPosition();
    wxPoint pt = ScreenToClient(sc_pt);
    SItem* item = x_FindItemByPos(pt);

    if(item)   {
        // clicked on an item
        m_MenuPanel = item->m_DockPanel;
        menu.reset(m_DockManager.GetDockPanelMenu(*m_MenuPanel));

    } else {
        // show generic menu
        menu.reset(new wxMenu());

        int cmd = kRestoreWindowFirst;

        NON_CONST_ITERATE(TItems, it, m_Items)    {
            SItem& item = **it;
            const IWMClient* client = item.m_DockPanel->GetClient();

            string label = client->GetClientLabel();
            if(label.size() > kMaxLabelChars)   {
                label.resize(40);
                label += "...";
            }

            wxString wxs = ToWxString(label);
            menu->Append(cmd, wxs);
            m_CmdToPanel[cmd] = item.m_DockPanel;

            cmd++;
        }

        menu->AppendSeparator();

        CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
        cmd_reg.AppendMenuItem(*menu, eCmdShowWindowsDlg);
    }

    // show context menu
    PopupMenu(menu.get());
}


void CMinPanelContainer::OnDockPanelCommand(wxCommandEvent& event)
{
    _ASSERT(m_MenuPanel);

    m_DockManager.OnDockPanelCommand(*m_MenuPanel, event.GetId());
    m_MenuPanel = NULL;
}


void CMinPanelContainer::OnRestoreWindow(wxCommandEvent& event)
{
    int cmd = event.GetId();
    map<int, CDockPanel*>::iterator it = m_CmdToPanel.find(cmd);

    if(it != m_CmdToPanel.end())    {
        CDockPanel* panel = it->second;
        NON_CONST_ITERATE(TItems, it, m_Items)    {
            SItem& item = **it;
            if(item.m_DockPanel == panel)  {
                m_DockManager.OnDockPanelCommand(*panel, eCmdWindowRestore);
                return;
            }
        }
    }
    _ASSERT(false); // invalid command or panel is not found
}


void CMinPanelContainer::OnUpdateWindowCommand(wxUpdateUIEvent& event)
{
	if (m_MenuPanel != NULL) {
		IWMClient* client = m_MenuPanel->GetClient();
		m_DockManager.GetWindowManager().OnUpdateWindowCommand_Client(event, client);
	}
}


void CMinPanelContainer::OnShowWindowsDlg(wxCommandEvent& event)
{
    m_DockManager.GetWindowManager().OnShowWindowsDlg(event);
}

END_NCBI_SCOPE
