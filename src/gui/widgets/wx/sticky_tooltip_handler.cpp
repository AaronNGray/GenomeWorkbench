/*  $Id: sticky_tooltip_handler.cpp 40202 2018-01-05 20:01:46Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbidbg.hpp>

#include <gui/widgets/wx/sticky_tooltip_handler.hpp>

#include <gui/framework/window_manager_service.hpp>
#include <gui/framework/service.hpp>

#include <gui/widgets/wx/dock_frames.hpp>
#include <gui/widgets/wx/dock_manager.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/utils/command.hpp>

#include <wx/settings.h>
#include <wx/window.h>
#include <wx/app.h>

BEGIN_NCBI_SCOPE


#define TTMGR_TIMER_ID 62210
#define TTMGR_TIMER_CLEAR_UNPINNED 62211

#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
std::vector<std::pair<wxWindow*,IStickyTooltipHandlerHost::CWindowRef> > 
    IStickyTooltipHandlerHost::sm_Windows;
#endif

IStickyTooltipHandlerHost::IStickyTooltipHandlerHost() 
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
:  m_TipGroupWinRef(NULL)
#endif
{
    m_ServiceLocator = NULL;
}

IStickyTooltipHandlerHost::~IStickyTooltipHandlerHost() 
{
}

void IStickyTooltipHandlerHost::TTHH_Init() 
{
    InitWindowGroup();
}

wxFrame* IStickyTooltipHandlerHost::GetUnderlyingFrame()
{
    CFloatingFrame* ff = NULL;
    wxFrame* f = NULL;

    for( wxWindow* p = TTHH_GetWindow(); p != NULL; p = p->GetParent() ){
        if( f == NULL ){ 
            f = dynamic_cast<wxFrame*>(p);
        }
        if( ff == NULL ){
           ff = dynamic_cast<CFloatingFrame*>(p);        
        }
    }

    _ASSERT( (f!=NULL) || (ff!=NULL) );
  
    if( (ff!=NULL) && (f!=ff) ){
        f = ff;
    }

    return f;
}

bool IStickyTooltipHandlerHost::IsFloatingWindow()
{
    wxFrame* f = GetUnderlyingFrame();
    if( f == NULL ) return false;

    return (dynamic_cast<CFloatingFrame*>(f) != NULL);
}

void IStickyTooltipHandlerHost::RaiseOverlappingWindows(wxRect r)
{
// On mac Carbon, tooltip windows do not jump to front.  And raising
// other windows causes some focus issues (need to click in
// your current window to get focus back at times)
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
    if (IsFloatingWindow())
        return;

    if (m_ServiceLocator != NULL) {
        IWindowManagerService* win_srv =
                        m_ServiceLocator->GetServiceByType<IWindowManagerService>();

        if (win_srv!=NULL) {
            // Bring all floating windows to the front, and then set the focus back
            // since raising those windows will change focus.            
            win_srv->RaiseFloatingWindowsInZOrder(r);
            TTHH_GetWindow()->SetFocus();
        }
    }
#endif
}

bool IStickyTooltipHandlerHost::IsDragging()
{
    if (m_ServiceLocator != NULL) {
        IWindowManagerService* win_srv =
            m_ServiceLocator->GetServiceByType<IWindowManagerService>();

        if (win_srv!=NULL) {
            return (win_srv->IsDragging());
        }
    }    

    return false;
}



void IStickyTooltipHandlerHost::InitWindowGroup(int dock_count)
{
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
    if (TTHH_GetWindow() == NULL ) {
        return;
    }

    wxFrame* f = GetUnderlyingFrame();
    std::vector<std::pair<wxWindow*,CWindowRef> >::iterator iter;

    WindowGroupRef view_ref;
    WindowGroupRef top_ref;

    for (iter=sm_Windows.begin(); iter != sm_Windows.end(); ++iter) {
        if ( (*iter).first == (wxWindow*)f ) {
            break;   
        }
    }

    bool floater = IsFloatingWindow();

    if (iter != sm_Windows.end()) {
        m_TipGroupWinRef = (*iter).second.m_Ref;
        return;
    }

    WindowGroupRef group_class = NULL;
    if (floater) {
	    group_class = GetWindowGroupOfClass(kFloatingWindowClass);
	    //_TRACE("Setting window class to floating");
    }
    else  {
	    group_class = GetWindowGroupOfClass(kDocumentWindowClass);
	    //_TRACE("Setting window class to document ");
    }
	
	
    CreateWindowGroup( kWindowGroupAttrSelectAsLayer | 
                       kWindowGroupAttrHideOnCollapse, &m_TipGroupWinRef );
    CreateWindowGroup( kWindowGroupAttrSelectAsLayer|  
                       kWindowGroupAttrHideOnCollapse, &view_ref );
    CreateWindowGroup( kWindowGroupAttrSelectAsLayer | 
                       kWindowGroupAttrHideOnCollapse, &top_ref );
	
    SetWindowGroupParent(m_TipGroupWinRef, top_ref);
    SetWindowGroupParent(view_ref, m_TipGroupWinRef);
    SetWindowGroupParent(top_ref, group_class);
	
    SetWindowGroup((WindowRef)f->MacGetTopLevelWindowRef(), view_ref);

    sm_Windows.push_back(std::pair<wxWindow*, CWindowRef>((wxWindow*)f,CWindowRef(m_TipGroupWinRef, dock_count)));
#endif
}


/*****************************************************************************/
/*****************************************************************************/


