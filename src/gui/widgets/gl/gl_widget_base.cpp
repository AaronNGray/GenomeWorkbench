/*  $Id: gl_widget_base.cpp 42142 2018-12-28 16:11:17Z katargir $
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

#include <gui/widgets/gl/gl_widget_base.hpp>

#include <gui/widgets/wx/popup_event.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/unfocused_controls.hpp>
#include <gui/widgets/gl/attrib_menu.hpp> 

#include <gui/opengl/glfont.hpp>

#include <gui/utils/command.hpp> //TODO for command definitions only

#include <gui/types.hpp>

#include <list>

#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/app.h>

#define WIDGET_PANE_ENTER_TIMER 6310

BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE( CGlWidgetPane, C3DCanvas )
    EVT_SIZE(CGlWidgetPane::OnSize)
    EVT_ENTER_WINDOW(CGlWidgetPane::OnEnterWindow)
    EVT_TIMER(WIDGET_PANE_ENTER_TIMER, CGlWidgetPane::OnTimer)
    EVT_LEFT_DOWN(CGlWidgetPane::OnMouseDown)
    EVT_RIGHT_DOWN(CGlWidgetPane::OnMouseDown)
    EVT_LEFT_DCLICK(CGlWidgetPane::OnMouseDown)
    EVT_RIGHT_DCLICK(CGlWidgetPane::OnMouseDown)
    EVT_MIDDLE_DOWN(CGlWidgetPane::OnMouseDown)
    EVT_LEFT_UP(CGlWidgetPane::OnMouseUp)
    EVT_RIGHT_UP(CGlWidgetPane::OnMouseUp)
    EVT_MIDDLE_UP(CGlWidgetPane::OnMouseUp)
    EVT_MOTION(CGlWidgetPane::OnMotion)
    EVT_MOUSEWHEEL(CGlWidgetPane::OnMouseWheel)
    EVT_MOUSE_CAPTURE_LOST(CGlWidgetPane::OnMouseCaptureLost)
    EVT_LEAVE_WINDOW(CGlWidgetPane::OnLeaveWindow)

    EVT_KEY_DOWN(CGlWidgetPane::OnKeyEvent)
    EVT_KEY_UP(CGlWidgetPane::OnKeyEvent)

    EVT_SET_FOCUS(CGlWidgetPane::OnSetFocus)
    EVT_KILL_FOCUS(CGlWidgetPane::OnKillFocus)

    EVT_COMMAND_RANGE(eCmdFirstStickyToolTipCmd, 
                      eCmdLastStickyToolTipCmd, 
                      wxEVT_COMMAND_BUTTON_CLICKED,
                      CGlWidgetPane::OnCmdEvent)
    EVT_BUTTON(eCmdTipRemoved, CGlWidgetPane::OnTipRemoved)
END_EVENT_TABLE()


void CGlWidgetPane::GHH_Redraw()
{
    Refresh();
}


void CGlWidgetPane::GHH_SetCursor(const wxCursor& cursor)
{
    SetCursor(cursor);
}

//static int capture_count = 0;

void CGlWidgetPane::GHH_CaptureMouse()
{
    //_ASSERT(capture_count >= 0);
    //capture_count++;
    //LOG_POST("CGlWidgetPane::GHH_CaptureMouse() " << capture_count);

//    _ASSERT(! HasCapture());
    if (HasCapture()) {
        ReleaseMouse();
    }
    CaptureMouse();
}


void CGlWidgetPane::GHH_ReleaseMouse()
{
    //_ASSERT(capture_count > 0);
    //capture_count--;
    //LOG_POST("CGlWidgetPane::GHH_ReleaseMouse() " << capture_count);

    if(HasCapture())    {
        ReleaseMouse();
    }
}


bool CGlWidgetPane::TC_NeedTooltip(const wxPoint & pt)
{
    return false;
}


string CGlWidgetPane::TC_GetTooltip(const wxRect & rect)
{
    return "";
}


wxWindow* CGlWidgetPane::TC_GetWindow()
{
    return this;
}

bool CGlWidgetPane::x_GrabFocus()
{
    bool grab = false;

    wxWindow* w = wxWindow::FindFocus();

    // By default only grab it from a sticky tooltip since we generally want
    // to restore focus to the underlying window when coming off a tooltip. Other
    // windows (especially text widgets) will not want to lose their focus just
    // because you hovered over something else.
    if (w != NULL) {        
        while (w != NULL && !grab) {
            if (w != NULL) {               
                if (w->GetName() ==  wxT("Sticky Tooltip"))
                    grab = true;
                    //_TRACE("Focus Parent: " << w->GetClassInfo()->GetClassName() 
                    //       << " " << w->GetName());
            }
            
            w = w->GetParent();
        }
        //_TRACE(" ");
    }

    return grab;
}



void CGlWidgetPane::OnEnterWindow(wxMouseEvent& /*event*/)
{
    // Doing this allows features such as minimap to work after the currsor
    // enters the window without having to first click on the window
    // to make it active.  It also allows focus to return to the window after
    // user mouses over a tooltip window.  In both cases it also means that
    // hotkeys for the window will work since the window having focus is
    // needed for hotkeys to be active.

    // There may be situations when you do not want the window to 'steal' focus
    // though, such as when you are in an edit control (this causes the cursor
    // to suddenly 'disappear' from the edit control)
    
    if (x_GrabFocus())
        this->SetFocus();

    // Can't just do fix here since sometimes window (like tooltip from project 
    // view) is still displayed when this event occurs.
    m_EnterTimer.Restart();
}

