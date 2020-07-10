/*  $Id: minimap_handler.cpp 42737 2019-04-08 18:13:09Z katargir $
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
 * Authors:  Vladimir Tereshkov, Andrey Yazhuk
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>


#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/i3dtexture.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/widgets/gl/minimap_handler.hpp>

#include <math.h>

#include <wx/utils.h>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CMinimapHandler

BEGIN_EVENT_TABLE(CMinimapHandler, wxEvtHandler)
    EVT_LEFT_DOWN(CMinimapHandler::OnLeftDown)
    EVT_LEFT_UP(CMinimapHandler::OnLeftUp)
    EVT_MOTION(CMinimapHandler::OnMotion)
    EVT_MOUSEWHEEL(CMinimapHandler::OnMouseWheel)
    EVT_MOUSE_CAPTURE_LOST(CMinimapHandler::OnMouseCaptureLost)
    EVT_KEY_DOWN(CMinimapHandler::OnKeyDown)
    EVT_KEY_UP(CMinimapHandler::OnKeyUp)
END_EVENT_TABLE()


CMinimapHandler::CMinimapHandler()
:   m_pTexture(NULL),
    m_XCoordLimit(1.0f),
    m_YCoordLimit(1.0f),
    m_Host(NULL),
    m_State(eIdle),
    m_PixPerNorm(50),
    m_MouseStart(0, 0),
    m_CurrPos(0, 0),
    m_InnerRectX(0), m_InnerRectY(0),
    m_InnerRectW(0), m_InnerRectH(0),
    m_ScaleColor(0.0f, 0.6f, 0.0f, 0.3f),
    m_TickColor(0.0f, 0.6f, 0.0f, 1.0f),
    m_RectColor(0.7f, 0.8f, 0.7f, 0.85f)
{
}


CMinimapHandler::~CMinimapHandler()
{
}


void CMinimapHandler::SetPane(CGlPane* pane)
{
    m_Pane = pane;
}


wxEvtHandler* CMinimapHandler::GetEvtHandler()
{
    return this;
}


void CMinimapHandler::SetHost(IMinimapHandlerHost* host)
{
    m_Host = host;
}


IGenericHandlerHost* CMinimapHandler::GetGenericHost()
{
    return dynamic_cast<IGenericHandlerHost*>(m_Host);
}


static int kGaugeMaxDim = 160;
static float kMinInnerRectDim = 5.0f;

void CMinimapHandler::Render(CGlPane& pane)
{
    CGlAttrGuard  AttrGuard(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_HINT_BIT);

    switch(m_State) {
    case eReadyMinimap:
    case eMinimap:  {
        if ( ! m_pTexture) {
            m_pTexture = m_Host->MMHH_GetTexture(m_XCoordLimit, m_YCoordLimit);
            if (m_pTexture) {
                m_pTexture->MakeCurrent();
            }
        }
        x_RenderMinimap(pane);
        break;
    }
    default:
        break;
    }
}


void CMinimapHandler::x_RenderMinimap(CGlPane& pane)
{
    IRender& gl = GetGl();

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    const TModelRect & visRect = pane.GetVisibleRect();
    const TModelRect & modRect = pane.GetModelLimitsRect();

    int vw = pane.GetViewport().Width();
    int vh = pane.GetViewport().Height();    
    
    int ext_w;
    int ext_h;

    if (vw > vh) {
        ext_w = kGaugeMaxDim;
        ext_h = kGaugeMaxDim*(((double)vh)/(double)vw);       
    }
    else  {
        ext_h = kGaugeMaxDim;
        ext_w = kGaugeMaxDim*(((double)vw)/(double)vh);
    } 

     m_scalex = (int)((double)ext_w) / (double)modRect.Width();
     m_scaley = (int)((double)ext_h) / (double)modRect.Height();

	// Set inner rect cursor size (but don't let it get too small):
	float w_h = ((float)vw)/((float)vh);
	m_InnerRectW = std::max((int) (visRect.Width()  * m_scalex), 
							(int) (kMinInnerRectDim * w_h) );
	m_InnerRectH = std::max((int) (visRect.Height() * m_scaley), 
							(int) (kMinInnerRectDim * 1.0f/w_h) );

    m_InnerRectX = (int) ((visRect.Left() - modRect.Left()) * m_scalex);
    m_InnerRectY = (int) ((-visRect.Top() + modRect.Top()) * m_scaley);

    pane.OpenPixels();

    int base_x = m_MarkerPos.x;
    int base_y = m_MarkerPos.y;
    int dim_y  = m_Host->MMHH_GetVPPosByY(0);

    // external rect
    int yb = dim_y - (base_y + ext_h/2);
    int yt = dim_y - (base_y - ext_h/2);
    int xl = base_x - ext_w / 2;
    int xr = base_x + ext_w / 2;
    m_Minimap.Init(xl, dim_y-yt, xr, dim_y-yb);
	
    // internal rect
    int yb_i = std::max(yt-m_InnerRectY-m_InnerRectH, yb);
    yb_i = std::min(yb_i, yt-m_InnerRectH);
    int yt_i = yb_i + m_InnerRectH;

    int xl_i = std::max(xl + m_InnerRectX, xl);
    xl_i = std::min(xl_i, xr-m_InnerRectW);
    int xr_i = xl_i + m_InnerRectW;
    m_MiniCursor.Init(xl_i, dim_y-yt_i, xr_i, dim_y-yb_i);

    // external/internal rects
    gl.Enable(GL_TEXTURE_2D);
    // texture should always be there (but check):
    if (m_pTexture) 
        m_pTexture->MakeCurrent();

    gl.ColorC(m_RectColor);
    gl.Begin(GL_QUADS);
        gl.TexCoord2f(0, 0); 
        gl.Vertex2d(xl, yb);
        gl.TexCoord2f(m_XCoordLimit, 0); 
        gl.Vertex2d(xr, yb);
        gl.TexCoord2f(m_XCoordLimit, m_YCoordLimit); 
        gl.Vertex2d(xr, yt);
        gl.TexCoord2f(0, m_YCoordLimit); 
        gl.Vertex2d(xl, yt);        
    gl.End();
    gl.Disable(GL_TEXTURE_2D);

    gl.ColorC(m_ScaleColor);
    gl.Begin(GL_QUADS);
        gl.Vertex2d(xl_i, yb_i);
        gl.Vertex2d(xr_i, yb_i);
        gl.Vertex2d(xr_i, yt_i);
        gl.Vertex2d(xl_i, yt_i);
    gl.End();

    gl.LineWidth(1.0);
    gl.ColorC(m_TickColor);
    gl.Begin(GL_LINE_LOOP);
        gl.Vertex2d(xl_i, yb_i);
        gl.Vertex2d(xr_i, yb_i);
        gl.Vertex2d(xr_i, yt_i);
        gl.Vertex2d(xl_i, yt_i);
    gl.End();

    gl.Begin(GL_LINE_LOOP);
        gl.Vertex2d(xl, yb);
        gl.Vertex2d(xr, yb);
        gl.Vertex2d(xr, yt);
        gl.Vertex2d(xl, yt);
    gl.End();

    pane.Close();

    CGlUtils::CheckGlError();
}


////////////////////////////////////////////////////////////////////////////////
/// event handlers
bool sIsMiniMapMode()
{
    return wxGetKeyState(wxKeyCode('M'));
}


void CMinimapHandler::OnLeftDown(wxMouseEvent& event)
{
    EState ready_st = eIdle, active_st = eIdle;
    if(sIsMiniMapMode())    {
        ready_st = eReadyMinimap;
        active_st = eMinimap;
    }

    if(ready_st != eIdle)    {
        wxPoint ms_pos = event.GetPosition();
        x_SwitchToReadyState(ready_st, ms_pos);
        x_SwithToActiveState(active_st, ms_pos);
        x_OnSelectCursor(ms_pos);
    } else {
        event.Skip();
    }
}


void CMinimapHandler::OnMotion(wxMouseEvent& event)
{
    m_LastMouse = event.GetPosition();

    if(event.Dragging())    {
        switch(m_State) {
        case eMinimap: {
            wxPoint ms_pos = event.GetPosition();

			// More intuitive to just have one mode of operation?
            //if (m_Substate == eNormal){
               // x_OnChangePan(ms_pos);
           // } 
			//if (m_Substate == eCursor){
                int x = ms_pos.x - m_MiniCursor.Left() - m_MiniCursor.Width() / 2;
                int y = ms_pos.y - m_MiniCursor.Bottom() - m_MiniCursor.Height() / 2;
                x_OnChangeZoomRectPan(x, -y);
            //}
            x_OnSelectCursor(ms_pos);
            break;
        }
        default:
            break;
        }
    } else {
        if(m_State != eIdle )    {
            wxPoint ms_pos = event.GetPosition();
            x_OnSelectCursor(ms_pos);
        } else {
            event.Skip();
        }
    }
}


void CMinimapHandler::x_OnChangePan(const wxPoint& ms_pos)
{
	m_CurrPos = ms_pos;

    int y1 = m_Host->MMHH_GetVPPosByY(m_MouseStart.y);
    int y2 = m_Host->MMHH_GetVPPosByY(m_CurrPos.y);

    TModelUnit m_x1 = m_Pane->UnProjectX(m_MouseStart.x);
    TModelUnit m_y1 = m_Pane->UnProjectY(y1);
    TModelUnit m_x2 = m_Pane->UnProjectX(m_CurrPos.x);
    TModelUnit m_y2 = m_Pane->UnProjectY(y2);

    m_Host->MMHH_Scroll(m_x1 - m_x2, m_y1 - m_y2);
    GetGenericHost()->GHH_Redraw();

    m_MouseStart = m_CurrPos;
}


void CMinimapHandler::OnLeftUp(wxMouseEvent& event)
{
    switch(m_State) {
    case eMinimap:
    case eReadyMinimap: {
        wxPoint ms_pos = event.GetPosition();
        if (m_Substate == eJumpTo)  {
            int x = ms_pos.x - m_MiniCursor.Left() - m_MiniCursor.Width() / 2;
            int y = ms_pos.y - m_MiniCursor.Bottom() - m_MiniCursor.Height() / 2;
            x_OnChangeZoomRectPan(x, -y);
        }
        x_OnEndMinimap(eReadyMinimap);
        x_OnSelectCursor(ms_pos);
        return;
    }
    default:
        break;
    }
    event.Skip();
}


void CMinimapHandler::OnMouseWheel(wxMouseEvent& event)
{
    if(sIsMiniMapMode()) {
        wxPoint ms_pos = event.GetPosition();
        switch(m_State) {
        case eIdle:
            x_SwitchToReadyState(eReadyMinimap, ms_pos);
            // continue
        case eReadyMinimap: {
            m_WheelTotalShift = 0;
            x_SwithToActiveState(eMinimap, ms_pos);
        } // continue
        case eMinimap: {
            m_WheelTotalShift += event.GetWheelRotation();
            //x_OnChangeScale(m_WheelTotalShift);
            // updating shift after clipping with Min and Max norm values
            m_WheelTotalShift = (int) ((m_StartNorm - m_CurrNorm) * m_PixPerNorm);
            break;
        }
        default:
            _ASSERT(false);
            break;
        }
        x_OnSelectCursor(ms_pos);
    } else {
        event.Skip();
    }
}


void CMinimapHandler::OnKeyDown(wxKeyEvent& event)
{
    if(m_State == eIdle)    {
        EState ready_st = sIsMiniMapMode() ? eReadyMinimap : eIdle;
        if(ready_st != eIdle)   {
            
            // On mac, event object in OnKeyDown does not have correct mouse position.
            x_SwitchToReadyState(ready_st, event.GetPosition());
            x_OnSelectCursor(event.GetPosition());
        }
    }
    if(m_State == eIdle)    {
        event.Skip();
    }
}


void CMinimapHandler::OnKeyUp(wxKeyEvent& event)
{
    if(! sIsMiniMapMode())  {
        switch(m_State) {
        case    eReadyMinimap:
        case    eMinimap: {
            x_OnEndMinimap(eIdle);
            m_State = eIdle;
            m_pTexture = NULL;
            x_OnSelectCursor(event.GetPosition());
            return;
        }
        default:
            break;
        }
    }
    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
/// Signal handlers

void CMinimapHandler::x_SwitchToReadyState(EState new_state, const wxPoint& ms_pos)
{
    _ASSERT(m_Host);
    if(m_State != new_state)   {
        m_State = new_state;

        // ms_pos doesn't work on mac for keydown events (large negative value).
        m_CurrPos = m_MarkerPos = m_LastMouse;

        if(m_State == eReadyMinimap)  {
            m_CurrNorm = m_StartNorm =
                x_ScaleToNorm(m_Host->MMHH_GetScale(IMinimapHandlerHost::eCurrent));

            m_MinNorm = x_ScaleToNorm(m_Host->MMHH_GetScale(IMinimapHandlerHost::eMin));
            m_MaxNorm = x_ScaleToNorm(m_Host->MMHH_GetScale(IMinimapHandlerHost::eMax));
        }
        GetGenericHost()->GHH_Redraw();
    }
}


void CMinimapHandler::x_SwithToActiveState(EState state, const wxPoint& ms_pos)
{
    _ASSERT(m_Host);

    m_State = state;
    m_MouseStart = ms_pos;

    m_ptStart = m_Pane->UnProject(m_MouseStart.x, m_Host->MMHH_GetVPPosByY(m_MouseStart.y));
    GetGenericHost()->GHH_CaptureMouse();
}


void CMinimapHandler::x_OnChangeZoomRectPan(int x, int y)
{
    m_Host->MMHH_Scroll(x / m_scalex, y / m_scaley);
    GetGenericHost()->GHH_Redraw();
}


void CMinimapHandler::x_OnEndMinimap(EState new_state)
{
    _ASSERT(new_state != eMinimap);

    m_State = new_state;
    m_StartNorm = m_CurrNorm;

    m_Host->MMHH_EndOp();
    GetGenericHost()->GHH_Redraw();
    GetGenericHost()->GHH_ReleaseMouse();
}


void CMinimapHandler::x_OnSelectCursor(const wxPoint& ms_pos)
{
    switch(m_State)    {
    case eIdle:
        m_CursorId = wxCURSOR_ARROW;
        break;
    case eReadyMinimap:
    case eMinimap:  {
        m_Substate = x_GetSubstate(ms_pos);
        switch (m_Substate){
        case eJumpTo:
            m_CursorId = wxCURSOR_HAND;
            break;
        default:
            m_CursorId = wxCURSOR_SIZING;
            break;
        }
        break;
    }
    default:
        break;
    }
}


TModelUnit  CMinimapHandler::x_ScaleToNorm(TModelUnit scale) const
{
    return log(scale);
}


TModelUnit  CMinimapHandler::x_NormToScale(TModelUnit norm) const
{
    return exp(norm);
}


CMinimapHandler::TSubstate CMinimapHandler::x_GetSubstate(const wxPoint& ms_pos)
{
    int x = ms_pos.x;
    int y = ms_pos.y;

    if ((y > m_MiniCursor.Bottom()  &&  y < m_MiniCursor.Top())  &&
        (x > m_MiniCursor.Left()  &&  x < m_MiniCursor.Right()))    {
        return eCursor;
    }
    else if ((y > m_Minimap.Bottom()  &&  y < m_Minimap.Top()) &&
            (x > m_Minimap.Left()  &&  x < m_Minimap.Right()))  {
        return eJumpTo;
    }
    return eNormal;
}


void CMinimapHandler::OnMouseCaptureLost(wxMouseCaptureLostEvent& evt)
{
    // should we do anything here?
}


END_NCBI_SCOPE