BEGIN_EVENT_TABLE(CStickyTooltipHandler, wxTimer)
    EVT_MOTION(CStickyTooltipHandler::OnMotion)
    EVT_RIGHT_DOWN(CStickyTooltipHandler::OnRightMouseDown)
    EVT_MIDDLE_DOWN(CStickyTooltipHandler::OnRightMouseDown)
    EVT_LEAVE_WINDOW(CStickyTooltipHandler::OnLeaveWindow)
    EVT_MOUSEWHEEL(CStickyTooltipHandler::OnMouseWheel)
    EVT_SIZE(CStickyTooltipHandler::OnSize)
    EVT_TIMER(TTMGR_TIMER_ID, CStickyTooltipHandler::OnTimer)
    EVT_TIMER(TTMGR_TIMER_CLEAR_UNPINNED, CStickyTooltipHandler::OnTimer)
        
    EVT_BUTTON(eCmdParentMove, CStickyTooltipHandler::OnWindowMoveCmd)
    EVT_BUTTON(eCmdSuspend, CStickyTooltipHandler::OnSuspendCmd)
    EVT_BUTTON(eCmdReCreate, CStickyTooltipHandler::OnReCreateCmd)
    EVT_BUTTON(eCmdDockMainWindow, CStickyTooltipHandler::OnMainWindowDockCmd)

    EVT_BUTTON(eCmdParentActivate, CStickyTooltipHandler::OnActivateCmd)
    EVT_BUTTON(eCmdParentShow, CStickyTooltipHandler::OnWindowShowCmd)    
    EVT_BUTTON(eCmdParentHide, CStickyTooltipHandler::OnWindowHideCmd)
    EVT_BUTTON(eCmdPinTip, CStickyTooltipHandler::OnPinTip)
    EVT_BUTTON(eCmdReleaseTip, CStickyTooltipHandler::OnTipRelease)
    EVT_BUTTON(eCmdMoveTip, CStickyTooltipHandler::OnTipMove)
END_EVENT_TABLE()



CStickyTooltipHandler::CStickyTooltipHandler() 
: m_Host(NULL)
, m_UnpinnedTip(NULL) 
, m_ActiveTip(NULL)
, m_Timer(this, TTMGR_TIMER_ID)
, m_SuspendCount(0)
, m_TimerClearUnpinned(this, TTMGR_TIMER_CLEAR_UNPINNED)
, m_MouseMoveThreshold(2.0f)
, m_UnpinnedMouseMoveThreshold(4.0f)
, m_ClearUnpinnedDelay(250)
, m_PopupDistance(0.22f)
// Base popup delay according to what is typical for individual platforms.
// Currently cannot be configured.
#if defined(NCBI_OS_MSWIN)
, m_TipPopupDelay(500)
#elif defined(NCBI_OS_MAC)
, m_TipPopupDelay(1000)
#elif defined(NCBI_OS_UNIX)
// default for GTK
, m_TipPopupDelay(500)
#else
// In case we compile something else..
, m_TipPopupDelay(500)
#endif
{
    RegisterCommands();
}