void CGlWidgetPane::OnTimer(wxTimerEvent& event)
{
    if (event.GetId() == WIDGET_PANE_ENTER_TIMER) {    
         DlgOverlayFix();
         Refresh();
    }
}

void CGlWidgetPane::CheckOverlayTimer()
{
    // If we just entered the window, reset window to deal with opengl/window overlay error
    // in vista (and later versions of windows).
    if (m_EnterTimer.IsRunning()) {
        // Try to make sure windows are fully deleted before executing the fix
        wxTheApp->ProcessIdle();

        // If time is insufficient, may still have problem (quick click after entering). 
        double delta = m_EnterTimer.Elapsed();
        m_EnterTimer.Reset();
        if (delta < 0.7) {
            // We still try to reset display, but recognize it may not work so we will 
            // retry after 0.7 seconds
            DlgOverlayFix();
            unsigned long delay = (unsigned long)((0.7-delta)*1000.0);
            if (m_GLOverlapFixTimer.IsRunning())
                m_GLOverlapFixTimer.Stop();

            m_GLOverlapFixTimer.Start(delay, true);   
        }
        else {
            DlgOverlayFix();
        }
    }
}

void CGlWidgetPane::OnMouseDown(wxMouseEvent& event)
{
    SetFocus();

    CheckOverlayTimer();

    wxPoint pos = event.GetPosition();
    int area = x_GetAreaByWindowPos(pos);
    if( ! x_Handlers_handle(event, area)) {
        event.Skip();
    }
}


void CGlWidgetPane::OnMouseUp(wxMouseEvent& event)
{
    bool handled = false;
    if (m_pCurrHandlerRec)  {
        handled = x_DispatchEventToHandler(event, m_pCurrHandlerRec);
    }
    if( ! handled)  {
        wxPoint pos = event.GetPosition();
        int area = x_GetAreaByWindowPos(pos);
        if( !x_Handlers_handle(event, area))
            event.Skip();       
    }    

    x_SetCurrHandler(NULL);
}

static bool trace = false;

void CGlWidgetPane::OnMotion(wxMouseEvent& event)
{

    bool handled = false;
    if(m_pCurrHandlerRec)  {
        handled = x_DispatchEventToHandler(event, m_pCurrHandlerRec);
    } 
    if (!handled) {
        wxPoint pos = event.GetPosition();
        int area = x_GetAreaByWindowPos(pos);
        handled = x_Handlers_handle(event, area);
    }
    if( ! handled)  {
        event.Skip();
    }
    else {
        CheckOverlayTimer();
    }
}


void CGlWidgetPane::OnMouseWheel(wxMouseEvent& event)
{
    // When user scrolls check for dialog overlay fix just like we do for
    // mouse down (since window has been updated)
    CheckOverlayTimer();

    if ( ! x_Handlers_handle(event, 0xFFFFFFFF, false))  {
        event.Skip();
    }
}


void CGlWidgetPane::OnMouseCaptureLost(wxMouseCaptureLostEvent& event)
{
    /// only currently active handler can capture mouse
    if(m_pCurrHandlerRec)  {
        x_DispatchEventToHandler(event, m_pCurrHandlerRec);
    }
}

void CGlWidgetPane::OnLeaveWindow(wxMouseEvent& event)
{
    if ( ! x_Handlers_handle(event, 0xFFFFFFFF, false))  {
        event.Skip();
    }
}

