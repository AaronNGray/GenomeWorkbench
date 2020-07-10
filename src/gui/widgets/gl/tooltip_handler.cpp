/*  $Id: tooltip_handler.cpp 22451 2010-10-27 22:03:18Z falkrb $
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

#include <gui/widgets/gl/tooltip_handler.hpp>
#include <gui/widgets/gl/tooltip_wnd.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/window.h>
#include <wx/timer.h>
#include <wx/settings.h>
#include <math.h>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CTooltip

CTooltip::~CTooltip()
{
    if (m_TooltipWnd) {
        m_TooltipWnd->Show(false);
        m_TooltipWnd->GetParent()->Refresh();
        m_TooltipWnd->Destroy();
    }
}

void CTooltip::SetText(const string & text)
{
    wxString tmp = ToWxString(text).Trim();
    m_bNewText = (tmp != m_Text);
    m_Text = tmp;
}

void CTooltip::Off()
{
    // Stop timer in case another tip was started with the timer but
    // not yet visible.
    Stop();
    m_On = false;

    if (m_TooltipWnd) {
        m_TooltipWnd->Show(false);
        m_TooltipWnd->GetParent()->Refresh();
        m_TooltipWnd->Destroy();
        m_TooltipWnd = NULL;

// wxFRAME_TOOL_WINDOW seems to be broken on linux and tooltip grabs focus (but mouse still works on underlying window)
// this sets focus back to main window when tool tip exits (tried Raise/Lower but still had flash, and not 100% reliable)   
        /*
#ifdef NCBI_OS_UNIX
        wxWindow* w = m_Host->TC_GetWindow();
        while (w->GetParent() != NULL)
            w = w->GetParent();
        w->Raise();
#endif
        */
    }
}

// switch tooltip On
void CTooltip::On(wxPoint point)
{
    if (m_TooltipWnd) {
        if (m_Text.IsEmpty()) {
            m_TooltipWnd->Show(false);
            m_TooltipWnd->GetParent()->Refresh();
            m_TooltipWnd->Destroy();
            m_TooltipWnd = NULL;
        }
        else {
            m_BasePos = point;
            Move();
        }
        return;
    }

    m_BasePos = point;
    if (m_Text.IsEmpty())
        return;

    // Make sure mouse hasn't move significantly since timer
    // was started:
    wxPoint current_pos = wxGetMousePosition() - m_Host->TC_GetWindow()->GetScreenPosition();
    wxPoint delta = m_BasePos - current_pos;
      
    if ( delta.x*delta.x + delta.y*delta.y > 16)
        return;

    m_ScreenPos = wxGetMousePosition();
    m_TooltipWnd = new CTooltipWnd(this);
    m_TooltipWnd->SetTooltipText(m_Text);
    m_TooltipWnd->Create(m_Host->TC_GetWindow(), wxID_ANY);
    Move();
    m_TooltipWnd->Show();
 
    m_TooltipWnd->GetParent()->Refresh();
}

void CTooltip::Move()
{
    if (m_TooltipWnd == NULL)
        return;

    wxWindow* hostWindow = m_Host->TC_GetWindow();
    int cursorY = wxSystemSettings::GetMetric(wxSYS_CURSOR_Y);
    wxRect clientRect = hostWindow->GetClientRect();
    wxSize size = m_TooltipWnd->GetSize();
    wxPoint pos = m_BasePos;

    if (pos.x + size.GetWidth() > clientRect.GetRight())
        pos.x = clientRect.GetRight() - size.GetWidth();
    if (pos.x < clientRect.GetLeft())
        pos.x = clientRect.GetLeft();

    if (pos.y + cursorY/2 + size.GetHeight() > clientRect.GetBottom()) {
        int y = pos.y - size.GetHeight();
        if (y >= clientRect.GetTop())
            pos.y = y;
        else
            pos.y += cursorY/2;
    }
    else
        pos.y += cursorY/2;

    // Get screen coordinates of window onto which tool tip
    // is displayed
    m_PosDelta = pos-m_BasePos;

    WinMove();

/*
    wxPoint pos = m_Host->TC_GetWindow()->ClientToScreen(m_Pos);
    int cursorY = wxSystemSettings::GetMetric(wxSYS_CURSOR_Y);
    int dindx = wxDisplay::GetFromPoint(pos);
    wxDisplay* display = new wxDisplay(dindx != wxNOT_FOUND ? dindx : 0);
    wxRect displayRect = display->GetClientArea();
    wxSize size = m_TooltipWnd->GetSize();

    if (pos.x + size.GetWidth() > displayRect.GetRight())
        pos.x = displayRect.GetRight() - size.GetWidth();
    if (pos.x < displayRect.GetLeft())
        pos.x = displayRect.GetLeft();

    if (pos.y + cursorY/2 + size.GetHeight() > displayRect.GetBottom()) {
        int y = pos.y - size.GetHeight();
        if (y >= displayRect.GetTop())
            pos.y = y;
        else
            pos.y += cursorY/2;
    }
    else
        pos.y += cursorY/2;

    m_TooltipWnd->Move(pos);
 */
}

void CTooltip::WinMove()
{
    if (m_TooltipWnd != NULL) {
        wxPoint screen_pos = m_Host->TC_GetWindow()->GetScreenPosition();
        m_TooltipWnd->Move(screen_pos + (m_BasePos + m_PosDelta));
    }
}

 bool CTooltip::IsShown() const
 {
     return m_TooltipWnd ? m_TooltipWnd->IsShown() : false;
 }