CStickyTooltipHandler::~CStickyTooltipHandler() 
{
    unsigned int i;

    if (NULL != m_UnpinnedTip) {
        m_UnpinnedTip->Show(false);
        m_UnpinnedTip->Destroy();
        m_UnpinnedTip = NULL;
    }

    for (i=0; i<m_ToolTips.size(); ++i) {
        m_ToolTips[i]->Show(false);
        m_ToolTips[i]->Destroy();
    }
    m_ToolTips.clear();

    for (i=0; i<m_SuspendedTips.size(); ++i) {
        m_SuspendedTips[i]->Destroy();
    }
    m_SuspendedTips.clear();

    UpdateTips();

    m_Timer.Stop();
}

void CStickyTooltipHandler::RegisterCommands()
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;
    
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    cmd_reg.RegisterCommand(eCmdCopyTipText, "Copy All", "Copy All", "", "");
    cmd_reg.RegisterCommand(eCmdSelectTipText, "Enable Select", "Enable Select", "", "", "", "", wxITEM_CHECK);
    cmd_reg.RegisterCommand(eCmdCopySelectedTipText, "Copy Selected", "Copy Selected", "", "");
    
}

void CStickyTooltipHandler::AddTip(CTooltipFrame* tt)
{
    std::vector<CTooltipFrame*>::iterator iter;

    // Only add tips for items that do not already have tips open
    for (iter=m_ToolTips.begin(); iter!=m_ToolTips.end(); ++iter) {
        if (*iter == tt) 
            break;
    }
    
    if (iter == m_ToolTips.end()) {
        m_ToolTips.push_back(tt);

        // The timer lets us bring forward the tooltip the mouse is on
        if (m_ToolTips.size() == 1)
            m_Timer.Start(200);
    }

    UpdateTips();
}

void CStickyTooltipHandler::AddUnpinned(CTooltipFrame* tt) 
{
    // Only one unpinned tip at a time (others should auto-close).
    ClearUnpinned();

    m_UnpinnedTip = tt;

    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, eCmdTipAdded);
    evt.SetEventObject(m_UnpinnedTip);
    m_Host->TTHH_GetWindow()->GetEventHandler()->ProcessEvent(evt);

    UpdateTips();
}

void CStickyTooltipHandler::PinUnpinned(CTooltipFrame* tt)
{
    _ASSERT(tt != NULL && tt == m_UnpinnedTip);

    // Move a tip from being the upinned tip to the list of pinned tips
    m_UnpinnedTip->SetPinned(true);
    AddTip(m_UnpinnedTip);
    m_UnpinnedTip = NULL;
}

void CStickyTooltipHandler::UnpinPinned(CTooltipFrame* tt)
{
    std::vector<CTooltipFrame*>::iterator iter;

    iter = std::find(m_ToolTips.begin(), m_ToolTips.end(), tt);

    _ASSERT(iter != m_ToolTips.end());

    // should already be clear, but just in case:
    ClearUnpinned();

    // User has pressed unpin-button on tip, so move it to unpinned
    (*iter)->SetPinned(false);
    if (m_ActiveTip == *iter)
        m_ActiveTip = NULL;
    m_UnpinnedTip = *iter;
    m_ToolTips.erase(iter);

    if (m_ToolTips.size() == 0)
        m_Timer.Stop();

    UpdateTips();
}

void CStickyTooltipHandler::ParentMove(const wxPoint&screen_pos)
{
    std::vector<CTooltipFrame*>::iterator iter;

    // Move all tips based on new window position
    for (iter=m_ToolTips.begin(); iter!=m_ToolTips.end(); ++iter) {
        (*iter)->UpdateScreenPosition(screen_pos);
    }

    UpdateTips();
}

void CStickyTooltipHandler::ParentShow(bool show)
{
    std::vector<CTooltipFrame*>::iterator iter;
    wxRect combined_rect;

    // Parent shown (maybe because tip selected) so show all tips
    for (iter=m_ToolTips.begin(); iter!=m_ToolTips.end(); ++iter) {
        (*iter)->Show(show);
        
        // When a notebook page is displayed, its tips must be checked against
        // bounds that could have changed while the page was hidden.  Pass 'true'
        // to indicate window may currently be hidden, but is about to show.
        if (show) {
            (*iter)->FitToWindow(true);
        }
        
        combined_rect.Union((*iter)->GetRect());
    }

    // Clear any unpinned tips before we hide (should generally not be there anyway if 
    // mouse if off active window...)
    ClearUnpinned();

    m_Host->RaiseOverlappingWindows(combined_rect);

    UpdateTips();
}