void CGlWidgetPane::OnKeyEvent(wxKeyEvent& event)
{   
#ifdef ATTRIB_MENU_SUPPORT
    if (event.GetEventType() == wxEVT_KEY_DOWN) {
        if (CAttribMenuInstance::GetInstance().KeyPress( event.GetKeyCode() )) {        
            Refresh();            
            return;
        }
    }
#endif

    x_HandleAccels(event);

    //trace = true;
    if ( ! x_Handlers_handle(event, 0xFFFFFFFF, false))  {
        event.Skip();
    }
    trace = false;
}


void CGlWidgetPane::OnSetFocus(wxFocusEvent& event)
{
    Refresh();
}


void CGlWidgetPane::OnKillFocus(wxFocusEvent& event)
{
    x_Handlers_handle(event, 0xFFFFFFFF, false);

    Refresh();
}

void CGlWidgetPane::OnSize(wxSizeEvent& event)
{
    if( ! x_Handlers_handle(event, 0xFFFFFFFF, true)) {
        event.Skip();
    }

    // This is for use in subclasses that want to know if size event was just 
    // part of the opengl fix.
    m_PseudoSized = false;
}

void CGlWidgetPane::OnCmdEvent(wxCommandEvent& event)
{
    if( ! x_Handlers_handle(event, 0xFFFFFFFF, true)) {
        event.Skip();
    }
}

void CGlWidgetPane::OnTipRemoved(wxCommandEvent& event)
{
	// For (occasional) windows gaphics driver issue with overlapping windows.  When
	// an overlapping tooltip is removed you may not get an 'enterwindow' event since
	// the cursor may not have gone inside the window.  
    m_EnterTimer.Restart();

	event.Skip();
}

void CGlWidgetPane::DlgOverlayFix()
{
    // Sometimes the size event will be handled before
    // DlgGLWinOverlay returns, sometimes not. Make
    // sure m_PseudoSized is set either way by setting it before
    // function is called.
#ifdef __WXMSW__
    m_PseudoSized = true;
    DlgGLWinOverlayFix(this);
#endif
}

CGlWidgetPane::CGlWidgetPane(wxWindow* parent, wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style)
    : C3DCanvas(parent, id, pos, size, style)
    , m_pCurrHandlerRec(NULL)
    , m_PopupMenuDisplayed(false)
    , m_GLOverlapFixTimer(this, WIDGET_PANE_ENTER_TIMER)
    , m_PseudoSized(false)
{
    SetBackgroundColour(wxColour(255, 255, 255));
    // May need to access opengl here so make sure context has been
    // initialized (e.g. for font metrics using texture fonts)
    x_SetupGLContext();

    // Initialize tooltip handler (virtual functions do not allow initialization in ctor)
    TTHH_Init();
}


CGlWidgetPane::~CGlWidgetPane()
{
}


int CGlWidgetPane::x_GetAreaByWindowPos(const wxPoint& pos)
{
    int w, h;
    GetClientSize(&w, &h);
    int vp_y = h - pos.y;
    return x_GetAreaByVPPos(pos.x, vp_y);
}


// default implementation, to be overriden on derived classes
int CGlWidgetPane::x_GetAreaByVPPos(int vp_x, int vp_y)
{
    return 0xFFFFFFFF;
}


///////////////////////////////////////////////////////////////////////////////
// Handlers management
bool CGlWidgetPane::x_RegisterHandler(IGlEventHandler* handler,
                                      int area, CGlPane* pane, int index)
{
    _ASSERT(handler  &&  area);
    ITERATE(THandlerRecList, it, m_lsHandlerRecs)    {
        if(it->m_pHandler == handler &&
           it->m_Area == area &&
           it->m_pPane == pane)
            return false; // already registered
    }
    SHandlerRec rec;
    rec.m_pHandler = handler;
    rec.m_Area = area;
    rec.m_pPane = pane;

    // Inserts handler into position index or at end of list 
    // if index >= inumber of handlers.
    if (index >= 0  &&  index < (int)m_lsHandlerRecs.size()) {
        int idx = 0;
        NON_CONST_ITERATE(THandlerRecList, it, m_lsHandlerRecs) {
            if (idx++ == index) {
                m_lsHandlerRecs.insert(it, rec);
                return true;
            }
        }
    } else {
        m_lsHandlerRecs.push_back(rec);
    }
    return true;
}


bool CGlWidgetPane::x_UnregisterHandler(IGlEventHandler* handler)
{
    NON_CONST_ITERATE(THandlerRecList, it, m_lsHandlerRecs)    {
        if(it->m_pHandler == handler)   {
            if (m_pCurrHandlerRec  &&  m_pCurrHandlerRec->m_pHandler == handler)
                m_pCurrHandlerRec = NULL;
            m_lsHandlerRecs.erase(it);
            return true;
        }
    }
    return false; // not registered
}


