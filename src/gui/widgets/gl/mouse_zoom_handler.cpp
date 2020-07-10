/*  $Id: mouse_zoom_handler.cpp 43037 2019-05-08 18:48:24Z katargir $
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

#include <gui/widgets/gl/mouse_zoom_handler.hpp>

#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>

#include <math.h>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CMouseZoomHandler

BEGIN_EVENT_TABLE(CMouseZoomHandler, wxEvtHandler)
    EVT_LEFT_DOWN(CMouseZoomHandler::OnLeftDown)
    EVT_LEFT_DCLICK(CMouseZoomHandler::OnLeftDown)
    EVT_LEFT_UP(CMouseZoomHandler::OnLeftUp)
    EVT_MOTION(CMouseZoomHandler::OnMotion)
    EVT_MIDDLE_DOWN(CMouseZoomHandler::OnMiddleDown)
    EVT_MIDDLE_UP(CMouseZoomHandler::OnMiddleUp)
    EVT_MOUSEWHEEL(CMouseZoomHandler::OnMouseWheel)
    EVT_RIGHT_DOWN(CMouseZoomHandler::OnOtherMouseEvents)
    EVT_KEY_DOWN(CMouseZoomHandler::OnKeyDown)
    EVT_KEY_UP(CMouseZoomHandler::OnKeyUp)
    EVT_MOUSE_CAPTURE_LOST(CMouseZoomHandler::OnMouseCaptureLost)
    EVT_KILL_FOCUS(CMouseZoomHandler::OnKillFocus)
END_EVENT_TABLE()


CMouseZoomHandler::CMouseZoomHandler()
:   m_Mode(e2D),
    m_Host(NULL),
    m_Pane(NULL),
    m_State(eIdle),
    m_CursorId(wxCURSOR_ARROW),
    m_PanMode(eLmouse),
    m_PanOnMouseDown(false),
    m_Reversed(false),
    m_PixPerNorm(50),
    m_MaxNorm(0),
    m_MinNorm(0),
    m_StartNorm(0),
    m_CurrNorm(0),
    m_MarkerPos(0, 0),
    m_CurrMouse(0, 0),
    m_MouseStart(0, 0),
    m_CurrPos(0, 0),
    m_HasCapture(false),
    m_WheelTotalShift(0),
    m_ScaleColor(0.0f, 0.5f, 0.0f, 0.3f),
    m_TickColor(1.0f, 1.0f, 1.0f),
    m_RectColor(0.0f, 0.5f, 0.0f, 0.1f)
{
    /// Genome Workbench specific stuff ///////////////////////////////////////
}


CMouseZoomHandler::~CMouseZoomHandler()
{
}


void CMouseZoomHandler::SetPane(CGlPane* pane)
{
    m_Pane = pane;
}


wxEvtHandler* CMouseZoomHandler::GetEvtHandler()
{
    return this;
}


void CMouseZoomHandler::SetMode(EMode mode)
{
    m_Mode = mode;
}


void CMouseZoomHandler::SetHost(IMouseZoomHandlerHost* pHost)
{
    m_Host = pHost;
}


IGenericHandlerHost* CMouseZoomHandler::GetGenericHost()
{
    return dynamic_cast<IGenericHandlerHost*>(m_Host);
}


static int kMarkerSize = 20;
static int kMarkerOffset = 10;
static int kGaugeW = 60;
static int kEndTickW = 20;
static int kMajorTickW = 14;
static int kMinorTickW = 8;
static int kCenterOffset = 4;


void CMouseZoomHandler::Render(CGlPane& pane)
{
    CGlAttrGuard  AttrGuard(GL_LINE_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_HINT_BIT);

    switch( m_State ){
    case eReadyScale:
    case eScale: 
        x_RenderScale(pane); 
        break;
    case eZoomRect: 
        x_RenderZoomRect(pane); 
        break;
    //case ePan: x_RenderPan(pane); break; // temporary disabled
    default: 
        break;
    }
}


void CMouseZoomHandler::x_RenderScale(CGlPane& pane)
{
    pane.OpenPixels();

    IRender& gl = GetGl();

    int base_x = m_MarkerPos.x;

    int down_y = x_NormToPixels(m_CurrNorm);
    int top_y = x_NormToPixels(m_MinNorm);
    int bottom_y = x_NormToPixels(m_MaxNorm);

    gl.ColorC(m_ScaleColor);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //drawing "gauge box"
    gl.Rectd(base_x - kGaugeW / 2, bottom_y - kCenterOffset,
             base_x + kGaugeW / 2, top_y + kCenterOffset);

    // drawing Markers
    int half = kMarkerSize / 2;
    x_DrawMarker(true, base_x - kGaugeW / 2 - kMarkerOffset, down_y, half);
    x_DrawMarker(true, base_x + kGaugeW / 2 + kMarkerOffset, down_y, -half);

    gl.ColorC(m_TickColor);
    gl.LineWidth(1.0);

    //drawing "gauge box"
    //gl.Rectd(base_x - kGaugeW / 2, bottom_y - kCenterOffset,
             //base_x + kGaugeW / 2, top_y + kCenterOffset);
    gl.Begin(GL_LINE_STRIP);
        gl.Vertex2d(base_x - kGaugeW / 2, bottom_y - kCenterOffset);
        gl.Vertex2d(base_x - kGaugeW / 2, top_y + kCenterOffset);
        gl.Vertex2d(base_x + kGaugeW / 2, top_y + kCenterOffset);
        gl.Vertex2d(base_x + kGaugeW / 2, bottom_y - kCenterOffset);
        gl.Vertex2d(base_x - kGaugeW / 2, bottom_y - kCenterOffset);
    gl.End();

    // drawing Markers
    x_DrawMarker(false, base_x - kGaugeW / 2 - kMarkerOffset, down_y, half);
    x_DrawMarker(false, base_x + kGaugeW / 2 + kMarkerOffset, down_y, -half);

    // draw Scale - lines and ticks
    gl.ColorC(m_TickColor);

    int x1 = base_x - kCenterOffset;
    int x2 = base_x + kCenterOffset;

    gl.Begin(GL_LINES);
        // draw vertical lines
        gl.Vertex2d(x1, top_y);
        gl.Vertex2d(x1, bottom_y);

        gl.Vertex2d(x2, top_y);
        gl.Vertex2d(x2, bottom_y);

        // draw min-max ticks
        x_DrawTicks(base_x, top_y, kEndTickW);
        x_DrawTicks(base_x, bottom_y, kEndTickW);

        // draw minor ticks every 0.2 of norm value
        int start_i = (int) ceil(m_MinNorm * 5);
        int end_i = (int) floor(m_MaxNorm * 5);

        // draw regular ticks
        for( int i = start_i; i <= end_i; i++ )   {
            TModelUnit norm = 0.2 * i;
            int y = x_NormToPixels(norm);
            int width = ((i - start_i) % 5) ? kMinorTickW : kMajorTickW;

            x_DrawTicks(base_x, y, width);
        }
    gl.End();

    pane.Close();
}


// draw green transparent rectangle for the "Zoom Rectangle" operation
void CMouseZoomHandler::x_RenderZoomRect(CGlPane& pane)
{
    pane.OpenPixels();
    const TVPRect& rc_vp = pane.GetViewport();

    int x1, x2, y1, y2;

    // horizontal bounds
    if(m_Mode == eVert) {
        x1 = rc_vp.Left();
        x2 = rc_vp.Right();
    } else {
        x1 = m_MouseStart.x;
        x2 = m_CurrPos.x;
    }
    if( x2 < x1 ){
        swap(x1, x2);
    }

    // vertical bounds
    if(m_Mode == eHorz) {
        y1 = rc_vp.Bottom();
        y2 = rc_vp.Top();
    } else {
        y1 = m_Host->MZHH_GetVPPosByY(m_MouseStart.y);
        y2 = m_Host->MZHH_GetVPPosByY(m_CurrPos.y);
    }
    if(y2 < y1)
        swap(y1, y2);

    IRender& gl = GetGl();

    // fill rectangle
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.ColorC(m_RectColor);

    gl.Rectd(x1, y1, x2, y2);

    gl.LineWidth(1.0f);
    gl.ColorC(m_ScaleColor);

    // draw rectangle countour
    gl.LineStipple(1, 0x0F0F);
    gl.Enable(GL_LINE_STIPPLE);

    gl.Begin(GL_LINES);
        gl.Vertex2d(x1, y2);
        gl.Vertex2d(x2, y2);

        gl.Vertex2d(x2, y2);
        gl.Vertex2d(x2, y1);

        gl.Vertex2d(x1, y2);
        gl.Vertex2d(x1, y1);

        gl.Vertex2d(x1, y1);
        gl.Vertex2d(x2, y1);
    gl.End();

    gl.Disable(GL_LINE_STIPPLE);

    pane.Close();
}


static const int kArrowBodyW = 10;
static const int kArrowHeadW = 20;
static const int kArrowHeadL = 20;


void CMouseZoomHandler::x_RenderPan(CGlPane& pane)
{
    pane.OpenPixels();

    IRender& gl = GetGl();

    gl.LineWidth(1.0f);
    gl.ColorC(m_ScaleColor);

    gl.Enable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

    int x1 = m_MouseStart.x;
    int y1 = m_Host->MZHH_GetVPPosByY(m_MouseStart.y);
    int x2 = m_CurrPos.x;
    int y2 = m_Host->MZHH_GetVPPosByY(m_CurrPos.y);

    double d_x = x2 - x1;
    double d_y = y2 - y1;
    double angle = 90.0 * atan2(d_y, d_x) / atan2(1.0, 0.0);
    double hyp = sqrt(d_x * d_x + d_y * d_y);

    // move and rotate origin and then draw arrow horizontally
    gl.Translated(x1, y1, 0);
    gl.Rotated(angle, 0.0, 0.0, 1.0);

    gl.Begin(GL_QUADS);
        if(hyp > kArrowHeadL)  { // draw full arrow
            gl.Vertex2d(0, kArrowBodyW);
            gl.Vertex2d(hyp - kArrowHeadL, kArrowBodyW);
            gl.Vertex2d(hyp - kArrowHeadL, -kArrowBodyW);
            gl.Vertex2d(0, -kArrowBodyW);

            gl.Vertex2d(hyp - kArrowHeadL, 0);
            gl.Vertex2d(hyp - kArrowHeadL, kArrowHeadW);
            gl.Vertex2d(hyp, 0);
            gl.Vertex2d(hyp - kArrowHeadL, -kArrowHeadW);
        } else { // draw simplified arrow
            gl.Vertex2d(0, 0);
            gl.Vertex2d(0, kArrowHeadW);
            gl.Vertex2d(hyp, 0);
            gl.Vertex2d(0, -kArrowHeadW);
        }
    gl.End();

    gl.Rotated(-angle, 0.0, 0.0, 1.0);
    gl.Translated(-x1, -y1, 0);

    //glDisable(GL_POLYGON_SMOOTH);

    pane.Close();
}


void CMouseZoomHandler::x_DrawMarker(bool fill, int x_c, int y_c, int half)
{
    IRender& gl = GetGl();

    if (fill) {
        gl.Begin(GL_TRIANGLE_STRIP);
            gl.Vertex2d(x_c - half, y_c + half);
            gl.Vertex2d(x_c - half, y_c - half);
            gl.Vertex2d(x_c, y_c + half);
            gl.Vertex2d(x_c, y_c - half);
            gl.Vertex2d(x_c + half, y_c);
        gl.End();
    }
    else {
        gl.Begin(GL_LINE_STRIP);
            gl.Vertex2d(x_c + half, y_c);
            gl.Vertex2d(x_c, y_c - half);
            gl.Vertex2d(x_c - half, y_c - half);
            gl.Vertex2d(x_c - half, y_c + half);
            gl.Vertex2d(x_c, y_c + half);
            gl.Vertex2d(x_c + half, y_c);
        gl.End();
    }
}


void CMouseZoomHandler::x_DrawTicks(int center_x, int y, int tick_w)
{
    int x1 = center_x - kCenterOffset;
    int x2 = center_x + kCenterOffset;

    IRender& gl = GetGl();

    gl.Vertex2d(x1 - tick_w, y);
    gl.Vertex2d(x1, y);

    gl.Vertex2d(x2, y);
    gl.Vertex2d(x2 + tick_w, y);
}


int  CMouseZoomHandler::x_NormToPixels(TModelUnit norm) const
{
    int y = m_Host->MZHH_GetVPPosByY(m_CurrPos.y);
    return y - (int) ((norm - m_CurrNorm) * m_PixPerNorm);
}


CMouseZoomHandler::EKeyState CMouseZoomHandler::x_GetKeyState()
{
    if(wxGetKeyState(wxKeyCode('Z'))) {
        return eZoomState;
    } else if(wxGetKeyState(wxKeyCode('R'))) {
        return eZoomRectState;
    } else if(wxGetKeyState(wxKeyCode('P'))) {
        return ePanState;
    }

    if (m_PanMode == CMouseZoomHandler::eLmouse) {
        // also go into pan mode on left mouse if NO key is pressed -
        // so go through all the 'major' keys here:
        // FIXME: Problems
        //    - First, the keys listed below are not complete. Many other
        //      keys may be used in some of the views, such as up/down
        //      page up/down, -/+, Tab, left/right arrow
        //    - Not efficient to go though all the keys one by one.
        int  k;
        for ( k='A'; k<='Z'; ++k) {
            if (wxGetKeyState((wxKeyCode)k))
            return eDefaultState;
        }
        for ( k='0'; k<='9'; ++k) { 
            if (wxGetKeyState((wxKeyCode)k))
                return eDefaultState;
        }

        if (wxGetKeyState(WXK_SHIFT) ||
            wxGetKeyState(WXK_ALT) ||            
            wxGetKeyState(WXK_CONTROL) ||
            wxGetKeyState(WXK_COMMAND))
                return eDefaultState;

        // No other keys pressed (char/number/special) so return pan state:
        return ePanState;
    }
    
    return eDefaultState;
}


CMouseZoomHandler::EKeyState CMouseZoomHandler::x_GetKeyState(wxKeyEvent& event)
{
    // There is a key event, never call x_GetKeyState() since
    // it may still return ePanState when certain keys are pressed
    // which is incorrect.
    EKeyState state = eDefaultState;
    // EKeyState state = x_GetKeyState();
    switch(event.GetKeyCode()) {
        case 'Z':
            state = eZoomState;
            break;
        case 'R':
            state = eZoomRectState;
            break;
        case 'P':
            state = ePanState;
            break;
        default:
            break;
    }
    return state;
}


void CMouseZoomHandler::OnLeftDown(wxMouseEvent& event)
{
    //LOG_POST("CMouseZoomHandler::OnLeftDown() m_State " << m_State);
    EKeyState state = CMouseZoomHandler::x_GetKeyState();

    EState ready_st = eIdle, active_st = eIdle;
    switch(state)   {
    case eZoomState: {
        ready_st = eReadyScale;
        active_st = eScale;
    }; break;
    case eZoomRectState: {
        ready_st = eReadyZoomRect;
        active_st = eZoomRect;
        x_OnSelectCursor();
    }; break;
    case ePanState: {
        ready_st = eReadyPan;
        active_st = ePan;
    }; break;
    default: break;
    }

    if(ready_st != eIdle)    {
        m_CurrMouse = event.GetPosition();
        x_SwitchToReadyState(ready_st, m_CurrMouse);

        if (ready_st != eReadyPan || m_PanOnMouseDown) {
            x_SwitchToActiveState(active_st, m_CurrMouse);
            x_OnSelectCursor();
        }
        else {
            event.Skip();
        }
    } else {
        event.Skip();
    }
    //LOG_POST("CMouseZoomHandler::OnLeftDown()  END m_State " << m_State);
}


void CMouseZoomHandler::OnLeftUp(wxMouseEvent& event)
{
    //LOG_POST("CMouseZoomHandler::OnLeftUp()  m_State " << m_State);

    wxPoint ms_pos = event.GetPosition();

    switch(m_State) {
    case eScale:    {
        int d_y = ms_pos.y - m_MouseStart.y;
        x_OnChangeScale(d_y);
        x_OnEndScale(eReadyScale);
        x_OnSelectCursor();
        break;
    }
    case eZoomRect: {
        x_OnChangeZoomRectPan(ms_pos);
        x_OnEndZoomRect(eReadyZoomRect);
        x_OnSelectCursor();
        break;
    }
    // Only switches to pan if mouse moves during pan:
    case eReadyPan:  {            
        m_State = eIdle;
        event.Skip();
      
        break;
    }
    case ePan:  {
        x_OnChangeZoomRectPan(ms_pos);
     
        if (!wxGetKeyState(wxKeyCode('P'))) {
            x_OnEndPan(eIdle);
        }
        else {
            x_OnEndPan(eReadyPan);
        }
        x_OnSelectCursor();        

        break;
    }
    default:
        event.Skip();
    }
    //LOG_POST("CMouseZoomHandler::OnLeftUp()  END m_State " << m_State);
}


void CMouseZoomHandler::OnMotion(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();

    if(event.Dragging())    {
        switch(m_State) {
        case eScale: {
            int d_y = pos.y - m_MouseStart.y;
            x_OnChangeScale(d_y);
            break;
        }
        case eZoomRect: {
            x_OnChangeZoomRectPan(pos);
            break;
        }
        case eReadyPan:  {
            // Don't capture mouse on pan until mouse starts to move.
            // otherwise the event may be used for selection.
            if(!m_HasCapture) {                
                x_SwitchToActiveState(ePan, m_CurrMouse);
                x_OnSelectCursor();
            }
            break;
        }
        case ePan:  {
            x_OnChangePan(pos);
            x_OnSelectCursor();
            break;
        }
        default:
            event.Skip();
        }
    } else {
        if (m_State != eIdle)    {
            x_OnSelectCursor();
        } else {
            m_CurrMouse = pos;
            event.Skip();
        }
    }
    
    m_CurrMouse = pos;
}


void CMouseZoomHandler::OnMouseWheel(wxMouseEvent& event)
{
    EKeyState state = x_GetKeyState();
    wxPoint pos = event.GetPosition();

    if(state == eZoomState) {
        switch(m_State) {
        case eIdle:
            x_SwitchToReadyState(eReadyScale, pos); // continue
        case eReadyScale: {
            m_WheelTotalShift = 0;
            x_SwitchToActiveState(eScale, pos);
        }  // continue
        case eScale: {
            //int delta =
            m_WheelTotalShift += event.GetWheelRotation()/10;
            x_OnChangeScale(m_WheelTotalShift);

            // updating shift after clipping with Min and Max norm values
            m_WheelTotalShift = (int) ((m_StartNorm - m_CurrNorm) * m_PixPerNorm);
            break;
        }
        default:
            //_ASSERT(false);
            break;
        }
        x_OnSelectCursor();
    } else {
        event.Skip();
    }
}


void CMouseZoomHandler::OnOtherMouseEvents(wxMouseEvent& event)
{
    if (m_State == eIdle) {
        event.Skip();
    }
    // Otherwise, ignore the event.
}


void CMouseZoomHandler::OnMiddleDown(wxMouseEvent& event)
{
    if( m_State == eIdle ){
        m_CurrMouse = event.GetPosition();
        x_SwitchToReadyState(eReadyScale, m_CurrMouse);
        x_SwitchToActiveState(eScale, m_CurrMouse);
        x_OnSelectCursor();
    } else {
        event.Skip();
    }
}


void CMouseZoomHandler::OnMiddleUp(wxMouseEvent& event)
{
    if( m_State == eScale ){
        x_OnCancel();
    } else {
        event.Skip();
    }
}


void CMouseZoomHandler::OnKeyDown(wxKeyEvent& event)
{
    //LOG_POST("CMouseZoomHandler::OnKeyDown() m_State " << m_State);

    if( m_State == eIdle ){
        EState ready_st = eIdle;
        EKeyState state = x_GetKeyState(event);

        switch(state) {
        case eZoomState:
            ready_st =  eReadyScale;
            break;
        case eZoomRectState:
            ready_st = eReadyZoomRect;
            break;
        case ePanState:
            ready_st = eReadyPan;
            break;
        default:
            break;
        }

        if( ready_st != eIdle ){
            //m_CurrMouse = event.GetPosition();
            x_SwitchToReadyState(ready_st, m_CurrMouse);
            x_OnSelectCursor();
        }
    }

    if( m_State == eIdle ){
        event.Skip();
    }
    //LOG_POST("CMouseZoomHandler::OnKeyDown() END m_State " << m_State);
}

void CMouseZoomHandler::OnKeyUp(wxKeyEvent& event)
{
    EKeyState state = x_GetKeyState();

    if( state != eZoomState  && state != eZoomRectState ){
        x_OnCancel();
    }

    if( m_State == eIdle ){
        event.Skip();
    }
}


void CMouseZoomHandler::x_OnChangePan(const wxPoint& ms_pos)
{
    m_CurrPos = ms_pos;

    TModelUnit m_x1 = m_Pane->UnProjectX(m_MouseStart.x);
    TModelUnit m_x2 = m_Pane->UnProjectX(m_CurrPos.x);

    int y1 = m_Host->MZHH_GetVPPosByY(m_MouseStart.y);
    int y2 = m_Host->MZHH_GetVPPosByY(m_CurrPos.y);
    TModelUnit m_y1 = m_Pane->UnProjectY(y1);
    TModelUnit m_y2 = m_Pane->UnProjectY(y2);

    m_Host->MZHH_Scroll(m_x1 - m_x2, m_y1 - m_y2);
    GetGenericHost()->GHH_Redraw();

    m_MouseStart = m_CurrPos;
}


void CMouseZoomHandler::x_OnCancel()
{
    switch( m_State ){
    case eIdle:
        break;
    case eReadyScale:
    case eReadyZoomRect:
    case eReadyPan:
    case eZoomRect:
    case ePan: {
        GetGenericHost()->GHH_Redraw();
        if( m_HasCapture ){
            x_ReleaseMouse();
        }
        m_State = eIdle;
        x_OnSelectCursor();
        break;
    }
    case eScale: {
        x_OnEndScale( eIdle );
        x_OnSelectCursor();
        break;
    }
    default:
        break;
    };
}

////////////////////////////////////////////////////////////////////////////////
/// Signal handlers

void CMouseZoomHandler::x_SwitchToReadyState(EState new_state, const wxPoint& pos)
{
    //LOG_POST("CMouseZoomHandler::x_SwitchToReadyState()");

    _ASSERT(m_Host);
    if(m_State != new_state) {
        m_State = new_state;

        m_CurrPos = m_MarkerPos = pos;

        if(m_State == eReadyScale)  {
            double curr_sc = m_Host->MZHH_GetScale(IMouseZoomHandlerHost::eCurrent);
            m_CurrNorm = m_StartNorm = x_ScaleToNorm(curr_sc);
            m_Reversed = curr_sc < 0;

            m_MinNorm = x_ScaleToNorm(m_Host->MZHH_GetScale(IMouseZoomHandlerHost::eMin));
            m_MaxNorm = x_ScaleToNorm(m_Host->MZHH_GetScale(IMouseZoomHandlerHost::eMax));
        }
        GetGenericHost()->GHH_Redraw();

        if(m_HasCapture)    {
            x_ReleaseMouse();
        }
    }
}


void CMouseZoomHandler::x_SwitchToActiveState(EState state, const wxPoint& pos)
{
    _ASSERT(m_Host);

    //LOG_POST("CMouseZoomHandler::x_SwitchToActiveState()  m_State " << m_State << " state " << state);

    m_State = state;
    m_MouseStart = pos;

    int vp_y = m_Host->MZHH_GetVPPosByY(m_MouseStart.y);
    m_ptStart = m_Pane->UnProject(m_MouseStart.x, vp_y);

    x_CaptureMouse();
}

/// d_y is the absolute shift in pixels from  the position where m_StartNorm have
/// been initialized
void CMouseZoomHandler::x_OnChangeScale(int d_y)
{
    if(d_y) {
        TModelUnit norm = m_StartNorm - ((TModelUnit) (d_y)) / m_PixPerNorm;
        norm = max(norm, m_MinNorm);
        norm = min(norm, m_MaxNorm);

        if(norm != m_CurrNorm)  {
            m_CurrNorm = norm;
            TModelUnit scale = x_NormToScale(norm);

            // Get mouse position relative to current transformation:
            int vp_y = m_Host->MZHH_GetVPPosByY(m_MouseStart.y);
            m_ptStart = m_Pane->UnProject(m_MouseStart.x, vp_y);

            m_Host->MZHH_SetScale(scale, m_ptStart);
        }
    }
}


void CMouseZoomHandler::x_OnEndScale(EState new_state)
{
    _ASSERT(new_state != eScale);

    m_State = new_state;
    m_StartNorm = m_CurrNorm;

    m_Host->MZHH_EndOp();
    GetGenericHost()->GHH_Redraw();
    x_ReleaseMouse();
}


void CMouseZoomHandler::x_OnChangeZoomRectPan(const wxPoint& pos)
{
    if(m_CurrPos != pos) {
        m_CurrPos = pos;

        GetGenericHost()->GHH_Redraw();
    }
}


void CMouseZoomHandler::x_OnEndZoomRect(EState new_state)
{
    m_State = new_state;

    TModelRect rc;

    int x1 = m_MouseStart.x;
    int y1 = m_Host->MZHH_GetVPPosByY(m_MouseStart.y);
    int x2 = m_CurrPos.x;
    int y2 = m_Host->MZHH_GetVPPosByY(m_CurrPos.y);

    if(x2 != x1  ||  y2 != y1)  {
        //const TVPRect& rc_vp = m_Pane->GetViewport();
        int left = min(x1, x2);
        int right = max(x1, x2);
        int bottom = min(y1, y2);
        int top = max(y1, y2);

        rc.SetLeft(m_Pane->UnProjectX(left));
        rc.SetRight(m_Pane->UnProjectX(right));
        rc.SetBottom(m_Pane->UnProjectY(bottom));
        rc.SetTop(m_Pane->UnProjectY(top));

        const TModelRect& rc_lim = m_Pane->GetModelLimitsRect();
        rc.IntersectWith(rc_lim);

        m_Host->MZHH_ZoomRect(rc);
        m_Host->MZHH_EndOp();
    }
    GetGenericHost()->GHH_Redraw();
    x_ReleaseMouse();
}


void CMouseZoomHandler::x_OnEndPan(EState new_state)
{
    m_State = new_state;

    int y1 = m_Host->MZHH_GetVPPosByY(m_MouseStart.y);
    int y2 = m_Host->MZHH_GetVPPosByY(m_CurrPos.y);

    TModelUnit m_x1 = m_Pane->UnProjectX(m_MouseStart.x);
    TModelUnit m_y1 = m_Pane->UnProjectY(y1);
    TModelUnit m_x2 = m_Pane->UnProjectX(m_CurrPos.x);
    TModelUnit m_y2 = m_Pane->UnProjectY(y2);

    m_Host->MZHH_Scroll(m_x1 - m_x2, m_y1 - m_y2);
    m_Host->MZHH_EndOp();
    GetGenericHost()->GHH_Redraw();
    x_ReleaseMouse();
}


void CMouseZoomHandler::x_OnSelectCursor(void)
{
    switch(m_State)    {
    case eIdle:
    case eReadyScale:
        m_CursorId = wxCURSOR_ARROW;
        break;
    case eScale:
        m_CursorId = wxCURSOR_HAND;
        break;
    case eReadyPan:
    case ePan:
        m_CursorId = wxCURSOR_SIZING;
        break;
    case eReadyZoomRect:
    case eZoomRect:
        m_CursorId = wxCURSOR_CROSS;
        break;
    default:
        break;
    }

    GetGenericHost()->GHH_SetCursor(wxCursor(m_CursorId));
}


void CMouseZoomHandler::OnMouseCaptureLost(wxMouseCaptureLostEvent& evt)
{
    x_OnCancel();
}


void CMouseZoomHandler::OnKillFocus(wxFocusEvent& event)
{
    x_OnCancel();
}

////////////////////////////////////////////////////////////////////////////////
/// helper functions

TModelUnit CMouseZoomHandler::x_ScaleToNorm(TModelUnit scale) const
{
    return log(fabs(scale));
}


TModelUnit  CMouseZoomHandler::x_NormToScale(TModelUnit norm) const
{
    double sc = exp(norm);
    return m_Reversed ? -sc : sc;
}


void CMouseZoomHandler::x_CaptureMouse()
{
    _ASSERT( ! m_HasCapture);
    GetGenericHost()->GHH_CaptureMouse();
    m_HasCapture = true;
}


void CMouseZoomHandler::x_ReleaseMouse()
{
    _ASSERT(m_HasCapture);    
    GetGenericHost()->GHH_ReleaseMouse();
    m_HasCapture = false;
}


END_NCBI_SCOPE