void CStickyTooltipHandler::Resize()
{
    ParentMove(m_Host->TTHH_GetWindow()->GetScreenPosition());
    wxRect combined_rect;

    // Parent window resized, make sure all toolips fit in resized parent
    std::vector<CTooltipFrame*>::iterator iter;
    for (iter=m_ToolTips.begin(); iter!=m_ToolTips.end(); ++iter) {
        (*iter)->FitToWindow();
        combined_rect.Union((*iter)->GetRect());       
    }

    m_Host->RaiseOverlappingWindows(combined_rect);

    UpdateTips();
}

// could I use EVT_ENTER_WINDOW to do this?
void CStickyTooltipHandler::OnTimer(wxTimerEvent& event)
{
    wxPoint m = wxGetMousePosition();
    if (event.GetId() == TTMGR_TIMER_CLEAR_UNPINNED) {        
        if (NULL != m_UnpinnedTip) {
            if (!m_UnpinnedTip->GetScreenRect().Contains(m)) { 
                ClearUnpinned();
                // Check for a new tip at the new position right away:
                wxPoint mouse_pos =
                    m - m_Host->TTHH_GetWindow()->GetScreenPosition();
                CheckForNewTip(mouse_pos);
            }
        }
    }
    else if (event.GetId() == TTMGR_TIMER_ID && m_ToolTips.size() > 0) {
        std::vector<CTooltipFrame*>::iterator iter;

        // If a popup menu is displayed, or the mouse has been captured for
        // some reason (zooming, panning, marquee select..) don't activate
        // any tooltips we happen to be passing over.
        if (m_Host->TTHH_PopupMenuDisplayed() || 
            wxWindow::GetCapture() != NULL)
            return;

        if (m_Host->IsDragging())
            return;

        // If mouse was on a tip at last check return (don't switch the tip). 
        // If it has moved from that tip, fit the tip back to the window.
        if (NULL != m_ActiveTip) {
            // don't switch focus away from tip if the tip has its own popup menu
            // displayed.
            if (m_ActiveTip->TipPopupMenuDisplayed())
                return;

	        // If the mouse has moved away from the tip, make it inactive:
            if (!m_ActiveTip->GetScreenRect().Contains(m) && 
                m_ActiveTip->IsShown()) {

                // Send event to host window so it can un-highlight
                // the glyph/element belonging to this tooltip
                wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, eCmdTipInactive);
                evt.SetEventObject(m_ActiveTip);
                m_Host->TTHH_GetWindow()->GetEventHandler()->ProcessEvent(evt);

                m_ActiveTip->FitToWindow();

                // If the tooltip has a url, and the user launches a browser, bringing the
                // other overlapping windows to front will also put gbench in front
                // of the launched browser, which is bad. Now if the tip is in front
                // of another gbench window, this 'hack' will allow it to stay in front
                // of that window until the user clicks on it, resizes the window etc.
                if (m_ActiveTip->LaunchedBrowser()) {                   
                    m_ActiveTip->SetLaunchedFalse();
                }
                else {
                    m_Host->RaiseOverlappingWindows(m_ActiveTip->GetRect());
                }

                m_ActiveTip = NULL;
            }
            else {
                return;
            }
        }

        // This tool tip manager window has focus if it, any of its parents
        // or any of the tool tips on the window have focus.  Do not
        // pop tool tips to the front if the window does not have focus.
        bool has_focus = false;
        wxWindow* w = m_Host->TTHH_GetWindow();
        wxWindow* focus_win = wxWindow::FindFocus();

        if (focus_win == NULL) {
            return;
        }

        // Check if this (tooltip-supporting) window is a child of the current 
        // focus window. Do not go past a CFloatingFrame in the parent-hierachy, 
        // because that is a top-level window and if the current focus window
        // is the main window, all (floating) frames are its children. (this way
        // only widgets docked in the main window will be included as children)              
        bool top = false;
        while (!has_focus && !top) {
            if (focus_win == w) {
                has_focus = true;
            }                            
            w = w->GetParent();
            if (w == NULL ||
                wxString(w->GetClassInfo()->GetClassName()) == wxT("CFloatingFrame"))
                top = true;
        }

        // If a parent of the this (tooltip) windows doesn't have focus, check to
        // see if one of the window's tooltips has focus (that also would make it
        // the active window).  Also, If there is an unpinned tip and the mouse 
        // is inside of it, do not try to switch to another tip
        if (m_UnpinnedTip != NULL) {
            if (m_UnpinnedTip->GetScreenRect().Contains(m))
                return;
            has_focus = checkChildFocus(focus_win, m_UnpinnedTip);
        }

        for (iter=m_ToolTips.begin(); iter!=m_ToolTips.end() && !has_focus;
             ++iter) {
            has_focus = checkChildFocus(focus_win, *iter);
        }        

        // This window, including sub-windows and enclosing frame(s), 
        // does not have focus
        if (!has_focus) {            
            return;
        }

        bool found = false;

        // See if the mouse has moved inside a different tip
        for (iter=m_ToolTips.begin(); iter!=m_ToolTips.end() && !found; ++iter) {
            wxRect r = (*iter)->GetScreenRect();
            if (r.Contains(m) && (*iter)->IsShown()) {
                m_ActiveTip = *iter;
                m_ActiveTip->SetToDefaultSize(); 
#ifdef __WXOSX_COCOA__
                m_ActiveTip->RemoveChildWindow();
                m_ActiveTip->AddChildWindow();
#else
                m_ActiveTip->Raise();
#endif
                // If the user has entered another tooltip, that is (also) cause for
                // clearing the unpinned tip (if any) since if the user had wanted
                // to keep it, they would pin it before going to another tip
                ClearUnpinned();

                // Send command to host window so it can highlight
                // the screen element that matches this tip
                wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, eCmdTipActive);
                evt.SetEventObject(m_ActiveTip);
                m_Host->TTHH_GetWindow()->GetEventHandler()->ProcessEvent(evt);

#ifdef NCBI_OS_LINUX
                // without this on GTK, we may wind up with a NULL focus, which 
                // prevents subsequent updates from working.
                m_ActiveTip->SetFocus();
#endif
                found = true;
            }            
        }
    }
}