void CGlWidgetPane::x_SetCurrHandler(SHandlerRec* rec)
{
    m_pCurrHandlerRec = rec;
}


bool CGlWidgetPane::x_Handlers_handle(wxEvent& event, int area, bool ignore_curr)
{
    if(trace)   {
        LOG_POST(Info << "CGlWidgetPane::x_Handlers_handle()");
    }
    bool res = false;

    SHandlerRec* p_first = ignore_curr ? NULL : m_pCurrHandlerRec;
    if(p_first)  {
        res = x_DispatchEventToHandler(event, p_first);
        if(trace)   {
            LOG_POST(Info << "first handler res " << res);
        }
        if(res) {
            x_SetCurrHandler(p_first);
            return res;
        }
    }
    if(res == 0)    {
        if(trace)   {
            LOG_POST(Info << "iterating by handlers");
        }
        // event was not handled by current handler - iterate through over handlers
        NON_CONST_ITERATE(THandlerRecList, it, m_lsHandlerRecs) {
            IGlEventHandler* handler = it->m_pHandler;
            if(trace)   {
                LOG_POST(Info << "  handler " << handler  << "  " << typeid(*handler).name());
                LOG_POST(Info << "  p_first->m_pHandler " << (p_first ? p_first->m_pHandler : NULL));
            }
            if((it->m_Area & area)
                    &&  (p_first == NULL || handler != p_first->m_pHandler) )    {
                if(trace)   {
                    LOG_POST(Info << "x_DispatchEventToHandler " << typeid(*handler).name());
                }
                res = x_DispatchEventToHandler(event, &*it);
                if(res) {
                    x_SetCurrHandler(&(*it));
                    return res;
                }
            }
        }
        if(trace)   {
            LOG_POST(Info << "iterating by handlers - END");
        }
    }
    return false;
}


void CGlWidgetPane::x_HandleAccels(wxKeyEvent& event)
{
// Attribute menus also use these keys. Attribute menu support should be enabled only
// for local debugging (not checked in)
#ifndef ATTRIB_MENU_SUPPORT
    if (GetParent() && event.GetEventType() == wxEVT_KEY_DOWN) {
        // Allow either = or + to be used for zoom in.  This will then work for
        // either 'ctrl+' or just '+'  Of course the plus key is also '=' so we 
        // accept either.
        if ((event.GetKeyCode() == WXK_NUMPAD_ADD) || (event.GetKeyCode() == int('='))){
            wxCommandEvent e(wxEVT_COMMAND_MENU_SELECTED, eCmdZoomIn);
            GetParent()->GetEventHandler()->ProcessEvent(e);
        }
        if ((event.GetKeyCode() == WXK_NUMPAD_ADD) || (event.GetKeyCode() == int('+'))){
            wxCommandEvent e(wxEVT_COMMAND_MENU_SELECTED, eCmdZoomIn);
            GetParent()->GetEventHandler()->ProcessEvent(e);
        }       
        else if ((event.GetKeyCode() == WXK_NUMPAD_SUBTRACT) || (event.GetKeyCode() == int('-'))){
            wxCommandEvent e(wxEVT_COMMAND_MENU_SELECTED, eCmdZoomOut);
            GetParent()->GetEventHandler()->ProcessEvent(e);
        }
        else if ((event.GetKeyCode() == WXK_NUMPAD_MULTIPLY) || (event.GetKeyCode() == int('8'))){
            wxCommandEvent e(wxEVT_COMMAND_MENU_SELECTED, eCmdZoomAll);
            GetParent()->GetEventHandler()->ProcessEvent(e);
        }
    }    
#endif
}


bool CGlWidgetPane::x_DispatchEventToHandler(wxEvent& event, SHandlerRec* rec)
{
    IGlEventHandler* handler = rec->m_pHandler;

    handler->SetPane(rec->m_pPane);
    wxEvtHandler* evt_handler = handler->GetEvtHandler();
    _ASSERT(evt_handler);
    bool res = evt_handler->ProcessEvent(event);
    handler->SetPane(NULL);
    return res;
}