// mouse move default tooltip handle
void CTooltip::HideAndShow(wxPoint point)
{
    m_BasePos = point;
    Off();
    m_On = true;   
    Start(400, wxTIMER_ONE_SHOT);
}

void CTooltip::Hide()
{
    m_On = false;

    // If another timer is starting, stop it too:
    Stop(); 
    Start(100, wxTIMER_ONE_SHOT );               
}

// wxTimer
void CTooltip::Notify()
{
    if (m_On)
        On(m_BasePos);
    else
        Off();
}

#define CTOOLTIP_HANDLER_TIMER_ID 5212010

BEGIN_EVENT_TABLE(CTooltipHandler, wxEvtHandler)
    EVT_LEFT_DOWN(CTooltipHandler::OnLeftDown)
    EVT_RIGHT_DOWN(CTooltipHandler::OnRightDown)
    EVT_MOTION(CTooltipHandler::OnMotion)

    EVT_KEY_DOWN(CTooltipHandler::OnKeyDown)
    EVT_KEY_UP(CTooltipHandler::OnKeyDown)
    EVT_MOUSE_CAPTURE_LOST(CTooltipHandler::OnMouseCaptureLost)
    EVT_KILL_FOCUS(CTooltipHandler::OnKillFocus)

    EVT_SIZE(CTooltipHandler::OnSize)

    EVT_TIMER(CTOOLTIP_HANDLER_TIMER_ID, CTooltipHandler::OnTimer)
END_EVENT_TABLE()


CTooltipHandler::CTooltipHandler()
:   m_Mode(eHideOnMove),
    m_Host(NULL),
    m_Pane(NULL),
    m_PosUpdateTimer(this, CTOOLTIP_HANDLER_TIMER_ID)
{   
}


CTooltipHandler::~CTooltipHandler()
{
}

wxEvtHandler* CTooltipHandler::GetEvtHandler()
{
    return this;
}

void CTooltipHandler::SetPane(CGlPane* pane)
{
    m_Pane = pane;
}


void CTooltipHandler::SetMode(EMode mode)
{
    m_Mode = mode;
}


void CTooltipHandler::SetHost(ITooltipHandlerHost* host)
{
    m_Host = host;
    m_Tip.SetHost(host);
}


void CTooltipHandler::OnLeftDown(wxMouseEvent& event)
{
    m_Tip.Off();
    event.Skip();
}


void CTooltipHandler::OnRightDown(wxMouseEvent& event)
{   
    m_Tip.Off();  
    event.Skip();
}

void CTooltipHandler::OnMouseCaptureLost(wxMouseCaptureLostEvent& event)
{
    m_Tip.Off();
    event.Skip();
}

void CTooltipHandler::OnKeyDown(wxKeyEvent& event)
{
    m_Tip.Off();
    event.Skip();
}


void CTooltipHandler::OnKillFocus(wxFocusEvent& event)
{

// wxFRAME_TOOL_WINDOW seems to be broken on linux and tooltip grabs focus (but mouse still works on underlying window)
// This event makes first tool tip disappear (and I'm not sure we need it anyway, but only disable on unix for now) 
#ifndef NCBI_OS_UNIX
    m_Tip.Off();
#endif
    event.Skip();
}




void CTooltipHandler::OnMotion(wxMouseEvent& event)
{   
    wxPoint pos   = event.GetPosition();        
    bool    bNeed = m_Host->TC_NeedTooltip(pos);

    if (bNeed) {      
        if (!m_PosUpdateTimer.IsRunning())
            m_PosUpdateTimer.Start(100, wxTIMER_CONTINUOUS);

        wxSize thePoint(1,1);
        wxRect tipArea(pos,  thePoint);
        m_Tip.SetText(m_Host->TC_GetTooltip(tipArea));      

        switch (m_Mode) {
        case eHideOnMove:   
            {{              
                m_Tip.HideAndShow(pos);
                break;
            }}
        case eTrackOnMove:  
            {{  
                if (m_Tip.NewText()) m_Tip.HideAndShow(pos);
                else m_Tip.On(pos);                
                break;
            }}
        case eStayOnMove:
            {{
                if (m_Tip.NewText()) m_Tip.On(pos);
                break;
            }}
        }
    }
    else {
       // Turns tip off when tips owning-window has focus (so just use ontimer for now)
       // m_Tip.SetText("");
       // m_Tip.Off();
    }   

    event.Skip();
}

void CTooltipHandler::OnSize(wxSizeEvent& event)
{
    m_Tip.Move();

    event.Skip();
}

void CTooltipHandler::OnTimer(wxTimerEvent& event)
{
    // This timer was first put in to handle EVT_MOVE events which were not being propogated
    // to all widgets.  But now it is only used because mousemove events also are not always
    // propogated to the tool tip handler after a tip has been popped up (specifically if you pop
    // up a tip on a view that doesn't currently have mouse focus, you can then pop up another scrreen via
    // the keyboard and leave the tool-tip 'hanging'.  This timer closes the tip if the mouse moves
    // no matter where the focus is)
    if (event.GetId() == CTOOLTIP_HANDLER_TIMER_ID) {
        //m_Tip.WinMove();
        wxPoint p = wxGetMousePosition();
        
        if  (m_Tip.IsOn() && m_Tip.IsShown() && (m_Tip.GetScreenPos() != p))
            m_Tip.Off();
    }
}

END_NCBI_SCOPE