void CStickyTooltipHandler::Suspend()
{
    ParentShow(false);
    ClearUnpinned();
    m_ActiveTip = NULL;

    // Suspend may be called more than once prior to ReCreate so don't 
    // erase suspended tips
    m_SuspendedTips.insert(m_SuspendedTips.end(), m_ToolTips.begin(), 
                           m_ToolTips.end());
 
    ++m_SuspendCount;

    m_ToolTips.clear();

    UpdateTips();
}

void CStickyTooltipHandler::ReCreate(int dock_count)
{
    std::vector<CTooltipFrame*>::iterator iter;

    --m_SuspendCount;
    //if (m_SuspendCount > 0)
    //    return;

    // If there are suspended tips there should be no active ones.  If you 
    // call InitWindowGroups, all tip windows Must be re-created (on MAC)
    if (m_SuspendedTips.size() > 0)
        _ASSERT(m_ToolTips.size() == 0);

    m_Host->InitWindowGroup(dock_count);
    
    // Some windows (such as in notebook) may be in hidden when drag/drop 
    // is completed - we do this to keep the tooltips of those tabs hidden 
    // as well.
    bool is_visible = m_Host->TTHH_GetWindow()->IsShownOnScreen();

    // Create a new copy of each tip, based on content and position of 
    // suspended tips, then delete suspended tips (if we don't do this,
    // parent window hierarchy is no longer correct and tips behave funny).
    for (iter=m_SuspendedTips.begin(); iter!=m_SuspendedTips.end(); ++iter)
    {      
        CTooltipFrame* tt = new CTooltipFrame( (*iter)->GetParent()
                                              ,(*iter)->GetName()
              ,(*iter)->GetParent()->GetScreenPosition() + (*iter)->GetPos()
                                              ,wxDefaultSize
                                              ,(*iter)->GetTipInfo()
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
                                              ,m_Host->GetWindowGroup()
#endif
                                              );    
        tt->SetTargetSize( (*iter)->GetTargetSize() );
        tt->Show(is_visible);
        tt->SetPinned(true);
        tt->SetLinkEventHandler(this);

        // Set pin graphic
        wxCommandEvent dummy_event;
        tt->OnPinButton(dummy_event);

        tt->FitToWindow();      
        m_Host->RaiseOverlappingWindows(tt->GetRect());
        (*iter)->Destroy();
        AddTip(tt);
    }

    m_SuspendedTips.clear();

    UpdateTips();
}

// On Mac frames other than the main app can appear in front of 
// their tool tips when they get focus. (but that is handled in 
// activate now)
void CStickyTooltipHandler::OnSetFocus()
{
}