/*
int CGlWidgetPane::x_Handlers_handle(CGUIEvent& event, int area, bool ignore_curr)
{
    int res = 0;

    SHandlerRec* p_first = ignore_curr ? NULL : m_pCurrHandlerRec;
    if(p_first)  {
        IGlEventHandler* handler = p_first->m_pHandler;
        res = handler->handle(m_Event, *p_first->m_pPane);
        if(res) {
            x_SetCurrHandler(p_first);
            return res;
        }
    }

    if(res == 0)    {
    // event was not handled by current handler - iterate through over handlers
        NON_CONST_ITERATE(THandlerRecList, it, m_lsHandlerRecs) {
            IGlEventHandler* handler = it->m_pHandler;
            if((it->m_Area & area)
                    &&  (p_first == NULL || handler != p_first->m_pHandler) )    {
                res = handler->handle(m_Event, *it->m_pPane);
                if(res) {
                    x_SetCurrHandler(&(*it));
                    return res;
                }
            }
        }
    }
    return 0;
}
*/


////////////////////////////////////////////////////////////////////////////////
/// class CGlWidgetBase

BEGIN_EVENT_TABLE( CGlWidgetBase, wxPanel )
    EVT_SIZE(CGlWidgetBase::OnSize)
    EVT_COMMAND_SCROLL(ID_VSCROPLLBAR, CGlWidgetBase::OnScroll)
    EVT_COMMAND_SCROLL(ID_HSCROPLLBAR, CGlWidgetBase::OnScroll)

    EVT_MENU(eCmdZoomIn, CGlWidgetBase::OnZoomIn)
    EVT_MENU(eCmdZoomInX, CGlWidgetBase::OnZoomInX)
    EVT_MENU(eCmdZoomInY, CGlWidgetBase::OnZoomInY)
    EVT_MENU(eCmdZoomInMouse, CGlWidgetBase::OnZoomInMouse)

    EVT_MENU(eCmdZoomOut, CGlWidgetBase::OnZoomOut)
    EVT_MENU(eCmdZoomOutX, CGlWidgetBase::OnZoomOutX)
    EVT_MENU(eCmdZoomOutY, CGlWidgetBase::OnZoomOutY)
    EVT_MENU(eCmdZoomOutMouse, CGlWidgetBase::OnZoomOutMouse)

    EVT_MENU(eCmdZoomAll, CGlWidgetBase::OnZoomAll)
    EVT_MENU(eCmdZoomAllX, CGlWidgetBase::OnZoomAllX)
    EVT_MENU(eCmdZoomAllY, CGlWidgetBase::OnZoomAllY)

    // by default unconditionally enable all zoom commands
    EVT_UPDATE_UI_RANGE( eCmdZoomIn, eCmdSetEqualScale, CGlWidgetBase::OnEnableCmdUpdate )

END_EVENT_TABLE()

CGlWidgetBase::CGlWidgetBase(
                  wxWindow* parent,
                  wxWindowID id,
                  const wxPoint& pos,
                  const wxSize& size,
                  long style,
                  const wxString& name)
    : wxPanel(parent, id, pos, size, style, name)
{
}


CGlWidgetBase::~CGlWidgetBase()
{
}


void CGlWidgetBase::Create()
{
    x_CreateControls();
    x_SetPortLimits();
    GetPort().ZoomAll();    
}


void CGlWidgetBase::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}


void CGlWidgetBase::LoadSettings()
{
    LOG_POST(Error << "CGlWidgetBase::LoadSettings() - override in derived classes");
}


void CGlWidgetBase::SaveSettings() const
{
    LOG_POST(Error << "CGlWidgetBase::SaveSettings() - override in derived classes");
}


CGlWidgetPane* CGlWidgetBase::x_GetPane()
{
    return (CGlWidgetPane*) FindWindowById(ID_GLCHILDPANE, this);
}


void CGlWidgetBase::OnSize(wxSizeEvent& event)
{
    wxSize size = event.GetSize();
    if (size.GetWidth() == 0 || size.GetHeight() == 0)
        return;

    Layout();

    CGlWidgetPane* child_pane = x_GetPane();
    if ( ! child_pane)
        return;

    int w, h;
    child_pane->GetClientSize(&w, &h);

    TVPRect rcVP(0, 0, w - 1, h - 1);
    GetPort().SetViewport(rcVP);
    GetPort().AdjustToLimits();
    x_UpdateScrollbars();
    Refresh();
}

