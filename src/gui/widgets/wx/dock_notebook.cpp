/*  $Id: dock_notebook.cpp 43813 2019-09-05 16:03:49Z katargir $
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

#include <gui/widgets/wx/dock_notebook.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/wx/dock_container.hpp>
#include <gui/widgets/wx/dock_panel.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>

#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/aui/aui.h>

#ifdef __WXMAC__
#    include <Carbon/Carbon.h>
#endif


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CDockNotebook

BEGIN_EVENT_TABLE(CDockNotebook, wxAuiNotebook)
    EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, CDockNotebook::OnPageClose)
    EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, CDockNotebook::OnPageChanged)
    EVT_AUINOTEBOOK_PAGE_CHANGING(wxID_ANY, CDockNotebook::OnPageChanging)
    EVT_AUINOTEBOOK_BEGIN_DRAG(wxID_ANY, CDockNotebook::OnTabBeginDrag)
    EVT_AUINOTEBOOK_DRAG_MOTION(wxID_ANY, CDockNotebook::OnTabDragMotion)
    EVT_AUINOTEBOOK_END_DRAG(wxID_ANY, CDockNotebook::OnTabEndDrag)
END_EVENT_TABLE()


const int kNotebookStyle =  wxAUI_NB_TAB_MOVE |
                            wxAUI_NB_TAB_EXTERNAL_MOVE |
                            wxAUI_NB_SCROLL_BUTTONS |
                            wxAUI_NB_WINDOWLIST_BUTTON |
                            wxAUI_NB_CLOSE_ON_ACTIVE_TAB |
#ifdef __WXGTK__
                            wxBORDER_SUNKEN
#else
                            wxBORDER_STATIC
#endif
                            ;

CDockNotebook::CDockNotebook( CDockContainer& container )
    : wxAuiNotebook(&container, wxID_ANY, wxDefaultPosition, wxDefaultSize, kNotebookStyle)
    , m_DockContainer( &container )
    , m_OutsideDrag( false )
    , m_HitTabIndex( -1 )
{
    SetLabel( wxT("dock_notebook") );

    // remove the border
    wxAuiDockArt* dock_art = m_mgr.GetArtProvider();
    dock_art->SetMetric( wxAUI_DOCKART_PANE_BORDER_SIZE, 0 );

    // set cutom art provider
    SetArtProvider( new CDockNotebookTabArt() );
}



CDockNotebook::~CDockNotebook()
{
    //LOG_POST("~CDockNotebook");
}


CDockContainer* CDockNotebook::GetDockContainer()
{
    return m_DockContainer;
}


void CDockNotebook::SetDockContainer(CDockContainer* cont)
{
    m_DockContainer = cont;
}

void CDockNotebook::Cont_Remove(wxWindow* child)
{
    size_t index = GetPageIndex(child);
    int selpage = GetSelection();
    RemovePage(index);

    // Selected page is not automatically updated.  Make sure the current
    // selection is not > num pages (unless page count is 0 - then setselection
    // to 0 since SetSelection takes a size_t). 
    // (maybe we could use wxNOT_FOUND, but its not documented to clear sel)
    if (selpage != wxNOT_FOUND && 
        selpage >= (int)GetPageCount() &&
        selpage > 0) {
           SetSelection(selpage-1);
    }
    child->Reparent(m_DockContainer);
}


void CDockNotebook::Cont_Replace(wxWindow* old_child, wxWindow* new_child)
{
    size_t index = GetPageIndex(old_child);
    RemovePage(index);
    old_child->Reparent(m_DockContainer);

    // workaround for notebook bug - it would not update the page selection, so we reset it
    int other_index = (index == 0) ? 1 : 0;
    SetSelection(other_index);

    string label = x_GetPageNameByWindow(new_child);
    InsertPage(index, new_child, ToWxString(label), false, wxNullBitmap);
    new_child->Reparent(this);
    SetSelection(index);
}

void CDockNotebook::RefreshPageByWindow( wxWindow* page )
{
    int page_ix = GetPageIndex( page );
    if( page_ix == wxNOT_FOUND ){
        return;
    }

    wxString label = ToWxString( x_GetPageNameByWindow(page) );
    wxString old_label = GetPageText( page_ix );
    if( label == old_label ){
        return;
    }
    
    SetPageText( page_ix, label );
}


string CDockNotebook::x_GetPageNameByWindow(wxWindow* window)
{
    if (window) {
        CDockPanel* panel = dynamic_cast<CDockPanel*>(window);
        if (panel) {
            IWMClient* wm_cli = panel->GetClient();
            IWMClient::ELabel el = IWMClient::eDefault;
            int viewOption = CGuiRegistry::GetInstance().GetInt("GBENCH.Application.ViewOptions.ViewTitle", 0);
            switch (viewOption) {
            case 0: el = IWMClient::eContent; break;
            case 1: el = IWMClient::eTypeAndContent; break;
            case 2: el = IWMClient::eDetailed; break;
            default:
                el = IWMClient::eDefault;
            }

            string l = wm_cli->GetClientLabel(el);
            if (l.empty()) {
                l = wm_cli->GetClientLabel(IWMClient::eContent);
            }
            return l;
        }
        CDockSplitter* splitter = dynamic_cast<CDockSplitter*>(window);
        if (splitter) {
            return "Splitter";
        }
    }
    return "";
}


void CDockNotebook::InsertPageAtHitPoint(wxWindow* page, const wxString& caption)
{
    if(m_HitTabIndex >=0  &&  m_HitTabIndex < (int) GetPageCount()) {
        int other_index = (m_HitTabIndex == 0) ? 1 : 0;
        SetSelection(other_index);

        InsertPage(m_HitTabIndex, page, caption, false, wxNullBitmap);
    } else {
        AddPage(page, caption);
    }
}


EDockEffect CDockNotebook::DropTest(const wxPoint& screen_pt, wxWindow*& target)
{
    if (GetPageCount() == 0) {
        target = NULL;
        return eNoEffect;
    }

    // activate pages as mouse moves over the tabs
    wxPoint client_pt = ScreenToClient(screen_pt);
    wxAuiTabCtrl* tabs = GetTabCtrlFromPoint(client_pt);
    wxWindow* over_tab = NULL;

    if (tabs   &&  tabs->TabHitTest(client_pt.x, client_pt.y, &over_tab))   {
        m_HitTabIndex = m_tabs.GetIdxFromWindow(over_tab);
        if(m_HitTabIndex != -1)  {
            SetSelection(m_HitTabIndex);
        }
    } else {
        m_HitTabIndex = -1;
    }

    wxRect rc = GetClientRect();
    rc.Inflate(-1, -1);
    if(rc.Contains(client_pt)) {
        // the effect is always the same - add a Tab
        target = this;
        return ePutInTab;
    } else {
        target = NULL;
        return eNoEffect;
    }
}


void CDockNotebook::OnPageClose(wxAuiNotebookEvent& event)
{
    //LOG_POST("CDockNotebook::OnPageClose()");

    int index = event.GetSelection();
    wxWindow* page = GetPage(index);

    m_DockContainer->OnTabClosePressed(page);   
    event.Veto();
}

void CDockNotebook::OnPageChanging(wxAuiNotebookEvent& event)
{
    //LOG_POST("CDockNotebook::OnPageChanging()");

    // force focusing of the view (hidden under this Tab)
    //
    // we do it to initate activation and make menus and other context dependent 
    // stuff available
    //
    int sel = GetSelection();
    if (sel >= 0) {
        wxWindow* page = GetPage(sel);
        if (page) {
            vector<IWMClient*> clients;
            m_DockContainer->GetClientsInWindow(page, clients);

            if (clients.size()) {
                IWMClient* cl = clients[0];
                wxWindow* cl_w = cl->GetWindow();
                cl_w->SetFocus();
            }
        }
    }

    // Prevent wxAuiTabCtrl getting focus
    if (event.GetSelection() == event.GetOldSelection())
        return;

    event.Skip();
}

void CDockNotebook::OnPageChanged(wxAuiNotebookEvent& event)
{
    int sel = event.GetOldSelection();
    if (sel >= 0) {
        wxWindow* page = GetPage(sel);
        if (page) {
            // Send message to child windows parent is being hidden (needed
            // for sticky tool tips)
            BroadcastCommandToChildWindows(page, eCmdParentHide);
        }
    }

    sel = event.GetSelection();
    if (sel >= 0) {
        wxWindow* page = GetPage(sel);
        if (page) {
            // Send message to child windows parent is being shown (needed
            // for sticky tool tips)
            BroadcastCommandToChildWindows(page, eCmdParentShow);
        }
    }

    event.Skip();
}


void CDockNotebook::OnTabBeginDrag(wxAuiNotebookEvent& event)
{
    //LOG_POST("CDockNotebook::OnTabBeginDrag");
    // AUI code
#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
    m_last_drag_x = 0;
#else
    m_lastDragX = 0;
#endif
    m_OutsideDrag = false;
}


// Dragging a tab may result in the tab being removed from the Notebook. When the last
// tab is removed the Notebook may be destroyed and so it will not be possible to
// handle events for the D&D session.
// To avoid problems we delegate dragging control to the Dock Container.
void CDockNotebook::OnTabDragMotion(wxAuiNotebookEvent& evt)
{
    //LOG_POST("CDockNotebook::OnTabDragMotion");

    if(m_OutsideDrag) {
        return; // we have transfered D&D handling to Dock Container already
    }

    // AUI code
    wxPoint screen_pt = ::wxGetMousePosition();
    wxPoint client_pt = ScreenToClient(screen_pt);
    wxPoint zero(0,0);

    wxAuiTabCtrl* src_tabs = (wxAuiTabCtrl*)evt.GetEventObject();
    wxAuiTabCtrl* dest_tabs = GetTabCtrlFromPoint(client_pt);

    if (dest_tabs == src_tabs)
    {
        if (src_tabs)   {
            src_tabs->SetCursor(wxCursor(wxCURSOR_ARROW));
        }

        // always hide the hint for inner-tabctrl drag
        m_mgr.HideHint(); //TODO

        wxPoint pt = dest_tabs->ScreenToClient(screen_pt);
        wxWindow* dest_location_tab;

        // this is an inner-tab drag/reposition
        if (dest_tabs->TabHitTest(pt.x, pt.y, &dest_location_tab))  {
            int src_idx = evt.GetSelection();
            int dest_idx = dest_tabs->GetIdxFromWindow(dest_location_tab);

            // prevent jumpy drag
            if ((src_idx == dest_idx) || dest_idx == -1 ||
#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
                (src_idx > dest_idx && m_last_drag_x <= pt.x) ||
                (src_idx < dest_idx && m_last_drag_x >= pt.x))  {
                m_last_drag_x = pt.x;
#else
                (src_idx > dest_idx && m_lastDragX <= pt.x) ||
                (src_idx < dest_idx && m_lastDragX >= pt.x))  {
                m_lastDragX = pt.x;
#endif
                return;
            }

            wxWindow* src_tab = dest_tabs->GetWindowFromIdx(src_idx);

            dest_tabs->MovePage(src_tab, dest_idx);
            dest_tabs->SetActivePage((size_t)dest_idx);
            dest_tabs->DoShowHide();
            dest_tabs->Refresh();
#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
            m_last_drag_x = pt.x;
#else
            m_lastDragX = pt.x;
#endif
        }

        //LOG_POST("Dragging inside Tab control");
        //LOG_POST("CDockNotebook::OnTabDragMotion END");
        return;
    }

    //if we reached this point the mouse is outside the tabs area
    //LOG_POST("Dragging OUTSIDE");

    if( ! m_OutsideDrag)    {
        m_OutsideDrag = true;
        // We need to delegate D&D handling to Dock Container.

        int index = evt.GetSelection();
        IDockableWindow* drag_w = dynamic_cast<IDockableWindow*>(GetPage(index));
        _ASSERT(drag_w);
        CBeginDragOutsideEvent post_event(drag_w);
        wxPostEvent(m_DockContainer, post_event);

        wxAuiTabCtrl* ctrl = dynamic_cast<wxAuiTabCtrl*>(evt.GetEventObject());
        if(GetCapture() == ctrl)    {
            ctrl->ReleaseMouse();
        }
    }
    //LOG_POST("CDockNotebook::OnTabDragMotion END");
}


void CDockNotebook::OnTabEndDrag(wxAuiNotebookEvent& event)
{
    // a tab has been dragged into a new position - notify the container
    m_DockContainer->OnUpdateTabOrder(*this);
    event.Skip();
}

void CDockNotebook::MakeSelectionVisible()
{
    int sel_ix = GetSelection();
    if( sel_ix < 0 ){ return; }

    wxWindow* sel_page = GetPage( sel_ix );
    if( sel_page == NULL ){ return; }

    wxAuiTabCtrl* tab_ctrl;
    int tab_ix;

    if( FindTab( sel_page, &tab_ctrl, &tab_ix ) ){

#if defined(__WXMSW__)
        tab_ctrl->HandlePaint();
#endif

        tab_ctrl->MakeTabVisible( tab_ix, tab_ctrl );
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CDockNotebookTabArt

CDockNotebookTabArt::CDockNotebookTabArt()
{
}


wxAuiTabArt* CDockNotebookTabArt::Clone()
{
    return new CDockNotebookTabArt();
}


void CDockNotebookTabArt::DrawBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect)
{
    // draw background
#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
    dc.SetBrush(m_bkbrush);
#else
    dc.SetBrush(m_bkBrush);
#endif
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(-1, -1, rect.GetWidth()+2, rect.GetHeight()+2);

    // draw base line
    dc.SetPen(*wxGREY_PEN);
    dc.DrawLine(0, rect.GetHeight()-1, rect.GetWidth(), rect.GetHeight()-1);
}


///////////////////////////////////////////////////////////////////////////////
/// CDockSplitter
CDockSplitter::CDockSplitter(CDockContainer& dock_cont, CSplitter::ESplitType type)
    : CSplitter(&dock_cont, wxID_ANY, type),
    m_DockContainer(&dock_cont)
{
    // we render it differently on MacOS
#ifndef __WXMAC__
    m_SepSize = 3;
#endif
}


CDockContainer* CDockSplitter::GetDockContainer()
{
    return m_DockContainer;
}


void CDockSplitter::SetDockContainer(CDockContainer* cont)
{
    m_DockContainer = cont;
}


void CDockSplitter::x_DrawSeparator(wxDC& dc, int x, int y, int w, int h)
{   
    wxColour cl_back = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    wxBrush brush(cl_back);

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(brush);
    dc.DrawRectangle(x, y, w, h);

#ifdef __WXMAC__THIS_CODE_NEEDS_TO_BE_CHANGED_EVERYTIME_OSX_UPGRADED
    /// draw a Mac-style round "dot" in the middle of the splitter
    //int yy = GetClientSize().y - y + h + 2; // center splitter notch
    HIRect rect = CGRectMake(x, y, w, h);

    HIThemeSplitterDrawInfo info;
    info.state = kThemeStateActive;// : kThemeStateInactive;
    info.adornment = kThemeAdornmentNone;

    WindowRef wr = WindowRef(this->MacGetTopLevelWindowRef());
    CGContextRef context;
    QDBeginCGContext (GetWindowPort(wr), &context);
    HIThemeDrawPaneSplitter(&rect, &info, context, kHIThemeOrientationInverted);
    QDEndCGContext(GetWindowPort(wr), &context);
#endif
}

void CDockSplitter::Cont_Remove(wxWindow* child)
{
    child->Reparent(m_DockContainer);
}


void CDockSplitter::Cont_Replace(wxWindow* old_child, wxWindow* new_child)
{
    int col = -1, row = -1;
    FindChild(old_child, col, row);

    old_child->Reparent(m_DockContainer);

    InsertToCell(new_child, col, row);
}


END_NCBI_SCOPE