void CStickyTooltipHandler::ClearUnpinned()
{
    if (NULL != m_UnpinnedTip) {       
        // Send event to host window so it can un-highlight
        // the glyph/elelemnt if it is currently active
        wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, eCmdTipInactive);
        evt.SetEventObject(m_UnpinnedTip);
        m_Host->TTHH_GetWindow()->GetEventHandler()->ProcessEvent(evt);

        // Separate command for keeping track of when tooltips are deleted
        // (The eCmdTipInactive tracks whether tip has focus, not whether it
        //  has been removed).
        wxCommandEvent evt2(wxEVT_COMMAND_BUTTON_CLICKED, eCmdTipRemoved);
        evt2.SetEventObject(m_UnpinnedTip);
        m_Host->TTHH_GetWindow()->GetEventHandler()->ProcessEvent(evt2);

        m_UnpinnedTip->Show(false);
        m_UnpinnedTip->Destroy();	
        m_UnpinnedTip = NULL;

        m_TimerClearUnpinned.Stop();

        UpdateTips();
    }
}
void CStickyTooltipHandler::OnSize(wxSizeEvent& event)
{
    Resize();
    event.Skip();
}

// Timer update function for tip manager.  The timer to call this is
// kicked off when we see a tip applies to the current mouse location,
// and this creates the tip if the mouse is still close enough that
// the tip is still 'needed'.
void CStickyTooltipHandler::Notify()
{
    wxPoint p = wxGetMousePosition();
    wxPoint delta = m_TipInitPos - p;

    wxRect screen_rect = m_Host->TTHH_GetWindow()->GetScreenRect();

    // Don't pop up a tool tip if the cursor has moved outside the window
    if (!screen_rect.Contains(p))
        return;

    // Don't pop up a tool tip if the right-click menu has come up.
    if (m_Host->TTHH_PopupMenuDisplayed())
        return;

    wxPoint unused_pt;
    wxWindow* w = m_Host->TTHH_GetWindow();
    wxWindow* mouse_win = ::wxFindWindowAtPointer(unused_pt);

    if (w != mouse_win)
        return;

    // make sure timer stops since it prevents new tips from coming up (really
    // should not be a problem)
    Stop();

    // If mouse moved too far to allow a tip to be displayed, check again.
    // This is needed because standard check only happens on mouse motion, 
    // and mouse may have stopped while on the glyph that needs a tooltip.
    if (delta.x*delta.x + delta.y*delta.y > 
        m_MouseMoveThreshold*m_MouseMoveThreshold) {
        wxPoint mouse_pos = p - m_Host->TTHH_GetWindow()->GetScreenPosition();
        CheckForNewTip(mouse_pos);
        return;
    }   

    // Only display tip if the host window is still displayed 
    // (window could change in short period since timer started)
    if (m_Host->TTHH_GetWindow()->IsShownOnScreen() ) {
        // Get the info for the tooltip.  In theory, this could be a different
        // tip than was identified in CheckForNewTip(), since the cursor
        // could have moved by up to 'm_MouseMoveThreshold'. It could also return
        // an empty tip, in which case we display nothing.
        wxSize pt(1,1);
        wxRect area(p - m_Host->TTHH_GetWindow()->GetScreenPosition(),  pt);       
        CTooltipInfo next_tip = m_Host->TTHH_GetTooltip(area);
        if (next_tip.GetTipID() != "") {
            int cursor_y = wxSystemSettings::GetMetric(wxSYS_CURSOR_Y);
            int cursor_x = wxSystemSettings::GetMetric(wxSYS_CURSOR_X);        

            if (p.y + cursor_y/3 > screen_rect.GetBottom())
                cursor_y = 0;
            if (p.x + cursor_x/3 > screen_rect.GetRight())
                cursor_x = 0;
    
            CTooltipFrame* tt = new CTooltipFrame( m_Host->TTHH_GetWindow()
                                                  ,wxT("wxWidgets Child Frame")
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__) || defined(__WXOSX_COCOA__)
		    	                                  ,wxPoint(p.x+4,p.y+4)
#else                                                      
		    	                                  ,wxPoint(p.x+(int)(((float)cursor_x)*m_PopupDistance), 
                                                           p.y+(int)(((float)cursor_y)*m_PopupDistance))
#endif
                                                  ,wxDefaultSize
                                                  ,next_tip
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
                                                  ,m_Host->GetWindowGroup()
#endif                                                  
                                                  );
            tt->Show(true);
            tt->SetLinkEventHandler(this);
            AddUnpinned(tt);

#ifdef __WXGTK__
            // GTK keeps sending mouse-move events to the (underlying) window 
            // until lmouseup, even though the mouse is in the newly created
            // tooltip window. This *looks* like the mouse has moved away from
            // the tip which closes the tip window.  Give the new window focus 
            // right away.  GTK only.
            
            tt->SetFocus();
#endif 

            tt->SetInitialMousePos(wxGetMousePosition());
        }
    }
}