void CGlWidgetBase::OnScroll(wxScrollEvent& event)
{
    // When user scrolls check for dialog overlay fix just like we do for
    // mouse down (since window has been updated)
    CGlWidgetPane* p = x_GetPane();
    if (p != NULL)
        p->CheckOverlayTimer();

    if (event.GetOrientation() == wxHORIZONTAL) {
        x_OnScrollX(event.GetPosition());
        x_UpdateOnHScroll();
    }
    else if (event.GetOrientation() == wxVERTICAL) {
        x_OnScrollY(event.GetPosition());
        x_UpdateOnVScroll();
    }

    // Don't want to record individual events when user is dragging
    // the scrollbar (just record the mouse up/release in that case).
    // But we do want to save position info for page up/down or clicking
    // on the scroll track (jumping straight to a new position).
    if (event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE ||
        event.GetEventType() == wxEVT_SCROLL_TOP ||
        event.GetEventType() == wxEVT_SCROLL_BOTTOM ||
        event.GetEventType() == wxEVT_SCROLL_LINEUP || 
        event.GetEventType() == wxEVT_SCROLL_LINEDOWN ||
        event.GetEventType() == wxEVT_SCROLL_PAGEUP ||
        event.GetEventType() == wxEVT_SCROLL_PAGEDOWN) {
            x_SaveStates();
    }
}


void CGlWidgetBase::OnZoomIn(wxCommandEvent& event)
{
    x_ZoomIn(CGlPane::fZoomXY);
}


void CGlWidgetBase::OnZoomInX(wxCommandEvent& event)
{
    x_ZoomIn(CGlPane::fZoomX);
}


void CGlWidgetBase::OnZoomInY(wxCommandEvent& event)
{
    x_ZoomIn(CGlPane::fZoomY);
}


void CGlWidgetBase::OnZoomInMouse(wxCommandEvent& event)
{
    CGlPane& port = GetPort();
    if(port.IsZoomInAvaiable())  {
        TModelUnit factor = port.GetZoomFactor();
        port.ZoomPoint(m_PopupPoint, factor, CGlPane::fZoomX | CGlPane::fZoomY);        
        x_UpdateOnZoom();
        x_SaveStates();
    }
}


void CGlWidgetBase::OnZoomOut(wxCommandEvent& event)
{
    x_ZoomOut(CGlPane::fZoomXY);
}


void CGlWidgetBase::OnZoomOutX(wxCommandEvent& event)
{
    x_ZoomOut(CGlPane::fZoomX);
}


void CGlWidgetBase::OnZoomOutY(wxCommandEvent& event)
{
    x_ZoomOut(CGlPane::fZoomY);
}


void CGlWidgetBase::OnZoomOutMouse(wxCommandEvent& event)
{
    CGlPane& port = GetPort();
    if(port.IsZoomOutAvaiable())  {
        TModelUnit factor = port.GetZoomFactor();
        port.ZoomPoint(m_PopupPoint, 1.0 / factor,
                       CGlPane::fZoomX | CGlPane::fZoomY);        
        x_UpdateOnZoom();
        x_SaveStates();
    }
}


void CGlWidgetBase::OnZoomAll(wxCommandEvent& event)
{
    x_ZoomAll(CGlPane::fZoomXY);
}


void CGlWidgetBase::OnZoomAllX(wxCommandEvent& event)
{
    x_ZoomAll(CGlPane::fZoomX);
}


void CGlWidgetBase::OnZoomAllY(wxCommandEvent& event)
{
    x_ZoomAll(CGlPane::fZoomY);
}


void CGlWidgetBase::OnEnableCmdUpdate(wxUpdateUIEvent& event)
{
    event.Enable(true);
}


void CGlWidgetBase::ZoomRect(const TModelRect&  rc)
{
   GetPort().ZoomRect(rc);   
   x_UpdateOnZoom();
   x_SaveStates();
}


void CGlWidgetBase::ZoomPoint(const TModelPoint& point, 
                              TModelUnit factor, 
                              CGlPane::EZoomOptions options)
{
    GetPort().ZoomPoint(point, factor, (int)options);    
    x_UpdateOnZoom();
    x_SaveStates();
}


void CGlWidgetBase::Scroll(TModelUnit d_x, TModelUnit d_y)
{
    GetPort().Scroll(d_x, d_y);
    x_UpdateOnZoom();
}

void CGlWidgetBase::x_ZoomIn(int options)
{
    CGlPane& port = GetPort();
    if(port.IsZoomInAvaiable())  { //### options
        port.ZoomInCenter(options);        
        x_UpdateOnZoom();
        x_SaveStates();
    }
}


void CGlWidgetBase::x_ZoomOut(int options)
{
    CGlPane& port = GetPort();
    if(port.IsZoomOutAvaiable())  {
        port.ZoomOutCenter(options);       
        x_UpdateOnZoom();
        x_SaveStates();
    }
}


