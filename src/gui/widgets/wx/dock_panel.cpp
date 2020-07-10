/*  $Id: dock_panel.cpp 44189 2019-11-14 14:51:36Z katargir $
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

#include <gui/widgets/wx/dock_panel.hpp>

#include <gui/widgets/wx/dock_container.hpp>
#include <gui/widgets/wx/dock_notebook.hpp>
#include <gui/widgets/wx/window_manager.hpp>
#include <gui/widgets/wx/nf_bitmap_button.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/dcclient.h>
#include <wx/settings.h>
#include <wx/artprov.h>
#include <wx/dcbuffer.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CDockPanel

#define ID_BTN_MENU     10101
#define ID_BTN_MINIMIZE 10102
#define ID_BTN_FLOAT    10103
#define ID_BTN_CLOSE    10104

BEGIN_EVENT_TABLE(CDockPanel, wxPanel)
    EVT_BUTTON(ID_BTN_MENU, CDockPanel::OnMenuButtonClick)
    EVT_BUTTON(ID_BTN_MINIMIZE, CDockPanel::OnMinimizeButtonClick)
    EVT_BUTTON(ID_BTN_FLOAT, CDockPanel::OnFloatButtonClick)
    EVT_BUTTON(ID_BTN_CLOSE, CDockPanel::OnCloseButtonClick)    
    EVT_CONTEXT_MENU(CDockPanel::OnContextMenu) 
    EVT_CHILD_FOCUS(CDockPanel::OnChildFocus)
    EVT_PAINT(CDockPanel::OnPaint)
    EVT_LEFT_DOWN(CDockPanel::OnLeftDown)
    EVT_LEFT_DCLICK(CDockPanel::OnLeftDown)
    EVT_LEFT_UP(CDockPanel::OnLeftUp)
    EVT_MOTION(CDockPanel::OnMotion)
    EVT_MOUSE_CAPTURE_LOST(CDockPanel::OnMouseCaptureLost)

    EVT_SET_FOCUS(CDockPanel::OnSetFocus)

END_EVENT_TABLE()


void CDockPanel::OnSetFocus( wxFocusEvent& event )
{
    event.Skip();
}




const long kwxBorderStyle =
#ifdef __WXGTK__
    wxBORDER_SUNKEN;
#else
    wxBORDER_STATIC;
#endif


CDockPanel::CDockPanel(CDockContainer* container,
                       CDockManager* manager,
                       IWMClient* client,
                       const wxString& name
                       )
 : wxPanel(container, wxID_ANY, wxDefaultPosition, wxSize(0, 0),
            wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | kwxBorderStyle,
            name),
    m_DockManager(manager),
    m_DockContainer(container),
    m_Client(NULL),
    m_CaptionHeight(6), // just to make it visible
    mf_IsActive(false),
    m_ShowingPopup(false),
    m_CaptionItem(NULL),
    m_Canceled(true)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    x_CreateControls();

    x_SetClient(*client);

    //LOG_POST("CDockPanel constructor " << this << "  " << client->GetClientLabel());
}

wxSize CDockPanel::ClientToWindowSize(const wxSize& size) const
{
    wxSize wsize = wxPanel::ClientToWindowSize(size);
    if (m_CaptionItem)
        wsize.y += m_CaptionItem->GetSize().GetHeight();
    return wsize;
}

void CDockPanel::ShowBorder(bool show)
{
    long styles = GetWindowStyleFlag();

    styles &= ~(wxBORDER_STATIC | wxBORDER_SIMPLE |
                wxBORDER_RAISED | wxBORDER_THEME | wxBORDER_NONE);
    styles |= (show ? kwxBorderStyle : wxBORDER_NONE);

    SetWindowStyleFlag(styles);

    Layout();
}


CDockPanel::~CDockPanel()
{
    //LOG_POST("CDockPanel() destructor");
    if(m_Client)    {
        ERR_POST("CDockPanel() destructor - CLIENT IS NOT NULL");
    }
}


void CDockPanel::x_CreateControls()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);

    // create Button Size
    m_CaptionSizer = new wxBoxSizer(wxHORIZONTAL);
    m_CaptionSizer->SetMinSize(40, 18);
    m_CaptionItem = m_CaptionSizer->Add(100, 0, 1, wxEXPAND | wxALIGN_LEFT);

    wxWindow* btn = x_CreateButton(ID_BTN_MENU, "wm_menu", "wm_menu_active");
    m_CaptionSizer->Add(btn, 0, wxALL, 2);

    btn = x_CreateButton(ID_BTN_MINIMIZE, "wm_minimize", "wm_minimize_active");
    m_CaptionSizer->Add(btn, 0, wxALL, 2);

    btn = x_CreateButton(ID_BTN_FLOAT, "wm_float", "wm_float_active");
    m_CaptionSizer->Add(btn, 0, wxALL, 2);

    btn = x_CreateButton(ID_BTN_CLOSE, "wm_close", "wm_close_active");
    m_CaptionSizer->Add(btn, 0, wxALL, 2);

    sizer->Add(m_CaptionSizer, 0, wxEXPAND);
}

wxWindow* CDockPanel::x_CreateButton(TCmdID id, const string& icon, const string& hover_icon)
{
    return new CNoFocusBitmapButton(this, id,
        wxArtProvider::GetBitmap(ToWxString(icon)),
        wxArtProvider::GetBitmap(ToWxString(hover_icon)),
        wxDefaultPosition);
}


void CDockPanel::x_SetClient(IWMClient& client)
{
    _ASSERT(&client != m_Client);

    //Freeze();

    m_Client = &client;

    wxWindow* client_w = m_Client->GetWindow();
    client_w->Reparent(this);
    GetSizer()->Add(client_w, 1, wxEXPAND);
    
    client_w->Show();
    Refresh();

    //Thaw();
}


void CDockPanel::RemoveClient(wxWindow* new_parent)
{
    _ASSERT(m_Client  &&  new_parent);

    //Freeze();

    wxWindow* client_w = m_Client->GetWindow();
    m_Client = NULL;

    client_w->Hide();
    client_w->Reparent(new_parent);
    GetSizer()->Detach(client_w);

    Refresh();

    //Thaw();
}

static const int kIconOffX = 1;
static const int kIconOffY = 1;
static const int kTextOffX = 4;

void CDockPanel::OnPaint(wxPaintEvent& event)
{
    // On mac, this gets called sometimes before window
    // has a proper size which, on wxWidgets 2.9.2, causes
    // this program to exit at src/common/dcbufcmn.cpp:108.
    wxSize s = this->GetSize();
    if (s.GetX() <= 0 || s.GetY() <= 0)
        return;

    UpdateButtons();

    wxAutoBufferedPaintDC dc(this);

    dc.SetPen(*wxTRANSPARENT_PEN);

    bool focus = HasFocus();

    // fill background
    wxSystemColour sys_colour = focus ? wxSYS_COLOUR_ACTIVECAPTION : wxSYS_COLOUR_INACTIVECAPTION;
    wxColor cl_back = wxSystemSettings::GetColour(sys_colour);
    wxBrush brush(cl_back);
    dc.SetBrush(brush);

    wxRect rc = m_CaptionItem->GetRect();
    wxRect client_rc = GetClientRect();
    rc.y = client_rc.x;

    rc.width = client_rc.width;

    dc.DrawRectangle(rc);

    if(m_Client)    {
        rc.width = m_CaptionItem->GetRect().width;
        x_DrawClientElements(dc, rc);
    }
}


// draw client's icon and label
void CDockPanel::x_DrawClientElements(wxDC& dc, const wxRect& rc)
{
    int pos_x = rc.x;

    // draw Client icon
    string alias = m_Client->GetIconAlias();
    wxBitmap bmp = wxArtProvider::GetBitmap(ToWxString(alias));
    if(bmp.IsOk())  {
        pos_x += kIconOffX;
        dc.DrawBitmap(bmp, pos_x, rc.y + kIconOffY);
        pos_x += bmp.GetWidth() + kIconOffX;
    }

    // draw Client color key
    IWMClient* client = GetClient();
    if (client  &&  client->GetColor()) {
        pos_x += kIconOffX;
        int y_off = 3;
        int bttn_size = rc.height  - y_off * 2;
        CRgbaColor color = *client->GetColor();
        wxBrush brush(wxColour(color.GetRedUC(),
            color.GetGreenUC(), color.GetBlueUC()));
        dc.SetBrush(brush);
        dc.DrawRectangle(pos_x, rc.y + y_off, bttn_size, bttn_size);
        pos_x += bttn_size + kIconOffX;
    }

    // draw Client label
    wxFontWeight weight = HasFocus() ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL;

    // This font appears about 2 points larger on windows than on other 
    // platforms.  The other platforms seem correct int their point sizes, but
    // windows is larger than expected.
#ifdef __WXMSW__
    wxFont font(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, weight);
#else
    wxFont font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, weight);
#endif
   
    dc.SetFont(font);

    wxColor cl_text = wxSystemSettings::GetColour(wxSYS_COLOUR_CAPTIONTEXT);
    dc.SetTextForeground(cl_text);

    // truncate label if needed
    pos_x += kTextOffX;
    int av_w = rc.x + rc.width - pos_x - kTextOffX;
    wxString label = ToWxString(m_Client->GetClientLabel());
    wxString s = TruncateText(dc, label, av_w);

    int w = 0, h = 0;
    dc.GetTextExtent(s, &w, &h);
    int pos_y = rc.y + (rc.height - h) / 2;

    dc.DrawText(s, pos_x, pos_y);
}


bool CDockPanel::ProcessEvent(wxEvent& event)
{
    // Redirect events to active child first. Stops the same event being processed repeatedly
    static wxEventType inEvent = wxEVT_NULL;
    if (inEvent == event.GetEventType())
        return false;
    inEvent = event.GetEventType();

    bool res = false;

    // first - forward menu commands to the client window
    if(m_Client  &&  event.IsCommandEvent())    {
        wxEventType type = event.GetEventType();
        if(type == wxEVT_UPDATE_UI  ||  type == wxEVT_COMMAND_MENU_SELECTED)    {

            wxEvtHandler* handler = m_Client->GetCommandHandler();
            _ASSERT(handler);

            res = handler->ProcessEvent(event);

            if(! res  &&  m_ShowingPopup) {
                wxUpdateUIEvent* ui_event = dynamic_cast<wxUpdateUIEvent*>(&event);

                if( ! ui_event)    {
                    int cmd = event.GetId();
                    CDockManager& manager = GetDockContainer()->GetDockManager();
                    res =  manager.OnDockPanelCommand(*this, cmd);
                }
            }
        }
    }

    // process as usual
    if ( ! res) {
        res = TParent::ProcessEvent(event);
    }
    
    inEvent = wxEVT_NULL;
    return res;
}


void CDockPanel::UpdateCaption()
{
    if( m_Client ){
        wxRect rc = m_CaptionItem->GetRect();
        rc.y = GetClientRect().x;
        RefreshRect( rc );
    }
}


void CDockPanel::UpdateButtons()
{
}


// checks if focus changed and redraw itself if necessary
void CDockPanel::UpdateFocusState()
{
    if(mf_IsActive != HasFocus())    {
        mf_IsActive = ! mf_IsActive;
        Refresh();
    }
}

// returns true if panel or one of it's children has FLTK focus
bool CDockPanel::HasFocus() const
{
    const wxWindow* target;

    CDockNotebook* notebook = dynamic_cast<CDockNotebook*>( GetParent() );
    if( notebook ){
        target = notebook;
    } else {
        target = this;
    }

    wxWindow* win = wxWindow::FindFocus();
    while( win && win != target ){
        win = win->GetParent();
    }

    if( win == NULL ){
        return false;
    }

    if( win == this ){
        return true;
    }

    int selpage = notebook->GetSelection();

    return (selpage != wxNOT_FOUND &&
            selpage >= 0 && 
            selpage < (int)notebook->GetPageCount() &&
            notebook->GetPage( selpage ) == this);
}

void CDockPanel::MakeActive( bool flag )
{
    mf_IsActive = flag;
}

void CDockPanel::x_ShowContextMenu()
{
    m_ShowingPopup = true;

    auto_ptr<wxMenu> menu(m_DockManager->GetDockPanelMenu(*this));
    PopupMenu(menu.get());

    m_ShowingPopup = false;
}


void CDockPanel::OnMenuButtonClick(wxCommandEvent& event)
{
    SetFocus();
    x_ShowContextMenu();
}


void CDockPanel::OnMinimizeButtonClick(wxCommandEvent& event)
{
    IWMClient* client = GetClient();
    m_DockContainer->GetDockManager().Minimize(*client);
}


void CDockPanel::OnFloatButtonClick(wxCommandEvent& event)
{
    IWMClient* client = GetClient();
    if( ! m_DockManager->IsFloating(*client))   {
        m_DockContainer->GetDockManager().MoveToFloatingFrame(*client);
    }
}


void CDockPanel::OnContextMenu(wxContextMenuEvent& event)
{
    SetFocus();
    x_ShowContextMenu();
}


void CDockPanel::OnChildFocus(wxChildFocusEvent& evt)
{
    wxWindow* focus = evt.GetWindow();

    string type = typeid(*focus).name();

    if( focus->GetId() == ID_BTN_CLOSE ){
        //LOG_POST("CDockPanel::OnChildFocus() " << evt.GetWindow() << "  " << type << " - no change");

        m_DockManager->GetWindowManager().OnFocusChanged(evt.GetWindow());
    } else {
        //LOG_POST("CDockPanel::OnChildFocus() " << evt.GetWindow() << "  " << type);

        m_DockManager->GetWindowManager().OnFocusChanged(evt.GetWindow());
    }

    Refresh();
}


void CDockPanel::OnCloseButtonClick(wxCommandEvent& event)
{
    _ASSERT(m_DockContainer);
    m_DockContainer->GetDockManager().OnCloseDockable(*this);
}


CDockContainer* CDockPanel::GetDockContainer()
{
    return m_DockContainer;
}


void CDockPanel::SetDockContainer(CDockContainer* dock_cont)
{
    m_DockContainer = dock_cont;
}


void CDockPanel::OnLeftDown(wxMouseEvent& evt)
{
    if(m_Client)    {
        m_Client->GetWindow()->SetFocus();
    }

    wxPoint pt = evt.GetPosition();
    //LOG_POST("DP::OnLeftDown()  CAPTURE MOUSE " << pt.x << " " << pt.y);

    CaptureMouse();

    m_ClickPoint = pt;
    m_Canceled = false;
}


void CDockPanel::OnMotion(wxMouseEvent& evt)
{
    wxPoint pt = evt.GetPosition();
    //LOG_POST("DP::OnMotion()  " << pt.x << " " << pt.y << "  m_Canceled " << m_Canceled);

    if(! m_Canceled) {
        if(evt.LeftIsDown()) {
            int drag_x_threshold = wxSystemSettings::GetMetric(wxSYS_DRAG_X);
            int drag_y_threshold = wxSystemSettings::GetMetric(wxSYS_DRAG_Y);

            if (abs(pt.x - m_ClickPoint.x) > drag_x_threshold ||
                abs(pt.y - m_ClickPoint.y) > drag_y_threshold) {
                // we release mouse to allow Window Manager to capture it
                if (HasCapture())   {
                    ReleaseMouse();
                }
                m_DockManager->GetWindowManager().OnDockPanelStartDrag(this, m_ClickPoint);
            }
        }
    }
}


void CDockPanel::OnLeftUp(wxMouseEvent& evt)
{
    //wxPoint pt = evt.GetPosition();
    //LOG_POST("DP::OnLeftUp()  " << pt.x << " " << pt.y);

    if (HasCapture())   {
        //WM_POST("ReleaseMouse()");
        ReleaseMouse();
    }
}


void CDockPanel::OnMouseCaptureLost(wxMouseCaptureLostEvent& evt)
{
    //LOG_POST("DP::OnMouseCaptureLost()");
}


void CDockPanel::FinishDrag()
{
    //WM_POST("CDockPanel::FinishDrag() m_Canceled = true;");
    m_Canceled = true;

    if (HasCapture())   {
        //WM_POST("ReleaseMouse()");
        ReleaseMouse();
    }
}


EDockEffect CDockPanel::DropTest(const wxPoint& sc_mouse_pos, wxWindow*& target)
{
    EDockEffect effect = eNoEffect;
    target = NULL;

    if(m_CaptionSizer)  {
        wxPoint pos = m_CaptionSizer->GetPosition();
        wxSize size = m_CaptionSizer->GetSize();
        wxRect bounds(pos, size);

        wxPoint pt = ScreenToClient(sc_mouse_pos);

        if(bounds.Contains(pt)) {
            effect = ePutInTab;
            target = this;
        }
    }
    return effect;
}

bool CDockPanel::Layout()
{
    // when CDockPanel is placed ina floating frame it is not repainted
    // automatically after Layout(), so we force it
    bool ok = TParent::Layout();
    Refresh();
    return ok;
}

END_NCBI_SCOPE