void CStickyTooltipHandler::CheckForNewTip(wxPoint pos)
{
    try {
        // If there is already an unpinned tip up, don't start the process of displaying
        // a new tip.  Let the user move the mouse (outside the borders of the unpinned
        // tip) to get the unpinned tip to go away first.
        if (NULL != m_UnpinnedTip) {
            // Mouse is moving in window with an unpinned tip.  Set a timer
            // to remove the tip (unless the mouse gets inside the tip which
            // will prevent the removal from happening).  Set a tiny distance
            // threshold before starting the timer to keep the tips from being
            // too "sensitive".  
            if (!m_TimerClearUnpinned.IsRunning()) {
                wxPoint m = wxGetMousePosition();
                wxPoint delta = m_UnpinnedTip->GetInitialMousePos() - m;
                if (delta.x*delta.x + delta.y*delta.y >
                    m_UnpinnedMouseMoveThreshold*m_UnpinnedMouseMoveThreshold) {
                    m_TimerClearUnpinned.Start(m_ClearUnpinnedDelay, true);
                }
            }

            return;
        }

        // See if a tooltip applies to this position.  Note that (in order to avoid
        // network delays) the returned id may not be consistent for tooltips
        // that are not already displayed.  
        string need_id = m_Host->TTHH_NeedTooltip(pos);

        bool tip_displayed = false;
        // With sticky tips, some may already be displayed. Don't display
        // multiple windows for the same tip
        for (unsigned int i = 0; i < m_ToolTips.size(); ++i) {
            if (m_ToolTips[i]->GetTipInfo().GetTipID() == need_id) {
                tip_displayed = true;
                m_ToolTips[i]->SetElementActive(true);
            }
            else {
                m_ToolTips[i]->SetElementActive(false);
            }
        }

        // If no tip currently displayed and one is needed, and we haven't set the
        // timer to display the next tip alredy, start a new tip
        if (!tip_displayed  &&
            need_id != "" &&
            !IsRunning()) {

            // Show tip after short delay, but do not show if mouse moves in 
            // meantime.  Keeps tips from popping up too quickly.
            m_TipInitPos = wxGetMousePosition();
            Start(m_TipPopupDelay, wxTIMER_ONE_SHOT);
        }
    } NCBI_CATCH("CheckForNewTip");
}

void CStickyTooltipHandler::OnMotion(wxMouseEvent& event)
{
    CheckForNewTip(event.GetPosition());
    event.Skip();
}


// Do this for middle button too.  Suppresses tooltips when right
// and middle button are being used.
void CStickyTooltipHandler::OnRightMouseDown(wxMouseEvent& event)
{
    ClearUnpinned();

    // If a timer was initiated to display a new tip, 
    // this kills the timer (and tip)
    Stop();
    event.Skip();
}

void CStickyTooltipHandler::OnMouseWheel(wxMouseEvent& event)
{
    CheckForNewTip(event.GetPosition());
    event.Skip();
}


// If mouse leaves the window, clear away any unpinned tips
void CStickyTooltipHandler::OnLeaveWindow(wxMouseEvent& event)
{
    // Don't unpin tip if mouse is still within the window area, since
    // it is most likely just on one of this windows own tooltips.
    wxRect screen_rect = m_Host->TTHH_GetWindow()->GetScreenRect();

    // Don't pop up a tool tip if the cursor has moved outside the window
    wxPoint p = wxGetMousePosition();
    if (screen_rect.Contains(p)) {
        event.Skip();
        return;
    }

    ClearUnpinned(); 

    // If a timer was initiated to display a new tip, 
    // this kills the timer (and tip)
    Stop();

    event.Skip();
}

void CStickyTooltipHandler::OnPinTip(wxCommandEvent& event)
{
    // Need the tool tip window for the event - we stored its pointer in
    // the button's (event objects) client data when the button was created
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(event.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());

        if (f != NULL) {
            if (f->IsPinned()) 
                UnpinPinned(f);
            else
                PinUnpinned(f);

            // Call event function directly rather than
            // have tip respond to event since we want this event
            // processing (in in STHandler) to happen first.
            f->OnPinButton(event);
        }
    }
}