void CGlWidgetBase::x_ZoomAll(int options)
{
    CGlPane& port = GetPort();
    if(port.IsZoomOutAvaiable())  {
        port.ZoomAll(options);        
        x_UpdateOnZoom();
        x_SaveStates();
    }
}

bool CGlWidgetBase::DoPopupMenu(wxMenu *menu, int x, int y)
{
    x_GetPane()->SetPopupMenuDisplayed(true);
    
    // blocking call..
    bool b = wxPanel::DoPopupMenu(menu,x,y);

    x_GetPane()->SetPopupMenuDisplayed(false);

    return b;
}

void CGlWidgetBase::x_CreateControls()
{
    _ASSERT( ! x_GetPane());

    x_CreatePane();

    CGlWidgetPane* child_pane = x_GetPane();
    _ASSERT(child_pane);

    wxFlexGridSizer* sizer = new wxFlexGridSizer(2, 2, 0, 0);
    this->SetSizer(sizer);

    sizer->AddGrowableCol(0);
    sizer->AddGrowableRow(0);

    sizer->Add(child_pane, 1, wxEXPAND);

    wxScrollBar*
    vsb = new CUnfocusedScrollbar(this, ID_VSCROPLLBAR, wxDefaultPosition,
                                  wxDefaultSize, wxSB_VERTICAL);
    sizer->Add(vsb, 0, wxEXPAND);

    wxScrollBar*
    hsb = new CUnfocusedScrollbar(this, ID_HSCROPLLBAR, wxDefaultPosition,
                                  wxDefaultSize, wxSB_HORIZONTAL);
    sizer->Add(hsb, 0, wxEXPAND);

    vsb->SetMinSize(wxSize(wxDefaultCoord, 4));
    hsb->SetMinSize(wxSize(4, wxDefaultCoord));

    sizer->AddSpacer(1);
}


void CGlWidgetBase::x_RedrawControls()
{
    CGlWidgetPane* child_pane = x_GetPane();
    if (child_pane) {
        child_pane->Refresh();
    }
}


////////////////////////////////////////////////////////////////////////////////
/// Update handlers

void CGlWidgetBase::x_Update()
{
    /// Update data strutures
    /// update m_Port (x_SetPortLimits)
    /// update visible rect in m_Port
    /// x_UpdateScrollbars() if necessary
    /// x_UpdatePanes() ?
    /// x_RedrawControls()
}

void CGlWidgetBase::x_UpdateOnZoom()
{
    x_UpdateScrollbars();
    x_RedrawControls();
    NotifyVisibleRangeChanged();
}

void CGlWidgetBase::x_UpdateOnHScroll()
{
    x_UpdateOnZoom();
}

void CGlWidgetBase::x_UpdateOnVScroll()
{
    x_UpdateOnZoom();
}

void CGlWidgetBase::NotifyVisibleRangeChanged()
{
    /// notify our parent that we've changed
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetRangeChanged);
    Send(&evt, ePool_Parent);
    Refresh();
}


static const double kPageInc = 0.8;
static const double kLineDiv = 50;


// we map the real model range to integer scrol range [0, kScrollRange]
// CNormalizer provides functions for converting between the model and scroll ranges
void CGlWidgetBase::x_UpdateScrollbars()
{
    wxScrollBar* scroll_bar_v = (wxScrollBar*)FindWindow(ID_VSCROPLLBAR);
    wxScrollBar* scroll_bar_h = (wxScrollBar*)FindWindow(ID_HSCROPLLBAR);

    CGlPane& port = GetPort();
    const TModelRect& rc_all = port.GetModelLimitsRect();
    const TModelRect& rc_vis = port.GetVisibleRect();

    if (scroll_bar_h)   {
        int range = (int)min(rc_all.Width(), (kLineDiv*rc_all.Width())/rc_vis.Width());
        // Need a min for cases where limits are small, e.g. 0.0 to 1.0
        range = std::max((int)kLineDiv,range);

        CNormalizer norm(rc_all.Left(), rc_all.Right(), range);
        int position = norm.RealToInt(rc_vis.Left());
        int thumb_size = norm.SizeToInt(rc_vis.Width());
        int page_size = norm.SizeToInt(rc_vis.Width() * kPageInc);

        scroll_bar_h->SetScrollbar(position, thumb_size, range, page_size);
    }
    if (scroll_bar_v)   {
        int range, position, page_size, thumb_size;

        if( rc_all.Height() > 0 ){
            range = (int) min( rc_all.Height(), (kLineDiv*rc_all.Height())/rc_vis.Height() );
            // Need a min for cases where limits are small, e.g. 0.0 to 1.0
            range = std::max((int)kLineDiv,range);

            // Remember, vertical dimension is flipped 
            // so calculations differ from horisontal case
            CNormalizer norm( rc_all.Bottom(), rc_all.Top(), range );

            // use Top, not Bottom because of the flipping
            position = range - norm.RealToInt( rc_vis.Top() );

            page_size = norm.SizeToInt( rc_vis.Height() * kPageInc );
            thumb_size = norm.SizeToInt( rc_vis.Height() );

        } else {
            range = (int) min( -rc_all.Height(), (kLineDiv*rc_all.Height())/rc_vis.Height() );
            // Need a min for cases where limits are small, e.g. 0.0 to 1.0
            range = std::max((int)kLineDiv,range);

            // Remember, vertical dimension is flipped 
            // so calculations differ from horisontal case
            CNormalizer norm( rc_all.Top(), rc_all.Bottom(), range );

            // use Top, not Bottom because of the flipping
            position = norm.RealToInt( rc_vis.Top() );

            page_size = norm.SizeToInt( -rc_vis.Height() * kPageInc );
            thumb_size = norm.SizeToInt( -rc_vis.Height() );
        }

        scroll_bar_v->SetScrollbar( position, thumb_size, range, page_size );
    }
}


void CGlWidgetBase::x_OnScrollX(int pos)
{
    CGlWidgetPane* child_pane = x_GetPane();
    CGlPane& port = GetPort();
    const TModelRect& rc_all = port.GetModelLimitsRect();
    const TModelRect& rc_vis = port.GetVisibleRect();

    int range = (int) min(rc_all.Width(), (kLineDiv*rc_all.Width())/rc_vis.Width());
    // Need a min for cases where limits are small, e.g. 0.0 to 1.0
    range = std::max((int)kLineDiv,range);

    CNormalizer norm(rc_all.Left(), rc_all.Right(), range);
    double d_x = norm.IntToReal(pos) - rc_vis.Left();
    port.Scroll(d_x, 0);

    if (child_pane) {
        child_pane->Refresh();
    }
}


void CGlWidgetBase::x_OnScrollY(int pos)
{
    CGlWidgetPane* child_pane = x_GetPane();
    CGlPane& port = GetPort();
    const TModelRect& rc_all = port.GetModelLimitsRect();
    const TModelRect& rc_vis = port.GetVisibleRect();

    int range;
    double d_y;

    if( rc_all.Height() > 0 ){
        range = (int) min( rc_all.Height(), (kLineDiv*rc_all.Height())/rc_vis.Height() );
        // Need a min for cases where limits are small, e.g. 0.0 to 1.0
        range = std::max((int)kLineDiv,range);

        CNormalizer norm( rc_all.Bottom(), rc_all.Top(), range );
        d_y = norm.IntToReal( range-pos ) - rc_vis.Top();
    } else {
        // Remember, vertical dimension is flipped 
        // so calculations differ from horisontal case
        range = (int) min( -rc_all.Height(), (kLineDiv*rc_all.Height())/rc_vis.Height() );
        // Need a min for cases where limits are small, e.g. 0.0 to 1.0
        range = std::max((int)kLineDiv,range);

        CNormalizer norm( rc_all.Top(), rc_all.Bottom(), range );
        d_y = norm.IntToReal( pos ) - rc_vis.Top();
    }

    port.Scroll( 0, d_y );

    if( child_pane ){
        child_pane->Refresh();
    }
}


void CGlWidgetBase::x_ShowDecoratedPopupMenu(wxMenu* menu)
{
    // send "Decorate" event to listeners
    CPopupMenuEvent evt(this, menu);
    Send(&evt, eDispatch_Default, ePool_Parent);

    // obtain the modified menu and clean it empty sections
    menu = evt.GetMenu();
    CleanupSeparators(*menu);

    // connect contributed wxEvtHandlers so that they can handle commands
    vector<wxEvtHandler*>& handlers = evt.GetHandlers();
    for( size_t  i = 0;  i < handlers.size();  i++  )   {
        PushEventHandler(handlers[i]);
    }

    // show the menu
    PopupMenu(menu);

    delete menu;

    // disconnect and destroy the handlers
    for( size_t  i = 0;  i < handlers.size();  i++  )   {
        wxEvtHandler* h = PopEventHandler();
        _ASSERT(h == handlers[i]);
        delete h;
    }
}


// dummy event handler
void CGlWidgetBase::OnAllEvents(CViewEvent::TEventObject  evt)
{
    _TRACE("unhandled event: CGlWidgetBase:OnViewEvent()");
}


END_NCBI_SCOPE