void CStickyTooltipHandler::OnTipRelease(wxCommandEvent& event)
{
    // Tip would still be the active tip on release, so we shouldn't
    // need to clip it to the window (wait until mouse leaves window)
    /*
    if (event.GetEventObject() != NULL) {
        CTooltipFrame* tt = dynamic_cast<CTooltipFrame*>(event.GetEventObject());
        if (tt != NULL) {
            tt->FitToWindow();
            m_Host->RaiseOverlappingWindows(tt->GetRect());
        }
    }
    */
}

void CStickyTooltipHandler::OnTipMove(wxCommandEvent& event)
{
    UpdateTips();
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, eCmdTipActive);
    evt.SetEventObject(m_ActiveTip);
    m_Host->TTHH_GetWindow()->GetEventHandler()->ProcessEvent(evt);
}

void CStickyTooltipHandler::OnWindowMoveCmd(wxCommandEvent& event)
{
// In Cocoa the frames are attached to the parents such that
// moving the parent moves the child. (NSWindow: addChildWindow:ordered:)
// Programtically moving the widget in this case should have no effect
// so it mostly works if we don't call ParentMove (under __WXOSX_COCOA__)
// BUT if you shake the widow hard enough (with the mouse) on cocoa,
// the tip can 'fall off' and this fixes that, so we leave it in.
//#ifndef  __WXOSX_COCOA__
    ParentMove(m_Host->TTHH_GetWindow()->GetScreenPosition());
//#endif
}

void CStickyTooltipHandler::OnSuspendCmd(wxCommandEvent& event)
{
    Suspend();
}

void CStickyTooltipHandler::OnReCreateCmd(wxCommandEvent& event)
{
    ReCreate(0);
}

void CStickyTooltipHandler::OnMainWindowDockCmd(wxCommandEvent& event)
{
    // Currently does same thing as recreate, but I left this in
    // in case we need to handle docking events differently at
    // some point)
    ReCreate(event.GetInt());
}

void CStickyTooltipHandler::OnActivateCmd(wxCommandEvent& event)
{
    // Only Mac (Carbon) - tooltips will fall behind main window on occasion,
    // esp. when a window is undocked from main window and user
    // subsequently clicks on main window title bar (activates).
        
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
    wxWindow* w = m_Host->TTHH_GetWindow();
    bool shown = true;
    while (w != NULL && shown) {
        //_TRACE(w->GetClassInfo()->GetClassName());
        
        if (!w->IsShown()) 
            shown = false;
        w = w->GetParent();
     }
    
    if (shown) {
        //_TRACE("Reshowing on activate.");
        ParentShow(false);    
        ParentShow(true);
    }
#endif        
}

void CStickyTooltipHandler::OnWindowShowCmd(wxCommandEvent& event)
{
    ParentShow(true);
}

void CStickyTooltipHandler::OnWindowHideCmd(wxCommandEvent& event)
{
    ParentShow(false);
}


bool CStickyTooltipHandler::checkChildFocus(wxWindow* fwin, wxWindow* checkwin)
{
    // Return true if any child of fwin currently has focus
    if (checkwin==fwin) return true;
    for (unsigned int i=0; i<checkwin->GetChildren().size(); ++i) {
        bool b = checkChildFocus(fwin, checkwin->GetChildren()[i]);
        if (b) return true;
    }

    return false;
}

void CStickyTooltipHandler::UpdateTips()
{
    if (m_Host != NULL) {
        std::vector<IStickyTooltipHandlerHost::TipLocation> tips;

        std::vector<CTooltipFrame*>  tip_windows = m_ToolTips;
        if (m_UnpinnedTip != NULL)
            tip_windows.push_back(m_UnpinnedTip);

        for (size_t i=0; i<tip_windows.size(); ++i) {
            wxPoint pos = tip_windows[i]->GetPos();
            wxSize sz = tip_windows[i]->GetSize();
            wxSize win_sz = m_Host->TTHH_GetWindow()->GetSize();

            IStickyTooltipHandlerHost::TipLocation tl;
            
            tl.TipRect.Init(pos.x, 
                            win_sz.GetHeight() - (pos.y + sz.GetHeight()),
                            pos.x + sz.GetWidth(), 
                            win_sz.GetHeight()-pos.y);
            tl.TipID = tip_windows[i]->GetTipInfo().GetTipID();
            tips.push_back(tl);
        }

        m_Host->x_SetCurrentTips(tips);
    }
}

END_NCBI_SCOPE
